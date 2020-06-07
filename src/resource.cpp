#include "resource.hpp"
#include "fbx_loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



Texture::Texture(const std::string& path){
    //read texture by using stb
    int w,h,n;
    unsigned char* data = stbi_load(path.c_str(),&w,&h,&n,4);
    if (data == NULL){
        std::cout << "failed to create texture:" << path << "\n";
        std::terminate();
    }
    
    //create opengl buffer
    glGenTextures(1,&m_tbo_rgba);
    glBindTexture(GL_TEXTURE_2D,m_tbo_rgba);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
    
    stbi_image_free(data);
}

Texture::~Texture(){
    glDeleteTextures(1,&m_tbo_rgba);
}

void Texture::Bind(GLint uniform_location,GLint texture_unit) const{
    glActiveTexture(GL_TEXTURE0+texture_unit);
    glBindTexture(GL_TEXTURE_2D,m_tbo_rgba);
    glUniform1i(uniform_location,texture_unit);
}









Shader::Shader(const std::string& vs_path,const std::string& fs_path){
    //read shader file
    std::string vs_file,fs_file;
    ReadFile(vs_file,vs_path);
    ReadFile(fs_file,fs_path);
    const char* vs_source = vs_file.c_str();
    const char* fs_source = fs_file.c_str();
    const GLint vs_length = vs_file.size();
    const GLint fs_length = fs_file.size();

    //create shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vs,1,&vs_source,&vs_length);
    glShaderSource(fs,1,&fs_source,&fs_length);

    //compile shader
    glCompileShader(vs);
    glCompileShader(fs);

    //compile error check
    GLint state;
    glGetShaderiv(vs,GL_COMPILE_STATUS,&state);
    if (state == GL_FALSE){
        std::cout << "failed to compile:" << vs_path << "\n";
        std::terminate();
    }
    glGetShaderiv(fs,GL_COMPILE_STATUS,&state);
    if (state == GL_FALSE){
        std::cout << "failed to compile:" << fs_path << "\n";
        std::terminate();
    }

    //create program object
    m_program = glCreateProgram();

    //attach shader
    glAttachShader(m_program,vs);
    glAttachShader(m_program,fs);
    
    //delete shader
    glDeleteShader(vs);
    glDeleteShader(fs);

    //link program
    glLinkProgram(m_program);
}

Shader::~Shader(){
    glDeleteProgram(m_program);
}

void Shader::Bind() const{
    glUseProgram(m_program);
}

void Shader::UnBind() const{
    glUseProgram(0);
}

GLint Shader::GetUniformLocation(const std::string& name) const{
    return glGetUniformLocation(m_program,(const GLchar*)name.c_str());
}

void Shader::ReadFile(std::string& text,const std::string& path){
    //open file
    std::FILE* fp = std::fopen(path.c_str(),"r");
    if (fp == NULL){
        std::cout << "failed to open" << path << "\n";
        std::terminate();
    }

    //file size
    std::fseek(fp,0,SEEK_END);
    size_t size = std::ftell(fp);

    //allocate memory
    char* src = new char[size+1];

    //read file
    std::fseek(fp,0,SEEK_SET);
    std::fread(src,1,size,fp);
    src[size] = '\0';

    //copy file source
    text = src;

    //free memory
    delete[] src;

    //close file
    std::fclose(fp);
}







Material::Material():m_shader(nullptr){}

void Material::SetShader(const Shader* shader){
    m_shader = shader;
}

const Shader* Material::GetShader() const{
    return m_shader;
}

void Material::AddTexture(const Texture* texture,const std::string& name){
    if (m_shader == nullptr){
        std::cout << "void Material::AddTexture" << "\n";
        std::terminate();
    }
    m_shader->Bind();
    m_textures.push_back(texture);
    m_uniform_locations.push_back(m_shader->GetUniformLocation(name));
    m_shader->UnBind();
}

void Material::Bind(GLint texture_unit_offset) const{
    for (size_t i = 0;i < m_textures.size();i++){
        m_textures[i]->Bind(m_uniform_locations[i],texture_unit_offset+(GLint)i);
    }
}









