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
		: position(position), color(color), intensity(intensity)
	{}

	const glm::vec3& getPosition() const
		{return position;}

	const glm::vec3& getColor() const
		{return color;}

	float getIntensity() const
		{return intensity;}

	/*TODO : delete after test*/
	glm::vec3& getPosition()
		{return position;}

	glm::vec3& getColor()
		{return color;}

	float& getIntensity()
		{return intensity;}
	/* ---------------------- */
	void setPosition(const glm::vec3& position)
		{this->position = position;}

	void setColor(const glm::vec3& color)
		{this->color = color;}

	void setIntensity(float intensity)
		{this->intensity = intensity;}

private:
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};


/*-- Directional Light --*/

class DirectionalLight : public Light
{

	float m_DirLightPhiAngleDegrees = 90.f;
	float m_DirLightThetaAngleDegrees = 45.f;
public:
	DirectionalLight(float phi = 90.f, float theta = 45.f, const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		: Light(glm::vec3(0), color, intensity), phiAngle(phi), thetaAngle(theta)
	{
		this->setPosition(this->computeDirection(phi, theta));
	}

	float getPhiAngle() const
		{return phiAngle;}

	float getThetaAngle() const
		{return thetaAngle;}

	const glm::vec3& getDirection() const
		{return this->getPosition();}

	/*TODO : delete after test*/
	float& getPhiAngle()
		{return phiAngle;}

	float& getThetaAngle()
		{return thetaAngle;}
	/* ---------------------- */

	void setPhiAngle(float phi)
	{
		phiAngle = phi;
		this->setDirection(phi, thetaAngle);
	}

	void setThetaAngle(float theta)
	{
		thetaAngle = theta;
		this->setDirection(phiAngle, theta);
	}

	void setDirection(float phi, float theta)
		{this->setPosition(this->computeDirection(phi, theta));}

private:
	float phiAngle;
	float thetaAngle;

	static glm::vec3 computeDirection(float phi, float theta);
};

}