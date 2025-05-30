#ifndef PDOGS_HPP
#define PDOGS_HPP
#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <functional>
#include <cassert>
#include <set>
#include <array>
#include <string>

namespace Feis
{
    struct GameManagerConfig
    {
        static constexpr int kBoardWidth = 62;
        static constexpr int kBoardHeight = 36;
        static constexpr std::size_t kGoalSize = 4;
        static constexpr std::size_t kConveyorBufferSize = 10;
        static constexpr int kNumberOfWalls = 100;
        static constexpr std::size_t kEndTime = 9000;
    };

    struct CellPosition
    {
        int row;
        int col;
        CellPosition &operator+=(const CellPosition &other)
        {
            row += other.row;
            col += other.col;
            return *this;
        }
    };

    bool operator==(const CellPosition &lhs, const CellPosition &rhs)
    {
        return lhs.row == rhs.row && lhs.col == rhs.col;
    }

    bool operator!=(const CellPosition &lhs, const CellPosition &rhs)
    {
        return !(lhs == rhs);
    }

    CellPosition operator+(const CellPosition &lhs, const CellPosition &rhs)
    {
        return {lhs.row + rhs.row, lhs.col + rhs.col};
    }

    enum class Direction
    {
        kTop = 0,
        kRight = 1,
        kBottom = 2,
        kLeft = 3
    };

    class GameBoard;

    class LayeredCell;


    class IGameInfo
    {
    public:
        virtual std::string GetLevelInfo() const = 0;
        virtual const LayeredCell &GetLayeredCell(CellPosition cellPosition) const = 0;
        virtual bool IsScoredProduct(int number) const = 0;
        virtual int GetScores() const = 0;
        virtual int GetEndTime() const = 0;
        virtual int GetElapsedTime() const = 0;
        virtual bool IsGameOver() const = 0;
    };

    class IGameManager : public IGameInfo
    {
    public:
        virtual void OnProductReceived(int number) = 0;
    };

    class Cell;

    class NumberCell;
    class CollectionCenterCell;
    class MiningMachineCell;
    class ConveyorCell;
    class CombinerCell;
    class WallCell;

    class CellVisitor
    {
    public:
        virtual void Visit(const NumberCell *cell) const {}
        virtual void Visit(const CollectionCenterCell *cell) const  {}
        virtual void Visit(const MiningMachineCell *cell) const  {}
        virtual void Visit(const ConveyorCell *cell) const  {}
        virtual void Visit(const CombinerCell *cell) const  {}
        virtual void Visit(const WallCell *cell) const  {}
    };

    class Cell
    {
    public:
        virtual void Accept(const CellVisitor *visitor) const = 0;
        virtual ~Cell() {}
    };

    class IBackgroundCell : public Cell
    {
    public:
        virtual bool CanBuild() const = 0;
        virtual ~IBackgroundCell() {}
    };

    class ForegroundCell : public Cell
    {
    public:
        ForegroundCell(CellPosition topLeftCellPosition) : topLeftCellPosition_(topLeftCellPosition) {}

        virtual std::size_t GetWidth() const { return 1; }

        virtual std::size_t GetHeight() const { return 1; }

        virtual CellPosition GetTopLeftCellPosition() const { return topLeftCellPosition_; }

        virtual bool CanRemove() const { return false; }

        virtual std::size_t GetCapacity(CellPosition cellPosition) const { return 0; }

        virtual void ReceiveProduct(CellPosition cellPosition, int number) { }

        virtual void UpdatePassOne(CellPosition cellPosition, GameBoard &board) { }

        virtual void UpdatePassTwo(CellPosition cellPosition, GameBoard &board) { }

        virtual ~ForegroundCell() {}

    protected:
        CellPosition topLeftCellPosition_;
    };

    class ICellRenderer
    {
    public:
        virtual void RenderPassOne(CellPosition position) const = 0;
        virtual void RenderPassTwo(CellPosition position) const = 0;
    };

