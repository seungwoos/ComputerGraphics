#version 410

in vec4 fColor;
out vec4 FragColor;
uniform int drawing_mode;

vec4 convert_into_grayscale(const vec4 in_color) {
    float r = in_color.r;
    float g = in_color.g;
    float b = in_color.b;
    
    float avg = (r+g+b)/3;
    return vec4(avg, avg, avg, 1);
}

vec4 convert_into_complementary(const vec4 in_color) {
    float r = in_color.r;
    float g = in_color.g;
    float b = in_color.b;

    return vec4((1-r), (1-g), (1-b), 1);
}

void main() {
    switch (drawing_mode) {
        case 0:
            FragColor = fColor;
            break;
        
        case 1:
            FragColor = convert_into_grayscale(fColor);
            break;
        
        case 2:
            FragColor = convert_into_complementary(fColor);
            break;
    }
}
   
