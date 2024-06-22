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


sf::Texture texture;
constexpr size_t MaxKeys = sf::Keyboard::KeyCount;

enum class Direction {
    Forward,
    Left,
    Backward,
    Right
};

void functionA(Direction d) {
    switch (d) {
    case Direction::Forward:
        std::cout << "Forward Pressed!\n";
        break;
    case Direction::Left:
        std::cout << "Left Pressed!\n";
        break;
    case Direction::Backward:
        std::cout << "Backward Pressed!\n";
        break;
    case Direction::Right:
        std::cout << "Right Pressed!\n";
        break;
    }
}

void functionAFinish(Direction d) {
    switch (d) {
    case Direction::Forward:
        std::cout << "Forward Released!\n";
        break;
    case Direction::Left:
        std::cout << "Left Released!\n";
        break;
    case Direction::Backward:
        std::cout << "Backwards Released!\n";
        break;
    case Direction::Right:
        std::cout << "Right Released!\n";
        break;
    }
}

void displayWindow() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "mySimpleGame");
    if (!texture.create(800, 600)) {
        fprintf(stderr, "VIDEO_DISPLAY: Error creating a texture\n");
        exit(1);
    }
    sf::Sprite sprite(texture);

    // keyboard input - W-A-S-D
    std::array<bool, MaxKeys> keyStates = { false };


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (!keyStates[event.key.code]) {
                    keyStates[event.key.code] = true;

                    if (event.key.code == sf::Keyboard::W) {
                        functionA(Direction::Forward);
                    }
                    if (event.key.code == sf::Keyboard::A) {
                        functionA(Direction::Left);
                    }
                    if (event.key.code == sf::Keyboard::S) {
                        functionA(Direction::Backward);
                    }
                    if (event.key.code == sf::Keyboard::D) {
                        functionA(Direction::Right);
                    }
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                keyStates[event.key.code] = false;

                if (event.key.code == sf::Keyboard::W) {
                    functionAFinish(Direction::Forward);
                }
                if (event.key.code == sf::Keyboard::A) {
                    functionAFinish(Direction::Left);
                }
                if (event.key.code == sf::Keyboard::S) {
                    functionAFinish(Direction::Backward);
                }
                if (event.key.code == sf::Keyboard::D) {
                    functionAFinish(Direction::Right);
                }
            }
        }

        window.clear();
        window.draw(sprite);
        window.display();
    }
}

void updateTexture(uint8_t* rgb_buffer) {
    sf::Uint8* pixels = rgb_buffer;
    texture.update(pixels);

    // needs to be freed still, not ideal solution
    free(rgb_buffer);
}

#endif
