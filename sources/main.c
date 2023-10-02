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

ReciveNode* newReciveNode()
{
	ReciveNode* tmp = malloc(sizeof(*tmp));
	if (tmp == NULL) return NULL;
	ZeroMemory(tmp, sizeof(*tmp));
	tmp->recived = malloc(MAX_PACKET * sizeof(tmp->recived[0]));
	if (tmp->recived == NULL) return NULL;
	ZeroMemory(tmp->recived, sizeof(MAX_PACKET * sizeof(tmp->recived[0])));
	return tmp;
}

void deleteReciveNode(ReciveNode* node)
{
	free(node->recived);
	free(node);
}

DWORD WINAPI reciveThread(LPVOID param)
{
	SOCKADDR_IN* ips = (isClient) ? NULL : ((ReciveThreadParam*)param)->recvAddr;
	for (;;)
	{
		int iResult = my_wrecive(&sock, reciveLast->recived, (isClient) ? NULL : &ips[revcivedMsgCount]);
		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT) continue;
			return 1;
		}

		reciveLast->recived[iResult] = L'\0';
		revcivedMsgCount++;

		reciveLast->next = newReciveNode();
		reciveLast->next->prev = reciveLast;
		reciveLast = reciveLast->next;
	}
	return 0;
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
		goto end;
	}

	cmd = GetStdHandle(STD_OUTPUT_HANDLE);
	cmdWindow = GetConsoleWindow();

	GetConsoleSize(cmd, &cmdCols, &cmdRows, NULL);

	arg_engine(argc, argv);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		handleWSAError(__LINE__);
		goto end;
	}

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);

	DWORD timeout = 1;
	iResult = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	if (iResult == SOCKET_ERROR)
	{
		handleWSAError(__LINE__);
		goto end;
	}

	setlocale(LC_ALL, "");
	//int a = _setmode(_fileno(stdout), _O_U16TEXT);
	int a = _setmode(_fileno(stdin), _O_U16TEXT);
	EnableVTMode();

	revcivedMsgCount = 0;
	reciveFirst = reciveLast = newReciveNode();
	if (reciveFirst == NULL)
	{
		printf("error while mallocing memory for reciveFirst\n");
		goto end;
	}

	if (isClient) runClient();
	else runServer();

end:
	closesocket(sock);
	system("Pause");
	WSACleanup();
	return 1;
}