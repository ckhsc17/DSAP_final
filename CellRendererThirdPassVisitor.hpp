#ifndef CELL_RENDERER_THIRD_PASS_VISITOR_HPP
#define CELL_RENDERER_THIRD_PASS_VISITOR_HPP
#include "PDOGS.hpp"

template <typename TGameRendererConfig>
class CellRendererThirdPassVisitor : public Feis::CellVisitor
{
public:
    using Direction = Feis::Direction;
    using CellPosition = Feis::CellPosition;

    CellRendererThirdPassVisitor(
        Drawer<TGameRendererConfig> *drawer, 
        CellPosition cellPosition)
        : drawer_(drawer), cellPosition_(cellPosition)
    {
    }
    void Visit(const Feis::CombinerCell *cell) const override
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
    void Visit(const Feis::WallCell *cell) const override
    {
        drawer_->DrawRectangle(cellPosition_, sf::Color(60, 60, 60));
    }

private:
    Drawer<TGameRendererConfig> * const drawer_;
    CellPosition cellPosition_;
};
#endif