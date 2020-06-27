//
//  main.cpp
//  HW10
//
//  Created by 승우 on 2020/05/12.
//  Copyright © 2020 승우. All rights reserved.
//

#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "/Users/ck/Desktop/GLUT.framework/Headers/glut.h"
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

#define FPUSH_VTX3_AT(p,i,vx,vy,vz)\
do {\
    GLuint i3 = 3*(i); \
    p[i3+0] = (float)(vx);\
    p[i3+1] = (float)(vy);\
    p[i3+2] = (float)(vz);\
} while(0)

typedef std::vector<GLfloat> GLvec;
typedef std::vector<GLuint> GLidxvec;

GLuint program;

GLuint vaos[5];
GLuint buffs[3];
GLuint cube_buffs[3];
GLuint cube_elem_buff;
GLuint sphere_buffs[3];
GLuint cone_element_buffs[2];
GLuint cylinder_element_buffs[3];
GLuint* torus_element_buffers;

std::vector<std::vector<GLuint>> torus_side_idx;

int active_vao = 0;
int mode = 0;

GLuint drawing_mode[3] = {GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLE_FAN};

GLidxvec side_idx, top_idx, bottom_idx;

GLvec vtx_pos[5], vtx_clrs[5], vtx_nml[5];
GLidxvec square_idx_list;
GLidxvec cone_idx_list[2];
GLidxvec cylinder_idx_list[3];

// Variables for display options
int show_wireframe = 0;

// HW9 - toggle and grid
int toggle = 0;
int grid = 0;

// grid
GLvec grid_pos;
GLuint grid_vao;
GLuint grid_buffs[2];
GLvec grid_clrs;

// Keyboard

GLfloat x_axis = 0.0f;
GLfloat y_axis = 0.0f;
GLfloat offset_angle = 0.0f;

// Mouse
int button_pressed[3] = {GLUT_UP, GLUT_UP, GLUT_UP};
int mouse_pos[2] = {0, 0};
void motion(int x, int y);

// Camera

struct Camera {
    enum{ ORTHOGRAPHIC, PERSPECTIVE };
    glm::vec3 eye;
    glm::vec3 center;
    glm::vec3 up;
    float zoom_factor;
    int projection_mode;
    float z_near;
    float z_far;
    float fovy;
    float x_right;
    
    glm::mat4 get_viewing() {return glm::lookAt(eye, center, up);}
    glm::mat4 parallel(double r, double aspect, double n, double f) {
        double l = -r;
        double width = 2*r;
        double height = width / aspect;
        double t = height/2;
        double b = -t;
        return glm::ortho(l, r, b, t, n, f);
    }

    glm::mat4 get_projection(float aspect) {
        glm::mat4 P;
        switch (projection_mode) {
            case ORTHOGRAPHIC:
                P = parallel(zoom_factor * x_right, aspect, z_near, z_far);
                break;
            case PERSPECTIVE:
                P = glm::perspective(zoom_factor * fovy, aspect, z_near, z_far);
                break;
        }
        return P;
    }

    Camera():
        eye(0,0,8),
        center(0,0,0),
        up(0, 1, 0),
        zoom_factor(1.0f),
        projection_mode(ORTHOGRAPHIC),
        z_near(0.01f),
        z_far(100.0f),
        fovy((float)(M_PI/180.0*(30.0))),
        x_right(1.2f)
    {}
    
};

Camera camera;

void init();
void keyboard(unsigned char key, int x, int y);
void display();

void mouse(int button, int state, int x, int y);
void special(int key, int x, int y);
void motion(int x, int y);

int build_program() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW10/HW10/viewing.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW10/HW10/viewing.frag"},
        {GL_NONE, NULL}
    };
    
    GLuint program = LoadShaders(shaders);
    glUseProgram(program);
    return program;
}

int main(int argc, char * argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Shading Primitives");
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(-1);
    }
    
    program = build_program();

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
}

