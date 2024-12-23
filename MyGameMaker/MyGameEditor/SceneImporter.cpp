#include "SceneImporter.h"
#include <map>
#include <filesystem>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include "ImageImporter.h"
#include "MyGameEngine/Component.h"
#include "MyGameEngine/Material.h"
#include "SceneManager.h"
using namespace std;
namespace fs = std::filesystem;
vec3 _translation = vec3(0.0f);	
glm::quat _rotation;

static mat4 aiMat4ToMat4(const aiMatrix4x4& aiMat) {
	mat4 mat;
	for (int c = 0; c < 4; ++c) for (int r = 0; r < 4; ++r) mat[c][r] = aiMat[r][c];
	return mat;
}

static void decomposeMatrix(const mat4& matrix, vec3& scale, glm::quat& rotation, vec3& translation) {
	// Extraer traslación
	translation = vec3(matrix[3]);
	// Extraer escala con control manual para evitar valores cercanos a cero
	scale = vec3(
		glm::length(vec3(matrix[0])) > 0.001f ? glm::length(vec3(matrix[0])) : 0.001f,
		glm::length(vec3(matrix[1])) > 0.001f ? glm::length(vec3(matrix[1])) : 0.001f,
		glm::length(vec3(matrix[2])) > 0.001f ? glm::length(vec3(matrix[2])) : 0.001f
	);
	// Remover escala de la matriz para extraer rotación
	mat4 rotationMatrix = matrix;
	if (scale.x > 0.001f) rotationMatrix[0] /= scale.x;
	if (scale.y > 0.001f) rotationMatrix[1] /= scale.y;
	if (scale.z > 0.001f) rotationMatrix[2] /= scale.z;
	rotation = glm::quat_cast(rotationMatrix);
}


bool containsSubstring(const std::string& str, const std::string& substr) {
	return str.find(substr) != std::string::npos;
}

static mat4 adjustFBXRotation(const mat4& matrix) {
	// Rotar el sistema de coordenadas del FBX para ajustarlo
	mat4 adjustment = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	return adjustment * matrix;
}

static glm::vec3 quaternionToEuler(const glm::quat& q) {
	glm::vec3 euler;

	// Roll (X-axis rotation)
	float sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	float cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	euler.x = glm::atan(sinr_cosp, cosr_cosp);

	// Pitch (Y-axis rotation)
	float sinp = 2 * (q.w * q.y - q.z * q.x);
	if (std::abs(sinp) >= 1)
		euler.y = std::copysign(glm::half_pi<float>(), sinp); // Clamp to ±90 degrees
	else
		euler.y = glm::asin(sinp);

	// Yaw (Z-axis rotation)
	float siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	float cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	euler.z = glm::atan(siny_cosp, cosy_cosp);

	return euler;
}


