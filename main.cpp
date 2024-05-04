#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <functional>
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

    void DrawRectangle(sf::Vector2f center, Direction direction)
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setOrigin(rectangle.getLocalBounds().width / 2, rectangle.getLocalBounds().height / 2);
        switch (direction) {
            case Direction::kTop:
                rectangle.rotate(90);
                break;
            case Direction::kRight:
                rectangle.rotate(180);
                break;
            case Direction::kBottom:
                rectangle.rotate(270);
                break;
            case Direction::kLeft:
                break;
        }
        rectangle.setFillColor(sf::Color(128, 128, 128));
        rectangle.setPosition(center);
        DrawShape(rectangle);
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
        switch (direction) {
            case Direction::kTop:
                arrow.rotate(270);
                break;
            case Direction::kRight:
                break;
            case Direction::kBottom:
                arrow.rotate(90);
                break;
            case Direction::kLeft:
                arrow.rotate(180);
                break;
        }
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
        text.setPosition(
            topLeft.x + TGameConfig::kCellSize / 2.0f,
            topLeft.y + TGameConfig::kCellSize / 2.0f);
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
class IForegroundCell : public ICell<TGameConfig>
{
public:
    virtual bool CanRemove() const = 0;

    virtual bool ReceiveProduct(int number) = 0;

    virtual void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig> &board) = 0;

    virtual void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition,
        const IBackgroundCell<TGameConfig> &backgroundCell) const = 0;

    virtual void RenderPassTwo(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i cellPosition) const = 0;
};

struct TransferedProduct
{
    Direction direction;
    int number;
};

template<typename TGameConfig, Direction kDirection>
bool SendProduct(std::size_t i, std::size_t j, GameBoard<TGameConfig>& board, int product) {
    switch (kDirection)
    {
    case Direction::kTop:
        if (i > 0)
        {
            auto foregroundCell = board.GetForeground(i - 1, j);
            if (foregroundCell) 
            {
                return foregroundCell->ReceiveProduct(product);
            }
        }
        break;
    case Direction::kRight:
        if (j + 1 < TGameConfig::kBoardWidth)
        {
            auto foregroundCell = board.GetForeground(i, j + 1);
            if (foregroundCell)
            {
                return foregroundCell->ReceiveProduct(product);
            }
        }
        break;
    case Direction::kBottom:
        if (i + 1 < TGameConfig::kBoardHeight)
        {
            auto foregroundCell = board.GetForeground(i + 1, j);
            if (foregroundCell) 
            {
                return foregroundCell->ReceiveProduct(product);
            }
        }
        break;
    case Direction::kLeft:
        if (j > 0)
        {
            auto foregroundCell = board.GetForeground(i, j - 1);
            if (foregroundCell)
            {
                return foregroundCell->ReceiveProduct(product);
            }
        }
        break;
    }
    return false;
}

