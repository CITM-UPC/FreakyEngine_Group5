
#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <cmath>

// Aplicar traslación
void Transform::translate(const vec3& v) {
    _mat = glm::translate(_mat, v);
}

// Rotar alrededor del eje Y (Yaw)
void Transform::rotateYaw(double radians) {
    yaw += radians;
    updateRotationMatrix();
}

// Rotar alrededor del eje X (Pitch)
void Transform::rotatePitch(double radians) {
    pitch += radians;
    updateRotationMatrix();
}

// Rotar alrededor del eje Z (Roll)
void Transform::rotateRoll(double radians) {
    roll += radians;
    updateRotationMatrix();
}

// Rotar alrededor de un eje arbitrario
void Transform::rotate(double rads, const vec3& v) {
    _mat = glm::rotate(_mat, rads, v);
}

// Actualizar la matriz de rotación con Yaw, Pitch y Roll
void Transform::updateRotationMatrix() {
    double cosYaw = cos(yaw);
    double sinYaw = sin(yaw);
    double cosPitch = cos(pitch);
    double sinPitch = sin(pitch);
    double cosRoll = cos(roll);
    double sinRoll = sin(roll);

    // Matriz de rotación basada en yaw, pitch y roll
    mat4 yawMatrix = {
        static_cast<float>(cosYaw), 0, static_cast<float>(sinYaw), 0,
        0, 1, 0, 0,
        static_cast<float>(-sinYaw), 0, static_cast<float>(cosYaw), 0,
        0, 0, 0, 1
    };

    mat4 pitchMatrix = {
        1, 0, 0, 0,
        0, static_cast<float>(cosPitch), static_cast<float>(-sinPitch), 0,
        0, static_cast<float>(sinPitch), static_cast<float>(cosPitch), 0,
        0, 0, 0, 1
    };

    mat4 rollMatrix = {
        static_cast<float>(cosRoll), static_cast<float>(-sinRoll), 0, 0,
        static_cast<float>(sinRoll), static_cast<float>(cosRoll), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // Actualizar la matriz de transformación combinando rotaciones
    _mat = yawMatrix * pitchMatrix * rollMatrix * _mat;
}

// Alinear la cámara con un vector Up del mundo
void Transform::alignCamera(const vec3& worldUp) {
    vec3 fwd = glm::normalize(_fwd);
    vec3 right = glm::normalize(glm::cross(worldUp, fwd));
    vec3 up = glm::cross(fwd, right);

    _left = right;
    _up = up;
    _fwd = fwd;

    _mat = mat4(
        vec4(_left, 0.0f),
        vec4(_up, 0.0f),
        vec4(_fwd, 0.0f),
        vec4(_pos, 1.0f)
    );
}

// Configurar orientación usando LookAt
void Transform::lookAt(const vec3& target) {
    _fwd = glm::normalize(_pos - target);
    _left = glm::normalize(glm::cross(vec3(0.0f, 1.0f, 0.0f), _fwd));
    _up = glm::cross(_fwd, _left);

    _mat[0] = vec4(_left, 0.0f);
    _mat[1] = vec4(_up, 0.0f);
    _mat[2] = vec4(-_fwd, 0.0f);
    _mat[3] = vec4(_pos, 1.0f);
}

// Escalar de forma relativa
void Transform::scale(const vec3& s) {
    _mat = glm::scale(_mat, s);
}

// Establecer escala absoluta
void Transform::setScale(const vec3& scale) {
    // Extraer las componentes actuales de la escala
    glm::vec3 currentScale = extractScale(_mat);

    // Calcular la matriz para restablecer la escala actual a 1
    mat4 scaleReset = glm::scale(mat4(1.0f), vec3(1.0f / currentScale.x, 1.0f / currentScale.y, 1.0f / currentScale.z));

    // Aplicar la nueva escala
    _mat = _mat * scaleReset;  // Eliminar la escala previa
    _mat = glm::scale(_mat, scale);  // Aplicar la escala absoluta
}

// Extraer ángulos de Euler de una matriz
glm::vec3 Transform::extractEulerAngles(const glm::mat4& mat) {
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);

    double yaw = atan2(forward.x, forward.z);
    double pitch = atan2(-forward.y, sqrt(forward.x * forward.x + forward.z * forward.z));
    double roll = atan2(left.y, up.y);

    return glm::vec3(
        glm::degrees(static_cast<float>(pitch)),
        glm::degrees(static_cast<float>(yaw)),
        glm::degrees(static_cast<float>(roll))
    );
}

// Extraer escala de una matriz
glm::vec3 Transform::extractScale(const glm::mat4& mat) {
    glm::vec3 left(mat[0][0], mat[0][1], mat[0][2]);
    glm::vec3 up(mat[1][0], mat[1][1], mat[1][2]);
    glm::vec3 forward(mat[2][0], mat[2][1], mat[2][2]);

    float scaleX = glm::length(left);
    float scaleY = glm::length(up);
    float scaleZ = glm::length(forward);

    return glm::vec3(scaleX, scaleY, scaleZ);
}

void Transform::setMatrix(const glm::mat4& matrix) {
    _mat = matrix;
}

