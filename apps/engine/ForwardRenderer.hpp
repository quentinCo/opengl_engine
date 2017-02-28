#pragma once

#include "Renderer.hpp"

namespace qc
{

class ForwardRenderer : public Renderer
{
	
public:
	ForwardRenderer() {}
	ForwardRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	virtual ~ForwardRenderer();

	ForwardRenderer(ForwardRenderer&& o);
	virtual ForwardRenderer& operator= (ForwardRenderer&& o);

	virtual void renderScene(const Scene& scene, const Camera& camera);

private:
	glmlv::GLProgram programForward;

	GLint uModelViewProjMatrix;
	GLint uModelViewMatrix;
	GLint uNormalMatrix;

	GLint uViewMatrix;
	//  Light
	GLint uDirectionalLights;
	GLint uDirectionalLightsNumber;

	GLint uPointLights;
	GLint uPointLightsNumber;

	void initUniforms();

	/*void renderMesh(const Mesh& mesh, const Camera& camera);
	void bindMeshMaterial(const Material& material);*/
};

}