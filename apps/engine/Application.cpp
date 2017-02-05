#include "Application.hpp"

#include <iostream>
#include <math.h>  
#include <unordered_set>

#include <imgui.h>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/load_obj.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

int Application::run()
{
    float clearColor[3] = { 0, 0, 0 };
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();
		drawGeoPass();

	// Compute Shader
		drawComputePass();
		
    // Uniforme Shading
		drawShadingPass();

        // GUI code:
		drawGUI(clearColor);

        /* Poll for and process events */
        glfwPollEvents();

        /* Swap front and back buffers*/
        m_GLFWHandle.swapBuffers();

        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus)
		{
			camera.updateViewController(float(ellapsedTime));
        }
    }

    return 0;
}

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }
{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file

	scene.addObj(m_AssetsRootPath / m_AppName / "models" / "crytek-sponza" / "sponza.obj");
	scene.addDirectionalLight(qc::DirectionalLight(90.f, 45.f, glm::vec3(1), 1.f));
	scene.addPointLight(qc::Light(glm::vec3(0, 1, 0), glm::vec3(1), 5.f));
		//loadScene();
	
    // Note: no need to bind a sampler for modifying it: the sampler API is already direct_state_access
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CW);

//	camera = qc::Camera(m_GLFWHandle, 70.f, 0.01f * m_SceneSize, m_SceneSize, m_SceneSize * 0.1f);
	camera = qc::Camera(m_GLFWHandle, 70.f, 0.01f * scene.getSceneSize(), scene.getSceneSize(), scene.getSceneSize() * 0.1f);

	// Geo program
	initForGeo();

	// Shading program
	initForShading();

    // GBufferTexture
    glGenTextures(GBufferTextureType::GBufferTextureCount, m_GBufferTextures);
    for(int i = 0; i < GBufferTextureType::GBufferTextureCount; i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, m_GBufferTextureFormat[i], m_nWindowWidth, m_nWindowHeight);
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // FBO init
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
        // Create buffer
    for(int i = 0; i < GBufferTextureType::GDepth; ++i)
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_GBufferTextures[i], 0);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_GBufferTextures[GBufferTextureType::GDepth], 0);

    GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4
    };
        // link out buffer and out
    glDrawBuffers(5, drawBuffers);
    
        // Check create
        // glCheckFramebufferStatus(GLenum target) -> target = target de glFramebufferTexture2D()
    GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if(res != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Error check frame buffer : " << res  << std::endl;

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    attachedToDraw = GL_COLOR_ATTACHMENT0;

	initScreenBuffers();

	initForCompute();
    std::cout << "End INIT" << std::endl;
}

void Application::drawGeoPass()
{
	m_programGeo.use();

	// Bind FBO Draw   
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);

	// Put here rendering code
	//  Give to glViewPort the GBuffer dimension (m_nWindowWidth -> GBuffer.width -- normalement)
	glViewport(0, 0, m_nWindowWidth, m_nWindowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& meshes = scene.getMeshes();
	for (const auto& mesh : meshes)
	{
		glm::mat4 mvMatrix, mvpMatrix, normalMatrix;
		camera.computeModelsMatrix(mesh.getModelMatrix(), mvMatrix, mvpMatrix, normalMatrix);

		glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
		glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
		glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		// Same sampler for all texture units
		glBindSampler(0, m_textureSampler);
		glBindSampler(1, m_textureSampler);
		glBindSampler(2, m_textureSampler);
		glBindSampler(3, m_textureSampler);

		// Set texture unit of each sampler
		glUniform1i(m_uKaSamplerLocation, 0);
		glUniform1i(m_uKdSamplerLocation, 1);
		glUniform1i(m_uKsSamplerLocation, 2);
		glUniform1i(m_uShininessSamplerLocation, 3);


		const auto bindMaterial = [&](const qc::Material& material)
		{
			glUniform3fv(m_uKaLocation, 1, glm::value_ptr(material.getColor(qc::Material::AMBIENT_COLOR)));
			glUniform3fv(m_uKdLocation, 1, glm::value_ptr(material.getColor(qc::Material::DIFFUSE_COLOR)));
			glUniform3fv(m_uKsLocation, 1, glm::value_ptr(material.getColor(qc::Material::SPECULAR_COLOR)));
			glUniform1f(m_uShininessLocation, material.getShininess());

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, material.getMap(qc::Material::AMBIENT_TEXTURE));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, material.getMap(qc::Material::DIFFUSE_TEXTURE));
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, material.getMap(qc::Material::SPECULAR_TEXTURE));
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, material.getMap(qc::Material::SPECULAR_HIGHT_LIGHT_TEXTURE));
		};

		glBindVertexArray(mesh.getVao().getPointer());

		const qc::Material* currentMaterial = nullptr;
		const std::vector<qc::Material>& materials = mesh.getMaterials();
		const qc::Material& defaultMaterial = qc::Mesh::defaultMaterial;

		const auto& shapes = mesh.getShapesData();
		// We draw each shape by specifying how much indices it carries, and with an offset in the global index buffer
		for (const auto shape : shapes)
		{
			const auto& material = (shape.materialIndex >= 0) ? materials[shape.materialIndex] : defaultMaterial;
			if (currentMaterial != &material)
			{
				bindMaterial(material);
				currentMaterial = &material;
			}

			glDrawElements(GL_TRIANGLES, shape.shapeSize, GL_UNSIGNED_INT, (const GLvoid*)(shape.shapeIndex * sizeof(GLuint)));
		}

		for (GLuint i : {0, 1, 2, 3})
			glBindSampler(0, m_textureSampler);

		// Unbind FBO Draw
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		/*
		// Read FBO
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
		glReadBuffer(attachedToDraw);
		glBlitFramebuffer(0,0, m_nWindowWidth, m_nWindowHeight, 0, 0, m_nWindowWidth, m_nWindowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		*/
	}
}

