#include "mediapipe/examples/desktop/DemoTask.h"
namespace mediapipe{


class CPUTask : public DemoTask{
public:
	Status Initilization(){
		//init graph
		MP_RETURN_IF_ERROR(init_graph(FLAGS_calculator_graph_config_file));
		//init source 
		MP_RETURN_IF_ERROR(init_capture_src());
		//init writer
		MP_RETURN_IF_ERROR(init_video_writer(FLAGS_output_video_path));
  		return OkStatus();
	}
	Status Run(){
		LOG(INFO) << "Start running the calculator graph.";
		ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
					graph.AddOutputStreamPoller(kOutputStream));
		MP_RETURN_IF_ERROR(graph.StartRun({}));

		LOG(INFO) << "Start grabbing and processing frames.";
		bool grab_frames = true;
		cv::Size m_frame_size = getFrameSize();

		while (grab_frames) {
			cv::Mat camera_frame;
			if(!getFrame(camera_frame)) break;
			auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
			mediapipe::ImageFormat::SRGB, m_frame_size.width, m_frame_size.height,
			mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);

			cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
			camera_frame.copyTo(input_frame_mat);


			// Send image packet into the graph.
			MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
				kInputStream, mediapipe::Adopt(input_frame.release())
								.At(mediapipe::Timestamp(updateTimeStamp()))));

			// Get the graph result packet, or stop if that fails.
			mediapipe::Packet packet;
			if (!poller.Next(&packet)) break;
			auto& output_frame = packet.Get<mediapipe::ImageFrame>();

			// Convert back to opencv for display or saving.
			cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
			cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
			if(!postProcessVideo(output_frame_mat)) grab_frames = false;
  		}
		//finalize
		LOG(INFO) << "Shutting down.";
		if (writer.isOpened()) writer.release();
		MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
		return graph.WaitUntilDone();
	}
};
}

int main(int argc, char** argv) {
	google::InitGoogleLogging(argv[0]);
	gflags::ParseCommandLineFlags(&argc, &argv, true);
	mediapipe::CPUTask cpu_task;

	::mediapipe::Status run_status = cpu_task.Initilization();
	if (!run_status.ok()) {
	LOG(ERROR) << "Failed to Init: " << run_status.message();
	} else {
	LOG(INFO) << "Success Init!";
	}

	run_status = cpu_task.Run();
	if (!run_status.ok()) {
	LOG(ERROR) << "Failed to run the graph: " << run_status.message();
	} else {
	LOG(INFO) << "Success Run!";
	}
	return 0;
}
