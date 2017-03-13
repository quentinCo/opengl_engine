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

	float getPower() const
		{return power;}

	void setPower(float p)
		{power = p;}

	virtual void update(float);
	
private:
	float power = 12;

};

}//! namespace physic

}//! namespace qc