#include "TextureImporter.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include "MyGameEngine/Texture.h"
#include <memory>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "MyGameEngine/Image.h"

static GLenum formatFromChannels(int channels) {
    switch (channels) {
    case 1: return GL_LUMINANCE;
    case 2: return GL_LUMINANCE_ALPHA;
    case 3: return GL_RGB;
    case 4: return GL_RGBA;
    default: return GL_RGB;
    }
}

bool TextureImporter::initialize() {
    ilInit();
    iluInit();
    ILenum err = ilGetError();
    if (err != IL_NO_ERROR) {
        return false;
    }
    return true;
}

std::unique_ptr<Image> TextureImporter::loadTexture(const std::string& filePath) {
    // Convert std::string (filePath) to std::wstring
    size_t size = filePath.size() + 1; // Include null terminator
    std::vector<wchar_t> wFilePath(size);
    size_t convertedChars = 0;
    mbstowcs_s(&convertedChars, wFilePath.data(), size, filePath.c_str(), size - 1);

    // DevIL image ID
    ILuint imgId;
    ilGenImages(1, &imgId);
    ilBindImage(imgId);

    // Load the image using wchar_t* (wide character string)
    if (!ilLoadImage((const wchar_t*)filePath.c_str())) {
        ILenum err = ilGetError();
        ilDeleteImages(1, &imgId);
        return nullptr;
    }

    // Get image properties
    int width = ilGetInteger(IL_IMAGE_WIDTH);
    int height = ilGetInteger(IL_IMAGE_HEIGHT);
    int channels = ilGetInteger(IL_IMAGE_CHANNELS);
    ILubyte* data = ilGetData();

    if (!data) {
        ilDeleteImages(1, &imgId);
        return nullptr;
    }

    // Create an Image instance and load the texture data
    auto image = std::make_unique<Image>();
    image->load(width, height, channels, data);

    // Free DevIL resources
    ilDeleteImages(1, &imgId);

    return image;
}

void TextureImporter::SaveTextureToFile(const std::shared_ptr<Image>& image, const std::string& filePath) {
    if (!image) {
        throw std::runtime_error("Cannot save a null image.");
    }

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }

    // Write image properties (width, height, channels)
    int width = image->width();
    int height = image->height();
    int channels = image->channels();
    outFile.write(reinterpret_cast<char*>(&width), sizeof(width));
    outFile.write(reinterpret_cast<char*>(&height), sizeof(height));
    outFile.write(reinterpret_cast<char*>(&channels), sizeof(channels));

    // Bind the texture and validate
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, formatFromChannels(channels), width, height, 0, formatFromChannels(channels), GL_UNSIGNED_BYTE, image->data());

    // Write image data
    const int dataSize = width * height * channels;
    std::vector<unsigned char> data(dataSize);
    glGetTexImage(GL_TEXTURE_2D, 0, formatFromChannels(channels), GL_UNSIGNED_BYTE, data.data());

    if (data.empty()) {
        throw std::runtime_error("Failed to retrieve texture data.");
    }

    outFile.write(reinterpret_cast<char*>(data.data()), dataSize);

    glDeleteTextures(1, &textureID);
    outFile.close();
}

std::shared_ptr<Image> TextureImporter::LoadTextureFromFile(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + filePath);
    }

    // Read image properties
    int width, height, channels;
    inFile.read(reinterpret_cast<char*>(&width), sizeof(width));
    inFile.read(reinterpret_cast<char*>(&height), sizeof(height));
    inFile.read(reinterpret_cast<char*>(&channels), sizeof(channels));

    if (width <= 0 || height <= 0 || channels <= 0) {
        throw std::runtime_error("Invalid texture properties read from file.");
    }

    // Read image data
    const int dataSize = width * height * channels;
    std::vector<unsigned char> data(dataSize);
    inFile.read(reinterpret_cast<char*>(data.data()), dataSize);

    if (!inFile || data.empty()) {
        throw std::runtime_error("Failed to read image data from file: " + filePath);
    }

    // Load data into Image object
    std::shared_ptr<Image> image = std::make_shared<Image>();
    image->load(width, height, channels, data.data());

    inFile.close();
    return image;
}


