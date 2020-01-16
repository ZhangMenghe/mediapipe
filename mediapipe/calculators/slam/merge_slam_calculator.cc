
#include <cmath>
#include <vector>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/gpu/gpu_buffer.h"
namespace mediapipe {

namespace {
    constexpr char kCameraPoseTag[] = "CAMERA_POSE";
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
}  
class MergeSLAMCalculator : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    LOG(INFO) << "====1111";
    cc->Inputs().Tag(kCameraPoseTag).Set<std::string>();
    cc->Inputs().Tag(kInputVideoTag).Set<GpuBuffer>();
    cc->Outputs().Tag(kOutputVideoTag).Set<GpuBuffer>();
LOG(INFO) << "====2222";
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) override {
    cc->SetOffset(TimestampDiff(0));

    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) override {
      LOG(INFO) << "====3333";
    cc->Outputs().Tag(kOutputVideoTag).Add(&cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>(), cc->InputTimestamp());
    LOG(INFO) << "===="<<cc->Inputs().Tag(kCameraPoseTag).Get<std::string>();
    return ::mediapipe::OkStatus();
  }
};
REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
