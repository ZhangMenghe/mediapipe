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
  constexpr char kInputVideoTag[] = "IMAGE";
  constexpr char kOutputVideoTag[] = "IMAGE";
  constexpr char kOutputCamPoseTag[] = "CAMERA_POSE";
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

	cc->Inputs().Tag(kInputVideoTag).Set<ImageFrame>();

	if (cc->Outputs().HasTag(kOutputCamPoseTag)){
		cc->Outputs().Tag(kOutputCamPoseTag).Set<std::string>();
	}
	if(cc->Outputs().HasTag(kOutputVideoTag)){
		cc->Outputs().Tag("IMAGE").Set<ImageFrame>();
	}
    return ::mediapipe::OkStatus();
}

::mediapipe::Status OrbSLAMCalculator::Open(CalculatorContext* cc) {
    cc->SetOffset(TimestampDiff(0));
	MP_RETURN_IF_ERROR(LoadOptions(cc));
	SLAM = new ORB_SLAM2::System(voc_file_,camera_config_file_,ORB_SLAM2::System::MONOCULAR,false);
    return ::mediapipe::OkStatus();
}
void slam_thread(ORB_SLAM2::System* sys, cv::Mat img, double stamp){
    cv::Mat pose = sys->TrackMonocular(img, stamp);
	LOG(INFO) << pose;

}
::mediapipe::Status OrbSLAMCalculator::Process(CalculatorContext* cc) {
	// int input_width = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Width();
	// int input_height = cc->Inputs().Tag(kInputVideoTag).Get<ImageFrame>().Height();

	const auto& input_img = cc->Inputs().Tag("IMAGE").Get<ImageFrame>();
	cv::Mat input_mat = formats::MatView(&input_img);

	// Pass the image to the SLAM system
    cv::Mat pose = SLAM->TrackMonocular(input_mat, (double)cc->InputTimestamp().Seconds());
	LOG(INFO) << pose;
	std::unique_ptr<ImageFrame> output_frame(
    	new ImageFrame(input_img.Format(), input_img.Width(), input_img.Height()));
	cv::Mat output_mat = formats::MatView(output_frame.get());
	input_mat.copyTo(output_mat);

	if (cc->Outputs().HasTag(kOutputCamPoseTag)){
		cc->Outputs().Tag(kOutputCamPoseTag).AddPacket(MakePacket<std::string>("AAAAA").At(cc->InputTimestamp()));
	} 
	if(cc->Outputs().HasTag(kOutputVideoTag)){
		cc->Outputs().Tag(kOutputVideoTag).Add(output_frame.release(), cc->InputTimestamp());
	}

  	

	return ::mediapipe::OkStatus();
}
::mediapipe::Status OrbSLAMCalculator::Close(CalculatorContext* cc) {
	 // Stop all threads
    SLAM->Shutdown();
	LOG(INFO)<<"shutting down....";
	// Save camera trajectory
    SLAM->SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");
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

