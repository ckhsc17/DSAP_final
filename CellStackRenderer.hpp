#ifndef CELL_STACK_RENDERER_HPP
#define CELL_STACK_RENDERER_HPP
#include "Drawer.hpp"
#include "CellRendererFirstPassVisitor.hpp"
#include "CellRendererSecondPassVisitor.hpp"
#include "CellRendererThirdPassVisitor.hpp"

template<typename TGameConfig>
class CellStackRenderer
{
public:
    using CellPosition = Feis::CellPosition;

    void RenderPassOne(
        const Feis::CellStack<TGameConfig> &cellStack,
        Drawer<TGameConfig> &renderer, 
        CellPosition position) const
    {
        auto foreground = cellStack.GetForeground();
        auto background = cellStack.GetBackground();

        if (foreground)
        {
            CellRendererFirstPassVisitor<TGameConfig> cellRenderer(&renderer, position, background.get());
            foreground->Accept(&cellRenderer);
            return;
        }

        if (background)
        {
            CellRendererFirstPassVisitor<TGameConfig> cellRenderer(&renderer, position, background.get());
            background->Accept(&cellRenderer);
            return;
        }
        
        renderer.DrawBorder(position);
    }

    void RenderPassTwo(
        const Feis::CellStack<TGameConfig> &cellStack,
        Drawer<TGameConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto foreground = cellStack.GetForeground();
        if (foreground)
        {
            CellRendererSecondPassVisitor<TGameConfig> cellRenderer(&drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }

    void RenderPassThree(
        const Feis::CellStack<TGameConfig> &cellStack,
        Drawer<TGameConfig> &drawer,
        CellPosition cellPosition) const
    {
        auto foreground = cellStack.GetForeground();
        if (foreground)
        {
            CellRendererThirdPassVisitor<TGameConfig> cellRenderer(&drawer, cellPosition);
            foreground->Accept(&cellRenderer);
        }
    }
};
#endif