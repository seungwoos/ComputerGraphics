
//
//  main.cpp
//  term
//
//  Created by HyeWon Jeon on 2020/06/04.
//  Copyright © 2020 HyeWon Jeon. All rights reserved.
//

//### viewing camera control
//- tumble tool -> mouse middle button + option(alt) + drag
//- track tool -> mouse left button + option(alt) + drag
//- zoom tool -> mouse left button + control + drag (in x axis - left to right)
//- dolly tool -> mouse middle button + control + drag (in x axis - left to right)
// rotate tool -> mouse left button + drag

#include <ode/ode.h>
#include <iostream>
#include <vector>

#include <stdlib.h>
#include <string.h>
#include <GL/glew.h> // in main.cpp

#include "/Users/ck/Desktop/ComputerGraphics/term/GLUT.framework/Headers/glut.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>

#define _USE_MATH_DEFINES
#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "LoadShaders.h"
#include "loadobj.h"
#include "premitive.hpp"
#include "util.hpp"

// OpenAL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <OpenAL/alut.h>

#define NUM_BUFFERS 4
#define NUM_SOURCES 4
#define NUM_ENVIRONMENTS 1

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0, 0.0,1.0,0.0};

ALfloat source0Pos[]={ -2.0, 0.0, 0.0};
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

ALuint  buffer[NUM_BUFFERS];
ALuint  source[NUM_SOURCES];
ALuint  environment[NUM_ENVIRONMENTS];

ALsizei size,freq;
ALenum  format;
ALvoid  *data;

using namespace std ;
using namespace tinyobj ;
using namespace glm ;

typedef unsigned long size_t ;

#define UVARS(name)         glGetUniformLocation(program, name)
#define dist(x1, y1, z1, x2, y2, z2)    sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2))

#define MAP_FIND(map_obj, item)\
   ((map_obj).find(item) != (map_obj).end())

#define IS_MIPMAP(flag) \
   ((flag) == GL_LINEAR_MIPMAP_LINEAR || \
   (flag) == GL_LINEAR_MIPMAP_NEAREST || \
   (flag) == GL_NEAREST_MIPMAP_LINEAR || \
   (flag) == GL_NEAREST_MIPMAP_NEAREST)

// variables for simulation world and collision handling
static dWorldID ode_world; // simulation world
static dSpaceID ode_space; // represent collision space
static dJointGroupID ode_contactgroup; // a group of contact joints
static bool pause = true ;

// variables for plane underground
static dGeomID ode_plane_geom;
static dTriMeshDataID ode_plane_data;
static vector<dTriIndex> ode_plane_index;

// for box
static dGeomID ode_box_geom;
static dTriMeshDataID ode_box_data;
static vector<dTriIndex> ode_box_index;
   
// ball
static dBodyID ode_ball_body;
static dGeomID ode_ball_geom;

/* define variables */
GLuint program ;
int shading_mode = 1;
GLuint width, height ;
double aspect ;
int state ;
int star_count = 0 ;
int ball_mode=0 ;
float gravity ;


int button_pressed[3] = {GLUT_UP, GLUT_UP, GLUT_UP} ;
int mouse_pos[2] = {0, 0} ;

// defined objects
obj ball ;
obj maze ;
obj box, box_star ;
obj star1, star2, star3, star4 ;

float Ia = 1.0f ;

/* define functions */
void init() ;
void ball_init(GLint program) ;
void maze_init(GLint program) ;
void box_init(GLint program) ;
void box_star_init(GLint program) ;
void star_init(obj *o, GLint program) ;
void ode_init() ;
void sound_init() ;

bool check_star(obj *o) ;
void check_finish() ;
void check_falling() ;

void keyboard(unsigned char key, int x, int y) ;
void mouse(int button, int state, int x, int y) ;
void motion(int x, int y) ;
static void nearCallback(void *data, dGeomID o1, dGeomID o2) ;

void display() ;
void draw_obj_model(obj o, int color_mode, int object_code);
void render(int color_mode=0);

Camera camera ;
bool falling = false;

