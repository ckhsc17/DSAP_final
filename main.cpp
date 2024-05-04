#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <functional>
#include <cassert>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

enum class Direction
{
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

    sf::Vector2i GetMousePosition() const
    {
        return sf::Mouse::getPosition(*window_);
    }

    void Clear()
    {
        window_->clear(sf::Color::Black);
    }

    void Display()
    {
        window_->display();
    }

    void DrawBorder(sf::Vector2f topLeft)
    {
        sf::RectangleShape rectangle(
            sf::Vector2f(
                TGameConfig::kCellSize - 2 * TGameConfig::kBorderSize,
                TGameConfig::kCellSize - 2 * TGameConfig::kBorderSize));

        rectangle.setOutlineColor(sf::Color(60, 60, 60));
        rectangle.setOutlineThickness(TGameConfig::kBorderSize);
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(topLeft + sf::Vector2f(TGameConfig::kBorderSize, TGameConfig::kBorderSize));
        window_->draw(rectangle);
    }

    void DrawRectangle(
        sf::Vector2f center,
        Direction direction,
        sf::Color color)
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setOrigin(rectangle.getLocalBounds().width / 2, rectangle.getLocalBounds().height / 2);
        rectangle.rotate(static_cast<int>(direction) * 90);
        rectangle.setFillColor(color);
        rectangle.setPosition(center);
        DrawShape(rectangle);
    }

    void DrawTriangle(
        sf::Vector2f center,
        Direction direction,
        sf::Color color)
    {
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(0, 0));
        triangle.setPoint(1, sf::Vector2f(TGameConfig::kCellSize, 0));
        triangle.setPoint(2, sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        triangle.setOrigin(triangle.getLocalBounds().width / 2, triangle.getLocalBounds().height / 2);
        triangle.rotate(static_cast<int>(direction) * 90);
        triangle.setFillColor(color);
        triangle.setPosition(center);
        DrawShape(triangle);
    }

    void DrawCircle(sf::Vector2f center, float radius, sf::Color color)
    {
        sf::CircleShape circle(radius);
        circle.setOrigin(circle.getLocalBounds().width / 2, circle.getLocalBounds().height / 2);
        circle.setFillColor(color);
        circle.setPosition(center);
        circle.setOutlineColor(sf::Color(60, 60, 60));
        circle.setOutlineThickness(2);
        DrawShape(circle);
    }

    void DrawArrow(sf::Vector2f center, Direction direction)
    {
        int offset = 2;
        sf::ConvexShape arrow(6);
        arrow.setPoint(0, sf::Vector2f(0, 0));
        arrow.setPoint(1, sf::Vector2f(-2 * offset, offset - TGameConfig::kCellSize / 2));
        arrow.setPoint(2, sf::Vector2f(0, offset - TGameConfig::kCellSize / 2));
        arrow.setPoint(3, sf::Vector2f(2 * offset, 0));
        arrow.setPoint(4, sf::Vector2f(0, TGameConfig::kCellSize / 2 - offset));
        arrow.setPoint(5, sf::Vector2f(-2 * offset, TGameConfig::kCellSize / 2 - offset));
        arrow.rotate((static_cast<int>(direction) + 3) * 90);
        arrow.setFillColor(sf::Color(60, 60, 60));
        arrow.setPosition(center);
        DrawShape(arrow);
    }

    void DrawShape(const sf::Shape &s)
    {
        window_->draw(s);
    }

    void DrawText(
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
        text.setPosition(topLeft + sf::Vector2f(TGameConfig::kCellSize / 2.0f, TGameConfig::kCellSize / 2.0f));
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

    virtual void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) const = 0;
};

template <typename TGameConfig>
class ForegroundCell : public ICell<TGameConfig>
{
public:
    ForegroundCell(sf::Vector2i topLeftCellPosition) : topLeftCellPosition_(topLeftCellPosition) {}

    virtual std::size_t GetWidth() const { return 1; }

    virtual std::size_t GetHeight() const { return 1; }

    virtual sf::Vector2i GetTopLeftCellPosition() const { return topLeftCellPosition_; }

    virtual bool CanRemove() const = 0;

    virtual std::size_t GetCapacity(sf::Vector2i cellPosition) const = 0;

