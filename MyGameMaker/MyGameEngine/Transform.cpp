#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>


void Transform::translate(const vec3& v) {
    _mat = glm::translate(_mat, v);
}

const vec3& Transform::GetRotation() const
{
    // Calculate the rotation matrix from the _left, _up, and _fwd vectors
    mat4 rotationMatrix = mat4(1.0);
    rotationMatrix[0] = vec4(_left, 0.0);
    rotationMatrix[1] = vec4(_up, 0.0);
    rotationMatrix[2] = vec4(_fwd, 0.0);

    // Extract Euler angles from the rotation matrix
    vec3 eulerAngles = glm::eulerAngles(glm::quat_cast(rotationMatrix));

    // Convert radians to degrees
    eulerAngles = glm::degrees(eulerAngles);

    return eulerAngles;
}
void Transform::setScale(const glm::vec3& scale) {
    glm::vec3 currentScale = extractScale(_mat);
    glm::vec3 newScale = scale;
    _mat[0] = glm::vec4(newScale.x, _mat[0][1], _mat[0][2], _mat[0][3]); // Scale x
    _mat[1] = glm::vec4(_mat[1][0], newScale.y, _mat[1][2], _mat[1][3]); // Scale y
    _mat[2] = glm::vec4(_mat[2][0], _mat[2][1], newScale.z, _mat[2][3]); // Scale z
}
void Transform::setPos(float x, float y, float z) {
    // Actualiza la posición del transform
    _pos = glm::vec3(x, y, z);

    // Recalcula la matriz de transformación usando la nueva posición
    //updateRotationMatrix();
}

void Transform::setRotation(float newPitch, float newYaw, float newRoll) {
   /* pitch = newPitch;
    yaw = newYaw;
    roll = newRoll;*/
    rotateWithVector( glm::vec3 (newPitch,newYaw,newRoll));
}

void Transform::rotate(double rads, const vec3& v) {
    _mat = glm::rotate(_mat, rads, v);
}
void Transform::rotateWithVector(const vec3& inputVector) {
    // Calcula el ángulo como la magnitud del vector
    double angle = glm::length(inputVector);

    // Si el ángulo es prácticamente cero, no hay rotación
    if (angle == 0.0f) return;

    // Normaliza el vector para obtener el eje de rotación
    vec3 axis = glm::normalize(inputVector);

    // Aplica la rotación a la matriz actual
    _mat = glm::rotate(_mat, angle, axis);
}
double Transform::getYaw() const {
    return yaw;
}

double Transform::getPitch() const {
    return pitch;
}

double Transform::getRoll() const {
    return roll;
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
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);

    float yaw = atan2(forward.x, forward.z);

    float pitch = atan2(-forward.y, sqrt(forward.x * forward.x + forward.z * forward.z));

    float roll = atan2(left.y, up.y);

    return glm::vec3(glm::degrees(pitch), glm::degrees(yaw), glm::degrees(roll));
}
glm::vec3 Transform::extractScale(const glm::mat4& mat) const {
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);

    float scaleX = glm::length(left);
    float scaleY = glm::length(up);
    float scaleZ = glm::length(forward);

    return glm::vec3(scaleX, scaleY, scaleZ);
}

const glm::mat4& Transform::getMatrix() const {
    return _mat;
}
