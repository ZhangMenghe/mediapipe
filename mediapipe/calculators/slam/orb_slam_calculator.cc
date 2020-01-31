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
#include <string>
#include <thread>
#include <regex>
#include <atomic>

#include "ipc.h"


namespace mediapipe{
namespace{
  constexpr char kInputVideoTag[] = "IMAGE_ALIGN";
  constexpr char name__[]="ipc-chat";
  ipc::channel rec_chanel { name__ };
  ipc::channel chat_chanel { name__ };
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
		Status TestIPC();
		::mediapipe::Status LoadOptions(CalculatorContext* cc);

		    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    	ORB_SLAM2::System *SLAM = nullptr;
		std::string camera_config_file_;
		std::string voc_file_;
		ipc::channel send_chanel;
		std::thread rec_thread;
};
REGISTER_CALCULATOR(OrbSLAMCalculator);

::mediapipe::Status OrbSLAMCalculator::GetContract(
    CalculatorContract* cc) {

	if (cc->Outputs().HasTag("IMAGE")) {
		RET_CHECK(cc->Outputs().HasTag("IMAGE"));
		cc->Inputs().Tag("IMAGE").Set<ImageFrame>();
		cc->Outputs().Tag("IMAGE").Set<ImageFrame>();
  	}else{
    RET_CHECK(cc->Inputs().HasTag(kInputVideoTag));
    cc->Inputs().Tag(kInputVideoTag).Set<ImageFrame>();
	}

    // cc->Inputs().Tag("CAMERA_POSE").Set<std::string>();
    // cc->Outputs().Tag("CAMERA_POSE").Set<std::string>();
    return ::mediapipe::OkStatus();
}
// void recv_thread_func(){
// 	while(1){
// 		auto buf = chat_chanel.recv();
// 		if (buf.empty()) continue;
// 		std::string dat { static_cast<char const *>(buf.data()), buf.size() - 1 };
// 		std::cout<<dat<<std::endl;
// 	}
// }
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

	// chat_chanel.send(input_mat.ptr(), input_img.Width() * input_img.Height());
	// 	int count = 0;
	// 	bool flag = true;
		// while(flag){
		// 	auto buf = chat_chanel.recv();
		// 	std::cout<<"wait for "<< count++ <<std::endl;
		// 	if(!buf.empty()){
		// 		std::string dat { static_cast<char const *>(buf.data()), buf.size() - 1 };
		// 		std::cout<<dat<<std::endl;
		// 		// break;
		// 		flag = false;
		// 	}

		// }

	// }
	// Pass the image to the SLAM system
    cv::Mat pose = SLAM->TrackMonocular(input_mat, (double)cc->InputTimestamp().Seconds());
	LOG(INFO) << pose;
	// std::thread r(slam_thread, SLAM, input_mat, (double)cc->InputTimestamp().Seconds());
	// r.join();
	// TESTBazelClass tc;tc.getMsg()
	// cc->Outputs().Tag("CAMERA_POSE").AddPacket(MakePacket<std::string>("test").At(cc->InputTimestamp()));
	std::unique_ptr<ImageFrame> output_frame(
    	new ImageFrame(input_img.Format(), input_img.Width(), input_img.Height()));
	cv::Mat output_mat = formats::MatView(output_frame.get());
	input_mat.copyTo(output_mat);

  	cc->Outputs().Tag("IMAGE").Add(output_frame.release(), cc->InputTimestamp());
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
std::size_t calc_unique_id() {
    static ipc::shm::handle g_shm { "__CHAT_ACC_STORAGE__", sizeof(std::atomic<std::size_t>) };
    return static_cast<std::atomic<std::size_t>*>(g_shm.get())->fetch_add(1, std::memory_order_relaxed);
}
void recThread(string id, std::regex reg){
	while(1){
		std::cout << "try to receive" << std::endl;
	auto buf = rec_chanel.recv();
	if (buf.empty()) return;
	std::string dat { static_cast<char const *>(buf.data()), buf.size() - 1 };
	std::smatch mid;
	if (std::regex_match(dat, mid, reg)) {
		if (mid.str(1) == id) {
			if (mid.str(2) == "q") {
				std::cout << "receiver quit..." << std::endl;
				return;
			}
			return;
		}
	}
	std::cout << dat << std::endl;
	}
}
Status OrbSLAMCalculator::TestIPC(){
	// char name__[] = "ipc-chat";
	char quit__[] = "q";
	char id__  [] = "c";
	std::string buf, id = id__ + std::to_string(calc_unique_id());
    std::regex  reg { "(c\\d+)> (.*)" };

    send_chanel = ipc::channel{ name__ };

// ipc::channel rec_chanel { name__ };
// std::thread r(recThread, id, reg);
// r.join();
// std::thread t(sendThread);
// t.join();
    // std::thread r {[&id, &reg] {
	// 	char name__[] = "ipc-chat";
	// 		char quit__[] = "q";
	// char id__  [] = "c";
    //     ipc::channel cc { name__ };
    //     std::cout << id << " is ready." << std::endl;
    //     while (1) {
    //         auto buf = cc.recv();
    //         if (buf.empty()) continue;
    //         std::string dat { static_cast<char const *>(buf.data()), buf.size() - 1 };
    //         std::smatch mid;
    //         if (std::regex_match(dat, mid, reg)) {
    //             if (mid.str(1) == id) {
    //                 if (mid.str(2) == quit__) {
    //                     std::cout << "receiver quit..." << std::endl;
    //                     return;
    //                 }
    //                 continue;
    //             }
    //         }
    //         std::cout << dat << std::endl;
    //     }
    // }};

    // while (1) {
    //     std::cin >> buf;
    //     send_chanel.send(id + "> " + buf);
    //     if (buf == quit__) break;
    // }

    
	return OkStatus();
}
} // namespace mediapipe

