#ifndef CELL_RENDERER_FIRST_PASS_VISITOR_HPP
#define CELL_RENDERER_FIRST_PASS_VISITOR_HPP
#include "PDOGS.hpp"
#include "Drawer.hpp"

template <typename TGameConfig>
class CellRendererFirstPassVisitor : public Feis::ICellVisitor<TGameConfig>
{
public:
    using CellPosition = Feis::CellPosition;
    using Direction = Feis::Direction;
    using IBackgroundCell = Feis::IBackgroundCell<TGameConfig>;

    CellRendererFirstPassVisitor(
        Drawer<TGameConfig> *drawer,
        CellPosition cellPosition,
        IBackgroundCell *backgroundCell)
        : drawer_(drawer), cellPosition_(cellPosition), backgroundCell_(backgroundCell)
    {
    }

    void Visit(const Feis::Cell<TGameConfig> *cell) const
    {
        cell->Accept(this);
    }

    void Visit(const Feis::NumberCell<TGameConfig> *cell) const override
    {
        drawer_->DrawBorder(cellPosition_);

        int number = cell->GetNumber();
        std::mt19937 gen(number);
        std::uniform_int_distribution<int> dis(0, 50);
        int r = dis(gen) + 128;
        int g = dis(gen) + 128;
        int b = dis(gen) + 128;
        sf::Color color(r, g, b);

        drawer_->DrawText(
            std::to_string(number),
            TGameConfig::kCellSize * 0.75f,
            color,
            cellPosition_);
    }

    void Visit(const Feis::CollectionCenterCell<TGameConfig> *cell) const override
    {
        if (cellPosition_ != cell->GetTopLeftCellPosition())
            return;

        sf::RectangleShape rectangle(
            sf::Vector2f(TGameConfig::kCellSize * cell->GetWidth(), TGameConfig::kCellSize * cell->GetHeight()));

        rectangle.setFillColor(sf::Color(0, 0, 180));
        rectangle.setPosition(drawer_->GetCellTopLeft(cellPosition_));
        drawer_->DrawShape(rectangle);

        sf::Vector2f scoreTextPosition =
            drawer_->GetCellTopLeft(cell->GetTopLeftCellPosition()) +
            sf::Vector2f(cell->GetWidth(), cell->GetHeight()) * 0.5f * static_cast<float>(TGameConfig::kCellSize) +
            sf::Vector2f(0, -10);

        drawer_->DrawText(
            std::to_string(cell->GetScores()),
            20,
            sf::Color::White,
            scoreTextPosition);

        drawer_->DrawText(
            "(" + std::to_string(TGameConfig::kCommonDivisor) + ")",
            16,
            sf::Color(0, 255, 0),
            scoreTextPosition + sf::Vector2f(0, 30));
    }

    void Visit(const Feis::MiningMachineCell<TGameConfig> *cell) const override
    {
        drawer_->DrawRectangle(cellPosition_, sf::Color(128, 0, 0));

        auto numberCell =
            dynamic_cast<const Feis::NumberCell<TGameConfig> *>(backgroundCell_);

        if (numberCell)
        {
            drawer_->DrawText(
                std::to_string(numberCell->GetNumber()),
                TGameConfig::kCellSize * 0.8,
                sf::Color::White,
                cellPosition_,
                static_cast<Direction>((static_cast<int>(cell->GetDirection()) + 1) % 4));
        }
    }

    void Visit(const Feis::ConveyorCell<TGameConfig> *cell) const override
    {
        drawer_->DrawBorder(cellPosition_);
        drawer_->DrawRectangle(cellPosition_, sf::Color(128, 128, 128));
        drawer_->DrawArrow(cellPosition_, cell->GetDirection());
    }

    void Visit(const Feis::CombinerCell<TGameConfig> *cell) const override
    {
        drawer_->DrawBorder(cellPosition_);
    }

private:
    Drawer<TGameConfig> *drawer_;
    Feis::CellPosition cellPosition_;
    IBackgroundCell *backgroundCell_;
};
#endif