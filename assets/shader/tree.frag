#version 330 core

uniform vec4 _color;

out vec4 fragCol;

void main() {
    fragCol = _color;
}
