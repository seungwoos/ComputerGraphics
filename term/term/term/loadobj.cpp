#include "loadobj.h"
#include "tiny_obj_loader.h"
#include <cmath>

using namespace std ;
using namespace tinyobj ;

// Compute a normal from three points.
void calc_normal(float* N, const float* v0, const float* v1, const float* v2)
{
    // Compute the cross product between two vectors:
    // v1 - v0 and v2 - v0.
    float v10[3];
    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    float v20[3];
    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v10[1] * v20[2] - v10[2] * v20[1];
    N[1] = v10[2] * v20[0] - v10[0] * v20[2];
    N[2] = v10[0] * v20[1] - v10[1] * v20[0];

    // Normalize the resulting vector.
    float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len2 > 0.0f) {
        float len = sqrtf(len2);
        N[0] /= len;
        N[1] /= len;
        N[2] /= len;
    }
}

// Load a Wavefront obj file.
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
    tinyobj::real_t scale)
{
    using namespace std;
    using namespace tinyobj;

    string err, warn; // fixed

    // Load the obj file.
    bool ret = LoadObj(&attrib, &shapes, &materials, &warn, // fixed
        &err, filename, basedir);

#ifdef _DEBUG
    if (!err.empty()) fprintf(stderr, "%s\n", err.c_str());
#endif
    if (!ret) return false;

    // Comptue the vertex and normal maps.
    // And resize the output vertex and normal lists.
    size_t num_of_out_vertices = 0;
    size_t num_of_shapes = shapes.size();
    vertex_map.resize(num_of_shapes);
    material_map.resize(num_of_shapes);
    for (size_t s = 0; s < num_of_shapes; ++s) {
        auto& mash = shapes[s].mesh;
        auto& faces = mash.num_face_vertices;
        
        vertex_map[s].clear();
        material_map[s].clear();
        if (faces.size() > 0)
        {
            size_t current_id = mash.material_ids[0];
            vertex_map[s].push_back(num_of_out_vertices);
            material_map[s].push_back(current_id);

            for (size_t f = 0; f < faces.size(); ++f)
            {
                size_t material_id = mash.material_ids[f];
                if (current_id != material_id) {
                    vertex_map[s].push_back(num_of_out_vertices);
                    material_map[s].push_back(current_id);
                    current_id = material_id;
                }
                num_of_out_vertices += faces[f];
            }
            vertex_map[s].push_back(num_of_out_vertices);
        }
    }
    vertices_out.resize(num_of_out_vertices * 3);
    normals_out.resize(num_of_out_vertices * 3);

    // Compute the scale parameter.
    size_t num_of_input_vertices = attrib.vertices.size() / 3;
    real_t vmin[3] = { INFINITY, INFINITY, INFINITY };
    real_t vmax[3] = { -INFINITY,-INFINITY,-INFINITY };
    for (size_t j = 0; j < num_of_input_vertices; ++j) {
        for (size_t i = 0; i < 3; ++i) {
            real_t val = attrib.vertices[j * 3 + i];
            if (vmin[i] > val) vmin[i] = val;
            if (vmax[i] < val) vmax[i] = val;
        }
    }
    real_t max_len = vmax[0] - vmin[0];
    if (max_len < vmax[1] - vmin[1]) max_len = vmax[1] - vmin[1];
    if (max_len < vmax[2] - vmin[2]) max_len = vmax[2] - vmin[2];
    real_t final_scale = scale / max_len;

    // Duplicate the source vertex and normal data
    // so that no vertices and normals are shared among faces.
    real_t n[3][3];
    real_t v[3][3];
    real_t* vertex_dst_ptr = vertices_out.data();
    real_t* normal_dst_ptr = normals_out.data();
    const real_t* vertex_src_ptr = attrib.vertices.data();
    const real_t* normal_src_ptr = (attrib.normals.size() ? attrib.normals.data() : NULL);

    for (size_t s = 0; s < num_of_shapes; ++s)
    {
        mesh_t& mesh = shapes[s].mesh;
        size_t num_of_faces = mesh.num_face_vertices.size();
        for (size_t f = 0; f < num_of_faces; ++f)
        {
            // Get indices to the three vertices of a triangle.
            index_t idx[3] = {
                mesh.indices[3 * f + 0],
                mesh.indices[3 * f + 1],
                mesh.indices[3 * f + 2]
            };

            // Get the three vertex positions of the current face.
            for (int k = 0; k < 3; ++k) {
                memcpy(v[k], vertex_src_ptr +
                    idx[k].vertex_index * 3, sizeof(real_t) * 3);
                for (int i = 0; i < 3; ++i)
                    v[k][i] *= final_scale;
            }

            // Get the normal vectors of the current face.
            if (normal_src_ptr == NULL) {
                calc_normal(n[0], v[0], v[1], v[2]);
                memcpy(n[1], n[0], sizeof(real_t) * 3);
                memcpy(n[2], n[0], sizeof(real_t) * 3);
            }
            else {
                memcpy(n[0], normal_src_ptr + idx[0].normal_index * 3, sizeof(real_t) * 3);
                memcpy(n[1], normal_src_ptr + idx[1].normal_index * 3, sizeof(real_t) * 3);
                memcpy(n[2], normal_src_ptr + idx[2].normal_index * 3, sizeof(real_t) * 3);
            }

            memcpy(vertex_dst_ptr, v, sizeof(real_t) * 9);
            memcpy(normal_dst_ptr, n, sizeof(real_t) * 9);
            vertex_dst_ptr += 9;
            normal_dst_ptr += 9;
        }
    }

    return true;
}

