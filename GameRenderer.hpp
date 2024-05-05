#include "PDOGS.hpp"
#include "Drawer.hpp"
#include "CellStackRenderer.hpp"

template <typename TGameRendererConfig>
class GameRenderer 
{
public:
    using GameManagerConfig = Feis::GameManagerConfig;

    GameRenderer(sf::RenderWindow *window) : renderer_(window)
    {
    }

    void Render(const Feis::GameManager &gameManager)
    {
        renderer_.Clear();

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

        int timeLeft = gameManager.GetEndTime() - gameManager.GetElapsedTime();

        renderer_.DrawText(
            std::to_string(timeLeft / (TGameRendererConfig::kFPS * 60) / 10) +
            std::to_string(timeLeft / (TGameRendererConfig::kFPS * 60) % 10) + 
            ":" + 
            std::to_string(timeLeft / (TGameRendererConfig::kFPS) % 60 / 10) +
            std::to_string(timeLeft / (TGameRendererConfig::kFPS) % 10), 
            20,
            sf::Color::White,
            sf::Vector2f(50, 30));


        renderer_.Display();
    }

private:
    Drawer<TGameRendererConfig> renderer_;
    CellStackRenderer<TGameRendererConfig> cellStackRenderer_;
};
