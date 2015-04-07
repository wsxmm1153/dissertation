#include "sphsimulator.h"

SPHParticles::SPHParticles()
	:particle_number_(0),
	positions_vbo_(0),
	positions_tbo_(0),
	velocitys_vbo_(0),
	velocitys_tbo_(0)
{

}

SPHParticles::~SPHParticles()
{
	if (positions_tbo_)
		glDeleteTextures(1, &positions_tbo_);
	if (velocitys_tbo_)
		glDeleteTextures(1, &velocitys_tbo_);
	if (positions_vbo_)
		glDeleteBuffers(1, &positions_vbo_);
	if (velocitys_vbo_)
		glDeleteBuffers(1, &velocitys_vbo_);
}

void SPHParticles::InitGPUResource(int particle_number)
{
	particle_number_ = particle_number;
	glGenBuffers(1, &positions_vbo_);
	glGenBuffers(1, &velocitys_vbo_);
	glGenTextures(1, &positions_tbo_);
	glGenTextures(1, &velocitys_tbo_);

	glBindBuffer(GL_ARRAY_BUFFER, positions_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*particle_number, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, velocitys_vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*particle_number, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_BUFFER, positions_tbo_);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, positions_vbo_);
	glBindTexture(GL_TEXTURE_BUFFER, velocitys_tbo_);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocitys_vbo_);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

SimulateDrawGrid::SimulateDrawGrid()
	:grid_x_(0),
	grid_y_(0),
	grid_z_(0),
	grid_head_tbo_(0),
	grid_count_tbo_(0),
	count_buffer_(0),
	head_buffer_(0)
{

}

SimulateDrawGrid::~SimulateDrawGrid()
{
	if (grid_head_tbo_)
		glDeleteTextures(1, &grid_head_tbo_);
	if (grid_count_tbo_)
		glDeleteTextures(1, &grid_head_tbo_);
	if (count_buffer_)
		glDeleteBuffers(1, &count_buffer_);
	if (head_buffer_)
		glDeleteBuffers(1, &head_buffer_);
}

void SimulateDrawGrid::InitGPUResource(const glm::ivec3 size_3d)
{
	grid_x_ = size_3d.x;
	grid_y_ = size_3d.y;
	grid_z_ = size_3d.z;

	//glGenBuffers(1, &grid_pbo_);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, grid_pbo_);
	//glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(GLint)*grid_x_*grid_y_*grid_z_, 
	//	0, GL_STATIC_DRAW);
	//GLint* data_ptr = (GLint*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
	//memset(data_ptr, 0, sizeof(GLint)*grid_x_*grid_y_*grid_z_);
	//glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, grid_pbo_);
	//glGenTextures(1, &grid_head_tbo);
	//glActiveTexture(GL_TEXTURE3);
	//glBindTexture(GL_TEXTURE_3D, grid_head_tbo);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	////glTexStorage3D(GL_TEXTURE_3D, 0, GL_R32I, grid_x_, grid_y_, grid_z_);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32I, grid_x_, grid_y_, grid_z_, 0,
	//	GL_RED_INTEGER, GL_INT, 0);
	//glBindTexture(GL_TEXTURE_3D, 0);

	//glGenTextures(1, &grid_count_tbo);
	//glActiveTexture(GL_TEXTURE4);
	//glBindTexture(GL_TEXTURE_3D, grid_count_tbo);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	////glTexStorage3D(GL_TEXTURE_3D, 0, GL_R32I, grid_x_, grid_y_, grid_z_);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R32I, grid_x_, grid_y_, grid_z_, 0,
	//	GL_RED_INTEGER, GL_INT, 0);
	//glBindTexture(GL_TEXTURE_3D, 0);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	glGenBuffers(1, &count_buffer_);
	glGenBuffers(1, &head_buffer_);
	glGenTextures(1, &grid_count_tbo_);
	glGenTextures(1, &grid_head_tbo_);

	glBindBuffer(GL_ARRAY_BUFFER, count_buffer_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLint)*grid_x_*grid_y_*grid_z_, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, head_buffer_);
	glBufferData(GL_ARRAY_BUFFER,sizeof(GLint)*grid_x_*grid_y_*grid_z_, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_BUFFER, grid_count_tbo_);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, count_buffer_);
	glBindTexture(GL_TEXTURE_BUFFER, grid_head_tbo_);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, head_buffer_);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

SPHSimulator::SPHSimulator()
	:particle_number_(0),
	size_x_(0.0f),
	size_y_(0.0f),
	size_z_(0.0f),
	smooth_length_(0.0f),
	gpu_particles_ptr_(0),
	gpu_grid_ptr_(0),
	simulator_program_(0),
	scene_voxel_data_tbo_(0)
{

}

SPHSimulator::~SPHSimulator()
{
	if (simulator_program_)
		glDeleteProgram(simulator_program_);
	if (gpu_grid_ptr_)
		delete gpu_grid_ptr_;
	if (gpu_particles_ptr_)
		delete gpu_particles_ptr_;
}

