// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// An example of sending OpenCV webcam frames into a MediaPipe graph.
// This example requires a linux computer and a GPU with EGL support drivers.

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
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gpu_shared_data_internal.h"
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

namespace mediapipe {
class GPUTask{
private:
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
  mediapipe::GlCalculatorHelper gpu_helper;

  const char kInputStream[12] = "input_video";
  const char kOutputStream[13] = "output_video";
  const char kWindowName[10] = "MediaPipe";
  const int write_fps = 30;//capture.get(cv::CAP_PROP_FPS)
  std::vector<std::string> frame_paths;
  std::vector<size_t> frame_timestamp_vec;
  size_t frame_nums;

  Status init_capture_src();
  bool postProcessVideo(cv::Mat frame);
  cv::Size getFrameSize();
  size_t updateTimeStamp();
  
  bool getFrame(cv::Mat& camera_frame);

public:
  Status Initilization();
  Status Run();
};

  size_t GPUTask::updateTimeStamp(){
    frame_timestamp++;
    if(load_type == FROM_FRAMES){
      // LOG(INFO)<<frame_timestamp_vec[frame_timestamp];
      return frame_timestamp_vec[frame_timestamp];
    } 
    return frame_timestamp;
  }

Status GPUTask::init_capture_src(){
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
          // t = t / 1e9;
          frame_timestamp_vec.push_back(t);
          // LOG(INFO)<<"=="<<s;
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

Status GPUTask::Initilization(){
  //init config
  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
      FLAGS_calculator_graph_config_file, &calculator_graph_config_contents));
  // LOG(INFO) << "Get calculator graph config contents: "
  //           << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  LOG(INFO) << "Initialize the calculator graph.";
  
  //init graph
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  //init gpu
  LOG(INFO) << "Initialize the GPU.";
  ASSIGN_OR_RETURN(auto gpu_resources, mediapipe::GpuResources::Create());
  MP_RETURN_IF_ERROR(graph.SetGpuResources(std::move(gpu_resources)));
  
  gpu_helper.InitializeForTest(graph.GetGpuResources().get());
  
  //init capture src
  RET_CHECK(init_capture_src().ok());
  
  //init writer
  if (!FLAGS_output_video_path.empty()) {
    LOG(INFO) << "Prepare video writer.";
    
    writer.open(FLAGS_output_video_path,
                mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                write_fps, getFrameSize());
    RET_CHECK(writer.isOpened());
  } else {
    cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
  }

  return OkStatus();
}
cv::Size GPUTask::getFrameSize(){
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
    }
  }
  return frame_size;
}
bool GPUTask::getFrame(cv::Mat& camera_frame){
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
    // if (load_type == FROM_CAMERA) 
    //   cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
  }
  return true;
}
bool GPUTask::postProcessVideo(cv::Mat frame){
  if (FLAGS_output_video_path.empty()) {
    cv::imshow(kWindowName, frame);
      const int pressed_key = cv::waitKey(5);
      if(pressed_key == 32){//space to pause
        while(cv::waitKey(0) != 32);
      }else if(pressed_key == 27 || pressed_key == 81 || pressed_key == 113)//esc, q, Q to exit
        return false;
    } else {
      writer.write(frame);
    }
    return true;
}
Status GPUTask::Run(){
  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                  graph.AddOutputStreamPoller(kOutputStream));
  MP_RETURN_IF_ERROR(graph.StartRun({}));
  
  
  LOG(INFO) << "Start grabbing and processing frames.";
  
  bool grab_frames = true;

  // Wrap Mat into an ImageFrame.

  cv::Size m_frame_size = getFrameSize();
  auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
      mediapipe::ImageFormat::SRGB, m_frame_size.width, m_frame_size.height,
      mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);

  bool pause = false;
  while(grab_frames){
    cv::Mat camera_frame;
    if(!getFrame(camera_frame)) break;
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Prepare and add graph input packet.
    MP_RETURN_IF_ERROR(
      gpu_helper.RunInGlContext([this, &input_frame]() -> Status {
        // Convert ImageFrame to GpuBuffer.
        auto texture = gpu_helper.CreateSourceTexture(*input_frame.get());
        auto gpu_frame = texture.GetFrame<mediapipe::GpuBuffer>();
        glFlush();
        texture.Release();
        // Send GPU image packet into the graph.
        MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
            kInputStream, mediapipe::Adopt(gpu_frame.release())
                              .At(mediapipe::Timestamp(updateTimeStamp()))));
        return OkStatus();
      }));
    // Get the graph result packet, or stop if that fails.
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) break;
    std::unique_ptr<mediapipe::ImageFrame> output_frame;
    // LOG(INFO) << packet.Get<std::string>();

    // LOG(INFO) << "====String info: "<< packet.Get<std::string>();
    // Convert GpuBuffer to ImageFrame.
    MP_RETURN_IF_ERROR(gpu_helper.RunInGlContext(
      [this,&packet,&output_frame]() -> Status {
        auto& gpu_frame = packet.Get<mediapipe::GpuBuffer>();
        auto texture = gpu_helper.CreateSourceTexture(gpu_frame);
        output_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormatForGpuBufferFormat(gpu_frame.format()),
            gpu_frame.width(), gpu_frame.height(),
            mediapipe::ImageFrame::kGlDefaultAlignmentBoundary);
        gpu_helper.BindFramebuffer(texture);
        const auto info =
            mediapipe::GlTextureInfoForGpuBufferFormat(gpu_frame.format(), 0);
        glReadPixels(0, 0, texture.width(), texture.height(), info.gl_format,
                      info.gl_type, output_frame->MutablePixelData());
        glFlush();
        texture.Release();
        return OkStatus();
      }));

    // Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = mediapipe::formats::MatView(output_frame.get());
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    // LOG(INFO) << "==="<<output_frame_mat.cols;
    if(!postProcessVideo(output_frame_mat)) grab_frames = false;
    // sleep(1);
  }

  //finalize
  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  return graph.WaitUntilDone();
}
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  mediapipe::GPUTask gpu_task;
  ::mediapipe::Status run_status = gpu_task.Initilization();
  

  if (!run_status.ok()) {
    LOG(ERROR) << "Failed to Init: " << run_status.message();
  } else {
    LOG(INFO) << "Success Init!";
  }

  run_status = gpu_task.Run();
    if (!run_status.ok()) {
    LOG(ERROR) << "Failed to run the graph: " << run_status.message();
  } else {
    LOG(INFO) << "Success Run!";
  }

  return 0;
}
