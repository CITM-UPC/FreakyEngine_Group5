#pragma once
#include <iostream>
#include "MyGameEngine/GameObject.h"

class BasicShapesManager {
public:
    static std::shared_ptr<Mesh> MakeTriangleMesh(double size);
    static std::shared_ptr<Mesh> MakeQuadMesh(double size);
    static std::shared_ptr<Mesh> MakeCubeMesh(double size);
    static void SetRequestedMesh(GameObject& go, int figureType, double size);

    // Añadimos las coordenadas del ratón
    static void createFigure(int figureType, std::vector<GameObject>& gameObjects, double size = 1.0, glm::vec3 mousePosition = glm::vec3(0.0f,0.0f,0.0f));
};
