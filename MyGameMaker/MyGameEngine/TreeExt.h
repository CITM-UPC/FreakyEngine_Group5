#pragma once

#include <list>
#include "readOnlyView.h"

template <class T>
class TreeExt {

protected:
    T* _parent = nullptr;
    std::list<T> _children;

public:
    auto& parent() const { return *_parent; }
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
        // Asegúrate de que el hijo pertenece a esta lista antes de intentar eliminarlo
        auto it = std::find_if(_children.begin(), _children.end(),
            [&](const T& existingChild) { return &existingChild == &child; });

        if (it != _children.end()) {
            _children.erase(it); // Eliminar el hijo si está en la lista
        }
    }

};
