PATH_TO_DATA=../data/slam
if [[ $1 == EU ]];then
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam_eu.pbtxt --input_frames_path=$PATH_TO_DATA/data --timestamp_series=$PATH_TO_DATA/timestamp.txt
else
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam.pbtxt --input_video_path=$PATH_TO_DATA/desk.mp4 
fi

