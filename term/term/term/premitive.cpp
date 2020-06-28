//
//  premitive.cpp
//  HW10
//
//  Created by HyeWon Jeon on 2020/05/08.
//  Copyright © 2020 HyeWon Jeon. All rights reserved.
//

#include "premitive.hpp"

/* define macro */
#define FPUSH_VTX3(p, vx, vy, vz) \
do { \
    p.push_back(vx) ; \
    p.push_back(vy) ; \
    p.push_back(vz) ; \
} while(0)

#define FPUSH_VTX3_AT(p, i, vx, vy, vz) \
do { \
    GLuint i3 = 3*(i) ; \
    p[i3+0] = (float)(vx) ; \
    p[i3+1] = (float)(vy) ; \
    p[i3+2] = (float)(vz) ; \
} while(0)

#define FSET_VTX3(vx, vy, vz, valx, valy, valz) \
do { \
    vx = (float)(valx) ; \
    vy = (float)(valy) ; \
    vz = (float)(valz) ; \
} while(0)


void get_box_3d(GLvec& p, GLvec& normals, GLfloat lx, GLfloat ly, GLfloat lz) {
    static const GLfloat box_vertices[] = {
        0.5f, 0.5f, -0.5f,      -0.5f, -0.5f, -0.5f,    -0.5f, 0.5f, -0.5f,     // side at z=-0.5
        0.5f, 0.5f, -0.5f,      0.5f, -0.5f, -0.5f,     -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,    -0.5f, -0.5f, 0.5f,     -0.5f, 0.5f, 0.5f,      // side at x=-0.5
        -0.5f, -0.5f, -0.5f,    -0.5f, 0.5f, 0.5f,     -0.5f, 0.5f, -0.5f,
        0.5f, -0.5f, 0.5f,      -0.5f, -0.5f, -0.5f,    0.5f, -0.5f, -0.5f,     // side at y=-0.5
        0.5f, -0.5f, 0.5f,      -0.5f, -0.5f, 0.5f,     -0.5f, -0.5f, -0.5f,

        -0.5f, 0.5f, 0.5f,      -0.5f, -0.5f, 0.5f,     0.5f, -0.5f, 0.5f,      // side at z=0.5
        0.5f, 0.5f, 0.5f,       -0.5f, 0.5f, 0.5f,      0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,       0.5f, -0.5f, -0.5f,     0.5f, 0.5f, -0.5f,      // side at x=0.5
        0.5f, -0.5f, -0.5f,     0.5f, 0.5f, 0.5f,       0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,       0.5f, 0.5f, -0.5f,      -0.5f, 0.5f, -0.5f,     // side at y=0.5
        0.5f, 0.5f, 0.5f,       -0.5f, 0.5f, -0.5f,     -0.5f, 0.5f, 0.5f,

    } ;

    p.resize(sizeof(box_vertices) / sizeof(GLfloat)) ;
    memcpy(p.data(), box_vertices, sizeof(box_vertices)) ;

    GLuint n = p.size() / 3 ;
    for (int i=0 ; i<n ; ++i) {
        p[3*i+0] *= lx ;
        p[3*i+1] *= ly ;
        p[3*i+2] *= lz ;
    }
    
    // compute normals for each vertex (box_vertices)
    normals.resize(n*3) ;
    float* cursor = normals.data() ;
    
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for z = -0.5
        cursor[0] = 0 ;     cursor[1] = 0 ;     cursor[2] = -1 ;
    }
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for x = -0.5
        cursor[0] = -1 ;     cursor[1] = 0 ;     cursor[2] = 0 ;
    }
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for y = -0.5
        cursor[0] = 0 ;     cursor[1] = -1 ;     cursor[2] = 0 ;
    }
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for z = 0.5
        cursor[0] = 0 ;     cursor[1] = 0 ;     cursor[2] = 1 ;
    }
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for x = 0.5
        cursor[0] = 1 ;     cursor[1] = 0 ;     cursor[2] = 0 ;
    }
    for (int i=0 ; i<6 ; i++, cursor+=3) { // for y = 0.5
        cursor[0] = 0 ;     cursor[1] = 1 ;     cursor[2] = 0 ;
    }
}

