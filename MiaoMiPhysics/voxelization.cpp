#include "voxelization.h"
#include <limits>
#include "shader.h"
#include "glincludes.h"
#include <assert.h>
#include <fstream>

#pragma warning(disable:4996)

VoxelStructure::VoxelStructure():
	width_(0),
	height_(0),
	depth_(0),
	voxel_data_(NULL)
{
	
}

VoxelStructure::~VoxelStructure()
{
	if (voxel_data_)
		delete voxel_data_;
}

VoxelStructure::VoxelStructure(int width, int height, int depth, unsigned char* data)
	:width_(width),
	height_(height),
	depth_(depth),
	voxel_data_(data)
{

}

GLuint VoxelStructure::Creat3DTexture()
{
	//GLuint buffer_handle;
	GLuint texture_handle;
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_3D, texture_handle);
	//glGenBuffers(1, &buffer_handle);

	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer_handle);
	//glBufferData(GL_PIXEL_UNPACK_BUFFER, sizeof(GLubyte) * width_*height_*depth_/8,
	//	voxel_data_, GL_STATIC_DRAW);
	//glTexStorage3D(GL_TEXTURE_3D, 0, GL_R8, width_/8, height_, depth_);

	//glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0,
	//	width_/8, height_, depth_, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width_, height_, depth_/8,
		0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, voxel_data_);

	//GLubyte* get_back_ptr = new GLubyte[width_/8*height_*depth_];
	//memset(get_back_ptr, 0, sizeof(GLubyte) * width_ * height_ * depth_ /8 );
	//glGetTexImage(GL_TEXTURE_3D, 0, GL_R8UI, GL_UNSIGNED_BYTE, get_back_ptr);
	glBindTexture(GL_TEXTURE_3D, 0);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	//for (int i = width_ *height_ / 8* 0; i < (width_*height_*depth_)/8; i++)
	//{
	//	printf("%c\t", get_back_ptr[i]);
	//	if ((i+1)%(width_/8) == 0)	
	//		printf("\n");
	//	if ((i+1)%(width_*height_/8) == 0)	
	//		printf("\n");
	//}
	//delete get_back_ptr;
	return texture_handle;
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
	memset(material_used_number_, 0, sizeof(int)*(1<<20));

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

