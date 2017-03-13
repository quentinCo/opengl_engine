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
		:Link(gConst)
	{}

	GravitationalLink(PhysicalObject* object1, PhysicalObject* object2)
		: Link(object1, object2, gConst)
	{}

	virtual void update(float);

private:
	//-- Gravitational constant
	static const float gConst;

};

}//! namespace physic

}//! namespace qc