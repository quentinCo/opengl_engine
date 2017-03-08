#include "Application.hpp"

#include <iostream>
#include <math.h> 

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

/*-------------------- APPLICATION  CONSTRUCTOR ----------------------------------*/

Application::Application(int argc, char** argv):
    m_AppPath { glmlv::fs::path{ argv[0] } },
    m_AppName { m_AppPath.stem().string() },
    m_ImGuiIniFilename { m_AppName + ".imgui.ini" },
    m_ShadersRootPath { m_AppPath.parent_path() / "shaders" },
    m_AssetsRootPath { m_AppPath.parent_path() / "assets" }
{
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str();

	/* Loading obj in main scene */
	scene.addObj(m_AssetsRootPath / m_AppName / "models" / "crytek-sponza" / "sponza.obj");
	scene.addObj(m_AssetsRootPath / m_AppName / "models" / "Maya" / "maya2.obj");

	/* Move Maya mesh */
	qc::graphic::Mesh& mesh = scene.getMeshes().back();
	mesh.setPosition(glm::vec3(500, 100, 0));

	/* Create Lights */
	//scene.addDirectionalLight(qc::graphic::DirectionalLight(90.f, 45.f, glm::vec3(1), 0.25f));
	
	/* Create Pre-def Material*/
	std::vector<std::shared_ptr<qc::graphic::Material>> preDefMaterials;
	for (int i = 0; i < 15; ++i)
	{
		preDefMaterials.push_back(std::make_shared<qc::graphic::Material>());
		auto& material = preDefMaterials.back();

		float r = static_cast<float>(std::rand()) / RAND_MAX;
		float v = static_cast<float>(std::rand()) / RAND_MAX;
		float b = static_cast<float>(std::rand()) / RAND_MAX;

		material->setColor(qc::graphic::Material::EMMISIVE_COLOR, glm::vec3(r, v, b));
	}

	/* Create Point Lights for Particules */
	std::srand(static_cast<unsigned int>(std::time(0))); //use current time as seed for random generator
	const glm::vec3& bboxMin = scene.getBboxMin();
	const glm::vec3& bboxMax = scene.getBboxMax();
	glm::vec3& dimScene = glm::abs(bboxMax - bboxMin);
	/*for (size_t i = 0; i < 1500; ++i) // 3500
	{
		float x = static_cast<float>(std::rand()) / RAND_MAX * dimScene.x - dimScene.x / 2.f;
		float y = static_cast<float>(std::rand()) / RAND_MAX * dimScene.y + 10;;
		float z = static_cast<float>(std::rand()) / RAND_MAX * dimScene.z - dimScene.z / 2.f;

		//float radius = static_cast<float>(std::rand()) / RAND_MAX * 500 + 50;
		float radius = static_cast<float>(std::rand()) / RAND_MAX * 200 + 50;
		float intensity = static_cast<float>(std::rand()) / RAND_MAX * 500 + 200;

		scene.addPointLight(qc::graphic::PointLight(radius, glm::vec3(x, y, z), glm::vec3(1), intensity));
	}*/
	
	scene.addPointLight(qc::graphic::PointLight(20, glm::vec3(200, 100, -260), glm::vec3(1, 0, 0), 300));
	scene.addPointLight(qc::graphic::PointLight(20, glm::vec3(-200, 100, -260), glm::vec3(0, 1, 0), 300));
	scene.addPointLight(qc::graphic::PointLight(20, glm::vec3(200, -100, -260), glm::vec3(0, 0, 1), 300));
	scene.addPointLight(qc::graphic::PointLight(20, glm::vec3(-200, -100, -260), glm::vec3(0, 1, 1), 300));
	scene.addPointLight(qc::graphic::PointLight(500, glm::vec3(-500, 50, 0), glm::vec3(0, 1, 1), 300));
	
	/* Link Particules and Point Lights */
	std::vector<qc::graphic::PointLight>& pointLights = scene.getPointLights();
	for (auto& it : pointLights)
	{
		int indexMat = static_cast<int>(static_cast<float>(std::rand()) / RAND_MAX * (preDefMaterials.size() - 1));
		scene.addParticules(qc::graphic::Particule(preDefMaterials[indexMat], &it));
	}
	scene.sortParticules();

	/* Physic */
	linkPhysicGraphic = std::map<qc::graphic::Particule*, int>();
	physicSystem = qc::physic::PhysicalSystem(qc::physic::PhysicalSystem::GRAVITATIONAL);
	auto& particules = scene.getParticules();
	for (auto& it : particules)
	{
		int temp = physicSystem.addObject(it.getPosition(), 1, 1, 500);
		linkPhysicGraphic.insert(std::make_pair(&it, temp));
	}

	/* Set scene lights ssbo */
	scene.setSsboDirectionalLights();
	scene.setSsboPointLights();

	/* Init camera and renderer */
	camera = qc::graphic::Camera(m_GLFWHandle, glm::vec3(0,0,0), glm::vec3(0,0,-1), 70.f, 0.01f * scene.getSceneSize(), scene.getSceneSize(), scene.getSceneSize() * 0.1f);
	forwardPlus = qc::graphic::ForwardPlusRenderer((m_ShadersRootPath / m_AppName), m_nWindowWidth, m_nWindowHeight);
	renderer = &forwardPlus;

    std::cout << "End INIT" << std::endl;
}


