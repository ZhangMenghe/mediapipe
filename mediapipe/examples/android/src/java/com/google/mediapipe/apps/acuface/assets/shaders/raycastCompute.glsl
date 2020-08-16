#version 310 es

#pragma multi_compile SHOW_ORGANS
#pragma multi_compile COLOR_GRAYSCALE COLOR_HSV COLOR_BRIGHT
#pragma multi_compile LIGHT_DIRECTIONAL LIGHT_SPOT LIGHT_POINT
#pragma multi_compile FLIPY

#extension GL_EXT_shader_io_blocks:require
#extension GL_EXT_geometry_shader:require

precision mediump float;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
layout(binding = 0, r32ui)readonly uniform mediump uimage3D srcTex;
layout(binding = 1, rgba8)writeonly uniform mediump image3D destTex;

//Uniforms for texture_baked
struct OpacityAdj{
    float overall;//0-1
    float lowbound; //slope adj, 0-1
    float cutoff;//0,1
};
//shaderd by tex and ray, but mutually exclusive
//uniform OpacityAdj uOpacitys;
uniform vec2 u_opacity[60];
uniform int u_widget_num;

//last bit indicates body(which doesn't belong to organs)
uniform uint u_maskbits;// = uint(31);
uniform uint u_organ_num;// = uint(4);
uniform vec3 u_tex_size;
uniform float u_contrast_low;
uniform float u_contrast_high;
uniform float u_brightness;
uniform int u_visible_bits;
uniform bool u_mask_color;
//uniform float u_contrast_level;
const vec3 ORGAN_COLORS[7]= vec3[7](vec3(0.24, 0.004, 0.64), vec3(0.008, 0.278, 0.99), vec3(0.75, 0.634, 0.996),
                            vec3(1, 0.87, 0.14),vec3(0.98, 0.88, 1.0),vec3(0.99, 0.106, 0.365), vec3(.0, 0.314,0.75));

// All components are in the range [0…1], including hue.
vec3 hsv2rgb(vec3 c){
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
vec3 transfer_scheme(float gray){
    return hsv2rgb(vec3(gray, 1.0, 1.0));
}
vec3 transfer_scheme(int cat, float gray){
//    return hsv2rgb(vec3(cat, 1.0, gray));
    vec3 gcolor = ORGAN_COLORS[cat - 1];
    return gcolor * gray;
}
//hot to color. H(0~180)
vec3 bright_scheme(float gray){
    return hsv2rgb(vec3((1.0 -gray) * 180.0 / 255.0, 1.0, 1.0));
}

float UpdateOpacityAlpha(int woffset, float alpha){
    vec2 lb = u_opacity[woffset], rb = u_opacity[woffset+3];
    if(alpha < lb.x || alpha > rb.x) return .0;
    vec2 lm = u_opacity[woffset+1], lt =u_opacity[woffset+2];
    vec2 rm = u_opacity[woffset+4], rt =u_opacity[woffset+5];
    float k = (lt.y - lm.y)/(lt.x - lm.x);
    if(alpha < lt.x) alpha*= k*(alpha - lm.x)+lm.y;
    else if(alpha < rt.x) alpha*=rt.y;
    else alpha*= -k *(alpha - rm.x)+rm.y;
    return alpha;
}
int getMaskBit(uint mask_value){
    //check body
    if(mask_value == uint(0)) return ((u_maskbits & uint(1)) == uint(1))? 0:-1;

    int CHECK_BIT = int(-1);
    //check if organ
    for(uint i=uint(0); i<u_organ_num; i++){
        if(((u_maskbits>> uint(i + uint(1))) & uint(1)) == uint(0)) continue;
        uint cbit = (mask_value>> i) & uint(1);
        if(cbit == uint(1)){
            CHECK_BIT = int(i) + 1;
            break;
        }
    }
    return CHECK_BIT;
}

uvec2 Sample(ivec3 pos){
    #ifdef FLIPY
        pos = ivec3(pos.x, uint(u_tex_size.y-float(pos.y)),pos.z);
    #endif
    uint value = imageLoad(srcTex, pos).r;
    //lower part as color, higher part as mask
    return uvec2(value&uint(0xffff), value>>uint(16));
}

//applied contrast, brightness, 12bit->8bit, return value 0-1
float TransferIntensityStepOne(uint intensity){
    //max value 4095
    float intensity_01 = float(intensity) * 0.0002442002442002442;

    if(intensity_01 > u_contrast_high||intensity_01 < u_contrast_low) intensity_01 = .0;

    intensity_01 = smoothstep(u_contrast_low, u_contrast_high, intensity_01);

//    intensity_01 = (intensity_01 - u_contrast_low) / (u_contrast_high - u_contrast_low) * u_contrast_level;
    intensity_01 = clamp(u_brightness+intensity_01 - 0.5, .0, 1.0);
    return intensity_01;
}

vec3 TransferColor(float intensity, int ORGAN_BIT){
    intensity = smoothstep(u_contrast_low, u_contrast_high, intensity);
    intensity= max(.0, min(1.0, intensity));

//    if(uScheme == 0) fragColor = vec4(vec3(intensity), 1.0);
//    else fragColor = vec4(hsv2rgb(transfer_scheme_hsv((vTexcoord.x > u_contrast_high)? 1.0:intensity)), 1.0);

    vec3 color = vec3(intensity);

    #ifdef COLOR_HSV
        color = transfer_scheme(intensity);
    #elif defined(COLOR_BRIGHT)
        color = bright_scheme(intensity);
    #endif

    #ifdef SHOW_ORGANS
        if(u_mask_color && ORGAN_BIT > int(0)) color = transfer_scheme(ORGAN_BIT, intensity);
    #endif
    return color;
}

void main(){
    ivec3 storePos = ivec3(gl_GlobalInvocationID.xyz);
    uvec2 sampled_value = Sample(storePos);
    int ORGAN_BIT = -1;
    #ifdef SHOW_ORGANS
        ORGAN_BIT = getMaskBit(sampled_value.y);
        if(ORGAN_BIT< 0) {imageStore(destTex, storePos, vec4(.0)); return;}
    #endif

    //intensity in 0-1
    float intensity = TransferIntensityStepOne(sampled_value.x);
    float alpha = .0;
    for(int i=0; i<u_widget_num; i++)
    if(((u_visible_bits >> i) & 1) == 1) alpha = max(alpha, UpdateOpacityAlpha(6*i, intensity));

    imageStore(destTex, storePos, vec4(TransferColor(intensity, ORGAN_BIT), alpha));
}