/* implement functions */
int main(int argc, char * argv[]) {
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(1024, 1024) ;
    glutCreateWindow(argv[0]) ;
    
    width = glutGet(GLUT_WINDOW_WIDTH) ;
    height = glutGet(GLUT_WINDOW_HEIGHT) ;
    aspect = 1.0 * width / height ;
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit() ;
    if (err != GLEW_OK) {
        fprintf(stderr, "error: %s\n", glewGetErrorString(err)) ;
        exit(EXIT_FAILURE) ;
    }
    
    printf("===== Choose ball =====\n") ;
    printf("   [1] Basket ball.\n") ;
    printf("   [2] Tennis ball.\n") ;
    printf("   [3] Moon.\n") ;
    printf("   [4] Earth.\n> ") ;
    scanf("%d", &ball_mode) ;
    if (ball_mode<1 || 4<ball_mode) {
        printf("No ball.\n") ;
        exit(-1) ;
    }
    
    init() ;
    sound_init();
    ball_init(program) ;
    box_init(program) ;
    box_star_init(program) ;
    maze_init(program) ;
    
    star_init(&star1, program) ;
    star_init(&star2, program) ;
    star_init(&star3, program) ;
    star_init(&star4, program) ;
    ode_init() ;

    /* callbacks */
    glutDisplayFunc(display) ;
    glutKeyboardFunc(keyboard) ;
    glutMouseFunc(mouse) ;
    glutMotionFunc(motion) ;
    alSourcePlay(source[0]);
    glutMainLoop() ;
    
    // closing ode and openAL
    dJointGroupDestroy(ode_contactgroup);
    dSpaceDestroy(ode_space);
    dWorldDestroy(ode_world);
    dCloseODE();
    
    alDeleteBuffers(NUM_BUFFERS, buffer) ;
    alDeleteSources(NUM_SOURCES, source) ;
    alutExit();
}

void init() {
    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/term/term/term/viewing.vert" },
        { GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/term/term/term/viewing.frag" },
        { GL_NONE, NULL }
    } ;
    program = LoadShaders(shaders);
    glUseProgram(program) ;
    
    glEnable(GL_DEPTH_TEST) ;
    glDepthFunc(GL_LESS) ;
    glEnable(GL_CULL_FACE) ;
    glCullFace(GL_BACK) ;
}

void sound_init() {
    alutInit(0, NULL);

    alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);

    alGetError(); // clear any error messages
    
    if(alGetError() != AL_NO_ERROR){
        printf("- Error creating buffers !!\n");
        exit(1);
    }
    else {
        printf("init() - No errors yet.");
    }
    
    // Generate buffers, or else no sound will happen!
    alGenBuffers(NUM_BUFFERS, buffer);
    
    // source[0] -- background music
    alutLoadWAVFile("/Users/ck/Desktop/ComputerGraphics/term/sound/bgm.wav", &format, &data, &size, &freq);
    alBufferData(buffer[0], format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    
    // source[1] -- when the ball hits the star
    alutLoadWAVFile("/Users/ck/Desktop/ComputerGraphics/term/sound/star.wav", &format, &data, &size, &freq);
    alBufferData(buffer[1], format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);

    // source[2] -- when the ball falls down
    alutLoadWAVFile("/Users/ck/Desktop/ComputerGraphics/term/sound/falling.wav", &format, &data, &size, &freq);
    alBufferData(buffer[2], format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);
    
    // source[3] -- when the user deletes the box
    alutLoadWAVFile("/Users/ck/Desktop/ComputerGraphics/term/sound/box.wav", &format, &data, &size, &freq);
    alBufferData(buffer[3], format, data, size, freq);
    alutUnloadWAV(format, data, size, freq);

    alGetError(); /* clear error */
    alGenSources(NUM_SOURCES, source);

    if(alGetError() != AL_NO_ERROR) {
        printf("- Error creating sources !!\n");
        exit(2);
    } else {
        printf("init - no errors after alGenSources\n");
    }
    
    // BGM
    alSourcef(source[0], AL_PITCH, 1.0f);
    alSourcef(source[0], AL_GAIN, 0.5f);
    alSourcefv(source[0], AL_POSITION, source0Pos);
    alSourcefv(source[0], AL_VELOCITY, source0Vel);
    alSourcei(source[0], AL_BUFFER,buffer[0]);
    alSourcei(source[0], AL_LOOPING, AL_TRUE);

    // Ball hits the star
    alSourcef(source[1], AL_PITCH, 1.0f);
    alSourcef(source[1], AL_GAIN, 4.0f);
    alSourcefv(source[1], AL_POSITION, source0Pos);
    alSourcefv(source[1], AL_VELOCITY, source0Vel);
    alSourcei(source[1], AL_BUFFER,buffer[1]);
    alSourcei(source[1], AL_LOOPING, AL_FALSE);
    
    // Ball falls down
    alSourcef(source[2], AL_PITCH, 1.0f);
    alSourcef(source[2], AL_GAIN, 15.0f);
    alSourcefv(source[2], AL_POSITION, source0Pos);
    alSourcefv(source[2], AL_VELOCITY, source0Vel);
    alSourcei(source[2], AL_BUFFER,buffer[2]);
    alSourcei(source[2], AL_LOOPING, AL_FALSE);
    
    // When the box is deleted
    alSourcef(source[3], AL_PITCH, 1.0f);
    alSourcef(source[3], AL_GAIN, 2.0f);
    alSourcefv(source[3], AL_POSITION, source0Pos);
    alSourcefv(source[3], AL_VELOCITY, source0Vel);
    alSourcei(source[3], AL_BUFFER,buffer[3]);
    alSourcei(source[3], AL_LOOPING, AL_FALSE);

}


