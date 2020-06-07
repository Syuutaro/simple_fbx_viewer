#version 330

layout(location = 0) in vec3 xyz;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

uniform mat4 world;
uniform mat4 view;
uniform mat4 perspective;

out vec2 _uv;
out vec3 _normal;

void main(){
    _uv = uv;
    _normal = normal;
    gl_Position = perspective*view*world*vec4(xyz,1.0);
}
