#define NOMINMAX

#include <iostream>
#include <algorithm>

#include <glmlv/load_obj.hpp>

#include "Scene.hpp"

using namespace qc;

//-- SETTER ---------------------------

void Scene::setSsboDirectionalLights()
{
	std::vector<Light> directionalPointLights;
	for (const auto& it : directionalLights)
		directionalPointLights.push_back(it);

	ssboDirectionalLights = BufferObject<Light>(directionalPointLights, GL_SHADER_STORAGE_BUFFER); // TODO : revoir
}


//-- ADD OBJ ---------------------------

void Scene::addObj(const glmlv::fs::path& pathfile)
{
	try
	{
		glmlv::ObjData data;

		loadObj(pathfile, data);

		bboxMin = glm::min(data.bboxMin, bboxMin);
		bboxMax = glm::max(data.bboxMax, bboxMax);

		// Init shape infos
		uint32_t indexOffset = 0;
		std::vector<qc::ShapeData> shapes;
		for (auto shapeID = 0; shapeID < data.indexCountPerShape.size(); ++shapeID)
		{
			shapes.emplace_back(data.indexCountPerShape[shapeID], indexOffset, data.materialIDPerShape[shapeID]);
			indexOffset += data.indexCountPerShape[shapeID];
		}

		Mesh mesh = qc::Mesh(data.vertexBuffer, data.indexBuffer, shapes);

		std::vector<qc::Material> materials;
		for (auto it : data.materials)
		{
			qc::Material material = qc::Material();

			material.setColor(qc::Material::AMBIENT_COLOR, it.Ka);
			glm::vec3 diffuseColor = (it.Kd == glm::vec3(0)) ? glm::vec3(1) : it.Kd;
			material.setColor(qc::Material::DIFFUSE_COLOR, it.Kd);
			material.setColor(qc::Material::SPECULAR_COLOR, it.Ks);
			material.setColor(qc::Material::EMMISIVE_COLOR, it.Ke);

			if (it.KaTextureId >= 0) material.setMap(qc::Material::AMBIENT_TEXTURE, data.textures[it.KaTextureId]);
			if (it.KdTextureId >= 0) material.setMap(qc::Material::DIFFUSE_TEXTURE, data.textures[it.KdTextureId]);
			if (it.KsTextureId >= 0) material.setMap(qc::Material::SPECULAR_TEXTURE, data.textures[it.KsTextureId]);
			if (it.shininessTextureId >= 0) material.setMap(qc::Material::SPECULAR_HIGHT_LIGHT_TEXTURE, data.textures[it.shininessTextureId]);

			materials.emplace_back(std::move(material));
		}

		mesh.setMaterials(materials);

		meshes.emplace_back(std::move(mesh));
	}
	catch (std::runtime_error e)
	{
		std::cerr << "Error during the loading." << std::endl;
	}	
}
