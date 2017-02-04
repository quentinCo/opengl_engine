#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>

#include <glm/glm.hpp>

#include <limits>

#include "BufferObject.hpp"
#include "ArrayObject.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class Application
{
public:
    enum GBufferTextureType
    {
        GPosition = 0,
        GNormal,
        GAmbient,
        GDiffuse,
        GGlossyShininess,
        GDepth, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
        GBufferTextureCount
    };
    const GLenum m_GBufferTextureFormat[GBufferTextureCount] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

    Application(int argc, char** argv);

    int run();
private:
    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Template" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    // Scene data in GPU:
	qc::Mesh mesh;

    float m_SceneSize = 0.f; // Used for camera speed and projection matrix parameters
	
    GLuint m_textureSampler = 0; // Only one sampler object since we will use the same sampling parameters for all textures

    glmlv::GLProgram m_programGeo;
    glmlv::ViewController m_viewController{ m_GLFWHandle.window(), 3.f };
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uNormalMatrixLocation;

    GLint m_uKaLocation;
    GLint m_uKdLocation;
    GLint m_uKsLocation;
    GLint m_uShininessLocation;
    GLint m_uKaSamplerLocation;
    GLint m_uKdSamplerLocation;
    GLint m_uKsSamplerLocation;
    GLint m_uShininessSamplerLocation;

    float m_DirLightPhiAngleDegrees = 90.f;
    float m_DirLightThetaAngleDegrees = 45.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.f;

    glm::vec3 m_PointLightPosition = glm::vec3(0, 1, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 5.f;

    // Deferred
    GLuint m_GBufferTextures[GBufferTextureCount];
    GLuint m_FBO;

    int attachedToDraw;


    // Shading
    glmlv::GLProgram m_programShading;

	GLint m_uScreenTexture;
	
    GLuint m_ScreenVAO = 0;
    GLuint m_ScreenVBO = 0;

	// Compute shader
	glmlv::GLProgram m_programCompute;
	GLuint m_screenTexture = 0;

	int m_workGroupCount[3];
	int m_workGroupSize[3];
	int m_workGroupInvocation;
	
	GLint m_uDirectionalLightDirLocation;
	GLint m_uDirectionalLightIntensityLocation;

	GLint m_uPointLightPositionLocation;
	GLint m_uPointLightIntensityLocation;

	GLint m_uGTextures[GBufferTextureCount];

    GLint m_uWindowsDim;

    
	// Some initialisation functions
	void drawGeoPass(const glm::mat4& viewMatrix, const glm::mat4& projMatrix);
	void drawComputePass(const glm::mat4& viewMatrix);
	void drawShadingPass();
	void drawGUI(float* clearColor);

	void loadScene();

	void initForGeo();
	void initForShading();
	void initScreenBuffers();
	void initForCompute();
};