    virtual void ReceiveProduct(sf::Vector2i cellPosition, int number) = 0;

    virtual void UpdatePassOne(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) = 0;

    virtual void UpdatePassTwo(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) = 0;

    virtual void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition,
        const IBackgroundCell<TGameConfig> &backgroundCell) const = 0;

    virtual void RenderPassTwo(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition) const = 0;

protected:
    sf::Vector2i topLeftCellPosition_;
};

struct TransferedProduct
{
    Direction direction;
    int number;
};

template <typename TGameConfig, Direction kDirection>
void SendProduct(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board, int product)
{
    sf::Vector2i targetCellPosition = cellPosition;

    switch (kDirection)
    {
    case Direction::kTop:
        if (cellPosition.y <= 0)
            return;
        targetCellPosition += sf::Vector2i(0, -1);
        break;
    case Direction::kRight:
        if (cellPosition.x + 1 >= TGameConfig::kBoardWidth)
            return;
        targetCellPosition += sf::Vector2i(1, 0);
        break;
    case Direction::kBottom:
        if (cellPosition.y + 1 >= TGameConfig::kBoardHeight)
            return;
        targetCellPosition += sf::Vector2i(0, 1);
        break;
    case Direction::kLeft:
        if (cellPosition.x <= 0)
            return;
        targetCellPosition += sf::Vector2i(-1, 0);
    }

    auto foregroundCell = board.GetForeground(targetCellPosition);

    if (foregroundCell)
    {
        foregroundCell->ReceiveProduct(targetCellPosition, product);
    }
}

template <typename TGameConfig, Direction kDirection>
std::size_t GetNeighborCapacity(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board)
{
    sf::Vector2i neighborCellPosition = cellPosition;

    switch (kDirection)
    {
    case Direction::kTop:
        if (cellPosition.y <= 0)
            return 0;
        neighborCellPosition += sf::Vector2i(0, -1);
        break;
    case Direction::kRight:
        neighborCellPosition += sf::Vector2i(1, 0);
        break;
    case Direction::kBottom:
        if (cellPosition.y + 1 >= TGameConfig::kBoardHeight)
            return 0;
        neighborCellPosition += sf::Vector2i(0, 1);
        break;
    case Direction::kLeft:
        if (cellPosition.x <= 0)
            return 0;
        neighborCellPosition += sf::Vector2i(-1, 0);
        break;
    }

    auto foregroundCell = board.GetForeground(neighborCellPosition);

    if (foregroundCell)
    {
        return foregroundCell->GetCapacity(neighborCellPosition);
    }
    return 0;
}

template <typename TGameConfig, Direction kOutDirection>
class ConveyorCell : public ForegroundCell<TGameConfig>
{
public:
    ConveyorCell(sf::Vector2i topLeftCellPosition) : ForegroundCell<TGameConfig>(topLeftCellPosition), products_{} {}

    bool CanRemove() const override
    {
        return true;
    }

    std::size_t GetCapacity(sf::Vector2i cellPosition) const override
    {
        for (std::size_t i = 0; i < products_.size(); ++i)
        {
            if (products_[products_.size() - 1 - i] != 0)
            {
                return i;
            }
        }
        return products_.size();
    }

    void ReceiveProduct(sf::Vector2i cellPosition, int number) override
    {
        assert(number != 0);
        assert(products_.back() == 0);
        products_.back() = number;
    }

    void UpdatePassOne(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
        std::size_t capacity = GetNeighborCapacity<TGameConfig, kOutDirection>(cellPosition, board);

        if (capacity >= 3)
        {
            if (products_[0] != 0)
            {
                SendProduct<TGameConfig, kOutDirection>(cellPosition, board, products_[0]);
                products_[0] = 0;
            }
        }

        if (capacity >= 2)
        {
            if (products_[0] == 0 && products_[1] != 0)
            {
                std::swap(products_[0], products_[1]);
            }
        }

        if (capacity >= 1)
        {
            if (products_[0] == 0 && products_[1] == 0 && products_[2] != 0)
            {
                std::swap(products_[1], products_[2]);
            }
        }
    }

