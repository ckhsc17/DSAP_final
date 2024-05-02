#include <iostream>
#include <memory>
#include <random>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class GameRenderer
{
public:
    GameRenderer(sf::RenderWindow *window, sf::Font *font) : window_(window), font_(font) {}

    sf::Vector2i GetMousePosition() const {
        return sf::Mouse::getPosition(*window_);
    }
    
    void Clear() {
        window_->clear(sf::Color::Black);
    }

    void DrawBorder(sf::Vector2f topLeft)
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE - 2 * PADDING_SIZE, CELL_SIZE - 2 * PADDING_SIZE));
        rectangle.setOutlineColor(sf::Color(60, 60, 60));
        rectangle.setOutlineThickness(PADDING_SIZE);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(PADDING_SIZE, PADDING_SIZE));
        window_->draw(rectangle);
    }
    void Draw(const sf::Shape &s) 
    {
        window_->draw(s);
    }
    void Draw(std::string str, unsigned int characterSize, sf::Color color, sf::Vector2f topLeft) 
    {
        sf::Text text;
        text.setFont(*font_);
        text.setString(str);
        text.setCharacterSize(characterSize);
        text.setFillColor(color);

        sf::FloatRect rect = text.getLocalBounds();
        text.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        text.setPosition(
            topLeft.x + CELL_SIZE / 2.0f,
            topLeft.y + CELL_SIZE / 2.0f);
        window_->draw(text);
    }
private:
    sf::RenderWindow *window_;
    sf::Font *font_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class ICell
{
public:
    virtual void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class IForegroundCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    virtual bool CanRemove(const IForegroundCell &) const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class IBackgroundCell : public ICell<CELL_SIZE, PADDING_SIZE>
{
public:
    virtual bool CanBuild() const = 0;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class WallBackgroundCell : public IBackgroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanBuild() const override
    {
        return false;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(60, 60, 60));
        rectangle.setPosition(topLeft);
        renderer.Draw(rectangle);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class CollectionCenterCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return false;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(150, 150, 150));
        rectangle.setPosition(topLeft);
        renderer.Draw(rectangle);
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
    std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> GetBackground() const
    {
        return background_;
    }
    void SetForegrund(std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        foreground_ = value;
    }
    void SetBackground(std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>> value)
    {
        background_ = value;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        if (foreground_)
        {
            foreground_->Render(renderer, topLeft);
        }
        else if (background_)
        {
            background_->Render(renderer, topLeft);
        }
        else
        {
            renderer.DrawBorder(topLeft);
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
    using Renderer = GameRenderer<CELL_SIZE, PADDING_SIZE>;
    using ForegroundCellPointer = std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>>;

    void SetForeground(std::size_t i, std::size_t j,  ForegroundCellPointer value)
    {
        auto &cellStack = cellStacks_[i][j];

        const auto background = cellStack.GetBackground();

        if (background != nullptr && !background->CanBuild())
        {
            return;
        }

        const auto &previousForeground = cellStack.GetForeground();

        if (previousForeground == nullptr || previousForeground->CanRemove(*value))
        {
            cellStack.SetForegrund(value);
        }
    }

    void SetBackground(std::size_t i, std::size_t j, BackgroundCellPointer value)
    {
        cellStacks_[i][j].SetBackground(value);
    }

    sf::Vector2i GetMouseCellPosition(const Renderer& renderer) const
    {
        const sf::Vector2i mousePosition = renderer.GetMousePosition();
        const sf::Vector2i relatedMousePosition = mousePosition - sf::Vector2i(LEFT, TOP);
        return sf::Vector2i(relatedMousePosition.x / CELL_SIZE, relatedMousePosition.y / CELL_SIZE);
    }

    bool IsMouseInsideBoard(const Renderer &renderer) const
    {
        sf::Vector2i mousePosition = GetMouseCellPosition(renderer);
        return mousePosition.x >= 0 && mousePosition.x < WIDTH &&
               mousePosition.y >= 0 && mousePosition.y < HEIGHT;
    }

    void Render(Renderer& renderer) const
    {
        for (std::size_t i = 0; i < HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < WIDTH; ++j)
            {
                cellStacks_[i][j].Render(
                    renderer,
                    sf::Vector2f(LEFT + j * CELL_SIZE, TOP + i * CELL_SIZE));
            }
        }

        if (!IsMouseInsideBoard(renderer))
        {
            return;
        }

        sf::Vector2i mousePosition = GetMouseCellPosition(renderer);

        renderer.Draw(
            "(" + std::to_string(mousePosition.x) + ", " + std::to_string(mousePosition.y) + ")", 
            20,
            sf::Color::White,
            sf::Vector2f(50, 20));
    }

private:
    std::array<std::array<CellStack<CELL_SIZE, PADDING_SIZE>, WIDTH>, HEIGHT> cellStacks_;
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class NumberBackgroundCell : public IBackgroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    NumberBackgroundCell(int number) : number_(number) {}

    bool CanBuild() const override
    {
        return true;
    }

    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        std::mt19937 gen(number_);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        renderer.Draw(std::to_string(number_), CELL_SIZE * 0.8f, color, topLeft);
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
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE - 2 * offset));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(0, offset));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(CELL_SIZE / 2, CELL_SIZE / 2));
        arrow.setPoint(1, sf::Vector2f(CELL_SIZE / 2 - 2 * offset, offset));
        arrow.setPoint(2, sf::Vector2f(CELL_SIZE / 2, offset));
        arrow.setPoint(3, sf::Vector2f(CELL_SIZE / 2 + 2 * offset, CELL_SIZE / 2));
        arrow.setPoint(4, sf::Vector2f(CELL_SIZE / 2, CELL_SIZE - offset));
        arrow.setPoint(5, sf::Vector2f(CELL_SIZE / 2 - 2 * offset, CELL_SIZE - offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft);
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class BottomToTopConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE - 2 * offset, CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(offset, 0));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(270);
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2, CELL_SIZE / 2));
        arrow.setFillColor(sf::Color(60, 60, 60));
        renderer.Draw(arrow);
    }
};

