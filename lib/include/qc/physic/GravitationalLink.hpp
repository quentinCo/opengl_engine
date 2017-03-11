#pragma once

#include "Link.hpp"

namespace qc
{

namespace physic
{

class GravitationalLink : public Link
{

public:
	GravitationalLink()
		:Link()
	{}

	GravitationalLink(PhysicalObject* object1, PhysicalObject* object2)
		: Link(object1, object2)
	{}

	virtual void update(float);

private:
	//-- Gravitational constant
	static const float gConst;

};

}//! namespace physic

}//! namespace qc