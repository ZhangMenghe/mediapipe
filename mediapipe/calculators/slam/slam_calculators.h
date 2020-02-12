#ifndef SLAM_CALCULATORS_H
#define SLAM_CALCULATORS_H

#include "mediapipe/framework/port/opencv_core_inc.h"
#define MAX_KEY_POINT 2000
#define MAX_TRACK_POINT 2000

namespace mediapipe{

struct SLAMData {
    bool plane_detected = false;
    //calibration
    cv::Mat camera_intrinsic;
    cv::Mat camera_mDistCoef;

    //tracking
    bool b_tracking_valid;
    cv::Mat camera_pose_mat;
    //plane
    cv::Mat plane_pose;
    cv::Mat plane_center;

    int kp_num;
    int mp_num;
    int rp_num;

    float keyPoints[4*MAX_KEY_POINT];
    cv::Point3f mapPoints[MAX_TRACK_POINT];
    cv::Point3f refPoints[MAX_TRACK_POINT];
    cv::Point3f planePoints[MAX_TRACK_POINT];
};
}
#endif
