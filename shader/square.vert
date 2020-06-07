#version 330

layout(location = 0) in vec3 xyz;
layout(location = 1) in vec2 uv;

uniform mat4 view;
uniform mat4 perspective;

out vec2 _uv;

void main(){
    _uv = uv;
    gl_Position = perspective*view*vec4(xyz,1.0);
}
