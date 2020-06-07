#version 330

in vec2 _uv;
out vec4 color;

void main(){
    ivec2 t = ivec2(500*_uv);
    int k = 10;
    int r = 5;
    if ((t.x%k < r && t.y%k < r) || (t.x%k >= r && t.y%k >= r)){
        color = vec4(1,1,1,1);
    }else{
        color = vec4(0,0,0,1);
    }
}
