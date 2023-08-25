gcc  -Iheaders -Wall -Os -s -o test sources/main.c sources/args.c sources/utils.c sources/client.c sources/server.c -fcommon -lws2_32
pause