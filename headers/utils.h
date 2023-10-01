#ifndef UTILS
#define UTILS

#include "main.h"

#include <conio.h>

#define isEscape(ch) (ch == 224 || ch == 0) // ONLY _getch() | _getwch()

#define FOREGROUND_BRIGHT_WHITE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY

#define ESC "\x1b"
#define CSI ESC "["
#define LESC L"\x1b"
#define LCSI L"\x1b["

int ctoh(char ch);
int int_length(int num);
BOOL is_wnumber(WCHAR* text, int len);
void handleWSAError(size_t line);
COORD GetConsoleCursorPosition(HANDLE hConsoleOutput);
BOOL GetConsoleSize(HANDLE hConsoleOutput, SHORT* cols, SHORT* rows, SHORT* top);
BOOL SetConsoleSize(HANDLE hConsoleOutput, SHORT cols, SHORT rows);
BOOL ScrollConsoleDown(HANDLE hConsoleOutput, SHORT iRows);
TCHAR ReadCharAtPos(HANDLE hConsoleOutput, COORD pos);
BOOL ReadLine(HANDLE hConsoleOutput, CHAR_INFO* buffer, DWORD charsToRead, SHORT startY);
BOOL WriteLine(HANDLE hConsoleOutput, CHAR_INFO* buffer, DWORD charsToWrite, SHORT startY);
BOOL EnableVTMode();
void PrintVerticalBorder(int pos);
void PrintHorizontalBorder(SHORT cols, BOOL fIsTop);
void printEmptyLine();
ULONGLONG GetEpochMs();
void ip_to_wstring(WCHAR* buffer, DWORD bufferLen, SOCKADDR_IN* ip);
int my_wrecive(SOCKET* socket, WCHAR* buffer, SOCKADDR_IN* senderAddr);
int my_wsend(SOCKET* socket, WCHAR* msg, SOCKADDR_IN* serverAddr);
BOOL set_addr(SOCKADDR_IN* addr);
void printColoredText(WCHAR* msg, int length);
void command_engine(WCHAR* command);

#endif