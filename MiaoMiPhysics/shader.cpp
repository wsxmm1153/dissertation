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
out vec4 vVaryingColor;\n
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
out vec3 vVaryingTexCoord;\n
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
uniform sampler3D volumeTex;\n
uniform int textureX;\n
uniform int textureY;\n
void main(void)\n
{\n
	//...
	vec2 tex2DCoord = vec2(gl_FragCoord.x/textureX, gl_FragCoord.y/textureY);\n
	vec3 back3DCoord = texture(backTex, tex2DCoord).xyz;\n
	vec3 front3DCoord = vVaryingTexCoord;\n

	vec3 len = back3DCoord - front3DCoord;\n
	float step = length(len)/(1.0/256.0);\n
	vec3 samplerStep = front3DCoord;\n
	vec3 colorSum = vec3(0.0, 0.0, 0.0);\n
	vec4 colorStep;\n
	float aStep = 0.0;\n
	while (step > 0)\n
	{\n
		colorStep= texture(volumeTex, samplerStep);\n
		float ascale = 0.1*colorStep.a;\n
		colorSum = colorSum + (1.0-aStep) * ascale * colorStep.rgb;\n
		//colorSum = colorSum + (1.0-aStep) * colorStep.rgb;\n
		aStep = aStep + (1.0-aStep)*ascale;\n
		if (aStep > 1.0)	break;\n
		samplerStep += normalize(len)*(1.0/256.0);\n
		step -= 1.0;\n
	}\n

	FragColor = vec4(colorSum, 1.0);\n
}\n
);

const char *drawDepthVertex = STRINGIFY(
#version 430\n
	in vec3 vVertex;\n
	uniform mat4 pvm;\n
	smooth out float z_value;\n
	void main(void)\n
{\n
vec4 pp = pvm * vec4(vVertex, 1.0);\n
z_value = pp.z;
gl_Position = pp;\n
}\n
);

const char *drawDepthFragment = STRINGIFY(
#version 430\n
	smooth in float z_value;\n
	//out vec4 FragColor;\n
	layout (r32f) uniform image2D depth_image;
	void main(void)\n
{\n
//p*v*m将z变换到了（-1.0f,1.0f）\n
vec4 FragColor = vec4(0.0, 0.0, (z_value+1.0f+10e-10f)/2.0f, 1.0f);\n
//test
//FragColor = vec4(0.0f, 0.0f, (zValue-0.9f)*5.0f, 1.0f);\n
imageStore(depth_image, ivec2(gl_FragCoord.xy), vec4(FragColor.z, FragColor.z, FragColor.z, FragColor.z));\n
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