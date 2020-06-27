#version 410

in vec4 fNormal;
in vec4 fPosition;
in vec2 fTexcoord;

uniform int ColorMode;  // color mode
uniform int ObjectCode; // object mode
uniform sampler2D sampler;  // sampler2D

uniform mat4 V;                                                     // For viewing transformation V

uniform vec3 Ia = vec3(0.2,1.0,0.2);                                                  // Ambient light
uniform vec3 Il = vec3(1.0);                                                 // Incident light intensity
uniform vec3 Ka;                                                    // Ambient light coefficient
uniform vec3 Ks;                                                    // Specular coefficient
uniform vec3 Kd;                                                    // Diffuse coefficient
uniform float c[3] = float[3](0.01, 0.001, 0.0);                                                     // Coefficient of attenuation factor fatt
uniform float n;                                                    // Specular intensity factor
uniform vec4 LightPos_wc = vec4(10, 10, 3, 1);                                           // Light position on world coordinate

out vec4 FragColor;

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
    if (ColorMode == 1) // for wireframes
        FragColor = vec4(0,0,1,1);
    if (ColorMode == 2) { // picking mode
        float nc = ObjectCode / 255.0;
        FragColor = vec4(nc, nc, nc, 1);
    }
    else {
        vec3 LightPos_ec = vec3(V * LightPos_wc);
        FragColor = shading(LightPos_ec, vec3(fPosition), vec3(fNormal));
        FragColor *= texture(sampler, fTexcoord);
    }
}

