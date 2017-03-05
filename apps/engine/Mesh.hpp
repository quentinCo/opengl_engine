#pragma once

#define NOMINMAX

#include <glmlv/simple_geometry.hpp>

#include "BufferObject.hpp"
#include "ArrayObject.hpp"
#include "Material.hpp"

namespace qc
{

struct ShapeData
{
	size_t shapeSize = 0;
	size_t shapeIndex = 0;
	int materialIndex = -1;

	ShapeData() {}
	ShapeData(size_t size, size_t index = 0, int mat = -1)
		: shapeSize(size), shapeIndex(index), materialIndex(mat)
	{}
};

class Mesh
{
//: NOT COPYABLE CLASS
public:
	static const Material defaultMaterial;

	Mesh(){}
	Mesh(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices, const std::vector<ShapeData> shapesData, const glm::vec3& position = glm::vec3(0));

	Mesh(const Mesh& o) = delete;
	Mesh& operator= (const Mesh& o) = delete;

	Mesh(Mesh&& o) = default;
	Mesh& operator= (Mesh&& o) = default;


	//-- GETTERS ---------------------------
	
	const ArrayObject<glmlv::Vertex3f3f2f>& getVao() const
		{return vao;}

	const std::vector<ShapeData>& getShapesData() const
		{return shapesData;}
	
	const glm::mat4& getModelMatrix() const
		{return modelMatrix;}

	const std::vector<Material>& getMaterials() const
		{return materials;}

	virtual glm::vec3 getPosition() const
		{return modelMatrix[3];}


	//-- SETTERS ---------------------------
	
	virtual void setPosition(const glm::vec3& position);

	virtual void setRotation(const float angle, const glm::vec3& axis)
		{modelMatrix = glm::rotate(modelMatrix, angle, axis);}

	virtual void setScale(const glm::vec3& scale)
		{modelMatrix = glm::scale(modelMatrix, scale);}

	void setShapesData(const std::vector<ShapeData>& shapesData)
		{this->shapesData = shapesData;}

	void setMaterials(std::vector<Material>& mat)
		{materials = std::move(mat);}

protected:
	//-- INIT BUFFERS ----------------------
	/*
		Initialise vbo, ibo and vao
	*/
	void initBuffers(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices);

private:

	BufferObject<glmlv::Vertex3f3f2f> vbo;
	BufferObject<uint32_t> ibo;
	ArrayObject<glmlv::Vertex3f3f2f> vao;

	//-- Information about the different shape and material that compose the mesh object
	std::vector<ShapeData> shapesData;

	glm::mat4 modelMatrix = glm::mat4();

	//-- Mesh materials
	std::vector<Material> materials;
};

} // namespace qc