#version 410

// for Picking object mode
uniform int ObjectCode;
uniform int ColorMode;  // color mode

// for Gouraud
out vec4 FragColor;
in vec4 fColorFront;
in vec4 fColorBack;
in vec4 fPosition;

// for phong
in vec4 fNormal;

// for texture
uniform sampler2D sampler ;
uniform int noSample ;
in vec2 fTexcoord ;

uniform mat4 T;
uniform int mode;
uniform vec4 uColor;

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


vec4 shading(vec3 LightPos_ec, vec3 vPosition_ec, vec3 vNormal_ec)
{
    vec3 N = normalize(vNormal_ec);
    vec3 L = LightPos_ec - vPosition_ec;
    float d = length(L); L = L/d;
    vec3 V = normalize(vec3(0.0) - vPosition_ec);
    vec3 R = reflect(-L, N);

    float fatt = min(1.0 / (c[0] + c[1]*d + c[2]*d*d), 1.0);

    float cos_theta = max(dot(N,L),0);
    float cos_alpha = max(dot(V,R),0);

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) /**vec3(fColor)*/;

    return vec4(I,1);
}

void gouraud_shading()
{
    if (gl_FrontFacing)
        FragColor = fColorFront ;
    else
        FragColor = fColorBack ;
}

void phong_shading()
{
//    vec3 LightPos_ec = vec3(V * LightPos_wc);

    if (gl_FrontFacing)
        FragColor = shading(LightPos_ec, vec3(fPosition), vec3(fNormal));
    else
        FragColor = shading(LightPos_ec, vec3(fPosition), vec3(-fNormal));
}

void main()
{
    if (ColorMode == 2) {
        float nc = ObjectCode / 255.0;
        FragColor = vec4(nc, nc, nc, 1);

    } else {
        switch(mode) {
            case 1:
            case 3:
                gouraud_shading();
                break;
            case 2:
                phong_shading();
                break;
        }
        if (noSample == 0)
            FragColor *= texture(sampler, fTexcoord) ;
    }
}
