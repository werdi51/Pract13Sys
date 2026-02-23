#include <vector>
#include <Windows.h>
#include <iostream>
#include <string>
//сделать список тех кто получил урон

using namespace std;

vector <HANDLE> Players;

HANDLE BOSS_Thread;

bool Game;


struct BossStats {

    long health = 9000000000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;

};

struct PlayerStats {

    long health = 500000;
    int damage = 12000;
    int specialDamage = 30000;
    int attackCooldown = 2;
    int specialCooldown = 5;
    int defense = 20;
    int dodgeChance = 15;
    char name[64];

};

struct PlayerStatus {
    PlayerStats PStats;
    bool IsAlive=true;
    DWORD LastAttack;
    DWORD LastSpecialAttack;
};


vector <PlayerStatus> PData;

BossStats _bossStats;

DWORD WINAPI Player(LPVOID param)
{
    PlayerStatus* _playerStatus=(PlayerStatus*)param;

    

    while (_playerStatus->IsAlive)
    {
        _playerStatus->PStats.health <= 0 ? _playerStatus->IsAlive = FALSE : _playerStatus->IsAlive = TRUE;
        DWORD TIME=GetTickCount();

        if (TIME - _playerStatus->LastAttack >= _playerStatus->PStats.attackCooldown)
        {

        }

        if (TIME - _playerStatus->LastSpecialAttack >= _playerStatus->PStats.specialCooldown)
        {

        }

    }

    return 0;
}

DWORD WINAPI Boss(LPVOID param)
{

    return 0;
}

DWORD WINAPI DRAW(LPVOID param)
{

    while (Game)
    {
        Sleep(200);

        int middle = Players.size() / 2;

        cout << "------------------------------------------------------------" << endl;

        for (int i = 0; i < Players.size(); i++)
        {
            cout << endl;
            cout << PData[i].PStats.name << " Урон: " << PData[i].PStats.damage;

            /*PData[i].PStats.health <= 0 ? PData[i].IsAlive == false : PData[i].IsAlive == true;*/

            cout << "\nЗдоровье:" << PData[i].PStats.health;

            if (i == middle)
            {
                cout << "\t\t хп Босса: " << _bossStats.health << " урон босса" << _bossStats.damage;
            }
            cout << endl;
        }

    }

    return 0;
}

//DWORD WINAPI ATTACK(LPVOID param)
//{
//    return 0;
//}
//
//DWORD WINAPI SPECIAL_ATTACK(LPVOID param)
//{
//    return 0;
//}

int main()
{
    Game = true;

    setlocale(0, "rus");


    /*std::string input;*/
    int input;

    std::cout << "Добро пожаловать в игру\n" << std::endl;
    std::cout << "Введите количество игроков : ";

    //BossStats BOSS_Initialiasing;

    std::cin >> input;

    PData.resize(input);
    for (int i = 0; i < input; i++)
    {
        string Pname = "Player " + to_string(i + 1);

        PData[i].LastAttack = GetTickCount() - PData[i].PStats.attackCooldown * 1000;
        PData[i].LastSpecialAttack = GetTickCount() - PData[i].PStats.specialCooldown * 1000;

        strcpy_s(PData[i].PStats.name, Pname.c_str());
    }

    for (int i = 0; i < input; i++)
    {
        HANDLE player = CreateThread(NULL, 0, Player, &PData[i], 0, NULL);

        if (player == NULL)
        {
            cout << "doesn't create " << GetLastError();
            return GetLastError();
        }

        Players.push_back(player);

    }

    BOSS_Thread = CreateThread(NULL, 0, Boss, NULL, 0, NULL);

    if (BOSS_Thread == NULL)
    {
        cout << "boss doesn't create  " << GetLastError();
        return GetLastError();
    }

    HANDLE Drawler = CreateThread(NULL, 0, DRAW, NULL, 0, NULL);
    if (Drawler == NULL)
    {
        cout << "draw doesn't create  " << GetLastError();
        return GetLastError();
    }

    /*WaitForMultipleObjects(input, Players.data(), TRUE, INFINITE);
    WaitForSingleObject(BOSS_Thread, INFINITE);*/

    WaitForSingleObject(Drawler, INFINITE);

    cout << "бб" << endl;
    return 0;
    
}

