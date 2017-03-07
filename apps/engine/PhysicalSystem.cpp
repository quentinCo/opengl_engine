#include "PhysicalSystem.hpp"

using namespace qc::physic;

PhysicalSystem::PhysicalSystem(PhysicType physicType)
{
	/*
	switch(physicType)
	{
		case GRAVITATIONAL:
			link = new GravitationalLink();
			break;
	}
	*/
}

// return index of physical particule
int PhysicalSystem::addObject(const glm::vec3& position, float mass, float radius, float radiusAttraction)
{
	objects.emplace_back(mass, radius, radiusAttraction);
	PhysicalObject& object = objects.back();
	object.setPosition(position);
	return static_cast<int>(objects.size() - 1);
}

void PhysicalSystem::update(float h)
{
	for (auto& it : objects)
		it.update(h);
}