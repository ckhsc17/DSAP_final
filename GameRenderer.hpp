#include "PDOGS.hpp"
#include "Drawer.hpp"
#include "CellStackRenderer.hpp"

template <typename TGameConfig>
class GameRenderer 
{
public:
    GameRenderer(sf::RenderWindow *window) : window_(window), renderer_(window, &font_)
    {
        font_.loadFromFile("/Library/Fonts/Arial Unicode.ttf");
    }

    void Render(const Feis::GameManager<TGameConfig> &gameManager)
    {
        window_->clear(sf::Color::Black);

        for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassOne(gameManager.GetCellStack(row, col), renderer_, {row, col});
            }
        }

        for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassTwo(gameManager.GetCellStack(row, col), renderer_, {row, col});
            }
        }

        for (int row = 0; row < TGameConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < TGameConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassThree(gameManager.GetCellStack(row, col), renderer_, {row, col});
            }
        }

        window_->display();
    }

private:
    sf::RenderWindow *window_;
    sf::Font font_;
    Drawer<TGameConfig> renderer_;
    CellStackRenderer<TGameConfig> cellStackRenderer_;
};
