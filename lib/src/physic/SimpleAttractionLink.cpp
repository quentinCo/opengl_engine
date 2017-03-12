#include <qc/physic/SimpleAttractionLink.hpp>

using namespace qc::physic;

void SimpleAttractionLink::update(float)
{
	if (!object1 || !object2)
		return;

	glm::vec3 direction = object2->getPosition() - object1->getPosition();
	float distance = static_cast<float>(glm::length(direction));

	float minDistAttraction = object2->getRadiusAttraction() + object1->getRadiusAttraction();
	if (distance > minDistAttraction)
		return;

	float l0 = object1->getRadius() + object2->getRadius();
	direction = glm::normalize(direction);
	glm::vec3 force = stiffness * direction * (l0 - distance);
//	force -= 2.f * (object1->getCelerity() - object2->getCelerity());

	object1->setForce(force);
	object2->setForce(-force);

/*	double dV = l->p1->v - l->p2->v;
	f = l->z + dV;

	l->p1->f -= f;
	l->p2->f += f;*/
}