#pragma once

#define EXPORT __declspec(dllexport)



class EXPORT MessageHandlingSystem
{
public:
	MessageHandlingSystem();

	void fatal(char * message);
	void fatal(const char * message);

	void operateErrorHandling();

	void sendClosingMessage();

	void directoryError(char * commandLineTag);

	void codexError(char * commandLineTag);

	static char * concatenateMessages(int elementCount, ...);
	static char * concatenateStrings(int elementCount, ...);
};