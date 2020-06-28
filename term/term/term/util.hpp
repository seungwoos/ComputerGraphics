//
//  util.hpp
//  term
//
//  Created by HyeWon Jeon on 2020/06/08.
//  Copyright © 2020 HyeWon Jeon. All rights reserved.
//

#ifndef util_hpp
#define util_hpp

//#pragma make_public(Camera)

#include <GL/glew.h> // in main.cpp
#include "/Users/ck/Desktop/ComputerGraphics/term/GLUT.framework/Headers/glut.h"
#include <glm/gtc/matrix_transform.hpp>
#include <ode/ode.h>
#include <math.h>
#include <vector>

typedef std::vector<GLfloat> GLvec ;
using namespace glm ;

mat4 parallel(double r, double aspect, double n, double f) ;

void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size) ;
void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size) ;

double dsElapsedTime() ;
mat4 compute_modelling_transf(dBodyID body) ;
mat4 compute_geom_transf(dGeomID geom) ;

struct Camera {
    enum {ORTHOGRAPHIC, PERSPECTIVE} ;
    vec3 eye ;
    vec3 center ;
    vec3 up ;
    float zoom_factor ;
    int projection_mode ;
    float z_near ;
    float z_far ;
    float fovy ;
    float x_right ;
    double aspect ;
    
    Camera() :
//        eye(5, 3, 5),
        eye(10, 6, 0),
        center(0, 0, 0),
        up(0, 1, 0),
        zoom_factor(1.0f),
        projection_mode(PERSPECTIVE),
        z_near(0.01f),
        z_far(100.0f),
        fovy((float)(M_PI/180.0*(30.0))),
        x_right(1.2f),
        aspect(1.0f)
    {}
    
    void init() {
        eye = vec3(10, 6, 0) ;
        center = vec3(0, 0, 0) ;
        up = vec3(0, 1, 0) ;
        zoom_factor = 1.0f ;
    }
    
    mat4 get_viewing() {
        return lookAt(eye, center, up) ;
    }
    
    mat4 get_projection() {
        mat4 P(1.0f) ;
        
        switch (projection_mode) {
            case ORTHOGRAPHIC :
                P = parallel(zoom_factor * x_right, aspect, z_near, z_far) ;
                break;
                
            case PERSPECTIVE :
                P = perspective(float(zoom_factor*(M_PI/180.0*(30.0))), float(aspect), z_near, z_far) ;
                break;
        }
        return P ;
    }
} ;

#endif /* util_hpp */