    void UpdatePassTwo(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
        for (std::size_t k = 3; k < products_.size(); ++k)
        {
            if (products_[k] != 0 && products_[k - 1] == 0 && products_[k - 2] == 0 && products_[k - 3] == 0)
            {
                std::swap(products_[k], products_[k - 1]);
            }
        }
    }

    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i position,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + position.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + position.y * TGameConfig::kCellSize);

        renderer.DrawBorder(topLeft);

        sf::Vector2f center = topLeft + sf::Vector2f(TGameConfig::kCellSize / 2, TGameConfig::kCellSize / 2);

        renderer.DrawRectangle(center, kOutDirection, sf::Color(128, 128, 128));

        renderer.DrawArrow(center, kOutDirection);
    }

    void RenderPassTwo(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition) const override
    {
        const sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + cellPosition.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + cellPosition.y * TGameConfig::kCellSize);

        for (std::size_t k = 0; k < products_.size(); ++k)
        {
            int product;
            sf::Vector2f offset;
            switch (kOutDirection)
            {
            case Direction::kTop:
                product = products_[k];
                offset = sf::Vector2f(0, TGameConfig::kCellSize * (-1.0f + (float)(k + 1) / products_.size()));
                break;
            case Direction::kRight:
                product = products_[products_.size() - 1 - k];
                offset = sf::Vector2f(TGameConfig::kCellSize * ((float)k / products_.size()), 0);
                break;
            case Direction::kBottom:
                product = products_[products_.size() - 1 - k];
                offset = sf::Vector2f(0, TGameConfig::kCellSize * ((float)k / products_.size()));
                break;
            case Direction::kLeft:
                product = products_[k];
                offset = sf::Vector2f(TGameConfig::kCellSize * (-1.0f + (float)(k + 1) / products_.size()), 0);
                break;
            }

            if (product != 0)
            {
                renderer.DrawCircle(
                    topLeft + offset + sf::Vector2f(TGameConfig::kCellSize / 2, TGameConfig::kCellSize / 2),
                    TGameConfig::kCellSize * 0.6,
                    product % TGameConfig::kCommonDivisor == 0 ? sf::Color(30, 60, 30) : sf::Color(30, 30, 30));

                renderer.DrawText(
                    std::to_string(product),
                    TGameConfig::kCellSize * 0.7,
                    sf::Color::White,
                    topLeft + offset);
            }
        }
    }

protected:
    std::array<int, TGameConfig::kConveyorBufferSize> products_;
};

template <typename TGameConfig, Direction kOutDirection>
class CombinerCell : public ForegroundCell<TGameConfig>
{
public:
    CombinerCell(sf::Vector2i topLeftCellPosition) : ForegroundCell<TGameConfig>(topLeftCellPosition), products_{} {}

    std::size_t GetWidth() const override
    {
        return kOutDirection == Direction::kTop || kOutDirection == Direction::kBottom ? 2 : 1;
    }

    std::size_t GetHeight() const override
    {
        return kOutDirection == Direction::kTop || kOutDirection == Direction::kBottom ? 1 : 2;
    }

    bool CanRemove() const override
    {
        return true;
    }

    bool IsMainCell(sf::Vector2i cellPosition) const
    {
        switch (kOutDirection)
        {
        case Direction::kTop:
        case Direction::kRight:
            return cellPosition != ForegroundCell<TGameConfig>::topLeftCellPosition_;
        case Direction::kBottom:
        case Direction::kLeft:
            return cellPosition == ForegroundCell<TGameConfig>::topLeftCellPosition_;
        }
    }

    std::size_t GetCapacity(sf::Vector2i cellPosition) const override
    {
        if (IsMainCell(cellPosition))
        {
            if (products_[0] == 0)
            {
                return TGameConfig::kConveyorBufferSize;
            }
            return 0;
        }

        if (products_[1] == 0)
        {
            return TGameConfig::kConveyorBufferSize;
        }
        return 0;
    }

    void ReceiveProduct(sf::Vector2i cellPosition, int number) override
    {
        assert(number != 0);

        if (IsMainCell(cellPosition))
        {
            products_[0] = number;
        }
        else
        {
            products_[1] = number;
        }
    }

    void UpdatePassOne(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
        if (!IsMainCell(cellPosition)) return;

        if (products_[0] != 0 && products_[1] != 0)
        {
            if (GetNeighborCapacity<TGameConfig, kOutDirection>(cellPosition, board) >= 3) {
                SendProduct<TGameConfig, kOutDirection>(cellPosition, board, products_[0] + products_[1]);
                products_[0] = 0;
                products_[1] = 0;
            }
        }
    }

