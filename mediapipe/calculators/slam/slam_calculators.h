#ifndef SLAM_CALCULATORS_H
#define SLAM_CALCULATORS_H

#include "mediapipe/framework/port/opencv_core_inc.h"
#define MAX_KEY_POINT 2000
#define MAX_TRACK_POINT 2000

namespace mediapipe{
struct SLAMData {
    bool b_tracking_valid;
    cv::Mat camera_pose_mat;
    int kp_num;
    int vp_num;
    int vpref_num;

    float kpoints[4*MAX_KEY_POINT];
    float vp_mpoints[4*MAX_TRACK_POINT];
    float vp_ref_mpoints[4*MAX_TRACK_POINT];
};
}
#endif
    // bool b_tracking_valid;
    // cv::Mat camera_pose_mat;
    

    // int kp_num;
    // int map_num;
    // int ref_num;

    // float keyPoints[4*MAX_KEY_POINT];
    // float mapPoints[4*MAX_TRACK_POINT];
    // float refPoints[4*MAX_TRACK_POINT];