    CellPosition GetNeighborCellPosition(CellPosition cellPosition, Direction direction)
    {
        switch (direction)
        {
        case Direction::kTop:
            return cellPosition + CellPosition{-1, 0};
        case Direction::kRight:
            return cellPosition + CellPosition{0, 1};
        case Direction::kBottom:
            return cellPosition + CellPosition{1, 0};
        case Direction::kLeft:
            return cellPosition + CellPosition{0, -1};
        }
        assert(false);
        return {0, 0};
    }

    std::size_t GetNeighborCapacity(const GameBoard &board, CellPosition cellPosition, Direction direction);

    void SendProduct(GameBoard &board, CellPosition cellPosition, Direction direction, int product);

    class ConveyorCell : public ForegroundCell
    {
    public:
        ConveyorCell(CellPosition topLeftCellPosition, Direction direction)
            : ForegroundCell(topLeftCellPosition),  products_{}, direction_{direction} {}

        int GetProduct(std::size_t i) const { return products_[i]; }

        std::size_t GetProductCount() const { return products_.size(); }

        Direction GetDirection() const { return direction_; }

        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }

        bool CanRemove() const override
        {
            return true;
        }

        std::size_t GetCapacity(CellPosition cellPosition) const override
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

        void ReceiveProduct(CellPosition cellPosition, int number) override
        {
            assert(number != 0);
            assert(products_.back() == 0);
            products_.back() = number;
        }

        void UpdatePassOne(CellPosition cellPosition, GameBoard &board) override
        {
            std::size_t capacity = GetNeighborCapacity(board, cellPosition, direction_);

            if (capacity >= 3)
            {
                if (products_[0] != 0)
                {
                    SendProduct(board, cellPosition, direction_, products_[0]);
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

        void UpdatePassTwo(CellPosition cellPosition, GameBoard &board) override
        {
            for (std::size_t k = 3; k < products_.size(); ++k)
            {
                if (products_[k] != 0 && products_[k - 1] == 0 && products_[k - 2] == 0 && products_[k - 3] == 0)
                {
                    std::swap(products_[k], products_[k - 1]);
                }
            }
        }

    protected:
        std::array<int, GameManagerConfig::kConveyorBufferSize> products_;

    private:
        Direction direction_;
    };

    class CombinerCell : public ForegroundCell
    {
    public:
        CombinerCell(CellPosition topLeft, Direction direction)
            : ForegroundCell(topLeft), direction_{direction}, firstSlotProduct_{}, secondSlotProduct_{} {}

        Direction GetDirection() const { return direction_; }

        int GetFirstSlotProduct() const { return firstSlotProduct_; }

        int GetSecondSlotProduct() const { return secondSlotProduct_; }

        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }

        std::size_t GetWidth() const override
        {
            return direction_ == Direction::kTop || direction_ == Direction::kBottom ? 2 : 1;
        }

        std::size_t GetHeight() const override
        {
            return direction_ == Direction::kTop || direction_ == Direction::kBottom ? 1 : 2;
        }

        bool CanRemove() const override
        {
            return true;
        }

        bool IsMainCell(CellPosition cellPosition) const
        {
            switch (direction_)
            {
            case Direction::kTop:
            case Direction::kRight:
                return cellPosition != ForegroundCell::topLeftCellPosition_;
            case Direction::kBottom:
            case Direction::kLeft:
                return cellPosition == ForegroundCell::topLeftCellPosition_;
            }
            assert(false);
            return false;
        }

        std::size_t GetCapacity(CellPosition cellPosition) const override
        {
            if (IsMainCell(cellPosition))
            {
                if (firstSlotProduct_ == 0)
                {
                    return GameManagerConfig::kConveyorBufferSize;
                }
                return 0;
            }

            if (secondSlotProduct_ == 0)
            {
                return GameManagerConfig::kConveyorBufferSize;
            }
            return 0;
        }

        void ReceiveProduct(CellPosition cellPosition, int number) override
        {
            assert(number != 0);

            if (IsMainCell(cellPosition))
            {
                firstSlotProduct_ = number;
            }
            else
            {
                secondSlotProduct_ = number;
            }
        }

        void UpdatePassOne(CellPosition cellPosition, GameBoard &board) override
        {
            if (!IsMainCell(cellPosition))
                return;

            if (firstSlotProduct_ != 0 && secondSlotProduct_ != 0)
            {
                if (GetNeighborCapacity(board, cellPosition, direction_) >= 3)
                {
                    SendProduct(board, cellPosition, direction_, firstSlotProduct_ + secondSlotProduct_);
                    firstSlotProduct_ = 0;
                    secondSlotProduct_ = 0;
                }
            }
        }
    private:
        Direction direction_;
        int firstSlotProduct_;
        int secondSlotProduct_;
    };

    class WallCell : public ForegroundCell
    {
    public:
        WallCell(CellPosition topLeft) : ForegroundCell(topLeft) {}

        bool CanRemove() const override
        {
            return false;
        }
        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }
    };

