#include <qc/graphic/Textures.hpp>

using namespace qc::graphic;

Texture::Texture(const glmlv::Image2DRGBA& tex)
{
	pointer = generateTexture(tex.width(), tex.height(), GL_RGB32F, tex.data());
}

Texture::Texture(Texture&& o)
{
	if (pointer != 0) glDeleteTextures(1, &pointer);
	pointer = o.pointer;
	o.pointer = 0;
}

Texture& Texture::operator=(Texture&& o)
{
	if (pointer != 0) glDeleteTextures(1, &pointer);
	pointer = o.pointer;
	o.pointer = 0;
	return *this;
}


//-- GENERATE TEXTURE -----------------

GLuint Texture::generateTexture(int width, int height, GLenum internalformat, const unsigned char* data)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, internalformat, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	if(data != 0) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}
