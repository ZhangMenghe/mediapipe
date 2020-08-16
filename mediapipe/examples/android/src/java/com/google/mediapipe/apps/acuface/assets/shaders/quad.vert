#version 300 es
precision mediump float;
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aTexcoord;

out vec2 vTexcoord;

void main(){
    vTexcoord = aTexcoord.xy;
    gl_Position = vec4(aPosition.xy, .0, 1.0);
}