void ball_init(GLint program) {
    string file_path ;
    float scale ;
    if (ball_mode==1) {
        file_path = "/Users/ck/Desktop/ComputerGraphics/term/obj/basketball_center.obj" ;
        scale = 0.3 ;
    } else if (ball_mode==2) {
        file_path = "/Users/ck/Desktop/ComputerGraphics/term/obj/tennisball.obj" ;
        scale = 0.2 ;
    } else if (ball_mode==3) {
        file_path = "/Users/ck/Desktop/ComputerGraphics/term/obj/moon.obj" ;
        scale = 0.5 ;
    } else if (ball_mode==4) {
        file_path = "/Users/ck/Desktop/ComputerGraphics/term/obj/earth.obj" ;
        scale = 0.6 ;
    }
    
    ball.is_obj_valid = load_obj(file_path.c_str(), "/Users/ck/Desktop/ComputerGraphics/term/obj/", ball.vertices, ball.normals, ball.vertex_map, ball.material_map, ball.attrib, ball.shapes, ball.materials);
    glActiveTexture(GL_TEXTURE0);
    ball.is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/term/obj/", ball.texcoords, ball.texmap, ball.attrib.texcoords, ball.shapes, ball.materials, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ;
    
    ball.colors.resize(ball.vertices.size());
    std::transform(ball.vertices.begin(), ball.vertices.end(), ball.vertices.begin(), std::bind1st(std::multiplies<tinyobj::real_t>() , scale));
    
    real_t _min=100, _max=-100 ;
    for (int i=0 ; i<ball.vertices.size() ; i+=3) {
        real_t v = ball.vertices[i] ;
        _min = (_min < v) ? _min : v ;
        _max = (_max > v) ? _max : v ;
    }
    ball.r = (_max - _min) / 2.0f ;

    // Generate vertex array objects.
    glGenVertexArrays(1, &ball.vao);
    glGenBuffers(4, ball.vbo);
    glBindVertexArray(ball.vao);
    
    bind_buffer(ball.vbo[0], ball.vertices, program, "vPosition", 3);
    bind_buffer(ball.vbo[1], ball.normals, program, "vNormal", 3);
    bind_buffer(ball.vbo[2], ball.colors, program, "vColor", 3);
    bind_buffer(ball.vbo[3], ball.texcoords, program, "vTexcoord", 2);
}

