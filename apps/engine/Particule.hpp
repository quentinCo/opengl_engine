#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "PointLight.hpp"
#include "Mesh.hpp"

namespace qc
{

class Particule : public PointLight
{

public:
	Particule(const float radiusAttenuation = 1, const glm::vec3& position = glm::vec3(0), const glm::vec3& color = glm::vec3(1), float intensity = 1.f)
		:PointLight(radiusAttenuation, position, color, intensity), particuleShape()
	{initShape();}
	
	Particule(const Particule& o)
		:PointLight(o)
	{initShape();}

	Particule& operator= (const Particule& o);

	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;

	//void setPosition(const glm::vec4& position);
	void setRotation(float angle, const glm::vec3& axis);
	
private:
	Mesh particuleShape;
	
	void initShape();
	
};

} //!namespace qc