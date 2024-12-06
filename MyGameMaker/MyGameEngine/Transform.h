#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "types.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Transform {
private:
    union {
        mat4 _mat = mat4(1.0);
        struct {
            vec3 _left; mat4::value_type _left_w;
            vec3 _up; mat4::value_type _up_w;
            vec3 _fwd; mat4::value_type _fwd_w;
            vec3 _pos; mat4::value_type _pos_w;
        };
    };

    // Variables para los �ngulos de rotaci�n
    double yaw = 0.0;
    double pitch = 0.0;
    double roll = 0.0;

  
public:
    const auto& mat() const { return _mat; }
    const auto& left() const { return _left; }
    const auto& up() const { return _up; }
    const auto& fwd() const { return _fwd; }
    const auto& pos() const { return _pos; }
    auto& pos() { return _pos; }

    const auto* data() const { return &_mat[0][0]; }

    Transform() = default;
    Transform(const mat4& mat) : _mat(mat) {}

    
   
    const vec3& GetRotation() const;
    void SetRotation(const vec3& eulerAngles);
    void translate(const vec3& v);
	void setPos(float x, float y, float z);
    double getYaw() const;
    double getPitch() const;
    double getRoll() const;
    void setRotation(float newPitch, float newYaw, float newRoll);
    void rotateYaw(double radians);
    void rotatePitch(double radians);
    void rotateRoll(double radians);
    void rotate(double rads, const vec3& v);
    void updateRotationMatrix();
    void lookAt(const vec3& target);
    void alignCamera(const vec3& worldUp = vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 extractEulerAngles(const glm::mat4& mat);
	glm::vec3 extractScale(const glm::mat4& mat);

    Transform operator*(const mat4& other) { return Transform(_mat * other); }
    Transform operator*(const Transform& other) { return Transform(_mat * other._mat); }
};

