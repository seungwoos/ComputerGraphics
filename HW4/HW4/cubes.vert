#version 410

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;
uniform mat4 T; // uniform - global shader variable

void main() {
    gl_Position = T * vPosition;
    fColor = vColor;
}
