#include "PDOGS.hpp"

void Test(int commonDividor, unsigned int seed);

void Test1A() { Test(1, 20); }
void Test1B() { Test(1, 825); }

void Test2A() { Test(2, 25); }
void Test2B() { Test(2, 123); }

void Test3A() { Test(3, 30); }
void Test3B() { Test(3, 321); }

void Test4A() { Test(4, 35); }
void Test4B() { Test(4, 12); }

void Test5A() { Test(5, 40); }
void Test5B() { Test(5, 39); }

int main() {
    int id;
    std::cin >> id;
    void (*f[])() = { Test1A, Test1B, Test2A, Test2B, Test3A, Test3B, Test4A, Test4B, Test5A, Test5B };
    f[id-1]();
}

// [YOUR CODE WILL BE PLACED HERE]

void Test(int commonDividor, unsigned int seed)
{
    GamePlayer player;
    Feis::GameManager gameManager(&player, commonDividor, seed);

    while (!gameManager.IsGameOver())
    {
        gameManager.Update();
    }
    
    std::cout << gameManager.GetScores() << std::endl;
}


