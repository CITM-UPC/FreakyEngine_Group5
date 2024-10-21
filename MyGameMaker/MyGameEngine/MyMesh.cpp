#include "MyMesh.h"

#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <IL/il.h>
#include <IL/ilu.h>
#include <string>


#define CHECKERS_HEIGHT 64
#define CHECKERS_WIDTH 64

// Función para convertir de const char* a const wchar_t*
std::wstring ConvertToWString(const char* charStr) {
    size_t newSize = strlen(charStr) + 1;
    wchar_t* wString = new wchar_t[newSize];
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wString, newSize, charStr, _TRUNCATE);
    std::wstring wStr(wString);
    delete[] wString;
    return wStr;
}


void MyMesh::InitBuffers() {

    glGenBuffers(1, &id_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, id_vertex);
    glBufferData(GL_ARRAY_BUFFER, num_vertex * 3 * sizeof(float), vertex, GL_STATIC_DRAW);

    glGenBuffers(1, &id_index);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_index);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_index * sizeof(unsigned int), index, GL_STATIC_DRAW);
}

void MyMesh::Draw() const {
    glEnableClientState(GL_VERTEX_ARRAY);

    // Enlazar el buffer de vértices y definir el puntero
    glBindBuffer(GL_ARRAY_BUFFER, id_vertex);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);

    if (id_texture != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, id_texture);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_index);
    //Discoment to see the mesh with color
    //glColor3ub(color.r, color.g, color.b);
    glDrawElements(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, nullptr);

    glDisableClientState(GL_VERTEX_ARRAY);

    // Deshabilitar la textura si estaba activada
    if (id_texture != 0) {
        glDisable(GL_TEXTURE_2D);
    }
}

// Función para cargar el archivo de la malla
void MyMesh::LoadFile(const char* file_path) {
    // Cargar la escena usando Assimp
    const aiScene* scene = aiImportFile(file_path, aiProcessPreset_TargetRealtime_MaxQuality);

    if (scene != nullptr && scene->HasMeshes()) {
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            num_vertex = scene->mMeshes[i]->mNumVertices;
            vertex = new float[num_vertex * 3];
            memcpy(vertex, scene->mMeshes[i]->mVertices, sizeof(float) * num_vertex * 3);

            if (scene->mMeshes[i]->HasFaces()) {
                num_index = scene->mMeshes[i]->mNumFaces * 3;
                index = new unsigned int[num_index];
                for (unsigned int j = 0; j < scene->mMeshes[i]->mNumFaces; ++j) {
                    if (scene->mMeshes[i]->mFaces[j].mNumIndices != 3) {
                        std::cout << "Warning: La geometría no es un triángulo." << std::endl;
                    }
                    else {
                        memcpy(&index[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(unsigned int));
                    }
                }
            }
            if (scene->HasMaterials()) {
                aiString texturePath;
                if (scene->mMaterials[scene->mMeshes[i]->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                    LoadTexture(texturePath.C_Str());
                }
            }
        }
    }
    else {
        std::cout << "Error: No se pudo cargar la escena correctamente." << std::endl;
    }
    //Descomentar por si peta al importar varias meshes 
    //aiReleaseImport(scene);
}


void MyMesh::LoadTexture(const char* texture_path) {
    // Inicializar DevIL
    ilInit();
    iluInit();

    // Generar una nueva imagen y cargarla
    ILuint il_img_id;
    ilGenImages(1, &il_img_id);
    ilBindImage(il_img_id);

    std::wstring wtexturepath = ConvertToWString(texture_path);
    if (!ilLoadImage(wtexturepath.c_str())) {
        std::cerr << "Error cargando la imagen: " << iluErrorString(ilGetError()) << std::endl;
        return;
    }

    // Obtener los datos de la imagen cargada
    int img_width = ilGetInteger(IL_IMAGE_WIDTH);
    int img_height = ilGetInteger(IL_IMAGE_HEIGHT);
    int img_bpp = ilGetInteger(IL_IMAGE_BPP); 
    int img_format = ilGetInteger(IL_IMAGE_FORMAT);
    unsigned char* img_data = ilGetData();

    // Generar la textura en OpenGL
    glGenTextures(1, &id_texture);
    glBindTexture(GL_TEXTURE_2D, id_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, img_bpp, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, img_data);

    // Configurar los parámetros de la textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    //ilInit();
    //iluInit();
    //auto il_img_id = ilGenImage();
    //ilBindImage(il_img_id);
    //const std::string& fileName = "Lenna.png";
    //ilLoadImage(fileName.c_str());

    //auto img_width = ilGetInteger(IL_IMAGE_WIDTH);
    //auto img_height = ilGetInteger(IL_IMAGE_HEIGHT);
    //auto img_bpp = ilGetInteger(IL_IMAGE_BPP);
    //auto img_format = ilGetInteger(IL_IMAGE_FORMAT);
    //auto img_data = ilGetData();

    ////Init Texture
    //for (int i = 0; i < texture.front().size(); i++)
    //{
    //    for (int j = 0; j < texture.size(); j++)
    //    {
    //        const glm::u8vec3 whiteColor(255, 255, 255);
    //        const glm::u8vec3 blackColor(0, 0, 0);
    //        texture[i][j] = (i / 8 + j / 8) % 2 == 0 ? whiteColor : blackColor;
    //    }
    //}
    //glGenTextures(1, &id_texture);
    //glBindTexture(GL_TEXTURE_2D, id_texture);
    //glTexImage2D(GL_TEXTURE_2D, 0, img_bpp, img_width, img_height, 0, img_format, GL_UNSIGNED_BYTE, img_data);
    //ilDeleteImage(il_img_id);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
}

// Destructor para liberar memoria
MyMesh::~MyMesh() {
    if (vertex != nullptr) {
        delete[] vertex;
    }
    if (index != nullptr) {
        delete[] index;
    }
}
