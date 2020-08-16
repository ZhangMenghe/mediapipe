#version 310 es

#extension GL_EXT_shader_io_blocks:require
#extension GL_EXT_geometry_shader:require

precision mediump float;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(binding = 0, rgba8ui) readonly uniform mediump uimage3D u_volume;
// layout(binding = 0, r8) readonly uniform mediump image3D u_volume;
//layout(binding = 0, r8ui)readonly uniform mediump uimage3D u_volume;


// layout (rgba16f, binding = 0) readonly uniform image3D u_volume;

layout(std140, binding = 0) writeonly buffer u_buffer_vertices
{
    vec4 output_vertices[];
};

//layout(std140, binding = 1) writeonly buffer u_buffer_normals
//{
//    vec4 output_normals[];
//};

layout(std430, binding = 2) readonly buffer u_buffer_triangle_table 
{
    int triangle_table[];
};

layout(std430, binding = 3) readonly buffer u_buffer_configuration_table 
{
    int configuration_table[];
};

//// The current cell index (xyz)
//const ivec3 cell_index = ivec3(gl_GlobalInvocationID.xyz);
//
//// The indices of the 8 neighbors that form the boundary of this cell
//const ivec3 neighbors[8] =
//{
//	cell_index,
//	cell_index + ivec3(  0, 0, 1 ),
//	cell_index + ivec3( -1, 0, 1 ),
//	cell_index + ivec3( -1, 0, 0 ),
//
//	cell_index + ivec3(  0, 1, 0 ),
//	cell_index + ivec3(  0, 1, 1 ),
//	cell_index + ivec3( -1, 1, 1 ),
//	cell_index + ivec3( -1, 1, 0 )
//};

struct Vertex 
{
	vec3 position;
	vec3 normal;
};
uniform bool u_multiple;
uniform uint u_mask_id;
uniform uint u_maskbits;// = uint(31);
uniform uint u_organ_num;// = uint(4);
// uint u_organ_num = 6;
// uint u_maskbits = 1+2+4+8+16+32;

bool check_mask_bit(uint value){
	for(uint i=uint(0); i<u_organ_num; i++){
		if(((u_maskbits>> uint(i+uint(1))) & uint(1)) == uint(0)) continue;
		uint cv = uint(pow(float(2), float(i)));
		if(value == cv) return true;
	}
	return false;
}

