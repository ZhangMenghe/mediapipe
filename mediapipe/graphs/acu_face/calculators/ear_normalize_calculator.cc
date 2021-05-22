
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {
using namespace cv;
namespace {
    constexpr char kEarObserversTag[] = "EAR_OBSERVER";
    constexpr char kOutputPosTag[] = "POS_MAT";
}  
// Convert an input image (GpuBuffer or ImageFrame) to ImageFrame.
class EarNormalizeCalculator : public CalculatorBase {
 public:
    EarNormalizeCalculator() {}

    static ::mediapipe::Status GetContract(CalculatorContract* cc);
    ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    int id = 0;
};
REGISTER_CALCULATOR(EarNormalizeCalculator);

// static
::mediapipe::Status EarNormalizeCalculator::GetContract(
    CalculatorContract* cc) {
    cc->Inputs().Index(0).Set<ImageFrame>();
    cc->Outputs().Index(0).Set<ImageFrame>();

	if (cc->Inputs().HasTag(kEarObserversTag)) 
        cc->Inputs().Tag(kEarObserversTag).Set<std::vector<float>>();

    if (cc->Outputs().HasTag(kOutputPosTag))
	    cc->Outputs().Tag(kOutputPosTag).Set<cv::Mat>();

	if (cc->Outputs().HasTag(kEarObserversTag)) 
        cc->Outputs().Tag(kEarObserversTag).Set<std::vector<float>>();
    return ::mediapipe::OkStatus();
}
::mediapipe::Status EarNormalizeCalculator::Process(CalculatorContext* cc){
    const auto& img_frame = cc->Inputs().Index(0).Get<ImageFrame>();
    auto image_rgb = formats::MatView(&img_frame);
    cv::Mat image;
    cvtColor(image_rgb, image, COLOR_RGB2GRAY);

    int size;
    float scaley, scalex, ang, cx, cy;

    if (cc->Inputs().HasTag(kEarObserversTag)){
        auto& observers = cc->Inputs().Tag(kEarObserversTag).Get<std::vector<float>>();
        size = int(observers[0]);
        scaley = observers[1], scalex = observers[2], ang = observers[3], cx = observers[4], cy = observers[5];
    }else{
        size = 96;
        scaley = (fmax(image.cols, image.rows) - 1.0)*0.5; scalex = scaley; ang = .0f; cx = (image.cols - 1.0)*0.5f; cy = (image.rows - 1.0)*0.5f;
    }

    auto output_img = absl::make_unique<ImageFrame>(ImageFormat::GRAY8, size, size);
    cv::Mat interpolated = mediapipe::formats::MatView(output_img.get());

    bool b_store_pos = cc->Outputs().HasTag(kOutputPosTag);
    auto pos_mat = b_store_pos?absl::make_unique<cv::Mat>(CV_32FC1, size, size):nullptr;

	double ratioy = (scaley/((size-1)/2.0)), ratiox = (scalex/((size-1)/2.0));
	for(int i=0; i < size; i++)
		for(int j=0; j < size; j++) {
			double xt = ratiox*(j-(size-1)/2.0), yt = ratioy*(i-(size-1)/2.0);
			double x = xt*cos(ang)-yt*sin(ang)+cx, y = xt*sin(ang)+yt*cos(ang)+cy;
			int u = x, v = y;
			double ul = x-u, vl = y-v;
			int u1 = u+1, v1 = v+1;
			u = max(0, min(image.cols-1, u));
			u1 = max(0, min(image.cols-1, u1));
			v = max(0, min(image.rows-1, v));
			v1 = max(0, min(image.rows-1, v1));
            if(b_store_pos) pos_mat->at<float>(i,j)=v*1000.f+u;

			double tmp = image.at<uchar>(v,u)*(1.0-ul)*(1.0-vl) + image.at<uchar>(v,u1)*ul*(1.0-vl) + image.at<uchar>(v1,u)*(1.0-ul)*vl + image.at<uchar>(v1,u1)*ul*vl;
			interpolated.at<uchar>(i,j) = max(0.0,min(255.0,tmp));	
		}

    // id++;
    // cv::imwrite( "debug_out/test_" + std::to_string(id) +".png" , interpolated);
    // cv::imwrite( "debug_out/crop_" + std::to_string(id) +".png" , image);
   
    cc->Outputs().Index(0).Add(output_img.release(), cc->InputTimestamp());
    if(b_store_pos) cc->Outputs().Index(0).Add(pos_mat.release(), cc->InputTimestamp());

    if(cc->Outputs().HasTag(kEarObserversTag)){
        auto output_observer = absl::make_unique<std::vector<float>>();
        output_observer->reserve(6);
        output_observer->emplace_back(size);
        output_observer->emplace_back(scaley);output_observer->emplace_back(scalex);
        output_observer->emplace_back(ang);output_observer->emplace_back(cx);output_observer->emplace_back(cy);
        
        std::cout<<"before: ";    
        for(int i=0;i<6;i++)std::cout<<output_observer->at(i)<<" ";
        std::cout<<std::endl;
        
        cc->Outputs().Tag(kEarObserversTag).Add(output_observer.release(), cc->InputTimestamp());

    }

    return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
