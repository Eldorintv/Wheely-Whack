#ifndef WWKEYBINDS_H
#define WWKEYBINDS_H

#include "WWhelpers.h"
#include "WWcamera.h"

extern Camera camera;

extern bool wIsPressed;
extern bool sIsPressed;
extern bool aIsPressed;
extern bool dIsPressed;

static bool leftMouseButtonIsPressed{ false };

static double xCursorPos{ 0 };
static double yCursorPos{ 0 };

static double m_cursorPrevX{ 0 };
static double m_cursorPrevY{ 0 };
static float angledx{ 0 };
static float angledy{ 0 };
static bool m_usePrevCursorPosition{ false };


// camera movement
static void translateViewMatrix(double deltaTime) {
    glm::mat4 translation(1.0f);
    glm::mat4 rotation(1.0f);

    float drivingSpeed = 2.0f * static_cast<float>(deltaTime);
    float angle = glm::radians(45.0f) * static_cast<float>(deltaTime);

    if (wIsPressed) {
        translation = glm::translate(translation, glm::vec3(0.0f, 0.0f, drivingSpeed));
    }
    if (sIsPressed) {
        translation = glm::translate(translation, glm::vec3(0.0f, 0.0f, -drivingSpeed));

        if (aIsPressed) {
            rotation *= glm::rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (dIsPressed) {
            rotation *= glm::rotate(rotation, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
    else {
        if (aIsPressed) {
            rotation = glm::rotate(rotation, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        if (dIsPressed) {
            rotation = glm::rotate(rotation, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
    //if (leftMouseButtonIsPressed) {
    //    rotation = glm::rotate(rotation, glm::radians(angledx), glm::vec3(0.0f, 1.0f, 0.0f));
    //}
    //if (leftMouseButtonIsPressed) {
    //    rotation = glm::rotate(rotation, glm::radians(angledy), glm::vec3(1.0f, 0.0f, 0.0f));
    //}
    glm::mat4 transform = translation * rotation;
    camera.updateCamera(transform);
}



static void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos) {
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

static void mouseButton_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "Left mouse button pressed\n";
        leftMouseButtonIsPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        std::cout << "Left mouse button released\n";
        leftMouseButtonIsPressed = false;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch (key) {
    case GLFW_KEY_W:
        switch (action) {
        case GLFW_PRESS:
            std::cout << "Key W Pressed\n";
            wIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key W Held Down\n";
            break;
        case GLFW_RELEASE:
            std::cout << "Key W Has Been Released\n";
            wIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_S:
        switch (action) {
        case GLFW_PRESS:
            std::cout << "Key S Pressed\n";
            sIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key S Held Down\n";
            break;
        case GLFW_RELEASE:
            std::cout << "Key S Has Been Released\n";
            sIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_A:
        switch (action) {
        case GLFW_PRESS:
            std::cout << "Key A Pressed\n";
            aIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key A Held Down\n";
            break;
        case GLFW_RELEASE:
            std::cout << "Key A Has Been Released\n";
            aIsPressed = false;
            break;
        }
        break;
    case GLFW_KEY_D:
        switch (action) {
        case GLFW_PRESS:
            std::cout << "Key D Pressed\n";
            dIsPressed = true;
            break;
        case GLFW_REPEAT:
            //std::cout << "Key D Held Down\n";
            break;
        case GLFW_RELEASE:
            std::cout << "Key D Has Been Released\n";
            dIsPressed = false;
            break;
        }
        break;
    }
}

#endif

