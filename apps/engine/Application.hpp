#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>

#include <glm/glm.hpp>

#include <limits>
#include <thread> 

#include <qc/graphic/Camera.hpp>
#include <qc/graphic/Scene.hpp>
#include <qc/graphic/Renderer.hpp>
#include <qc/graphic/ForwardPlusRenderer.hpp>
#include <qc/physic/PhysicalSystem.hpp>

class Application
{
public:
	using RenderOptions = qc::graphic::Renderer::RenderOptions;
	using PhysicType = qc::physic::PhysicalSystem::PhysicType;

    Application(int argc, char** argv);

	//-- RUN -------------------------------
	/*
		Run the application and rendering loop
	*/
    int run();

private:
	//-- Window
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ static_cast<int>(m_nWindowWidth), static_cast<int>(m_nWindowHeight), "QC OpenGL Engine" };

	//-- Paths Directories 
    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

	//-- Camera 
	qc::graphic::Camera camera = qc::graphic::Camera(m_GLFWHandle);

	//-- Scene 
	qc::graphic::Scene scene;

	//-- Renderers 
	qc::graphic::Renderer* renderer = nullptr;
	qc::graphic::ForwardPlusRenderer forwardPlus;
	RenderOptions renderOptions = RenderOptions::RENDER_ALL;
    
	//-- Predef Materials for particules init
	std::vector<std::shared_ptr<qc::graphic::Material>> preDefMaterials;

	//-- Physic
	std::thread physic;
	qc::physic::PhysicalSystem physicSystem;

	std::vector<int> linkPhysicGraphic; // particule index / physicalObj index //TODO change struct

	PhysicType physicLinkType;

	//-- UI variables
	glm::vec3 bboxMax;
	glm::vec3 bboxMin;

	//---- Lights
	const int initNbParticules = 350;
	int nbPointLight;
	bool nbParticulesChange = false;

	//---- Physic
	bool activePhysic = false;
	bool resetSystem = false;
	float discretizationFrequency = 100.f;
	
	//-- INIT LIGHTS ---------------------
	void initLights();
	qc::graphic::PointLight getRandPointLight(glm::vec3& dimScene, int i);

	//-- INIT PARTICULES -----------------
	void initParticules();


	//-- INIT PHYSIC ---------------------
	void initPhysic();


	//-- UPDATE PHYSIC -------------------
	void updatePhysic();


	//-- RESET PHYSICAL PARTICULES SYSTEM 
	void resetPhysicalParticulesSystem();

	//-- SYNCHRO GRAPHIC PHYSIC ----------
	void updateGraphicFromPhysic();

	//-- CHANGE NB PARTICULES ------------
	void changeNbParticules();

	void incrementPointLights();
	void decrementParticules();

	//-- RENDER GUI ----------------------
	/*
		UI rendering
	*/
	void renderGUI(float* clearColor);
	void renderGraphicOption();
	void renderPhysicOption();
};