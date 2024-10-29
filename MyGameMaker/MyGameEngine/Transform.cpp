#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


void Transform::translate(const vec3& v) {
    _mat = glm::translate(_mat, v);
}

void Transform::rotateYaw(double radians) {
    yaw += radians; // Acumular el yaw
    updateRotationMatrix(); // Actualiza la matriz de rotaci�n
}

void Transform::rotatePitch(double radians) {
    pitch += radians; // Acumular el pitch
    updateRotationMatrix(); // Actualiza la matriz de rotaci�n
}

void Transform::rotateRoll(double radians) {
    roll += radians; // Acumular el roll
    updateRotationMatrix(); // Actualiza la matriz de rotaci�n
}
void Transform::rotate(double rads, const vec3& v) {
    _mat = glm::rotate(_mat, rads, v);
}
void Transform::updateRotationMatrix() {
    // Calcula los cosenos y senos de los �ngulos
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);

    // Matriz de rotaci�n para Yaw
    mat4 yawMatrix = {
        cosYaw, 0, sinYaw, 0,
        0, 1, 0, 0,
        -sinYaw, 0, cosYaw, 0,
        0, 0, 0, 1
    };

    // Matriz de rotaci�n para Pitch
    mat4 pitchMatrix = {
        1, 0, 0, 0,
        0, cosPitch, -sinPitch, 0,
        0, sinPitch, cosPitch, 0,
        0, 0, 0, 1
    };

    // Combinamos las matrices de rotaci�n
    _mat = pitchMatrix * yawMatrix * _mat; // Aplicamos primero yaw y luego pitch
}


