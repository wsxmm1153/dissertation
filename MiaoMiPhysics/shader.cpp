#include "shader.h"

const char *commonVertex = STRINGIFY(
#version 420\n
in vec3 vVertex;\n
in vec3 vColor;\n
uniform mat4 mv;\n
uniform mat4 p;\n
out vec4 vVaryingColor;\n
void main(void)\n
{\n
	gl_Position = p * mv * vec4(vVertex, 1.0);\n
	vVaryingColor = vec4(1.0, 1.0, 1.0, 1.0);\n
}\n
);

const char *commonFragment = STRINGIFY(
#version 420\n
	in vec4 vVaryingColor;\n
out vec4 FragColor;\n
void main(void)\n
{\n
	FragColor = vVaryingColor;\n
}\n
);

const char *backFaceTexcoordsVertex = STRINGIFY(
#version 430\n
in vec3 vVertex;\n
in vec3 vTexcoord;\n
uniform mat4 mv;\n
uniform mat4 p;\n
smooth out vec4 vVaryingColor;\n
void main(void)\n
{\n
	gl_Position = p * mv * vec4(vVertex, 1.0);\n
	vVaryingColor = vec4(vTexcoord, 1.0);\n
}
);

const char *backFaceTexcoordsFragment = STRINGIFY(
#version 430\n
in vec4 vVaryingColor;\n
out vec4 FragColor;\n
void main(void)\n
{\n
	FragColor = vVaryingColor;\n
}\n
);

const char *rayCastingVertex = STRINGIFY(
#version 430\n
in vec3 vVertex;\n
in vec3 vTexcoord;\n
uniform mat4 mv;\n
uniform mat4 p;\n
smooth out vec3 vVaryingTexCoord;\n
void main(void)\n
{\n
	gl_Position = p * mv * vec4(vVertex, 1.0);\n
	vVaryingTexCoord = vTexcoord;\n
}\n
);

const char *rayCastingFragment = STRINGIFY(
#version 430\n
in vec3 vVaryingTexCoord;\n
out vec4 FragColor;\n
uniform sampler2D backTex;\n
uniform usampler3D volumeTex;\n
uniform int textureX;\n
uniform int textureY;\n
uniform int width_3d;\n
void main(void)\n
{\n
	//...
	vec2 tex2DCoord = vec2(gl_FragCoord.x/textureX, gl_FragCoord.y/textureY);\n
	vec3 back3DCoord = texture(backTex, tex2DCoord).xyz;\n
	vec3 front3DCoord = vVaryingTexCoord;\n

	vec3 len = - back3DCoord + front3DCoord;\n
	float step = length(len)/(1.0/256.0);\n
	vec3 samplerStep = back3DCoord;\n
	vec4 colorSum = vec4(0.0, 0.0, 0.0, 0.0);\n
	vec4 colorStep;\n
	//float aStep = 1.0;\n
	while (step > 0)\n
	{\n
		vec3 s_step = samplerStep;\n
		uint inside = /*floatBitsToUint*/( texture(volumeTex, s_step).r);\n

		float i_step = 1.0f / float(width_3d/8);

		float offset_1 = mod(s_step.z, i_step);
		
		uint offset_2 = uint(floor(offset_1 * 8.0f / i_step));
		
		uint b_offset = (1 << (7 - offset_2));\n
		uint is_inside = (b_offset & inside);\n

		//if (is_inside == (1 << 7))\n
		if (is_inside > 0)\n
		{\n
			if (offset_2 == 0)
				colorStep = vec4(1.0, 1.0, 1.0, 0.3);\n
			else if (offset_2 == 1)
				colorStep = vec4(1.0, 0.0, 0.0, 0.3);\n
			else if (offset_2 == 2)
				colorStep = vec4(1.0, 0.5, 0.0, 0.3);\n
			else if (offset_2 == 3)
				colorStep = vec4(1.0, 1.0, 0.0, 0.3);\n
			else if (offset_2 == 4)
				colorStep = vec4(0.0, 1.0, 0.0, 0.3);\n
			else if (offset_2 == 5)
				colorStep = vec4(0.0, 0.0, 1.0, 0.3);\n
			else if (offset_2 == 6)
				colorStep = vec4(0.0, 0.5, 0.3, 0.3);\n
			else if (offset_2 == 7)
				colorStep = vec4(1.0, 0.0, 1.0, 0.3);\n
		}\n
		else\n
			colorStep = vec4(0.0f, 0.0f, 0.0f, 0.0f);\n

		//test step
		//if (offset_2 == 0)
		//	colorStep = vec4(1.0, 1.0, 1.0, 0.1);\n
		//else if (offset_2 == 1)
		//colorStep = vec4(1.0, 0.0, 0.0, 0.1);\n
		//else if (offset_2 == 2)
		//colorStep = vec4(1.0, 0.5, 0.0, 0.1);\n
		//else if (offset_2 == 3)
		//colorStep = vec4(1.0, 1.0, 0.0, 0.1);\n
		//else if (offset_2 == 4)
		//colorStep = vec4(0.0, 1.0, 0.0, 0.1);\n
		//else if (offset_2 == 5)
		//colorStep = vec4(0.0, 0.0, 1.0, 0.1);\n

		//else if (offset_2 == 6)
		//colorStep = vec4(0.0, 0.5, 0.3, 0.1);\n
		//else if (offset_2 == 7)
		//colorStep = vec4(1.0, 0.0, 1.0, 0.1);\n


		//colorSum = colorSum + (1.0-colorSum.a) * colorStep.a * colorStep;\n
		//colorSum.a = colorSum.a + (1.0-colorSum.a)*colorStep.a;\n

		colorSum = colorStep * colorStep.a + (1.0-colorStep.a) * colorSum;\n

		samplerStep += normalize(len)*(1.0/256.0);\n
		step -= 1.0;\n
	}\n
	FragColor = colorSum;\n
	//FragColor = vec4(back3DCoord, 1.0);\n
}\n
);

