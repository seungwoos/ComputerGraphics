//
//  util.cpp
//  term
//
//  Created by HyeWon Jeon on 2020/06/08.
//  Copyright © 2020 HyeWon Jeon. All rights reserved.
//

#include "util.hpp"

#ifdef dSINGLE
#define dEpsilon FLT_EPSILON
#else
#define dEpsilon DBL_EPSILON
#endif


glm::mat4 parallel(double r, double aspect, double n, double f) {
    double l = -r ;
    double width = 2*r ;
    double height = width / aspect ;
    double t = height/2 ;
    double b = -t ;
    return ortho(l, r, b, t, n, f) ;
}

void bind_buffer(GLint buffer, GLvec& vec, int program, const GLchar* attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vec.size(), vec.data(), GL_STATIC_DRAW);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

void bind_buffer(GLint buffer, int program, const GLchar* attri_name, GLint attri_size) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLuint location = glGetAttribLocation(program, attri_name);
    glVertexAttribPointer(location, attri_size, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
}

double dsElapsedTime() {
    static double prev = 0.0;
    double curr = clock() / 10000000.0;
    if (!prev)
            prev = curr;
    double retval = curr - prev;
    prev = curr;
    if (retval > 1.0) retval = 1.0;
    if (retval < dEpsilon) retval = dEpsilon;
    return retval;
}

mat4 compute_modelling_transf(dBodyID body) {
    // give rigid body에 대해서 적절한 transformation matrix 만들어서 리턴해주기
    mat4 M(1.0f);

    const dReal* pos = dBodyGetPosition(body);
    const dReal* rot = dBodyGetRotation(body);

    M[3] = vec4(pos[0], pos[1], pos[2], 1.0f);

    for (int i = 0; i < 3; ++i) // for each column
        for(int j=0;j<3;++j) //for each row
            M[i][j] = rot[j * 4 + i];

    return M;
}

mat4 compute_geom_transf(dGeomID geom) {
    // give rigid body에 대해서 적절한 transformation matrix 만들어서 리턴해주기
    mat4 M(1.0f);
    
    const dReal* pos = dGeomGetPosition(geom);
    const dReal* rot = dGeomGetRotation(geom);
    
    M[3] = vec4(pos[0], pos[1], pos[2], 1.0f);
    
    for (int i = 0; i < 3; ++i) // for each column
        for(int j=0;j<3;++j) //for each row
            M[i][j] = rot[j * 4 + i];
    
    return M;
}
