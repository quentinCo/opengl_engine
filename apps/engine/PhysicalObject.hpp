#pragma once

#include <glm/glm.hpp>

#include "Updatable.hpp"

namespace qc
{

namespace physic
{

class PhysicalObject : public Updatable
{

public:
	PhysicalObject();

	PhysicalObject(float m, float r, float rAttract);

	const glm::vec3& getPosition() const
		{return position;}

	const glm::vec3& getCelerity() const
		{return celerity;}

	const glm::vec3& getForce() const
		{return force;}

	float getMass() const
		{return mass;}

	float getRadius() const
		{return radius;}

	float getRadiusAttraction() const
		{return radiusAttraction;}

	void setPosition(const glm::vec3& pos)
		{position = pos;}

	void setCelerity(const glm::vec3& cel)
		{celerity = cel;}

	void setForce(const glm::vec3& force)
		{this->force = force;}

	virtual void update(float h);

private:
	glm::vec3 position;
	glm::vec3 celerity;
	glm::vec3 force;

	float mass;
	float radius;
	float radiusAttraction;
};

} //! namespace physic

} //! namespace qc