const char *drawDepthVertex = STRINGIFY(
#version 430\n
	in vec3 vVertex;\n
	uniform mat4 vm;\n
	uniform mat4 p;
	smooth out float z_value;\n
	void main(void)\n
{\n
vec4 pp = vm * vec4(vVertex, 1.0);\n
//z_value = pp.z / pp.w;
gl_Position = p * pp;\n
z_value = gl_Position.z / gl_Position.w;
}\n
);

const char *drawDepthFragment = STRINGIFY(
#version 430\n
	smooth in float z_value;\n
	out vec4 FragColor;\n
	layout (r32f) uniform image2D depth_image;
	void main(void)\n
{\n
//p*v*m��z�任���ˣ�-1.0f,1.0f��\n
//if(z_value < 0.0f)	discard;
FragColor = vec4(/*(z_value+1.0f)/2.0f*/gl_FragCoord.z
, gl_FragCoord.z, gl_FragCoord.z, 1.0f);\n
//test
//FragColor = vec4(0.0f, 0.0f, (zValue-0.9f)*5.0f, 1.0f);\n
//imageStore(depth_image, ivec2(gl_FragCoord.xy), vec4(FragColor.z, FragColor.z, FragColor.z, FragColor.z));\n
//imageStore(depth_image, ivec2(gl_FragCoord.xy), vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z));\n
}\n
);

const char *sphGridComputer = STRINGIFY(
#version 430\n
layout (local_size_x = 128) in;
layout (rgba32f, binding = 2) uniform volatile imageBuffer position_image;
layout (rgba32f, binding = 3) uniform volatile imageBuffer velocity_image;
layout (r32i, binding = 0) uniform volatile iimageBuffer grid_image_head;
layout (r32i, binding = 1) uniform volatile iimageBuffer grid_image_count;
layout (rgba32f, binding = 4) uniform volatile imageBuffer 	position_image_out;
layout (rgba32f, binding = 5) uniform volatile imageBuffer velocity_image_out;
//layout (r32i, binding = 4) uniform volatile iimageBuffer lock_image;
uniform float time_step;
uniform float smooth_length;
uniform ivec3 grid_size;
uniform float denisity_smooth_factor;
uniform float press_factor;
uniform float a_smooth_factor;
uniform float viscosity;
uniform vec3 a_outside;

int grid_hash(const int x, const int y, const int z)
{
	int hash_val = x + y * grid_size.x + z * grid_size.x * grid_size.y;
	return hash_val;
}

void main(void)\n
{\n
	int item_index = int(gl_GlobalInvocationID.x);
	vec4 position = imageLoad(position_image, item_index);
	vec4 velocity = imageLoad(velocity_image, item_index);

	/*********1 init grid(positions)*************************/
	vec4 grid_hashf = position / smooth_length;
	vec4 grid_hash_floor = floor(grid_hashf);
	//if (abs(grid_hashf.x - float(grid_size.x) * grid_hash_floor.x) < 10e-30)
	//	grid_hash_floor.x -= 1.0f;
	//if (abs(grid_hashf.y - float(grid_size.y) * grid_hash_floor.y) < 10e-30)
	//	grid_hash_floor.y -= 1.0f;
	//if (abs(grid_hashf.z - float(grid_size.z) * grid_hash_floor.z) < 10e-30)
	//	grid_hash_floor.z -= 1.0f;
	/*********grid hash********/
	ivec4 grid_hashi = ivec4(int(grid_hash_floor.x),
		int(grid_hash_floor.y), int(grid_hash_floor.z), 0);
	int hash_val = grid_hash(grid_hashi.x, grid_hashi.y, grid_hashi.z);
	/*********grid hash********/
	int old_head = imageAtomicExchange(grid_image_head,
		hash_val, item_index);
	imageAtomicAdd(grid_image_count, hash_val, 1);

	position.w = intBitsToFloat(old_head);
	//position.w = float(old_head);
	//barrier();
	//memoryBarrier();
	//int count_ = imageLoad(grid_image_count, 
	//	hash_val).x;
	//if (count_ > 8)
	//	position.w = 1.0f;
	//else
	//	position.w = -1.0f;
	//memoryBarrier();
	imageStore(position_image_out, item_index, position);
	imageStore(velocity_image_out, item_index, velocity);
	/*********1 init grid(positions)*************************/
}\n
);
const char *sphDenisityComputer = STRINGIFY(
#version 430\n
	layout (local_size_x = 128) in;
layout (rgba32f, binding = 2) uniform volatile imageBuffer position_image;
layout (rgba32f, binding = 3) uniform volatile imageBuffer velocity_image;
layout (r32i, binding = 0) uniform volatile iimageBuffer grid_image_head;
layout (r32i, binding = 1) uniform volatile iimageBuffer grid_image_count;
layout (rgba32f, binding = 4) uniform volatile imageBuffer 	position_image_out;
layout (rgba32f, binding = 5) uniform volatile imageBuffer velocity_image_out;
//layout (r32i, binding = 4) uniform volatile iimageBuffer lock_image;
uniform float time_step;
uniform float smooth_length;
uniform ivec3 grid_size;
uniform float denisity_smooth_factor;
uniform float press_factor;
uniform float a_smooth_factor;
uniform float viscosity;
uniform vec3 a_outside;

int grid_hash(const int x, const int y, const int z)
{
	int hash_val = x + y * grid_size.x + z * grid_size.x * grid_size.y;
	return hash_val;
}

void main(void)\n
{\n
int item_index = int(gl_GlobalInvocationID.x);
vec4 position = imageLoad(position_image, item_index);
vec4 velocity = imageLoad(velocity_image, item_index);

/*********1 init grid(positions)*************************/
vec4 grid_hashf = position / smooth_length;
vec4 grid_hash_floor = floor(grid_hashf);
//if (abs(grid_hashf.x - float(grid_size.x)) < 10e-30)
//	grid_hash_floor.x -= 1.0f;
//if (abs(grid_hashf.y - float(grid_size.y)) < 10e-30)
//	grid_hash_floor.y -= 1.0f;
//if (abs(grid_hashf.z - float(grid_size.z)) < 10e-30)
//	grid_hash_floor.z -= 1.0f;
/*********grid hash********/
ivec4 grid_hashi = ivec4(int(grid_hash_floor.x),
	int(grid_hash_floor.y), int(grid_hash_floor.z), 0);
int hash_val = grid_hash(grid_hashi.x, grid_hashi.y, grid_hashi.z);
/*********grid hash********/

/************2 denisity(grid)****************************/
int w_i = grid_hashi.x;
int h_j = grid_hashi.y;
int d_k = grid_hashi.z;
float denisity_i = 0.0f;
vec3 r_i = position.xyz;
for (int i = -1; i <= 1; i++)
{
	for (int j = -1; j <= 1; j++)
	{
		for (int k = -1; k <= 1; k++)
		{
			ivec3 grid_id = ivec3(w_i+i, h_j+j, d_k+k);
			if (grid_id.x == grid_size.x ||
				grid_id.y == grid_size.y ||
				grid_id.z == grid_size.z ||
				grid_id.x == -1 ||
				grid_id.y == -1 ||
				grid_id.z == -1 ) 
			{
				//position.w = 1.0f;
				continue;
			}
			int hash_j = grid_hash(grid_id.x, grid_id.y, grid_id.z);
			volatile int p_count = imageLoad(grid_image_count, hash_j).x;
			//p_count = 0;
			//if (p_count > 50)
			//{
			//	//position.w = 1.0f;
			//	p_count = 50;
			//}
			//p_count--;
			volatile int ptr = imageLoad(grid_image_head, hash_j).x;
			while(p_count > 0)
			{
				p_count--;
				vec4 position_next = imageLoad(position_image, ptr);

				vec3 r_j = position_next.xyz;
				float distance_ij = distance(r_i, r_j);
				if (distance_ij <= smooth_length)
				{
					float be_sum_sqrt = smooth_length * smooth_length
						- distance_ij*distance_ij;
					denisity_i += pow(be_sum_sqrt, 3.0f);
				}
				ptr = floatBitsToInt(position_next.w);
				//ptr = int(position_next.w);
				if (ptr < 0)
					break;
			}
		}
	}
}
denisity_i *= denisity_smooth_factor;
velocity.w = denisity_i;
imageStore(position_image_out, item_index, position);
imageStore(velocity_image_out, item_index, velocity);
//groupMemoryBarrier();
//denisity_i -= 2000.0f;
/************2 denisity(grid)****************************/
}\n
	);

