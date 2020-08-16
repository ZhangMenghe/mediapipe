#version 300 es

precision mediump float;

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aZposTex;//first is z pos, second is z texture pos

uniform mat4 uMVP;
out vec3 vTexcoord;

void main(){
    vTexcoord = vec3(aPosition.xy + 0.5, aZposTex.y);
    gl_Position = uMVP * vec4(aPosition, aZposTex.x, 1.0);
}
