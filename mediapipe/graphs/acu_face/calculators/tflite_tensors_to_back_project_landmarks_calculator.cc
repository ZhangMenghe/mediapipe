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
#include "mediapipe/framework/formats/landmark.pb.h"

namespace mediapipe {

// A calculator for converting TFLite tensors to to a float or a float vector.
//
// Input:
//  TENSORS - Vector of TfLiteTensor of type kTfLiteFloat32. Only the first
//            tensor will be used.
// Output:

// Usage example:
// node {
//   calculator: "TfliteTensorsToBackProjectLandmarksCalculator"
//   input_stream: "TENSORS:output_tensors"
//   input_stream: "EAR_OBSERVER:input_observer"
//   output_stream: "EAR_OBSERVER:adjust_observer"
// }
namespace {
    constexpr char kInputTensorTag[] = "TENSORS";
    constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS";
    constexpr char kInputPosTag[] = "POS_MAT";
    constexpr char kFlagTag[] = "EAR_FLIPPED";
    constexpr char kInputImgSize[] = "IMG_SIZE";
} 

class TfliteTensorsToBackProjectLandmarksCalculator : public CalculatorBase {
 public:
    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;

    ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    const static int num_landmarks_ = 55;
};
REGISTER_CALCULATOR(TfliteTensorsToBackProjectLandmarksCalculator);

::mediapipe::Status TfliteTensorsToBackProjectLandmarksCalculator::GetContract(
    CalculatorContract* cc) {
    RET_CHECK(cc->Inputs().HasTag(kInputTensorTag));
    cc->Inputs().Tag(kInputTensorTag).Set<std::vector<TfLiteTensor>>();

    if (cc->Inputs().HasTag(kFlagTag)) 
        cc->Inputs().Tag(kFlagTag).Set<bool>();
    if (cc->Inputs().HasTag(kInputImgSize)) 
        cc->Inputs().Tag(kInputImgSize).Set<cv::Size>();
    if (cc->Inputs().HasTag(kInputPosTag))
	    cc->Inputs().Tag(kInputPosTag).Set<cv::Mat>();

    if (cc->Outputs().HasTag(kNormLandmarksTag)) 
        cc->Outputs().Tag(kNormLandmarksTag).Set<NormalizedLandmarkList>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status TfliteTensorsToBackProjectLandmarksCalculator::Open(
    CalculatorContext* cc) {
  cc->SetOffset(TimestampDiff(0));

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TfliteTensorsToBackProjectLandmarksCalculator::Process(
    CalculatorContext* cc) {
    RET_CHECK(!cc->Inputs().Tag("TENSORS").IsEmpty());

    const auto& input_tensors =
        cc->Inputs().Tag("TENSORS").Get<std::vector<TfLiteTensor>>();
    
    // TODO: Add option to specify which tensor to take from.
    const TfLiteTensor* raw_tensor = &input_tensors[0];
    const float* raw_floats = raw_tensor->data.f;

    auto& pos_mat = cc->Inputs().Tag(kInputPosTag).Get<cv::Mat>();
    int pos_rows = pos_mat.rows, pos_cols = pos_mat.cols;
    int hsize = int(pos_rows * 0.5f);

    bool b_flipped = cc->Inputs().Tag(kFlagTag).Get<bool>();
    auto img_size = cc->Inputs().Tag(kInputImgSize).Get<cv::Size>();
    NormalizedLandmarkList output_landmarks;

    for(int i=0;i<num_landmarks_; i++){
        auto input_x = raw_floats[i*2]*hsize+hsize, input_y=raw_floats[i*2+1]*hsize+hsize;
        NormalizedLandmark* new_landmark = output_landmarks.add_landmark();
        auto value = pos_mat.at<float>(
            int(fmin(pos_rows-1, input_y)),
            int(fmin(pos_cols-1, input_x))
        );
        int y = int(value / 1000), x = value-y*1000;
        if(b_flipped) x = img_size.width - x;

        new_landmark->set_x(float(x) / img_size.width);
        new_landmark->set_y(float(y) / img_size.height);
    }
    
    cc->Outputs()
        .Tag(kNormLandmarksTag)
        .AddPacket(MakePacket<NormalizedLandmarkList>(output_landmarks)
                       .At(cc->InputTimestamp()));
    return ::mediapipe::OkStatus();

}
}  // namespace mediapipe