const char *sphAccelerationComputer = STRINGIFY(
#version 430\n
	layout (local_size_x = 128) in;
layout (rgba32f, binding = 2) uniform volatile imageBuffer position_image;
layout (rgba32f, binding = 3) uniform volatile imageBuffer velocity_image;
layout (r32i, binding = 0) uniform volatile iimageBuffer grid_image_head;
layout (r32i, binding = 1) uniform volatile iimageBuffer grid_image_count;
layout (rgba32f, binding = 4) uniform volatile imageBuffer 	position_image_out;
layout (rgba32f, binding = 5) uniform volatile imageBuffer velocity_image_out;
//layout (r32i, binding = 4) uniform volatile iimageBuffer lock_image;
uniform float time_step;
uniform float smooth_length;
uniform ivec3 grid_size;
uniform float denisity_smooth_factor;
uniform float press_factor;
uniform float a_smooth_factor;
uniform float viscosity;
uniform vec3 a_outside;

int grid_hash(const int x, const int y, const int z)
{
	int hash_val = x + y * grid_size.x + z * grid_size.x * grid_size.y;
	return hash_val;
}

void main(void)\n
{\n
int item_index = int(gl_GlobalInvocationID.x);
vec4 position = imageLoad(position_image, item_index);
vec4 velocity = imageLoad(velocity_image, item_index);

/*********1 init grid(positions)*************************/
vec4 grid_hashf = position / smooth_length;
vec4 grid_hash_floor = floor(grid_hashf);
//if (abs(grid_hashf.x - float(grid_size.x)) < 10e-30)
//	grid_hash_floor.x -= 1.0f;
//if (abs(grid_hashf.y - float(grid_size.y)) < 10e-30)
//	grid_hash_floor.y -= 1.0f;
//if (abs(grid_hashf.z - float(grid_size.z)) < 10e-30)
//	grid_hash_floor.z -= 1.0f;
/*********grid hash********/
ivec4 grid_hashi = ivec4(int(grid_hash_floor.x),
	int(grid_hash_floor.y), int(grid_hash_floor.z), 0);
int hash_val = grid_hash(grid_hashi.x, grid_hashi.y, grid_hashi.z);
/*********grid hash********/
/********3 acceleration(denisity)************************/
int w_i = grid_hashi.x;
int h_j = grid_hashi.y;
int d_k = grid_hashi.z;
vec3 r_i = position.xyz;
vec3 acceleration_p = vec3(0.0f, 0.0f, 0.0f);
vec3 acceleration_miu = vec3(0.0f, 0.0f, 0.0f);
vec3 u_i = velocity.xyz;
float denisity_i = velocity.w;
float p_i = press_factor * (denisity_i - 1000.0f);
int grid_count = imageLoad(grid_image_head, hash_val).x;
float denisity_test = (imageLoad(velocity_image, int(0))).w;
//barrier();
for (int i = -1; i <= 1; i++)
{
	for (int j = -1; j <= 1; j++)
	{
		for (int k = -1; k <= 1; k++)
		{
			ivec3 grid_id = ivec3(w_i+i, h_j+j, d_k+k);
			if ((grid_id.x >= grid_size.x) ||
				(grid_id.y >= grid_size.y) ||
				(grid_id.z >= grid_size.z) ||
				(grid_id.x < 0) ||
				(grid_id.y < 0) ||
				(grid_id.z < 0) ) 
				continue;
			int hash_j = grid_hash(grid_id.x, grid_id.y, grid_id.z);
			volatile int p_count = imageLoad(grid_image_count, hash_j).x;

			if (p_count <= 0)
				continue;
			
			volatile int ptr = imageLoad(grid_image_head, hash_j).x;

			for(int t = 0; t < p_count; t++)
			{
				vec4 position_next = imageLoad(position_image, ptr);

				vec3 r_j = position_next.xyz;
				float distance_ij = distance(r_i, r_j);
				if (distance_ij <= smooth_length)
				{
					vec4 velocity_denisity = imageLoad(velocity_image, ptr);
					float denisity_j = velocity_denisity.w;
					//if (denisity_j <= 0.0f)
					//	denisity_test = 1.0f;
					vec3 u_j = velocity_denisity.xyz;
					float p_j = press_factor * (denisity_j - 1000.0f);
					if (distance_ij/**denisity_i*denisity_j */> 0.0f)
					{
						acceleration_p += (r_i-r_j) * (p_i + p_j)
							*pow(smooth_length - distance_ij, 2.0f)*0.5f
							/(denisity_i*denisity_j*distance_ij);
					}
					else if(ptr != item_index)
					{
						distance_ij = 10e-20f;
						acceleration_p += (r_i-r_j) * (p_i + p_j)
							*pow(smooth_length - distance_ij, 2.0f)*0.5f
							/(denisity_i*denisity_j*distance_ij);
					}

					acceleration_miu += (smooth_length - distance_ij)
							/(denisity_i*denisity_j)
							*(u_j - u_i);
				}
				ptr = floatBitsToInt(position_next.w);
				if (ptr < 0)
					break;
			}
		}
	}
}
acceleration_miu *= (viscosity*a_smooth_factor);
acceleration_p *= a_smooth_factor;
vec3 acceleration_i = a_outside + acceleration_p + acceleration_miu;
/********3 acceleration(denisity)************************/

/*********4 pos/vel(acceleration)************************/
vec3 position_new = position.xyz + time_step * velocity.xyz
	+ 0.5f * time_step * time_step * acceleration_i;
vec3 velocity_new = velocity.xyz + time_step * acceleration_i;
//boundary
//if (position_new.x >= 0.5f)
//{
//	position_new.x = 0.5f;
//	//velocity_new.x = -velocity_new.x;
//	velocity_new.x = -velocity_new.x * 0.5;
//}
//if (position_new.y >= 1.0f)
//{
//	position_new.y = 1.0f;
//	//velocity_new.y = -velocity_new.y;
//
//	velocity_new.y = -velocity_new.y * 0.5;
//}
//if (position_new.z >= 0.5f)
//{
//	position_new.z = 0.5f;
//	//velocity_new.z = -velocity_new.z;
//	velocity_new.z = -velocity_new.z * 0.5;
//}
//
//if (position_new.x <= 0.0f)
//{
//	position_new.x = 0.0f;
//	//velocity_new.x = -velocity_new.x;
//	velocity_new.x = -velocity_new.x * 0.5;
//}
//if (position_new.y <= 0.0f)
//{
//	position_new.y = 0.0f;
//	//velocity_new.y = -velocity_new.y;
//	velocity_new.y = -velocity_new.y * 0.5;
//}
//if (position_new.z <= 0.0f)
//{
//	position_new.z = 0.0f;
//	//velocity_new.z = -velocity_new.z;
//	velocity_new.z = -velocity_new.z * 0.5;
//}

/*********4 pos/vel(acceleration)************************/

/******************5 reset*******************************/
imageStore(position_image_out, item_index, vec4(position_new.xyz, denisity_test));
imageStore(velocity_image_out, item_index, vec4(velocity_new.xyz, denisity_test));
/******************5 reset*******************************/
}\n
	);

