#include "SceneManager.h"
#include "MyGameEngine/GameObject.h"
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/Image.h"
#include "Console.h"
#include <sstream>
#include "ModelImporter.h"
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <fstream>
#include <filesystem>

ModelImporter SceneManager::modelImporter;

std::vector<GameObject> SceneManager::gameObjectsOnScene;
GameObject* SceneManager::selectedObject = nullptr;
void SceneManager::spawnBakerHouse()
{
    GameObject go;
    auto mesh = std::make_shared<Mesh>();
    mesh->LoadFile("Assets/BakerHouse.fbx");
    go.setMesh(mesh);
    auto imageTexture = std::make_shared<Image>();
    imageTexture->loadTexture("Assets/Baker_House.png");
    go.setTextureImage(imageTexture);
    go.transform().pos() = vec3(4, 0, 0);
    go.setName("GameObject (" + std::to_string(gameObjectsOnScene.size()) + ")");
    SceneManager::gameObjectsOnScene.push_back(go);
}


void SceneManager::LoadGameObject(const std::string& filePath) {
    auto mesh = std::make_shared<Mesh>();

    GameObject go;
    mesh->LoadFile(filePath.c_str());
    go.setMesh(mesh);
    go.setName("GameObject (" + std::to_string(gameObjectsOnScene.size()) + ")");
    gameObjectsOnScene.push_back(go);
    Console::Instance().Log("Fbx imported succesfully.");
}

GameObject* SceneManager::getGameObject(int index) {
    return &gameObjectsOnScene[index];
}

void SceneManager::DestroyGameObject(GameObject* go) {
    auto it = std::find_if(gameObjectsOnScene.begin(), gameObjectsOnScene.end(),
        [](const GameObject& obj) { return &obj == selectedObject; });

    if (it != gameObjectsOnScene.end())
    {
        gameObjectsOnScene.erase(it);
        selectedObject = nullptr;
        Console::Instance().Log("Object deleted");
    }
    else if (go->parent().children().size() > 0)
    {
        GameObject* parent = &go->parent();
        parent->removeChild(*go);
        selectedObject = nullptr;
        Console::Instance().Log("Child object deleted");
    }
    else
    {
        Console::Instance().Log("Object not found");
    }
}
void SceneManager::saveScene(const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        Console::Instance().Log("Failed to open file for saving scene: " + filePath);
        return;
    }

    size_t gameObjectCount = SceneManager::gameObjectsOnScene.size();
    outFile.write(reinterpret_cast<const char*>(&gameObjectCount), sizeof(gameObjectCount));

    for (const auto& gameObject : SceneManager::gameObjectsOnScene) {
        // Save GameObject name
        const std::string& name = gameObject.getName();
        size_t nameLength = name.size();
        outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        outFile.write(name.data(), nameLength);

        // Save Transform
        const auto& transform = gameObject.GetComponent<TransformComponent>()->transform();
        glm::vec3 position = transform.pos();
        glm::vec3 scale = transform.extractScale(transform.mat());
        glm::vec3 rotation = transform.extractEulerAngles(transform.mat());
        outFile.write(reinterpret_cast<const char*>(&position), sizeof(position));
        outFile.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
        outFile.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));

        // Save Mesh
        if (gameObject.hasMesh()) {
            const auto& mesh = gameObject.getMeshPtr();
            bool hasMesh = true;
            outFile.write(reinterpret_cast<const char*>(&hasMesh), sizeof(hasMesh));

            // Save vertices
            const auto& vertices = mesh->vertices();
            size_t vertexCount = vertices.size();
            outFile.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
            outFile.write(reinterpret_cast<const char*>(vertices.data()), vertexCount * sizeof(glm::vec3));

            // Save indices
            const auto& indices = mesh->indices();
            size_t indexCount = indices.size();
            outFile.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
            outFile.write(reinterpret_cast<const char*>(indices.data()), indexCount * sizeof(unsigned int));

            // Save texture coordinates
            const auto& texCoords = mesh->texCoords();
            size_t texCoordCount = texCoords.size();
            outFile.write(reinterpret_cast<const char*>(&texCoordCount), sizeof(texCoordCount));
            outFile.write(reinterpret_cast<const char*>(texCoords.data()), texCoordCount * sizeof(glm::vec2));

            // Save bounding box
            const auto& boundingBox = mesh->boundingBox();
            outFile.write(reinterpret_cast<const char*>(&boundingBox), sizeof(BoundingBox));
        }
        else {
            bool hasMesh = false;
            outFile.write(reinterpret_cast<const char*>(&hasMesh), sizeof(hasMesh));
        }
    }

    outFile.close();
    Console::Instance().Log("Scene saved to " + filePath);
}

void SceneManager::loadScene(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        Console::Instance().Log("Failed to open file for loading scene: " + filePath);
        return;
    }

    SceneManager::gameObjectsOnScene.clear();

    size_t gameObjectCount;
    inFile.read(reinterpret_cast<char*>(&gameObjectCount), sizeof(gameObjectCount));

    for (size_t i = 0; i < gameObjectCount; ++i) {
        GameObject gameObject;

        // Load GameObject name
        size_t nameLength;
        inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        std::string name(nameLength, '\0');
        inFile.read(name.data(), nameLength);
        gameObject.setName(name);

        // Load Transform
        glm::vec3 position, scale, rotation;
        inFile.read(reinterpret_cast<char*>(&position), sizeof(position));
        inFile.read(reinterpret_cast<char*>(&scale), sizeof(scale));
        inFile.read(reinterpret_cast<char*>(&rotation), sizeof(rotation));

        Transform transform;
        transform.translate(position);
        transform.setScale(scale);
        transform.setRotation(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z));

        // Load Mesh
        bool hasMesh;
        inFile.read(reinterpret_cast<char*>(&hasMesh), sizeof(hasMesh));
        if (hasMesh) {
            auto mesh = std::make_shared<Mesh>();

            // Load vertices
            size_t vertexCount;
            inFile.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
            std::vector<glm::vec3> vertices(vertexCount);
            inFile.read(reinterpret_cast<char*>(vertices.data()), vertexCount * sizeof(glm::vec3));

            // Load indices
            size_t indexCount;
            inFile.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
            std::vector<unsigned int> indices(indexCount);
            inFile.read(reinterpret_cast<char*>(indices.data()), indexCount * sizeof(unsigned int));

            // Load texture coordinates
            size_t texCoordCount;
            inFile.read(reinterpret_cast<char*>(&texCoordCount), sizeof(texCoordCount));
            std::vector<glm::vec2> texCoords(texCoordCount);
            inFile.read(reinterpret_cast<char*>(texCoords.data()), texCoordCount * sizeof(glm::vec2));

            // Load bounding box
            BoundingBox boundingBox;
            inFile.read(reinterpret_cast<char*>(&boundingBox), sizeof(BoundingBox));

            mesh->load(vertices.data(), vertices.size(), indices.data(), indices.size());
            if (!texCoords.empty()) {
                mesh->loadTexCoords(texCoords.data(), texCoords.size());
            }

            // Add Mesh to GameObject
            gameObject.setMesh(mesh);
        }

        gameObject.AddComponent<TransformComponent>()->transform() = transform;
        SceneManager::gameObjectsOnScene.push_back(gameObject);
    }

    inFile.close();
    Console::Instance().Log("Scene loaded from " + filePath);
}




