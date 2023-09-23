#include "server.h"
#include "utils.h"

typedef struct User {
	SOCKADDR_IN ip;
	WCHAR* name;
	UINT timeout;
} User;

typedef struct TimeoutThreadParam
{
	User* connects;
	int* curConnects;
} TimeoutThreadParam;

DWORD WINAPI timeoutThreadCallback(LPVOID param)
{
	User* connects = ((TimeoutThreadParam*)param)->connects;
	int* curConnects = ((TimeoutThreadParam*)param)->curConnects;
	WCHAR addressBuffer[IPLEN + 1]; // 255.255.255.255\0
	for (;;)
	{
		Sleep(1000);
		for (int i = 0; i < *curConnects; ++i)
		{
			if (--connects[i].timeout > 0) continue;

			ip_to_wstring(addressBuffer, sizeof(addressBuffer) / sizeof(addressBuffer[0]), &connects[i].ip);

			printf("[%i]: %ls:%i timed out\n", i, addressBuffer, connects[i].ip.sin_port);

			for (int k = i + 1; k < *curConnects; ++k)
			{
				connects[k - 1] = connects[k];
			}

			--i;
			--*curConnects;
		}
	}
	return 0;
}

int find_addr_in_users(SOCKADDR_IN* addr, User* users)
{
	USHORT tmpPort = addr->sin_port;
	ULONG tmpIp = addr->sin_addr.s_addr;

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		SOCKADDR_IN tmp = users[i].ip;
		if (tmp.sin_addr.s_addr == tmpIp && tmp.sin_port == tmpPort) return i;
		else if (tmp.sin_port == 0) break;
	}
	return -1;
}

void runServer()
{
	printf("this is server\n");

	SOCKADDR_IN recvAddr = { 0 };
	SOCKADDR_IN senderAddr = { 0 };

	WCHAR recvBuf[MAX_PACKET] = { 0 };
	WCHAR addressBuffer[IPLEN + 1]; // 255.255.255.255\0

	User connections[MAX_CONNECTIONS];
	int curConnects = 0;
	int prevCurConnects = 0;

	ZeroMemory(&connections, sizeof(connections));

	if (set_addr(&recvAddr))
	{
		closesocket(sock);
		return;
	}

	int iResult = bind(sock, (SOCKADDR*)&recvAddr, sizeof(recvAddr));
	if (iResult != 0)
	{
		handleWSAError(__LINE__);
		closesocket(sock);
		return;
	}

	TimeoutThreadParam param = { connections, &curConnects };
	HANDLE timeoutThread = CreateThread(NULL, 0, timeoutThreadCallback, &param, 0, NULL);
	if (timeoutThread == NULL)
	{
		printf("error while making timeout thread: %li\n", GetLastError());
		return;
	}

	for (;;)
	{
		if (prevCurConnects != curConnects)
		{
			printf("connections: %i\n", curConnects);
			prevCurConnects = curConnects;
		}

		iResult = my_wrecive(&sock, &recvBuf[10], &senderAddr);
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT) continue;
			else return;
		}

		recvBuf[iResult + 10] = L'\0';

		ip_to_wstring(addressBuffer, sizeof(addressBuffer) / sizeof(addressBuffer[0]), &senderAddr);
		printf("message received \"%ls\" from %ls:%i\n", &recvBuf[10], addressBuffer, senderAddr.sin_port);

		int place = find_addr_in_users(&senderAddr, connections);
		if (place == -1)
		{
			place = curConnects;
			connections[place].ip = senderAddr;
			printf("added %ls:%i to list with timeout %us\n", addressBuffer, senderAddr.sin_port, TIMEOUT);
			++curConnects;
		}

		connections[place].timeout = TIMEOUT;

		if (iResult < 1) continue;

		WCHAR tmp_char = recvBuf[10];
		swprintf(recvBuf, sizeof(recvBuf) / sizeof(recvBuf[0]), L"%.10i", curConnects);
		recvBuf[10] = tmp_char;

		for (int i = 0; i < curConnects; i++)
		{
			SOCKADDR_IN tmp = connections[i].ip;
			if (tmp.sin_port != 0)
			{
				ip_to_wstring(addressBuffer, sizeof(addressBuffer) / sizeof(addressBuffer[0]), &tmp);
				printf("[%i]: sending \"%ls\" to %ls:%i\n", i, &recvBuf[10], addressBuffer, tmp.sin_port);
				if (my_wsend(&sock, recvBuf, &tmp) == SOCKET_ERROR) return;
			}
		}
	}

	closesocket(sock);
}