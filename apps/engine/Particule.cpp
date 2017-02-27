#include "Particule.hpp"

using namespace qc;

Particule& Particule::operator= (const Particule& o)
{
	setPosition(o.getPosition());
	setColor(o.getColor());
	setIntensity(o.getIntensity());
	setRadiusAttenuation(o.getRadiusAttenuation());

	initShape();
	return *this;
}
/*
void Particule::setPosition(const glm::vec4& position)
{
	particuleShape.setPosition(position);
	//PointLight::setPosition(position);
}*/

void Particule::setRotation(float angle, const glm::vec3& axis)
	{particuleShape.setRotation(angle, axis);}

void Particule::initShape()
{
	/*float radius = 1;
	std::vector<glmlv::Vertex3f3f2f> vertices;
	std::vector<uint32_t> index;

	vertices.emplace_back(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec2(0));
	for (int i = 0; i < 10; ++i)
	{
		float x = static_cast<float>(radius * std::cos(i * 2 * M_PI / 10));
		float y = static_cast<float>(radius * std::sin(i * 2 * M_PI / 10));
		vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0, 0, 1), glm::vec2(0));

		index.push_back(i + 1);
		index.push_back(0);
		if (i > 0) index.push_back(i + 1);
	}
	index.push_back(1);

	Material mat = Material();
	mat.setColor(Material::EMMISIVE_COLOR, getColor()); // TODO : change for white ?
	std::vector<Material> materials = { mat };

	ShapeData shapeData = ShapeData(index.size());
	std::vector<ShapeData> shapes = { shapeData };

	particuleShape = Mesh(vertices, index, shapes, getPosition());
	particuleShape.setMaterials(materials);*/
}
