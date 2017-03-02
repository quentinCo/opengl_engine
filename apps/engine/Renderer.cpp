#include <functional>

#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"

using namespace qc;

Renderer::Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: shaderDirectory(shaderDirectory), windowWidth(static_cast<GLsizei>(windowWidth)), windowHeight(static_cast<GLsizei>(windowHeight))
{
	initOpenGLProperties();
}

Renderer::~Renderer()
{
	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	if (bufferTexEmissivePass) glDeleteTextures(1, &bufferTexEmissivePass);
	if (fboEmissivePass) glDeleteFramebuffers(1, &fboEmissivePass);
	if (screenVboGather) glDeleteBuffers(1, &screenVboGather);
	if (screenVaoGather) glDeleteBuffers(1, &screenVaoGather);
}

Renderer::Renderer(Renderer&& o)
	: shaderDirectory(o.shaderDirectory), windowWidth(o.windowWidth), windowHeight(o.windowHeight), fboEmissivePass(o.fboEmissivePass),
	uKa(o.uKa), uKd(o.uKd), uKs(o.uKs),	uShininess (o.uShininess), uKaSampler(o.uKaSampler), uKdSampler(o.uKdSampler), uKsSampler(o.uKsSampler),
	uShininessSampler(o.uShininessSampler), programEmissivePass(std::move(o.programEmissivePass)), uMVPMatrixEmissivePass(o.uMVPMatrixEmissivePass),
	uMVMatrixEmissivePass(o.uMVMatrixEmissivePass), uKe(o.uKe), programBlurPass (std::move(o.programBlurPass)), bufferBlurredTexPass1(o.bufferBlurredTexPass1),
	bufferBlurredTex (o.bufferBlurredTex),
	uInitTex (o.uInitTex), uWindowDimBlur (o.uWindowDimBlur), uDirectionBlur (o.uDirectionBlur), programGatherPass(std::move(o.programGatherPass)),
	screenVaoGather (o.screenVaoGather), screenVboGather (o.screenVboGather), uTexScreen (o.uTexScreen)
{
	if (bufferTexEmissivePass) glDeleteTextures(1, &bufferTexEmissivePass);
	bufferTexEmissivePass = o.bufferTexEmissivePass;
	o.bufferTexEmissivePass = 0;

	o.fboEmissivePass = 0;

	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;

}

Renderer& Renderer::operator=(Renderer&& o)
{
	if (bufferTexEmissivePass) glDeleteTextures(1, &bufferTexEmissivePass);
	bufferTexEmissivePass = o.bufferTexEmissivePass;
	o.bufferTexEmissivePass = 0;

	fboEmissivePass = o.fboEmissivePass;
	o.fboEmissivePass = 0;

	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;

	uKa = o.uKa;
	uKd = o.uKd;
	uKs = o.uKs;
	uShininess = o.uShininess;
	uKaSampler = o.uKaSampler;
	uKdSampler = o.uKdSampler;
	uKsSampler = o.uKsSampler;
	uShininessSampler = o.uShininessSampler;

	programEmissivePass = std::move(o.programEmissivePass);
	uMVPMatrixEmissivePass = o.uMVPMatrixEmissivePass;
	uMVMatrixEmissivePass = o.uMVMatrixEmissivePass;
	uKe = o.uKe;

	programBlurPass = std::move(o.programBlurPass);
	bufferBlurredTexPass1 = o.bufferBlurredTexPass1;
	bufferBlurredTex = o.bufferBlurredTex;
	uInitTex = o.uInitTex;
	uWindowDimBlur = o.uWindowDimBlur;
	uDirectionBlur = o.uDirectionBlur;

	programGatherPass = std::move(o.programGatherPass);
	screenVaoGather = o.screenVaoGather;
	screenVboGather = o.screenVboGather;
	uTexScreen = o.uTexScreen;

	return *this;
}

void Renderer::initOpenGLProperties()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
}

void Renderer::initEmissivePass() 
{
	programEmissivePass = glmlv::compileProgram({ shaderDirectory / "general" / "emissiveElement.vs.glsl" , shaderDirectory / "general" / "emissiveElement.fs.glsl" });

	// TODO : create class -> FrameBuffer(size, textures, depth = true);
	glGenTextures(1, &bufferTexEmissivePass);
	glBindTexture(GL_TEXTURE_2D, bufferTexEmissivePass);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, windowWidth, windowHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fboEmissivePass);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboEmissivePass);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTexEmissivePass, 0);
	
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (res != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Error check emissive frame buffer : " << res << std::endl;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	uMVPMatrixEmissivePass = glGetUniformLocation(programEmissivePass.glId(), "uModelViewProjMatrix");
	uMVMatrixEmissivePass = glGetUniformLocation(programEmissivePass.glId(), "uModelViewMatrix");
	uNormalMatrixEmissivePass = -1; // TODO

	uKe = glGetUniformLocation(programEmissivePass.glId(), "uKe");
}

