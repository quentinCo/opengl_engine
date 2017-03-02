#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp> 

#include "Particule.hpp"

using namespace qc;

/*
void Particule::setPosition(const glm::vec4& position)
{
	particuleShape.setPosition(position);
	//PointLight::setPosition(position);
}*/

void Particule::setPosition(const glm::vec4& position)
{
	Mesh::setPosition(position);
	if (pointLight)
		pointLight->setPosition(glm::vec4(getPosition(), 1));
}

void Particule::setRotation(const float angle, const glm::vec3& axis)
{
	Mesh::setRotation(angle, axis);
	normal = glm::rotate(normal, angle, axis);
	std::cout << "normal = " << normal.x << " -- " << normal.y << " -- " << normal.z << std::endl;
	//leftVector = glm::rotate(leftVector, angle, axis);
	//upVector = glm::rotate(upVector, angle, axis);
}
/*
void Particule::updateOrientation()
{
	glm::vec3 cameraPosition = camera->getPosition();
	glm::vec3 cameraDirection = normalize(getPosition() - cameraPosition);
	float dotX = dot(normalize(glm::vec2(cameraDirection.x, cameraDirection.z)), normalize(glm::vec2(normal.x, normal.z)));
	float dotY = dot(normalize(glm::vec2(cameraDirection.y, cameraDirection.z)), normalize(glm::vec2(normal.y, normal.z)));
	float phi = acos(std::max(-1.f, std::min(1.f,dotX)));
	float theta = acos(std::max(-1.f, std::min(1.f, dotY)));
	if(phi != 0)
		std::cout << "phi = " << phi << std::endl;
	setRotation(phi, glm::vec3(0, 1, 0));
	setRotation(theta, glm::vec3(1, 0, 0));
}
*/
void Particule::initShape()
{
	float radius = /*(pointLight != nullptr) ? (100 * pointLight->getRadiusAttenuation() / pointLight->getIntensity()) :*/ 10;

	glmlv::SimpleGeometry sp = glmlv::makeSphere(10);
	std::vector<glmlv::Vertex3f3f2f> vertices = sp.vertexBuffer;
	std::vector<uint32_t> index = sp.indexBuffer;

	/*vertices.emplace_back(glm::vec3(0),normal, glm::vec2(-1));
	for (int i = 0; i < 20; ++i)
	{
		float x = static_cast<float>(radius * std::cos(i * 2 * M_PI / 10));
		float y = static_cast<float>(radius * std::sin(i * 2 * M_PI / 10));
		vertices.emplace_back(glm::vec3(x, y, 0), normal, glm::vec2(-1));
		if (i > 0) index.push_back(i + 1);
		index.push_back(0);
		index.push_back(i + 1);
	}
	index.push_back(1);*/
	initBuffers(vertices, index);

	Material mat = Material();
	if (pointLight != nullptr)
	{
		setPosition(pointLight->getPosition());
		mat.setColor(Material::EMMISIVE_COLOR, pointLight->getColor()); // TODO : change for white ?
		mat.setColor(Material::DIFFUSE_COLOR, pointLight->getColor());
	}
	else
	{
		mat.setColor(Material::EMMISIVE_COLOR, glm::vec3(1));
		mat.setColor(Material::DIFFUSE_COLOR, glm::vec3(1));
	}
	std::vector<Material> materials = { mat };
	setMaterials(materials);

	ShapeData shapeData = ShapeData(index.size(), 0, 0);
	std::vector<ShapeData> shapes = { shapeData };
	setShapesData(shapes);

	setScale(glm::vec3(radius));
}
