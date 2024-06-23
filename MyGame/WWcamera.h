#ifndef WWCAMERA_H
#define WWCAMERA_H

#include "WWhelpers.h"

class Camera {
public:
	glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f);
	glm::vec3 front = glm::vec3(1.0f, 0.5, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	glm::mat4 getViewMatrix() {
		return glm::lookAt(position, position + front, up);
	}
};



#endif