    void UpdatePassTwo(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
    }

    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + cellPosition.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + cellPosition.y * TGameConfig::kCellSize);

        renderer.DrawBorder(topLeft);

        sf::Vector2f center = topLeft + sf::Vector2f(TGameConfig::kCellSize / 2, TGameConfig::kCellSize / 2);

        if (IsMainCell(cellPosition))
        {
            renderer.DrawRectangle(
                center, kOutDirection, 
                products_[0] != 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
            renderer.DrawArrow(center, kOutDirection);
        }
        else
        {
            renderer.DrawTriangle(
                center, 
                kOutDirection,
                products_[1] != 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));  
        }
    }

    void RenderPassTwo(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition) const override
    {
    }

protected:
    std::array<int, 2> products_;
};

template <typename TGameConfig>
class WallCell : public ForegroundCell<TGameConfig>
{
public:
    bool CanRemove() const override
    {
        return false;
    }
    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setFillColor(sf::Color(60, 60, 60));
        rectangle.setPosition(sf::Vector2f(
            TGameConfig::kBoardLeft + cellPosition.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + cellPosition.y * TGameConfig::kCellSize));
        renderer.DrawShape(rectangle);
    }
};

template <typename TGameConfig>
class CollectionCenterCell : public ForegroundCell<TGameConfig>
{
public:
    CollectionCenterCell(
        sf::Vector2i topLeftCellPosition,
        std::function<void()> onProductReceived,
        std::function<int()> getScores)
        : ForegroundCell<TGameConfig>(topLeftCellPosition),
          onProductReceived_(onProductReceived),
          getScores_(getScores) {}

    std::size_t GetWidth() const override
    {
        return TGameConfig::kGoalSize;
    }
    std::size_t GetHeight() const override
    {
        return TGameConfig::kGoalSize;
    }
    bool CanRemove() const override
    {
        return false;
    }
    std::size_t GetCapacity(sf::Vector2i cellPosition) const override
    {
        return TGameConfig::kConveyorBufferSize;
    }
    void ReceiveProduct(sf::Vector2i cellPosition, int number) override
    {
        assert(number != 0);
        if (number % TGameConfig::kCommonDivisor == 0)
        {
            onProductReceived_();
        }
    }

    void UpdatePassOne(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
    }

    void UpdatePassTwo(sf::Vector2i cellPosition, GameBoard<TGameConfig> &board) override
    {
    }

    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i position,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        if (position != ForegroundCell<TGameConfig>::topLeftCellPosition_)
            return;

        sf::RectangleShape rectangle(
            sf::Vector2f(TGameConfig::kCellSize * GetWidth(), TGameConfig::kCellSize * GetHeight()));
        rectangle.setFillColor(sf::Color(0, 0, 180));
        rectangle.setPosition(sf::Vector2f(
            TGameConfig::kBoardLeft + position.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + position.y * TGameConfig::kCellSize));
        renderer.DrawShape(rectangle);

        sf::Vector2f scoreTextPosition =
            sf::Vector2f(TGameConfig::kBoardLeft, TGameConfig::kBoardTop) +
            sf::Vector2f(
                (ForegroundCell<TGameConfig>::topLeftCellPosition_.x + GetWidth() / 2.0f - 0.5f) * TGameConfig::kCellSize,
                (ForegroundCell<TGameConfig>::topLeftCellPosition_.y + GetHeight() / 2.0f - 0.5f) * TGameConfig::kCellSize) +
            sf::Vector2f(0, -10);

        renderer.DrawText(
            std::to_string(getScores_()),
            20,
            sf::Color::White,
            scoreTextPosition);

        renderer.DrawText(
            "(" + std::to_string(TGameConfig::kCommonDivisor) + ")",
            16,
            sf::Color(0, 255, 0),
            scoreTextPosition + sf::Vector2f(0, 30));
    }

    void RenderPassTwo(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) const override
    {
    }

private:
    std::function<void()> onProductReceived_;
    std::function<int()> getScores_;
};

