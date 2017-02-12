#include <glm/gtc/type_ptr.hpp>

#include "ForwardPlusRenderer.hpp"

using namespace qc;

ForwardPlusRenderer::ForwardPlusRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: Renderer(shaderDirectory, windowWidth, windowHeight)
{
	initDepthPass();
	initDepthDebug();
	//initUniforms();
}

ForwardPlusRenderer::~ForwardPlusRenderer()
{
	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	if (depthMap) glDeleteSamplers(1, &depthMap);
	if (textureSampler) glDeleteSamplers(1, &textureSampler);
}

ForwardPlusRenderer::ForwardPlusRenderer(ForwardPlusRenderer&& o)
	:Renderer(o), programDepthPass(std::move(o.programDepthPass)), fboDepth(o.fboDepth), depthMap(o.depthMap), uDepthModelViewProjMatrix(o.uDepthModelViewProjMatrix),
	programForward(std::move(o.programForward)), uModelViewProjMatrix(o.uModelViewProjMatrix), uModelViewMatrix(o.uModelViewMatrix),
	uNormalMatrix(o.uNormalMatrix), uViewMatrix(o.uViewMatrix), uDirectionalLights(o.uDirectionalLights), uDirectionalLightsNumber(o.uDirectionalLightsNumber),
	uPointLights(o.uPointLights), uPointLightsNumber(o.uPointLightsNumber), uKa(o.uKa), uKd(o.uKd), uKs(o.uKs), uShininess(o.uShininess), uKaSampler(o.uKaSampler),
	uKdSampler(o.uKdSampler), uKsSampler(o.uKsSampler), uShininessSampler(o.uShininessSampler)
{
	o.programDepthPass = glmlv::GLProgram();

	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	fboDepth = o.fboDepth;
	o.fboDepth = 0;

	if (depthMap) glDeleteSamplers(1, &depthMap);
	depthMap = o.depthMap;
	o.depthMap = 0;

	programDebugDepth = std::move(o.programDebugDepth);
	o.programDebugDepth = glmlv::GLProgram();

	screenVao = o.screenVao;
	o.screenVao = 0;
	screenVbo = o.screenVbo;
	o.screenVbo = 0;
	uDepthMap = o.uDepthMap;
	o.uDepthMap = 0;

	o.programForward = glmlv::GLProgram();

	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;
}

ForwardPlusRenderer& ForwardPlusRenderer::operator= (ForwardPlusRenderer&& o)
{
	shaderDirectory = o.shaderDirectory;
	windowWidth = o.windowWidth;
	windowHeight = o.windowHeight;

	programDepthPass = std::move(o.programDepthPass);
	o.programForward = glmlv::GLProgram();

	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	fboDepth = o.fboDepth;
	o.fboDepth = 0;

	if (depthMap) glDeleteSamplers(1, &depthMap);
	depthMap = o.depthMap;
	o.depthMap = 0;

	uDepthModelViewProjMatrix = o.uDepthModelViewProjMatrix;

	programDebugDepth = std::move(o.programDebugDepth);
	o.programDebugDepth = glmlv::GLProgram();

	screenVao = o.screenVao;
	o.screenVao = 0;
	screenVbo = o.screenVbo;
	o.screenVbo = 0;
	uDepthMap = o.uDepthMap;
	o.uDepthMap = 0;

	programForward = std::move(o.programForward);
	o.programForward = glmlv::GLProgram();

	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;

	uModelViewProjMatrix = o.uModelViewProjMatrix;
	uModelViewMatrix = o.uModelViewMatrix;
	uNormalMatrix = o.uNormalMatrix;

	uViewMatrix = o.uViewMatrix;

	uDirectionalLights = o.uDirectionalLights;
	uDirectionalLightsNumber = o.uDirectionalLightsNumber;

	uPointLights = o.uPointLights;
	uPointLightsNumber = o.uPointLightsNumber;

	uKa = o.uKa;
	uKd = o.uKd;
	uKs = o.uKs;
	uShininess = o.uShininess;
	uKaSampler = o.uKaSampler;
	uKdSampler = o.uKdSampler;
	uKsSampler = o.uKsSampler;
	uShininessSampler = o.uShininessSampler;

	return *this;
}

void ForwardPlusRenderer::renderScene(const Scene& scene, const Camera& camera)
{
	renderDepthPass(scene, camera);
	renderDepthDebug();
	//renderShadingPass(scene, camera);
}

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

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initDepthPass" << std::endl;
}

