#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

#include "include/TEST_BAZEL.h"

#include<Eigen/Dense>
#include "System.h"
using Eigen::MatrixXd;
namespace mediapipe{
  namespace{
    constexpr char kInputVideoTag[] = "IMAGE_ALIGN";
  }
class OrbSLAMCalculator : public CalculatorBase{
public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    RET_CHECK(cc->Inputs().HasTag(kInputVideoTag));
    cc->Inputs().Tag(kInputVideoTag).Set<ImageFrame>();
    cc->Inputs().Tag("CAMERA_POSE").Set<std::string>();
    cc->Outputs().Tag("CAMERA_POSE").Set<std::string>();
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) final {
    cc->SetOffset(TimestampDiff(0));
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) final {
    int input_width = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Width();
    int input_height = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Height();

    const auto& input_img = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>();
    cv::Mat input_mat = formats::MatView(&input_img);

    // LOG(INFO) << "====size: "<< input_mat.cols<<"=="<<input_mat.rows;
    TESTBazelClass tc;
    cc->Outputs().Tag("CAMERA_POSE").AddPacket(MakePacket<std::string>(tc.getMsg()).At(cc->InputTimestamp()));

    return ::mediapipe::OkStatus();
  }
};
REGISTER_CALCULATOR(OrbSLAMCalculator);
} // namespace mediapipe

