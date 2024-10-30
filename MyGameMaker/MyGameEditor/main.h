#pragma once
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::vec2 getMousePosition();
glm::vec3 screenToWorld(const glm::vec2& mousePos, float depth, const glm::mat4& projection, const glm::mat4& view);