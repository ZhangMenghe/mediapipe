#ifndef MEDIAPIPE_GPU_GL_PIPELINE_PRIMITIVE_H
#define MEDIAPIPE_GPU_GL_PIPELINE_PRIMITIVE_H
namespace mediapipe{
const unsigned int quad_indices[6]= {0,1,2,
                                     0,2,3};
const float quad_vertices[12]={
        0.5f, 1.0f, .0f,//top-right
        -0.5f,1.0f, .0f,//top-left
        -0.5f, .0f, .0f,//bottom-left
        0.5f, .0f,  .0f//bottom-right
};
const float quad_vertices_2d[8]={
        0.5f, .5f,//top-right
        -0.5f, .5f,//top-left
        -0.5f, -.5f,//bottom-left
        0.5f, -.5f,//bottom-right
};
const float quad_vertices_tex[24]={
        0.5f, 1.0, .0f,    1.0, 1.0, .0f,//top-right
        -0.5f, 1.0, .0f,     .0, 1.0, .0f,//top-left
        -0.5f, .0f, .0f,     .0,  .0,  .0f,//bottom-left
        0.5f, .0f, .0f,    1.0,  .0,  .0f,//bottom-right
};
const float quad_vertices_tex_standard[24] = {//Word          Texture
        -1.0f,-1.0f,.0,	0.0f,0.0f,.0,
        1.0f,-1.0f,	.0,	1.0f,0.0f,.0,
        1.0f,1.0f,	.0, 1.0f,1.0f,.0,
        -1.0f,1.0f,	.0, 0.0f,1.0f,.0
};

const float cuboid[24] = {//World
        -0.5f,-0.5f,0.5f,
        0.5f,-0.5f,0.5f,
        0.5f,0.5f,0.5f,
        -0.5f,0.5f,0.5f,
        -0.5f,-0.5f,-0.5f,
        0.5f,-0.5f,-0.5f,
        0.5f,0.5f,-0.5f,
        -0.5f,0.5f,-0.5f,
};

const float cuboid_with_texture[48] = {//World
        -0.5f,-0.5f,0.5f,   0.0f,0.0f,1.0f,
        0.5f,-0.5f,0.5f,    1.0f,0.0f,1.0f,
        0.5f,0.5f,0.5f,	    1.0f,1.0f,1.0f,
        -0.5f,0.5f,0.5f,    0.0f,1.0f,1.0f,
        -0.5f,-0.5f,-0.5f,  0.0f,0.0f,0.0f,
        0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,
        0.5f,0.5f,-0.5f,    1.0f,1.0f,0.0f,
        -0.5f,0.5f,-0.5f,   0.0f,1.0f,0.0f,
};
const unsigned int cuboid_indices[36] = { 0,1,2,0,2,3,	//front
                                          4,6,7,4,5,6,	//back
                                          4,0,3,4,3,7,	//left
                                          1,5,6,1,6,2,	//right
                                          3,2,6,3,6,7,	//top
                                          4,5,1,4,1,0,	//bottom
};
}
#endif