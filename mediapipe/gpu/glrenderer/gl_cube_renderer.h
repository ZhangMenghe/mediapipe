#ifndef MEDIAPIPE_GPU_GLRENDERER_GL_CUBE_RENDERER_H_
#define MEDIAPIPE_GPU_GLRENDERER_GL_CUBE_RENDERER_H_

#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_base.h"
#include "mediapipe/gpu/glPipeline/Shader.h"
#include <glm/glm.hpp> 
#include <glm/gtc/type_ptr.hpp>

namespace mediapipe {

class CubeRenderer {
 public:
  CubeRenderer() {}
  
  ::mediapipe::Status GlSetup();
  // Creates the rendering program with customized fragment shader 
  ::mediapipe::Status GlSetup(const std::string vertex_shader, const std::string frag_shader, const std::string geo_shader);

  ::mediapipe::Status GlRender(glm::mat4 mvp);
  ::mediapipe::Status GlRender(glm::mat4 mvp, glm::mat4 model_mat, glm::vec4 color);

  // Deletes the rendering program. Must be called withn the GL context where it was created.
  void GlTeardown();

 private:
  Shader* shader_= nullptr;
  GLuint program_ = 0;
  GLuint vao_;
  float point_size = 5.0f;
  GLfloat color[3] = {1.0f, .0f, .0f};
  glm::mat4 mvp_ = glm::mat4(1.0f);
  GLuint u_mvp;
};

}  // namespace mediapipe

#endif  // MEDIAPIPE_GPU_GLRENDERER_GL_CUBE_RENDERER_H_
