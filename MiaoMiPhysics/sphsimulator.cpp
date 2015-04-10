#include "sphsimulator.h"
#include "voxelization.h"
SPHParticles::SPHParticles()
	:particle_number_(0),
	buffer_exchange_(0)
{
	positions_tbo_[0] = positions_tbo_[1] = 0;
	velocitys_tbo_[0] = velocitys_tbo_[1] = 0;
	positions_vbo_[0] = positions_vbo_[1] = 0;
	velocitys_vbo_[0] = velocitys_vbo_[1] = 0;
}

SPHParticles::~SPHParticles()
{
	for(int i = 0; i < 2; i++)
	{
		if (positions_tbo_[i])
			glDeleteTextures(1, &positions_tbo_[i]);
		if (velocitys_tbo_[i])
			glDeleteTextures(1, &velocitys_tbo_[i]);
		if (positions_vbo_[i])
			glDeleteBuffers(1, &positions_vbo_[i]);
		if (velocitys_vbo_[i])
			glDeleteBuffers(1, &velocitys_vbo_[i]);
	}

}

void SPHParticles::InitGPUResource(int particle_number)
{
	particle_number_ = particle_number;
	glGenBuffers(2, positions_vbo_);
	glGenBuffers(2, velocitys_vbo_);
	glGenTextures(2, positions_tbo_);
	glGenTextures(2, velocitys_tbo_);
	for (int i = 0; i < 2; i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, positions_vbo_[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*particle_number,
			0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, velocitys_vbo_[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*particle_number,
			0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindTexture(GL_TEXTURE_BUFFER, positions_tbo_[i]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, positions_vbo_[i]);
		glBindTexture(GL_TEXTURE_BUFFER, velocitys_tbo_[i]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocitys_vbo_[i]);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
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
	denisity_program_(0),
	acceleration_program_(0),
	grid_program_(0),
	collision_program_(0),
	scene_voxel_data_tbo_(0),
	buffer_in_(0)
{

}

SPHSimulator::~SPHSimulator()
{
	if (simulator_program_)
		glDeleteProgram(simulator_program_);
	if (grid_program_)
		glDeleteProgram(grid_program_);
	if (acceleration_program_)
		glDeleteProgram(acceleration_program_);
	if (denisity_program_)
		glDeleteProgram(denisity_program_);
	if (collision_program_)
		glDeleteProgram(collision_program_);
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
	InitScene(glm::vec3(0.25f, 0.25f, 0.25f), 0.5f,
		".\\voxelfiles\\earth_voxel_256.txt");
}

void SPHSimulator::InitSimulation()
{
	//simulator_program_ = compileComputer(sphSimulatorComputer);
	buffer_in_ = 0;
	gpu_particles_ptr_->buffer_exchange_ = buffer_in_;
	grid_program_ = compileComputer(sphGridComputer);
	denisity_program_ = compileComputer(sphDenisityComputer);
	acceleration_program_ = compileComputer(sphAccelerationComputer);
	collision_program_ = compileComputer(collisionComputer);

	int x_ = gpu_grid_ptr_->grid_x_;
	int y_ = gpu_grid_ptr_->grid_y_;
	int z_ = gpu_grid_ptr_->grid_z_;
	float xx = 0.1f;
	float yy = 0.9f;
	float zz = 0.1f;
	GLfloat* init_float_p = new GLfloat[particle_number_*4];
	GLfloat* init_float_v = new GLfloat[particle_number_*4];
	for(int i = 0; i < particle_number_; i++)
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
	for (int i = 0; i < 2; i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->positions_vbo_[i]);
		glBufferData(GL_ARRAY_BUFFER, particle_number_*4*sizeof(GLfloat),
			init_float_p, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->velocitys_vbo_[i]);
		glBufferData(GL_ARRAY_BUFFER, particle_number_*4*sizeof(GLfloat),
			init_float_v, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	delete init_float_v;
	delete init_float_p;
}

void SPHSimulator::display(float time_step)
{
	//reset grid count
	//int x_ = gpu_grid_ptr_->grid_x_;
	//int y_ = gpu_grid_ptr_->grid_y_;
	//int z_ = gpu_grid_ptr_->grid_z_;

	//GLint* init_count = new GLint[x_*y_*z_];
	//memset(init_count, 0, sizeof(GLint)*x_*y_*z_);
	//GLint* init_head = new GLint[x_*y_*z_];
	//memset(init_head, 0xffff, sizeof(GLint)*x_*y_*z_);

	//glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->count_buffer_);
	//glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*sizeof(GLint),
	//	init_count, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->head_buffer_);
	//glBufferData(GL_ARRAY_BUFFER, x_*y_*z_*sizeof(GLint),
	//	init_head, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//delete init_head;
	//delete init_count;

	//glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	//glUseProgram(simulator_program_);
	//
	//glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_,
	//	0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_,
	//	0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	//glBindImageTexture(0, gpu_grid_ptr_->grid_head_tbo_,
	//	0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	//glBindImageTexture(1, gpu_grid_ptr_->grid_count_tbo_,
	//	0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);

	//GLint uniform_loc;

	//uniform_loc = glGetUniformLocation(simulator_program_, "time_step");
	//glUniform1f(uniform_loc, time_step);

	//uniform_loc = glGetUniformLocation(simulator_program_, "smooth_length");
	//glUniform1f(uniform_loc, SMOOTH_LENGTH);

	//uniform_loc = glGetUniformLocation(simulator_program_, "grid_size");
	//glUniform3i(uniform_loc,
	//	gpu_grid_ptr_->grid_x_,
	//	gpu_grid_ptr_->grid_y_,
	//	gpu_grid_ptr_->grid_z_);

	//float factors = PARTICLE_MASS * 315.0f / (64.0f * M_PI * pow(SMOOTH_LENGTH, 9.0f));
	//uniform_loc = glGetUniformLocation(simulator_program_, "denisity_smooth_factor");
	//glUniform1f(uniform_loc, factors);

	//factors = PRESSURE_CONSTANT;
	//uniform_loc = glGetUniformLocation(simulator_program_, "press_factor");
	//glUniform1f(uniform_loc, factors);

	//factors = PARTICLE_MASS * 45.0f / (M_PI * pow(SMOOTH_LENGTH, 6.0f));
	//uniform_loc = glGetUniformLocation(simulator_program_, "a_smooth_factor");
	//glUniform1f(uniform_loc, factors);

	//factors = MOTION_DAMPING;
	//uniform_loc = glGetUniformLocation(simulator_program_, "viscosity");
	//glUniform1f(uniform_loc, factors);

	//uniform_loc = glGetUniformLocation(simulator_program_, "a_outside");
	//glUniform3f(uniform_loc, 1.0f, -10.0f, 0.0f);
	//glFinish();
	//glDispatchCompute(NUM, 1, 1);
	//
	////glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	//glUseProgram(0);
	//glFinish();
	buffer_in_++;
	buffer_in_ %= 2;
	gpu_particles_ptr_->buffer_exchange_ = buffer_in_;
	gridStep();
	denisityStep();
	accelerationStep(time_step);
	collisionStep();
}

void SPHSimulator::gridStep()
{
	//reset grid count
	int x_ = gpu_grid_ptr_->grid_x_;
	int y_ = gpu_grid_ptr_->grid_y_;
	int z_ = gpu_grid_ptr_->grid_z_;

	GLint* init_count = new GLint[x_*y_*z_];
	memset(init_count, 0, sizeof(GLint)*x_*y_*z_);
	GLint* init_head = new GLint[x_*y_*z_];
	memset(init_head, 0xffffffff, sizeof(GLint)*x_*y_*z_);

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
	glUseProgram(grid_program_);

	glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(0, gpu_grid_ptr_->grid_head_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(1, gpu_grid_ptr_->grid_count_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(4, gpu_particles_ptr_->positions_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, gpu_particles_ptr_->velocitys_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLint uniform_loc;

	uniform_loc = glGetUniformLocation(grid_program_, "time_step");
	glUniform1f(uniform_loc, 0);

	uniform_loc = glGetUniformLocation(grid_program_, "smooth_length");
	glUniform1f(uniform_loc, SMOOTH_LENGTH);

	uniform_loc = glGetUniformLocation(grid_program_, "grid_size");
	glUniform3i(uniform_loc,
		gpu_grid_ptr_->grid_x_,
		gpu_grid_ptr_->grid_y_,
		gpu_grid_ptr_->grid_z_);

	float factors = PARTICLE_MASS * 315.0f / (64.0f * M_PI * pow(SMOOTH_LENGTH, 9.0f));
	uniform_loc = glGetUniformLocation(grid_program_, "denisity_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = PRESSURE_CONSTANT;
	uniform_loc = glGetUniformLocation(grid_program_, "press_factor");
	glUniform1f(uniform_loc, factors);

	factors = PARTICLE_MASS * 45.0f / (M_PI * pow(SMOOTH_LENGTH, 6.0f));
	uniform_loc = glGetUniformLocation(grid_program_, "a_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = MOTION_DAMPING;
	uniform_loc = glGetUniformLocation(grid_program_, "viscosity");
	glUniform1f(uniform_loc, factors);

	uniform_loc = glGetUniformLocation(grid_program_, "a_outside");
	glUniform3f(uniform_loc, 1.0f, -10.0f, 0.0f);
	glFinish();
	glDispatchCompute(NUM, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	glFinish();

	//glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->count_buffer_);
	//GLint* cpu_ptr = (GLint*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	////GLint* count_array = new GLint[x_*y_*z_];
	//for (int i = 0; i < x_*y_*z_; i++)
	//{
	//	printf("%d", (int)cpu_ptr[i]);
	//	if ((i+1)%x_ == 0)	printf("\n");
	//	if ((i+1)%(x_*y_) == 0)	
	//		printf("\n");
	//	//count_array[i] = cpu_ptr[i];
	//}
	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glBindBuffer(GL_ARRAY_BUFFER, gpu_grid_ptr_->head_buffer_);
	//GLint* cpu_ptr_h = (GLint*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	//GLint* head_array = new GLint[x_*y_*z_];
	//for (int i = 0; i < x_*y_*z_; i++)
	//{
	//	head_array[i] = cpu_ptr_h[i];
	//}
	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->positions_vbo_);
	//GLfloat* cpu_ptr_next = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	////for (int i = 0; i < particle_number_; i++)
	////{
	////	if (cpu_ptr[4*i+3] >=  (float)(x_*y_*z_))
	////		printf("%d ", (int)cpu_ptr[4*i+3]);
	////	//if ((i+1)%x_ == 0)	printf("\n");
	////	//if ((i+1)%(x_*y_) == 0)	
	////	//	printf("\n");
	////}

	//for (int i = 0; i < x_*y_*z_; i++)
	//{
	//	if (count_array[i] > 0)
	//	{
	//		//printf("%d: ", count_array[i]);
	//		int ptr = head_array[i];
	//		//printf("%d ", ptr);
	//		int count_t = 0;
	//		while(ptr >= 0)
	//		{
	//			//printf("%d ", (int)cpu_ptr_next[ptr*4+3]);
	//			count_t++;
	//			ptr = (int)cpu_ptr_next[ptr*4+3];
	//		}
	//		if (count_t != count_array[i])
	//		{
	//			printf("%d\n", i);
	//		}
	//	}
	//}
	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//delete count_array;
	//delete head_array;
}

void SPHSimulator::denisityStep()
{
	//reset grid count
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(denisity_program_);

	glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(0, gpu_grid_ptr_->grid_head_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(1, gpu_grid_ptr_->grid_count_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(4, gpu_particles_ptr_->positions_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(5, gpu_particles_ptr_->velocitys_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	GLint uniform_loc;

	uniform_loc = glGetUniformLocation(denisity_program_, "time_step");
	glUniform1f(uniform_loc, 0);

	uniform_loc = glGetUniformLocation(denisity_program_, "smooth_length");
	glUniform1f(uniform_loc, SMOOTH_LENGTH);

	uniform_loc = glGetUniformLocation(denisity_program_, "grid_size");
	glUniform3i(uniform_loc,
		gpu_grid_ptr_->grid_x_,
		gpu_grid_ptr_->grid_y_,
		gpu_grid_ptr_->grid_z_);

	float factors = PARTICLE_MASS * 315.0f / (64.0f * M_PI * pow(SMOOTH_LENGTH, 9.0f));
	uniform_loc = glGetUniformLocation(denisity_program_, "denisity_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = PRESSURE_CONSTANT;
	uniform_loc = glGetUniformLocation(denisity_program_, "press_factor");
	glUniform1f(uniform_loc, factors);

	factors = PARTICLE_MASS * 45.0f / (M_PI * pow(SMOOTH_LENGTH, 6.0f));
	uniform_loc = glGetUniformLocation(denisity_program_, "a_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = MOTION_DAMPING;
	uniform_loc = glGetUniformLocation(denisity_program_, "viscosity");
	glUniform1f(uniform_loc, factors);

	uniform_loc = glGetUniformLocation(denisity_program_, "a_outside");
	glUniform3f(uniform_loc, 0.0f, 0.0f, 0.0f);
	glFinish();
	glDispatchCompute(NUM, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	glFinish();

	//glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->velocitys_vbo_[buffer_in_]);
	//GLfloat* cpu_ptr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	//for (int i = 0; i < particle_number_; i++)
	//{
	//	//if (cpu_ptr[4*i+3] <= 0.0f)
	//		printf("%d ", (int)cpu_ptr[4*i+3]);
	//	//if ((i+1)%x_ == 0)	printf("\n");
	//	//if ((i+1)%(x_*y_) == 0)	
	//	//	printf("\n");
	//}
	//glUnmapBuffer(GL_ARRAY_BUFFER);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SPHSimulator::accelerationStep(float time_step)
{
	//reset grid count

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(acceleration_program_);

	glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(0, gpu_grid_ptr_->grid_head_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(1, gpu_grid_ptr_->grid_count_tbo_,
		0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	glBindImageTexture(4, gpu_particles_ptr_->positions_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(5, gpu_particles_ptr_->velocitys_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	GLint uniform_loc;

	uniform_loc = glGetUniformLocation(acceleration_program_, "time_step");
	glUniform1f(uniform_loc, time_step);

	uniform_loc = glGetUniformLocation(acceleration_program_, "smooth_length");
	glUniform1f(uniform_loc, SMOOTH_LENGTH);

	uniform_loc = glGetUniformLocation(acceleration_program_, "grid_size");
	glUniform3i(uniform_loc,
		gpu_grid_ptr_->grid_x_,
		gpu_grid_ptr_->grid_y_,
		gpu_grid_ptr_->grid_z_);

	float factors = PARTICLE_MASS * 315.0f / (64.0f * M_PI * pow(SMOOTH_LENGTH, 9.0f));
	uniform_loc = glGetUniformLocation(acceleration_program_, "denisity_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = PRESSURE_CONSTANT;
	uniform_loc = glGetUniformLocation(acceleration_program_, "press_factor");
	glUniform1f(uniform_loc, factors);

	factors = PARTICLE_MASS * 45.0f / (M_PI * pow(SMOOTH_LENGTH, 6.0f));
	uniform_loc = glGetUniformLocation(acceleration_program_, "a_smooth_factor");
	glUniform1f(uniform_loc, factors);

	factors = MOTION_DAMPING;
	uniform_loc = glGetUniformLocation(acceleration_program_, "viscosity");
	glUniform1f(uniform_loc, factors);

	uniform_loc = glGetUniformLocation(acceleration_program_, "a_outside");
	glUniform3f(uniform_loc, 0.0f, -10.0f, 0.0f);
	glFinish();
	glDispatchCompute(NUM, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	glFinish();
}

void SPHSimulator::InitScene(glm::vec3 middle_pos_in_xyz, float scale_xyz,
	const char* voxel_file_name)
{
	scene_structure_ptr_ = VoxelMaker::LoadVoxelFromFile(voxel_file_name);
	scene_voxel_data_tbo_ = scene_structure_ptr_->Creat3DTexture();

	float scalef;
	int scene_x, scene_y, scene_z;
	scene_structure_ptr_->get_size(scene_x, scene_y, scene_z);
	if (scale_xyz > 0.0f)
	{
		scalef = ((float)scene_x)/scale_xyz;
	}
	else
		scalef = (float)scene_x;
	scene_matrix_ = glm::scale(glm::mat4(1.0f),
		glm::vec3(scalef, scalef, scalef));

	scene_matrix_ = glm::translate(scene_matrix_,
		glm::vec3(0.5f, 0.5f, 0.5f)- middle_pos_in_xyz);
}

void SPHSimulator::collisionStep()
{
	//reset grid count

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(collision_program_);

	glBindImageTexture(2, gpu_particles_ptr_->positions_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	glBindImageTexture(3, gpu_particles_ptr_->velocitys_tbo_[buffer_in_],
		0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	//scene
	glBindImageTexture(0, scene_voxel_data_tbo_, 0, GL_TRUE,
		0, GL_READ_ONLY, GL_R8UI);
	glBindImageTexture(4, gpu_particles_ptr_->positions_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindImageTexture(5, gpu_particles_ptr_->velocitys_tbo_[(buffer_in_+1)%2],
		0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	GLint uniform_loc;

	uniform_loc = glGetUniformLocation(collision_program_, "scene_matrix");
	glUniformMatrix4fv(uniform_loc, 1, GL_FALSE, glm::value_ptr(scene_matrix_));

	int scene_x, scene_y, scene_z;
	scene_structure_ptr_->get_size(scene_x, scene_y, scene_z);

	uniform_loc = glGetUniformLocation(collision_program_, "scene_size");
	glUniform3i(uniform_loc, scene_x, scene_y, scene_z);

	glFinish();
	glDispatchCompute(NUM, 1, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);
	glFinish();

	glBindBuffer(GL_ARRAY_BUFFER, gpu_particles_ptr_->positions_vbo_[(buffer_in_+1)%2]);
	GLfloat* cpu_ptr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
	for (int i = 0; i < particle_number_; i++)
	{
		if (cpu_ptr[4*i+3] > 1.0f/* &&cpu_ptr[4*i+3] < 256.0f*/)
			printf("%d: %f \n", i, cpu_ptr[4*(i+1)+3]);
		//if ((i+1)%x_ == 0)	printf("\n");
		//if ((i+1)%(x_*y_) == 0)	
		//	printf("\n");
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}