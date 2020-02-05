#include <cmath>
#include <vector>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"

//gpu
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gl_quad_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_point_renderer.h"
#include "mediapipe/calculators/slam/slam_calculators.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtx/string_cast.hpp>
namespace mediapipe {
namespace {
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
    constexpr char kInputSLAMTag[] = "SLAM_OUT";

    void GetCurrentOpenGLCameraMatrix(const cv::Mat& mCameraPose, glm::mat4*view_mat){
      cv::Mat Rwc(3,3,CV_32FC1);
      cv::Mat twc(3,1,CV_32FC1);
      {
        Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
        twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
      }
      float data[16]={
            Rwc.at<float>(0,0), Rwc.at<float>(0,1), Rwc.at<float>(0,2), twc.at<float>(0),
            Rwc.at<float>(1,0), Rwc.at<float>(1,1), Rwc.at<float>(1,2), twc.at<float>(1),
            Rwc.at<float>(2,0), Rwc.at<float>(2,1), Rwc.at<float>(2,2), twc.at<float>(2),
            0.0, .0f,.0f,1.0f
      };
      memcpy(glm::value_ptr(*view_mat), &data[0], 16 * sizeof(float));
    }
    void getColMajorMatrixFromMat(float M[],cv::Mat &Tcw){
    M[0] = Tcw.at<float>(0,0);
    M[1] = Tcw.at<float>(1,0);
    M[2] = Tcw.at<float>(2,0);
    M[3]  = 0.0;
    M[4] = Tcw.at<float>(0,1);
    M[5] = Tcw.at<float>(1,1);
    M[6] = Tcw.at<float>(2,1);
    M[7]  = 0.0;
    M[8] = Tcw.at<float>(0,2);
    M[9] = Tcw.at<float>(1,2);
    M[10] = Tcw.at<float>(2,2);
    M[11]  = 0.0;
    M[12] = Tcw.at<float>(0,3);
    M[13] = Tcw.at<float>(1,3);
    M[14] = Tcw.at<float>(2,3);
    M[15]  = 1.0;
}
    void fromCV2GLM(const cv::Mat& cvmat, glm::mat4* glmmat) {
    if (cvmat.cols != 4 || cvmat.rows != 4 || cvmat.type() != CV_32FC1) {
        return;
    }
    
    memcpy(glm::value_ptr(*glmmat), cvmat.data, 16 * sizeof(float));
    *glmmat = glm::transpose(*glmmat);
}


}
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
    std::unique_ptr<QuadRenderer> quad_renderer_;
    std::unique_ptr<PointRenderer> point_renderer_;

    float* point_cloud;
    int point_num;
    bool b_render_point = true;
    float img_width = .0f, img_height=.0f;
    glm::mat4 projMat;
    
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
        PointRenderer* prenderer = point_renderer_.get();
        auto slam_data =  cc->Inputs().Tag(kInputSLAMTag).Get<SLAMData*>();
        bool draw_real = false;
        if(draw_real&&slam_data->b_tracking_valid){
          glm::mat4 world2Cam;
          GetCurrentOpenGLCameraMatrix(slam_data->camera_pose_mat, &world2Cam);
          
          glm::mat4 Cam2World;
          fromCV2GLM(slam_data->camera_pose_mat, &Cam2World);


          // glm::mat4 viewMat = glm::mat4(1.0f);
          auto mCameraPose = slam_data->camera_pose_mat;
          cv::Mat Rwc(3,3,CV_32FC1);

          cv::Mat twc(3,1,CV_32FC1);
          {
            Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
            twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
          }
          float data[16]={
                Rwc.at<float>(0,0), Rwc.at<float>(0,1), Rwc.at<float>(0,2), twc.at<float>(0),
                Rwc.at<float>(1,0), Rwc.at<float>(1,1), Rwc.at<float>(1,2), twc.at<float>(1),
                Rwc.at<float>(2,0), Rwc.at<float>(2,1), Rwc.at<float>(2,2), twc.at<float>(2),
                0.0, .0f,.0f,1.0f
          };
          glm::vec4 foward = Cam2World * glm::vec4(.0,.0,-1.0f,1.0f);
          glm::vec4 up = Cam2World*glm::vec4(0,   1.0f,    0, 1.0f);
          glm::vec4 c  = Cam2World*glm::vec4(.0f, .0f, .0f, 1.0f);

          glm::mat4 viewMat = glm::lookAt(
            glm::vec3(c.x, c.y, c.z)/c.w,
            glm::vec3(foward.x, foward.y, foward.z)/foward.z,
            glm::vec3(up.x, up.y, up.z)/ up.w
          );
          

          // for(int i=0;i<slam_data->vp_num;i++){
          //   auto data = &slam_data->vp_mpoints[4*i];
          //   LOG(INFO)<<"pt: "<<data[0] <<" " << data[1]<<" "<< data[2]<<" "<<data[3];
          // }

          MP_RETURN_IF_ERROR(prenderer->GlRender(
            projMat *Cam2World,//viewMat, 
            &slam_data->vp_mpoints[0], 
            slam_data->vp_num));  
 
        }else{
          MP_RETURN_IF_ERROR(prenderer->GlRender(&slam_data->kpoints[0], slam_data->kp_num));  
        }


          //  }

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
    MP_RETURN_IF_ERROR(gpu_helper.Open(cc));
    //debug
    // if(cc->Inputs().HasTag(kInputKeyPoints)) b_render_point = true;

    point_num = 5;
    float tmp_point_cloud[20] = {
      -0.8f, -0.8f, .0f, 1.0f,
      -0.8f, 0.8f, .0f, 1.0f,
      0.8f, -0.8f, .0f, 1.0f,
      0.8f, 0.8f, .0f, 1.0f,
      .0f, .0f, .0f, 1.0f,
    };
    point_cloud = new float[20];
    memcpy(point_cloud, tmp_point_cloud, 20*sizeof(float));
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
REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
