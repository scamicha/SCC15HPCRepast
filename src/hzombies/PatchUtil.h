/**
 *
 * Created for SC15 Student Cluster Competetion.
 *
 * Author: Byung H. Park, Oak Ridge National Laboratory
 *
 */

#ifndef patch_utils_h
#define patch_utils_h

#include "relogo/Turtle.h"
#include "relogo/Patch.h"

/**
 * Operator(() that implements move of a turtle to a patch
 * 
 * Hoony Park, ORNL
 */
class MoveToPatch {
private:
    repast::relogo::Patch* patch;
public:
	MoveToPatch(repast::relogo::Patch* p) : patch(p){}
	virtual ~MoveToPatch() {}
	void operator()(repast::relogo::Turtle* turtle) const;
};


void MoveToPatch::operator()(repast::relogo::Turtle* turtle) const {
    turtle->moveTo(patch);
}

#endif
