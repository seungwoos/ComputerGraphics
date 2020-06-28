#version 410

in vec4 fColorFront;
in vec4 fColorBack;
out vec4 FragColor;

uniform int ColorMode;

void main(){
    if ( ColorMode == 1 ) { // for wireframe
        FragColor = vec4(0,0,1,1);
        return;
    }
    
    if ( gl_FrontFacing )
        FragColor = fColorFront;
    else
        FragColor = fColorBack;
}
