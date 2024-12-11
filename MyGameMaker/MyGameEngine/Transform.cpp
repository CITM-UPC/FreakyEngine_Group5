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
    vec3 currentPos = _pos;
    glm::vec3 newScale = scale;

    glm::vec3 left = glm::normalize(glm::vec3(_mat[0]));
    glm::vec3 up = glm::normalize(glm::vec3(_mat[1]));
    glm::vec3 forward = glm::normalize(glm::vec3(_mat[2]));

    _mat[0] = vec4(left * newScale.x, 0.0f);
    _mat[1] = vec4(up * newScale.y, 0.0f);
    _mat[2] = vec4(forward * newScale.z, 0.0f);
    _mat[3] = vec4(currentPos, 1.0f);
}
void Transform::setPos(float x, float y, float z) {
    // Actualiza la posición del transform
    _pos = glm::vec3(x, y, z);
}
void Transform::updateRotationMatrix() {
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosRoll = cos(roll);
    float sinRoll = sin(roll);

    mat4 yawMatrix = {
        cosYaw, 0, sinYaw, 0,
        0, 1, 0, 0,
        -sinYaw, 0, cosYaw, 0,
        0, 0, 0, 1
    };

    mat4 pitchMatrix = {
        1, 0, 0, 0,
        0, cosPitch, -sinPitch, 0,
        0, sinPitch, cosPitch, 0,
        0, 0, 0, 1
    };

    mat4 rollMatrix = {
        cosRoll, -sinRoll, 0, 0,
        sinRoll, cosRoll, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    vec3 currentScale = extractScale(_mat);
    vec3 currentPos = _pos;

    _mat = rollMatrix * pitchMatrix * yawMatrix;

    // Restore position and scale
    _mat[3] = vec4(currentPos, 1.0f);
    setScale(currentScale);
}
void Transform::setRotation(float newPitch, float newYaw, float newRoll) {
    pitch = newPitch;
    yaw = newYaw;
    roll = newRoll;
    updateRotationMatrix();
}

void Transform::rotate(double rads, const glm::vec3& v) {
    glm::mat4 currentTransform = _mat;
    glm::vec3 currentScale = extractScale(currentTransform);
    glm::vec3 currentPos = _pos;

    // Apply the rotation
    _mat = glm::rotate(currentTransform, static_cast<float>(rads), v);

    // Restore position and scale
    _mat[3] = glm::vec4(currentPos, 1.0f);
    setScale(currentScale);
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
glm::vec3 Transform::extractEulerAngles(const glm::mat4& mat) const{
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
