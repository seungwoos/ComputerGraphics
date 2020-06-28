#version 410

out vec4 fNormal;
out vec4 fPosition;

in vec4 vPosition;                                                  // vertex position vector
in vec4 vNormal;                                                    // vertex normal vector --> determining shader color

uniform mat4 M;                                                     // For modeling transformation M
uniform mat4 P;                                                     // For perspective transformation P
uniform mat4 V;                                                     // For viewing transformation V
//uniform mat4 VM = V * M;                                                    // VM = V * M
//uniform mat4 U = transpose(inverse(VM));
uniform mat4 VM;
uniform mat4 U;

uniform vec3 LightPos_ec;                                           // Light position on eye coordinate

void main(){
    VM = V * M;
    U = transpose(inverse(VM));
    
    vec3 vNormal_ec = vec3(normalize(U*vNormal));                   // Normalize with repect to eye coordinate system
    vec3 vPosition_ec = vec3(VM * vPosition);                      // Convert vPosition to eye coordinate system.
    gl_Position = P * vec4(vPosition_ec, 1);
    
    fNormal = vec4(vNormal_ec, 1);
    fPosition = vec4(vPosition_ec, 1);
}
