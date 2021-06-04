
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
    const static int RIGHT_EAR_RECT = 0, LEFT_EAR_RECT=1;

    const static int refLeftEarIdxs[4] = {
        356,454,323,361
    };
    const static int refRightEarIdxs[7] = {
        /*21,*/162, 127, 234, 93, 132, 58, 172
    };
    const static int refNoseUpDownIdxs[6] = {
        6,197,195,5,4,1
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
std::vector<float> getEarBoundingBox(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy){
    // float a1 = ay - cy, a2 = by-dy;
    // float b1 = ax - cx, b2 = bx - dx;
    // float c1 = a1 * ax + b1 * cy, c2 = a2 * bx + b2 * dy;

    float ki1 = (cx-ax)/(cy-ay), ki2=(dx-bx)/(dy-by);
    float my = ay, ny=by;
    float mx = ki1 * (my-ay)+ax, nx = ki2*(ny-by)+bx;


    // float mx = (c1-b1*my)/a1, nx=(c2-b2*ny)/a2;

    // std::cout<<my<<", "<<mx<<std::endl;

    float rx = fmax(mx, nx);
    float height = std::abs(my - ny);
    float width = 0.6f * height;
    
    float centerX = rx - width * 0.5f;//center is 0.5
    float centerY = (my+ny) * 0.5f;//h_mid);//
    return std::vector<float>{height, width ,centerX, centerY};
}
::mediapipe::Status EarRectsFromFaceLandmarksCalculator::Process(CalculatorContext* cc) {
    int ear_num = (cc->Inputs().HasTag(kInputLandMarksVectorTag) && !cc->Inputs().Tag(kInputLandMarksVectorTag).IsEmpty())?1:0;
    auto norm_rects = absl::make_unique<std::vector<NormalizedRect>>(ear_num);

    if(ear_num > 0){
        //norm landmark, width/height .0 ~ 1.0,
		auto& nlandmarks_vec = cc->Inputs().Tag(kInputLandMarksVectorTag).Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
		auto landmarks = nlandmarks_vec[0];

        auto nt = landmarks.landmark(refNoseUpDownIdxs[0]);
        auto nb = landmarks.landmark(refNoseUpDownIdxs[5]);

        auto nose_width = landmarks.landmark(358).x() - landmarks.landmark(129).x();
        auto nose_len = nb.y()-nt.y();

        // std::cout<<"test: "<<nose_width / nose_len<<std::endl;

        bool head_up=(nose_width / nose_len)>1.2;
        const NormalizedLandmark& lt = landmarks.landmark(refRightEarIdxs[0]);
        auto lb = landmarks.landmark(refRightEarIdxs[5]);


        auto res = getEarBoundingBox(lt.x(), lt.y(), lb.x(), lb.y(), nb.x(), nb.y(), nt.x(), nt.y());
        if(head_up){
            res[0]*=1.2f; res[3]+=0.05f;
        }
        int i=0;
        norm_rects->at(i).set_height(res[0]);
        norm_rects->at(i).set_width(res[1]);
        norm_rects->at(i).set_x_center(res[2]);//center is 0.5
        norm_rects->at(i).set_y_center(res[3]);//h_mid);//
        norm_rects->at(i).set_rect_id(RIGHT_EAR_RECT);

        // auto h_mid = landmarks.landmark(refRightEarIdxs[4]).y();


        
        // auto ltx = lt.x() * 2.0-1.0, lty = lt.y()* 2.0-1.0;
        // auto lbx = lb.x() * 2.0-1.0, lby = lb.y()* 2.0-1.0;
        // auto rx = (test_length / nose_width > 0.5f)?landmarks.landmark(58).x():landmarks.landmark(215).x();

        // float height = std::abs(lt.y() - lb.y());
        // float width = 0.6f * height;
        // // std::cout<<"height: "<<height<<std::endl;

        // int i=0;
        // norm_rects->at(i).set_width(width);
        // norm_rects->at(i).set_height(height);
        // norm_rects->at(i).set_x_center(rx - width * 0.5f);//center is 0.5
        // norm_rects->at(i).set_y_center((lb.y()+lt.y()) * 0.5f);//h_mid);//
        // norm_rects->at(i).set_rect_id(RIGHT_EAR_RECT);
        // std::cout<<"rect: "<<norm_rects->at(i).x_center()<<" "<<norm_rects->at(i).y_center()<<std::endl;

	}else{
        //empty
        norm_rects->emplace_back(NormalizedRect());
    }

    cc->Outputs()
        .Tag(kEarNormRectsTag)
        .Add(norm_rects.release(), cc->InputTimestamp());
              
    return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
