#include "utils.h"

int ctoh(char ch)
{
	switch (ch)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': case 'a': return 10;
	case 'B': case 'b': return 11;
	case 'C': case 'c': return 12;
	case 'D': case 'd': return 13;
	case 'E': case 'e': return 14;
	case 'F': case 'f': return 15;
	default: return -1;
	}
}

int int_length(int num)
{
	if (num >= 100000)
	{
		if (num >= 10000000)
		{
			if (num >= 1000000000) return 10;
			if (num >= 100000000) return 9;
			return 8;
		}
		if (num >= 1000000) return 7;
		return 6;
	}
	else
	{
		if (num >= 1000)
		{
			if (num >= 10000) return 5;
			return 4;
		}
		else
		{
			if (num >= 100) return 3;
			if (num >= 10) return 2;
			return 1;
		}
	}
}

void handleWSAError(size_t line)
{
	char* errMsg;
	int WSAError = WSAGetLastError();
	switch (WSAError)
	{
	case 10054:		errMsg = "couldn't connect to server/client";	break;
	case 0:			errMsg = "no error while error SUS";			break;
	default:		errMsg = "not defined error";					break;
	}

	printf("[line %llu] %s: %i\n", line, errMsg, WSAError);
}

COORD GetConsoleCursorPosition(HANDLE hConsoleOutput)
{
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	return (GetConsoleScreenBufferInfo(hConsoleOutput, &cbsi)) ? cbsi.dwCursorPosition : (COORD) { 0, 0 };
}

BOOL GetConsoleSize(HANDLE hConsoleOutput, SHORT* cols, SHORT* rows, SHORT* top)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(hConsoleOutput, &csbi)) return FALSE;
	if (cols != NULL) *cols = csbi.dwSize.X;
	if (rows != NULL) *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	if (top != NULL) *top = csbi.srWindow.Top;
	return TRUE;
}

BOOL SetConsoleSize(HANDLE hConsoleOutput, SHORT cols, SHORT rows)
{
	SHORT oldr = 0, oldc = 0;
	if (!GetConsoleSize(hConsoleOutput, &oldc, &oldr, NULL)) return FALSE;

	if (cols == 0) cols = oldc;
	if (rows == 0) rows = oldr;

	COORD newSize = { cols, rows };
	SMALL_RECT rect = { 0, 0, cols - 1, rows - 1 };

	if (oldr <= rows)
	{
		if (oldc <= cols)
		{
			// increasing both dimensions
		BUFWIN:
			SetConsoleScreenBufferSize(hConsoleOutput, newSize);
			SetConsoleWindowInfo(hConsoleOutput, TRUE, &rect);
		}
		else
		{
			// cols--, rows+
			SMALL_RECT tmp = { 0, 0, cols - 1, oldr - 1 };
			SetConsoleWindowInfo(hConsoleOutput, TRUE, &tmp);
			goto BUFWIN;
		}
	}
	else
	{
		if (oldc <= cols)
		{
			// cols+, rows--
			SMALL_RECT tmp = { 0, 0, oldc - 1, rows - 1 };
			SetConsoleWindowInfo(hConsoleOutput, TRUE, &tmp);
			goto BUFWIN;
		}
		else
		{
			// cols--, rows--
			SetConsoleWindowInfo(hConsoleOutput, TRUE, &rect);
			SetConsoleScreenBufferSize(hConsoleOutput, newSize);
		}
	}
	return TRUE;
}

