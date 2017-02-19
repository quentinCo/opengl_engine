#pragma once

#include "Renderer.hpp"

namespace qc
{

class ForwardPlusRenderer : public Renderer
{
	
public:
	ForwardPlusRenderer() {}
	ForwardPlusRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	virtual ~ForwardPlusRenderer();

	ForwardPlusRenderer(ForwardPlusRenderer&& o);
	virtual ForwardPlusRenderer& operator= (ForwardPlusRenderer&& o);

	virtual void renderScene(const Scene& scene, const Camera& camera);

private:
	// Depth Pass
	glmlv::GLProgram programDepthPass;

	GLuint fboDepth = 0;
	GLuint depthMap = 0;

	GLuint uDepthModelViewProjMatrix;

	// Depth Debug
	glmlv::GLProgram programDebugDepth;
	GLuint screenVao = 0;
	GLuint screenVbo = 0;
	GLint uDepthMap = 0;

	// Light Culling
	glmlv::GLProgram programLightCullingPass;
	
	glm::vec3 nbComputeBlock;
	std::vector<int> pointLightsIndex;
	BufferObject<int> ssboPointLightsIndex;
	std::vector<glm::vec4> debugLight;
	BufferObject<glm::vec4> ssboDebug; // Debug
	GLuint uDebugOutput;

	GLuint uPointLightsForCulling;
	GLuint uPointLightsNumberForCulling;
	GLuint uPointLightsIndexForCulling;
	
	GLuint uInverseProjMatrix;
	GLuint uViewProjMatrixForCulling;
	GLuint uViewMatrixForCulling;
	GLuint uProjMatrixForCulling;
	GLuint uWindowDim;

	GLuint uDepthMapForCulling;

	// Shading Pass
	glmlv::GLProgram programForward; // TODO: change name

	GLuint textureSampler = 0;

	GLint uModelViewProjMatrix;
	GLint uModelViewMatrix;
	GLint uNormalMatrix;

	GLint uViewMatrix;
	//  Light
	GLint uDirectionalLights;
	GLint uDirectionalLightsNumber;

	GLint uPointLights;
	GLint uPointLightsNumber;

	//  Color
	GLint uKa;
	GLint uKd;
	GLint uKs;
	GLint uShininess;
	GLint uKaSampler;
	GLint uKdSampler;
	GLint uKsSampler;
	GLint uShininessSampler;

	void initDepthPass();
	void initDepthDebug(); // TODO : delete
	void initLightCullingPass();

	void initUniforms();

	void renderDepthPass(const Scene& scene, const Camera& camera);
	void renderDepthDebug();
	void renderLightCullingPass(const Scene& scene, const Camera& camera);
	void renderShadingPass(const Scene& scene, const Camera& camera);
	void renderMeshShaddingPass(const Mesh& mesh, const Camera& camera);
	void bindMeshMaterial(const Material& material);
};

}