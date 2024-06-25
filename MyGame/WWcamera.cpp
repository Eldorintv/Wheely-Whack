#include "WWcamera.h"


Camera::Camera(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
	this->viewMatrix = glm::lookAt(position, target, up);
}

Camera::Camera() {
	this->viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getViewMatrix() const {
	return this->viewMatrix;
}

void Camera::updateCamera(const glm::mat4& transform) {
	viewMatrix = transform * viewMatrix;
}

void Camera::resetViewMatrix(const glm::vec3& newPosition) {
	viewMatrix = glm::lookAt(newPosition, glm::vec3(newPosition.x + 1, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::getPosition() const {
	glm::mat4 viewInverse = glm::inverse(viewMatrix);
	return glm::vec3(viewInverse[3]);
}
