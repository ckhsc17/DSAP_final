#include <iostream>
#include <SFML/Window.hpp>

int main(int, char**){
    sf::VideoMode mode = sf::VideoMode(1024, 1024/4*3);
    
    sf::Window window(mode, "DSAP Final Project");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
    }
}
