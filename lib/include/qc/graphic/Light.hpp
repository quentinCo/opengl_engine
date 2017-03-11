#pragma once

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace qc
{

namespace graphic
{

class Light
{
public:
	Light(const glm::vec3& position = glm::vec3(0), const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		: position(position, 1), color(color), intensity(intensity)
	{}


	//-- GETTERS ----------------------------

	const glm::vec4& getPosition() const
		{return position;}

	const glm::vec3& getColor() const
		{return color;}

	float getIntensity() const
		{return intensity;}

	/*TODO : delete after test*/
	glm::vec4& getPosition()
		{return position;}

	glm::vec3& getColor()
		{return color;}

	float& getIntensity()
		{return intensity;}
	/* ---------------------- */


	//-- SETTERS ---------------------------
	
	void setPosition(const glm::vec4& position)
		{this->position = position;}

	void setColor(const glm::vec3& color)
		{this->color = color;}

	void setIntensity(float intensity)
		{this->intensity = intensity;}

private:
	//-- position
	glm::vec4 position;

	//-- light color
	glm::vec3 color;

	//-- light intensity
	float intensity;
};
} //! namespace graphic
} //! namespace qc