
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {
using namespace cv;
namespace {
    constexpr char kLandmarksTag[] = "LANDMARKS";
    constexpr char kNormLandmarksTag[] = "NORM_LANDMARKS";
    constexpr char kOutputPosTag[] = "POS_MAT";
    constexpr char kFlagTag[] = "EAR_FLIPPED";
    constexpr char kInputImgSize[] = "IMG_SIZE";
}  
// Convert an input image (GpuBuffer or ImageFrame) to ImageFrame.
class BackProjectNormalizedLandmarksCalculator : public CalculatorBase {
 public:
    BackProjectNormalizedLandmarksCalculator() {}

    static ::mediapipe::Status GetContract(CalculatorContract* cc);
    ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    int id = 0;
};
REGISTER_CALCULATOR(BackProjectNormalizedLandmarksCalculator);

// static
::mediapipe::Status BackProjectNormalizedLandmarksCalculator::GetContract(
    CalculatorContract* cc) {
	if (cc->Inputs().HasTag(kLandmarksTag)) 
        cc->Inputs().Tag(kLandmarksTag).Set<NormalizedLandmarkList>();
	if (cc->Inputs().HasTag(kFlagTag)) 
        cc->Inputs().Tag(kFlagTag).Set<std::vector<bool>>();
    if (cc->Inputs().HasTag(kInputImgSize)) 
        cc->Inputs().Tag(kInputImgSize).Set<cv::Size>();

    if (cc->Outputs().HasTag(kNormLandmarksTag)) 
        cc->Outputs().Tag(kNormLandmarksTag).Set<NormalizedLandmarkList>();
    if (cc->Outputs().HasTag(kOutputPosTag))
	    cc->Outputs().Tag(kOutputPosTag).Set<cv::Mat>();

    return ::mediapipe::OkStatus();
}
::mediapipe::Status BackProjectNormalizedLandmarksCalculator::Process(CalculatorContext* cc){
	auto& input_landmarks = cc->Inputs().Tag(kLandmarksTag).Get<LandmarkList>();
    NormalizedLandmarkList output_landmarks;
    
    auto& pos_mat = cc->Inputs().Tag(kOutputPosTag).Get<cv::Mat>();
    int pos_rows = pos_mat.rows, pos_cols = pos_mat.cols;

    bool b_flipped = cc->Inputs().Tag(kFlagTag).Get<bool>();
    auto img_size = cc->Inputs().Tag(kInputImgSize).Get<cv::Size>();

    for(int i=0;i<input_landmarks.landmark_size(); i++){
        const Landmark& landmark = input_landmarks.landmark(i);
        NormalizedLandmark* new_landmark = output_landmarks.add_landmark();
        auto value = pos_mat.at<float>(
            int(fmin(pos_rows-1, landmark.y())),
            int(fmin(pos_cols-1, landmark.x()))
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
