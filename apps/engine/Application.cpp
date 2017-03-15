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

	/* Create Lights */
	initLights();

	/* Create Particules */
	initParticules();

	/* Physic */
	initPhysic();

	/* Set scene lights ssbo */
//	scene.addDirectionalLight(qc::graphic::DirectionalLight(45, 45, glm::vec3(0.5,0.5,0), 0.25));
//	scene.setSsboDirectionalLights();

	/* Init camera and renderer */
	camera = qc::graphic::Camera(m_GLFWHandle, glm::vec3(0,100,0), glm::vec3(0,0,-1), 70.f, 0.01f * scene.getSceneSize(), scene.getSceneSize(), scene.getSceneSize() * 0.1f);
	forwardPlus = qc::graphic::ForwardPlusRenderer((m_ShadersRootPath), m_nWindowWidth, m_nWindowHeight);
	renderer = &forwardPlus;
}


/*-------------------------------  RUN  ------------------------------------------*/

int Application::run()
{
	float clearColor[3] = { 0, 0, 0 };

	for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
	{
		const auto seconds = glfwGetTime();

		/* Update Physic */
		if(activePhysic)
			physic = std::thread(&Application::updatePhysic, this);

		/* Render Scene */
		renderer->render(scene, camera);

		/* Update Graphic from Physic */
		if (activePhysic)
		{
			physic.join();
			updateGraphicFromPhysic();
		}			

		/* Poll for and process events */
		glfwPollEvents();

		/* Render GUI */
		renderGUI(clearColor);

		/* Swap front and back buffers */
		m_GLFWHandle.swapBuffers();

		/* Update camera */
		auto ellapsedTime = glfwGetTime() - seconds;
		if (!(ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard))
			camera.updateViewController(float(ellapsedTime));

		/* Event "key escape" - Quite */
		if (glfwGetKey(m_GLFWHandle.window(), GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(m_GLFWHandle.window(), GLFW_TRUE);

		if (resetSystem)
			resetPhysicalParticulesSystem();
	}

	return 0;
}


/*---------------------------  INIT LIGHTS  ---------------------------------------*/
void Application::initLights()
{
	//std::srand(static_cast<unsigned int>(std::time(0))); //use current time as seed for random generator
	glm::vec3 bboxMin = scene.getBboxMin() * (2 / 3.f);
	glm::vec3 bboxMax =scene.getBboxMax() * (2 / 3.f);
	glm::vec3& dimScene = glm::abs(bboxMax - bboxMin);
	for (size_t i = 0; i < 250; ++i) // 5000 // limite with physique 250
	{
		float x = static_cast<float>(std::rand()) / RAND_MAX * dimScene.x - dimScene.x / 2.f;
		float y = static_cast<float>(std::rand()) / RAND_MAX * dimScene.y + 10;
		float z = static_cast<float>(std::rand()) / RAND_MAX * dimScene.z - dimScene.z / 2.f;

		float radius; 
		float intensity;		
		if (i % 50 == 0)
		{
			intensity = static_cast<float>(std::rand()) / RAND_MAX * 500 + 1000;
			radius = static_cast<float>(std::rand()) / RAND_MAX * 200 + 200;
		}
		else
		{
			intensity = static_cast<float>(std::rand()) / RAND_MAX * 200 + 200;
			radius = static_cast<float>(std::rand()) / RAND_MAX * 100 + 50;
		}

		scene.addPointLight(qc::graphic::PointLight(radius, glm::vec3(x, y, z), glm::vec3(1), intensity));
	}
	scene.addPointLight(qc::graphic::PointLight(200, glm::vec3(0, 100, 0), glm::vec3(1), 2000));
	scene.addPointLight(qc::graphic::PointLight(100, glm::vec3(0, 110, 0), glm::vec3(1), 100));
	scene.setSsboPointLights();
}


//-- INIT PARTICULES -----------------
void Application::initParticules()
{
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

	/* Link Particules and Point Lights */
	std::vector<qc::graphic::PointLight>& pointLights = scene.getPointLights();
	for (auto& it : pointLights)
	{
		int indexMat = static_cast<int>(static_cast<float>(std::rand()) / RAND_MAX * (preDefMaterials.size() - 1));
		scene.addParticules(qc::graphic::Particule(preDefMaterials[indexMat], 1, &it));
	}
	scene.sortParticules();
}


//-- INIT PHYSIC ---------------------
void Application::initPhysic()
{
	linkPhysicGraphic = std::map<qc::graphic::Particule*, int>();

	//physicLinkType = PhysicType::GRAVITATIONAL;
	physicLinkType = PhysicType::SIMPLE_ATTRACTION;

	physicSystem = qc::physic::PhysicalSystem(physicLinkType);
	physicSystem.setBboxMax(scene.getBboxMax() * (2 / 3.f));
	physicSystem.setBboxMin(scene.getBboxMin() * (2 / 3.f));

	auto& particules = scene.getParticules();

	for (auto& it : particules)
	{
		float mass;
		mass = it.getIntensity() * it.getRadiusAttenuation();
		if (it.getIntensity() < 1000)
			mass /= 2000000; // ratio Sun / Earth

		float radius = it.getRadius();
		float radiusAttraction = 1.5f * it.getRadiusAttenuation();

		int temp = physicSystem.addObject(it.getPosition(), mass, radius, radiusAttraction);
		linkPhysicGraphic.insert(std::make_pair(&it, temp));
	}
}

//-- UPDATE PHYSIC -------------------
void Application::updatePhysic()
{
	physicSystem.update(1 / discretizationFrequency);
}

//-- RESET PHYSICAL PARTICULES SYSTEM 
void Application::resetPhysicalParticulesSystem()
{
	activePhysic = false;

	scene.clearParticules();
	scene.clearPointLight();
	physicSystem.clearObjects();

	initLights();
	initParticules();
	initPhysic();

	resetSystem = false;

	ImGui_ImplGlfwGL3_InvalidateDeviceObjects(); // Use to fixe the INVALIDE of ImGui and the particule reset
}

//-- SYNCHRO GRAPHIC PHYSIC ----------
void Application::updateGraphicFromPhysic()
{
	for (auto& it : linkPhysicGraphic)
	{
		const auto& physicalObject = physicSystem.getPhysicalObject(it.second);
		it.first->setPosition(glm::vec4(physicalObject.getPosition(), 1));
	}
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
		ImGui::Text("Nb Particules (Point Lights) %d", scene.getParticules().size());
		
		if (ImGui::Button("Render All Post Process Pass"))
		{
			renderOptions = RenderOptions::RENDER_ALL;
			renderer->setRenderPostProcess(renderOptions);
		}
		
		std::string titleButton = ((renderOptions & RenderOptions::RENDER_EMISSIVE) == RenderOptions::RENDER_EMISSIVE) ? "Dont Render Emissive" : "Render Emissive";
		if (ImGui::Button(titleButton.c_str()))
		{
			if ((renderOptions & RenderOptions::RENDER_EMISSIVE) == RenderOptions::RENDER_EMISSIVE)
				renderOptions = static_cast<RenderOptions>(renderOptions & ~(RenderOptions::RENDER_EMISSIVE | RenderOptions::RENDER_BLUR));
			else
				renderOptions = static_cast<RenderOptions>(renderOptions | RenderOptions::RENDER_EMISSIVE);

			renderer->setRenderPostProcess(renderOptions);
		}


		titleButton = ((renderOptions & RenderOptions::RENDER_BLUR) == RenderOptions::RENDER_BLUR) ? "Dont Render Blur" : "Render Blur";
		if (ImGui::Button(titleButton.c_str()))
		{
			if ((renderOptions & RenderOptions::RENDER_EMISSIVE) == RenderOptions::RENDER_EMISSIVE)
			{
				if ((renderOptions & RenderOptions::RENDER_BLUR) == RenderOptions::RENDER_BLUR)
					renderOptions = static_cast<RenderOptions>(renderOptions & ~RenderOptions::RENDER_BLUR);
				else
					renderOptions = static_cast<RenderOptions>(renderOptions | RenderOptions::RENDER_BLUR);
			}

			renderer->setRenderPostProcess(renderOptions);
		}

		if((renderOptions & RenderOptions::RENDER_BLUR) == RenderOptions::RENDER_BLUR)
			ImGui::SliderInt("Nb blur iteration", &(renderer->getNbBlurPass()), 1, 10);


		titleButton = ((renderOptions & RenderOptions::RENDER_POINT_LIGHTS) == RenderOptions::RENDER_POINT_LIGHTS) ? "Dont Render Point Lights" : "Render Point Lights";
		if (ImGui::Button(titleButton.c_str()))
		{
			if ((renderOptions & RenderOptions::RENDER_POINT_LIGHTS) == RenderOptions::RENDER_POINT_LIGHTS)
				renderOptions = static_cast<RenderOptions>(renderOptions & ~RenderOptions::RENDER_POINT_LIGHTS);
			else
				renderOptions = static_cast<RenderOptions>(renderOptions | RenderOptions::RENDER_POINT_LIGHTS);

			renderer->setRenderPostProcess(renderOptions);
		}
		
		titleButton = ((renderOptions & RenderOptions::RENDER_DIR_LIGHTS) == RenderOptions::RENDER_DIR_LIGHTS) ? "Dont Render Dir Lights" : "Render Dir Lights";
		if (ImGui::Button(titleButton.c_str()))
		{
			if ((renderOptions & RenderOptions::RENDER_DIR_LIGHTS) == RenderOptions::RENDER_DIR_LIGHTS)
				renderOptions = static_cast<RenderOptions>(renderOptions & ~RenderOptions::RENDER_DIR_LIGHTS);
			else
				renderOptions = static_cast<RenderOptions>(renderOptions | RenderOptions::RENDER_DIR_LIGHTS);

			renderer->setRenderPostProcess(renderOptions);
		}

		titleButton = (activePhysic) ? "Unactive Physic" : "Active Physic";
		if (ImGui::Button(titleButton.c_str()))
			activePhysic = !activePhysic;

		ImGui::SameLine();
		titleButton = "Reset Celerity";
		if (ImGui::Button(titleButton.c_str()))
			physicSystem.resetCelerities();
		
		ImGui::SameLine();
		titleButton = "Reset System";
		if (ImGui::Button(titleButton.c_str()))
			resetSystem = true;
		
		int physicType = static_cast<int>(physicLinkType);
		if (ImGui::RadioButton("Simple Attraction", &physicType, PhysicType::SIMPLE_ATTRACTION) ||
			ImGui::RadioButton("Lennard Jones", &physicType, PhysicType::LENNARD_JONES) ||
			ImGui::RadioButton("Gravitational", &physicType, PhysicType::GRAVITATIONAL))
		{
			physicSystem.resetCelerities();
		}

		if (physicType != physicLinkType)
		{
			physicLinkType = static_cast<PhysicType>(physicType);
			physicSystem.setPhysicType(physicLinkType);
		}

		if (physicLinkType == PhysicType::SIMPLE_ATTRACTION)
		{
			float k = physicSystem.getLink()->getStiffness();
			if(ImGui::SliderFloat("Stiffness coeff", &k, 0.001f, 50.f, "%.4f"))
				physicSystem.getLink()->setStiffness(k);
		}
		else if (physicLinkType == PhysicType::LENNARD_JONES)
		{
			qc::physic::LennardJonesLink* link = static_cast<qc::physic::LennardJonesLink*>(physicSystem.getLink());
			float k = link->getStiffness();
			if (ImGui::SliderFloat("Potential well", &k, 1.f, 10.f))
				link->setStiffness(k);

			k = link->getPower();
			if (ImGui::SliderFloat("Power factor", &k, 1/12.f, 12.f))
				link->setPower(k);
		}

		ImGui::SliderFloat("Physical Discretization Frequency", &discretizationFrequency, 5.f, 200.f);

		ImGui::End();
	}


	ImGui::Render();
}