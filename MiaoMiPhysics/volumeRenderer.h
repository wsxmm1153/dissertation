#ifndef _VOLUME_RENDERER_
#define _VOLUME_RENDERER_

#include "glincludes.h"

class VolumeRenderer
{
public:
	//VolumeRenderer();
	~VolumeRenderer();
	enum Modes
	{
		RayCasting
	};
	static void drawPhysicsWorld(GLuint texture, glm::mat4* mv, glm::mat4* p);
	static VolumeRenderer* createVolumeRenderer(GLuint texture, glm::mat4* mv, glm::mat4* p, Modes mode);
	static VolumeRenderer* createVolumeRenderer();
	static VolumeRenderer* getInstance();

	void setScreen(GLuint width, GLuint height){_screen_width = width, _screen_height = height;}
	void set3DTexture(GLuint texture){_texture_3d = texture;}
	void setMvpMatrix(glm::mat4* mv, glm::mat4* p){_mv_matrix = mv;_project_matrix = p;}
	void draw();

	void setMode(Modes mode){_mode = mode;}

	//void setTransformFunc();
private:
	GLuint _texture_3d;
	GLuint _back_face_tex;
	glm::mat4* _mv_matrix;
	glm::mat4* _project_matrix;
	GLuint _back_program;
	GLuint _draw_program;
	GLuint _screen_width;
	GLuint _screen_height;
	Modes _mode;
	GLuint _box_vao;
	GLuint _back_vao;
	GLuint _fbo;
	static VolumeRenderer* _self;
	//about transform function..
	void _init_programs();
	void _init_buffers();
	void _draw_back_face_tex();
	void _draw_volume();
	VolumeRenderer();
};

#endif