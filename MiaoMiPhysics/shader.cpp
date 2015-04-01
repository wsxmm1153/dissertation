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
				colorStep = vec4(1.0, 1.0, 1.0, 0.1);\n
			else if (offset_2 == 1)
				colorStep = vec4(1.0, 0.0, 0.0, 0.1);\n
			else if (offset_2 == 2)
				colorStep = vec4(1.0, 0.5, 0.0, 0.1);\n
			else if (offset_2 == 3)
				colorStep = vec4(1.0, 1.0, 0.0, 0.1);\n
			else if (offset_2 == 4)
				colorStep = vec4(0.0, 1.0, 0.0, 0.1);\n
			else if (offset_2 == 5)
				colorStep = vec4(0.0, 0.0, 1.0, 0.1);\n
			else if (offset_2 == 6)
				colorStep = vec4(0.0, 0.5, 0.3, 0.1);\n
			else if (offset_2 == 7)
				colorStep = vec4(1.0, 0.0, 1.0, 0.1);\n
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
//p*v*m将z变换到了（-1.0f,1.0f）\n
//if(z_value < 0.0f)	discard;
FragColor = vec4(/*(z_value+1.0f)/2.0f*/gl_FragCoord.z
, gl_FragCoord.z, gl_FragCoord.z, 1.0f);\n
//test
//FragColor = vec4(0.0f, 0.0f, (zValue-0.9f)*5.0f, 1.0f);\n
//imageStore(depth_image, ivec2(gl_FragCoord.xy), vec4(FragColor.z, FragColor.z, FragColor.z, FragColor.z));\n
//imageStore(depth_image, ivec2(gl_FragCoord.xy), vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z));\n
}\n
);

const char *sphSimulatorComputer = STRINGIFY(
#version 430\n
layout (local_size_x = 128) in;
layout (rgba32f, binding = 0) uniform imageBuffer position_image;
layout (rgba32f, binding = 1) uniform imageBuffer velocity_image;
layout (r32i, binding = 2) uniform iimage3D grid_image_head;
layout (r32ui, binding = 3) uniform uimage3D grid_image_count;
//unifrom usampler3D scene_voxel_texture; later:collision ditection.
uniform float time_step;
uniform float smooth_length;
uniform ivec3 grid_size;
uniform float denisity_smooth_factor;
uniform float press_factor;
uniform float a_smooth_factor;
uniform float viscosity;
uniform vec3 a_outside;
void main(void)\n
{\n
	int item_index = int(gl_GlobalInvocationID.x);
	vec4 position = imageLoad(position_image, item_index);
	vec4 velocity = imageLoad(velocity_image, item_index);

	/*********1 init grid(positions)*************************/
	vec4 grid_hashf = pos / smooth_length;
	grid_hashf = floor(grid_hashf);
	/*********grid hash********/
	ivec4 grid_hashi = ivec4(int(grid_hashf.x),
		int(grid_hashf.y), int(grid_hashf.z), 0);
	/*********grid hash********/
	int old_head = imageAtomicExchange(grid_image_head,
		grid_hashi.xyz, item_index);
	imageAtomicAdd(grid_image_count, grid_hashi.xyz, 1);
	position.w = intBitsToFloat(next);
	imageStore(position_image, item_index, position);
	memoryBarrier();
	/*********1 init grid(positions)*************************/

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
				if (grid_id.x >= grid_size.x ||
					grid_id.y >= grid_size.y ||
					grid_id.z >= grid_size.z ||
					grid_id.x < 0 ||
					grid_id.y < 0 ||
					grid_id.z < 0 ) 
					continue;
				uint p_count = imageLoad(grid_image_count, grid_id);
				int ptr = imageLoad(grid_image_head, grid_id);
				while(p_count--)
				{
					vec4 position_next = imageLoad(position_image, ptr);
					ptr = floatBitsToInt(position_next.w);
					vec3 r_j = position_next.xyz;
					float distance_ij = length(r_i, r_j);
					if (distance_ij < smooth_length)
					{
						float be_sum_sqrt = smooth_length * smooth_length
							- distance_ij*distance_ij;
						denisity_i += (be_sum_sqrt*be_sum_sqrt);
					}
				}
			}
		}
	}
	denisity_i *= denisity_smooth_factor;
	velocity.w = denisity_i;
	imageStore(velocity_image, item_index, velocity);
	memoryBarrier();
	/************2 denisity(grid)****************************/

	/********3 acceleration(denisity)************************/
	vec3 acceleration_p = vec3(0.0f, 0.0f, 0.0f);
	vec3 acceleration_miu = vec3(0.0f, 0.0f, 0.0f);
	vec3 u_i = velocity.xyz;
	float p_i = press_factor * (denisity_i - 1000.0f);
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				ivec3 grid_id = ivec3(w_i+i, h_j+j, d_k+k);
				if (grid_id.x >= grid_size.x ||
					grid_id.y >= grid_size.y ||
					grid_id.z >= grid_size.z ||
					grid_id.x < 0 ||
					grid_id.y < 0 ||
					grid_id.z < 0 ) 
					continue;
				uint p_count = imageLoad(grid_image_count, grid_id);
				int ptr = imageLoad(grid_image_head, grid_id);
				while(p_count--)
				{
					vec4 position_next = imageLoad(position_image, ptr);
					ptr = floatBitsToInt(position_next.w);
					vec3 r_j = position_next.xyz;
					float distance_ij = length(r_i, r_j);
					if (distance_ij < smooth_length)
					{
						vec4 velocity_denisity = imageLoad(velocity_image, ptr);
						float denisity_j = velocity_denisity.w;
						vec3 u_j = velocity_denisity.xyz;
						float p_j = press_factor * (denisity_j - 1000.0f);
						if (distance_ij > 1e-30)
						{
							acceleration_p += (p_i + p_j)
								*pow(smooth_length - distance_ij, 2.0f)*0.5f
								/(denisity_i*denisity_j*distance_ij)
								*(r_i-r_j);
						}
						acceleration_miu += (smooth_length - distance_ij)
							/(denisity_i*denisity_j)
							*(u_j - u_i);
					}
				}
			}
		}
	}
	acceleration_miu *= (viscosity*a_smooth_factor);
	acceleration_p *= a_smooth_factor;
	vec3 acceleration_i = a_outside + acceleration_p + acceleration_miu;
	/********3 acceleration(denisity)************************/

	/*********4 pos/vel(acceleration)************************/
	vec3 position_new = position + 0.5f * time_step * time_step * acceleration_i;
	vec3 velocity_new = velocity + time_step * acceleration_i;
	/*********4 pos/vel(acceleration)************************/

	/******************5 reset*******************************/
	imageStore(position_image, item_index, vec4(position_new, denisity_i));
	imageStore(velocity_image, item_index, vec4(velocity_new, denisity_i));
	memoryBarrier();
	/******************5 reset*******************************/
}\n
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