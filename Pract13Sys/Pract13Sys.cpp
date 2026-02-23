#include <vector>
#include <Windows.h>
#include <iostream>
#include <string>

using namespace std;

vector <HANDLE> Players;

HANDLE BOSS_Thread;

bool Game;

CRITICAL_SECTION Critical_Section;

HANDLE ATTACK_EVENT;

bool EVIL_WIN = false;


struct BossStats {

    long health = 900000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;

};

struct BossStatus
{
    BossStats BStats;
    DWORD LastAttack;
    DWORD LastSpecialAttack;
    bool missed;
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
    int id;
    PlayerStats PStats;
    bool IsAlive=true;
    DWORD LastAttack;
    DWORD LastSpecialAttack;
    char Status[64]="стою";
    bool Dodge;
    long DamageDeal;
};
struct Tops
{
    string name;
    long Damage;
};


vector <PlayerStatus> PData;

BossStatus _bossStatus;



DWORD WINAPI Player(LPVOID param)
{
    PlayerStatus* _playerStatus=(PlayerStatus*)param;


    while (_playerStatus->IsAlive && Game)
    {
        /*_playerStatus->PStats.health <= 0 ? _playerStatus->IsAlive = FALSE : _playerStatus->IsAlive = TRUE;*/
        if (_playerStatus->PStats.health <= 0)
        {
            _playerStatus->IsAlive = false;
            break;
        }

        DWORD TIME=GetTickCount();
        //WaitForSingleObject(ATTACK_EVENT, INFINITE);

        DWORD ULT = WaitForSingleObject(ATTACK_EVENT, 100);

        if (ULT==WAIT_OBJECT_0)
        {
            strcpy_s(_playerStatus->Status, "Ранен ультой, оглушён");
            Sleep(2000);
        }
        strcpy_s(_playerStatus->Status, "Жду");

        if (TIME - _playerStatus->LastAttack >= _playerStatus->PStats.attackCooldown * 1000)
        {
            //SetEvent(ATTACK_EVENT);
            EnterCriticalSection(&Critical_Section);

            strcpy_s(_playerStatus->Status, "Сражаюсь");

            _bossStatus.BStats.health -= _playerStatus->PStats.damage * (100 - _bossStatus.BStats.resist)/100;

            _playerStatus->DamageDeal += _playerStatus->PStats.damage * (100 - _bossStatus.BStats.resist) / 100;

            LeaveCriticalSection(&Critical_Section);
            //strcpy_s(_playerStatus->Status, "Жду");


            _playerStatus->LastAttack = TIME;
            Sleep(1000);

        }

        if (TIME - _playerStatus->LastSpecialAttack >= _playerStatus->PStats.specialCooldown * 1000)
        {
            //SetEvent(ATTACK_EVENT);
            EnterCriticalSection(&Critical_Section);

            strcpy_s(_playerStatus->Status, "ультую");

            _bossStatus.BStats.health -= _playerStatus->PStats.specialDamage * (100 - _bossStatus.BStats.resist) / 100;

            _playerStatus->DamageDeal += _playerStatus->PStats.specialDamage * (100 - _bossStatus.BStats.resist) / 100;

            LeaveCriticalSection(&Critical_Section);
            //strcpy_s(_playerStatus->Status, "Жду");

            _playerStatus->LastSpecialAttack = TIME;
            Sleep(1000);

        }


    }

    strcpy_s(_playerStatus->Status, "умер");


    return 0;
}

DWORD WINAPI Boss(LPVOID param)
{
    BossStatus* StatusBoss = (BossStatus*)param;

    while (_bossStatus.BStats.health>0 && Game)
    {
        DWORD TIME = GetTickCount();
        if (TIME - StatusBoss->LastAttack>=StatusBoss->BStats.attackCooldown * 1000)
        {
            EnterCriticalSection(&Critical_Section);
            int id = -1;
            int weakest=999999999;

            for (int i = 0; i < PData.size(); i++)
            {
                if (PData[i].IsAlive)
                {
                    if (PData[i].PStats.health < weakest)
                    {
                        weakest = PData[i].PStats.health;
                        id = i;
                    }
                }
            }
            int roll = rand() % 100;
            if (roll > PData[id].PStats.dodgeChance) {
                PData[id].PStats.health -= _bossStatus.BStats.damage * (100 - PData[id].PStats.defense) / 100;
                StatusBoss->missed = false;
                LeaveCriticalSection(&Critical_Section);
                StatusBoss->LastAttack = TIME;
            }
            else {
                StatusBoss->missed = true;
                LeaveCriticalSection(&Critical_Section);
            }



        }

        if (TIME - StatusBoss->LastSpecialAttack >= StatusBoss->BStats.specialCooldown * 1000)
        {
            SetEvent(ATTACK_EVENT);
            Sleep(2000);
            EnterCriticalSection(&Critical_Section);
            for (int i = 0; i < PData.size(); i++)
            {
                if (PData[i].IsAlive) 
                {
                    PData[i].PStats.health -= _bossStatus.BStats.specialDamage * (1-0.05 *(PData.size()-1));
                }
            }
            LeaveCriticalSection(&Critical_Section);
            ResetEvent(ATTACK_EVENT);
        }
    }
    return 0;
}

