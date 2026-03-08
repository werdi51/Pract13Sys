#include <vector>
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;

vector <HANDLE> Players;
HANDLE BOSS_Thread;

bool Game;
CRITICAL_SECTION Critical_Section;          

HANDLE BOSS_SPECIAL_EVENT, BOSS_ATTACK, PLAYER_ATTACK, PLAYER_SPECIAL;
bool EVIL_WIN = false;


struct BossStats {
    long health = 900000;
    int resist = 44;
    int damage = 73843;
    int specialDamage = 150000;
    int attackCooldown = 5;
    int specialCooldown = 10;
};

struct BossStatus {
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
    bool IsAlive = true;
    DWORD LastAttack;
    DWORD LastSpecialAttack;
    char Status[64] = "стою";
    bool Dodge;
    long DamageDeal = 0;
};

struct Tops {
    string name;
    long Damage;
};

vector <PlayerStatus> PData;
BossStatus _bossStatus;

vector<bool> playerAttackTotal;
vector<bool> playerSpecialTotal;
vector<long> playerDamageDealt;       
vector<long> playerSpecialDamageDealt; 

int bossTargetId = -1;


void PlayerToBoss(int playerIndex) {

    if (playerAttackTotal[playerIndex]) {

        long dmg = playerDamageDealt[playerIndex];
        _bossStatus.BStats.health -= dmg;

        PData[playerIndex].DamageDeal += dmg;
        playerAttackTotal[playerIndex] = false;

    }

    if (playerSpecialTotal[playerIndex]) {

        long dmg = playerSpecialDamageDealt[playerIndex];
        _bossStatus.BStats.health -= dmg;

        PData[playerIndex].DamageDeal += dmg;

        playerSpecialTotal[playerIndex] = false;


    }


}


void BossToPlayer(PlayerStatus* player, BossStatus* boss) {

    int roll = rand() % 100;

    if (roll > player->PStats.dodgeChance) {

        player->PStats.health -= boss->BStats.damage *(100 - player->PStats.defense) / 100;

        strcpy_s(player->Status, "Атакован боссом");
    }
    else {
        strcpy_s(player->Status, "Увернулся от атаки");
    }
}

void BossToPlayers(PlayerStatus* player, BossStatus* boss) {

    long dmg = boss->BStats.specialDamage *(1 - 0.05 * (PData.size() - 1));

    player->PStats.health -= dmg;


    strcpy_s(player->Status, "Ранен ультой");

}


DWORD WINAPI Player(LPVOID param) {
    PlayerStatus* _playerStatus = (PlayerStatus*)param;
    int idx = _playerStatus->id - 1;

    HANDLE waitHandles[2] = { BOSS_ATTACK, BOSS_SPECIAL_EVENT };

    while (_playerStatus->IsAlive && Game) {
        DWORD currentTime = GetTickCount();

        DWORD nextAttack = _playerStatus->LastAttack + _playerStatus->PStats.attackCooldown * 1000;
        DWORD waitAttack = (nextAttack > currentTime) ? (nextAttack - currentTime) : 0;

        DWORD nextSpecial = _playerStatus->LastSpecialAttack + _playerStatus->PStats.specialCooldown * 1000;
        DWORD waitSpecial = (nextSpecial > currentTime) ? (nextSpecial - currentTime) : 0;

        DWORD waitTime = min(waitAttack, waitSpecial);
        DWORD result = WaitForMultipleObjects(2, waitHandles, FALSE, waitTime);

        if (result == WAIT_OBJECT_0) {        
            int target = bossTargetId;
            if (target == _playerStatus->id) {
                BossToPlayer(_playerStatus, &_bossStatus);   
            }
        }
        else if (result == WAIT_OBJECT_0 + 1) {
            BossToPlayers(_playerStatus, &_bossStatus);     
        }
        else if (result == WAIT_TIMEOUT) {
            currentTime = GetTickCount();
            bool attacked = false;

            // Обычная
            if (currentTime - _playerStatus->LastAttack >=
                _playerStatus->PStats.attackCooldown * 1000) {

                long dmg = _playerStatus->PStats.damage *
                    (100 - _bossStatus.BStats.resist) / 100;

                EnterCriticalSection(&Critical_Section);
                playerAttackTotal[idx] = true;
                playerDamageDealt[idx] = dmg;
                LeaveCriticalSection(&Critical_Section);

                SetEvent(PLAYER_ATTACK);
                _playerStatus->LastAttack = currentTime;

                strcpy_s(_playerStatus->Status, "Атакую");
                attacked = true;
            }

            // Спец
            if (currentTime - _playerStatus->LastSpecialAttack >=
                _playerStatus->PStats.specialCooldown * 1000) {
                long dmg = _playerStatus->PStats.specialDamage *
                    (100 - _bossStatus.BStats.resist) / 100;

                EnterCriticalSection(&Critical_Section);
                playerSpecialTotal[idx] = true;
                playerSpecialDamageDealt[idx] = dmg;
                LeaveCriticalSection(&Critical_Section);

                SetEvent(PLAYER_ATTACK);
                _playerStatus->LastSpecialAttack = currentTime;
                strcpy_s(_playerStatus->Status, "Ультую");
                attacked = true;
            }

            if (!attacked) {
                Sleep(0);
            }
        }
        else {
            break;
        }

        if (_playerStatus->PStats.health <= 0) {
            _playerStatus->IsAlive = false;
            strcpy_s(_playerStatus->Status, "Умер");
        }
    }
    return 0;
}

