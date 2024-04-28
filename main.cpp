#include <iostream>
#include <memory>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class CellRenderer
{
public:
    static void RenderBorder(sf::RenderWindow &window, sf::Vector2f topLeft)
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE - 2 * PADDING_SIZE, CELL_SIZE - 2 * PADDING_SIZE));
        rectangle.setOutlineColor(sf::Color(60, 60, 60));
        rectangle.setOutlineThickness(PADDING_SIZE);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(PADDING_SIZE, PADDING_SIZE));
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class ICell
{
public:
    virtual void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class IForegroundCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    virtual bool CanModify() const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class IBackgroundCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class WallBackgroundCell : public IBackgroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(60, 60, 60));
        rectangle.setPosition(topLeft);
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class CollectionCenterCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanModify() const override
    {
        return false;
    }
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(150, 150, 150));
        rectangle.setPosition(topLeft);
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class CellStack : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> GetForeground() const
    {
        return foreground_;
    }
    void SetForegrund(std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        foreground_ = value;
    }
    void SetBackground(std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        background_ = value;
    }
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        if (foreground_)
        {
            foreground_->Render(window, font, topLeft);
        }
        else if (background_)
        {
            background_->Render(window, font, topLeft);
        }
        else
        {
            CellRenderer<CELL_SIZE, PADDING_SIZE>::RenderBorder(window, topLeft);
        }
    }

private:
    std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> foreground_;
    std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> background_;
};

template <
    std::size_t WIDTH,
    std::size_t HEIGHT,
    std::size_t CELL_SIZE,
    std::size_t PADDING_SIZE,
    int TOP = 80,
    int LEFT = 20>
class GameBoard
{
public:
    bool CanModify(sf::Vector2i cellPosition) const {
        const auto& foregroundCell = cellStacks_[cellPosition.y][cellPosition.x].GetForeground();
        return foregroundCell == nullptr || foregroundCell->CanModify();
    }

    void SetForeground(
        std::size_t i,
        std::size_t j,
        std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        cellStacks_[i][j].SetForegrund(value);
    }
    void SetBackground(
        std::size_t i,
        std::size_t j,
        std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        cellStacks_[i][j].SetBackground(value);
    }

    sf::Vector2i GetMouseCellPosition(const sf::RenderWindow &window) const
    {
        const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        const sf::Vector2i relatedMousePosition = mousePosition - sf::Vector2i(LEFT, TOP);
        return sf::Vector2i(relatedMousePosition.x / CELL_SIZE, relatedMousePosition.y / CELL_SIZE);
    }

    bool IsMouseInsideBoard(const sf::RenderWindow &window) const
    {
        sf::Vector2i mousePosition = GetMouseCellPosition(window);
        return mousePosition.x >= 0 && mousePosition.x < WIDTH &&
               mousePosition.y >= 0 && mousePosition.y < HEIGHT;
    }

    void Render(sf::RenderWindow &window, const sf::Font &font) const
    {
        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH; ++j)
            {
                cellStacks_[i][j].Render(
                    window,
                    font,
                    sf::Vector2f(LEFT + j * CELL_SIZE, TOP + i * CELL_SIZE));
            }
        }

        if (!IsMouseInsideBoard(window))
        {
            return;
        }

        sf::Vector2i mousePosition = GetMouseCellPosition(window);

        sf::Text currentCellPositionText;
        currentCellPositionText.setString("(" + std::to_string(mousePosition.x) + ", " + std::to_string(mousePosition.y) + ")");
        currentCellPositionText.setFont(font);
        currentCellPositionText.setCharacterSize(20);
        currentCellPositionText.setPosition(20, 20);
        window.draw(currentCellPositionText);
    }

private:
    std::array<std::array<CellStack<CELL_SIZE, PADDING_SIZE>, WIDTH>, HEIGHT> cellStacks_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class NumberBackgroundCell : public IBackgroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    NumberBackgroundCell(int number) : number_(number) {}

    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        CellRenderer<CELL_SIZE, PADDING_SIZE>::RenderBorder(window, topLeft);

        std::mt19937 gen(number_);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(number_));
        text.setCharacterSize(CELL_SIZE * 0.8f);
        text.setFillColor(color);

        sf::FloatRect rect = text.getLocalBounds();
        text.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        text.setPosition(
            topLeft.x + CELL_SIZE / 2.0f,
            topLeft.y + CELL_SIZE / 2.0f);
        window.draw(text);
    }

