#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLProgram.hpp>

#include "Scene.hpp"
#include "Camera.hpp"

namespace qc
{
	
namespace graphic
{

class Renderer
{
public:
	using SharedMaterial = std::shared_ptr<Material>;

	enum RenderOptions
	{
		DONT_RENDER = 0,
		RENDER_EMISSIVE = 1 << 0,
		RENDER_BLUR = 1 << 1,
		RENDER_POINT_LIGHTS = 1 << 2,
		RENDER_DIR_LIGHTS = 1 << 3,
		RENDER_ALL = 0xFFFF
	};

	Renderer() {}
	Renderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight);
	~Renderer();

	Renderer(Renderer&& o);
	virtual Renderer& operator=(Renderer&& o);


	//-- GETTERS ---------------------------
	
	GLsizei getWindowWidth() const
		{return windowWidth;}
	
	GLsizei getWindowHeight() const
		{return windowHeight;}

	int& getNbBlurPass()
		{return nbBlurPass;}


	//-- SETTERS ---------------------------

	void setWindowsDimmenssion(size_t width, size_t height)
	{
		windowWidth = static_cast<GLsizei>(width);
		windowHeight = static_cast<GLsizei>(height);
	}
	
	void setTexCompositingLayer(int numLayer, GLuint *tex)
		{if(numLayer < 10) compositingTextures[numLayer] = tex; }

	void setNbBlurPass(int nb)
		{nbBlurPass = nb;}

	void setRenderOptions(RenderOptions pass)
		{renderOptions = pass;}

	//-- RENDER ---------------------------
	/*
		Render a scene.
		Execute the different pass of the render:
			- pre-pass
			- render
			- post process
	*/
	virtual void render(const Scene& scene, const Camera& camera)
	{
		prePassRendering(scene, camera);
		renderScene(scene, camera);
	
		if(renderOptions != DONT_RENDER)
			postProcessPass(scene, camera);
	}

protected:
	RenderOptions renderOptions = RENDER_ALL;

	//-- Path to the shader directory
	glmlv::fs::path shaderDirectory;

	//-- Window dimension
	GLsizei windowWidth;
	GLsizei windowHeight;

	//-- Materials Uniform
	GLuint textureSampler = 0;

	GLint uKa;
	GLint uKd;
	GLint uKs;
	GLint uKe;
	GLint uShininess;
	GLint uKaSampler;
	GLint uKdSampler;
	GLint uKsSampler;
	GLint uShininessSampler;
	GLint uNormalSampler;

	//-- Emissive Rendering Variables
	//---- Emissive Pass Program
	glmlv::GLProgram programEmissivePass;

	//---- Matrix
	GLint uMVPMatrixEmissivePass;
	
	//-- Post Process Variables
	//---- Blur Process Varibales
	//------ Blur Pass Program
	glmlv::GLProgram programBlurPass;

	//------ Textures for blurring
	int nbBlurPass = 1;
	GLuint bufferBlurredTexPass1 = 0;
	GLuint bufferBlurred = 0;
	GLuint uInitTex = 0;

	//------ Uniform to adjust blur direction
	GLuint uDirectionBlur;
	
	GLuint uTexSizeBlur;
	
	//-- Gather (Compositing) Pass Variables
	//---- Gather Pass Program
	glmlv::GLProgram programGatherPass;
	
	//---- Screen Rendering support
	GLuint screenVaoGather = 0;
	GLuint screenVboGather = 0;

	//---- Textures to gather
	GLuint* compositingTextures[4];
	GLuint uCompositingTextures[4];


	//-- INIT OPENGL PROPERTIES ------------
	/*
		Initialise some opengl properties.
	*/
	void initOpenGLProperties();


	//-- INIT EMISSIVE PASS ----------------
	virtual void initEmissivePass();
	
	//-- INIT BLUR PASS --------------------
	void initBlurPass();

	//-- INIT GATHER PASS ------------------
	void initGatherPass(int nbTexPass);

	//-- RENDERING STEP FUNCTIONS ----------
	virtual void prePassRendering(const Scene& scene, const Camera& camera) = 0;
	virtual void renderScene(const Scene& scene, const Camera& camera) = 0;
	virtual void postProcessPass(const Scene& scene, const Camera& camera) = 0;

	//-- RENDER MESH -----------------------
	/*
		Render a mesh, bind vao, ibo and materials.
	*/
	virtual void renderMesh(const Mesh& mesh, const Camera& camera, GLint& uMVPMatrix, GLint& uMVMatrix, GLint& uNormalMatrix);
	
	//-- BIND MESH MATERIAL ----------------
	virtual void bindMeshMaterial(const SharedMaterial& material);

	//-- RENDER EMISSIVE PASS --------------
	/*
		Render emissive mesh it's a simplified renderMesh.
	*/
	virtual void renderParticules(const Scene& scene, const Camera& camera);

	//-- RENDER EMISSIVE MESH ---------------
	virtual void renderParticule(const Mesh& mesh, const Camera& camera, SharedMaterial& currentMaterial);
	
	//-- BIND EMISSIVE MATERIAL ------------
	virtual void bindEmissiveMaterial(const SharedMaterial& material);
	
	//-- POST PROCESS BLUR PASS ------------
	void postProcessBlurPass(GLuint tex);
	
	//-- POST PROCESS DIRECTIONAL BLUR PAS -
	/*
		Blur the picture in a direction:
			0 : horizontal
			1 : vertical
	*/
	void postProcessDirectionalBlurPass(int direction);

	//-- RENDER GATHER PASS ---------------
	/*
		Do the compositing of the different setted layer.
	*/
	virtual void renderGatherPass();


	//-- BIND UBOS-----------------
	/*
		Bind an ubo
	*/
	template<typename T>
	static void bindUbos(const std::vector<T>& data, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ubo, GLenum usage)
	{
		glUniformBlockBinding(program.glId(), uniform, bindingIndex);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo.getPointer());
		if (usage != GL_STREAM_READ) glBufferData(GL_UNIFORM_BUFFER, data.size() * sizeof(T), data.data(), usage);
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, ubo.getPointer(), 0, sizeof(T) * data.size());
	}
	
	//-- BIND SSBOS -----------------
	/*
		Bind a ssbo
	*/
	template<typename T>
	static void bindSsbos(const std::vector<T>& data, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ssbo, GLenum usage)
	{
		glShaderStorageBlockBinding(program.glId(), uniform, bindingIndex);
				
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.getPointer());
		if(usage != GL_STREAM_READ) glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(), usage);
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo.getPointer(), 0, sizeof(T) * data.size());
	}

	template<typename T>
	static void bindSsbosToRead(size_t dataSize, GLuint bindingIndex, GLint uniform, glmlv::GLProgram& program, const BufferObject<T>& ssbo)
	{
		glShaderStorageBlockBinding(program.glId(), uniform, bindingIndex);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo.getPointer());
		glBindBufferRange(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo.getPointer(), 0, sizeof(T) * dataSize);
	}

};

}//! namespace graphic

}//! namespace qc