    class CollectionCenterCell : public ForegroundCell
    {
    public:
        CollectionCenterCell(
            CellPosition topLeft,
            IGameManager *gameManager)
            : ForegroundCell(topLeft),
              gameManager_{gameManager} {}

        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }

        std::size_t GetWidth() const override
        {
            return GameManagerConfig::kGoalSize;
        }
        std::size_t GetHeight() const override
        {
            return GameManagerConfig::kGoalSize;
        }
        std::size_t GetCapacity(CellPosition cellPosition) const override
        {
            return GameManagerConfig::kConveyorBufferSize;
        }
        void ReceiveProduct(CellPosition cellPosition, int number) override
        {
            assert(number != 0);

            if (number)
            {
                gameManager_->OnProductReceived(number);
            }
        }
        int GetScores() const
        {
            return gameManager_->GetScores();
        }

    private:
        IGameManager *gameManager_;
    };

    class LayeredCell
    {
    public:
        std::shared_ptr<ForegroundCell> GetForeground() const
        {
            return foreground_;
        }
        std::shared_ptr<IBackgroundCell> GetBackground() const
        {
            return background_;
        }
        bool CanBuild() const
        {
            return foreground_ == nullptr &&
                   (background_ == nullptr || background_->CanBuild());
        }

        void SetForegrund(const std::shared_ptr<ForegroundCell> &value)
        {
            foreground_ = value;
        }
        void SetBackground(const std::shared_ptr<IBackgroundCell> &value)
        {
            background_ = value;
        }

    private:
        std::shared_ptr<ForegroundCell> foreground_;
        std::shared_ptr<IBackgroundCell> background_;
    };


    class GameBoard
    {
    public:
        const LayeredCell &GetLayeredCell(CellPosition cellPosition) const
        {
            return layeredCells_[cellPosition.row][cellPosition.col];
        }

        bool CanBuild(const std::shared_ptr<ForegroundCell> &cell)
        {
            if (cell == nullptr)
            {
                return false;
            }

            CellPosition cellPosition = cell->GetTopLeftCellPosition();

            if (cellPosition.col < 0 || cellPosition.col + cell->GetWidth() > GameManagerConfig::kBoardWidth ||
                cellPosition.row < 0 || cellPosition.row + cell->GetHeight() > GameManagerConfig::kBoardHeight)
            {
                return false;
            }

            for (std::size_t i = 0; i < cell->GetHeight(); ++i)
            {
                for (std::size_t j = 0; j < cell->GetWidth(); ++j)
                {
                    if (!layeredCells_[cellPosition.row + i][cellPosition.col + j].CanBuild())
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        template <typename TCell, typename... TArgs>
        bool Build(CellPosition cellPosition, TArgs... args)
        {
            auto cell = std::make_shared<TCell>(cellPosition, args...);
            if (!CanBuild(cell))
                return false;

            CellPosition topLeft = cell->GetTopLeftCellPosition();

            for (std::size_t i = 0; i < cell->GetHeight(); ++i)
            {
                for (std::size_t j = 0; j < cell->GetWidth(); ++j)
                {
                    layeredCells_[topLeft.row + i][topLeft.col + j].SetForegrund(cell);
                }
            }
            return true;
        }

        void Remove(CellPosition cellPosition)
        {
            auto foreground = layeredCells_[cellPosition.row][cellPosition.col].GetForeground();

            if (foreground != nullptr)
            {
                if (foreground->CanRemove())
                {
                    auto topLeftCellPosition = foreground->GetTopLeftCellPosition();

                    for (std::size_t i = 0; i < foreground->GetHeight(); ++i)
                    {
                        for (std::size_t j = 0; j < foreground->GetWidth(); ++j)
                        {
                            layeredCells_[topLeftCellPosition.row + i][topLeftCellPosition.col + j].SetForegrund(nullptr);
                        }
                    }
                }
            }
        }

        void SetBackground(CellPosition cellPosition, std::shared_ptr<IBackgroundCell> value)
        {
            layeredCells_[cellPosition.row][cellPosition.col].SetBackground(value);
        }

        void Update()
        {
            for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
                {
                    auto &layeredCell = layeredCells_[row][col];
                    auto foreground = layeredCell.GetForeground();
                    if (foreground != nullptr)
                    {
                        foreground->UpdatePassOne({row, col}, *this);
                    }
                }
            }
            for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
                {
                    auto &layeredCell = layeredCells_[row][col];
                    auto foreground = layeredCell.GetForeground();
                    if (foreground != nullptr)
                    {
                        foreground->UpdatePassTwo({row, col}, *this);
                    }
                }
            }
        }

    private:
        std::array<std::array<LayeredCell, GameManagerConfig::kBoardWidth>, GameManagerConfig::kBoardHeight> layeredCells_;
    };

    bool IsWithinBoard(CellPosition cellPosition)
    {
        return cellPosition.row >= 0 && cellPosition.row < GameManagerConfig::kBoardHeight &&
               cellPosition.col >= 0 && cellPosition.col < GameManagerConfig::kBoardWidth;
    }

    void SendProduct(GameBoard &board, CellPosition cellPosition, Direction direction, int product)
    {
        CellPosition targetCellPosition = GetNeighborCellPosition(cellPosition, direction);

        if (!IsWithinBoard(targetCellPosition))
            return;

        auto foregroundCell = board.GetLayeredCell(targetCellPosition).GetForeground();

        if (foregroundCell)
        {
            foregroundCell->ReceiveProduct(targetCellPosition, product);
        }
    }

    std::size_t GetNeighborCapacity(const GameBoard &board, CellPosition cellPosition, Direction direction)
    {
        CellPosition neighborCellPosition = GetNeighborCellPosition(cellPosition, direction);

        if (!IsWithinBoard(neighborCellPosition))
            return 0;

        auto foregroundCell = board.GetLayeredCell(neighborCellPosition).GetForeground();

        if (foregroundCell)
        {
            return foregroundCell->GetCapacity(neighborCellPosition);
        }
        return 0;
    }

    class NumberCell : public IBackgroundCell
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

        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }

    private:
        int number_;
    };

    class BackgroundCellFactory
    {
    public:
        BackgroundCellFactory(unsigned int seed) : gen_(seed) {}

        std::shared_ptr<IBackgroundCell> Create()
        {
            int val = gen_() % 30;

            std::set<int> numbers = {1, 2, 3, 5, 7, 11};

            if (numbers.count(val))
            {
                return std::make_shared<NumberCell>(val);
            }
            else
            {
                return nullptr;
            }
        }

    private:
        std::mt19937 gen_;
    };

    class MiningMachineCell : public ForegroundCell
    {
    public:
        MiningMachineCell(CellPosition topLeft, Direction direction)
            : ForegroundCell(topLeft), direction_{direction}, elapsedTime_{0} {}

        Direction GetDirection() const { return direction_; }

        void Accept(const CellVisitor *visitor) const override
        {
            visitor->Visit(this);
        }

        bool CanRemove() const override
        {
            return true;
        }
        std::size_t GetCapacity(CellPosition cellPosition) const override
        {
            return 0;
        }
        void ReceiveProduct(CellPosition cellPosition, int number) override
        {
        }
        void UpdatePassOne(CellPosition cellPosition, GameBoard &board) override
        {
            ++elapsedTime_;
            if (elapsedTime_ >= 100)
            {
                auto numberCell =
                    dynamic_cast<const NumberCell *>(board.GetLayeredCell(cellPosition).GetBackground().get());

                if (numberCell && GetNeighborCapacity(board, cellPosition, direction_) >= 3)
                {
                    SendProduct(board, cellPosition, direction_, numberCell->GetNumber());
                }

                elapsedTime_ = 0;
            }
        }
    private:
        Direction direction_;
        std::size_t elapsedTime_;
    };

    enum class PlayerActionType
    {
        None,
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

    struct PlayerAction
    {
        PlayerActionType type;
        CellPosition cellPosition;
    };

    class IGamePlayer
    {
    public:
        virtual PlayerAction GetNextAction(const IGameInfo& info) = 0;
    };

    class GameManager : public IGameManager
    {
    public:
        struct CollectionCenterConfig
        {
            static constexpr int kLeft = GameManagerConfig::kBoardWidth / 2 - GameManagerConfig::kGoalSize / 2;
            static constexpr int kTop = GameManagerConfig::kBoardHeight / 2 - GameManagerConfig::kGoalSize / 2;
        };

        GameManager(
            IGamePlayer* player,
            int commonDividor, 
            unsigned int seed) 
            : elapsedTime_{}, endTime_{GameManagerConfig::kEndTime}, player_(player), board_(), commonDividor_{commonDividor}, scores_{}
        {
            static_assert(GameManagerConfig::kBoardWidth % 2 == 0, "WIDTH must be even");

            BackgroundCellFactory backgroundCellFactory(seed);

            for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
                {
                    auto backgroundCell = backgroundCellFactory.Create();

                    board_.SetBackground({row, col}, backgroundCell);
                }
            };

            auto collectionCenterTopLeftCellPosition =
                CellPosition{CollectionCenterConfig::kTop, CollectionCenterConfig::kLeft};

            board_.template Build<CollectionCenterCell>(collectionCenterTopLeftCellPosition, this);

            std::mt19937 gen(seed);

            for (int k = 1; k <= GameManagerConfig::kNumberOfWalls; ++k)
            {
                std::uniform_int_distribution<int> disRow(0, GameManagerConfig::kBoardHeight - 1);
                std::uniform_int_distribution<int> disCol(0, GameManagerConfig::kBoardWidth - 1);
                CellPosition cellPosition;
                cellPosition.row = disRow(gen);
                cellPosition.col = disCol(gen);
                if (board_.GetLayeredCell(cellPosition).GetForeground() == nullptr)
                {
                    board_.template Build<WallCell>(cellPosition);
                }
            }
        }

        bool IsGameOver() const override
        {
            return elapsedTime_ >= endTime_;
        }

        int GetEndTime() const override { return endTime_; }

        int GetElapsedTime() const override { return elapsedTime_; }

        std::string GetLevelInfo() const override
        {
            return "(" + std::to_string(commonDividor_) + ")";
        }

        bool IsScoredProduct(int number) const override
        {
            return number % commonDividor_ == 0;
        }

        void OnProductReceived(int number) override
        {
            assert(number != 0);

            if (number % commonDividor_ == 0)
            {
                AddScore();
            }
        }

        int GetScores() const override
        {
            return scores_;
        }

        const LayeredCell &GetLayeredCell(CellPosition cellPosition) const override
        {
            return board_.GetLayeredCell(cellPosition);
        }

        void AddScore()
        {
            scores_++;
        }

        void Update()
        {
            if (elapsedTime_ >= endTime_) return;

            ++elapsedTime_;
            
            if (elapsedTime_ % 3 == 0) 
            {
                PlayerAction playerAction = player_->GetNextAction(*this);

                switch (playerAction.type)
                {
                case PlayerActionType::None:
                    break;
                case PlayerActionType::BuildLeftOutMiningMachine:
                    board_.template Build<MiningMachineCell>(playerAction.cellPosition, Direction::kLeft);
                    break;
                case PlayerActionType::BuildTopOutMiningMachine:
                    board_.template Build<MiningMachineCell>(playerAction.cellPosition, Direction::kTop);
                    break;
                case PlayerActionType::BuildRightOutMiningMachine:
                    board_.template Build<MiningMachineCell>(playerAction.cellPosition, Direction::kRight);
                    break;
                case PlayerActionType::BuildBottomOutMiningMachine:
                    board_.template Build<MiningMachineCell>(playerAction.cellPosition, Direction::kBottom);
                    break;
                case PlayerActionType::BuildLeftToRightConveyor:
                    board_.template Build<ConveyorCell>(playerAction.cellPosition, Direction::kRight);
                    break;
                case PlayerActionType::BuildTopToBottomConveyor:
                    board_.template Build<ConveyorCell>(playerAction.cellPosition, Direction::kBottom);
                    break;
                case PlayerActionType::BuildRightToLeftConveyor:
                    board_.template Build<ConveyorCell>(playerAction.cellPosition, Direction::kLeft);
                    break;
                case PlayerActionType::BuildBottomToTopConveyor:
                    board_.template Build<ConveyorCell>(playerAction.cellPosition, Direction::kTop);
                    break;
                case PlayerActionType::BuildTopOutCombiner:
                    board_.template Build<CombinerCell>(playerAction.cellPosition, Direction::kTop);
                    break;
                case PlayerActionType::BuildRightOutCombiner:
                    board_.template Build<CombinerCell>(playerAction.cellPosition, Direction::kRight);
                    break;
                case PlayerActionType::BuildBottomOutCombiner:
                    board_.template Build<CombinerCell>(playerAction.cellPosition, Direction::kBottom);
                    break;
                case PlayerActionType::BuildLeftOutCombiner:
                    board_.template Build<CombinerCell>(playerAction.cellPosition, Direction::kLeft);
                    break;
                case PlayerActionType::Clear:
                    board_.Remove(playerAction.cellPosition);
                    break;
                }
            }

            board_.Update();
        }

    private:
        std::size_t elapsedTime_;
        std::size_t endTime_;
        IGamePlayer* player_;
        GameBoard board_;
        int commonDividor_;
        int scores_;
    };
}
#endif

