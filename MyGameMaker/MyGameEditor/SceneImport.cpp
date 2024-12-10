#include "SceneImport.h"

void SceneImport::SaveSceneToFile(const std::vector<std::shared_ptr<GameObject>>& sceneObjects, const std::string& filePath) {
    std::filesystem::path path(filePath);

    if (!std::filesystem::exists(path.parent_path())) {
        throw std::runtime_error("Directory does not exist: " + path.parent_path().string());
    }

    std::ofstream outFile(filePath, std::ios::out);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    // Guardar el número de GameObjects
    outFile << sceneObjects.size() << "\n";

    for (const auto& gameObject : sceneObjects) {
        if (!gameObject) continue;

        // Guardar el nombre
        outFile << gameObject->GetName() << "\n";

        // Guardar Transform (posición, rotación y escala)
        const auto& transform = gameObject->GetComponent<TransformComponent>()->transform();
        const glm::vec3 position = transform.pos();
        const glm::vec3 rotation = transform.GetRotation(); // Suponiendo que devuelve Euler angles
        const glm::vec3 scale = transform.extractScale(transform.mat());

        outFile << position.x << " " << position.y << " " << position.z << "\n";
        outFile << rotation.x << " " << rotation.y << " " << rotation.z << "\n";
        outFile << scale.x << " " << scale.y << " " << scale.z << "\n";
    }

    outFile.close();
}

std::vector<std::shared_ptr<GameObject>> SceneImport::LoadSceneFromFile(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::in);
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }

    size_t objectCount;
    inFile >> objectCount;

    std::vector<std::shared_ptr<GameObject>> sceneObjects;
    sceneObjects.reserve(objectCount);

    for (size_t i = 0; i < objectCount; ++i) {
        std::string name;
        inFile >> name;

        auto gameObject = std::make_shared<GameObject>(name);

        glm::vec3 position, rotation, scale;

        // Leer posición, rotación y escala
        inFile >> position.x >> position.y >> position.z;
        inFile >> rotation.x >> rotation.y >> rotation.z;
        inFile >> scale.x >> scale.y >> scale.z;

        auto& transform = gameObject->GetComponent<TransformComponent>()->transform();
        transform.setPos(position.x, position.y, position.z);
        transform.setRotation(rotation.x, rotation.y, rotation.z); // Ajusta si se necesitan cuaterniones
        transform.setScale(scale);

        sceneObjects.push_back(gameObject);
    }

    inFile.close();
    return sceneObjects;
}