#ifndef MEDIAPIPE_GPU_GL_PIPELINE_MESH_H_
#define MEDIAPIPE_GPU_GL_PIPELINE_MESH_H_

#include "mediapipe/framework/port/status.h"
#include "mediapipe/gpu/gl_base.h"
namespace mediapipe{
class Mesh {
public:
	Mesh();
	~Mesh();

	void BindVAO() const;
	void BindVBO() const;
	void BindIBO() const;
	void unBind() const;

	inline void ElementCount(unsigned int x) {
		mElementCount = x;
	}
	inline unsigned int ElementCount() const {
		return mElementCount;
	}
	static void InitQuadWithTex(GLuint &vao, const float* vertices,
			int vertex_num, const unsigned int* indices, int indice_num);
	static void InitQuad(GLuint &vao, const float* vertices, int vertex_num,
			const unsigned int* indices, int indice_num);
	static void InitQuad(GLuint &vao, GLuint &vbo, const unsigned int* indices, int indice_num);
private:
	unsigned int mElementCount;
	GLuint mVAO;
	GLuint mVBO;
	GLuint mIBO;
};
}
#endif