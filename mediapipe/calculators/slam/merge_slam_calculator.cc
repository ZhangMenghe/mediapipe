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

#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/gpu/gl_shader_helper.pb.h"
#include "mediapipe/util/resource_util.h"

#include <fstream>

namespace mediapipe {
namespace {
    constexpr char kInputVideoTag[] = "IMAGE_GPU";
    constexpr char kOutputVideoTag[] = "IMAGE_GPU";
    constexpr char kInputSLAMTag[] = "SLAM_OUT";
    constexpr char kRaycastShaderName[] = "DICOM_RAYCAST";
    int64 timestamp=0;
    void fromCV2GLM(const cv::Mat& cvmat, glm::mat4* glmmat) {
      if (cvmat.cols != 4 || cvmat.rows != 4 || cvmat.type() != CV_32FC1) {
          LOG(INFO)<<cvmat.cols<<" "<<cvmat.rows;
          return;
      }
      memcpy(glm::value_ptr(*glmmat), cvmat.data, 16 * sizeof(float));
      *glmmat = glm::transpose(*glmmat);
    }

    glm::mat4 getGLProjMatFromCV(cv::Mat mat, float width, float height, float zfar, float znear){
      float fx = mat.at<float>(0,0);  float fy = mat.at<float>(1,1);
      float cx = mat.at<float>(0,2);  float cy = mat.at<float>(1,2);
      glm::mat4 m;
      m[0][0] = 2.0 * fx / width;        m[0][1] = 0.0;                      m[0][2] = 0.0;                                  m[0][3] = 0.0;
      m[1][0] = 0.0;                     m[1][1] = -2.0 * fy / height;       m[1][2] = 0.0;                                  m[1][3] = 0.0;
      // m[1][0] = 0.0;                     m[1][1] = 2.0 * fy / height;        m[1][2] = 0.0;                                  m[1][3] = 0.0;
      m[2][0] = 1.0 - 2.0 * cx / width;  m[2][1] = 2.0 * cy / height - 1.0;  m[2][2] = (zfar + znear) / (znear - zfar);      m[2][3] = -1.0;
      m[3][0] = 0.0;                     m[3][1] = 0.0;                      m[3][2] = 2.0 * zfar * znear / (znear - zfar);  m[3][3] = 0.0;
      return m;
    }
    glm::mat4 getGLModelViewMatrixFromCV(cv::Mat cam_pose){
        cv::Mat rotMat = cam_pose.colRange(0, 3).rowRange(0, 3);
        cv::Mat transVec = cam_pose.col(3).rowRange(0, 3);

        // LOG(INFO)<<"CAM POS "<<transVec.at<float>(0, 0)<<" "<<transVec.at<float>(1, 0)<< " "<<transVec.at<float>(2, 0);
        cv::Mat Rt = cv::Mat::zeros(4, 4, CV_32FC1);
        for (int y = 0; y < 3; y++) {
          for (int x = 0; x < 3; x++) {
              Rt.at<float>(y, x) = rotMat.at<float>(y, x);
          }
          Rt.at<float>(y, 3) = transVec.at<float>(y, 0);
        }
        Rt.at<float>(3, 3) = 1.0;

        //OpenGL has reversed Y & Z coords
        cv::Mat reverseYZ = cv::Mat::eye(4, 4, CV_32FC1);
        reverseYZ.at<float>(1, 1) = -1;
        reverseYZ.at<float>(2, 2) = -1;
            
        cv::Mat mvMat = reverseYZ * Rt;
        glm::mat4 res;
        fromCV2GLM(mvMat, &res);
        return res;
    }

