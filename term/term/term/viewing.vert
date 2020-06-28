#version 410

in vec4 vColor;
in vec4 vPosition;
in vec4 vNormal;
in vec2 vTexcoord ;

out vec4 fNormal;
out vec4 fPosition;
out vec4 fColorFront ;
out vec4 fColorBack ;
out vec2 fTexcoord ;

uniform mat4 T;    // Additional transformation performed before modeling transformation
uniform int mode;

uniform vec3 LightPos_ec ;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 Ka ;
uniform vec3 Kd ;
uniform vec3 Ks ;
uniform float n ;

uniform vec3 Il = vec3(1.0);
uniform vec3 Ia ;
uniform vec3 c = vec3(0.01, 0.001, 0.0);

vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec) // represented in eye coord sys
{
    vec3 N = normalize(vNormal_ec);
    vec3 L = LightPos_ec - vPosition_ec;
    float d = length(L); L = L/d; // make L unit vector
    vec3 V = normalize(vec3(0.0) - vPosition_ec); // normalized vector from sample point to eye
    vec3 R = reflect(-L, N);

    float fatt = min(1.0 / (c[0] + c[1]*d + c[2]*d*d), 1.0); // attenuation

    float cos_theta = max(dot(N,L),0);
    float cos_alpha = max(dot(V,R),0);

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) ;

    return vec4(I,1);
}

void default_shading()
{
    gl_Position = P*V*M*T * vPosition;
    fColorFront = vColor;
    fColorBack = vColor;
}

void gouraud_shading()
{
    mat4 VMT = V*M*T;
    mat4 U = transpose(inverse(VMT));
    vec3 vNormal_ec = vec3(normalize(U*vNormal));
    vec3 vPosition_ec = vec3(VMT * vPosition);

    gl_Position = P * vec4(vPosition_ec, 1);

    fColorFront = shading(LightPos_ec, vPosition_ec, vNormal_ec);
    fColorBack = shading(LightPos_ec, vPosition_ec, -vNormal_ec);
}

void phong_shading()
{
    mat4 VMT = V*M*T;
    mat4 U = transpose(inverse(VMT));
    vec3 vNormal_ec = vec3(normalize(U*vNormal));
    vec3 vPosition_ec = vec3(VMT * vPosition);

    gl_Position = P * vec4(vPosition_ec, 1);
    fNormal = vec4(vNormal_ec, 1);
    fPosition = vec4(vPosition_ec, 1);

}

void main()
{
    switch(mode) {
        case 1:
            gouraud_shading();
            break;
        case 2:
            phong_shading();
            break;
        default:
            default_shading();
            break;
    }
    fTexcoord = vTexcoord ;
}
