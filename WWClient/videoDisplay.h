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

//void functionA(Direction d) {
//    switch (d) {
//    case Direction::Forward:
//        std::cout << "Forward Pressed!\n";
//        break;
//    case Direction::Left:
//        std::cout << "Left Pressed!\n";
//        break;
//    case Direction::Backward:
//        std::cout << "Backward Pressed!\n";
//        break;
//    case Direction::Right:
//        std::cout << "Right Pressed!\n";
//        break;
//    }
//}
//
//void functionAFinish(Direction d) {
//    switch (d) {
//    case Direction::Forward:
//        std::cout << "Forward Released!\n";
//        break;
//    case Direction::Left:
//        std::cout << "Left Released!\n";
//        break;
//    case Direction::Backward:
//        std::cout << "Backwards Released!\n";
//        break;
//    case Direction::Right:
//        std::cout << "Right Released!\n";
//        break;
//    }
//}

void displayWindow(UDPSend6& sender) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode(800, 600), "mySimpleGame", sf::Style::Default, settings);
    if (!texture.create(800, 600)) {
        fprintf(stderr, "VIDEO_DISPLAY: Error creating a texture\n");
        exit(1);
    }
    sf::Sprite sprite(texture);

    // keyboard input - W-A-S-D
    std::array<bool, MaxKeys> keyStates = { false };

    texture.setSmooth(true);

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
                        std::cout << "Sending receiver report\n";
                        char buf[2] = "W";
                        sender.send(buf, static_cast<int>(strlen(buf)));
                    }
                    if (event.key.code == sf::Keyboard::A) {
                        char buf[2] = "A";
                        sender.send(buf, static_cast<int>(strlen(buf)));
                    }
                    if (event.key.code == sf::Keyboard::S) {
                        char buf[2] = "S";
                        sender.send(buf, static_cast<int>(strlen(buf)));
                    }
                    if (event.key.code == sf::Keyboard::D) {
                        char buf[2] = "D";
                        sender.send(buf, static_cast<int>(strlen(buf)));
                    }
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                keyStates[event.key.code] = false;

                if (event.key.code == sf::Keyboard::W) {
                    char buf[2] = "w";
                    sender.send(buf, static_cast<int>(strlen(buf)));
                }
                if (event.key.code == sf::Keyboard::A) {
                    char buf[2] = "a";
                    sender.send(buf, static_cast<int>(strlen(buf)));
                }
                if (event.key.code == sf::Keyboard::S) {
                    char buf[2] = "s";
                    sender.send(buf, static_cast<int>(strlen(buf)));
                }
                if (event.key.code == sf::Keyboard::D) {
                    char buf[2] = "d";
                    sender.send(buf, static_cast<int>(strlen(buf)));
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
