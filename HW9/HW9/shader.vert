#version 410

in vec4 vPosition;
in vec4 vColor;
out vec4 fColor;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main() {
    gl_Position = P * V * M * vPosition;
    fColor = vColor;
}
