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
	Particule(std::shared_ptr<Material> mat, PointLight* pointLight = nullptr)
		:Mesh(), pointLight(pointLight)
	{initShape(mat);}

	Particule(Particule&& o) = default;
	Particule& operator= (Particule&& o) = default;
	
	//-- SETTERS ---------------------------

	virtual void setPosition(const glm::vec4& position);

private:
	//-- pointer to a pointlight if lighted particule
	PointLight* pointLight;
	
	static std::shared_ptr<Material> defaultMaterial;

	//-- INIT SHAPE ------------------------
	/*
		Init the vbo, ibo, vao, shape and material for the particule
	*/
	void initShape(std::shared_ptr<Material> mat);
	
};

} //! namespace graphic

} //!namespace qc