//
//  main.cpp
//  HW11
//
//  Created by 승우 on 2020/05/15.
//  Copyright © 2020 승우. All rights reserved.
//
#define GL_SILENCE_DEPRECATION

#include "loadobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "/Users/ck/Desktop/ComputerGraphics/HW11/GLUT.framework/Headers/glut.h"
#include "LoadShaders.h"

#define _USE_MATH_DEFINES // to use M_PI
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#define FPUSH_VTX3(p,vx,vy,vz)\
do {\
    p.push_back(vx); \
    p.push_back(vy); \
    p.push_back(vz); \
} while(0)

#define UVAR(name, i) glGetUniformLocation(program[i], name)
#define UVARS(name) UVAR(name, shading_mode)

typedef std::vector<GLfloat> GLvec;

// for load_obj()
std::vector<tinyobj::real_t> vertices;
std::vector<tinyobj::real_t> normals;
std::vector<tinyobj::real_t> colors;
std::vector<std::vector<size_t>> vertex_map;
std::vector<std::vector<size_t>> material_map;
std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
bool is_obj_valid = false;

int shading_mode = 0;
int is_wireframe_visible = 0;

GLuint program[2];
GLuint vao[2];
GLuint vbo[2];

int model = 0;

// grid
int grid = 0;
GLvec grid_pos;
GLvec grid_clrs;
GLuint grid_vao;
GLuint grid_vbo[2];

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
    
};

Camera camera;

void init();
void draw_obj_model(int color_mode);
void display();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size);
void bind_buffer(GLuint buffer, int program, const GLchar *attri_name, GLint attri_size);
//void cb_menu(int value);

int main(int argc, char * argv[]) {
    
    printf("Select number (0 or 1): ");
    scanf("%d", &model);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("ObjViewer");
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(-1);
    }
    
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
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

void bind_buffer(GLuint buffer, GLvec& vec, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void bind_buffer(GLuint buffer, int program, const GLchar *attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '1' :
        case '2' :
            shading_mode = key - '1';
            printf("Shading mode: %d\n", shading_mode);
            glutPostRedisplay();
            break;
        case 'g' :
            grid ? grid = 0 : grid = 1; printf("g is pressed\n"); glutPostRedisplay(); break;
        case 'w' :
            is_wireframe_visible ? is_wireframe_visible = 0 : is_wireframe_visible = 1;
            printf("w is pressed\n");
            glutPostRedisplay();
            break;
    }
}

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

void draw_obj_model(int color_mode) {
    glUniform1i(UVARS("ColorMode"), color_mode);
    glBindVertexArray(vao[shading_mode]);
    
    for (size_t i = 0; i < shapes.size(); ++i) {
        
        for (size_t j = 0; j < material_map[i].size(); ++j) {
            //Set material properties
            
            int m_id = material_map[i][j];
            if (m_id < 0) {
                glUniform1f(UVARS("n"), 10.0f);
                glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f);
                glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f);
                glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f);
            }
            else {
                glUniform1f(UVARS("n"), materials[m_id].shininess);
                glUniform3fv(UVARS("Ka"), 1, materials[m_id].ambient);
                glUniform3fv(UVARS("Kd"), 1, materials[m_id].diffuse);
                glUniform3fv(UVARS("Ks"), 1, materials[m_id].specular);
                
            }
            
            glDrawArrays(GL_TRIANGLES, vertex_map[i][j], vertex_map[i][j+1] - vertex_map[i][j]);
        }
    }
}