template <std::size_t CELL_SIZE, std::size_t PADDING_SIZE>
class TopToBottomConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE - 2 * offset, CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(offset, 0));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(CELL_SIZE / 2, CELL_SIZE / 2));
        arrow.setPoint(1, sf::Vector2f(offset, CELL_SIZE / 2 - 2 * offset));
        arrow.setPoint(2, sf::Vector2f(offset, CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(CELL_SIZE / 2, CELL_SIZE / 2 + 2 * offset));
        arrow.setPoint(4, sf::Vector2f(CELL_SIZE - offset, CELL_SIZE / 2));
        arrow.setPoint(5, sf::Vector2f(CELL_SIZE - offset, CELL_SIZE / 2 - 2 * offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft);
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class BottomToRightConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, CELL_SIZE));
        shape.setPoint(1, sf::Vector2f(offset, 4 * offset));
        shape.setPoint(2, sf::Vector2f(4 * offset, offset));
        shape.setPoint(3, sf::Vector2f(CELL_SIZE, offset));
        shape.setPoint(4, sf::Vector2f(CELL_SIZE, CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(CELL_SIZE - offset, CELL_SIZE));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(315);
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2 + 0.5 * offset, CELL_SIZE / 2 + 0.5 * offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class LeftToBottomConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(0, offset));
        shape.setPoint(1, sf::Vector2f(CELL_SIZE - 4 * offset, offset));
        shape.setPoint(2, sf::Vector2f(CELL_SIZE - offset, 4 * offset));
        shape.setPoint(3, sf::Vector2f(CELL_SIZE - offset, CELL_SIZE));
        shape.setPoint(4, sf::Vector2f(offset, CELL_SIZE));
        shape.setPoint(5, sf::Vector2f(0, CELL_SIZE - offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(45);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2 - 0.5 * offset, CELL_SIZE / 2 + 0.5 * offset));
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class TopToLeftConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, 0));
        shape.setPoint(1, sf::Vector2f(CELL_SIZE - offset, 0));
        shape.setPoint(2, sf::Vector2f(CELL_SIZE - offset, CELL_SIZE - 4 * offset));
        shape.setPoint(3, sf::Vector2f(CELL_SIZE - 4 * offset, CELL_SIZE - offset));
        shape.setPoint(4, sf::Vector2f(0, CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(0, offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(135);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2 - 0.5 * offset, CELL_SIZE / 2 - 0.5 * offset));
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class RightToLeftConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }
    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);
        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE - 2 * offset));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(0, offset));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(180);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2, CELL_SIZE / 2));
        renderer.Draw(arrow);
    }
};

template <int CELL_SIZE, std::size_t PADDING_SIZE>
class RightToTopConveyorCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }

    void Render(GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, 0));
        shape.setPoint(1, sf::Vector2f(CELL_SIZE - offset, 0));
        shape.setPoint(2, sf::Vector2f(CELL_SIZE, offset));
        shape.setPoint(3, sf::Vector2f(CELL_SIZE, CELL_SIZE - offset));
        shape.setPoint(4, sf::Vector2f(4 * offset, CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(offset, CELL_SIZE - 4 * offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, CELL_SIZE / 2 - offset));
        arrow.setRotation(225);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(CELL_SIZE / 2 + 0.5 * offset, CELL_SIZE / 2 - 0.5 * offset));
        renderer.Draw(arrow);
    }
};

template <
    std::size_t CELL_SIZE,
    std::size_t PADDING_SIZE>
