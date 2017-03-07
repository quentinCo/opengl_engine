#include <vector>

#include <glm/glm.hpp>

#include "PhysicalObject.hpp"
#include "Link.hpp"
#include "GravitationalLink.hpp"
#include "Updatable.hpp"

namespace qc
{

namespace physic
{

class PhysicalSystem : public Updatable
{

public:
	enum PhysicType
	{
		GRAVITATIONAL
	};

	PhysicalSystem()
	{}

	PhysicalSystem(PhysicType physicType);

	const std::vector<PhysicalObject>& getPhysicalObjects() const
		{return objects;}

	const PhysicalObject& getPhysicalObject(int index) const
		{return objects[index];}

	// return index of physical particule
	//PhysicalObject* addObject(const glm::vec3& position, float mass, float radius, float radiusAttraction);
	int addObject(const glm::vec3& position, float mass, float radius, float radiusAttraction);

	virtual void update(float h);

private:
	std::vector<PhysicalObject> objects;
	//Link

};

}//! namespace physic

}//! namespace qc