void init() {
    
    // Callback Functions
    int menu_id = glutCreateMenu(cb_menu);

    glutAddMenuEntry("Orthographic projection", 0);
    glutAddMenuEntry("Perspective projection", 1);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    // load obj files using load_obj() : 0 for bunny object and 1 for sports car object
    tinyobj::attrib_t attrib;
    
    if (model == 0) { // choosing bunny.obj
        is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/HW11/obj_models/bunny.obj", "/Users/ck/Desktop/ComputerGraphics/HW11/obj_models/", vertices, normals, vertex_map, material_map, attrib, shapes, materials);
        if (is_obj_valid)
            printf("Load bunny object\n");
    }
    
    if (model == 1) { // choosing sportsCar.obj
        is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/HW11/obj_models/sportsCar.obj", "/Users/ck/Desktop/ComputerGraphics/HW11/obj_models/", vertices, normals, vertex_map, material_map, attrib, shapes, materials);
        if (is_obj_valid)
            printf("Load sports car object\n");
    }
    
    if (!is_obj_valid)
        printf("Error loading object file\n");
    
    colors.resize(vertices.size());

    ShaderInfo shaders[2][3] = {
        {       {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW11/HW11/phong.vert"},
                {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW11/HW11/phong.frag"},
                {GL_NONE, NULL},
        },
        {       {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW11/HW11/phong.vert"},
                {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW11/HW11/phong.frag"},
                {GL_NONE, NULL},
        },
    };
    
    
    for (int i = 0; i < 2; ++i)
        program[i] = LoadShaders(shaders[i]);
    
    // Generate vertex array objects
    glGenVertexArrays(2, vao);
    glGenBuffers(2, vbo);
    
    glBindVertexArray(vao[0]);
    bind_buffer(vbo[0], vertices, program[0], "vPosition", 3);
    bind_buffer(vbo[1], normals, program[0], "vNormal", 3);
//    bind_buffer(vbo[2], colors, program[0], "vColor", 3);
    
    // Bind vertex array objects
    for (int i = 1; i < 2; ++i) {
        glBindVertexArray(vao[i]);
        bind_buffer(vbo[0], program[i], "vPosition", 3);
        bind_buffer(vbo[1], program[i], "vNormal", 3);
//        bind_buffer(vbo[2], program[i], "vColor", 3);
    }
    
    // Grid
//    get_grid(grid_pos, 5, 5, 10, 10);
//
//    int N = (int) grid_pos.size();
//
//    grid_clrs.resize(N);
//    
//    for (int i = 0; i < N; ++i) {
//        grid_clrs[i] = 1.0f;
//    }
//
//    glGenVertexArrays(1, &grid_vao);
//    glBindVertexArray(grid_vao);
//
//    glGenBuffers(2, grid_vbo);
//    for (int i = 0; i < 2; ++i) {
//        bind_buffer(grid_vbo[0], grid_pos, program[i], "vPosition", 3);
//        bind_buffer(grid_vbo[1], grid_clrs, program[i], "vColor", 3);
//    }
    
    glEnable(GL_DEPTH_TEST);    // Enable depth test
    glDepthFunc(GL_LESS);       // Accept fragment that is closer to the camera than the existing one
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void display() {

    using namespace glm;
    using namespace std;
    using namespace tinyobj;

    glClearColor(0.8, 0.8, 0.8, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, 1024, 1024);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1, 1);

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    double aspect = 1.0 * width / height;
    
    mat4 V = camera.get_viewing();                  // viewing transformation matrix
    mat4 P = camera.get_projection(aspect);         // projection transformation matrix
    mat4 M(1.0f);
    
    vec4 LightPos_wc = vec4(10, 10, 3, 1);
    vec3 LightPos_ec = vec3(V * LightPos_wc);
        
    
    if ( is_obj_valid ) {
        // Bind the current program before assigning any value to its uniform variables
        
        glUseProgram(program[shading_mode]);
        GLint location_m = glGetUniformLocation(program[shading_mode], "M");
        GLint location_v = glGetUniformLocation(program[shading_mode], "V");
        GLint location_p = glGetUniformLocation(program[shading_mode], "P");
        GLint location_lp = glGetUniformLocation(program[shading_mode], "LightPos_ec");
        
        glUniformMatrix4fv(location_m, 1, GL_FALSE, value_ptr(M));
        glUniformMatrix4fv(location_v, 1, GL_FALSE, value_ptr(V));
        glUniformMatrix4fv(location_p, 1, GL_FALSE, value_ptr(P));
        glUniform3fv(location_lp, 1, value_ptr(LightPos_ec));
        
        // Draw the imported model
        draw_obj_model(0);
    }
        
    if ( is_wireframe_visible ) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUseProgram(program[shading_mode]);
        draw_obj_model(1);
    }
    
    if ( grid ) {
        glBindVertexArray(grid_vao);
        glDrawArrays(GL_LINES, 0, grid_pos.size());
    }
    
    glDisable(GL_POLYGON_OFFSET_FILL);
    
    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();

}

