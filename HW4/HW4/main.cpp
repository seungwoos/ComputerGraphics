//
//  main.cpp
//  HW4
//
//  Created by 승우 on 2020/03/27.
//  Copyright © 2020 승우. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLUT/glut.h>
#include "LoadShaders.h"
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

typedef std::vector<GLfloat> GLvec;

GLuint vao, vbo[2];
GLvec vertices, colors;

GLfloat scaling_factor = 1.0f;
GLuint drawing_mode = 0;

GLuint program;

void init();
void keyboard(unsigned char key, int x, int y);
void display();

int main(int argc, char * argv[]) {
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Rotating Cube");
    
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
        case '1': drawing_mode = 0; glutPostRedisplay(); break;
        case '2': drawing_mode = 1; glutPostRedisplay(); break;
        case '3': drawing_mode = 2; glutPostRedisplay(); break;
        case '+': scaling_factor += 0.1; glutPostRedisplay(); break;
        case '-': scaling_factor -= 0.1; glutPostRedisplay(); break;
    }
}

int build_program() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW4/HW4/cubes.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW4/HW4/cubes.frag"},
        {GL_NONE, NULL}
    };
    
    GLuint program = LoadShaders(shaders);
    glUseProgram(program);
    return program;
}

void get_box_3d(GLvec &p, GLfloat lx, GLfloat ly, GLfloat lz) {
    static const GLfloat box_vertices[] = {
         0.5f,  0.5f, -0.5f,    -0.5f, -0.5f, -0.5f,    -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,     0.5f, -0.5f, -0.5f,    -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,    -0.5f, -0.5f,  0.5f,    -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,    -0.5f,  0.5f,  0.5f,    -0.5f,  0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,    -0.5f, -0.5f, -0.5f,     0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,    -0.5f, -0.5f,  0.5f,    -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,    -0.5f, -0.5f,  0.5f,     0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,    -0.5f,  0.5f,  0.5f,     0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,     0.5f, -0.5f, -0.5f,     0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,     0.5f,  0.5f,  0.5f,     0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,     0.5f,  0.5f, -0.5f,    -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,    -0.5f,  0.5f, -0.5f,    -0.5f,  0.5f,  0.5f,
    };
    
    p.resize(sizeof(box_vertices) / sizeof(GLfloat));
    memcpy(p.data(), box_vertices, sizeof(box_vertices));
    size_t n = p.size()/3;
    for (int i = 0; i < n; ++i) {
        p[3 * i + 0] *= lx;
        p[3 * i + 1] *= ly;
        p[3 * i + 2] *= lz;
    }
}

void get_box_random_color(GLvec &color) {
    color.resize(108);
    
    GLfloat *p = color.data();
    for (size_t i = 0; i < 6; ++i) {
        GLfloat side_color[3] = {
            0.5f * rand() / RAND_MAX + 0.5f,
            0.5f * rand() / RAND_MAX + 0.5f,
            0.5f * rand() / RAND_MAX + 0.5f
        };
        for (size_t j = 0; j < 6; ++j) {
            *p++ = side_color[0];
            *p++ = side_color[1];
            *p++ = side_color[2];

        }
        
    }
}

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void init() {
    //srand(clock());
    program = build_program();
    get_box_3d(vertices, 1, 1, 1);
    get_box_random_color(colors);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(2, vbo);
    
    bind_buffer(vbo[0], vertices, program, "vPosition", 3);
    bind_buffer(vbo[1], colors, program, "vColor", 3);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao);
    
    GLint location;
    GLfloat theta = 0.00001f * clock();
    location = glGetUniformLocation(program, "drawing_mode");
    glUniform1i(location, drawing_mode);
    
    using namespace glm;
    mat4 T(1.0f);
    T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
    T = rotate(T, theta, vec3(0.0f, 0.1f, 0.0f));
    T = rotate(T, theta, vec3(1.0f, 0.0f, 0.0f));
    T = scale(T, vec3(scaling_factor));
    
    location = glGetUniformLocation(program, "T");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));
    
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/3);
    glFlush();
    glutSwapBuffers();

    glutPostRedisplay();

}
