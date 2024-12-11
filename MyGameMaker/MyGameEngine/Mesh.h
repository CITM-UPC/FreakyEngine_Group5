#pragma once
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <vector>
#include <glm/glm.hpp>
#include "BufferObject.h"
#include "BoundingBox.h"



class Mesh
{
	std::vector<glm::vec3> _vertices;
	std::vector<unsigned int> _indices;
	std::vector<glm::vec2> _texCoords;
	std::vector<glm::u8vec3> _colors;

	BufferObject _vertexBuffer;
	BufferObject _indexBuffer;
	BufferObject _texCoordsBuffer;
	BufferObject _normalsBuffer;
	BufferObject _colorsBuffer;
	BufferObject _texCoords_buffer;

	unsigned int _idTexture = 0;

	BoundingBox _boundingBox;


public:

	Mesh();
	Mesh(const Mesh& other);
		// Copiar todos los miembros necesarios
	
	const auto& vertices() const { return _vertices; }
	const auto& indices() const { return _indices; }
	const auto& boundingBox() const { return _boundingBox; }
	const auto& texCoords() const { return _texCoords; }
	const auto& colors() const { return _colors; }

	void load(const glm::vec3* vertices, size_t num_verts, unsigned int* indices, size_t num_indexs);
	void loadTexCoords(const glm::vec2* tex_coords, size_t num_tex_coords);
	void loadNormals(const glm::vec3* normals, size_t num_normals);
	void loadColors(const glm::u8vec3* colors, size_t num_colors);
	void draw() const;

	void LoadFile(const char* filePath);

	// Load Texture
	void drawNormals(const glm::mat4& modelMatrix);
	void drawNormalsPerFace(const glm::mat4& modelMatrix);
	bool LoadCustomFile(const std::string& filePath);



};