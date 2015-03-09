#include "voxelization.h"
#include <limits>
#include "shader.h"
#include "glincludes.h"
#include <assert.h>
//#include "camera.h"

VoxelStructure::VoxelStructure():
	width_(0),
	height_(0),
	depth_(0),
	voxel_data_(NULL)
{
	
}

VoxelStructure::~VoxelStructure()
{

}

VoxelMaker::VoxelMaker():
	width_(0),
	height_(0),
	depth_(0),
	data_buffer_loc_(NULL),
	draw_depth_program_(0),
	vertice_buffer_handel_(0),
	cell_size_(0.0f)
{
	draw_depth_program_ = compileProgram(drawDepthVertex, drawDepthFragment);
	//draw_depth_program_ = compileProgram(commonVertex, commonFragment);
	glGenBuffers(1, &vertice_buffer_handel_);
	vertices_max_ = vertices_min_ = glm::vec3(1.0f);
	memset(material_used_number_, 0, sizeof(bool)*256);

	material_count_ = 1;
}

VoxelMaker::~VoxelMaker()
{
	if (data_buffer_loc_)
	{
		delete data_buffer_loc_;
	}
	vertices_.clear();
	if (draw_depth_program_)
	{
		glDeleteProgram(draw_depth_program_);
	}
}

