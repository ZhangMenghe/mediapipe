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
  int frame_count = 0;
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
	frame_count++;
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
		CameraData camera;
		//calibration
		SLAM->GetTracking()->GetCalibration(camera.intrinsic, camera.DistCoef);

		// Pass the image to the SLAM system
		camera.pose = SLAM->TrackMonocular(input_mat, (double)cc->InputTimestamp().Seconds());
		camera.valid = !camera.pose.empty();slam_data_out->camera = camera;
		if(!camera.valid){
		 	cc->Outputs().Tag(kOutputSLAMTag).AddPacket(MakePacket<SLAMData*>(slam_data_out.get()).At(cc->InputTimestamp()));
			return ::mediapipe::OkStatus();
		}
		
		//plane
		// planeData plane;
		// cv::Mat Plane2World=cv::Mat::zeros(4,4,CV_32FC1);
		// cv::Mat p_c= cv::Mat::zeros(3,1,CV_32FC1);

		// if(frame_count %50 == 0 )plane.valid = false;
		// if(!plane.valid){
		// 	// LOG(INFO)<<"Update plane";
		// 	ORB_SLAM2::PlaneDetector* pd = SLAM->GetPlane(camera.pose, Plane2World, p_c);
		// 	if(p_c.at<float>(0,0) != .0f && p_c.at<float>(1,0) != .0f && p_c.at<float>(2,0)!= .0f){
		// 		plane.valid = true;
		// 		plane.pose = Plane2World;
		// 		plane.center= p_c;
				
		// 		const std::vector<ORB_SLAM2::MapPoint*> &trackPoints = pd->GetPlanePoints();
		// 		plane.points.num = trackPoints.size();
		// 		for(size_t i=0, iend=trackPoints.size(); i<iend&&i<MAX_TRACK_POINT;i++){
		// 			cv::Point3f pos = cv::Point3f(trackPoints[i]->GetWorldPos());
		// 			plane.points.data[i] = pos;
		// 		}
		// 	}
		// }
		// slam_data_out->plane = plane;

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

		cvPoints mp_points;
		mp_points.num = 0;
		for(size_t i=0, iend=vpMPs.size(); i<iend && mp_points.num < MAX_TRACK_POINT;i++){
			if(vpMPs[i]->isBad() )//|| spRefMPs.count(vpMPs[i]))
				continue;
			cv::Point3f pos = cv::Point3f(vpMPs[i]->GetWorldPos());
			mp_points.data[mp_points.num] = pos;
			mp_points.num++;
    	}
		
		slam_data_out->mapPoints = mp_points;


		cvPoints rf_points;
		rf_points.num = 0;
		for(set<ORB_SLAM2::MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++){
			if(rf_points.num >= MAX_TRACK_POINT) break;
			if((*sit)->isBad() )
				continue;
			cv::Point3f pos = cv::Point3f((*sit)->GetWorldPos());
			rf_points.data[rf_points.num] = pos;
			rf_points.num++;
    	}
		slam_data_out->refPoints = rf_points;

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