VoxelStructure* VoxelMaker::MakeObjToVoxel(const char* obj_path, int voxel_size)
{
	VoxelMaker* voxel_maker_ptr = new VoxelMaker();

	//加载顶点vector
	Util ut;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	ut.loadOBJ(obj_path, voxel_maker_ptr->vertices_, texcoords, normals);
	GLfloat* vertice_data = new GLfloat[voxel_maker_ptr->vertices_.size() * 3 * sizeof(GLfloat)];
	for (unsigned int i = 0; i < (voxel_maker_ptr->vertices_).size(); i++)
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

	VoxelStructure* voxel_structure = new VoxelStructure(
		x, y, z,
		voxel_maker_ptr->CreatEasyVoxel());

	//delete voxel_maker_ptr->data_buffer_loc_;

	
	//out to file test
	ofstream ofs;
	ofs.open("test.txt");
	for (int i = 0; i < x*y*z ; i ++)
	{
		if (voxel_maker_ptr->data_buffer_loc_[i] > 10)
		{
			ofs << (int)(voxel_maker_ptr->data_buffer_loc_[i])%7;
		}
		else
			ofs << (int)(voxel_maker_ptr->data_buffer_loc_[i]);
		if ((i+1) % x == 0)
		{
			ofs << "\n";
		}
		if ((i+1) % (x*y) == 0)
		{
			ofs << "\n";
		}
		if ((i+1) % (x*y*z) == 0)
		{
			ofs << "\n";
		}
	}
	ofs.flush();
	ofs.close();
	delete voxel_maker_ptr;
	return voxel_structure;
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

	size -= 2;

	if (size > VOXEL_DATA_BUFFER_MAX_SIZE - 2)
	{
		size = VOXEL_DATA_BUFFER_MAX_SIZE - 2;
	}
	
	if (size%2)
	{
		size--;
	}

	assert(max_axis > 10e-30); 
	float scale_f = (float)size / max_axis;
	
	int box_size_i[3];
	for (int i = 0; i < 3; i++)
	{
		box_size_f[i] *= scale_f;
		if (abs(box_size_f[i] - (float)size) < 1e-30)
		{
			box_size_i[i] = size;
		}
		else 
		{
			int size_i = (int)box_size_f[i];
			if (abs(box_size_f[i] - (float)size_i) < -10e-30f)
			{
				box_size_i[i] = size_i;
			}
			else
			{
				box_size_i[i] = size_i + 1;
				vertices_max_[i] = ((float)box_size_i[i]) / scale_f + vertices_min_[i]; 
			}
			if (box_size_i[i] % 2)
			{
				box_size_i[i] += 1;
				vertices_max_[i] = ((float)box_size_i[i]) / scale_f + vertices_min_[i]; 
			}
		}
		//if (/*box_size_i[i] % 2*/i == 2)
		//{
		//	box_size_i[i] += 15;
		//	vertices_max_[i] = ((float)box_size_i[i]) / scale_f + vertices_min_[i]; 
		//}
	}

	width_ = box_size_i[0];
	height_ = box_size_i[1];
	depth_ = box_size_i[2];

	cell_size_ = (vertices_max_.x - vertices_min_.x)/width_;
	//material_used_number_[NOT_SURE] = width_*height_*depth_;
	//data_buffer_loc_ = new unsigned char[width_ * height_ * depth_];
	width_ += 2;
	height_ += 2;
	depth_ += 2;
	vertices_min_ -= glm::vec3(cell_size_, cell_size_, cell_size_);
	vertices_max_ += glm::vec3(cell_size_, cell_size_, cell_size_);

	if (width_ % 8)
	{
		int pre_width = width_;
		width_ = ((int)floor((float)width_ / 8.0f))*8 + 8;
		vertices_max_.x += (width_-pre_width) * cell_size_;
	}

	if (depth_ % 8)
	{
		int pre_width = depth_;
		depth_ = ((int)floor((float)depth_ / 8.0f))*8 + 8;
		vertices_max_.z += (depth_-pre_width) * cell_size_;
	}

	material_used_number_[NOT_SURE] = width_*height_*depth_;
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
	glDisable(GL_CULL_FACE);
	//glDepthRange(0.0f, 1.0f);

	//vertex vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertice_buffer_handel_);
	GLint vertice_loc = glGetAttribLocation(draw_depth_program_, "vVertex");
	glEnableVertexAttribArray(vertice_loc);
	if (vertice_loc >= 0)
		glVertexAttribPointer(vertice_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//generate out image and initial it
	GLuint depth_texture_ids[6];
	GLuint frame_buffers[6];
	GLuint frame_buffer_depth_buffer[6];
	glGenTextures(6, depth_texture_ids);
	glGenFramebuffers(6, frame_buffers);
	glGenRenderbuffers(6, frame_buffer_depth_buffer);
	unsigned int image_width[3] = {size.x, size.z, size.z};
	unsigned int image_height[3] = {size.y, size.y, size.x};

	for (int i = 0; i < 6; i++)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffers[i]);

		glBindRenderbuffer(GL_RENDERBUFFER, frame_buffer_depth_buffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, image_width[i/2], image_height[i/2]);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frame_buffer_depth_buffer[i]);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i]);
		//glTexStorage2D(GL_TEXTURE_2D, 0, GL_R32F, 
		//	image_width[i/2], image_height[i/2]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 
			image_width[i/2], image_height[i/2],
			0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_2D, depth_texture_ids[i], 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
	/*********************gpu setup*****************************/

	/*********************matrix initial************************/
	glm::mat4 eye_model, view, projection;
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
	for(int i = 0; i < 3; i++)
	{
		eye_model = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glViewport(0, 0, image_width[i], image_height[i]);
		up = glm::vec3(0.0f, 0.0f, 0.0f);
		eye = look_at;
		glm::vec3 half_size = (vertice_max - vertice_min)/2.0f;
		//front
		if (i == 0)
		{
			eye.z = vertice_min.z;
			up = glm::vec3(0.0f, 1.0f, 0.0f);
			projection = glm::ortho(
				-half_size.x, half_size.x,
				-half_size.y, half_size.y,
				0.0f, half_size.z*2.0f);
		}
		else if (i == 1)
		{
			eye.x = vertice_min.x;
			up = glm::vec3(0.0f, 1.0f, 0.0f);
			projection = glm::ortho(
				-half_size.z, half_size.z,
				-half_size.y, half_size.y,
				0.0f, half_size.x*2.0f);
		}
		else
		{
			eye.y = vertice_min.y;
			up = glm::vec3(-1.0f, 0.0f, 0.0f);
			projection = glm::ortho(
				-half_size.z, half_size.z,
				-half_size.x, half_size.x,
				0.0f, half_size.y*2.0f);
		}

		model = glm::mat4(1.0f);
		view = glm::lookAt(eye, look_at, up);

		//projection = glm::ortho(
		//	-half_size[i%3], half_size[i%3],
		//	-half_size[(i+1)%3], half_size[(i+1)%3],
		//	0.0f, half_size[(i+2)%3]*2.0f);

		//如果只生成中心图，则深度测试改为较大通过
		//glClearDepth(1.0f);
		//glDepthFunc(GL_LESS);

		DrawSixTimes(view*model, projection, frame_buffers[i*2], vao);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i*2]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, cpu_buffer + cpu_buffer_offset[i]);
		glBindTexture(GL_TEXTURE_2D, 0);

		//test
		//for (int j = 0; j < image_width[i]*image_height[i]; j++)
		//{
		//	float _depth = cpu_buffer[j + cpu_buffer_offset[i]];
		//	if (_depth > 0.0f)
		//	{
		//		printf("1");
		//	}
		//	else	printf("0");
		//	if ((j+1)%image_width[i] == 0)
		//	{
		//		printf("\n");
		//	}
		//}

		//back

		if (i == 0)
		{
			eye.z = vertice_max.z;
			up = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else if (i == 1)
		{
			eye.x = vertice_max.x;
			up = glm::vec3(0.0f, 1.0f, 0.0f);
		}
		else
		{
			eye.y = vertice_max.y;
			up = glm::vec3(-1.0f, 0.0f, 0.0f);
		}

		//glClearDepth(-1.0f);
		//glDepthFunc(GL_GREATER);

		//view矩阵
		view = glm::lookAt(eye, look_at, up);

		DrawSixTimes(view*model, projection, frame_buffers[i*2 + 1], vao);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ids[i*2 + 1]);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, 
			cpu_buffer + cpu_buffer_offset[i] + image_width[i]*image_height[i]);
		glBindTexture(GL_TEXTURE_2D, 0);

		//test
		//for (int j = 0; j < image_width[i]*image_height[i]; j++)
		//{
		//	float _depth = cpu_buffer[j + cpu_buffer_offset[i] + image_width[i]*image_height[i]];
		//	if (_depth > 0.0f)
		//	{
		//		printf("1");
		//	}
		//	else	printf("0");
		//	if ((j+1)%image_width[i] == 0)
		//	{
		//		printf("\n");
		//	}
		//}
	}
	/*********************render 6 times************************/
	
	glDeleteVertexArrays(1, &vao);
	glDeleteTextures(6, depth_texture_ids);
	glDeleteFramebuffers(6, frame_buffers);
	glDeleteRenderbuffers(6, frame_buffer_depth_buffer);
	glFinish();

	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
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
		glm::vec3((size.x + 0) * cell_size,
		(size.y + 0) * cell_size,
		(size.z + 0) * cell_size);
}

