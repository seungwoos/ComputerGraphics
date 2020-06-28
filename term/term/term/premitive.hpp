//
//  premitive.hpp
//  HW10
//
//  Created by HyeWon Jeon on 2020/05/08.
//  Copyright © 2020 HyeWon Jeon. All rights reserved.
//

#ifndef premitive_hpp
#define premitive_hpp

#include <stdio.h>
#include <math.h>
#include <vector>
#include <GL/glew.h> // in main.cpp
#include <glm/gtc/matrix_transform.hpp>

using namespace std ;
typedef vector<GLfloat> GLvec ;


void get_box_3d(GLvec& p, GLvec& normals, GLfloat lx, GLfloat ly, GLfloat lz) ;
void get_sphere_3d(GLvec& p, GLvec& normals, GLfloat r, GLint subh, GLint suba) ;
void get_cone_3d(GLvec& p, GLvec& normals, vector<GLuint>& side_idx, vector<GLuint>& bottom_idx, GLfloat radius, GLfloat height, GLint n) ;
void get_cylinder_3d(GLvec& p, GLvec& normals, vector<GLuint>& side_idx, vector<GLuint>& top_idx, vector<GLuint>& bottom_idx, GLfloat radius, GLfloat height, GLint n) ;
void get_torus_3d(GLvec& p, GLvec& normals, vector<vector<GLuint>>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh) ;
void get_grid(GLvec& p, GLfloat w, GLfloat h, int m, int n) ;

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset) ;
void get_color_box_by_pos(GLvec& c, GLvec& p, GLfloat offset) ;

struct box_obj {
    GLvec vertices, colors, normals ;
    GLuint vao, vbo[3] ;
    glm::mat4 trans_mat ;
    
} ;

#endif /* premitive_hpp */
