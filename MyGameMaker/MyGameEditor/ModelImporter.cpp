#include "ModelImporter.h"
#include <filesystem>

std::shared_ptr<Mesh> ModelImporter::ImportModel(const char* filePath)
{

	auto mesh = std::make_shared<Mesh>();
	const aiScene* scene = aiImportFile(filePath, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr && scene->HasMeshes()) {
		std::vector<glm::vec3> all_vertices;
		std::vector<unsigned int> all_indices;
		std::vector<glm::vec2> all_texCoords;
		std::vector<glm::vec3> all_normals;
		std::vector<glm::u8vec3> all_colors;

		unsigned int vertex_offset = 0;

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];


			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				all_vertices.push_back(glm::vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z));
			}


			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				aiFace& face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; k++) {
					all_indices.push_back(face.mIndices[k] + vertex_offset);
				}
			}


			if (mesh->HasTextureCoords(0)) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_texCoords.push_back(glm::vec2(mesh->mTextureCoords[0][j].x, -mesh->mTextureCoords[0][j].y));
				}
			}


			if (mesh->HasNormals()) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_normals.push_back(glm::vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z));
				}
			}

			if (mesh->HasVertexColors(0)) {
				for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
					all_colors.push_back(glm::u8vec3(mesh->mColors[0][j].r * 255, mesh->mColors[0][j].g * 255, mesh->mColors[0][j].b * 255));
				}
			}

			vertex_offset += mesh->mNumVertices;
		}

		mesh->load(all_vertices.data(), all_vertices.size(), all_indices.data(), all_indices.size());

		if (!all_texCoords.empty()) {
			mesh->loadTexCoords(all_texCoords.data(), all_texCoords.size());
		}

		if (!all_normals.empty()) {
			mesh->loadNormals(all_normals.data(), all_normals.size());
		}

		if (!all_colors.empty()) {
			mesh->loadColors(all_colors.data(), all_colors.size());
		}

		aiReleaseImport(scene);
	}
	return mesh;
}

void ModelImporter::SaveMeshToFile(const std::shared_ptr<Mesh>& mesh, const std::string& filePath)
{
	std::filesystem::path path(filePath);

	if (!std::filesystem::exists(path.parent_path())) {
		throw std::runtime_error("Directory does not exist: " + path.parent_path().string());
	}

	std::ofstream outFile(filePath, std::ios::out);
	if (!outFile.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	outFile << mesh;
	outFile.close();
}

std::shared_ptr<Mesh> ModelImporter::LoadModelFromFile(const std::string& filePath)
{
	std::ifstream inFile(filePath, std::ios::in);
	if (!inFile.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + filePath);
	}
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
	inFile >> mesh;
	inFile.close();
	return mesh;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Mesh>& mesh)
{
	if (!mesh) {
		return os;
	}
	const auto& vertices = mesh->vertices();
	os << vertices.size() << "\n";
	for (const auto& vertex : vertices) {
		os << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
	}
	const auto& indices = mesh->indices();
	os << indices.size() << "\n";
	for (const auto& index : indices) {
		os << index << "\n";
	}
	const auto& texCoords = mesh->texCoords();
	os << texCoords.size() << "\n";
	for (const auto& texCoord : texCoords) {
		os << texCoord.x << " " << texCoord.y << "\n";
	}

	const auto& boundingBox = mesh->boundingBox();
	os << boundingBox.min.x << " " << boundingBox.min.y << " " << boundingBox.min.z << "\n";
	os << boundingBox.max.x << " " << boundingBox.max.y << " " << boundingBox.max.z << "\n";

	return os;
}

std::istream& operator>>(std::istream& is, std::shared_ptr<Mesh>& mesh)
{
	if (!mesh) {
		mesh = std::make_shared<Mesh>();
	}
	size_t verticesSize;
	is >> verticesSize;
	std::vector<glm::vec3> vertices(verticesSize);
	for (auto& vertex : vertices) {
		is >> vertex.x >> vertex.y >> vertex.z;
	}
	size_t indicesSize;
	is >> indicesSize;
	std::vector<unsigned int> indices(indicesSize);
	for (auto& index : indices) {
		is >> index;
	}
	size_t texCoordsSize;
	is >> texCoordsSize;
	std::vector<glm::vec2> texCoords(texCoordsSize);
	for (auto& texCoord : texCoords) {
		is >> texCoord.x >> texCoord.y;
	}
	mesh->load(vertices.data(), vertices.size(), indices.data(), indices.size());
	if (!texCoords.empty()) {
		mesh->loadTexCoords(texCoords.data(), texCoords.size());
	}
	glm::vec3 min, max;
	is >> min.x >> min.y >> min.z;
	is >> max.x >> max.y >> max.z;
    BoundingBox boundingBox;
    boundingBox.min = min;
    boundingBox.max = max;

	return is;
}