void VoxelMaker::DrawSixTimes(const glm::mat4& vm, const glm::mat4& p, GLuint framebuffer_id, GLuint vao)
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id);
	//绑定帧缓冲区应该在clear前面
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glUseProgram(draw_depth_program_);
	
	GLenum fboBuffers[] = {
		GL_COLOR_ATTACHMENT0
	};
	glDrawBuffers(1, fboBuffers);
	GLint vm_mat_loc = glGetUniformLocation(draw_depth_program_, "vm");
	glUniformMatrix4fv(vm_mat_loc, 1, GL_FALSE, glm::value_ptr(vm));

	GLint p_mat_loc = glGetUniformLocation(draw_depth_program_, "p");
	glUniformMatrix4fv(p_mat_loc, 1, GL_FALSE, glm::value_ptr(p));

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());

	glBindVertexArray(0);
	glUseProgram(0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void VoxelMaker::VoxelizationLogical()
{
	data_buffer_loc_ = new int[width_ * height_ * depth_];
	memset(data_buffer_loc_, 0, width_ * height_ * depth_*sizeof(int));
	glm::ivec3 start_min = glm::ivec3(0, 0, 0);
	glm::ivec3 end_max = glm::ivec3(width_-1, height_-1, depth_-1);

	//ScanMaterials(start_min, end_max);
	
	//前方高能！！！
	
	FillVoxels(start_min, end_max, width_, 0);
	FillVoxels(start_min, end_max, height_, 1);
	FillVoxels(start_min, end_max, depth_, 2);

	glm::ivec3 size = end_max - start_min + glm::ivec3(1, 1, 1);
	int step = (size.y > size.x)? size.x:size.y;
	step = (step > size.z) ? size.z:step;
	step /= 2;
	if (step == 0)
		step = 1;
	bool size_reducing = true;
	
	while (((material_used_number_[NOT_SURE] > 0) || size_reducing) && (step >= 1))
	{
		size_reducing = true;
		for (int i = 0; i < 3; i++)
		{
			if (step < 1)	break;
			glm::ivec3 size = end_max - start_min + glm::ivec3(1, 1, 1);
			int step_init = (size.y > size.x)? size.x:size.y;
			step_init = (step_init > size.z) ? size.z:step_init;
			step_init /= 2;
			if (step_init == 0)
				step_init = 3;
			if (step > step_init)
				step = step_init;
			
			size_reducing &= FillVoxels(start_min, end_max, step, i);
			printf("step : %d, material number : %d, not sure number: %d\n", step, material_count_,
				material_used_number_[NOT_SURE]);
			step --;
		}
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

//static int test[128][128];
bool VoxelMaker::FillVoxels(glm::ivec3& start_min, glm::ivec3& end_max,
	 int scan_step, int step_axis)
{
	int material_count_current = material_count_;
	//ScanMaterials(start_min, end_max);

	glm::ivec3 scan_end_3d = end_max;
	glm::ivec3 scan_start_3d = start_min;

	scan_start_3d[step_axis] = start_min[step_axis];
	scan_end_3d[step_axis] = start_min[step_axis] + scan_step;
	FillPlane(start_min, end_max, (step_axis + 1) % 3, start_min[(step_axis + 1) % 3]);
	FillPlane(start_min, end_max, (step_axis + 2) % 3, start_min[(step_axis + 2) % 3]);
	FillPlane(start_min, end_max, (step_axis + 1) % 3, end_max[(step_axis + 1) % 3]);
	FillPlane(start_min, end_max, (step_axis + 2) % 3, end_max[(step_axis + 2) % 3]);
	//FillPlane(start_min, end_max, step_axis, scan_start_3d[step_axis]);
	while(scan_end_3d[step_axis] <= end_max[step_axis])
	{
		FillPlane(start_min, end_max, step_axis, scan_start_3d[step_axis]);
		FillPlane(start_min, end_max, step_axis, scan_end_3d[step_axis]);
		ScanMaterials(scan_start_3d, scan_end_3d);
		scan_start_3d[step_axis] += scan_step;
		scan_end_3d[step_axis] += scan_step;
		FillPlane(start_min, end_max, (step_axis + 1) % 3, start_min[(step_axis + 1) % 3]);
		FillPlane(start_min, end_max, (step_axis + 2) % 3, start_min[(step_axis + 2) % 3]);
		FillPlane(start_min, end_max, (step_axis + 1) % 3, end_max[(step_axis + 1) % 3]);
		FillPlane(start_min, end_max, (step_axis + 2) % 3, end_max[(step_axis + 2) % 3]);
	}

	ScanMaterials(scan_start_3d, end_max);

	int max_int = 1028;
	glm::ivec3 start_min_t = glm::ivec3(max_int, max_int, max_int);
	glm::ivec3 end_max_t = glm::ivec3(-max_int,-max_int, -max_int);

	for (int i = 0; i < width_*depth_*height_; i++)
	{
		int index = i;
		int x, y, z;
		x = index%width_;
		y = index%(width_*height_) / width_;
		z = index/(width_*height_);
		glm::ivec3 current_index = glm::ivec3(x, y, z);
		if (
			data_buffer_loc_[index] != INSIDE_SURFACE && 
			data_buffer_loc_[index] != OUTSIDE
			)
		{
			if (current_index.x < start_min_t.x)	start_min_t.x = current_index.x;
			if (current_index.y < start_min_t.y)	start_min_t.y = current_index.y;
			if (current_index.z < start_min_t.z)	start_min_t.z = current_index.z;
			if (current_index.x > end_max_t.x)	end_max_t.x = current_index.x;
			if (current_index.y > end_max_t.y)	end_max_t.y = current_index.y;
			if (current_index.z > end_max_t.z)	end_max_t.z = current_index.z;
		}

	}
	bool size_reducing = true;
	bool material_changing = true;
	if (material_count_current - material_count_ == 0)
		material_changing = false;
	if (start_min == start_min_t && end_max == end_max_t)
		size_reducing = false;

	if (start_min_t.x == 1028)
		return false;
	start_min = start_min_t;
	end_max = end_max_t;
	
	return size_reducing || material_changing;
}

//carefully coded
bool VoxelMaker::LocationDepth(DepthDirection& direction, glm::ivec3& location,
	glm::ivec3& start_point, int depth_index, float depth_value, glm::ivec3 start_min, glm::ivec3 end_max)
{
	glm::ivec3 current_size = end_max - start_min + glm::ivec3(1, 1, 1);
	assert(current_size.x > 0 && current_size.y > 0 && current_size.z > 0);

	const int texture_sizes[6] = {
		current_size.x * current_size.y,
		current_size.x * current_size.y,
		current_size.y * current_size.z,
		current_size.y * current_size.z,
		current_size.z * current_size.x,
		current_size.z * current_size.x
	};

	int accumulate_size = texture_sizes[XOY_MIN_Z_UP_Y];
	direction = XOY_MIN_Z_UP_Y;
	
	while(depth_index >= accumulate_size)
	{
		assert(direction < 6);
		int i = direction+1;
		direction = static_cast<VoxelMaker::DepthDirection>(i);
		accumulate_size += texture_sizes[i];
	}

	glm::ivec3 current_flat;

	int current_index = depth_index - (accumulate_size - texture_sizes[direction]/*-1*/);
	if (current_index < 0)	current_index = 0;

	switch (direction)
	{
	case XOY_MAX_Z_UP_Y:
		current_flat.z = end_max.z;
		current_flat.x = /*current_size.x - 1 - */current_index %current_size.x + start_min.x;
		current_flat.y = current_index / current_size.x + start_min.y;
		break;
	case XOY_MIN_Z_UP_Y:
		current_flat.z = start_min.z;
		current_flat.x = current_size.x - 1 - current_index %current_size.x + start_min.x;
		current_flat.y = current_index / current_size.x + start_min.y;
		break;
	case YOZ_MIN_X_UP_Z:
		current_flat.x = start_min.x;
		current_flat.z = current_index % current_size.z + start_min.z;
		current_flat.y = current_index / current_size.z + start_min.y;
		break;
	case YOZ_MAX_X_UP_Z:
		current_flat.x = end_max.x;
		current_flat.z = current_size.z - 1 - current_index % current_size.z + start_min.z;
		current_flat.y = current_index / current_size.z + start_min.y;
		break;
	case ZOX_MIN_Y_DOWN_X:
		current_flat.y = start_min.y;
		current_flat.z = current_index % current_size.z + start_min.z;
		current_flat.x = current_size.x - 1 - current_index / current_size.z + start_min.x;
		break;
	case ZOX_MAX_Y_DOWN_X:
		current_flat.y = end_max.y;
		current_flat.z = current_size.z - 1 - current_index % current_size.z + start_min.z;
		current_flat.x = current_size.x - 1 - current_index / current_size.z + start_min.x;
		break;
	}

	start_point = current_flat;

	if (depth_value <= 0.0f)
	{
		location = start_point + Forward_Sum[direction] * (current_size[(direction/2 + 2)%3]-1);
		return false;
	}
	glm::ivec3 end_point = start_point + Forward_Sum[direction] * (current_size[(direction/2 + 2)%3]-1);
	{
		depth_value *= current_size[(direction/2 + 2) % 3];
		int depth_count = (int)(depth_value) ;

		if (depth_value > (float)depth_count)
			depth_count++;
		if (depth_count == 0)
		{
			location = start_point/* + Forward_Sum[direction]*/;
		}
		else if (depth_count == current_size[(direction/2 + 2)%3])
		{
			location = end_point/* + Forward_Sum[direction]*/;
		}
		else
			location = start_point + Forward_Sum[direction] * (depth_count-1);
	}
	return true;
}

int VoxelMaker::FindMaterial(glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	glm::ivec3 assert_int = end_point-start_point;

	int depth_axis = (direction/2 + 2) % 3;

	if ((assert_int[depth_axis] * Forward_Sum[direction][depth_axis]) < 0)
		return NOT_SURE;

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
	int current_state = NOT_SURE;
	while (point_index != end_p)
	{
		int current_index = point_index.x +
			point_index.y * width_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		//point_index[depth_axis] = current_axis_point;
		if (data_buffer_loc_[current_index] != NOT_SURE&&data_buffer_loc_[current_index] != INSIDE_SURFACE)
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
		point_index.y * width_
		+ point_index.z * width_ * height_;
	if (data_buffer_loc_[current_index] != NOT_SURE&&data_buffer_loc_[current_index] != INSIDE_SURFACE)
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
		assert(material_count_ < (1<<20));
		for (int i = 3; i < (1<<20); i++)
		{
			if (material_used_number_[i] == 0)
			{
				current_state = i;
				//map begin/////////////////////////////
				std::map<int, std::set<int> >::iterator it = material_map_.find(current_state);
				if (it == material_map_.end())
				{
					std::set<int> i_set;
					material_map_.insert(std::pair<int, std::set<int> >(current_state, i_set));
				}
				//map end//////////////////////////////
				return current_state;
			}
		}
	}
	return current_state;
}

void VoxelMaker::FillWithMaterial(int material,glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	if (material == NOT_SURE)	return;
	glm::ivec3 assert_int = end_point-start_point;

	int depth_axis = (direction/2 + 2) % 3;
	
	if ((assert_int[depth_axis] * Forward_Sum[direction][depth_axis]) < 0)
		return;

	//int current_axis_point = start_point[depth_axis];
	glm::ivec3 point_index = start_point;
	bool is_outside = false;
	while (point_index != end_point)
	{
		int current_index = point_index.x +
			point_index.y * width_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		int state_before = data_buffer_loc_[current_index];
		//assert(state_before != INSIDE_SURFACE);
		is_outside = false;
		switch (state_before)
		{
		case NOT_SURE:
			FillVoxelWith(material, current_index);
			break;
		case OUTSIDE:
			FillWithOutSide(start_point, end_point, direction);
			is_outside = true;
			break;
		case INSIDE_SURFACE:
			break;
		default:
			if (state_before != material)
			{
				if (material_used_number_[material] < material_used_number_[state_before])
				{
					int tmp = material;
					material = state_before;
					state_before = tmp;
					//FillVoxelWith(material, current_index);
				}
				//for (int i = 0; i < width_*height_*depth_; i++)
				//{
				//	if (data_buffer_loc_[i] == state_before)
				//	{
				//		FillVoxelWith(material, i);
				//		if (material_used_number_[state_before] == 0)	break;
				//	}
				//}
				//map begin/////////////////////////////
				std::map<int, std::set<int> >::iterator it 
					= material_map_.find(state_before);
				assert(it != material_map_.end());
				std::set<int>& state_before_set = it->second;
				std::set<int>::iterator set_it = state_before_set.begin();
				while (set_it != state_before_set.end())
				{
					FillVoxelWith(material, *set_it);
					set_it++;
				}
				state_before_set.clear();
				material_map_.erase(it);
				//map end//////////////////////////////

			}
			break;
		}
		if (is_outside)
			break;
	}
	if (!is_outside)
	{
		assert(point_index == end_point);
		int current_index = point_index.x +
			point_index.y * width_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		//point_index += Forward_Sum[direction];
		int state_before = data_buffer_loc_[current_index];
		//assert(state_before != INSIDE_SURFACE);
		switch (state_before)
		{
		case NOT_SURE:
			FillVoxelWith(material, current_index);
			break;
		case OUTSIDE:
			assert(material == OUTSIDE);
			FillWithOutSide(start_point, end_point, direction);
			break;
		case INSIDE_SURFACE:
			break;
		default:
			if (state_before != material)
			{
				if (material_used_number_[material] < material_used_number_[state_before])
				{
					int tmp = material;
					material = state_before;
					state_before = tmp;
					FillVoxelWith(material, current_index);
				}
				//for (int i = 0; i < width_*height_*depth_; i++)
				//{
				//	if (data_buffer_loc_[i] == state_before)
				//	{
				//		FillVoxelWith(material, i);
				//		if (material_used_number_[state_before] == 0)	break;
				//	}
				//}
				//map begin/////////////////////////////
				std::map<int, std::set<int> >::iterator it 
					= material_map_.find(state_before);
				assert(it != material_map_.end());
				std::set<int>& state_before_set = it->second;
				std::set<int>::iterator set_it = state_before_set.begin();
				while (set_it != state_before_set.end())
				{
					FillVoxelWith(material, *set_it);
					set_it++;
				}
				state_before_set.clear();
				material_map_.erase(it);
				//map end//////////////////////////////
			}
			break;
		}
	}

}

void VoxelMaker::FillWithOutSide(glm::ivec3 start_point, glm::ivec3 end_point, DepthDirection direction)
{
	int depth_axis = (direction/2 + 2) % 3;
	glm::ivec3 assert_int = end_point-start_point;

	//assert(start_point[depth_axis] < width_);
	if ((assert_int[depth_axis] * Forward_Sum[direction][depth_axis]) < 0)
		return;
	glm::ivec3 point_index = start_point;

	while (point_index != end_point)
	{
		int current_index = point_index.x +
			point_index.y * width_
			+ point_index.z * width_ * height_;
		//current_axis_point++;
		point_index += Forward_Sum[direction];
		int state_before = data_buffer_loc_[current_index];
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
				//for (int i = 0; i < width_*height_*depth_; i++)
				//{
				//	if (data_buffer_loc_[i] == state_before)
				//	{
				//		FillVoxelWith(OUTSIDE, i);
				//		if (material_used_number_[state_before] == 0)	break;
				//	}
				//}
				//map begin/////////////////////////////
				std::map<int, std::set<int> >::iterator it
					= material_map_.find(state_before);
				assert(it != material_map_.end());
				std::set<int>& state_before_set = it->second;
				std::set<int>::iterator set_it = state_before_set.begin();
				while (set_it != state_before_set.end())
				{
					FillVoxelWith(OUTSIDE, *set_it);
					set_it++;
				}
				state_before_set.clear();
				material_map_.erase(it);
				//map end//////////////////////////////
			}
			break;
		}
	}
	//
	int current_index = point_index.x +
		point_index.y * width_
		+ point_index.z * width_ * height_;
	//current_axis_point++;
	//point_index += Forward_Sum[direction];
	int state_before = data_buffer_loc_[current_index];
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
			//for (int i = 0; i < width_*height_*depth_; i++)
			//{
			//	if (data_buffer_loc_[i] == state_before)
			//	{
			//		FillVoxelWith(OUTSIDE, i);
			//		if (material_used_number_[state_before] == 0)	break;
			//	}
			//}

			//map begin/////////////////////////////
			std::map<int, std::set<int> >::iterator it
				= material_map_.find(state_before);
			assert(it != material_map_.end());
			std::set<int>& state_before_set = it->second;
			std::set<int>::iterator set_it = state_before_set.begin();
			while (set_it != state_before_set.end())
			{
				FillVoxelWith(OUTSIDE, *set_it);
				set_it++;
			}
			state_before_set.clear();
			material_map_.erase(it);
			//map end//////////////////////////////
		}
		break;
	}
}


