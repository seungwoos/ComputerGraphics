#pragma once

#include "tiny_obj_loader.h"
#include "stb_image.h"
#include <glm/gtc/matrix_transform.hpp>
#include "/Users/ck/Desktop/ComputerGraphics/term/GLUT.framework/Headers/glut.h"
#include <vector>
#include <map>

#define MAP_FIND(map_obj, item)\
   ((map_obj).find(item) != (map_obj).end())

#define IS_MIPMAP(flag) \
   ((flag) == GL_LINEAR_MIPMAP_LINEAR || \
   (flag) == GL_LINEAR_MIPMAP_NEAREST || \
   (flag) == GL_NEAREST_MIPMAP_LINEAR || \
   (flag) == GL_NEAREST_MIPMAP_NEAREST)

bool load_obj(
    const char* filename,
    const char* basedir,
    std::vector<tinyobj::real_t>& vertices_out,
    std::vector<tinyobj::real_t>& normals_out,
    std::vector<std::vector<size_t>>& vertex_map,
    std::vector<std::vector<size_t>>& material_map,
    tinyobj::attrib_t& attrib,
    std::vector<tinyobj::shape_t>& shapes,
    std::vector<tinyobj::material_t>& materials,
    tinyobj::real_t scale = 1.0f
);

struct obj {
    std::vector<tinyobj::real_t> vertices;
    std::vector<tinyobj::real_t> normals;
    std::vector<tinyobj::real_t> colors;
    std::vector<std::vector<size_t>> vertex_map;
    std::vector<std::vector<size_t>> material_map;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::vector<tinyobj::real_t> texcoords;
    std::map<std::string, size_t> texmap;
    bool is_obj_valid = false ;
    bool is_tex_valid = false ;
    tinyobj::attrib_t attrib;

    GLuint vao ;
    GLuint vbo[4] ;
    
    GLfloat theta_x=0.0f, theta_z=0.0f ;
    glm::mat4 trans_mat = glm::mat4(1.0f) ;
    GLfloat x, y, z, r ;
} ;

bool load_tex( const char* basedir, std::vector<tinyobj::real_t>& texcoords_out, std::map<std::string, size_t>& texmap_out, const std::vector<tinyobj::real_t>& texcoords, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials, GLint min_filter, GLint mag_filter) ;

static bool has_file(const char* filepath) ;
GLuint generate_tex(const char* tex_file_path, GLint min_filter, GLint mag_filter) ;