void SPHSimulator::InitGPUResource(const int particle_number, 
	const glm::vec3 scene_size, const float smooth_length)
{
	particle_number_ = particle_number;
	size_x_ = scene_size.x;
	size_y_ = scene_size.y;
	size_z_ = scene_size.z;
	smooth_length_ = smooth_length;
	gpu_particles_ptr_ = new SPHParticles();
	gpu_grid_ptr_ = new SimulateDrawGrid();
	gpu_particles_ptr_->InitGPUResource(particle_number);
	int grid_x, grid_y, grid_z;
	grid_x = (int)(size_x_/smooth_length_) + 1;
	grid_y = (int)(size_y_/smooth_length_) + 1;
	grid_z = (int)(size_z_/smooth_length_) + 1;

	size_x_ = (float)grid_x * smooth_length_;
	size_y_ = (float)grid_y * smooth_length_;
	size_z_ = (float)grid_z * smooth_length_;

	gpu_grid_ptr_->InitGPUResource(glm::ivec3(grid_x,
		grid_y, grid_z));
}

void SPHSimulator::InitSimulation()
{
	simulator_program_ = compileComputer(sphSimulatorComputer);

	int x_ = gpu_grid_ptr_->grid_x_;
	int y_ = gpu_grid_ptr_->grid_y_;
	int z_ = gpu_grid_ptr_->grid_z_;
	float xx = 0.0f;
	float yy = 0.9f;
	float zz = 0.0f;
	GLfloat* init_float_p = new GLfloat[x_*y_*z_*4];
	GLfloat* init_float_v = new GLfloat[x_*y_*z_*4];
	for(int i = 0; i < x_*y_*z_; i++)
	{
		init_float_v[i*4+0] = 0.0f;
		init_float_v[i*4+1] = 0.0f;
		init_float_v[i*4+2] = 0.0f;
		init_float_v[i*4+3] = 0.0f;

		init_float_p[i*4+0] = xx;
		init_float_p[i*4+1] = yy;
		init_float_p[i*4+2] = zz;
		init_float_p[i*4+3] = 0.0f;

		xx += INIT_DISTANCE;
		if (xx > BOUNDARY)
		{
			xx = 0.0f;
			zz += INIT_DISTANCE;
		}
		if (zz > BOUNDARY)
		{
			zz = 0.0f;
			yy -= INIT_DISTANCE;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->positions_vbo_);
	glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*4*sizeof(GLfloat),
		init_float_p, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->velocitys_vbo_);
	glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*4*sizeof(GLfloat),
		init_float_v, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete init_float_v;
	delete init_float_p;
}

void SPHSimulator::display(float time_step)
{
	//reset grid count
	int x_ = gpu_grid_ptr_->grid_x_;
	int y_ = gpu_grid_ptr_->grid_y_;
	int z_ = gpu_grid_ptr_->grid_z_;

	GLint* init_count = new GLint[x_*y_*z_];
	memset(init_count, 0, sizeof(GLint)*x_*y_*z_);
	GLint* init_head = new GLint[x_*y_*z_];
	memset(init_head, 0xffff, sizeof(GLint)*x_*y_*z_);

	glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->count_buffer_);
	glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*sizeof(GLint),
		init_count, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->head_buffer_);
	glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*sizeof(GLint),
		init_head, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete init_head;
	delete init_count;

	glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	glUseProgram(simulator_program_);
	
	glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(0, gpu_grid_ptr_->grid_head_tbo_,
		0, GL_TRUE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(1, gpu_grid_ptr_->grid_count_tbo_,
		0, GL_TRUE, 0, GL_READ_WRITE, GL_R32I);

	GLint uniform_loc;

	uniform_loc = glGetUniformLocation(simulator_program_, "time_step");
	glUniform1f(uniform_loc, time_step);

	uniform_loc = glGetUniformLocation(simulator_program_, "smooth_length");
	glUniform1f(uniform_loc, SMOOTH_LENGTH);

	uniform_loc = glGetUniformLocation(simulator_program_, "grid_size");
	glUniform3i(uniform_loc,
		gpu_grid_ptr_->grid_x_,
		gpu_grid_ptr_->grid_y_,
		gpu_grid_ptr_->grid_z_);

	float factors = PARTICLE_MASS * 315.0f / (64.0f * M_PI * pow(SMOOTH_LENGTH, 9.0f));
	uniform_loc = glGetUniformLocation(simulator_program_, "denisity_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = PRESSURE_CONSTANT;
	uniform_loc = glGetUniformLocation(simulator_program_, "press_factor");
	glUniform1f(uniform_loc, factors);

	factors = PARTICLE_MASS * 45.0f / (M_PI * pow(SMOOTH_LENGTH, 6.0f));
	uniform_loc = glGetUniformLocation(simulator_program_, "a_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = MOTION_DAMPING;
	uniform_loc = glGetUniformLocation(simulator_program_, "viscosity");
	glUniform1f(uniform_loc, factors);

	uniform_loc = glGetUniformLocation(simulator_program_, "a_outside");
	glUniform3f(uniform_loc, 0.0f, -10.0f, 0.0f);
	glFinish();
	glDispatchCompute(NUM, 1, 1);
	
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	glFinish();
}