#version 410

in vec4 vPosition;
uniform mat4 T; // uniform - global shader variable

void main() {
    gl_Position = T * vPosition;
}
