#include <glm/gtc/type_ptr.hpp>

#include <qc/graphic/ForwardPlusRenderer.hpp>

using namespace qc::graphic;

ForwardPlusRenderer::ForwardPlusRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: Renderer(shaderDirectory, windowWidth, windowHeight)
{
	initDepthPass();
	initLightCullingPass();
	initShadingPass();

	initEmissivePass();
	initBlurPass();
	initGatherPass(2);
}

ForwardPlusRenderer::~ForwardPlusRenderer()
{
	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	if (depthMap) glDeleteSamplers(1, &depthMap);
	if (fboShadingPass) glDeleteFramebuffers(1, &fboShadingPass);
	if (shadingRenderedTexture) glDeleteSamplers(2, shadingRenderedTexture);
}

ForwardPlusRenderer::ForwardPlusRenderer(ForwardPlusRenderer&& o)
	:Renderer(std::move(o)), programDepthPass(std::move(o.programDepthPass)), fboDepth(o.fboDepth), depthMap(o.depthMap), uDepthModelViewProjMatrix(o.uDepthModelViewProjMatrix),
	programLightCullingPass(std::move(o.programLightCullingPass)), nbComputeBlock(o.nbComputeBlock), uPoinLightIndexForShading(o.uPoinLightIndexForShading),
	uWindowDimForShading(o.uWindowDimForShading), ssboPointLightsIndex(std::move(o.ssboPointLightsIndex)), uPointLightsForCulling(o.uPointLightsForCulling),
	uPointLightsNumberForCulling(o.uPointLightsNumberForCulling),uPointLightsIndexForCulling(o.uPointLightsIndexForCulling), uInverseProjMatrix(o.uInverseProjMatrix),
	uViewMatrixForCulling(o.uViewMatrixForCulling),	uWindowDim(o.uWindowDim),uDepthMapForCulling(o.uDepthMapForCulling), programShadingPass(std::move(o.programShadingPass)),
	uModelViewProjMatrixForShading(o.uModelViewProjMatrixForShading), uModelViewMatrixForShading(o.uModelViewMatrixForShading),
	uNormalMatrixForShading(o.uNormalMatrixForShading), uViewMatrixForShading(o.uViewMatrixForShading),
	uDirectionalLights(o.uDirectionalLights), uDirectionalLightsNumber(o.uDirectionalLightsNumber),
	uPointLights(o.uPointLights), uPointLightsNumber(o.uPointLightsNumber)
{
	o.programDepthPass = glmlv::GLProgram();

	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	fboDepth = o.fboDepth;
	o.fboDepth = 0;

	if (depthMap) glDeleteSamplers(1, &depthMap);
	depthMap = o.depthMap;
	o.depthMap = 0;

	o.programLightCullingPass = glmlv::GLProgram();

	o.programShadingPass = glmlv::GLProgram();

	if (fboShadingPass) glDeleteFramebuffers(1, &fboShadingPass);
	fboShadingPass = o.fboShadingPass;
	o.fboShadingPass = 0;

	if (shadingRenderedTexture) glDeleteSamplers(NB_TEXTURE, shadingRenderedTexture);
	for (int i = 0; i < NB_TEXTURE; ++i)
	{
		shadingRenderedTexture[i] = o.shadingRenderedTexture[i];
		o.shadingRenderedTexture[i] = 0;
	}
}

