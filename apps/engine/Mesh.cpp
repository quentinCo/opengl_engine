#include "Mesh.hpp"

using namespace qc;

Mesh::Mesh(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices, const std::vector<ShapeData> shapesData, const glm::vec3& position)
	: shapesData(shapesData)
{
	modelMatrix = glm::translate(modelMatrix, position);
	initBuffers(vertices, indices);
}

void Mesh::initBuffers(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices)
{
	vbo = BufferObject<glmlv::Vertex3f3f2f>(vertices);
	ibo = BufferObject<uint32_t>(indices);
	vao = ArrayObject<glmlv::Vertex3f3f2f>(vbo, ibo);
}
