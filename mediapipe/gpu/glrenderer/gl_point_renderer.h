#ifndef MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_
#define MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_

#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_base.h"
#include <glm/glm.hpp> 
#include <glm/gtc/type_ptr.hpp>

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
  ::mediapipe::Status GlRender(glm::mat4 mvp, float* pointCloudData, int num);

  void setColor(float r, float g, float b){
    point_color[0] = r;    point_color[1] = g;    point_color[2] = b; 
    glUseProgram(program_);
    glUniform3fv(u_point_color, 1, point_color);
    glUseProgram(0);
  }
  void setPointSize(float sz){
    point_size = sz;
    glUseProgram(program_);
    glUniform1f(u_point_size, point_size);
    glUseProgram(0);
  }

  // Deletes the rendering program. Must be called withn the GL context where it was created.
  void GlTeardown();
  

 private:
  GLuint program_ = 0;
  GLuint vao_, vbo_;

  const int MAX_POINTS = 50000;
  float point_size = 10.0f;
  GLfloat point_color[3] = {.0f, 0.5f, 1.0f};
  glm::mat4 mvp_ = glm::mat4(1.0f);
  
  GLuint u_point_color, u_point_size, u_mvp;
};

}  // namespace mediapipe

#endif  // MEDIAPIPE_GPU_GLRENDERER_GL_POINT_RENDERER_H_