VoxelMaker* VoxelMaker::MakeObjToVoxel(const char* obj_path, int voxel_size)
{
	VoxelMaker* voxel_maker_ptr = new VoxelMaker();

	//加载顶点vector
	Util ut;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	ut.loadOBJ(obj_path, voxel_maker_ptr->vertices_, texcoords, normals);
	GLfloat* vertice_data = new GLfloat[voxel_maker_ptr->vertices_.size() * 3 * sizeof(GLfloat)];
	for (int i = 0; i < (voxel_maker_ptr->vertices_).size(); i++)
	{
		vertice_data[i*3 + 0] = (voxel_maker_ptr->vertices_[i]).x;
		vertice_data[i*3 + 1] = (voxel_maker_ptr->vertices_[i]).y;
		vertice_data[i*3 + 2] = (voxel_maker_ptr->vertices_[i]).z;
	}
	glBindBuffer(GL_ARRAY_BUFFER, voxel_maker_ptr->vertice_buffer_handel_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * (voxel_maker_ptr->vertices_).size(), 
		vertice_data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	delete vertice_data;
	normals.clear();
	texcoords.clear();

	//生成体数据
	voxel_maker_ptr->SetSize(voxel_size);
	int x, y, z;
	voxel_maker_ptr->GetSize(x, y, z);
	//test
	voxel_maker_ptr->VoxelizationLogical();
	//	glm::ivec3(x-1,y-1,z-1));
	for (int i = 0; i < x*y; i++)
	{
		printf("%d", voxel_maker_ptr->data_buffer_loc_[i]);

		if ((i+1)%x == 0)
			printf("\n");
	}

	//暂定
	return voxel_maker_ptr;
}

void VoxelMaker::FindBoundingBox()
{
	float X_max = -numeric_limits<float>::max(),
		X_min = numeric_limits<float>::max(),
		Y_max = -numeric_limits<float>::max(),
		Y_min = numeric_limits<float>::max(),
		Z_max = -numeric_limits<float>::max(), 
		Z_min = numeric_limits<float>::max();

	std::vector<glm::vec3>::iterator it = vertices_.begin();
	while(it != vertices_.end())
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

	vertices_max_.x = X_max;
	vertices_max_.y = Y_max;
	vertices_max_.z = Z_max;
	vertices_min_.x = X_min;
	vertices_min_.y = Y_min;
	vertices_min_.z = Z_min;
}

void VoxelMaker::SetSize(int size)
{
	FindBoundingBox();
	glm::vec3 v_max = vertices_max_;
	glm::vec3 v_min = vertices_min_;
	
	float box_size_f[3] = {
		v_max.x - v_min.x,
		v_max.y - v_min.y,
		v_max.z - v_min.z
	};
	float max_axis = (box_size_f[0] > box_size_f[1]) ? box_size_f[0]:box_size_f[1];
	max_axis = (max_axis > box_size_f[2]) ? max_axis : box_size_f[2];

	if (size > VOXEL_DATA_BUFFER_MAX_SIZE)
	{
		size = VOXEL_DATA_BUFFER_MAX_SIZE;
	}
	
	assert(max_axis > 10e-30); 
	float scale_f = (float)size / max_axis;
	
	int box_size_i[3];
	for (int i = 0; i < 3; i++)
	{
		box_size_f[i] *= scale_f;
		if (abs(box_size_f[i] - (float)size) < 10e-10)
		{
			box_size_i[i] = size;
		}
		else 
		{
			int size_i = (int)box_size_f[i];
			if (abs(box_size_f[i] - (float)size_i) < -10e-30)
			{
				box_size_i[i] = size_i;
			}
			else
			{
				box_size_i[i] = size_i + 1;
				vertices_max_[i] = (float)box_size_i[i] / scale_f + vertices_min_[i]; 
			}
		}
	}

	width_ = box_size_i[0];
	height_ = box_size_i[1];
	depth_ = box_size_i[2];

	cell_size_ = (vertices_max_.x - vertices_min_.x)/width_;
	material_used_number_[NOT_SURE] = width_*height_*depth_;
	//data_buffer_loc_ = new unsigned char[width_ * height_ * depth_];
}

void VoxelMaker::FindMiddle(glm::vec3 current_max, glm::vec3 current_min, 
	glm::vec3& middle_position)
{
	glm::vec3 v_max = current_max;
	glm::vec3 v_min = current_min;
	
	middle_position = (v_max + v_min) / 2.0f;
}

float* VoxelMaker::DrawDepth(glm::ivec3 start_min, glm::ivec3 end_max)
{
	//
	glm::ivec3 size = end_max - start_min + glm::ivec3(1, 1, 1);
	int buffer_size = size.x*size.y*2 + size.x*size.z*2 + size.y*size.z*2;
	float* cpu_buffer = new float[buffer_size];
	memset(cpu_buffer, -2, (buffer_size) * sizeof(float));

	/*********************gpu setup*****************************/
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	//vertex vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertice_buffer_handel_);
	GLint vertice_loc = glGetAttribLocation(draw_depth_program_, "vVertex");
	glEnableVertexAttribArray(vertice_loc);
	if (vertice_loc >= 0)
		glVertexAttribPointer(vertice_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//generate out image and initial it
	GLuint depth_texture_ids[6];
	glGenTextures(6, depth_texture_ids);
	unsigned int image_width[3] = {size.x, size.y, size.z};
	unsigned int image_height[3] = {size.y, size.z, size.x};

	for (int i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i]);
		glTexStorage2D(GL_TEXTURE_2D, 0, GL_R32F, 
			image_width[i/2], image_height[i/2]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 
			image_width[i/2], image_height[i/2],
			0, GL_RED, GL_FLOAT, cpu_buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	/*********************gpu setup*****************************/

	/*********************matrix initial************************/
	glm::mat4 model = glm::mat4(1.0f), view, projection;
	glm::vec3 eye, up, look_at;
	glm::vec3 vertice_max, vertice_min;
	FindBoundingBox(vertice_max, vertice_min, start_min, end_max);
	FindMiddle(vertice_max, vertice_min, look_at);
	/*********************matrix initial************************/

	/*********************render 6 times************************/
	//ortho()是以eye为基准的。
	unsigned int cpu_buffer_offset[] = {
		0,
		size.x * size.y * 2,
		size.y * size.z * 2 + size.x * size.y * 2
	};
	for(int i = 2; i < 3; i++)
	{
		glViewport(0, 0, image_width[i], image_height[i]);
		eye = glm::vec3(look_at.x, look_at.y, look_at.z);	
		up = glm::vec3(0.0f, 0.0f, 0.0f);
		//front
		if (i == 0)
		{
			eye[(i+2)%3] = vertice_max[(i+2)%3];
			up[(i+1)%3] = 1.0f;
		}
		else if (i == 1)
		{
			eye[(i+2)%3] = vertice_min[(i+2)%3];
			up[(i+1)%3] = -1.0f;
		}
		else
		{
			eye[(i+2)%3] = vertice_max[(i+2)%3];
			up[(i+1)%3] = 1.0f;
		}
		view = glm::lookAt(eye, look_at, up);
		glm::vec3 vertice_min_t = vertice_min - look_at;
		glm::vec3 vertice_max_t = vertice_max - look_at;
		projection = glm::ortho(
			vertice_min_t[i]-(10e-10f), vertice_max_t[i],
			vertice_min_t[(i+1)%3], vertice_max_t[(i+1)%3],
			0.0f, vertice_max_t[(i+2)%3] - vertice_min_t[(i+2)%3]);
		DrawSixTimes(projection*view*model, depth_texture_ids[i*2], vao);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i*2]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, cpu_buffer + cpu_buffer_offset[i]);
		glBindTexture(GL_TEXTURE_2D, 0);

		//back
		if (i == 0)
		{
			eye[(i+2)%3] = vertice_min[(i+2)%3];
			up[(i+1)%3] = 1.0f;
		}
		else if (i == 1)
		{
			eye[(i+2)%3] = vertice_max[(i+2)%3];
			up[(i+1)%3] = -1.0f;
		}
		else
		{
			eye[(i+2)%3] = vertice_min[(i+2)%3];
			up[(i+1)%3] = 1.0f;
		}
		view = glm::lookAt(eye, look_at, up);
		projection = glm::ortho(
			vertice_min_t[(i+1)%3], vertice_max_t[(i+1)%3],
			vertice_min_t[i], vertice_max_t[i],
			0.0f-(10e-30f), -vertice_max_t[(i+2)%3] + vertice_min_t[(i+2)%3] + (10e-30f));
		DrawSixTimes(projection*view*model, depth_texture_ids[i*2 + 1], vao);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i*2 + 1]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, 
			cpu_buffer + cpu_buffer_offset[i] + image_width[i]*image_height[i]);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	/*********************render 6 times************************/
	
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(6, depth_texture_ids);
	glFinish();
	return cpu_buffer;
	//return NULL;
}

