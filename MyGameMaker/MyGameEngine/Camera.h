#pragma once

#include "Transform.h"
#include <list>
#include "types.h"
#include <vector>
#include <GL/glew.h>


class Camera {

public:
	double fov = glm::radians(60.0);
	double aspect = 16.0 / 9.0 ;
	double zNear = 0.1;
	double zFar = 128.0;
	std::list<Plane> frustumPlanesList; // Lista de planos del frustum
private:
	Transform _transform;

public:
	const auto& transform() const { return _transform; }
	auto& transform() { return _transform; }

	mat4 projection() const;
	mat4 view() const;
	std::list<Plane> frustumPlanes() const;

	mat4 viewMatrix;
	mat4 projectionMatrix;
	mat4 viewProjectionMatrix;

	void UpdateCamera(Transform transform);
	void UpdateMainCamera();
	void UpdateProjection();
	void UpdateView(Transform transform);
	void UpdateViewProjection();
	void UpdateFrustumPlanes();

	
};