void maze_init(GLint program) {
    maze.is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/term/obj/Maze.obj", "/Users/ck/Desktop/ComputerGraphics/term/obj/", maze.vertices, maze.normals, maze.vertex_map, maze.material_map, maze.attrib, maze.shapes, maze.materials);
    glActiveTexture(GL_TEXTURE0);
    maze.is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/term/obj/", maze.texcoords, maze.texmap, maze.attrib.texcoords, maze.shapes, maze.materials, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ;

    maze.colors.resize(maze.vertices.size());
    std::transform(maze.vertices.begin(), maze.vertices.end(), maze.vertices.begin(), std::bind1st(std::multiplies<tinyobj::real_t>() , 5));

    // Generate vertex array objects.
    glGenVertexArrays(1, &maze.vao);
    glGenBuffers(4, maze.vbo);
    glBindVertexArray(maze.vao);
    
    bind_buffer(maze.vbo[0], maze.vertices, program, "vPosition", 3);
    bind_buffer(maze.vbo[1], maze.normals, program, "vNormal", 3);
    bind_buffer(maze.vbo[2], maze.colors, program, "vColor", 3);
    bind_buffer(maze.vbo[3], maze.texcoords, program, "vTexcoord", 2);
}

void box_init(GLint program) {
    box.is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/term/obj/crate.obj", "/Users/ck/Desktop/ComputerGraphics/term/obj/", box.vertices, box.normals, box.vertex_map, box.material_map, box.attrib, box.shapes, box.materials);
    glActiveTexture(GL_TEXTURE0);
    box.is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/term/obj/", box.texcoords, box.texmap, box.attrib.texcoords, box.shapes, box.materials, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ;

    box.colors.resize(box.vertices.size());
    std::transform(box.vertices.begin(), box.vertices.end(), box.vertices.begin(), std::bind1st(std::multiplies<tinyobj::real_t>() , 0.7));

    // Generate vertex array objects.
    glGenVertexArrays(1, &box.vao);
    glGenBuffers(4, box.vbo);
    glBindVertexArray(box.vao);
    
    bind_buffer(box.vbo[0], box.vertices, program, "vPosition", 3);
    bind_buffer(box.vbo[1], box.normals, program, "vNormal", 3);
    bind_buffer(box.vbo[2], box.colors, program, "vColor", 3);
    bind_buffer(box.vbo[3], box.texcoords, program, "vTexcoord", 2);
}


void box_star_init(GLint program) {
    box_star.is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/term/obj/star.obj", "/Users/ck/Desktop/ComputerGraphics/term/obj/", box_star.vertices, box_star.normals, box_star.vertex_map, box_star.material_map, box_star.attrib, box_star.shapes, box_star.materials);
    glActiveTexture(GL_TEXTURE0);
    box_star.is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/term/obj/", box_star.texcoords, box_star.texmap, box_star.attrib.texcoords, box_star.shapes, box_star.materials, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ;

    box_star.colors.resize(box_star.vertices.size());
    std::transform(box_star.vertices.begin(), box_star.vertices.end(), box_star.vertices.begin(), std::bind1st(std::multiplies<tinyobj::real_t>() , 0.3));

    // Generate vertex array objects.
    glGenVertexArrays(1, &box_star.vao);
    glGenBuffers(4, box_star.vbo);
    glBindVertexArray(box_star.vao);
    
    bind_buffer(box_star.vbo[0], box_star.vertices, program, "vPosition", 3);
    bind_buffer(box_star.vbo[1], box_star.normals, program, "vNormal", 3);
    bind_buffer(box_star.vbo[2], box_star.colors, program, "vColor", 3);
    bind_buffer(box_star.vbo[3], box_star.texcoords, program, "vTexcoord", 2);
}

void star_init(obj *s, GLint program) {
    s->is_obj_valid = load_obj("/Users/ck/Desktop/ComputerGraphics/term/obj/star.obj", "/Users/ck/Desktop/ComputerGraphics/term/obj/", s->vertices, s->normals, s->vertex_map, s->material_map, s->attrib, s->shapes, s->materials);
    glActiveTexture(GL_TEXTURE0);
    s->is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/term/obj/", s->texcoords, s->texmap, s->attrib.texcoords, s->shapes, s->materials, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR) ;
    
    s->colors.resize(s->vertices.size());
    std::transform(s->vertices.begin(), s->vertices.end(), s->vertices.begin(), std::bind1st(std::multiplies<tinyobj::real_t>() , 0.7));
    
    real_t _min=100, _max=-100 ;
    for (int i=0 ; i<s->vertices.size() ; i+=3) {
        real_t v = s->vertices[i] ;
        _min = (_min < v) ? _min : v ;
        _max = (_max > v) ? _max : v ;
    }
    s->r = (_max - _min) / 2.0f ;
    
    // Generate vertex array objects.
    glGenVertexArrays(1, &(s->vao));
    glGenBuffers(4, s->vbo);
    glBindVertexArray(s->vao);
    
    bind_buffer(s->vbo[0], s->vertices, program, "vPosition", 3);
    bind_buffer(s->vbo[1], s->normals, program, "vNormal", 3);
    bind_buffer(s->vbo[2], s->colors, program, "vColor", 3);
    bind_buffer(s->vbo[3], s->texcoords, program, "vTexcoord", 2);
}

