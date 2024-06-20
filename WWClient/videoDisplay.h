#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <SFML/Graphics.hpp>


sf::Texture texture;

void displayWindow() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "mySimpleGame");
    if (!texture.create(800, 600)) {
        fprintf(stderr, "VIDEO_DISPLAY: Error creating a texture\n");
        exit(1);
    }
    sf::Sprite sprite(texture);


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
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
