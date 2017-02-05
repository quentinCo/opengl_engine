#include "Camera.hpp"

using namespace qc;

Camera::Camera(glmlv::GLFWHandle& glfwHandle, float fovy, float nearPlan, float farPlan, float speed)
	: viewController(glfwHandle.window(), 0.3f), viewportSize(glfwHandle.framebufferSize()),
	fovY(fovy), nearPlan(nearPlan), farPlan(farPlan), speed(speed)
{
	computeProjMatrix(viewportSize, fovy, nearPlan, farPlan, projMatrix);
	viewController.setSpeed(speed);
	viewMatrix = viewController.getViewMatrix();
}

void Camera::setViewPortSize(const glm::ivec2& viewportSize)
{
	this->viewportSize = viewportSize;
	computeProjMatrix(viewportSize, fovY, nearPlan, farPlan, projMatrix);
}

void Camera::setFovY(float fovy)
{
	fovY = fovy;
	computeProjMatrix(viewportSize, fovY, nearPlan, farPlan, projMatrix);
}
void Camera::setNearPlan(float nearPlan)
{
	this->nearPlan = nearPlan;
	computeProjMatrix(viewportSize, fovY, nearPlan, farPlan, projMatrix);
}

void Camera::setFarPlan(float farPlan)
{
	this->farPlan = farPlan;
	computeProjMatrix(viewportSize, fovY, nearPlan, farPlan, projMatrix);
}

void Camera::setSpeed(float speed)
{
	this->speed = speed;
	viewController.setSpeed(speed);	
}

void Camera::updateViewController(float time)
{
	viewController.update(time);
	viewMatrix = viewController.getViewMatrix();
}

void Camera::computeModelsMatrix(const glm::mat4& modelMatrix, glm::mat4& mvMatrix, glm::mat4& mvpMatrix, glm::mat4& normalMatrix) const
{
	mvMatrix = viewMatrix * modelMatrix;
	mvpMatrix = projMatrix * mvMatrix;
	normalMatrix = glm::transpose(glm::inverse(mvMatrix));
}

void Camera::computeProjMatrix(const glm::ivec2 viewportSize, float fovy, float nearPlan, float farPlan, glm::mat4& res)
	{res = glm::perspective(fovy, float(viewportSize.x) / viewportSize.y, nearPlan, farPlan);}

