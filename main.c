#include "main.h"
#include "args.h"
#include "utils.h"
#include "server.h"
#include "client.h"

#include <locale.h>
#include <io.h> //for _setmode
#include <fcntl.h> //for _O_U16TEXT

void arg_processing(argLength length, argText arg, int index, char** argv)
{
	if (length == LENGTH_SHORT)
	{
		switch (arg.chr)
		{
		case 's':
			isClient = FALSE;
			break;
		}
	}
	else
	{
		if (strcmp(arg.string, "server") == 0) isClient = FALSE;
		else if (strcmp(arg.string, "height") == 0)
		{
			cmdRows = atoi(argv[index + 1]);
			SetConsoleSize(cmd, cmdCols, cmdRows);
		}
		else if (strcmp(arg.string, "width") == 0)
		{
			cmdCols = atoi(argv[index + 1]);
			SetConsoleSize(cmd, cmdCols, cmdRows);
		}
	}
}

int main(int argc, char* argv[])
{
	isClient = TRUE;
	mouseWheel = 0;
	mouseHook = NULL;

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %i\n", iResult);
		return 1;
	}

	cmd = GetStdHandle(STD_OUTPUT_HANDLE);
	cmdWindow = GetConsoleWindow();

	GetConsoleSize(cmd, &cmdCols, &cmdRows, NULL);

	arg_engine(argc, argv);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		handleWSAError(__LINE__);
		return 1;
	}

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);

	DWORD timeout = 1;
	iResult = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if (iResult == SOCKET_ERROR)
	{
		handleWSAError(__LINE__);
		closesocket(sock);
		return 1;
	}

	setlocale(LC_ALL, "");
	//int a = _setmode(_fileno(stdout), _O_U16TEXT);
	int a = _setmode(_fileno(stdin), _O_U16TEXT);
	EnableVTMode();

	if (isClient) runClient();
	else runServer();

	system("Pause");
	WSACleanup();
	return 0;
}