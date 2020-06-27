//
//  main.cpp
//  HW12
//
//  Created by 승우 on 2020/05/25.
//  Copyright © 2020 승우. All rights reserved.
//

#include "loadobj.h"
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "/Users/ck/Desktop/ComputerGraphics/HW12/GLUT.framework/Headers/glut.h"
#include "LoadShaders.h"

#define _USE_MATH_DEFINES // to use M_PI
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include <iostream>
#define UVARS(name) glGetUniformLocation(program, name)

// For HW12
#define MAP_FIND(map_obj, item)\
    ((map_obj).find(item) != (map_obj).end())
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define IS_MIPMAP(flag)\
    ((flag) == GL_LINEAR_MIPMAP_LINEAR || \
     (flag) == GL_LINEAR_MIPMAP_NEAREST || \
     (flag) == GL_NEAREST_MIPMAP_LINEAR || \
     (flag) == GL_NEAREST_MIPMAP_NEAREST)

using namespace glm;
using namespace std;
using namespace tinyobj;

// Global variables
enum {MODEL_EARTH, MODEL_MOON, NUM_OF_MODELS};

const char* model_files[NUM_OF_MODELS] = {
    "/Users/ck/Desktop/ComputerGraphics/HW12/earth_moon_models/earth.obj", "/Users/ck/Desktop/ComputerGraphics/HW12/earth_moon_models/moon.obj"
};

float model_scales[NUM_OF_MODELS] = {1.0f, 0.3f};

struct PlanetState {
    GLfloat theta;
    GLfloat orbit_theta;
    GLfloat orbit_radius;
    
    PlanetState():
        theta(0),
        orbit_theta(0),
        orbit_radius(0){}
    
    mat4 get_transf() {
        mat4 M(1.0f);
        M = rotate(M, orbit_theta, vec3(0.1f, 1.f, 0.f));
        M = translate(M, vec3(orbit_radius, 0, 0));
        M = rotate(M, theta, vec3(0.f, 1.f, 0.f));
        return M;
    }
};

int program;
GLuint vao[NUM_OF_MODELS];
GLuint vbo[NUM_OF_MODELS][4];

typedef std::vector<GLfloat> GLvec;

PlanetState mstate[NUM_OF_MODELS];
vector<real_t> vertices[NUM_OF_MODELS];
vector<real_t> normals[NUM_OF_MODELS];
vector<real_t> colors[NUM_OF_MODELS];
vector<vector<size_t>> vertex_map[NUM_OF_MODELS];
vector<vector<size_t>> material_map[NUM_OF_MODELS];
vector<shape_t> shapes[NUM_OF_MODELS];
vector<material_t> materials[NUM_OF_MODELS];
vector<real_t> texcoords[NUM_OF_MODELS];
map<string, size_t> texmap[NUM_OF_MODELS];

bool is_obj_valid = false;
bool is_tex_valid = false;

void init();
void display();

static bool has_file(const char* filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (fp == 0) {
        fclose(fp);
        return false;
    }
    return true;
}

GLuint generate_tex(const char* tex_file_path, GLint min_filter, GLint mag_filter) {
    int width, height, num_of_components;
    unsigned char* image = stbi_load(tex_file_path, &width, &height, &num_of_components, STBI_default);
    
    if (!image) {
        fprintf(stderr, "Failed to open %s\n", tex_file_path);
        return false;
    }
    
    // Generate a tecture object and set its parameters.
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    
    bool is_supported = true;

    switch(num_of_components) {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            break;
        default:
            is_supported = false;
            break;
    }
    
    if (IS_MIPMAP(min_filter) || IS_MIPMAP(mag_filter))
        glGenerateMipmap(GL_TEXTURE_2D);
    
    // Release the loaded image data.
    stbi_image_free(image);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    if (!is_supported) {
        fprintf(stderr, "Unsupported image format: %d components\n", num_of_components);
        glDeleteTextures(1, &texture_id);
        texture_id = -1;
    }
    
    return texture_id;
}

