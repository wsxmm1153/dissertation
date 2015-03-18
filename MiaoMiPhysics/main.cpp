#include "glincludes.h"
#include "camera.h"
#include <stdio.h>
#include "volumeRenderer.h"
#include "voxelization.h"

static VoxelMaker* voxel_maker_ptr_s;
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
	
	voxel_maker_ptr_s->GetSize(_x, _y, _z);
	int *pVolume = voxel_maker_ptr_s->data_buffer_loc_;

	float* image_data = new float[_x*_y*_z*4];

	for (int i = 0; i < _x*_y*_z; i++)
	{
		if (pVolume[i] == 2)//if outside, black
		{
			image_data[i*4] = 0.0f;
			image_data[i*4+1] = 0.0f;
			image_data[i*4+2] = 0.0f;
			image_data[i*4+3] = 0.0f;
		}

		else if (pVolume[i] == 1)//if inside surface, white
		{
			image_data[i*4] = 1.0f;
			image_data[i*4+1] = 1.0f;
			image_data[i*4+2] = 1.0f;
			image_data[i*4+3] = 0.5f;
		}
		else if (pVolume[i] == 0)//if not sure, red
		{
			image_data[i*4] = 0.1f;
			image_data[i*4+1] = 0.0f;
			image_data[i*4+2] = 0.0f;
			image_data[i*4+3] = 0.1f;
		}
		else//other materials, blue or green
		{
			float color = (glm::atan((float)pVolume[i])
				+ 3.1416f/2.0f) / 3.1416f;
			//color = 0.0f;
			if (color > 0.5f)
			{
				image_data[i*4] = 0.0f;
				image_data[i*4+1] = color;
				image_data[i*4+2] = 0.0f;
				image_data[i*4+3] = 0.1f;
			}
			else
			{
				image_data[i*4] = 0.0f;
				image_data[i*4+1] = 0.0f;
				image_data[i*4+2] = color;
				image_data[i*4+3] = 0.1f;
			}
		}
	}
	delete voxel_maker_ptr_s;

	glGenTextures(1,&texName);

	glBindTexture(GL_TEXTURE_3D,texName);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,_x,_y,_z,0,GL_LUMINANCE,GL_INT,pVolume);
	//glTexImage3D(GL_TEXTURE_3D,0,GL_INTENSITY,XMAX,YMAX,ZMAX,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,pVolume);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, _x, _y, _z, 0, GL_RGBA, GL_FLOAT, image_data);
	//glTextureImage3DEXT(texName, GL_TEXTURE_3D,0,GL_INTENSITY,_x,_y,_z,0,
	//	GL_LUMINANCE,GL_UNSIGNED_BYTE,pVolume);
	//delete [] pVolume;

	glBindTexture(GL_TEXTURE_3D, 0);
	
	delete image_data;

	return true;
}

void init(void)
{
	glewInit();
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_SMOOTH);
	/***************test voxel maker*************************/
	voxel_maker_ptr_s = VoxelMaker::MakeObjToVoxel("2.obj", 290);
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
