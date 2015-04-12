#ifndef SPH_SIMULATER_H_
#define SPH_SIMULATER_H_

#include "glincludes.h"
class VoxelStructure;
class FluidRenderer;
class SPHParticles
{
public:
	friend class SPHSimulator;
	friend class FluidRenderer;
	SPHParticles();
	~SPHParticles();
	void InitGPUResource(int particle_number);
	GLuint positions_vbo(){return positions_vbo_[buffer_exchange_];}
	GLint* particle_number_ptr(){return &particle_number_;}
protected:
private:
	int particle_number_;
	GLuint positions_tbo_[2];
	GLuint velocitys_tbo_[2];
	GLuint positions_vbo_[2];
	GLuint velocitys_vbo_[2];
	int	buffer_exchange_;
};

class SimulateDrawGrid
{
public:
	friend class SPHSimulator;
	SimulateDrawGrid();
	~SimulateDrawGrid();
	void InitGPUResource(const glm::ivec3 size_3d);
protected:
private:
	int grid_x_, grid_y_, grid_z_;
	GLuint grid_head_tbo_;
	GLuint grid_count_tbo_;
	GLuint count_buffer_;
	GLuint head_buffer_;
};

class SPHSimulator
{
public:
	SPHSimulator();
	~SPHSimulator();
	void InitScene(glm::vec3 middle_pos_in_xyz,
		float scale_xyz, const char* voxel_file_name);
	void InitGPUResource(const int particle_number, 
		const glm::vec3 scene_size, const float smooth_length);
	void InitSimulation();
	void display(float time_step);
	SPHParticles* gpu_particles_ptr_;
protected:
private:
	int particle_number_;
	float size_x_, size_y_, size_z_;
	float smooth_length_;
	//SPHParticles* gpu_particles_ptr_;
	SimulateDrawGrid* gpu_grid_ptr_;
	GLuint simulator_program_;
	GLuint grid_program_;
	GLuint denisity_program_;
	GLuint acceleration_program_;
	GLuint collision_program_;
	GLuint scene_voxel_data_tbo_;
	GLuint lock_image_tbo_;
	GLuint lock_image_buffer_;
	int buffer_in_;
	VoxelStructure* scene_structure_ptr_;
	glm::mat4 scene_matrix_;
	bool add_particle_;
	//other objects in scene....

	void gridStep();
	void denisityStep();
	void accelerationStep(float time_step);
	void collisionStep();
};

#endif