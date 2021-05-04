
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/formats/rect.pb.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"



namespace mediapipe {
using namespace cv;
namespace {
    constexpr char kEarNormRectTag[] = "NORM_RECT";
}  
// Convert an input image (GpuBuffer or ImageFrame) to ImageFrame.
class FineCropEarCpuCalculator : public CalculatorBase {
 public:
  FineCropEarCpuCalculator() {}

  static ::mediapipe::Status GetContract(CalculatorContract* cc);

  ::mediapipe::Status Open(CalculatorContext* cc) override;
  ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    cv::Mat open_kernel;
    // Ptr<cv::ximgproc::StructuredEdgeDetection> pDollar;
    int frame_width = -1, frame_height = -1;
};
REGISTER_CALCULATOR(FineCropEarCpuCalculator);

// static
::mediapipe::Status FineCropEarCpuCalculator::GetContract(
    CalculatorContract* cc) {
    cc->Inputs().Index(0).Set<ImageFrame>();
    cc->Outputs().Index(0).Set<ImageFrame>();

    if (cc->Inputs().HasTag(kEarNormRectTag))
	    cc->Inputs().Tag(kEarNormRectTag).Set<NormalizedRect>();

    if (cc->Outputs().HasTag(kEarNormRectTag))
	    cc->Outputs().Tag(kEarNormRectTag).Set<NormalizedRect>();

    return ::mediapipe::OkStatus();
}
::mediapipe::Status FineCropEarCpuCalculator::Open(CalculatorContext* cc){
    open_kernel = getStructuringElement( cv::MORPH_RECT, Size( 5,5 ));
    return ::mediapipe::OkStatus();
}
::mediapipe::Status FineCropEarCpuCalculator::Process(CalculatorContext* cc){
    const auto& img = cc->Inputs().Index(0).Get<ImageFrame>();

    auto frame = formats::MatView(&img);
    Mat frame_HSV, hsv_mask;
    // Convert from BGR to HSV colorspace
    cvtColor(frame, frame_HSV, COLOR_RGB2HSV);
    
    // Detect the object based on HSV Range Values
    inRange(frame_HSV, Scalar(0, 48, 80), Scalar(20, 255, 255), hsv_mask);

    Mat opening;
    morphologyEx(hsv_mask, opening, cv::MORPH_OPEN, open_kernel);

    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    findContours( opening, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

    int target_c_id = -1;
    double max_area = .0;
    for (unsigned int i = 0;  i < contours.size();  i++){
    //  std::cout << "# of contour points: " << contours[i].size() << std::endl;
        auto area = contourArea(contours[i]);
        if(max_area < area){
          max_area = area; target_c_id = i;
        }
    }


  
    auto& input_rect = cc->Inputs().Tag(kEarNormRectTag).Get<NormalizedRect>();
    auto output_rect = absl::make_unique<NormalizedRect>(input_rect);

    if(target_c_id!=-1){
      auto br = boundingRect(contours[target_c_id]);
      //  std::cout<<"bounding rect : "<<bounding_rect.width<<" "<<bounding_rect.height<<" "<<bounding_rect.x<<" "<<bounding_rect.y<<std::endl;

      if(frame_width < 0){
        frame_width = frame.cols; frame_height = frame.rows;
      }

      frame = frame(br);
      
      output_rect->set_width(input_rect.width() * float(br.width / frame_width));
      output_rect->set_height(input_rect.height() * float(br.height / frame_height));
      output_rect->set_x_center(input_rect.x_center() + br.x/frame_width * 0.5f);
      output_rect->set_y_center(input_rect.y_center() + br.y/frame_height * 0.5f);
    }
    
    cc->Outputs().Tag(kEarNormRectTag).Add(output_rect.release(), cc->InputTimestamp());
    auto output_img = absl::make_unique<ImageFrame>(img.Format(), frame.cols, frame.rows);
    cv::Mat output_mat = mediapipe::formats::MatView(output_img.get());
    output_mat = frame.clone();

    cc->Outputs().Index(0).Add(output_img.release(), cc->InputTimestamp());

    return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
