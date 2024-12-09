// FileDropHandler.cpp
#include "FileDropHandler.h"


#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <iostream>

static const glm::ivec2 WINDOW_SIZE2(1280, 720);

void FileDropHandler::handleFileDrop(const std::string& filePath, const glm::mat4& projection, const glm::mat4& view) {
    auto extension = getFileExtension(filePath);
    auto imageTexture = std::make_shared<Image>();
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (extension == "obj" || extension == "fbx" || extension == "dae") {
        SceneManager::LoadGameObject(filePath);
        SceneManager::getGameObject(SceneManager::gameObjectsOnScene.size() - 1)->transform().pos() = screenToWorld(glm::vec2(mouseX, mouseY), 10.0f, projection, view);
        SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
    } else if (extension == "png" || extension == "jpg" || extension == "bmp") {
        imageTexture->loadTexture(filePath);

        GameObject* hitObject = raycastFromMouseToGameObject(mouseX, mouseY, projection, view, WINDOW_SIZE2);
        if (hitObject) {
            hitObject->setTextureImage(imageTexture);
            std::cout << "Texture applied to GameObject under mouse." << std::endl;
            Console::Instance().Log("Texture applied to GameObject under mouse.");
        } else {
            std::cout << "No GameObject under mouse to apply texture." << std::endl;
            Console::Instance().Log("No GameObject under mouse to apply texture.");
        }
    } else {
        std::cout << "Unsupported file extension: " << extension << std::endl;
        Console::Instance().Log("Unsupported file extension: " + extension);
    }
}

std::string FileDropHandler::getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    return filePath.substr(dotPos + 1);
}

glm::vec3 FileDropHandler::screenToWorld(const glm::vec2& screenPos, float depth, const glm::mat4& projection, const glm::mat4& view) {
    // Implement the screen to world conversion logic here
    // This is a placeholder implementation
    return glm::vec3(screenPos, depth);
}

GameObject* FileDropHandler::raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& windowSize) {
    // Implement the raycast logic here
    // This is a placeholder implementation
    return nullptr;
}
