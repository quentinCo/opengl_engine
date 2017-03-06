#include <algorithm>

#include "Mesh.hpp"

using namespace qc;

const Mesh::SharedMaterial Mesh::defaultMaterial = std::make_unique<Material>();

Mesh::Mesh(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices, const std::vector<ShapeData> shapesData, const glm::vec3& position)
	: shapesData(shapesData)
{
	modelMatrix = glm::translate(modelMatrix, position);
	sortShape();
	initBuffers(vertices, indices);
}


//-- SETTER ----------------------------

void Mesh::setPosition(const glm::vec3& position)
{
	glm::vec3 delta = position - glm::vec3(modelMatrix[3]);
	modelMatrix = glm::translate(modelMatrix, delta);
}


void Mesh::setShapesData(const std::vector<ShapeData>& shapesData)
{
	this->shapesData = std::move(shapesData);
	sortShape();
}

//-- INIT BUFFERS ----------------------

void Mesh::initBuffers(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices)
{
	vbo = std::make_unique<Vbo>(vertices);
	ibo = std::make_unique<Ibo>(indices);
	vao = std::make_unique<Vao>(*vbo, *ibo);
}


//-- SORT SHAPE -----------------------

void Mesh::sortShape()
{
	if (shapesData.size() < 2)
		return;

	std::sort(shapesData.begin(), shapesData.end(), [](ShapeData& a, ShapeData& b) {
		return a.materialPointer < b.materialPointer;
	});
}