#pragma once

#include <vector>

#include <glm/vec3.hpp>

#include <glmlv/filesystem.hpp>
#include <glmlv/load_obj.hpp>

#include "Mesh.hpp"
#include "Light.hpp"
#include "DirectionalLight.hpp"
#include "PointLight.hpp"
#include "Particule.hpp"

namespace qc
{
	
namespace graphic
{

class Scene
{
public:
	Scene() {}

	Scene(const Scene& o) = delete;
	Scene& operator= (const Scene& o) = delete;

	Scene(Scene&& o) = default;
	Scene& operator= (Scene&& o) = default;


	//-- GETTERS ---------------------------
	const glm::vec3& getBboxMin() const
		{return bboxMin;}

	const glm::vec3& getBboxMax() const
		{return bboxMax;}

	const std::vector<Mesh>& getMeshes() const
		{return meshes;}

	std::vector<Mesh>& getMeshes()
		{return meshes;}

	const std::vector<PointLight>& getPointLights() const
		{return pointLights;}

	const std::vector<Light>& getDirectionalLights() const
		{return directionalLights;}

	const std::vector<Particule>& getParticules() const
		{return particules;}

	// TODO : delete or not
	std::vector<Particule>& getParticules()
		{return particules;}
	
	std::vector<PointLight>& getPointLights()
		{return pointLights;}

	std::vector<Light>& getDirectionalLights()
		{return directionalLights;}
	// ------------

	const float getSceneSize() const
		{return glm::length(bboxMax - bboxMin);}

	const BufferObject<PointLight>& getSsboPointLights() const
		{return ssboPointLights;}

	const BufferObject<Light>& getSsboDirectionalLights() const
		{return ssboDirectionalLights;}


	//-- SETTERS ---------------------------

	void setSsboPointLights()
		{ssboPointLights = BufferObject<PointLight>(pointLights, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);}

	void setSsboDirectionalLights();


	//-- ADD OBJ ---------------------------
	/*
		Load and add an obj to the scene
	*/
	void addObj(const glmlv::fs::path& pathfile );
	void addObj(Mesh& mesh)
		{meshes.emplace_back(std::move(mesh));}


	//-- ADD POINT LIGHT -------------------
	PointLight* addPointLight(const PointLight& light);


	//-- ADD DIRECTIONAL LIGHT -------------
	void addDirectionalLight(const DirectionalLight& light)
		{directionalLights.push_back(light);}


	//-- ADD PARTICULES --------------------
	Particule* addParticules(Particule& particule);

	//-- REMOVE POINTLIGHTS ----------------
	void removePointLights(unsigned int index, int nb);

	//-- REMOVE PARTICULES -----------------
	void removeParticules(unsigned int index, int nb);

	//-- CLEAR POINT LIGHT -----------------
	void clearPointLight()
	{
		pointLights.clear();
		setSsboPointLights();
	}

	//-- CLEAR PARTICULES ------------------
	void clearParticules()
		{particules.clear();}

	//-- SORT PARTICULES -------------------
	/*
		Sort particules according the materials.
		To avoid to change again and again the material and earn some frame.
	*/
	void sortParticules();

private:
	std::vector<Mesh> meshes;
	std::vector<PointLight> pointLights;
	std::vector<Light> directionalLights; // In Light cause we don't use the directional lights settings
	std::vector<Particule> particules;

	glm::vec3 bboxMin = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 bboxMax = glm::vec3(std::numeric_limits<float>::lowest());

	BufferObject<PointLight> ssboPointLights;
	BufferObject<Light> ssboDirectionalLights;

};

}//! namespace graphic

}//! namespace qc