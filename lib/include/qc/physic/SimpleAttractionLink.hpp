#pragma once

#include "Link.hpp"

namespace qc
{

namespace physic
{

class SimpleAttractionLink : public Link
{

public:
	SimpleAttractionLink()
		:Link()
	{}

	SimpleAttractionLink(PhysicalObject* object1, PhysicalObject* object2, float k)
		:Link(object1, object2, k)
	{}

	virtual void update(float);
};

}//! namespace physic

}//! namespace qc