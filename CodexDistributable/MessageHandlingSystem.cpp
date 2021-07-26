#include "MessageHandlingSystem.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdarg.h>

MessageHandlingSystem::MessageHandlingSystem()
{

}


char * MessageHandlingSystem::concatenateMessages(int elementCount, ...)
{
	int totalSize = 0;
	char * totalMessage;

	va_list args;
	va_start(args, elementCount);
	
	for (int i = 0; i < elementCount;i++)
	{
		totalSize += strlen(va_arg(args, char *))+1;
	}
	va_end(args);

	totalSize++;

	totalMessage = (char *)calloc(1, totalSize);

	va_start(args, elementCount);


	for (int j = 0; j < elementCount; j++)
	{
		sprintf_s(totalMessage, totalSize, "%s%s%s", totalMessage, " ", va_arg(args, char *));
	}



	va_end(args);

	return totalMessage;
}

char * MessageHandlingSystem::concatenateStrings(int elementCount, ...)
{
	int totalSize = 0;
	char * totalMessage;

	va_list args;
	va_start(args, elementCount);

	for (int i = 0; i < elementCount; i++)
	{
		totalSize += strlen(va_arg(args, char *)) + 1;
	}
	va_end(args);

	totalSize++;

	totalMessage = (char *)calloc(1, totalSize);

	va_start(args, elementCount);


	for (int j = 0; j < elementCount; j++)
	{
		sprintf_s(totalMessage, totalSize, "%s%s", totalMessage, va_arg(args, char *));
	}


	va_end(args);

	return totalMessage;
}

void MessageHandlingSystem::directoryError(char * commandLineTag)
{
	char * errorMessage = concatenateMessages(3, "The command line argument: ", commandLineTag, " is a nonexistant directory");
	fatal(errorMessage);
	free(commandLineTag);
}

void MessageHandlingSystem::codexError(char * commandLineTag)
{
	char * errorMessage = concatenateMessages(3, "The command line argument: ", commandLineTag, " is not a recognized codex");
	fatal(errorMessage);
	free(commandLineTag);
}

void MessageHandlingSystem::fatal(const char * message)
{
	std::cerr << message << std::endl;

}

void MessageHandlingSystem::fatal(char * message)
{
	std::cerr << message << std::endl;
}



