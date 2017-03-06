#pragma once

#include <glad/glad.h>

#include <glmlv/Image2DRGBA.hpp>

namespace qc
{

class Texture
{

public:
	Texture(){}

	Texture(const glmlv::Image2DRGBA& tex);

	~Texture()
		{if (pointer != 0) glDeleteTextures(1, &pointer);}
	
	Texture(const Texture& o) = delete;
	Texture& operator=(const Texture& o) = delete;

	Texture(Texture&& o);
	Texture& operator=(Texture&& o);

	//-- GETTERS -----------------------------
	GLuint getPointer() const
		{return pointer;}


private:
	//-- Pointer to the texture
	GLuint pointer = 0;

	//-- GENERATE TEXTURES -------------------
	static GLuint generateTexture(int width, int height, GLenum internalformat = GL_RGB32F, const unsigned char* data = 0);

};

}