void get_sphere_3d(GLvec& p, GLvec& normals, GLfloat r, GLint subh, GLint suba) {
    for (int i=1 ; i<=subh ; ++i) {
        double theta0 = M_PI * (i-1) / subh ;
        double theta1 = M_PI * i / subh ;
        
        double y0   = r * cos(theta0) ;
        double rst0 = r * sin(theta0) ;
        double y1   = r * cos(theta1) ;
        double rst1 = r * sin(theta1) ;
        
        for (int j=1 ; j<=suba ; ++j) {
            double phi0 = 2 * M_PI * (j-1) / suba ;
            double phi1 = 2 * M_PI * j / suba ;
            
            double cp0 = cos(phi0) ;
            double sp0 = sin(phi0) ;
            double cp1 = cos(phi1) ;
            double sp1 = sin(phi1) ;
            
            float vx0, vy0, vz0, vx1, vy1, vz1 ;
            float vx2, vy2, vz2, vx3, vy3, vz3 ;
            
            FSET_VTX3(vx0, vy0, vz0, sp0*rst0, y0, cp0*rst0) ;
            FSET_VTX3(vx1, vy1, vz1, sp0*rst1, y1, cp0*rst1) ;
            FSET_VTX3(vx2, vy2, vz2, sp1*rst0, y0, cp1*rst0) ;
            FSET_VTX3(vx3, vy3, vz3, sp1*rst1, y1, cp1*rst1) ;
            
            if (i<subh) {
                // first triangle (v0 - v1 - v3)
                FPUSH_VTX3(p, vx0, vy0, vz0) ;
                FPUSH_VTX3(p, vx1, vy1, vz1) ;
                FPUSH_VTX3(p, vx3, vy3, vz3) ;
                
                FPUSH_VTX3(normals, vx0/r, vy0/r, vz0/r) ;
                FPUSH_VTX3(normals, vx1/r, vy1/r, vz1/r) ;
                FPUSH_VTX3(normals, vx3/r, vy3/r, vz3/r) ;
            }

            if (1 < i) {
                // second triangle (v3 - v2 - v0)
                FPUSH_VTX3(p, vx3, vy3, vz3) ;
                FPUSH_VTX3(p, vx2, vy2, vz2) ;
                FPUSH_VTX3(p, vx0, vy0, vz0) ;
                
                FPUSH_VTX3(normals, vx3/r, vy3/r, vz3/r) ;
                FPUSH_VTX3(normals, vx2/r, vy2/r, vz2/r) ;
                FPUSH_VTX3(normals, vx0/r, vy0/r, vz0/r) ;
            }
        }
    }
}

void get_cone_3d(GLvec& p, GLvec& normals, vector<GLuint>& side_idx, vector<GLuint>& bottom_idx, GLfloat radius, GLfloat height, GLint n) {
    // p save all point of cone
    GLfloat half_height = height / 2 ;
    GLfloat theta, x, z ;
    
    FPUSH_VTX3(p, 0, half_height, 0) ; // top vertex
    FPUSH_VTX3(normals, 0, 1, 0) ;
    side_idx.push_back(0) ;
    bottom_idx.push_back(2*n+3) ;
    for (int i=0 ; i<=n ; ++i) {
        theta = (GLfloat)(2.0 * M_PI * i / n) ;
        x = radius * sin(theta) ;
        z = radius * cos(theta) ;
        FPUSH_VTX3(p, x, -half_height, z) ;
        FPUSH_VTX3(normals, x/radius, 0, z/radius) ;
        side_idx.push_back(i+1) ; // save 0 to n+1
        bottom_idx.push_back(2*n+2-i) ; // save n+3 to 2n+3
    }
    FPUSH_VTX3(p, 0, -half_height, 0) ; // bottom center vertex
    FPUSH_VTX3(normals, 0, -1, 0) ;
    
    // add duplicated vertex to modeling normal vector
    for (int i=0 ; i<=n ; ++i) {
        theta = (GLfloat)(2.0 * M_PI * i / n) ;
        x = radius * sin(theta) ;
        z = radius * cos(theta) ;
        FPUSH_VTX3(p, x, -half_height, z) ;
        FPUSH_VTX3(normals, 0, -1, 0) ;
    }
}

