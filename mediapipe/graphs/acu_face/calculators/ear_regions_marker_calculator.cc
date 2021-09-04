// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <vector>
#include <unordered_map>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgcodecs_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gl_simple_shaders.h"
#include "mediapipe/gpu/shader_util.h"
#include "mediapipe/gpu/gl_shader_helper.pb.h"

#include "GLPipeline/Texture.h"

using namespace cv;

namespace {
	enum { ATTRIB_VERTEX, ATTRIB_TEXTURE_POSITION, NUM_ATTRIBUTES };

	constexpr char kGpuBufferTag[] = "IMAGE_GPU";
	constexpr char kEarNormRectsTag[] = "NORM_RECTS_EAR";
	constexpr char kEarLandMarksVectorTag[] = "VECTOR_EAR";

	const float kTemplateLandmarks[110]={
		205.0f, 100.0f, 175.0f, 61.0f, 140.0f, 28.0f, 97.0f, 17.0f, 57.0f, 37.0f, 34.0f, 71.0f, 23.0f, 109.0f, 17.0f, 151.0f, 16.0f, 193.0f, 15.0f, 235.0f, 14.0f, 272.0f, 23.0f, 312.0f, 37.0f, 342.0f, 51.0f, 378.0f, 76.0f, 412.0f, 99.0f, 435.0f, 128.0f, 458.0f, 160.0f, 465.0f, 191.0f, 462.0f, 210.0f, 444.0f, 174.0f, 208.0f, 176.0f, 171.0f, 171.0f, 129.0f, 152.0f, 95.0f, 129.0f, 71.0f, 96.0f, 59.0f, 66.0f, 73.0f, 49.0f, 106.0f, 42.0f, 143.0f, 41.0f, 180.0f, 39.0f, 217.0f, 37.0f, 248.0f, 41.0f, 284.0f, 55.0f, 314.0f, 74.0f, 344.0f, 188.0f, 238.0f, 175.0f, 265.0f, 173.0f, 297.0f, 186.0f, 321.0f, 189.0f, 347.0f, 157.0f, 340.0f, 138.0f, 306.0f, 105.0f, 299.0f, 77.0f, 281.0f, 64.0f, 251.0f, 71.0f, 219.0f, 89.0f, 190.0f, 114.0f, 183.0f, 139.0f, 175.0f, 165.0f, 172.0f, 48.0f, 200.0f, 60.0f, 162.0f, 72.0f, 135.0f, 80.0f, 103.0f, 98.0f, 80.0f
	};
	constexpr char kRecolorFileName[] = "crop_ear_recolor_ato.jpg";
	const int kTemplateWidth = 226;
	const int kTemplateHeight = 475;
	const int kLandmarkNum = 55;
	const float kBlendAlpha = 0.7f;

	//
	void make_delaunay(int width, int height, const float* landmarks, std::vector<std::vector<int>>& trianlge_lst){
		cv::Rect rect(0, 0, width, height);
		cv::Subdiv2D subdiv(rect);
		std::vector<cv::Point2i> points(kLandmarkNum);
		for(int i=0; i<kLandmarkNum; i++){
			points[i] = cv::Point2i(landmarks[2*i], landmarks[2*i+1]);
		}
		
		//create a map
		std::unordered_map<std::string, int> points_dict;
		auto encode_str = [](cv::Point2i p){
			return std::to_string(p.x) + "#" + std::to_string(p.y);
		};

		int id = 0;
		for(auto it = points.begin(); it!=points.end(); it++){
			subdiv.insert(*it);
			points_dict[encode_str(*it)]=id++;
		}

		//draw_delaunay
		std::vector<cv::Vec6f> triangleList;
		subdiv.getTriangleList(triangleList);
		std::vector<cv::Point2i> pt(3);

		for(auto i=0; i<triangleList.size();i++){
			auto t = triangleList[i];
			pt[0] = cv::Point2i(cvRound(t[0]), cvRound(t[1]));
			pt[1] = cv::Point2i(cvRound(t[2]), cvRound(t[3]));
			pt[2] = cv::Point2i(cvRound(t[4]), cvRound(t[5]));

			if(rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2])){
				trianlge_lst.push_back(
					std::vector<int>{points_dict[encode_str(pt[0])], points_dict[encode_str(pt[1])], points_dict[encode_str(pt[2])]}
				);
			}
		}
	}
	// Apply affine transform calculated using srcTri and dstTri to src
