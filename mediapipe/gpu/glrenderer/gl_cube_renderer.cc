#include "mediapipe/gpu/glrenderer/gl_cube_renderer.h"

#include "mediapipe/framework/port/ret_check.h"
#include "mediapipe/gpu/gl_simple_shaders.h"
#include "mediapipe/gpu/shader_util.h"
#include "mediapipe/gpu/glPipeline/Mesh.h"
#include "mediapipe/gpu/glPipeline/Primitive.h"


namespace mediapipe {
namespace{
  enum { ATTRIB_VERTEX, NUM_ATTRIBUTES };
}

::mediapipe::Status CubeRenderer::GlSetup() {
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

  Mesh::InitQuad(vao_, cuboid, 8, cuboid_indices, 36);

  glUseProgram(program_);
  auto u_color = glGetUniformLocation(program_, "color");
  glUniform3fv(u_color, 1, color);

  auto u_point_size = glGetUniformLocation(program_, "point_size");
  glUniform1f(u_point_size, point_size);

  u_mvp = glGetUniformLocation(program_, "mvp");
  glUniformMatrix4fv(u_mvp, 1, GL_FALSE, &mvp_[0][0]);
  glUseProgram(0);
  return ::mediapipe::OkStatus();
}
Status CubeRenderer::GlSetup(
  const std::string vertex_shader, const std::string frag_shader, const std::string geo_shader){
    Mesh::InitQuad(vao_, cuboid, 8, cuboid_indices, 36);
    shader_ = new Shader();
    if(!vertex_shader.empty()) shader_->AddShader(GL_VERTEX_SHADER, vertex_shader);
    if(!frag_shader.empty()) shader_->AddShader(GL_FRAGMENT_SHADER, frag_shader);
    if(!geo_shader.empty()) shader_->AddShader(GL_GEOMETRY_SHADER, geo_shader);
    RET_CHECK(shader_->CompileAndLink());
    
    return ::mediapipe::OkStatus();
  }


void CubeRenderer::GlTeardown() {
  if (program_) {
    glDeleteProgram(program_);
    program_ = 0;
  }
}

::mediapipe::Status CubeRenderer::GlRender(glm::mat4 mvp) {
  glUseProgram(program_);
  glUniformMatrix4fv(u_mvp, 1, GL_FALSE, &mvp[0][0]);
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
        
  glUseProgram(0);
  return ::mediapipe::OkStatus();
}
::mediapipe::Status CubeRenderer::GlRender(glm::mat4 mvp, glm::mat4 model_mat, glm::vec4 color){
  //  glEnable(GL_BLEND);
  //   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //   glEnable(GL_DEPTH_TEST);

  GLuint sp = shader_->Use();
    Shader::Uniform(sp, "uViewProjMat", mvp);
    Shader::Uniform(sp, "uModelMat", model_mat);
    Shader::Uniform(sp, "uColor", color);

    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  shader_->UnUse();

    // glDisable(GL_BLEND);
    // glDisable(GL_DEPTH_TEST);
  return ::mediapipe::OkStatus();
}


}  // namespace mediapipe