template <typename TGameConfig>
class EmptyCell
{
public:
    static void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i position)
    {
        sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + position.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + position.y * TGameConfig::kCellSize);

        renderer.DrawBorder(topLeft);
    }
};

template <typename TGameConfig>
class CellStack : public ICell<TGameConfig>
{
public:
    std::shared_ptr<ForegroundCell<TGameConfig>> GetForeground() const
    {
        return foreground_;
    }
    std::shared_ptr<IBackgroundCell<TGameConfig>> GetBackground() const
    {
        return background_;
    }
    bool CanBuild() const
    {
        return foreground_ == nullptr &&
               (background_ == nullptr || background_->CanBuild());
    }

    void SetForegrund(const std::shared_ptr<ForegroundCell<TGameConfig>> &value)
    {
        foreground_ = value;
    }
    void SetBackground(const std::shared_ptr<IBackgroundCell<TGameConfig>> &value)
    {
        background_ = value;
    }
    void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) const
    {
        if (foreground_)
        {
            foreground_->RenderPassOne(renderer, position, *background_.get());
        }
        else if (background_)
        {
            background_->RenderPassOne(renderer, position);
        }
        else
        {
            EmptyCell<TGameConfig>::RenderPassOne(renderer, position);
        }
    }
    void RenderPassTwo(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i position) const
    {
        if (foreground_)
        {
            foreground_->RenderPassTwo(renderer, position);
        }
    }

private:
    std::shared_ptr<ForegroundCell<TGameConfig>> foreground_;
    std::shared_ptr<IBackgroundCell<TGameConfig>> background_;
};

template <typename TGameConfig>
class GameBoard
{
public:
    using Renderer = GameRenderer<TGameConfig>;
    using ForegroundCellPointer = std::shared_ptr<ForegroundCell<TGameConfig>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<TGameConfig>>;

    ForegroundCellPointer GetForeground(sf::Vector2i cellPosition) const
    {
        return cellStacks_[cellPosition.y][cellPosition.x].GetForeground();
    }
    BackgroundCellPointer GetBackground(sf::Vector2i cellPosition) const
    {
        return cellStacks_[cellPosition.y][cellPosition.x].GetBackground();
    }

    bool CanBuild(const std::shared_ptr<ForegroundCell<TGameConfig>> &cell)
    {
        if (cell == nullptr)
        {
            return false;
        }

        sf::Vector2i cellPosition = cell->GetTopLeftCellPosition();

        if (cellPosition.x < 0 || cellPosition.x + cell->GetWidth() > TGameConfig::kBoardWidth ||
            cellPosition.y < 0 || cellPosition.y + cell->GetHeight() > TGameConfig::kBoardHeight)
        {
            return false;
        }

        for (std::size_t i = 0; i < cell->GetHeight(); ++i)
        {
            for (std::size_t j = 0; j < cell->GetWidth(); ++j)
            {
                if (!cellStacks_[cellPosition.y + i][cellPosition.x + j].CanBuild())
                {
                    return false;
                }
            }
        }
        return true;
    }

    template<typename TCell, typename... TArgs>
    bool Build(sf::Vector2i cellPosition, TArgs ...args)
    {
        auto cell = std::make_shared<TCell>(cellPosition, args...);
        if (!CanBuild(cell))
            return false;

        sf::Vector2i topLeftPosition = cell->GetTopLeftCellPosition();

        for (std::size_t i = 0; i < cell->GetHeight(); ++i)
        {
            for (std::size_t j = 0; j < cell->GetWidth(); ++j)
            {
                cellStacks_[topLeftPosition.y + i][topLeftPosition.x + j].SetForegrund(cell);
            }
        }
        return true;
    }

    void Remove(sf::Vector2i cellPosition)
    {
        auto foreground = cellStacks_[cellPosition.y][cellPosition.x].GetForeground();

        if (foreground != nullptr)
        {
            if (foreground->CanRemove())
            {
                auto topLeftCellPosition = foreground->GetTopLeftCellPosition();

                for (std::size_t i = 0; i < foreground->GetHeight(); ++i)
                {
                    for (std::size_t j = 0; j < foreground->GetWidth(); ++j)
                    {
                        cellStacks_[topLeftCellPosition.y + i][topLeftCellPosition.x + j].SetForegrund(nullptr);
                    }
                }
            }
        }
    }

