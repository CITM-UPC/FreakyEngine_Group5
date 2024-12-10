#pragma once

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include "MyGameEngine/GameObject.h"

class SceneImport {
public:
    static void SaveSceneToFile(const std::vector<std::shared_ptr<GameObject>>& sceneObjects, const std::string& filePath);
    static std::vector<std::shared_ptr<GameObject>> LoadSceneFromFile(const std::string& filePath);
};
