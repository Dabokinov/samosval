#include <Windows.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <stdio.h>
#include "Weapons.h"

#include <fcntl.h>
#include <io.h>
#include <cstdio>

#define FG_RED "\033[31m"
#define FG_PURPLE "\033[35m"
#define FG_GREEN "\033[32m"
#define FG_YELLOW "\033[33m"
#define FG_WHITE "\033[0m"

//// Define SYSTEM_INFORMATION_CLASS and SYSTEM_PROCESS_INFORMATION
//typedef enum _SYSTEM_INFORMATION_CLASS {
//    SystemBasicInformation = 0,
//    SystemProcessInformation = 5,
//    // Other classes can be added as needed
//} SYSTEM_INFORMATION_CLASS;
//
//typedef struct _SYSTEM_PROCESS_INFORMATION {
//    ULONG NextEntryOffset;
//    ULONG NumberOfThreads;
//    LARGE_INTEGER Reserved[3];
//    LARGE_INTEGER CreateTime;
//    LARGE_INTEGER UserTime;
//    LARGE_INTEGER KernelTime;
//    UNICODE_STRING ImageName;
//    ULONG BasePriority;
//    HANDLE ProcessId;
//    HANDLE InheritedFromProcessId;
//    ULONG HandleCount;
//    ULONG SessionId;
//    ULONG_PTR PageDirectoryBase;
//    SIZE_T PeakVirtualSize;
//    SIZE_T VirtualSize;
//    ULONG PageFaultCount;
//    SIZE_T PeakWorkingSetSize;
//    SIZE_T WorkingSetSize;
//    SIZE_T QuotaPeakPagedPoolUsage;
//    SIZE_T QuotaPagedPoolUsage;
//    SIZE_T QuotaPeakNonPagedPoolUsage;
//    SIZE_T QuotaNonPagedPoolUsage;
//    SIZE_T PagefileUsage;
//    SIZE_T PeakPagefileUsage;
//    SIZE_T PrivatePageCount;
//    LARGE_INTEGER ReadOperationCount;
//    LARGE_INTEGER WriteOperationCount;
//    LARGE_INTEGER OtherOperationCount;
//    LARGE_INTEGER ReadTransferCount;
//    LARGE_INTEGER WriteTransferCount;
//    LARGE_INTEGER OtherTransferCount;
//} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;
//
//
//// Add NtQuerySystemInformation hook
//typedef NTSTATUS(WINAPI* NtQuerySystemInformation_t)(SYSTEM_INFORMATION_CLASS, PVOID, ULONG, PULONG);
//NtQuerySystemInformation_t originalNtQuerySystemInformation;
//
//// Hook function to modify the process list
//NTSTATUS WINAPI HookedNtQuerySystemInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength) {
//    NTSTATUS status = originalNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
//
//    if (SystemInformationClass == SystemProcessInformation) {
//        // Modify SystemInformation to hide processes
//        // This example shows how you would traverse the process list and hide specific processes
//        PSYSTEM_PROCESS_INFORMATION pCurrent = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
//        PSYSTEM_PROCESS_INFORMATION pPrev = nullptr;
//
//        while (pCurrent->NextEntryOffset != 0) {
//            if (wcscmp(pCurrent->ImageName.Buffer, L"target_process.exe") == 0) {
//                if (pPrev) {
//                    pPrev->NextEntryOffset += pCurrent->NextEntryOffset;
//                }
//            }
//            pPrev = pCurrent;
//            pCurrent = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pCurrent + pCurrent->NextEntryOffset);
//        }
//    }
//
//    return status;
//}
//
//// Function to hook NtQuerySystemInformation
//void HookNtQuerySystemInformation() {
//    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
//    originalNtQuerySystemInformation = (NtQuerySystemInformation_t)GetProcAddress(hNtDll, "NtQuerySystemInformation");
//
//    // Apply the hook (could use a hooking library here like MinHook)
//    DWORD oldProtect;
//    VirtualProtect(originalNtQuerySystemInformation, sizeof(LPVOID), PAGE_EXECUTE_READWRITE, &oldProtect);
//    *((LPVOID*)originalNtQuerySystemInformation) = HookedNtQuerySystemInformation;
//    VirtualProtect(originalNtQuerySystemInformation, sizeof(LPVOID), oldProtect, &oldProtect);
//}

int currentwep = 0;
int scope = 0;
int barrel = 0;
int randomizer = 10;
int playerfov = 90;          // Ограничение FOV от 70 до 90
float playersens = 0.300;    // Начальная чувствительность = 0.300
bool enabled = false;
bool hidden = false; // Переменная для отслеживания состояния скрытия окна

