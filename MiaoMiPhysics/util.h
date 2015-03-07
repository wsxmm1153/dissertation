#ifndef UTIL_H
#define UTIL_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
using namespace std;

class Util
{
    public:
        Util();
        virtual ~Util();
        bool loadOBJ(const char * path,std::vector<glm::vec3> & out_vertices,std::vector<glm::vec2> & out_uvs,std::vector<glm::vec3> & out_normals);
        char *textFileRead(char *fn);
    protected:
    private:
};

#endif // UTIL_H
