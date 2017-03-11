#include <qc/physic/GravitationalLink.hpp>

using namespace qc::physic;

const float GravitationalLink::gConst = static_cast<float>( 6.67384 /* pow(10, -11)*/);

void GravitationalLink::update(float h)
{
	if (!object1 || !object2)
		return;

	glm::vec3 direction = object2->getPosition() - object1->getPosition();
	float distance = static_cast<float>(glm::length(direction));
	float minDist = object2->getRadiusAttraction() + object1->getRadiusAttraction();
	if (distance > minDist)
		return;

	direction = glm::normalize(direction);
	glm::vec3 force = -gConst * object1->getMass() * object2->getMass() * direction / (distance * distance);

	object1->addToForce(-force);
	object2->addToForce(force);
}