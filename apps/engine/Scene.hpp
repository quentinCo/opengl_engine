#pragma once

#include <vector>

#include <glm/vec3.hpp>

#include <glmlv/filesystem.hpp>

#include "Mesh.hpp"
#include "Light.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "Particule.hpp"

namespace qc
{

class Scene
{
public:
	Scene() {}

	Scene(const Scene& o) = delete;
	Scene& operator= (const Scene& o) = delete;

	Scene(Scene&& o) = default;
	Scene& operator= (Scene&& o) = default;

	const std::vector<Mesh>& getMeshes() const
		{return meshes;}

	std::vector<Mesh>& getMeshes()
		{return meshes;}


	const std::vector<PointLight>& getPointLights() const
		{return pointLights;}

	const std::vector<DirectionalLight>& getDirectionalLights() const
		{return directionalLights;}

	const std::vector<Particule>& getParticules() const
		{return particules;}

	// TODO : delete or not
	std::vector<Particule>& getParticules()
		{return particules;}
	
	std::vector<PointLight>& getPointLights()
		{return pointLights;}

	std::vector<DirectionalLight>& getDirectionalLights()
		{return directionalLights;}
	// ------------

	const float getSceneSize() const
		{return glm::length(bboxMax - bboxMin);}

	const BufferObject<PointLight>& getSsboPointLights() const
		{return ssboPointLights;}

	const BufferObject<Light>& getSsboDirectionalLights() const
		{return ssboDirectionalLights;}

	void setSsboPointLights()
		{ssboPointLights = BufferObject<PointLight>(pointLights, GL_SHADER_STORAGE_BUFFER);}

	void setSsboDirectionalLights();

	void addObj(const glmlv::fs::path& pathfile );
	void addObj(Mesh& mesh)
	{
		meshes.push_back(std::move(mesh));
	}

	void addPointLight(const PointLight& light)
		{pointLights.push_back(light);}

	void addDirectionalLight(const DirectionalLight& light)
		{directionalLights.push_back(light);}

	void addParticules(Particule& particule)
	{
		particules.push_back(std::move(particule));
//		meshes.push_back(std::move(particule));
	}

private:
	std::vector<Mesh> meshes;
	std::vector<PointLight> pointLights;
	std::vector<DirectionalLight> directionalLights; // TODO : revoir passer DirectionalLight -> Light
	std::vector<Particule> particules;

	glm::vec3 bboxMin = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 bboxMax = glm::vec3(std::numeric_limits<float>::lowest());

	BufferObject<PointLight> ssboPointLights;
	BufferObject<Light> ssboDirectionalLights;

};

}