#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_core_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/calculators/slam/orb_slam_calculator.pb.h"

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
		::mediapipe::Status LoadOptions(CalculatorContext* cc);

		ORB_SLAM2::System *SLAM = nullptr;
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
  const auto& options =
      cc->Options<::mediapipe::OrbSLAMCalculatorOptions>();
LOG(INFO) << "===="<<options.voc_path();
LOG(INFO) << "===="<<options.camera_path();

//   // Get model name.
//   if (!options.model_path().empty()) {
//     auto model_path = options.model_path();

//     ASSIGN_OR_RETURN(model_path_, mediapipe::PathToResourceAsFile(model_path));
//   } else {
//     LOG(ERROR) << "Must specify path to TFLite model.";
//     return ::mediapipe::Status(::mediapipe::StatusCode::kNotFound,
//                                "Must specify path to TFLite model.");
//   }

//   // Get execution modes.
//   gpu_inference_ = options.use_gpu();

  return ::mediapipe::OkStatus();
}
} // namespace mediapipe

