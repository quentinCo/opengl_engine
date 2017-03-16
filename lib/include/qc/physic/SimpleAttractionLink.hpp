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
		:Link(), absorption(0)
	{}

	SimpleAttractionLink(PhysicalObject* object1, PhysicalObject* object2, float k)
		:Link(object1, object2, k), absorption(0)
	{}
	

	float getAbsorption() const
		{return absorption;}

	float getMaxAbsorption() const
		{return 1000;}

	void setAbsorption(float a)
		{absorption = a;}

	virtual void update(float);

private:
	float absorption;
};

}//! namespace physic

}//! namespace qc