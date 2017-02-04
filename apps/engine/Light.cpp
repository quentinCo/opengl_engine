#include "Light.hpp"

using namespace qc;

glm::vec3 DirectionalLight::computeDirection(float phi, float theta)
{
	float phiRadian = glm::radians(phi);
	float thetaRadian = glm::radians(theta);
	const auto cosPhi = glm::cos(phiRadian);
	const auto sinPhi = glm::sin(phiRadian);
	const auto sinTheta = glm::sin(thetaRadian);
	return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadian), cosPhi * sinTheta);
}