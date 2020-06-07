#version 330

layout(location = 0) in vec3 xyz;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in ivec4 bone_index;
layout(location = 4) in vec4 bone_weight;

//bbp = bone bind pose
//i = inverse
//t = transpose
uniform sampler1D bbp_i;
uniform sampler1D bbp_iti;
uniform sampler1D bp;
uniform sampler1D bp_it;

uniform mat4 world;
uniform mat4 view;
uniform mat4 perspective;

out vec2 _uv;
out vec3 _normal;

void main(){
    mat4 m_bbp_i[4],m_bbp_iti[4],m_bp[4],m_bp_it[4];
    for (int i = 0;i < 4;i++){
        for (int j = 0;j < 4;j++){
            m_bbp_i[i][j] = texelFetch(bbp_i,4*bone_index[i]+j,0);
            m_bbp_iti[i][j] = texelFetch(bbp_iti,4*bone_index[i]+j,0);
            m_bp[i][j] = texelFetch(bp,4*bone_index[i]+j,0);
            m_bp_it[i][j] = texelFetch(bp_it,4*bone_index[i]+j,0);
        }
    }
    
    mat4 bone_matrix_xyz = bone_weight[0]*m_bp[0]*m_bbp_i[0]
                          +bone_weight[1]*m_bp[1]*m_bbp_i[1]
                          +bone_weight[2]*m_bp[2]*m_bbp_i[2]
                          +bone_weight[3]*m_bp[3]*m_bbp_i[3];
    
    mat4 bone_matrix_normal = bone_weight[0]*m_bp_it[0]*m_bbp_iti[0]
                             +bone_weight[1]*m_bp_it[1]*m_bbp_iti[1]
                             +bone_weight[2]*m_bp_it[2]*m_bbp_iti[2]
                             +bone_weight[3]*m_bp_it[3]*m_bbp_iti[3];
    
    _uv = uv;
    _normal = normalize((bone_matrix_normal*vec4(normal,0)).xyz);
    gl_Position = perspective*view*world*bone_matrix_xyz*vec4(xyz,1);
}