#ifndef USE_GUI
void Test(int commonDividor, unsigned int seed);

void Test1A() { Test(1, 20); }
void Test1B() { Test(1, 0 /* HIDDEN */); }

void Test2A() { Test(2, 25); }
void Test2B() { Test(2, 0 /* HIDDEN */); }

void Test3A() { Test(3, 30); }
void Test3B() { Test(3, 0 /* HIDDEN */); }

void Test4A() { Test(4, 35); }
void Test4B() { Test(4, 0 /* HIDDEN */); }

void Test5A() { Test(5, 40); }
void Test5B() { Test(5, 0 /* HIDDEN */); }

int main() {
    int id;
    std::cin >> id;
    void (*f[])() = { Test1A, Test1B, Test2A, Test2B, Test3A, Test3B, Test4A, Test4B, Test5A, Test5B };
    f[id-1]();
}

// [YOUR CODE WILL BE PLACED HERE]


// [YOUR CODE WILL BE PLACED HERE]
//先解決CollectionCenterCell十字座標上的NumberCell

// global variable to record action
// declare a global direction enum struct to record the current direction we want to put
// declare a global position variable to record the current position
// declare a global "radius" variable to circularly iterate / rotate out from the CollectionCenterCell

// 
int x = Feis::GameManagerConfig::kBoardWidth / 2 - Feis::GameManagerConfig::kGoalSize / 2;
int y = Feis::GameManagerConfig::kBoardHeight / 2 - Feis::GameManagerConfig::kGoalSize / 2;

