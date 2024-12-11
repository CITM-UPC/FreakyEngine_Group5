#include "SceneManager.h"
#include "MyGameEngine/GameObject.h"
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/Image.h"
#include "Console.h"
#include <sstream>
#include "ModelImporter.h"
#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

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
    std::ofstream outFile(filePath, std::ios::out);
    if (!outFile) {
        Console::Instance().Log("Failed to open file for saving scene: " + filePath);
        return;
    }

    outFile << "{\n\"GameObjects\": [\n";
    for (size_t i = 0; i < SceneManager::gameObjectsOnScene.size(); ++i) {
        const auto& go = SceneManager::gameObjectsOnScene[i];
        const auto& transform = go.GetComponent<TransformComponent>()->transform();

        outFile << "  {\n";
        //      outFile << "    \"UID\": " << go.getUUID() << ",\n";
        outFile << "    \"Name\": \"" << go.getName() << "\",\n";
        outFile << "    \"Transform\": {\n";
        outFile << "      \"Position\": [" << transform.pos().x << ", " << transform.pos().y << ", " << transform.pos().z << "],\n";
        outFile << "      \"Scale\": [" << transform.extractScale(transform.mat()).x << ", " << transform.extractScale(transform.mat()).y << ", " << transform.extractScale(transform.mat()).z << "],\n";
        outFile << "      \"Rotation\": [" << transform.extractEulerAngles(transform.mat()).x << ", " << transform.extractEulerAngles(transform.mat()).y << ", " << transform.extractEulerAngles(transform.mat()).z << "]\n";
        outFile << "    }";

        // Guardar datos de la malla
        if (go.hasMesh()) {
            const auto& mesh = go.getMeshPtr();
            outFile << ",\n    \"Mesh\": {\n";

            // Serializar vértices
            const auto& vertices = mesh->vertices();
            outFile << "      \"Vertices\": [";
            for (size_t j = 0; j < vertices.size(); ++j) {
                outFile << "[" << vertices[j].x << ", " << vertices[j].y << ", " << vertices[j].z << "]";
                if (j != vertices.size() - 1) outFile << ", ";
            }
            outFile << "],\n";

            // Serializar índices
            const auto& indices = mesh->indices();
            outFile << "      \"Indices\": [";
            for (size_t j = 0; j < indices.size(); ++j) {
                outFile << indices[j];
                if (j != indices.size() - 1) outFile << ", ";
            }
            outFile << "],\n";

            // Serializar coordenadas de textura
            const auto& texCoords = mesh->texCoords();
            outFile << "      \"TexCoords\": [";
            for (size_t j = 0; j < texCoords.size(); ++j) {
                outFile << "[" << texCoords[j].x << ", " << texCoords[j].y << "]";
                if (j != texCoords.size() - 1) outFile << ", ";
            }
            outFile << "],\n";

            // Serializar caja delimitadora
            const auto& boundingBox = mesh->boundingBox();
            outFile << "      \"BoundingBox\": {\n";
            outFile << "        \"Min\": [" << boundingBox.min.x << ", " << boundingBox.min.y << ", " << boundingBox.min.z << "],\n";
            outFile << "        \"Max\": [" << boundingBox.max.x << ", " << boundingBox.max.y << ", " << boundingBox.max.z << "]\n";
            outFile << "      }\n";

            outFile << "    }";
        }

        outFile << "\n  }";
        if (i != SceneManager::gameObjectsOnScene.size() - 1) outFile << ",";
        outFile << "\n";
    }
    outFile << "]\n}";
    outFile.close();

    Console::Instance().Log("Scene saved to " + filePath);
}

void SceneManager::loadScene(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::in);
    if (!inFile) {
        Console::Instance().Log("Failed to open file for loading scene: " + filePath);
        return;
    }

    SceneManager::gameObjectsOnScene.clear(); // Clear current scene
    std::string line;

    while (std::getline(inFile, line)) {
        if (line.find("\"UID\":") != std::string::npos) {
            GameObject gameObject;
            gameObject.setUUID(std::stoi(line.substr(line.find(":") + 1)));

            std::string name;
            Transform transform;
            std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

            // Parse JSON for GameObject properties
            while (std::getline(inFile, line) && line.find("}") == std::string::npos) {
                if (line.find("\"Name\":") != std::string::npos) {
                    name = line.substr(line.find(":") + 2);
                    name.erase(name.find_last_of("\""));
                    gameObject.setName(name);
                }
                else if (line.find("\"Position\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 pos;
                    char delim;
                    iss >> pos.x >> delim >> pos.y >> delim >> pos.z;
                    transform.translate(pos);
                }
                else if (line.find("\"Scale\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 scale;
                    char delim;
                    iss >> scale.x >> delim >> scale.y >> delim >> scale.z;
                    transform.setScale(scale);
                }
                else if (line.find("\"Rotation\":") != std::string::npos) {
                    std::istringstream iss(line.substr(line.find("[") + 1));
                    glm::vec3 rotation;
                    char delim;
                    iss >> rotation.x >> delim >> rotation.y >> delim >> rotation.z;
                    transform.setRotation(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
                }
                else if (line.find("\"Mesh\":") != std::string::npos) {
                    // Parse mesh properties
                    while (std::getline(inFile, line) && line.find("}") == std::string::npos) {
                        if (line.find("\"Vertices\":") != std::string::npos) {
                            std::vector<glm::vec3> vertices;
                            while (std::getline(inFile, line) && line.find("]") == std::string::npos) {
                                glm::vec3 vertex;
                                std::sscanf_s(line.c_str(), " [%f, %f, %f]", &vertex.x, &vertex.y, &vertex.z);
                                vertices.push_back(vertex);
                            }
                            mesh->load(vertices.data(), vertices.size(), nullptr, 0);
                        }
                        else if (line.find("\"Indices\":") != std::string::npos) {
                            std::vector<unsigned int> indices;
                            while (std::getline(inFile, line) && line.find("]") == std::string::npos) {
                                unsigned int index;
                                std::sscanf_s(line.c_str(), " %u", &index);
                                indices.push_back(index);
                            }
                            mesh->load(nullptr, 0, indices.data(), indices.size());
                        }
                        else if (line.find("\"TexCoords\":") != std::string::npos) {
                            std::vector<glm::vec2> texCoords;
                            while (std::getline(inFile, line) && line.find("]") == std::string::npos) {
                                glm::vec2 texCoord;
                                std::sscanf_s(line.c_str(), " [%f, %f]", &texCoord.x, &texCoord.y);
                                texCoords.push_back(texCoord);
                            }
                            mesh->loadTexCoords(texCoords.data(), texCoords.size());
                        }
                    }
                }
            }

            // Assign components and mesh
            gameObject.AddComponent<TransformComponent>()->transform() = transform;
            if (mesh->vertices().size() > 0) { // Check if mesh has been loaded
                gameObject.setMesh(mesh);
            }

            // Add GameObject to the scene
            SceneManager::gameObjectsOnScene.push_back(gameObject);
        }
    }

    inFile.close();
    Console::Instance().Log("Scene loaded from " + filePath);
}

