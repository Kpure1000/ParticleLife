#version 330 core
layout (location = 0) in vec2 lPos;
layout (location = 1) in vec4 lCol;

out VS_IN {
    vec4 ginColor;
    mat4 ginVP;
} vs_in;

uniform mat4 _view_proj;

void main() {
    gl_Position = _view_proj * vec4(lPos.x, lPos.y, 0.0, 1.0f);
    vs_in.ginColor = lCol;
    vs_in.ginVP = _view_proj;
}
