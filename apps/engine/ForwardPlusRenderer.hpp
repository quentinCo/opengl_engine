#pragma once

#include "Renderer.hpp"

namespace qc
{

namespace graphic
{

//! NOT COPYABLE CLASS 
class ForwardPlusRenderer : public Renderer
{
	enum ShadingTexture
	{
		SCENE_TEXTURE = 0,
//		EMISSIVE_TEXTURE,
		DEPTH_TEXTURE,
		NB_TEXTURE
	};

public:
	ForwardPlusRenderer() {}
	ForwardPlusRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	virtual ~ForwardPlusRenderer();

	ForwardPlusRenderer(ForwardPlusRenderer&& o);
	virtual ForwardPlusRenderer& operator= (ForwardPlusRenderer&& o);

private:
	//-- Depth Pass Variables
	//---- Depth Pass Program
	glmlv::GLProgram programDepthPass;

	//---- Frame buffer
	GLuint fboDepth = 0;
	GLuint depthMap = 0;

	//---- Model View Proj Martix
	GLuint uDepthModelViewProjMatrix;

	//-- Depth Debug Variables 
	glmlv::GLProgram programDebugDepth;
	GLuint screenVao = 0;
	GLuint screenVbo = 0;
	GLint uDepthMap = 0;

	//-- Light culling variables
	//---- Light Culling Program
	glmlv::GLProgram programLightCullingPass;
	
	//---- Block nomber compute on base of 32 threads
	glm::vec3 nbComputeBlock;

	//---- Vector for the storing of the lights index that have an influence on a tile (32 * 32) - use to initialise the ssbo
	std::vector<int> pointLightsIndex; // TODO : delete

	//---- Ssbo to store the lights index.
	BufferObject<int> ssboPointLightsIndex;

	//---- Uniform variables for lights
	//------ Uniform for send point lights to shader
	GLuint uPointLightsForCulling;
	//------ Uniform for send the number of point lights
	GLuint uPointLightsNumberForCulling;
	//------ Uniform for the ssbo
	GLuint uPointLightsIndexForCulling;
	
	//------ Matrix
	GLuint uInverseProjMatrix;
	GLuint uViewProjMatrixForCulling;
	GLuint uViewMatrixForCulling;
	GLuint uProjMatrixForCulling;
	
	//------ Uniform use to verify that the thread doesn't handle a pixel that's on the screen
	GLuint uWindowDim;

	//------ Depth map of the scene 
	GLuint uDepthMapForCulling;

	//-- Shading Pass Variables
	//---- Shading Pass Program
	glmlv::GLProgram programShadingPass;

	GLuint fboShadingPass = 0;
	GLuint shadingRenderedTexture[NB_TEXTURE];

	//---- Matrix
	GLint uModelViewProjMatrixForShading;
	GLint uModelViewMatrixForShading;
	GLint uNormalMatrixForShading;

	GLint uViewMatrixForShading;

	//---- Windows dimensions use to find the tile of a pixel
	GLuint uWindowDimForShading;

	//---- Uniforms for directional lights
	GLint uDirectionalLights;
	GLint uDirectionalLightsNumber;

	//---- Uniforms for point lights
	GLint uPointLights;
	GLint uPointLightsNumber;
	GLint uPoinLightIndexForShading;

	//-- Emissive Pass
	GLuint fboEmissivePass = 0;
	GLuint emissiveTexture[2];
	
	
	//-- INIT DEPTH PASS -------------------
	/*
		Initialise every variables for the depth pass
	*/
	void initDepthPass();
	void initDepthDebug(); // TODO : delete
	
	
	//-- INIT LIGHT CULLING PASS ------------
	/*
		Initialise every variables for the light culling pass
	*/
	void initLightCullingPass();


	//-- INIT SHADING PASS -------------------
	/*
		Initialise every variables for the shading pass
	*/
	void initShadingPass();


	//-- INIT EMISSIVE PASS -----------------
	void initEmissivePass();

	//-- PARENT RENDERING FUNCTIONS ----------
	virtual void prePassRendering(const Scene& scene, const Camera& camera);
	virtual void renderScene(const Scene& scene, const Camera& camera);
	virtual void postProcessPass(const Scene& scene, const Camera& camera);

	//-- RENDER DEPTH PASS------------------
	void renderDepthPass(const Scene& scene, const Camera& camera);
	void renderDepthDebug();

	//-- RENDER LIGHT CULLING PASS ---------
	void renderLightCullingPass(const Scene& scene, const Camera& camera);

	//-- RENDER SHADING PASS ---------------
	void renderShadingPass(const Scene& scene, const Camera& camera);

	//-- RENDER EMISSIVE PASS --------------
	void renderEmissivePass(const Scene& scene, const Camera& camera);
};

} //! namespace graphic

} //! namespace qc