#include "Material.hpp"

using namespace qc;

Material::Material()
	: shininess(0)
{
	for (auto& it : colors)
		it = glm::vec3(0);

	colors[DIFFUSE_COLOR] = glm::vec3(1);

	for (auto& it : textures)
		it = 0;
}

Material::~Material()
{
	for (auto& it : textures)
		if (it) glDeleteTextures(1, &it);
}

Material::Material(const Material& o)
	: shininess(o.shininess)
{
	for (size_t i = 0; i < NB_COLOR; ++i)
		colors[i] = o.colors[i];
}

Material& Material::operator= (const Material& o)
{
	for (size_t i = 0; i < NB_COLOR; ++i)
		colors[i] = o.colors[i];

	shininess = o.shininess;

	return *this;
}

Material::Material(Material&& o)
	: shininess(o.shininess)
{
	for (size_t i = 0; i < NB_COLOR; ++i)
		colors[i] = std::move(o.colors[i]);
	
	for (size_t i = 0; i < NB_TEXTURE; ++i)
	{
		if (textures[i]) glDeleteTextures(1, &textures[i]);
		textures[i] = o.textures[i];
		o.textures[i] = 0;
	}
}

Material& Material::operator= (Material&& o)
{
	for (size_t i = 0; i < NB_COLOR; ++i)
		colors[i] = std::move(o.colors[i]);

	shininess = o.shininess;

	for (size_t i = 0; i < NB_TEXTURE; ++i)
	{
		if (textures[i]) glDeleteTextures(1, &textures[i]);
		textures[i] = o.textures[i];
		o.textures[i] = 0;
	}

	return *this;
}

void Material::setMap(const MATERIALS_TEXTURE& map, const glmlv::Image2DRGBA& tex)
{
	GLuint& texture = textures[map];

	if (texture != 0)
		glDeleteTextures(1, &texture);

	generateTexture(texture, tex);
}

void Material::generateTexture(GLuint& texture, const glmlv::Image2DRGBA& tex)
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, static_cast<GLsizei>(tex.width()), static_cast<GLsizei>(tex.height()));
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(tex.width()), static_cast<GLsizei>(tex.height()), GL_RGBA, GL_UNSIGNED_BYTE, tex.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}
