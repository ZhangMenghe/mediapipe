#include <cmath>
#include <vector>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"

//gpu
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gl_quad_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_point_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_cube_renderer.h"
#include "mediapipe/calculators/slam/slam_calculators.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtx/string_cast.hpp>
#include <opencv2/calib3d.hpp>

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/gpu/gl_shader_helper.pb.h"
#include "mediapipe/util/resource_util.h"


namespace mediapipe {
namespace {
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
    constexpr char kInputSLAMTag[] = "SLAM_OUT";
    constexpr char kRaycastShaderName[] = "DICOM_RAYCAST";
    void fromCV2GLM(const cv::Mat& cvmat, glm::mat4* glmmat) {
      if (cvmat.cols != 4 || cvmat.rows != 4 || cvmat.type() != CV_32FC1) {
          LOG(INFO)<<cvmat.cols<<" "<<cvmat.rows;
          return;
      }
      memcpy(glm::value_ptr(*glmmat), cvmat.data, 16 * sizeof(float));
      *glmmat = glm::transpose(*glmmat);
    }

    glm::mat4 getGLProjMatFromCV(cv::Mat mat, float width, float height, float zfar, float znear){
      float fx = mat.at<float>(0,0);  float fy = mat.at<float>(1,1);
      float cx = mat.at<float>(0,2);  float cy = mat.at<float>(1,2);
      // // LOG(INFO)<<"cm "<<fx <<" "<< fy <<" "<<cx <<" "<<cy;
      glm::mat4 m;
      m[0][0] = 2.0 * fx / width; m[0][1] = 0.0;  m[0][2] = 0.0;  m[0][3] = 0.0;
      m[1][0] = 0.0;  m[1][1] = -2.0 * fy / height;  m[1][2] = 0.0;  m[1][3] = 0.0;
      m[2][0] = 1.0 - 2.0 * cx / width;  m[2][1] = 2.0 * cy / height - 1.0;  m[2][2] = (zfar + znear) / (znear - zfar);  m[2][3] = -1.0;
      m[3][0] = 0.0;  m[3][1] = 0.0;  m[3][2] = 2.0 * zfar * znear / (znear - zfar);  m[3][3] = 0.0;
      return m;
    }
    glm::mat4 getGLModelViewMatrixFromCV(cv::Mat rotMat, cv::Mat transVec){
        // LOG(INFO)<<"CAM POS "<<transVec.at<float>(0, 0)<<" "<<transVec.at<float>(1, 0)<< " "<<transVec.at<float>(2, 0);
        cv::Mat Rt = cv::Mat::zeros(4, 4, CV_32FC1);
        for (int y = 0; y < 3; y++) {
          for (int x = 0; x < 3; x++) {
              Rt.at<float>(y, x) = rotMat.at<float>(y, x);
          }
          Rt.at<float>(y, 3) = transVec.at<float>(y, 0);
        }
        Rt.at<float>(3, 3) = 1.0;

        //OpenGL has reversed Y & Z coords
        cv::Mat reverseYZ = cv::Mat::eye(4, 4, CV_32FC1);
        reverseYZ.at<float>(1, 1) = -1;
        reverseYZ.at<float>(2, 2) = -1;
            
        cv::Mat mvMat = reverseYZ * Rt;
        glm::mat4 res;
        fromCV2GLM(mvMat, &res);
        return res;
    }

}
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
    std::unique_ptr<QuadRenderer> quad_renderer_;
    std::unique_ptr<PointRenderer> point_renderer_;
    std::unique_ptr<CubeRenderer> cube_renderer_;

