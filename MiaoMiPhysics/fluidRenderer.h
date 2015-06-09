#ifndef FLUID_RENDERER_H_
#define FLUID_RENDERER_H_

#include <string>
#include "util.h"
#include "glincludes.h"

class FluidRenderer
{
public:
	FluidRenderer();
	FluidRenderer(GLint* w_p, GLint* h_p,
		GLuint pos_vbo, GLint* particle_number);
	~FluidRenderer();
	void DrawFluids();
	void DrawVolume();
	void InitPointDraw();
	void DrawPoints(glm::mat4* model_view, glm::mat4* projection);
	void InitScreenSpaceDraw();
	void DrawScreenSpace(glm::mat4* model_view, glm::mat4* projection);
	void InitScene(const char* file_name);
	void DrawScene(glm::mat4* model_view, glm::mat4* projection);
	void OutPut();
	glm::vec3 scene_min_;
	glm::vec3 scene_size_;
protected:
private:
	GLuint positons_vbo_;
	GLint* particle_number_;
	GLuint point_program_;
	GLuint depth_program_;
	GLuint thick_program_;
	GLuint screenspace_program_;
	GLint* screen_width_;
	GLint* screen_height_;
	GLuint depth_fbo_;
	GLuint thick_fbo_;
	GLuint s_depth_fbo_;
	GLuint scene_fbo_;
	GLuint fluid_fbo_;
	GLuint depth_texture_;
	GLuint thick_texture_;
	GLuint s_depth_texture_;
	GLuint scene_texture_;
	GLuint fluid_texture_;
	GLuint depth_rbos_[5];
	GLuint scene_p_vbo_;
	GLuint scene_n_vbo_;
	GLuint scene_t_vbo_;
	GLuint phone_program_;
	GLuint output_program_;
	GLint scene_v_size_;
	GLuint easy_vao_;
};

#endif