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
   
    void removeChild(const T& child) { return _children.remove((child)); }

    void unparent() {
        if (!isRoot() && _parent != nullptr) { 
            _parent->removeChild(*static_cast<T*>(this)); 
            _parent = nullptr; 
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
