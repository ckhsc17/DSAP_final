#ifndef PDOGS_HPP
#define PDOGS_HPP
#include <iostream>
#include <memory>
#include <random>
#include <queue>
#include <functional>
#include <cassert>

namespace Feis
{
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

    template <typename TGameConfig>
    class GameBoard;

    template <typename TGameConfig>
    class Cell;

    template <typename TGameConfig>
    class ICellVisitor;

    template <typename TGameConfig>
    class Cell
    {
    public:
        virtual void Accept(const ICellVisitor<TGameConfig> *visitor) const = 0;
    };

    template <typename TGameConfig>
    class IBackgroundCell : public Cell<TGameConfig>
    {
    public:
        virtual bool CanBuild() const = 0;
    };

    template <typename TGameConfig>
    class ForegroundCell : public Cell<TGameConfig>
    {
    public:
        ForegroundCell(CellPosition topLeftCellPosition) : topLeftCellPosition_(topLeftCellPosition) {}

        virtual std::size_t GetWidth() const { return 1; }

        virtual std::size_t GetHeight() const { return 1; }

        virtual CellPosition GetTopLeftCellPosition() const { return topLeftCellPosition_; }

        virtual bool CanRemove() const = 0;

        virtual std::size_t GetCapacity(CellPosition cellPosition) const = 0;

        virtual void ReceiveProduct(CellPosition cellPosition, int number) = 0;

        virtual void UpdatePassOne(CellPosition cellPosition, GameBoard<TGameConfig> &board) = 0;

        virtual void UpdatePassTwo(CellPosition cellPosition, GameBoard<TGameConfig> &board) = 0;

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
    }

    template <typename TGameConfig>
    bool IsWithinBoard(CellPosition cellPosition)
    {
        return cellPosition.row >= 0 && cellPosition.row < TGameConfig::kBoardHeight &&
               cellPosition.col >= 0 && cellPosition.col < TGameConfig::kBoardWidth;
    }

    template <typename TGameConfig>
    void SendProduct(GameBoard<TGameConfig> &board, CellPosition cellPosition, Direction direction, int product)
    {        
        CellPosition targetCellPosition = GetNeighborCellPosition(cellPosition, direction);

        if (!IsWithinBoard<TGameConfig>(targetCellPosition))
            return;

        auto foregroundCell = board.GetForeground(targetCellPosition);

        if (foregroundCell)
        {
            foregroundCell->ReceiveProduct(targetCellPosition, product);
        }
    }


    template <typename TGameConfig>
    std::size_t GetNeighborCapacity(
        const GameBoard<TGameConfig> &board, CellPosition cellPosition, Direction direction)
    {
        CellPosition neighborCellPosition = GetNeighborCellPosition(cellPosition, direction);

        if (!IsWithinBoard<TGameConfig>(neighborCellPosition))
            return 0;

        auto foregroundCell = board.GetForeground(neighborCellPosition);

        if (foregroundCell)
        {
            return foregroundCell->GetCapacity(neighborCellPosition);
        }
        return 0;
    }

    template <typename TGameConfig>
    class ConveyorCell : public ForegroundCell<TGameConfig>
    {
    public:
        ConveyorCell(CellPosition topLeftCellPosition, Direction direction)
            : ForegroundCell<TGameConfig>(topLeftCellPosition), direction_{direction}, products_{} {}

        int GetProduct(std::size_t i) const { return products_[i]; }

        std::size_t GetProductCount() const { return products_.size();}

        Direction GetDirection() const { return direction_; }

        void Accept(const ICellVisitor<TGameConfig> *visitor) const override
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

