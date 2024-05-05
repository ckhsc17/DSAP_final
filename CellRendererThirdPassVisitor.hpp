#ifndef CELL_RENDERER_THIRD_PASS_VISITOR_HPP
#define CELL_RENDERER_THIRD_PASS_VISITOR_HPP
#include "PDOGS.hpp"

template <typename TGameConfig>
class CellRendererThirdPassVisitor : public Feis::ICellVisitor<TGameConfig>
{
public:
    using Direction = Feis::Direction;
    using CellPosition = Feis::CellPosition;

    CellRendererThirdPassVisitor(Drawer<TGameConfig> *drawer, CellPosition cellPosition)
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
    }
    void Visit(const Feis::CombinerCell<TGameConfig> *cell) const override
    {
        auto direction = cell->GetDirection();
        
        if (cell->IsMainCell(cellPosition_))
        {
            drawer_->DrawRectangle(
                cellPosition_, 
                cell->GetFirstSlotProduct() != 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
            drawer_->DrawArrow(cellPosition_, direction);
        }
        else
        {
            drawer_->DrawTriangle(
                cellPosition_,
                direction,
                cell->GetSecondSlotProduct() != 0 ? sf::Color::Yellow : sf::Color(200, 200, 200));
        }
    }
private:
    Drawer<TGameConfig> *drawer_;
    CellPosition cellPosition_;
};
#endif