int move_x = -1;
int move_y = 0;
int radius = 1;
bool flag = true;

//標記現在相對CollectingCenterCell的方向
Feis::Direction d;

class GamePlayer : public Feis::IGamePlayer 
{
public:
    //找到可得分數字格isNumberCell and canBuild function
    bool IsTargetNumberCellAndCanBuild(const Feis::CellPosition& pos, const Feis::IGameInfo& info)
    {
        const auto& cell = info.GetLayeredCell(pos);
        auto backgroundCell = cell.GetBackground();
        int TARGET = 1;
        std::string tar = info.GetLevelInfo(); 
        TARGET = tar[1] - '0';
        
        if (backgroundCell != nullptr) 
        {
            auto numberCell = std::dynamic_pointer_cast<Feis::NumberCell>(backgroundCell);
            if (numberCell && backgroundCell->CanBuild() && numberCell->GetNumber() % TARGET == 0)
            {
                return true;
            }
        }
        return false;
    }
    
    void UpdateOffset(int& move_x, int& move_y)
    {
        if (move_x < 0 && std::abs(move_x) < x) {
            move_x --;
            std::cout << "hi1" << std::endl;
        } 
        else if (move_x < 0) {
            std::cout << "hi2" << std::endl;
            if (move_y > -4) {
                move_x = -1;
                move_y --;
            }
            else {
                move_x = 3;
                move_y = 0;
            }
        }

        //right
        else if (move_x >= 3 && move_x < x) move_x ++;
        else if (move_x >= 3 && move_x >= x) {
            if (move_y > -4) {
                move_x = 3;
                move_y --;
            }
            else {
                move_x = 0;
                move_y = 1;
            }
        }
        
        //top
        else if (move_y > 0 && move_y < y) move_y ++;
        else if (move_y > 0 && move_y >= y) {
            if (move_x < 3) {
                move_y = 1;
                move_x ++;
            }
            else {
                move_x = 0;
                move_y = -3;
            }
        }
        
        /*
        //down
        else if (move_y <= -3 && std::abs(move_y < y)) move_y --;
        else if (move_y <= -3 && )
        */
    }

