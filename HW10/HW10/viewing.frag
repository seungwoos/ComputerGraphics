#version 410

out vec4 FragColor;
in vec4 fColor;
in vec4 fPosition;
in vec4 fNormal;

uniform mat4 T;
uniform int mode;
uniform vec4 uColor;

uniform mat4 M;
uniform mat4 P;
uniform mat4 V;
uniform mat4 U;

uniform vec3 Ia = vec3(0.2,1.0,0.2);
uniform vec3 Il = vec3(1.0);
uniform float Ka = 0.3;
uniform float Ks = 0.5;
uniform float Kd = 0.8;
uniform float c[3] = float[3](0.01, 0.001, 0.0);
uniform float n = 10.0;
uniform vec4 LightPos_wc = vec4(10, 10, 3, 1);

vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec)
{
    vec3 N = normalize(vNormal_ec);
    vec3 L = LightPos_ec - vPosition_ec;
    float d = length(L);
    L = L/d;
    vec3 V = normalize(vec3(0.0) - vPosition_ec);
    vec3 R = reflect(-L, N);

    float fatt = min(1.0 / (c[0] + c[1]*d + c[2]*d*d), 1.0);

    float cos_theta = max(dot(N,L),0);
    float cos_alpha = max(dot(V,R),0);

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) /**vec3(fColor)*/;
    return vec4(I,1);
}

void default_shading()
{
    FragColor = fColor;
}

void gouraud_shading()
{
    FragColor = fColor;
}

void phong_shading()
{
//    mat4 VMT = V*M*T;
//    mat4 U = transpose(inverse(VMT));
//    vec3 fNormal_ec = vec3(normalize(U*fNormal));
//    vec3 fPosition_ec = vec3(VMT * fPosition);
    vec3 LightPos_ec = vec3(V * LightPos_wc);

    FragColor = shading(LightPos_ec, vec3(fPosition), vec3(fNormal));
}

void main(){
    
    switch(mode){
       case 0:
           default_shading();
           break;
       case 1:
           gouraud_shading();
           break;
       case 2:
           phong_shading();
           break;
       case 3:
           FragColor = uColor;
           break;
       }
       
}

