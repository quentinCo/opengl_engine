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

		renderer.renderScene(scene, camera);

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
	
	camera = qc::Camera(m_GLFWHandle, 70.f, 0.01f * scene.getSceneSize(), scene.getSceneSize(), scene.getSceneSize() * 0.1f);
	renderer = qc::Renderer((m_ShadersRootPath / m_AppName), m_nWindowWidth, m_nWindowHeight);

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