void ode_init() {
    int mass ;
    
    if (ball_mode==1) {
        mass = 10 ;
        gravity = - 9.8 ;
    } else if (ball_mode==2) {
        mass = 5 ;
        gravity = - 9.8 ;
    } else if (ball_mode==3) { // moon
        mass = 20 ;
        gravity = -1.62 ;
    } else if (ball_mode==4) { // earch
        mass = 40 ;
        gravity = -20 ;
    }
    
    dMatrix3 R;
    dMass m; // mass obj
    dRSetIdentity(R);

    dInitODE2(0);
    ode_world = dWorldCreate();              // Make a simulation world. and assign number to ode_world
    ode_space = dSimpleSpaceCreate(0);       // Make a collision space. 적은 수의 obj들에 적합
    ode_contactgroup = dJointGroupCreate(0); // Make a contact group.
    dWorldSetGravity(ode_world, 0, gravity, 0); // Set gravity.
    //dWorldSetGravity(ode_world, 0, -0.1, 0); // Set gravity.
    
    // for plane
    int n = (int)(maze.vertices.size() / 3);
    ode_plane_index.resize(n);
    for (int i = 0; i < n; ++i)
        ode_plane_index[i] = i;
    
    ode_plane_data = dGeomTriMeshDataCreate(); // transfer vertex attrib to ODE
    dGeomTriMeshDataBuildSingle(ode_plane_data, maze.vertices.data(), 3*sizeof(float), n, ode_plane_index.data(), n, 3*sizeof(dTriIndex));
    ode_plane_geom = dCreateTriMesh(ode_space, ode_plane_data, 0, 0, 0);
    
    // for box
    int k = (int)(box.vertices.size() / 3);
    ode_box_index.resize(k);
    for (int i = 0; i < k; ++i)
        ode_box_index[i] = i;
    
    ode_box_data = dGeomTriMeshDataCreate(); // transfer vertex attrib to ODE
    dGeomTriMeshDataBuildSingle(ode_box_data, box.vertices.data(), 3*sizeof(float), k, ode_box_index.data(), k, 3*sizeof(dTriIndex));
    ode_box_geom = dCreateTriMesh(ode_space, ode_box_data, 0, 0, 0);
    
    // for ball
    ode_ball_body = dBodyCreate(ode_world);
    dBodySetPosition(ode_ball_body, 0, 0, 0); // initially set the trimesh position to the origin
    dBodySetRotation(ode_ball_body, R);
    dBodySetLinearVel(ode_ball_body, 0, 0, 0);
    dBodySetAngularVel(ode_ball_body, 0, 0, 0);

    ode_ball_geom = dCreateSphere(ode_space, ball.r);
    dGeomSetBody(ode_ball_geom, ode_ball_body);
    dMassSetSphereTotal(&m, mass, ball.r);

    dBodySetMass(ode_ball_body, &m);
    dBodySetPosition(ode_ball_body, 2, 2, -2);
}

void display() {
    double stepsize = 0.005; // 5ms simulation step size
    double dt = dsElapsedTime(); // 이전 call에서 얼마나 시간 지났는지 ..
    int no_of_steps = (int)ceilf(dt / stepsize);
    
    for (int i = 0; !pause && i < no_of_steps; ++i) { // we are not in pause state, do the simulation
        dSpaceCollide(ode_space, 0, &nearCallback); // nearCallback -> collision handler (callback function)
        dWorldQuickStep(ode_world, stepsize);
        dJointGroupEmpty(ode_contactgroup); // remove all contact joint constraints ...
    }
    
    render() ;
    check_finish() ;
    check_falling() ;
}