GameObject graphicObjectFromNode(const aiScene& scene, const aiNode& node, const vector<shared_ptr<Mesh>>& meshes, const vector<shared_ptr<Material>>& materials, bool isFromStreet2, const std::string& path){	
	
	GameObject obj;

	mat4 localMatrix = aiMat4ToMat4(node.mTransformation);
	if (isFromStreet2) {
		//localMatrix = adjustFBXRotation(localMatrix);
	}

	vec3 scale, translation;
	glm::quat rotation;
	decomposeMatrix(localMatrix, scale, rotation, translation);

	// Convert quaternion to Euler angles
	glm::vec3 euler = quaternionToEuler(rotation);
	float pitch = glm::degrees(euler.x);
	float yaw = glm::degrees(euler.y);
	float roll = glm::degrees(euler.z);

	obj.name = node.mName.C_Str();

	if (containsSubstring(obj.name, "$AssimpFbx$_Translation")) {
		_translation = translation;
	}
	if (containsSubstring(obj.name, "$AssimpFbx$_Rotation")) {
		_rotation = rotation;
	}
	if (!containsSubstring(obj.name, "$AssimpFbx$")) {
		obj.GetComponent<TransformComponent>()->transform().setPos(_translation.x, _translation.y, _translation.z);
		//obj.GetComponent<TransformComponent>()->transform().setRotation(pitch, yaw, roll);

		if (isFromStreet2) {
			obj.GetComponent<TransformComponent>()->transform().rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		_translation = vec3(0.0f);
		_rotation = glm::quat();
	}

	for (unsigned int i = 0; i < node.mNumMeshes; ++i) {
		const auto* fbx_mesh = scene.mMeshes[node.mMeshes[i]];
		auto mesh = meshes[node.mMeshes[i]];
		auto material = materials[fbx_mesh->mMaterialIndex];
		obj.setMesh(mesh);
		obj.texture() = material->texture;
		obj.setTextureImage(material->texture.imagePtr());
		obj.setMaterial(material);
		obj.modelPath = path;
		obj.texturePath = material->texture.path;
		SceneManager::gameObjectsOnScene.push_back(obj);
	}

	for (unsigned int i = 0; i < node.mNumChildren; ++i) {
		graphicObjectFromNode(scene, *node.mChildren[i], meshes, materials, isFromStreet2,path);
	}

	return obj;
}

static vector<shared_ptr<Mesh>> createMeshesFromFBX(const aiScene& scene) {
	vector<shared_ptr<Mesh>> meshes;
	for (unsigned int i = 0; i < scene.mNumMeshes; ++i) {
		const aiMesh* fbx_mesh = scene.mMeshes[i];
		auto mesh_ptr = make_shared<Mesh>();

		vector<unsigned int> indices(fbx_mesh->mNumFaces * 3);
		for (unsigned int j = 0; j < fbx_mesh->mNumFaces; ++j) {
			indices[j * 3 + 0] = fbx_mesh->mFaces[j].mIndices[0];
			indices[j * 3 + 1] = fbx_mesh->mFaces[j].mIndices[1];
			indices[j * 3 + 2] = fbx_mesh->mFaces[j].mIndices[2];
		}

		mesh_ptr->load(reinterpret_cast<const glm::vec3*>(fbx_mesh->mVertices), fbx_mesh->mNumVertices, indices.data(), indices.size());
		if (fbx_mesh->HasTextureCoords(0)) {
			vector<glm::vec2> texCoords(fbx_mesh->mNumVertices);
			for (unsigned int j = 0; j < fbx_mesh->mNumVertices; ++j) texCoords[j] = glm::vec2(fbx_mesh->mTextureCoords[0][j].x, fbx_mesh->mTextureCoords[0][j].y);
			mesh_ptr->loadTexCoords(texCoords.data(), texCoords.size());
		}
		if (fbx_mesh->HasNormals()) mesh_ptr->loadNormals(reinterpret_cast<glm::vec3*>(fbx_mesh->mNormals), fbx_mesh->mNumVertices);
		if (fbx_mesh->HasVertexColors(0)) {
			vector<glm::u8vec3> colors(fbx_mesh->mNumVertices);
			for (unsigned int j = 0; j < fbx_mesh->mNumVertices; ++j) colors[j] = glm::u8vec3(fbx_mesh->mColors[0][j].r * 255, fbx_mesh->mColors[0][j].g * 255, fbx_mesh->mColors[0][j].b * 255);
			mesh_ptr->loadColors(colors.data(), colors.size());
		}
		meshes.push_back(mesh_ptr);
	}
	return meshes;
}

static vector<shared_ptr<Material>> createMaterialsFromFBX(const aiScene& scene, const fs::path& basePath) {

	vector<shared_ptr<Material>> materials;
	map<string, shared_ptr<Image>> images;

	for (unsigned int i = 0; i < scene.mNumMaterials; ++i) {
		const auto* fbx_material = scene.mMaterials[i];
		auto material = make_shared<Material>();

		if (fbx_material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString texturePath;
			fbx_material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
			const string textureFileName = fs::path(texturePath.C_Str()).filename().string();
			const auto image_itr = images.find(textureFileName);
			material.get()->texture.path = texturePath.C_Str();
			if (image_itr != images.end()) material->texture.setImage(image_itr->second);
			else {
				auto image = ImageImporter::loadFromFile((basePath / textureFileName).string());
				images.insert({ textureFileName, image });
				material->texture.setImage(image);
			}

			auto uWrapMode = aiTextureMapMode_Wrap;
			auto vWrapMode = aiTextureMapMode_Wrap;
			fbx_material->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(0), uWrapMode);
			fbx_material->Get(AI_MATKEY_MAPPINGMODE_V_DIFFUSE(0), vWrapMode);
			assert(uWrapMode == aiTextureMapMode_Wrap);
			assert(vWrapMode == aiTextureMapMode_Wrap);

			unsigned int flags = 0;
			fbx_material->Get(AI_MATKEY_TEXFLAGS_DIFFUSE(0), flags);
			assert(flags == 0);
		}

		aiColor4D color;
		fbx_material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->color = color4(color.r * 255, color.g * 255, color.b * 255, color.a * 255);

		materials.push_back(material);
	}
	return materials;
}

GameObject SceneImporter::loadFromFile(const std::string& path) {
	const aiScene* fbx_scene = aiImportFile(path.c_str(), aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_JoinIdenticalVertices | aiProcess_GenUVCoords | aiProcess_TransformUVCoords | aiProcess_FlipUVs);
	const auto meshes = createMeshesFromFBX(*fbx_scene);
	const auto materials = createMaterialsFromFBX(*fbx_scene, fs::absolute(path).parent_path());
	GameObject fbx_obj;
	if (path == "Assets/street2.fbx") {
		 fbx_obj = graphicObjectFromNode(*fbx_scene, *fbx_scene->mRootNode, meshes, materials, true,path);
	}
	else {
		fbx_obj = graphicObjectFromNode(*fbx_scene, *fbx_scene->mRootNode, meshes, materials,false,path);

	}
	fbx_obj.modelPath = path;
	if (fbx_obj.material().get() != nullptr) {
		fbx_obj.texturePath = fbx_obj.material().get()->texture.path;

	}
	aiReleaseImport(fbx_scene);
	return fbx_obj;
}

