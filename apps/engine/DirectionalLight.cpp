#include "DirectionalLight.hpp"

using namespace qc;


//-- COMPUTE DIRECTION -----------------

glm::vec4 DirectionalLight::computeDirection(float phi, float theta)
{
	float phiRadian = glm::radians(phi);
	float thetaRadian = glm::radians(theta);
	const auto cosPhi = glm::cos(phiRadian);
	const auto sinPhi = glm::sin(phiRadian);
	const auto sinTheta = glm::sin(thetaRadian);
	return glm::vec4(sinPhi * sinTheta, glm::cos(thetaRadian), cosPhi * sinTheta, 0);
}