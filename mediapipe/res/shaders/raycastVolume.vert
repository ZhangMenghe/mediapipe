#version 300 es
precision mediump float;

layout (location = 0) in vec4 aPosition;
uniform mat4 uModelMat, uViewProjMat;
// layout (location = 1) in vec3 aTexcoord;

// out vec2 vTexcoord;

void main(){
    // vTexcoord = aTexcoord.xy;
    gl_Position = uViewProjMat* uModelMat* aPosition;
}