static bool has_file(const char* filepath) {
    FILE *fp;
    fp = fopen(filepath, "rb") ;
    if (fp!=NULL) {
        fclose(fp);
        return true;
     }
     return false;
}

bool load_tex(
              const char* basedir, // base directory where texture files are located
              vector<real_t>& texcoords_out, // output texture coord
              map<string, size_t>& texmap_out, // output map from texture filef name to its texture id

              const vector<real_t>& texcoords, // input texture coord
              const vector<shape_t>& shapes, // list of shapes
              const vector<material_t>& materials, // list of materials
              
              GLint min_filter = GL_LINEAR_MIPMAP_LINEAR,
              GLint mag_filter = GL_LINEAR_MIPMAP_LINEAR ) {
    
    /* 1. compute valid texture coordinates for each vectex */
    size_t total_num_of_vectices = 0 ;
    size_t num_of_shapes = shapes.size() ;
    
    for (size_t s=0 ; s<num_of_shapes ; ++s) {
        total_num_of_vectices += shapes[s].mesh.indices.size() ;
    }
    // textcoords_out -> s0 t0 s1 t1 s2 t2 s3 t3 ...
    texcoords_out.resize(total_num_of_vectices*2) ; // each vertex has s, t
    
    real_t* texcoords_dst_ptr = texcoords_out.data();
    const real_t* texcoords_src_ptr = texcoords.size() > 0 ? texcoords.data() : NULL;
    
    for (size_t s = 0; s < num_of_shapes; ++s) {
        const mesh_t& mesh = shapes[s].mesh;
        size_t num_of_faces = mesh.indices.size() / 3;
        
        for (size_t f = 0; f < num_of_faces; ++f) {
            // Get indices to the three vertices of a triangle.
            int idx[3] = {
                mesh.indices[3 * f + 0].texcoord_index,
                mesh.indices[3 * f + 1].texcoord_index,
                mesh.indices[3 * f + 2].texcoord_index
            
            };
            // Compute and copy valid texture coordinates.
            real_t tc[3][2]; // texture coord
            if (texcoords_src_ptr != NULL) {
                if (idx[0] < 0 || idx[1] < 0 || idx[2] < 0) {
                    fprintf(stderr, "Invalid texture coordinate index\n");
                    return false;
                }
                for (size_t i = 0; i < 3; ++i) {  // fill 3 vertex of triangle
                    memcpy(tc[i], texcoords_src_ptr + idx[i] * 2, sizeof(real_t) * 2);
                    tc[i][1] = 1.0f - tc[i][1];// flip the t coordinate.
                }
            } else {
                tc[0][0] = tc[0][1] = 0;
                tc[1][0] = tc[1][1] = 0;
                tc[2][0] = tc[2][1] = 0;
            }
            memcpy(texcoords_dst_ptr, tc, sizeof(real_t) * 6);
            texcoords_dst_ptr += 6;
        }
    }
    
    /* 2. make a texture object for each material */
    GLuint texture_id;
    size_t num_of_materials = materials.size();
    
    for (size_t m = 0; m < num_of_materials; ++m) {
        const material_t& mat = materials[m];
        const string& texname = mat.diffuse_texname;
        
        if (texname.empty()) continue;
        if (MAP_FIND(texmap_out, texname)) continue; // does not match with any item
            
        // Open the texture image file.
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
        texmap_out[texname] = texture_id;
    }
    return true ;
}


GLuint generate_tex(
                    const char* tex_file_path,
                    GLint min_filter,
                    GLint mag_filter) {
    int width, height, num_of_components;
    unsigned char* image = stbi_load(
                                     tex_file_path, &width, &height,
                                     &num_of_components, STBI_default) ;
    if (!image) {
        fprintf(stderr, "Failed to open %s\n", tex_file_path);
        return false;
    }
    
    // Generate a texture object and set its parameters.
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    
    bool is_supported = true;
    switch (num_of_components) {
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
