#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>

#include "UDP6send.h"

enum class Direction {
    Forward,
    Left,
    Backward,
    Right
};

void displayWindow(UDPSend6& sender);

void updateTexture(uint8_t* rgb_buffer);

#endif
