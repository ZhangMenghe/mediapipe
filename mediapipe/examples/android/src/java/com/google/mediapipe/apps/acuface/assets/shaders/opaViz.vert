#version 300 es
precision mediump float;
layout (location = 0) in vec2 aPosition;
uniform vec2 uOffset;
uniform vec2 uScale;
void main(){
    gl_Position = vec4(vec2(aPosition.x-0.5, aPosition.y) *uScale + uOffset , .0, 1.0);
}