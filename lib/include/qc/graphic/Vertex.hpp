#pragma once

#include <glm/glm.hpp>
#include <glmlv/glmlv.hpp>

namespace qc
{

namespace graphic
{

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texPoint, glm::vec3 tangent = glm::vec3(0), glm::vec3 bitangent = glm::vec3(0))
		: position(position), normal(normal), texCoords(texPoint), tangent(tangent), bitangent(bitangent)
	{}

	Vertex(glmlv::Vertex3f3f2f vertex, glm::vec3 tangent = glm::vec3(0), glm::vec3 bitangent = glm::vec3(0))
		: position(vertex.position), normal(vertex.normal), texCoords(vertex.texCoords), tangent(tangent), bitangent(bitangent)
	{}
};

}

}