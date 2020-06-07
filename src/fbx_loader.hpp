#ifndef FBX_LOADER_HPP
#define FBX_LOADER_HPP

#include "library.hpp"
#include "define.hpp"
#include "matrix.hpp"

#include "fbxsdk.h"

class FBXMeshLoader{
public:
    struct Mesh{
        std::vector<vec3> xyz;          //size = 3*polygon_count
        std::vector<vec2> uv;           //size = 3*polygon_count
        std::vector<vec3> normal;       //size = 3*polygon_count
        std::vector<int> bone_index;    //size = 4*3*polygon_count
        std::vector<FLOAT> bone_weight; //size = 4*3*polygon_count
        std::string texture;
    };
    struct Skeleton{
        //bbp = bone bind pose,i = inverse,t = transpose
        std::vector<mat4> bbp_i;  //for xyz,size = bone_count
        std::vector<mat4> bbp_iti;//for normal,size = bone_count
    };
private:
    std::vector<FbxNode*> m_fskeleton_nodes;
    std::vector<FbxNode*> m_fmesh_nodes;
    std::vector<FbxMesh*> m_fmeshes;
    
    FbxMatrix m_axis_transform;
    
    std::vector<Mesh> m_meshes;
    Skeleton m_skeleton;
public:
    FBXMeshLoader(const std::string& path);
    const std::vector<FBXMeshLoader::Mesh>& GetMeshes() const;
    const FBXMeshLoader::Skeleton& GetSkeleton() const;
    mat4 GetNormalizingTransform() const;
    void PrintData() const;
private:
    int GetBoneIndex(FbxNode* fskeleton_node) const;
    void TraverseNodeTree(FbxNode* fnode);
    void LoadMeshes();
    void LoadSkeleton();
};


class FBXAnimationLoader{
public:
    struct Animation{
        double duration;
        size_t frame_count;
        std::vector<mat4> bp;//for xyz,size = frame_count*bone_count
        std::vector<mat4> bp_it;//for normal,size = frame_count*bone_count
    };
private:
    std::vector<FbxNode*> m_fskeleton_nodes;
    FbxMatrix m_axis_transform;
    Animation m_animation;
public:
    FBXAnimationLoader(const std::string& path);
    const FBXAnimationLoader::Animation& GetAnimation() const;
    void PrintData() const;
private:
    void TraverseNodeTree(FbxNode* fnode);
    void LoadAnimation(FbxScene* fscene);
};

#endif // FBX_LOADER_HPP
