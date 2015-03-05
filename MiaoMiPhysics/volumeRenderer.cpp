#include "volumeRenderer.h"
VolumeRenderer* VolumeRenderer::_self = NULL;

VolumeRenderer::VolumeRenderer():
	_texture_3d(0),
	_back_face_tex(0),
	_back_program(0),
	_draw_program(0),
	_box_vao(0),
	_back_vao(0),
	_screen_width(SCREENWIDTH),
	_screen_height(SCREENHEIGHT),
	_mode(VolumeRenderer::RayCasting),
	_mv_matrix(NULL),
	_project_matrix(NULL)
{

}

VolumeRenderer::~VolumeRenderer()
{
	if (_back_face_tex)
	{
		glDeleteTextures(1, &_back_face_tex);
	}
	if (_back_program)
	{
		glDeleteProgram(_back_program);
	}
	if (_draw_program)
	{
		glDeleteProgram(_draw_program);
	}
	if (_back_vao)
	{
		glDeleteVertexArrays(1, &_back_vao);
	}
	if (_box_vao)
	{
		glDeleteVertexArrays(1, &_box_vao);
	}
	if (_self)
	{
		delete _self;
	}
}

void VolumeRenderer::drawPhysicsWorld(GLuint texture, glm::mat4* mv, glm::mat4* p)
{
	//auto renderer = createVolumeRenderer(texture, mv, p, RayCasting);
	if (_self)
	{
		_self->draw();
	}
	else
	{
		_self = createVolumeRenderer(texture, mv, p, RayCasting);
		_self->draw();
	}
}
//This is a single instance.Use it as physics test.
//Given a 3D texture,use this function to draw it out in the middle of the screen.

VolumeRenderer* VolumeRenderer::createVolumeRenderer(GLuint texture, glm::mat4 *mv, glm::mat4 *p, Modes mode)
{
	auto renderer = createVolumeRenderer();
	renderer->_texture_3d = texture;
	renderer->_mv_matrix = mv;
	renderer->_project_matrix = p;
	renderer->_mode = mode;

	return renderer;
}

VolumeRenderer* VolumeRenderer::createVolumeRenderer()
{
	auto renderer = new VolumeRenderer();

	return renderer;
}


void VolumeRenderer::draw()
{
	switch (_mode)
	{
	case RayCasting:
	default:
		//ray casting
		_init_programs();
		_init_buffers();
		_draw_back_face_tex();
		_draw_volume();
		break;
	}
}

void VolumeRenderer::_init_programs()
{
	switch (_mode)
	{
	case RayCasting:
	default:
		//ray casting
		if (_back_program)
		{
			return;
		}
		_back_program = compileProgram(backFaceTexcoordsVertex, backFaceTexcoordsFragment);
		_draw_program = compileProgram(rayCastingVertex, rayCastingFragment);
		break;
	}
}

