#ifndef MEDIAPIPE_DEMO_TASK_H
#define MEDIAPIPE_DEMO_TASK_H

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/commandlineflags.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include <fstream>

DEFINE_string(calculator_graph_config_file, "",
              "Name of file containing text format CalculatorGraphConfig proto.");
DEFINE_string(input_video_path, "",
              "Full path of video to load. "
              "If not provided, attempt to use a webcam.");
DEFINE_string(input_frames_path, "",
              "Full path of frames of video to load. ");
DEFINE_string(output_video_path, "",
              "Full path of where to save result (.mp4 only). "
              "If not provided, show result in a window.");
DEFINE_string(timestamp_series, "",
              "Full path to predefined timestamps");
namespace mediapipe{
class DemoTask{
protected:
    enum SrcLoadType{
        FROM_CAMERA = 0,
        FROM_VIDEO,
        FROM_FRAMES
    };
    CalculatorGraph graph;
    SrcLoadType load_type;
    cv::VideoCapture capture;
    cv::VideoWriter writer;
    cv::Size frame_size = cv::Size(0,0);
    size_t frame_timestamp = 0;
    const char kInputStream[12] = "input_video";
    const char kOutputStream[22] = "output_video";//13 "output_video";
    const char kWindowName[10] = "MediaPipe";
    const int write_fps = 30;//capture.get(cv::CAP_PROP_FPS)
    std::vector<std::string> frame_paths;
    std::vector<size_t> frame_timestamp_vec;
    size_t frame_nums;

    Status init_capture_src(){
        LOG(INFO) << "Initialize the camera or load the video.";
        if(!FLAGS_input_video_path.empty()) load_type = FROM_VIDEO;
        else if(!FLAGS_input_frames_path.empty()) load_type = FROM_FRAMES;
        else load_type = FROM_CAMERA;

        if(load_type == FROM_FRAMES){
        //load images
        std::ifstream fTimes;
        fTimes.open(FLAGS_timestamp_series.c_str());
        frame_timestamp_vec.reserve(5000);
        frame_paths.reserve(5000);
        while(!fTimes.eof()){
            std::string s;getline(fTimes,s);
            if(!s.empty()){
                std::stringstream ss; ss << s;
                frame_paths.push_back(FLAGS_input_frames_path + "/" + ss.str() + ".png");
                double t;ss >> t;
                //t/1e9
                frame_timestamp_vec.push_back(t);
            }
        }
        frame_nums = frame_paths.size();
        RET_CHECK(frame_nums > 0);
        }else{
            if(load_type == FROM_CAMERA) capture.open(0);
            else capture.open(FLAGS_input_video_path);
            RET_CHECK(capture.isOpened());
	    }
	    return OkStatus();
    }
    bool postProcessVideo(cv::Mat frame){
        if (FLAGS_output_video_path.empty()) {
            cv::imshow(kWindowName, frame);
            // Press any key to exit.
            const int pressed_key = cv::waitKey(5);
            if (pressed_key >= 0 && pressed_key != 255) return false;
        } else {
            writer.write(frame);
        }
        return true;
    }
    cv::Size getFrameSize(){
        if(frame_size.width == 0){
        if(load_type!=FROM_FRAMES){
            cv::Mat frame;
            capture.read(frame);                    // Consume first frame.
            capture.set(cv::CAP_PROP_POS_AVI_RATIO, 0);  // Rewind to beginning.
            frame_size = frame.size();
        }else{
            cv::Mat tframe;
            if(!getFrame(tframe)) return frame_size;
            frame_size = tframe.size(); 
        }}
        return frame_size;
    }
    size_t updateTimeStamp(){
        frame_timestamp++;
        if(load_type == FROM_FRAMES) return frame_timestamp_vec[frame_timestamp];
        return frame_timestamp;
    }

    bool getFrame(cv::Mat& camera_frame){
        if(load_type == FROM_FRAMES){
        // LOG(INFO) << frame_paths[frame_timestamp];
        cv::Mat gray_frame = cv::imread(frame_paths[frame_timestamp], -1);
        if(gray_frame.empty())return false;
        cv::cvtColor(gray_frame,camera_frame, cv::COLOR_GRAY2RGB);
        }else{
        // Capture opencv camera or video frame.
        cv::Mat camera_frame_raw;
        capture >> camera_frame_raw;
        if (camera_frame_raw.empty()) return false;  // End of video.
        cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
        if (load_type == FROM_CAMERA) 
            cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
        }
        return true;
    }
    Status init_graph(std::string config_file_path){
        std::string calculator_graph_config_contents;
        MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
        config_file_path, &calculator_graph_config_contents));

        mediapipe::CalculatorGraphConfig config =
        mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
        calculator_graph_config_contents);

        LOG(INFO) << "Initialize the calculator graph.";
        MP_RETURN_IF_ERROR(graph.Initialize(config));
        return OkStatus();
    }
    Status init_video_writer(std::string out_path){
        if(out_path == ""){
            cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
            return OkStatus();
        }
        
        LOG(INFO) << "Prepare video writer.";
        writer.open(FLAGS_output_video_path,
                    mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    write_fps, getFrameSize());
        RET_CHECK(writer.isOpened());
        return OkStatus();
    }

public:
	Status Initilization();
	Status Run();
};
}

#endif