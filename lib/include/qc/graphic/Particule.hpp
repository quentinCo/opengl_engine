#pragma once

#include "PointLight.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"

namespace qc
{
	
namespace graphic
{

class Particule : public Mesh
{

public:
	Particule(std::shared_ptr<Material> mat, float radius = 1, PointLight* pointLight = nullptr)
		:Mesh(), radius(radius), pointLight(pointLight)
	{initShape(mat);}

	~Particule() {}

	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;

	//-- GETTERS ---------------------------
	float getRadius() const
		{return radius;}

	float getIntensity() const
		{return (pointLight) ? pointLight->getIntensity() : 0;}

	float getRadiusAttenuation() const
		{return (pointLight) ? pointLight->getRadiusAttenuation() : 0;}
	
	//-- SETTERS ---------------------------
	void setRadius(float radius)
	{
		this->radius = radius;
		setScale(glm::vec3(radius));
	}

	virtual void setPosition(const glm::vec4& position);

private:
	//--
	float radius;

	//-- pointer to a pointlight if lighted particule
	PointLight* pointLight;

	//-- INIT SHAPE ------------------------
	/*
		Init the vbo, ibo, vao, shape and material for the particule
	*/
	void initShape(std::shared_ptr<Material> mat);
	
};

} //! namespace graphic

} //!namespace qc