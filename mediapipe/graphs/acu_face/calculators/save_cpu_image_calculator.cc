
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"



namespace mediapipe {

// Convert an input image (GpuBuffer or ImageFrame) to ImageFrame.
class SaveCPUImageCalculator : public CalculatorBase {
 public:
  SaveCPUImageCalculator() {}

  static ::mediapipe::Status GetContract(CalculatorContract* cc);

//   ::mediapipe::Status Open(CalculatorContext* cc) override;
  ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    std::string  OUTPUT_PATH = "debug_out/crop_";
    int id = 0;
};
REGISTER_CALCULATOR(SaveCPUImageCalculator);

// static
::mediapipe::Status SaveCPUImageCalculator::GetContract(
    CalculatorContract* cc) {
    cc->Inputs().Index(0).Set<ImageFrame>();
    return ::mediapipe::OkStatus();
}

::mediapipe::Status SaveCPUImageCalculator::Process(CalculatorContext* cc){
  if(id %5== 0){

  const auto& img = cc->Inputs().Index(0).Get<ImageFrame>();
  cv::imwrite( OUTPUT_PATH + std::to_string(id/5) +".png" , formats::MatView(&img));
  }
  id++;
  return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