const char *collisionComputer = STRINGIFY(
#version 430\n
layout (local_size_x = 128) in;
layout (rgba32f, binding = 2) uniform volatile imageBuffer position_image_pre;
layout (rgba32f, binding = 3) uniform volatile imageBuffer velocity_image_pre;
layout (rgba32f, binding = 4) uniform volatile imageBuffer 	position_image_next;
layout (rgba32f, binding = 5) uniform volatile imageBuffer velocity_image_next;
layout (r8ui, binding = 0) uniform volatile uimage3D scene_image;
uniform mat4 scene_matrix;
uniform ivec3 scene_size;

int sampleFromScene(const ivec3 pos)
{
	if (pos.x >= scene_size.x || 
		pos.y >= scene_size.y ||
		pos.z >= scene_size.z ||
		pos.x < 0 ||
		pos.y < 0 ||
		pos.z < 0)
		return 0;
	//uvec4
	uint data = (imageLoad(scene_image, ivec3(pos.xy, (pos.z)/8))).r;

	uint offset_z = uint(mod(pos.z, 8));
	uint b_offset = (1 << (7 - offset_z));
	uint is_inside = (b_offset & data);
	if (is_inside > 0)
	//if (data != 0)
		return 1;
	return 0;
	//ivec3 size_t = imageSize(scene_image);
	//return size_t.z;
}
float weightInPos(const ivec3 pos)
{
	int weight_i = 0;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				int dist = abs(i)+abs(j)+abs(k);
				weight_i += sampleFromScene(ivec3(i, j, k) + pos)*(4-dist);
			}
		}
	}
	return float(weight_i);
}
ivec4 collisionCoord(const vec4 pos_pre, const vec4 pos_next)
{
	vec3 deta_3f = vec3(pos_next.xyz) - vec3(pos_pre.xyz);
	float deta_len = length(deta_3f);
	if (deta_len < 1.0e-30f)
	{
		ivec3 coord = ivec3(round(pos_next.xyz));
		//if (sampleFromScene(coord) == 1)
		if (weightInPos(coord) > 0.0f)
			return ivec4(coord.xyz, 1);
		return ivec4(0, 0, 0, 0);
	}
	float deta_max = 0.0f;
	int base_i = 0;
	for (int i = 0; i < 3 ; i++)
	{
		if (deta_max < abs(deta_3f[i]) && abs(deta_3f[i]) != 0.0f)
		{
			deta_max = abs(deta_3f[i]);
			base_i = i;
		}
	}
	deta_3f /= (abs(deta_3f[base_i]));

	ivec3 pos_pre_i = ivec3(round(pos_pre.xyz));
	ivec3 pos_next_i = ivec3(round(pos_next.xyz));
	int step = abs(pos_next_i[base_i] - pos_pre_i[base_i])+1;
	//int test;
	for (int i = 0; i < step; i++)
	{
		ivec3 current_coord = ivec3(round(pos_pre.xyz + deta_3f*float(i)));
		//test = sampleFromScene(current_coord);
		//if (sampleFromScene(current_coord) == 1)
		if (weightInPos(current_coord) > 0.0f)
			return ivec4(current_coord, 1);
	}
	//int test = sampleFromScene(pos_next_i);
	int test = int(step);
	return ivec4(0, 0, 0, 0);
}

