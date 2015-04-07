////how to use it
//#include "shaderSrc.h"
// 
//extern const char *shaderName = STRINGIFY(
//	shader source(string)
//	);
//compileProgram(vertexSrc, fragmentSrc, geometrySrc);

#ifndef SHADER_SRC
#define SHADER_SRC

#define STRINGIFY(A) #A

extern const char* commonVertex;
extern const char* commonFragment;
extern const char* backFaceTexcoordsVertex;
extern const char* backFaceTexcoordsFragment;
extern const char* rayCastingVertex;
extern const char* rayCastingFragment;
extern const char* drawDepthVertex;
extern const char* drawDepthFragment;
extern const char* sphSimulatorComputer;
extern const char* pointSpriteVertexShader;
extern const char* pointSpriteFragmentShader;

#include <GL/glew.h>
#include <stdio.h>

GLuint compileProgram(const char* vertexSrc, const char* fragmentSrc);

GLuint compileComputer(const char* computeShaderStr);

#endif