template <typename TGameConfig, Direction kOutDirection>
class ConveyorCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove() const override
    {
        return true;
    }

    bool ReceiveProduct(int number) override
    {       
        if (products_[products_.size() - 1] != 0 ||
            products_[products_.size() - 2] != 0)
        {
            return false;
        }

        products_.back() = number;

        return true;
    }

    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig> &board) override
    {
        if (products_[0] != 0)
        {
            if (SendProduct<TGameConfig, kOutDirection>(i, j, board, products_[0])) 
            {
                products_[0] = 0;
            }
        }

        if (products_[0] == 0 && products_[1] != 0)
        {
            std::swap(products_[0], products_[1]);
        }

        for (std::size_t k = 2; k < products_.size(); ++k)
        {
            if (products_[k] != 0 && products_[k - 1] == 0 && products_[k - 2] == 0)
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

        renderer.DrawRectangle(center, kOutDirection);

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
            switch (kOutDirection) {
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
                    product % TGameConfig::kCommonDivisor == 0 ?
                        sf::Color(30, 60, 30) :
                        sf::Color(30, 30, 30) );

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

template <typename TGameConfig>
class WallBackgroundCell : public IBackgroundCell<TGameConfig>
{
public:
    bool CanBuild() const override
    {
        return false;
    }
    void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i cellPosition) const override
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
class CollectionCenterCell : public IForegroundCell<TGameConfig>
{
public:
    CollectionCenterCell(std::function<void()> onProductReceived) : onProductReceived_(onProductReceived) 
    {
    }
    bool CanRemove() const override
    {
        return false;
    }
    bool ReceiveProduct(int number) override
    {
        if (number != 0 && number % TGameConfig::kCommonDivisor == 0) 
        {
            onProductReceived_();
        }
        return true;
    }
    void Update(std::size_t i, std::size_t j, GameBoard<TGameConfig> &board) override
    {
    }

    void RenderPassOne(
        GameRenderer<TGameConfig> &renderer,
        sf::Vector2i position,
        const IBackgroundCell<TGameConfig> &backgroundCell) const override
    {
        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setFillColor(sf::Color(150, 150, 150));
        rectangle.setPosition(sf::Vector2f(
            TGameConfig::kBoardLeft + position.x * TGameConfig::kCellSize,
            TGameConfig::kBoardTop + position.y * TGameConfig::kCellSize));
        renderer.DrawShape(rectangle);
    }

    void RenderPassTwo(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) const override
    {
    }
private:
    std::function<void()> onProductReceived_;
};

template <typename TGameConfig>
class EmptyCell 
{
public:
    static void RenderPassOne(GameRenderer<TGameConfig> &renderer, sf::Vector2i position) {
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
    std::shared_ptr<IForegroundCell<TGameConfig>> GetForeground() const
    {
        return foreground_;
    }
    std::shared_ptr<IBackgroundCell<TGameConfig>> GetBackground() const
    {
        return background_;
    }
    void SetForegrund(const std::shared_ptr<IForegroundCell<TGameConfig>> &value)
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
    BackgroundCellPointer GetBackground(std::size_t i, std::size_t j) const
    {
        return cellStacks_[i][j].GetBackground();
    }

    void SetForeground(std::size_t i, std::size_t j, ForegroundCellPointer value)
    {
        auto &cellStack = cellStacks_[i][j];

        const auto background = cellStack.GetBackground();

        if (background != nullptr && !background->CanBuild())
        {
            return;
        }

        const auto &previousForeground = cellStack.GetForeground();

        if (previousForeground == nullptr || previousForeground->CanRemove())
        {
            cellStack.SetForegrund(value);
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
                    foreground->Update(i, j, *this);
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

template <
    typename TGameConfig,
    Direction kDirection,
    std::size_t PRODUCTION_TIME = 100>

class MiningMachineCell : public IForegroundCell<TGameConfig>
{
public:
    bool CanRemove() const override
    {
        return true;
    }
    bool ReceiveProduct(int number) override
    {
        return false;
    }
    void Update(
        std::size_t i, std::size_t j,
        GameBoard<TGameConfig> &board) override
    {
        ++elapsedTime_;
        if (elapsedTime_ >= PRODUCTION_TIME)
        {
            auto numberCell =
                dynamic_cast<const NumberCell<TGameConfig> *>(board.GetBackground(i, j).get());

            if (numberCell)
            {
                SendProduct<TGameConfig, kDirection>(i, j, board, numberCell->GetNumber());
            }

            elapsedTime_ = 0;
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

        sf::RectangleShape rectangle(sf::Vector2f(TGameConfig::kCellSize, TGameConfig::kCellSize));
        rectangle.setFillColor(sf::Color(128, 0, 0));
        rectangle.setPosition(topLeft);
        renderer.DrawShape(rectangle);

        auto numberCell = dynamic_cast<const NumberCell<TGameConfig> *>(&backgroundCell);

        if (numberCell)
        {
            Direction direction;
            switch (kDirection) {
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
    Clear,                      
};

template <typename TGameConfig>
class GameManager
{
public:
    using Renderer = GameRenderer<TGameConfig>;
    using ForegroundCellPointer = std::shared_ptr<IForegroundCell<TGameConfig>>;
    using BackgroundCellPointer = std::shared_ptr<IBackgroundCell<TGameConfig>>;

    struct LeftCollectionCenter
    {
        static constexpr int kLeft = TGameConfig::kBoardWidth / 4 - TGameConfig::kGoalSize / 2;
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
            for (std::size_t j = 0; j < TGameConfig::kBoardWidth / 2; ++j)
            {
                std::shared_ptr<IBackgroundCell<TGameConfig>> backgroundCell =
                    backgroundCellFactory.Create();
                board_.SetBackground(i, j, backgroundCell);
                board_.SetBackground(i, TGameConfig::kBoardWidth - 1 - j, backgroundCell);
            }
        };

        auto leftCollectionCenterCell = std::make_shared<CollectionCenterCell<TGameConfig>>([&]() {
            AddScore();
        });

        for (int i = LeftCollectionCenter::kTop; i < LeftCollectionCenter::kTop + TGameConfig::kGoalSize; ++i)
        {
            for (int j = LeftCollectionCenter::kLeft; j < LeftCollectionCenter::kLeft + TGameConfig::kGoalSize; ++j)
            {
                board_.SetForeground(i, j, leftCollectionCenterCell);
            }
        }
    }

    void AddScore() {
        scores_++;
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
        case PlayerAction::BuildLeftOutMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<TGameConfig, Direction::kLeft>>();
            break;
        case PlayerAction::BuildTopOutMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<TGameConfig, Direction::kTop>>();
            break;
        case PlayerAction::BuildRightOutMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<TGameConfig, Direction::kRight>>();
            break;
        case PlayerAction::BuildBottomOutMiningMachine:
            nextForegroundCell = std::make_shared<MiningMachineCell<TGameConfig, Direction::kBottom>>();
            break;
        case PlayerAction::BuildLeftToRightConveyor:
            nextForegroundCell = std::make_shared<ConveyorCell<TGameConfig, Direction::kRight>>();
            break;
        case PlayerAction::BuildTopToBottomConveyor:
            nextForegroundCell = std::make_shared<ConveyorCell<TGameConfig, Direction::kBottom>>();
            break;
        case PlayerAction::BuildRightToLeftConveyor:
            nextForegroundCell = std::make_shared<ConveyorCell<TGameConfig, Direction::kLeft>>();
            break;
        case PlayerAction::BuildBottomToTopConveyor:
            nextForegroundCell = std::make_shared<ConveyorCell<TGameConfig, Direction::kTop>>();
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

        sf::Vector2f scoreTextPosition =
            sf::Vector2f(TGameConfig::kBoardLeft, TGameConfig::kBoardTop) +
            sf::Vector2f(
                (LeftCollectionCenter::kLeft + TGameConfig::kGoalSize / 2.0f - 0.5f) * TGameConfig::kCellSize, 
                (LeftCollectionCenter::kTop + TGameConfig::kGoalSize / 2.0f - 0.5f) * TGameConfig::kCellSize) + 
            sf::Vector2f(0, -10);

        renderer_.DrawText(
            std::to_string(scores_),
            20,
            sf::Color::White,
            scoreTextPosition);

        renderer_.DrawText(
            "(" + std::to_string(TGameConfig::kCommonDivisor) + ")",
            16,
            sf::Color(30, 100, 30),
            scoreTextPosition + sf::Vector2f(0, 30));

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
                case sf::Keyboard::J:
                    playerAction = PlayerAction::BuildLeftOutMiningMachine;
                    break;
                case sf::Keyboard::I:
                    playerAction = PlayerAction::BuildTopOutMiningMachine;
                    break;
                case sf::Keyboard::L:
                    playerAction = PlayerAction::BuildRightOutMiningMachine;
                    break;
                case sf::Keyboard::K:
                    playerAction = PlayerAction::BuildBottomOutMiningMachine;
                    break;
                case sf::Keyboard::S:
                    playerAction = PlayerAction::BuildTopToBottomConveyor;
                    break;
                case sf::Keyboard::A:
                    playerAction = PlayerAction::BuildRightToLeftConveyor;
                    break;
                case sf::Keyboard::W:
                    playerAction = PlayerAction::BuildBottomToTopConveyor;
                    break;
                case sf::Keyboard::D:
                    playerAction = PlayerAction::BuildLeftToRightConveyor;
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
