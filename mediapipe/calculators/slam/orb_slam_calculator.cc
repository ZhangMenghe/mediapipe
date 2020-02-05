#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/canonical_errors.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/calculators/slam/orb_slam_calculator.pb.h"
#include "mediapipe/util/resource_util.h"
#include "System.h"
#include "mediapipe/calculators/slam/slam_calculators.h"



namespace mediapipe{
namespace{
  constexpr char kInputVideoTag[] = "IMAGE";
  constexpr char kOutputVideoTag[] = "IMAGE";
  constexpr char kOutputSLAMTag[] = "SLAM_OUT";
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
  		std::unique_ptr<SLAMData> slam_data_out;
		std::string camera_config_file_;
		std::string voc_file_;
		float img_width = .0f, img_height = .0f;
};
REGISTER_CALCULATOR(OrbSLAMCalculator);

::mediapipe::Status OrbSLAMCalculator::GetContract(
    CalculatorContract* cc) {

	cc->Inputs().Tag(kInputVideoTag).Set<ImageFrame>();

	if (cc->Outputs().HasTag(kOutputSLAMTag)){
		cc->Outputs().Tag(kOutputSLAMTag).Set<SLAMData*>();
	}
	
	if(cc->Outputs().HasTag(kOutputVideoTag)){
		cc->Outputs().Tag("IMAGE").Set<ImageFrame>();
	}
    return ::mediapipe::OkStatus();
}

::mediapipe::Status OrbSLAMCalculator::Open(CalculatorContext* cc) {
    cc->SetOffset(TimestampDiff(0));
	MP_RETURN_IF_ERROR(LoadOptions(cc));
	SLAM = new ORB_SLAM2::System(
		voc_file_,camera_config_file_,
		ORB_SLAM2::System::MONOCULAR, 
		/*use viewer*/false);
	slam_data_out = absl::make_unique<SLAMData>();
	
    return ::mediapipe::OkStatus();
}

::mediapipe::Status OrbSLAMCalculator::Process(CalculatorContext* cc) {
	const auto& input_img = cc->Inputs().Tag("IMAGE").Get<ImageFrame>();
	if(img_width == 0){
		img_width = (float)input_img.Width();
		img_height = (float)input_img.Height();
	}
	cv::Mat input_mat = formats::MatView(&input_img);

	//output an image
	if(cc->Outputs().HasTag(kOutputVideoTag)){
		std::unique_ptr<ImageFrame> output_frame(
		new ImageFrame(input_img.Format(), input_img.Width(), input_img.Height()));
		cv::Mat output_mat = formats::MatView(output_frame.get());
		input_mat.copyTo(output_mat);
		cc->Outputs().Tag(kOutputVideoTag).Add(output_frame.release(), cc->InputTimestamp());
	}
	if (cc->Outputs().HasTag(kOutputSLAMTag) ){
		//calibration
		SLAM->GetTracking()->GetCalibration(slam_data_out->camera_intrinsic, slam_data_out->camera_mDistCoef);

		// Pass the image to the SLAM system
		auto pose = SLAM->TrackMonocular(input_mat, (double)cc->InputTimestamp().Seconds());

		if(pose.empty()){
			slam_data_out->b_tracking_valid = false;
		 	cc->Outputs().Tag(kOutputSLAMTag).AddPacket(MakePacket<SLAMData*>(slam_data_out.get()).At(cc->InputTimestamp()));
			return ::mediapipe::OkStatus();
		}

		slam_data_out->b_tracking_valid = true;
		slam_data_out->camera_pose_mat = pose;

		//keypoints
		auto kps = SLAM->GetTrackedKeyPointsUn();
		slam_data_out->kp_num = kps.size();
		for(int i=0; i<slam_data_out->kp_num; i++){
			slam_data_out->keyPoints[4*i] = (kps[i].pt.x / img_width) * 2.0f - 1.0f; 
			slam_data_out->keyPoints[4*i+1] = kps[i].pt.y / img_height* 2.0f - 1.0f;
          	slam_data_out->keyPoints[4*i+2] = .0f;slam_data_out->keyPoints[4*i+3] = 1.0f;
		}

		//real-world points
		ORB_SLAM2::Map* mpMap = SLAM->GetMap();
		if(!mpMap) LOG(ERROR)<<"MAP ERROR";
		const std::vector<ORB_SLAM2::MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    	const std::vector<ORB_SLAM2::MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();
		std::set<ORB_SLAM2::MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

		slam_data_out->mp_num = 0;
        //   LOG(INFO)<<"CHECK 1";
		
		for(size_t i=0, iend=vpMPs.size(); i<iend && slam_data_out->mp_num < MAX_TRACK_POINT;i++){
			if(vpMPs[i]->isBad() )//|| spRefMPs.count(vpMPs[i]))
				continue;
			cv::Point3f pos = cv::Point3f(vpMPs[i]->GetWorldPos());
			slam_data_out->mapPoints[slam_data_out->mp_num] = pos;
			slam_data_out->mp_num++;
    	}
        //   LOG(INFO)<<"CHECK 2";

		slam_data_out->rp_num = 0;
		for(set<ORB_SLAM2::MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++){
			if(slam_data_out->mp_num >= MAX_TRACK_POINT) break;
			if((*sit)->isBad())
				continue;
			slam_data_out->refPoints[slam_data_out->rp_num] = cv::Point3f((*sit)->GetWorldPos());
			slam_data_out->rp_num++;
		}
        //   LOG(INFO)<<"CHECK 3";

		cc->Outputs().Tag(kOutputSLAMTag).AddPacket(MakePacket<SLAMData*>(slam_data_out.get()).At(cc->InputTimestamp()));
		// cc->Outputs().Tag(kOutputKeyPoints).AddPacket(MakePacket<std::vector<cv::KeyPoint>>(SLAM->GetTrackedKeyPointsUn()).At(cc->InputTimestamp()));
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

