#ifndef FILE_DROP_HANDLER_H
#define FILE_DROP_HANDLER_H

#include <string>
#include <memory>
#include "glm/glm.hpp"
#include "MyGameEngine/Image.h"
#include "MyGameEngine/GameObject.h"
#include "SceneManager.h"
#include "Console.h"
#include "ModelImporter.h"
#include "TextureImporter.h"

class FileDropHandler
{
  
public:
    FileDropHandler() = default;
    FileDropHandler(const FileDropHandler&) = delete;
    FileDropHandler(FileDropHandler&&) noexcept = delete;
    FileDropHandler& operator=(const FileDropHandler&) = delete;

    ~FileDropHandler() = default;

    // Handles a file drop, processes the file based on its extension
    void handleFileDrop(const std::string& filePath, const glm::mat4& projection, const glm::mat4& view);

    // Converts screen coordinates to world coordinates
    glm::vec3 screenToWorld(const glm::vec2& mousePos, float depth, const glm::mat4& projection, const glm::mat4& view);

    // Performs a raycast from the mouse position and returns the GameObject hit
    GameObject* raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize);

private:
    // Helper function to get the file extension from a file path
    std::string getFileExtension(const std::string& filePath);

    // Computes a ray from the mouse position in screen coordinates
    glm::vec3 getRayFromMouse(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize);

    // Checks for intersection between a ray and a bounding box
    bool intersectRayWithBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const BoundingBox& bbox);
};

#endif // FILE_DROP_HANDLER_H
