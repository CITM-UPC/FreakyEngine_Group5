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
void Transform::alignCamera(const vec3& worldUp) {

    vec3 fwd = glm::normalize(_fwd);
    vec3 right = glm::normalize(glm::cross(worldUp, fwd));
    vec3 up = glm::cross(fwd, right);


    _left = right;
    _up = up;
    _fwd = fwd;
    _pos = _pos;
    _mat = mat4(vec4(_left, 0.0f), vec4(_up, 0.0f), vec4(_fwd, 0.0f), vec4(_pos, 1.0f));
}

void Transform::lookAt(const vec3& target) {
    _fwd = glm::normalize(  _pos- target);
    _left = glm::normalize(glm::cross(vec3(0, 1, 0), _fwd));
    _up = glm::cross(_fwd, _left);

    _mat[0] = vec4(_left, 0.0);
    _mat[1] = vec4(_up, 0.0);
    _mat[2] = vec4(-_fwd, 0.0);
    _mat[3] = vec4(_pos, 1.0);
}
glm::vec3 Transform::extractEulerAngles(const glm::mat4& mat) {
    // Extrae los vectores de la matriz de rotaci�n
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);

    // Calcula yaw (rotaci�n alrededor del eje Y)
    float yaw = atan2(forward.x, forward.z);

    // Calcula pitch (rotaci�n alrededor del eje X)
    float pitch = atan2(-forward.y, sqrt(forward.x * forward.x + forward.z * forward.z));

    // Calcula roll (rotaci�n alrededor del eje Z)
    float roll = atan2(left.y, up.y);

    // Devuelve los �ngulos en grados
    return glm::vec3(glm::degrees(pitch), glm::degrees(yaw), glm::degrees(roll));
}
// Funci�n para extraer la escala de una matriz de transformaci�n
glm::vec3 Transform::extractScale(const glm::mat4& mat) {
    // Extraer los vectores de orientaci�n de la matriz de transformaci�n
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);

    // Calcular la escala como la longitud de cada vector de orientaci�n
    float scaleX = glm::length(left);
    float scaleY = glm::length(up);
    float scaleZ = glm::length(forward);

    // Devolver la escala como un vector
    return glm::vec3(scaleX, scaleY, scaleZ);
}



