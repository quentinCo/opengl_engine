#include <glm/gtc/type_ptr.hpp>

#include "ForwardPlusRenderer.hpp"

using namespace qc;

ForwardPlusRenderer::ForwardPlusRenderer(const glmlv::fs::path& shaderDirectory, size_t windowWidth, size_t windowHeight)
	: Renderer(shaderDirectory, windowWidth, windowHeight)
{
	initDepthPass();
	//initDepthDebug();
	initLightCullingPass();
	initShadingPass();
}

ForwardPlusRenderer::~ForwardPlusRenderer()
{
	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	if (depthMap) glDeleteSamplers(1, &depthMap);
	if (textureSampler) glDeleteSamplers(1, &textureSampler);
}

ForwardPlusRenderer::ForwardPlusRenderer(ForwardPlusRenderer&& o)
	:Renderer(o), programDepthPass(std::move(o.programDepthPass)), fboDepth(o.fboDepth), depthMap(o.depthMap), uDepthModelViewProjMatrix(o.uDepthModelViewProjMatrix),
	programLightCullingPass(std::move(o.programLightCullingPass)), nbComputeBlock(o.nbComputeBlock), pointLightsIndex(pointLightsIndex), uPoinLightIndexForShading(o.uPoinLightIndexForShading),
	uWindowDimForShading(o.uWindowDimForShading),
	ssboPointLightsIndex(std::move(o.ssboPointLightsIndex)),/* ssboDebug(std::move(o.ssboDebug)), debugLight(o.debugLight), uDebugOutput(o.uDebugOutput), */uPointLightsForCulling(o.uPointLightsForCulling), uPointLightsNumberForCulling(o.uPointLightsNumberForCulling),
	uPointLightsIndexForCulling(o.uPointLightsIndexForCulling), uInverseProjMatrix(o.uInverseProjMatrix), uViewMatrixForCulling(o.uViewMatrixForCulling),
	uViewProjMatrixForCulling(o.uViewProjMatrixForCulling), uProjMatrixForCulling(o.uProjMatrixForCulling), uWindowDim(o.uWindowDim),
	uDepthMapForCulling(o.uDepthMapForCulling), programShadingPass(std::move(o.programShadingPass)), uModelViewProjMatrixForShading(o.uModelViewProjMatrixForShading),
	uModelViewMatrixForShading(o.uModelViewMatrixForShading), uNormalMatrixForShading(o.uNormalMatrixForShading), uViewMatrixForShading(o.uViewMatrixForShading),
	uDirectionalLights(o.uDirectionalLights), uDirectionalLightsNumber(o.uDirectionalLightsNumber),
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
	//TODO :delete
	programDebugDepth = std::move(o.programDebugDepth);
	o.programDebugDepth = glmlv::GLProgram();

	screenVao = o.screenVao;
	o.screenVao = 0;
	screenVbo = o.screenVbo;
	o.screenVbo = 0;
	uDepthMap = o.uDepthMap;
	o.uDepthMap = 0;
	//--------------
	o.programLightCullingPass = glmlv::GLProgram();

	o.programShadingPass = glmlv::GLProgram();

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
	o.programDepthPass = glmlv::GLProgram();

	if (fboDepth) glDeleteFramebuffers(1, &fboDepth);
	fboDepth = o.fboDepth;
	o.fboDepth = 0;

	if (depthMap) glDeleteSamplers(1, &depthMap);
	depthMap = o.depthMap;
	o.depthMap = 0;

	uDepthModelViewProjMatrix = o.uDepthModelViewProjMatrix;

	// TODO : delete
	programDebugDepth = std::move(o.programDebugDepth);
	o.programDebugDepth = glmlv::GLProgram();

	screenVao = o.screenVao;
	o.screenVao = 0;
	screenVbo = o.screenVbo;
	o.screenVbo = 0;
	uDepthMap = o.uDepthMap;
	o.uDepthMap = 0;
	// --------------

	programLightCullingPass = std::move(o.programLightCullingPass);
	o.programLightCullingPass = glmlv::GLProgram();
	nbComputeBlock = o.nbComputeBlock;
	pointLightsIndex = o.pointLightsIndex;
	uPoinLightIndexForShading = o.uPoinLightIndexForShading;
	uWindowDimForShading = o.uWindowDimForShading;
	ssboPointLightsIndex = std::move(o.ssboPointLightsIndex);
	/*ssboDebug = std::move(o.ssboDebug);
	debugLight = o.debugLight;
	uDebugOutput = o.uDebugOutput;*/
	uPointLightsForCulling = o.uPointLightsForCulling;
	uPointLightsNumberForCulling = o.uPointLightsNumberForCulling;
	uPointLightsIndexForCulling = o.uPointLightsIndexForCulling;
	uInverseProjMatrix = o.uInverseProjMatrix;
	uViewMatrixForCulling = o.uViewMatrixForCulling;
	uViewProjMatrixForCulling = o.uViewProjMatrixForCulling;
	uProjMatrixForCulling = o.uProjMatrixForCulling,
	uWindowDim = o.uWindowDim;
	uDepthMapForCulling = o.uDepthMapForCulling;

	programShadingPass = std::move(o.programShadingPass);
	o.programShadingPass = glmlv::GLProgram();

	if (textureSampler) glDeleteSamplers(1, &textureSampler);
	textureSampler = o.textureSampler;
	o.textureSampler = 0;

	uModelViewProjMatrixForShading = o.uModelViewProjMatrixForShading;
	uModelViewMatrixForShading = o.uModelViewMatrixForShading;
	uNormalMatrixForShading = o.uNormalMatrixForShading;

	uViewMatrixForShading = o.uViewMatrixForShading;

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
	renderLightCullingPass(scene, camera);
	//renderDepthDebug();
	renderShadingPass(scene, camera);
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

