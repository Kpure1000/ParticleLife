#version 330 core
layout (location = 0) in vec2 lPos;

uniform mat4 _view_proj;

void main() {
    gl_Position = _view_proj * vec4(lPos.x, lPos.y, 0.0, 1.0f);
}