Skeleton::Skeleton(const std::vector<mat4>& bbp_i,const std::vector<mat4>& bbp_iti){
    //bone count
    m_bone_count = bbp_i.size();
    
    //bbp_i
    glGenTextures(1,&m_tbo_bbp_i);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bbp_i);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,4*m_bone_count,0,GL_RGBA,GL_FLOAT,bbp_i.data());
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glBindTexture(GL_TEXTURE_1D,0);
    
    //bbp_iti
    glGenTextures(1,&m_tbo_bbp_iti);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bbp_iti);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,4*m_bone_count,0,GL_RGBA,GL_FLOAT,bbp_iti.data());
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glBindTexture(GL_TEXTURE_1D,0);
    
    //bp
    glGenTextures(1,&m_tbo_bp);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,4*m_bone_count,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glBindTexture(GL_TEXTURE_1D,0);
    
    //bp_it
    glGenTextures(1,&m_tbo_bp_it);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp_it);
    glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,4*m_bone_count,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glBindTexture(GL_TEXTURE_1D,0);
}

Skeleton::~Skeleton(){
    glDeleteTextures(1,&m_tbo_bbp_i);
    glDeleteTextures(1,&m_tbo_bbp_iti);
    glDeleteTextures(1,&m_tbo_bp);
    glDeleteTextures(1,&m_tbo_bp_it);
}

size_t Skeleton::GetBoneCount() const{
    return m_bone_count;
}

void Skeleton::Update(const std::vector<mat4>& bp,const std::vector<mat4>& bp_it){
    //bp
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp);
    glTexSubImage1D(GL_TEXTURE_1D,0,0,4*m_bone_count,GL_RGBA,GL_FLOAT,bp.data());
    glBindTexture(GL_TEXTURE_1D,0);
    
    //bp_it
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp_it);
    glTexSubImage1D(GL_TEXTURE_1D,0,0,4*m_bone_count,GL_RGBA,GL_FLOAT,bp_it.data());
    glBindTexture(GL_TEXTURE_1D,0);
}

void Skeleton::Bind(GLint uniform_location_bbp_i,
                    GLint uniform_location_bbp_iti,
                    GLint uniform_location_bp,
                    GLint uniform_location_bp_it,
                    GLint texture_unit_offset) const
{
    //bbp_i
    glActiveTexture(GL_TEXTURE0+texture_unit_offset+0);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bbp_i);
    glUniform1i(uniform_location_bbp_i,texture_unit_offset+0);
    
    //bbp_iti
    glActiveTexture(GL_TEXTURE0+texture_unit_offset+1);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bbp_iti);
    glUniform1i(uniform_location_bbp_iti,texture_unit_offset+1);
    
    //bp
    glActiveTexture(GL_TEXTURE0+texture_unit_offset+2);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp);
    glUniform1i(uniform_location_bp,texture_unit_offset+2);
    
    //bp_it
    glActiveTexture(GL_TEXTURE0+texture_unit_offset+3);
    glBindTexture(GL_TEXTURE_1D,m_tbo_bp_it);
    glUniform1i(uniform_location_bp_it,texture_unit_offset+3);
}









SubMesh::SubMesh(const std::vector<vec3>& xyz,
                 const std::vector<vec2>& uv,
                 const std::vector<vec3>& normal,
                 const std::vector<int>& bone_index,
                 const std::vector<FLOAT>& bone_weight,
                 const Material* material)
{
    //vao
    glGenVertexArrays(1,&m_vao);
    glBindVertexArray(m_vao);

    //xyz
    glGenBuffers(1,&m_vbo_xyz);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_xyz);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vec3)*xyz.size(),xyz.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);

    //uv
    glGenBuffers(1,&m_vbo_uv);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_uv);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vec2)*uv.size(),uv.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
    
    //normal
    glGenBuffers(1,&m_vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_normal);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vec3)*normal.size(),normal.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
    
    //bone index
    glGenBuffers(1,&m_vbo_bone_index);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_bone_index);
    glBufferData(GL_ARRAY_BUFFER,sizeof(int)*bone_index.size(),bone_index.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3,4,GL_INT,0,0);//caution
    
    //bone weight
    glGenBuffers(1,&m_vbo_bone_weight);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_bone_weight);
    glBufferData(GL_ARRAY_BUFFER,sizeof(FLOAT)*bone_weight.size(),bone_weight.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4,4,GL_FLOAT,GL_FALSE,0,0);
    
    //unbind vao
    glBindVertexArray(0);
    
    //polygon vertex count
    m_polygon_vertex_count = xyz.size();
    
    //material
    m_material = material;
}