bool check_star(obj *s) {
    const dReal* pos = dBodyGetPosition(ode_ball_body) ;
    
    if (dist(s->x, s->y, s->z, pos[0], pos[1], pos[2])<s->r) {
        alSourcePlay(source[1]) ;
        s->is_obj_valid = false ;
        box_star.is_obj_valid = false ;
        star_count += 1 ;
        return false ;
    }
    return true ;
}

void check_finish() {
    if (!star1.is_obj_valid && !star2.is_obj_valid && !star3.is_obj_valid && !star4.is_obj_valid) {
        star1.is_obj_valid = true ;
        star2.is_obj_valid = true ;
        star3.is_obj_valid = true ;
        star4.is_obj_valid = true ;
        box.is_obj_valid = true ;
        star_count = 0 ;
        box_star.is_obj_valid = true ;
        pause = true ;
        camera.init() ;
        ode_init() ;
    }
}

void check_falling() {
    const dReal* pos = dBodyGetPosition(ode_ball_body) ;
    if (!falling && pos[1] < -5.0f){
        falling = true;
        alSourceStop(source[0]) ;
        alSourcePlay(source[2]) ;
    }
    
    if (falling && pos[1] < gravity*100.0f) {
        dJointGroupDestroy(ode_contactgroup) ;
        dSpaceDestroy(ode_space) ;
        dWorldDestroy(ode_world) ;
        dCloseODE() ;
        
        alDeleteBuffers(NUM_BUFFERS, buffer) ;
        alDeleteSources(NUM_SOURCES, source) ;
        alutExit() ;
        
        exit(1) ;
    }
}


void draw_obj_model(obj o, int color_mode, int object_code) {
    glUniform1i(UVARS("ObjectCode"), object_code) ;
    glUniform1i(UVARS("ColorMode"), color_mode) ;
    glBindVertexArray(o.vao) ;
    
    for (size_t i = 0; i < o.shapes.size(); ++i) {
        for (size_t j = 0; j < o.material_map[i].size(); ++j) {
            int m_id = o.material_map[i][j] ;
            if (m_id < 0) {
                glUniform1f(UVARS("n"), 10.0f);
                glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f) ;
                glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f) ;
                glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f) ;
            } else {
                glUniform1f(UVARS("n"), o.materials[m_id].shininess);
                glUniform3fv(UVARS("Ka"), 1, o.materials[m_id].ambient) ;
                glUniform3fv(UVARS("Kd"), 1, o.materials[m_id].diffuse) ;
                glUniform3fv(UVARS("Ks"), 1, o.materials[m_id].specular) ;
            }
            // apply texture
            auto texitem = o.texmap.find(o.materials[m_id].diffuse_texname);
            if (o.is_tex_valid && texitem != o.texmap.end()) {
                glActiveTexture(GL_TEXTURE0) ;
                glBindTexture(GL_TEXTURE_2D, texitem->second) ;
                glUniform1i(UVARS("sampler"), 0) ;
                glUniform1i(UVARS("noSample"), 0) ;
            } else {
                glUniform1i(UVARS("noSample"), 1) ;
            }
            glDrawArrays(GL_TRIANGLES, o.vertex_map[i][j], o.vertex_map[i][j+1] - o.vertex_map[i][j]) ;
        }
    }
}