DWORD WINAPI DRAW(LPVOID param)
{
    while (Game)
    {
        int DeadPlayers = 0;
        for (int i = 0; i < PData.size(); i++)
        {
            if (!PData[i].IsAlive)
            {
                DeadPlayers += 1;
            }
        }
        if (DeadPlayers==PData.size() || _bossStatus.BStats.health <= 0)
        {
            Game = false;
            if (_bossStatus.BStats.health > 0)
            {
                EVIL_WIN = true;
            }
        }

        Sleep(200);
        int middle = Players.size() / 2;

        cout << "------------------------------------------------------------" << endl;

        EnterCriticalSection(&Critical_Section);
        for (int i = 0; i < Players.size(); i++)
        {
            cout << endl;
            cout << PData[i].PStats.name << " Урон: " << PData[i].PStats.damage << flush;

            /*PData[i].PStats.health <= 0 ? PData[i].IsAlive == false : PData[i].IsAlive == true;*/

            cout << "\nЗдоровье:" << PData[i].PStats.health<<" Статус: " << PData[i].Status << flush;

            if (i == middle)
            {
                cout << "\t\t хп Босса: " << _bossStatus.BStats.health << " урон босса" << (_bossStatus.BStats.damage <<_bossStatus.missed? " Промазал" : " Атаковал") << flush;
            }
            cout << endl;
        }
        LeaveCriticalSection(&Critical_Section);


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
    srand(time(0));

    ATTACK_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);

    Game = true;

    setlocale(0, "rus");

    InitializeCriticalSection(&Critical_Section);


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

        PData[i].PStats.attackCooldown = 3 + rand() % 9;

        PData[i].PStats.specialCooldown = 6 + rand() % 14;

        PData[i].LastAttack = GetTickCount() - PData[i].PStats.attackCooldown * 1000;
        PData[i].LastSpecialAttack = GetTickCount() - PData[i].PStats.specialCooldown * 1000;

        PData[i].id = i + 1;

        PData[i].PStats.health = 100000 + rand() % 300000;

        PData[i].PStats.damage = 10000 + rand() % 50000;

        PData[i].PStats.specialDamage = 20000 + rand() % 70000;

        PData[i].PStats.defense = 10 + rand() % 30;

        PData[i].PStats.dodgeChance = 10 + rand() % 30;

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
    _bossStatus.BStats.health = 500000 + rand() % 900000;

    _bossStatus.BStats.resist = 30 + rand() % 40;
    
    _bossStatus.BStats.damage = 50000 + rand() % 100000;

    _bossStatus.BStats.specialDamage = 100000 + rand() % 150000;

    _bossStatus.BStats.attackCooldown = 5 + rand() % 15;

    _bossStatus.BStats.specialCooldown = 15 + rand() % 30;

    _bossStatus.LastAttack = GetTickCount() - _bossStatus.BStats.attackCooldown * 1000;
    _bossStatus.LastSpecialAttack = GetTickCount() - _bossStatus.BStats.specialCooldown * 1000;



    BOSS_Thread = CreateThread(NULL, 0, Boss, &_bossStatus, 0, NULL);

    if (BOSS_Thread == NULL)
    {
        cout << "boss doesn't create  " << GetLastError();
        return GetLastError();
    }

    HANDLE Drawler = CreateThread(NULL, 0, DRAW, NULL, 0, NULL);
    SetThreadPriority(Drawler, THREAD_PRIORITY_HIGHEST);
    if (Drawler == NULL)
    {
        cout << "draw doesn't create  " << GetLastError();
        return GetLastError();
    }

    /*WaitForMultipleObjects(input, Players.data(), TRUE, INFINITE);
    WaitForSingleObject(BOSS_Thread, INFINITE);*/
    WaitForSingleObject(Drawler, INFINITE);

    cout << "ИГРА ЗАКОНЧЕНА" << endl;
    cout << "ПОБЕДИЛ" << (EVIL_WIN ? " БОСС" : "И ИГРОКИ") << endl;

    vector <Tops> results;
    string NAME="";
    long max = -1;
    int id = -1;

    while (!PData.empty())
    {
        for (int i = 0; i < PData.size(); i++)
        {
            if (PData[i].DamageDeal > max) 
            {
                max = PData[i].DamageDeal;
                NAME = PData[i].PStats.name;
                id = i;
            }
        }
        Tops newTop;
        newTop.name = NAME;
        newTop.Damage = max;

        results.push_back(newTop);
        PData.erase(PData.begin() + id);

        NAME = -1;
        max = -1;
    }
    
    for (int i = 0; i < results.size(); i++)
    {
        cout << "ТОП-" << i + 1 <<"     " << results[i].name << "\n     Нанесенный урон:" << results[i].Damage << "\n" << endl;
    }

    DeleteCriticalSection(&Critical_Section);
    return 0;
    
}

