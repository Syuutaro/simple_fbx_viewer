#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "define.hpp"
#include "matrix.hpp"


//cylindrical coordinate system
//position = [radius,theta,height]

class Camera{
private:
    FLOAT m_near;
    FLOAT m_far;
    FLOAT m_fovy;
    FLOAT m_aspect;
    
    vec3 m_position;//cylindrical coordinate
    vec3 m_target;  //orthogonal coordinate
public:
    void SetPerspectiveParameters(FLOAT near,FLOAT far,FLOAT fovy,FLOAT aspect);
    void SetPosition(const vec3& position);
    void SetTargetY(FLOAT y);
    void MoveRadius(FLOAT delta);
    void MoveTheta(FLOAT delta);
    void MoveHeight(FLOAT delta);
    void MoveTargetY(FLOAT delta);
    const vec3& GetPosition() const;
    mat4 GetViewMatrix() const;
    mat4 GetPerspectiveMatrix() const;
};




#endif // CAMERA_HPP
