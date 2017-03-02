#pragma once

#include "PointLight.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

namespace qc
{

class Particule : public Mesh
{

public:
	Particule(Camera* camera, PointLight* pointLight = nullptr)
		:Mesh(), camera(camera), pointLight(pointLight)
	{
		assert(camera);
		initShape();
	}
	/*
	Particule(const Particule& o)
		:Mesh(), pointLight(o.pointLight)
	{initShape();}

	Particule& operator= (const Particule& o);
	*/
	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;

	virtual void setPosition(const glm::vec4& position);
	virtual void setRotation(const float angle, const glm::vec3& axis);

	//void updateOrientation();
	
private:
	PointLight* pointLight;
	Camera* camera;

	glm::vec3 upVector = glm::vec3(0, 1, 0);
	glm::vec3 leftVector = glm::vec3(1, 0, 0);
	glm::vec3 normal = glm::vec3(0, 0, -1);

	void initShape();
	
};

} //!namespace qc