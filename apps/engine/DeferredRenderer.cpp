#include <glm/gtc/type_ptr.hpp>

#include "DeferredRenderer.hpp"

using namespace qc;

const GLenum DeferredRenderer::gBufferTextureFormat[GBUFFER_NB_TEXTURE] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

DeferredRenderer::DeferredRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: Renderer(shaderDirectory, windowWidth, windowHeight)
{
	initOpenGLProperties();
	initGeoPassVariables();
	initComputePassVariables();
	initShadingPassVariables();
}

DeferredRenderer::~DeferredRenderer()
{
	if (screenVbo) glDeleteBuffers(1, &screenVbo);
	if (screenVao) glDeleteBuffers(1, &screenVao);
	if (gBufferTextures) glDeleteTextures(GBUFFER_NB_TEXTURE, gBufferTextures);
	if (fbo) glDeleteFramebuffers(1, &fbo);
	if (screenTexture) glDeleteTextures(1, &screenTexture);
}

DeferredRenderer::DeferredRenderer(DeferredRenderer&& o)
	: programGeoPass(std::move(o.programGeoPass)), uModelViewProjMatrix(o.uModelViewProjMatrix),
	uModelViewMatrix(o.uModelViewMatrix), uNormalMatrix(o.uNormalMatrix), fbo(o.fbo),
	programShadingPass(std::move(o.programShadingPass)), screenVao(o.screenVao), screenVbo(o.screenVbo), uScreenTexture(o.uScreenTexture),
	programComputePass(std::move(o.programComputePass)), screenTexture(o.screenTexture), uDirectionalLights(o.uDirectionalLights),
	uDirectionalLightsNumber(o.uDirectionalLightsNumber), uPointLights(o.uPointLights), uPointLightsNumber(o.uPointLightsNumber), uViewMatrix(o.uViewMatrix),
	uWindowDim(o.uWindowDim)
{
	o.programGeoPass = glmlv::GLProgram();

	for (size_t i = 0; i < GBUFFER_NB_TEXTURE; ++i)
	{
		gBufferTextures[i] = o.gBufferTextures[i];
		o.gBufferTextures[i] = 0;

		uGTextures[i] = o.uGTextures[i];
	}
	o.fbo = 0;
	o.programShadingPass = glmlv::GLProgram();
	o.screenVao = 0;
	o.screenVbo = 0;
	o.programComputePass = glmlv::GLProgram();
	o.screenTexture = 0;
}

DeferredRenderer& DeferredRenderer::operator= (DeferredRenderer&& o)
{
	shaderDirectory = o.shaderDirectory;
	windowWidth = o.windowWidth;
	windowHeight = o.windowHeight;

	programGeoPass = std::move(o.programGeoPass);
	o.programGeoPass = glmlv::GLProgram();
	
	uModelViewProjMatrix = o.uModelViewProjMatrix;
	uModelViewMatrix = o.uModelViewMatrix;
	uNormalMatrix = o.uNormalMatrix;
	uKa = o.uKa;
	uKd = o.uKd;
	uKs = o.uKs;
	uShininess = o.uShininess;
	uKaSampler = o.uKaSampler;
	uKdSampler = o.uKdSampler;
	uKsSampler = o.uKsSampler;
	uShininessSampler = o.uShininessSampler;

	for (size_t i = 0; i < GBUFFER_NB_TEXTURE; ++i)
	{
		gBufferTextures[i] = o.gBufferTextures[i];
		o.gBufferTextures[i] = 0;

		uGTextures[i] = o.uGTextures[i];
	}

	fbo = o.fbo;
	o.fbo = 0;

	programShadingPass = std::move(o.programShadingPass);
	o.programShadingPass = glmlv::GLProgram();

	screenVao = o.screenVao;
	o.screenVao = 0;
	screenVbo = o.screenVbo;
	o.screenVbo = 0;
	uScreenTexture = o.uScreenTexture;

	programComputePass = std::move(o.programComputePass);
	o.programComputePass = glmlv::GLProgram();

	screenTexture = o.screenTexture;
	o.screenTexture = 0;

	uDirectionalLights = o.uDirectionalLights;
	uDirectionalLightsNumber = o.uDirectionalLightsNumber;
	uPointLights = o.uPointLights;
	uPointLightsNumber = o.uPointLightsNumber;
	uViewMatrix = o.uViewMatrix;

	uWindowDim = o.uWindowDim;
	return *this;
}

