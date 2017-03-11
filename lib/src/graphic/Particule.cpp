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
	// init shape and buffers
	glmlv::SimpleGeometry sp = glmlv::makeSphere(4);
//	glmlv::SimpleGeometry sp = glmlv::makeCube();
	std::vector<glmlv::Vertex3f3f2f> vertices = sp.vertexBuffer;
	std::vector<uint32_t> index = sp.indexBuffer;

	initBuffers(vertices, index);

	// init materials
	std::vector<std::shared_ptr<Material>> materials;
	materials.push_back(mat);
	setMaterials(materials);
	if (pointLight)
	{
		setPosition(pointLight->getPosition());
		pointLight->setColor(mat->getColor(Material::EMMISIVE_COLOR));
	}

	// init shape object
	ShapeData shapeData = ShapeData(index.size(), 0, mat);
	std::vector<ShapeData> shapes = { shapeData };
	setShapesData(shapes);

	setScale(glm::vec3(radius));
}