bool load_tex(const char* basedir, vector<real_t>& texcoords_out, map<string, size_t>& texmap_out, const vector<real_t>& texcoords, const vector<shape_t>& shapes, const vector<material_t>& materials, GLint min_filter=GL_LINEAR_MIPMAP_LINEAR, GLint mag_filter=GL_LINEAR_MIPMAP_LINEAR) {
    
    // 1. Compute valid texture coordinates for each vertex.
    size_t total_num_of_vertices = 0;
    size_t num_of_shapes = shapes.size();
    
    for (size_t s = 0; s < num_of_shapes; ++s) {
        total_num_of_vertices += shapes[s].mesh.indices.size();
    }
    texcoords_out.resize(total_num_of_vertices * 2);
    
    real_t* texcoords_dst_ptr = texcoords_out.data();
    const real_t* texcoords_src_ptr = texcoords.size() > 0 ? texcoords.data() : NULL;
    
    for (size_t s = 0; s < num_of_shapes; ++s) {
        const mesh_t& mesh = shapes[s].mesh;
        size_t num_of_faces = mesh.indices.size() / 3;
        for (size_t f = 0; f < num_of_faces; ++f) {
            // Get indices to the three vertices of a triangle
            int idx[3] = {
                mesh.indices[3 * f + 0].texcoord_index,
                mesh.indices[3 * f + 1].texcoord_index,
                mesh.indices[3 * f + 2].texcoord_index
            };
            
            // Compute and copy valid texture coordinaates
            real_t tc[3][2];
            if (texcoords_src_ptr != NULL) {
                if (idx[0] < 0 || idx[1] < 0 || idx[2] < 0) {
                    fprintf(stderr, "Invalid texture coordinate index\n");
                    return false;
                }
                for (size_t i = 0; i < 3; ++i) {
                    memcpy(tc[i], texcoords_src_ptr + idx[i] * 2, sizeof(real_t) * 2);
                    tc[i][1] = 1.0f - tc[i][1];
                }
            }
            else {
                tc[0][0] = tc[0][1] = 0;
                tc[1][0] = tc[1][1] = 0;
                tc[2][0] = tc[2][1] = 0;
            }
            
            memcpy(texcoords_dst_ptr, tc, sizeof(real_t) * 6);
            texcoords_dst_ptr += 6;
        }
    }
    
    // 2. Make a texture object for each material
    GLuint texture_id;
    size_t num_of_materials = materials.size();
    
    for (size_t m = 0; m < num_of_materials; ++m) {
        const material_t& mat = materials[m];
        const string& texname = mat.diffuse_texname;
        if (texname.empty()) continue;
        if (MAP_FIND(texmap_out, texname)) continue;
        
        // Open the texture image file
        string full_texpath = texname;
        if (!has_file(full_texpath.c_str())) {
            full_texpath = basedir + texname;

            if (!has_file(full_texpath.c_str())) {
                fprintf(stderr, "Failed to find %s\n", texname.c_str());
                return false;
            }
        }
        // Generate a texture object.
        texture_id = generate_tex(full_texpath.c_str(), min_filter, mag_filter);
        
        if (texture_id < 0)
            return false;
        
        // Register the texture id.
        texmap_out[texname] =texture_id;
    }
    return true;
}

void draw_obj_model(int model_idx, int color_mode, int object_code=0) {
    glUniform1i(UVARS("ColorMode"), color_mode);
    glUniform1i(UVARS("ObjectCode"), object_code);
    
    glBindVertexArray(vao[model_idx]);
    
    auto& _shapes = shapes[model_idx];
    auto& _materials = materials[model_idx];
    auto& _vertex_map = vertex_map[model_idx];
    auto& _material_map = material_map[model_idx];
    auto& _texmap = texmap[model_idx];
    
    for (size_t i = 0; i < _shapes.size(); ++i) {
        
        for (size_t j = 0; j < _material_map[i].size(); ++j) {
            
            // Set material properties for material j of shape i
            int m_id = _material_map[i][j];
            if (m_id < 0) {
                glUniform1f(UVARS("n"), 10.0f);
                glUniform3f(UVARS("Ka"), 0.3f, 0.3f, 0.3f);
                glUniform3f(UVARS("Kd"), 1.0f, 1.0f, 1.0f);
                glUniform3f(UVARS("Ks"), 0.8f, 0.8f, 0.8f);
            }
            else {
                glUniform1f(UVARS("n"), _materials[m_id].shininess);
                glUniform3fv(UVARS("Ka"), 1, _materials[m_id].ambient);
                glUniform3fv(UVARS("Kd"), 1, _materials[m_id].diffuse);
                glUniform3fv(UVARS("Ks"), 1, _materials[m_id].specular);
                
                auto texitem = _texmap.find(_materials[m_id].diffuse_texname);
                if (texitem != _texmap.end()) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texitem->second);
                    glUniform1i(UVARS("sampler"), 0);
                }
            }
            
            glDrawArrays(GL_TRIANGLES, _vertex_map[i][j], _vertex_map[i][j+1] - _vertex_map[i][j]);
        }
    }
}

