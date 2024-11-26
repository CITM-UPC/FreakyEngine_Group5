#pragma once

#include "types.h"

class Transform {
private:
    union {
        mat4 _mat = mat4(1.0);  // Matriz de transformación inicializada como identidad
        struct {
            vec3 _left; mat4::value_type _left_w;
            vec3 _up; mat4::value_type _up_w;
            vec3 _fwd; mat4::value_type _fwd_w;
            vec3 _pos; mat4::value_type _pos_w;
        };
    };

    // Variables para los ángulos de rotación (en radianes)
    double yaw = 0.0;
    double pitch = 0.0;
    double roll = 0.0;

public:
    // Acceso a los valores internos de la matriz y vectores
    const auto& mat() const { return _mat; }
    const auto& left() const { return _left; }
    const auto& up() const { return _up; }
    const auto& fwd() const { return _fwd; }
    const auto& pos() const { return _pos; }
    auto& pos() { return _pos; } // Permitir modificar la posición

    const auto* data() const { return &_mat[0][0]; }

    // Constructores
    Transform() = default;
    Transform(const mat4& mat) : _mat(mat) {}

    // Métodos de transformación
    void translate(const vec3& v);
    void rotateYaw(double radians);
    void rotatePitch(double radians);
    void rotateRoll(double radians);
    void rotate(double rads, const vec3& v);
    void updateRotationMatrix();

    // Métodos relacionados con la orientación
    void lookAt(const vec3& target);
    void alignCamera(const vec3& worldUp = vec3(0.0f, 1.0f, 0.0f));

    // Métodos para escala
    void scale(const vec3& s);          // Aplicar un escalado relativo
    void setScale(const vec3& scale);  // Establecer un escalado absoluto

    // Métodos para extraer propiedades de la matriz
    glm::vec3 extractEulerAngles(const glm::mat4& mat); // Obtener ángulos Euler
    glm::vec3 extractScale(const glm::mat4& mat);       // Obtener escala

    // Operadores para combinar transformaciones
    Transform operator*(const mat4& other) { return Transform(_mat * other); }
    Transform operator*(const Transform& other) { return Transform(_mat * other._mat); }


    void setMatrix(const glm::mat4& matrix);

};
