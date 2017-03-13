#include <algorithm>

#include <qc/physic/PhysicalSystem.hpp>
#include <qc/physic/GravitationalLink.hpp>
#include <qc/physic/SimpleAttractionLink.hpp>
#include <qc/physic/LennardJonesLink.hpp>

using namespace qc::physic;

PhysicalSystem::PhysicalSystem(PhysicType physicType)
	:physicalType(physicType)
{
	physicalLink = getPhysicalLink();	
}

PhysicalSystem::PhysicalSystem(const PhysicalSystem& o)
	: objects(o.objects), physicalType(o.physicalType)
{
	if (physicalLink != nullptr)
		delete physicalLink;

	physicalLink = getPhysicalLink();
}

PhysicalSystem PhysicalSystem::operator= (const PhysicalSystem& o)
{
	objects = o.objects;
	if (physicalLink != nullptr)
		delete physicalLink;

	physicalType = o.physicalType;
	physicalLink = getPhysicalLink();
	
	return *this;
}

PhysicalSystem::PhysicalSystem(PhysicalSystem&& o)
	: objects(std::move(o.objects)), physicalType(o.physicalType)
{
	if (physicalLink != nullptr)
		delete physicalLink;

	physicalLink = o.physicalLink;
	o.physicalLink = 0;
}

PhysicalSystem PhysicalSystem::operator=(PhysicalSystem&& o)
{
	objects = std::move(o.objects);
	if (physicalLink != nullptr)
		delete physicalLink;

	physicalType = o.physicalType;
	physicalLink = o.physicalLink;
	o.physicalLink = 0;

	return *this;
}

void PhysicalSystem::setPhysicType(PhysicType type)
{
	physicalType = type;
	physicalLink = getPhysicalLink();
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
		updatesListe.push_back(&it);

	while (updatesListe.size() != 1)
	{
		physicalLink->setObject1(updatesListe.front());
		for (int i = 1; i < updatesListe.size(); ++i)
		{
			physicalLink->setObject2(updatesListe[i]);
			physicalLink->update(h);
		}
		updatesListe.pop_front();
	}
	updatesListe.pop_front();

	for (auto& it : objects)
	{
		it.update(h);
		glm::vec3 position = it.getPosition();
		glm::vec3 celerity = it.getCelerity();
		bool changeDirection = false;
		for (int i = 0; i < 3; ++i)
		{
			if (position[i] >= bboxMax[i] || position[i] <= bboxMin[i])
			{
				changeDirection = true;
				celerity[i] = -celerity[i] * 0.5;
				position[i] = glm::min(bboxMax[i], glm::max(bboxMin[i], position[0]));
			}
		}
		if (changeDirection)
		{
			it.setCelerity(celerity);
			it.setPosition(position);
		}
	}
}

Link* PhysicalSystem::getPhysicalLink()
{
	if (physicalLink)
		delete physicalLink;

	Link* link = nullptr;
	switch (physicalType)
	{
		case SIMPLE_ATTRACTION:
			link = new SimpleAttractionLink();
			break;
		case LENNARD_JONES:
			link = new LennardJonesLink();
			break;
		case GRAVITATIONAL:
			link = new GravitationalLink();
			break;
	}
	return link;
}