        void UpdatePassOne(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
            std::size_t capacity = GetNeighborCapacity<TGameConfig>(board, cellPosition, direction_);

            if (capacity >= 3)
            {
                if (products_[0] != 0)
                {
                    SendProduct<TGameConfig>(board, cellPosition, direction_, products_[0]);
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

        void UpdatePassTwo(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
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
        std::array<int, TGameConfig::kConveyorBufferSize> products_;

    private:
        Direction direction_;
    };

    template <typename TGameConfig>
    class CombinerCell : public ForegroundCell<TGameConfig>
    {
    public:
        CombinerCell(CellPosition topLeft, Direction direction)
            : ForegroundCell<TGameConfig>(topLeft), direction_{direction}, firstSlotProduct_{}, secondSlotProduct_{} {}

        Direction GetDirection() const { return direction_; }
        
        int GetFirstSlotProduct() const { return firstSlotProduct_; }

        int GetSecondSlotProduct() const { return secondSlotProduct_; }

        void Accept(const ICellVisitor<TGameConfig> *visitor) const override
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
                return cellPosition != ForegroundCell<TGameConfig>::topLeftCellPosition_;
            case Direction::kBottom:
            case Direction::kLeft:
                return cellPosition == ForegroundCell<TGameConfig>::topLeftCellPosition_;
            }
        }

        std::size_t GetCapacity(CellPosition cellPosition) const override
        {
            if (IsMainCell(cellPosition))
            {
                if (firstSlotProduct_ == 0)
                {
                    return TGameConfig::kConveyorBufferSize;
                }
                return 0;
            }

            if (secondSlotProduct_ == 0)
            {
                return TGameConfig::kConveyorBufferSize;
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

        void UpdatePassOne(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
            if (!IsMainCell(cellPosition))
                return;

            if (firstSlotProduct_ != 0 && secondSlotProduct_ != 0)
            {
                if (GetNeighborCapacity<TGameConfig>(board, cellPosition, direction_) >= 3)
                {
                    SendProduct<TGameConfig>(board, cellPosition, direction_, firstSlotProduct_ + secondSlotProduct_);
                    firstSlotProduct_ = 0;
                    secondSlotProduct_ = 0;
                }
            }
        }

        void UpdatePassTwo(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
        }

    private:
        int firstSlotProduct_;
        int secondSlotProduct_;
        Direction direction_;
    };

    template <typename TGameConfig>
    class WallCell : public ForegroundCell<TGameConfig>
    {
    public:
        bool CanRemove() const override
        {
            return false;
        }
    };

    template <typename TGameConfig>
    class CollectionCenterCell : public ForegroundCell<TGameConfig>
    {
    public:
        CollectionCenterCell(
            CellPosition topLeft,
            std::function<void()> onProductReceived,
            std::function<int()> getScores)
            : ForegroundCell<TGameConfig>(topLeft),
              onProductReceived_(onProductReceived),
              getScores_(getScores) {}

        void Accept(const ICellVisitor<TGameConfig> *visitor) const override
        {
            visitor->Visit(this);
        }

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
        std::size_t GetCapacity(CellPosition cellPosition) const override
        {
            return TGameConfig::kConveyorBufferSize;
        }
        void ReceiveProduct(CellPosition cellPosition, int number) override
        {
            assert(number != 0);
            if (number % TGameConfig::kCommonDivisor == 0)
            {
                onProductReceived_();
            }
        }

        void UpdatePassOne(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
        }

        void UpdatePassTwo(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
        }

        int GetScores() const {
            return getScores_();
        }

    private:
        std::function<void()> onProductReceived_;
        std::function<int()> getScores_;
    };

    template <typename TGameConfig>
    class CellStack
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

    private:
        std::shared_ptr<ForegroundCell<TGameConfig>> foreground_;
        std::shared_ptr<IBackgroundCell<TGameConfig>> background_;
    };

    template <typename TGameConfig>
    class GameBoard
    {
    public:
        const CellStack<TGameConfig> &GetCellStack(int row, int col) const
        {
            return cellStacks_[row][col];
        }

        std::shared_ptr<ForegroundCell<TGameConfig>> GetForeground(CellPosition cellPosition) const
        {
            return cellStacks_[cellPosition.row][cellPosition.col].GetForeground();
        }

        std::shared_ptr<IBackgroundCell<TGameConfig>> GetBackground(CellPosition cellPosition) const
        {
            return cellStacks_[cellPosition.row][cellPosition.col].GetBackground();
        }

        bool CanBuild(const std::shared_ptr<ForegroundCell<TGameConfig>> &cell)
        {
            if (cell == nullptr)
            {
                return false;
            }

            CellPosition cellPosition = cell->GetTopLeftCellPosition();

            if (cellPosition.col < 0 || cellPosition.col + cell->GetWidth() > TGameConfig::kBoardWidth ||
                cellPosition.row < 0 || cellPosition.row + cell->GetHeight() > TGameConfig::kBoardHeight)
            {
                return false;
            }

            for (std::size_t i = 0; i < cell->GetHeight(); ++i)
            {
                for (std::size_t j = 0; j < cell->GetWidth(); ++j)
                {
                    if (!cellStacks_[cellPosition.row + i][cellPosition.col + j].CanBuild())
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
                    cellStacks_[topLeft.row + i][topLeft.col + j].SetForegrund(cell);
                }
            }
            return true;
        }

        void Remove(CellPosition cellPosition)
        {
            auto foreground = cellStacks_[cellPosition.row][cellPosition.col].GetForeground();

            if (foreground != nullptr)
            {
                if (foreground->CanRemove())
                {
                    auto topLeftCellPosition = foreground->GetTopLeftCellPosition();

                    for (std::size_t i = 0; i < foreground->GetHeight(); ++i)
                    {
                        for (std::size_t j = 0; j < foreground->GetWidth(); ++j)
                        {
                            cellStacks_[topLeftCellPosition.row + i][topLeftCellPosition.col + j].SetForegrund(nullptr);
                        }
                    }
                }
            }
        }

        void SetBackground(CellPosition cellPosition, std::shared_ptr<IBackgroundCell<TGameConfig>> value)
        {
            cellStacks_[cellPosition.row][cellPosition.col].SetBackground(value);
        }

        void Update()
        {
            for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
                {
                    auto &cellStack = cellStacks_[row][col];
                    auto foreground = cellStack.GetForeground();
                    if (foreground != nullptr)
                    {
                        foreground->UpdatePassOne({row, col}, *this);
                    }
                }
            }
            for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
                {
                    auto &cellStack = cellStacks_[row][col];
                    auto foreground = cellStack.GetForeground();
                    if (foreground != nullptr)
                    {
                        foreground->UpdatePassTwo({row, col}, *this);
                    }
                }
            }
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

        void Accept(const ICellVisitor<TGameConfig> *visitor) const override
        {
            visitor->Visit(this);
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
        std::size_t PRODUCTION_TIME = 100>

    class MiningMachineCell : public ForegroundCell<TGameConfig>
    {
    public:
        MiningMachineCell(CellPosition topLeft, Direction direction)
            : ForegroundCell<TGameConfig>(topLeft), direction_{direction}, elapsedTime_{0} {}

        Direction GetDirection() const { return direction_; }

        void Accept(const ICellVisitor<TGameConfig> *visitor) const override
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
        void UpdatePassOne(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
            ++elapsedTime_;
            if (elapsedTime_ >= PRODUCTION_TIME)
            {
                auto numberCell =
                    dynamic_cast<const NumberCell<TGameConfig> *>(board.GetBackground(cellPosition).get());

                if (numberCell && GetNeighborCapacity<TGameConfig>(board, cellPosition, direction_) >= 3)
                {
                    SendProduct<TGameConfig>(board, cellPosition, direction_, numberCell->GetNumber());
                }

                elapsedTime_ = 0;
            }
        }

        void UpdatePassTwo(CellPosition cellPosition, GameBoard<TGameConfig> &board) override
        {
        }

    private:
        Direction direction_;
        std::size_t elapsedTime_;
    };

    template <typename TGameConfig>
    class ICellVisitor
    {
    public:
        virtual void Visit(const NumberCell<TGameConfig> *cell) const = 0;
        virtual void Visit(const CollectionCenterCell<TGameConfig> *cell) const = 0;
        virtual void Visit(const MiningMachineCell<TGameConfig> *cell) const = 0;
        virtual void Visit(const ConveyorCell<TGameConfig> *cell) const = 0;
        virtual void Visit(const CombinerCell<TGameConfig> *cell) const = 0;
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
        struct CollectionCenterConfig
        {
            static constexpr int kLeft = TGameConfig::kBoardWidth / 2 - TGameConfig::kGoalSize / 2;
            static constexpr int kTop = TGameConfig::kBoardHeight / 2 - TGameConfig::kGoalSize / 2;
        };

        GameManager() : board_(), scores_{}
        {
            static_assert(TGameConfig::kBoardWidth % 2 == 0, "WIDTH must be even");

            BackgroundCellFactory<TGameConfig> backgroundCellFactory;

            // Setup Background
            for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
            {
                for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
                {
                    auto backgroundCell = backgroundCellFactory.Create();

                    board_.SetBackground({row, col}, backgroundCell);
                }
            };

            auto collectionCenterTopLeftCellPosition =
                CellPosition{CollectionCenterConfig::kTop, CollectionCenterConfig::kLeft};

            board_.template Build<CollectionCenterCell<TGameConfig>>(
                collectionCenterTopLeftCellPosition,
                [&]()
                { AddScore(); },
                [&]()
                { return scores_; });
        }

        const CellStack<TGameConfig> &GetCellStack(int row, int col) const
        {
            return board_.GetCellStack(row, col);
        }

        void AddScore()
        {
            scores_++;
        }

        void DoAction(PlayerAction playerAction, CellPosition cellPosition)
        {
            switch (playerAction)
            {
            case PlayerAction::BuildLeftOutMiningMachine:
                board_.template Build<MiningMachineCell<TGameConfig>>(cellPosition, Direction::kLeft);
                break;
            case PlayerAction::BuildTopOutMiningMachine:
                board_.template Build<MiningMachineCell<TGameConfig>>(cellPosition, Direction::kTop);
                break;
            case PlayerAction::BuildRightOutMiningMachine:
                board_.template Build<MiningMachineCell<TGameConfig>>(cellPosition, Direction::kRight);
                break;
            case PlayerAction::BuildBottomOutMiningMachine:
                board_.template Build<MiningMachineCell<TGameConfig>>(cellPosition, Direction::kBottom);
                break;
            case PlayerAction::BuildLeftToRightConveyor:
                board_.template Build<ConveyorCell<TGameConfig>>(cellPosition, Direction::kRight);
                break;
            case PlayerAction::BuildTopToBottomConveyor:
                board_.template Build<ConveyorCell<TGameConfig>>(cellPosition, Direction::kBottom);
                break;
            case PlayerAction::BuildRightToLeftConveyor:
                board_.template Build<ConveyorCell<TGameConfig>>(cellPosition, Direction::kLeft);
                break;
            case PlayerAction::BuildBottomToTopConveyor:
                board_.template Build<ConveyorCell<TGameConfig>>(cellPosition, Direction::kTop);
                break;
            case PlayerAction::BuildTopOutCombiner:
                board_.template Build<CombinerCell<TGameConfig>>(cellPosition, Direction::kTop);
                break;
            case PlayerAction::BuildRightOutCombiner:
                board_.template Build<CombinerCell<TGameConfig>>(cellPosition, Direction::kRight);
                break;
            case PlayerAction::BuildBottomOutCombiner:
                board_.template Build<CombinerCell<TGameConfig>>(cellPosition, Direction::kBottom);
                break;
            case PlayerAction::BuildLeftOutCombiner:
                board_.template Build<CombinerCell<TGameConfig>>(cellPosition, Direction::kLeft);
                break;
            case PlayerAction::Clear:
                board_.Remove(cellPosition);
                break;
            }
        }

        void Update()
        {
            board_.Update();
        }

    private:
        GameBoard<TGameConfig> board_;
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
}
#endif