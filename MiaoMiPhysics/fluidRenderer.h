#ifndef FLUID_RENDERER_H_
#define FLUID_RENDERER_H_

#include "glincludes.h"
class FluidRenderer
{
public:
	FluidRenderer();
	FluidRenderer(GLuint pos_vbo, GLint* particle_number);
	~FluidRenderer();
	void DrawFluids();
	void DrawVolume();
	void InitPointDraw();
	void DrawPoints(glm::mat4* model_view, glm::mat4* projection);
protected:
private:
	GLuint positons_vbo_;
	GLint* particle_number_;
	GLuint point_program_;
};

#endif