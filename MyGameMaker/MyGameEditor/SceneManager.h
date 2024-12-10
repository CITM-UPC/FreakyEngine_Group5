#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "MyGameEngine/GameObject.h"
#include "SceneImport.h"
class SceneManager
{
public:
	static void LoadGameObject(const std::string& filePath);
	static void spawnBakerHouse();
	static GameObject* getGameObject(int index);
	static void DestroyGameObject(GameObject* go);
	static void saveScene(const std::string& filePath);
	static void loadScene(const std::string& filePath);

public: 
	static std::vector<GameObject> gameObjectsOnScene;
	static GameObject* selectedObject;

};

