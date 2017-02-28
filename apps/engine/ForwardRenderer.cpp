#include <glm/gtc/type_ptr.hpp>

#include "ForwardRenderer.hpp"

using namespace qc;

ForwardRenderer::ForwardRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: Renderer(shaderDirectory, windowWidth, windowHeight)
{
	initUniforms();
}

ForwardRenderer::~ForwardRenderer()
{}

ForwardRenderer::ForwardRenderer(ForwardRenderer&& o)
	: programForward(std::move(o.programForward)),uModelViewProjMatrix(o.uModelViewProjMatrix), uModelViewMatrix(o.uModelViewMatrix),
	uNormalMatrix(o.uNormalMatrix), uViewMatrix(o.uViewMatrix), uDirectionalLights(o.uDirectionalLights), uDirectionalLightsNumber(o.uDirectionalLightsNumber),
	uPointLights(o.uPointLights), uPointLightsNumber(o.uPointLightsNumber)
{}

ForwardRenderer& ForwardRenderer::operator= (ForwardRenderer&& o)
{
	programForward = std::move(o.programForward);

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

void ForwardRenderer::renderScene(const Scene& scene, const Camera& camera)
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

	Renderer::bindSsbos(directionalPointLights, 1, uDirectionalLights, programForward, scene.getSsboDirectionalLights(), GL_STREAM_DRAW);
	glUniform1i(uDirectionalLightsNumber, static_cast<GLint>(directionalLights.size()));

	Renderer::bindSsbos(pointLights, 2, uPointLights, programForward, scene.getSsboPointLights(), GL_STREAM_DRAW);
	glUniform1i(uPointLightsNumber, static_cast<GLint>(pointLights.size()));

	for (const auto& mesh : meshes)
	{
		//renderMesh(mesh, camera);
		renderMesh(mesh, camera, uModelViewProjMatrix, uModelViewMatrix, uNormalMatrix);
	}

}

void ForwardRenderer::initUniforms()
{
	programForward = glmlv::compileProgram({ shaderDirectory / "general" / "geometryPass.vs.glsl" , shaderDirectory / "forward" / "forward.fs.glsl" });

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
}
/*
void ForwardRenderer::renderMesh(const Mesh& mesh, const Camera& camera)
{
	glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
	camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

	glUniformMatrix4fv(uModelViewMatrix, 1, FALSE, glm::value_ptr(mvMatrix));
	glUniformMatrix4fv(uModelViewProjMatrix, 1, FALSE, glm::value_ptr(mvpMatrix));
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


void ForwardRenderer::bindMeshMaterial(const Material& material)
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