#include "Mesh.h"


#include <GL/glew.h>
using namespace std;


//When load method is called you should unload the previous data
void Mesh::load(const glm::vec3* vertices, size_t num_verts, unsigned int* indices, size_t num_index)
{


	_vertices_buffer.loadData(vertices, num_verts * sizeof(glm::vec3));
	_indices_buffer.loadIndices(indices, num_index);

	_tex_coords_buffer.unload();
	_normals_buffer.unload();
	_colors_buffer.unload();

	_vertices.clear();
	_indices.clear();

	_vertices.assign(vertices, vertices + num_verts);
	_indices.assign(indices, indices + num_index);
}

void Mesh::loadTexCoords(const glm::vec2* tex_coords, size_t num_tex_coords)
{
	_tex_coords_buffer.loadData(tex_coords, num_tex_coords * sizeof(glm::vec2));
}

void Mesh::loadNormals(const glm::vec3* normals, size_t num_normals)
{
	_normals_buffer.loadData(normals, num_normals * sizeof(glm::vec3));
}

void Mesh::loadColors(const glm::u8vec3* colors, size_t num_colors)
{
	_colors_buffer.loadData(colors, num_colors * sizeof(glm::u8vec3));
}

void Mesh::draw() const
{
	if (_tex_coords_buffer.id()) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		_tex_coords_buffer.bind();
		glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
	}

	if (_normals_buffer.id()) {
		glEnableClientState(GL_NORMAL_ARRAY);
		_normals_buffer.bind();
		glNormalPointer(GL_FLOAT, 0, nullptr);
	}

	if (_colors_buffer.id()) {
		glEnableClientState(GL_COLOR_ARRAY);
		_colors_buffer.bind();
		glColorPointer(3, GL_UNSIGNED_BYTE, 0, nullptr);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	_vertices_buffer.bind();
	glVertexPointer(3, GL_FLOAT, 0, nullptr);

	_indices_buffer.bind();
	glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, nullptr);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (_colors_buffer.id())
		glDisableClientState(GL_COLOR_ARRAY);
	if (_normals_buffer.id())
		glDisableClientState(GL_NORMAL_ARRAY);
	if (_tex_coords_buffer.id())
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}