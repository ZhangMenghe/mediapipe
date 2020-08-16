#version 310 es

#extension GL_EXT_shader_io_blocks:require
precision mediump float;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoord;

out VS_OUT {
    vec3 raydir;
    vec3 screenPos;
    vec3 FragPos;
    vec3 TexCoords;
} vs_out;

uniform mat4 uModelMat, uVPMat;
uniform vec3 uCamposObjSpace;

void main(void){
    vs_out.raydir = aPos - uCamposObjSpace;
    vs_out.FragPos = vec3(uModelMat * vec4(aPos, 1.0f));
    vec4 screen_pos = uVPMat * vec4(vs_out.FragPos, 1.0);
    vs_out.screenPos = screen_pos.xyw;
    vs_out.TexCoords = aTexCoord;
    gl_Position = screen_pos;
}
