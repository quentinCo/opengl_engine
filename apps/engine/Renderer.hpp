#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLProgram.hpp>

#include "Scene.hpp"
#include "Camera.hpp"

namespace qc
{

class Renderer
{/*
	enum MeshRenderType
	{
		RENDER_CLASSIC_MESH,
		RENDER_EMISSIVE_MESH
	};*/

public:
	Renderer() {}
	Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	~Renderer();

	/*
	Renderer(const Renderer& o) = delete;
	virtual Renderer& operator=(const Renderer& o) = delete;
	*/
	Renderer(Renderer&& o);
	virtual Renderer& operator=(Renderer&& o);

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

	// Materials
	GLuint textureSampler = 0;

	GLint uKa;
	GLint uKd;
	GLint uKs;
	GLint uShininess;
	GLint uKaSampler;
	GLint uKdSampler;
	GLint uKsSampler;
	GLint uShininessSampler;

	// Emissive Render
	glmlv::GLProgram programEmissivePass;

	GLuint bufferTexEmissivePass = 0;
	GLuint fboEmissivePass = 0;

	GLint uMVPMatrixEmissivePass;
	GLint uMVMatrixEmissivePass;
	GLint uNormalMatrixEmissivePass;

	GLint uKe;

	// Post Process Pass
	//  Blur
	glmlv::GLProgram programBlurPass;

	GLuint bufferBlurredTexPass1 = 0;
	GLuint bufferBlurredTex = 0;
	GLuint uInitTex;
	GLuint uWindowDimBlur;
	GLuint uDirectionBlur;
	//GLuint fboBlur = 0; // No fbo in compute

	// Final Pass
	glmlv::GLProgram programGatherPass;

	/*GLuint fboGather;
	std::vector<GLuint> bufferRenderedPass;*/

	GLuint screenVaoGather = 0;
	GLuint screenVboGather = 0;
	GLuint uTexScreen = 0;


	void initOpenGLProperties();

	virtual void initEmissivePass();
	void initBlurPass();
	void initGatherPass();

	virtual void renderMesh(const Mesh& mesh, const Camera& camera, GLint& uMVPMatrix, GLint& uMVMatrix, GLint& uNormalMatrix);
	virtual void bindMeshMaterial(const Material& material);

	virtual void renderEmissivePass(const Scene& scene, const Camera& camera);
	virtual void bindEmissiveMaterial(const Material& material);
	
	void postProcessBlurPass(GLuint tex/*, GLuint texRes*/);
	void postProcessDirectionalBlurPass(int direction);

	virtual void renderGatherPass(GLuint tex);

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
		if(usage != GL_STREAM_READ) glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), usage);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo.getPointer(), 0, sizeof(T) * data.size());
	}
};

}