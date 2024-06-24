#ifndef WWCAMERA_H
#define WWCAMERA_H

#include "WWhelpers.h"

class Camera {
public:

	Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
		this->viewMatrix = glm::lookAt(position, target, up);
	}

	Camera() {
		this->viewMatrix = glm::lookAt(glm::vec3(0.1f, 0.5f, 0.1f), glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::mat4 getViewMatrix() const {
		return this->viewMatrix;
	}

	void updateCamera(const glm::mat4& transform) {
		viewMatrix = transform * viewMatrix;
	}

	glm::vec3 getPosition() {
		glm::mat4 viewInverse = glm::inverse(viewMatrix);
		return glm::vec3(viewInverse[3]);
	}

private:
	glm::mat4 viewMatrix;
	glm::vec3 velocity;
};



#endif