    void save_point_cloud(cvPoints points, std::vector<cv::Point2f> projectedPoints, cv::Mat transVec,
                          float img_width, float img_height){
      LOG(INFO)<<timestamp<<"\n";
      std::ofstream file;
      if(points.num <= 0) return;      
      file.open("points/"+std::to_string(timestamp) + ".txt", std::ofstream::out | std::ofstream::app);
      
      for(int idx = 0; idx<projectedPoints.size(); idx++){
        cv::Point2f r1 = projectedPoints[idx];
        if(r1.x > img_width || r1.x<.0f || r1.y>img_height||r1.y<.0) continue;

        auto point = points.data[idx];
        file<<r1.y<<" "<<r1.x<<" "<<point.x - transVec.at<float>(0,0)<<" "<<point.y - transVec.at<float>(1,0)<<" "<<point.z- transVec.at<float>(2,0)<<"\n";
      }
      
      projectedPoints.clear();
      file.close();
    }
    void save_point_cloud_gl(cvPoints points, glm::mat4 mvp, cv::Mat transVec){
      LOG(INFO)<<timestamp<<"\n";
      std::ofstream file;
      if(points.num <= 0) return;      
      file.open("mappoints/0221/"+std::to_string(timestamp) + ".txt", std::ofstream::out | std::ofstream::app);
      
      for(auto point:points.data){
        glm::vec4 r = glm::vec4(point.x, point.y, point.z, 1.0f);
        glm::vec4 proj = mvp * r;
        // LOG(INFO)<<proj.x << " "<<proj.y <<" "<< proj.z<<" "<<proj.w;
        proj = proj / proj.w;
        if(proj.x < -1.0 || proj.x>1.0 || proj.y < -1.0 || proj.y > 1.0) continue;
        file<<(1.0 - (proj.y * 0.5 +0.5)) * 480<<" "<<(proj.x * 0.5 +0.5) * 640<<" "<<point.x - transVec.at<float>(0,0)<<" "<<point.y - transVec.at<float>(1,0)<<" "<<point.z- transVec.at<float>(2,0)<<"\n";
      }
      file.close();

      /// save camera status
      file.open("camera/0221/"+std::to_string(timestamp) + ".txt", std::ofstream::out | std::ofstream::app);
      for(int i=0; i<4; i++){
        file<<mvp[i][0]<<" "<<mvp[i][1]<<" "<<mvp[i][2]<<" "<<mvp[i][3]<<"\n";
      }
      file<< transVec.at<float>(0,0)<<" "<<transVec.at<float>(1,0)<<" "<<transVec.at<float>(2,0)<<"\n";
      file.close();

    }

}
class MergeSLAMCalculator : public CalculatorBase {
private:
    GlCalculatorHelper gpu_helper;
    std::unique_ptr<QuadRenderer> quad_renderer_;
    std::unique_ptr<PointRenderer> point_renderer_;
    std::unique_ptr<PointRenderer> ppoint_renderer_;

    std::unique_ptr<CubeRenderer> cube_renderer_;
    std::unique_ptr<CubeRenderer> plane_renderer_;

    float* point_cloud;
    int point_num;
    bool b_render_point = true;
    float img_width = .0f, img_height=.0f;
    glm::mat4 projMat;
    bool old_one = true;

    float map_point[4* MAX_TRACK_POINT];
    float plane_point[4* MAX_TRACK_POINT];

		::mediapipe::Status LoadOptions(CalculatorContext* cc);
    
    //Rendering functions
    Status draw_plane(planeData pd, glm::mat4 vp);
    Status draw_keypoints(PointRenderer* kprenderer, float* data, int num);
    Status draw_mappoints(cvPoints mapPoints, CameraData camera, glm::mat4 vp,bool b_cvproject = false);
    Status draw_objects(glm::mat4 vp);
    Status RenderGPU(CalculatorContext* cc);
    