ForwardPlusRenderer& ForwardPlusRenderer::operator= (ForwardPlusRenderer&& o)
{
	Renderer::operator=(std::move(o));
	shaderDirectory = o.shaderDirectory;
	windowWidth = o.windowWidth;
	windowHeight = o.windowHeight;

	programDepthPass = std::move(o.programDepthPass);
	o.programDepthPass = glmlv::GLProgram();

	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	fboDepth = o.fboDepth;
	o.fboDepth = 0;

	if (depthMap) glDeleteSamplers(1, &depthMap);
	depthMap = o.depthMap;
	o.depthMap = 0;

	uDepthModelViewProjMatrix = o.uDepthModelViewProjMatrix;

	programLightCullingPass = std::move(o.programLightCullingPass);
	o.programLightCullingPass = glmlv::GLProgram();
	nbComputeBlock = o.nbComputeBlock;
	uPoinLightIndexForShading = o.uPoinLightIndexForShading;
	uWindowDimForShading = o.uWindowDimForShading;
	ssboPointLightsIndex = std::move(o.ssboPointLightsIndex);
	uPointLightsForCulling = o.uPointLightsForCulling;
	uPointLightsNumberForCulling = o.uPointLightsNumberForCulling;
	uPointLightsIndexForCulling = o.uPointLightsIndexForCulling;
	uInverseProjMatrix = o.uInverseProjMatrix;
	uViewMatrixForCulling = o.uViewMatrixForCulling;
	uWindowDim = o.uWindowDim;
	uDepthMapForCulling = o.uDepthMapForCulling;

	programShadingPass = std::move(o.programShadingPass);
	o.programShadingPass = glmlv::GLProgram();

	if (fboShadingPass) glDeleteFramebuffers(1, &fboShadingPass);
	fboShadingPass = o.fboShadingPass;
	o.fboShadingPass = 0;

	if (shadingRenderedTexture) glDeleteSamplers(NB_TEXTURE, shadingRenderedTexture);
	for (int i = 0; i < NB_TEXTURE; ++i)
	{
		shadingRenderedTexture[i] = o.shadingRenderedTexture[i];
		o.shadingRenderedTexture[i] = 0;
	}

	uModelViewProjMatrixForShading = o.uModelViewProjMatrixForShading;
	uModelViewMatrixForShading = o.uModelViewMatrixForShading;
	uNormalMatrixForShading = o.uNormalMatrixForShading;

	uViewMatrixForShading = o.uViewMatrixForShading;

	uDirectionalLights = o.uDirectionalLights;
	uDirectionalLightsNumber = o.uDirectionalLightsNumber;

	uPointLights = o.uPointLights;
	uPointLightsNumber = o.uPointLightsNumber;

	return *this;
}


