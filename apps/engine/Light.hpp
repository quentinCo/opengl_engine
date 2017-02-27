#pragma once

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace qc
{
/*-- Light --*/
class Light
{
public:
	Light(const glm::vec3& position = glm::vec3(0), const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		: position(position, 1), color(color), intensity(intensity)
	{}

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
	void setPosition(const glm::vec4& position)
		{this->position = position;}

	void setColor(const glm::vec3& color)
		{this->color = color;}

	void setIntensity(float intensity)
		{this->intensity = intensity;}

private:
	glm::vec4 position;
	glm::vec3 color;
	float intensity;
};
}