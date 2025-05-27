// [// [YOUR CODE WILL BE PLACED HERE]
// scanning version
// while true 減少return{0,0}的次數
// 要蓋combinor或許勢必要用到clear（判斷完打掉原本的蓋combinor）
// 或是只要不是倍數的就直接先蓋combinor碰運氣？

// global variable to record action
// declare a global direction enum struct to record the current direction we want to put
// declare a global position variable to record the current position
// declare a global "radius" variable to circularly iterate / rotate out from the CollectionCenterCell

// 
int x = Feis::GameManagerConfig::kBoardWidth / 2 - Feis::GameManagerConfig::kGoalSize / 2;
int y = Feis::GameManagerConfig::kBoardHeight / 2 - Feis::GameManagerConfig::kGoalSize / 2;

bool flag2 = true;

int combinor = 0;
bool flag = true;
int TARGET = 0;

bool waiting = true; //waiting to build a new miningCell
int direction; //1:left 2:top 3:right 4:bottom
Feis::CellPosition offset;
Feis::CellPosition current;
bool removeCell = false;


//std::queue<Feis::LayeredCell> numberCellsQueue;
std::queue<Feis::CellPosition> numberCellsQueue;
std::vector<std::pair<Feis::CellPosition, int>> numberCellsWithDistance;

//store direction information
int** directions = new int*[36];

//準備起飛
int** passedNumCells = new int*[36];

class GamePlayer : public Feis::IGamePlayer 
{
public:

    int CalculateDistance(const Feis::CellPosition& pos) {
        // Calculate Manhattan distance from CollectionCenterCell
        return std::abs(pos.col - x) + std::abs(pos.row - y);
    }

