#include <vector>
#include <deque>

#include <glm/glm.hpp>

#include "PhysicalObject.hpp"
#include "Link.hpp"
#include "Updatable.hpp"
#include "GravitationalLink.hpp"
#include "SimpleAttractionLink.hpp"
#include "LennardJonesLink.hpp"

namespace qc
{

namespace physic
{

class PhysicalSystem : public Updatable
{

public:
	enum PhysicType
	{
		SIMPLE_ATTRACTION,
		LENNARD_JONES,
		GRAVITATIONAL
	};

	PhysicalSystem()
	{}

	PhysicalSystem(PhysicType physicType);

	~PhysicalSystem()
		{if(physicalLink != nullptr) delete physicalLink;}

	PhysicalSystem(const PhysicalSystem& o);
	PhysicalSystem operator= (const PhysicalSystem& o);

	PhysicalSystem(PhysicalSystem&& o);
	PhysicalSystem operator=(PhysicalSystem&& o);

	const std::vector<PhysicalObject>& getPhysicalObjects() const
		{return objects;}

	const PhysicalObject& getPhysicalObject(int index) const
		{return objects[index];}
	
	const glm::vec3& getBboxMin() const
		{return bboxMin;}

	const glm::vec3& getBboxMax() const
		{return bboxMax;}

	const PhysicType getPhysicType() const
		{return physicalType;}

	Link* getLink() const
		{return physicalLink;}

	void setBboxMin(const glm::vec3& min)
		{bboxMin = min;}

	void setBboxMax(const glm::vec3& max)
		{bboxMax = max;}

	void setPhysicType(PhysicType type);

	// return index of physical particule
	//PhysicalObject* addObject(const glm::vec3& position, float mass, float radius, float radiusAttraction);
	int addObject(const glm::vec3& position, float mass, float radius, float radiusAttraction);

	void removeObject(unsigned int index);

	void removeObjects(unsigned int index, int nb);

	void clearObjects()
		{objects.clear();}

	void resetCelerities();

	virtual void update(float h);

private:
	std::vector<PhysicalObject> objects;
	Link* physicalLink = nullptr;

	PhysicType physicalType;
	std::deque<PhysicalObject*> updatesListe;

	glm::vec3 bboxMin = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 bboxMax = glm::vec3(std::numeric_limits<float>::lowest());

	Link* getPhysicalLink();

};

}//! namespace physic

}//! namespace qc

