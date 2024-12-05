#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "TreeExt.h"
#include "Transform.h"
#include "Texture.h"
#include "BoundingBox.h"
#include "Mesh.h"
#include <string>

class GameObject : public TreeExt<GameObject> {
private:
    Transform _transform;                       // Transformaci�n del objeto
    glm::u8vec3 _color = glm::u8vec3(255, 255, 255); // Color del objeto
    Texture _texture;                           // Textura del objeto
    std::shared_ptr<Mesh> _mesh_ptr;            // Puntero a la malla
    bool _active = true;                        // Estado de activaci�n
    std::string name;                           // Nombre del objeto
    mutable bool hasCreatedCheckerTexture = false; // Indica si la textura de cuadros ha sido creada

public:
    bool hasCheckerTexture = false;

    // M�todos para acceder y modificar propiedades
    const auto& transform() const { return _transform; }
    auto& transform() { return _transform; }

    const auto& color() const { return _color; }
    auto& color() { return _color; }

    const auto& texture() const { return _texture; }
    auto& texture() { return _texture; }

    const auto& mesh() const { return *_mesh_ptr; }
    auto& mesh() { return *_mesh_ptr; }

    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    bool operator==(const GameObject& other) const {
        // Comparar los objetos por el nombre, o cualquier criterio �nico
        return this->name == other.name;
    }

    // Transformaci�n global del objeto
    Transform worldTransform() const {
        return isRoot() ? _transform : parent().worldTransform() * _transform;
    }

    // C�lculo de las cajas de colisi�n
    BoundingBox localBoundingBox() const; // Definir en el .cpp
    BoundingBox boundingBox() const { return _transform.mat() * localBoundingBox(); }
    BoundingBox worldBoundingBox() const; // Definir en el .cpp

    // M�todos para manejar textura y malla
    void setTextureImage(const std::shared_ptr<Image>& img_ptr) { _texture.setImage(img_ptr); }
    void setMesh(const std::shared_ptr<Mesh>& mesh_ptr) { _mesh_ptr = mesh_ptr; }

    // Comprobaciones de existencia de textura y malla
    bool hasTexture() const { return _texture.id(); }
    bool hasMesh() const { return _mesh_ptr != nullptr; }

    // M�todo para dibujar el objeto
    void draw() const; // Definir en el .cpp

    // Activaci�n del objeto
    void setActive(bool active) { _active = active; }
    bool isActive() const { return _active; }

    // Inicializaci�n de textura de cuadros
    void initializeCheckerTexture();

    // M�todo para desparenteo
    void unparent() {
        if (!isRoot()) {              // Si tiene un padre
            parent().removeChild(*this); // Eliminar este objeto del padre
            _parent = nullptr;         // Actualizar la referencia del padre
        }
    }


};
