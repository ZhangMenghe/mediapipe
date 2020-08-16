#version 300 es
precision mediump float;

out vec4 fragColor;
in vec2 vTexcoord;

uniform sampler2D uSampler;

void main(){
    fragColor = texture(uSampler, vTexcoord);
}
