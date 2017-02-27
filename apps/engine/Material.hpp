#pragma once

#include <glad/glad.h>

#include <glm/vec3.hpp>

#include <glmlv/Image2DRGBA.hpp>

#include <map>

namespace qc
{

class Material
{

public:
	enum MATERIALS_COLOR
	{
		AMBIENT_COLOR = 0,
		DIFFUSE_COLOR,
		SPECULAR_COLOR,
		EMMISIVE_COLOR,
		NB_COLOR
	};

	enum MATERIALS_TEXTURE
	{
		AMBIENT_TEXTURE = 0,
		DIFFUSE_TEXTURE,
		SPECULAR_TEXTURE,
		SPECULAR_HIGHT_LIGHT_TEXTURE,
		NB_TEXTURE
	};

	Material();
	
	~Material();

	Material(const Material& o);
	Material& operator= (const Material& o);

	Material(Material&& o);
	Material& operator= (Material&& o);

	const glm::vec3& getColor(const MATERIALS_COLOR& color) const
		{return colors[color];}

	float getShininess() const
		{return shininess;}

	GLuint getMap(const MATERIALS_TEXTURE& map) const
		{return textures[map];}
	
	void setColor(const MATERIALS_COLOR& color, const glm::vec3& value)
	{
		if (color != NB_COLOR)
			colors[color] = value;
	}

	void setShininess(float shininess)
		{this->shininess = shininess;}

	void setMap(const MATERIALS_TEXTURE& map, const glmlv::Image2DRGBA& tex);

private:
	glm::vec3 colors[NB_COLOR];
	float shininess;
	GLuint textures[NB_TEXTURE];

	static void generateTexture(GLuint& texture, const glmlv::Image2DRGBA& tex);

};

} // namespace qc