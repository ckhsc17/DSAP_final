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
    using IGameManager = Feis::IGameManager;
    using CellPosition = Feis::CellPosition;

    void RenderPassOne(
        const IGameManager &gameManager,
        Drawer<TGameRendererConfig> &renderer, 
        CellPosition position) const
    {
        auto& cellStack = gameManager.GetCellStack(position);

        auto foreground = cellStack.GetForeground();
        auto background = cellStack.GetBackground();

        if (foreground)
        {
            CellRendererFirstPassVisitor<TGameRendererConfig> cellRenderer(&gameManager, &renderer, position, background.get());
            foreground->Accept(&cellRenderer);
            return;
        }

        if (background)
        {
            CellRendererFirstPassVisitor<TGameRendererConfig> cellRenderer(&gameManager, &renderer, position, background.get());
            background->Accept(&cellRenderer);
            return;
        }
        
        renderer.DrawBorder(position);
    }

    void RenderPassTwo(
        const IGameManager &gameManager,
        Drawer<TGameRendererConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto& cellStack = gameManager.GetCellStack(cellPosition);

        auto foreground = cellStack.GetForeground();

        if (foreground)
        {
            CellRendererSecondPassVisitor<TGameRendererConfig> cellRenderer(&gameManager, &drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }

    void RenderPassThree(
        const IGameManager &gameManager,
        Drawer<TGameRendererConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto& cellStack = gameManager.GetCellStack(cellPosition);

        auto foreground = cellStack.GetForeground();
        if (foreground)
        {
            CellRendererThirdPassVisitor<TGameRendererConfig> cellRenderer(&drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }
};
#endif