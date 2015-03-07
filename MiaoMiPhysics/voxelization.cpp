#include "voxelization.h"
#include <limits>
#include "shader.h"
#include "glincludes.h"
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
	vertice_buffer_handel_(0)
{
	//draw_depth_program_ = compileProgram(drawDepthVertex, drawDepthFragment);
	draw_depth_program_ = compileProgram(commonVertex, commonFragment);
	glGenBuffers(1, &vertice_buffer_handel_);
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

	//GLuint vao;
	glGenVertexArrays(1, &(voxel_maker_ptr->vao_));
	glBindVertexArray(voxel_maker_ptr->vao_);
	glBindBuffer(GL_ARRAY_BUFFER, voxel_maker_ptr->vertice_buffer_handel_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * (voxel_maker_ptr->vertices_).size(), vertice_data, GL_STATIC_DRAW);
	GLint vertice_loc = glGetAttribLocation(voxel_maker_ptr->draw_depth_program_, "vVertex");
	glEnableVertexAttribArray(vertice_loc);
	if (vertice_loc >= 0)
		glVertexAttribPointer(vertice_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete vertice_data;

	normals.clear();
	texcoords.clear();

	//生成体数据
	voxel_maker_ptr->SetSize(voxel_size);
	//暂定
	return voxel_maker_ptr;
}

void VoxelMaker::FindBoundingBox()
{
	float X_max = numeric_limits<float>::min(),
		X_min = numeric_limits<float>::max(),
		Y_max = numeric_limits<float>::min(),
		Y_min = numeric_limits<float>::max(),
		Z_max = numeric_limits<float>::min(), 
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
			if (abs(box_size_f[i] - (float)size_i) < 10e-10)
			{
				box_size_i[i] = size_i;
			}
			else
			{
				box_size_i[i] = size_i + 1;
				vertices_max_[i] = (float)box_size_i[i] / scale_f; 
			}
		}
	}

	width_ = box_size_i[0];
	height_ = box_size_i[1];
	depth_ = box_size_i[2];

	data_buffer_loc_ = new unsigned char[width_ * height_ * depth_];
}

void VoxelMaker::FindMiddle(glm::vec3 current_max, glm::vec3 current_min, glm::vec3& middle_position)
{
	glm::vec3 v_max = current_max;
	glm::vec3 v_min = current_min;
	
	middle_position = (v_max + v_min) / 2.0f;
}

float* VoxelMaker::DrawDepth(glm::ivec3 start_min, glm::ivec3 size, glm::mat4 *mv,
	glm::mat4 *p)
{
	glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glm::mat4 model = glm::mat4(1.0f);
	glm::vec3 eye, up, look_at;
	glm::vec3 vertice_max, vertice_min;
	FindBoundingBox(vertice_max, vertice_min, size, start_min);
	FindMiddle(vertice_max, vertice_min, look_at);
	eye = glm::vec3(0.0f, 0.0f, -1000.0f);
	up = glm::vec3(0.0f, 1000.0f, 0.0f);

	//glm::mat4 view = glm::lookAt(eye, look_at, up);
	//glm::mat4 projection = glm::ortho(vertice_min.x, vertice_max.x, vertice_min.y, vertice_max.y, vertice_min.z, vertice_max.z);
	//glm::mat4 view = View * Model;
	//glm::mat4 projection = projection;
	//GLuint vao;
	//glGenVertexArrays(1, &vao);
	//glBindVertexArray(vao);
	//glBindBuffer(GL_ARRAY_BUFFER, vertice_buffer_handel_);
	//GLint vertice_loc = glGetAttribLocation(draw_depth_program_, "vVertex");
	//glEnableVertexAttribArray(vertice_loc);
	//if (vertice_loc >= 0)
	//	glVertexAttribPointer(vertice_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);

	glUseProgram(draw_depth_program_);
	
	GLint mv_mat_loc = glGetUniformLocation(draw_depth_program_, "mv");
	glUniformMatrix4fv(mv_mat_loc, 1, GL_FALSE, glm::value_ptr(*mv));
	GLint p_mat_loc = glGetUniformLocation(draw_depth_program_, "p");
	glUniformMatrix4fv(mv_mat_loc, 1, GL_FALSE, glm::value_ptr(*p));

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
	
	glBindVertexArray(0);
	glUseProgram(0);
	return NULL;
}

void VoxelMaker::FindBoundingBox(glm::vec3& vertices_max, glm::vec3& vertices_min,
	glm::ivec3 size, glm::ivec3 min_start)
{
	vertices_max = vertices_max_;
	vertices_min = vertices_min_;
	float cell_size = vertices_max_.x - vertices_min_.x;
	cell_size /= (float)width_;
	vertices_min += glm::vec3(min_start.x * cell_size,
		min_start.y * cell_size,
		min_start.z * cell_size);

	vertices_max = vertices_min + 
		glm::vec3(size.x * cell_size,
		size.y * cell_size,
		size.z * cell_size);
}