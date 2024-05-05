#ifndef CELL_RENDERER_SECOND_PASS_VISITOR_HPP
#define CELL_RENDERER_SECOND_PASS_VISITOR_HPP
#include "PDOGS.hpp"

template <typename TGameConfig>
class CellRendererSecondPassVisitor : public Feis::ICellVisitor<TGameConfig>
{
public:
    using Direction = Feis::Direction;
    using CellPosition = Feis::CellPosition;

    CellRendererSecondPassVisitor(Drawer<TGameConfig> *drawer, CellPosition cellPosition)
        : drawer_(drawer), cellPosition_(cellPosition)
    {
    }
    void Visit(const Feis::NumberCell<TGameConfig> *cell) const override
    {
    }
    void Visit(const Feis::CollectionCenterCell<TGameConfig> *cell) const override
    {
    }
    void Visit(const Feis::MiningMachineCell<TGameConfig> *cell) const override
    {
    }
    void Visit(const Feis::ConveyorCell<TGameConfig> *cell) const override
    {
        std::size_t productCount = cell->GetProductCount();
                
        for (std::size_t k = 0; k < productCount; ++k)
        {
            int product;
            sf::Vector2f offset;

            switch (cell->GetDirection())
            {
            case Direction::kTop:
                product = cell->GetProduct(k);
                offset = sf::Vector2f(0, TGameConfig::kCellSize * (-1.0f + (float)(k + 1) / productCount));
                break;
            case Direction::kRight:
                product = cell->GetProduct(productCount - 1 - k);
                offset = sf::Vector2f(TGameConfig::kCellSize * (float)k / productCount, 0);
                break;
            case Direction::kBottom:
                product = cell->GetProduct(productCount - 1 - k);
                offset = sf::Vector2f(0, TGameConfig::kCellSize * (float)k / productCount);
                break;
            case Direction::kLeft:
                product = cell->GetProduct(k);
                offset = sf::Vector2f(TGameConfig::kCellSize * (-1.0f + (float)(k + 1) / productCount), 0);
                break;
            }

            if (product != 0)
            {
                auto center =
                    drawer_->GetCellTopLeft(cellPosition_) +
                    offset + 
                    sf::Vector2f(TGameConfig::kCellSize / 2, TGameConfig::kCellSize / 2);

                drawer_->DrawCircle(
                    center,
                    TGameConfig::kCellSize * 0.6,
                    product % TGameConfig::kCommonDivisor == 0 ? sf::Color(30, 60, 30) : sf::Color(30, 30, 30));

                drawer_->DrawText(
                    std::to_string(product),
                    TGameConfig::kCellSize * 0.7,
                    sf::Color::White,
                    center);
            }
        }
    }
    void Visit(const Feis::CombinerCell<TGameConfig> *cell) const override
    {
    }
private:
    Drawer<TGameConfig> *drawer_;
    CellPosition cellPosition_;
};
#endif