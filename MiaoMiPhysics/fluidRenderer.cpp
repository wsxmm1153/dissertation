#include "fluidRenderer.h"

FluidRenderer::FluidRenderer()
	:positons_vbo_(0),
	particle_number_(0)
{
}

FluidRenderer::FluidRenderer(GLuint pos_vbo, GLint* particle_number)
{
	positons_vbo_ = pos_vbo;
	particle_number_ = particle_number;
}

void FluidRenderer::InitPointDraw()
{
	point_program_ = compileProgram(pointSpriteVertexShader,
		pointSpriteFragmentShader);
}

void FluidRenderer::DrawPoints(glm::mat4* model_view, glm::mat4* projection)
{
	glClearColor(0.0, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER);
	glEnable(GL_POINT_SPRITE);
	glUseProgram(point_program_);
	glUniform1f(glGetUniformLocation(point_program_, "pointScale"),
		SCREENHEIGHT / tanf(60.0f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(point_program_, "pointRadius"),
		0.025f);
	glUniformMatrix4fv(glGetUniformLocation(point_program_, "modelview"),
		1, GL_FALSE, glm::value_ptr(*model_view));
	glUniformMatrix4fv(glGetUniformLocation(point_program_, "projection"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glBindBuffer(GL_ARRAY_BUFFER, positons_vbo_);
	GLint v_loc = glGetAttribLocation(point_program_, "vVertex");
	glEnableVertexAttribArray(v_loc);
	glVertexAttribPointer(v_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	/*****************/
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, *particle_number_);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
	glDisable(GL_POINT_SPRITE);
}