DWORD WINAPI Boss(LPVOID param) {
    BossStatus* StatusBoss = (BossStatus*)param;

    while (_bossStatus.BStats.health > 0 && Game) {
        DWORD currentTime = GetTickCount();

        DWORD nextAttack = StatusBoss->LastAttack + StatusBoss->BStats.attackCooldown * 1000;
        DWORD waitAttack = (nextAttack > currentTime) ? (nextAttack - currentTime) : 0;

        DWORD nextSpecial = StatusBoss->LastSpecialAttack + StatusBoss->BStats.specialCooldown * 1000;
        DWORD waitSpecial = (nextSpecial > currentTime) ? (nextSpecial - currentTime) : 0;

        DWORD waitTime = min(waitAttack, waitSpecial);
        DWORD result = WaitForSingleObject(PLAYER_ATTACK, waitTime);

        if (result == WAIT_OBJECT_0) {
            EnterCriticalSection(&Critical_Section);
            for (size_t i = 0; i < PData.size(); ++i) {
                PlayerToBoss(i);   
            }
            LeaveCriticalSection(&Critical_Section);
        }
        else if (result == WAIT_TIMEOUT) {
            currentTime = GetTickCount();

            // Обычная
            if (currentTime - StatusBoss->LastAttack >= StatusBoss->BStats.attackCooldown * 1000) {
                int weakestIdx = -1;
                long minHealth = LONG_MAX;
                for (size_t i = 0; i < PData.size(); ++i) {
                    if (PData[i].IsAlive && PData[i].PStats.health < minHealth) {
                        minHealth = PData[i].PStats.health;
                        weakestIdx = i;
                    }
                }
                if (weakestIdx != -1) {
                    EnterCriticalSection(&Critical_Section);
                    bossTargetId = PData[weakestIdx].id;
                    LeaveCriticalSection(&Critical_Section);

                    SetEvent(BOSS_ATTACK);
                }
                StatusBoss->LastAttack = currentTime;
            }

            // Спец
            if (currentTime - StatusBoss->LastSpecialAttack >= StatusBoss->BStats.specialCooldown * 1000) {
                PulseEvent(BOSS_SPECIAL_EVENT);
                StatusBoss->LastSpecialAttack = currentTime;
            }
        }
    }
    return 0;
}

    DWORD WINAPI DRAW(LPVOID param) {
        while (Game) {
            int deadCount = 0;
            for (auto& p : PData)
                if (!p.IsAlive) deadCount++;

            if (deadCount == PData.size() || _bossStatus.BStats.health <= 0) {
                Game = false;
                if (_bossStatus.BStats.health > 0)
                    EVIL_WIN = true;
                break;
            }

        Sleep(200);

       int middle = Players.size() / 2;

        cout << "------------------------------------------------------------" << endl;

        EnterCriticalSection(&Critical_Section); 
        for (int i = 0; i < Players.size(); i++) {
            cout << endl;
            cout << PData[i].PStats.name << " Урон: " << PData[i].PStats.damage << flush;

            cout << "\nЗдоровье:" << PData[i].PStats.health << " Статус: " << PData[i].Status << flush;

            if (i == middle) {
                cout << "\t\t хп Босса: " << _bossStatus.BStats.health
                    << " урон босса " << (_bossStatus.missed ? "Промазал" : "Атаковал") << flush;
            }
            cout << endl;
        }
        LeaveCriticalSection(&Critical_Section);
    }
    return 0;
}

