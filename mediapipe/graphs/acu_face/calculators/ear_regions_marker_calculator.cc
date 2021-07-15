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

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gl_simple_shaders.h"
#include "mediapipe/gpu/shader_util.h"


namespace {
	enum { ATTRIB_VERTEX, ATTRIB_TEXTURE_POSITION, NUM_ATTRIBUTES };

	constexpr char kGpuBufferTag[] = "IMAGE_GPU";
	constexpr char kEarNormRectsTag[] = "NORM_RECTS_EAR";
	constexpr char kInputEarLandMarksVectorTag[] = "VECTOR_EAR";
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

	bool initialized_;
	mediapipe::GlCalculatorHelper gpu_helper_;
	GLuint program_ = 0;
	float *lmpoints = nullptr, *right_ear_points = nullptr;

};
REGISTER_CALCULATOR(EarRegionsMarkerCalculator);

// static
::mediapipe::Status EarRegionsMarkerCalculator::GetContract(CalculatorContract* cc) {
    if (cc->Inputs().HasTag(kGpuBufferTag)) 
        cc->Inputs().Tag(kGpuBufferTag).Set<mediapipe::GpuBuffer>();
    if(cc->Inputs().HasTag(kEarNormRectsTag)) 
        cc->Inputs().Tag(kEarNormRectsTag).Set<std::vector<NormalizedRect>>();
    if(cc->Inputs().HasTag(kInputEarLandMarksVectorTag))
        cc->Inputs().Tag(kInputEarLandMarksVectorTag).Set<std::vector<::mediapipe::NormalizedLandmarkList>>();

	if (cc->Outputs().HasTag(kGpuBufferTag))
		cc->Outputs().Tag(kGpuBufferTag).Set<mediapipe::GpuBuffer>();
 	MP_RETURN_IF_ERROR(mediapipe::GlCalculatorHelper::UpdateContract(cc));

    return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRegionsMarkerCalculator::Open(CalculatorContext* cc) {
  cc->SetOffset(TimestampDiff(0));
  if (cc->Inputs().HasTag(kGpuBufferTag))
	MP_RETURN_IF_ERROR(gpu_helper_.Open(cc));
  initialized_ = false;
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


  //prepare ear info
  
//   std::vector<faceRect> ear_rects;
//   if (cc->Inputs().HasTag(kEarNormRectsTag) && !cc->Inputs().Tag(kEarNormRectsTag).IsEmpty()) {
// 		const auto& rects = cc->Inputs().Tag(kEarNormRectsTag).Get<std::vector<NormalizedRect>>();
//     for(auto rect:rects){
// 		  ear_rects.push_back(faceRect(
//         rect.x_center(), 
//         rect.y_center(), 
//         rect.width(),
//         rect.height(), 
//         rect.rotation(), true));
//         // std::cout<<"=====rect: "<<rect.x_center()<<" "<<rect.y_center()<<std::endl;
//     }
//   }
//   bool ear_landmark_valid = false;
//   //prepare ear landmarks
// 	if(cc->Inputs().HasTag(kInputEarLandMarksVectorTag) && !cc->Inputs().Tag(kInputEarLandMarksVectorTag).IsEmpty()){
// 		auto& nlandmarks_vec = cc->Inputs().Tag(kInputEarLandMarksVectorTag).Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
// 		auto landmarks = nlandmarks_vec[0];

//     if(right_ear_points == nullptr) right_ear_points = new float[3 * landmarks.landmark_size()];
//     for(int i=0; i<landmarks.landmark_size();i++){
//         const NormalizedLandmark& landmark = landmarks.landmark(i);
//         right_ear_points[3*i] = landmark.x();right_ear_points[3*i+1] = landmark.y();right_ear_points[3*i+2] = .0f;
//         // std::cout<<right_ear_points[3*i]<<" "<<right_ear_points[3*i+1]<<std::endl;
//      }
//     //  std::cout<<"======="<<std::endl;
//     ear_landmark_valid = true; 
//   }   
//    acuGenerator::instance()->onDraw(fr, mask_full, ear_rects, land_mark_valid?lmpoints:nullptr, ear_landmark_valid?right_ear_points:nullptr);


	  // Run recolor shader on GPU.
  {
    gpu_helper_.BindFramebuffer(dst_tex);  // GL_TEXTURE0

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(img_tex.target(), img_tex.name());

    GlRender();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
  }

	// Send result image in GPU packet.
	auto output = dst_tex.GetFrame<mediapipe::GpuBuffer>();
	cc->Outputs().Tag(kGpuBufferTag).Add(output.release(), cc->InputTimestamp());

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

    void main() {
    //   vec4 weight = texture2D(mask, sample_coordinate);
    
	vec4 fore_value = texture2D(frame, sample_coordinate);

    vec4 recolor_value = vec4(1.0, 1.0, .0, 1.0);
	fragColor = mix(fore_value, recolor_value, 0.5);
    }
  )";

  // shader program and params
  mediapipe::GlhCreateProgram(mediapipe::kBasicVertexShader, frag_src.c_str(),
                              NUM_ATTRIBUTES, &attr_name[0], attr_location,
                              &program_);
  RET_CHECK(program_) << "Problem initializing the program.";
  glUseProgram(program_);
  glUniform1i(glGetUniformLocation(program_, "frame"), 1);
  return ::mediapipe::OkStatus();
}


::mediapipe::Status EarRegionsMarkerCalculator::Close(CalculatorContext* cc) {
	gpu_helper_.RunInGlContext([this] {
		if (program_) glDeleteProgram(program_);
		program_ = 0;
	});
	return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
