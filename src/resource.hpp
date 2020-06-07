#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "library.hpp"
#include "define.hpp"
#include "matrix.hpp"

#include <OpenGL/gl3.h>



class Texture{
private:
    //CPU側でデータを持つべきか否か・・・
    //std::vector<unsigned char> m_rgba;
    
    GLuint m_tbo_rgba;
public:
    Texture(const std::string& path);
    ~Texture();
    
    void Bind(GLint uniform_location,GLint texture_unit) const;
};


class Shader{
private:
    GLuint m_program;
public:
    Shader(const std::string& vs_path,const std::string& fs_path);
    ~Shader();
    
    void Bind() const;
    void UnBind() const;
    
    GLint GetUniformLocation(const std::string& name) const;
private:
    void ReadFile(std::string& text,const std::string& path);
};


class Material{
private:
    const Shader* m_shader;
    std::vector<const Texture*> m_textures;
    std::vector<GLint> m_uniform_locations;
public:
    Material();
    void SetShader(const Shader* shader);
    const Shader* GetShader() const;
    
    //本来ならTexture以外にもスカラーやベクトルを受け付ける
    void AddTexture(const Texture* texture,const std::string& name);
    
    void Bind(GLint texture_unit_offset) const;
};


class Skeleton{
private:
    //CPU側でデータを持つべきか否か・・・
    //std::vector<mat4> m_bbp_i;  //size = bone_count
    //std::vector<mat4> m_bbp_iti;//size = bone_count
    //std::vector<mat4> m_bp;     //size = bone_count
    //std::vector<mat4> m_bp_it;  //size = bone_count
    
    size_t m_bone_count;
    
    GLuint m_tbo_bbp_i;
    GLuint m_tbo_bbp_iti;
    GLuint m_tbo_bp;
    GLuint m_tbo_bp_it;
public:
    Skeleton(const std::vector<mat4>& bbp_i,const std::vector<mat4>& bbp_iti);
    ~Skeleton();
    
    size_t GetBoneCount() const;
    
    void Update(const std::vector<mat4>& bp,const std::vector<mat4>& bp_it);
    
    void Bind(GLint uniform_location_bbp_i,
              GLint uniform_location_bbp_iti,
              GLint uniform_location_bp,
              GLint uniform_location_bp_it,
              GLint texture_unit_offset) const;
};


class SubMesh{
private:
    //CPU側でデータを持つべきか否か・・・
    //std::vector<vec3> m_xyz;         //size = 3*polygon_count
    //std::vector<vec2> m_uv;          //size = 3*polygon_count
    //std::vector<vec3> m_normal;      //size = 3*polygon_count
    //std::vector<int> m_bone_index;   //size = 4*3*polygon_count
    //std::vector<FLOAT> m_bone_weight;//size = 4*3*polygon_count
    
    GLuint m_vao;
    GLuint m_vbo_xyz;
    GLuint m_vbo_uv;
    GLuint m_vbo_normal;
    GLuint m_vbo_bone_index;
    GLuint m_vbo_bone_weight;
    
    size_t m_polygon_vertex_count;
    
    const Material* m_material;
public:
    SubMesh(const std::vector<vec3>& xyz,
            const std::vector<vec2>& uv,
            const std::vector<vec3>& normal,
            const std::vector<int>& bone_index,
            const std::vector<FLOAT>& bone_weight,
            const Material* material);
    ~SubMesh();
    
    void Draw() const;
    
    const Material* GetMaterial() const;
};


class Mesh{
private:
    //sub mesh
    std::vector<SubMesh*> m_sub_meshes;
    
    //skeleton
    Skeleton* m_skeleton;
    
    //今回だけの特別仕様
    //transform for normalizing mesh
    mat4 m_normalizing_transform;
public:
    //今回だけの特別仕様
    Mesh(const std::string& asset_dir_path,
         const std::string& mesh_file_path,
         const Shader* shader,
         bool is_skeletal);
    ~Mesh();
    
    const SubMesh* GetSubMesh(size_t index) const;
    size_t GetSubMeshCount() const;
    
    Skeleton* GetSkeleton() const;
    
    mat4 GetNormalizingTransform() const;
};


class Animation{
private:
    double m_duration;
    size_t m_frame_count;
    std::vector<mat4> m_bp;   //size = frame_count*bone_count
    std::vector<mat4> m_bp_it;//size = frame_count*bone_count
public:
    Animation(const std::string& path);
    ~Animation();
    
    double GetDuration() const;
    
    void Sample(std::vector<mat4>& bp,std::vector<mat4>& bp_it,double time) const;
};


class AnimationController{
public:
    struct Transition{
        bool is_immediate;
        double duration;//normalized time [0,1]
        double offset;  //normalized time [0,1]
        std::string dst_state_id;
    };
    struct State{
        const Animation* animation;
        bool auto_transition_exist;
        bool user_transition_exist;
        Transition auto_transition;
        std::map<std::string,Transition> user_transitions;//key == trigger
    };
private:
    Skeleton* m_skeleton;
    
    std::map<std::string,State> m_states;
    
    double m_current_time;
    double m_transition_beg1;//src state
    double m_transition_end1;//src state
    double m_transition_beg2;//dst state
    double m_transition_end2;//dst state
    const State* m_current_state;
    const Transition* m_current_transition;
    
    //used for calculation
    std::vector<mat4> m_buf[6];
public:
    AnimationController(Skeleton* skeleton,
                        const std::map<std::string,State>& states,
                        const std::string& entry_state_id);
    ~AnimationController();
    
    void DoAutoTransition();
    void DoUserTransition(const std::string& trigger);
    
    void UpdateAnimation(double dt);
};


class ResourceManager{
private:
    static ResourceManager* m_instance;
    std::map<std::string,Texture*> m_textures;
    std::map<std::string,Shader*> m_shaders;
    std::map<std::string,Mesh*> m_meshes;
    std::map<std::string,Animation*> m_animations;
private:
    ResourceManager();
public:
    //singleton
    static void CreateInstance();
    static void DeleteInstance();
    static ResourceManager* GetInstance();
    
    Texture* LoadTexture(const std::string& path);
    Shader* LoadShader(const std::string& vs_path,const std::string& fs_path);
    Mesh* LoadMesh(const std::string& asset_dir_path,const std::string& mesh_file_path,const Shader* shader,bool is_skeletal);
    Animation* LoadAnimation(const std::string& path);
    void UnLoadResource();
};

#endif // RESOURCE_HPP
