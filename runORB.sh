PATH_TO_DATA=../ORB_SLAM2/Examples/Monocular
PATH_TO_VIDEO = ../data/slam
if [[ $1 == EU ]];then
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam_eu.pbtxt --input_frames_path=$PATH_TO_DATA/mav0/cam0/data --timestamp_series=$PATH_TO_DATA/EuRoC_TimeStamps/V101.txt
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
    
    GLOG_logtostderr=1 bazel-bin/mediapipe/examples/desktop/orb_slam/orb_slam --calculator_graph_config_file=mediapipe/graphs/orb_slam/orb_slam.pbtxt --input_video_path=$PATH_TO_VIDEO/$2.mp4
fi

