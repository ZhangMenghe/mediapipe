#version 300 es

precision mediump float;

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aTexcoord;

uniform vec2 uOffset;
uniform vec2 uScale;
out vec2 vTexcoord;

void main(){
    vTexcoord = aTexcoord.xy;
    gl_Position = vec4(aPosition.xy * uScale + uOffset, .0, 1.0);
}