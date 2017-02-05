#pragma once

#include <glm/geometric.hpp>

#include <glmlv/GLFWHandle.hpp>
#include <glmlv/ViewController.hpp>

namespace qc
{

class Camera
{
public:
	Camera(glmlv::GLFWHandle& glfwHandle, float fovy = 0, float nearPlan = 0.1f, float farPlan = 100.f, float speed = 1.f);
	
	const glm::mat4& getProjMatrix() const
		{return projMatrix;}

	const glm::mat4& getViewMatrix() const
		{return viewController.getViewMatrix();}

	void setViewPortSize(const glm::ivec2& viewportSize);
	void setFovY(float fovy);
	void setNearPlan(float nearPlan);
	void setFarPlan(float farPlan);
	void setSpeed(float speed);

	void Camera::updateViewController(float time);
	
	void computeModelsMatrix(const glm::mat4& modelMatrix, glm::mat4& mvMatrix, glm::mat4& mvpMatrix, glm::mat4& normalMatrix);

private:
	glm::ivec2 viewportSize;
	float fovY;
	float nearPlan;
	float farPlan;
	float speed;

	glmlv::ViewController viewController;
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;

	static void computeProjMatrix(const glm::ivec2 viewportSize, float fovy, float nearPlan, float farPlan, glm::mat4& res);

	/*
	const auto viewportSize = m_GLFWHandle.framebufferSize();
	const auto projMatrix = glm::perspective(70.f, float(viewportSize.x) / viewportSize.y, 0.01f * m_SceneSize, m_SceneSize);
	const auto viewMatrix = m_viewController.getViewMatrix();

	const auto mvMatrix = viewMatrix * mesh.getModelMatrix();
	const auto mvpMatrix = projMatrix * mvMatrix;
	const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));
	*/
};

} // namespace qc