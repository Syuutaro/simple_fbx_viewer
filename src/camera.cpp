#include "camera.hpp"


void Camera::SetPerspectiveParameters(FLOAT near,FLOAT far,FLOAT fovy,FLOAT aspect){
    m_near = near;
    m_far = far;
    m_fovy = fovy;
    m_aspect = aspect;
}

void Camera::SetPosition(const vec3& position){
    m_position = position;
}

void Camera::SetTargetY(FLOAT y){
    m_target = vec3({0,y,0});
}

void Camera::MoveRadius(FLOAT delta){
    m_position[0] += delta;
    if (m_position[0] < 0){
        m_position[0] = 0;
    }
}

void Camera::MoveTheta(FLOAT delta){
    m_position[1] += delta;
}

void Camera::MoveHeight(FLOAT delta){
    m_position[2] += delta;
    m_target[1] += delta;
}

void Camera::MoveTargetY(FLOAT delta){
    m_target[1] += delta;
}

const vec3& Camera::GetPosition() const{
    return m_position;
}

mat4 Camera::GetViewMatrix() const{
    //camera position in world coordinate
    vec3 p;
    p[0] = m_position[0]*std::sin(m_position[1]);
    p[1] = m_position[2];
    p[2] = m_position[0]*std::cos(m_position[1]);

    //orthonormal basis of camera
    vec3 ax,ay,az;
    az = normalize(p-m_target);
    if (az == vec3({0,1,0})){
        ax = vec3({0,0,1});
        ay = vec3({1,0,0});
    }else if (az == vec3({0,-1,0})){
        ax = vec3({0,0,1});
        ay = vec3({-1,0,0});
    }else{
        ax = normalize(cross(vec3({0,1,0}),az));
        ay = cross(az,ax);
    }

    //attitude matrix
    mat3 A;
    A.SetColumn(0,ax);
    A.SetColumn(1,ay);
    A.SetColumn(2,az);
    const mat3& At = A.Transpose();

    //view matrix
    mat4 view;
    view.SetColumn(0,vec4(At.GetColumn(0),0));
    view.SetColumn(1,vec4(At.GetColumn(1),0));
    view.SetColumn(2,vec4(At.GetColumn(2),0));
    view.SetColumn(3,vec4(At*p*(-1),1));
    return view;
}

mat4 Camera::GetPerspectiveMatrix() const{
    //f
    FLOAT f = 1/std::tan(0.5*m_fovy*PI/360);

    //perspective matrix
    mat4 perspective;
    perspective.SetColumn(0,vec4({f/m_aspect,0,0,0}));
    perspective.SetColumn(1,vec4({0,f,0,0}));
    perspective.SetColumn(2,vec4({0,0,-(m_far+m_near)/(m_far-m_near),-1}));
    perspective.SetColumn(3,vec4({0,0,-2*m_far*m_near/(m_far-m_near),0}));
    return perspective;
}