void ForwardPlusRenderer::initDepthDebug()
{
	programDebugDepth = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusDepthPassDebug.vs.glsl" , shaderDirectory / "forwardPlus" / "forwardPlusDepthPassDebug.fs.glsl" });

	uDepthMap = glGetUniformLocation(programDebugDepth.glId(), "uDepthMap");

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
		std::cerr << "Error : " << error << " in initDepthDebug" << std::endl;
}

void ForwardPlusRenderer::initUniforms()
{
	programForward = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusShadingPass.vs.glsl" , shaderDirectory / "forwardPlus" / "forwardPlusShadingPass.fs.glsl" });

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uModelViewProjMatrix = glGetUniformLocation(programForward.glId(), "uModelViewProjMatrix");
	uModelViewMatrix = glGetUniformLocation(programForward.glId(), "uModelViewMatrix");
	uNormalMatrix = glGetUniformLocation(programForward.glId(), "uNormalMatrix");

	uViewMatrix = glGetUniformLocation(programForward.glId(), "uViewMatrix");

	uDirectionalLights = glGetProgramResourceIndex(programForward.glId(), GL_SHADER_STORAGE_BLOCK, "uDirectionalLights");
	uDirectionalLightsNumber = glGetUniformLocation(programForward.glId(), "uDirectionalLightsNumber");

	uPointLights = glGetProgramResourceIndex(programForward.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLights");
	uPointLightsNumber = glGetUniformLocation(programForward.glId(), "uPointLightsNumber");
	
	uKa = glGetUniformLocation(programForward.glId(), "uKa");
	uKd = glGetUniformLocation(programForward.glId(), "uKd");
	uKs = glGetUniformLocation(programForward.glId(), "uKs");
	uShininess = glGetUniformLocation(programForward.glId(), "uShininess");

	uKaSampler = glGetUniformLocation(programForward.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(programForward.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(programForward.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(programForward.glId(), "uShininessSampler");

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initUniforms" << std::endl;
}

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

		glBindVertexArray(mesh.getVao().getPointer());

		for (const auto& shape : shapes)
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shape.shapeSize), GL_UNSIGNED_INT, (const GLvoid*)(shape.shapeIndex * sizeof(GLuint)));
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ForwardPlusRenderer::renderDepthDebug()
{
	programDebugDepth.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(screenVao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(uDepthMap, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
}

void ForwardPlusRenderer::renderShadingPass(const Scene& scene, const Camera& camera)
{
	programForward.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& directionalLights = scene.getDirectionalLights();
	std::vector<Light> directionalPointLights; //TODO: revoir
	for (const auto& it : directionalLights)
		directionalPointLights.push_back(it);

	const auto& pointLights = scene.getPointLights();
	const auto& meshes = scene.getMeshes();

	glUniformMatrix4fv(uViewMatrix, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));

	Renderer::bindSsbos(directionalPointLights, 1, uDirectionalLights, programForward, scene.getSsboDirectionalLights());
	glUniform1i(uDirectionalLightsNumber, static_cast<GLint>(directionalLights.size()));

	Renderer::bindSsbos(pointLights, 2, uPointLights, programForward, scene.getSsboPointLights());
	glUniform1i(uPointLightsNumber, static_cast<GLint>(pointLights.size()));

	for (const auto& mesh : meshes)
	{
		renderMeshShaddingPass(mesh, camera);
	}
}

void ForwardPlusRenderer::renderMeshShaddingPass(const Mesh& mesh, const Camera& camera)
{
	glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
	camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

	glUniformMatrix4fv(uModelViewProjMatrix, 1, FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(uModelViewMatrix, 1, FALSE, glm::value_ptr(mvMatrix));
	glUniformMatrix4fv(uNormalMatrix, 1, FALSE, glm::value_ptr(normalMatrix));

	glBindSampler(0, textureSampler);
	glBindSampler(1, textureSampler);
	glBindSampler(2, textureSampler);
	glBindSampler(3, textureSampler);

	glUniform1i(uKaSampler, 0);
	glUniform1i(uKdSampler, 1);
	glUniform1i(uKsSampler, 2);
	glUniform1i(uShininessSampler, 3);

	const auto& materials = mesh.getMaterials();
	const auto& shapes = mesh.getShapesData();
	const auto& defaultMaterial = Mesh::defaultMaterial;
	const Material* currentMaterial = nullptr;

	glBindVertexArray(mesh.getVao().getPointer());

	for (const auto& shape : shapes)
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


void ForwardPlusRenderer::bindMeshMaterial(const Material& material)
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