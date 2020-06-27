//
//  main.cpp
//  HW6
//
//  Created by 승우 on 2020/04/08.
//  Copyright © 2020 승우. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLUT/glut.h>
#include "LoadShaders.h"
#include <vector>

#define _USE_MATH_DEFINES // to use M_PI
#include <math.h>

#include <string.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#define FPUSH_VTX3(p,vx,vy,vz)\
do {\
    p.push_back(vx); \
    p.push_back(vy); \
    p.push_back(vz); \
} while(0)

#define FSET_VTX3(vx, vy, vz, valx, valy, valz)\
do {\
    vx = (float)(valx);\
    vy = (float)(valy);\
    vz = (float)(valz);\
} while(0)

typedef std::vector<GLfloat> GLvec;

int show_wireframe = 0, show_vertices = 0;
int num_of_vertices = 0;

GLuint vao, vbo[2];

GLuint program;
int draw_mode = 0;
GLvec vertices, colors;
GLfloat r = 0.5;
GLint subh = 30;
GLint suba = 30;

void init();
void keyboard(unsigned char key, int x, int y);
void get_sphere_3d(GLvec& p, float r, int subh, int suba);
void display();

int main(int argc, char * argv[]) {
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(700, 700);
    glutCreateWindow("Solar System");
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(-1);
    }
    
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    return 0;
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': draw_mode = 1; show_wireframe = 1; show_vertices = 0; glutPostRedisplay(); break;
        case 'v': draw_mode = 1; show_vertices = 1; show_wireframe = 0; glutPostRedisplay(); break;
    }
}

int build_program() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW6/HW6/solar.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW6/HW6/solar.frag"},
        {GL_NONE, NULL}
    };
    
    GLuint program = LoadShaders(shaders);
    glUseProgram(program);
    return program;
}

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

// Redundant computation -> Store previous value of theta and phi into prev_theta and prev_phi
void get_sphere_3d(GLvec& p, GLfloat r, GLint subh, GLint suba) {
    
    double prev_theta = 0, prev_phi = 0;
    
    // calculating theta (the angle between vector and y-axis)
    for (int i = 1; i <= subh; ++i) {
        
        double theta0 = 0;
        
        if (i == 1) theta0 = M_PI * (i-1) / subh;
        else theta0 = prev_theta;
        
        double theta1 = M_PI * i / subh;
        
        double y0 = r * cos(theta0);
        double rst0 = r * sin(theta0);
        double y1 = r * cos(theta1);
        double rst1 = r * sin(theta1);
        
        prev_theta = theta1;
        // calculating phi (the angle between xz-projected vector and z-axis)
        for (int j = 1; j <= suba; ++j) {
            
            double phi0 = 0;
            
            if (j == 1) phi0 = 2 * M_PI * (j-1) / suba;
            else phi0 = prev_phi;
            
            double phi1 = 2 * M_PI * j / suba;
            
            prev_phi = phi1;
            
            double cp0 = cos(phi0);
            double sp0 = sin(phi0);
            double cp1 = cos(phi1);
            double sp1 = sin(phi1);
            
            float vx0, vy0, vz0, vx1, vy1, vz1;
            float vx2, vy2, vz2, vx3, vy3, vz3;
            
            FSET_VTX3(vx0, vy0, vz0, sp0*rst0, y0, cp0*rst0);
            FSET_VTX3(vx1, vy1, vz1, sp0*rst1, y1, cp0*rst1);
            FSET_VTX3(vx2, vy2, vz2, sp1*rst0, y0, cp1*rst0);
            FSET_VTX3(vx3, vy3, vz3, sp1*rst1, y1, cp1*rst1);
            
            if (i < subh) { // first triangle ( v0 - v1 - v3)
                FPUSH_VTX3(p, vx0, vy0, vz0);
                FPUSH_VTX3(p, vx1, vy1, vz1);
                FPUSH_VTX3(p, vx3, vy3, vz3);
            }
            
            if (1 < i) { // second triangle (v3- v2- v0)
                FPUSH_VTX3(p, vx3, vy3, vz3);
                FPUSH_VTX3(p, vx2, vy2, vz2);
                FPUSH_VTX3(p, vx0, vy0, vz0);

            }
        }
    }
}

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset) {
    GLfloat max_val[3] = {-INFINITY, -INFINITY, -INFINITY};
    GLfloat min_val[3] = {INFINITY, INFINITY, INFINITY};
    
    int n = (int)(p.size() / 3);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 3; ++j) {
            GLfloat val = p[i * 3 + j];
            if (max_val[j] < val) max_val[j] = val;
            else if (min_val[j] > val) min_val[j] = val;
        }
    }
    
    GLfloat width[3] = {
        max_val[0] - min_val[0],
        max_val[1] - min_val[1],
        max_val[2] - min_val[2]
    };
    
    c.resize(p.size());
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 3; ++j) {
            int k = i * 3 + j;
            c[k] = std::fminf((p[k] - min_val[j]) / width[j] + offset, 1.0f);
        }
    }
}

