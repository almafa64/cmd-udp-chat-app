#ifndef MAIN
#define MAIN

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12) // fix for udp sending to disconnected client crash

#define IPLEN 15
#define PORTLEN 5
#define MAX_CONNECTIONS 20
#define TIMEOUT 5
#define MAX_PACKET 1024			// with default name (anom) you can write MAX_PACKET - 10 characters
#define MAX_LINE_HISTORY 20
#define MAX_MSG_MEMORY 20

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static_assert(TIMEOUT >= 1, "TIMEOUT cannot be smaller than 1");

BOOL isClient;
SOCKET sock;

int mouseWheel;
HHOOK mouseHook;

HANDLE cmd;
HWND cmdWindow;
SHORT cmdCols, cmdRows;

typedef struct ReciveNode
{
	WCHAR* recived;
	struct ReciveNode* next;
	struct ReciveNode* prev;
	SOCKADDR_IN ip;
} ReciveNode;

ReciveNode* newReciveNode();
void deleteReciveNode(ReciveNode** node);

ReciveNode *reciveFirst, *reciveLast;
DWORD WINAPI reciveThread(LPVOID param);

#endif