void SmoothMove(float x, float y, float delay) {
    int steps = 10 + (rand() % 20);  // Количество шагов для плавного движения
    float stepX = x / steps;
    float stepY = y / steps;

    for (int i = 0; i < steps; ++i) {
        // Добавляем небольшие случайные колебания для более реалистичного поведения
        float randomX = (rand() % 3 - 1) * 0.5;
        float randomY = (rand() % 3 - 1) * 0.5;
        // Движение мыши
        mouse_event(MOUSEEVENTF_MOVE, (int)(stepX + randomX), (int)(stepY + randomY), 0, 0);
        // Задержка между шагами
        Sleep((int)(delay * 1000 / steps));
    }
}

//// Функция контроля отдачи
//void ControlRecoil(int weaponIndex) {
//    if (enabled) {
//        // Используем паттерн из Weapons.h
//        const std::vector<vector2>& pattern = Weapons::ak::pattern;  // Замените на текущее оружие
//        float delay = Weapons::ak::delay;  // Задержка для текущего оружия
//
//        // Проходим по паттерну отдачи
//        for (size_t i = 0; i < pattern.size(); ++i) {
//            // Прерываем контроль, если кнопка мыши отпущена
//            if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) break;
//
//            // Рассчитываем отдачу с учетом чувствительности игрока
//            float recoilX = pattern[i].x / playersens;
//            float recoilY = pattern[i].y / playersens;
//
//            // Выполняем плавное движение мыши
//            SmoothMove(recoilX, recoilY, delay);
//        }
//    }
//}

void HideWindowFromTaskbar(HWND hwnd)
{
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    style |= WS_EX_TOOLWINDOW;
    style &= ~WS_EX_APPWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, style);
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOW);
}

void ShowWindowOnTaskbar(HWND hwnd)
{
    LONG style = GetWindowLong(hwnd, GWL_EXSTYLE);
    style &= ~WS_EX_TOOLWINDOW;
    style |= WS_EX_APPWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, style);
    ShowWindow(hwnd, SW_HIDE);
    ShowWindow(hwnd, SW_SHOW);
}

void DrawGui()
{
    system("cls");
    system("color 5");
    _setmode(_fileno(stdout), _O_TEXT);
    std::cout << FG_WHITE << " ----------------------------------------------------------- " << std::endl;

    std::cout << FG_WHITE << "              " "[" << FG_PURPLE << "WEAPONS" << FG_WHITE << "]" << "          |"  FG_WHITE << "        " "[" << FG_PURPLE << "ATTACHMENTS" << FG_WHITE << "]" << std::endl;

    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "0" << FG_WHITE << "]" << "AK-47" << "             |" << FG_WHITE << "        " "[" << FG_PURPLE << "+" << FG_WHITE << "]" << "No Scope" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "1" << FG_WHITE << "]" << "Thompson" << "          |" << FG_WHITE << "        " "[" << FG_PURPLE << "6" << FG_WHITE << "]" << "Holo" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "2" << FG_WHITE << "]" << "Custom SMG" << "        |" << FG_WHITE << "        " "[" << FG_PURPLE << "7" << FG_WHITE << "]" << "8x" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "3" << FG_WHITE << "]" << "LR-300" << "            |" << FG_WHITE << "        " "[" << FG_PURPLE << "BARREL" << FG_WHITE << "]" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "4" << FG_WHITE << "]" << "MP5A4" << "             |" << FG_WHITE << "        " "[" << FG_PURPLE << "/" << FG_WHITE << "]" << "Supressor" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "Num." << FG_WHITE << "]" << "S.A.R." << "         |" << FG_WHITE << "        " "[" << FG_PURPLE << "*" << FG_WHITE << "]" << "Boost" << std::endl;
    std::cout << FG_WHITE << "            " "[" << FG_PURPLE << "5" << FG_WHITE << "]" << "M249" << "              |" << FG_WHITE << "        " "[" << FG_PURPLE << "-" << FG_WHITE << "]" << "No Barrel" << std::endl;

    std::cout << " ----------------------------------------------------------- " << std::endl << std::endl;

    std::cout << FG_WHITE << "           " "[" << FG_PURPLE << "FOV" << FG_WHITE << "] Current FOV: " << FG_GREEN << playerfov << FG_WHITE << " [F7/F8]" << std::endl;
    std::cout << FG_WHITE << "     " "[" << FG_PURPLE << "Sensitivity" << FG_WHITE << "] Current Sensitivity: " << FG_GREEN << playersens << FG_WHITE << " [F5/F6]" << std::endl;

    std::cout << std::endl;
    std::cout << FG_WHITE << "                 " "[" << FG_GREEN << "i" << FG_WHITE << "]" << "Current Held Weapon: ";
    switch (currentwep)
    {
    case 0:
        std::cout << "AK-47" << std::endl;
        break;
    case 1:
        std::cout << "Thompson" << std::endl;
        break;
    case 2:
        std::cout << "Custom SMG" << std::endl;
        break;
    case 3:
        std::cout << "LR-300" << std::endl;
        break;
    case 4:
        std::cout << "MP5A4" << std::endl;
        break;
    case 5:
        std::cout << "S.A.R." << std::endl;
        break;
    case 6:
        std::cout << "M249" << std::endl;
        break;
    }
    std::cout << FG_WHITE << "                 " "[" << FG_GREEN << "i" << FG_WHITE << "]" << "Current Scope: ";
    switch (scope)
    {
    case 0:
        std::cout << "None" << std::endl;
        break;
    case 1:
        std::cout << "Holo" << std::endl;
        break;
    case 2:
        std::cout << "X8" << std::endl;
        break;
    }
    std::cout << FG_WHITE << "                 " "[" << FG_GREEN << "i" << FG_WHITE << "]" << "Current Barrel:";
    switch (barrel)
    {
    case 0:
        std::cout << "None" << std::endl;
        break;
    case 1:
        std::cout << "Suppressor" << std::endl;
        break;
    case 2:
        std::cout << "Boost" << std::endl;
        break;
    }
    std::cout << std::endl;

    std::cout << FG_WHITE << "                        " "[" << (enabled ? FG_GREEN "F2" : FG_RED "F2") << FG_WHITE << "]" << "Enabled";
}

