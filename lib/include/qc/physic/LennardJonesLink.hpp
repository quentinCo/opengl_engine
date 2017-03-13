#pragma once

#include "Link.hpp"

namespace qc
{

namespace physic
{

class LennardJonesLink : public Link
{

public:
	LennardJonesLink()
		:Link()
	{}

	LennardJonesLink(PhysicalObject* object1, PhysicalObject* object2)
		: Link(object1, object2)
	{}

	virtual void update(float);
	
};

}//! namespace physic

}//! namespace qc