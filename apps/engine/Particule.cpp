#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp> 

#include "Particule.hpp"

using namespace qc;

//-- SETTER ----------------------------

void Particule::setPosition(const glm::vec4& position)
{
	Mesh::setPosition(position);
	if (pointLight)
		pointLight->setPosition(glm::vec4(getPosition(), 1));
}


//-- INIT SHAPE -------------------------

void Particule::initShape()
{
	float radius = /*(pointLight != nullptr) ? (100 * pointLight->getRadiusAttenuation() / pointLight->getIntensity()) :*/ 1;

	// TODO : change for Billboard Particules
	// init shape and buffers
	glmlv::SimpleGeometry sp = glmlv::makeSphere(6);
	std::vector<glmlv::Vertex3f3f2f> vertices = sp.vertexBuffer;
	std::vector<uint32_t> index = sp.indexBuffer;

	initBuffers(vertices, index);

	// init materials
	std::vector<std::shared_ptr<Material>> materials;
	materials.push_back(std::make_unique<Material>());

	auto& mat = materials.back();
	if (pointLight != nullptr)
	{
		setPosition(pointLight->getPosition());
		mat->setColor(Material::EMMISIVE_COLOR, pointLight->getColor());
	}
	else
		mat->setColor(Material::EMMISIVE_COLOR, glm::vec3(1));

	setMaterials(materials);

	// init shape object
	ShapeData shapeData = ShapeData(index.size(), 0, mat);
	std::vector<ShapeData> shapes = { shapeData };
	setShapesData(shapes);

	setScale(glm::vec3(radius));
}
