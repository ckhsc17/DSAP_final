#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

enum class Direction {
    kTop = 0,
    kRight = 1,
    kBottom = 2,
    kLeft = 3
};

template <typename TGameConfig> 
class GameBoard;

template <typename TGameConfig>
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
    
    void Display() {
        window_->display();
    }

    void DrawBorder(sf::Vector2f topLeft)
    {
        sf::RectangleShape rectangle(
            sf::Vector2f(
                TGameConfig::CELL_SIZE - 2 * TGameConfig::PADDING_SIZE,
                TGameConfig::CELL_SIZE - 2 * TGameConfig::PADDING_SIZE));

        rectangle.setOutlineColor(sf::Color(60, 60, 60));
        rectangle.setOutlineThickness(TGameConfig::PADDING_SIZE);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(TGameConfig::PADDING_SIZE, TGameConfig::PADDING_SIZE));
        window_->draw(rectangle);
    }

    void Draw(const sf::Shape &s) 
    {
        window_->draw(s);
    }
    
    void Draw(
        std::string str, 
        unsigned int characterSize, 
        sf::Color color, 
        sf::Vector2f topLeft,
        Direction direction = Direction::kTop) 
    {
        sf::Text text;
        text.setFont(*font_);
        text.setString(str);
        text.setCharacterSize(characterSize);
        text.setFillColor(color);

        sf::FloatRect rect = text.getLocalBounds();
        text.setOrigin(rect.left + rect.width / 2.0f, rect.top + rect.height / 2.0f);
        text.setPosition(
            topLeft.x + TGameConfig::CELL_SIZE / 2.0f,
            topLeft.y + TGameConfig::CELL_SIZE / 2.0f);
        text.setRotation(90 * static_cast<int>(direction));
        window_->draw(text);
    }
private:
    sf::RenderWindow *window_;
    sf::Font *font_;
};

template <typename TGameConfig>
class ICell
{
};


template <typename TGameConfig>
class IBackgroundCell : public ICell<TGameConfig>
{
public:
    virtual bool CanBuild() const = 0;
    virtual void Render(GameRenderer<TGameConfig>& renderer, sf::Vector2f topLeft) const = 0;
};

template <typename TGameConfig>
class IForegroundCell : public ICell<TGameConfig>
{
public:
    virtual bool CanRemove(const IForegroundCell &) const = 0;

    virtual void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) = 0;

    virtual void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft, 
        const IBackgroundCell<TGameConfig>& backgroundCell) const = 0;

};


template <typename TGameConfig>
class IConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    virtual void EnqueueProduct(int number) = 0;
};

template <typename TGameConfig>
class WallBackgroundCell : public IBackgroundCell<TGameConfig>
{
public:
    bool CanBuild() const override
    {
        return false;
    }
    void Render(GameRenderer<TGameConfig>& renderer, sf::Vector2f topLeft) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE));
        rectangle.setFillColor(sf::Color(60, 60, 60));
        rectangle.setPosition(topLeft);
        renderer.Draw(rectangle);
    }
};

template <typename TGameConfig>
class CollectionCenterCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return false;
    }
    
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }

    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE));
        rectangle.setFillColor(sf::Color(150, 150, 150));
        rectangle.setPosition(topLeft);
        renderer.Draw(rectangle);
    }
};

template <typename TGameConfig>
class CellStack : public ICell<TGameConfig>
{
public:
    std::shared_ptr<IForegroundCell<TGameConfig>> GetForeground() const
    {
        return foreground_;
    }
    std::shared_ptr<IBackgroundCell<TGameConfig>> GetBackground() const
    {
        return background_;
    }
    void SetForegrund(const std::shared_ptr<IForegroundCell<TGameConfig>>& value)
    {
        foreground_ = value;
    }
    void SetBackground(const std::shared_ptr<IBackgroundCell<TGameConfig>>& value)
    {
        background_ = value;
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft) const
    {

        if (foreground_)
        {
            foreground_->Render(renderer, topLeft, *background_.get());
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
    std::shared_ptr<IForegroundCell<TGameConfig>> foreground_;
    std::shared_ptr<IBackgroundCell<TGameConfig>> background_;
};

template <typename TGameConfig>
class GameBoard
{
public:
    using Renderer = GameRenderer<TGameConfig>;
    using ForegroundCellPointer = std::shared_ptr<IForegroundCell<TGameConfig>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<TGameConfig>>;

    ForegroundCellPointer GetForeground(std::size_t i, std::size_t j) const
    {
        return cellStacks_[i][j].GetForeground();
    }

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
        const sf::Vector2i relatedMousePosition = 
        mousePosition - sf::Vector2i(TGameConfig::LEFT, TGameConfig::TOP);
        return sf::Vector2i(
            relatedMousePosition.x / TGameConfig::CELL_SIZE, 
            relatedMousePosition.y / TGameConfig::CELL_SIZE);
    }

    bool IsMouseInsideBoard(const Renderer &renderer) const
    {
        sf::Vector2i mousePosition = GetMouseCellPosition(renderer);
        return mousePosition.x >= 0 && mousePosition.x < TGameConfig::BOARD_WIDTH &&
               mousePosition.y >= 0 && mousePosition.y < TGameConfig::BOARD_HEIGHT;
    }

    void Update() 
    {
        for (std::size_t i = 0; i < TGameConfig::BOARD_HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::BOARD_WIDTH; ++j)
            {
                auto &cellStack = cellStacks_[i][j];
                auto foreground = cellStack.GetForeground();
                if (foreground != nullptr)
                {
                    foreground->Update(i, j, *this);
                }
            }
        }
    }

