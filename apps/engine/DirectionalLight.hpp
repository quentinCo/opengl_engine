#pragma once

#include "Light.hpp"

namespace qc
{
class DirectionalLight : public Light
{
public:
	DirectionalLight(float phi = 90.f, float theta = 45.f, const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		: Light(glm::vec4(0), color, intensity), phiAngle(phi), thetaAngle(theta)
	{
		this->setPosition(this->computeDirection(phi, theta));
	}


	//-- GETTERS ----------------------------

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


	//-- SETTERS ---------------------------

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
	//-- Orientation angles
	float phiAngle;
	float thetaAngle;

	//-- COMPUTE DIRECTION -----------------
	/*
		Compute the directionnal light vector according to phi and theta.
	*/
	static glm::vec4 computeDirection(float phi, float theta);
};
}//! namespace qc