float Randomize(float val, int perc)
{
    LARGE_INTEGER time_mayne;
    QueryPerformanceCounter((LARGE_INTEGER*)&time_mayne);

    srand((unsigned long)(time_mayne.LowPart));
    float range = val * ((float)(perc) / 100);

    if (range <= 0.5) return val;
    if (range > 0.5) range = 1;

    int result = 1 + (rand() % (int)range);

    if ((1 + (rand() % 1) > 0)) return val + result;
    else return val + (result * -1);

}

void QuerySleep(int ms)
{
    LONGLONG timerResolution;
    LONGLONG wantedTime;
    LONGLONG currentTime;

    QueryPerformanceFrequency((LARGE_INTEGER*)&timerResolution);
    timerResolution /= 1000;

    QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
    wantedTime = currentTime / timerResolution + ms;
    currentTime = 0;
    while (currentTime < wantedTime)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
        currentTime /= timerResolution;
    }
}

void Smoothing(double delay, double control_time, float x, float y) 
{
	int x_ = 1, y_ = 1, t_ = 0;
	for (int i = 1; i <= (int)control_time; ++i) 
	{
		int xI = i * x / (int)control_time;
		int yI = i * y / (int)control_time;
		int tI = i * (int)control_time / (int)control_time;
		mouse_event(1, (int)xI - (int)x_, (int)yI - (int)y_, 0, 0);
		QuerySleep((int)tI - (int)t_);
		x_ = xI; y_ = yI; t_ = tI;
	}
	QuerySleep((int)delay - (int)control_time);
}

float getScope(float val)
{
    if (scope == 1)
        return val * 1.2;
    if (scope == 2)
        return val * 3.84;
    return val;
}

float tofovandsens(float sens, int fov, float val)
{
    float a = (0.5 * fov * val) / (sens * 90);
    return getScope(a);
}

