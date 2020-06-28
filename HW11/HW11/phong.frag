#version 410

in vec4 fNormal;
in vec4 fPosition;
uniform int ColorMode;
uniform vec3 LightPos_ec;

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

    vec3 I = Ia * Ka + fatt * Il * (Kd * cos_theta + Ks * pow(cos_alpha, n)) /* vec3(vColor)*/;

    return vec4(I,1);
}

void main(){
    if (ColorMode == 1) {
        FragColor = vec4(0,0,1,1);
        return;
    }
    
    FragColor = shading(LightPos_ec, vec3(fNormal), vec3(fPosition));
}

