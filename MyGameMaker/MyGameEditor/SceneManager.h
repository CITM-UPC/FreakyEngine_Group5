#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "MyGameEngine/GameObject.h"
class SceneManager
{
public:
	static void LoadGameObject(const std::string& filePath);
	static void spawnBakerHouse();
	static GameObject* getGameObject(int index);
	static void DestroyGameObject(GameObject* go);

public: 
	static std::vector<GameObject> gameObjectsOnScene;
	static GameObject* selectedObject;

};

