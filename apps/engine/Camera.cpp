#define _USE_MATH_DEFINES
#define NOMINMAX
#include <cmath>

#include "Camera.hpp"

using namespace qc;

Camera::Camera(glmlv::GLFWHandle& glfwHandle, glm::vec3& position, glm::vec3& front,
	float fovy, float nearPlan, float farPlan, float speedMove, float speedRotate)
	: window(glfwHandle.window()) , position(position), frontVector(glm::normalize(front)), viewportSize(glfwHandle.framebufferSize()),
	fovY(fovy), nearPlan(nearPlan), farPlan(farPlan), speedMove(speedMove), speedRotate(speedRotate)
{
	theta = std::asin(frontVector.y);
	phi = std::acos(frontVector.z / std::cos(theta));
	computeDirectionsVector();
	computeProjMatrix();
	viewMatrix = glm::lookAt(position, position + frontVector, upVector);
}


//-- SETTERS -----------------------------

void Camera::setViewPortSize(const glm::ivec2& viewportSize)
{
	this->viewportSize = viewportSize;
	computeProjMatrix();
}

void Camera::setFovY(float fovy)
{
	fovY = fovy;
	computeProjMatrix();
}

void Camera::setNearPlan(float nearPlan)
{
	this->nearPlan = nearPlan;
	computeProjMatrix();
}

void Camera::setFarPlan(float farPlan)
{
	this->farPlan = farPlan;
	computeProjMatrix();
}


//-- UPDATE VIEW CONTROLLER ------------

void Camera::updateViewController(float time)
{
	bool hasMoved = false;

	if (moving(time))
		hasMoved = true;
	
	if (rotation(time))
		hasMoved = true;

	if(hasMoved)
		viewMatrix = glm::lookAt(position, position + frontVector, upVector);
}


//-- COMPUTE MODELS MATRIX --------------

void Camera::computeModelsMatrix(const glm::mat4& modelMatrix, glm::mat4& mvMatrix, glm::mat4& mvpMatrix, glm::mat4& normalMatrix) const
{
	mvMatrix = viewMatrix * modelMatrix;
	mvpMatrix = projMatrix * mvMatrix;
	normalMatrix = glm::transpose(glm::inverse(mvMatrix));
}

//-- COMPUTE DIRECTIONS VECTOR ---------

void Camera::computeDirectionsVector()
{
	frontVector = glm::vec3(std::cos(theta)*std::sin(phi), std::sin(theta), std::cos(theta)*std::cos(phi));
	leftVector = glm::vec3(std::sin(phi + M_PI * 0.5), 0, std::cos(phi + M_PI * 0.5));
	upVector = glm::cross(frontVector, leftVector);
}


//-- MOVING -----------------------------

bool Camera::moving(float time)
{
	glm::vec3 translation = glm::vec3(0);

	if (glfwGetKey(window, GLFW_KEY_W))
		translation += speedMove * time * frontVector;
	if (glfwGetKey(window, GLFW_KEY_S))
		translation -= speedMove * time * frontVector;
	
	if (glfwGetKey(window, GLFW_KEY_A))
		translation += speedMove * time * leftVector;
	if (glfwGetKey(window, GLFW_KEY_D))
		translation -= speedMove * time * leftVector;

	if (glfwGetKey(window, GLFW_KEY_SPACE))
		translation += speedMove * time * glm::vec3(0, 1, 0);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		translation -= speedMove * time * glm::vec3(0, 1, 0);

	if (translation != glm::vec3(0))
	{
		position += translation;
		return true;
	}
	return false;
}


//-- ROTATION ----------------------------

bool Camera::rotation(float time)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		canRotate = true;
		glfwGetCursorPos(window, &lastCursorPosition.x, &lastCursorPosition.y);
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		canRotate = false;

	if (canRotate)
	{
		glm::dvec2 currentCursorPosition;
		glfwGetCursorPos(window, &currentCursorPosition.x, &currentCursorPosition.y);
		glm::dvec2 delta = currentCursorPosition - lastCursorPosition;

		if (delta != glm::dvec2(0))
		{
			lastCursorPosition = currentCursorPosition;
			phi -= static_cast<float>(delta.x * speedRotate);
			theta = static_cast<float>(glm::min(M_PI * 0.5, glm::max(-M_PI * 0.5, theta - delta.y * speedRotate)));

			computeDirectionsVector();
			return true;
		}
	}	
	return false;
}
