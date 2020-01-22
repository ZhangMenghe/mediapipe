#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"

// #include <Eigen/Dense>
#include "System.h"

// using Eigen::MatrixXd;
    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    // ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR,true);
namespace mediapipe{
namespace{
  constexpr char kInputVideoTag[] = "IMAGE_ALIGN";
}

class OrbSLAMCalculator : public CalculatorBase {
	public:
		OrbSLAMCalculator() = default;
		~OrbSLAMCalculator() override = default;

		static ::mediapipe::Status GetContract(CalculatorContract* cc);

		// From Calculator.
		::mediapipe::Status Open(CalculatorContext* cc) override;
		::mediapipe::Status Process(CalculatorContext* cc) override;
		::mediapipe::Status Close(CalculatorContext* cc) override;
	private:

};
REGISTER_CALCULATOR(OrbSLAMCalculator);

::mediapipe::Status OrbSLAMCalculator::GetContract(
    CalculatorContract* cc) {
    RET_CHECK(cc->Inputs().HasTag(kInputVideoTag));
    cc->Inputs().Tag(kInputVideoTag).Set<ImageFrame>();
    cc->Inputs().Tag("CAMERA_POSE").Set<std::string>();
    cc->Outputs().Tag("CAMERA_POSE").Set<std::string>();
    return ::mediapipe::OkStatus();
}

::mediapipe::Status OrbSLAMCalculator::Open(CalculatorContext* cc) {
    cc->SetOffset(TimestampDiff(0));
    return ::mediapipe::OkStatus();
}

::mediapipe::Status OrbSLAMCalculator::Process(CalculatorContext* cc) {
	int input_width = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Width();
	int input_height = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Height();

	const auto& input_img = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>();
	cv::Mat input_mat = formats::MatView(&input_img);

	// LOG(INFO) << "====size: "<< input_mat.cols<<"=="<<input_mat.rows;
	// TESTBazelClass tc;
	// cc->Outputs().Tag("CAMERA_POSE").AddPacket(MakePacket<std::string>(tc.getMsg()).At(cc->InputTimestamp()));

	return ::mediapipe::OkStatus();
}
::mediapipe::Status OrbSLAMCalculator::Close(CalculatorContext* cc) {
  return ::mediapipe::OkStatus();
}

} // namespace mediapipe

