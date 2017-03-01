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
}

Renderer::Renderer(Renderer&& o)
	: shaderDirectory(o.shaderDirectory), windowWidth(o.windowWidth), windowHeight(o.windowHeight), uKa(o.uKa), uKd(o.uKd), uKs(o.uKs),
	uShininess (o.uShininess), uKaSampler(o.uKaSampler), uKdSampler(o.uKdSampler), uKsSampler(o.uKsSampler), uShininessSampler(o.uShininessSampler),
	programEmissivePass(std::move(o.programEmissivePass)), uMVPMatrixEmissivePass(o.uMVPMatrixEmissivePass), uMVMatrixEmissivePass(o.uMVMatrixEmissivePass), uKe(o.uKe)
{
	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;
}

Renderer& Renderer::operator=(Renderer&& o)
{
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

	uMVPMatrixEmissivePass = glGetUniformLocation(programEmissivePass.glId(), "uModelViewProjMatrix");
	uMVMatrixEmissivePass = glGetUniformLocation(programEmissivePass.glId(), "uModelViewMatrix");
	uNormalMatrixEmissivePass = -1; // TODO

	uKe = glGetUniformLocation(programEmissivePass.glId(), "uKe");
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
}

void Renderer::bindEmissiveMaterial(const Material& material)
{
	glUniform3fv(uKe, 1,glm::value_ptr(material.getColor(Material::EMMISIVE_COLOR)));
}