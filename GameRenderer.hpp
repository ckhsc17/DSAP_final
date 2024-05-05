#include "PDOGS.hpp"
#include "Drawer.hpp"
#include "CellStackRenderer.hpp"

template <typename TGameRendererConfig>
class GameRenderer 
{
public:
    using GameManagerConfig = Feis::GameManagerConfig;

    GameRenderer(sf::RenderWindow *window) : window_(window), renderer_(window, &font_)
    {
        font_.loadFromFile("/Library/Fonts/Arial Unicode.ttf");
    }

    void Render(const Feis::GameManager &gameManager)
    {
        window_->clear(sf::Color::Black);

        for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassOne(gameManager, renderer_, {row, col});
            }
        }

        for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassTwo(gameManager, renderer_, {row, col});
            }
        }

        for (int row = 0; row < GameManagerConfig::kBoardHeight; ++row)
        {
            for (int col = 0; col < GameManagerConfig::kBoardWidth; ++col)
            {
                cellStackRenderer_.RenderPassThree(gameManager, renderer_, {row, col});
            }
        }

        window_->display();
    }

private:
    sf::RenderWindow *window_;
    sf::Font font_;
    Drawer<TGameRendererConfig> renderer_;
    CellStackRenderer<TGameRendererConfig> cellStackRenderer_;
};
