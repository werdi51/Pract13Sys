#include <iostream>
#include <Windows.h>
#include <string>

int main(int argc, char* argv[])
{
    if (argc >= 2) {
        int num = std::atoi(argv[1]);
        std::string title = "Naperstki_" + std::to_string(num);
        SetConsoleTitleA(title.c_str()); 
    }

    char map[20][20] = {
        "-------------------",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |",
        "|                 |"
    };
    for (int i = 0; i < 18; i++) {
        std::cout << map[i] << std::endl;
    }

    std::cin.get();
    return 0;
}