    float* point_cloud;
    int point_num;
    bool b_render_point = true;
    float img_width = .0f, img_height=.0f;
    glm::mat4 projMat;
    float map_point[4* MAX_TRACK_POINT];
		::mediapipe::Status LoadOptions(CalculatorContext* cc);
    Status RenderGpu(CalculatorContext* cc){
      auto& input = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
      GlTexture src1 = gpu_helper.CreateSourceTexture(input);

      int output_width = input.width(), output_height = input.height();
      auto dst = gpu_helper.CreateDestinationTexture(output_width, output_height,
                                              input.format());
      gpu_helper.BindFramebuffer(dst);  // GL_TEXTURE0
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), src1.name());

      
      if(!quad_renderer_){
          quad_renderer_ = absl::make_unique<QuadRenderer>();
          MP_RETURN_IF_ERROR(quad_renderer_->GlSetup());
      }
      QuadRenderer* qrenderer = quad_renderer_.get();
      
      MP_RETURN_IF_ERROR(qrenderer->GlRender(
        src1.width(), src1.height(), dst.width(), dst.height(), FrameScaleMode::kFit, FrameRotation::kNone, false, false, false));
        // if(b_render_point){
        if(!point_renderer_){
            point_renderer_ = absl::make_unique<PointRenderer>();
            MP_RETURN_IF_ERROR(point_renderer_->GlSetup());
        }
        if(!cube_renderer_){
            cube_renderer_ = absl::make_unique<CubeRenderer>();
            MP_RETURN_IF_ERROR(cube_renderer_->GlSetup());
        }
        
        CubeRenderer* cube_r = cube_renderer_.get();
        

        PointRenderer* prenderer = point_renderer_.get();

        auto slam_data =  cc->Inputs().Tag(kInputSLAMTag).Get<SLAMData*>();
        bool draw_real = true;

        if(draw_real&&slam_data->b_tracking_valid&&slam_data->mp_num){
          cv::Mat rVec;
          cv::Rodrigues(slam_data->camera_pose_mat.colRange(0, 3).rowRange(0, 3), rVec);
          cv::Mat tVec = slam_data->camera_pose_mat.col(3).rowRange(0, 3);


          // glm::mat4 proj_mat, view_mat;
          auto proj_mat = getGLProjMatFromCV(slam_data->camera_intrinsic, img_width, img_height, 500.0f, 0.1f);
          auto view_mat = getGLModelViewMatrixFromCV(slam_data->camera_pose_mat.colRange(0, 3).rowRange(0, 3), tVec);
          glm::mat4 mvp_gl = proj_mat * view_mat;

          cube_r->GlRender(
            mvp_gl
          * glm::translate(glm::mat4(1.0f), glm::vec3(.0,.0,1.0f)) //glm::vec3(-0.2f, 0.5f, 0.5f)) 
          * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)));
        

          for(int i=0;i<slam_data->mp_num;i++){
            map_point[4*i] = slam_data->mapPoints[i].x;
            map_point[4*i+1] = slam_data->mapPoints[i].y;
            map_point[4*i+2] = slam_data->mapPoints[i].z;
            map_point[4*i+3] = 1.0f;
            // LOG(INFO)<<slam_data->mapPoints[i].x<<" "<<slam_data->mapPoints[i].y<<" "<< slam_data->mapPoints[i].z;
          }

          MP_RETURN_IF_ERROR(prenderer->GlRender(mvp_gl, map_point, slam_data->mp_num)); 
          

          /*std::vector<cv::Point2f> projectedPoints;
          cv::projectPoints(
            std::vector<cv::Point3f>(slam_data->mapPoints,slam_data->mapPoints+sizeof(cv::Point3f)*slam_data->mp_num), 
            rVec, tVec, 
            slam_data->camera_intrinsic, 
            slam_data->camera_mDistCoef, 
            projectedPoints);
          
          int i = 0;
          for(int j=0; j<projectedPoints.size(); j++){
              if(i > MAX_TRACK_POINT) break;
              cv::Point2f r1 = projectedPoints[j];
              if(r1.x > img_width || r1.x<.0f || r1.y>img_height||r1.y<.0) continue;
              map_point[4*i] = (r1.x / img_width) * 2.0f - 1.0f; 
              map_point[4*i+1] = r1.y / img_height* 2.0f - 1.0f;
              map_point[4*i+2] = .0f;map_point[4*i+3] = 1.0f;
              i++;
              // if(j < 10) {
              //   LOG(INFO)<<"3d p" <<slam_data->mapPoints[j].x<<" "<<slam_data->mapPoints[j].y<<" "<<slam_data->mapPoints[j].z;
              //   LOG(INFO)<<"POINTS "<< map_point[4*j] << " "<< map_point[4*j+1];
              // }
          }
          
          MP_RETURN_IF_ERROR(prenderer->GlRender(map_point, i));*/
        }else{
          MP_RETURN_IF_ERROR(prenderer->GlRender(&slam_data->keyPoints[0], slam_data->kp_num));  
        }

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), 0);

      // Execute GL commands, before getting result.
      glFlush();

      auto output = dst.GetFrame<GpuBuffer>();
      cc->Outputs().Tag("IMAGE_GPU").Add(output.release(), cc->InputTimestamp());
      return ::mediapipe::OkStatus();
    }
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag(kInputVideoTag).Set<GpuBuffer>();
    cc->Outputs().Tag(kOutputVideoTag).Set<GpuBuffer>();
    if (cc->Inputs().HasTag(kInputSLAMTag)){
      cc->Inputs().Tag(kInputSLAMTag).Set<SLAMData*>();
    }
    MP_RETURN_IF_ERROR(GlCalculatorHelper::UpdateContract(cc));
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) override {
    cc->SetOffset(TimestampDiff(0));
	  LoadOptions(cc);
    MP_RETURN_IF_ERROR(gpu_helper.Open(cc));
    // cv::namedWindow("MapDrawer", /*flags=WINDOW_AUTOSIZE*/ 1);

    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) override {
    auto frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
    if(img_width == 0){
        img_width = (float)frame.width();
        img_height = (float)frame.height();
        float fov = 2*atan(img_height * 0.5 / 458.0f);//glm::radians(45.0f);
		   projMat = glm::perspective(fov, img_width/img_height, 0.1f, 100.0f);

      }

    return gpu_helper.RunInGlContext(
        [this, cc]() -> ::mediapipe::Status { 
          // LOG(INFO) << "=============";
          return RenderGpu(cc); 
          });
  }
};
Status MergeSLAMCalculator::LoadOptions(
    CalculatorContext* cc) {
  // Get calculator options specified in the graph.
	const auto& options = cc->Options<::mediapipe::glShaderHelperOptions>();
  if(options.shader_files_size()){
    for(auto file : options.shader_files()){
      if(file.shader_name() == kRaycastShaderName){
        std::string vs_txt, fs_txt;
        MP_RETURN_IF_ERROR(mediapipe::GetResourceContents(file.vs_path(), &vs_txt));
		    MP_RETURN_IF_ERROR(mediapipe::GetResourceContents(file.frag_path(), &fs_txt));
        LOG(INFO)<<"VS: "<<vs_txt;
        LOG(INFO)<<"Shader readed: "<<file.shader_name();
      }
    }
  }
	return ::mediapipe::OkStatus();
}

REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