    void SetBackground(std::size_t i, std::size_t j, BackgroundCellPointer value)
    {
        cellStacks_[i][j].SetBackground(value);
    }

    sf::Vector2i GetMouseCellPosition(const Renderer &renderer) const
    {
        const sf::Vector2i mousePosition = renderer.GetMousePosition();
        const sf::Vector2i relatedMousePosition =
            mousePosition - sf::Vector2i(TGameConfig::kBoardLeft, TGameConfig::kBoardTop);
        return sf::Vector2i(
            relatedMousePosition.x / TGameConfig::kCellSize,
            relatedMousePosition.y / TGameConfig::kCellSize);
    }

    bool IsMouseInsideBoard(const Renderer &renderer) const
    {
        sf::Vector2i mousePosition = GetMouseCellPosition(renderer);
        return mousePosition.x >= 0 && mousePosition.x < TGameConfig::kBoardWidth &&
               mousePosition.y >= 0 && mousePosition.y < TGameConfig::kBoardHeight;
    }

    void Update()
    {
        for (std::size_t i = 0; i < TGameConfig::kBoardHeight; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth; ++j)
            {
                auto &cellStack = cellStacks_[i][j];
                auto foreground = cellStack.GetForeground();
                if (foreground != nullptr)
                {
                    foreground->UpdatePassOne(sf::Vector2i(j, i), *this);
                }
            }
        }
        for (std::size_t i = 0; i < TGameConfig::kBoardHeight; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth; ++j)
            {
                auto &cellStack = cellStacks_[i][j];
                auto foreground = cellStack.GetForeground();
                if (foreground != nullptr)
                {
                    foreground->UpdatePassTwo(sf::Vector2i(j, i), *this);
                }
            }
        }
    }

    void Render(Renderer &renderer) const
    {
        for (std::size_t i = 0; i < TGameConfig::kBoardHeight; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth; ++j)
            {
                cellStacks_[i][j].RenderPassOne(renderer, sf::Vector2i(j, i));
            }
        }
        for (std::size_t i = 0; i < TGameConfig::kBoardHeight; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth; ++j)
            {
                cellStacks_[i][j].RenderPassTwo(renderer, sf::Vector2i(j, i));
            }
        }

        if (!IsMouseInsideBoard(renderer))
        {
            return;
        }

        sf::Vector2i mousePosition = GetMouseCellPosition(renderer);

        // Mouse Position
        renderer.DrawText(
            "(" + std::to_string(mousePosition.x) + ", " + std::to_string(mousePosition.y) + ")",
            20,
            sf::Color::White,
            sf::Vector2f(50, 20));
    }

private:
    std::array<std::array<CellStack<TGameConfig>, TGameConfig::kBoardWidth>, TGameConfig::kBoardHeight> cellStacks_;
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

    void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i cellPosition) const override
    {
        const sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + cellPosition.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + cellPosition.y * TGameConfig::kCellSize);

        renderer.DrawBorder(topLeft);

        std::mt19937 gen(number_);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        renderer.DrawText(std::to_string(number_), TGameConfig::kCellSize * 0.75f, color, topLeft);
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

        if (std::count(numbers.begin(), numbers.end(), val))
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

template <
    typename TGameConfig,
    Direction kDirection,
    std::size_t PRODUCTION_TIME = 100>

class MiningMachineCell : public ForegroundCell<TGameConfig>
{
public:
    MiningMachineCell(sf::Vector2i topLeftCellPosition) : ForegroundCell<TGameConfig>(topLeftCellPosition), elapsedTime_(0) {}

    bool CanRemove() const override
    {
        return true;
    }
    std::size_t GetCapacity(sf::Vector2i cellPosition) const override
    {
        return 0;
    }
    void ReceiveProduct(sf::Vector2i cellPosition, int number) override
    {
    }
    void UpdatePassOne(
        sf::Vector2i cellPosition,
        GameBoard<TGameConfig> &board) override
    {
        ++elapsedTime_;
        if (elapsedTime_ >= PRODUCTION_TIME)
        {
            auto numberCell =
                dynamic_cast<const NumberCell<TGameConfig> *>(board.GetBackground(cellPosition).get());

            if (numberCell && GetNeighborCapacity<TGameConfig, kDirection>(cellPosition, board) >= 3)
            {
                SendProduct<TGameConfig, kDirection>(cellPosition, board, numberCell->GetNumber());
            }

            elapsedTime_ = 0;
        }
    }