void Renderer::initBlurPass()
{
	programBlurPass = glmlv::compileProgram({ shaderDirectory / "postProcess" / "blur.cs.glsl" });

	glGenTextures(1, &bufferBlurredTexPass1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferBlurredTexPass1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, bufferBlurredTexPass1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &bufferBlurredTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferBlurredTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, bufferBlurredTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	uInitTex = glGetUniformLocation(programBlurPass.glId(), "uInitTex");
	uWindowDimBlur = glGetUniformLocation(programBlurPass.glId(), "uWindowDim");
	uDirectionBlur = glGetUniformLocation(programBlurPass.glId(), "uDirection");
}

void Renderer::initGatherPass()
{
	programGatherPass = glmlv::compileProgram({ shaderDirectory / "deferred" / "shadingPass.vs.glsl" , shaderDirectory / "deferred" / "shadingPass.fs.glsl" });

	glm::vec2 triangle[3];
	triangle[0] = glm::vec2(-1);
	triangle[1] = glm::vec2(3, -1);
	triangle[2] = glm::vec2(-1, 3);

	glGenBuffers(1, &screenVboGather);
	glBindBuffer(GL_ARRAY_BUFFER, screenVboGather);
	glBufferStorage(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec2), triangle, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &screenVaoGather);
	glBindVertexArray(screenVaoGather);
	glBindBuffer(GL_ARRAY_BUFFER, screenVboGather);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	uTexScreen = glGetUniformLocation(programGatherPass.glId(), "uScreenTexture");
}

//void Renderer::renderMesh(const Mesh& mesh, const Camera& camera, GLint& uMVPMatrix, GLint& uMVMatrix, GLint& uNormalMatrix, MeshRenderType meshRenderType)
void Renderer::renderMesh(const Mesh& mesh, const Camera& camera, GLint& uMVPMatrix, GLint& uMVMatrix, GLint& uNormalMatrix)
{
	glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
	camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

	glUniformMatrix4fv(uMVPMatrix, 1, FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(uMVMatrix, 1, FALSE, glm::value_ptr(mvMatrix));
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

void Renderer::renderEmissivePass(const Scene& scene, const Camera& camera)
{
	programEmissivePass.use();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboEmissivePass);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& particules = scene.getParticules();

	for (const auto& particule : particules)
	{
		glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
		camera.computeModelsMatrix(particule.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

		glUniformMatrix4fv(uMVPMatrixEmissivePass, 1, FALSE, glm::value_ptr(mvpMatrix));
		glUniformMatrix4fv(uMVMatrixEmissivePass, 1, FALSE, glm::value_ptr(mvMatrix));
		
		const auto& materials = particule.getMaterials();
		const auto& shapes = particule.getShapesData();
		const auto& defaultMaterial = Mesh::defaultMaterial;
		const Material* currentMaterial = nullptr;

		glBindVertexArray(particule.getVao().getPointer());

		for (const auto& shape : shapes)
		{
			const auto& material = (shape.materialIndex >= 0) ? materials[shape.materialIndex] : defaultMaterial;
			if (currentMaterial != &material)
			{
				bindEmissiveMaterial(material);
				currentMaterial = &material;
			}

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(shape.shapeSize), GL_UNSIGNED_INT, (const GLvoid*)(shape.shapeIndex * sizeof(GLuint)));
		}
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, fboEmissivePass);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);*/
}

void Renderer::bindEmissiveMaterial(const Material& material)
{
	glUniform3fv(uKe, 1,glm::value_ptr(material.getColor(Material::EMMISIVE_COLOR)));
}


void Renderer::postProcessBlurPass(GLuint tex)
{
	programBlurPass.use();

	glUniform2fv(uWindowDimBlur, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	glBindImageTexture(0, bufferBlurredTexPass1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uInitTex, 0);


	postProcessDirectionalBlurPass(0); // TODO enum;

	glBindImageTexture(0, bufferBlurredTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, bufferBlurredTexPass1);

	postProcessDirectionalBlurPass(1);
}

void Renderer::postProcessDirectionalBlurPass(int direction)
{
	glUniform1i(uDirectionBlur, direction);

	glDispatchCompute((GLuint)ceil(windowWidth / 32.f), (GLuint)ceil(windowHeight / 32.f), 1);
	GLenum err = glGetError();
	assert(err == GL_NO_ERROR);
}

void Renderer::renderGatherPass(GLuint tex)
{
	programGatherPass.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(screenVaoGather);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uTexScreen, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