/*-------------------------------  RUN  ------------------------------------------*/

int Application::run()
{
	float clearColor[3] = { 0, 0, 0 };

	for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
	{
		const auto seconds = glfwGetTime();

		/* Render Scene */
		renderer->render(scene, camera);

		/* Update Physic */
		physicSystem.update(1 / discretizationFrequency);
		for (auto& it : linkPhysicGraphic)
		{
			const auto& physicalObject = physicSystem.getPhysicalObject(it.second);
			it.first->setPosition(glm::vec4(physicalObject.getPosition(), 1));
		}

		/* Poll for and process events */
		glfwPollEvents();

		/* Render GUI */
		renderGUI(clearColor);

		/* Swap front and back buffers */
		m_GLFWHandle.swapBuffers();

		/* Update camera */
		auto ellapsedTime = glfwGetTime() - seconds;
		auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		if (!guiHasFocus)
		{
			camera.updateViewController(float(ellapsedTime));
		}

		/* Event "key escape" - Quite */
		if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(m_GLFWHandle.window(), GLFW_TRUE);
	}

	return 0;
}


/*---------------------------  RENDER GUI  ---------------------------------------*/

void Application::renderGUI(float* clearColor)
{
	ImGui_ImplGlfwGL3_NewFrame();
	{
		ImGui::Begin("GUI");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::ColorEditMode(ImGuiColorEditMode_RGB);
		if (ImGui::ColorEdit3("clearColor", clearColor)) {
			glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.f);
		}

		std::string titleButton = "";

		if (ImGui::Button("Render All Post Process Pass"))
		{
			postProcessPass = RenderPostProcessPass::RENDER_ALL;
			renderer->setRenderPostProcess(postProcessPass);
		}

		if ((postProcessPass & RenderPostProcessPass::RENDER_EMISSIVE) == RenderPostProcessPass::RENDER_EMISSIVE)
			titleButton = "Dont Render Emissive";
		else
			titleButton = "Render Emissive";

		if (ImGui::Button(titleButton.c_str()))
		{
			if ((postProcessPass & RenderPostProcessPass::RENDER_EMISSIVE) == RenderPostProcessPass::RENDER_EMISSIVE)
				postProcessPass = static_cast<RenderPostProcessPass>(postProcessPass & ~(RenderPostProcessPass::RENDER_EMISSIVE | RenderPostProcessPass::RENDER_BLUR));
			else
				postProcessPass = static_cast<RenderPostProcessPass>(postProcessPass | RenderPostProcessPass::RENDER_EMISSIVE);

			renderer->setRenderPostProcess(postProcessPass);
		}

		if ((postProcessPass & RenderPostProcessPass::RENDER_BLUR) == RenderPostProcessPass::RENDER_BLUR)
			titleButton = "Dont Render Blur";
		else
			titleButton = "Render Blur";

		if (ImGui::Button(titleButton.c_str()))
		{
			if ((postProcessPass & RenderPostProcessPass::RENDER_EMISSIVE) == RenderPostProcessPass::RENDER_EMISSIVE)
			{
				if ((postProcessPass & RenderPostProcessPass::RENDER_BLUR) == RenderPostProcessPass::RENDER_BLUR)
					postProcessPass = static_cast<RenderPostProcessPass>(postProcessPass & ~RenderPostProcessPass::RENDER_BLUR);
				else
					postProcessPass = static_cast<RenderPostProcessPass>(postProcessPass | RenderPostProcessPass::RENDER_BLUR);
			}

			renderer->setRenderPostProcess(postProcessPass);
		}

		ImGui::SliderFloat("Physical Discretization Frequency", &discretizationFrequency, 1.f, 1000.f);

/*		
		ImGui::RadioButton("GPosition", &attachedToDraw, GL_COLOR_ATTACHMENT0); ImGui::SameLine();
		ImGui::RadioButton("GNormal", &attachedToDraw, GL_COLOR_ATTACHMENT1);
		ImGui::RadioButton("GAmbient", &attachedToDraw, GL_COLOR_ATTACHMENT2); ImGui::SameLine();
		ImGui::RadioButton("GDiffuse", &attachedToDraw, GL_COLOR_ATTACHMENT3);
		ImGui::RadioButton("GGlossyShininess", &attachedToDraw, GL_COLOR_ATTACHMENT4);
*/
		if (ImGui::CollapsingHeader("Directional Light"))
		{
			auto& directionalLights = scene.getDirectionalLights();
			for (size_t i = 0; i < directionalLights.size(); ++i)
			{
				size_t j = i + 1;
				std::string name = "Directional Light " + std::to_string(j);
				if (ImGui::CollapsingHeader(name.c_str()))
				{
					auto& directionalLight = directionalLights[i];
					name = "DirLightDirection " + std::to_string(j);
					ImGui::ColorEdit3(name.c_str(), glm::value_ptr(directionalLight.getPosition()));
					name = "DirLightColor " + std::to_string(j);
					ImGui::ColorEdit3(name.c_str(), glm::value_ptr(directionalLight.getColor()));
					name = "DirLightIntensity " + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &directionalLight.getIntensity(), 0.1f, 0.f, 100.f);
					name = "Phi Angle " + std::to_string(j);
					std::string name2 = "Theta Angle " + std::to_string(j);
					if (ImGui::DragFloat(name.c_str(), &directionalLight.getPhiAngle(), 1.0f, 0.0f, 360.f) ||
						ImGui::DragFloat(name2.c_str(), &directionalLight.getThetaAngle(), 1.0f, 0.0f, 180.f)) {
						directionalLight.setDirection(directionalLight.getPhiAngle(), directionalLight.getThetaAngle());
					}
				}
			}			
		}

		if (ImGui::CollapsingHeader("Point Light"))
		{
			auto& pointLights = scene.getPointLights();
			for (size_t i = 0; i < pointLights.size(); ++i)
			{
				size_t j = i + 1;
				std::string name = "Point Light" + std::to_string(j);
				if (ImGui::CollapsingHeader(name.c_str()))
				{
					auto& pointLight = pointLights[i];
					name = "PointLightColor" + std::to_string(j);
					ImGui::ColorEdit3(name.c_str(), glm::value_ptr(pointLight.getColor()));
					name = "PointLightIntensity" + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &pointLight.getIntensity(), 0.1f, 0.f, 16000.f);
					name = "PointAttenuationRadius" + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &pointLight.getRadiusAttenuation(), 0.1f, 20.f, 1000.f);
					name = "ConstantAttenuation" + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &pointLight.getConstantAttenuation(), 0.1f, 1.f, 100.f);
					name = "LinearAttenuation" + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &pointLight.getLinearAttenuation(), 0.1f, 1.f, 100.f);
					name = "QuadraticAttenuation" + std::to_string(j);
					ImGui::DragFloat(name.c_str(), &pointLight.getQuadraticAttenuation(), 0.1f, 1.f, 100.f);
					name = "Position" + std::to_string(j);
					ImGui::InputFloat3(name.c_str(), glm::value_ptr(pointLight.getPosition()));
				}
			}
		}

		ImGui::End();
	}


	ImGui::Render();
}