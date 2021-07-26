#pragma once
#include "MessageHandlingSystem.h"
#include "CodexBus.h"

enum CommandLineTag { InputDirectory = 0, OutputDirectory = 1, InputType = 2, OutputType = 3, PluginDirectory = 4, PrintCodexes = 5 };

struct CommandLineArgumentSet
{
	bool commandsReadSucessfully();
	char * inputDirectory = nullptr;
	char * outputDirectory = nullptr;
	char * inputCodex = nullptr;
	char * outputCodex = nullptr;
	char * pluginLocation = nullptr;
	bool printCodexes = false;
	char * getMessages();

	void setProperTag(char * value, int currentTag);

};

class CommandLineProcessor
{
private:
	static const char * currentCommandLineTags[] ;
	static const int commandLineTagCount;
	static int checkIfCommandLineTag(const char * count);
	static bool checkCommandLineValue(char * commandLineTag, int MatchedCommandLineTag, MessageHandlingSystem *);
public:
	static CommandLineArgumentSet * processCommandLineArguments(MessageHandlingSystem * messageHandler,CodexBus* codexBus,int argCount,char ** commandLineArguments);
	static bool checkCodexValues(CodexBus *  codexBus, CommandLineArgumentSet * commands, MessageHandlingSystem * messager);
};