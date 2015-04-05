#ifndef CAMERA
#define CAMERA

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
//#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>

extern int buttenState = GLUT_LEFT_BUTTON;
extern int ox = 0, oy = 0;
//GLfloat camera_trans[] = {0.0f, 0.0f, -4.0f};
//GLfloat camera_rot[] = {0.0f, 0.0f, 0.0f};
//GLfloat camera_trans_offset[] = {0.0f , 0.0f, -4.0f};
//GLfloat camera_rot_offset[] = {0.0f, 0.0f, 0.0f};
extern const GLfloat offset = 0.1f;
extern glm::vec3 camera_trans = glm::vec3(0.0f, 0.0f, -4.0f);
extern glm::vec3 camera_rot = glm::vec3(0.0f, 0.0f, 0.0f);
extern glm::vec3 camera_trans_offset = glm::vec3(0.0f, 0.0f, -4.0f);
extern glm::vec3 camera_rot_offset = glm::vec3(0.0f, 0.0f, 0.0f);

extern glm::mat4 Model = glm::mat4(1.0f);
extern glm::mat4 View = glm::mat4(1.0f);
extern glm::mat4 Projection = glm::mat4(1.0f);

void cameraDisplay()
{
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//for (int c = 0; c < 3; ++c)
	//{
	//	camera_trans_offset[c] += (camera_trans[c] - camera_trans_offset[c]) * offset;
	//	camera_rot_offset[c] += (camera_rot[c] - camera_rot_offset[c]) * offset;
	//}
	//glTranslatef(camera_trans_offset[0], camera_trans_offset[1], camera_trans_offset[2]);
	//glRotatef(camera_rot_offset[0], 1.0f, 0.0f, 0.0f);
	//glRotatef(camera_rot_offset[1], 0.0f, 1.0f, 0.0f);
	camera_trans_offset += (camera_trans - camera_trans_offset) * offset;
	camera_rot_offset += (camera_rot - camera_rot_offset) * offset;

	glm::mat4 translateView = glm::translate(glm::mat4(1.0f), camera_trans_offset);
	glm::mat4 rotateXView = glm::rotate(translateView, camera_rot_offset.x, glm::vec3(1.0f, 0.0f, 0.0f));
	View = glm::rotate(rotateXView, camera_rot_offset.y, glm::vec3(0.0f, 1.0f, 0.0f));
	//Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
}

void mouse(int butten, int state, int x, int y)
{
	if(state == GLUT_DOWN)
		buttenState |= 1<<butten;
	else if(state == GLUT_UP)
		buttenState = 0;
	ox = x; oy = y;

	glutPostRedisplay();
}

void motion(int x, int y)
{
	GLfloat dx, dy;
	dx = (GLfloat)(x - ox); dy = (GLfloat)(y - oy);
	if(buttenState & 4)//right:z
		camera_trans.z += dy/100.0f * 0.5f * fabs(camera_trans.z);
	else if (buttenState & 2)//middle:x/y translate
	{
		camera_trans.x += dx/100.0f;
		camera_trans.y -= dy/100.0f;
	}
	else if (buttenState & 1)//left:rotate
	{
		camera_rot.x += dy/5.0f;
		camera_rot.y += dx/5.0f;
	}
	ox = x; oy = y;

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
}

void cameraLoop()
{
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
}

#endif