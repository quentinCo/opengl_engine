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
#include "DeferredRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "ForwardPlusRenderer.hpp"

class Application
{
public:
    enum RendererType
    {
		DEFERRED = 0,
		FORWARD,
		FORWARD_PLUS
    };

    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ static_cast<int>(m_nWindowWidth), static_cast<int>(m_nWindowHeight), "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;
	
    // Scene data in GPU:
	qc::Scene scene;

	qc::Camera camera = qc::Camera(m_GLFWHandle);

	qc::Renderer* renderer = nullptr;

	int chosenRenderer = FORWARD_PLUS;
	int currentRenderer = FORWARD_PLUS;

	qc::DeferredRenderer deferred;
	qc::ForwardRenderer forward;
	qc::ForwardPlusRenderer forwardPlus;
    
	// Some initialisation functions
	void drawGUI(float* clearColor);

};