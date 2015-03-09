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

		SEMI_SURE = 3
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
protected:
	enum DepthDirection
	{
		XOY_MAX_Z_UP_Y = 0,
		XOY_MIN_Z_DOWN_Y = 1,
		YOZ_MAX_X_UP_Z =2,
		YOZ_MIN_X_DOWN_Z = 3,
		ZOX_MAX_Y_UP_X = 4,
		ZOX_MIN_Y_DOWN_x = 5
	};

	//data
	int width_;
	int height_;
	int depth_;
	unsigned char* data_buffer_loc_;
	std::vector<glm::vec3> vertices_;
	GLuint vertice_buffer_handel_;
	GLuint draw_depth_program_;
	glm::vec3 vertices_max_, vertices_min_;
	GLfloat cell_size_;
	int material_used_number_[256];
	int material_count_;

	void SetSize(int size);
	void CreatVoxel();
	//将某正交投影深度图保存到内存，返回内存起始地址
	float* DrawDepth(
		glm::ivec3 start_min,
		glm::ivec3 end_max
		);

	void DrawSixTimes(const glm::mat4& pvm, GLuint texture_id, GLuint vao);

	void FindMiddle(glm::vec3 current_max, glm::vec3 current_min, glm::vec3& middle_position);

	void FindBoundingBox(glm::vec3& vertices_max, glm::vec3& vertices_min,
		glm::ivec3 start_min, glm::ivec3 end_max);
	void FindBoundingBox();

	void VoxelizationLogical();
	//返回不确定的个数,并且改变填充包围盒
	int FillVoxels(glm::ivec3& start_min, glm::ivec3& end_max, int unsure_num);
	//void FindUnsureBox(glm::ivec3& start_min, glm::ivec3& end_max);
	bool LocationDepth(DepthDirection& direction, glm::ivec3& location, int depth_index, float depth_value,
		glm::ivec3 start_min, glm::ivec3 end_max);
	
	//实现时注意更新material_count_
	unsigned char FindMaterial(glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction);
	//start_point就是包围盒起始点，实现时计算
	//实现时注意更新material_used_number_
	void FillWithMaterial(unsigned char material, glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction);
	void FillInsideSurface(glm::ivec3 point_index);

	//carefully fill the voxel
	void FillVoxelWith(unsigned char material, int index);
private:
};

#endif