#pragma once
#define NOMINMAX

#include <vector>

#include <glm/vec3.hpp>

#include <glmlv/filesystem.hpp>

#include "Mesh.hpp"
#include "Light.hpp"

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

	const std::vector<Light>& getPointLights() const
		{return pointLights;}

	const std::vector<DirectionalLight>& getDirectionalLights() const
		{return directionalLights;}

	// TODO : delete
	std::vector<Light>& getPointLights()
		{return pointLights;}

	std::vector<DirectionalLight>& getDirectionalLights()
		{return directionalLights;}
	// ------------

	const float getSceneSize() const
		{return glm::length(bboxMax - bboxMin);}

	const BufferObject<Light>& getUboPointLights() const
		{return uboPointLights;}

	const BufferObject<DirectionalLight>& getUboDirectionalLights() const
		{return uboDirectionalLights;}

	void setUboPointLights()
		{uboPointLights = BufferObject<Light>(pointLights, GL_UNIFORM_BUFFER);}

	void setUboDirectionalLights()
		{uboDirectionalLights = BufferObject<DirectionalLight>(directionalLights, GL_UNIFORM_BUFFER);}

	void addObj(const glmlv::fs::path& pathfile );

	void addPointLight(const Light& light)
		{pointLights.push_back(light);}

	void addDirectionalLight(const DirectionalLight& light)
		{directionalLights.push_back(light);}

private:
	std::vector<Mesh> meshes;
	std::vector<Light> pointLights;
	std::vector<DirectionalLight> directionalLights; // TODO : revoir passer DirectionalLight -> Light

	glm::vec3 bboxMin = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 bboxMax = glm::vec3(std::numeric_limits<float>::lowest());

	BufferObject<Light> uboPointLights;
	BufferObject<DirectionalLight> uboDirectionalLights;
};

}