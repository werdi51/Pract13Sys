#include <iostream>
#include <Windows.h>
#include <string>
#include <vector>	


#include <io.h>
#include <fcntl.h>

//https://www.codeincodeblock.com/2011/03/move-console-windows-using-codeblock.html
//																										ANSI ↔ Unicode

using namespace std;

static int CountGlasses = 3;



STARTUPINFOA si = { sizeof(si) };
PROCESS_INFORMATION pi[3];
vector<HWND> hWnds(3);


void Glass()
{
	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			cout << " ";
		}
	}
	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 100; j++) 
		{
			cout << "-";
		}
	}
};
void Dirt_Glass()
{

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

void CreateGlasses()
{
	STARTUPINFOA si = { sizeof(si) };
	PROCESS_INFORMATION pi[3];
	vector<HWND> hWnds(3);

	std::string appPath = "C:\\Users\\anton\\OneDrive\\Desktop\\Sys\\Proect\\Naperstki\\x64\\Debug\\Glass.exe";

	if (appPath== "C:\\Users\\anton\\OneDrive\\Desktop\\Sys\\Proect\\Naperstki\\x64\\Debug\\Glass.exe")
	{
		cout << "вставь свой путь к процессу стакана" << endl;
		
	}
	else
	{


		for (int i = 0; i < CountGlasses; i++) {

			std::string cmdLine = appPath + " " + std::to_string(i + 1);

			BOOL success = CreateProcessA(
				NULL,                   // Имя модуля (можно NULL, если указано в командной строке)
				(LPSTR)cmdLine.c_str(), // Полная командная строка
				NULL,                   // Атрибуты безопасности процесса
				NULL,                   // Атрибуты безопасности потока
				FALSE,                  // Не наследовать дескрипторы
				CREATE_NEW_CONSOLE,      // Создать новое окно консоли
				NULL,                   // Использовать окружение родителя
				NULL,                   // Текущая директория
				&si,                    // STARTUPINFO
				&pi[i]                  // PROCESS_INFORMATION
			);

			if (!success) {
				std::cerr << "Failed to create process " << i + 1 << std::endl;
			}
		}
		Sleep(1000);

		for (int i = 0; i < CountGlasses; i++) {
			std::string title = "Naperstki_" + std::to_string(i + 1);
			hWnds[i] = FindWindowA(NULL, title.c_str());
			if (hWnds[i] == NULL) {
				std::cerr << "Window " << i + 1 << " not found" << std::endl;
			}
			else {
				std::cout << "Window " << i + 1 << " found" << std::endl;
			}
		}




















		//ПАРАМЕТРЫ ДЛЯ SetWindowPos

		// РАЗМЕР ОКНА
		//ПЕРВОЕ-ПУТЬ
		//ВТОРОЕ-ГЛУБИНА ОКНА (ОСЬ Z)
		//ТРЕТЬЕ-пОЗИЦИЯ ПО X ОСИ
		//ЧЕТВЕРТОЕ- ПОЗИЦИЯ ПО Y ОСИ (ВЫСОТЕ)
		//ПЯТОЕ-ШИРИНА ОКНА
		//ШЕСТОЕ-ВЫСОТА ОКНА
		//СЕДЬМОЕ-КАКИЕ НИБУДЬ ЗАПРЕТЫ (СТОИТ ЗАПРЕТ НА ИЗМЕНЕНИЕ Z ОСИ ИЗ ЗА ЧЕГО 2 ПАРАМЕТР ПУСТОЕ Т.К. НЕ НУЖЕН)

		for (int i = 0; i < CountGlasses; i++) {
			if (hWnds[i]) {
				SetWindowPos(hWnds[i], NULL, 100 + i * 500, 500, 300, 500, SWP_NOZORDER);
			}
		}
























		HANDLE hProcesses[3];
		for (int i = 0; i < CountGlasses; i++) {
			hProcesses[i] = pi[i].hProcess;
		}
		WaitForMultipleObjects(3, hProcesses, TRUE, INFINITE);

		for (int i = 0; i < CountGlasses; i++) {
			CloseHandle(pi[i].hProcess);
			CloseHandle(pi[i].hThread);
		}

	}


}
int main()
{
	setlocale(0, "rus");
	CreateGlasses();





	return 0;
}