SubMesh::~SubMesh(){
    glDeleteVertexArrays(1,&m_vao);
    glDeleteBuffers(1,&m_vbo_xyz);
    glDeleteBuffers(1,&m_vbo_uv);
    glDeleteBuffers(1,&m_vbo_normal);
    glDeleteBuffers(1,&m_vbo_bone_index);
    glDeleteBuffers(1,&m_vbo_bone_weight);
    delete m_material;
}

void SubMesh::Draw() const{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES,0,m_polygon_vertex_count);
    glBindVertexArray(0);
}

const Material* SubMesh::GetMaterial() const{
    return m_material;
}








Mesh::Mesh(const std::string& asset_dir_path,
           const std::string& mesh_file_path,
           const Shader* shader,
           bool is_skeletal)
{
    //load fbx file
    FBXMeshLoader loader(mesh_file_path);
    const std::vector<FBXMeshLoader::Mesh>& mh = loader.GetMeshes();
    const FBXMeshLoader::Skeleton& sn = loader.GetSkeleton();
    
    //create skeleton
    if (is_skeletal){
        m_skeleton = new Skeleton(sn.bbp_i,sn.bbp_iti);
    }else{
        m_skeleton = nullptr;
    }
    
    //normalizing transform
    m_normalizing_transform = loader.GetNormalizingTransform();
    
    //create sub meshes
    m_sub_meshes.resize(mh.size());
    for (size_t i = 0;i < m_sub_meshes.size();i++){
        
        //今回はマテリアルファイルを用意しない
        //通常、マテリアル、シェーダーの作成はマテリアルファイルのロード時に行うので、以下の処理はイレギュラー
        //通常、sub meshが持っているマテリアル名をキーにしてResourceManagerからマテリアルのインスタンスを得る
        //マテリアルファイルにシェーダー変数名と型のリストがあることを期待する
        
        //create material
        Material* material = new Material();
        material->SetShader(shader);
        if (mh[i].texture != ""){
            //テクスチャ名が格納されているときのみロード
            const Texture* texture = ResourceManager::GetInstance()->LoadTexture(asset_dir_path+"/texture/"+mh[i].texture);
            material->AddTexture(texture,"diffuse_texture");
        }
        
        //create sub mesh
        if (is_skeletal){
            m_sub_meshes[i] = new SubMesh(mh[i].xyz,mh[i].uv,mh[i].normal,mh[i].bone_index,mh[i].bone_weight,material);
        }else{
            m_sub_meshes[i] = new SubMesh(mh[i].xyz,mh[i].uv,mh[i].normal,std::vector<int>(),std::vector<FLOAT>(),material);
        }
    }
}

Mesh::~Mesh(){
    for (size_t i = 0;i < m_sub_meshes.size();i++){
        delete m_sub_meshes[i];
    }
    delete m_skeleton;
}

const SubMesh* Mesh::GetSubMesh(size_t index) const{
    return m_sub_meshes[index];
}

size_t Mesh::GetSubMeshCount() const{
    return m_sub_meshes.size();
}

Skeleton* Mesh::GetSkeleton() const{
    return m_skeleton;
}

mat4 Mesh::GetNormalizingTransform() const{
    return m_normalizing_transform;
}









Animation::Animation(const std::string& path){
    //load fbx file
    FBXAnimationLoader loader(path);
    const FBXAnimationLoader::Animation& an = loader.GetAnimation();
    
    m_duration = an.duration;
    m_frame_count = an.frame_count;
    m_bp = an.bp;
    m_bp_it = an.bp_it;
}

