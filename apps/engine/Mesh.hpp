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
	std::shared_ptr<Material> materialPointer = nullptr;

	ShapeData() {}
	ShapeData(size_t size, size_t index = 0, std::shared_ptr<Material> mat = nullptr)
		: shapeSize(size), shapeIndex(index), materialPointer(mat)
	{}
};

class Mesh
{
//: NOT COPYABLE CLASS
public:
	using Vbo = BufferObject<glmlv::Vertex3f3f2f>;
	using Ibo = BufferObject<uint32_t>;
	using Vao = ArrayObject<glmlv::Vertex3f3f2f>;
	using SharedMaterial = std::shared_ptr<Material>;
	using SharedTexture = std::shared_ptr<Texture>;

	static const SharedMaterial defaultMaterial;

	Mesh(){}
	Mesh(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices, const std::vector<ShapeData> shapesData, const glm::vec3& position = glm::vec3(0));

	Mesh(const Mesh& o) = delete;
	Mesh& operator= (const Mesh& o) = delete;

	Mesh(Mesh&& o) = default;
	Mesh& operator= (Mesh&& o) = default;


	//-- GETTERS ---------------------------
	
	const std::unique_ptr<Vao>& getVao() const
		{return vao;}

	const std::vector<ShapeData>& getShapesData() const
		{return shapesData;}
	
	const glm::mat4& getModelMatrix() const
		{return modelMatrix;}

	const std::vector<SharedMaterial>& getMaterials() const
		{return materials;}

	const std::vector<SharedTexture>& getTextures() const
		{return textures;}

	virtual glm::vec3 getPosition() const
		{return modelMatrix[3];}


	//-- SETTERS ---------------------------
	
	virtual void setPosition(const glm::vec3& position);

	virtual void setRotation(const float angle, const glm::vec3& axis)
		{modelMatrix = glm::rotate(modelMatrix, angle, axis);}

	virtual void setScale(const glm::vec3& scale)
		{modelMatrix = glm::scale(modelMatrix, scale);}

	void setShapesData(const std::vector<ShapeData>& shapesData);

	void setMaterials(std::vector<SharedMaterial>& mat)
		{materials = std::move(mat);}

	void setTextures(std::vector<SharedTexture>& tex)
		{textures = std::move(tex);}

protected:
	//-- INIT BUFFERS ----------------------
	/*
		Initialise vbo, ibo and vao
	*/
	void initBuffers(const std::vector<glmlv::Vertex3f3f2f>& vertices, const std::vector<uint32_t>& indices);

private:

	std::unique_ptr<Vbo> vbo;
	std::unique_ptr<Ibo> ibo;
	std::unique_ptr<Vao> vao;

	//-- Information about the different shape and material that compose the mesh object
	std::vector<ShapeData> shapesData;

	glm::mat4 modelMatrix = glm::mat4();

	//-- Mesh materials
	std::vector<SharedMaterial> materials;
	std::vector<SharedTexture> textures;

	void sortShape();
};

} // namespace qc