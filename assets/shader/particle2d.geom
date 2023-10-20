#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in VS_IN {
    vec4 ginColor;
    mat4 ginVP;
} vs_in[];

out vec4 finColor;

void main() {
    finColor = vs_in[0].ginColor;
    mat4 VP = vs_in[0].ginVP;
    
    vec4 offset_x = VP * vec4(0.2, 0.0, 0.0, 0.0);
    vec4 offset_y = VP * vec4(0.0, 0.2, 0.0, 0.0);
    //
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + offset_x;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + offset_x + offset_y;
    EmitVertex();
    //
    gl_Position = gl_in[0].gl_Position + offset_x + offset_y;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + offset_y;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    EndPrimitive();
}