#include "FileDropHandler.h"
#include <iostream>
#include "Console.h" // Include for logging
#include "SceneManager.h"
#include <SDL2/SDL_mouse.h>
#include "MyGameEngine/Image.h"




static const glm::ivec2 WINDOW_SIZE1(1280, 720);

void FileDropHandler::handleFileDrop(const std::string& filePath, const glm::mat4& projection, const glm::mat4& view)
{
    auto extension = getFileExtension(filePath);
    auto imageTexture = std::make_shared<Image>();
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    if (extension == "obj" || extension == "fbx" || extension == "dae") {
        SceneManager::LoadGameObject(filePath);
        SceneManager::getGameObject(SceneManager::gameObjectsOnScene.size() - 1)->transform().pos() =
            screenToWorld(glm::vec2(mouseX, mouseY), 10.0f, projection, view);
        SceneManager::selectedObject = &SceneManager::gameObjectsOnScene.back();
    }
    else if (extension == "png" || extension == "jpg" || extension == "bmp") {
        imageTexture->loadTexture(filePath);
        GameObject* hitObject = raycastFromMouseToGameObject(mouseX, mouseY, projection, view, WINDOW_SIZE1);
        if (hitObject) {
            hitObject->setTextureImage(imageTexture);
            Console::Instance().Log("Texture applied to GameObject under mouse.");
        }
        else {
            Console::Instance().Log("No GameObject under mouse to apply texture.");
        }
    }
    else {
        Console::Instance().Log("Unsupported file extension: " + extension);
    }
}

glm::vec3 FileDropHandler::screenToWorld(const glm::vec2& mousePos, float depth, const glm::mat4& projection, const glm::mat4& view)
{
    float x = (2.0f * mousePos.x) / WINDOW_SIZE1.x - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y) / WINDOW_SIZE1.y;
    glm::vec4 clipCoords(x, y, -1.0f, 1.0f);


    glm::vec4 eyeCoords = glm::inverse(projection) * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);


    glm::vec3 worldRay = glm::vec3(glm::inverse(view) * eyeCoords);
    worldRay = glm::normalize(worldRay);


    glm::vec3 cameraPosition = glm::vec3(glm::inverse(view)[3]);
    return cameraPosition + worldRay * depth;
}

GameObject* FileDropHandler::raycastFromMouseToGameObject(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize)
{
    glm::vec3 rayOrigin = glm::vec3(glm::inverse(view) * glm::vec4(0, 0, 0, 1));
    glm::vec3 rayDirection = getRayFromMouse(mouseX, mouseY, projection, view, viewportSize);

    GameObject* hitObject = nullptr;

    for (auto& go : SceneManager::gameObjectsOnScene) {
        // Transformar bounding box del padre al espacio global
        BoundingBox globalBoundingBox = go.worldTransform().mat() * go.localBoundingBox();
        if (intersectRayWithBoundingBox(rayOrigin, rayDirection, globalBoundingBox)) {
            hitObject = &go;

            // Verificar los hijos en el espacio global
            for (auto& child : go.children()) {
                BoundingBox childGlobalBoundingBox = child.worldTransform().mat() * child.localBoundingBox();
                if (intersectRayWithBoundingBox(rayOrigin, rayDirection, childGlobalBoundingBox)) {
                    hitObject = &child;
                    break;
                }
            }
            break;
        }
    }
    return hitObject;
}

std::string FileDropHandler::getFileExtension(const std::string& filePath)
{
    size_t dotPosition = filePath.rfind('.');
    return (dotPosition == std::string::npos) ? "" : filePath.substr(dotPosition + 1);
    return std::string();
}

glm::vec3 FileDropHandler::getRayFromMouse(int mouseX, int mouseY, const glm::mat4& projection, const glm::mat4& view, const glm::ivec2& viewportSize)
{
    float x = (2.0f * mouseX) / viewportSize.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / viewportSize.y;
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayEye));
    return rayWorld;
}

bool FileDropHandler::intersectRayWithBoundingBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const BoundingBox& bbox)
{
    float tmin = (bbox.min.x - rayOrigin.x) / rayDirection.x;
    float tmax = (bbox.max.x - rayOrigin.x) / rayDirection.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (bbox.min.y - rayOrigin.y) / rayDirection.y;
    float tymax = (bbox.max.y - rayOrigin.y) / rayDirection.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (bbox.min.z - rayOrigin.z) / rayDirection.z;
    float tzmax = (bbox.max.z - rayOrigin.z) / rayDirection.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    return tmin >= 0.0f;
}