void VolumeRenderer::_init_buffers()
{
	if (_back_face_tex)
	{
		return;
	}
	const GLfloat vertice[] ={
		-1.0f, 1.0f*YMAX/XMAX, -1.0f*ZMAX/XMAX,
		1.0f, 1.0f*YMAX/XMAX, -1.0f*ZMAX/XMAX,
		1.0f, 1.0f*YMAX/XMAX, 1.0f*ZMAX/XMAX,
		-1.0f, 1.0f*YMAX/XMAX, 1.0f*ZMAX/XMAX,
		-1.0f, -1.0f*YMAX/XMAX, -1.0f*ZMAX/XMAX,
		1.0f, -1.0f*YMAX/XMAX, -1.0f*ZMAX/XMAX,
		1.0f, -1.0f*YMAX/XMAX, 1.0f*ZMAX/XMAX,
		-1.0f, -1.0f*YMAX/XMAX, 1.0f*ZMAX/XMAX
	};

	const GLfloat texcoord[] = {
		0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	//const GLubyte index[] = 
	//{
	//	3, 2, 6, 7,
	//	1, 5, 6, 2,
	//	6, 5, 4, 7,
	//	5, 1, 0, 4,
	//	0, 3, 7, 4,
	//	0, 1, 2, 3
	//};

	const GLubyte index[] = 
	{
		3, 2, 6,
		3, 6, 7,
		1, 5, 6,
		1, 6, 2,
		6, 5, 4,
		6, 4, 7,
		5, 1, 0,
		5, 0, 4,
		0, 3, 7, 
		0, 7, 4,
		0, 1, 2,
		0, 2, 3
	};

	GLuint verticeBuffer;
	GLuint indexBuffer;
	GLuint texcoordBuffer;

	//temp texture vao
	glGenVertexArrays(1, &_back_vao);
	glBindVertexArray(_back_vao);

	glGenBuffers(1, &verticeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, verticeBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertice), vertice, GL_STATIC_DRAW);
	GLuint bvLoc = glGetAttribLocation(_back_program, "vVertex");
	glEnableVertexAttribArray(bvLoc);
	glVertexAttribPointer(bvLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &texcoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoord), texcoord, GL_STATIC_DRAW);
	GLuint tvLoc = glGetAttribLocation(_back_program, "vTexcoord");
	glEnableVertexAttribArray(tvLoc);
	glVertexAttribPointer(tvLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index), index, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//frame buffer for creating texture
	glGenTextures(1, &_back_face_tex);
	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
	glBindTexture(GL_TEXTURE_2D, _back_face_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _back_face_tex, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei)_screen_width, (GLsizei)_screen_height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//vao for screen
	glGenVertexArrays(1, &_box_vao);
	glBindVertexArray(_box_vao);
	glBindBuffer(GL_ARRAY_BUFFER, verticeBuffer);
	bvLoc = glGetAttribLocation(_draw_program, "vVertex");
	glEnableVertexAttribArray(bvLoc);
	glVertexAttribPointer(bvLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
	tvLoc = glGetAttribLocation(_draw_program, "vTexcoord");
	glEnableVertexAttribArray(tvLoc);
	glVertexAttribPointer(tvLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void VolumeRenderer::_draw_back_face_tex()
{
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glUseProgram(_back_program);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	GLenum fboBuffers[] = {
		GL_COLOR_ATTACHMENT0
	};

	glDrawBuffers(1, fboBuffers);	

	glBindVertexArray(_back_vao);

	GLuint pLocation = glGetUniformLocation(_back_program, "p");
	GLuint mvLocation = glGetUniformLocation(_back_program, "mv");
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, glm::value_ptr(*_project_matrix));
	glUniformMatrix4fv(mvLocation, 1, GL_FALSE, glm::value_ptr(*_mv_matrix));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glFinish();
}

void VolumeRenderer::_draw_volume()
{
	//test back texture
	//**************************************
	/*glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);

	glClearColor(0.9f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _back_face_tex);
	glBegin(GL_QUADS);
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.1f, 0.1f);

		glTexCoord2f(1.0, 0.0f);
		glVertex2f(0.9f, 0.1f);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(0.9f, 0.9f);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0.1f, 0.9f);
	}
	glEnd();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);*/
	//**************************************
	//test end

	//draw volume
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(_draw_program);

	glEnable(GL_TEXTURE_2D);
	if (_back_face_tex)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _back_face_tex);
		GLuint texLoc = glGetUniformLocation(_draw_program, "backTex");
		glUniform1i(texLoc, 0);
	}

	glEnable(GL_TEXTURE_3D);
	if (_texture_3d)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, _texture_3d);
		GLuint tex3DLoc = glGetUniformLocation(_draw_program, "volumeTex");
		glUniform1i(tex3DLoc, 1);
	}

	glBindVertexArray(_back_vao);

	GLuint pLocation = glGetUniformLocation(_draw_program, "p");
	GLuint mvLocation = glGetUniformLocation(_draw_program, "mv");
	glUniformMatrix4fv(pLocation, 1, GL_FALSE, glm::value_ptr(*_project_matrix));
	glUniformMatrix4fv(mvLocation, 1, GL_FALSE, glm::value_ptr(*_mv_matrix));
	glUniform1i(glGetUniformLocation(_draw_program, "textureX"), _screen_width);
	glUniform1i(glGetUniformLocation(_draw_program, "textureY"), _screen_height);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glFinish();
}