#ifndef VOXELIZATION_H_
#define VOXELIZATION_H_

#define VOXEL_DATA_BUFFER_MAX_SIZE 1024//体数据一维的最大长度

#include "util.h"
#include <vector>

class VoxelStructure
{
public:
	enum VoxelState
	{
		NOT_SURE = 0,
		INSIDE_SURFACE = 1,
		OUTSIDE = 2,

		INSIDE_0 = 3,
		INSIDE_1 = 4,
		INSIDE_2 = 5,
		INSIDE_3 = 6,
		INSIDE_4 = 7,
		INSIDE_5 = 8,
		INSIDE_6 = 9
		//...
	};
	VoxelStructure();
	virtual ~VoxelStructure();
	void get_size(int& width, int& height, int& depth)
	{
		width = width_;
		height = height_;
		depth = depth_;
	}
private:
	int width_;
	int height_;
	int depth_;
	unsigned char* voxel_data_;
};

class VoxelMaker
{
public:
	enum VoxelState
	{
		NOT_SURE = 0,
		INSIDE_SURFACE = 1,
		OUTSIDE = 2,

		INSIDE_0 = 3,
		INSIDE_1 = 4,
		INSIDE_2 = 5,
		INSIDE_3 = 6,
		INSIDE_4 = 7,
		INSIDE_5 = 8,
		INSIDE_6 = 9
		//...
	};
	VoxelMaker();
	virtual ~VoxelMaker();
	//static VoxelStructure* MakeObjToVoxel(const char* obj_path, int voxel_size);
	//暂时测试用
	static VoxelMaker* MakeObjToVoxel(const char* obj_path, int voxel_size);
	static VoxelStructure* LoadVoxelFromFile(const char* voxel_path);
	static void SaveToFile(const VoxelStructure* voxel, char* path);
	void GetSize(int& width, int& height, int& depth) {width = width_;height = height_; depth = depth_;}

	//暂定为public测试
	//float* DrawDepth(
	//	glm::ivec3 start_min,
	//	glm::ivec3 end_max
	//	);
protected:
	//data
	int width_;
	int height_;
	int depth_;
	unsigned char* data_buffer_loc_;
	std::vector<glm::vec3> vertices_;
	GLuint vertice_buffer_handel_;
	GLuint draw_depth_program_;
	glm::vec3 vertices_max_, vertices_min_;
	GLuint vao_;

	void SetSize(int size);
	void CreatVoxel();
	//将某正交投影深度图保存到内存，返回内存起始地址
	float* DrawDepth(
		glm::ivec3 start_min,
		glm::ivec3 size
		);

	void DrawSixTimes(const glm::mat4& pvm, GLuint texture_id, GLuint vao);

	void FindMiddle(glm::vec3 current_max, glm::vec3 current_min, glm::vec3& middle_position);

	void FindBoundingBox(glm::vec3& vertices_max, glm::vec3& vertices_min,
		glm::ivec3 start_min, glm::ivec3 end_max);
	void FindBoundingBox();
private:
};

#endif