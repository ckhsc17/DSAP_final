#include <iostream>
#include <memory>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class ICell
{
public:
    virtual void Render(sf::RenderWindow &window, sf::Vector2f topLeft) const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class EmptyCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    void Render(sf::RenderWindow &window, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setOutlineColor(sf::Color(60, 60, 60));
        rectangle.setOutlineThickness(PADDING_SIZE);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(PADDING_SIZE, PADDING_SIZE));
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class WallCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    void Render(sf::RenderWindow &window, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(60, 60, 60));
        rectangle.setPosition(topLeft);
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class GoalCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    void Render(sf::RenderWindow &window, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(200, 200, 200));
        rectangle.setPosition(topLeft);
        window.draw(rectangle);
    }
};

template <
    std::size_t WIDTH,
    std::size_t HEIGHT,
    std::size_t CELL_SIZE,
    std::size_t PADDING_SIZE>
class GameBoard
{
public:
    void SetCell(std::size_t i, std::size_t j, std::shared_ptr<ICell<CELL_SIZE, PADDING_SIZE>> value)
    {
        cells_[i][j] = value;
    }

    void Render(sf::RenderWindow &window) const
    {
        const sf::Vector2f topLeft(20.f, 80.f);

        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH; ++j)
            {
                cells_[i][j]->Render(
                    window,
                    topLeft + sf::Vector2f(j * CELL_SIZE, i * CELL_SIZE));
            }
        }
    }

private:
    std::array<std::array<std::shared_ptr<ICell<CELL_SIZE, PADDING_SIZE>>, WIDTH>, HEIGHT> cells_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class NumberCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    NumberCell(int number) : number_(number) {}

    void Render(sf::RenderWindow &window, sf::Vector2f topLeft) const override
    {
        std::mt19937 gen(number_);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        sf::RectangleShape rectangle(
            sf::Vector2f(CELL_SIZE - 2 * PADDING_SIZE, CELL_SIZE - 2 * PADDING_SIZE));
        rectangle.setOutlineColor(color);
        rectangle.setOutlineThickness(PADDING_SIZE);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(PADDING_SIZE, PADDING_SIZE));
        window.draw(rectangle);

        sf::Font font;
        font.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(number_));
        text.setCharacterSize(CELL_SIZE * 0.8f);
        text.setFillColor(color);

        sf::FloatRect rect = text.getLocalBounds();
        text.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        text.setPosition(
            rectangle.getPosition().x + rectangle.getSize().x / 2.0f,
            rectangle.getPosition().y + rectangle.getSize().y / 2.0f);
        window.draw(text);
    }

private:
    int number_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class CellFactory
{
public:
    std::shared_ptr<ICell<CELL_SIZE, PADDING_SIZE>> Create()
    {
        int val = gen_() % 40;

        std::vector<int> numbers = {1, 2, 3, 5, 7, 11};

        if (val == 0)
        {
            return std::make_shared<WallCell<CELL_SIZE, PADDING_SIZE>>();
        }
        else if (std::count(numbers.begin(), numbers.end(), val))
        {
            return std::make_shared<NumberCell<CELL_SIZE, PADDING_SIZE>>(val);
        }
        else
        {
            return std::make_shared<EmptyCell<CELL_SIZE, PADDING_SIZE>>();
        }
    }

private:
    std::mt19937 gen_;
};

template <
    std::size_t WIDTH,
    std::size_t HEIGHT,
    std::size_t CELL_SIZE,
    std::size_t GOAL_SIZE = 4,
    std::size_t PADDING_SIZE = 1>
class GameManager
{
public:
    GameManager()
    {
        static_assert(WIDTH % 2 == 0, "WIDTH must be even");

        CellFactory<CELL_SIZE, PADDING_SIZE> cellFactory;

        sf::Rect<int> leftGoal(
            WIDTH / 4 - GOAL_SIZE / 2,
            HEIGHT / 2 - GOAL_SIZE / 2,
            GOAL_SIZE,
            GOAL_SIZE);

        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH / 2; ++j)
            {
                std::shared_ptr<ICell<CELL_SIZE, PADDING_SIZE>> cell;
                if (leftGoal.contains(j, i))
                {
                    cell = std::make_shared<GoalCell<CELL_SIZE, PADDING_SIZE>>();
                }
                else
                {
                    cell = cellFactory.Create();
                }
                board_.SetCell(i, j, cell);
                board_.SetCell(i, WIDTH - 1 - j, cell);
            }
        }
    }

    void Update()
    {
    }

    void Render(sf::RenderWindow &window) const
    {
        board_.Render(window);
    }

private:
    GameBoard<WIDTH, HEIGHT, CELL_SIZE, PADDING_SIZE> board_;
};

int main(int, char **)
{
    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project");

    GameManager<62, 36, 20> gameManager;

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