Animation::~Animation(){}

double Animation::GetDuration() const{
    return m_duration;
}

void Animation::Sample(std::vector<mat4>& bp,std::vector<mat4>& bp_it,double time) const{
    //bone count
    size_t bone_count = m_bp.size()/m_frame_count;
    
    //check bp size
    if (bone_count != bp.size()){
        std::cout << "Error/AnimationClip::SampleBonePose" << "\n";
        std::terminate();
    }
    
    //frame interval
    double frame_interval = m_duration/(m_frame_count-1);
    
    //sampled frame
    size_t sampled_frame = 0;
    for (size_t i = 0;i <= m_frame_count-2;i++){
        if (time >= frame_interval*i && time <= frame_interval*(i+1)){
            sampled_frame = i;
            break;
        }
    }
    
    //weight
    double w = (time-frame_interval*sampled_frame)/frame_interval;
    
    //linear interpolation
    for (size_t i = 0;i < bone_count;i++){
        //bp
        const mat4& m1 = m_bp[sampled_frame*bone_count+i];
        const mat4& m2 = m_bp[(sampled_frame+1)*bone_count+i];
        bp[i] = m1*(1-w)+m2*w;
        
        //bp_it
        const mat4& m3 = m_bp_it[sampled_frame*bone_count+i];
        const mat4& m4 = m_bp_it[(sampled_frame+1)*bone_count+i];
        bp_it[i] = m3*(1-w)+m4*w;
    }
}









AnimationController::AnimationController(Skeleton* skeleton,
                                         const std::map<std::string,State>& states,
                                         const std::string& entry_state_id)
{
    //check states
    for (auto i = states.begin();i != states.end();++i){
        const State& state = i->second;
        
        //auto transition
        if (state.auto_transition_exist){
            const Transition& transition = state.auto_transition;
            if (states.count(transition.dst_state_id) == 0){
                std::cout << "void AnimationController::Initialize" << "\n";
                std::cout << "auto transition's dst state:" << transition.dst_state_id << "does not exist" << "\n";
                std::terminate();
            }
        }
        
        //user transition
        if (state.user_transition_exist){
            for (auto j = state.user_transitions.begin();j != state.user_transitions.end();j++){
                const Transition& transition = j->second;
                if (states.count(transition.dst_state_id) == 0){
                    std::cout << "void AnimationController::Initialize" << "\n";
                    std::cout << "user transition's dst state:" << transition.dst_state_id << "does not exist" << "\n";
                    std::terminate();
                }
            }
        }
    }
    
    //check entry state
    if (states.count(entry_state_id) == 0){
        std::cout << "void AnimationController::Initialize" << "\n";
        std::cout << "entry state:" << entry_state_id << "does not exist" << "\n";
        std::terminate();
    }
    
    
    m_skeleton = skeleton;
    m_states = states;
    
    m_current_time = 0;
    m_current_state = &m_states.at(entry_state_id);
    m_current_transition = nullptr;
    
    for (int i = 0;i < 6;i++){
        m_buf[i].resize(m_skeleton->GetBoneCount());
    }
}

AnimationController::~AnimationController(){}

void AnimationController::DoAutoTransition(){
    //遷移が既に発動しているならは発動しない
    if (m_current_transition != nullptr){
        return;
    }
    
    //現在のステートがそもそも自動遷移を持っていなかったら、発動しない
    if (!m_current_state->auto_transition_exist){
        return;
    }
    
    //自動遷移の場合、遷移は全てnon immediateとして扱う
    //遷移が発動する条件は
    //１　現在時刻が遷移区間に入った後（遷移区間はアニメーションの終端を含む）
    //２　遷移先の遷移区間がduration内に収まっていること
    //現在時刻が自動遷移区間に入る前なら発動
    const Transition& transition = m_current_state->auto_transition;
    const State& dst_state = m_states.at(m_current_state->auto_transition.dst_state_id);
    
    double transition_beg1 = m_current_state->animation->GetDuration()*(1-transition.duration);
    double transition_end1 = m_current_state->animation->GetDuration();
    double transition_beg2 = dst_state.animation->GetDuration()*transition.offset;
    double transition_end2 = transition_beg2+transition_end1-transition_beg1;
    
    if (m_current_time >= transition_beg1 && transition_end2 <= dst_state.animation->GetDuration()){
        m_current_transition = &transition;
        m_transition_beg1 = transition_beg1;
        m_transition_end1 = transition_end1;
        m_transition_beg2 = transition_beg2;
        m_transition_end2 = transition_end2;
    }
}