void Application::drawComputePass()
{
	// launch compute shaders
	m_programCompute.use();

	const auto& viewMatrix = camera.getViewMatrix();

	const auto& directionalLights = scene.getDirectionalLights();
	const auto& pointLights = scene.getPointLights();

	const auto& directionalLight = directionalLights[0];
	const auto& pointLight = pointLights[0];

	glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(glm::normalize(directionalLight.getDirection()), 0))));
	glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(directionalLight.getColor() * directionalLight.getIntensity()));

	glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(viewMatrix * glm::vec4(pointLight.getPosition(), 1))));
	glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(pointLight.getColor() * pointLight.getIntensity()));

	glUniform2fv(m_uWindowsDim, 1, glm::value_ptr(glm::vec2(m_nWindowWidth, m_nWindowHeight)));

	for (int i = 0; i < GBufferTextureType::GDepth; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_GBufferTextures[i]);
		glUniform1i(m_uGTextures[i], i);
	}

	glDispatchCompute((GLuint)ceil(m_nWindowWidth / 32.f), (GLuint)ceil(m_nWindowHeight / 32.f), 1);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		int xGroup = ceil(m_nWindowWidth / 32.f) * 32;
		int yGroup = ceil(m_nWindowHeight / 32.f) * 32;
		std::cout << m_nWindowWidth << " -- " << m_nWindowHeight << std::endl;
		std::cout << xGroup << " -- " << yGroup << " -- " << yGroup * xGroup << std::endl;
		std::cout << m_workGroupInvocation << std::endl;
		std::cerr << "glGetError() : " << err << std::endl;
		exit(1);
	}
	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}

