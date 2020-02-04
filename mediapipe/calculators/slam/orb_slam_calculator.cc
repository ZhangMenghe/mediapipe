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
// void fromCV2GLM(const cv::Mat& cvmat, glm::mat4* glmmat) {
    // if (cvmat.cols != 4 || cvmat.rows != 4 || cvmat.type() != CV_32FC1) {
    //     cout << "Matrix conversion error!" << endl;
    //     return;
    // }
    // memcpy(glm::value_ptr(*glmmat), cvmat.data, 16 * sizeof(float));
// 
// }
// void fromCVCamPose2ViewMat(const cv::Mat& mCameraPose, glm::mat4*view_mat){
// 	cv::Mat Rwc(3,3,CV_32FC1);
// 	cv::Mat twc(3,1,CV_32FC1);
// 	{
// 		Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
// 		twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
// 	}
// 	float data[16]={
// 		Rwc.at<float>(0,0), Rwc.at<float>(0,1), Rwc.at<float>(0,2), twc.at<float>(0),
//         Rwc.at<float>(1,0), Rwc.at<float>(1,1), Rwc.at<float>(1,2), twc.at<float>(1),
//         Rwc.at<float>(2,0), Rwc.at<float>(2,1), Rwc.at<float>(2,2), twc.at<float>(2),
//         0.0, .0f,.0f,1.0f
// 	};
// 	// 	float data[16]={
// 	// 	Rwc.at<float>(0,0), Rwc.at<float>(1,0), Rwc.at<float>(2,0), twc.at<float>(0),
//     //     Rwc.at<float>(0,1), Rwc.at<float>(1,1), Rwc.at<float>(2,1), twc.at<float>(1),
//     //     Rwc.at<float>(0,2), Rwc.at<float>(1,2), Rwc.at<float>(2,2), twc.at<float>(2),
//     //     0.0, .0f,.0f,1.0f
// 	// };
// 	memcpy(glm::value_ptr(*view_mat), &data[0], 16 * sizeof(float));
// }
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
		// Pass the image to the SLAM system
		auto pose = SLAM->TrackMonocular(input_mat, (double)cc->InputTimestamp().Seconds());

		if(pose.empty()){
			std::vector<float> data;
		 	cc->Outputs().Tag(kOutputSLAMTag).AddPacket(MakePacket<SLAMData*>(slam_data_out.get()).At(cc->InputTimestamp()));
			return ::mediapipe::OkStatus();
		}
		slam_data_out->camera_pose_mat = pose;
		auto kps = SLAM->GetTrackedKeyPointsUn();
		slam_data_out->kp_num = kps.size();
		for(int i=0; i<slam_data_out->kp_num; i++){
			slam_data_out->kpoints[4*i] = (kps[i].pt.x / img_width) * 2.0f - 1.0f; 
			slam_data_out->kpoints[4*i+1] = kps[i].pt.y / img_height* 2.0f - 1.0f;
          	slam_data_out->kpoints[4*i+2] = .0f;slam_data_out->kpoints[4*i+3] = 1.0f;
		}

		//real-world points
		ORB_SLAM2::Map* mpMap = SLAM->GetMap();
		if(!mpMap) LOG(ERROR)<<"MAP ERROR";
		const std::vector<ORB_SLAM2::MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    	const std::vector<ORB_SLAM2::MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();
		std::set<ORB_SLAM2::MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

		slam_data_out->vp_num = 0;
		for(size_t i=0, iend=vpMPs.size(); i<iend;i++){
			if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
				continue;
			cv::Mat pos = vpMPs[i]->GetWorldPos();
			slam_data_out->vp_mpoints[4*i] = pos.at<float>(0); 
			slam_data_out->vp_mpoints[4*i+1] = pos.at<float>(1);
          	slam_data_out->vp_mpoints[4*i+2] = pos.at<float>(2);slam_data_out->vp_mpoints[4*i+3] = 1.0f;
			slam_data_out->vp_num++;
    	}
		slam_data_out->vpref_num = 0;
		int i=0;
		for(set<ORB_SLAM2::MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++){
			if((*sit)->isBad())
				continue;
			slam_data_out->vpref_num++;	
			cv::Mat pos = (*sit)->GetWorldPos();
			slam_data_out->vp_ref_mpoints[4*i] = pos.at<float>(0); 
			slam_data_out->vp_ref_mpoints[4*i+1] = pos.at<float>(1);
          	slam_data_out->vp_ref_mpoints[4*i+2] = pos.at<float>(2);slam_data_out->vp_ref_mpoints[4*i+3] = 1.0f;
			i++;
		}

		

		// glm::mat4 Projection = glm::perspective(glm::radians(45.0f), img_width/img_height, 0.1f, 100.0f);

		// Camera matrix
		// glm::mat4 View = glm::lookAt(
		// 	glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
		// 	glm::vec3(0,0,0), // and looks at the origin
		// 	glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
		// 	);

		// int point_num = vpMPs.size();//kps.size();
		// int point_num = kps.size();

		// glm::mat4 view_mat;
		// fromCVCamPose2ViewMat(pose, &view_mat);
		// glm::mat4 mvp = Projection * view_mat;

		// std::vector<float>data;//(4 * point_num, 1.0f);
        
		// std::vector<float>data(4 * point_num, 1.0f);

		// for(int i=0; i<point_num; i++){
        //   data[4*i] = (kps[i].pt.x / img_width) * 2.0f - 1.0f; data[4*i+1] = kps[i].pt.y / img_height* 2.0f - 1.0f;
        //   data[4*i+2] = .0f;

		// if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
		// 	continue;
        // cv::Mat pos = vpMPs[i]->GetWorldPos();
		// data.push_back(pos.at<float>(0));data.push_back(pos.at<float>(1));data.push_back(pos.at<float>(2));data.push_back(1.0f);


		// if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
		// continue;
        // cv::Mat pos = vpMPs[i]->GetWorldPos();
        // glm::vec4 pw(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2), 1.0f);
		
		  
		// glm::vec4 pt = mvp * pw;
		// pt = pt/ pt.w;
		// LOG(INFO)<<"pt: "<< pos.at<float>(0)<<" "<< pos.at<float>(1)<<" "<< pos.at<float>(2)<<" "<<pt.x <<" "<<pt.y<<" "<<pt.z;
		// data.push_back(pt.x);data.push_back(pt.y);data.push_back(pt.z);data.push_back(1.0f);
		  
        // }
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

