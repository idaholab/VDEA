#pragma once
#include "CodexBase.h"
#include "CodexHeader.h"
enum InstalledCodexes {Demo=0};

class CodexBus
{
private:
	CodexBase ** currentCodexes;
	int currentCodexCount;

public:
	CodexBus();
	void initalizeCodexes();
	bool initalizedSucessfully();
	void initializeCodexPlugins(char * pluginDirectory, MessageHandlingSystem *);
	char * getMessages();
	int checkIfTypeExists(const char * codexID);
	CodexBase * getCodex(char *);
	void addPluginToCurrentCodexes(CodexBase *);
	void printAllCodexes(MessageHandlingSystem *);
	CodexHeader * GetCurrentCodexRepresentation();
};
