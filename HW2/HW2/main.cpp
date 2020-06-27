//
//  main.cpp
//  HW2
//
//  Created by 승우 on 2020/03/12.
//  Copyright © 2020 승우. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLUT/glut.h>
#include "LoadShaders.h"
#define _USE_MATH_DEFINES // to use M_PI
#include <math.h>
#include <vector>

#define VSET2(v,a,b) do {(v)[0]=(a); (v)[1]=(b);} while(0)
#define VSET2PP(v,a,b) do {VSET2(v,a,b); v+=2;} while(0)

typedef std::vector<GLfloat> GLvec; // store vertex positions and colors

GLuint VertexArrays[3]; // for circles and rectangle [circle, triangle, circle]
GLuint Buffers[3][2];   // 2D array [circle or rectangle][position or color]
GLuint active_vao = 0;  // index for the active vertex array object 1-circle 2-rectangle
GLvec vertices[2];      // 2d vertex list [circle][rectangle]
GLvec colors[3];        // vertex color (r,g,b) list

void init();
void keyboard(unsigned char key, int x, int y);
void display();

int main(int argc, char * argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(512, 512);
    glutCreateWindow("VAO example");

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
    
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1': active_vao = 0; glutPostRedisplay(); break;
        case '2': active_vao = 1; glutPostRedisplay(); break;
        case '3': active_vao = 2; glutPostRedisplay(); break;
    }
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(VertexArrays[active_vao]);
    glDrawArrays(active_vao != 1? GL_TRIANGLE_FAN : GL_TRIANGLES, 0, vertices[active_vao].size() / 2);
    glFlush();
    glutSwapBuffers();
}

int build_program() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW2/HW2/vao_example.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW2/HW2/vao_example.frag"},
        {GL_NONE, NULL}
    };
    
    GLuint program = LoadShaders(shaders);
    glUseProgram(program);
    return program;
}

void get_circle_2d(GLvec& p, int sectors, GLfloat radius) {
    p.resize((sectors+2)*2);
    GLfloat* data = p.data();
    
    // Push the center of a circle.
    VSET2PP(data, 0, 0);
    
    // Push all circular points that constructs the circle.
    for (int i = 0; i <= sectors; ++i) {
        GLfloat phi = (GLfloat)(2 * M_PI * i / sectors);
        GLfloat x = radius * cos(phi);
        GLfloat y = radius * sin(phi);
        VSET2PP(data, x, y);
    }
}

void get_rect_2d(GLvec& p, GLfloat width, GLfloat height) {
    GLfloat w2 = width / 2;
    GLfloat h2 = height / 2;
    
    p.resize(12);
    GLfloat *data = p.data();
    
    // first triangle
    VSET2PP(data, -w2, -h2);
    VSET2PP(data, +w2, -h2);
    VSET2PP(data, -w2, +h2);
    
    // second triangle
    VSET2PP(data, +w2, -h2);
    VSET2PP(data, +w2, +h2);
    VSET2PP(data, -w2, +h2);
    
}

void get_vertex_color(GLvec& color, GLuint n, GLfloat r, GLfloat g, GLfloat b) {
    color.resize(n * 3);
    for (GLuint i = 0; i < n; ++i) {
        color[i * 3 + 0] = r;
        color[i * 3 + 1] = g;
        color[i * 3 + 2] = b;
    }
}

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void share_buffer(GLuint buffer, int program, const GLchar* attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void init() {
    
    int program = build_program(); // compile shader and generate executable program
    
    get_circle_2d(vertices[0], 50, 1.0f); // 0.5f - radius
    get_rect_2d(vertices[1], 1.0f, 0.8f); // width, height

    get_vertex_color(colors[0], vertices[0].size()/2, 0.8f, 0.2f, 0.5f); // # of vertices, RGB color
    get_vertex_color(colors[1], vertices[1].size()/2, 0.9f, 0.9f, 0.1f);
    get_vertex_color(colors[2], vertices[0].size()/2, 0.3f, 0.3f, 1.0f);
    
    glGenVertexArrays(3, VertexArrays);

    for (int i = 0; i < 3; i++) {
        glBindVertexArray(VertexArrays[i]);
        glGenBuffers(2, Buffers[i]);

        if (i != 2)
            bind_buffer(Buffers[i][0], vertices[i], program, "vPosition", 2);
        else
            share_buffer(Buffers[0][0], program, "vPosition", 2);
        

        bind_buffer(Buffers[i][1], colors[i], program, "vColor", 3);

    }

}