void render(int color_mode=0) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glViewport(0, 0, 1024, 1024);
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(1, 1);
    
    mat4 V(1.0f);
    mat4 P(1.0f);
    
   glUseProgram(program);
    
    if (is_obj_valid) {
        for (int i = 0; i < NUM_OF_MODELS; ++i) {
            mat4 M = mstate[i].get_transf();
            
            // set uniform variables of shaders for model i
            GLint location_m = glGetUniformLocation(program, "M");
            GLint location_v = glGetUniformLocation(program, "V");
            GLint location_p = glGetUniformLocation(program, "P");
            
            glUniformMatrix4fv(location_m, 1, GL_FALSE, value_ptr(M));
            glUniformMatrix4fv(location_v, 1, GL_FALSE, value_ptr(V));
            glUniformMatrix4fv(location_p, 1, GL_FALSE, value_ptr(P));

            draw_obj_model(i, color_mode, i+1);
        }
    }
    
    if (color_mode != 2) {
        glutSwapBuffers();
    }
}

void mouse(int button, int state, int x, int y) {
    render(2);
    glDisable(GL_POLYGON_OFFSET_FILL);
    glFlush();
    glutPostRedisplay();
    
    if (state == GLUT_UP) {
        unsigned char res[4];
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        glReadPixels(x, height -y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);
        
        switch(res[0]) {
            case 1: printf("The earth is clicked!\n"); break;
            case 2: printf("The moon is clicked!\n"); break;
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

int main(int argc, char * argv[]) {
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(1024, 1024);
    glutCreateWindow("Texturing Spheres and Picking");
    
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(-1);
    }
    
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMainLoop();
}

void init() {
    
    ShaderInfo shaders[] = {
        {GL_VERTEX_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW12/HW12/phong.vert"},
        {GL_FRAGMENT_SHADER, "/Users/ck/Desktop/ComputerGraphics/HW12/HW12/phong.frag"},
        {GL_NONE, NULL}
    };
    
    program = LoadShaders(shaders);
    mstate[MODEL_MOON].orbit_radius = 1.0f;
    
    for (size_t k = 0; k < NUM_OF_MODELS; ++k) {
        attrib_t attrib;
        
        is_obj_valid = load_obj(model_files[k], "/Users/ck/Desktop/ComputerGraphics/HW12/earth_moon_models/", vertices[k], normals[k], vertex_map[k], material_map[k], attrib, shapes[k], materials[k], model_scales[k]);
        
        glActiveTexture(GL_TEXTURE0);
        is_tex_valid = load_tex("/Users/ck/Desktop/ComputerGraphics/HW12/earth_moon_models/", texcoords[k], texmap[k], attrib.texcoords, shapes[k], materials[k], GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

        colors[k].resize(vertices[k].size());
        
        glGenVertexArrays(1, &vao[k]);
        glBindVertexArray(vao[k]);

        glGenBuffers(4, vbo[k]);
        bind_buffer(vbo[k][0], vertices[k], program, "vPosition_mc", 3);
        bind_buffer(vbo[k][1], normals[k], program, "vNormal_mc", 3);
        bind_buffer(vbo[k][2], colors[k], program, "vColor", 3);
        bind_buffer(vbo[k][3], texcoords[k], program, "vTexcoord", 2);
        
    }
    
    glEnable(GL_DEPTH_TEST); // Enable depth test
    glDepthFunc(GL_LESS); // Accept fragment it is closer to the camera than the existing one
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

}

void display() {
    render();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glFlush();

    for (int i = 0; i < NUM_OF_MODELS; ++i) {
        mstate[i].theta = clock() * 0.00001f;
        if (mstate[i].orbit_radius > 0) {
            mstate[i].orbit_theta = clock() * 0.000001f;
        }
    }
    glutPostRedisplay();
}