void AnimationController::DoUserTransition(const std::string& trigger){
    //トリガーが未登録なら発動しない
    if (m_current_state->user_transitions.count(trigger) == 0){
        return;
    }
    
    //遷移が既に発動しているなら発動しない
    if (m_current_transition != nullptr){
        return;
    }
    
    //現在のステートがそもそもユーザー遷移を持っていなかったら、発動しない
    if (!m_current_state->user_transition_exist){
        return;
    }
    
    //ユーザー遷移の場合、遷移がimmediateかそうでないかで場合分けをする
    const Transition& transition = m_current_state->user_transitions.at(trigger);
    const State& dst_state = m_states.at(transition.dst_state_id);
    if (transition.is_immediate){
        //immediate
        //遷移が発動する条件は
        //１　遷移元の遷移区間がduration内に収まっていること（遷移区間はアニメーションの終端を含まない）
        //２　遷移先の遷移区間がduration内に収まっていること
        double transition_beg1 = m_current_time;
        double transition_end1 = transition_beg1+m_current_state->animation->GetDuration()*transition.duration;
        double transition_beg2 = dst_state.animation->GetDuration()*transition.offset;
        double transition_end2 = transition_beg2+transition_end1-transition_beg1;
        
        if (transition_end1 <= m_current_state->animation->GetDuration() && transition_end2 <= dst_state.animation->GetDuration()){
            m_current_transition = &transition;
            m_transition_beg1 = transition_beg1;
            m_transition_end1 = transition_end1;
            m_transition_beg2 = transition_beg2;
            m_transition_end2 = transition_end2;
        }
    }else{
        //non immediate
        //遷移が発動する条件は
        //１　現在時刻が遷移区間に入る前（遷移区間はアニメーションの終端を含む）
        //２　遷移先の遷移区間がduration内に収まっていること
        double transition_beg1 = m_current_state->animation->GetDuration()*(1-transition.duration);
        double transition_end1 = m_current_state->animation->GetDuration();
        double transition_beg2 = dst_state.animation->GetDuration()*transition.offset;
        double transition_end2 = transition_beg2+transition_end1-transition_beg1;
        
        if (m_current_time < transition_beg1 && transition_end2 <= dst_state.animation->GetDuration()){
            m_current_transition = &transition;
            m_transition_beg1 = transition_beg1;
            m_transition_end1 = transition_end1;
            m_transition_beg2 = transition_beg2;
            m_transition_end2 = transition_end2;
        }
    }
}