void get_cylinder_3d(GLvec& p, GLvec& normals, vector<GLuint>& side_idx, vector<GLuint>& top_idx, vector<GLuint>& bottom_idx, GLfloat radius, GLfloat height, GLint n) {
    p.resize(3*(4*n+6)) ;
    normals.resize(3*(4*n+56)) ;
    
    GLfloat half_height = height / 2 ;
    GLfloat theta, x, z ;
    
    FPUSH_VTX3_AT(p, 0, 0, half_height, 0) ;
    FPUSH_VTX3_AT(normals, 0, 0, 1, 0) ;
    FPUSH_VTX3_AT(p, 3*n+4, 0, -half_height, 0) ;
    FPUSH_VTX3_AT(normals, 3*n+4, 0, -1, 0) ;
    top_idx.push_back(0) ;
    bottom_idx.push_back(3*n+4) ;
    for (int i=0 ; i<=n ; ++i) {
        theta = (GLfloat)(2.0 * M_PI * i / n) ;
        x = radius * sin(theta) ;
        z = radius * cos(theta) ;
        // top
        FPUSH_VTX3_AT(p, i+1, x, half_height, z) ;
        FPUSH_VTX3_AT(normals, i+1, 0, 1, 0) ;
        // side
        FPUSH_VTX3_AT(p, 2*i+n+2, x, half_height, z) ;
        FPUSH_VTX3_AT(p, 2*i+n+3, x, -half_height, z) ;
            // i changed normal vector's y 1 -> 0 (ppt -> 1/r)
        FPUSH_VTX3_AT(normals, 2*i+n+2, x/radius, 0, z/radius) ;
        FPUSH_VTX3_AT(normals, 2*i+n+3, x/radius, 0, z/radius) ;
        // bottom
        FPUSH_VTX3_AT(p, 4*n+5 -i, x, -half_height, z) ;
        FPUSH_VTX3_AT(normals, 4*n+5 -i, 0, -1, 0) ;
        top_idx.push_back(i+1) ;
        side_idx.push_back(2*i+n+2) ;
        side_idx.push_back(2*i+n+3) ;
        bottom_idx.push_back(i+3*n+5) ;
    }
    // side -> n+2, n+3, ... 3n+3
    // top -> 0, 1, 2, ... n+1
    // bottom -> 3n+4, 4n+5, ... 4n+5
}

void get_torus_3d(GLvec& p, GLvec& normals, vector<vector<GLuint>>& side_idx, GLfloat r0, GLfloat r1, GLint na, GLint nh) {
    GLfloat x_base, z_base, theta1, theta2, l ;
    GLuint ak, bk ;
    side_idx.resize(nh) ;
    
    for (int i=0 ; i<=na ; i++) {
        theta1 = (GLfloat)(2.0 * M_PI * i / na) ;
        x_base = (r0+r1) * sin(theta1) ;
        z_base = (r0+r1) * cos(theta1) ;
        
        for (int j=0 ; j<=nh ; j++) {
            theta2 = (GLfloat)(2.0 * M_PI * j / nh) ;
            l = r1 * cos(theta2) ;
            FPUSH_VTX3(p, x_base+l*sin(theta1), r1*sin(theta2), z_base+l*cos(theta1)) ;
            FPUSH_VTX3(normals, (l*sin(theta1))/r1, sin(theta2), (l*cos(theta1))/r1 ) ;
        }
    }
    
    for (int i=0 ; i<nh ; i++) {
        ak = i * (na+1) ;
        bk = (i+1) * (na+1) ;
        for (int j=0 ; j<=na ; j++) {
            side_idx[i].push_back(ak+j) ;
            side_idx[i].push_back(bk+j) ;
        }
    }
}

void get_grid(GLvec& p, GLfloat w, GLfloat h, int m, int n) {
    GLfloat x0 = -0.5f * w ;
    GLfloat x1 = +0.5f * w ;
    GLfloat z0 = -0.5f * h ;
    GLfloat z1 = +0.5f * h ;
    
    for (int i=0 ; i<=m ; ++i) {
        GLfloat x = x0 + w*i / m ;
        FPUSH_VTX3(p, x, 0, z0) ;
        FPUSH_VTX3(p, x, 0, z1) ;
    }
    for (int i=0 ; i<=n ; ++i) {
        GLfloat z = z0 + h*i / n ;
        FPUSH_VTX3(p, x0, 0, z) ;
        FPUSH_VTX3(p, x1, 0, z) ;
    }
}

void get_color_3d_by_pos(GLvec& c, GLvec& p, GLfloat offset) {
    GLfloat max_val[3] = {-INFINITY, -INFINITY, -INFINITY} ;
    GLfloat min_val[3] = {INFINITY, INFINITY, INFINITY} ;
    
    int n = (int)(p.size() / 3) ;
    for (int i=0 ; i<n ; ++i) {
        for (int j=0 ; j<3 ; ++j) {
            GLfloat val = p[i*3+j] ;
            if (max_val[j] < val)
                max_val[j] = val ;
            else if (min_val[j] > val)
                min_val[j] = val ;
        }
    }
    
    GLfloat width[3] = {
        max_val[0] - min_val[0],
        max_val[1] - min_val[1],
        max_val[2] - min_val[2]
    } ;
    
    c.resize(p.size()) ;
    for (int i=0 ; i<n ; ++i) {
        for (int j=0 ; j<3 ; ++j) {
            int k = i*3+j ;
            c[k] = fminf((p[k]-min_val[j]) / width[j] + offset, 1.0f) ;
        }
    }
}

void get_color_box_by_pos(GLvec& c, GLvec& p, GLfloat offset) {
    c.resize(p.size()) ;
    int n = (int)(p.size() / 3) ;

    for (int i=0 ; i<n ; ++i) {
        c[i*3+0] = 0.58 ;
        c[i*3+1] = 0.29 ;
        c[i*3+2] = 0.0 ;
    }
}
