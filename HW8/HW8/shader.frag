#version 410

in vec4 fColor;
out vec4 FragColor;
uniform int draw_mode;

void main() {
    switch(draw_mode) {
        case 0:
            FragColor = fColor;
            break;
            
        case 1:
            FragColor = vec4(0,0,0,1);
            break;
    }
}
