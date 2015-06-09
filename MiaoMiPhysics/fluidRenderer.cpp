#include "fluidRenderer.h"
#include <limits>
FluidRenderer::FluidRenderer()
	:positons_vbo_(0),
	particle_number_(0)
{
}

FluidRenderer::FluidRenderer(GLint* w_p, GLint* h_p,
	GLuint pos_vbo, GLint* particle_number)
{
	screen_width_ = w_p;
	screen_height_ = h_p;
	positons_vbo_ = pos_vbo;
	particle_number_ = particle_number;
}

void FluidRenderer::InitPointDraw()
{
	point_program_ = compileProgram(pointSpriteVertexShader,
		pointSpriteFragmentShader);
}

void FluidRenderer::InitScreenSpaceDraw()
{
	depth_program_ = compileProgram(pointSpriteVertexShader, depthTextureFragmentShader);
	thick_program_ = compileProgram(pointSpriteVertexShader, thickTextureFragmentShader);
	screenspace_program_ = compileProgram(pointSpriteVertexShader, screenSpaceFragmentShader);
	output_program_ = compileProgram(OutPutVertex, OutPutFragment);
	GLfloat vv[] = {
		0.0f, 0.0f,
		SCREENWIDTH, 0.0f,
		0.0f, SCREENHEIGHT,
		0.0f, SCREENHEIGHT,
		SCREENWIDTH, 0.0f,
		SCREENWIDTH, SCREENHEIGHT
	};

	GLfloat tt[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	GLuint vbos[2];
	glGenBuffers(2, vbos);
	glGenVertexArrays(1, &easy_vao_);
	glBindVertexArray(easy_vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vv), vv, GL_STATIC_DRAW);
	GLint loc0 = glGetAttribLocation(output_program_, "vVertex");
	glEnableVertexAttribArray(loc0);
	glVertexAttribPointer(loc0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tt), tt, GL_STATIC_DRAW);
	loc0 = glGetAttribLocation(output_program_, "vTexCoords");
	glEnableVertexAttribArray(loc0);
	glVertexAttribPointer(loc0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_POINT_SPRITE);
	glEnable(GL_PROGRAM_POINT_SIZE);
	
	glGenRenderbuffers(4, depth_rbos_);
	glGenTextures(1, &depth_texture_);
	glGenFramebuffers(1, &depth_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_fbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, depth_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_texture_, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbos_[0]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenTextures(1, &thick_texture_);
	glGenFramebuffers(1, &thick_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, thick_fbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, thick_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, thick_texture_, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbos_[1]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenTextures(1, &s_depth_texture_);
	glGenFramebuffers(1, &s_depth_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_depth_fbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, s_depth_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_depth_texture_, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbos_[2]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenTextures(1, &scene_texture_);
	glGenFramebuffers(1, &scene_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scene_fbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[3]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, scene_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_texture_, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbos_[3]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenTextures(1, &fluid_texture_);
	glGenFramebuffers(1, &fluid_fbo_);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fluid_fbo_);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[4]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, fluid_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fluid_texture_, 0);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rbos_[4]);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FluidRenderer::DrawPoints(glm::mat4* model_view, glm::mat4* projection)
{
	glClearColor(0.0, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_DEPTH_TEST);
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

void FluidRenderer::DrawScreenSpace(glm::mat4* model_view, glm::mat4* projection)
{
	GLint v_loc;
	GLfloat radius = PARTICLE_RADIUS;
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[4]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);

	glBindTexture(GL_TEXTURE_2D, depth_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, thick_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, fluid_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT);

	//depth texture
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depth_fbo_);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(depth_program_);
	glUniform1f(glGetUniformLocation(depth_program_, "pointScale"),
		SCREENHEIGHT / tanf(60.0f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(depth_program_, "pointRadius"),
		radius);
	glUniformMatrix4fv(glGetUniformLocation(depth_program_, "modelview"),
		1, GL_FALSE, glm::value_ptr(*model_view));
	glUniformMatrix4fv(glGetUniformLocation(depth_program_, "projection"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glUniformMatrix4fv(glGetUniformLocation(depth_program_, "projection_mat"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glBindBuffer(GL_ARRAY_BUFFER, positons_vbo_);
	v_loc = glGetAttribLocation(depth_program_, "vVertex");
	glEnableVertexAttribArray(v_loc);
	glVertexAttribPointer(v_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_POINTS, 0, *particle_number_);

	//draw thick
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, thick_fbo_);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glUseProgram(thick_program_);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glUniform1f(glGetUniformLocation(thick_program_, "pointScale"),
		SCREENHEIGHT / tanf(60.0f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(thick_program_, "pointRadius"),
		radius);
	glUniformMatrix4fv(glGetUniformLocation(thick_program_, "modelview"),
		1, GL_FALSE, glm::value_ptr(*model_view));
	glUniformMatrix4fv(glGetUniformLocation(thick_program_, "projection"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glUniformMatrix4fv(glGetUniformLocation(thick_program_, "projection_mat"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glUniform1i(glGetUniformLocation(thick_program_, "textureX"), SCREENWIDTH);
	glUniform1i(glGetUniformLocation(thick_program_, "textureY"), SCREENHEIGHT);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, s_depth_texture_);
	glUniform1i(glGetUniformLocation(thick_program_, "scene_depth"), 2);

	glBindBuffer(GL_ARRAY_BUFFER, positons_vbo_);
	v_loc = glGetAttribLocation(thick_program_, "vVertex");
	glEnableVertexAttribArray(v_loc);
	glVertexAttribPointer(v_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, *particle_number_);


	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fluid_fbo_);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(screenspace_program_);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_DEPTH_TEST);
	glUniform1f(glGetUniformLocation(screenspace_program_, "pointScale"),
		SCREENHEIGHT / tanf(60.0f*0.5f*(float)M_PI/180.0f));
	glUniform1f(glGetUniformLocation(screenspace_program_, "pointRadius"),
		radius);
	glUniformMatrix4fv(glGetUniformLocation(screenspace_program_, "modelview"),
		1, GL_FALSE, glm::value_ptr(*model_view));
	glUniformMatrix4fv(glGetUniformLocation(screenspace_program_, "projection"),
		1, GL_FALSE, glm::value_ptr(*projection));
	glm::mat4 pro_trans = glm::transpose(*projection);
	glUniformMatrix4fv(glGetUniformLocation(screenspace_program_, "projection_trans"),
		1, GL_FALSE, glm::value_ptr(pro_trans));
	glUniform1i(glGetUniformLocation(screenspace_program_, "textureX"), SCREENWIDTH);
	glUniform1i(glGetUniformLocation(screenspace_program_, "textureY"), SCREENHEIGHT);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depth_texture_);
	glUniform1i(glGetUniformLocation(screenspace_program_, "texture"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, thick_texture_);
	glUniform1i(glGetUniformLocation(screenspace_program_, "texture_thick"), 1);

	GLfloat g[49] = {
		0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067,
		0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292,
		0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117,
		0.00038771, 0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373, 0.00038771,
		0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117,
		0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292,
		0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067
	};

	glUniform1fv(glGetUniformLocation(screenspace_program_, "gaosi"), 49, g);
	glBindBuffer(GL_ARRAY_BUFFER, positons_vbo_);
	v_loc = glGetAttribLocation(screenspace_program_, "vVertex");
	glEnableVertexAttribArray(v_loc);
	glVertexAttribPointer(v_loc, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	glDrawArrays(GL_POINTS, 0, *particle_number_);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void FluidRenderer::InitScene(const char* file_name)
{
	Util ut;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texcoords;
	std::vector<glm::vec3> normals;
	ut.loadOBJ(file_name, vertices, texcoords, normals);
	scene_v_size_ = vertices.size();
	phone_program_ = compileProgram(phongVertex, phongFragment);
	glGenBuffers(1, &scene_p_vbo_);
	glGenBuffers(1, &scene_n_vbo_);
	glGenBuffers(1, &scene_t_vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, scene_p_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * scene_v_size_ * 3,
		&vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, scene_n_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * scene_v_size_,
		&normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, scene_t_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * scene_v_size_,
		&normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	float X_max = -numeric_limits<float>::max(),
		X_min = numeric_limits<float>::max(),
		Y_max = -numeric_limits<float>::max(),
		Y_min = numeric_limits<float>::max(),
		Z_max = -numeric_limits<float>::max(), 
		Z_min = numeric_limits<float>::max();

	std::vector<glm::vec3>::iterator it = vertices.begin();
	while(it != vertices.end())
	{
		glm::vec3 vertice = *it;
		if (X_max < vertice.x)	X_max = vertice.x;
		if (Y_max < vertice.y)	Y_max = vertice.y;
		if (Z_max < vertice.z)	Z_max = vertice.z;
		if (X_min > vertice.x)	X_min = vertice.x;
		if (Y_min > vertice.y)	Y_min = vertice.y;
		if (Z_min > vertice.z)	Z_min = vertice.z;
		it ++;
	}
	scene_min_.x = X_min;
	scene_min_.y = Y_min;
	scene_min_.z = Z_min;
	scene_size_ = glm::vec3(X_max-X_min, Y_max-Y_min, Z_max-Z_min);
}

void FluidRenderer::DrawScene(glm::mat4* model_view, glm::mat4* projection)
{
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbos_[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREENWIDTH, SCREENHEIGHT);
	glBindTexture(GL_TEXTURE_2D, s_depth_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCREENWIDTH, SCREENHEIGHT
		, 0, GL_RGBA, GL_FLOAT, 0);
	//draw scene depth
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_depth_fbo_);
	glUseProgram(phone_program_);
	glClearColor(0.0f, 0.0f, 0.0f, 100.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glBindBuffer(GL_ARRAY_BUFFER, scene_p_vbo_);
	GLuint loc = glGetAttribLocation(phone_program_, "vVertex");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_n_vbo_);
	loc = glGetAttribLocation(phone_program_, "vNormal");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_t_vbo_);
	loc = glGetAttribLocation(phone_program_, "vTexCoords");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	loc = glGetUniformLocation(phone_program_, "mv");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*model_view));
	loc = glGetUniformLocation(phone_program_, "p");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*projection));
	loc = glGetUniformLocation(phone_program_, "diffuseColor");
	glUniform4f(loc, 0.0f, 0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, scene_v_size_);
	glUseProgram(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//GLint loc;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scene_fbo_);
	glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(phone_program_);
	glBindBuffer(GL_ARRAY_BUFFER, scene_p_vbo_);
	loc = glGetAttribLocation(phone_program_, "vVertex");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_n_vbo_);
	loc = glGetAttribLocation(phone_program_, "vNormal");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, scene_t_vbo_);
	loc = glGetAttribLocation(phone_program_, "vTexCoords");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	loc = glGetUniformLocation(phone_program_, "mv");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*model_view));
	loc = glGetUniformLocation(phone_program_, "p");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*projection));
	loc = glGetUniformLocation(phone_program_, "diffuseColor");
	glUniform4f(loc, 0.5f, 0.3f, 0.7f, 1.0f);
	glDrawArrays(GL_TRIANGLES, 0, scene_v_size_);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FluidRenderer::OutPut()
{
	glUseProgram(output_program_);
	glBindVertexArray(easy_vao_);
	GLint loc = glGetUniformLocation(output_program_, "mv");
	glm::mat4 m_V = glm::mat4(1.0f);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_V));
	loc = glGetUniformLocation(output_program_, "p");
	m_V = glm::ortho(0.0f, (float)(SCREENWIDTH), 0.0f, (float)(SCREENHEIGHT));
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m_V));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, scene_texture_);
	glUniform1i(glGetUniformLocation(output_program_, "scene_image"), 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, fluid_texture_);
	glUniform1i(glGetUniformLocation(output_program_, "fluid_image"), 4);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glUseProgram(0);
}