    void Render(Renderer& renderer) const
    {
        for (std::size_t i = 0; i < TGameConfig::BOARD_HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::BOARD_WIDTH; ++j)
            {
                cellStacks_[i][j].Render(
                    renderer,
                    sf::Vector2f(
                        TGameConfig::LEFT + j * TGameConfig::CELL_SIZE, 
                        TGameConfig::TOP + i * TGameConfig::CELL_SIZE));
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
    std::array<std::array<CellStack<TGameConfig>, TGameConfig::BOARD_WIDTH>, TGameConfig::BOARD_HEIGHT> cellStacks_;
};

template <typename TGameConfig>
class NumberCell : public IBackgroundCell<TGameConfig>
{
public:
    NumberCell(int number) : number_(number) {}

    int GetNumber() const
    {
        return number_;
    }
    
    bool CanBuild() const override
    {
        return true;
    }

    void Render(GameRenderer<TGameConfig>& renderer, sf::Vector2f topLeft) const override
    {
        renderer.DrawBorder(topLeft);

        std::mt19937 gen(number_);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        renderer.Draw(std::to_string(number_), TGameConfig::CELL_SIZE * 0.75f, color, topLeft);
    }

private:
    int number_;
};

template <typename TGameConfig>
class BackgroundCellFactory
{
public:
    std::shared_ptr<IBackgroundCell<TGameConfig>> Create()
    {
        int val = gen_() % 40;

        std::vector<int> numbers = {1, 2, 3, 5, 7, 11};

        if (val == 0)
        {
            return std::make_shared<WallBackgroundCell<TGameConfig>>();
        }
        else if (std::count(numbers.begin(), numbers.end(), val))
        {
            return std::make_shared<NumberCell<TGameConfig>>(val);
        }
        else
        {
            return nullptr;
        }
    }

private:
    std::mt19937 gen_;
};

template <typename TGameConfig>
class LeftToRightConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(
            sf::Vector2f(
                TGameConfig::CELL_SIZE, 
                TGameConfig::CELL_SIZE - 2 * offset));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(0, offset));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(1, sf::Vector2f(TGameConfig::CELL_SIZE / 2 - 2 * offset, offset));
        arrow.setPoint(2, sf::Vector2f(TGameConfig::CELL_SIZE / 2, offset));
        arrow.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE / 2 + 2 * offset, TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(4, sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE - offset));
        arrow.setPoint(5, sf::Vector2f(TGameConfig::CELL_SIZE / 2 - 2 * offset, TGameConfig::CELL_SIZE - offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft);
        renderer.Draw(arrow);
    }
};

template <typename TGameConfig>
class BottomToTopConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE - 2 * offset, TGameConfig::CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(offset, 0));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(270);
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE / 2));
        arrow.setFillColor(sf::Color(60, 60, 60));
        renderer.Draw(arrow);
    }
};

template <typename TGameConfig>
class TopToBottomConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {

    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE - 2 * offset, TGameConfig::CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(offset, 0));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(1, sf::Vector2f(offset, TGameConfig::CELL_SIZE / 2 - 2 * offset));
        arrow.setPoint(2, sf::Vector2f(offset, TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE / 2 + 2 * offset));
        arrow.setPoint(4, sf::Vector2f(TGameConfig::CELL_SIZE - offset, TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(5, sf::Vector2f(TGameConfig::CELL_SIZE - offset, TGameConfig::CELL_SIZE / 2 - 2 * offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft);
        renderer.Draw(arrow);
    }
};

template <typename TGameConfig>
class BottomToRightConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, TGameConfig::CELL_SIZE));
        shape.setPoint(1, sf::Vector2f(offset, 4 * offset));
        shape.setPoint(2, sf::Vector2f(4 * offset, offset));
        shape.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE, offset));
        shape.setPoint(4, sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(TGameConfig::CELL_SIZE - offset, TGameConfig::CELL_SIZE));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(315);
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2 + 0.5 * offset, TGameConfig::CELL_SIZE / 2 + 0.5 * offset));
        arrow.setFillColor(sf::Color(60, 60, 60));
        renderer.Draw(arrow);
    }
};

