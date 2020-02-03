#include <cmath>
#include <vector>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"

//gpu
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gl_quad_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_point_renderer.h"
namespace mediapipe {
namespace {
    constexpr char kCameraPoseTag[] = "CAMERA_POSE";
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
}  
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
    std::unique_ptr<QuadRenderer> quad_renderer_;
    std::unique_ptr<PointRenderer> point_renderer_;

    float* point_cloud;
    int point_num;

    Status RenderGpu(CalculatorContext* cc){
      auto& input = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
      GlTexture src1 = gpu_helper.CreateSourceTexture(input);

      int output_width = input.width(), output_height = input.height();
      auto dst = gpu_helper.CreateDestinationTexture(output_width, output_height,
                                              input.format());
      gpu_helper.BindFramebuffer(dst);  // GL_TEXTURE0
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), src1.name());
      LOG(INFO)<<"BEFORE";
      if(!point_renderer_){
          point_renderer_ = absl::make_unique<PointRenderer>();
          MP_RETURN_IF_ERROR(point_renderer_->GlSetup());
      }
      if(!quad_renderer_){
          quad_renderer_ = absl::make_unique<QuadRenderer>();
          MP_RETURN_IF_ERROR(quad_renderer_->GlSetup());
      }
      PointRenderer* prenderer = point_renderer_.get();
      QuadRenderer* qrenderer = quad_renderer_.get();
      
      MP_RETURN_IF_ERROR(qrenderer->GlRender(
        src1.width(), src1.height(), dst.width(), dst.height(), FrameScaleMode::kFit, FrameRotation::kNone, false, false, false));
      MP_RETURN_IF_ERROR(prenderer->GlRender(point_cloud, point_num));  
      
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), 0);

      // Execute GL commands, before getting result.
      glFlush();

LOG(INFO) <<"AFTER";
      auto output = dst.GetFrame<GpuBuffer>();
      cc->Outputs().Tag("IMAGE_GPU").Add(output.release(), cc->InputTimestamp());
LOG(INFO) <<"AFTER2";
return ::mediapipe::OkStatus();
    }
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag(kCameraPoseTag).Set<std::string>();
    cc->Inputs().Tag(kInputVideoTag).Set<GpuBuffer>();
    cc->Outputs().Tag(kOutputVideoTag).Set<GpuBuffer>();
    MP_RETURN_IF_ERROR(GlCalculatorHelper::UpdateContract(cc));
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) override {
    cc->SetOffset(TimestampDiff(0));
    MP_RETURN_IF_ERROR(gpu_helper.Open(cc));
    //debug
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
    LOG(INFO)<<"ASDFASDFASDFASDF";
    auto frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();

    return gpu_helper.RunInGlContext(
        [this, cc]() -> ::mediapipe::Status { 
          // LOG(INFO) << "=============";
          return RenderGpu(cc); 
          });
  }
};
REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
