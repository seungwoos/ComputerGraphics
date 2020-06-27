#version 410

in vec4 fColor;
out vec4 FragColor;
uniform int draw_mode;

void main() {
    switch(draw_mode) {
        case 0:
            FragColor = fColor;
            break;
        case 1: // black
            FragColor = vec4(0,0,0,1);
            break;
        case 2: // red
            FragColor = vec4(1,0,0,0);
            break;
        case 3: // green
            FragColor = vec4(0,1,0,0);
            break;
        case 4: // green
            FragColor = vec4(0,0,1,0);
            break;
        case 5:
            FragColor = vec4(1,1,1,1);
            break;

    }
}
