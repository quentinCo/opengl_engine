#pragma once

namespace qc
{

namespace physic
{

class Updatable
{
public:
	virtual void update(float h) = 0;
};

} // ! namespace physic

} //! namespace qc