void ForwardPlusRenderer::initLightCullingPass()
{
	programLightCullingPass = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusPointLightCulling.cs.glsl"});

	nbComputeBlock = glm::vec3((GLuint)ceil(windowWidth / 32.f), (GLuint)ceil(windowHeight / 32.f), 1);

	pointLightsIndex = std::vector<int>(static_cast<int>(nbComputeBlock.x * nbComputeBlock.y * 200)); // TODO : find a better solution.
	ssboPointLightsIndex = BufferObject<int>(pointLightsIndex, GL_SHADER_STORAGE_BUFFER);
	/*
	uDebugOutput = glGetProgramResourceIndex(programLightCullingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uDebugOutput");
	debugLight = std::vector<float>(static_cast<int>(nbComputeBlock.x * nbComputeBlock.y * 200));
	ssboDebug = BufferObject<float>(debugLight, GL_SHADER_STORAGE_BUFFER);
	*/
	uPointLightsForCulling = glGetProgramResourceIndex(programLightCullingPass.glId(), GL_SHADER_STORAGE_BLOCK,"uPointLights");
	uPointLightsNumberForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uPointLightsNumber");
	uPointLightsIndexForCulling = glGetProgramResourceIndex(programLightCullingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLightsIndex");

	uInverseProjMatrix = glGetUniformLocation(programLightCullingPass.glId(), "uInverseProjMatrix");
	uViewMatrixForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uViewMatrix");
	uViewProjMatrixForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uViewProjMatrix");
	uProjMatrixForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uProjMatrix");
	uWindowDim = glGetUniformLocation(programLightCullingPass.glId(), "uWindowDim");

	uDepthMapForCulling = glGetUniformLocation(programLightCullingPass.glId(), "uDepthMap");

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		std::cerr << "Error : " << error << " in initDepthPass" << std::endl;
}


void ForwardPlusRenderer::initShadingPass()
{
	programShadingPass = glmlv::compileProgram({ shaderDirectory / "forwardPlus" / "forwardPlusShadingPass.vs.glsl" , shaderDirectory / "forwardPlus" / "forwardPlusShadingPass.fs.glsl" });

	glGenSamplers(1, &textureSampler);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uModelViewProjMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uModelViewProjMatrix");
	uModelViewMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uModelViewMatrix");
	uNormalMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uNormalMatrix");

	uViewMatrixForShading = glGetUniformLocation(programShadingPass.glId(), "uViewMatrix");

	uDirectionalLights = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uDirectionalLights");
	uDirectionalLightsNumber = glGetUniformLocation(programShadingPass.glId(), "uDirectionalLightsNumber");

	uPointLights = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLights");
	uPointLightsNumber = glGetUniformLocation(programShadingPass.glId(), "uPointLightsNumber");
	
	uPoinLightIndexForShading = glGetProgramResourceIndex(programShadingPass.glId(), GL_SHADER_STORAGE_BLOCK, "uPointLightsIndex");

	uWindowDimForShading = glGetUniformLocation(programShadingPass.glId(), "uWindowDim");

	uKa = glGetUniformLocation(programShadingPass.glId(), "uKa");
	uKd = glGetUniformLocation(programShadingPass.glId(), "uKd");
	uKs = glGetUniformLocation(programShadingPass.glId(), "uKs");
	uShininess = glGetUniformLocation(programShadingPass.glId(), "uShininess");

	uKaSampler = glGetUniformLocation(programShadingPass.glId(), "uKaSampler");
	uKdSampler = glGetUniformLocation(programShadingPass.glId(), "uKdSampler");
	uKsSampler = glGetUniformLocation(programShadingPass.glId(), "uKsSampler");
	uShininessSampler = glGetUniformLocation(programShadingPass.glId(), "uShininessSampler");

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

void ForwardPlusRenderer::renderLightCullingPass(const Scene& scene, const Camera& camera)
{
	const std::vector<PointLight>& pointLights = scene.getPointLights();
	if (pointLights.size() <= 0)
		return;

	programLightCullingPass.use();

	glm::mat4 invProjMatrix = glm::inverse(camera.getProjMatrix());
	glm::mat4 viewProj = camera.getProjMatrix() * camera.getViewMatrix();
	glUniformMatrix4fv(uInverseProjMatrix, 1, FALSE, glm::value_ptr(invProjMatrix));
	glUniformMatrix4fv(uViewMatrixForCulling, 1, FALSE, glm::value_ptr(camera.getViewMatrix()));
	glUniformMatrix4fv(uViewProjMatrixForCulling, 1, FALSE, glm::value_ptr(viewProj));
	glUniformMatrix4fv(uProjMatrixForCulling, 1, FALSE, glm::value_ptr(camera.getProjMatrix()));
	glUniform2fv(uWindowDim, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	Renderer::bindSsbos(pointLights, 0, uPointLightsForCulling, programLightCullingPass, scene.getSsboPointLights(), GL_STREAM_DRAW);
	glUniform1i(uPointLightsNumberForCulling, static_cast<GLint>(pointLights.size()));

	Renderer::bindSsbos(pointLightsIndex, 1, uPointLightsIndexForCulling, programLightCullingPass, ssboPointLightsIndex, GL_STREAM_READ);

	//Renderer::bindSsbos(debugLight, 2, uDebugOutput, programLightCullingPass, ssboDebug, GL_STREAM_READ);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(uDepthMapForCulling, 0);

	glDispatchCompute((GLuint)nbComputeBlock.x, (GLuint)nbComputeBlock.y, (GLuint)nbComputeBlock.z);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		int xGroup = static_cast<int>(ceil(windowWidth / 16.f) * 16);
		int yGroup = static_cast<int>(ceil(windowHeight / 16.f) * 16);
		std::cerr << windowWidth << " -- " << windowHeight << std::endl;
		std::cerr << xGroup << " -- " << yGroup << " -- " << yGroup * xGroup << std::endl;
		std::cerr << "glGetError() : " << err << std::endl;
		exit(1);
	}

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	/*
	for (int i = 0; i < 4; i++)
	{
		std::cout << "[" << viewProj[i][0] << " ; " << viewProj[i][1] << " ; " << viewProj[i][2] << " ; " << viewProj[i][3] << "]" << std::endl;
	}
	std::cout << "\n" << std::endl;
	
	float* debug = (float*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, debugLight.size() * sizeof(float), GL_MAP_READ_BIT);
	std::cout.precision(4);
	bool ignor = false;
	int count = 0;
	for (int y = 0; y < ceil(windowHeight / 32.f); y++)
	{
		for (int x = 0; x < ceil(windowWidth / 32.f); x++)
		{
			for (int j = 0; j < 200; j++)
			{
				int i = (x + y * (ceil(windowWidth / 32.f))) * 200 + j;
				if (debug[i] < 99999)
				{
					if (i % 200 == 0)
					{
						ignor = false;
						std::cout << "\n[" << x * 32 << "; " << y * 32 << "] : " << debug[i];
						count++;
					}
					else if (debug[i] == 0 && ignor)
					{
					}
					else
					{
						std::cout << " -- " << debug[i];
						count++;
					}
				}
				else
				{
					ignor = true;
				}
			}
		}
	}
	std::cout << "\ncout = " << count << std::endl;
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ForwardPlusRenderer::renderShadingPass(const Scene& scene, const Camera& camera)
{
	programShadingPass.use();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& directionalLights = scene.getDirectionalLights();
	std::vector<Light> directionalPointLights; //TODO: revoir
	for (const auto& it : directionalLights)
		directionalPointLights.push_back(it);

	const auto& pointLights = scene.getPointLights();
	const auto& meshes = scene.getMeshes();

	glUniformMatrix4fv(uViewMatrixForShading, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
	glUniform2fv(uWindowDimForShading, 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));

	if(directionalLights.size() > 0)
		Renderer::bindSsbos(directionalPointLights, 1, uDirectionalLights, programShadingPass, scene.getSsboDirectionalLights(), GL_STREAM_DRAW);
	
	glUniform1i(uDirectionalLightsNumber, static_cast<GLint>(directionalLights.size()));

	if (pointLights.size() > 0)
	{
		Renderer::bindSsbos(pointLights, 2, uPointLights, programShadingPass, scene.getSsboPointLights(), GL_STREAM_DRAW);
		Renderer::bindSsbos(pointLightsIndex, 3, uPoinLightIndexForShading, programShadingPass, ssboPointLightsIndex, GL_STREAM_READ);
	}
	
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

	glUniformMatrix4fv(uModelViewProjMatrixForShading, 1, FALSE, glm::value_ptr(mvpMatrix));
	glUniformMatrix4fv(uModelViewMatrixForShading, 1, FALSE, glm::value_ptr(mvMatrix));
	glUniformMatrix4fv(uNormalMatrixForShading, 1, FALSE, glm::value_ptr(normalMatrix));

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