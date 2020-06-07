#version 330

in vec3 _normal;

out vec4 color;

void main(){
    color = vec4(_normal,1);
}
