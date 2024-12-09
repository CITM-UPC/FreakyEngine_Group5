// FileDropHandler.h
#pragma once

#include <string>
#include <memory>
#include "glm/glm.hpp"
#include "MyGameEngine/Image.h"
#include "MyGameEngine/GameObject.h"
#include "SceneManager.h"
#include "Console.h"
#include "ModelImporter.h"

class FileDropHandler {
    ModelImporter modelImporter;
public:
    void handleFileDrop(const std::string& filePath, const glm::mat4& projection, const glm::mat4& view);
    void LoadTexture(const std::string& path, GameObject& go);
   // void LoadCustomFile(const std::string& path, GameObject& go);
    void LoadCustomFile(const char* path);
     int freakCounter = 0;
     std::string findAvailableName(const std::string& baseName);
     bool isObjectWithNameExists(const std::string& name);

private:
    std::string getFileExtension(const std::string& filePath);
    glm::vec3 screenToWorld(const glm::vec2& screenPos, float depth, const glm::mat4& projection, const glm::mat4& view);
    GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& windowSize);
};
