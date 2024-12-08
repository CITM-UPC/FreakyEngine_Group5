#pragma once

#include <list>
#include "readOnlyView.h"
#include <iostream>
template <class T>
class TreeExt {
private:
    T* _parent = nullptr;
    std::list<T> _children;

public:
    auto& parent() const { return *_parent; }
    auto* parent2() const { return _parent; }  // Pointer-safe parent accessor
    auto children() const { return readOnlyListView<T>(_children); }
    auto& root() const { return _parent ? _parent->root() : *this; }
    bool isRoot() const { return !_parent; }

    template <typename ...Args>
    auto& emplaceChild(Args&&... args) {
        _children.emplace_back(std::forward<Args>(args)...);
        _children.back()._parent = static_cast<T*>(this);
        return _children.back();
    }
   
    void removeChild(T& child) {
        auto it = std::find_if(_children.begin(), _children.end(),
            [&](const T& c) { return &c == &child; }); // Busca el hijo en la lista

        if (it != _children.end()) {
            it->_parent = nullptr; // Limpia el puntero al padre
            _children.erase(it);   // Elimina el hijo de la lista
        }
        else {
            std::cerr << "Attempted to remove a child that does not exist." << std::endl;
        }
    }

    void unparent() {
        if (!isRoot() && _parent != nullptr) { // Verifica si tiene padre válido
            std::cout << "Unparenting object: " << this << " from parent: " << _parent << std::endl;
            _parent->removeChild(*static_cast<T*>(this)); // Remueve el hijo del padre
            _parent = nullptr; // Limpia la referencia al padre
        }
        else {
            std::cerr << "Unparent called on a root object or with invalid _parent." << std::endl;
        }
    }
    auto& setParent(T& newParent) {
        // Check if the object already has a parent
        if (_parent) {
            _parent->removeChild(*static_cast<T*>(this));
        }

        // Set the new parent
        _parent = &newParent;

        // Add the object to the new parent's children
        newParent._children.push_back(std::move(*static_cast<T*>(this)));
        return _parent;
    }
    
};
