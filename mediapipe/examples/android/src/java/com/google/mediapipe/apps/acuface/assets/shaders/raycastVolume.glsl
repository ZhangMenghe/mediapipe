#version 310 es

#pragma multi_compile CUTTING_PLANE

#extension GL_EXT_shader_io_blocks:require
#extension GL_EXT_geometry_shader:require

precision mediump float;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(binding = 0, rgba8)readonly uniform mediump image3D srcTex;
layout(binding = 1, rgba8)writeonly uniform mediump image2D destTex;

uniform vec2 u_con_size;
uniform float u_fov;
uniform mat4 u_WorldToModel;
uniform mat4 u_CamToWorld;

uniform vec4 u_plane_color;
uniform vec3 uCamposObjSpace;
uniform float usample_step_inverse;
struct Plane{
    vec3 p;
    vec3 normal;
    vec3 s1, s2, s3;
};
uniform Plane uPlane;
const float constantNCP = 1.0;
const float alpha_tolerance = 0.01;
vec3 VolumeSize;

vec2 RayCube(vec3 ro, vec3 rd, vec3 extents) {
    vec3 tMin = (-extents - ro) / rd;
    vec3 tMax = (extents - ro) / rd;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    return vec2(max(max(t1.x, t1.y), t1.z), min(min(t2.x, t2.y), t2.z));
}
float RayPlane(vec3 ro, vec3 rd, vec3 planep, vec3 planen) {
    float d = dot(planen, rd);
    float t = dot(planep - ro, planen);
    return d > 1e-5 ? (t / d) : (t > .0 ? 1e5 : -1e5);
}

bool intersectRayWithSquare(vec3 M, vec3 s1, vec3 s2, vec3 s3){
    vec3 dms1 = M-s1;
    vec3 ds21 = s2 - s1; vec3 ds31 = s3 - s1; //should be perpendicular to each other
    float u = dot(dms1, ds21);
    float v = dot(dms1, ds31);
    return (u >= 0.0 && u <= dot(ds21, ds21)
    && v >= 0.0 && v <= dot(ds31,ds31));
}

vec4 Sample(vec3 p){
    vec3 coord = clamp(p, vec3(.0), vec3(1.0));//vec3(usample_step_inverse), vec3(1.0-usample_step_inverse));
    return clamp(imageLoad(srcTex, ivec3(VolumeSize *coord)), vec4(.0), vec4(1.0));
}
vec4 subDivide(vec3 p, vec3 ro, vec3 rd, float t, float StepSize){
    float t0 = t - StepSize * 4.0;
    float t1 = t;
    float tm;

    #define BINARY_SUBDIV tm = (t0 + t1) * .5; p = ro + rd * tm; if (Sample(p).a > .01) t1 = tm; else t0 = tm;
    BINARY_SUBDIV
    BINARY_SUBDIV
    BINARY_SUBDIV
    BINARY_SUBDIV
    #undef BINARY_SUBDIV
    t = tm;
    return Sample(p);
}
vec4 Volume(vec3 ro, vec3 rd, float head, float tail){
    //todo:better way???
    if(VolumeSize.z <2.0) return clamp(Sample(ro+rd*head), vec4(.0), vec4(1.0));

    vec4 sum = vec4(.0);
    int steps = 0;
    float pd = .0;

    float vmax = .0;
    bool last_succeeded = true;
    float high_bound = 0.01;

    float step_size = usample_step_inverse;
    vec3 p;
    vec4 val_color;
    for(float t = head; t<tail; ){
        if(sum.a >= 0.95 ||steps>800) break;
        p = ro + rd * t;
        val_color = Sample(p);

        if(val_color.a > 0.01){
            pd = (1.0 - sum.a) * val_color.a;

            if (pd > high_bound){
                step_size/=2.0; high_bound= min(high_bound*2.0, 1.0);last_succeeded = false;
            }else{
                sum.rgb += (1.0 - sum.a) *  val_color.a* val_color.rgb;
                sum.a += pd;
                t += step_size;

                if (last_succeeded){ step_size*=2.0; high_bound/=2.0; }
                else last_succeeded = true;
            }
        }else{
            t+=4.0*usample_step_inverse;
        }
        steps++;
    }
    return vec4(sum.rgb, clamp(sum.a, 0.0, 1.0));
}

//vec4 Volume(vec3 ro, vec3 rd, float head, float tail){
//    //todo:better way???
//    if(VolumeSize.z <2.0) return clamp(Sample(ro+rd*head), vec4(.0), vec4(1.0));
//
//    vec4 sum = vec4(.0);
//    int steps = 0; float pd = .0;
//
//    float vmax = .0;
//    for(float t = head; t<tail; ){
//        if(sum.a >= 0.95) break;
//        vec3 p = ro + rd * t;
//        vec4 val_color = Sample(p);
//        if(val_color.a > 0.01){
//            if(pd < 0.01) val_color = subDivide(p, ro, rd, t, usample_step_inverse);
//            sum.rgb += (1.0 - sum.a) *  val_color.a* val_color.rgb;
//            sum.a += (1.0 - sum.a) * val_color.a;
//        }
//        t += val_color.a > 0.01? usample_step_inverse: usample_step_inverse * 4.0;
//        steps++;
//        pd = sum.a;
//    }
//    return vec4(sum.rgb, clamp(sum.a, 0.0, 1.0));
//}

vec4 tracing(float u, float v){
    float tangent = tan(u_fov / 2.0); // angle in radians
    float ar = (float(u_con_size.x) / u_con_size.y);

    vec3 ro = uCamposObjSpace;
    vec3 rd = vec3(normalize(u_WorldToModel * u_CamToWorld *vec4(u* tangent*ar, v*tangent, -constantNCP, .0)));

    vec2 intersect = RayCube(ro, rd, vec3(0.5));
    intersect.x = max(.0, intersect.x);
    VolumeSize = vec3(imageSize(srcTex));

    bool drawed_square=false; bool blocked_by_plane=false;
    //plane
    #ifdef CUTTING_PLANE
    float t;
    if(dot(uPlane.normal, -uCamposObjSpace) > .0){
        t = RayPlane(ro, rd, uPlane.p, uPlane.normal);
        blocked_by_plane = (t <= intersect.x);
        intersect.x = max(intersect.x, t);
    }
    else{t = RayPlane(ro, rd, uPlane.p, -uPlane.normal); intersect.y = min(intersect.y, t);}

    drawed_square = (abs(t) < 1000.0)?intersectRayWithSquare(ro+rd*t, uPlane.s1, uPlane.s2, uPlane.s3):false;

    if(blocked_by_plane && intersect.x <= intersect.y) return drawed_square?mix(u_plane_color, Volume(ro + 0.5, rd, intersect.x, intersect.y), u_plane_color.a): Volume(ro + 0.5, rd, intersect.x, intersect.y);
    #endif
    if(intersect.y < intersect.x || blocked_by_plane) return drawed_square?mix(u_plane_color, vec4(.0), u_plane_color.a):vec4(.0);

    return Volume(ro + 0.5, rd, intersect.x, intersect.y);
}
void main() {
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);
    float cx = float(storePos.x); float cy = float(storePos.y);
    if ( cx >= u_con_size.x ||  cy >= u_con_size.y) return;
    cy = u_con_size.y - cy;
    vec2 uv = (vec2(cx, cy) + 0.5) / u_con_size * 2.0 - 1.0;
    imageStore(destTex, ivec2(cx, cy), tracing(uv.x, uv.y));
}