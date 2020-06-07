#include "library.hpp"
#include "resource.hpp"
#include "camera.hpp"
#include "json.hpp"

#include <OpenGL/gl3.h>
#include <SDL2/SDL.h>

//このクラスは使い捨て
class Square{
private:
    GLuint m_vao;
    GLuint m_ibo;
    GLuint m_vbo_xyz;
    GLuint m_vbo_uv;
    GLuint m_vbo_normal;
public:
    Square();
    ~Square();
    void Draw(const Camera& camera,const Shader& shader) const;
};

Square::Square(){
    //data
    const GLfloat r = 100;
    const GLfloat xyz[] = {
        r,0,r,
        r,0,-r,
        -r,0,-r,
        -r,0,r
    };
    
    const GLfloat uv[] = {
        1,1,
        0,1,
        0,0,
        1,0
    };
    
    const GLfloat normal[] = {
        0,1,0,
        0,1,0,
        0,1,0,
        0,1,0
    };
    
    const GLuint polygon_vertex[] = {
        0,1,2,
        0,2,3
    };
    
    //vao
    glGenVertexArrays(1,&m_vao);
    glBindVertexArray(m_vao);

    //xyz
    glGenBuffers(1,&m_vbo_xyz);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_xyz);
    glBufferData(GL_ARRAY_BUFFER,sizeof(xyz),xyz,GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
           
    //uv
    glGenBuffers(1,&m_vbo_uv);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_uv);
    glBufferData(GL_ARRAY_BUFFER,sizeof(uv),uv,GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);
    
    //normal
    glGenBuffers(1,&m_vbo_normal);
    glBindBuffer(GL_ARRAY_BUFFER,m_vbo_normal);
    glBufferData(GL_ARRAY_BUFFER,sizeof(normal),normal,GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
           
    //polygon vertex
    glGenBuffers(1,&m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(polygon_vertex),polygon_vertex,GL_STATIC_DRAW);
    
    //unbind vao
    glBindVertexArray(0);
}

Square::~Square(){
    glDeleteVertexArrays(1,&m_vao);
    glDeleteBuffers(1,&m_ibo);
    glDeleteBuffers(1,&m_vbo_xyz);
    glDeleteBuffers(1,&m_vbo_uv);
    glDeleteBuffers(1,&m_vbo_normal);
}

void Square::Draw(const Camera& camera,const Shader& shader) const{
    //activate shader pass
    shader.Bind();
    
    //bind camera
    const mat4& view = camera.GetViewMatrix();
    const mat4& perspective = camera.GetPerspectiveMatrix();
    glUniformMatrix4fv(shader.GetUniformLocation("view"),1,GL_FALSE,(const GLfloat*)&view);
    glUniformMatrix4fv(shader.GetUniformLocation("perspective"),1,GL_FALSE,(const GLfloat*)&perspective);
    
    //draw
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
    
    //deactivate shader pass
    shader.UnBind();
}






class StopWatch{
private:
    uint64_t m_start_time;
public:
    StopWatch();
    void Start();
    void Reset();
    uint64_t GetElapsedTime();
};

StopWatch::StopWatch():m_start_time(0){}
    
