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
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"
#include "tensorflow/lite/interpreter.h"

namespace mediapipe {

// A calculator for converting TFLite tensors to to a float or a float vector.
//
// Input:
//  TENSORS - Vector of TfLiteTensor of type kTfLiteFloat32. Only the first
//            tensor will be used.
// Output:

// Usage example:
// node {
//   calculator: "TfLiteTensorsToFlippedEar"
//   input_stream: "input_video"
//   input_stream: "TENSORS:output_tensors"
//   output_stream: "output_left"
//   output_stream: "EAR_FLIPPED:ear_flipped"
// }
namespace {
    constexpr char kInputTensorTag[] = "TENSORS";
    constexpr char kFlagTag[] = "EAR_FLIPPED";
} 

class TfLiteTensorsToFlippedEar : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc);

  ::mediapipe::Status Open(CalculatorContext* cc) override;

  ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
  int id=0;
};
REGISTER_CALCULATOR(TfLiteTensorsToFlippedEar);

::mediapipe::Status TfLiteTensorsToFlippedEar::GetContract(
    CalculatorContract* cc) {
  cc->Inputs().Index(0).Set<ImageFrame>();
  cc->Outputs().Index(0).Set<ImageFrame>();
  
  RET_CHECK(cc->Inputs().HasTag(kInputTensorTag));
  cc->Inputs().Tag(kInputTensorTag).Set<std::vector<TfLiteTensor>>();

  if (cc->Outputs().HasTag(kFlagTag)) {
    cc->Outputs().Tag(kFlagTag).Set<bool>();
  }

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TfLiteTensorsToFlippedEar::Open(
    CalculatorContext* cc) {
  cc->SetOffset(TimestampDiff(0));

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TfLiteTensorsToFlippedEar::Process(
    CalculatorContext* cc) {
    RET_CHECK(!cc->Inputs().Tag("TENSORS").IsEmpty());

    const auto& input_tensors =
        cc->Inputs().Tag("TENSORS").Get<std::vector<TfLiteTensor>>();
    // TODO: Add option to specify which tensor to take from.
    const TfLiteTensor* raw_tensor = &input_tensors[0];
    const float* raw_floats = raw_tensor->data.f;

    bool need_flip = raw_floats[0] < raw_floats[1];

    if (cc->Outputs().HasTag(kFlagTag))
      cc->Outputs().Tag(kFlagTag).Add(new bool(need_flip), cc->InputTimestamp());

    const auto& img = cc->Inputs().Index(0).Get<ImageFrame>();
    auto frame = formats::MatView(&img);

    auto output_img = absl::make_unique<ImageFrame>(img.Format(), frame.cols, frame.rows);
    cv::Mat output_mat = mediapipe::formats::MatView(output_img.get());

    if(need_flip){
        cv::Mat tmp;
        cv::flip(frame, tmp, 1);
        tmp.copyTo(output_mat);
    }else{
        frame.copyTo(output_mat);
    }
    cc->Outputs().Index(0).Add(output_img.release(), cc->InputTimestamp());
    
    // cv::imwrite( "debug_out/crop_" + std::to_string(id++) +".png" , output_mat);

    return ::mediapipe::OkStatus();
}
}  // namespace mediapipe