class MiningMachineCell : public IForegroundCell<CELL_SIZE, PADDING_SIZE>
{
public:
    bool CanRemove(const IForegroundCell<CELL_SIZE, PADDING_SIZE> &) const override
    {
        return true;
    }

    void Render(
        GameRenderer<CELL_SIZE, PADDING_SIZE>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        sf::RectangleShape rectangle(sf::Vector2f(CELL_SIZE, CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft);

        renderer.Draw(rectangle);
    }
private:
    std::shared_ptr<GameRenderer<CELL_SIZE, PADDING_SIZE>> renderer_;
};

enum class PlayerAction
{
    BuildMiningMachine,
    BuildLeftToRightConveyor,
    BuildLeftToBottomConveyor,
    BuildTopToBottomConveyor,
    BuildTopToLeftConveyor,
    BuildRightToLeftConveyor,
    BuildRightToTopConveyor,
    BuildBottomToTopConveyor,
    BuildBottomToRightConveyor,
    Clear,
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
    using Renderer = GameRenderer<CELL_SIZE, PADDING_SIZE>;
    using ForegroundCellPointer = std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<CELL_SIZE, PADDING_SIZE>>;

    GameManager(sf::RenderWindow *window, sf::Font *font) : board_(), renderer_(window, font)
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

    bool IsMouseInsideBoard()
    {
        return board_.IsMouseInsideBoard(renderer_);
    }

    void Exec(PlayerAction playerAction)
    {
        sf::Vector2i mouseCellPosition = board_.GetMouseCellPosition(renderer_);
        std::shared_ptr<IForegroundCell<CELL_SIZE, PADDING_SIZE>> nextForegroundCell = nullptr;
        switch (playerAction)
        {
        case PlayerAction::BuildMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildLeftToRightConveyor:
            nextForegroundCell = std::make_shared<LeftToRightConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildLeftToBottomConveyor:
            nextForegroundCell = std::make_shared<LeftToBottomConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildTopToBottomConveyor:
            nextForegroundCell = std::make_shared<TopToBottomConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildTopToLeftConveyor:
            nextForegroundCell = std::make_shared<TopToLeftConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildRightToLeftConveyor:
            nextForegroundCell = std::make_shared<RightToLeftConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildRightToTopConveyor:
            nextForegroundCell = std::make_shared<RightToTopConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildBottomToTopConveyor:
            nextForegroundCell = std::make_shared<BottomToTopConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::BuildBottomToRightConveyor:
            nextForegroundCell = std::make_shared<BottomToRightConveyorCell<CELL_SIZE, PADDING_SIZE>>();
            break;
        case PlayerAction::Clear:
            nextForegroundCell = nullptr;
            break;
        }
        board_.SetForeground(mouseCellPosition.y, mouseCellPosition.x, nextForegroundCell);
    }

    void Update()
    {
    }

    void Render()
    {
        renderer_.Clear();
        board_.Render(renderer_);
    }

private:
    GameBoard<WIDTH, HEIGHT, CELL_SIZE, PADDING_SIZE> board_;
    GameRenderer<CELL_SIZE, PADDING_SIZE> renderer_;
};

int main(int, char **)
{
    PlayerAction playerAction = PlayerAction::BuildLeftToRightConveyor;

    sf::Font font;

    font.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project");

    GameManager<62, 36, 20> gameManager(&window, &font);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (gameManager.IsMouseInsideBoard())
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        gameManager.Exec(playerAction);
                    }
                }
            }

            if (event.type == sf::Event::KeyReleased)
            {
                switch (event.key.code)
                {
                case sf::Keyboard::Tilde:
                    playerAction = PlayerAction::Clear;
                    break;
                case sf::Keyboard::M:
                    playerAction = PlayerAction::BuildMiningMachine;
                    break;
                case sf::Keyboard::Num1:
                    playerAction = PlayerAction::BuildLeftToRightConveyor;
                    break;
                case sf::Keyboard::Num2:
                    playerAction = PlayerAction::BuildLeftToBottomConveyor;
                    break;
                case sf::Keyboard::Num3:
                    playerAction = PlayerAction::BuildTopToBottomConveyor;
                    break;
                case sf::Keyboard::Num4:
                    playerAction = PlayerAction::BuildTopToLeftConveyor;
                    break;
                case sf::Keyboard::Num5:
                    playerAction = PlayerAction::BuildRightToLeftConveyor;
                    break;
                case sf::Keyboard::Num6:
                    playerAction = PlayerAction::BuildRightToTopConveyor;
                    break;
                case sf::Keyboard::Num7:
                    playerAction = PlayerAction::BuildBottomToTopConveyor;
                    break;
                case sf::Keyboard::Num8:
                    playerAction = PlayerAction::BuildBottomToRightConveyor;
                    break;
                default:
                    break;
                }
            }
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        gameManager.Update();
        gameManager.Render();
        window.display();
    }
}
