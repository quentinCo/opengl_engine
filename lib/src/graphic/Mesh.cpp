#include <algorithm>

#include <qc/graphic/Mesh.hpp>

using namespace qc::graphic;

const Mesh::SharedMaterial Mesh::defaultMaterial = std::make_unique<Material>();

Mesh::Mesh(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices, const std::vector<ShapeData> shapesData, const glm::vec3& position)
	: shapesData(shapesData)
{
	modelMatrix = glm::translate(modelMatrix, position);
	sortShape();
	std::vector<Vertex> completVertices;
	computeTangentAndBitangent(vertices, indices, completVertices);
	initBuffers(completVertices, indices);
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

void Mesh::initBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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


//-- COMPUTE TANGENT AND BITANGENT ---
void Mesh::computeTangentAndBitangent(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& ibo, std::vector<Vertex>& res)
{
	for (const auto& it : vertices)
		res.emplace_back(it);

	for (int i = 0; i < ibo.size(); i += 3)
	{
		glm::vec3 edges[2];
		glm::vec2 deltaUVs[2];

		for (int j = 0; j < 2; ++j)
		{
			edges[j] = res[ibo[i]].position - res[ibo[i + j + 1]].position;
			deltaUVs[j] = res[ibo[i]].texCoords - res[ibo[i + j + 1]].texCoords;
		}

		float fractional = 1.0f / (deltaUVs[0].x * deltaUVs[1].y - deltaUVs[1].x * deltaUVs[0].y);

		glm::vec3 tangent;
		tangent.x = fractional * (deltaUVs[1].y * edges[0].x - deltaUVs[0].y * edges[1].x);
		tangent.y = fractional * (deltaUVs[1].y * edges[0].y - deltaUVs[0].y * edges[1].y);
		tangent.z = fractional * (deltaUVs[1].y * edges[0].z - deltaUVs[0].y * edges[1].z);
		
		glm::vec3 bitangent;
		bitangent.x = fractional * (-deltaUVs[1].x * edges[0].x + deltaUVs[0].x * edges[1].x);
		bitangent.y = fractional * (-deltaUVs[1].x * edges[0].y + deltaUVs[0].x * edges[1].y);
		bitangent.z = fractional * (-deltaUVs[1].x * edges[0].z + deltaUVs[0].x * edges[1].z);

		for (int j = 0; j < 3; ++j)
		{
			res[ibo[i + j]].tangent += tangent;
			res[ibo[i + j]].bitangent += tangent;
		}
	}

	for (auto it : res)
	{
		it.tangent = glm::normalize(it.tangent);
		it.bitangent = glm::normalize(it.bitangent);
	}
}