#include "WWhelpers.h"
#include "WWboundingbox.h"

bool BoundingBox::isInside(const glm::vec3& vert, const float err) const {
	return (vert.x + err >= min.x
		&& vert.y + err >= min.y
		&& vert.z + err >= min.z
		&& vert.x - err <= max.x
		&& vert.y - err <= max.y
		&& vert.z - err <= max.z);
}

BoundingBox::BoundingBox() : min(FLT_MAX), max(FLT_MIN) {}

BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

void BoundingBox::expand(const glm::vec3& vert) {
	min = glm::min(min, vert);
	max = glm::max(max, vert);
}

BoundingBox computeBoundingBox(const std::vector<glm::vec3>& vertices) {
	if (vertices.empty()) return BoundingBox{ glm::vec3(0.0f), glm::vec3(0.0f) };
	
	BoundingBox box;

	for (const auto& vertex : vertices) {
		box.expand(vertex);
	}
	return box;
}

BoundingBox computeBoundingBox(const std::vector<Vertex>& vertices) {
	if (vertices.empty()) return BoundingBox{ glm::vec3(0.0f), glm::vec3(0.0f) };

	BoundingBox box;

	for (const auto& vertex : vertices) {
		box.expand(vertex.pos);
	}
	return box;
}