    int DecideDirection(const Feis::CellPosition& pos, const Feis::IGameInfo& info) {
        if (flag2) {
            //std::cout << "info in DecideDirection: " << &info << std::endl;
            flag2 = false;
        }
        Feis::CellPosition offset;
        const Feis::GameManager& gameManager = dynamic_cast<const Feis::GameManager&>(info);
        
        //如果三邊都被擋住就把前面的東西覆蓋掉（尚未實作）
        //std::vector<Feis::CellPosition> offsets;

        //邊界處理
        if (pos.col >= 61) return 1;
        if (pos.col <= 0) return 3;
        if (pos.row <= 0) return 4;
        if (pos.row >= 35) return 2;

        //這步可以再優化但先這樣
        const Feis::LayeredCell& rightCell = gameManager.GetLayeredCell(pos + Feis::CellPosition {0,1});
        const Feis::LayeredCell& leftCell = gameManager.GetLayeredCell(pos + Feis::CellPosition {0,-1});
        const Feis::LayeredCell& upCell = gameManager.GetLayeredCell(pos + Feis::CellPosition {-1,0});
        const Feis::LayeredCell& downCell = gameManager.GetLayeredCell(pos + Feis::CellPosition {1,0});
        //auto foregroundCell = currentCell.GetForeground();
        //auto backgroundCell = currentCell.GetBackground();

        //auto rightMiner = std::dynamic_pointer_cast<Feis::MiningMachineCell>(rightCell.GetForeground());
        //auto leftMiner = std::dynamic_pointer_cast<Feis::MiningMachineCell>(leftCell.GetForeground());
        //auto upMiner = std::dynamic_pointer_cast<Feis::MiningMachineCell>(upCell.GetForeground());
        //auto downMiner = std::dynamic_pointer_cast<Feis::MiningMachineCell>(downCell.GetForeground());

        //根據current所在位置決定預設方向
        //left
        if (pos.col < x) {
            //直接用vector記錄所有wallCell位置會更快嗎？
            //std::cout << "hi1" << std::endl;
            
            //segmentation fault / bus error, 因為碰到CollectionCenterCell!
            //查看右邊的cell
            //const Feis::LayeredCell& currentCell = gameManager.GetLayeredCell(pos + Feis::CellPosition {1,0});
            
            //wallCell or numberCell blocking the way, filtering out collectionCenterCell
            if (x <= pos.col+1 && pos.col+1 <= x+3 && y <= pos.row && pos.row <= y+3) return 3; //加個特判避免他直接閃掉CollectionCenter
            if ((directions[pos.row][pos.col+1] == 1) || (rightCell.GetForeground() != nullptr && !rightCell.GetForeground()->CanRemove()) || (rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                //std::cout << "left turned" << std::endl;
                //可能中間會有很多對向的輸送帶（？
                if (pos.row <= 17) {
                    if ((directions[pos.row+1][pos.col] == 2) || (downCell.GetForeground() != nullptr && !downCell.GetForeground()->CanRemove()) || (downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row-1][pos.col] == 4) || (upCell.GetForeground() != nullptr && !upCell.GetForeground()->CanRemove()) || (upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine (這個判斷有可能吃掉conveyor)
                            if ((rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 3;
                            }
                            if ((downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 4;
                            }
                            waiting = true; //暫時放棄這條路線
                        }
                        return 2;
                    }
                    return 4;
                }
                else if (pos.row >= 18) {
                    if ((directions[pos.row-1][pos.col] == 4) || (upCell.GetForeground() != nullptr && !upCell.GetForeground()->CanRemove()) || (upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row+1][pos.col] == 2) || (downCell.GetForeground() != nullptr && !downCell.GetForeground()->CanRemove()) || (downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆或路而是之前蓋的miningMachine
                            if ((rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 3;
                            }
                            if ((upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 2;
                            }
                            waiting = true; //暫時放棄這條路線
                        }
                        return 4;
                    }
                    return 2;
                }
            }
            return 3;
        }
        //right
        else if (pos.col > x+3) {
            //查看左邊的cell
            
            if (x <= pos.col-1 && pos.col-1 <= x+3 && y <= pos.row && pos.row <= y+3) return 1;
            if ((directions[pos.row][pos.col-1] == 3) || (leftCell.GetForeground() != nullptr && !leftCell.GetForeground()->CanRemove()) || (leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                
                if (pos.row <= 17) {    //std::cout << "right turned" << std::endl;
                    if ((directions[pos.row+1][pos.col] == 2) || (downCell.GetForeground() != nullptr && !downCell.GetForeground()->CanRemove()) || (downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row-1][pos.col] == 4) || (upCell.GetForeground() != nullptr && !upCell.GetForeground()->CanRemove()) || (upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 1;
                            }
                            if ((downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 4;
                            }
                            waiting = true; //暫時放棄這條路線
                        }
                        return 2;
                    }
                    return 4;
                }
            
                else if (pos.row >= 18) {
                    if ((directions[pos.row-1][pos.col] == 4) || (upCell.GetForeground() != nullptr && !upCell.GetForeground()->CanRemove()) || (upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row+1][pos.col] == 2) || (downCell.GetForeground() != nullptr && !downCell.GetForeground()->CanRemove()) || (downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 1;
                            }
                            if ((upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 2;
                            }
                            waiting = true; //暫時放棄這條路線
                        }
                        return 4;
                    }
                    return 2;
                }
            }
            return 1;
        }
        //up
        else if (pos.row < y) {
            if (x <= pos.col && pos.col <= x+3 && y <= (pos.row)+1 && (pos.row)+1 <= y+3) return 4;
            
            if ((directions[pos.row+1][pos.col] == 2) || (downCell.GetForeground() != nullptr && !downCell.GetForeground()->CanRemove()) || (downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                if (pos.col <= 30) {
                    if ((directions[pos.row][pos.col+1] == 1) || (rightCell.GetForeground() != nullptr && !rightCell.GetForeground()->CanRemove()) || (rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row][pos.col-1] == 3) || (leftCell.GetForeground() != nullptr && !leftCell.GetForeground()->CanRemove()) || (leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 4;
                            }
                            if ((rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 3;
                            }
                            waiting = true;
                        }
                        return 1;
                    }
                    return 3; //可再想想
                }

                else if (pos.col >= 31) {
                    if ((directions[pos.row-1][pos.col] == 3) || (leftCell.GetForeground() != nullptr && !leftCell.GetForeground()->CanRemove()) || (leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row][pos.col+1] == 1) || (rightCell.GetForeground() != nullptr && !rightCell.GetForeground()->CanRemove()) || (rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((downCell.GetForeground() != nullptr && downCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 4;
                            }
                            if ((leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 1;
                            }
                            waiting = true;
                        }
                        return 3;
                    }
                    return 1;
                }
            }
            return 4;
        }
        //down
        else if (pos.row > y+3) {
            if (x <= pos.col && pos.col <= x+3 && y <= pos.row-1 && pos.row-1 <= y+3) return 2;
            if ((directions[pos.row-1][pos.col] == 4) || (upCell.GetForeground() != nullptr && !upCell.GetForeground()->CanRemove()) || (upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                if (pos.col <= 30) {
                    if ((directions[pos.row][pos.col+1] == 1) || (rightCell.GetForeground() != nullptr && !rightCell.GetForeground()->CanRemove()) || (rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row][pos.col-1] == 3) || (leftCell.GetForeground() != nullptr && !leftCell.GetForeground()->CanRemove()) || (leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 2;
                            }
                            if ((rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 3;
                            }
                            waiting = true; //方向上還是牆那就先真的算了
                        }
                        return 1;
                    }
                    return 3;
                }

                else if (pos.col >= 31) {
                    if ((directions[pos.row][pos.col-1] == 4) || (leftCell.GetForeground() != nullptr && !leftCell.GetForeground()->CanRemove()) || (leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                        if ((directions[pos.row][pos.col+1] == 4) || (rightCell.GetForeground() != nullptr && !rightCell.GetForeground()->CanRemove()) || (rightCell.GetForeground() != nullptr && rightCell.GetBackground() != nullptr)) {
                            //如果該方向上不是牆而是之前蓋的miningMachine
                            if ((upCell.GetForeground() != nullptr && upCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 2;
                            }
                            if ((leftCell.GetForeground() != nullptr && leftCell.GetBackground() != nullptr)) {
                                removeCell = true;
                                return 1;
                            }
                            waiting = true; //方向上還是牆那就先真的算了
                        }
                        return 3;
                    }
                    return 1;
                }
            }
            return 2;
        }
    }

    // Can return 9000 times at most
    Feis::PlayerAction GetNextAction(const Feis::IGameInfo& info) override 
    {
        std::string tar = info.GetLevelInfo(); 
        TARGET = tar[1] - '0';
        // Extract game information from IGameInfo interface
        const Feis::GameManager& gameManager = dynamic_cast<const Feis::GameManager&>(info);

        //先在固定位置蓋combinorCell (再退一步，先管左右兩大區就好)
        /*
        if (TARGET != 1 && combinor < 4) {
            combinor += 1;
            if (combinor == 1) {
                return {Feis::PlayerActionType::BuildRightOutCombiner, {17,28}};
            }
            else if (combinor == 2) {
                return {Feis::PlayerActionType::BuildLeftOutCombiner, {17,33}};
            }
            else if (combinor == 3) {
                return {Feis::PlayerActionType::BuildTopOutCombiner, {20,30}};
            }
            else {
                return {Feis::PlayerActionType::BuildBottomOutCombiner, {15,29}};
            }
            
        }
        else std::cout << "combinor built" << std::endl;

        */
        //skillfully scans the GameBoard
        //先暫時暴力掃
        if (flag) {
            //std::cout << "info: " << &info << std::endl;
            // initialize directions
            for (int i = 0; i < 62; i++) {
                directions[i] = new int[62];
            }
            for (int i = 0; i < 36; i++) {
                for (int j = 0; j < 62; j++) {
                    directions[i][j] = 0;
                }
            }

            //if (TARGET != 1) x -= 1;
            
            // Scan the entire board
            // 先看哪一區有比較多可以配對的？
            for (int i = 0; i < Feis::GameManagerConfig::kBoardHeight; ++i) {
                for (int j = 0; j < Feis::GameManagerConfig::kBoardWidth; ++j) {
                    if (y <= i && i <= y+3 && x <= j && j <= x+3) continue;
                    Feis::CellPosition pos{i, j}; //i:row(y座標) j:column(x座標)
                    const Feis::LayeredCell& cell = gameManager.GetLayeredCell(pos);
                    
                    auto foregroundCell = cell.GetForeground();
                    auto backgroundCell = cell.GetBackground();

                    auto numberCell = std::dynamic_pointer_cast<Feis::NumberCell>(backgroundCell);
                    //&& numberCell->GetNumber() % TARGET == 0
                    if (numberCell && backgroundCell->CanBuild() && numberCell->GetNumber() % TARGET == 0) {
                        int distance = CalculateDistance(pos);
                        numberCellsWithDistance.push_back({pos, distance});
                    }
                    /*
                    else if (numberCell && backgroundCell->CanBuild()) {
                        if (TARGET == 3 && numberCell->GetNumber() != 7) {
                            int distance = CalculateDistance(pos);
                            numberCellsWithDistance.push_back({pos, distance});
                        }
                        //4: 一邊(3,7,11)，一邊(1,5) / 或2+2
                        else if (TARGET == 4) {
                            int distance = CalculateDistance(pos);
                            numberCellsWithDistance.push_back({pos, distance});
                        }
                        //5可以再想想
                    }
                    */
                }
            }

            // Sort cells by distance to CollectionCenterCell (closer cells first)
            std::sort(numberCellsWithDistance.begin(), numberCellsWithDistance.end(), [](const auto& a, const auto& b) {
                return a.second < b.second;
            });

            // Push sorted cells into the queue
            for (const auto& entry : numberCellsWithDistance) {
                numberCellsQueue.push(entry.first);
            }

            /*
            while (!numberCellsQueue.empty()) {
                std::cout << "numberCell position: " << numberCellsQueue.front().col << " " << numberCellsQueue.front().row << std::endl;
                numberCellsQueue.pop();
            }
            */

           
            

            flag = false;
            
        }
        //std::cout << x << " " << y;
        //return {Feis::PlayerActionType::BuildRightOutMiningMachine, Feis::CellPosition {17,28}};


        // Get current layered cell at the central position
        // build miningMachine
        if (waiting) {
            if (numberCellsQueue.empty()) {
                //std::cout << "all numberCells processed" << std::endl;
                return {Feis::PlayerActionType::None, {0,0}};
            }
            current = numberCellsQueue.front();
            //std::cout << "current numberCell: " << current.col << " " << current.row << std::endl;
            direction = DecideDirection(current, info);
            //std::cout << "hi direction: " << direction << std::endl;

            directions[current.row][current.col] = direction;

            numberCellsQueue.pop();
            waiting = false;

            //萬一走到死亡三角裡？ ->先讓他走進去，但設一個remove = true
            if (removeCell) {
                removeCell = false;
                offset = Feis::CellPosition {0,0}; // reset for removing cases
                return
                 {Feis::PlayerActionType::Clear, current};
            }

            switch (direction) {
                case 1: {
                    //left
                    //offset = Feis::CellPosition {-1,0};
                    offset = Feis::CellPosition {0,-1};
                    return {Feis::PlayerActionType::BuildLeftOutMiningMachine, current};
                }
                case 2: {
                    //top
                    offset = Feis::CellPosition {-1,0};
                    return {Feis::PlayerActionType::BuildTopOutMiningMachine, current};
                }
                case 3: {
                    //right
                    offset = Feis::CellPosition {0,1}; //y,x
                    return {Feis::PlayerActionType::BuildRightOutMiningMachine, current};
                }
                case 4: {
                    //down
                    offset = Feis::CellPosition {1,0};
                    return {Feis::PlayerActionType::BuildBottomOutMiningMachine, current};
                }
            }
            
            //std::cout << "machine built" << std::endl;
        }
        // build conveyorCell
        else {
            current += offset;

            const Feis::LayeredCell& currentCell = gameManager.GetLayeredCell(current);
            auto foregroundCell = currentCell.GetForeground();
            auto backgroundCell = currentCell.GetBackground();
            //std::cout << "conveyor current: " << current.col << " " << current.row << std::endl;

            // Arrive at collection center
            if (x <= current.col && current.col <= x+3 && y <= current.row && current.row <= y+3) {
                waiting = true;
                //return {Feis::PlayerActionType::None, {0,0}};
            }
            
            
            
            


            //萬一走到死亡三角裡？ ->先讓他走進去，但設一個remove = true
            //刪除當下那個位置的cell，將offset設成0,0，下一輪重新判斷
            if (removeCell) {
                //std::cout << "removeCell here" << std::endl;
                removeCell = false;
                offset = Feis::CellPosition {0,0}; // reset for removing cases
                return {Feis::PlayerActionType::Clear, current};
            }
            
            //注意放在removeCell後面判斷
            //如果已經存在conveyorCell或反正上面有東西了 ->代表轉向後已經接上其他輸送鏈了，或一些邊界例外
            if (foregroundCell != nullptr) {
                waiting = true;
            } 

            //std::cout << "hi5" << std::endl;
            direction = DecideDirection(current, info);
            //std::cout << "hi direction: " << direction << std::endl;

            directions[current.row][current.col] = direction;

            switch (direction) {
                case 1: {
                    //left
                    offset = Feis::CellPosition {0,-1}; //下一個尋找的方向
                    return {Feis::PlayerActionType::BuildRightToLeftConveyor, current};
                }
                case 2: {
                    offset = Feis::CellPosition {-1,0};
                    return {Feis::PlayerActionType::BuildBottomToTopConveyor, current};
                }
                case 3: {
                    offset = Feis::CellPosition {0,1}; //y,x
                    return {Feis::PlayerActionType::BuildLeftToRightConveyor, current};
                }
                case 4: {
                    offset = Feis::CellPosition {1,0};
                    return {Feis::PlayerActionType::BuildTopToBottomConveyor, current};
                }
            }

            
        }

        
        
        //處理十字以外的其他座標

        //int x = CollectionCenterConfig::kLeft;
        // explore the position on the left of the CollectionCenterCell first, then go to top, right, bottom
        // from the position of CollectionCenterCell, use GetNeighborCellPosition() and the current direction (e.g. (-1,0) for left first) to access the neighbor cell
        // After accessing a position on the grid, consider the following:
        // if (is NumberCell) -> put a MiningMachineCell on pointing at the CollectionCenterCell's direction; also check if there is a wallCell or another NumberCell immediately blocking the way; if yes, change the direction (for example if originally the cell was going right, turn to up or down)
        // else if (is WallCell) -> skip
        // else -> it is an empty cell, then put a ConveyorCell on it pointing at the CollectionCell; also check if there is a wallCell or another NumberCell immediately blocking the way; if yes, change the direction (for example if originally the cell was going right, turn to up or down)
        // update the cellPosition to access the next cell
        //return {Feis::PlayerActionType::None, Feis::CellPosition {0,0}};
    }    
};

// [YOUR CODE WILL BE PLACED HERE]