private:
    int number_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class BackgroundCellFactory
{
public:
    std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> Create()
    {
        int val = gen_() % 40;

        std::vector<int> numbers = {1, 2, 3, 5, 7, 11};

        if (val == 0)
        {
            return std::make_shared<WallBackgroundCell<CELL_SIZE, PADDING_SIZE>>();
        }
        else if (std::count(numbers.begin(), numbers.end(), val))
        {
            return std::make_shared<NumberBackgroundCell<CELL_SIZE, PADDING_SIZE>>(val);
        }
        else
        {
            return nullptr;
        }
    }

private:
    std::mt19937 gen_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class LeftToRightConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanModify() const override
    {
        return true;
    }
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        CellRenderer<CELL_SIZE, PADDING_SIZE>::RenderBorder(window, topLeft);
        int yOffset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE - 2 * yOffset));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(0, yOffset));
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class BottomToUpConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanModify() const override
    {
        return true;
    }
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        CellRenderer<CELL_SIZE, PADDING_SIZE>::RenderBorder(window, topLeft);

        int xOffset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE - 2 * xOffset, CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(xOffset, 0));
        window.draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class BottomToRightConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    void Render(sf::RenderWindow &window, const sf::Font &font, sf::Vector2f topLeft) const override
    {
        CellRenderer<CELL_SIZE, PADDING_SIZE>::RenderBorder(window, topLeft);

        int xOffset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(xOffset, CELL_SIZE));
        shape.setPoint(1, sf::Vector2f(xOffset, 4 * xOffset));
        shape.setPoint(2, sf::Vector2f(4 * xOffset, xOffset));
        shape.setPoint(3, sf::Vector2f(CELL_SIZE, xOffset));
        shape.setPoint(4, sf::Vector2f(CELL_SIZE, CELL_SIZE - xOffset));
        shape.setPoint(5, sf::Vector2f(CELL_SIZE - xOffset, CELL_SIZE));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        window.draw(shape);
    }
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

        BackgroundCellFactory<CELL_SIZE, PADDING_SIZE> backgroundCellFactory;

        // Setup Background
        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH / 2; ++j)
            {
                std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> backgroundCell =
                    backgroundCellFactory.Create();
                board_.SetBackground(i, j, backgroundCell);
                board_.SetBackground(i, WIDTH - 1 - j, backgroundCell);
            }
        }

        sf::Rect<int> leftCollectionCenter(
            WIDTH / 4 - GOAL_SIZE / 2,
            HEIGHT / 2 - GOAL_SIZE / 2,
            GOAL_SIZE,
            GOAL_SIZE);

        auto leftCollectionCenterCell = std::make_shared<CollectionCenterCell<CELL_SIZE, PADDING_SIZE>>();

        for (int i = leftCollectionCenter.top; i < leftCollectionCenter.top + leftCollectionCenter.height; ++i)
        {
            for (int j = leftCollectionCenter.left; j < leftCollectionCenter.left + leftCollectionCenter.width; ++j)
            {
                board_.SetForeground(i, j, leftCollectionCenterCell);
            }
        }
    }

    bool IsMouseInsideBoard(const sf::RenderWindow &window) {
        return board_.IsMouseInsideBoard(window);
    }

    void Build(const sf::RenderWindow &window)
    {
        sf::Vector2i mouseCellPosition = board_.GetMouseCellPosition(window);
        if (board_.CanModify(mouseCellPosition)) {
            board_.SetForeground(mouseCellPosition.y, mouseCellPosition.x, std::make_shared<LeftToRightConveyorCell<CELL_SIZE, PADDING_SIZE>>());
        }
    }

    void Clear(const sf::RenderWindow& window) 
    {
        sf::Vector2i mouseCellPosition = board_.GetMouseCellPosition(window);
        
        if (board_.CanModify(mouseCellPosition))
        {
            board_.SetForeground(mouseCellPosition.y, mouseCellPosition.x, nullptr);
        }
    }

    void Update()
    {
    }

    void Render(sf::RenderWindow &window, const sf::Font &font) const
    {
        window.clear(sf::Color::Black);
        board_.Render(window, font);
    }

private:
    GameBoard<WIDTH, HEIGHT, CELL_SIZE, PADDING_SIZE> board_;
};

int main(int, char **)
{
    sf::Font font;
    font.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project");

    GameManager<62, 36, 20> gameManager;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (gameManager.IsMouseInsideBoard(window)) 
                {
                    if (event.mouseButton.button == sf::Mouse::Left) 
                    {
                        gameManager.Build(window);
                    } 
                    else 
                    {
                        gameManager.Clear(window);
                    }
                }
            }
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        gameManager.Update();
        gameManager.Render(window, font);
        window.display();
    }
}
