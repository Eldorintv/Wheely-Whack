#include "WWhelpers.h"
#include "WWboundingbox.h"

bool BoundingBox::isInside(const glm::vec3& vert) const {
	return (vert.x >= min.x
		&& vert.y >= min.y
		&& vert.z >= min.z
		&& vert.x <= max.x
		&& vert.y <= max.y
		&& vert.z <= max.z);
}

BoundingBox::BoundingBox() : min(FLT_MAX), max(FLT_MIN) {}

void BoundingBox::expand(const glm::vec3& vert) {
	min = glm::min(min, vert);
	max = glm::max(max, vert);
}

static BoundingBox computeBoundingBox(const std::vector<Vertex>& vertices) {
	BoundingBox box;
	for (const auto& vertex : vertices) {
		box.expand(vertex.pos);
	}
	return box;
}