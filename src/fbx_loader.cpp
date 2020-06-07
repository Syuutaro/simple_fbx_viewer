#include "fbx_loader.hpp"


static FbxMatrix create_axis_transform(FbxScene* fscene){
    int sign;
    FbxAxisSystem::EUpVector up_vector = fscene->GetGlobalSettings().GetAxisSystem().GetUpVector(sign);
    FbxAxisSystem::ECoordSystem coord_sys = fscene->GetGlobalSettings().GetAxisSystem().GetCoorSystem();
    FbxMatrix axis_transform;
    if (up_vector == FbxAxisSystem::eXAxis){
        if (coord_sys == FbxAxisSystem::eRightHanded){
            axis_transform.SetRow(0,FbxVector4(0,0,1,0));
            axis_transform.SetRow(1,FbxVector4(1,0,0,0));
            axis_transform.SetRow(2,FbxVector4(0,1,0,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }else{
            axis_transform.SetRow(0,FbxVector4(0,1,0,0));
            axis_transform.SetRow(1,FbxVector4(1,0,0,0));
            axis_transform.SetRow(2,FbxVector4(0,0,1,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }
    }else if (up_vector == FbxAxisSystem::eYAxis){
        if (coord_sys == FbxAxisSystem::eRightHanded){
            axis_transform.SetRow(0,FbxVector4(1,0,0,0));
            axis_transform.SetRow(1,FbxVector4(0,1,0,0));
            axis_transform.SetRow(2,FbxVector4(0,0,1,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }else{
            axis_transform.SetRow(0,FbxVector4(0,0,1,0));
            axis_transform.SetRow(1,FbxVector4(0,1,0,0));
            axis_transform.SetRow(2,FbxVector4(1,0,0,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }
    }else{
        if (coord_sys == FbxAxisSystem::eRightHanded){
            axis_transform.SetRow(0,FbxVector4(0,1,0,0));
            axis_transform.SetRow(1,FbxVector4(0,0,1,0));
            axis_transform.SetRow(2,FbxVector4(1,0,0,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }else{
            axis_transform.SetRow(0,FbxVector4(1,0,0,0));
            axis_transform.SetRow(1,FbxVector4(0,0,1,0));
            axis_transform.SetRow(2,FbxVector4(0,1,0,0));
            axis_transform.SetRow(3,FbxVector4(0,0,0,1));
        }
    }
    return axis_transform;
}



FBXMeshLoader::FBXMeshLoader(const std::string& path){
    //initialize fbxsdk
    FbxManager* fmanager = FbxManager::Create();
    FbxScene* fscene = FbxScene::Create(fmanager,"");
    FbxImporter* fimporter = FbxImporter::Create(fmanager,"");
    if (!fimporter->Initialize(path.c_str())){
        std::cout << "failed to read fbx file:" << path << "\n";
        std::terminate();
    }
    fimporter->Import(fscene);
    
    //triangulate scene
    FbxGeometryConverter geometryConverter(fmanager);
    geometryConverter.Triangulate(fscene,true);
    geometryConverter.RemoveBadPolygonsFromMeshes(fscene);
    geometryConverter.SplitMeshesPerMaterial(fscene,true);
    
    //traverse node tree
    TraverseNodeTree(fscene->GetRootNode());
    
    //create axis transform
    m_axis_transform = create_axis_transform(fscene);
    
    //check fmesh count
    if (m_fmeshes.size() == 0){
        std::cout << "failed to load fbx file:" << path << "\n";
        std::cout << "mesh data is not included" << "\n";
        std::terminate();
    }
    
    //load meshes
    LoadMeshes();
    
    //load skeleton
    LoadSkeleton();
    
    //terminate fbxsdk
    fmanager->Destroy();
    
    //PrintData();
}

const std::vector<FBXMeshLoader::Mesh>& FBXMeshLoader::GetMeshes() const{
    return m_meshes;
}

const FBXMeshLoader::Skeleton& FBXMeshLoader::GetSkeleton() const{
    return m_skeleton;
}

mat4 FBXMeshLoader::GetNormalizingTransform() const{
    //calculate mesh range
    FLOAT range_x[2],range_y[2],range_z[2];
    for (size_t i = 0;i < m_meshes.size();i++){
        for (size_t j = 0;j < m_meshes[i].xyz.size();j++){
            if (i == 0 && j == 0){
                range_x[0] = m_meshes[i].xyz[j][0];
                range_x[1] = m_meshes[i].xyz[j][0];
                range_y[0] = m_meshes[i].xyz[j][1];
                range_y[1] = m_meshes[i].xyz[j][1];
                range_z[0] = m_meshes[i].xyz[j][2];
                range_z[1] = m_meshes[i].xyz[j][2];
            }else{
                range_x[0] = std::min(range_x[0],m_meshes[i].xyz[j][0]);
                range_x[1] = std::max(range_x[1],m_meshes[i].xyz[j][0]);
                range_y[0] = std::min(range_y[0],m_meshes[i].xyz[j][1]);
                range_y[1] = std::max(range_y[1],m_meshes[i].xyz[j][1]);
                range_z[0] = std::min(range_z[0],m_meshes[i].xyz[j][2]);
                range_z[1] = std::max(range_z[1],m_meshes[i].xyz[j][2]);
            }
        }
    }
    
    //standard point of aabb(y-axis up)
    FLOAT p[3];
    p[0] = 0.5*(range_x[0]+range_x[1]);
    p[1] = range_y[0];
    p[2] = 0.5*(range_z[0]+range_z[1]);
    
    //scale
    FLOAT l = std::max(range_x[1]-range_x[0],std::max(range_y[1]-range_y[0],range_z[1]-range_z[0]));
    FLOAT s;
    if (l == range_y[1]-range_y[0]){
        s = 1/l;
    }else{
        s = 1/(0.5*l);
    }
    
    //translate matrix
    mat4 T;
    T.SetRow(0,vec4({1,0,0,-p[0]}));
    T.SetRow(1,vec4({0,1,0,-p[1]}));
    T.SetRow(2,vec4({0,0,1,-p[2]}));
    T.SetRow(3,vec4({0,0,0,1}));
    
    //scale matrix
    mat4 S;
    S.SetRow(0,vec4({s,0,0,0}));
    S.SetRow(1,vec4({0,s,0,0}));
    S.SetRow(2,vec4({0,0,s,0}));
    S.SetRow(3,vec4({0,0,0,1}));
    
    //normalizing transform matrix
    return S*T;
}


void FBXMeshLoader::PrintData() const{
    for (int i = 0;i < m_meshes.size();i++){
        //mesh id
        std::cout << "mesh:" << i << "\n";
        
        //xyz
        for (int j = 0;j < m_meshes[i].xyz.size();j++){
            const vec3& v = m_meshes[i].xyz[j];
            std::cout << "xyz " << j << ":[" << v[0] << "," << v[1] << "," << v[2] << "]" << "\n";
        }
        
        //uv
        for (int j = 0;j < m_meshes[i].uv.size();j++){
            const vec2& v = m_meshes[i].uv[j];
            std::cout << "uv " << j << ":[" << v[0] << "," << v[1] << "]" << "\n";
        }
        
        //normal
        for (int j = 0;j < m_meshes[i].normal.size();j++){
            const vec3& v = m_meshes[i].normal[j];
            std::cout << "normal " << j << ":[" << v[0] << "," << v[1] << "," << v[2] << "]" << "\n";
        }
        
        //bone index,weight
        for (int j = 0;j < m_meshes[i].bone_index.size();j++){
            if (j%4 == 0){
                const int* id = &(m_meshes[i].bone_index[j]);
                const FLOAT* wt = &(m_meshes[i].bone_weight[j]);
                std::cout << "bone index " << j/4 << ":[" << id[0] << "," << id[1] << "," << id[2] << "," << id[3] << "]" << "\n";
                std::cout << "bone weight " << j/4 << ":[" << wt[0] << "," << wt[1] << "," << wt[2] << "," << wt[3] << "]" << "\n";
            }
        }
        
        //texture
        std::cout << "texture:" << m_meshes[i].texture << "\n";
        
        std::cout << "\n";
    }
    
    //skeleton
    std::cout << "skeleton" << "\n";
    std::cout << "bbp_i" << "\n";
    for (size_t i = 0;i < m_skeleton.bbp_i.size();i++){
        const mat4& m = m_skeleton.bbp_i[i];
        const vec4& r0 = m.GetRow(0);
        const vec4& r1 = m.GetRow(1);
        const vec4& r2 = m.GetRow(2);
        const vec4& r3 = m.GetRow(3);
        std::cout << "bone:" << i << "\n";
        std::cout << "    " << r0[0] << " " << r0[1] << " " << r0[2] << " " << r0[3] << "\n";
        std::cout << "    " << r1[0] << " " << r1[1] << " " << r1[2] << " " << r1[3] << "\n";
        std::cout << "    " << r2[0] << " " << r2[1] << " " << r2[2] << " " << r2[3] << "\n";
        std::cout << "    " << r3[0] << " " << r3[1] << " " << r3[2] << " " << r3[3] << "\n";
        std::cout << "\n";
    }
    std::cout << "bbp_iti" << "\n";
    for (size_t i = 0;i < m_skeleton.bbp_iti.size();i++){
        const mat4& m = m_skeleton.bbp_iti[i];
        const vec4& r0 = m.GetRow(0);
        const vec4& r1 = m.GetRow(1);
        const vec4& r2 = m.GetRow(2);
        const vec4& r3 = m.GetRow(3);
        std::cout << "bone:" << i << "\n";
        std::cout << "    " << r0[0] << " " << r0[1] << " " << r0[2] << " " << r0[3] << "\n";
        std::cout << "    " << r1[0] << " " << r1[1] << " " << r1[2] << " " << r1[3] << "\n";
        std::cout << "    " << r2[0] << " " << r2[1] << " " << r2[2] << " " << r2[3] << "\n";
        std::cout << "    " << r3[0] << " " << r3[1] << " " << r3[2] << " " << r3[3] << "\n";
        std::cout << "\n";
    }
    
    std::cout << "\n\n";
    
}

int FBXMeshLoader::GetBoneIndex(FbxNode* fskeleton_node) const{
    for (int i = 0;i < m_fskeleton_nodes.size();i++){
        if (m_fskeleton_nodes[i] == fskeleton_node){
            return i;
        }
    }
    return -1;
}


void FBXMeshLoader::TraverseNodeTree(FbxNode* fnode){
    //null check
    if (fnode == NULL){
        std::cout << "fbx node is nullptr" << "\n";
        std::terminate();
    }
    
    //fskeleton,fmesh
    int fattribute_count = fnode->GetNodeAttributeCount();
    for (int i = 0;i < fattribute_count;i++){
        FbxNodeAttribute* fattribute = fnode->GetNodeAttributeByIndex(i);
        FbxNodeAttribute::EType ftype = fattribute->GetAttributeType();
        if (ftype == FbxNodeAttribute::eSkeleton){
            //fskeleton node
            m_fskeleton_nodes.push_back(fnode);
        }else if (ftype == FbxNodeAttribute::eMesh){
            //fmesh node
            m_fmesh_nodes.push_back(fnode);
            
            //fmesh
            m_fmeshes.push_back((FbxMesh*)fattribute);
        }else{
            //nothing
        }
    }

    //traverse child node
    int child_count = fnode->GetChildCount();
    for (int i = 0;i < child_count;i++){
        TraverseNodeTree(fnode->GetChild(i));
    }
}


void FBXMeshLoader::LoadMeshes(){
    for (int k = 0;k < m_fmeshes.size();k++){
        //fmesh node
        FbxNode* fmesh_node = m_fmesh_nodes[k];
        
        //fmesh
        FbxMesh* fmesh = m_fmeshes[k];
        
        //fmesh node transform
        const FbxAMatrix& fmesh_node_transform = fmesh_node->EvaluateGlobalTransform();
        
        //mesh
        m_meshes.push_back(Mesh());
        Mesh& mesh = m_meshes.back();
        
        //transform for xyz,normal
        mat4 transform_xyz;
        {
            const FbxMatrix& tmp = FbxMatrix(fmesh_node_transform.Transpose())*m_axis_transform;
            const FbxVector4& r0 = tmp.GetRow(0);
            const FbxVector4& r1 = tmp.GetRow(1);
            const FbxVector4& r2 = tmp.GetRow(2);
            const FbxVector4& r3 = tmp.GetRow(3);
            transform_xyz.SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
            transform_xyz.SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
            transform_xyz.SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
            transform_xyz.SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
        }
        mat4 transform_normal;
        {
            const FbxMatrix& tmp = (FbxMatrix(fmesh_node_transform.Transpose())*m_axis_transform).Inverse().Transpose();
            const FbxVector4& r0 = tmp.GetRow(0);
            const FbxVector4& r1 = tmp.GetRow(1);
            const FbxVector4& r2 = tmp.GetRow(2);
            const FbxVector4& r3 = tmp.GetRow(3);
            transform_normal.SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
            transform_normal.SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
            transform_normal.SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
            transform_normal.SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
        }
        
        //polygon vertex
        int polygon_vertex_count = fmesh->GetPolygonVertexCount();
        int* polygon_vertices = fmesh->GetPolygonVertices();
        
        //initialize xyz,uv,normal
        mesh.xyz.resize(polygon_vertex_count);
        mesh.uv.resize(polygon_vertex_count);
        mesh.normal.resize(polygon_vertex_count);
        for (int i = 0;i < polygon_vertex_count;i++){
            mesh.xyz[i] = vec3();
            mesh.uv[i] = vec2();
            mesh.normal[i] = vec3();
        }
        
        //initialize bone_index,bone_weight
        mesh.bone_index.resize(4*polygon_vertex_count);
        mesh.bone_weight.resize(4*polygon_vertex_count);
        for (int i = 0;i < 4*polygon_vertex_count;i++){
            mesh.bone_index[i] = -1;
            mesh.bone_weight[i] = 0;
        }
        
        
        //xyz
        int xyz_count = fmesh->GetControlPointsCount();
        FbxVector4* xyz_array = fmesh->GetControlPoints();
        for (int i = 0;i < polygon_vertex_count;i++){
            int idx = polygon_vertices[i];
            if (idx < xyz_count){
                vec4 xyz;
                xyz[0] = xyz_array[idx][0];
                xyz[1] = xyz_array[idx][1];
                xyz[2] = xyz_array[idx][2];
                xyz[3] = 1;
                mesh.xyz[i] = transform_xyz*xyz;
            }
        }
        
        //flayer
        FbxLayer* flayer = fmesh->GetLayer(0);
        if (flayer != NULL){
            //uv
            FbxLayerElementUV* flayer_uv = flayer->GetUVs();
            if (flayer_uv != NULL){
                FbxLayerElement::EMappingMode uv_mapping_mode = flayer_uv->GetMappingMode();
                FbxLayerElement::EReferenceMode uv_reference_mode = flayer_uv->GetReferenceMode();
                const FbxLayerElementArrayTemplate<FbxVector2>& uv_direct_array = flayer_uv->GetDirectArray();
                const FbxLayerElementArrayTemplate<int>& uv_index_array = flayer_uv->GetIndexArray();
                
                if (uv_mapping_mode == FbxLayerElement::eByPolygonVertex){
                    if (uv_reference_mode == FbxLayerElement::eDirect){
                        for (int i = 0;i < uv_direct_array.GetCount();i++){
                            mesh.uv[i][0] = uv_direct_array[i][0];
                            mesh.uv[i][1] = uv_direct_array[i][1];
                        }
                    }else if (uv_reference_mode == FbxLayerElement::eIndexToDirect){
                        for (int i = 0;i < uv_index_array.GetCount();i++){
                            mesh.uv[i][0] = uv_direct_array[uv_index_array[i]][0];
                            mesh.uv[i][1] = uv_direct_array[uv_index_array[i]][1];
                        }
                    }else{
                        //nothing
                    }
                }else{
                    //nothing
                    //uv_mapping_mode == FbxLayerElement::eByControlPoint
                    //uv_mapping_mode == FbxLayerElement::eByPolygon
                    //uv_mapping_mode == FbxLayerElement::eByEdge
                    //uv_mapping_mode == FbxLayerElement::eAllSame
                }
            }
            
            //normal
            FbxLayerElementNormal* flayer_normal = flayer->GetNormals();
            if (flayer_normal != NULL){
                FbxLayerElement::EMappingMode normal_mapping_mode = flayer_normal->GetMappingMode();
                FbxLayerElement::EReferenceMode normal_reference_mode = flayer_normal->GetReferenceMode();
                const FbxLayerElementArrayTemplate<FbxVector4>& normal_direct_array = flayer_normal->GetDirectArray();
                const FbxLayerElementArrayTemplate<int>& normal_index_array = flayer_normal->GetIndexArray();
                
                if (normal_mapping_mode == FbxLayerElement::eByPolygonVertex){
                    if (normal_reference_mode == FbxLayerElement::eDirect){
                        for (int i = 0;i < normal_direct_array.GetCount();i++){
                            vec4 normal;
                            normal[0] = normal_direct_array[i][0];
                            normal[1] = normal_direct_array[i][1];
                            normal[2] = normal_direct_array[i][2];
                            normal[3] = 0;
                            mesh.normal[i] = transform_normal*normal;
                        }
                    }else if (normal_reference_mode == FbxLayerElement::eIndexToDirect){
                        for (int i = 0;i < normal_index_array.GetCount();i++){
                            vec4 normal;
                            normal[0] = normal_direct_array[normal_index_array[i]][0];
                            normal[1] = normal_direct_array[normal_index_array[i]][1];
                            normal[2] = normal_direct_array[normal_index_array[i]][2];
                            normal[3] = 0;
                            mesh.normal[i] = transform_normal*normal;
                        }
                    }else{
                        //nothing
                    }
                }else{
                    //nothing
                    //normal_mapping_mode == FbxLayerElement::eByControlPoint
                    //normal_mapping_mode == FbxLayerElement::eByPolygon
                    //normal_mapping_mode == FbxLayerElement::eByEdge
                    //normal_mapping_mode == FbxLayerElement::eAllSame
                }
            }
            
            //material
            FbxLayerElementMaterial* flayer_material = flayer->GetMaterials();
            if (flayer_material != NULL){
                const FbxLayerElementArrayTemplate<int>& material_index_array = flayer_material->GetIndexArray();
                FbxSurfaceMaterial* fmaterial = m_fmesh_nodes[k]->GetMaterial(material_index_array[0]);
                FbxProperty property = fmaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
                if (property.GetSrcObjectCount<FbxFileTexture>() != 0){
                    FbxFileTexture* texture = property.GetSrcObject<FbxFileTexture>(0);
                    std::string filename = std::string(FbxPathUtils::GetFileName(texture->GetFileName()).Buffer());
                    mesh.texture = filename;
                }
            }
            
            //fskin
            FbxSkin* fskin = (FbxSkin*)fmesh->GetDeformer(0);
            if (fskin != NULL){
                //bone_index,bone_weight per vertex
                std::vector<int> bi_per_vert(4*xyz_count);
                std::vector<FLOAT> bw_per_vert(4*xyz_count);
                for (size_t i = 0;i < 4*xyz_count;i++){
                    bi_per_vert[i] = -1;
                    bw_per_vert[i] = 0;
                }
                
                int fcluster_count = fskin->GetClusterCount();
                for (int i = 0;i < fcluster_count;i++){
                    //fcluster
                    FbxCluster* fcluster = fskin->GetCluster(i);
                    
                    //fskeleton node
                    FbxNode* fskeleton_node = fcluster->GetLink();
                    
                    //bone index
                    int bone_index = GetBoneIndex(fskeleton_node);
                    
                    //vertex indices and vertex weights
                    int vertex_count = fcluster->GetControlPointIndicesCount();
                    int* vertex_indices = fcluster->GetControlPointIndices();
                    double* vertex_weights = fcluster->GetControlPointWeights();
                    for (int j = 0;j < vertex_count;j++){
                        int offset = 4*vertex_indices[j];
                        if (bi_per_vert[offset+0] == -1){
                            bi_per_vert[offset+0] = bone_index;
                            bw_per_vert[offset+0] = vertex_weights[j];
                        }else if (bi_per_vert[offset+1] == -1){
                            bi_per_vert[offset+1] = bone_index;
                            bw_per_vert[offset+1] = vertex_weights[j];
                        }else if (bi_per_vert[offset+2] == -1){
                            bi_per_vert[offset+2] = bone_index;
                            bw_per_vert[offset+2] = vertex_weights[j];
                        }else if (bi_per_vert[offset+3] == -1){
                            bi_per_vert[offset+3] = bone_index;
                            bw_per_vert[offset+3] = vertex_weights[j];
                        }else{
                            //nothing
                        }
                    }
                }
                
                //bone_index,bone_weight per polygon vertex
                for (int i = 0;i < polygon_vertex_count;i++){
                    mesh.bone_index[4*i+0] = bi_per_vert[4*polygon_vertices[i]+0];
                    mesh.bone_index[4*i+1] = bi_per_vert[4*polygon_vertices[i]+1];
                    mesh.bone_index[4*i+2] = bi_per_vert[4*polygon_vertices[i]+2];
                    mesh.bone_index[4*i+3] = bi_per_vert[4*polygon_vertices[i]+3];
                    mesh.bone_weight[4*i+0] = bw_per_vert[4*polygon_vertices[i]+0];
                    mesh.bone_weight[4*i+1] = bw_per_vert[4*polygon_vertices[i]+1];
                    mesh.bone_weight[4*i+2] = bw_per_vert[4*polygon_vertices[i]+2];
                    mesh.bone_weight[4*i+3] = bw_per_vert[4*polygon_vertices[i]+3];
                }
                
            }
            
        }
        
    }
}


//How to get bone bind pose matrix
//1 FbxCluster::GetTransformLinkMatrix(FbxAMatrix&)
//2 FbxNode::EvaluateGlobalTransform() of skeleton type node

void FBXMeshLoader::LoadSkeleton(){
    m_skeleton.bbp_i.resize(m_fskeleton_nodes.size());
    m_skeleton.bbp_iti.resize(m_fskeleton_nodes.size());
    for (size_t i = 0;i < m_fmeshes.size();i++){
        //fmesh
        FbxMesh* fmesh = m_fmeshes[i];
        
        //fskin
        FbxSkin* fskin = (FbxSkin*)fmesh->GetDeformer(0);
        if (fskin != NULL){
            int fcluster_count = fskin->GetClusterCount();
            for (int j = 0;j < fcluster_count;j++){
                //fcluster
                FbxCluster* fcluster = fskin->GetCluster(j);
                
                //fskeleton node
                FbxNode* fskeleton_node = fcluster->GetLink();
                
                //bone index
                int bone_index = GetBoneIndex(fskeleton_node);
                
                //bbp_i
                {
                    FbxAMatrix m;
                    const FbxMatrix& bbp_i = (FbxMatrix(fcluster->GetTransformLinkMatrix(m).Transpose())*m_axis_transform).Inverse();
                    const FbxVector4& r0 = bbp_i.GetRow(0);
                    const FbxVector4& r1 = bbp_i.GetRow(1);
                    const FbxVector4& r2 = bbp_i.GetRow(2);
                    const FbxVector4& r3 = bbp_i.GetRow(3);
                    m_skeleton.bbp_i[bone_index].SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
                    m_skeleton.bbp_i[bone_index].SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
                    m_skeleton.bbp_i[bone_index].SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
                    m_skeleton.bbp_i[bone_index].SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
                }
                
                //bbp_iti
                {
                    FbxAMatrix m;
                    const FbxMatrix& bbp_iti = (FbxMatrix(fcluster->GetTransformLinkMatrix(m).Transpose())*m_axis_transform).Inverse().Transpose().Inverse();
                    const FbxVector4& r0 = bbp_iti.GetRow(0);
                    const FbxVector4& r1 = bbp_iti.GetRow(1);
                    const FbxVector4& r2 = bbp_iti.GetRow(2);
                    const FbxVector4& r3 = bbp_iti.GetRow(3);
                    m_skeleton.bbp_iti[bone_index].SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
                    m_skeleton.bbp_iti[bone_index].SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
                    m_skeleton.bbp_iti[bone_index].SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
                    m_skeleton.bbp_iti[bone_index].SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
                }
            }
        }
    }
}









FBXAnimationLoader::FBXAnimationLoader(const std::string& path){
    //initialize fbxsdk
    FbxManager* fmanager = FbxManager::Create();
    FbxScene* fscene = FbxScene::Create(fmanager,"");
    FbxImporter* fimporter = FbxImporter::Create(fmanager,"");
    if (!fimporter->Initialize(path.c_str())){
        std::cout << "failed to read fbx file:" << path << "\n";
        std::terminate();
    }
    fimporter->Import(fscene);
    
    //traverse node tree
    TraverseNodeTree(fscene->GetRootNode());
    
    //create axis transform
    m_axis_transform = create_axis_transform(fscene);
    
    //check fskeleton count
    if (m_fskeleton_nodes.size() == 0){
        std::cout << "failed to load fbx animation file:" << path << "\n";
        std::cout << "fbx skeleton count must be nonzero" << "\n";
        std::terminate();
    }
    
    //load animation
    LoadAnimation(fscene);
    
    //terminate fbxsdk
    fmanager->Destroy();
    
    //PrintData();
}

const FBXAnimationLoader::Animation& FBXAnimationLoader::GetAnimation() const{
    return m_animation;
}

void FBXAnimationLoader::PrintData() const{
    //each frame,each bone
    int bone_count = m_fskeleton_nodes.size();
    for (int i = 0;i < m_animation.bp.size();i++){
        if (i%bone_count == 0){
            std::cout << "frame:" << i/bone_count << "\n";
        }
        
        //bp
        {
            const mat4& m = m_animation.bp[i];
            const vec4& r0 = m.GetRow(0);
            const vec4& r1 = m.GetRow(1);
            const vec4& r2 = m.GetRow(2);
            const vec4& r3 = m.GetRow(3);
            std::cout << "bp:" << i%bone_count << "\n";
            std::cout << "    " << r0[0] << " " << r0[1] << " " << r0[2] << " " << r0[3] << "\n";
            std::cout << "    " << r1[0] << " " << r1[1] << " " << r1[2] << " " << r1[3] << "\n";
            std::cout << "    " << r2[0] << " " << r2[1] << " " << r2[2] << " " << r2[3] << "\n";
            std::cout << "    " << r3[0] << " " << r3[1] << " " << r3[2] << " " << r3[3] << "\n";
            std::cout << "\n";
        }
        
        //bp_it
        {
            const mat4& m = m_animation.bp_it[i];
            const vec4& r0 = m.GetRow(0);
            const vec4& r1 = m.GetRow(1);
            const vec4& r2 = m.GetRow(2);
            const vec4& r3 = m.GetRow(3);
            std::cout << "bp_it:" << i%bone_count << "\n";
            std::cout << "    " << r0[0] << " " << r0[1] << " " << r0[2] << " " << r0[3] << "\n";
            std::cout << "    " << r1[0] << " " << r1[1] << " " << r1[2] << " " << r1[3] << "\n";
            std::cout << "    " << r2[0] << " " << r2[1] << " " << r2[2] << " " << r2[3] << "\n";
            std::cout << "    " << r3[0] << " " << r3[1] << " " << r3[2] << " " << r3[3] << "\n";
            std::cout << "\n";
        }
        
    }
    
    std::cout << "\n\n";
}

void FBXAnimationLoader::TraverseNodeTree(FbxNode* fnode){
    //null check
    if (fnode == NULL){
        std::cout << "fbx node is nullptr" << "\n";
        std::terminate();
    }

    //node attribute
    int fattribute_count = fnode->GetNodeAttributeCount();
    for (int i = 0;i < fattribute_count;i++){
        FbxNodeAttribute* fattribute = fnode->GetNodeAttributeByIndex(i);
        FbxNodeAttribute::EType ftype = fattribute->GetAttributeType();
        //fskeleton
        if (ftype == FbxNodeAttribute::eSkeleton){
            m_fskeleton_nodes.push_back(fnode);
        }
    }

    //traverse child node
    int child_count = fnode->GetChildCount();
    for (int i = 0;i < child_count;i++){
        TraverseNodeTree(fnode->GetChild(i));
    }
}


void FBXAnimationLoader::LoadAnimation(FbxScene* fscene){
    //animation info
    FbxAnimStack* fanim_stack = fscene->GetCurrentAnimationStack();
    if (fanim_stack == NULL){
        return;
    }
    FbxTime frame_time = FbxTime::GetOneFrameValue(FbxTime::eFrames30);
    FbxTime start_time = fanim_stack->LocalStart.Get();
    FbxTime stop_time = fanim_stack->LocalStop.Get();
    
    //load bone poses of each frame
    size_t frame_count = 0;
    FbxTime current_time = start_time;
    while(1){
        if (current_time <= stop_time){
            //bp
            for (int i = 0;i < m_fskeleton_nodes.size();i++){
                const FbxMatrix& m = FbxMatrix(m_fskeleton_nodes[i]->EvaluateGlobalTransform(current_time).Transpose())*m_axis_transform;
                const FbxVector4& r0 = m.GetRow(0);
                const FbxVector4& r1 = m.GetRow(1);
                const FbxVector4& r2 = m.GetRow(2);
                const FbxVector4& r3 = m.GetRow(3);
                
                mat4 bp;
                bp.SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
                bp.SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
                bp.SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
                bp.SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
                m_animation.bp.push_back(bp);
            }
            
            //bp_it
            for (int i = 0;i < m_fskeleton_nodes.size();i++){
                const FbxMatrix& m = (FbxMatrix(m_fskeleton_nodes[i]->EvaluateGlobalTransform(current_time).Transpose())*m_axis_transform).Inverse().Transpose();
                const FbxVector4& r0 = m.GetRow(0);
                const FbxVector4& r1 = m.GetRow(1);
                const FbxVector4& r2 = m.GetRow(2);
                const FbxVector4& r3 = m.GetRow(3);
                
                mat4 bp_it;
                bp_it.SetRow(0,vec4({(FLOAT)r0[0],(FLOAT)r0[1],(FLOAT)r0[2],(FLOAT)r0[3]}));
                bp_it.SetRow(1,vec4({(FLOAT)r1[0],(FLOAT)r1[1],(FLOAT)r1[2],(FLOAT)r1[3]}));
                bp_it.SetRow(2,vec4({(FLOAT)r2[0],(FLOAT)r2[1],(FLOAT)r2[2],(FLOAT)r2[3]}));
                bp_it.SetRow(3,vec4({(FLOAT)r3[0],(FLOAT)r3[1],(FLOAT)r3[2],(FLOAT)r3[3]}));
                m_animation.bp_it.push_back(bp_it);
            }
            
            //update frame count
            frame_count++;
            
            //update current time
            current_time += frame_time;
        }else{
            break;
        }
    }
    
    //duration,frame count
    m_animation.duration = frame_time.GetSecondDouble()*(frame_count-1);
    m_animation.frame_count = frame_count;
    
}
