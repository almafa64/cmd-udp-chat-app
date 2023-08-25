#include "client.h"
#include "utils.h"

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode != HC_ACTION || GetForegroundWindow() != cmdWindow) return CallNextHookEx(NULL, nCode, wParam, lParam);

	MSLLHOOKSTRUCT* tmp = (MSLLHOOKSTRUCT*)lParam;
	if (wParam == WM_MOUSEWHEEL) mouseWheel = HIWORD(tmp->mouseData) == 120 ? 1 : -1;

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

DWORD WINAPI messageThread(LPVOID lpParam)
{
	mouseHook = SetWindowsHookExW(WH_MOUSE_LL, MouseHookProc, NULL, 0);
	if (mouseHook == NULL) return 1;
	GetMessageW(NULL, NULL, 0, 0);
	return 0;
}

BOOL CALLBACK ctrl_handler(DWORD dwCtrlType)
{
	printf("unhooking\n");
	if (mouseHook)
	{
		UnhookWindowsHookEx(mouseHook);
		ExitProcess(0);
		mouseHook = NULL;
	}
	return TRUE;
}

void runClient()
{
	printf("this is client\n");

	SOCKADDR_IN servaddr;
	if (set_addr(&servaddr))
	{
		closesocket(sock);
		return;
	}

	WCHAR name[30 + sizeof(CSI"m: ")] = { 0 }; // <name><CSI>m: \0
	const int nameWordSize = sizeof(name) / sizeof(name[0]);
	printf("name (nothing -> anom): ");
	fflush(stdin);
	fflush(stdout);
	fgetws(name, 31, stdin);
	int nameLen = wcslen(name);
	if (name[nameLen - 1] == L'\n') --nameLen;
	if (nameLen == 0)
	{
		wcscpy_s(name, nameWordSize, L"anom");
		nameLen = 4;
	}
	name[nameLen] = L'\0';

	int iResult = my_wsend(&sock, L"", &servaddr);
	if (iResult == SOCKET_ERROR) return;

	printf(CSI "?1049h"); // enable alt cmd

	// print out ip and name on top
	printf(CSI "?25l" CSI "3;%dr" "name: ", cmdRows - 5); // hide cursor -> Set scrolling margins to 3, h-5

	WCHAR* tmpMsg = malloc(sizeof(name));
	wcscpy_s(tmpMsg, nameWordSize, name);
	printColoredText(tmpMsg, nameWordSize);
	free(tmpMsg);

	PrintHorizontalBorder(cmdCols, TRUE);
	printf(CSI "%d;1H", cmdRows - 4); // jump cursor
	PrintHorizontalBorder(cmdCols, FALSE);

	printf(CSI "3;1H"); //jump to (1;3)

	name[nameLen++] = L'\x1b'; //ESC
	name[nameLen++] = L'[';
	name[nameLen++] = L'm';
	name[nameLen++] = L':';
	name[nameLen++] = L' ';
	name[nameLen] = L'\0';

	// SCROLL MARGIN
	size_t marginHeight = (((size_t)cmdRows) - 5) - 3;
	size_t marginAbsBottom = ((size_t)cmdRows) - 5 - 1, marginAbsTop = (size_t)3 - 1;
	int yAbsPos = 0;
	size_t line_size = cmdCols;
	size_t lines_size = line_size * (MAX_LINE_HISTORY + 1 + marginHeight);
	CHAR_INFO* lines = malloc(lines_size * sizeof(*lines));
	if (lines == NULL)
	{
		printf("couldn't allocate memory: %li\n", GetLastError());
		return;
	}
	for (size_t i = 0; i < lines_size; ++i)
	{
		lines[i].Char.UnicodeChar = L' ';
		lines[i].Attributes = FOREGROUND_BRIGHT_WHITE;
	}

	WCHAR recvMsg[MAX_PACKET] = { 0 };
	WCHAR sendMsg[MAX_PACKET] = { 0 };
	int realMsgLength = nameLen, printMsgLength = sizeof("message: ") - 1;

	wcscpy_s(sendMsg, MAX_PACKET, name);

	// MOUSE
	int scrolled = 0;

	if (!SetConsoleCtrlHandler(ctrl_handler, TRUE))
	{
		printf("couldn't set ctrl handler %li\n", GetLastError());
		return;
	}

	HANDLE mouseThread = CreateThread(NULL, 0, messageThread, NULL, 0, NULL);
	if (mouseThread == NULL)
	{
		printf("error while making mouse thread: %li\n", GetLastError());
		return;
	}

	for (;;)
	{
		switch (mouseWheel)
		{
		case -1: //down
			mouseWheel = 0;
			if (MAX_LINE_HISTORY == 0)
			{
				printf(CSI "S");
				break;
			}

			if (ReadCharAtPos(cmd, (COORD) { 0, marginAbsBottom }) == L' ') break;
			if (scrolled == MAX_LINE_HISTORY)
			{
				size_t lastLine = lines_size - line_size;
				errno_t error = memcpy_s(lines, lines_size * sizeof(*lines), &lines[line_size], lastLine * sizeof(*lines));
				if (error != 0)
				{
					printf("error while copying: %i", error);
					return;
				}
				for (; lastLine < lines_size; ++lastLine)
				{
					PCHAR_INFO tmp = &lines[lastLine];
					tmp->Char.UnicodeChar = L' ';
					tmp->Attributes = FOREGROUND_BRIGHT_WHITE;
				}

				--scrolled;
			}
			ReadLine(cmd, &lines[line_size * (scrolled)], cmdCols, marginAbsTop);
			printf(CSI "S");
			WriteLine(cmd, &lines[line_size * (marginHeight + ++scrolled)], cmdCols, marginAbsBottom);
			break;
		case 1: //up
			mouseWheel = 0;
			if (scrolled == 0) break;
			ReadLine(cmd, &lines[line_size * (marginHeight + scrolled)], cmdCols, marginAbsBottom);
			printf(CSI "T");
			WriteLine(cmd, &lines[line_size * (--scrolled)], cmdCols, marginAbsTop);
			break;
		}

		while (_kbhit())
		{
			WCHAR tmp = _getwch();
			if (isEscape(tmp))
			{
				switch (_getwch())
				{
					// TODO idk
				}
				goto rec;
			}

			printf(ESC "7");

			if (realMsgLength == nameLen)	printf(CSI "%d;1H" "message: ", cmdRows - 3);
			else							printf(CSI "%d;%dH", cmdRows - 3 + printMsgLength / cmdCols, printMsgLength % cmdCols + 1);

			switch (tmp)
			{
			case L'\r':
			case L'\n':
				sendMsg[realMsgLength] = L'\0';

				realMsgLength = nameLen;
				printMsgLength = sizeof("message: ") - 1;

				if (sendMsg[realMsgLength] == L'/') command_engine(&sendMsg[realMsgLength + 1]);
				else
				{
					iResult = my_wsend(&sock, sendMsg, &servaddr);
					if (iResult == SOCKET_ERROR) return;
				}

				printf(CSI "%d;1H" CSI "2K\n" CSI "2K\n" CSI "2K\n" CSI "2K", cmdRows - 3); //clear bottom
				break;
			case L'\b':
				if (realMsgLength > nameLen)
				{
					--realMsgLength;
					--printMsgLength;
					printf(CSI "D" CSI "P");
				}
				break;
			default:
				if (realMsgLength >= MAX_PACKET - 10 - 1) break;	// <user count (10 number)><name> + \0
				sendMsg[realMsgLength] = tmp;
				++printMsgLength;
				++realMsgLength;
				printf("%lc", tmp);
				break;
			}

			printf(ESC "8");
		}

	rec:
		iResult = my_wrecive(&sock, recvMsg, NULL);
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT) continue;
			return;
		}
		recvMsg[iResult] = '\0';

		iResult -= 10;

		if (MAX_LINE_HISTORY != 0)
		{
			int n = (iResult / 120) + 1;
			yAbsPos += n;

			if (yAbsPos > marginHeight)
			{
				for (;;) //scroll until empty space (ReadCharAtPos(cmd, (COORD) { 0, marginAbsBottom }) == L' ')
				{
					//TODO make function "void history_save_write_down()" (extend "void history_save_down()")
					if (ReadCharAtPos(cmd, (COORD) { 0, marginAbsBottom }) == L' ') break;
					if (scrolled == MAX_LINE_HISTORY)
					{
						size_t lastLine = lines_size - line_size;
						errno_t error = memcpy_s(lines, lines_size * sizeof(*lines), &lines[line_size], lastLine * sizeof(*lines));
						if (error != 0)
						{
							printf("error while copying: %i", error);
							return;
						}
						for (; lastLine < lines_size; ++lastLine)
						{
							PCHAR_INFO tmp = &lines[lastLine];
							tmp->Char.UnicodeChar = L' ';
							tmp->Attributes = FOREGROUND_BRIGHT_WHITE;
						}

						--scrolled;
					}
					ReadLine(cmd, &lines[line_size * (scrolled)], cmdCols, marginAbsTop);
					printf(CSI "S");
					WriteLine(cmd, &lines[line_size * (marginHeight + ++scrolled)], cmdCols, marginAbsBottom);
				}

				for (int i = 0; i < n; ++i)
				{
					//TODO make function "void history_save_down()"
					if (scrolled == MAX_LINE_HISTORY)
					{
						size_t lastLine = lines_size - line_size;
						errno_t error = memcpy_s(lines, lines_size * sizeof(*lines), &lines[line_size], lastLine * sizeof(*lines));
						if (error != 0)
						{
							printf("error while copying: %i", error);
							return;
						}
						for (; lastLine < lines_size; ++lastLine)
						{
							PCHAR_INFO tmp = &lines[lastLine];
							tmp->Char.UnicodeChar = L' ';
							tmp->Attributes = FOREGROUND_BRIGHT_WHITE;
						}

						--scrolled;
					}
					ReadLine(cmd, &lines[line_size * (scrolled++)], cmdCols, marginAbsTop + i);
				}
			}
		}

		/*WCHAR tmp_char = recvMsg[10];
		recvMsg[10] = L'\0';
		int count = _wtoi(recvMsg);
		printf(ESC"7" CSI"1;%lluH" "user count: %i" ESC"8", line_size - sizeof("user count: ") + 2 - int_length(count), count);
		recvMsg[10] = tmp_char;*/

		printColoredText(&recvMsg[10], iResult);
	}
	closesocket(sock);
}