template <typename TGameConfig>
class LeftToBottomConveyorCell : public IConveyorCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void EnqueueProduct(int number) override {
        products_.push(number);
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(0, offset));
        shape.setPoint(1, sf::Vector2f(TGameConfig::CELL_SIZE - 4 * offset, offset));
        shape.setPoint(2, sf::Vector2f(TGameConfig::CELL_SIZE - offset, 4 * offset));
        shape.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE - offset, TGameConfig::CELL_SIZE));
        shape.setPoint(4, sf::Vector2f(offset, TGameConfig::CELL_SIZE));
        shape.setPoint(5, sf::Vector2f(0, TGameConfig::CELL_SIZE - offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(45);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2 - 0.5 * offset, TGameConfig::CELL_SIZE / 2 + 0.5 * offset));
        renderer.Draw(arrow);
    }
private:
    std::queue<int> products_;
};

template <typename TGameConfig>
class TopToLeftConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, 0));
        shape.setPoint(1, sf::Vector2f(TGameConfig::CELL_SIZE - offset, 0));
        shape.setPoint(2, sf::Vector2f(TGameConfig::CELL_SIZE - offset, TGameConfig::CELL_SIZE - 4 * offset));
        shape.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE - 4 * offset, TGameConfig::CELL_SIZE - offset));
        shape.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(0, offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(135);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2 - 0.5 * offset, TGameConfig::CELL_SIZE / 2 - 0.5 * offset));
        renderer.Draw(arrow);
    }
};



template <typename TGameConfig>
class RightToLeftConveyorCell : public IConveyorCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void EnqueueProduct(int number) override {
        products_.push(number);
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);
        int offset = 2;
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE - 2 * offset));
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(topLeft + sf::Vector2f(0, offset));
        renderer.Draw(rectangle);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(180);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2, TGameConfig::CELL_SIZE / 2));
        renderer.Draw(arrow);
    }
private:
    std::queue<int> products_;
};

template <typename TGameConfig>
class RightToTopConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board) override
    {
    }
    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        renderer.DrawBorder(topLeft);

        int offset = 2;
        sf::ConvexShape shape(6);
        shape.setPoint(0, sf::Vector2f(offset, 0));
        shape.setPoint(1, sf::Vector2f(TGameConfig::CELL_SIZE - offset, 0));
        shape.setPoint(2, sf::Vector2f(TGameConfig::CELL_SIZE, offset));
        shape.setPoint(3, sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE - offset));
        shape.setPoint(4, sf::Vector2f(4 * offset, TGameConfig::CELL_SIZE - offset));
        shape.setPoint(5, sf::Vector2f(offset, TGameConfig::CELL_SIZE - 4 * offset));
        shape.setFillColor(sf::Color(128, 128, 128));
        shape.setPosition(topLeft);
        renderer.Draw(shape);

        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::CELL_SIZE / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::CELL_SIZE / 2 - offset));
        arrow.setRotation(225);
        arrow.setScale(1.1, 1.1);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(topLeft + sf::Vector2f(TGameConfig::CELL_SIZE / 2 + 0.5 * offset, TGameConfig::CELL_SIZE / 2 - 0.5 * offset));
        renderer.Draw(arrow);
    }
};



template <
    typename TGameConfig,
    Direction DIRECTION,
    std::size_t PRODUCTION_TIME = 100>

class MiningMachineCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove(const IForegroundCell<TGameConfig> &) const override
    {
        return true;
    }

    void Update(
        std::size_t i, std::size_t j, 
        GameBoard<TGameConfig>& board) override
    {
        ++elapsedTime_;
        if (elapsedTime_ >= PRODUCTION_TIME) 
        {
            auto numberCell = 
                dynamic_cast<const NumberCell<TGameConfig>*>(board.GetForeground(i, j).get());

            if (numberCell) 
            {
                std::cout << numberCell->GetNumber() << std::endl;
                switch (DIRECTION) {
                    case Direction::kTop:
                        if (j > 0) {
                            auto target = 
                                dynamic_cast<RightToLeftConveyorCell<TGameConfig>*>(board.GetForeground(i, j - 1).get());
                            if (target) {
                                target->EnqueueProduct(numberCell->GetNumber());
                            }
                        }
                        break;
                    case Direction::kRight:
                        break;
                    case Direction::kBottom:
                        break;
                    case Direction::kLeft:
                        break;
                }
            }
            elapsedTime_ = 0;
        }
    }

    void Render(
        GameRenderer<TGameConfig>& renderer, 
        sf::Vector2f topLeft,
        const IBackgroundCell<TGameConfig>& backgroundCell) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::CELL_SIZE, TGameConfig::CELL_SIZE));
        rectangle.setFillColor(sf::Color(128, 0, 0));
        rectangle.setPosition(topLeft);
        renderer.Draw(rectangle);

        auto numberCell = dynamic_cast<const NumberCell<TGameConfig>*>(&backgroundCell);

        if (numberCell) 
        {
            renderer.Draw(
                std::to_string(numberCell->GetNumber()), 
                TGameConfig::CELL_SIZE * 0.8,
                 sf::Color::White,
                  topLeft,
                  DIRECTION);
        }

    }
