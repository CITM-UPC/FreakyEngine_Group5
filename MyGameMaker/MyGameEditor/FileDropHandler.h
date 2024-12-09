// FileDropHandler.h
#pragma once

#include <string>
#include <memory>
#include "glm/glm.hpp"
#include "MyGameEngine/Image.h"
#include "SceneManager.h"
#include "Console.h"

class FileDropHandler {
public:
    void handleFileDrop(const std::string& filePath, const glm::mat4& projection, const glm::mat4& view);

private:
    std::string getFileExtension(const std::string& filePath);
    glm::vec3 screenToWorld(const glm::vec2& screenPos, float depth, const glm::mat4& projection, const glm::mat4& view);
    GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& windowSize);
};