void VoxelMaker::FillInsideSurface(glm::ivec3 point_index)
{
	int index = point_index.x +
		point_index.y * width_
		+ point_index.z * width_ * height_;
	//test
	//if (point_index.z == 15)
	//{
	//	test[point_index.y][point_index.x] = 1;
	//}

	FillVoxelWith(INSIDE_SURFACE, index);
}

int VoxelMaker::Find_Hash(const int x,const int y,const int z)
{
	return (x + y * width_ + z * width_ * height_);
}

void VoxelMaker::FillVoxelWith(int material, int index)
{
	if (material == NOT_SURE)
		return;
	int current_state = data_buffer_loc_[index];
	material_used_number_[current_state]--;
	if (material_used_number_[current_state] <= 0)	material_count_--;
	data_buffer_loc_[index] = material;
	if (material_used_number_[material] <= 0) material_count_++;
	material_used_number_[material]++;
	//map begin/////////////////////////////
	if (material >= 3)
	{
		std::map<int, std::set<int> >::iterator it = material_map_.find(material);
		assert(it != material_map_.end());
		std::set<int>& this_set = it->second;
		this_set.insert(index);
	}
	//map end///////////////////////////////
}

void VoxelMaker::ScanMaterials(const glm::ivec3 start_min, const glm::ivec3 end_max)
{
	float* depth_ptr = DrawDepth(start_min, end_max);
	glm::ivec3 size = end_max - start_min + glm::ivec3(1,1,1);
	if ((size.x < 1) || (size.y < 1) || (size.z < 1))
		return;
	glm::ivec3 current_surface_loc, start_point;
	DepthDirection direction;
	
	int size_i = size.x*size.y*2 + size.x*size.z*2+size.y*size.z*2;
	
	//memset(test, 0, sizeof(int)*128*128);
	for (int i = 0; i < size_i; i++)
	{
		bool is_inside = LocationDepth(direction, current_surface_loc, 
			start_point, i, depth_ptr[i], start_min, end_max);
		if (current_surface_loc.x < 0 || current_surface_loc.y < 0 || current_surface_loc.z < 0
			||current_surface_loc.x >= width_ || current_surface_loc.y >= height_ || current_surface_loc.z >= depth_ ||
			start_point.x < 0 || start_point.y < 0 || start_point.z < 0||
			start_point.x >= width_ || start_point.y >= height_ || start_point.z >= depth_)
		{
			printf("%d", i);
			system("pause");
		}

		int start_hash = Find_Hash(start_point.x, start_point.y, start_point.z);
		while (data_buffer_loc_[start_hash] == INSIDE_SURFACE)
		{	
			start_point += Forward_Sum[direction];
			start_hash = Find_Hash(start_point.x, start_point.y, start_point.z);
		}

		if (is_inside)
		{
			//test[current_surface_loc.x][current_surface_loc.y] = 1;
			FillInsideSurface(current_surface_loc);
			if (start_point != current_surface_loc)
			{
				int current_hash = Find_Hash(current_surface_loc.x, current_surface_loc.y, current_surface_loc.z);
				while (data_buffer_loc_[current_hash] == INSIDE_SURFACE)
				{
					current_surface_loc += Backward_Sum[direction];
					current_hash = Find_Hash(current_surface_loc.x, current_surface_loc.y, current_surface_loc.z);
				}
				int material = FindMaterial(start_point, current_surface_loc, direction);
				FillWithMaterial(material, start_point, current_surface_loc, direction);
			}
		}
		else
		{
			int current_hash = Find_Hash(current_surface_loc.x, current_surface_loc.y, current_surface_loc.z);
			while (data_buffer_loc_[current_hash] == INSIDE_SURFACE)
			{
				current_surface_loc += Backward_Sum[direction];
				current_hash = Find_Hash(current_surface_loc.x, current_surface_loc.y, current_surface_loc.z);
			}
			int material = FindMaterial(start_point, current_surface_loc, direction);
			FillWithMaterial(material, start_point, current_surface_loc, direction);
		}
		if (material_count_ > (1<<20))	break;

	}
	//printf("material no : %d ;\n", material_count_);
	//for (int j = 0; j < height_; j++)
	//{
	//	for (int i = 0; i < width_; i ++)
	//	{
	//		printf("%d", test[j][i]);
	//	}
	//	printf("\n");
	//}

	delete depth_ptr;
}

