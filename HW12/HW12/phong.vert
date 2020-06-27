#version 410

out vec4 fPosition;
out vec4 fNormal;
out vec2 fTexcoord;

in vec4 vPosition_mc;                                                  // vertex position vector
in vec4 vNormal_mc;                                                    // vertex normal vector --> determining shader color
in vec4 vColor;
in vec2 vTexcoord;                                                  // texture coordinates

uniform mat4 M;                                                     // For modeling transformation M
uniform mat4 P;                                                     // For perspective transformation P
uniform mat4 V;                                                     // For viewing transformation V
uniform mat4 U;                                                     // Transformation matrix for normals
uniform mat4 VM;                                            // VM = V * M

uniform vec4 LightPos_wc = vec4(10, 10, 3, 1);                                           // Light position on world coordinate

void main(){
    mat4 VM = V * M;
    mat4 U = transpose(inverse(VM));
    
    vec3 vNormal_ec = vec3(normalize(U * vNormal_mc));                   // Normalize with repect to eye coordinate system
    vec3 vPosition_ec = vec3(VM * vPosition_mc);                      // Convert vPosition to eye coordinate system.
    vec3 LightPos_ec = vec3(V * LightPos_wc);
    
    gl_Position = P * vec4(vPosition_ec, 1);
    
    fNormal = vec4(vNormal_ec, 1);
    fPosition = vec4(vPosition_ec, 1);
    
    fTexcoord = vTexcoord;
}
