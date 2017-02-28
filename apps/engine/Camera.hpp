#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/GLFWHandle.hpp>

namespace qc
{

class Camera
{
public:
	Camera(glmlv::GLFWHandle& glfwHandle, glm::vec3& position  = glm::vec3(0), glm::vec3& front = glm::vec3(0,0,-1),
		float fovy = 0, float nearPlan = 0.1f, float farPlan = 100.f, float speedMove = 1.f, float speedRotate = 0.01f);
	
	const glm::mat4& getProjMatrix() const
		{return projMatrix;}

	const glm::mat4& getViewMatrix() const
		{return viewMatrix;}

	const glm::vec3& getPosition() const
		{return position;}

	void setViewPortSize(const glm::ivec2& viewportSize);
	void setFovY(float fovy);
	void setNearPlan(float nearPlan);
	void setFarPlan(float farPlan);
	void setSpeedMove(float speed)
		{this->speedMove = speed;}
	void setSpeedRotate(float speed)
		{this->speedRotate = speed;}

	void Camera::updateViewController(float time);
	
	void computeModelsMatrix(const glm::mat4& modelMatrix, glm::mat4& mvMatrix, glm::mat4& mvpMatrix, glm::mat4& normalMatrix) const;

private:
	GLFWwindow* window;
	float speedMove;
	float speedRotate;

	glm::vec3 frontVector;
	glm::vec3 leftVector;
	glm::vec3 upVector;

	glm::vec3 position;

	float phi;
	float theta;

	bool canRotate = false;
	glm::dvec2 lastCursorPosition = glm::vec2(0,0);
//--------------------------
	glm::ivec2 viewportSize;
	float fovY;
	float nearPlan;
	float farPlan;
//--------------------------

	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;

	void computeDirectionsVector();

	void computeProjMatrix()
		{projMatrix = glm::perspective(fovY, float(viewportSize.x) / viewportSize.y, nearPlan, farPlan);}

	bool moving(float time);
	bool rotation(float time);

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