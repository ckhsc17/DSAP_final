#include "PDOGS.hpp"

#include <iostream>
#include <fstream>

using namespace Feis;

class GamePlayerWithHistory : public IGamePlayer
{
public:
    GamePlayerWithHistory(const std::string &filename)
    {
        std::ifstream inFile(filename);
        PlayerAction action;
        int type;
        while (inFile >> action.cellPosition.row >> action.cellPosition.col >> type)
        {
            action.type = static_cast<PlayerActionType>(type);
            actions_.push(action);
        }
    }

    PlayerAction GetNextAction(const IGameInfo& info) 
    {
        if (actions_.empty())
        {
            return {PlayerActionType::None, {0, 0}};
        }
        const auto action = actions_.front();
        actions_.pop();
        return action;
    }
private:
    std::queue<PlayerAction> actions_;
};

int main()
{
    GamePlayerWithHistory player("gameplay.txt");
    GameManager gameManager(&player, 3, 20);

    while (!gameManager.IsGameOver())
    {
        gameManager.Update();
    }
    
    std::cout << gameManager.GetScores() << std::endl;
}