void set_color(GLfloat r, GLfloat g, GLfloat b, GLfloat a = 1.0f) {
    using namespace glm;
    GLuint location_t = glGetUniformLocation(program, "mode");
    glUniform1i(location_t, 3);

    GLuint location = glGetUniformLocation(program, "uColor");
    glUniform4fv(glGetUniformLocation(program, "uColor"), 1, value_ptr(vec4(r, g, b, a)));
}

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1': active_vao = 0; glutPostRedisplay(); break;
        case '2': active_vao = 1; glutPostRedisplay(); break;
        case '3': active_vao = 2; glutPostRedisplay(); break;
        case '4': active_vao = 3; glutPostRedisplay(); break;
        case '5': active_vao = 4; glutPostRedisplay(); break;
        case '6': active_vao = 5; glutPostRedisplay(); break;
        case 'w': show_wireframe ? show_wireframe = 0 : show_wireframe = 1; glutPostRedisplay(); break;
        case 'a': toggle ? toggle = 0 : toggle = 1; glutPostRedisplay(); break;
        case 'g': grid ? grid = 0 : grid = 1; glutPostRedisplay(); break;
        case 'D': mode = 0; glutPostRedisplay(); break;
        case 'G': mode = 1; glutPostRedisplay(); break;
        case 'P': mode = 2; glutPostRedisplay(); break;
    }
}

void get_box_3d(GLvec &p, GLvec &normals, GLfloat lx, GLfloat ly, GLfloat lz) {
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
    
    // Compute normals
    normals.resize(n * 3);
    float* cursor = normals.data();
    
    // For vertices on the side at z = -0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = 0; cursor[1] = 0; cursor[2] = -1;}
    // For vertices on the side at x = -0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = -1; cursor[1] = 0; cursor[2] = 0;}
    // For vertices on the side at y = -0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = 0; cursor[1] = -1; cursor[2] = 0;}
    // For vertices on the side at z = 0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = 0; cursor[1] = 0; cursor[2] = 1;}
    // For vertices on the side at x = 0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = 1; cursor[1] = 0; cursor[2] = 0;}
    // For vertices on the side at y = 0.5
    for (int i = 0; i < 6; ++i, cursor += 3) {cursor[0] = 0; cursor[1] = 1 ; cursor[2] = 0;}
}

void get_sphere_3d(GLvec& p, GLvec& normals, GLfloat r, GLint subh, GLint suba) {
    
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
                FPUSH_VTX3(normals, vx0/r, vy0/r, vz0/r);
                FPUSH_VTX3(normals, vx1/r, vy1/r, vz1/r);
                FPUSH_VTX3(normals, vx3/r, vy3/r, vz3/r);

            }
            
            if (1 < i) { // second triangle (v3- v2- v0)
                FPUSH_VTX3(p, vx3, vy3, vz3);
                FPUSH_VTX3(p, vx2, vy2, vz2);
                FPUSH_VTX3(p, vx0, vy0, vz0);
                FPUSH_VTX3(normals, vx3/r, vy3/r, vz3/r);
                FPUSH_VTX3(normals, vx2/r, vy2/r, vz2/r);
                FPUSH_VTX3(normals, vx0/r, vy0/r, vz0/r);

            }
        }
    }
}

