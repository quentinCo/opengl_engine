#pragma once

#include "PointLight.hpp"
#include "Mesh.hpp"

namespace qc
{

class Particule : public Mesh
{

public:
	Particule(PointLight* pointLight = nullptr)
		:Mesh(), pointLight(pointLight)
	{initShape();}
	/*
	Particule(const Particule& o)
		:Mesh(), pointLight(o.pointLight)
	{initShape();}

	Particule& operator= (const Particule& o);
	*/
	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;

	//void setPosition(const glm::vec4& position);
	
private:
	PointLight* pointLight;
	glm::vec3 normal;

	void initShape();
	
};

} //!namespace qc