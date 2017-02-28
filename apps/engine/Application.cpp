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

		if (chosenRenderer != currentRenderer)
		{
			currentRenderer = chosenRenderer;
			if (currentRenderer == DEFERRED)
				renderer = &deferred;
			else if (currentRenderer == FORWARD)
				renderer = &forward;
			else if (currentRenderer == FORWARD_PLUS)
				renderer = &forwardPlus;
		}
		/*
		glm::vec3 positionCamera = glm::inverse(camera.getViewMatrix())[3];
		auto& meshes = scene.getMeshes();
		for (auto& mesh : meshes)
		{
			glm::vec3 meshPosition = mesh.getModelMatrix()[3];
			glm::vec3 normal = glm::vec3(0, 0, -1);
			float angle = acos(dot((positionCamera - meshPosition), normal) / (normal.length() *  (positionCamera - meshPosition).length()));
			mesh.setRotation(angle, glm::vec3(0, 0, 1));
		}
		*/
		renderer->renderScene(scene, camera);

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
		/*
		for (auto& mesh : meshes)
		{
			glm::vec3 meshPosition = mesh.getModelMatrix()[3];
			glm::vec3 normal = glm::vec3(0, 0, -1);
			float angle = acos(dot((positionCamera - meshPosition), normal) / (normal.length() *  (positionCamera - meshPosition).length()));
			mesh.setRotation(angle, glm::vec3(0, 0, 1));
		}*/
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
	scene.addObj(m_AssetsRootPath / m_AppName / "models" / "Maya" / "maya2.obj");

	std::vector<qc::Mesh>& meshes = scene.getMeshes();
	meshes[1].setPosition(glm::vec3(500, 100, 0));

	scene.addDirectionalLight(qc::DirectionalLight(90.f, 45.f, glm::vec3(1), 1.f));
	//scene.addDirectionalLight(qc::DirectionalLight(45.f, 45.f, glm::vec3(1,0,1), 0.2f));
	/*std::srand(static_cast<unsigned int>(std::time(0))); //use current time as seed for random generator
	for (size_t i = 0; i < 3500; ++i) // 3500
	{
		float x = static_cast<float>(std::rand()) / RAND_MAX * 2500 - 1250;
		float y = static_cast<float>(std::rand()) / RAND_MAX * 1000;// +100;
		float z = static_cast<float>(std::rand()) / RAND_MAX * 1000 - 500;

		float r = static_cast<float>(std::rand()) / RAND_MAX;
		float v = static_cast<float>(std::rand()) / RAND_MAX;
		float b = static_cast<float>(std::rand()) / RAND_MAX;

		//float radius = static_cast<float>(std::rand()) / RAND_MAX * 500 + 50;
		float radius = static_cast<float>(std::rand()) / RAND_MAX * 100 + 50;
		float intensity = static_cast<float>(std::rand()) / RAND_MAX * 500 + 200;

		scene.addPointLight(qc::PointLight(radius, glm::vec3(x, y, z), glm::vec3(r,v,b), intensity));
	}*/

	scene.addPointLight(qc::PointLight(20, glm::vec3(200, 100, -260), glm::vec3(1, 0, 0), 300));
	scene.addPointLight(qc::PointLight(20, glm::vec3(-200, 100, -260), glm::vec3(0, 1, 0), 300));
	scene.addPointLight(qc::PointLight(20, glm::vec3(200, -100, -260), glm::vec3(0, 0, 1), 300));
	scene.addPointLight(qc::PointLight(20, glm::vec3(-200, -100, -260), glm::vec3(0, 1, 1), 300));
	scene.addPointLight(qc::PointLight(500, glm::vec3(-500, 50, 0), glm::vec3(0, 1, 1), 300));

	std::vector<qc::PointLight>& pointLights = scene.getPointLights();
	for(auto& it : pointLights)
		scene.addParticules(qc::Particule(&it));

	scene.setSsboDirectionalLights();
	scene.setSsboPointLights();
//---------------------------------
	/*float radius = 100;
	std::vector<glmlv::Vertex3f3f2f> vertices;
	std::vector<uint32_t> index;
	/*
	vertices.emplace_back(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec2(0));
	for (int i = 0; i < 20; ++i)
	{
		float x = static_cast<float>(radius * std::cos(i * 2 * M_PI / 10));
		float y = static_cast<float>(radius * std::sin(i * 2 * M_PI / 10));
		vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0, 0, -1), glm::vec2(-1));
		if (i > 0) index.push_back(i + 1);
		index.push_back(0);
		index.push_back(i + 1);
	}
	index.push_back(1);
	glmlv::SimpleGeometry sp = glmlv::makeSphere(radius);
	vertices = sp.vertexBuffer;
	index = sp.indexBuffer;
	qc::Material mat = qc::Material();
	mat.setColor(qc::Material::DIFFUSE_COLOR, glm::vec3(1,0,1)); // TODO : change for white ?
	std::vector<qc::Material> materials;
	materials.emplace_back(std::move(mat));

	qc::ShapeData shapeData = qc::ShapeData(index.size(),0,0);
	std::vector<qc::ShapeData> shapes = { shapeData };

	qc::Mesh particuleShape = qc::Mesh(vertices, index, shapes, glm::vec3(-500,2*radius,0));
	particuleShape.setMaterials(materials);
	particuleShape.setRotation(90, glm::vec3(0, 1, 0));
	scene.addObj(particuleShape);

	*/
//---------------------------------	
	camera = qc::Camera(m_GLFWHandle, glm::vec3(0,0,0), glm::vec3(0,0,-1), 70.f, 0.01f * scene.getSceneSize(), scene.getSceneSize(), scene.getSceneSize() * 0.1f);
	deferred = qc::DeferredRenderer((m_ShadersRootPath / m_AppName), m_nWindowWidth, m_nWindowHeight);
	forward = qc::ForwardRenderer((m_ShadersRootPath / m_AppName), m_nWindowWidth, m_nWindowHeight);
	forwardPlus = qc::ForwardPlusRenderer((m_ShadersRootPath / m_AppName), m_nWindowWidth, m_nWindowHeight);
	renderer = &forwardPlus;

    std::cout << "End INIT" << std::endl;
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

		ImGui::RadioButton("Deferred", &chosenRenderer, DEFERRED); ImGui::SameLine();
		ImGui::RadioButton("Forward", &chosenRenderer, FORWARD); ImGui::SameLine();
		ImGui::RadioButton("Forward Plus", &chosenRenderer, FORWARD_PLUS);
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