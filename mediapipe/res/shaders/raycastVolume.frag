#version 300 es
precision mediump float;

out vec4 gl_FragColor;
// in vec2 vTexcoord;

// uniform sampler2D uSampler;
uniform vec4 uColor;
void main(){
    gl_FragColor = uColor;
    // gl_FragColor = texture(uSampler, vTexcoord);
}