void DeferredRenderer::renderScene(const Scene& scene, const Camera& camera)
{
	renderGeoPass(scene, camera);
	renderComputePass(scene, camera);
	renderShadingPass();
}

void DeferredRenderer::initGeoPassVariables()
{
	programGeoPass = glmlv::compileProgram({ shaderDirectory / "general" / "geometryPass.vs.glsl", shaderDirectory / "deferred" / "geometryPass.fs.glsl" });

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uModelViewProjMatrix = glGetUniformLocation(programGeoPass.glId(), "uModelViewProjMatrix");
	uModelViewMatrix = glGetUniformLocation(programGeoPass.glId(), "uModelViewMatrix");
	uNormalMatrix = glGetUniformLocation(programGeoPass.glId(), "uNormalMatrix");

	uKa = glGetUniformLocation(programGeoPass.glId(), "uKa");
	uKd = glGetUniformLocation(programGeoPass.glId(), "uKd");
	uKs = glGetUniformLocation(programGeoPass.glId(), "uKs");
	uShininess = glGetUniformLocation(programGeoPass.glId(), "uShininess");
	uKaSampler = glGetUniformLocation(programGeoPass.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(programGeoPass.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(programGeoPass.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(programGeoPass.glId(), "uShininessSampler");

	initFBOGeoPass();

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initGeoPass" << std::endl;
}

void DeferredRenderer::initFBOGeoPass()
{
	// GBufferTexture
	glGenTextures(GBUFFER_NB_TEXTURE, gBufferTextures);
	for (int i = 0; i < GBUFFER_NB_TEXTURE; i++)
	{
		glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, gBufferTextureFormat[i], windowWidth, windowHeight);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	// FBO init
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	for (int i = 0; i < GBUFFER_DEPTH; ++i)
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, gBufferTextures[i], 0);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBufferTextures[GBUFFER_DEPTH], 0);

	GLenum drawBuffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4
	};
	glDrawBuffers(5, drawBuffers);

	GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (res != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error check frame buffer : " << res << std::endl;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void DeferredRenderer::initComputePassVariables()
{
	programComputePass = glmlv::compileProgram({ shaderDirectory / "deferred" / "computePass.cs.glsl" });

	// Gen texture
	glGenTextures(1, &screenTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, screenTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	uDirectionalLights = glGetProgramResourceIndex(programComputePass.glId(), GL_SHADER_STORAGE_BLOCK, "uDirectionalLights");
	uDirectionalLightsNumber = glGetUniformLocation(programComputePass.glId(), "uDirectionalLightsNumber");

	uPointLights = glGetProgramResourceIndex(programComputePass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLights");
	uPointLightsNumber = glGetUniformLocation(programComputePass.glId(), "uPointLightsNumber");

	uViewMatrix = glGetUniformLocation(programComputePass.glId(), "uViewMatrix");

	uGTextures[0] = glGetUniformLocation(programComputePass.glId(), "uGPosition");
	uGTextures[1] = glGetUniformLocation(programComputePass.glId(), "uGNormal");
	uGTextures[2] = glGetUniformLocation(programComputePass.glId(), "uGAmbient");
	uGTextures[3] = glGetUniformLocation(programComputePass.glId(), "uGDiffuse");
	uGTextures[4] = glGetUniformLocation(programComputePass.glId(), "uGlossyShininess");

	uWindowDim = glGetUniformLocation(programComputePass.glId(), "uWindowDim");

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initComputePass" << std::endl;
}

void DeferredRenderer::initShadingPassVariables()
{
	programShadingPass = glmlv::compileProgram({ shaderDirectory / "deferred" / "shadingPass.vs.glsl",shaderDirectory / "deferred" / "shadingPass.fs.glsl" });

	uScreenTexture = glGetUniformLocation(programShadingPass.glId(), "uScreenTexture");

	// screen buffers
	glm::vec2 triangle[3];
	triangle[0] = glm::vec2(-1);
	triangle[1] = glm::vec2(3, -1);
	triangle[2] = glm::vec2(-1, 3);

	glGenBuffers(1, &screenVbo);
	glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
	glBufferStorage(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec2), triangle, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &screenVao);
	glBindVertexArray(screenVao);
	glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initShaddingPass" << std::endl;
}

void DeferredRenderer::renderGeoPass(const Scene& scene, const Camera& camera)
{
	programGeoPass.use();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& meshes = scene.getMeshes();
	for (const auto& mesh : meshes)
	{
		//renderGeoPassMesh(mesh, camera);
		renderMesh(mesh, camera, uModelViewProjMatrix, uModelViewMatrix, uNormalMatrix);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}
/*
void DeferredRenderer::renderGeoPassMesh(const Mesh& mesh, const Camera& camera)
{
	glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
	camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

	glUniformMatrix4fv(uModelViewProjMatrix, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(uModelViewMatrix, 1, GL_FALSE, glm::value_ptr(mvMatrix));
	glUniformMatrix4fv(uNormalMatrix, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	// Same sampler for all texture units
	glBindSampler(0, textureSampler);
	glBindSampler(1, textureSampler);
	glBindSampler(2, textureSampler);
	glBindSampler(3, textureSampler);

	// Set texture unit of each sampler
	glUniform1i(uKaSampler, 0);
	glUniform1i(uKdSampler, 1);
	glUniform1i(uKsSampler, 2);
	glUniform1i(uShininessSampler, 3);

	glBindVertexArray(mesh.getVao().getPointer());

	const Material* currentMaterial = nullptr;
	const std::vector<Material>& materials = mesh.getMaterials();
	const Material& defaultMaterial = Mesh::defaultMaterial;

	const auto& shapes = mesh.getShapesData();
	for (const auto shape : shapes)
	{
		const auto& material = (shape.materialIndex >= 0) ? materials[shape.materialIndex] : defaultMaterial;
		if (currentMaterial != &material)
		{
			bindMeshMaterial(material);
			currentMaterial = &material;
		}

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shape.shapeSize), GL_UNSIGNED_INT, (const GLvoid*)(shape.shapeIndex * sizeof(GLuint)));
	}

	for (GLuint i : {0, 1, 2, 3})
		glBindSampler(i, 0);
}

void DeferredRenderer::bindMeshMaterial(const Material& material)
{
	glUniform3fv(uKa, 1, glm::value_ptr(material.getColor(Material::AMBIENT_COLOR)));
	glUniform3fv(uKd, 1, glm::value_ptr(material.getColor(Material::DIFFUSE_COLOR)));
	glUniform3fv(uKs, 1, glm::value_ptr(material.getColor(Material::SPECULAR_COLOR)));
	glUniform1f(uShininess, material.getShininess());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, material.getMap(Material::AMBIENT_TEXTURE));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, material.getMap(Material::DIFFUSE_TEXTURE));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, material.getMap(Material::SPECULAR_TEXTURE));
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, material.getMap(Material::SPECULAR_HIGHT_LIGHT_TEXTURE));
}
*/
void DeferredRenderer::renderComputePass(const Scene& scene, const Camera& camera)
{
	programComputePass.use();

	const auto& viewMatrix = camera.getViewMatrix();

	const auto& directionalLights = scene.getDirectionalLights();
	std::vector<Light> directionalPointLights;
	for (const auto& it : directionalLights)
		directionalPointLights.push_back(it);

	const auto& pointLights = scene.getPointLights();

	if (directionalLights.size() > 0)
		Renderer::bindSsbos(directionalPointLights, 1, uDirectionalLights, programComputePass, scene.getSsboDirectionalLights(), GL_STREAM_DRAW);
	
	glUniform1i(uDirectionalLightsNumber, static_cast<GLint>(directionalLights.size()));

	if (pointLights.size() > 0)
		Renderer::bindSsbos(pointLights, 2, uPointLights, programComputePass, scene.getSsboPointLights(), GL_STREAM_DRAW);
	
	glUniform1i(uPointLightsNumber, static_cast<GLint>(pointLights.size()));

	glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));

	glUniform2fv(uWindowDim, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	for (int i = 0; i < GBUFFER_DEPTH; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, gBufferTextures[i]);
		glUniform1i(uGTextures[i], i);
	}

	glDispatchCompute((GLuint)ceil(windowWidth / 32.f), (GLuint)ceil(windowHeight / 32.f), 1);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		int xGroup = static_cast<int>(ceil(windowWidth / 32.f) * 32);
		int yGroup = static_cast<int>(ceil(windowHeight / 32.f) * 32);
		std::cerr << windowWidth << " -- " << windowHeight << std::endl;
		std::cerr << xGroup << " -- " << yGroup << " -- " << yGroup * xGroup << std::endl;
		std::cerr << "glGetError() : " << err << std::endl;
		exit(1);
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void DeferredRenderer::renderShadingPass()
{
	programShadingPass.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(screenVao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glUniform1i(uScreenTexture, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
