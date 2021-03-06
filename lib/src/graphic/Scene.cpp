#define NOMINMAX

#include <iostream>
#include <algorithm>

#include <qc/graphic/Scene.hpp>

using namespace qc::graphic;

//-- SETTER ---------------------------

void Scene::setSsboDirectionalLights()
{
	ssboDirectionalLights = BufferObject<Light>(directionalLights, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW); // TODO : revoir
}


//-- ADD OBJ ---------------------------

void Scene::addObj(const glmlv::fs::path& pathfile)
{
	try
	{
		glmlv::ObjData data;

		glmlv::loadObj(pathfile, data);

		// Init scene limite
		bboxMin = glm::min(data.bboxMin, bboxMin);
		bboxMax = glm::max(data.bboxMax, bboxMax);

		// Init mesh textures
		std::vector<std::shared_ptr<Texture>> textures;
		for (const auto& it : data.textures)
			textures.emplace_back(std::make_shared<Texture>(it));

		// Init mesh materials
		std::vector<std::shared_ptr<Material>> materials;
		for (auto& it : data.materials)
		{
			materials.push_back(std::make_unique<Material>());
			auto& material = materials.back();

			material->setColor(Material::AMBIENT_COLOR, it.Ka);
			glm::vec3 diffuseColor = (it.Kd == glm::vec3(0)) ? glm::vec3(1) : it.Kd;
			material->setColor(Material::DIFFUSE_COLOR, it.Kd);
			material->setColor(Material::SPECULAR_COLOR, it.Ks);
			material->setColor(Material::EMMISIVE_COLOR, it.Ke);

			if (it.KaTextureId >= 0) material->setTexture(Material::AMBIENT_TEXTURE, textures[it.KaTextureId]);
			if (it.KdTextureId >= 0) material->setTexture(Material::DIFFUSE_TEXTURE, textures[it.KdTextureId]);
			if (it.KsTextureId >= 0) material->setTexture(Material::SPECULAR_TEXTURE, textures[it.KsTextureId]);
			if (it.shininessTextureId >= 0) material->setTexture(Material::SPECULAR_HIGHT_LIGHT_TEXTURE, textures[it.shininessTextureId]);
			if (it.BumpTextureId >= 0) material->setTexture(Material::NORMAL_TEXTURE, textures[it.BumpTextureId]); // Trick normal map save in bump
		}

		// Init mesh shape infos
		uint32_t indexOffset = 0;
		std::vector<ShapeData> shapes;
		for (auto shapeID = 0; shapeID < data.indexCountPerShape.size(); ++shapeID)
		{
			shapes.emplace_back(data.indexCountPerShape[shapeID], indexOffset, materials[data.materialIDPerShape[shapeID]]);
			indexOffset += data.indexCountPerShape[shapeID];
		}

		Mesh mesh = Mesh(data.vertexBuffer, data.indexBuffer, shapes);

		mesh.setTextures(textures);
		mesh.setMaterials(materials);

		meshes.emplace_back(std::move(mesh));
	}
	catch (std::runtime_error e)
	{
		std::cerr << "Error during the loading." << std::endl;
	}	
}

//-- ADD POINT LIGHT -------------------
PointLight* Scene::addPointLight(const PointLight& light)
{
	pointLights.push_back(light);
	return &(pointLights.back());
}

//-- ADD PARTICULES --------------------
Particule* Scene::addParticules(Particule& particule)
{
	particules.emplace_back(std::move(particule));
	return &(particules.back());
}

//-- REMOVE POINTLIGHTS ----------------
void Scene::removePointLights(unsigned int index, int nb)
{
	if (index >= pointLights.size())
		return;

	auto end = (index + nb > pointLights.size()) ? pointLights.end() : pointLights.begin() + index + nb;
	pointLights.erase(pointLights.begin() + index, end);
}

//-- REMOVE PARTICULES -----------------
void Scene::removeParticules(unsigned int index, int nb)
{
	if (index >= particules.size())
		return;

	auto end = (index + nb > particules.size()) ? particules.end() : particules.begin() + index + nb;
	particules.erase(particules.begin() + index, end);
}

//-- SORT PARTICULES -------------------

void Scene::sortParticules()
{
	std::sort(particules.begin(), particules.end(), [](const Particule& a, const Particule& b) {
		if (b.getMaterials().size() == 0)
			return false;
		if (a.getMaterials().size() == 0)
			return true;
		return a.getMaterials()[0] < b.getMaterials()[0];
	});
}