void render(int color_mode) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) ;
    //glViewport(0,0,1024,1024);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) ;
    glClearColor(0.7, 0.7, 0.7, 1) ;
    glPolygonOffset(1, 1) ;
    
    glUniform3f(UVARS("LightPos_ec"), 7, 7, 5) ;
    
    GLfloat theta = 0.000005f * clock() ;
    mat4 V = camera.get_viewing() ;
    mat4 P = camera.get_projection() ;
    mat4 M(1.0f), T(1.0f) ;
    
    glUniformMatrix4fv(UVARS("V"), 1, GL_FALSE, value_ptr(V)) ;
    glUniformMatrix4fv(UVARS("P"), 1, GL_FALSE, value_ptr(P)) ;
    glUniformMatrix4fv(UVARS("M"), 1, GL_FALSE, value_ptr(M)) ;
    glUniform1i(UVARS("mode"), shading_mode) ;
    glUniform3f(UVARS("Ia"), Ia, Ia, Ia);

    if (maze.is_obj_valid) {
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(compute_geom_transf(ode_plane_geom))) ;
        draw_obj_model(maze, color_mode, 2) ;
    }
    
    if (box.is_obj_valid) {
        dMatrix3 cur_mat ;
        dQSetIdentity(cur_mat) ;

        T = compute_geom_transf(ode_plane_geom) ;
        T = translate(T, vec3(-2, 0.4, -2)) ;
        
        for (int i=0 ; i<3 ; ++i) {
            for (int j=0 ; j<3 ; ++j)
                cur_mat[j*4+i] = T[i][j] ;
        }
        
        dGeomSetPosition(ode_box_geom, T[3][0], T[3][1], T[3][2]) ;
        dGeomSetRotation(ode_box_geom, cur_mat) ;
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
        draw_obj_model(box, color_mode, 5) ;
        
        if (star_count>0) {
            T = translate(T, vec3(0.0, 0.5, 0.0)) ;
            T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)) ;
            glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
            draw_obj_model(box_star, color_mode, 3) ;
        }
    }
    
    if (star1.is_obj_valid && check_star(&star1)) {
        T = compute_geom_transf(ode_plane_geom) ;
        T = translate(T, vec3(0.4, 0.1, 0.4)) ;
        T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)) ;
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
        draw_obj_model(star1, color_mode, 3) ;
        
        vec4 pos = vec4(0.0f) ;
        pos[3] = 1.0f ;
        pos = T*pos ;
        star1.x = pos[0] ; star1.y = pos[1] ; star1.z = pos[2] ;
    }
    
    if (star2.is_obj_valid && check_star(&star2)) {
        T = compute_geom_transf(ode_plane_geom) ;
        T = translate(T, vec3(1.2, 0.1, 0.4)) ;
        T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)) ;
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
        draw_obj_model(star2, color_mode, 3) ;

        vec4 pos = vec4(0.0f) ;
        pos[3] = 1.0f ;
        pos = T*pos ;
        star2.x = pos[0] ; star2.y = pos[1] ; star2.z = pos[2] ;
    }

    if (star3.is_obj_valid && check_star(&star3)) {
        T = compute_geom_transf(ode_plane_geom) ;
        T = translate(T, vec3(2.0, 0.1, 2.0)) ;
        T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)) ;
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
        draw_obj_model(star3, color_mode, 3) ;

        vec4 pos = vec4(0.0f) ;
        pos[3] = 1.0f ;
        pos = T*pos ;
        star3.x = pos[0] ; star3.y = pos[1] ; star3.z = pos[2] ;
    }

    if (star4.is_obj_valid && check_star(&star4)) {
        T = compute_geom_transf(ode_plane_geom) ;
        T = translate(T, vec3(0.4, 0.1, 1.2)) ;
        T = rotate(T, theta, vec3(0.0f, 1.0f, 0.0f)) ;
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(T)) ;
        draw_obj_model(star4, color_mode, 3) ;

        vec4 pos = vec4(0.0f) ;
        pos[3] = 1.0f ;
        pos = T*pos ;
        star4.x = pos[0] ; star4.y = pos[1] ; star4.z = pos[2] ;
    }
    
    if (ball.is_obj_valid) {
        glUniformMatrix4fv(UVARS("T"), 1, GL_FALSE, value_ptr(compute_modelling_transf(ode_ball_body))) ;
        draw_obj_model(ball, color_mode, 1) ;
    }
    
    if (color_mode != 2) {
        glFlush() ;
        glutSwapBuffers() ;
        glutPostRedisplay() ;
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1':
            shading_mode = 1 ;
            break ;
        case '2':
            shading_mode = 2 ;
            break ;
        case 'P':
            pause = !pause ;
            break ;
        case '+' :
            Ia = (Ia < 0.99) ? Ia+0.01 : 1.0f ;
            break ;
        case '-' :
            Ia = (Ia > 0.01) ? Ia-0.01 : 0.0f ;
            break ;
    }
    printf("shading mode: %d | curr key : %c \n", shading_mode, key);
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    render(2);
    glFlush();

    button_pressed[button] = state ;
    mouse_pos[0] = x ;
    mouse_pos[1] = y ;
        
    if (state == GLUT_UP) {
        unsigned char res[4];
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        
        glReadPixels(x, height-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
        
        switch(res[0]) {
            case 1:
                ode_init();
                break;
            case 5:
                if (star_count>0) {
                    box.is_obj_valid = false ;
                    dGeomDestroy(ode_box_geom) ;
                    alSourcePlay(source[3]) ;
                }
                break ;
        }
    }
}