vec3 findNormal(const ivec3 pos)
{
	vec3 normal_ = vec3(0.0f, 1.0f, 0.0f);
	//float weight_p = weightInPos(pos);
	normal_.x = weightInPos(pos+ivec3(1,0,0)) - weightInPos(pos+ivec3(0,0,0));
	normal_.y = weightInPos(pos+ivec3(0,1,0)) - weightInPos(pos+ivec3(0,0,0));
	normal_.z = weightInPos(pos+ivec3(0,0,1)) - weightInPos(pos+ivec3(0,0,0));
	return normalize(normal_);
}

void main(void)
{
	mat4 scene_matrix_inverse = inverse(scene_matrix);
	int item_index = int(gl_GlobalInvocationID.x);
	vec4 position_pre = imageLoad(position_image_pre, item_index);
	vec4 velocity_pre = imageLoad(velocity_image_pre, item_index);
	vec4 position_next = imageLoad(position_image_next, item_index);
	vec4 velocity_next = imageLoad(velocity_image_next, item_index);

	vec4 position_pre_in_image = scene_matrix * vec4(position_pre.xyz, 1.0f);
	vec4 position_next_in_image = scene_matrix * vec4(position_next.xyz, 1.0f);
	float test = 0.0f;
	float test2 = 0.0f;
	//bool b_test = true;
	//if (b_test)
	//{
	//	test = sampleFromScene(ivec3(56, 128, 128));
	//}
	ivec4 collision_v4 = collisionCoord(position_pre_in_image,
		position_next_in_image);
	//test = (distance(position_pre.xyz,position_next.xyz));
	test = float(collision_v4.y);
	if (collision_v4.w == 1)
	{
		vec3 normal = findNormal(ivec3(collision_v4.xyz));
		test2 = normal.x;
		vec3 pos_deta = position_next.xyz - position_pre.xyz;
		//if (dot(normal, pos_deta)<0.0f)
		//normal = -normal;
		//reflect
		float v_length = length(velocity_next.xyz);
		vec3 velocity_collision = normalize(velocity_pre.xyz
			- 1.0f * dot(normal, velocity_next.xyz) * normal) * v_length * 1.0f;
		velocity_next = vec4(velocity_collision.xyz, velocity_next.w);

		vec3 pos_in_image_3f = vec3(float(collision_v4.x),
			float(collision_v4.y),
			float(collision_v4.z));
		float p_w = position_next.w;
		//position_next = scene_matrix_inverse * vec4(pos_in_image_3f.xyz, 1.0f);
		position_next = position_pre + velocity_next * 0.7e-3f;
		position_next.w = p_w;
	}
	//scene limit
	if (position_next.x >= 0.5f)
	{
		position_next.x = 0.5f;
		//velocity_new.x = -velocity_new.x;
		velocity_next.x = -velocity_next.x * 0.1;
	}
	if (position_next.y >= 1.0f)
	{
		position_next.y = 1.0f;
		//velocity_new.y = -velocity_new.y;

		velocity_next.y = -velocity_next.y * 0.1;
	}
	if (position_next.z >= 0.5f)
	{
		position_next.z = 0.5f;
		//velocity_new.z = -velocity_new.z;
		velocity_next.z = -velocity_next.z * 0.1;
	}

	if (position_next.x <= 0.0f)
	{
		position_next.x = 0.0f;
		//velocity_new.x = -velocity_new.x;
		velocity_next.x = -velocity_next.x * 0.1;
	}
	if (position_next.y <= 0.0f)
	{
		position_next.y = 0.0f;
		//velocity_new.y = -velocity_new.y;
		velocity_next.y = -velocity_next.y * 0.1;
	}
	if (position_next.z <= 0.0f)
	{
		position_next.z = 0.0f;
		//velocity_new.z = -velocity_new.z;
		velocity_next.z = -velocity_next.z * 0.1;
	}
	//if boundary, reset
	//if (position_next.y <= 0.0f)
	//{
	//	position_next.y = 0.9f;
	//	position_next.x = abs(0.25f - position_next.x);
	//	position_next.z = abs(0.25f - position_next.z);
	//	velocity_next.xyz = vec3(0.0f, 0.0f, 0.0f);
	//	test = -1.0f;
	//}

	imageStore(position_image_next, item_index, vec4(position_next.xyz,
		1.0f));
	imageStore(velocity_image_next, item_index, vec4(velocity_next.xyz,
		1.0f));
	imageStore(position_image_pre, item_index, vec4(position_next.xyz,
		1.0f));
	imageStore(velocity_image_pre, item_index, vec4(velocity_next.xyz,
		1.0f));
}
);

