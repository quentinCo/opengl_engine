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
	/*
	template<typename T>
	static void bindUbos(const std::vector<T>& data, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ubo)
	{
		glUniformBlockBinding(program.glId(), uniform, bindingIndex);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo.getPointer());
		glBufferData(GL_UNIFORM_BUFFER, data.size() * sizeof(T), data.data(), GL_STREAM_DRAW);
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, ubo.getPointer(), 0, sizeof(T) * data.size());
	}
	*/
	template<typename T>
	static void bindSsbos(const std::vector<T>& data, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ssbo, GLenum usage)
	{
		glShaderStorageBlockBinding(program.glId(), uniform, bindingIndex);
				
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.getPointer());
		glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), usage);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo.getPointer(), 0, sizeof(T) * data.size());
	}
};

}