//-- INIT DEPTH PASS -------------------
void ForwardPlusRenderer::initDepthPass()
{
	programDepthPass = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusDepthPass.vs.glsl" , shaderDirectory / "forwardPlus" / "forwardPlusDepthPass.fs.glsl" });

	uDepthModelViewProjMatrix = glGetUniformLocation(programDepthPass.glId(), "uModelViewProjMatrix");

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fboDepth);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDepth);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (res != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error check frame buffer : " << res << std::endl;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

//-- INIT LIGHT CULLING PASS -----------
void ForwardPlusRenderer::initLightCullingPass()
{
	programLightCullingPass = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusPointLightCulling.cs.glsl"});

	nbComputeBlock = glm::vec3((GLuint)ceil(windowWidth / 16.f), (GLuint)ceil(windowHeight / 16.f), 1);

	ssboPointLightsIndex = BufferObject<int>(static_cast<size_t>(nbComputeBlock.x * nbComputeBlock.y * 200), GL_SHADER_STORAGE_BUFFER);
	
	uPointLightsForCulling = glGetProgramResourceIndex(programLightCullingPass.glId(), GL_SHADER_STORAGE_BLOCK,"uPointLights");
	uPointLightsNumberForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uPointLightsNumber");
	uPointLightsIndexForCulling = glGetProgramResourceIndex(programLightCullingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLightsIndex");

	uInverseProjMatrix = glGetUniformLocation(programLightCullingPass.glId(), "uInverseProjMatrix");
	uViewMatrixForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uViewMatrix");
	uWindowDim = glGetUniformLocation(programLightCullingPass.glId(), "uWindowDim");

	uDepthMapForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uDepthMap");
}


//-- INIT SHADING PASS ----------------
void ForwardPlusRenderer::initShadingPass()
{
	programShadingPass = glmlv::compileProgram({ shaderDirectory / "general" / "geometryPass.vs.glsl" , shaderDirectory / "forwardPlus" / "forwardPlusShadingPass.fs.glsl" });

	glGenTextures(NB_TEXTURE, shadingRenderedTexture);
	for (int i = 0; i < DEPTH_TEXTURE; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, shadingRenderedTexture[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, windowWidth, windowHeight);
	}

	glBindTexture(GL_TEXTURE_2D, shadingRenderedTexture[DEPTH_TEXTURE]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, windowWidth, windowHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fboShadingPass);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboShadingPass);
	for(int i = 0; i < DEPTH_TEXTURE; ++i)
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, shadingRenderedTexture[i], 0);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadingRenderedTexture[DEPTH_TEXTURE], 0);	

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

	GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (res != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error check shading frame buffer : " << res << std::endl;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uModelViewProjMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uModelViewProjMatrix");
	uModelViewMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uModelViewMatrix");
	uNormalMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uNormalMatrix");

	uViewMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uViewMatrix");

	uDirectionalLights = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uDirectionalLights");
	uDirectionalLightsNumber = glGetUniformLocation(programShadingPass.glId(), "uDirectionalLightsNumber");

	uPointLights = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLights");
	uPointLightsNumber = glGetUniformLocation(programShadingPass.glId(), "uPointLightsNumber");
	
	uPoinLightIndexForShading = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLightsIndex");

	uWindowDimForShading = glGetUniformLocation(programShadingPass.glId(), "uWindowDim");

	uKa = glGetUniformLocation(programShadingPass.glId(), "uKa");
	uKd = glGetUniformLocation(programShadingPass.glId(), "uKd");
	uKs = glGetUniformLocation(programShadingPass.glId(), "uKs");
	uShininess = glGetUniformLocation(programShadingPass.glId(), "uShininess");

	uKaSampler = glGetUniformLocation(programShadingPass.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(programShadingPass.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(programShadingPass.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(programShadingPass.glId(), "uShininessSampler");
	uNormalSampler = glGetUniformLocation(programShadingPass.glId(), "uNormalSampler");
}


//-- RENDER DEPTH PASS ------------------
void ForwardPlusRenderer::prePassRendering(const Scene& scene, const Camera& camera)
{
	renderDepthPass(scene, camera);
	if((renderOptions & RENDER_POINT_LIGHTS) == RENDER_POINT_LIGHTS)
		renderLightCullingPass(scene, camera);
}


//-- RENDER DEPTH PASS ------------------
void ForwardPlusRenderer::renderScene(const Scene& scene, const Camera& camera)
{
	renderShadingPass(scene, camera);
}


//-- RENDER DEPTH PASS ------------------
void ForwardPlusRenderer::postProcessPass(const Scene& scene, const Camera& camera)
{
	if ((renderOptions & RENDER_BLUR) == RENDER_BLUR)
	{
		postProcessBlurPass(shadingRenderedTexture[EMISSIVE_TEXTURE]);
		for(int i = 1; i < nbBlurPass; ++i)
			postProcessBlurPass(bufferBlurred);

		setTexCompositingLayer(2, &bufferBlurred);
	}
	else
		setTexCompositingLayer(2, 0);

	renderGatherPass();	
}


//-- RENDER DEPTH PASS ------------------
void ForwardPlusRenderer::renderDepthPass(const Scene& scene, const Camera& camera)
{
	programDepthPass.use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDepth);
	glViewport(0, 0, windowWidth, windowHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& meshes = scene.getMeshes();

	for (const auto& mesh : meshes)
	{
		glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
		camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

		glUniformMatrix4fv(uDepthModelViewProjMatrix, 1, FALSE, glm::value_ptr(mvpMatrix));
		
		const auto& shapes = mesh.getShapesData();

		glBindVertexArray(mesh.getVao()->getPointer());

		for (const auto& shape : shapes)
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shape.shapeSize), GL_UNSIGNED_INT, (const GLvoid*)(shape.shapeIndex * sizeof(GLuint)));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cerr << "glGetError() : " << err << std::endl;
		exit(1);
	}
}


