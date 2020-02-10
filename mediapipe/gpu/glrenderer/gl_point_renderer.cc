#include "mediapipe/gpu/glrenderer/gl_point_renderer.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/gpu/gl_simple_shaders.h"
#include "mediapipe/gpu/shader_util.h"

namespace mediapipe {
namespace{
  enum { ATTRIB_VERTEX, NUM_ATTRIBUTES };
}

::mediapipe::Status PointRenderer::GlSetup() {
  // return GlSetup(kNoTextureVertexShader,kFlatColorFragmentShader,{"point_size"}, {"color"});
    const GLint attr_location[NUM_ATTRIBUTES] = {
      ATTRIB_VERTEX
  };
  const GLchar* attr_name[NUM_ATTRIBUTES] = {
      "position"
  };
  GlhCreateProgram(kNoTextureVertexShader, kFlatColorFragmentShader, NUM_ATTRIBUTES,
                   &attr_name[0], attr_location, &program_);
  RET_CHECK(program_) << "Problem initializing the program.";

  //Generate VAO and bind
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  //Generate VBO and bind
  glGenBuffers(1, &vbo_);

  //dynamic feed data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * MAX_POINTS * 4, nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(ATTRIB_VERTEX);
  glVertexAttribPointer(ATTRIB_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //uniform
  u_point_size = glGetUniformLocation(program_, "point_size");
  u_point_color = glGetUniformLocation(program_, "color");
  u_mvp = glGetUniformLocation(program_, "mvp");
  RET_CHECK(u_point_size != -1 && u_point_size!=-1 && u_mvp!=-1)
          << "could not find uniform in point drawing" ;
  
  glUseProgram(program_);
  glUniform3fv(u_point_color, 1, point_color);
  glUniform1f(u_point_size, point_size);
  glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp_));
  glUseProgram(0);

  return ::mediapipe::OkStatus();
}

void PointRenderer::GlTeardown() {
  if (program_) {
    glDeleteProgram(program_);
    program_ = 0;
  }
}

::mediapipe::Status PointRenderer::GlRender(float* pointCloudData, int num) {
  num = std::min(MAX_POINTS, num);
  // update data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0,  num* 4 * sizeof(float), pointCloudData);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Draw.
  glUseProgram(program_);  
  glBindVertexArray(vao_);
  glDrawArrays(GL_POINTS, 0, num);
  glBindVertexArray(0);
  glUseProgram(0);
  return ::mediapipe::OkStatus();
}
::mediapipe::Status PointRenderer::GlRender(glm::mat4 mvp, float* pointCloudData, int num) {
  mvp_ = mvp;
  glUseProgram(program_);
  glUniformMatrix4fv(u_mvp, 1, GL_FALSE, glm::value_ptr(mvp_));
  return GlRender(pointCloudData,num);
}


}  // namespace mediapipe