void applyAffineTransform(Mat &warpImage, Mat &src, std::vector<Point2f> &srcTri, std::vector<Point2f> &dstTri){
    
    // Given a pair of triangles, find the affine transform.
    Mat warpMat = getAffineTransform( srcTri, dstTri );
    
    // Apply the Affine Transform just found to the src image
    warpAffine( src, warpImage, warpMat, warpImage.size(), INTER_LINEAR, BORDER_REFLECT_101);
}

// Warps and alpha blends triangular regions from img1 and img2 to img
void morphTriangle(Mat &img1, Mat &img2, Mat &img, std::vector<Point2f> &t1, std::vector<Point2f> &t2, std::vector<Point2f> &t, double alpha){
    
    // Find bounding rectangle for each triangle
    Rect r = boundingRect(t);
    Rect r1 = boundingRect(t1);
    Rect r2 = boundingRect(t2);
    
    // Offset points by left top corner of the respective rectangles
    std::vector<Point2f> t1Rect, t2Rect, tRect;
    std::vector<Point> tRectInt;
    for(int i = 0; i < 3; i++)
    {
        tRect.push_back( Point2f( t[i].x - r.x, t[i].y -  r.y) );
        tRectInt.push_back( Point(t[i].x - r.x, t[i].y - r.y) ); // for fillConvexPoly
        
        t1Rect.push_back( Point2f( t1[i].x - r1.x, t1[i].y -  r1.y) );
        t2Rect.push_back( Point2f( t2[i].x - r2.x, t2[i].y - r2.y) );
    }
    
    // Get mask by filling triangle
    Mat mask = Mat::zeros(r.height, r.width, CV_32FC3);
    fillConvexPoly(mask, tRectInt, Scalar(1.0, 1.0, 1.0), 16, 0);
    
    // Apply warpImage to small rectangular patches
    Mat img1Rect, img2Rect;
    img1(r1).copyTo(img1Rect);
    img2(r2).copyTo(img2Rect);
    
    Mat warpImage1 = Mat::zeros(r.height, r.width, img1Rect.type());
    Mat warpImage2 = Mat::zeros(r.height, r.width, img2Rect.type());
    
    applyAffineTransform(warpImage1, img1Rect, t1Rect, tRect);
    applyAffineTransform(warpImage2, img2Rect, t2Rect, tRect);
    
    // Alpha blend rectangular patches
    Mat imgRect = (1.0 - alpha) * warpImage1 + alpha * warpImage2;
    
    // Copy triangular region of the rectangular patch to the output image
    multiply(imgRect,mask, imgRect);
    multiply(img(r), Scalar(1.0,1.0,1.0) - mask, img(r));
    img(r) = img(r) + imgRect;
}


}  // namespace

namespace mediapipe {

class EarRegionsMarkerCalculator : public CalculatorBase {
public:
	EarRegionsMarkerCalculator() = default;
	~EarRegionsMarkerCalculator() override = default;

	static ::mediapipe::Status GetContract(CalculatorContract* cc);

	::mediapipe::Status Open(CalculatorContext* cc) override;
	::mediapipe::Status Process(CalculatorContext* cc) override;
	::mediapipe::Status Close(CalculatorContext* cc) override;

private:
	::mediapipe::Status LoadOptions(CalculatorContext* cc);
	::mediapipe::Status InitGpu(CalculatorContext* cc);
	::mediapipe::Status RenderGpu(CalculatorContext* cc);

