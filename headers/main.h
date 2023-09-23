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

#pragma comment(lib, "Ws2_32.lib")

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12) // fix for udp sending to disconnected client crash

#define IPLEN 15
#define PORTLEN 5
#define MAX_CONNECTIONS 20
#define TIMEOUT 20
#define MAX_PACKET 1024			// with default name (anom) you can write MAX_PACKET - 20 characters
#define MAX_LINE_HISTORY 20

BOOL isClient;
SOCKET sock;

int mouseWheel;
HHOOK mouseHook;

HANDLE cmd;
HWND cmdWindow;
SHORT cmdCols, cmdRows;

#endif