#version 300 es

precision mediump float;
uniform int uScheme;
uniform int u_widget_num;
uniform int u_visible_bits;
uniform vec2 u_opacity[60];
uniform float u_contrast_low;
uniform float u_contrast_high;

out vec4 fragColor;
in vec2 vTexcoord;
// All components are in the range [0…1], including hue.
vec3 hsv2rgb(vec3 c){
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
vec3 transfer_scheme_hsv(float gray){
    if(uScheme == 1) return vec3(gray, 1.0, 1.0);
    return vec3((1.0 -gray) * 180.0 / 255.0, 1.0, 1.0);
}
float get_intensity(int uid_offset, float posx){
    vec2 lb = u_opacity[uid_offset], rb = u_opacity[uid_offset+3];
    if(posx < lb.x || posx > rb.x) return .0;

    vec2 lm = u_opacity[uid_offset+1], lt =u_opacity[uid_offset+2];
    vec2 rm = u_opacity[uid_offset+4], rt =u_opacity[uid_offset+5];
    float k = (lt.y - lm.y)/(lt.x - lm.x);
    if(posx < lt.x) return k*(posx - lm.x)+lm.y;
    if(posx < rt.x) return rt.y;
    return -k *(posx - rm.x)+rm.y;
}
vec4 get_mixture(float posx, vec3 gray){
    if(uScheme == 0) return vec4(gray.r);
    vec3 color_hsv = transfer_scheme_hsv(posx);
    return vec4(hsv2rgb(color_hsv),gray.r);
}
void main(){
    float intensity = vTexcoord.x;
//



    if(vTexcoord.y >0.66){
        intensity = smoothstep(u_contrast_low, u_contrast_high, intensity);
        intensity= max(.0, min(1.0, intensity));

        if(uScheme == 0) fragColor = vec4(vec3(intensity), 1.0);
        else fragColor = vec4(hsv2rgb(transfer_scheme_hsv((vTexcoord.x > u_contrast_high)? 1.0:intensity)), 1.0);
    }else{
//        if(intensity > u_contrast_high||intensity < u_contrast_low) intensity = .0;
//        intensity = smoothstep(u_contrast_low, u_contrast_high, intensity);

        float gray = .0;
        for(int i=0; i<u_widget_num; i++){
            if(((u_visible_bits >> i) & 1) == 1) gray = max(gray, get_intensity(6*i, intensity));
        }
        if(vTexcoord.y<0.33) fragColor = vec4(vec3(gray), 1.0);
        else{
            intensity = smoothstep(u_contrast_low, u_contrast_high, intensity);
            intensity= max(.0, min(1.0, intensity));

            fragColor = get_mixture(intensity, vec3(gray));
        }
    }
}