void VoxelMaker::FindBoundingBox(glm::vec3& vertices_max, glm::vec3& vertices_min,
	glm::ivec3 start_min, glm::ivec3 end_max)
{
	vertices_max = vertices_max_;
	vertices_min = vertices_min_;
	glm::ivec3 size = end_max - start_min + glm::ivec3(1, 1, 1);
	float cell_size = vertices_max_.x - vertices_min_.x;
	cell_size /= (float)width_;
	vertices_min += glm::vec3(start_min.x * cell_size,
		start_min.y * cell_size,
		start_min.z * cell_size);

	vertices_max = vertices_min + 
		glm::vec3(size.x * cell_size,
		size.y * cell_size,
		size.z * cell_size);
}

void VoxelMaker::DrawSixTimes(const glm::mat4& pvm, GLuint texture_id, GLuint vao)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glUseProgram(draw_depth_program_);

	GLint pvm_mat_loc = glGetUniformLocation(draw_depth_program_, "pvm");
	glUniformMatrix4fv(pvm_mat_loc, 1, GL_FALSE, glm::value_ptr(pvm));

	glBindImageTexture(0, texture_id, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	GLint image_loc = glGetUniformLocation(draw_depth_program_, "depth_image");
	glUniform1i(image_loc, 0);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glBindVertexArray(0);
	glUseProgram(0);
	//glBindImageTexture(0, 0 , 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
}

void VoxelMaker::VoxelizationLogical()
{
	data_buffer_loc_ = new unsigned char[width_ * height_ * depth_];
	memset(data_buffer_loc_, 0, width_ * height_ * depth_);
	glm::ivec3 start_min = glm::ivec3(0, 0, 0);
	glm::ivec3 end_max = glm::ivec3(width_-1, height_-1, depth_-1);
	int num_unsure = width_*depth_*height_;
	int iterate_count = 0;
	while(num_unsure > 0)
	{
		num_unsure = FillVoxels(start_min, end_max);
		//FindUnsureBox(start_min, end_max);
		iterate_count++;
		if (iterate_count > 1)	break;
	}
}

const glm::ivec3 Forward_Sum[6] = 
{
	glm::ivec3(0, 0, 1),
	glm::ivec3(0, 0, -1),
	glm::ivec3(1, 0, 0),
	glm::ivec3(-1, 0, 0),
	glm::ivec3(0, 1, 0),
	glm::ivec3(0, -1, 0)
};

