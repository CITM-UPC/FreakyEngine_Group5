#pragma once


#include <vector>
#include <glm/glm.hpp>

#include "BufferObject.h"

class Mesh
{
private:

	std::vector<glm::vec3> _vertices;
	std::vector<unsigned int> _indices;


	BufferObject _vertices_buffer;
	BufferObject _indices_buffer;
	BufferObject _tex_coords_buffer;
	BufferObject _normals_buffer;
	BufferObject _colors_buffer;

public:

	const auto& vertices() const { return _vertices; }
	const auto& indices() const { return _indices; }

	void load(const glm::vec3* vertives, size_t num_verts, unsigned int* indices, size_t num_index);
	void loadTexCoords(const glm::vec2* tex_coords, size_t num_tex_coords);
	void loadNormals(const glm::vec3* normals, size_t num_normals);
	void loadColors(const glm::u8vec3* colors, size_t num_colors);
	void draw() const;


};

