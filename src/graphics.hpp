#ifndef __GRAPHICS_HPP
#define __GRAPHICS_HPP

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h> //for malloc/free
#include <math.h>
#include <string.h>
#include <ft2build.h>
#include <thread>
#include <iostream>
#include <unistd.h>
#include FT_FREETYPE_H

#define WIND_DEPTH 60
#define REFRESH_RATE 10

/* Lighting conditions */
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 20.0f, 100.0f, 100.0f, 20.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 110.0f };

class graphics {
        std::thread graphics_thread;
        int init = 0;
    public:
        /* public function */
        int graphics_main(int argc, char** argv);    
};

#endif // __GRAPHICS_HPP