    void UpdatePassTwo(
        sf::Vector2i cellPosition,
        GameBoard<TGameConfig> &board) override
    {
    }

    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i position,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        sf::Vector2f topLeft(
            TGameConfig::kBoardLeft + position.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + position.y * TGameConfig::kCellSize);

        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setFillColor(sf::Color(128, 0, 0));
        rectangle.setPosition(topLeft);
        renderer.DrawShape(rectangle);

        auto numberCell = dynamic_cast<const NumberCell<TGameConfig> *>(&backgroundCell);

        if (numberCell)
        {
            Direction direction;
            switch (kDirection)
            {
            case Direction::kTop:
                direction = Direction::kRight;
                break;
            case Direction::kRight:
                direction = Direction::kBottom;
                break;
            case Direction::kBottom:
                direction = Direction::kLeft;
                break;
            case Direction::kLeft:
                direction = Direction::kTop;
                break;
            }
            renderer.DrawText(
                std::to_string(numberCell->GetNumber()),
                TGameConfig::kCellSize * 0.8,
                sf::Color::White,
                topLeft,
                direction);
        }
    }
    void RenderPassTwo(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) const override
    {
    }

private:
    std::size_t elapsedTime_;
};

enum class PlayerAction
{
    BuildLeftOutMiningMachine,
    BuildTopOutMiningMachine,
    BuildRightOutMiningMachine,
    BuildBottomOutMiningMachine,
    BuildLeftToRightConveyor,
    BuildTopToBottomConveyor,
    BuildRightToLeftConveyor,
    BuildBottomToTopConveyor,
    BuildTopOutCombiner,
    BuildRightOutCombiner,
    BuildBottomOutCombiner,
    BuildLeftOutCombiner,
    Clear,
};

template <typename TGameConfig>
class GameManager
{
public:
    using Renderer = GameRenderer<TGameConfig>;
    using ForegroundCellPointer = std::shared_ptr<ForegroundCell<TGameConfig>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<TGameConfig>>;

    struct CollectionCenterConfig
    {
        static constexpr int kLeft = TGameConfig::kBoardWidth / 2 - TGameConfig::kGoalSize / 2;
        static constexpr int kTop = TGameConfig::kBoardHeight / 2 - TGameConfig::kGoalSize / 2;
    };

    GameManager(sf::RenderWindow *window, sf::Font *font) : board_(), renderer_(window, font), scores_{}
    {
        static_assert(TGameConfig::kBoardWidth % 2 == 0, "WIDTH must be even");

        window->setFramerateLimit(TGameConfig::kFPS);

        BackgroundCellFactory<TGameConfig> backgroundCellFactory;

        // Setup Background
        for (std::size_t i = 0; i < TGameConfig::kBoardHeight; ++i)
        {
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth; ++j)
            {
                std::shared_ptr<IBackgroundCell<TGameConfig>> backgroundCell =
                    backgroundCellFactory.Create();

                board_.SetBackground(i, j, backgroundCell);
            }
        };

        sf::Vector2i collectionCenterTopLeftCellPosition =
            sf::Vector2i(CollectionCenterConfig::kLeft, CollectionCenterConfig::kTop);

