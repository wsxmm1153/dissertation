#include "glincludes.h"
#include "camera.h"
#include <stdio.h>
#include "volumeRenderer.h"
#include "voxelization.h"

//static VoxelMaker* voxel_maker_ptr_s;
static VoxelStructure* voxel_struct_ptr;
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
	
	voxel_struct_ptr->get_size(_x, _y, _z);

	texName = voxel_struct_ptr->Creat3DTexture();

	glBindTexture(GL_TEXTURE_3D, texName);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	return true;
}

void init(void)
{
	glewInit();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	/***************test voxel maker*************************/
	voxel_struct_ptr = VoxelMaker::MakeObjToVoxel("2.obj", 128);
	/***************test voxel maker*************************/

	/***************test renderer*************************/
	loadTextures();
	/***************test renderer*************************/
}

void display()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	/***************test renderer*************************/
	cameraDisplay();
	glm::mat4 mv = View * Model;
	glm::mat4 p = Projection;
	//int _x,_y,_z;
	//voxel_maker_ptr_s->GetSize(_x, _y, _z);
	//VolumeRenderer::drawPhysicsWorld(texName, &mv, &p,XMAX, YMAX, ZMAX);
	VolumeRenderer::drawPhysicsWorld(texName, &mv, &p,_x, _y, _z);
	/***************test renderer*************************/

	/***************test voxel maker*************************/
	//int _x,_y,_z;
	//voxel_maker_ptr_s->GetSize(_x, _y, _z);
	//voxel_maker_ptr_s->DrawDepth(glm::ivec3(0, 0, 0), glm::ivec3(_x, _y, _z));
	/***************test voxel maker*************************/

	glutPostRedisplay();
	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(argv[0]);
	init();

	glutDisplayFunc(display);
	cameraLoop();

	glutMainLoop();

	return 0;
}
