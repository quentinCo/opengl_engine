#include <qc/physic/LennardJonesLink.hpp>

using namespace qc::physic;

void LennardJonesLink::update(float)
{
	if (!object1 || !object2)
		return;

	glm::vec3 direction = object2->getPosition() - object1->getPosition();
	float distance = static_cast<float>(glm::length(direction));
	float l0 = object2->getRadius() + object1->getRadius();

	direction = glm::normalize(direction);
	glm::vec3 force = (static_cast<float>(4.f * stiffness * (pow(l0 / distance, power) - pow(l0 / distance, power * 0.5f))) * direction) / distance;

	object1->addToForce(-force);
	object2->addToForce(force);
}