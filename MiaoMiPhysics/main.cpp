#include "glincludes.h"
#include "camera.h"
#include <stdio.h>
#include "volumeRenderer.h"
#include "voxelization.h"

static VoxelMaker* voxel_maker_ptr_s;
static GLuint texName;
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

	//FILE *pFile = fopen(RAWFILENAME,"rb");
	//if (NULL == pFile) {
	//	return false;
	//}

	//int size = XMAX*YMAX*ZMAX;
	//unsigned char *pVolume = new unsigned char[size];
	//bool ok = (size == fread(pVolume,sizeof(unsigned char), size,pFile));
	//fclose(pFile);	
	int _x,_y,_z;
	voxel_maker_ptr_s->GetSize(_x, _y, _z);
	unsigned char *pVolume = voxel_maker_ptr_s->data_buffer_loc_;
	for (int i = 0; i < _x*_y*_z; i++)
	{
		if (pVolume[i] == 2)
		{
			pVolume[i] = 0;
		}
		else if (pVolume[i] == 1)
		{
			pVolume[i] = 207;
		}
		else
			pVolume[i] = 207;
	}
	glGenTextures(1,&texName);

	glBindTexture(GL_TEXTURE_3D,texName);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,_x,_y,_z,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,pVolume);
	//glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,XMAX,YMAX,ZMAX,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,pVolume);

	//delete [] pVolume;
	delete voxel_maker_ptr_s;

	return true;
}

void init(void)
{
	glewInit();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	/***************test voxel maker*************************/
	voxel_maker_ptr_s = VoxelMaker::MakeObjToVoxel("2.obj", 32);
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
	VolumeRenderer::drawPhysicsWorld(texName, &mv, &p);
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
