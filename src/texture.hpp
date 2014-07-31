#pragma once
#include "image.hpp"

class Texture {
public:
	unsigned int id;

	Texture();
	~Texture();

	void init();
	void upload(Image& image);
	void draw();
};