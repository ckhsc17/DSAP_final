#ifndef CELL_STACK_RENDERER_HPP
#define CELL_STACK_RENDERER_HPP
#include "Drawer.hpp"
#include "CellRendererFirstPassVisitor.hpp"
#include "CellRendererSecondPassVisitor.hpp"
#include "CellRendererThirdPassVisitor.hpp"

template<typename TGameRendererConfig>
class CellStackRenderer
{
public:
    using IGameInfo = Feis::IGameInfo;
    using CellPosition = Feis::CellPosition;

    void RenderPassOne(
        const IGameInfo &info,
        Drawer<TGameRendererConfig> &renderer, 
        CellPosition position) const
    {
        auto& cellStack = info.GetCellStack(position);

        auto foreground = cellStack.GetForeground();
        auto background = cellStack.GetBackground();

        if (foreground)
        {
            CellRendererFirstPassVisitor<TGameRendererConfig> cellRenderer(&info, &renderer, position, background.get());
            foreground->Accept(&cellRenderer);
            return;
        }

        if (background)
        {
            CellRendererFirstPassVisitor<TGameRendererConfig> cellRenderer(&info, &renderer, position, background.get());
            background->Accept(&cellRenderer);
            return;
        }
        
        renderer.DrawBorder(position);
    }

    void RenderPassTwo(
        const IGameInfo &info,
        Drawer<TGameRendererConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto& cellStack = info.GetCellStack(cellPosition);

        auto foreground = cellStack.GetForeground();

        if (foreground)
        {
            CellRendererSecondPassVisitor<TGameRendererConfig> cellRenderer(&info, &drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }

    void RenderPassThree(
        const IGameInfo &info,
        Drawer<TGameRendererConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto& cellStack = info.GetCellStack(cellPosition);

        auto foreground = cellStack.GetForeground();
        if (foreground)
        {
            CellRendererThirdPassVisitor<TGameRendererConfig> cellRenderer(&info, &drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }
};
#endif