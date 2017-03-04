#pragma once

#include "Light.hpp"

namespace qc
{
class PointLight : public Light
{
public:
	PointLight(const float radiusAttenuation = 1, const glm::vec3& position = glm::vec3(0), const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		: Light(position, color, intensity), radiusAttenuation(radiusAttenuation)
	{
		assert(radiusAttenuation != 0);
		this->computeAttenuationCoeff();
	}

	//-- GETTER ----------------------------

	float getRadiusAttenuation() const
		{return radiusAttenuation;}

	/*TODO : delete after test*/
	float& getRadiusAttenuation()
	{
		this->computeAttenuationCoeff();
		return radiusAttenuation;
	}

	float& getConstantAttenuation()
		{return constantAttenuation;}

	float& getLinearAttenuation()
		{return linearAttenuation;}

	float& getQuadraticAttenuation()
		{return quadraticAttenuation;}
	/* ---------------------- */
	
	//-- SETTERS --------------------------
	
	void setRadiusAttenuation(float radius)
	{
		radiusAttenuation = radius;
		this->computeAttenuationCoeff();
	}

private:
	float radiusAttenuation;
	float constantAttenuation = 1;
	float linearAttenuation = 1;
	float quadraticAttenuation = 1;

	//-- COMPUTE ATTENUATION COEFF-----------------
	/*
		Compute the attenuations coefficient according to the radius
	*/
	void computeAttenuationCoeff()
	{
		constantAttenuation = 1;
		linearAttenuation = 2 * getIntensity() / radiusAttenuation;
		quadraticAttenuation = getIntensity() / (radiusAttenuation * radiusAttenuation);
	}

};
}//! namespace qc