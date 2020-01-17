#include <cmath>
#include <vector>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/gpu/gpu_buffer.h"

//gpu
#include "mediapipe/gpu/gl_calculator_helper.h"
namespace mediapipe {
namespace {
    constexpr char kCameraPoseTag[] = "CAMERA_POSE";
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
}  
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
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
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) override {
    auto frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();

    MP_RETURN_IF_ERROR(gpu_helper.RunInGlContext(
            [this, cc]() -> ::mediapipe::Status {
      auto& input_gpu_frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
      auto src_tex = gpu_helper.CreateSourceTexture(input_gpu_frame);

      std::unique_ptr<mediapipe::ImageFrame> output_frame;
      output_frame = absl::make_unique<mediapipe::ImageFrame>(
              mediapipe::ImageFormatForGpuBufferFormat(input_gpu_frame.format()),
              input_gpu_frame.width(), input_gpu_frame.height(),
              mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
      gpu_helper.BindFramebuffer(src_tex); 
      const auto info =
          mediapipe::GlTextureInfoForGpuBufferFormat(input_gpu_frame.format(), 0);
      glReadPixels(0, 0, src_tex.width(), src_tex.height(), info.gl_format,
                    info.gl_type, output_frame->MutablePixelData());
      glFlush();
      src_tex.Release();


      auto dst_tex = gpu_helper.CreateSourceTexture(*output_frame.get());
      auto output_gpu_frame = dst_tex.GetFrame<mediapipe::GpuBuffer>();
      glFlush();
      dst_tex.Release();

      cc->Outputs().Tag("IMAGE_GPU").Add(output_gpu_frame.release(), cc->InputTimestamp());
    }));
    return ::mediapipe::OkStatus();
  }
};
REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