float map(ivec3 p){
	uint sc = uint(imageLoad(u_volume, p).r);
	if(u_multiple){
		if(check_mask_bit(sc)) return -float(sc);
		return 1.0;
	}else{
		if(sc == u_mask_id)return -1.0;// sc.a;
		return 1.0;
	}
}
vec3 CalculateGradient(ivec3 p){
    const ivec3 e = ivec3(1, 0, 0);

    vec3 n = vec3(map(p + e.xyy) - map(p - e.xyy),  // Gradient x
                  map(p + e.yxy) - map(p - e.yxy),  // Gradient y
                  map(p + e.yyx) - map(p - e.yyx)); // Gradient z

    return normalize(n);

}
Vertex find_vertex(float isolevel, ivec3 p1, ivec3 p2, float value_1, float value_2)
{
	// Grab the two vertices at either end of the edge between `index_1` and `index_2`
//	vec3 p1 = neighbors[edge.x];
//	vec3 p2 = neighbors[edge.y];

	// The normals are stored in the YZW / GBA channels of the volume texture
	vec3 n1 = CalculateGradient(p1);//imageLoad(u_volume, ivec3(p1)).gba;
	vec3 n2 = CalculateGradient(p2);//imageLoad(u_volume, ivec3(p2)).gba;

	const float eps = 0.00001;

	if (abs(isolevel - value_1) < eps) 
	{
		return Vertex(vec3(p1), n1);
	}
	if (abs(isolevel - value_2) < eps) 
	{
		return Vertex(vec3(p2), n2);
	}
	if (abs(value_1 - value_2) < eps) 
	{
		return Vertex(vec3(p1), n1);
	}

	float mu = (isolevel - value_1) / (value_2 - value_1);

	vec3 p = vec3(p1) + mu * (vec3(p2 - p1));
	vec3 n = n1 + mu * (n2 - n1);

	return Vertex(p, normalize(n));
}
void march(in ivec3 cell_index){
	ivec3 volume_size = imageSize(u_volume);
	vec3 inv_volume_size = 1.0 / vec3(volume_size);

	// Avoid sampling outside of the volume bounds
	if (cell_index.x == 0 || 
		cell_index.y == (volume_size.y - 1) || 
		cell_index.z == (volume_size.z - 1)) return;

	ivec3 neighbors[8] = ivec3[8](
		cell_index,
		cell_index + ivec3(  0, 0, 1 ),
		cell_index + ivec3( -1, 0, 1 ),
		cell_index + ivec3( -1, 0, 0 ),

		cell_index + ivec3(  0, 1, 0 ),
		cell_index + ivec3(  0, 1, 1 ),
		cell_index + ivec3( -1, 1, 1 ),
		cell_index + ivec3( -1, 1, 0 )
	);
	ivec2 edge_table[12] = ivec2[12](
		ivec2(0,1),ivec2(1,2),ivec2(2,3),ivec2(3,0),ivec2(4,5),ivec2(5,6),
		ivec2(6,7),ivec2(7,4),ivec2(0,4),ivec2(1,5),ivec2(2,6),ivec2(3,7));

	// Calculate which of the 256 configurations this cell is 
	float values[8];
	int configuration = 0;
	for (int i = 0; i < 8; ++i){

		values[i] = map(neighbors[i]);

		if(values[i] < .0) configuration |= 1 << i;
	}

	// Normally, we could `return` here, but we want to make sure to "clear"
	// the vertex buffer at unused indices, otherwise the drawing gets super
	// wonky...
	if (configuration_table[configuration] == 0) 
	{		
		return;
	}

	// Grab all of the (interpolated) vertices along each of the 12 edges of this cell
	Vertex vertex_list[12];
	if (configuration_table[configuration] != 0) 
	{
		for (int i = 0; i < 12; ++i)
		{
			if (int(configuration_table[configuration] & (1 << i)) != 0)
			{	
				ivec2 edge = edge_table[i];
				vertex_list[i] = find_vertex(0.0, neighbors[edge.x], neighbors[edge.y], values[edge.x], values[edge.y]);
			}
		}
	}
	
	// Construct triangles based on this cell's configuration and the vertices calculated above
	vec4 empty = vec4(0.0);
	int cell_start_memory = (cell_index.x +
								   cell_index.y * volume_size.x + 
								   cell_index.z * volume_size.x * volume_size.y) * 15; // 15 = 5 tris per cell, 3 coordinates (xyz) each

	int triangle_start_memory = configuration * 16; // 16 = the size of each "row" in the triangle table
	int max_triangles = 5;

	for (int i = 0; i < max_triangles; ++i)
	{
		int idx = cell_start_memory +  3 * i;

		if (triangle_table[triangle_start_memory + 3 * i] != -1)
		{
			Vertex vertex;
			vec3 position;
			vec3 normal;


			vertex = vertex_list[triangle_table[triangle_start_memory + (3 * i + 0)]];
			position = vertex.position * inv_volume_size;
			position = position * 2.0 - 1.0;
			output_vertices[2*(idx + 0)] = vec4(position, 1.0);
			output_vertices[2*(idx + 0)+1] = vec4(vertex.normal, 1.0);

			//			output_normals[cell_start_memory +  3 * i + 0] = vec4(vertex.normal, 1.0);

			vertex = vertex_list[triangle_table[triangle_start_memory + (3 * i + 1)]];
			position = vertex.position * inv_volume_size;
			position = position * 2.0 - 1.0;
			output_vertices[2*(idx + 1)] = vec4(position, 1.0);
			output_vertices[2*(idx + 1)+1] = vec4(vertex.normal, 1.0);

			//			output_vertices[cell_start_memory +  3 * i + 1] = vec4(position, 1.0);
//			output_normals[cell_start_memory +  3 * i + 1] = vec4(vertex.normal, 1.0);

			vertex = vertex_list[triangle_table[triangle_start_memory + (3 * i + 2)]];
			position = vertex.position * inv_volume_size;
			position = position * 2.0 - 1.0;
			output_vertices[2*(idx + 2)] = vec4(position, 1.0);
			output_vertices[2*(idx + 2)+1] = vec4(vertex.normal, 1.0);

			//output_vertices[cell_start_memory +  3 * i + 2] = vec4(position, 1.0);
//			output_normals[cell_start_memory +  3 * i + 2] = vec4(vertex.normal, 1.0);
		}
		else
		{
			output_vertices[2*idx] = empty;
			output_vertices[2*idx+1] = empty;

			output_vertices[2*(idx+1)] = empty;
			output_vertices[2*(idx+1)+1] = empty;
			output_vertices[2*(idx+2)] = empty;
			output_vertices[2*(idx+2)+1] = empty;
			//			output_vertices[cell_start_memory + (3 * i + 0)] = empty;
//			output_vertices[cell_start_memory + (3 * i + 1)] = empty;
//			output_vertices[cell_start_memory + (3 * i + 2)] = empty;
		}
	} 
}

void main(){
	// Resolution of the 3D texture (W, H, D) - integer values
    ivec3 dims = imageSize(u_volume);

    // The 3D coordinates of this compute shader thread
    ivec3 cell_index = ivec3(gl_GlobalInvocationID.xyz);
    march(cell_index);
}