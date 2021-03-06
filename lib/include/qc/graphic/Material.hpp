#pragma once

#include <glad/glad.h>

#include <glm/vec3.hpp>

#include <glmlv/Image2DRGBA.hpp>

#include <map>

#include "Textures.hpp"

namespace qc
{

namespace graphic
{

class Material
{

public:
	using SharedTexture = std::shared_ptr<Texture>;

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
		NORMAL_TEXTURE,
		NB_TEXTURE
	};

	Material();


	//-- GETTERS ---------------------------
	
	const glm::vec3& getColor(const MATERIALS_COLOR& color) const
		{return colors[color];}

	float getShininess() const
		{return shininess;}

	SharedTexture getTexture(const MATERIALS_TEXTURE& map) const
		{return (texturePointers[map]) ? texturePointers[map] : defaultTex;}
	
	
	//-- SETTERS --------------------------
	
	void setColor(const MATERIALS_COLOR& color, const glm::vec3& value)
	{
		if (color != NB_COLOR)
			colors[color] = value;
	}

	void setShininess(float shininess)
		{this->shininess = shininess;}

	void setTexture(const MATERIALS_TEXTURE& map, SharedTexture texIndex)
		{texturePointers[map] = texIndex;}

private:
	//-- material colors
	glm::vec3 colors[NB_COLOR];

	float shininess;
	
	//-- material textures
	SharedTexture texturePointers[NB_TEXTURE];
	static SharedTexture defaultTex;
};

} //!namespace graphic

} //! namespace qc