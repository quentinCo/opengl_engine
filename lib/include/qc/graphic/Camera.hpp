#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/geometric.hpp>

#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glmlv/GLFWHandle.hpp>

namespace qc
{
	
namespace graphic
{

class Camera
{
public:
	Camera(glmlv::GLFWHandle& glfwHandle, glm::vec3 position  = glm::vec3(0), glm::vec3 front = glm::vec3(0,0,-1),
		float fovy = 0, float nearPlan = 0.1f, float farPlan = 100.f, float speedMove = 1.f, float speedRotate = 0.01f);
	

	//-- GETTERS -------------------------

	const glm::mat4& getProjMatrix() const
		{return projMatrix;}

	const glm::mat4& getViewMatrix() const
		{return viewMatrix;}

	const glm::vec3& getPosition() const
		{return position;}

	const glm::vec3& getUpVector() const
		{return upVector;}

	const glm::vec3& getLeftVector() const
		{return leftVector;}

	const glm::vec3& getFrontVector() const
		{return frontVector;}


	//-- SETTERS --------------------------

	void setViewPortSize(const glm::ivec2& viewportSize);
	void setFovY(float fovy);
	void setNearPlan(float nearPlan);
	void setFarPlan(float farPlan);

	void setSpeedMove(float speed)
		{this->speedMove = speed;}

	void setSpeedRotate(float speed)
		{this->speedRotate = speed;}

	//-- UPDATE VIEW CONTROLLER -----------
	/*
		Update the camera position and orientation.
		Capture mouse and key events 
			Z - Q - S - D : Front - Left - Back - Right
			Shift - Space : Down - Up
			Left click : unlock camera rotation.
			Right click : lock camera rotation.
	*/
	void updateViewController(float time);
	
	//-- COMPUTE MODELS MATRIX -------------------------------
	/*
		Compute : Model View Matrix - Model View Proj Matrix and Normal Matrix for a specific model.
	*/
	void computeModelsMatrix(const glm::mat4& modelMatrix, glm::mat4& mvMatrix, glm::mat4& mvpMatrix, glm::mat4& normalMatrix) const;


	//-- COMPUTE MODEL VIEW PROJ MATRIX -----------------------
	/*
		Compute : Model View Matrix for a specific model.
	*/
	void computeMVPMatrix(const glm::mat4& modelMatrix, glm::mat4& mvpMatrix) const
		{mvpMatrix = projMatrix * viewMatrix * modelMatrix;}

private:
	//-- Pointer to the window for the event gestion
	GLFWwindow* window;

	//-- Velocities for the deplacement
	float speedMove;
	float speedRotate;

	//-- Camera vectors
	glm::vec3 frontVector;
	glm::vec3 leftVector;
	glm::vec3 upVector;

	//-- Position
	glm::vec3 position;

	//-- Camera angles
	float phi;
	float theta;

	//-- Variables for camera rotation
	bool canRotate = false;
	glm::dvec2 lastCursorPosition = glm::vec2(0,0);

	//-- Optic variables
	glm::ivec2 viewportSize;
	float fovY;
	float nearPlan;
	float farPlan;

	//-- Camera matrices
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;


	//-- COMPUTE DIRECTIONS VECTOR ---------------------------
	/*
		Compute the front, up and left vector according to the phi and theta angles.
	*/
	void computeDirectionsVector();


	//-- COMPUTE PROJ MATRIX ---------------------------------
	void computeProjMatrix()
		{projMatrix = glm::perspective(fovY, float(viewportSize.x) / viewportSize.y, nearPlan, farPlan);}

	//-- MOVING ----------------------------------------------
	/*
		Verify the different key event.
		Increment the position in function.

		Return true if an update is done
	*/
	bool moving(float time);


	//-- ROTATION --------------------------------------------
	/*
		Verify the mouse events.
		Update angles in function.

		Return true if an update is done
	*/
	bool rotation(float time);
};

} //!namespace graphic

} //! namespace qc