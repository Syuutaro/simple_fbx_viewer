#version 330

in vec2 _uv;
uniform sampler2D diffuse_texture;

out vec4 color;

void main(){
    color = texture(diffuse_texture,vec2(_uv.x,1-_uv.y));
}