void Application::drawShadingPass()
{
	m_programShading.use();
	/*--------------------------------------------------*/
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ! \ A NE SURTOUT PAS OUBLIER
	/*------------------------------------------------*/

	glBindVertexArray(m_ScreenVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screenTexture);
	glUniform1i(m_uScreenTexture, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Application::drawGUI(float* clearColor)
{
	ImGui_ImplGlfwGL3_NewFrame();
	{
		ImGui::Begin("GUI");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
		if (ImGui::ColorEdit3("clearColor", clearColor)) {
			glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
		}

		ImGui::RadioButton("GPosition", &attachedToDraw, GL_COLOR_ATTACHMENT0); ImGui::SameLine();
		ImGui::RadioButton("GNormal", &attachedToDraw, GL_COLOR_ATTACHMENT1);
		ImGui::RadioButton("GAmbient", &attachedToDraw, GL_COLOR_ATTACHMENT2); ImGui::SameLine();
		ImGui::RadioButton("GDiffuse", &attachedToDraw, GL_COLOR_ATTACHMENT3);
		ImGui::RadioButton("GGlossyShininess", &attachedToDraw, GL_COLOR_ATTACHMENT4);

		if (ImGui::CollapsingHeader("Directional Light"))
		{
			auto& directionalLights = scene.getDirectionalLights();
			auto& directionalLight = directionalLights[0];
			ImGui::ColorEdit3("DirLightColor", glm::value_ptr(directionalLight.getColor()));
			ImGui::DragFloat("DirLightIntensity", &directionalLight.getIntensity(), 0.1f, 0.f, 100.f);
			if (ImGui::DragFloat("Phi Angle", &directionalLight.getPhiAngle(), 1.0f, 0.0f, 360.f) ||
				ImGui::DragFloat("Theta Angle", &directionalLight.getThetaAngle(), 1.0f, 0.0f, 180.f)) {
				directionalLight.setDirection(directionalLight.getPhiAngle(), directionalLight.getThetaAngle());
			}
		}

		if (ImGui::CollapsingHeader("Point Light"))
		{
			auto& pointLights = scene.getPointLights();
			auto& pointLight = pointLights[0];
			ImGui::ColorEdit3("PointLightColor", glm::value_ptr(pointLight.getColor()));
			ImGui::DragFloat("PointLightIntensity", &pointLight.getIntensity(), 0.1f, 0.f, 16000.f);
			ImGui::InputFloat3("Position", glm::value_ptr(pointLight.getPosition()));
		}

		ImGui::End();
	}


	ImGui::Render();
}
/*
void Application::loadScene()
{
	const auto objPath = m_AssetsRootPath / m_AppName / "models" / "crytek-sponza" / "sponza.obj";

	glmlv::ObjData data;
	try
	{
		loadObj(objPath, data);
	}
	catch (std::runtime_error e)
	{
		std::cerr << "Error during the loading." << std::endl;
		exit(1);
	}
	m_SceneSize = glm::length(data.bboxMax - data.bboxMin);

	std::cout << "# of shapes    : " << data.shapeCount << std::endl;
	std::cout << "# of materials : " << data.materialCount << std::endl;
	std::cout << "# of vertex    : " << data.vertexBuffer.size() << std::endl;
	std::cout << "# of triangles    : " << data.indexBuffer.size() / 3 << std::endl;

	// Init shape infos
	uint32_t indexOffset = 0;
	std::vector<qc::ShapeData> shapes;
	for (auto shapeID = 0; shapeID < data.indexCountPerShape.size(); ++shapeID)
	{
		shapes.emplace_back(data.indexCountPerShape[shapeID], indexOffset, data.materialIDPerShape[shapeID]);
		indexOffset += data.indexCountPerShape[shapeID];
	}

	mesh = qc::Mesh(data.vertexBuffer, data.indexBuffer, shapes);

	std::vector<qc::Material> materials;
	for (auto it : data.materials)
	{
		qc::Material material = qc::Material();

		material.setColor(qc::Material::AMBIENT_COLOR, it.Ka);
		glm::vec3 diffuseColor = (it.Kd == glm::vec3(0)) ? glm::vec3(1) : it.Kd;
		material.setColor(qc::Material::DIFFUSE_COLOR, it.Kd);
		material.setColor(qc::Material::SPECULAR_COLOR, it.Ks);

		if (it.KaTextureId >= 0) material.setMap(qc::Material::AMBIENT_TEXTURE, data.textures[it.KaTextureId]);
		if (it.KdTextureId >= 0) material.setMap(qc::Material::DIFFUSE_TEXTURE, data.textures[it.KdTextureId]);
		if (it.KsTextureId >= 0) material.setMap(qc::Material::SPECULAR_TEXTURE, data.textures[it.KsTextureId]);
		if (it.shininessTextureId >= 0) material.setMap(qc::Material::SPECULAR_HIGHT_LIGHT_TEXTURE, data.textures[it.shininessTextureId]);

		materials.emplace_back(std::move(material));
	}

	mesh.setMaterials(materials);
}
*/
void Application::initForGeo()
{
	m_programGeo = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "geometryPass.vs.glsl", m_ShadersRootPath / m_AppName / "geometryPass.fs.glsl" });
	m_programGeo.use();

	m_uModelViewProjMatrixLocation = glGetUniformLocation(m_programGeo.glId(), "uModelViewProjMatrix");
	m_uModelViewMatrixLocation = glGetUniformLocation(m_programGeo.glId(), "uModelViewMatrix");
	m_uNormalMatrixLocation = glGetUniformLocation(m_programGeo.glId(), "uNormalMatrix");

	m_uKaLocation = glGetUniformLocation(m_programGeo.glId(), "uKa");
	m_uKdLocation = glGetUniformLocation(m_programGeo.glId(), "uKd");
	m_uKsLocation = glGetUniformLocation(m_programGeo.glId(), "uKs");
	m_uShininessLocation = glGetUniformLocation(m_programGeo.glId(), "uShininess");
	m_uKaSamplerLocation = glGetUniformLocation(m_programGeo.glId(), "uKaSampler");
	m_uKdSamplerLocation = glGetUniformLocation(m_programGeo.glId(), "uKdSampler");
	m_uKsSamplerLocation = glGetUniformLocation(m_programGeo.glId(), "uKsSampler");
	m_uShininessSamplerLocation = glGetUniformLocation(m_programGeo.glId(), "uShininessSampler");
    
    std::cout << "End initForGeo() " << std::endl;
}

