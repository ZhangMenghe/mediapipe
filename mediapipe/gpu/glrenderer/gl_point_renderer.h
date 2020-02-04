#ifndef MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_
#define MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_

#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_base.h"
#include "mediapipe/gpu/scale_mode.pb.h"

namespace mediapipe {

class PointRenderer {
 public:
  PointRenderer() {}
  
  ::mediapipe::Status GlSetup();
  // Creates the rendering program with customized fragment shader 
  ::mediapipe::Status GlSetup(
    const GLchar* custom_vertex_shader,
    const GLchar* custom_frag_shader,
    const std::vector<const GLchar*>& custom_vt_uniforms,
    const std::vector<const GLchar*>& custom_frame_uniforms);

  ::mediapipe::Status GlRender(float* pointCloudData, int num);
  // Deletes the rendering program. Must be called withn the GL context where it was created.
  void GlTeardown();

 private:
  GLuint program_ = 0;
  GLuint vao_, vbo_;

  const int MAX_POINTS = 600;
  float point_size = 5.0f;
  GLfloat point_color[3] = {1.0f, 0.5f, .0f};
  
  GLuint u_point_color, u_point_size;
};

}  // namespace mediapipe

#endif  // MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_