        board_.template Build<CollectionCenterCell<TGameConfig>>(
            collectionCenterTopLeftCellPosition, 
            [&](){ AddScore(); },
            [&](){ return scores_; });
    }

    void AddScore()
    {
        scores_++;
    }

    bool IsMouseInsideBoard()
    {
        return board_.IsMouseInsideBoard(renderer_);
    }

    void DoAction(PlayerAction playerAction)
    {
        sf::Vector2i mouseCellPosition = board_.GetMouseCellPosition(renderer_);

        switch (playerAction)
        {
        case PlayerAction::BuildLeftOutMiningMachine:
            board_.template Build<MiningMachineCell<TGameConfig, Direction::kLeft>>(mouseCellPosition);
            break;
        case PlayerAction::BuildTopOutMiningMachine:
            board_.template Build<MiningMachineCell<TGameConfig, Direction::kTop>>(mouseCellPosition);
            break;
        case PlayerAction::BuildRightOutMiningMachine:
            board_.template Build<MiningMachineCell<TGameConfig, Direction::kRight>>(mouseCellPosition);
            break;
        case PlayerAction::BuildBottomOutMiningMachine:
            board_.template Build<MiningMachineCell<TGameConfig, Direction::kBottom>>(mouseCellPosition);
            break;
        case PlayerAction::BuildLeftToRightConveyor:
            board_.template Build<ConveyorCell<TGameConfig, Direction::kRight>>(mouseCellPosition);
            break;
        case PlayerAction::BuildTopToBottomConveyor:
            board_.template Build<ConveyorCell<TGameConfig, Direction::kBottom>>(mouseCellPosition);
            break;
        case PlayerAction::BuildRightToLeftConveyor:
            board_.template Build<ConveyorCell<TGameConfig, Direction::kLeft>>(mouseCellPosition);
            break;
        case PlayerAction::BuildBottomToTopConveyor:
            board_.template Build<ConveyorCell<TGameConfig, Direction::kTop>>(mouseCellPosition);
            break;
        case PlayerAction::BuildTopOutCombiner:
            board_.template Build<CombinerCell<TGameConfig, Direction::kTop>>(mouseCellPosition);
            break;
        case PlayerAction::BuildRightOutCombiner:
            board_.template Build<CombinerCell<TGameConfig, Direction::kRight>>(mouseCellPosition);
            break;
        case PlayerAction::BuildBottomOutCombiner:
            board_.template Build<CombinerCell<TGameConfig, Direction::kBottom>>(mouseCellPosition);
            break;
        case PlayerAction::BuildLeftOutCombiner:
            board_.template Build<CombinerCell<TGameConfig, Direction::kLeft>>(mouseCellPosition);
            break;
        case PlayerAction::Clear:
            board_.Remove(mouseCellPosition);
            break;
        }
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
    int scores_;
};

class GameConfig
{
public:
    static constexpr int kFPS = 30;
    static constexpr std::size_t kBoardWidth = 62;
    static constexpr std::size_t kBoardHeight = 36;
    static constexpr int kCellSize = 20;
    static constexpr int kBoardLeft = 20;
    static constexpr int kBoardTop = 60;
    static constexpr std::size_t kGoalSize = 4;
    static constexpr int kBorderSize = 1;
    static constexpr std::size_t kConveyorBufferSize = 10;
    static constexpr int kCommonDivisor = 2;
};

int main(int, char **)
{
    PlayerAction playerAction = PlayerAction::BuildLeftToRightConveyor;

    sf::Font font;

    font.loadFromFile("/Library/Fonts/Arial Unicode.ttf");

    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project", sf::Style::Close);

    GameManager<GameConfig> gameManager(&window, &font);

    const std::map<sf::Keyboard::Key, PlayerAction> playerActionKeyboardMap = {
        {sf::Keyboard::J, PlayerAction::BuildLeftOutMiningMachine},
        {sf::Keyboard::I, PlayerAction::BuildTopOutMiningMachine},
        {sf::Keyboard::L, PlayerAction::BuildRightOutMiningMachine},
        {sf::Keyboard::K, PlayerAction::BuildBottomOutMiningMachine},
        {sf::Keyboard::D, PlayerAction::BuildLeftToRightConveyor},
        {sf::Keyboard::S, PlayerAction::BuildTopToBottomConveyor},
        {sf::Keyboard::A, PlayerAction::BuildRightToLeftConveyor},
        {sf::Keyboard::W, PlayerAction::BuildBottomToTopConveyor},
        {sf::Keyboard::Num1, PlayerAction::BuildTopOutCombiner},
        {sf::Keyboard::Num2, PlayerAction::BuildRightOutCombiner},
        {sf::Keyboard::Num3, PlayerAction::BuildBottomOutCombiner},
        {sf::Keyboard::Num4, PlayerAction::BuildLeftOutCombiner},
        {sf::Keyboard::Backspace, PlayerAction::Clear},
    };

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
                if (playerActionKeyboardMap.count(event.key.code)) {
                    playerAction = playerActionKeyboardMap.at(event.key.code);
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
