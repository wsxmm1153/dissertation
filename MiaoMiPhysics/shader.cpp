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