void Application::initForShading()
{
	m_programShading = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "shadingPass.vs.glsl", m_ShadersRootPath / m_AppName / "shadingPass.fs.glsl" });
	m_programShading.use();

	m_uScreenTexture = glGetUniformLocation(m_programShading.glId(), "uScreenTexture");
    
    std::cout << "End initForShading() " << std::endl;
}

void Application::initScreenBuffers()
{
	glm::vec2 triangle[3];
	triangle[0] = glm::vec2(-1);
	triangle[1] = glm::vec2(3, -1);
	triangle[2] = glm::vec2(-1, 3);

	glGenBuffers(1, &m_ScreenVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO);
	glBufferStorage(GL_ARRAY_BUFFER, 3 * sizeof(glm::vec2), triangle, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	const GLint position = glGetAttribLocation(m_programShading.glId(), "aPosition");
	glGenVertexArrays(1, &m_ScreenVAO);
	glBindVertexArray(m_ScreenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_ScreenVBO);
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
    
    std::cout << "End initScreenBuffers() " << std::endl;
}

void Application::initForCompute()
{
	m_programCompute = glmlv::compileProgram({ m_ShadersRootPath / m_AppName / "computePass.cs.glsl"});
	m_programCompute.use();

	// Gen texture
	glGenTextures(1, &m_screenTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_screenTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_nWindowWidth, m_nWindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, m_screenTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Retrieve work group count max
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &m_workGroupCount[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &m_workGroupCount[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &m_workGroupCount[2]);

    std::cout << "m_workGroupCount" << std::endl;
    for(int i = 0; i < 3; ++i)
        std::cout << m_workGroupCount[i] << " - ";
    std::cout << std::endl;

	// Retrieve work group size max
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &m_workGroupSize[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &m_workGroupSize[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &m_workGroupSize[2]);

    std::cout << "m_workGroupSize" << std::endl;
    for(int i = 0; i < 3; ++i)
        std::cout << m_workGroupSize[i] << " - ";
    std::cout << std::endl;

	// Retrieve work group invocation
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_workGroupInvocation);
    std::cout << "m_workGroupInvocation : " << m_workGroupInvocation << std::endl;


	m_uDirectionalLightDirLocation = glGetUniformLocation(m_programCompute.glId(), "uDirectionalLightDir");
	m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_programCompute.glId(), "uDirectionalLightIntensity");

	m_uPointLightPositionLocation = glGetUniformLocation(m_programCompute.glId(), "uPointLightPosition");
	m_uPointLightIntensityLocation = glGetUniformLocation(m_programCompute.glId(), "uPointLightIntensity");

	m_uGTextures[0] = glGetUniformLocation(m_programCompute.glId(), "uGPosition");
	m_uGTextures[1] = glGetUniformLocation(m_programCompute.glId(), "uGNormal");
	m_uGTextures[2] = glGetUniformLocation(m_programCompute.glId(), "uGAmbient");
	m_uGTextures[3] = glGetUniformLocation(m_programCompute.glId(), "uGDiffuse");
	m_uGTextures[4] = glGetUniformLocation(m_programCompute.glId(), "uGlossyShininess");

    m_uWindowsDim = glGetUniformLocation(m_programCompute.glId(), "uWindowsDim");

    std::cout << "End initForCompute() " << std::endl;
}