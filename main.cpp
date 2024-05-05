#include "PDOGS.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "GameRenderer.hpp"

using namespace Feis;

struct GameConfig {
    static constexpr int kFPS = 30;
};

struct GameRendererConfig
{
    static constexpr int kCellSize = 20;
    static constexpr int kBoardLeft = 20;
    static constexpr int kBoardTop = 60;
    static constexpr int kBorderSize = 1;
};

CellPosition GetMouseCellPosition(const sf::RenderWindow &window)
{
    const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
    const sf::Vector2i relatedMousePosition =
        mousePosition - sf::Vector2i(GameRendererConfig::kBoardLeft, GameRendererConfig::kBoardTop);
    return {relatedMousePosition.y / GameRendererConfig::kCellSize,
            relatedMousePosition.x / GameRendererConfig::kCellSize};
}

int main(int, char **)
{
    sf::VideoMode mode = sf::VideoMode(1280, 1024);

    sf::RenderWindow window(mode, "DSAP Final Project", sf::Style::Close);

    window.setFramerateLimit(GameConfig::kFPS);

    GameManager gameManager(3, 20);

    const std::map<sf::Keyboard::Key, PlayerAction> playerActionKeyboardMap = {
        {sf::Keyboard::J, PlayerAction::BuildLeftOutMiningMachine},
        {sf::Keyboard::I, PlayerAction::BuildTopOutMiningMachine},
        {sf::Keyboard::L, PlayerAction::BuildRightOutMiningMachine},
        {sf::Keyboard::K, PlayerAction::BuildBottomOutMiningMachine},
        {sf::Keyboard::D, PlayerAction::BuildLeftToRightConveyor},
        {sf::Keyboard::S, PlayerAction::BuildTopToBottomConveyor},
        {sf::Keyboard::A, PlayerAction::BuildRightToLeftConveyor},
        {sf::Keyboard::W, PlayerAction::BuildBottomToTopConveyor},
        {sf::Keyboard::Num1, PlayerAction::BuildTopOutCombiner},
        {sf::Keyboard::Num2, PlayerAction::BuildRightOutCombiner},
        {sf::Keyboard::Num3, PlayerAction::BuildBottomOutCombiner},
        {sf::Keyboard::Num4, PlayerAction::BuildLeftOutCombiner},
        {sf::Keyboard::Backspace, PlayerAction::Clear},
    };

    PlayerAction playerAction = PlayerAction::BuildLeftToRightConveyor;

    GameRenderer<GameRendererConfig> gameRenderer(&window);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::MouseButtonReleased)
            {
                CellPosition mouseCellPosition = GetMouseCellPosition(window);

                if (IsWithinBoard(mouseCellPosition))
                {
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        gameManager.DoAction(playerAction, mouseCellPosition);
                    }
                }
            }

            if (event.type == sf::Event::KeyReleased)
            {
                if (playerActionKeyboardMap.count(event.key.code))
                {
                    playerAction = playerActionKeyboardMap.at(event.key.code);
                }
            }
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        gameManager.Update();

        gameRenderer.Render(gameManager);  
    }
}