const glm::ivec3 Backward_Sum[6] = 
{
	glm::ivec3(0, 0, -1),
	glm::ivec3(0, 0, 1),
	glm::ivec3(-1, 0, 0),
	glm::ivec3(1, 0, 0),
	glm::ivec3(0, -1, 0),
	glm::ivec3(0, 1, 0)
};

int VoxelMaker::FillVoxels(glm::ivec3& start_min, glm::ivec3& end_max)
{
	float* depth_ptr = DrawDepth(start_min, end_max);
	glm::ivec3 size = end_max - start_min + glm::ivec3(1,1,1);
	glm::ivec3 current_surface_loc, start_point;
	DepthDirection direction;
	
	int size_i = size.x*size.y*2 + size.x*size.z*2+size.y*size.z*2;
	for (int i = 0; i < size_i; i++)
	{
		bool is_inside = LocationDepth(direction, current_surface_loc, 
			start_point, i, depth_ptr[i], start_min, end_max);
		//glm::ivec3 start_point = current_surface_loc + Backward_Sum[direction] * current_surface_loc[direction];
		if (is_inside)
		{
			FillInsideSurface(current_surface_loc);
			if (start_point != current_surface_loc)
			{
				unsigned int material = FindMaterial(start_point, current_surface_loc + Backward_Sum[direction], direction);
				FillWithMaterial(material, start_point, current_surface_loc + Backward_Sum[direction], direction);
			}
		}
		else
		{
			unsigned int material = FindMaterial(start_point, current_surface_loc, direction);
			FillWithMaterial(material, start_point, current_surface_loc, direction);
		}
		if (material_count_ > 254)	break;
	}
	//
	//glm::ivec3 temp = start_min;
	//start_min = end_max;
	//end_max = temp;
	int max_int = 1028;
	start_min = glm::ivec3(max_int, max_int, max_int);
	end_max = glm::ivec3(-max_int,-max_int, -max_int);

	for (int i = 0; i < width_*depth_*height_; i++)
	{
		int index = i;
		int x, y, z;
		x = index%width_;
		y = index%(width_*height_) / height_;
		z = index/(width_*height_);
		glm::ivec3 current_index = glm::ivec3(x, y, z);
		if (data_buffer_loc_[index] == NOT_SURE)
		{
			if (current_index.x < start_min.x)	start_min.x = current_index.x;
			if (current_index.y < start_min.y)	start_min.y = current_index.y;
			if (current_index.z < start_min.z)	start_min.z = current_index.z;
			if (current_index.x > end_max.x)	end_max.x = current_index.x;
			if (current_index.y > end_max.y)	end_max.y = current_index.y;
			if (current_index.x > end_max.z)	end_max.z = current_index.z;
		}
	}
	return material_used_number_[NOT_SURE];
}