void AnimationController::UpdateAnimation(double dt){
    //現在時刻を更新
    m_current_time += dt;
    
    //自動遷移を発動させる
    //場合によってはキャンセルされる
    DoAutoTransition();
    
    //遷移が発動中かどうかで場合分け
    if (m_current_transition == nullptr){
        //現在、遷移が発動していない
        //アニメーションの終端を超えていたら時間をループさせる
        if (m_current_time > m_current_state->animation->GetDuration()){
            m_current_time = 0;
        }
        
        //現在のステートのアニメーションのみからサンプリング
        m_current_state->animation->Sample(m_buf[0],m_buf[1],m_current_time);
        
        //スケルトンの更新
        m_skeleton->Update(m_buf[0],m_buf[1]);
    }else{
        //現在、遷移が発動している
        //クロスフェード対象区間に入る前か、入っている最中か、出た後かで場合分け
        if (m_current_time < m_transition_beg1){
            //入る前
            //現在のステートのアニメーションのみからサンプリング
            m_current_state->animation->Sample(m_buf[0],m_buf[1],m_current_time);
            
            //スケルトンの更新
            m_skeleton->Update(m_buf[0],m_buf[1]);
        }else if (m_current_time >= m_transition_beg1 && m_current_time <= m_transition_end1){
            //入っている最中
            //遷移元ステートの重み
            double w = (m_transition_end1-m_current_time)/(m_transition_end1-m_transition_beg1);
            
            //遷移元ステートと遷移先ステートのアニメーションをサンプリングして線形補間
            double time1 = m_current_time;
            double time2 = m_transition_beg2+(m_current_time-m_transition_beg1);
            m_current_state->animation->Sample(m_buf[0],m_buf[1],time1);
            m_states.at(m_current_transition->dst_state_id).animation->Sample(m_buf[2],m_buf[3],time2);
            
            for (size_t i = 0;i < m_buf[0].size();i++){
                m_buf[4][i] = m_buf[0][i]*w+m_buf[2][i]*(1-w);
                m_buf[5][i] = m_buf[1][i]*w+m_buf[3][i]*(1-w);
            }
            
            //スケルトンの更新
            m_skeleton->Update(m_buf[4],m_buf[5]);
        }else if (m_current_time > m_transition_end1){
            //出た後
            //遷移終了
            m_current_time = m_transition_end2+(m_current_time-m_transition_end1);
            m_current_state = &m_states.at(m_current_transition->dst_state_id);
            m_current_transition = nullptr;
            
            //現在のステートのアニメーションのみからサンプリング
            m_current_state->animation->Sample(m_buf[0],m_buf[1],m_current_time);
            
            //スケルトンの更新
            m_skeleton->Update(m_buf[0],m_buf[1]);
        }
    }
}







ResourceManager* ResourceManager::m_instance = nullptr;
ResourceManager::ResourceManager(){}

void ResourceManager::CreateInstance(){
    if (m_instance == nullptr){
        m_instance = new ResourceManager();
    }
}

void ResourceManager::DeleteInstance(){
    delete m_instance;
    m_instance = nullptr;
}

ResourceManager* ResourceManager::GetInstance(){
    if (m_instance == nullptr){
        std::cout << "ResourceManager has not yet been initialized" << "\n";
        std::terminate();
    }
    return m_instance;
}


Texture* ResourceManager::LoadTexture(const std::string& path){
    if (m_textures.count(path) == 0){
        Texture* texture = new Texture(path);
        m_textures[path] = texture;
        return texture;
    }else{
        return m_textures.at(path);
    }
}

Shader* ResourceManager::LoadShader(const std::string& vs_path,const std::string& fs_path){
    if (m_shaders.count(vs_path+fs_path) == 0){
        Shader* shader = new Shader(vs_path,fs_path);
        m_shaders[vs_path+fs_path] = shader;
        return shader;
    }else{
        return m_shaders.at(vs_path+fs_path);
    }
}

Mesh* ResourceManager::LoadMesh(const std::string& asset_dir_path,
                                const std::string& mesh_file_path,
                                const Shader* shader,
                                bool is_skeletal)
{
    if (m_meshes.count(mesh_file_path) == 0){
        Mesh* mesh = new Mesh(asset_dir_path,mesh_file_path,shader,is_skeletal);
        m_meshes[mesh_file_path] = mesh;
        return mesh;
    }else{
        return m_meshes.at(mesh_file_path);
    }
}

Animation* ResourceManager::LoadAnimation(const std::string& path){
    if (m_animations.count(path) == 0){
        Animation* animation = new Animation(path);
        m_animations[path] = animation;
        return animation;
    }else{
        return m_animations.at(path);
    }
}

void ResourceManager::UnLoadResource(){
    for (auto i = m_textures.begin();i != m_textures.end();++i){
        delete i->second;
    }
    for (auto i = m_shaders.begin();i != m_shaders.end();++i){
        delete i->second;
    }
    for (auto i = m_meshes.begin();i != m_meshes.end();++i){
        delete i->second;
    }
    for (auto i = m_animations.begin();i != m_animations.end();++i){
        delete i->second;
    }
    m_textures.clear();
    m_shaders.clear();
    m_meshes.clear();
    m_animations.clear();
}
