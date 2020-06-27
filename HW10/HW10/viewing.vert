#version 410

in vec4 vPosition;                                                  // vertex position vector
in vec4 vNormal;                                                    // vertex normal vector --> determining shader color
in vec4 vColor;                                                     // vertex color vector

out vec4 fColor;
out vec4 fNormal;
out vec4 fPosition;

uniform mat4 T;                                                     // Additional transformation performed before modeling transformation
uniform int mode;                                                   // Shading mode
uniform mat4 M;                                                     // For modeling transformation M
uniform mat4 P;                                                     // For perspective transformation P
uniform mat4 V;                                                     // For viewing transformation V
uniform mat4 U;

uniform vec3 Ia = vec3(0.2,1.0,0.2);                                // Ambient light
uniform vec3 Il = vec3(1.0);                                        // Incident light intensity
uniform float Ka = 0.3;                                             // Ambient light coefficient
uniform float Ks = 0.5;                                             // Specular coefficient
uniform float Kd = 0.8;                                             // Diffuse coefficient
uniform float c[3] = float[3](0.01, 0.001, 0.0);                    // Coefficient of attenuation factor fatt
uniform float n = 10.0;                                             //
uniform vec4 LightPos_wc = vec4(10, 10, 3, 1);                      // Light position on world coordinate

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

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n))/**vec3(vColor)*/;

    return vec4(I,1);
}

void default_shading()
{
    gl_Position = P*V*M*T * vPosition;
    fColor = vColor;
}

void gouraud_shading()
{
    mat4 VMT = V*M*T;
    mat4 U = transpose(inverse(VMT));                               // Transpose the inverse of transformation matrices (n' = (M^-1)^T dot n)
    vec3 vNormal_ec = vec3(normalize(U*vNormal));                   // Normalize with repect to eye coordinate system
    vec3 vPosition_ec = vec3(VMT * vPosition);                      // Convert vPosition to eye coordinate system.
    vec3 LightPos_ec = vec3(V * LightPos_wc);                       // Convert world coordinate system of light position into eye coordinate

    gl_Position = P * vec4(vPosition_ec, 1);
    fColor = shading(LightPos_ec, vPosition_ec, vNormal_ec);
}

void phong_shading()
{
    mat4 VMT = V*M*T;
    mat4 U = transpose(inverse(VMT));
    vec3 vNormal_ec = vec3(normalize(U*vNormal));
    vec3 vPosition_ec = vec3(VMT * vPosition);
    vec3 LightPos_ec = vec3(V * LightPos_wc);
    
    gl_Position = P * vec4(vPosition_ec, 1);
    fNormal = vec4(vNormal_ec, 1);
    fPosition = vec4(vPosition_ec, 1);
}


void main(){
    
    switch(mode)
    {
        case 0:    // use vertex color data
        case 3:    // use constant user color uColor
            default_shading();
            break;
        case 1:
            gouraud_shading();
            break;
        case 2:
            phong_shading();
            break;
    }
}
