#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "MyGameEngine/GameObject.h"
#include "ModelImporter.h"
#include "TextureImporter.h"
class SceneManager
{
public:
	static void LoadGameObject(const std::string& filePath);
	static void spawnBakerHouse();
	static GameObject* getGameObject(int index);
	static void DestroyGameObject(GameObject* go);
	static void saveScene(const std::string& filePath);
	static void loadScene(const std::string& filePath);
	static ModelImporter modelImporter;
	static TextureImporter textureImporter;

public:
	static std::vector<GameObject> gameObjectsOnScene;
	static GameObject* selectedObject;

};