#include "WWkeybinds.h"

// camera movement
void translateViewMatrix(double deltaTime) {
    glm::mat4 translation(1.0f);
    glm::mat4 rotation(1.0f);

    float acceleration = 0.1f * static_cast<float>(deltaTime);
    float angle = glm::radians(45.0f) * static_cast<float>(deltaTime);

    if (wIsPressed && !camera.carStarted) {
        camera.carStarted = true;
    }
    else {
        if (aIsPressed && camera.carStarted) {
            rotation = glm::rotate(rotation, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (dIsPressed && camera.carStarted) {
            rotation = glm::rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }

    if (camera.carStarted) {
        camera.velocity += acceleration * static_cast<float>(deltaTime);
        if (camera.velocity > camera.maxSpeed) {
            translation = glm::translate(translation, glm::vec3(0.0f, 0.0f, camera.maxSpeed));
        }
        else {
            translation = glm::translate(translation, glm::vec3(0.0f, 0.0f, camera.velocity));
        }
    }

    glm::mat4 transform = translation * rotation;
    camera.updateCamera(transform);
}



void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos) {
    //std::cout << xPos << " : " << yPos << "\n";
    if (!m_usePrevCursorPosition) {
        xCursorPos = xPos;
        yCursorPos = yPos;
        m_usePrevCursorPosition = true;
    }

    float dx = static_cast<float>(xPos - xCursorPos);
    float dy = static_cast<float>(yPos - yCursorPos);

    xCursorPos = xPos;
    yCursorPos = yPos;

    float slow = 15.0f;
    angledx = -dx / slow;
    angledy = -dy / slow;
}

void mouseButton_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "Left mouse button pressed\n";
        leftMouseButtonIsPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        std::cout << "Left mouse button released\n";
        leftMouseButtonIsPressed = false;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
    case GLFW_KEY_W:
        switch (action) {
        case GLFW_PRESS:
            //std::cout << "Key W Pressed\n";
            wIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key W Held Down\n";
            break;
        case GLFW_RELEASE:
            //std::cout << "Key W Has Been Released\n";
            wIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_S:
        switch (action) {
        case GLFW_PRESS:
            //std::cout << "Key S Pressed\n";
            sIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key S Held Down\n";
            break;
        case GLFW_RELEASE:
            //std::cout << "Key S Has Been Released\n";
            sIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_A:
        switch (action) {
        case GLFW_PRESS:
            //std::cout << "Key A Pressed\n";
            aIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key A Held Down\n";
            break;
        case GLFW_RELEASE:
            //std::cout << "Key A Has Been Released\n";
            aIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_D:
        switch (action) {
        case GLFW_PRESS:
            //std::cout << "Key D Pressed\n";
            dIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key D Held Down\n";
            break;
        case GLFW_RELEASE:
            //std::cout << "Key D Has Been Released\n";
            dIsPressed = false;
            break;
        }
        break;
    }
}

