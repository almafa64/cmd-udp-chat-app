#ifndef ARGS
#define ARGS

typedef enum argLength {
	LENGTH_ERROR,
	LENGTH_SHORT,
	LENGTH_LONG
} argLength;

typedef union argText {
	char* string;
	char chr;
} argText;

void arg_engine(int argc, char** argv);
void arg_processing(argLength length, argText arg, int index, char** argv);

#endif