bool VoxelMaker::FindNotSureOnPlane(const glm::ivec3 start_min, const glm::ivec3 end_max,
	int direction_axis, int direction_index, 
	glm::ivec3& point_coord)
{
	int z_axis = direction_axis;
	int x_axis = (direction_axis + 1) % 3;
	int y_axix = (direction_axis + 2) % 3;
	glm::ivec3 coord = start_min;
	coord[z_axis] = direction_index;

	for (int i = start_min[x_axis]; i <= end_max[x_axis]; i++)
	{
		for (int j = start_min[y_axix]; j <= end_max[y_axix]; j++)
		{
			coord[x_axis] = i;	coord[y_axix] = j;
			int current_index = Find_Hash(coord.x, coord.y, coord.z);
			if (data_buffer_loc_[current_index] == NOT_SURE)
			{
				point_coord = coord;
				return true;
			}
		}
	}
	point_coord = start_min;
	return false;
}

//hard coding....
void VoxelMaker::FillPlane(const glm::ivec3 start_min, 
	const glm::ivec3 end_max, int direction_axis, int direction_index)
{
	int z_axis = direction_axis;
	int x_axis = (direction_axis + 1) % 3;
	int y_axix = (direction_axis + 2) % 3;

	glm::ivec3 point_0, point_1, point_2, point_3, point_4, point_5, point_6, point_7, point_8;
	bool has_not_sure = FindNotSureOnPlane(start_min, end_max, 
		direction_axis, direction_index, point_0);
	while (has_not_sure)
	{
		point_1 = point_2 = point_3 = point_4 = point_5 = point_6 = point_7 = point_8 = point_0;
		point_1[x_axis] = start_min[x_axis];
		point_1[y_axix] = start_min[y_axix];

		point_2[x_axis] = end_max[x_axis];

		point_3[x_axis] = start_min[x_axis];
		
		point_4[x_axis] = end_max[x_axis];
		point_4[y_axix] = end_max[y_axix];

		point_5[y_axix] = start_min[y_axix];

		point_6[y_axix] = end_max[y_axix];

		point_7[x_axis] = end_max[x_axis];
		point_7[y_axix] = start_min[y_axix];

		point_8[x_axis] = start_min[x_axis];
		point_8[y_axix] = end_max[y_axix];

		ScanMaterials(point_0, point_2);
		ScanMaterials(point_3, point_0);
		ScanMaterials(point_0, point_6);
		ScanMaterials(point_5, point_0);
		ScanMaterials(point_6, point_4);
		ScanMaterials(point_8, point_6);
		ScanMaterials(point_1, point_5);
		ScanMaterials(point_5, point_7);
		ScanMaterials(point_1, point_3);
		ScanMaterials(point_3, point_8);
		ScanMaterials(point_7, point_2);
		ScanMaterials(point_2, point_4);

		ScanMaterials(point_3, point_6);
		ScanMaterials(point_0, point_4);
		ScanMaterials(point_1, point_0);
		ScanMaterials(point_5, point_2);

		ScanMaterials(point_1, point_2);
		ScanMaterials(point_3, point_4);

		ScanMaterials(point_1, point_4);

		has_not_sure = FindNotSureOnPlane(start_min, end_max,
			direction_axis, direction_index, point_0);
	}
}

