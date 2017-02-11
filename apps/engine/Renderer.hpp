#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLProgram.hpp>

#include "Scene.hpp"
#include "Camera.hpp"

namespace qc
{

class Renderer
{

public:
	enum GBufferTextureType
	{
		GBUFFER_POSITION = 0,
		GBUFFER_NORMAL,
		GBUFFER_AMBIENT,
		GBUFFER_DIFFUSE,
		GBUFFER_GLOSSY_SHININESS,
		GBUFFER_DEPTH, // On doit créer une texture de depth mais on écrit pas directement dedans dans le FS. OpenGL le fait pour nous (et l'utilise).
		GBUFFER_NB_TEXTURE
	};

	static const GLenum gBufferTextureFormat[GBUFFER_NB_TEXTURE];

	Renderer() {}
	Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);

	GLsizei getWindowWidth() const
		{return windowWidth;}
	
	GLsizei getWindowHeight() const
		{return windowHeight;}

	void setWindowsDimmenssion(size_t width, size_t height)
	{
		windowWidth = static_cast<GLsizei>(width);
		windowHeight = static_cast<GLsizei>(height);
	}

	virtual void renderScene(const Scene& scene, const Camera& camera) = 0;

protected:
	glmlv::fs::path shaderDirectory;
	GLsizei windowWidth;
	GLsizei windowHeight;

	void initOpenGLProperties();

	template<typename T>
	static void bindUbos(const std::vector<T>& data, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ubo)
	{
		glUniformBlockBinding(program.glId(), uniform, bindingIndex);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo.getPointer());
		glBufferData(GL_UNIFORM_BUFFER, data.size() * sizeof(T), data.data(), GL_STREAM_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, ubo.getPointer(), 0, sizeof(T) * data.size());
	}
};

}