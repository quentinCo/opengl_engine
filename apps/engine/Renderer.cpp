#include <glm/gtc/type_ptr.hpp>

#include "Renderer.hpp"

using namespace qc;

Renderer::Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: shaderDirectory(shaderDirectory), windowWidth(static_cast<GLsizei>(windowWidth)), windowHeight(static_cast<GLsizei>(windowHeight))
{
	initOpenGLProperties();
}

void Renderer::initOpenGLProperties()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);
}