void get_cone_3d(GLvec&p, GLidxvec& side_idx, GLidxvec& bottom_idx, GLvec &normals, GLfloat radius, GLfloat height, GLint n) {
    GLfloat half_height = height / 2;
    GLfloat theta, x, z;
        
    FPUSH_VTX3(p, 0, half_height, 0);
    FPUSH_VTX3(normals, 0, 1, 0);
    side_idx.push_back(0);
    
    for (int i = 0; i <= n; ++i) {
        theta = (GLfloat) (2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        FPUSH_VTX3(p, x, -half_height, z);
        FPUSH_VTX3(normals, x/radius, 0, z/radius);
        side_idx.push_back(i+1);
        bottom_idx.push_back(n+2-i);
    }
    FPUSH_VTX3(p, 0, -half_height, 0);
    FPUSH_VTX3(normals, 0, -1, 0);
    bottom_idx.push_back(1);

    for (int i = n+2; i <= 2*n+3; i++) {
        theta = (GLfloat) (2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        FPUSH_VTX3(p, x, -half_height, z);
        FPUSH_VTX3(normals, 0, -1, 0);
    }
}


void get_cylinder_3d(GLvec& p, GLidxvec& side_idx, GLidxvec& top_idx, GLidxvec& bottom_idx, GLvec& normals, GLfloat radius, GLfloat height, GLint n) {
    GLfloat half_height = height / 2;
    GLfloat theta, x, z;
    p.resize(3 * (4*n + 6));
    normals.resize(3 * (4*n + 6));
    
    // Top
    FPUSH_VTX3_AT(p, 0, 0, half_height, 0);
    FPUSH_VTX3_AT(normals, 0, 0, 1, 0);
    top_idx.push_back(0);

    for (int i = 0; i <= n; ++i) {
        theta = (GLfloat) (2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        FPUSH_VTX3_AT(p, i+1, x, half_height, z);
        FPUSH_VTX3_AT(normals, i+1, 0, 1, 0);
        top_idx.push_back(i+1);
    }
    // Side
    for (int i = 0; i <= n; ++i) {
        theta = (GLfloat) (2.0 * M_PI * i / n);
        x = radius * sin(theta);
        z = radius * cos(theta);
        int tmp = 2 * i + n + 2;
        FPUSH_VTX3_AT(p, tmp, x, half_height, z);
        FPUSH_VTX3_AT(normals, tmp, x/radius, 0, z/radius);
        FPUSH_VTX3_AT(p, tmp+1, x, -half_height, z);
        FPUSH_VTX3_AT(normals, tmp+1, x/radius, 0, z/radius);
        side_idx.push_back(tmp);
        side_idx.push_back(tmp+1);
    }
    // Bottom
    FPUSH_VTX3_AT(p, 3*n+4, 0, -half_height, 0) ;
    FPUSH_VTX3_AT(normals, 3*n+4, 0, -1, 0);
    bottom_idx.push_back(3*n + 4);
    
    for(int i = n; i >= 0; --i){
        theta = (GLfloat)(2.0 * M_PI * i / n) ;
        x = radius * sin(theta) ;
        z = radius * cos(theta) ;
        int tmp = 4 * n + 5 - i;
        FPUSH_VTX3_AT(p, tmp, x, -half_height, z) ;
        FPUSH_VTX3_AT(normals, tmp, 0, -1, 0);
        bottom_idx.push_back(tmp) ;
    }
     

    printf("SIZE_IDX: %d\n", side_idx.size());

    for (int i = 0; i < side_idx.size(); ++i)
        printf("%d\t", side_idx[i]);
    printf("\nBOTTOM_IDx%d\n", bottom_idx.size());

    for (int i = 0; i < bottom_idx.size(); ++i)
        printf("%d\t", bottom_idx[i]);

}

void get_torus_3d(GLvec& p, std::vector<std::vector<GLuint>>& side_idx, GLvec& normals, GLfloat r0, GLfloat r1, GLint na, GLint nh) {
    GLfloat theta, x_ij, y_j, z_ij, phi;
    side_idx.resize(nh);
    
    for (int i = 0; i < nh; ++i) {
        theta = (GLfloat)(2.0 * M_PI * i/ nh);
        GLfloat x_i = (r0 + r1) * sin(theta);
        GLfloat z_i = (r0 + r1) * cos(theta);
        
        for (int j = 0; j <= na; ++j) {
            phi = (GLfloat)(2.0 * M_PI * j/ na);
            
            GLfloat l_j = r1 * cos(phi);
            GLfloat dx_ij = l_j * sin(theta);
            GLfloat dz_ij = l_j * cos(theta);
            
            x_ij = x_i + dx_ij;
            y_j = r1 * sin(phi);
            z_ij = z_i + dz_ij;
            
            FPUSH_VTX3(p, x_ij, y_j, z_ij);
            FPUSH_VTX3(normals, x_ij/r1, y_j/r1, z_ij/r1);
            
            side_idx[i].push_back(i*(na + 1) + j);
            side_idx[i].push_back((i+1)*(na+1) + j);
        }
    }
    
    for (int j = 0; j <= na; ++j) {
        FPUSH_VTX3(p, p[j * 3 + 0], p[j * 3 + 1], p[j * 3 + 2]);
        FPUSH_VTX3(normals, normals[j * 3 + 0], normals[j * 3 + 1], normals[j * 3 + 2]);
    }
}

void get_grid(GLvec& p, GLfloat w, GLfloat h, int m, int n) {
    GLfloat x0 = -0.5f * w;
    GLfloat x1 = +0.5f * w;
    GLfloat z0 = -0.5f * w;
    GLfloat z1 = +0.5f * w;
    for (int i = 0; i <= m; i++) {
        GLfloat x = x0 + w*i / m;
        FPUSH_VTX3(p, x, 0, z0);
        FPUSH_VTX3(p, x, 0, z1);
    }
    for (int i = 0; i <= n; i++) {
        GLfloat z = z0 + h*i / n;
        FPUSH_VTX3(p, x0, 0, z);
        FPUSH_VTX3(p, x1, 0, z);
    }
}

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset = 0) {
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


void draw_primitives(GLuint active_vao, const GLfloat* trans_mat) {
    
    GLint location_m = glGetUniformLocation(program, "M");
    glUniformMatrix4fv(location_m, 1, GL_FALSE, trans_mat);
    
    switch(active_vao) {
        case 0: // Box
            glBindVertexArray(vaos[active_vao]);
            glDrawArrays(GL_TRIANGLES, 0, vtx_pos[0].size());

            break;
        case 1: // Sphere
            glBindVertexArray(vaos[active_vao]);
            glDrawArrays(GL_TRIANGLES, 0, vtx_pos[1].size());
            break;
        case 2: // Cone
            glBindVertexArray(vaos[active_vao]);
            for (int i = 0; i < 2; ++i) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_element_buffs[i]);
                glDrawElements(GL_TRIANGLE_FAN, cone_idx_list[i].size(), GL_UNSIGNED_INT, NULL);
            }
            break;
        case 3: // Cylinder
            glBindVertexArray(vaos[active_vao]);
            for (int i = 0; i < 3; ++i) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[i]);
                glDrawElements(drawing_mode[i], cylinder_idx_list[i].size(), GL_UNSIGNED_INT, NULL);
            }
            break;
            
        case 4: { // Torus
            glBindVertexArray(vaos[active_vao]);
            GLuint n = torus_side_idx.size();
            for (int i = 0; i < n; ++i) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffers[i]);
                glDrawElements(GL_TRIANGLE_STRIP, torus_side_idx[i].size(), GL_UNSIGNED_INT, NULL);
            }
            break;
        }
        case 5: // Grid
            glBindVertexArray(grid_vao);
            glDrawArrays(GL_LINES, 0, grid_pos.size());
            break;
    }
}

