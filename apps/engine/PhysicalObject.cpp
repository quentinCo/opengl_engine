#include "PhysicalObject.hpp"

using namespace qc::physic;

PhysicalObject::PhysicalObject()
	: position(glm::vec3(0)), celerity(glm::vec3(1)), force(glm::vec3(0)), mass(0), radius(0), radiusAttraction(0)
{}

PhysicalObject::PhysicalObject(float m, float r, float rAttract)
	: PhysicalObject()
{
	mass = m;
	radius = r;
	radiusAttraction = rAttract;
}

void PhysicalObject::update(float h)
{
	position += h * celerity;
}