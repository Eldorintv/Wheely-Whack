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
void translateViewMatrix(double deltaTime);

void cursorPosition_callback(GLFWwindow* window, double xPos, double yPos);

void mouseButton_callback(GLFWwindow* window, int button, int action, int mods);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif

