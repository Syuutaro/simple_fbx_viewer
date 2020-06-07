#version 330

in vec2 _uv;

out vec4 color;

void main(){
    color = vec4(vec2(_uv.x,1-_uv.y),1,1);
}
