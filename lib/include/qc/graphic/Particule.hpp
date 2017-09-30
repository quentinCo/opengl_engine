#pragma once

#include <glmlv/simple_geometry.hpp>

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
	Particule(std::shared_ptr<Material> mat, float radius = 1, PointLight* pointLight = nullptr, int pointLightIndex = -1)
		:Mesh(), radius(radius), pointLight(pointLight), pointLightIndex(pointLightIndex)
	{initShape(mat);}

	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;

	//-- GETTERS ---------------------------
	float getRadius() const
		{return radius;}

	float getIntensity() const
		{return (pointLight) ? pointLight->getIntensity() : 0;}

	float getRadiusAttenuation() const
		{return (pointLight) ? pointLight->getRadiusAttenuation() : 0;}

	int getPointLightIndex() const
		{return pointLightIndex;}
	
	//-- SETTERS ---------------------------
	void setRadius(float radius)
	{
		this->radius = radius;
		setScale(glm::vec3(radius));
	}

	void setPointLight(PointLight* light)
		{pointLight = light;}

	virtual void setPosition(const glm::vec4& position);

private:
	//--
	float radius;

	//-- pointer to a pointlight if lighted particule
	PointLight* pointLight;
	int pointLightIndex;

	//-- INIT SHAPE ------------------------
	/*
		Init the vbo, ibo, vao, shape and material for the particule
	*/
	void initShape(std::shared_ptr<Material> mat);
	
};

} //! namespace graphic

} //!namespace qc