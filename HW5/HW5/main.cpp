//
//  main.cpp
//  HW5
//
//  Created by 승우 on 2020/03/31.
//  Copyright © 2020 승우. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLUT/glut.h>
#include "LoadShaders.h"
#include <vector>
#include <string.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

typedef std::vector<GLfloat> GLvec;

const GLuint num_of_models = 3;

const char* obj_filepath[num_of_models] = {
    "house.obj",
    "sphere.obj",
    "teapot.obj"
};

GLvec vertices[num_of_models];
std::vector<GLuint> faces[num_of_models];

GLuint vao[num_of_models];
GLuint vbo[num_of_models][2];

GLuint active_vao = 0;

GLuint program;

void init();
void keyboard(unsigned char key, int x, int y);
void display();

int main(int argc, char * argv[]) {
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Obj Loader");
    
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
        case '1': active_vao = 0; glutPostRedisplay(); break;
        case '2': active_vao = 1; glutPostRedisplay(); break;
        case '3': active_vao = 2; glutPostRedisplay(); break;
    }
}

int build_program() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW5/HW5/obj.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW5/HW5/obj.frag"},
        {GL_NONE, NULL}
    };
    
    GLuint program = LoadShaders(shaders);
    glUseProgram(program);
    return program;
}

void bind_buffer(GLint buffer, std::vector<GLuint>& vec, int program) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vec.size(), vec.data(), GL_STATIC_DRAW);
}

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

bool load_obj(GLvec& vertices, std::vector<GLuint>& faces, const char* filepath) {
    
    char path[100] = "/Users/ck/Desktop/ComputerGraphics/HW5/obj_files/";
    strcat(path, filepath);
    
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Error opening the file\n");
        return false;
    }
    
    while(1) {
        char lineHeader[128];

        if (fscanf(fp, "%s", lineHeader) == EOF) {
            break;
        }
        
        if (strcmp(lineHeader, "v") == 0) {
            float v_idx[3];
                
            fscanf(fp, " %f %f %f\n ", &v_idx[0], &v_idx[1], &v_idx[2]);
            vertices.push_back(v_idx[0]);
            vertices.push_back(v_idx[1]);
            vertices.push_back(v_idx[2]);

        }
        else if(strcmp(lineHeader, "f") == 0) {
            unsigned int vertex_idx[3], uv_idx[3];
            
            // for house.obj (f d d d)
            if (strcmp(filepath, obj_filepath[0]) == 0)  // house.obj
                fscanf(fp, "%d %d %d\n", &vertex_idx[0], &vertex_idx[1], &vertex_idx[2]);

            // for teapot.obj and sphere.obj (f d/d d/d d/d)
            else
                fscanf(fp, "%d/%d %d/%d %d/%d\n", &vertex_idx[0], &uv_idx[0], &vertex_idx[1], &uv_idx[1], &vertex_idx[2], &uv_idx[2]);

            faces.push_back(vertex_idx[0] - 1);
            faces.push_back(vertex_idx[1] - 1);
            faces.push_back(vertex_idx[2] - 1);
        }

    }
    
    fclose(fp);
    
    return true;
}

void init() {
    srand(clock());
    program = build_program();
    for (int i = 0; i < num_of_models; ++i) {
        load_obj(vertices[i], faces[i], obj_filepath[i]);
        
        glGenVertexArrays(1, &vao[i]);
        glBindVertexArray(vao[i]);
        glGenBuffers(2, vbo[i]);
        bind_buffer(vbo[i][0], vertices[i], program, "vPosition", 3);
        bind_buffer(vbo[i][1], faces[i], program);
    }
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glLineWidth(1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void display() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vao[active_vao]);
    
    GLint location;
    GLfloat theta = 0.00001f * clock();
    location = glGetUniformLocation(program, "active_vao");
    glUniform1i(location, active_vao);
    
    using namespace glm;
    mat4 T(1.0f);
    T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f));
    
    location = glGetUniformLocation(program, "T");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(T));
    
    glDrawElements(GL_TRIANGLES, faces[active_vao].size(), GL_UNSIGNED_INT, (void *)0);
    glFlush();
    glutSwapBuffers();

    glutPostRedisplay();

}
