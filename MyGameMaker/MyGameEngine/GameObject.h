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
    Transform _transform;                       // Transformación del objeto
    glm::u8vec3 _color = glm::u8vec3(255, 255, 255); // Color del objeto
    Texture _texture;                           // Textura del objeto
    std::shared_ptr<Mesh> _mesh_ptr;            // Puntero a la malla
    bool _active = true;                        // Estado de activación
                               // Nombre del objeto
    mutable bool hasCreatedCheckerTexture = false; // Indica si la textura de cuadros ha sido creada

public:
    std::string name;
    bool hasCheckerTexture = false;

    // Métodos para acceder y modificar propiedades
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
        // Comparar los objetos por el nombre, o cualquier criterio único
        return this->name == other.name;
    }
    bool operator!=(const GameObject& other) const {
        return !(*this == other);
    }

    // Transformación global del objeto
    Transform worldTransform() const {
        return isRoot() ? _transform : parent().worldTransform() * _transform;
    }
    Transform localTransform() const {
        return _transform;
    }
    void setTransform(const Transform& transform) {
        _transform = transform;
    }

    // Cálculo de las cajas de colisión
    BoundingBox localBoundingBox() const; // Definir en el .cpp
    BoundingBox boundingBox() const { return _transform.mat() * localBoundingBox(); }
    BoundingBox worldBoundingBox() const; // Definir en el .cpp

    // Métodos para manejar textura y malla
    void setTextureImage(const std::shared_ptr<Image>& img_ptr) { _texture.setImage(img_ptr); }
    void setMesh(const std::shared_ptr<Mesh>& mesh_ptr) { _mesh_ptr = mesh_ptr; }

    // Comprobaciones de existencia de textura y malla
    bool hasTexture() const { return _texture.id(); }
    bool hasMesh() const { return _mesh_ptr != nullptr; }

    // Método para dibujar el objeto
    void draw() const; // Definir en el .cpp

    // Activación del objeto
    void setActive(bool active) { _active = active; }
    bool isActive() const { return _active; }

    // Inicialización de textura de cuadros
    void initializeCheckerTexture();

    // Verificar si un objeto es ancestro de otro
    bool isAncestorOf(const GameObject& potentialDescendant) const {
        const GameObject* current = &potentialDescendant;
        std::unordered_set<const GameObject*> visited;

        while (current != nullptr) {
            // Verificar si hay ciclos
            if (visited.count(current)) {
                std::cerr << "Cycle detected in hierarchy!" << std::endl;
                return false; // Evita ciclos
            }
            visited.insert(current);

            // Si el actual es 'this', entonces 'this' es ancestro
            if (current == this) return true;

            // Subir al padre de manera segura
            current = current->parent2();
        }

        return false; // No se encontró que 'this' sea ancestro
    }


    void Destroy()
    {
        if (!this) {
            return;
        }

        //destroyed = true;

        /*for (auto& component : components)
        {
            component.second->Destroy();
        }*/

        for (auto& child : children())
        {
            child.Destroy();
        }
    }


};
