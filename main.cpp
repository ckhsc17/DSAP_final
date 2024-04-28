#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

template <
    std::size_t WIDTH,
    std::size_t HEIGHT,
    std::size_t CELL_SIZE,
    std::size_t PADDING_SIZE = 1>
class GameBoard
{
public:
    void Render(sf::RenderWindow &window) const
    {
        const sf::Vector2f topLeft(20.f, 20.f);

        sf::RectangleShape rectangle(
            sf::Vector2f(
                CELL_SIZE - 2 * PADDING_SIZE,
                CELL_SIZE - 2 * PADDING_SIZE));

        rectangle.setFillColor(sf::Color(128, 128, 128));

        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH; ++j)
            {
                rectangle.setPosition(
                    topLeft.x + j * CELL_SIZE + PADDING_SIZE,
                    topLeft.y + i * CELL_SIZE + PADDING_SIZE);
                window.draw(rectangle);
            }
        }
    }

private:
    std::array<std::array<bool, WIDTH>, HEIGHT> cells_;
};

template <std::size_t WIDTH, std::size_t HEIGHT, std::size_t CELL_SIZE>
class GameManager
{
public:
    void Update()
    {
    }

    void Render(sf::RenderWindow &window) const
    {
        board_.Render(window);
    }

private:
    GameBoard<WIDTH, HEIGHT, CELL_SIZE> board_;
};

int main(int, char **)
{
    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project");

    GameManager<50, 40, 20> gameManager;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        gameManager.Update();
        gameManager.Render(window);
        window.display();
    }
}
