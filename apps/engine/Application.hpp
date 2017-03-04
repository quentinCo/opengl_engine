#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

#include <limits>

#include "Camera.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "ForwardPlusRenderer.hpp"

class Application
{
public:

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
	qc::Camera camera = qc::Camera(m_GLFWHandle);

	//-- Scene 
	qc::Scene scene;

	//-- Renderers 
	qc::Renderer* renderer = nullptr;
	qc::ForwardPlusRenderer forwardPlus;
    

	//-- RENDER GUI ----------------------
	/*
		UI rendering
	*/
	void renderGUI(float* clearColor);
};