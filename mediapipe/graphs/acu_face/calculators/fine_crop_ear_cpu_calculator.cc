
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
    constexpr char kFlagTag[] = "FLAG";
    constexpr char kOutputImgSize[] = "IMG_SIZE";
    constexpr char kEarFlipTag[] = "EAR_FLIPPED";

    const static int RIGHT_EAR_RECT = 0, LEFT_EAR_RECT=1;
}  
// Convert an input image (GpuBuffer or ImageFrame) to ImageFrame.
class FineCropEarCpuCalculator : public CalculatorBase {
 public:
    FineCropEarCpuCalculator() {}

    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;
    ::mediapipe::Status Process(CalculatorContext* cc) override;
  private:
    int id = 0;
    cv::Mat open_kernel;
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

    if (cc->Outputs().HasTag(kFlagTag))
      cc->Outputs().Tag(kFlagTag).Set<bool>();
    if (cc->Outputs().HasTag(kOutputImgSize)) 
        cc->Outputs().Tag(kOutputImgSize).Set<cv::Size>();
    if (cc->Outputs().HasTag(kEarFlipTag))
        cc->Outputs().Tag(kEarFlipTag).Set<bool>();
    return ::mediapipe::OkStatus();
}
::mediapipe::Status FineCropEarCpuCalculator::Open(CalculatorContext* cc){
    open_kernel = getStructuringElement( cv::MORPH_RECT, Size( 5,5 ));
    return ::mediapipe::OkStatus();
}
::mediapipe::Status FineCropEarCpuCalculator::Process(CalculatorContext* cc){
    const auto& img = cc->Inputs().Index(0).Get<ImageFrame>();
    auto frame = formats::MatView(&img);

    if (!(frame.type() == CV_8UC3 || frame.type() == CV_8UC4)) {
      LOG(ERROR) << "Only 3 or 4 channel 8-bit input image supported";
      
    }

    auto& input_rect = cc->Inputs().Tag(kEarNormRectTag).Get<NormalizedRect>();
    auto output_rect = absl::make_unique<NormalizedRect>(input_rect);

    if (cc->Outputs().HasTag(kEarFlipTag))
      cc->Outputs().Tag(kEarFlipTag).Add(new bool(input_rect.rect_id() == LEFT_EAR_RECT), cc->InputTimestamp());

    if(input_rect.width() == 0 || input_rect.height() == 0){
          cc->Outputs().Tag(kFlagTag).Add(new bool(false), cc->InputTimestamp());
    }else{
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

      const int offset = 10;

      if(target_c_id!=-1){
        auto br = boundingRect(contours[target_c_id]);
        int off_x = max(0, br.x - offset), off_y = max(0, br.y-offset);
        br.width+=abs(off_x - br.x); br.y+=abs(off_y-br.y);
        br.x = off_x; br.y=off_y;

        // std::cout<<"bounding rect : "<<br.width<<" "<<br.height<<" "<<br.x<<" "<<br.y<<std::endl;
        float frame_width = frame.cols; float frame_height = frame.rows;
        float img_width = frame_width / input_rect.width();
        float c2x_abs = input_rect.x_center() * img_width + (frame_width-br.width)*0.5f;

        float img_height = frame_height / input_rect.height();
        float c2y_abs = input_rect.y_center() * img_height - (frame_height-br.height)*0.5f;

        output_rect->set_width(input_rect.width() * br.width / frame_width);
        output_rect->set_height(input_rect.height() * br.height / frame_height);
        output_rect->set_x_center(c2x_abs / img_width);
        output_rect->set_y_center(c2y_abs / img_height);

        frame = frame(br);
        // if(id %5 ==0) cv::imwrite( "debug_out/crop_" + std::to_string(id/5) +".png" , frame);
      }
      cc->Outputs().Tag(kFlagTag).Add(new bool(true), cc->InputTimestamp());
    
    }
    cc->Outputs().Tag(kEarNormRectTag).Add(output_rect.release(), cc->InputTimestamp());

    auto output_img = absl::make_unique<ImageFrame>(img.Format(), frame.cols, frame.rows);
    cv::Mat output_mat = mediapipe::formats::MatView(output_img.get());
    frame.copyTo(output_mat);

    cc->Outputs().Index(0).Add(output_img.release(), cc->InputTimestamp());
   

    auto output_size = absl::make_unique<cv::Size>();
    output_size->width = frame.cols; output_size->height=frame.rows;
    cc->Outputs().Tag(kOutputImgSize).Add(output_size.release(), cc->InputTimestamp());

    return ::mediapipe::OkStatus();
}

}  // namespace mediapipe