//carefully coded
bool VoxelMaker::LocationDepth(DepthDirection& direction, glm::ivec3& location,
	glm::ivec3& start_point, int depth_index, float depth_value, glm::ivec3 start_min, glm::ivec3 end_max)
{
	glm::ivec3 current_size = end_max - start_min + glm::ivec3(1, 1, 1);
	assert(current_size.x > 0 && current_size.y > 0 && current_size.z > 0);

	const int texture_sizes[6] = {
		current_size.x * current_size.y,//2
		current_size.x * current_size.y,//2
		current_size.y * current_size.z,//2
		current_size.y * current_size.z,//2
		current_size.z * current_size.x,//4
		current_size.z * current_size.x//4
	};

	int accumulate_size = texture_sizes[XOY_MAX_Z_UP_Y];
	direction = XOY_MAX_Z_UP_Y;
	
	while(depth_index >= accumulate_size)
	{
		assert(direction < 6);
		int i = direction+1;
		accumulate_size += texture_sizes[i];
		direction = static_cast<VoxelMaker::DepthDirection>(i);
	}
	glm::ivec3 current_flat;
	current_flat[(direction/2 + 2) % 3] = 0;
	current_flat[direction/2] = depth_index%current_size[direction/2];
	current_flat[(direction/2 + 1) % 3] = (depth_index - (accumulate_size - texture_sizes[direction]))/current_size[direction/2];
	//glm::ivec3 frag_step = glm::ivec3(1, 1, 1) - Forward_Sum[direction];
	//glm::ivec3 frag_step = glm::ivec3(1, 1, 1);
	//frag_step[(direction/2 + 2) % 3] = 0;
	//glm::ivec3 start_point;
	if (direction%2 == 0)
	{
		start_point = current_flat + start_min;
		//start_point[(direction/2 + 2) % 3] = start_min[(direction/2 + 2) % 3];
	}
	else
	{
		//current_flat[direction/2] = current_size[direction/2] - depth_index%current_size[direction/2];
		//current_flat[(direction/2 + 1) % 3] = current_size[(direction/2 + 1) % 3] - depth_index/current_size[direction/2];
		start_point = current_flat + start_min;
		start_point[(direction/2 + 2) % 3] = end_max[(direction/2 + 2) % 3];
		start_point[(direction/2) % 3] = end_max[(direction/2) % 3] - start_point[(direction/2) % 3];
	}

	if (depth_value < 0.0f)
	{
		//location = start_point + Forward_Sum[direction] * current_size[(direction/2 + 2)%3] - glm::ivec3(1,1,1);
		location = start_point + Forward_Sum[direction] * (current_size[(direction/2 + 2)%3]-1);
		return false;
	}
	depth_value *= (end_max[(direction/2 + 2) % 3]-start_min[(direction/2 + 2) % 3]);
	int depth_count = (int)(depth_value) ;
	if (depth_value > (float)depth_count)
		depth_count++;
	location = start_point + Forward_Sum[direction] * (depth_count-1);

	return true;
}

unsigned char VoxelMaker::FindMaterial(glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	int depth_axis = (direction/2 + 2) % 3;
	//start_point[depth_axis]--;
	glm::ivec3 start_p = start_point + Backward_Sum[direction];
	//end_point[depth_axis]++;
	glm::ivec3 end_p = end_point + Forward_Sum[direction];
	//assert(start_point[depth_axis] < width_);
	glm::ivec3 global_size = glm::ivec3(width_, height_, depth_);
	if (start_p[depth_axis] < 0 ||start_p[depth_axis] >= global_size[depth_axis] ||
		end_p[depth_axis] < 0 || end_p[depth_axis] >= global_size[depth_axis])
		return OUTSIDE;
	//int current_axis_point = start_point[depth_axis];
	glm::ivec3 point_index = start_p;
	unsigned char current_state = NOT_SURE;
	while (point_index != end_p)
	{
		int current_index = point_index.x +
			point_index.y * height_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		//point_index[depth_axis] = current_axis_point;
		if (data_buffer_loc_[current_index] != NOT_SURE||data_buffer_loc_[current_index] != INSIDE_SURFACE)
		{
			current_state = data_buffer_loc_[current_index];
		}
		if (data_buffer_loc_[current_index] == OUTSIDE)
		{
			current_state = OUTSIDE;
			return OUTSIDE;
		}
	}

	//in the end
	int current_index = point_index.x +
		point_index.y * height_
		+ point_index.z * width_ * height_;
	if (data_buffer_loc_[current_index] != NOT_SURE||data_buffer_loc_[current_index] != INSIDE_SURFACE)
	{
		current_state = data_buffer_loc_[current_index];
	}
	if (data_buffer_loc_[current_index] == OUTSIDE)
	{
		current_state = OUTSIDE;
		return OUTSIDE;
	}
	//

	if (current_state == NOT_SURE)
	{
		assert(material_count_ < 256);
		for (int i = 3; i < 256; i++)
		{
			if (material_used_number_[i] == 0)
			{
				current_state = (unsigned char)i;
				return current_state;
			}
		}
	}
	return current_state;
}