// Callback Functions

void cb_menu(int value) {
    camera.projection_mode = value;
}

void mouse(int button, int state, int x, int y) {
    button_pressed[button] = state;
    mouse_pos[0] = x;
    mouse_pos[1] = y;
}

void motion(int x, int y) {
    using namespace glm;
        
    int modifiers = glutGetModifiers();
    int is_alt_active = modifiers & GLUT_ACTIVE_ALT;
    int is_cmd_active = modifiers & GLUT_ACTIVE_COMMAND;
    int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT;
    
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    GLfloat dx = 1.f*(x - mouse_pos[0]) / w;
    GLfloat dy = -1.f*(y - mouse_pos[1]) / h;
    
    bool redraw = false;
    
    if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN) {
        // Tumble tool - Mouse Left Button + Option + Drag
        if (is_alt_active) {
            vec4 disp(camera.eye - camera.center, 1);
            
            GLfloat alpha = 2.0f;
            mat4 V = camera.get_viewing();
            mat4 Rx = rotate(mat4(1.0f), alpha*dy, vec3(transpose(V)[0]));
            mat4 Ry = rotate(mat4(1.0f), -alpha*dx, vec3(0,1,0));
            mat4 R = Ry * Rx;
            camera.eye = camera.center + vec3(R*disp);
            camera.up = mat3(R) * camera.up;
            redraw = true;
        }
        
        // Zoom tool - Mouse Left Button + Command + Drag
        if (is_cmd_active) {
            if ((dx < 0) || (dy < 0))
                camera.zoom_factor *= 0.95f;
            else if ((dx > 0) || (dy > 0))
                camera.zoom_factor *= 1.05f;
            
            redraw = true;
        }
    }
    
    if (button_pressed[GLUT_MIDDLE_BUTTON] == GLUT_DOWN) {
        // Track tool - Mouse Middle Button + Option + Drag
        if (is_alt_active) {

            mat4 VT = transpose(camera.get_viewing());
            camera.eye += vec3(-dx * VT[0] + -dy * VT[1]);
            camera.center += vec3(-dx * VT[0] + -dy * VT[1]);
            redraw = true;
        }
        
        // Dolly tool - Mouse Middle Button + Command + Drag
        if (is_cmd_active) {
            glm::vec3 disp = camera.eye - camera.center;
            if ((dx < 0) || (dy < 0))
                camera.eye = camera.center + 0.95f*disp;
            else if ((dx > 0) || (dy > 0))
                camera.eye = camera.center + 1.05f*disp;
            
            redraw = true;
        }
    }
        
    mouse_pos[0] = x;
    mouse_pos[1] = y;
    
    if (redraw) {
        glutPostRedisplay();
    }
}