void draw_sphere(const GLfloat* trans_mat) {
    GLint location_t = glGetUniformLocation(program, "T");

    glUniformMatrix4fv(location_t, 1, GL_FALSE, trans_mat);
    glDrawArrays(GL_TRIANGLES, 0, num_of_vertices);
}

void init() {
    srand(clock());
    program = build_program();
    
    // get vertices of spheres and colors with respect to vertices
    get_sphere_3d(vertices, r, subh, suba);
    get_color_3d_by_pos(colors, vertices, 0);
    
    num_of_vertices = vertices.size() / 3;
    
    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    // VBO
    glGenBuffers(2, vbo);
    bind_buffer(vbo[0], vertices, program, "vPosition", 3);
    bind_buffer(vbo[1], colors, program, "vColor", 3);
    
    glEnable(GL_DEPTH_TEST); // Enable depth test
    glDepthFunc(GL_LESS); // Accept fragment it is closer to the camera than the existing one
}

void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);
    
    GLint location;
    GLfloat theta = 0.00001f * clock();
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonOffset(1, 1);
    
    
    using namespace glm;
    mat4 T_sun(1.0f);
    T_sun = rotate(T_sun, theta, vec3(0.0f, 1.0f, 0.0f));
    

    mat4 T_earth(1.0f);
    T_earth = rotate(T_earth, theta, vec3(0.0f, 1.0f, 0.0f));
    T_earth = translate(T_earth, vec3(0.8f, 0.0f, 0.0f));
    T_earth = rotate(T_earth, theta, vec3(0.0f, 1.0f, 0.0f));
    T_earth = scale(T_earth, vec3(0.3f));

    mat4 T_moon(1.0f);
    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f));
    T_moon = translate(T_moon, vec3(0.8f, 0.0f, 0.0f));
    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f));
    T_moon = translate(T_moon, vec3(0.2f, 0.0f, 0.0f));
    T_moon = rotate(T_moon, theta, vec3(0.0f, 1.0f, 0.0f));
    T_moon = scale(T_moon, vec3(0.1f));


    location = glGetUniformLocation(program, "draw_mode");
    glUniform1i(location, 0);
    
    draw_sphere(value_ptr(T_sun));
    draw_sphere(value_ptr(T_earth));
    draw_sphere(value_ptr(T_moon));
    glDisable(GL_POLYGON_OFFSET_FILL);
    
    if (show_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1);
        
        glUniform1i(location, 1);
        draw_sphere(value_ptr(T_sun));
        draw_sphere(value_ptr(T_earth));
        draw_sphere(value_ptr(T_moon));
    }
    if (show_vertices) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        glPointSize(3);

        glUniform1i(location, 1);
        draw_sphere(value_ptr(T_sun));
        draw_sphere(value_ptr(T_earth));
        draw_sphere(value_ptr(T_moon));

    }

    glFlush();
    glutSwapBuffers();

    glutPostRedisplay();

}
