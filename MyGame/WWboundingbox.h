#ifndef WWBOUNDINGBOX_H
#define WWBOUNDINGBOX_H

#include "WWhelpers.h"

class BoundingBox {
public:
	glm::vec3 min;
	glm::vec3 max;

	BoundingBox();

	void expand(const glm::vec3& vert);

	bool isInside(const glm::vec3& vert) const;
};



#endif