const char *pointSpriteVertexShader = STRINGIFY(
#version 430\n
	in vec4 vVertex; \n
	uniform float pointRadius; \n
	uniform float pointScale; \n
	smooth out vec3 vNormal; \n
	uniform mat4 modelview; \n
	uniform mat4 projection; \n
	smooth out vec4 eyePos;\n
	void main()\n
{ \n
vec3 posEye = vec3(modelview * vec4(vVertex.xyz, 1.0)); \n
float dist = length(posEye); \n
	
gl_PointSize = pointRadius * (pointScale / dist); \n

gl_Position = projection * modelview * vec4(vVertex.xyz, 1.0); \n

eyePos = vec4(posEye.xyz, pointRadius);\n
	vNormal = vec3(0.0, 0.0, 1.0); \n
}\n
); 


const char *pointSpriteFragmentShader = STRINGIFY(
#version 430\n
	in vec3 vNormal; \n
	void main()\n
{ \n
const vec3 lightDir = vec3(0.577, 0.577, 0.577); \n

	// calculate normal from texture coordinates\n
	vec3 N; \n
	//N.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);\n
	N.xy = gl_PointCoord*vec2(2.0, -2.0) + vec2(-1.0, 1.0); \n

	float mag = dot(N.xy, N.xy); \n

	if (mag > 1.0) discard;   // kill pixels outside circle\n
	//if (vNormal.x < 0.5f) discard;\n
N.z = sqrt(1.0 - mag); \n
	//N = normalize(vNormal);\n
	//calculate lighting\n
	float diffuse = max(0.0, dot(lightDir, N)); \n

	gl_FragColor = vec4(vNormal, 1.0f) * diffuse; \n

	vec3 reflection = reflect(-lightDir, N); \n
	float reflectAngle = max(0.0, dot(N, reflection)); \n
	float spec = pow(reflectAngle, 64); \n
	vec4 specColor = vec4(1.0, 1.0, 1.0, 1.0) * gl_FragColor * spec; \n
	gl_FragColor += specColor; \n
}\n
);

const char *depthTextureFragmentShader = STRINGIFY(
#version 430\n
	in vec4 eyePos;\n
	in vec3 vNormal; \n
	uniform mat4 projection_mat;\n
	void main()\n
{ \n
vec3 N; \n
N.xy = gl_PointCoord*vec2(2.0, -2.0) + vec2(-1.0, 1.0); \n
float mag = dot(N.xy, N.xy); \n
if (mag > 1.0) discard;   // kill pixels outside circle\n

N.z = sqrt(1.0 - mag); \n
vec3 pixPos = eyePos.xyz;/* + N * eyePos.w;*/\n
pixPos.z += N.z * eyePos.w;
vec4 pixPos_S = projection_mat * vec4(pixPos, 1.0f);\n
float depthColor = pixPos_S.z / pixPos_S.w;
gl_FragColor = vec4(depthColor, 0.0f, 0.0f, 1.0f);
}\n
);

const char *thickTextureFragmentShader = STRINGIFY(
#version 430\n
	uniform int textureX;\n
	uniform int textureY;\n
	in vec4 eyePos;\n
	//in vec3 vNormal; \n
	uniform mat4 projection_mat;\n
	uniform sampler2D scene_depth; \n
	void main()\n
{ \n
vec3 N; \n
N.xy = gl_PointCoord*vec2(2.0, -2.0) + vec2(-1.0, 1.0); \n
float mag = dot(N.xy, N.xy); \n
if (mag > 1.0) discard;   // kill pixels outside circle\n

N.z = sqrt(1.0 - mag); \n
	vec3 pixPos = eyePos.xyz;/* + N * eyePos.w;*/\n
	pixPos.z -= N.z * eyePos.w;\n
vec4 pixPos_S = projection_mat * vec4(pixPos, 1.0f); \n
vec2 texCoord = vec2((gl_FragCoord.x) / textureX, (gl_FragCoord.y) / textureY); \n
vec4 depthr = texture2D(scene_depth, texCoord); \n
gl_FragColor = depthr;\n
		//if (pixPos_S.z /*/ pixPos_S.w*/ > depthr.r /** 10.0f*/)\n
		if (gl_FragCoord.z > depthr.r)\n
			//discard; \n
			gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n
		else\n
	gl_FragColor = vec4(0.0f, 0.2f, 0.8f, 0.1f);\n
}\n
	);