int main() {

    srand(time(0));

    PLAYER_ATTACK = CreateEvent(NULL, FALSE, FALSE, NULL);  
    BOSS_ATTACK = CreateEvent(NULL, FALSE, FALSE, NULL);    
    BOSS_SPECIAL_EVENT = CreateEvent(NULL, TRUE, FALSE, NULL);

    Game = true;
    setlocale(0, "rus");
    InitializeCriticalSection(&Critical_Section);

    int input;

    cout << "Добро пожаловать\nВведите количество игроков ";
    cin >> input;

    PData.resize(input);

    playerAttackTotal.resize(input, false);
    playerSpecialTotal.resize(input, false);

    playerDamageDealt.resize(input, 0);
    playerSpecialDamageDealt.resize(input, 0);

    for (int i = 0; i < input; ++i) {

        string Pname = "Player " + to_string(i + 1);
        PData[i].id = i + 1;

        PData[i].PStats.attackCooldown = 3 + rand() % 9;
        PData[i].PStats.specialCooldown = 6 + rand() % 14;

        PData[i].LastAttack = GetTickCount() - PData[i].PStats.attackCooldown * 1000;
        PData[i].LastSpecialAttack = GetTickCount() - PData[i].PStats.specialCooldown * 1000;

        PData[i].PStats.health = 100000 + rand() % 300000;
        PData[i].PStats.damage = 10000 + rand() % 50000;

        PData[i].PStats.specialDamage = 20000 + rand() % 70000;
        PData[i].PStats.defense = 10 + rand() % 30;

        PData[i].PStats.dodgeChance = 10 + rand() % 30;
        strcpy_s(PData[i].PStats.name, Pname.c_str());

    }

    // игрокм
    for (int i = 0; i < input; ++i) {
        HANDLE player = CreateThread(NULL, 0, Player, &PData[i], 0, NULL);
        if (!player) {
            cout << "Ошибка создания игрока " << GetLastError();
            return GetLastError();
        }
        Players.push_back(player);
    }

    // босс

    _bossStatus.BStats.health = 500000 + rand() % 900000;
    _bossStatus.BStats.resist = 30 + rand() % 40;

    _bossStatus.BStats.damage = 50000 + rand() % 100000;
    _bossStatus.BStats.specialDamage = 100000 + rand() % 150000;

    _bossStatus.BStats.attackCooldown = 5 + rand() % 15;
    _bossStatus.BStats.specialCooldown = 15 + rand() % 30;

    _bossStatus.LastAttack = GetTickCount() - _bossStatus.BStats.attackCooldown * 1000;
    _bossStatus.LastSpecialAttack = GetTickCount() - _bossStatus.BStats.specialCooldown * 1000;
    _bossStatus.missed = false;

    BOSS_Thread = CreateThread(NULL, 0, Boss, &_bossStatus, 0, NULL);
    if (!BOSS_Thread) {
        cout << "Ошибка создания босса " << GetLastError();
        return GetLastError();
    }

    HANDLE Drawler = CreateThread(NULL, 0, DRAW, NULL, 0, NULL);
    SetThreadPriority(Drawler, THREAD_PRIORITY_HIGHEST);
    if (!Drawler) {
        cout << "Ошибка создания отрисовки " << GetLastError();
        return GetLastError();
    }

    WaitForSingleObject(Drawler, INFINITE);

    cout << "ИГРА ЗАКОНЧЕНА" << endl;
    cout << "ПОБЕДИЛ " << (EVIL_WIN ? "БОСС" : "ИГРОКИ") << endl;

    vector<Tops> results;
    vector<PlayerStatus> tempPData = PData;

    while (!tempPData.empty()) {

        long maxDmg = -1;
        int idx = -1;

        for (size_t i = 0; i < tempPData.size(); ++i) {

            if (tempPData[i].DamageDeal > maxDmg) {

                maxDmg = tempPData[i].DamageDeal;
                idx = i;

            }
        }
        results.push_back({ tempPData[idx].PStats.name, maxDmg });

        tempPData.erase(tempPData.begin() + idx);

    }

    for (size_t i = 0; i < results.size(); ++i)

        cout << "ТОП-" << i + 1 << "     " << results[i].name

        << "\n     Нанесенный урон:" << results[i].Damage << "\n" << endl;


    DeleteCriticalSection(&Critical_Section);

    CloseHandle(PLAYER_ATTACK);
    CloseHandle(BOSS_ATTACK);
    CloseHandle(BOSS_SPECIAL_EVENT);

    return 0;
}