void VoxelMaker::FillWithMaterial(unsigned char material,glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	int depth_axis = (direction/2 + 2) % 3;
	assert(start_point[depth_axis] < width_);

	//int current_axis_point = start_point[depth_axis];
	glm::ivec3 point_index = start_point;
	
	while (point_index != end_point)
	{
		int current_index = point_index.x +
			point_index.y * height_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		unsigned char state_before = data_buffer_loc_[current_index];
		//assert(state_before != INSIDE_SURFACE);
		switch (state_before)
		{
		case NOT_SURE:
			FillVoxelWith(material, current_index);
			break;
		case OUTSIDE:
			FillWithOutSide(start_point, end_point, direction);
			break;
		case INSIDE_SURFACE:
			break;
		default:
			if (state_before != material)
			{
				for (int i = 0; i < width_*height_*depth_; i++)
				{
					if (data_buffer_loc_[i] == state_before)
					{
						FillVoxelWith(material, i);
						if (material_used_number_[state_before] == 0)	break;
					}
				}
			}
			break;
		}
	}
	//
	int current_index = point_index.x +
		point_index.y * height_
		+ point_index.z * width_ * height_;
	//current_axis_point++;
	//point_index += Forward_Sum[direction];
	unsigned char state_before = data_buffer_loc_[current_index];
	//assert(state_before != INSIDE_SURFACE);
	switch (state_before)
	{
	case NOT_SURE:
		FillVoxelWith(material, current_index);
		break;
	case OUTSIDE:
		//assert(material == OUTSIDE);
		FillWithOutSide(start_point, end_point, direction);
		break;
	case INSIDE_SURFACE:
		break;
	default:
		if (state_before != material)
		{
			for (int i = 0; i < width_*height_*depth_; i++)
			{
				if (data_buffer_loc_[i] == state_before)
				{
					FillVoxelWith(material, i);
					if (material_used_number_[state_before] == 0)	break;
				}
			}
		}
		break;
	}
}

void VoxelMaker::FillWithOutSide(glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	int depth_axis = (direction/2 + 2) % 3;
	assert(start_point[depth_axis] < width_);

	//int current_axis_point = start_point[depth_axis];
	glm::ivec3 point_index = start_point;

	while (point_index != end_point)
	{
		int current_index = point_index.x +
			point_index.y * height_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		unsigned char state_before = data_buffer_loc_[current_index];
		//assert(state_before != INSIDE_SURFACE);
		switch (state_before)
		{
		case NOT_SURE:
			FillVoxelWith(OUTSIDE, current_index);
			break;
		case OUTSIDE:
			break;
		case INSIDE_SURFACE:
			break;
		default:
			if (state_before != OUTSIDE)
			{
				for (int i = 0; i < width_*height_*depth_; i++)
				{
					if (data_buffer_loc_[i] == state_before)
					{
						FillVoxelWith(OUTSIDE, i);
						if (material_used_number_[state_before] == 0)	break;
					}
				}
			}
			break;
		}
	}
	//
	int current_index = point_index.x +
		point_index.y * height_
		+ point_index.z * width_ * height_;
	//current_axis_point++;
	//point_index += Forward_Sum[direction];
	unsigned char state_before = data_buffer_loc_[current_index];
	//assert(state_before != INSIDE_SURFACE);
	switch (state_before)
	{
	case NOT_SURE:
		FillVoxelWith(OUTSIDE, current_index);
		break;
	case OUTSIDE:
		//assert(material == OUTSIDE);
		//FillWithOutSide(start_point, end_point, direction);
		break;
	case INSIDE_SURFACE:
		break;
	default:
		if (state_before != OUTSIDE)
		{
			for (int i = 0; i < width_*height_*depth_; i++)
			{
				if (data_buffer_loc_[i] == state_before)
				{
					FillVoxelWith(OUTSIDE, i);
					if (material_used_number_[state_before] == 0)	break;
				}
			}
		}
		break;
	}
}


void VoxelMaker::FillInsideSurface(glm::ivec3 point_index)
{
	int index = point_index.x +
		point_index.y * height_
		+ point_index.z * width_ * height_;
	FillVoxelWith(INSIDE_SURFACE, index);
}

void VoxelMaker::FillVoxelWith(unsigned char material, int index)
{
	unsigned char current_state = data_buffer_loc_[index];
	material_used_number_[current_state]--;
	if (material_used_number_[current_state] <= 0)	material_count_--;
	data_buffer_loc_[index] = material;
	if (material_used_number_[material] <= 0) material_count_++;
	material_used_number_[material]++;
}