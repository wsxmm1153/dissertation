#include "glincludes.h"
#include "camera.h"
#include <stdio.h>
#include "volumeRenderer.h"
#include "voxelization.h"
#include "fluidRenderer.h"
#include "sphsimulator.h"
#include <Windows.h>

static GLint screen_width, screen_height;
float stime;
char sfps[40] = "MiaoMiPhysics, fps:  ";
static float total = 0;	
static unsigned int tcount = 0;
int scount = 0;
float initTime;
//static VoxelMaker* voxel_maker_ptr_s;
//static VoxelStructure* voxel_struct_ptr;
static SPHSimulator* simulator_ptr;
static FluidRenderer* renderer_ptr;
static GLuint texName;
static int _x,_y,_z;
//box and its index

static GLfloat vertice[] ={
	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f
};

static GLfloat color[] = {
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f,
	0.0f, 1.0f, 1.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f
};

static GLfloat texcoord[] = {
	0.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 1.0f,
	0.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 1.0f
};

static GLushort index[] = 
{
	3, 2, 6, 7,
	1, 5, 6, 2,
	6, 5, 4, 7,
	5, 1, 0, 4,
	0, 3, 7, 4,
	0, 1, 2, 3
};

//the file is .raw
bool loadTextures() {

	/*	FILE *pFile = fopen(RAWFILENAME,"rb");
	if (NULL == pFile) {
	return false;
	}

	int size = XMAX*YMAX*ZMAX;
	unsigned char *pVolume = new unsigned char[size];
	bool ok = (size == fread(pVolume,sizeof(unsigned char), size,pFile));
	fclose(pFile);*/	
	
	//voxel_struct_ptr->get_size(_x, _y, _z);

	//texName = voxel_struct_ptr->Creat3DTexture();

	//glBindTexture(GL_TEXTURE_3D, texName);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//VoxelMaker::SaveToFile(voxel_struct_ptr, ".\\voxelfiles\\earth_voxel_1024.txt");

	return true;
}

void init(void)
{
	glewInit();
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	//glShadeModel(GL_SMOOTH);
	/***************test voxel maker*************************/
	//voxel_struct_ptr = VoxelMaker::MakeObjToVoxel("earth.obj", 1024);
	//voxel_struct_ptr = VoxelMaker::LoadVoxelFromFile(".\\voxelfiles\\rabbit_voxel_256.txt");
	/***************test voxel maker*************************/

	/***************test renderer*************************/
	//loadTextures();
	/***************test renderer*************************/

	/***************test simulator*************************/
	simulator_ptr = new SPHSimulator();
	
	simulator_ptr->InitGPUResource(NUM, glm::vec3(1.0f, 1.0f, 1.0f),
		SMOOTH_LENGTH);
	simulator_ptr->InitSimulation();
	GLuint pos_vbo = simulator_ptr->gpu_particles_ptr_->positions_vbo();
	GLint* number_ptr = simulator_ptr->gpu_particles_ptr_->particle_number_ptr();
	renderer_ptr = new FluidRenderer(&screen_width, &screen_height ,pos_vbo, number_ptr);
	//renderer_ptr->InitPointDraw();
	renderer_ptr->InitScreenSpaceDraw();
	renderer_ptr->InitScene("2.obj");
	/***************test simulator*************************/
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.1f);
	
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	cameraDisplay();
	/***************test renderer*************************/
	//glm::mat4 mv = View * Model;
	//glm::mat4 p = Projection;
	//int _x,_y,_z;
	//voxel_maker_ptr_s->GetSize(_x, _y, _z);
	//VolumeRenderer::drawPhysicsWorld(texName, &mv, &p,XMAX, YMAX, ZMAX);
	//VolumeRenderer::drawPhysicsWorld(texName, &mv, &p,_x, _y, _z);
	/***************test renderer*************************/

	/***************test voxel maker*************************/
	//int _x,_y,_z;
	//voxel_maker_ptr_s->GetSize(_x, _y, _z);
	//voxel_maker_ptr_s->DrawDepth(glm::ivec3(0, 0, 0), glm::ivec3(_x, _y, _z));
	/***************test voxel maker*************************/

	/***************test simulator*************************/
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(Model, glm::vec3(-1.0f, -1.0f, -1.0f));
	model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
	//model = glm::translate(model, glm::vec3(-0.25f, -0.25f, -0.25f));
	glm::mat4 mv = View * model;
	
	glm::mat4 p = Projection;
	simulator_ptr->display(TIME_STEP);
	glFinish();
	//renderer_ptr->DrawPoints(&mv, &p);
	//renderer_ptr->DrawScreenSpace(&mv, &p);
	glFinish();
	glm::vec3 center = renderer_ptr->scene_min_ + 0.5f * renderer_ptr->scene_size_;
	float scale_v = 0.49f/(renderer_ptr->scene_size_.x); 
	glm::mat4 s_model = glm::mat4(1.0f);
	//s_model = glm::translate(s_model, center);
	s_model = glm::scale(s_model, glm::vec3(scale_v, scale_v, scale_v));
	//s_model = glm::translate(s_model, -center);
	s_model = glm::translate(s_model, -renderer_ptr->scene_min_);

	glm::mat4 s_mv = mv*s_model;
	renderer_ptr->DrawScene(&s_mv, &p);
	renderer_ptr->DrawScreenSpace(&mv, &p);
	renderer_ptr->OutPut();
	/***************test simulator*************************/
	//glutWireCube(1.0f);


	float t = (float)timeGetTime();
	float dt = t - stime;
	total += dt*0.001f;
	stime = t;
	if(total > 1.0f)
	{
		sprintf(&sfps[15], "%f", tcount*1.0f);
		total = 0;
		tcount = 0;
	}
	tcount ++;
	scount ++;
	if (scount == 150)
	{
		scount = 0;
	}
	glutSetWindowTitle(sfps);

	glutPostRedisplay();
	glutSwapBuffers();
}

void keybord(unsigned char key, int x, int y)
{
	if (key == 32)
	{
		simulator_ptr->add_particle_ = true;
		float shutdownTime = (float)timeGetTime();
		float meanFps = 1000.0f * scount / (shutdownTime - initTime);
		std::cout << "meanFPS: " << meanFps << '\n' << std::endl;

		scount = 0;
		initTime = (float)timeGetTime();
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	if(h == 0)
		h = 1;
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(60.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);
	//glMatrixMode(GL_MODELVIEW);
	Projection = glm::perspective(60.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f);

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	screen_width = w;
	screen_height = h;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	glutKeyboardFunc(keybord);
	init();
	glutDisplayFunc(display);
	initTime = (float)timeGetTime();
	glutReshapeFunc(reshape);
	cameraLoop();
	
	glutMainLoop();

	return 0;
}
