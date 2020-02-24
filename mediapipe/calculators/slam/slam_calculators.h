#ifndef SLAM_CALCULATORS_H
#define SLAM_CALCULATORS_H

#include "mediapipe/framework/port/opencv_core_inc.h"
#define MAX_KEY_POINT 2000
#define MAX_TRACK_POINT 2000

namespace mediapipe{

struct cvPoints{
    int num;
    cv::Point3f data[MAX_TRACK_POINT];
};
struct planeData{
    bool valid = false;
    cv::Mat pose;
    cv::Mat center;
    cvPoints points;
};
struct CameraData{
    bool valid;
    cv::Mat intrinsic;
    cv::Mat DistCoef;
    cv::Mat pose;
};
struct SLAMData {
    //calibration
    CameraData camera;
    //plane
    planeData plane;

    //key and world points
    int kp_num;
    float keyPoints[4*MAX_KEY_POINT];
    cvPoints mapPoints;
    cvPoints refPoints;
};
}
#endif
