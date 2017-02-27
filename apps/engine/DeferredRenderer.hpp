#pragma once

#include "Renderer.hpp"

namespace qc
{

class DeferredRenderer : public Renderer
{

public:	
	enum GBufferTextureType
	{
		GBUFFER_POSITION = 0,
		GBUFFER_NORMAL,
		GBUFFER_AMBIENT,
		GBUFFER_DIFFUSE,
		GBUFFER_GLOSSY_SHININESS,
		GBUFFER_DEPTH, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
		GBUFFER_NB_TEXTURE
	};

	static const GLenum gBufferTextureFormat[GBUFFER_NB_TEXTURE];

	DeferredRenderer()
		:Renderer()
	{}

	DeferredRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	virtual ~DeferredRenderer();

	DeferredRenderer(DeferredRenderer&& o);
	virtual DeferredRenderer& operator= (DeferredRenderer&& o);

	virtual void renderScene(const Scene& scene, const Camera& camera);

private:
	// Geo Pass variables
	glmlv::GLProgram programGeoPass;

	GLuint textureSampler = 0;

	// View uniforms
	GLint uModelViewProjMatrix;
	GLint uModelViewMatrix;
	GLint uNormalMatrix;

	// Material uniforms
	GLint uKa;
	GLint uKd;
	GLint uKs;
	GLint uShininess;
	GLint uKaSampler;
	GLint uKdSampler;
	GLint uKsSampler;
	GLint uShininessSampler;

	// Fragement shader
	GLuint gBufferTextures[GBUFFER_NB_TEXTURE];
	GLuint fbo = 0;

	// Shading Pass variables
	glmlv::GLProgram programShadingPass;

	// Screen Texture support
	GLuint screenVao = 0;
	GLuint screenVbo = 0;

	// Texture uniform
	GLint uScreenTexture;

	// Compute Pass variables
	glmlv::GLProgram programComputePass;

	// Screen texture
	GLuint screenTexture = 0;

	// Light uniforms
	GLint uDirectionalLights;
	GLint uDirectionalLightsNumber;

	GLint uPointLights;
	GLint uPointLightsNumber;

	GLint uViewMatrix;

	// Texture uniforms
	GLint uGTextures[GBUFFER_NB_TEXTURE];

	// Windows dimension uniform (for thread control in shader)
	GLint uWindowDim;

	void initGeoPassVariables();
	void initFBOGeoPass();
	void initComputePassVariables();
	void initShadingPassVariables();
	
	void renderGeoPass(const Scene& scene, const Camera& camera);
	void renderGeoPassMesh(const Mesh& mesh, const Camera& camera);
	void bindMeshMaterial(const Material& material);

	void renderComputePass(const Scene& scene, const Camera& camera);

	void renderShadingPass();
};

} // namespace qc