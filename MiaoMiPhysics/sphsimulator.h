#ifndef SPH_SIMULATER_H_
#define SPH_SIMULATER_H_

#include "glincludes.h"
class SPHParticles
{
public:
	friend class SPHSimulator;
	SPHParticles();
	~SPHParticles();
	void InitGPUResource(int particle_number);
protected:
private:
	int particle_number_;
	GLuint positions_tbo_;
	GLuint velocitys_tbo_;
	GLuint positions_vbo_;
	GLuint velocitys_vbo_;
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
	GLuint grid_head_tbo;
	GLuint grid_count_tbo;
};

class SPHSimulator
{
public:
	SPHSimulator();
	~SPHSimulator();
	void InitScene();
	void InitGPUResource(const int particle_number, 
		const glm::vec3 scene_size, const float smooth_length);
	void InitSimulation();
	void display(float time_step);

protected:
private:
	int particle_number_;
	float size_x_, size_y_, size_z_;
	float smooth_length_;
	SPHParticles* gpu_particles_ptr_;
	SimulateDrawGrid* gpu_grid_ptr_;
	GLuint simulator_program_;
	GLuint scene_voxel_data_tbo_;
	//other objects in scene....
};

#endif