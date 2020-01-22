#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/calculators/slam/orb_slam_calculator.pb.h"
#include "mediapipe/util/resource_util.h"

#include "System.h"
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
		::mediapipe::Status LoadOptions(CalculatorContext* cc);

		    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    	ORB_SLAM2::System *SLAM = nullptr;
		std::string camera_config_file_;
		std::string voc_file_;
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
	MP_RETURN_IF_ERROR(LoadOptions(cc));
	SLAM = new ORB_SLAM2::System(voc_file_,camera_config_file_,ORB_SLAM2::System::MONOCULAR,false);
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

::mediapipe::Status OrbSLAMCalculator::LoadOptions(
    CalculatorContext* cc) {
  // Get calculator options specified in the graph.
	const auto& options = cc->Options<::mediapipe::OrbSLAMCalculatorOptions>();
	// LOG(INFO) << "===="<<options.voc_path();
	// LOG(INFO) << "===="<<options.camera_path();

	// Get model name.
	if (!options.camera_path().empty()) {
		auto camera_config_file = options.camera_path();
		auto voc_file = options.voc_path();
		ASSIGN_OR_RETURN(camera_config_file_, mediapipe::PathToResourceAsFile(camera_config_file));
		ASSIGN_OR_RETURN(voc_file_, mediapipe::PathToResourceAsFile(voc_file));
	} else {
		LOG(ERROR) << "Must specify path to camera configs and vocabulary file.";
		return ::mediapipe::Status(::mediapipe::StatusCode::kNotFound,
								"Must specify path to TFLite model.");
	}

	return ::mediapipe::OkStatus();
}
} // namespace mediapipe

