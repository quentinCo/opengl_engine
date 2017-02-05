#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"

using namespace qc;

const GLenum Renderer::gBufferTextureFormat[GBUFFER_NB_TEXTURE] = { GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGB32F, GL_RGBA32F, GL_DEPTH_COMPONENT32F };

Renderer::Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: shaderDirectory(shaderDirectory), windowWidth(windowWidth), windowHeight(windowHeight)
{
	initOpenGLProperties();
	initGeoPassVariables();
	initComputePassVariables();
	initShadingPassVariables();
}

Renderer::~Renderer()
{
	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	if (screenVbo) glDeleteBuffers(1, &screenVbo);
	if (screenVao) glDeleteBuffers(1, &screenVao);
	if(gBufferTextures) glDeleteTextures(GBUFFER_NB_TEXTURE, gBufferTextures);
	if (fbo) glDeleteFramebuffers(1, &fbo);
	if (screenTexture) glDeleteTextures(1, &screenTexture);
}

Renderer::Renderer(Renderer&& o)
	:shaderDirectory(o.shaderDirectory) ,windowWidth (o.windowWidth), windowHeight (o.windowHeight), programGeoPass (std::move(o.programGeoPass)),
	textureSampler(o.textureSampler), uModelViewProjMatrix (o.uModelViewProjMatrix), uModelViewMatrix (o.uModelViewMatrix), uNormalMatrix (o.uNormalMatrix),
	uKa (o.uKa), uKd (o.uKd), uKs (o.uKs), uShininess (o.uShininess), uKaSampler (o.uKaSampler), uKdSampler (o.uKdSampler), uKsSampler (o.uKsSampler),
	uShininessSampler (o.uShininessSampler), fbo (o.fbo), programShadingPass (std::move(o.programShadingPass)), screenVao (o.screenVao), screenVbo (o.screenVbo),
	uScreenTexture (o.uScreenTexture), programComputePass (std::move(o.programComputePass)), screenTexture (o.screenTexture), uDirectionalLightDirection (o.uDirectionalLightDirection),
	uDirectionalLightIntensity (o.uDirectionalLightIntensity), uPointLightPosition (o.uPointLightPosition), uPointLightIntensity (o.uPointLightIntensity), uWindowDim (o.uWindowDim)
{
	o.programGeoPass = glmlv::GLProgram();
	o.textureSampler = 0;

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

Renderer& Renderer::operator= (Renderer&& o)
{
	shaderDirectory = o.shaderDirectory;
	windowWidth = o.windowWidth;
	windowHeight = o.windowHeight;
	programGeoPass = std::move(o.programGeoPass);
	o.programGeoPass = glmlv::GLProgram();

	textureSampler = o.textureSampler;
	o.textureSampler = 0;

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
	uDirectionalLightDirection = o.uDirectionalLightDirection;
	uDirectionalLightIntensity = o.uDirectionalLightIntensity;
	uPointLightPosition = o.uPointLightPosition;
	uPointLightIntensity = o.uPointLightIntensity;

	uWindowDim = o.uWindowDim;
	return *this;
}

void Renderer::renderScene(const Scene& scene, const Camera& camera)
{
	renderGeoPass(scene, camera);
	renderComputePass(scene, camera);
	renderShadingPass();
}

void Renderer::initOpenGLProperties()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
}

void Renderer::initGeoPassVariables()
{
	programGeoPass = glmlv::compileProgram({ shaderDirectory / "geometryPass.vs.glsl", shaderDirectory / "geometryPass.fs.glsl" });

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
}

void Renderer::initFBOGeoPass()
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
	//attachedToDraw = GL_COLOR_ATTACHMENT0;
}

void Renderer::initComputePassVariables()
{
	programComputePass = glmlv::compileProgram({ shaderDirectory / "computePass.cs.glsl" });

	// Gen texture
	glGenTextures(1, &screenTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, screenTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	uDirectionalLightDirection = glGetUniformLocation(programComputePass.glId(), "uDirectionalLightDir");
	uDirectionalLightIntensity = glGetUniformLocation(programComputePass.glId(), "uDirectionalLightIntensity");

	uPointLightPosition = glGetUniformLocation(programComputePass.glId(), "uPointLightPosition");
	uPointLightIntensity = glGetUniformLocation(programComputePass.glId(), "uPointLightIntensity");

	uGTextures[0] = glGetUniformLocation(programComputePass.glId(), "uGPosition");
	uGTextures[1] = glGetUniformLocation(programComputePass.glId(), "uGNormal");
	uGTextures[2] = glGetUniformLocation(programComputePass.glId(), "uGAmbient");
	uGTextures[3] = glGetUniformLocation(programComputePass.glId(), "uGDiffuse");
	uGTextures[4] = glGetUniformLocation(programComputePass.glId(), "uGlossyShininess");

	uWindowDim = glGetUniformLocation(programComputePass.glId(), "uWindowsDim");
}

void Renderer::initShadingPassVariables()
{
	programShadingPass = glmlv::compileProgram({shaderDirectory / "shadingPass.vs.glsl",shaderDirectory / "shadingPass.fs.glsl" });
	
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
}

void Renderer::renderGeoPass(const Scene& scene, const Camera& camera)
{
	programGeoPass.use();
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& meshes = scene.getMeshes();
	for (const auto& mesh : meshes)
	{
		renderGeoPassMesh(mesh, camera);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Renderer::renderGeoPassMesh(const Mesh& mesh, const Camera& camera)
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

	const qc::Material* currentMaterial = nullptr;
	const std::vector<qc::Material>& materials = mesh.getMaterials();
	const qc::Material& defaultMaterial = qc::Mesh::defaultMaterial;

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
		glBindSampler(0, textureSampler);

}

void Renderer::bindMeshMaterial(const Material& material)
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

void Renderer::renderComputePass(const Scene& scene, const Camera& camera)
{
	programComputePass.use();

	const auto& viewMatrix = camera.getViewMatrix();

	const auto& directionalLights = scene.getDirectionalLights();
	const auto& pointLights = scene.getPointLights();

	const auto& directionalLight = directionalLights[0];
	const auto& pointLight = pointLights[0];

	glUniform3fv(uDirectionalLightDirection, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(directionalLight.getDirection()), 0))));
	glUniform3fv(uDirectionalLightIntensity, 1, glm::value_ptr(directionalLight.getColor() * directionalLight.getIntensity()));

	glUniform3fv(uPointLightPosition, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLight.getPosition(), 1))));
	glUniform3fv(uPointLightIntensity, 1, glm::value_ptr(pointLight.getColor() * pointLight.getIntensity()));

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

void Renderer::renderShadingPass()
{
	programShadingPass.use();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindVertexArray(screenVao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);
	glUniform1i(uScreenTexture, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
