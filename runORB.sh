PATH_TO_DATA=../data/slam
if [[ $1 == EU ]];then
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam_eu.pbtxt --input_frames_path=$PATH_TO_DATA/data --timestamp_series=$PATH_TO_DATA/timestamp.txt
else
    if [ -d "frames/"$2 ]; then
        rm -rf "frames/"$2
    fi
    if [ -d "mappoints/"$2 ]; then
        rm -rf "mappoints/"$2
    fi
    if [ -d "frame_with_points/"$2 ]; then
        rm -rf "frame_with_points/"$2
    fi
    # mkdir "frames/"$2
    # mkdir "mappoints/"$2
    # mkdir "frame_with_points/"$2
    
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam.pbtxt --input_video_path=$PATH_TO_DATA/$2.mp4 --output_frame_path="frames/"$2"/" --output_framewpoint_path="frame_with_points/"$2"/"
fi

