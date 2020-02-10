#include <cmath>
#include <vector>
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/port/ret_check.h"

//gpu
#include "mediapipe/gpu/gl_calculator_helper.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/gpu/gl_quad_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_point_renderer.h"
#include "mediapipe/gpu/glrenderer/gl_cube_renderer.h"
#include "mediapipe/calculators/slam/slam_calculators.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtx/string_cast.hpp>
#include <opencv2/calib3d.hpp>

namespace mediapipe {
namespace {
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
    constexpr char kInputSLAMTag[] = "SLAM_OUT";
    void fromCV2GLM(const cv::Mat& cvmat, glm::mat4* glmmat) {
      if (cvmat.cols != 4 || cvmat.rows != 4 || cvmat.type() != CV_32FC1) {
          LOG(INFO)<<cvmat.cols<<" "<<cvmat.rows;
          return;
      }
      memcpy(glm::value_ptr(*glmmat), cvmat.data, 16 * sizeof(float));
      // *glmmat = glm::transpose(*glmmat);
    }
glm::mat4 getGLViewMatFromCV(glm::mat4 rotation, glm::mat4 translation){
    //   glm::mat4 d; d[1][1]=-1.0f; d[2][2]=-1.0f;
    // // LOG(INFO) << "d: "<<glm::to_string(d) << std::endl;
    // rotation = rotation * d;
    // translation = translation *d;
    // // LOG(INFO) << "r: "<<glm::to_string(rotation) << std::endl;
    // // LOG(INFO) << "t: "<<glm::to_string(translation) << std::endl;
    // rotation[0][3] = translation[0][0];rotation[1][3] = translation[1][0];rotation[2][3] = translation[2][0];rotation[3][3] = 1.0f;
    
    

    // return rotation;



}
cv::Mat getGLProjectionMatrixFromCV(cv::Mat scaledCameraMatrix, float imageWidth, float imageHeight, float far, float near){
  cv::Mat projMat = cv::Mat::zeros(4, 4, CV_32FC1);
  projMat.at<float>(0, 0) = 2*scaledCameraMatrix.at<float>(0, 0)/imageWidth;
  projMat.at<float>(0, 2) = -1 + (2*scaledCameraMatrix.at<float>(0, 2)/imageWidth);
  projMat.at<float>(1, 1) = 2*scaledCameraMatrix.at<float>(1, 1)/imageHeight;
  projMat.at<float>(1, 2) = -1 + (2*scaledCameraMatrix.at<float>(1, 2)/imageHeight);
  projMat.at<float>(2, 2) = -(far+near)/(far-near);
  projMat.at<float>(2, 3) = -2*far*near/(far-near);
  projMat.at<float>(3, 2) = -1;

  //since we are in landscape mode
  // cv::Mat rot2D = cv::Mat::eye(4, 4, CV_32FC1);
  // rot2D.at<float>(0, 0) = rot2D.at<float>(1, 1) = 0;
  // rot2D.at<float>(0, 1) = 1;
  // rot2D.at<float>(1, 0) = -1;

  // projMat = rot2D * projMat;
  return projMat;
  // glm::mat4 res;
  // fromCV2GLM(projMat, &res);
  // return res;
}
glm::mat4 getGLModelViewMatrixFromCV(cv::Mat expandedR, cv::Mat t){
    // cv::Mat expandedR;
    // cv::Rodrigues(r, expandedR);
    LOG(INFO)<<"CAM POS "<<t.at<float>(0, 0)<<" "<<t.at<float>(1, 0)<< " "<<t.at<float>(2, 0);
    cv::Mat Rt = cv::Mat::zeros(4, 4, CV_32FC1);
    for (int y = 0; y < 3; y++) {
      for (int x = 0; x < 3; x++) {
          Rt.at<float>(y, x) = expandedR.at<float>(y, x);
      }
      Rt.at<float>(y, 3) = t.at<float>(y, 0);
    }
    Rt.at<float>(3, 3) = 1.0;

    //OpenGL has reversed Y & Z coords
    cv::Mat reverseYZ = cv::Mat::eye(4, 4, CV_32FC1);
    reverseYZ.at<float>(1, 1) = -1;
    reverseYZ.at<float>(2, 2) = -1;
        
    cv::Mat mvMat = reverseYZ * Rt;
    // return mvMat;
    glm::mat4 res;
    fromCV2GLM(mvMat, &res);
    return res;
}
glm::mat4 getViewMatFromCV(cv::Mat mat){
  glm::mat4 pose, trans=glm::mat4(1.0f);
  // fromCV2GLM(mat, &pose);
  pose[0][0] = mat.at<float>(0,0);pose[0][1] = mat.at<float>(0,1);pose[0][2] = mat.at<float>(0,2);
  pose[1][0] = mat.at<float>(1,0);pose[1][1] = mat.at<float>(1,1);pose[1][2] = mat.at<float>(1,2);
  pose[2][0] = mat.at<float>(2,0);pose[2][1] = mat.at<float>(2,1);pose[2][2] = mat.at<float>(2,2);
  pose[0][3] = .0f;pose[1][3] = .0f;pose[2][3] = .0f;pose[3][3] = 1.0f;
  trans[0][3] = mat.at<float>(0,3);trans[1][3] = mat.at<float>(1,3);trans[2][3] = mat.at<float>(2,3);trans[3][3] = 1.0f;
  // pose = glm::transpose(pose);
  // trans[0][3] = mat.at<float>(3,0);trans[1][3] = mat.at<float>(3,1);trans[2][3] = mat.at<float>(3,2);trans[3][3] = 1.0f;
  
  return getGLViewMatFromCV(pose, trans);
    // glm::mat4 d; d[1][1]=-1.0f; d[2][2]=-1.0f;
    // // LOG(INFO) << "d: "<<glm::to_string(d) << std::endl;
    // glm::mat4 pose;
    // pose[0][0] = mat.at<float>(0,0);pose[0][1] = mat.at<float>(0,1);pose[0][2] = mat.at<float>(0,2);
    // pose[1][0] = mat.at<float>(1,0);pose[1][1] = mat.at<float>(1,1);pose[1][2] = mat.at<float>(1,2);
    // pose[2][0] = mat.at<float>(2,0);pose[2][1] = mat.at<float>(2,1);pose[2][2] = mat.at<float>(2,2);
    // pose = glm::transpose(pose);
    // pose = d*pose;
    // pose = glm::transpose(pose);
    
    // pose[0][3] = mat.at<float>(0,3);pose[1][3] = mat.at<float>(1,3);pose[2][3] = mat.at<float>(2,3);
    // pose[3][0] = .0f;pose[3][1] = .0f;pose[3][2] = .0f;pose[3][3] = 1.0f;
    //       // LOG(INFO) << glm::to_string(pose) << std::endl;

    // return pose;


}
glm::mat4 getGLProjMatFromCV(cv::Mat mat, float width, float height, float zfar, float znear){
  float fx = mat.at<float>(0,0);
  float fy = mat.at<float>(1,1);
  float cx = mat.at<float>(0,2);
  float cy = mat.at<float>(1,2);
  // LOG(INFO)<<"cm "<<fx <<" "<< fy <<" "<<cx <<" "<<cy;
  glm::mat4 m;
  m[0][0] = 2.0 * fx / width;
  m[0][1] = 0.0;
  m[0][2] = 0.0;
  m[0][3] = 0.0;

  m[1][0] = 0.0;
  m[1][1] = 2.0 * fy / height;
  m[1][2] = 0.0;
  m[1][3] = 0.0;

  m[2][0] = 1.0 - 2.0 * cx / width;
  m[2][1] = 2.0 * cy / height - 1.0;
  m[2][2] = (zfar + znear) / (znear - zfar);
  m[2][3] = -1.0;

  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 2.0 * zfar * znear / (znear - zfar);
  m[3][3] = 0.0;
  return m;
}


}
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
    std::unique_ptr<QuadRenderer> quad_renderer_;
    std::unique_ptr<PointRenderer> point_renderer_;
    std::unique_ptr<CubeRenderer> cube_renderer_;