BOOL ScrollConsoleDown(HANDLE hConsoleOutput, SHORT iRows)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

	if (!GetConsoleScreenBufferInfo(hConsoleOutput, &csbiInfo))
	{
		printf("GetConsoleScreenBufferInfo (%lu)\n", GetLastError());
		return FALSE;
	}

	SMALL_RECT srctWindow = csbiInfo.srWindow;
	srctWindow.Top += iRows;
	srctWindow.Bottom += iRows;

	if (!SetConsoleWindowInfo(hConsoleOutput, TRUE, &srctWindow))
	{
		printf("SetConsoleWindowInfo (%lu)\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

TCHAR ReadCharAtPos(HANDLE hConsoleOutput, COORD pos)
{
	TCHAR tmp;
	DWORD read = 0;
	if (!ReadConsoleOutputCharacterW(hConsoleOutput, &tmp, 1, pos, &read) || read == 0) return '\0';
	return tmp;
}

BOOL ReadLine(HANDLE hConsoleOutput, CHAR_INFO* buffer, DWORD charsToRead, SHORT startY)
{
	SMALL_RECT t = { 0, startY, charsToRead - 1, startY };
	return ReadConsoleOutput(hConsoleOutput, buffer, (COORD) { charsToRead, 1 }, (COORD) { 0, 0 }, & t);
}
BOOL WriteLine(HANDLE hConsoleOutput, CHAR_INFO* buffer, DWORD charsToWrite, SHORT startY)
{
	SMALL_RECT t = { 0, startY, charsToWrite - 1, startY };
	return WriteConsoleOutput(hConsoleOutput, buffer, (COORD) { charsToWrite, 1 }, (COORD) { 0, 0 }, & t);
}

BOOL EnableVTMode()
{
	DWORD dwMode = 0;
	if (!GetConsoleMode(cmd, &dwMode)) return FALSE;
	return SetConsoleMode(cmd, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void PrintVerticalBorder(int pos)
{
	printf(ESC "(0" CSI "104;93m");		// Enter Line drawing mode -> bright yellow on bright blue

	switch (pos)
	{
	case 1: {
		COORD tmp = GetConsoleCursorPosition(cmd);
		SetConsoleCursorPosition(cmd, (COORD) { tmp.X, tmp.Y - 1 });
		printf("w" CSI "B" CSI "D");	// top junction -> move cursor down -> move cursor left
		break;
	}
	case -1: {
		COORD tmp = GetConsoleCursorPosition(cmd);
		SetConsoleCursorPosition(cmd, (COORD) { tmp.X, tmp.Y + 1 });
		printf("v" CSI "A" CSI "D");	// bottom junction -> move cursor up -> move cursor left
		break;
	}
	}

	printf("x" CSI "0m" ESC "(B");		// in line drawing mode, \x78 -> \u2502 "Vertical Bar" -> restore color -> exit line drawing mode
}

void PrintHorizontalBorder(SHORT cols, BOOL fIsTop)
{
	printf(ESC "(0" CSI "104;93m" "%c", fIsTop ? 'l' : 'm');	// Enter Line drawing mode -> Make the border bright yellow on bright blue -> print left corner 

	for (int i = 1; i < cols - 1; i++)
	{
		printf("q");											// in line drawing mode, \x71 -> \u2500 "HORIZONTAL SCAN LINE-5"
	}

	printf("%c" CSI "0m" ESC "(B", fIsTop ? 'k' : 'j');			// print right corner -> restore color -> exit line drawing mode
}

ULONGLONG GetEpochMs()
{
	FILETIME tmp;
	GetSystemTimeAsFileTime(&tmp);
	return (ULONGLONG)tmp.dwLowDateTime | ((ULONGLONG)(tmp.dwHighDateTime) << 32LL);
}

int my_wrecive(SOCKET* socket, WCHAR* buffer, SOCKADDR_IN* senderAddr)
{
	int i = sizeof(*senderAddr);
	int res = recvfrom(*socket, (char*)buffer, MAX_PACKET * sizeof(*buffer), 0, (SOCKADDR*)senderAddr, (isClient) ? NULL : &i);
	if (res == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAETIMEDOUT)
		{
			handleWSAError(__LINE__);
			closesocket(*socket);
		}
	}
	else res >>= 1;
	return res;
}

int my_wsend(SOCKET* socket, WCHAR* msg, SOCKADDR_IN* serverAddr)
{
	int res = sendto(*socket, (char*)msg, wcslen(msg) * sizeof(*msg), 0, (SOCKADDR*)serverAddr, sizeof(*serverAddr));
	if (res == SOCKET_ERROR)
	{
		handleWSAError(__LINE__);
		closesocket(*socket);
	}
	return res;
}

BOOL set_addr(SOCKADDR_IN* addr)
{
	WCHAR ip_port[IPLEN + 1 + PORTLEN + 1]; // 255.255.255.255:65535\0

	printf("address (x.y.z.w -> x.y.z.w:8080, :x -> 127.0.0.1:x, nothing -> 127.0.0.1:8080)\n(e.g.: 192.168.1.11:27015): ");
	fgetws(ip_port, sizeof(ip_port) / sizeof(ip_port[0]), stdin);
	int ip_portSize = wcslen(ip_port);
	if (ip_port[ip_portSize - 1] == L'\n') ip_port[--ip_portSize] = L'\0';

	WCHAR _ip[IPLEN + 1] = L"127.0.0.1";
	WCHAR _port[PORTLEN + 1] = L"8080";
	int ipLen = 10, portLen = 5, i = 0;

	BOOL writeIp = TRUE;

	if (ip_portSize > 0)
	{
		if (ip_port[0] != L':')
		{
			int dots = 0;
			for (int tags = 0; i < ip_portSize; ++i)
			{
				char cur = ip_port[i];
				if (cur == L'.')
				{
					dots++;
					if (tags == 0)
					{
						printf("one of the group non existent!\n");
						return 0;
					}
					tags = 0;
				}
				else if (cur == L':') break;
				else if (!iswdigit(cur))
				{
					printf("only numbers!\n");
					return TRUE;
				}
				else if (i > IPLEN)
				{
					printf("the ip is too long!\n");
					return TRUE;
				}
				else if (++tags > 3)
				{
					printf("one of the group is too long (max 3)!\n");
					return TRUE;
				}
			}
			if (dots != 3 || ip_port[i - 1] == L'.')
			{
				printf("wrong ip format (x.y.z.w)!\n");
				return TRUE;
			}

			for (i = 0, ipLen = 0; i <= ip_portSize; ++i)
			{
				char cur = ip_port[i];
				if (cur == L':')
				{
					_ip[ipLen++] = L'\0';
					break;
				}
				else _ip[ipLen++] = cur;
			}
		}
		else if (!isClient) writeIp = FALSE;

		if (ip_port[ip_portSize - 1] != L':') portLen = 0;
		for (++i; i <= ip_portSize; ++i)
		{
			if (portLen > 5)
			{
				printf("port is too long!\n");
				return TRUE;
			}
			_port[portLen++] = ip_port[i];
		}
	}
	else if (!isClient) writeIp = FALSE;

	if (!writeIp)
	{
		WCHAR serverTmp[] = L"0.0.0.0";
		for (int i = 0; i < sizeof(serverTmp) / sizeof(serverTmp[0]); i++)
		{
			_ip[i] = serverTmp[i];
		}
	}

	int size = sizeof(*addr);
	WSAStringToAddressW(_ip, AF_INET, NULL, (SOCKADDR*)addr, &size);
	addr->sin_port = htons((USHORT)_wtoi(_port));
	addr->sin_family = AF_INET;
	return FALSE;
}

void printEmptyLine()
{
	printf(CSI "2K");
}

void printColoredText(WCHAR* msg, int length)
{
	WCHAR* color = wcschr(msg, L'§');
	if (color != NULL)
	{
		int size = length;
		WCHAR* text2 = malloc((size_t)size * sizeof(msg[0]));
		text2[0] = L'\0';
		WCHAR* prev_color = msg;
		do
		{
			int bg = ctoh(color[1]);
			if (bg == -1)
			{
				printf("wrong hex character '%lc' at index %llu", color[1], color + 1 - msg);
				free(text2);
				return;
			}
			bg <<= 4;
			int fg = ctoh(color[2]);
			if (fg == -1)
			{
				printf("wrong hex character '%lc' at index %llu", color[2], color + 2 - msg);
				free(text2);
				return;
			}
			bg |= fg;

			size += 2 + 1 + 3 + 1; //<text><foreground>;<background>
			WCHAR* tmp = realloc(text2, (size_t)size * sizeof(text2[0]));
			if (tmp == NULL)
			{
				printf("couldn't alloc tmp\n");
				free(text2);
				return;
			}
			text2 = tmp;

			msg[color + 0 - msg] = LCSI[0];
			msg[color + 1 - msg] = LCSI[1];
			msg[color + 2 - msg] = L'\0';

			WCHAR b_fg[3], b_bg[4];
			int tmp_fg = 30, tmp_bg = 40;

			if (bg & FOREGROUND_RED)         tmp_fg += 1;
			if (bg & FOREGROUND_GREEN)       tmp_fg += 2;
			if (bg & FOREGROUND_BLUE)        tmp_fg += 4;
			if (bg & FOREGROUND_INTENSITY)   tmp_fg += 60;

			if (bg & BACKGROUND_RED)         tmp_bg += 1;
			if (bg & BACKGROUND_GREEN)       tmp_bg += 2;
			if (bg & BACKGROUND_BLUE)        tmp_bg += 4;
			if (bg & BACKGROUND_INTENSITY)   tmp_bg += 60;

			swprintf(b_fg, 3, L"%i", tmp_fg);
			swprintf(b_bg, 4, L"%i", tmp_bg);

			wcscat_s(text2, size, prev_color);
			wcscat_s(text2, size, b_bg);
			wcscat_s(text2, size, L";");
			wcscat_s(text2, size, b_fg);
			wcscat_s(text2, size, L"m");

			color += 3; // jump after the 2. hex
			prev_color = color;
		} while ((color = wcschr(color, L'§')) != NULL);

		wcscat_s(text2, size, prev_color);

		printf("%S\n" CSI "0m", text2);

		free(text2);
	}
	else printf("%S\n", msg);
}

void command_engine(WCHAR* command)
{
	if (wcscmp(command, L"help") == 0)
	{
		STARTUPINFOA si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);

		const CHAR exe[] = "C:\\Windows\\System32\\cmd.exe";
		CHAR arg[] = "/c cls &" "echo commands:&echo \t/help - help menu (this)&echo:&echo You can color your texts (and even your name) with the paragraph symbol ('§') and then write 2 hex codes, background hex and foreground hex (check color /? in cmd for the colors)&echo:&" "pause";
/*
commands:
/help - help menu (this)

You can color your texts (and even your name) with the paragraph symbol ('§') and then write 2 hex codes, background hex and foreground hex (check color /? in cmd for the colors)

*/
/*
for e in a.split("\n"):
	if(e == ""): print("echo:", end="&")
	else:
		e = e.replace("\t", "\\t")
		print(f"echo {e}", end="&")
*/

		if (!CreateProcessA(exe, arg, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
		{
			printf(CSI"2K" CSI "%i;0H" "Failed to create new cmd window %li", GetConsoleCursorPosition(cmd).Y + 1, GetLastError());
			char a = _getch();
			return;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		printf(CSI"2K" CSI "%i;0H" "\"%S\" command is invalid", GetConsoleCursorPosition(cmd).Y + 1, command);
		char a = _getch();
	}
}