#include "SceneManager.h"
#include "MyGameEngine/GameObject.h"
#include "MyGameEngine/Mesh.h"
#include "MyGameEngine/Image.h"
#include "Console.h"

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

void SceneManager::SaveScene(const std::string& filePath) {
    try {
        std::vector<std::shared_ptr<GameObject>> sceneObjects;
        for (auto& obj : gameObjectsOnScene) {
            sceneObjects.push_back(std::make_shared<GameObject>(obj));
        }
        SceneImport::SaveSceneToFile(sceneObjects, filePath);
        Console::Instance().Log("Scene saved successfully to " + filePath);
    }
    catch (const std::exception& ex) {
        Console::Instance().Log(std::string("Failed to save scene: ") + ex.what());
    }
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
