#ifndef WWCAMERA_H
#define WWCAMERA_H

#include "WWhelpers.h"

class Camera {
public:
	bool carStarted = false;
	float maxSpeed = 0.3f;
	float velocity = 0.005f;

	Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up);

	Camera();

	glm::mat4 getViewMatrix() const;

	void updateCamera(const glm::mat4& transform);

	void resetViewMatrix(const glm::vec3& newPosition = glm::vec3(0.0f, 0.5f, 0.0f));

	glm::vec3 getPosition() const;


private:
	glm::mat4 viewMatrix;
};



#endif
