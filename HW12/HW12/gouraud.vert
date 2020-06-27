#version 410

out vec4 fColorFront;
out vec4 fColorBack;
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

uniform vec3 Ia = vec3(0.2,1.0,0.2);                                                  // Ambient light
uniform vec3 Il = vec3(1.0);                                                 // Incident light intensity
uniform vec3 Ka;                                                    // Ambient light coefficient
uniform vec3 Ks;                                                    // Specular coefficient
uniform vec3 Kd;                                                    // Diffuse coefficient
uniform float c[3] = float[3](0.01, 0.001, 0.0);                                                     // Coefficient of attenuation factor fatt
uniform float n;                                                    // Specular intensity factor
uniform vec4 LightPos_wc = vec4(10, 10, 3, 1);                                           // Light position on world coordinate

vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec)
{
    vec3 N = normalize(vNormal_ec);                                 // Normal vector
    vec3 L = LightPos_ec - vPosition_ec;                            // Light vector
    float d = length(L); L = L/d;                                   // d = length of light vector
    vec3 V = normalize(vec3(0.0) - vPosition_ec);                   // View vector
    vec3 R = reflect(-L, N);                                        // Reflection vector

    float fatt = min(1.0 / (c[0] + c[1]*d + c[2]*d*d), 1.0);        // Attenuation factor

    float cos_theta = max(dot(N,L),0);                              // Angle between vector N and L, minimum value = 0
    float cos_alpha = max(dot(V,R),0);                              // Angle between vector V and R, minimum value = 0

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n))/* * vec3(vColor)*/;

    return vec4(I,1);
}

void main(){
    mat4 VM = V * M;
    mat4 U = transpose(inverse(VM));
    vec3 vNormal_ec = vec3(normalize(U * vNormal_mc));                   // Normalize with repect to eye coordinate system
    vec3 vPosition_ec = vec3(VM * vPosition_mc);                      // Convert vPosition to eye coordinate system.
    vec3 LightPos_ec = vec3(V * LightPos_wc);
    
    gl_Position = P * vec4(vPosition_ec, 1);

    fColorFront = shading(LightPos_ec, vPosition_ec, vNormal_ec);
    fColorBack = shading(LightPos_ec, vPosition_ec, -vNormal_ec);
    
    fTexcoord = vTexcoord;
}