 public:
		static ::mediapipe::Status GetContract(CalculatorContract* cc);
		::mediapipe::Status Open(CalculatorContext* cc) override;
		::mediapipe::Status Process(CalculatorContext* cc) override;
		::mediapipe::Status Close(CalculatorContext* cc) override;
};

Status MergeSLAMCalculator::draw_plane(planeData pd, glm::mat4 vp){
  if(!pd.valid) return ::mediapipe::OkStatus();

  //plane points
  if(!ppoint_renderer_){
      ppoint_renderer_ = absl::make_unique<PointRenderer>();
      MP_RETURN_IF_ERROR(ppoint_renderer_->GlSetup());
      ppoint_renderer_->setColor(.0,.0,1.0);
      ppoint_renderer_->setPointSize(8.0f);
  }
  PointRenderer* pprenderer = ppoint_renderer_.get();
  
  CubeRenderer* plane_r = plane_renderer_.get();

  glm::vec3 pp = glm::vec3(pd.center.at<float>(0,0),pd.center.at<float>(1,0),pd.center.at<float>(2,0));
  glm::mat4 pm;
  fromCV2GLM(pd.pose, &pm);
  // auto pm_test = getGLModelViewMatrixFromCV(pd.pose);

    plane_r->GlRender(
    vp, 
    glm::translate(glm::mat4(1.0f), pp)* glm::scale(glm::mat4(1.0f), glm::vec3(1.f, 0.1f, 1.f)),
    glm::vec4(.0f,1.0f,.0f,0.1f));
    
    //draw plane points
    // MP_RETURN_IF_ERROR(pprenderer->GlRender(pproj_points, slam_data->mp_num));


  for(int i=0;i<pd.points.num;i++){
    plane_point[4*i] = pd.points.data[i].x;
    plane_point[4*i+1] = pd.points.data[i].y;
    plane_point[4*i+2] = pd.points.data[i].z;
    plane_point[4*i+3] = 1.0f;
  }

  MP_RETURN_IF_ERROR(pprenderer->GlRender(vp, plane_point, pd.points.num)); 

            
  /*if(slam_data->plane_detected){
    cv::Mat Plane2Camera = slam_data->camera_pose_mat*slam_data->plane_pose;
    std::vector<cv::Point3f> drawPoints(8);
      drawPoints[0] = cv::Point3f(0,0,0);
      drawPoints[1] = cv::Point3f(0.3,0.0,0.0);
      drawPoints[2] = cv::Point3f(0.0,0,0.3);
      drawPoints[3] = cv::Point3f(0.0,0.3,0);
      drawPoints[4] =cv::Point3f(0,0.3,0.3);
      drawPoints[5] =cv::Point3f(0.3,0.3,0.3);
      drawPoints[6] =cv::Point3f(0.3,0,0.3);
      drawPoints[7] =cv::Point3f(0.3,0.3,0);
      cv::Mat Rcp ,Tcp;
      cv::Rodrigues(Plane2Camera.rowRange(0,3).colRange(0,3),Rcp);
      Tcp = Plane2Camera.col(3).rowRange(0,3);
      std::vector<cv::Point2f> projectedPoints;
      cv::projectPoints(drawPoints, Rcp, Tcp,slam_data->camera_intrinsic, slam_data->camera_mDistCoef,projectedPoints);

  }*/
  return ::mediapipe::OkStatus();
}

Status MergeSLAMCalculator::draw_keypoints(PointRenderer* kprenderer, float* data, int num){
  MP_RETURN_IF_ERROR(kprenderer->GlRender(data, num));
  return ::mediapipe::OkStatus();
}

Status MergeSLAMCalculator::draw_mappoints(cvPoints mapPoints, CameraData camera, glm::mat4 vp, bool b_cvproject){
  if(mapPoints.num == 0) return ::mediapipe::OkStatus(); 
  cv::Mat rVec;
  cv::Rodrigues(camera.pose.colRange(0, 3).rowRange(0, 3), rVec);
  //opencv projection
  if(b_cvproject){
    std::vector<cv::Point2f> projectedPoints;
      cv::projectPoints(std::vector<cv::Point3f>(
        mapPoints.data,mapPoints.data+sizeof(cv::Point3f)*mapPoints.num),
        rVec, camera.pose.col(3).rowRange(0, 3), 
        camera.intrinsic, camera.DistCoef, 
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

    // save_point_cloud(mapPoints, projectedPoints, camera.pose.col(3).rowRange(0, 3), img_width, img_height);

    MP_RETURN_IF_ERROR(point_renderer_->GlRender(map_point, i));
  }else{
    for(int i=0;i<mapPoints.num;i++){
      map_point[4*i] = mapPoints.data[i].x;
      map_point[4*i+1] = mapPoints.data[i].y;
      map_point[4*i+2] = mapPoints.data[i].z;
      map_point[4*i+3] = 1.0f;
    }
    // save_point_cloud_gl(mapPoints, vp, camera.pose.col(3).rowRange(0, 3));
    MP_RETURN_IF_ERROR(point_renderer_->GlRender(vp, map_point, mapPoints.num));
  }
    return ::mediapipe::OkStatus();
}

Status MergeSLAMCalculator::draw_objects(glm::mat4 vp){
	CubeRenderer* cube_r = cube_renderer_.get();
	cube_r->GlRender(vp, 
			glm::translate(glm::mat4(1.0f), glm::vec3(.0,.0,1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)),
			glm::vec4(.0, 0.8, 0.8, 0.5));

}
Status MergeSLAMCalculator::RenderGPU(CalculatorContext* cc){
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
      auto slam_data =  cc->Inputs().Tag(kInputSLAMTag).Get<SLAMData*>();
      auto cam = slam_data->camera;

      if(cam.valid){
        std::cout<<"CAMERA VALID"<<std::endl;
        LOG(INFO)<<"CAMERA VALID";
        cv::Mat rVec;
        cv::Rodrigues(cam.pose.colRange(0, 3).rowRange(0, 3), rVec);
        cv::Mat tVec = cam.pose.col(3).rowRange(0, 3);

        auto proj_mat = getGLProjMatFromCV(cam.intrinsic, img_width, img_height, 500.0f, 0.1f);
        auto view_mat = getGLModelViewMatrixFromCV(cam.pose);
        glm::mat4 mvp_gl = proj_mat * view_mat;
        MP_RETURN_IF_ERROR(draw_mappoints(slam_data->refPoints, slam_data->camera, mvp_gl));
        
        // MP_RETURN_IF_ERROR(draw_plane(slam_data->plane, mvp_gl));
        // MP_RETURN_IF_ERROR(draw_objects(mvp_gl));
      }
      // else{
      //   std::cout<<"CAMERA in VALID"<<std::endl;

      //   LOG(INFO)<<"CAMERA IN VALID";
        draw_keypoints(point_renderer_.get(), slam_data->keyPoints, slam_data->kp_num);
      // }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(src1.target(), 0);

    // Execute GL commands, before getting result.
    glFlush();

    auto output = dst.GetFrame<GpuBuffer>();
    cc->Outputs().Tag("IMAGE_GPU").Add(output.release(), cc->InputTimestamp());
    return ::mediapipe::OkStatus();
}

Status MergeSLAMCalculator::GetContract(CalculatorContract* cc) {
    cc->Inputs().Tag(kInputVideoTag).Set<GpuBuffer>();
    cc->Outputs().Tag(kOutputVideoTag).Set<GpuBuffer>();
    if (cc->Inputs().HasTag(kInputSLAMTag)){
      cc->Inputs().Tag(kInputSLAMTag).Set<SLAMData*>();
    }
    MP_RETURN_IF_ERROR(GlCalculatorHelper::UpdateContract(cc));
    return ::mediapipe::OkStatus();
}

Status MergeSLAMCalculator::Open(CalculatorContext* cc){
  cc->SetOffset(TimestampDiff(0));
  LoadOptions(cc);
  MP_RETURN_IF_ERROR(gpu_helper.Open(cc));
  // cv::namedWindow("MapDrawer", /*flags=WINDOW_AUTOSIZE*/ 1);

  return ::mediapipe::OkStatus();
}

::mediapipe::Status MergeSLAMCalculator::Process(CalculatorContext* cc){
  auto frame = cc->Inputs().Tag(kInputVideoTag).Get<GpuBuffer>();
  timestamp = cc->InputTimestamp().Value();

  if(img_width == 0){
      img_width = (float)frame.width();
      img_height = (float)frame.height();
      float fov = 2*atan(img_height * 0.5 / 458.0f);//glm::radians(45.0f);
      projMat = glm::perspective(fov, img_width/img_height, 0.1f, 100.0f);
    }

  return gpu_helper.RunInGlContext(
      [this, cc]() -> ::mediapipe::Status { 
        return RenderGPU(cc); 
        });
}
::mediapipe::Status MergeSLAMCalculator::Close(CalculatorContext* cc) {
	return ::mediapipe::OkStatus();
}
Status MergeSLAMCalculator::LoadOptions(
    CalculatorContext* cc) {
  // Get calculator options specified in the graph.
	const auto& options = cc->Options<::mediapipe::glShaderHelperOptions>();
  if(options.shader_files_size()){
    for(auto file : options.shader_files()){
      if(file.shader_name() == kRaycastShaderName){
        std::string vs_txt, fs_txt, geo_txt;
        MP_RETURN_IF_ERROR(mediapipe::GetResourceContents(file.vs_path(), &vs_txt));
		    MP_RETURN_IF_ERROR(mediapipe::GetResourceContents(file.frag_path(), &fs_txt));
        // LOG(INFO)<<"VS: "<<vs_txt;
        // LOG(INFO)<<"Shader readed: "<<file.shader_name();
        
        cube_renderer_ = absl::make_unique<CubeRenderer>();
        MP_RETURN_IF_ERROR(cube_renderer_->GlSetup(vs_txt, fs_txt, geo_txt));
        
        plane_renderer_ = absl::make_unique<CubeRenderer>();
        MP_RETURN_IF_ERROR(plane_renderer_->GlSetup(vs_txt, fs_txt, geo_txt));

        point_renderer_ = absl::make_unique<PointRenderer>();
        MP_RETURN_IF_ERROR(point_renderer_->GlSetup());
      old_one = false;
      }
    }
  }
	return ::mediapipe::OkStatus();
}

REGISTER_CALCULATOR(MergeSLAMCalculator);

}  // namespace mediapipe
