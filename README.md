# My CMD based UDP chat server

## Info about it
This was my first project using UDP on windows. I did this to harden my C knowledge and to learn the basics of networking on windows.

## Usage
1. Download one of the .exe and .bat from x64/\<version>
2. run both
3. follow instructions on window
4. chat with yourself

after finishing the login you can use commands (use /help to get help)



#### PS
If you looked into client.c, then yes, the mouse has a full thread for itself. This is because this is a cmd application and GetMessage only works with window applications, so it waits forever for a message (but there will be no message) -> fully freezes thread.