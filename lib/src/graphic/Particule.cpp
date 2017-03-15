#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp> 

#include <qc/graphic/Particule.hpp>

using namespace qc::graphic;

//-- SETTER ----------------------------

void Particule::setPosition(const glm::vec4& position)
{
	Mesh::setPosition(position);
	if (pointLight)
		pointLight->setPosition(glm::vec4(getPosition(), 1));
}


//-- INIT SHAPE -------------------------

void Particule::initShape(std::shared_ptr<Material> mat)
{
	// TODO : change for Billboard Particules
	glmlv::SimpleGeometry sp = glmlv::makeSphere(4);

	// init materials
	std::vector<std::shared_ptr<Material>> materials;
	materials.push_back(mat);
	if (pointLight)
	{
		setPosition(pointLight->getPosition());
		pointLight->setColor(mat->getColor(Material::EMMISIVE_COLOR));
	}
	
	// init shape object
	ShapeData shapeData = ShapeData(sp.indexBuffer.size(), 0, mat);
	std::vector<ShapeData> shapes = { shapeData };

	// init shape and buffers
	std::vector<Vertex> completVertices;
	for (const auto& it : sp.vertexBuffer)
		completVertices.emplace_back(it);

	initBuffers(completVertices, sp.indexBuffer);

	setMaterials(materials);
	setShapesData(shapes);

	setScale(glm::vec3(radius));
}