int main()
{
    int count = 0;

    //// Hook NtQuerySystemInformation to hide the process
    //HookNtQuerySystemInformation();

    SetConsoleTitle(L"Wagner");

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(handle, &info);
    COORD new_size =
    {
        info.srWindow.Right - info.srWindow.Left + 1,
        info.srWindow.Bottom - info.srWindow.Top + 1
    };
    SetConsoleScreenBufferSize(handle, new_size);

    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        MoveWindow(hwnd, 800, 200, 520, 360, TRUE);
    }
    SetLayeredWindowAttributes(hwnd, 1000, 1000, LWA_ALPHA);

    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);

    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);

    DrawGui();

    while (true)
    {
        if (GetAsyncKeyState(VK_F3) & 0x8000)
        {
            hidden = !hidden;
            if (hidden)
            {
                HideWindowFromTaskbar(hwnd);
            }
            else
            {
                ShowWindowOnTaskbar(hwnd);
            }
            Sleep(200); // Защита от дребезга
        }

        if (GetKeyState(VK_NUMPAD0) & 0x8000)
        {
            if (currentwep != 0)
            {
                currentwep = 0;
                DrawGui();
            }
        }
        if (GetKeyState(VK_NUMPAD4) & 0x8000)
        {
            if (currentwep != 1)
            {
                currentwep = 1;
                DrawGui();
            }
        }
        if (GetKeyState(VK_NUMPAD3) & 0x8000)
        {
            if (currentwep != 2)
            {
                currentwep = 2;
                DrawGui();
            }
        }
        if (GetKeyState(VK_NUMPAD1) & 0x8000)
        {
            if (currentwep != 3)
            {
                currentwep = 3;
                DrawGui();
            }
        }
        if (GetKeyState(VK_NUMPAD2) & 0x8000)
        {
            if (currentwep != 4)
            {
                currentwep = 4;
                DrawGui();
            }
        }
        if (GetKeyState(VK_DECIMAL) & 0x8000)
        {
            if (currentwep != 5)
            {
                currentwep = 5;
                DrawGui();
            }
        }
        if (GetKeyState(VK_NUMPAD5) & 0x8000)
        {
            if (currentwep != 6)
            {
                currentwep = 6;
                DrawGui();
            }
        }
        if (GetAsyncKeyState(VK_F2) == -32767)
        {
            enabled = !enabled;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_ADD) == -32767)
        {
            scope = 0;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_NUMPAD7) == -32767)
        {
            scope = 1;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_NUMPAD6) == -32767)
        {
            scope = 2;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_SUBTRACT) == -32767)
        {
            barrel = 0;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_DIVIDE) == -32767)
        {
            barrel = 1;
            DrawGui();
        }
        if (GetAsyncKeyState(VK_MULTIPLY) == -32767)
        {
            barrel = 2;
            DrawGui();
        }

        // Изменение чувствительности
        if (GetAsyncKeyState(VK_F5) & 0x8000)
        {
            playersens -= 0.01;
            if (playersens < 0.1) playersens = 0.1;  // Ограничение минимальной чувствительности
            DrawGui();
        }
        if (GetAsyncKeyState(VK_F6) & 0x8000)
        {
            playersens += 0.01;
            if (playersens > 5.0) playersens = 5.0;  // Ограничение максимальной чувствительности
            DrawGui();
        }

        // Ограничение FOV от 70 до 90
        if (GetAsyncKeyState(VK_F7) & 0x8000)
        {
            playerfov -= 1;
            if (playerfov < 70) playerfov = 70;  // Минимальное значение FOV
            DrawGui();
        }
        if (GetAsyncKeyState(VK_F8) & 0x8000)
        {
            playerfov += 1;
            if (playerfov > 90) playerfov = 90;  // Максимальное значение FOV
            DrawGui();
        }

        if (enabled == true)
        {
            if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON))
            {
                switch (currentwep)
                {
                case 0:
                    if (count < Weapons::ak::pattern.size())
                    {
                        Smoothing(Weapons::ak::delay, Weapons::ak::controltime.at(count), Randomize(tofovandsens(playersens, playerfov, Weapons::ak::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::ak::pattern.at(count).y), randomizer));
                        count++;
                    }
                    break;
                case 1:
                    if (count < Weapons::thompson::pattern.size())
                    {
                        Smoothing(Weapons::thompson::delay, Weapons::thompson::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::thompson::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::thompson::pattern.at(count).y), randomizer));
                        count++;
                    }
                    break;
                case 2:
                    if (count < Weapons::smg::pattern.size())
                    {
                        Smoothing(Weapons::smg::delay, Weapons::smg::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::smg::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::smg::pattern.at(count).y), randomizer));
                        count++;
                    }
                    break;
                case 3:
                    if (count < Weapons::lr::pattern.size())
                    {
                        Smoothing(Weapons::lr::delay, Weapons::lr::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::lr::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::lr::pattern.at(count).y), randomizer));
                        count++;
                    }
                    break;
                case 4:
                  if (count < Weapons::mp5::pattern.size())
                   {
                        Smoothing(Weapons::mp5::delay, Weapons::mp5::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::mp5::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::mp5::pattern.at(count).y), randomizer));
                       count++;
                   }
                    break;
                case 5:
                   if (count < Weapons::semi::pattern.size())
                    {
                        Smoothing(Weapons::semi::delay, Weapons::semi::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::semi::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::semi::pattern.at(count).y), randomizer));
                    }
                    break;
               case 6:
                    Smoothing(Weapons::m249::delay, Weapons::m249::delay, Randomize(tofovandsens(playersens, playerfov, Weapons::m249::pattern.at(count).x), randomizer), Randomize(tofovandsens(playersens, playerfov, Weapons::m249::pattern.at(count).y), randomizer));
                    break;
                default:
                    break;
                }

            }
            else
                count = 0;
        }

    }

}