    // Can return 9000 times at most
    Feis::PlayerAction GetNextAction(const Feis::IGameInfo& info) override 
    {
        //std::cout << x << " " << y;
        //return {Feis::PlayerActionType::BuildRightOutMiningMachine, Feis::CellPosition {17,28}};

        
        // Extract game information from IGameInfo interface
        const Feis::GameManager& gameManager = dynamic_cast<const Feis::GameManager&>(info);

        // Get current layered cell at the central position
        Feis::CellPosition currentCellPos{y + move_y, x + move_x};
        //std::cout << "x,y: " << x << " " << y << std::endl;
        //std::cout << "currentCellPos: " << currentCellPos.row << " " << currentCellPos.col << std::endl;
        const Feis::LayeredCell& currentCell = gameManager.GetLayeredCell(currentCellPos);
        auto foregroundCell = currentCell.GetForeground();
        auto backgroundCell = currentCell.GetBackground();

        if (backgroundCell != nullptr) 
        {
            auto numberCell = std::dynamic_pointer_cast<Feis::NumberCell>(backgroundCell);
            //先不論如何遇到numberCell就蓋MiningMachineCell
            if (numberCell && backgroundCell->CanBuild())
            {
                std::cout << "found numberCell!" << std::endl;
                //std::cout << "move_x: " << move_x << std::endl;
                //std::cout << "currentCellPos: " << currentCellPos.row << " " << currentCellPos.col << std::endl;
                //記得判斷牆壁
                if (move_x < 0) {
                    UpdateOffset(move_x, move_y);
                    return {Feis::PlayerActionType::BuildRightOutMiningMachine, currentCellPos};
                }
                else if (move_x > 0) {
                    UpdateOffset(move_x, move_y);
                    return {Feis::PlayerActionType::BuildLeftOutMiningMachine, currentCellPos};
                }
                //下面為當x=0的時候
                else if (move_y < 0) {
                    UpdateOffset(move_x, move_y);
                    return {Feis::PlayerActionType::BuildTopOutMiningMachine, currentCellPos};
                }
                else if (move_y > 0) {
                    UpdateOffset(move_x, move_y);
                    return {Feis::PlayerActionType::BuildBottomOutMiningMachine, currentCellPos};
                }
            }
            /*
            // wall
            // 目前作法會犧牲掉緊鄰牆壁的numberCell
            else {
                if (move_x < 0) {
                    return {Feis::PlayerActionType::BuildBottomToTopConveyor, currentCellPos + Feis::CellPosition {-1, 0}};
                }
                else if (move_x > 0) {
                    return {Feis::PlayerActionType::BuildBottomToTopConveyor, currentCellPos + Feis::CellPosition {1, 0}};
                }
                //下面為當x=0的時候
                else if (move_y < 0) {
                    return {Feis::PlayerActionType::BuildLeftToRightConveyor, currentCellPos + Feis::CellPosition {0, -1}};
                }
                else if (move_y > 0) {
                    return {Feis::PlayerActionType::BuildLeftToRightConveyor, currentCellPos + Feis::CellPosition {0, 1}};
                }
            };
            */
        }

        else if (foregroundCell == nullptr) {
            if (move_x < 0) {
                std::cout << "BuildLeftToRightConveyor" << std::endl;
                UpdateOffset(move_x, move_y);
                return {Feis::PlayerActionType::BuildLeftToRightConveyor, currentCellPos};
            }
            else if (move_x > 0) {
                UpdateOffset(move_x, move_y);
                return {Feis::PlayerActionType::BuildRightToLeftConveyor, currentCellPos};
            }
            else if (move_y < 0) {
                UpdateOffset(move_x, move_y);
                return {Feis::PlayerActionType::BuildBottomToTopConveyor, currentCellPos};
            }
            else if (move_y > 0) {
                UpdateOffset(move_x, move_y);
                return {Feis::PlayerActionType::BuildTopToBottomConveyor, currentCellPos};
            }
        }

        //std::cout << "hi" << std::endl;
        // update the offsets
        //left
        
        
        //處理十字以外的其他座標

        //int x = CollectionCenterConfig::kLeft;
        // explore the position on the left of the CollectionCenterCell first, then go to top, right, bottom
        // from the position of CollectionCenterCell, use GetNeighborCellPosition() and the current direction (e.g. (-1,0) for left first) to access the neighbor cell
        // After accessing a position on the grid, consider the following:
        // if (is NumberCell) -> put a MiningMachineCell on pointing at the CollectionCenterCell's direction; also check if there is a wallCell or another NumberCell immediately blocking the way; if yes, change the direction (for example if originally the cell was going right, turn to up or down)
        // else if (is WallCell) -> skip
        // else -> it is an empty cell, then put a ConveyorCell on it pointing at the CollectionCell; also check if there is a wallCell or another NumberCell immediately blocking the way; if yes, change the direction (for example if originally the cell was going right, turn to up or down)
        // update the cellPosition to access the next cell

    }    
};

// [YOUR CODE WILL BE PLACED HERE]


void Test(int commonDividor, unsigned int seed)
{
    GamePlayer player;
    Feis::GameManager gameManager(&player, commonDividor, seed);

    while (!gameManager.IsGameOver())
    {
        gameManager.Update();
    }
    
    std::cout << gameManager.GetScores() << std::endl;
}


#endif
