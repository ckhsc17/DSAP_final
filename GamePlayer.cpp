class GamePlayer : public Feis::IGamePlayer 
{
public:
    Feis::PlayerAction GetNextAction(const Feis::IGameInfo& info) override 
    {
        return {Feis::PlayerActionType::None, {0, 0}};
    }    
};