//-- RENDER LIGHT CULLING PASS --------------
void ForwardPlusRenderer::renderLightCullingPass(const Scene& scene, const Camera& camera)
{
	const std::vector<PointLight>& pointLights = scene.getPointLights();
	if (pointLights.size() <= 0)
		return;

	programLightCullingPass.use();

	// Camera and screen characteristic
	glm::mat4 invProjMatrix = glm::inverse(camera.getProjMatrix());
	glm::mat4 viewProj = camera.getProjMatrix() * camera.getViewMatrix();
	glUniformMatrix4fv(uInverseProjMatrix, 1, FALSE, glm::value_ptr(invProjMatrix));
	glUniformMatrix4fv(uViewMatrixForCulling, 1, FALSE, glm::value_ptr(camera.getViewMatrix()));
	glUniform2fv(uWindowDim, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	// Bind ssbo
	Renderer::bindSsbos(pointLights, 0, uPointLightsForCulling, programLightCullingPass, scene.getSsboPointLights(), GL_STREAM_DRAW);
	glUniform1i(uPointLightsNumberForCulling, static_cast<GLint>(pointLights.size()));

	Renderer::bindSsbosToRead(static_cast<size_t>(nbComputeBlock.x * nbComputeBlock.y * 200), 1, uPointLightsIndexForCulling, programLightCullingPass, ssboPointLightsIndex);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(uDepthMapForCulling, 0);

	glDispatchCompute((GLuint)nbComputeBlock.x, (GLuint)nbComputeBlock.y, (GLuint)nbComputeBlock.z);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		int xGroup = static_cast<int>(ceil(windowWidth / 16.f) * 16);
		int yGroup = static_cast<int>(ceil(windowHeight / 16.f) * 16);
		std::cerr << windowWidth << " -- " << windowHeight << std::endl;
		std::cerr << xGroup << " -- " << yGroup << " -- " << yGroup * xGroup << std::endl;
		std::cerr << "glGetError() : " << err << std::endl;
		exit(1);
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

//-- RENDER SHADING PASS -------------------
void ForwardPlusRenderer::renderShadingPass(const Scene& scene, const Camera& camera)
{
	programShadingPass.use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboShadingPass);
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(uViewMatrixForShading, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
	glUniform2fv(uWindowDimForShading, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	//-- load directional light
	loadDirLights(scene);

	//-- load point light
	loadPointLights(scene);
	
	//-- render meshes
	const auto& meshes = scene.getMeshes();
	for (const auto& mesh : meshes)
		renderMesh(mesh, camera, uModelViewProjMatrixForShading, uModelViewMatrixForShading, uNormalMatrixForShading);

	if((renderOptions & RENDER_EMISSIVE) == RENDER_EMISSIVE)
		renderParticules(scene, camera);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	setTexCompositingLayer(0, &(shadingRenderedTexture[SCENE_TEXTURE]));
	setTexCompositingLayer(1, &(shadingRenderedTexture[EMISSIVE_TEXTURE]));
}


void ForwardPlusRenderer::loadDirLights(const Scene& scene)
{
	if ((renderOptions & RENDER_DIR_LIGHTS) == RENDER_DIR_LIGHTS)
	{
		const auto& directionalLights = scene.getDirectionalLights();
		if (directionalLights.size() > 0)
			Renderer::bindSsbos(directionalLights, 1, uDirectionalLights, programShadingPass, scene.getSsboDirectionalLights(), GL_STREAM_DRAW);

		glUniform1i(uDirectionalLightsNumber, static_cast<GLint>(directionalLights.size()));
	}
	else
		glUniform1i(uDirectionalLightsNumber, 0);
}

void ForwardPlusRenderer::loadPointLights(const Scene& scene)
{
	if ((renderOptions & RENDER_POINT_LIGHTS) == RENDER_POINT_LIGHTS)
	{
		const auto& pointLights = scene.getPointLights();
		if (pointLights.size() > 0)
		{
			Renderer::bindSsbos(pointLights, 2, uPointLights, programShadingPass, scene.getSsboPointLights(), GL_STREAM_DRAW);
			Renderer::bindSsbosToRead(static_cast<size_t>(nbComputeBlock.x * nbComputeBlock.y * 200), 3, uPoinLightIndexForShading, programShadingPass, ssboPointLightsIndex);
		}
		glUniform1i(uPointLightsNumber, static_cast<GLint>(pointLights.size()));
	}
	else
		glUniform1i(uPointLightsNumber, 0);
}

