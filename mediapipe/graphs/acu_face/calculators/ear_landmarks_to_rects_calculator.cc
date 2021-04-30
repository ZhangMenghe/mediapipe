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

#include <cmath>
#include <vector>

// #include "Eigen/Core"
// #include "mediapipe/calculators/util/landmarks_to_floats_calculator.pb.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/matrix.h"
#include "mediapipe/framework/port/ret_check.h"


namespace mediapipe {

namespace {

constexpr char kLandmarksTag[] = "LANDMARKS";
constexpr char kNormalizedRectTag[] = "NORM_RECT_EAR";
constexpr char kFlagTag[] = "FLAG";

}  // namespace

// # Converts the ear detection into a rectangle (normalized by image size)
// # that encloses the ear.
// node {
//   calculator: "LandmarksToRectsCalculator"
//   input_stream: "LANDMARKS:ear_landmarks"
//   output_stream: "NORM_RECT:ear_rect_from_landmarks"
//   output_stream: "FLAG:ear_presence"
// }
class EarLandmarksToRectsCalculator : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag(kLandmarksTag).Set<NormalizedLandmarkList>();

    if (cc->Outputs().HasTag(kFlagTag))
      cc->Outputs().Tag(kFlagTag).Set<bool>();

    //output single ear RECT
    if (cc->Outputs().HasTag(kNormalizedRectTag))
	    cc->Outputs().Tag(kNormalizedRectTag).Set<NormalizedRect>();

    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) override {
    // cc->SetOffset(TimestampDiff(0));
    // const auto& options =
    //     cc->Options<::mediapipe::LandmarksToFloatsCalculatorOptions>();
    // num_dimensions_ = options.num_dimensions();
    // // Currently number of dimensions must be within [1, 3].
    // RET_CHECK_GE(num_dimensions_, 1);
    // RET_CHECK_LE(num_dimensions_, 3);
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) override {
    auto norm_rect = absl::make_unique<NormalizedRect>();
    // float width = 2.0f;
    // norm_rect->set_width(width);
    // norm_rect->set_height(1.0);
    // norm_rect->set_x_center(0.5f*width);
    // norm_rect->set_y_center(.0f);

    // cc->Outputs()
    //     .Tag(kNormalizedRectTag)
    //     .Add(norm_rect.release(), cc->InputTimestamp());
      

    // Only process if there's input landmarks.
    if (cc->Inputs().Tag(kLandmarksTag).IsEmpty()) {
        cc->Outputs().Tag(kFlagTag)
                     .Add(new bool(false), cc->InputTimestamp());
        cc->Outputs()
        .Tag(kNormalizedRectTag)
        .Add(norm_rect.release(), cc->InputTimestamp());
      return ::mediapipe::OkStatus();
    }

  float min_x = 1.0f, min_y = 1.0f, max_x = -1.0f, max_y=-1.0f;
  auto& landmarks = cc->Inputs().Tag(kLandmarksTag).Get<::mediapipe::NormalizedLandmarkList>();
    // if(lmpoints == nullptr) lmpoints = new float[3*landmarks.landmark_size()];
    for(int i=0; i<landmarks.landmark_size();i++){
      const NormalizedLandmark& landmark = landmarks.landmark(i);
      if(landmark.x() < min_x) min_x = landmark.x();
      if(landmark.x() > max_x) max_x = landmark.x();
      if(landmark.y() < min_y) min_y = landmark.y();
      if(landmark.y() > max_y) max_y = landmark.y();

      // std::cout<<"landmark: "<<landmark.x()<<" "<<landmark.y()<<" "<<landmark.z()<<std::endl;
      // lmpoints[3*i] = landmark.x();lmpoints[3*i+1] = landmark.y();lmpoints[3*i+2] = landmark.z();
    }
    norm_rect->set_width(max_x - min_x);
    norm_rect->set_height(max_y - min_y);
    norm_rect->set_x_center(0.5f*(max_x + min_x));
    norm_rect->set_y_center(0.5f*(max_y + min_y));

    // std::cout<<norm_rect->height()<<"  "<<norm_rect->width()<<std::endl;
    cc->Outputs()
        .Tag(kNormalizedRectTag)
        .Add(norm_rect.release(), cc->InputTimestamp());

    cc->Outputs().Tag(kFlagTag)
                    .Add(new bool(true), cc->InputTimestamp());
    // const auto& input_landmarks =
    //     cc->Inputs().Tag(kLandmarksTag).Get<NormalizedLandmarkList>();

    // if (cc->Outputs().HasTag(kFloatsTag)) {
    //   auto output_floats = absl::make_unique<std::vector<float>>();
    //   for (int i = 0; i < input_landmarks.landmark_size(); ++i) {
    //     const NormalizedLandmark& landmark = input_landmarks.landmark(i);
    //     output_floats->emplace_back(landmark.x());
    //     if (num_dimensions_ > 1) {
    //       output_floats->emplace_back(landmark.y());
    //     }
    //     if (num_dimensions_ > 2) {
    //       output_floats->emplace_back(landmark.z());
    //     }
    //   }

    //   cc->Outputs()
    //       .Tag(kFloatsTag)
    //       .Add(output_floats.release(), cc->InputTimestamp());
    // } else {
    //   auto output_matrix = absl::make_unique<Matrix>();
    //   output_matrix->setZero(num_dimensions_, input_landmarks.landmark_size());
    //   for (int i = 0; i < input_landmarks.landmark_size(); ++i) {
    //     (*output_matrix)(0, i) = input_landmarks.landmark(i).x();
    //     if (num_dimensions_ > 1) {
    //       (*output_matrix)(1, i) = input_landmarks.landmark(i).y();
    //     }
    //     if (num_dimensions_ > 2) {
    //       (*output_matrix)(2, i) = input_landmarks.landmark(i).z();
    //     }
    //   }
    //   cc->Outputs()
    //       .Tag(kMatrixTag)
    //       .Add(output_matrix.release(), cc->InputTimestamp());
    // }
    return ::mediapipe::OkStatus();
  }

 private:
  int num_dimensions_ = 0;
};
REGISTER_CALCULATOR(EarLandmarksToRectsCalculator);

}  // namespace mediapipe
