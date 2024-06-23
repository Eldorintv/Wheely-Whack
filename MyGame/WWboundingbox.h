#ifndef WWBOUNDINGBOX_H
#define WWBOUNDINGBOX_H

#include "WWhelpers.h"

class BoundingBox {
public:
	glm::vec3 min;
	glm::vec3 max;

	BoundingBox() : min(FLT_MAX), max(FLT_MIN) {}

	void expand(const glm::vec3& vert) {
		min = glm::min(min, vert);
		max = glm::max(max, vert);
	}

	bool isInside(const glm::vec3& vert) const {
		return (vert.x >= min.x
			&& vert.y >= min.y
			&& vert.z >= min.z
			&& vert.x <= max.x
			&& vert.y <= max.y
			&& vert.z <= max.z);
	}
};

BoundingBox computeBoundingBox(const std::vector<Vertex>& vertices) {
	BoundingBox box;
	for (const auto& vertex : vertices) {
		box.expand(vertex.pos);
	}
	return box;
}


#endif
