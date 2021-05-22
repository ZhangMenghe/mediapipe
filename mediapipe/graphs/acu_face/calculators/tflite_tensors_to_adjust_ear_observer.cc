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
//   calculator: "TfLiteTensorsToAdjustEarObserver"
//   input_stream: "TENSORS:output_tensors"
//   input_stream: "EAR_OBSERVER:input_observer"
//   output_stream: "EAR_OBSERVER:adjust_observer"
// }
namespace {
    constexpr char kEarObserversTag[] = "EAR_OBSERVER";
    constexpr char kInputTensorTag[] = "TENSORS";
} 

class TfLiteTensorsToAdjustEarObserver : public CalculatorBase {
 public:
    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;

    ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    const static int num_landmarks_ = 55;
    void adjustParameters(std::vector<cv::Point2d> ldmk, int size, float &scaley, float &scalex, float &ang, float &cx, float &cy, bool oriented, bool scaled);
};
REGISTER_CALCULATOR(TfLiteTensorsToAdjustEarObserver);

::mediapipe::Status TfLiteTensorsToAdjustEarObserver::GetContract(
    CalculatorContract* cc) {
    RET_CHECK(cc->Inputs().HasTag(kInputTensorTag));
    cc->Inputs().Tag(kInputTensorTag).Set<std::vector<TfLiteTensor>>();

    cc->Inputs().Tag(kEarObserversTag).Set<std::vector<float>>();
    cc->Outputs().Tag(kEarObserversTag).Set<std::vector<float>>();
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TfLiteTensorsToAdjustEarObserver::Open(
    CalculatorContext* cc) {
  cc->SetOffset(TimestampDiff(0));

  return ::mediapipe::OkStatus();
}
void TfLiteTensorsToAdjustEarObserver::adjustParameters(std::vector<cv::Point2d> ldmk, int size, float &scaley, float &scalex, float &ang, float &cx, float &cy, bool oriented, bool scaled){
	double ratioy = (scaley/((size-1)/2.0)), ratiox = ((scaled ? scalex : scaley)/((size-1)/2.0));

	/* align landmark coordinate space to the original image */
	/* compute principal components and bounding box for aligned landmarks */
	cv::Mat data_pts = cv::Mat(55, 2, CV_64FC1);
	for(int i=0; i < 55; i++) {
		double xt = ratiox*(ldmk[i].x-(size-1)/2.0), yt = ratioy*(ldmk[i].y-(size-1)/2.0);
		data_pts.at<double>(i, 0) = xt*cos(ang)-yt*sin(ang)+cx;
		data_pts.at<double>(i, 1) = xt*sin(ang)+yt*cos(ang)+cy;
	}
	cv::PCA pca_analysis(data_pts, cv::Mat(), 0);

	/* set orientation of the ear as the direction of the first principal component */
	double angle = atan2(pca_analysis.eigenvectors.at<double>(0, 1), pca_analysis.eigenvectors.at<double>(0, 0));
	while(angle < 0.0)
		angle += 2.0*M_PI;
	if(angle > M_PI)
		angle -= M_PI;
	ang = angle-M_PI/2.0;

	/* set scale as two times the deviation of the first principal component */
	scaley = 2.0*sqrt(pca_analysis.eigenvalues.at<double>(0, 0));
	scalex = 2.0*sqrt(pca_analysis.eigenvalues.at<double>(0, 1));

	if(!oriented) {
		/* set center as the center of the bounding box */
		cv::Point2d tl = {DBL_MAX, DBL_MAX}, br = {-DBL_MAX, -DBL_MAX};
		for(int i=0; i < 55; i++) {
			tl.x = fmin(tl.x, data_pts.at<double>(i, 0));
			tl.y = fmin(tl.y, data_pts.at<double>(i, 1));
			br.x = fmax(br.x, data_pts.at<double>(i, 0));
			br.y = fmax(br.y, data_pts.at<double>(i, 1));
		}
		cx = (tl.x+br.x)/2.0;
		cy = (tl.y+br.y)/2.0;
	}
	else {
		/* set center as the center of the oriented bounding box */
		cv::Mat norm_pts = pca_analysis.project(data_pts);
		double top=-DBL_MAX, bottom=DBL_MAX, left=DBL_MAX, right=-DBL_MAX;
		for(int i=0; i < 55; i++) {
			top = fmax(top, norm_pts.at<double>(i, 0));
			bottom = fmin(bottom, norm_pts.at<double>(i, 0));
			right = fmax(right, norm_pts.at<double>(i, 1));
			left = fmin(left, norm_pts.at<double>(i, 1));
		}
		cx = (bottom+top)/2.0*pca_analysis.eigenvectors.at<double>(0, 0) + (left+right)/2.0*pca_analysis.eigenvectors.at<double>(1, 0) + pca_analysis.mean.at<double>(0, 0);
		cy = (bottom+top)/2.0*pca_analysis.eigenvectors.at<double>(0, 1) + (left+right)/2.0*pca_analysis.eigenvectors.at<double>(1, 1) + pca_analysis.mean.at<double>(0, 1);
	}
}

::mediapipe::Status TfLiteTensorsToAdjustEarObserver::Process(
    CalculatorContext* cc) {
    RET_CHECK(!cc->Inputs().Tag("TENSORS").IsEmpty());

    int size;
    float scaley, scalex, ang, cx, cy;

    auto& observers = cc->Inputs().Tag(kEarObserversTag).Get<std::vector<float>>();
    size = int(observers[0]);
    scaley = observers[1], scalex = observers[2], ang = observers[3], cx = observers[4], cy = observers[5];

    int hsize = int(size * 0.5f);

    const auto& input_tensors =
        cc->Inputs().Tag("TENSORS").Get<std::vector<TfLiteTensor>>();
    // TODO: Add option to specify which tensor to take from.
    const TfLiteTensor* raw_tensor = &input_tensors[0];
    const float* raw_floats = raw_tensor->data.f;

    std::vector<cv::Point2d> landmarks;
    landmarks.reserve(num_landmarks_);
    for(int i = 0; i<num_landmarks_; i++){
        landmarks.emplace_back(cv::Point2d(raw_floats[i*2]*hsize+hsize, raw_floats[i*2+1]*hsize+hsize));
    }
    adjustParameters(landmarks, size, scaley, scalex, ang, cx, cy, false, false);

    if(cc->Outputs().HasTag(kEarObserversTag)){
        auto output_observer = absl::make_unique<std::vector<float>>();
        output_observer->reserve(6);
        output_observer->emplace_back(size);
        output_observer->emplace_back(scaley);output_observer->emplace_back(scalex);
        output_observer->emplace_back(ang);output_observer->emplace_back(cx);output_observer->emplace_back(cy);
        
                std::cout<<"after: ";    
        for(int i=0;i<6;i++)std::cout<<output_observer->at(i)<<" ";
        std::cout<<std::endl;
        
        cc->Outputs().Tag(kEarObserversTag).Add(output_observer.release(), cc->InputTimestamp());
    }
    
    return ::mediapipe::OkStatus();
}
}  // namespace mediapipe
