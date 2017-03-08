#pragma once

#include "Updatable.hpp"
#include "PhysicalObject.hpp"

namespace qc
{

namespace physic
{

class Link : public Updatable
{

public:
	
	Link()
	{}

	Link(PhysicalObject* object1, PhysicalObject* object2)
		: object1(object1), object2(object2)
	{}

	PhysicalObject* getObject1()
		{return object1;}

	PhysicalObject* getObject2()
		{return object2;}

	void setObject1(PhysicalObject* object)
		{object1 = object;}
	
	void setObject2(PhysicalObject* object)
		{object2 = object;}

	virtual void update(float h) = 0;

protected:
	PhysicalObject* object1;
	PhysicalObject* object2;

};


}//! namespace physic

}//! namespace qc