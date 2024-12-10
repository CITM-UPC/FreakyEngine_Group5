
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
    findAvailableName(filePath);
    if (extension == "obj" || extension == "fbx" || extension == "dae") {
        auto mesh = std::make_shared<Mesh>();
        mesh = modelImporter.ImportModel(filePath.c_str());
        modelImporter.SaveMeshToFile(mesh, "Library/CustomFreaks/StandardFreak" + std::to_string(freakCounter) + ".freak");
        freakCounter++;
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
            auto imageTexture = std::make_shared<Image>();
            imageTexture = textureImporter.loadTexture(filePath.c_str());
            textureImporter.SaveTextureToFile(imageTexture, "Library/CustomTexture/StandardTexture" + std::to_string(textureCounter) + ".texture");
            std::cout << "No GameObject under mouse to apply texture." << std::endl;
            Console::Instance().Log("No GameObject under mouse to apply texture.");
        }
    }
    else if (extension == "freak") {
        

            auto mesh = std::make_shared<Mesh>();
            GameObject go;
            mesh = modelImporter.LoadModelFromFile(filePath);
            go.modelPath = filePath;
            go.setMesh(mesh);
            SceneManager::gameObjectsOnScene.push_back(go);
            int newID = SceneManager::gameObjectsOnScene[SceneManager::gameObjectsOnScene.size() - 1].id;
            go.id = newID + 1;
        
    }
    else if (extension == "texture") {
        

            auto imageTexture = std::make_shared<Image>();
            imageTexture = textureImporter.LoadTextureFromFile(filePath);
            GameObject* hitObject = raycastFromMouseToGameObject(mouseX, mouseY, projection, view, WINDOW_SIZE2);
            hitObject->setTextureImage(imageTexture);
          
        
    }
    else {
        std::cout << "Unsupported file extension: " << extension << std::endl;
        Console::Instance().Log("Unsupported file extension: " + extension);
    }
}
std::string FileDropHandler::findAvailableName(const std::string& baseName) {
    int counter = 1;
    std::string newName = baseName;
    while (isObjectWithNameExists(newName)) {
        newName = baseName + std::to_string(counter);
        counter++;
    }
    return newName;
}

bool FileDropHandler::isObjectWithNameExists(const std::string& name) {
    for (const auto& gameObject : SceneManager::gameObjectsOnScene) {
        if (gameObject.getName() == name) {
            return true;
        }
    }
    return false;
}

void FileDropHandler::LoadCustomFile(const char* path)
{
    // Load file
	
    
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

void FileDropHandler::LoadTexture(const std::string& path, GameObject& go) {
    auto imageTexture = std::make_shared<Image>();

    if (imageTexture->loadTexture(path), imageTexture->width() > 0) {
        go.setTextureImage(imageTexture);
        std::cout << "Texture loaded and applied to GameObject: " << go.getName() << std::endl;
        Console::Instance().Log("Texture loaded and applied to GameObject: " + go.getName());
    }
    else {
        std::cout << "Failed to load texture from path: " << path << std::endl;
        Console::Instance().Log("Failed to load texture from path: " + path);
    }
}
