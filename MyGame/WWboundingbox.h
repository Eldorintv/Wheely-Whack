#ifndef WWBOUNDINGBOX_H
#define WWBOUNDINGBOX_H

#include "WWhelpers.h"

class BoundingBox {
public:
	glm::vec3 min;
	glm::vec3 max;

	BoundingBox();
	BoundingBox(const glm::vec3& min, const glm::vec3& max);

	void expand(const glm::vec3& vert);

	bool isInside(const glm::vec3& vert) const;
};

BoundingBox computeBoundingBox(const std::vector<glm::vec3>& vertices);
BoundingBox computeBoundingBox(const std::vector<Vertex>& vertices);

#endif