void special(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            x_axis -= 0.05f; glutPostRedisplay(); break;
        case GLUT_KEY_RIGHT:
            x_axis += 0.05f; glutPostRedisplay(); break;
        case GLUT_KEY_UP:
            y_axis += 0.05f; glutPostRedisplay(); break;
        case GLUT_KEY_DOWN:
            y_axis -= 0.05f; glutPostRedisplay(); break;
        case GLUT_KEY_HOME:
            offset_angle -= 0.5f; glutPostRedisplay(); break;
        case GLUT_KEY_END:
            offset_angle += 0.5f; glutPostRedisplay(); break;

    }
}
void init() {
    // Callback Functions
    int menu_id = glutCreateMenu(cb_menu);
    
    glutAddMenuEntry("Orthographic projection", 0);
    glutAddMenuEntry("Perspective projection", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    GLchar* attri_name[3] = {"vPosition", "vNormal", "vColor"};
    GLvec* cube_vtx_list[3] = {vtx_pos, vtx_nml, vtx_clrs};
    
    // 3D box
    get_box_3d(vtx_pos[0], vtx_nml[0], 1, 1, 1);
    get_color_3d_by_pos(vtx_clrs[0], vtx_pos[0]);
    
    glGenVertexArrays(1, vaos);
    glBindVertexArray(vaos[0]);
        
    glGenBuffers(3, cube_buffs);
    for (int i = 0; i < 3; ++i) {
        bind_buffer(cube_buffs[i], *cube_vtx_list[i], program, attri_name[i], 3);
    }

    // 3D sphere
    get_sphere_3d(vtx_pos[1], vtx_nml[1], 0.5, 20, 20);
    get_color_3d_by_pos(vtx_clrs[1], vtx_pos[1]);
    
    GLvec* sphere_vtx_list[3] = {vtx_pos + 1, vtx_nml + 1, vtx_clrs + 1};

    glGenVertexArrays(1, vaos + 1);
    glBindVertexArray(vaos[1]);
    
    glGenBuffers(3, sphere_buffs);
    for (int i = 0; i < 3; ++i) {
        bind_buffer(sphere_buffs[i], *sphere_vtx_list[i], program, attri_name[i], 3);
    }

    // 3D cone

    get_cone_3d(vtx_pos[2], cone_idx_list[0], cone_idx_list[1], vtx_nml[2], 0.5, 1.0, 20);
    get_color_3d_by_pos(vtx_clrs[2], vtx_pos[2]);
    
    GLvec* cone_vtx_list[3] = {vtx_pos + 2, vtx_nml + 2, vtx_clrs + 2};

    glGenVertexArrays(1, vaos + 2);
    glBindVertexArray(vaos[2]);
    
    glGenBuffers(3, buffs);
    for (int i = 0; i < 3; ++i) {
        bind_buffer(buffs[i], *cone_vtx_list[i], program, attri_name[i], 3);
    }
        
    glGenBuffers(2, cone_element_buffs);
    for (int i = 0; i < 2; ++i) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_element_buffs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*cone_idx_list[i].size(), cone_idx_list[i].data(), GL_STATIC_DRAW);
    }
   
    // 3D cylinder
    get_cylinder_3d(vtx_pos[3], cylinder_idx_list[0], cylinder_idx_list[1], cylinder_idx_list[2], vtx_nml[3], 0.5, 1.0, 30);
    get_color_3d_by_pos(vtx_clrs[3], vtx_pos[3]);
    
    GLvec* cylinder_vtx_list[3] = {vtx_pos + 3, vtx_nml + 3, vtx_clrs + 3};

    glGenVertexArrays(1, vaos + 3);
    glBindVertexArray(vaos[3]);
    
    glGenBuffers(3, buffs);
    for (int i = 0; i < 3; ++i) {
        bind_buffer(buffs[i], *cylinder_vtx_list[i], program, attri_name[i], 3);
    }
    
    glGenBuffers(3, cylinder_element_buffs);
    for (int i = 0; i < 3; ++i) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinder_element_buffs[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*cylinder_idx_list[i].size(), cylinder_idx_list[i].data(), GL_STATIC_DRAW);
    }
    
    // 3D torus
    get_torus_3d(vtx_pos[4], torus_side_idx, vtx_nml[4], 0.3, 0.3, 30, 30);
    get_color_3d_by_pos(vtx_clrs[4], vtx_pos[4]);
    
    GLvec* torus_vtx_list[3] = {vtx_pos + 4, vtx_nml + 4, vtx_clrs + 4};
    
    glGenVertexArrays(1, vaos + 4);
    glBindVertexArray(vaos[4]);
    
    glGenBuffers(3, buffs);
    for (int i = 0; i < 3; ++i) {
        bind_buffer(buffs[i], *torus_vtx_list[i], program, attri_name[i], 3);
    }
    
    GLuint n = torus_side_idx.size();
    torus_element_buffers = new GLuint[n];
    
    glGenBuffers(n, torus_element_buffers);
    for (int i = 0; i < n; ++i) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_element_buffers[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*torus_side_idx[i].size(), torus_side_idx[i].data(), GL_STATIC_DRAW);

    }
    
    //grid
    get_grid(grid_pos, 20, 20, 10, 10);
    get_color_3d_by_pos(grid_clrs, grid_pos);
    
    glGenVertexArrays(1, &grid_vao);
    glBindVertexArray(grid_vao);
    
    GLvec* grid_vtx_list[2] = {&grid_pos, &grid_clrs};
    
    GLchar* grid_attri_name[2] = {"vPosition", "vColor"};

    glGenBuffers(2, grid_buffs);
    for (int i = 0; i < 2; ++i) {
        bind_buffer(grid_buffs[i], *grid_vtx_list[i], program, grid_attri_name[i], 3);
    }
    
    glEnable(GL_DEPTH_TEST); // Enable depth test
    glDepthFunc(GL_LESS); // Accept fragment it is closer to the camera than the existing one
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void draw_toggle() {
    using namespace glm;
     mat4 M(1.0f);
     M = translate(M, vec3(x_axis, y_axis, 0.0f));
     M = rotate(M, radians(offset_angle), vec3(0.0f, 1.0f, 0.0f));
     
     mat4 S(1.0f);
     S = scale(S, vec3(0.05f, 1.0f, 0.05f));
     
     mat4 Tx(1.0f);
     Tx = translate(Tx, vec3(0.5f, 0.0f, 0.0f));
     Tx = rotate(Tx, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
             
     mat4 Ty(1.0f);
     Ty = translate(Ty, vec3(0.0f, 0.5f, 0.0f));
     
     mat4 Tz(1.0f);
     Tz = translate(Tz, vec3(0.0f, 0.0f, 0.5f));
     Tz = rotate(Tz, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

     set_color(1, 0, 0);
     draw_primitives(3, value_ptr(M*Tx*S));
    
     set_color(0, 1, 0);
     draw_primitives(3, value_ptr(M*Ty*S));
    
     set_color(0, 0, 1);
     draw_primitives(3, value_ptr(M*Tz*S));
}

void draw_grid() {
    using namespace glm;
    glLineWidth(3.0f);
    mat4 xz(1.0f);
    
    set_color(1, 1, 1);
    draw_primitives(5, value_ptr(xz));
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,1024,1024);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    GLuint location = glGetUniformLocation(program, "mode");
    glUniform1i(location, 0);

    using namespace glm;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    double aspect = 1.0 * width / height;
    
    mat4 V = camera.get_viewing();
    mat4 P = camera.get_projection(aspect);
    mat4 T = mat4(1.0f);
    
    GLint location_v = glGetUniformLocation(program, "V");
    glUniformMatrix4fv(location_v, 1, GL_FALSE, value_ptr(V));
    
    GLint location_p = glGetUniformLocation(program, "P");
    glUniformMatrix4fv(location_p, 1, GL_FALSE, value_ptr(P));
    
    GLint location_t = glGetUniformLocation(program, "T");
    glUniformMatrix4fv(location_t, 1, GL_FALSE, value_ptr(T));
    
    if (toggle) {
        draw_toggle();
    }
    
    if (grid) {
        draw_grid();
    }

    if (active_vao < 5) {
        mat4 M(1.0f);
        M = translate(M, vec3(x_axis, y_axis, 0.0f));
        M = rotate(M, radians(offset_angle), vec3(0.0f, 1.0f, 0.0f));
        
        GLuint location_t = glGetUniformLocation(program, "mode");
        glUniform1i(location_t, mode);

        draw_primitives(active_vao, value_ptr(M));

        if (show_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1);
            set_color(0, 0, 0);
            draw_primitives(active_vao, value_ptr(M));

        }
        glDisable(GL_POLYGON_OFFSET_FILL);

    }
    else {
        // Rotate composite model along y axis
        mat4 comp(1.0f);
        comp = translate(comp, vec3(x_axis, y_axis, 0.0f));
        comp = rotate(comp, radians(offset_angle), vec3(0.0f, 1.0f, 0.0f));

        // Corn
        mat4 corn(1.0f);
        corn = translate(corn, vec3(-0.3f, 0.7f, 0.0f));
        corn = scale(corn, vec3(1.0f, 0.2f, 1.0f));
        
        // Body 1
        mat4 body(1.0f);
        body = translate(body, vec3(-0.3f, 0.35f, 0.0f));
        body = scale(body, vec3(0.5f, 0.5f, 0.5f));
        
        // Body 2
        mat4 body2(1.0f);
        body2 = translate(body2, vec3(-0.05f, -0.1f, 0.0f));
        body2 = scale(body2, vec3(1.0f, 0.4f, 0.5f));
        
        // Wheel - front left
        mat4 wheelfl(1.0f);
        wheelfl = translate(wheelfl, vec3(0.3f, -0.3f, -0.35f));
        wheelfl = scale(wheelfl, vec3(0.3f, 0.3f, 0.3f));
        wheelfl = rotate(wheelfl, radians(-x_axis*10), vec3(0.0f, 0.0f, 1.0f));
        wheelfl = rotate(wheelfl, radians(offset_angle), vec3(0.0f, 0.0f, 1.0f));
        wheelfl = rotate(wheelfl, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
                
        // Wheel - front right
        mat4 wheelfr(1.0f);
        wheelfr = translate(wheelfr, vec3(0.3f, -0.3f, 0.35f));
        wheelfr = scale(wheelfr, vec3(0.3f, 0.3f, 0.3f));
        wheelfr = rotate(wheelfr, radians(-x_axis*10), vec3(0.0f, 0.0f, 1.0f));
        wheelfr = rotate(wheelfr, radians(-offset_angle), vec3(0.0f, 0.0f, 1.0f));
        wheelfr = rotate(wheelfr, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
                
        // Wheel - back left
        mat4 wheelbl(1.0f);
        wheelbl = translate(wheelbl, vec3(-0.3f, -0.3f, -0.35f));
        wheelbl = scale(wheelbl, vec3(0.3f, 0.3f, 0.3f));
        wheelbl = rotate(wheelbl, radians(-x_axis*10), vec3(0.0f, 0.0f, 1.0f));
        wheelbl = rotate(wheelbl, radians(offset_angle), vec3(0.0f, 0.0f, 1.0f));
        wheelbl = rotate(wheelbl, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        
        // Wheel - back right
        mat4 wheelbr(1.0f);
        wheelbr = translate(wheelbr, vec3(-0.3f, -0.3f, 0.35f));
        wheelbr = scale(wheelbr, vec3(0.3f, 0.3f, 0.3f));
        wheelbr = rotate(wheelbr, radians(-x_axis*10), vec3(0.0f, 0.0f, 1.0f));
        wheelbr = rotate(wheelbr, radians(-offset_angle), vec3(0.0f, 0.0f, 1.0f));
        wheelbr = rotate(wheelbr, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
                
        // Cylinder on body
        mat4 cyl(1.0f);
        cyl = translate(cyl, vec3(0.15f, 0.0f, 0.0f));
        cyl = rotate(cyl, radians(-x_axis * 10), vec3(1.0f, 0.0f, 0.0f));
        cyl = rotate(cyl, radians(-offset_angle), vec3(1.0f, 0.0f, 0.0f));
        cyl = rotate(cyl, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        cyl = scale(cyl, vec3(0.5f, 0.4f, 0.5f));

        // Cylinder on front wheels
        mat4 cyl2(1.0f);
        cyl2 = translate(cyl2, vec3(0.3f, -0.3f, 0.0f));
        cyl2 = rotate(cyl2, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        cyl2 = scale(cyl2, vec3(0.1f, 0.7f, 0.1f));

        // Cylinder on back wheels
        mat4 cyl3(1.0f);
        cyl3 = translate(cyl3, vec3(-0.3f, -0.3f, 0.0f));
        cyl3 = rotate(cyl3, radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        cyl3 = scale(cyl3, vec3(0.1f, 0.7f, 0.1f));

        // Circle 1 on the front of the body 2
        mat4 eye1(1.0f);
        eye1 = translate(eye1, vec3(0.45f, -0.1f, 0.1f));
        eye1 = rotate(eye1, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        eye1 = scale(eye1, vec3(0.1f, 0.1f, 0.1f));
        
        // Circle 2 on the front of the body 2
        mat4 eye2(1.0f);
        eye2 = translate(eye2, vec3(0.45f, -0.1f, -0.1f));
        eye2 = rotate(eye2, radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
        eye2 = scale(eye2, vec3(0.1f, 0.1f, 0.1f));
   
        GLuint location_t = glGetUniformLocation(program, "mode");
        glUniform1i(location_t, mode);
        
        draw_primitives(2, value_ptr(comp*corn));
        draw_primitives(0, value_ptr(comp*body));
        draw_primitives(0, value_ptr(comp*body2));
        draw_primitives(4, value_ptr(comp*wheelfl));
        draw_primitives(4, value_ptr(comp*wheelfr));
        draw_primitives(4, value_ptr(comp*wheelbl));
        draw_primitives(4, value_ptr(comp*wheelbr));
        draw_primitives(3, value_ptr(comp*cyl));
        draw_primitives(3, value_ptr(comp*cyl2));
        draw_primitives(3, value_ptr(comp*cyl3));
        draw_primitives(1, value_ptr(comp*eye1));
        draw_primitives(1, value_ptr(comp*eye2));

        if (show_wireframe) {

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1);
            
            set_color(0, 0, 0);
            draw_primitives(2, value_ptr(comp*corn));
            draw_primitives(0, value_ptr(comp*body));
            draw_primitives(0, value_ptr(comp*body2));
            draw_primitives(4, value_ptr(comp*wheelfl));
            draw_primitives(4, value_ptr(comp*wheelfr));
            draw_primitives(4, value_ptr(comp*wheelbl));
            draw_primitives(4, value_ptr(comp*wheelbr));
            draw_primitives(3, value_ptr(comp*cyl));
            draw_primitives(3, value_ptr(comp*cyl2));
            draw_primitives(3, value_ptr(comp*cyl3));
            draw_primitives(1, value_ptr(comp*eye1));
            draw_primitives(1, value_ptr(comp*eye2));
        }
    }
    
    glDisable(GL_POLYGON_OFFSET_FILL);

    glFlush();
    glutSwapBuffers();

    glutPostRedisplay();

        
}
