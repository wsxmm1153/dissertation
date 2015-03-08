#ifndef _GL_INCLUDE_S_
#define _GL_INCLUDE_S_

#define SCREENHEIGHT 768
#define SCREENWIDTH 1024

//raw files
//******************************************
//#define RAWFILENAME "Bucky32X32X32.raw"
//#define XMAX 32
//#define YMAX 32
//#define ZMAX 32
//******************************************
#define RAWFILENAME "Engine256X256X256.raw"
#define XMAX 256
#define YMAX 256
#define ZMAX 256
//******************************************
//#define RAWFILENAME "CTA512X512X79.raw"
//#define XMAX 512
//#define YMAX 512
//#define ZMAX 79
//******************************************
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

#endif
