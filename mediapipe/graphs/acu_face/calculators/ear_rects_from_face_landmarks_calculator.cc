
#include <cmath>

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/calculator_options.pb.h"
#include "mediapipe/framework/formats/location_data.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_calculator_helper.h"

namespace mediapipe {

//   input_stream: "VECTOR:multi_face_landmarks"
//   input_stream: "NORM_RECTS:face_rects_from_landmarks"
//   output_stream: "NORM_RECTS:ear_rects_from_face"
namespace {
    constexpr char kGpuBufferTag[] = "IMAGE_GPU";
    constexpr char kInputLandMarksVectorTag[] = "VECTOR";
    constexpr char kNormRectsTag[] = "NORM_RECTS";
    constexpr char kEarNormRectsTag[] = "NORM_RECTS_EAR";
    const static int refLeftEarIdxs[4] = {
        356,454,323,361
    };
    const static int refRightEarIdxs[4] = {
        127, 234, 93, 132
    };
}  // namespace

class EarRectsFromFaceLandmarksCalculator : public CalculatorBase {
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc);

//   ::mediapipe::Status Open(CalculatorContext* cc) override;
  ::mediapipe::Status Process(CalculatorContext* cc) override;
//   ::mediapipe::Status Close(CalculatorContext* cc) override;
};
REGISTER_CALCULATOR(EarRectsFromFaceLandmarksCalculator);


// static
::mediapipe::Status EarRectsFromFaceLandmarksCalculator::GetContract(CalculatorContract* cc) {
    RET_CHECK(!cc->Inputs().GetTags().empty());
    RET_CHECK(!cc->Outputs().GetTags().empty());
    //video
    if (cc->Inputs().HasTag(kGpuBufferTag))
        cc->Inputs().Tag(kGpuBufferTag).Set<mediapipe::GpuBuffer>();

    //landmarks
    if(cc->Inputs().HasTag(kInputLandMarksVectorTag))
        cc->Inputs().Tag(kInputLandMarksVectorTag).Set<std::vector<::mediapipe::NormalizedLandmarkList>>();
  
    //Input face RECTS
    if (cc->Inputs().HasTag(kNormRectsTag))
	    cc->Inputs().Tag(kNormRectsTag).Set<std::vector<NormalizedRect>>();

    //Output rects
    if(cc->Outputs().HasTag(kEarNormRectsTag))
        cc->Outputs().Tag(kEarNormRectsTag).Set<std::vector<NormalizedRect>>();
    
    return ::mediapipe::OkStatus();
}

::mediapipe::Status EarRectsFromFaceLandmarksCalculator::Process(CalculatorContext* cc) {
        // std::vector<NormalizedRect> ear_rects;
    int ear_num = (cc->Inputs().HasTag(kInputLandMarksVectorTag) && !cc->Inputs().Tag(kInputLandMarksVectorTag).IsEmpty())?1:0;
    auto norm_rects = absl::make_unique<std::vector<NormalizedRect>>(ear_num);

    if(ear_num > 0){
		auto& nlandmarks_vec = cc->Inputs().Tag(kInputLandMarksVectorTag).Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
		auto landmarks = nlandmarks_vec[0];

        const NormalizedLandmark& lt = landmarks.landmark(refRightEarIdxs[0]);
        auto lb = landmarks.landmark(refRightEarIdxs[3]);
        auto ltx = lt.x() * 2.0-1.0, lty = lt.y()* 2.0-1.0;
        auto lbx = lb.x() * 2.0-1.0, lby = lb.y()* 2.0-1.0;

        float height = std::abs(lty - lby) * 1.5f;
        float width = 0.5f * height;
        int i=0;
        norm_rects->at(i).set_width(width);
        norm_rects->at(i).set_height(height);
        norm_rects->at(i).set_x_center((lbx+ltx) * 0.5f);//center is 0.5
        norm_rects->at(i).set_y_center((lby+lty) * 0.5f);
	}
    // else{
        // float width = 2.0f;
        // for(int i=0; i<ear_num; i++){
        //     norm_rects->at(i).set_width(width);
        //     norm_rects->at(i).set_height(1.0);
        //     norm_rects->at(i).set_x_center(0.5f*width);
        //     norm_rects->at(i).set_y_center(.0f);
        // }
    // }
    
    norm_rects->emplace_back(NormalizedRect());

    cc->Outputs()
        .Tag(kEarNormRectsTag)
        .Add(norm_rects.release(), cc->InputTimestamp());
              
    return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