    float* point_cloud;
    int point_num;
    bool b_render_point = true;
    float img_width = .0f, img_height=.0f;
    glm::mat4 projMat;
    float map_point[4* MAX_TRACK_POINT];
    
    Status RenderGpu(CalculatorContext* cc){
      auto& input = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
      GlTexture src1 = gpu_helper.CreateSourceTexture(input);

      int output_width = input.width(), output_height = input.height();
      auto dst = gpu_helper.CreateDestinationTexture(output_width, output_height,
                                              input.format());
      gpu_helper.BindFramebuffer(dst);  // GL_TEXTURE0
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), src1.name());

      
      if(!quad_renderer_){
          quad_renderer_ = absl::make_unique<QuadRenderer>();
          MP_RETURN_IF_ERROR(quad_renderer_->GlSetup());
      }
      QuadRenderer* qrenderer = quad_renderer_.get();
      
      MP_RETURN_IF_ERROR(qrenderer->GlRender(
        src1.width(), src1.height(), dst.width(), dst.height(), FrameScaleMode::kFit, FrameRotation::kNone, false, false, false));
        // if(b_render_point){
        if(!point_renderer_){
            point_renderer_ = absl::make_unique<PointRenderer>();
            MP_RETURN_IF_ERROR(point_renderer_->GlSetup());
        }
        if(!cube_renderer_){
            cube_renderer_ = absl::make_unique<CubeRenderer>();
            MP_RETURN_IF_ERROR(cube_renderer_->GlSetup());
        }
        
        CubeRenderer* cube_r = cube_renderer_.get();
        

        PointRenderer* prenderer = point_renderer_.get();

        auto slam_data =  cc->Inputs().Tag(kInputSLAMTag).Get<SLAMData*>();
        bool draw_real = true;

        if(draw_real&&slam_data->b_tracking_valid&&slam_data->mp_num){
          cv::Mat rVec;
          cv::Rodrigues(slam_data->camera_pose_mat.colRange(0, 3).rowRange(0, 3), rVec);
          cv::Mat tVec = slam_data->camera_pose_mat.col(3).rowRange(0, 3);


          // glm::mat4 proj_mat, view_mat;
          auto proj_mat = getGLProjMatFromCV(slam_data->camera_intrinsic, img_width, img_height, 1000.0f, 0.01f);
          auto view_mat = getGLModelViewMatrixFromCV(slam_data->camera_pose_mat.colRange(0, 3).rowRange(0, 3), tVec);
          // proj_mat = glm::transpose(proj_mat);
          glm::mat4 mvp_gl = proj_mat * view_mat;
          cube_r->GlRender(mvp_gl 
          * glm::translate(glm::mat4(1.0f), glm::vec3(.0f, .0f, 1.0f)) 
          
          * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)));

          /*for(int i=0;i<slam_data->mp_num;i++){
            map_point[4*i] = tVec.at<float>(0, 0);//slam_data->mapPoints[i].x;
            map_point[4*i+1] = tVec.at<float>(1, 0);//-slam_data->mapPoints[i].y;
            map_point[4*i+2] = tVec.at<float>(2, 0)+1.0f;//-slam_data->mapPoints[i].z;
            map_point[4*i+3] = 1.0f;
            // LOG(INFO)<<slam_data->mapPoints[i].x<<" "<<slam_data->mapPoints[i].y<<" "<< slam_data->mapPoints[i].z;
          }

          // glm::mat4 proj_mat, view_mat;
          auto proj_mat = getGLProjMatFromCV(slam_data->camera_intrinsic, img_width, img_height, 1000.0f, 0.01f);
          auto view_mat = getGLModelViewMatrixFromCV(slam_data->camera_pose_mat.colRange(0, 3).rowRange(0, 3), tVec);
          // proj_mat = glm::transpose(proj_mat);
          glm::mat4 mvp_gl = proj_mat * view_mat;

        
        
          // cv:: Mat mvp = proj_mat * view_mat;
          // glm::mat4 mvp_gl;
          // fromCV2GLM(mvp, &mvp_gl);
          // cv::Mat p = cv::Mat::ones(1, 4, CV_32FC1);

          // for(int j=0;j<slam_data->mp_num;j++){
          //   // p.at<float>(0,0) = slam_data->mapPoints[j].x;
          //   // p.at<float>(0,1) = slam_data->mapPoints[j].y;
          //   // p.at<float>(0,2) = slam_data->mapPoints[j].z;
          //   cv::Vec4f p(slam_data->mapPoints[j].x,slam_data->mapPoints[j].y,slam_data->mapPoints[j].z,1.0f);
          //   cv::Mat pp = mvp * cv::Mat(p);
          //   // cv::Mat p = mvp * cv::Mat(slam_data->mapPoints[j]);
          //   LOG(INFO)<<"p: "<< pp;
          //     map_point[4*j] = pp.at<float>(0, 0) / pp.at<float>(0, 3) ; 
          //     map_point[4*j+1] = pp.at<float>(0, 1)/ pp.at<float>(0, 3);
          //     map_point[4*j+2] = .0f;map_point[4*j+3] = 1.0f;

          // }

          MP_RETURN_IF_ERROR(prenderer->GlRender(mvp_gl, map_point, slam_data->mp_num)); */
          

          std::vector<cv::Point2f> projectedPoints;
          cv::projectPoints(
            std::vector<cv::Point3f>(slam_data->mapPoints,slam_data->mapPoints+sizeof(cv::Point3f)*slam_data->mp_num), 
            rVec, tVec, 
            slam_data->camera_intrinsic, 
            slam_data->camera_mDistCoef, 
            projectedPoints);
          
          int i = 0;
          for(int j=0; j<projectedPoints.size(); j++){
              if(i > MAX_TRACK_POINT) break;
              cv::Point2f r1 = projectedPoints[j];
              if(r1.x > img_width || r1.x<.0f || r1.y>img_height||r1.y<.0) continue;
              map_point[4*i] = (r1.x / img_width) * 2.0f - 1.0f; 
              map_point[4*i+1] = r1.y / img_height* 2.0f - 1.0f;
              map_point[4*i+2] = .0f;map_point[4*i+3] = 1.0f;
              i++;
          }
          
          MP_RETURN_IF_ERROR(prenderer->GlRender(map_point, i));
        }else{
          MP_RETURN_IF_ERROR(prenderer->GlRender(&slam_data->keyPoints[0], slam_data->kp_num));  
        }

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(src1.target(), 0);

      // Execute GL commands, before getting result.
      glFlush();

      auto output = dst.GetFrame<GpuBuffer>();
      cc->Outputs().Tag("IMAGE_GPU").Add(output.release(), cc->InputTimestamp());
      return ::mediapipe::OkStatus();
    }
 public:
  static ::mediapipe::Status GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag(kInputVideoTag).Set<GpuBuffer>();
    cc->Outputs().Tag(kOutputVideoTag).Set<GpuBuffer>();
    if (cc->Inputs().HasTag(kInputSLAMTag)){
      cc->Inputs().Tag(kInputSLAMTag).Set<SLAMData*>();
    }
    MP_RETURN_IF_ERROR(GlCalculatorHelper::UpdateContract(cc));
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Open(CalculatorContext* cc) override {
    cc->SetOffset(TimestampDiff(0));
    MP_RETURN_IF_ERROR(gpu_helper.Open(cc));
    //debug
    // if(cc->Inputs().HasTag(kInputKeyPoints)) b_render_point = true;

    point_num = 5;
    float tmp_point_cloud[20] = {
      -0.8f, -0.8f, .0f, 1.0f,
      -0.8f, 0.8f, .0f, 1.0f,
      0.8f, -0.8f, .0f, 1.0f,
      0.8f, 0.8f, .0f, 1.0f,
      .0f, .0f, .0f, 1.0f,
    };
    point_cloud = new float[20];
    memcpy(point_cloud, tmp_point_cloud, 20*sizeof(float));
    return ::mediapipe::OkStatus();
  }

  ::mediapipe::Status Process(CalculatorContext* cc) override {
    auto frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
    if(img_width == 0){
        img_width = (float)frame.width();
        img_height = (float)frame.height();
        float fov = 2*atan(img_height * 0.5 / 458.0f);//glm::radians(45.0f);
		   projMat = glm::perspective(fov, img_width/img_height, 0.1f, 100.0f);

      }

    return gpu_helper.RunInGlContext(
        [this, cc]() -> ::mediapipe::Status { 
          // LOG(INFO) << "=============";
          return RenderGpu(cc); 
          });
  }
};
REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
