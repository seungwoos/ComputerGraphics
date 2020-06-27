#version 410

in vec4 fColorFront;
in vec4 fColorBack;
in vec2 fTexcoord;

uniform int ColorMode;  // color mode
uniform int ObjectCode; // object mode
uniform sampler2D sampler;  // sampler2D

out vec4 FragColor;

void main(){
    if (ColorMode == 1) // for wireframes
        FragColor = vec4(0,0,1,1);
    if (ColorMode == 2) { // picking mode
        float nc = ObjectCode / 255.0;
        FragColor = vec4(nc, nc, nc, 1);
    }
    else {
        if ( gl_FrontFacing )
            FragColor = fColorFront;
        else
            FragColor = fColorBack;
        
        FragColor *= texture(sampler, fTexcoord);
    }
}

