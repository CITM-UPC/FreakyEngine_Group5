#pragma once

#include <ostream>
#include <istream>
#include <string>
#include <GL/glew.h>

class Image {

	unsigned int _id = 0;
	unsigned short _width = 0;
	unsigned short _height = 0;
	unsigned char _channels = 0;

public:
	unsigned int id() const { return _id; }
	auto width() const { return _width; }
	auto height() const { return _height; }
	auto channels() const { return _channels; }

	Image() = default;
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;
	Image(Image&& other) noexcept;
	Image& operator=(Image&& other) noexcept = delete;
	~Image();

	void bind() const;
	void load(int width, int height, int channels, void* data);
	void loadTexture(const std::string& path);
	static GLenum formatFromChannels(int channels) {
		switch (channels) {
		case 1: return GL_LUMINANCE;
		case 2: return GL_LUMINANCE_ALPHA;
		case 3: return GL_RGB;
		case 4: return GL_RGBA;
		default: return GL_RGB;
		}
	}
};

std::ostream& operator<<(std::ostream& os, const Image& img);
std::istream& operator>>(std::istream& is, Image& img);