void motion(int x, int y) {
    int modifiers = glutGetModifiers() ;
    int is_option_active = modifiers & GLUT_ACTIVE_ALT ;
    int is_control_active = modifiers & GLUT_ACTIVE_CTRL ;
    int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT ;
    
    int w = glutGet(GLUT_WINDOW_WIDTH) ;
    int h = glutGet(GLUT_WINDOW_HEIGHT) ;
    GLfloat dx = 1.0f * (x-mouse_pos[0]) / w ;
    GLfloat dy = -1.0f * (y-mouse_pos[1]) / h ;
    
    // add code to deal with mouse motion
    if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN) { // track tool
        if (is_option_active) { // track tool
            mat4 VT = transpose(camera.get_viewing()) ;
            camera.eye += vec3(-dx*VT[0] + (-dy)*VT[1]) ;
            camera.center += vec3(-dx*VT[0] + (-dy)*VT[1]) ;
            
        } else if (is_control_active) { // zoom tools
            vec3 disp = camera.eye - camera.center ;
            if (dx>0)
                camera.eye = camera.center + 0.95f*disp ; // make vector shorten
            else
                camera.eye = camera.center + 1.05f*disp ;
        } else if (is_shift_active) { // tumble tools
            vec4 disp(camera.eye-camera.center, 1) ;
            
            GLfloat alpha = 2.0f ;
            mat4 V = camera.get_viewing() ;
            mat4 Rx = rotate(mat4(1.0f), alpha*dy, vec3(transpose(V)[0])) ; // elevation
            mat4 Ry = rotate(mat4(1.0f), -alpha*dx, vec3(0, 1, 0)) ; // azimuth
            mat4 R = Ry*Rx ;
            camera.eye = camera.center + vec3(R*disp) ;
            camera.up = mat3(R) * camera.up ;
        } else { // rotate maze
            GLfloat alpha = 2.0f ;
            dMatrix3 cur_mat ;
            dQSetIdentity(cur_mat) ;
            
            mat4 glm_mat = compute_geom_transf(ode_plane_geom) ;
            
            if (abs(dx) > abs(dy)) {
                glm_mat = rotate(glm_mat, -alpha*dx, vec3(1.0f, 0.0f, 0.0f)) ;
            } else {
                glm_mat = rotate(glm_mat, alpha*dy, vec3(0.0f, 0.0f, 1.0f)) ;
            }
                        
            for (int i=0 ; i<3 ; ++i) {
                for (int j=0 ; j<3 ; ++j)
                    cur_mat[j*4+i] = glm_mat[i][j] ;
             }
            dGeomSetRotation(ode_plane_geom, cur_mat) ;
        }
    }
    
    mouse_pos[0] = x ;
    mouse_pos[1] = y ;
    glutPostRedisplay() ;
}

static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
    const int N = 100;
    dContact contact[N];
    int n = dCollide(o1, o2, N, &(contact[0].geom), sizeof(dContact));
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            contact[i].surface.mode = dContactSoftERP | dContactSoftCFM;
            contact[i].surface.mu = 0.8; // Friction coefficient
            contact[i].surface.soft_erp = 0.9;
            contact[i].surface.soft_cfm = 0.01;
            
            dJointID c = dJointCreateContact(ode_world, ode_contactgroup, &contact[i]);
            dBodyID body1 = dGeomGetBody(contact[i].geom.g1);
            dBodyID body2 = dGeomGetBody(contact[i].geom.g2);
 
            dJointAttach(c, body1, body2);
        }
    }
}