const char *screenSpaceFragmentShader = STRINGIFY(
	#version 430\n
	uniform int textureX; \n
	uniform int textureY; \n
	uniform sampler2D texture; \n
	uniform sampler2D texture_thick; \n
	uniform sampler2D scene_depth;\n
uniform float gaosi[49];\n
uniform mat4 projection_trans;\n
in vec4 eyePos; \n
float smoothDepth(sampler2D texture, vec2 coord, int textureX, int textureY)\n
{\n
	vec2 offsetX = vec2(1.0f/textureX, 0.0f);\n
	vec2 offsetY = vec2(0.0f, 1.0/textureY);\n
	float smooth_ = 0.0f;\n
	float depth_ = texture2D(texture, coord).x;
	//for(int i=-3; i<=3; i++)\n
	//	for(int j=-3; j<=3; j++)\n
	//	{\n
	//	vec2 coordi = coord + float(i)*offsetX + float(j)*offsetY;\n
	//	float t_x = texture2D(texture, coordi).x;\n
	//	if ((t_x >= 0.0f) && (t_x < 1.0f))\n
	//		smooth_ += t_x * gaosi[(i+3)*7 + j+3];\n
	//	else \n
	//		smooth_ += 1.0f * gaosi[(i+3)*7 + j+3];\n
	//	}\n
	float wsum = 0.0f; \n
	for (int i = -3; i <= 3; i++)
		for (int j = -3; j <= 3; j++)
		{
			vec2 coordi = coord + float(i)*offsetX + float(j)*offsetY;\n
			float sample_ = texture2D(texture, coordi).x;\n
			float r_ = pow(offsetX.x * float(i), 2.0f) + pow(offsetY.y * float(j), 2.0f);\n
			float w_ = exp(-r_ * 2.0f / 49.0f / 49.0f);
			float r2_ = sample_ - depth_;
			float g_ = exp(-r2_*r2_* 2.0f / 49.0f / 49.0f);
			smooth_ += sample_ * w_ * g_;
			wsum += w_ * g_;
		}
	return smooth_ / wsum; \n
}\n
void main(void)\n
	{ \n

	const vec3 lightDir = normalize(vec3(1.0f, 1.0f, 1.0f)); \n
	vec2 texCoord = vec2((gl_FragCoord.x) / textureX, (gl_FragCoord.y) / textureY); \n
	//float z = texture2D(texture, texCoord).x;\n
	float z = smoothDepth(texture, texCoord, textureX, textureY); \n
	vec4 depthr = texture2D(scene_depth, texCoord); \n


	vec2 r = gl_PointCoord*vec2(2.0, -2.0) + vec2(-1.0, 1.0);\n
	float mag = dot(r.xy, r.xy);	\n
	if (mag > 1.0) discard;\n

	if (gl_FragCoord.z > depthr.r)\n
			discard;\n
	vec4 viewCoord = vec4(texCoord * 2.0 - vec2(1.0, 1.0), z, 1.0);\n
	vec4 eyePos = projection_trans * viewCoord;\n

	vec2 texCoordX = vec2((float(gl_FragCoord.x)+1.0f) / float(textureX) 
		, (float(gl_FragCoord.y)) / float(textureY));\n
	//float zX = texture2D(texture, texCoordX).x;
	float zX = smoothDepth(texture, texCoordX, textureX, textureY);\n

	vec4 viewCoordX = vec4(texCoordX * 2.0 - vec2(1.0, 1.0), zX, 1.0);\n
	vec4 eyePosX = projection_trans * viewCoordX;\n

	vec2 texCoordY = vec2((float(gl_FragCoord.x)) / float(textureX) , (float(gl_FragCoord.y)+1.0f) / float(textureY));\n
	//float zY = texture2D(texture, texCoordY).x;\n
	float zY = smoothDepth(texture, texCoordY, textureX, textureY);\n
	vec4 viewCoordY = vec4(texCoordY * 2.0 - vec2(1.0, 1.0), zY, 1.0);\n
	vec4 eyePosY = projection_trans * viewCoordY;\n

	vec3 ddx = vec3(eyePos - eyePosX);\n
	vec3 ddx2 = vec3(eyePosX - eyePos);\n
	if (abs(ddx.z) > abs(ddx2.z))\n
	{\n
		ddx = ddx2;\n
	}\n

	vec3 ddy = vec3(eyePos - eyePosY);\n
	vec3 ddy2 = vec3(eyePosY - eyePos);\n
	if (abs(ddy.z) < abs(ddy2.z))\n
	{\n
		ddy = ddy2;\n
	}\n

	vec3 n = normalize(cross(ddx, ddy));\n

	// calculate normal from texture coordinates\n
	float diffuse = max(0.0, dot(lightDir, n));\n
	
	//gl_FragColor = vec4(0.0, 0.0, pow(gl_FragCoord.z, 3), 1.0);\n
	vec4 thick_color = texture2D(texture_thick, texCoord);\n
	//gl_FragColor = vec4(0.0, 0.0,0.5, 1.0);\n
	gl_FragColor = /*vec4(-1.0f, -1.0f, -1.0f, -1.0f) +*/ thick_color;\n
	gl_FragColor *= diffuse;\n
	vec3 reflection = reflect(-lightDir, n);\n
	float reflectAngle = max(0.0, dot(n, reflection));\n
	float spec = pow(reflectAngle, 8);\n
	vec4 specColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * spec; \n
	gl_FragColor += specColor;\n
	//gl_FragColor += vec4(0.1f, 0.1f, 0.1f, 0.0);\n
}\n
);
	