unsigned char* VoxelMaker::CreatEasyVoxel()
{
	unsigned char* data_loc = new unsigned char[width_ * height_ * depth_ / 8 
		* sizeof(unsigned char)];

	//for (int i = 0; i < width_*height_*depth_/8; i++)
	//{
	//	int current_material;
	//	unsigned char easy_material;
	//	unsigned char packed_material = 0;
	//	for (int j = 0; j < 8; j++)
	//	{
	//		current_material = data_buffer_loc_[i*8 + j];
	//		switch (current_material)
	//		{
	//		case OUTSIDE:
	//			easy_material = 0;
	//			break;
	//		case NOT_SURE:
	//		case INSIDE_SURFACE:
	//		default:
	//			easy_material = 1;
	//			break;
	//		}
	//		packed_material <<= 1;
	//		if (easy_material) {
	//			packed_material |= easy_material;
	//		}
	//	}
	//	data_loc[i] = packed_material;
	//}

	for (int d = 0; d < depth_/8; d++)
	{
		for (int i = 0; i < width_*height_; i ++)
		{
			int current_material;
			unsigned char easy_material;
			unsigned char packed_material = 0;
			for (int j = 0; j < 8; j++)
			{
				current_material = data_buffer_loc_[i + (j+8*d)*width_*height_];
				switch (current_material)
				{
				case OUTSIDE:
					easy_material = 0;
					break;
				case NOT_SURE:
				case INSIDE_SURFACE:
				default:
					easy_material = 1;
					break;
				}
				packed_material <<= 1;
				if (easy_material) {
					packed_material |= easy_material;
				}
			}
			data_loc[i+width_*height_*d] = packed_material;
		}
	}
	//delete data_buffer_loc_;
	//for (int i = 0; i < width_*height_*depth_/8; i++)
	//{
	//	printf("%d\t", data_loc[i]);
	//	if ((i+1)%(width_/8) == 0)	printf("\n");
	//	if ((i+1)%(width_*height_/8) == 0)	printf("\n");
	//}
	return data_loc;
}