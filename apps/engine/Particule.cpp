#define _USE_MATH_DEFINES
#include <cmath>

#include "Particule.hpp"

using namespace qc;

/*
void Particule::setPosition(const glm::vec4& position)
{
	particuleShape.setPosition(position);
	//PointLight::setPosition(position);
}*/

void Particule::initShape()
{
	float radius = /*(pointLight != nullptr) ? (100 * pointLight->getRadiusAttenuation() / pointLight->getIntensity()) :*/ 10;
	std::vector<glmlv::Vertex3f3f2f> vertices;
	std::vector<uint32_t> index;

	vertices.emplace_back(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec2(0));
	for (int i = 0; i < 20; ++i)
	{
		float x = static_cast<float>(radius * std::cos(i * 2 * M_PI / 10));
		float y = static_cast<float>(radius * std::sin(i * 2 * M_PI / 10));
		vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0, 0, -1), glm::vec2(-1));
		if (i > 0) index.push_back(i + 1);
		index.push_back(0);
		index.push_back(i + 1);
	}
	index.push_back(1);
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
}