const char* phongVertex = STRINGIFY(
#version 430\n
	in vec3 vVertex; \n
	in vec3 vNormal; \n
	in vec2 vTexCoords; \n
	uniform mat4 mv; \n
	uniform mat4 p; \n
	//uniform vec3 vLightPosition;\n
	smooth out vec3 vVaryingNormal; \n
	//smooth out vec3 vVaryingLightDir;\n
	smooth out vec2 vVaryingTexCoords; \n
	smooth out vec4 eyePos;\n
	void main(void)\n
{ \n
vVaryingNormal = normalize((mv * vec4(vNormal, 1.0) - \n
mv * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n
vec4 vPosition = mv * vec4(vVertex, 1.0); \n
vec3 vPosition3 = vPosition.xyz / vPosition.w; \n
		//vVaryingLightDir = vLightPosition - vPosition3;\n
		vVaryingTexCoords = vTexCoords; \n
		gl_Position = p * vPosition; \n
		eyePos = vec4(gl_Position.xyz, 1.0f); \n
}\n
); 

const char* phongFragment = STRINGIFY(
#version 430\n
	out vec4 vFragColor; \n
	//uniform vec4 ambientColor;\n
	uniform vec4 diffuseColor; \n
	//uniform vec4 specularColor;\n
	//uniform sampler2D colorMap;\n
	in vec3 vVaryingNormal; \n
	in vec3 vVaryingLightDir; \n
	in vec2 vVaryingTexCoords; \n
	in vec4 eyePos; \n
	void main(void)\n
{ \n
float diff = max(0.0, dot(normalize(vVaryingNormal), \n
normalize(vec3(1.0f, 1.0f, 1.0f)))); \n

vFragColor = diff * diffuseColor; \n
float dt = (eyePos.z / eyePos.w); \n
vFragColor.a = dt; \n
vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal))); \n
float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection)); \n
		if (diff != 0)\n
		{ \n
			float fSpec = pow(spec, 128.0); \n
			vFragColor.rgb += vec3(fSpec, fSpec, fSpec); \n
		}\n
		//vFragColor = vec4(dt, dt, dt, dt);
}\n
); 

	const char* phongVertex2 = STRINGIFY(
#version 430\n
		in vec3 vVertex; \n
		in vec3 vNormal; \n
		in vec2 vTexCoords; \n
		uniform mat4 mv; \n
		uniform mat4 p; \n
	//uniform vec3 vLightPosition;\n
	smooth out vec3 vVaryingNormal; \n
	//smooth out vec3 vVaryingLightDir;\n
	smooth out vec2 vVaryingTexCoords; \n
	smooth out vec4 eyePos;\n
	void main(void)\n
	{ \n
	vVaryingNormal = normalize((mv * vec4(vNormal, 1.0) - \n
	mv * vec4(0.0, 0.0, 0.0, 1.0)).xyz); \n
	vec4 vPosition = mv * vec4(vVertex, 1.0); \n
	vec3 vPosition3 = vPosition.xyz / vPosition.w; \n
		//vVaryingLightDir = vLightPosition - vPosition3;\n
		vVaryingTexCoords = vTexCoords; \n
		gl_Position = p * vPosition; \n
		//eyePos = vec4(gl_Position.xyz, 1.0f); \n
		eyePos = gl_Position; \n
	}\n
	);

	const char* phongFragment2 = STRINGIFY(
		#version 430\n
		out vec4 vFragColor; \n
		//uniform vec4 ambientColor;\n
		uniform vec4 diffuseColor; \n
		//uniform vec4 specularColor;\n
		//uniform sampler2D colorMap;\n
		in vec3 vVaryingNormal; \n
		in vec3 vVaryingLightDir; \n
		in vec2 vVaryingTexCoords; \n
		in vec4 eyePos; \n
		void main(void)\n
	{ \n
	float diff = max(0.0, dot(normalize(vVaryingNormal), \n
	normalize(vec3(1.0f, 1.0f, 1.0f)))); \n

	vFragColor = diff * diffuseColor; \n
	float dt = (eyePos.z/* / eyePos.w*/); \n
	dt = gl_FragCoord.z; \n
		
	vFragColor = vec4(dt, dt, dt, dt); \n
	}\n
	);

	const char* OutPutVertex = STRINGIFY(
#version 430\n
	in vec2 vVertex;
	in vec2 vTexCoords;
	uniform mat4 mv;
	uniform mat4 p;
	
	smooth out vec2 vVaryingTexCoords;
		void main(void)
	{
		vec4 vPosition = mv * vec4(vVertex, 0.0, 1.0);;
		vVaryingTexCoords = vTexCoords;
		gl_Position = p * vPosition;
	}
	);

	const char* OutPutFragment = STRINGIFY(
#version 430\n
		out vec4 vFragColor;
	in vec2 vVaryingTexCoords;
	uniform sampler2D fluid_image;
	uniform sampler2D scene_image;

	void main(void)
	{
		//vFragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
		vFragColor = texture2D(fluid_image, vVaryingTexCoords)
			+ texture2D(scene_image, vVaryingTexCoords);
	}
	);

GLuint compileProgram(const char* vertexSrc, const char* fragmentSrc)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertexShader, 1, &vertexSrc, 0);
	glShaderSource(fragmentShader, 1, &fragmentSrc, 0);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	GLuint program = glCreateProgram();

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		char temp[256];
		glGetProgramInfoLog(program, 256, 0, temp);
		printf("Fail to link program:\n%s\n", temp);
		glDeleteProgram(program);
		program = 0;
	}

	return program;
}

GLuint compileComputer(const char* computeShaderStr)
{
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

	glShaderSource(computeShader, 1, &computeShaderStr, 0);

	glCompileShader(computeShader);

	GLuint program = glCreateProgram();

	glAttachShader(program, computeShader);

	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		char temp[256];
		glGetProgramInfoLog(program, 256, 0, temp);
		printf("Fail to link program:\n%s\n", temp);
		glDeleteProgram(program);
		program = 0;
	}

	return program;
}