void StopWatch::Start(){
    if (m_start_time == 0){
        m_start_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

void StopWatch::Reset(){
    m_start_time = 0;
}

uint64_t StopWatch::GetElapsedTime(){
    if (m_start_time != 0){
        uint64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return t-m_start_time;
    }else{
        return 0;
    }
}







class Scene{
private:
    bool m_is_skeletal;
    const Shader* m_mesh_shader;
    const Shader* m_square_shader;
    const Mesh* m_mesh;
    const Square* m_square;
    AnimationController* m_animation_controller;
    Camera* m_camera;
public:
    Scene(const std::string& asset_dir_path);
    ~Scene();
    void HandleEvent(const SDL_Event& event);
    void Update(double dt);
    void Render();
};

Scene::Scene(const std::string& asset_dir_path){
    //create resource manager
    ResourceManager::CreateInstance();
       
    //load scene file
    JSON json(asset_dir_path+"/scene.json");
       
    //load shader
    const std::string& color = json["mesh"]["color"].GetString();
    m_is_skeletal = json["mesh"]["is_skeletal"].GetBoolean();
    if (m_is_skeletal){
        if (color == "texture"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/skeletal.vert","./shader/texture.frag");
        }else if (color == "uv"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/skeletal.vert","./shader/uv.frag");
        }else if (color == "normal"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/skeletal.vert","./shader/normal.frag");
        }
    }else{
        if (color == "texture"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/non_skeletal.vert","./shader/texture.frag");
        }else if (color == "uv"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/non_skeletal.vert","./shader/uv.frag");
        }else if (color == "normal"){
            m_mesh_shader = ResourceManager::GetInstance()->LoadShader("./shader/non_skeletal.vert","./shader/normal.frag");
        }
    }
    m_square_shader = ResourceManager::GetInstance()->LoadShader("./shader/square.vert","./shader/square.frag");
    
    //load mesh
    const std::string& mesh_file_path = asset_dir_path+"/mesh/"+json["mesh"]["filename"].GetString();
    m_mesh = ResourceManager::GetInstance()->LoadMesh(asset_dir_path,mesh_file_path,m_mesh_shader,m_is_skeletal);
    
    //create square
    m_square = new Square();
    
    //create animation controller
    m_animation_controller = nullptr;
    if (m_is_skeletal){
        //create animation state
        std::map<std::string,AnimationController::State> states;
        size_t state_count = json["animation_controller"]["states"].GetElementCount();
        for (size_t i = 0;i < state_count;i++){
            //state node
            const JSON::Node& state_node = json["animation_controller"]["states"][i];
            
            //state
            AnimationController::State state;
            
            //state id
            const std::string& id = state_node["id"].GetString();
            
            //load animation
            const std::string& path = asset_dir_path+"/animation/"+state_node["animation"].GetString();
            state.animation = ResourceManager::GetInstance()->LoadAnimation(path);
            
            //auto transition
            if (state_node["auto_transition"].GetMemberCount() != 0){
                state.auto_transition_exist = true;
                state.auto_transition.duration = state_node["auto_transition"]["duration"].GetNumber();
                state.auto_transition.offset = state_node["auto_transition"]["offset"].GetNumber();
                state.auto_transition.dst_state_id = state_node["auto_transition"]["dst_state_id"].GetString();
            }else{
                state.auto_transition_exist = false;
            }
            
            //user transition
            size_t transition_count = state_node["user_transitions"].GetElementCount();
            if (transition_count != 0){
                state.user_transition_exist = true;
                for (size_t j = 0;j < transition_count;j++){
                    //trigger
                    const std::string& trigger = state_node["user_transitions"][j]["trigger"].GetString();
                    
                    //transition
                    AnimationController::Transition transition;
                    transition.is_immediate = state_node["user_transitions"][j]["is_immediate"].GetBoolean();
                    transition.duration = state_node["user_transitions"][j]["duration"].GetNumber();
                    transition.offset = state_node["user_transitions"][j]["offset"].GetNumber();
                    transition.dst_state_id = state_node["user_transitions"][j]["dst_state_id"].GetString();
                    
                    //register transition
                    state.user_transitions[trigger] = transition;
                }
            }else{
                state.user_transition_exist = false;
            }
            
            //register state
            states[id] = state;
        }
        
        //entry state id
        const std::string& entry_state_id = json["animation_controller"]["entry_state_id"].GetString();
        
        //create animation controller
        m_animation_controller = new AnimationController(m_mesh->GetSkeleton(),states,entry_state_id);
    }
    
    //create camera
    m_camera = new Camera();
    m_camera->SetPerspectiveParameters(1,1000,45,1200.0/800);
    m_camera->SetPosition(vec3({5,0,0.5}));
    m_camera->SetTargetY(0.5);
}


Scene::~Scene(){
    delete m_animation_controller;
    delete m_camera;
    ResourceManager::GetInstance()->UnLoadResource();
    ResourceManager::DeleteInstance();
}


void Scene::HandleEvent(const SDL_Event& event){
    static bool is_mouse_down = false;
    if (event.type == SDL_MOUSEMOTION && is_mouse_down){
        m_camera->MoveTheta(-0.01*event.motion.xrel);
        m_camera->MoveHeight(0.01*event.motion.yrel);
    }else if (event.type == SDL_MOUSEWHEEL){
        m_camera->MoveRadius(0.5*event.wheel.y);
    }else if (event.type == SDL_MOUSEBUTTONUP){
        is_mouse_down = false;
    }else if (event.type == SDL_MOUSEBUTTONDOWN){
        is_mouse_down = true;
    }else if (event.type == SDL_KEYDOWN){
        //camera
        if (event.key.keysym.sym == SDLK_UP){
            m_camera->MoveTargetY(0.05);
        }else if (event.key.keysym.sym == SDLK_DOWN){
            m_camera->MoveTargetY(-0.05);
        }
        
        //animation controller
        if (m_is_skeletal){
            std::string key;
            key = event.key.keysym.sym;
            m_animation_controller->DoUserTransition(key);
        }
    }
}


void Scene::Update(double dt){
    //update animation
    if (m_is_skeletal){
        m_animation_controller->UpdateAnimation(dt);
    }
}


void Scene::Render(){
    //render square
    m_square->Draw(*m_camera,*m_square_shader);
    
    
    //render mesh
    //camera parameter
    const mat4& world = m_mesh->GetNormalizingTransform();
    const mat4& view = m_camera->GetViewMatrix();
    const mat4& perspective = m_camera->GetPerspectiveMatrix();
    
    //skeleton
    const Skeleton* skeleton = m_mesh->GetSkeleton();
    
    //draw sub meshes
    size_t sub_mesh_count = m_mesh->GetSubMeshCount();
    for (size_t i = 0;i < sub_mesh_count;i++){
        //sub mesh
        const SubMesh* sub_mesh = m_mesh->GetSubMesh(i);
        
        //material
        const Material* material = sub_mesh->GetMaterial();
        
        //bind shader
        const Shader* shader = material->GetShader();
        shader->Bind();
        
        //bind skeleton
        if (m_is_skeletal){
            skeleton->Bind(shader->GetUniformLocation("bbp_i"),
                           shader->GetUniformLocation("bbp_iti"),
                           shader->GetUniformLocation("bp"),
                           shader->GetUniformLocation("bp_it"),
                           0);
        }
        
        //bind material
        material->Bind(4);
        
        //bind camera
        glUniformMatrix4fv(shader->GetUniformLocation("world"),1,GL_FALSE,(const GLfloat*)&world);
        glUniformMatrix4fv(shader->GetUniformLocation("view"),1,GL_FALSE,(const GLfloat*)&view);
        glUniformMatrix4fv(shader->GetUniformLocation("perspective"),1,GL_FALSE,(const GLfloat*)&perspective);
        
        //draw sub mesh
        sub_mesh->Draw();
        
        //unbind shader
        shader->UnBind();
    }
    
}







int main(){
    //input asset directory path
    std::cout << "Input absolute path to asset directory" << "\n";
    std::string asset_dir_path;
    std::cin >> asset_dir_path;
    
    //initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "failed to initialize SDL" << "\n";
        std::terminate();
    }
    
    //set opengl attribute
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,2);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);
    
    //create window
    SDL_Window* window = SDL_CreateWindow("window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1200,800,
                                          SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
    if (window == NULL){
        std::cout << "failed to create window" << "\n";
        std::terminate();
    }
    
    //create opengl context
    SDL_GLContext glcontext = SDL_GL_CreateContext(window);
    if (glcontext == NULL){
        std::cout << "failed to create opengl context" << "\n";
        std::terminate();
    }
    
    //rendering setting
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1);
    glClearColor(0,0,0,1);
    
    //create scene
    Scene scene(asset_dir_path);
    
    //event roop
    StopWatch sw;
    double frame_rate = 60;
    SDL_Event event;
    while (1){
        //start stopwtch
        sw.Reset();
        sw.Start();
        
        //handle event
        if (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                break;
            }else{
                scene.HandleEvent(event);
            }
        }

        //clear
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        
        //update scene
        scene.Update(1.0/frame_rate);
        
        //render scene
        scene.Render();
        
        //update window
        SDL_GL_SwapWindow(window);
        
        //stop stopwatch
        uint64_t elapsed_time = sw.GetElapsedTime();
        if (elapsed_time < 1000.0/frame_rate){
            SDL_Delay(1000.0/frame_rate-elapsed_time);
        }
    }
    
    //terminate SDL
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

