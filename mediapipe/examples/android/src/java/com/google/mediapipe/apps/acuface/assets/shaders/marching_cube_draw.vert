#version 310 es
#extension GL_EXT_shader_io_blocks:require

precision mediump float;
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
out VS_OUT
{
	vec3 position;
	vec3 normal; // Draw with flat shading
} vs_out;
uniform mat4 uMVP;

void main(){
    vec3 pos = vec3(aPos.x, -aPos.y, aPos.z);
    gl_Position = uMVP * vec4(pos,1.0);
    
    vs_out.position = pos;
    vs_out.normal = normalize(aNormal.xyz);
}