	void GlRender();

	void prepare_dest_image_landmarks(int width, int height, NormalizedLandmarkList& input_landmarks, float* & output_landmarks);
	void morph_recolor_image(cv::Mat& dest_img, const float* dest_landmarks);
	void project_landmarks(NormalizedRect& input_rect, NormalizedLandmarkList& input_landmarks, NormalizedLandmarkList& output_landmarks);

	bool initialized_;
	mediapipe::GlCalculatorHelper gpu_helper_;
	GLuint program_ = 0;
	std::string m_asset_path;
	cv::Mat m_img_recolor;

	std::vector<cv::Mat> m_morph_imgs, m_mask_imgs;
	//normalized coord of the left bottom corner + width and height
	float m_dest_rects[2][4];
	// Texture *morph_left, *morph_right, *mask_left, *mask_right;
	int gid = 0;
};
REGISTER_CALCULATOR(EarRegionsMarkerCalculator);

// static
::mediapipe::Status EarRegionsMarkerCalculator::GetContract(CalculatorContract* cc) {
    if (cc->Inputs().HasTag(kGpuBufferTag)) 
        cc->Inputs().Tag(kGpuBufferTag).Set<mediapipe::GpuBuffer>();
    if(cc->Inputs().HasTag(kEarNormRectsTag)) 
        cc->Inputs().Tag(kEarNormRectsTag).Set<std::vector<NormalizedRect>>();
    if(cc->Inputs().HasTag(kEarLandMarksVectorTag))
        cc->Inputs().Tag(kEarLandMarksVectorTag).Set<std::vector<::mediapipe::NormalizedLandmarkList>>();

	if (cc->Outputs().HasTag(kGpuBufferTag))
		cc->Outputs().Tag(kGpuBufferTag).Set<mediapipe::GpuBuffer>();
	if(cc->Outputs().HasTag(kEarLandMarksVectorTag))
		cc->Outputs().Tag(kEarLandMarksVectorTag).Set<std::vector<::mediapipe::NormalizedLandmarkList>>();
    
 	  MP_RETURN_IF_ERROR(mediapipe::GlCalculatorHelper::UpdateContract(cc));

    return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRegionsMarkerCalculator::Open(CalculatorContext* cc) {
	cc->SetOffset(TimestampDiff(0));
	if (cc->Inputs().HasTag(kGpuBufferTag))MP_RETURN_IF_ERROR(gpu_helper_.Open(cc));
	initialized_ = false;

	MP_RETURN_IF_ERROR(LoadOptions(cc));

	return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRegionsMarkerCalculator::Process(CalculatorContext* cc) {
    MP_RETURN_IF_ERROR(
    gpu_helper_.RunInGlContext([this, &cc]() -> ::mediapipe::Status {
        if (!initialized_) {
            MP_RETURN_IF_ERROR(InitGpu(cc));
            initialized_ = true;
        }
        MP_RETURN_IF_ERROR(RenderGpu(cc));
        return ::mediapipe::OkStatus();
    }));
    return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRegionsMarkerCalculator::RenderGpu(CalculatorContext* cc) {
	const Packet& input_packet = cc->Inputs().Tag(kGpuBufferTag).Value();
	const auto& input_buffer = input_packet.Get<mediapipe::GpuBuffer>();
	auto img_tex = gpu_helper_.CreateSourceTexture(input_buffer);
	auto dst_tex = gpu_helper_.CreateDestinationTexture(img_tex.width(), img_tex.height());
  
	std::vector<cv::Mat> dest_imgs;
	if (cc->Inputs().HasTag(kEarNormRectsTag) && !cc->Inputs().Tag(kEarNormRectsTag).IsEmpty()) {
		const auto& rects = cc->Inputs().Tag(kEarNormRectsTag).Get<std::vector<NormalizedRect>>();
		int id = 0;
		for(auto rect:rects){
			if(rect.width() == 0)continue;

			float xoff = rect.x_center() - 0.5f*rect.width();
			float yoff = rect.y_center() - 0.5f*rect.height();
			m_dest_rects[id][0]=xoff; m_dest_rects[id][1]=yoff; m_dest_rects[id][2]=rect.width();m_dest_rects[id][3]=rect.height();
			id++;
			auto frame_width = rect.width()* img_tex.width();
			auto frame_height = rect.height()* img_tex.height();
			
			std::unique_ptr<ImageFrame> frame = absl::make_unique<ImageFrame>(
				ImageFormatForGpuBufferFormat(input_buffer.format()), 
				frame_width, frame_height, ImageFrame::kGlDefaultAlignmentBoundary);

			gpu_helper_.BindFramebuffer(img_tex);
			const auto info = GlTextureInfoForGpuBufferFormat(input_buffer.format(), 0);
			glReadPixels(xoff * img_tex.width(), 
						yoff * img_tex.height(),
						frame_width, 
						frame_height, 
						info.gl_format,
						info.gl_type, frame->MutablePixelData());
			glFlush();
			dest_imgs.push_back( mediapipe::formats::MatView(frame.release()));
			// cv::imwrite( "debug_out/crop_" + std::to_string(gid++) +".png" , dest_imgs.back());
		}
		if(dest_imgs.empty()){
			cc->Outputs().Tag(kGpuBufferTag).AddPacket(input_packet);
			img_tex.Release();
			dst_tex.Release();
			return ::mediapipe::OkStatus();
		}
	}

	//prepare ear landmarks
	std::vector<float*> dest_points;
    auto output_landmark_vec = absl::make_unique<std::vector<NormalizedLandmarkList>>();

	m_morph_imgs.clear();
	m_mask_imgs.clear();

	if(!cc->Inputs().Tag(kEarLandMarksVectorTag).IsEmpty()
    && !cc->Inputs().Tag(kEarNormRectsTag).IsEmpty()){
		auto& landmarks_vec = cc->Inputs().Tag(kEarLandMarksVectorTag).Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
    	auto& rect_vec = cc->Inputs().Tag(kEarNormRectsTag).Get<std::vector<NormalizedRect>>();
    	int num = landmarks_vec.size();
		for(int i=0; i<num; i++){
			NormalizedLandmarkList output_landmarks;
			auto srect = rect_vec[i];
			auto slandmarks = landmarks_vec[i];
			
			if(dest_imgs[i].cols > 0 && dest_imgs[i].rows>0){
				dest_points.push_back(new float[2*kLandmarkNum]);
				prepare_dest_image_landmarks(dest_imgs[i].cols, dest_imgs[i].rows, slandmarks, dest_points.back());
				morph_recolor_image(dest_imgs[i], dest_points[i]);
			}

			project_landmarks(srect, slandmarks, output_landmarks);
			output_landmark_vec->emplace_back(output_landmarks);
		}

  }
  		int width = m_morph_imgs[0].size().width;
		int height = m_morph_imgs[0].size().height;

		std::unique_ptr<ImageFrame> morph_frame = absl::make_unique<ImageFrame>(
			ImageFormat::SRGB, width, height,
			ImageFrame::kGlDefaultAlignmentBoundary);
		m_morph_imgs[0].copyTo(formats::MatView(morph_frame.get()));
		auto morph_tex_right = gpu_helper_.CreateSourceTexture(*morph_frame.release());

		std::unique_ptr<ImageFrame> mask_frame = absl::make_unique<ImageFrame>(
			ImageFormat::GRAY8, width, height,
			ImageFrame::kGlDefaultAlignmentBoundary);
		m_mask_imgs[0].copyTo(formats::MatView(mask_frame.get()));
		auto mask_tex_right = gpu_helper_.CreateSourceTexture(*mask_frame.release());

	  // Run recolor shader on GPU.
  {
    gpu_helper_.BindFramebuffer(dst_tex);  // GL_TEXTURE0

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(img_tex.target(), img_tex.name());
	
	if(!m_morph_imgs.empty()){

		glActiveTexture(GL_TEXTURE1);
    	glBindTexture(morph_tex_right.target(), morph_tex_right.name());

		glActiveTexture(GL_TEXTURE2);
    	glBindTexture(mask_tex_right.target(), mask_tex_right.name());
	}

    GlRender();
	glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

  	glUniform4f(glGetUniformLocation(program_, "u_rect_right"), m_dest_rects[0][0], m_dest_rects[0][1], m_dest_rects[0][2], m_dest_rects[0][3]);
  	glUniform4f(glGetUniformLocation(program_, "u_rect_left"), m_dest_rects[1][0], m_dest_rects[1][1], m_dest_rects[1][2], m_dest_rects[1][3]);
	glFlush();
  }
	
	// Send result image in GPU packet.
	auto output = dst_tex.GetFrame<mediapipe::GpuBuffer>();
	cc->Outputs().Tag(kGpuBufferTag).Add(output.release(), cc->InputTimestamp());
	
	cc->Outputs().Tag(kEarLandMarksVectorTag).Add(output_landmark_vec.release(), cc->InputTimestamp());
	// Cleanup
	img_tex.Release();
	dst_tex.Release();
	

	return ::mediapipe::OkStatus();
}
void EarRegionsMarkerCalculator::GlRender() {
  static const GLfloat square_vertices[] = {
      -1.0f, -1.0f,  // bottom left
      1.0f,  -1.0f,  // bottom right
      -1.0f, 1.0f,   // top left
      1.0f,  1.0f,   // top right
  };
  static const GLfloat texture_vertices[] = {
      0.0f, 0.0f,  // bottom left
      1.0f, 0.0f,  // bottom right
      0.0f, 1.0f,  // top left
      1.0f, 1.0f,  // top right
  };

  // program
  glUseProgram(program_);

  // vertex storage
  GLuint vbo[2];
  glGenBuffers(2, vbo);
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // vbo 0
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), square_vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(ATTRIB_VERTEX);
  glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, nullptr);

  // vbo 1
  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), texture_vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(ATTRIB_TEXTURE_POSITION);
  glVertexAttribPointer(ATTRIB_TEXTURE_POSITION, 2, GL_FLOAT, 0, 0, nullptr);

  // draw
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // cleanup
  glDisableVertexAttribArray(ATTRIB_VERTEX);
  glDisableVertexAttribArray(ATTRIB_TEXTURE_POSITION);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(2, vbo);
}

::mediapipe::Status EarRegionsMarkerCalculator::LoadOptions(CalculatorContext* cc) {
	const auto& options = cc->Options<::mediapipe::glShaderHelperOptions>();
	m_asset_path = options.shader_res_path();
	m_img_recolor = cv::imread(m_asset_path+std::string(kRecolorFileName), -1);
  	return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRegionsMarkerCalculator::InitGpu(CalculatorContext* cc) {

  const GLint attr_location[NUM_ATTRIBUTES] = {
      ATTRIB_VERTEX,
      ATTRIB_TEXTURE_POSITION,
  };
  const GLchar* attr_name[NUM_ATTRIBUTES] = {
      "position",
      "texture_coordinate",
  };

  // A shader to blend a color onto an image where the mask > 0.
  // The blending is based on the input image luminosity.
  const std::string frag_src = R"(
  #if __VERSION__ < 130
    #define in varying
  #endif  // __VERSION__ < 130

  #ifdef GL_ES
    #define fragColor gl_FragColor
    precision highp float;
  #else
    #define lowp
    #define mediump
    #define highp
    #define texture2D texture
    out vec4 fragColor;
  #endif  // defined(GL_ES)

    in vec2 sample_coordinate;
    uniform sampler2D frame;
    uniform sampler2D morph_right;//morph_left
    uniform sampler2D mask_right;//mask_left

	uniform vec4 u_rect_right;
	uniform vec4 u_rect_left;

	bool inRect(vec2 lb, vec2 sz, vec2 pos){
		if(sz.x < .0 || sz.y < .0) return false;
		return (pos.x > lb.x && pos.x < lb.x+sz.x && pos.y > lb.y && pos.y < lb.y+sz.y);
	}
    void main() {
		vec4 fore_value = texture2D(frame, sample_coordinate);

		// if(inRect(u_rect_left.xy, u_rect_left.zw, sample_coordinate)){
		// 	vec2 sp = sample_coordinate - u_rect_left.xy;
		// 	float maskv = texture2D(mask_left, sp).r;

		// 	fragColor = texture2D(morph_left, sample_coordinate) * maskv + fore_value * (1.0-maskv);
		// 	return;
		// }
		if(inRect(u_rect_right.xy, u_rect_right.zw, sample_coordinate)){
			vec2 sp = (sample_coordinate - u_rect_right.xy) / u_rect_right.zw;

			float maskv = texture2D(mask_right, sp);
			// fragColor = vec4(maskv, .0,.0,1.0);
			
			fragColor = vec4(texture2D(morph_right, sp).rgb, 1.0) * maskv + fore_value * (1.0-maskv);
			// fragColor = vec4(1.0,1.0,.0,1.0);
			return;
		}

		fragColor = fore_value;
    }
  )";

  // shader program and params
  mediapipe::GlhCreateProgram(mediapipe::kBasicVertexShader, frag_src.c_str(),
                              NUM_ATTRIBUTES, &attr_name[0], attr_location,
                              &program_);
  RET_CHECK(program_) << "Problem initializing the program.";
  glUseProgram(program_);
  glUniform1i(glGetUniformLocation(program_, "frame"), 0);
//   glUniform1i(glGetUniformLocation(program_, "morph_left"), 2);
  glUniform1i(glGetUniformLocation(program_, "morph_right"), 1);
//   glUniform1i(glGetUniformLocation(program_, "mask_left"), 4);
  glUniform1i(glGetUniformLocation(program_, "mask_right"), 2);

  return ::mediapipe::OkStatus();
}


::mediapipe::Status EarRegionsMarkerCalculator::Close(CalculatorContext* cc) {
	gpu_helper_.RunInGlContext([this] {
		if (program_) glDeleteProgram(program_);
		program_ = 0;
	});
	return ::mediapipe::OkStatus();
}
void EarRegionsMarkerCalculator::prepare_dest_image_landmarks(int width, int height, 
																NormalizedLandmarkList& input_landmarks, 
																float* & points){
	for(int i=0; i<input_landmarks.landmark_size();i++){
		const NormalizedLandmark& landmark = input_landmarks.landmark(i);				
		points[2*i] = landmark.x() * width;
		points[2*i+1] = landmark.y() * height;

		if(points[2*i] >= width) points[2*i]-=1.f;
		points[2*i] = fmax(.0f, points[2*i]);
		if(points[2*i+1] >= height) points[2*i+1]-=1.f;
		points[2*i+1] = fmax(.0f, points[2*i+1]);
	}
}
void EarRegionsMarkerCalculator::morph_recolor_image(cv::Mat& dest_img, const float* dest_landmarks){
	cv::Mat img_recolor_resized;
	cv::resize(m_img_recolor, img_recolor_resized, dest_img.size());

	img_recolor_resized.convertTo(img_recolor_resized, CV_32F);
    dest_img.convertTo(dest_img, CV_32F);

	cv::Mat imgMorph = Mat::zeros(dest_img.size(), CV_32FC3);

	//resize template landmarks based on dest shape
	float Rx = (float)dest_img.cols/kTemplateWidth, Ry = (float)dest_img.rows / kTemplateHeight;

	std::vector<cv::Point2f> points1(kLandmarkNum), points2(kLandmarkNum), points(kLandmarkNum);

	for(int n=0; n<kLandmarkNum; n++){
		points1[n] = cv::Point2f(Rx * kTemplateLandmarks[2*n], Ry * kTemplateLandmarks[2*n+1]);
		points2[n] = cv::Point2f(dest_landmarks[2*n], dest_landmarks[2*n+1]);
		
		float mx = (1.0f - kBlendAlpha) * points1[n].x + kBlendAlpha * points2[n].x;
		float my =  (1.0f - kBlendAlpha) * points1[n].y + kBlendAlpha * points2[n].y;
		points[n] = cv::Point2f(mx, my);
	}

	std::vector<std::vector<int>> trianlge_lst;
	make_delaunay(dest_img.cols, dest_img.rows, dest_landmarks, trianlge_lst);

	for(std::vector<int>&tlst : trianlge_lst){
		int x = tlst[0], y=tlst[1], z=tlst[2];
		std::vector<cv::Point2f> t1, t2, t;
		t1.push_back( points1[x] ); t1.push_back( points1[y] ); t1.push_back( points1[z] );
		t2.push_back( points2[x] ); t2.push_back( points2[y] ); t2.push_back( points2[z] );
		t.push_back( points[x] ); t.push_back( points[y] ); t.push_back( points[z] );
		
		morphTriangle(img_recolor_resized, dest_img, imgMorph, t1, t2, t, kBlendAlpha);
	}

	imgMorph.convertTo(imgMorph, CV_8UC3);

	m_morph_imgs.push_back(imgMorph);
	

	cv::Mat morph_gray, thresh;
	cv::cvtColor(imgMorph, morph_gray, cv::COLOR_BGR2GRAY);
	cv::threshold(morph_gray, thresh, 50, 255, cv::THRESH_BINARY);

	thresh.convertTo(thresh, CV_8U);
	m_mask_imgs.push_back(thresh);

	// cv::imwrite( "debug_out/morph_" + std::to_string(gid) +".png" , imgMorph);
	// cv::imwrite( "debug_out/mask_" + std::to_string(gid) +".png" , thresh);
	// gid++;
	  //create morph and mask texture

	

}
void EarRegionsMarkerCalculator::project_landmarks(NormalizedRect& input_rect, NormalizedLandmarkList& input_landmarks, NormalizedLandmarkList& output_landmarks){
    float max_x = -10000, min_x = 10000;
    float max_y = -10000, min_y = 10000;

    for (int i = 0; i < input_landmarks.landmark_size(); ++i) {
      const NormalizedLandmark& landmark = input_landmarks.landmark(i);
      NormalizedLandmark* new_landmark = output_landmarks.add_landmark();

      const float x = landmark.x() - 0.5f;
      const float y = landmark.y() - 0.5f;
      const float angle = .0f;
          // options.ignore_rotation() ? 0 : input_rect.rotation();
      float new_x = std::cos(angle) * x - std::sin(angle) * y;
      float new_y = std::sin(angle) * x + std::cos(angle) * y;

      new_x = new_x * input_rect.width() + input_rect.x_center();
      new_y = new_y * input_rect.height() + input_rect.y_center();
      const float new_z =
          landmark.z() * input_rect.width();  // Scale Z coordinate as X.

      new_landmark->set_x(new_x);
      new_landmark->set_y(new_y);
      new_landmark->set_z(new_z);
      
      if(x < min_x) min_x = x;
      else if(x>max_x) max_x = x;

      if(y < min_y) min_y = y;
      else if(y > max_y) max_y = y;

      // Keep visibility as is.
      new_landmark->set_visibility(landmark.visibility());
    }
}

}  // namespace mediapipe