private:
    std::size_t elapsedTime_;
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

template <typename TGameConfig>
class GameManager
{
public:
    using Renderer = GameRenderer<TGameConfig>;
    using ForegroundCellPointer = std::shared_ptr<IForegroundCell<TGameConfig>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<TGameConfig>>;

    GameManager(sf::RenderWindow *window, sf::Font *font) : board_(), renderer_(window, font)
    {
        static_assert(TGameConfig::BOARD_WIDTH % 2 == 0, "WIDTH must be even");

        BackgroundCellFactory<TGameConfig> backgroundCellFactory;

        // Setup Background
        for (std::size_t i = 0; i < TGameConfig::BOARD_HEIGHT; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::BOARD_WIDTH / 2; ++j)
            {
                std::shared_ptr<IBackgroundCell<TGameConfig>> backgroundCell =
                    backgroundCellFactory.Create();
                board_.SetBackground(i, j, backgroundCell);
                board_.SetBackground(i, TGameConfig::BOARD_WIDTH - 1 - j, backgroundCell);
            }
        }

        sf::Rect<int> leftCollectionCenter(
            TGameConfig::BOARD_WIDTH / 4 - TGameConfig::GOAL_SIZE / 2,
            TGameConfig::BOARD_HEIGHT / 2 - TGameConfig::GOAL_SIZE / 2,
            TGameConfig::GOAL_SIZE,
            TGameConfig::GOAL_SIZE);

        auto leftCollectionCenterCell = std::make_shared<CollectionCenterCell<TGameConfig>>();

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

    void DoAction(PlayerAction playerAction)
    {
        sf::Vector2i mouseCellPosition = board_.GetMouseCellPosition(renderer_);
        std::shared_ptr<IForegroundCell<TGameConfig>> nextForegroundCell = nullptr;
        switch (playerAction)
        {
        case PlayerAction::BuildMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<TGameConfig, Direction::kTop>>();
            break;
        case PlayerAction::BuildLeftToRightConveyor:
            nextForegroundCell = std::make_shared<LeftToRightConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildLeftToBottomConveyor:
            nextForegroundCell = std::make_shared<LeftToBottomConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildTopToBottomConveyor:
            nextForegroundCell = std::make_shared<TopToBottomConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildTopToLeftConveyor:
            nextForegroundCell = std::make_shared<TopToLeftConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildRightToLeftConveyor:
            nextForegroundCell = std::make_shared<RightToLeftConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildRightToTopConveyor:
            nextForegroundCell = std::make_shared<RightToTopConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildBottomToTopConveyor:
            nextForegroundCell = std::make_shared<BottomToTopConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::BuildBottomToRightConveyor:
            nextForegroundCell = std::make_shared<BottomToRightConveyorCell<TGameConfig>>();
            break;
        case PlayerAction::Clear:
            nextForegroundCell = nullptr;
            break;
        }
        board_.SetForeground(mouseCellPosition.y, mouseCellPosition.x, nextForegroundCell);
    }

    void Update()
    {
        board_.Update();
    }

    void Render()
    {
        renderer_.Clear();
        board_.Render(renderer_);
        renderer_.Display();
    }

private:
    GameBoard<TGameConfig> board_;
    GameRenderer<TGameConfig> renderer_;
};

class GameConfig {
public:
    static constexpr std::size_t BOARD_WIDTH = 62;
    static constexpr std::size_t BOARD_HEIGHT = 36;
    static constexpr int CELL_SIZE = 20;
    static constexpr int LEFT = 20;
    static constexpr int TOP = 20;
    static constexpr std::size_t GOAL_SIZE = 4;
    static constexpr int PADDING_SIZE = 1;
};

int main(int, char **)
{
    PlayerAction playerAction = PlayerAction::BuildLeftToRightConveyor;

    sf::Font font;

    font.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project");

    GameManager<GameConfig> gameManager(&window, &font);

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
                        gameManager.DoAction(playerAction);
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
    }
}
