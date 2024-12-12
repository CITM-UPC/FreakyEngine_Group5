#pragma once

#include <memory>
#include "Image.h"
#include <string>

class Texture
{
public:
	enum WrapModes { Repeat, MirroredRepeat, Clamp };
	WrapModes wrapMode = Repeat;

	enum Filters { Nearest, Linear };
	Filters filter = Nearest;

private:
	std::shared_ptr<Image> _img_ptr;

public:

    std::string path;
	unsigned int id() const { return _img_ptr ? _img_ptr->id() : 0; }
	void bind() const;
	void setImage(const std::shared_ptr<Image>& img_ptr) { _img_ptr = img_ptr; }
	const auto& image() const { return *_img_ptr; }
	std::shared_ptr<Image> imagePtr() const { return _img_ptr; }
};

