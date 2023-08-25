#include "args.h"

argLength isArg(char* arg)
{
	char tmp = arg[1];
	if (arg[0] == '-' && tmp != '\0')
	{
		if (tmp == '-' && arg[2] != '\0') return LENGTH_LONG;
		else return LENGTH_SHORT;
	}
	else return LENGTH_ERROR;
}

void arg_engine(int argc, char** argv)
{
	int argIndex = 0;
	argLength t;
	for (int i = 1; i < argc; ++i)
	{
		char* tmp = argv[i];
		if (argIndex == 0)
		{
			t = isArg(tmp);
			if (t == LENGTH_ERROR) continue;
		}

		char* arg = &tmp[t + argIndex];
		if (t == LENGTH_SHORT)
		{
			arg_processing(t, (argText) { .chr = arg[0] }, i, argv);

			if (tmp[t + ++argIndex] == '\0')
			{
				argIndex = 0;
				continue;
			}
			else --i;
		}
		else arg_processing(t, (argText){.string = arg}, i, argv);
	}
}