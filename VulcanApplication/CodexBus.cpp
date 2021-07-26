#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <filesystem>
#include "CodexBus.h"
#include "DataExchangeCodex.h"
#include "TDMSCodex.h"
#include "MatlabCodex.h"
#include "RecursionEngine.h"


CodexBus::CodexBus()
{
	currentCodexCount = 3;
	currentCodexes = (CodexBase **)malloc(sizeof(CodexBase*)*currentCodexCount);
}

void CodexBus::initalizeCodexes()
{
	currentCodexes[0] = new DataExchangeCodex();
	currentCodexes[1] = new TDMSCodex();
	currentCodexes[2] = new MatlabCodex();
	//This needs to build the entire list of all of the codexes in order to be used
	//This is where they will be added
}

bool CodexBus::initalizedSucessfully()
{
	//This is just a stub at the moment because it needs to have the codexes to be created in order to be read
	return true;
}

CodexBase * CodexBus::getCodex(char * identifier)
{
	for (int i = 0; i < currentCodexCount; i++)
	{
		if (std::strcmp(identifier, currentCodexes[i]->getIdentifier()) == 0)
		{
			return currentCodexes[i];
		}
	}
	return nullptr;
}

int CodexBus::checkIfTypeExists(const char * codexID)
{
	for (int i = 0; i < currentCodexCount; i++)
	{
		const char * currentCodexToCheck = currentCodexes[i]->getIdentifier();
		if (std::strcmp(currentCodexToCheck, codexID) == 0)
		{
			return i;
		}
	}
	return -1;
}

void CodexBus::initializeCodexPlugins(char * pluginDirectory, MessageHandlingSystem * currentMessageHandler)
{
	std::filesystem::path directoryPath = std::filesystem::path(pluginDirectory);

	if (std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath))
	{
		RecursionEngine::FileListing * plugins = RecursionEngine::scanDirectory(pluginDirectory);
		for (int i = 0; i < plugins->totalFileCount; i++)
		{
			//check to make sure these plugins are accurate

			if (checkExtension(plugins->filePaths[i], ".dll"))
			{
				HINSTANCE currentLibrary = LoadLibraryA(plugins->filePaths[i]);

				if (!currentLibrary)
				{
					std::cout << "Well The library does not seem to exist, that is not good" << std::endl;
					currentMessageHandler->codexError(currentMessageHandler->concatenateMessages(3, "Codex Plugin: ", plugins->filePaths[i], " Is not loading"));
					return;
				}

				typedef CodexBase * (__cdecl * pluginBase) (void);

				pluginBase nextPlugin = (pluginBase) GetProcAddress(currentLibrary, "constructCodex");
				
				if (!nextPlugin)
				{
					currentMessageHandler->codexError(currentMessageHandler->concatenateMessages(3, "Codex Plugin: ", plugins->filePaths[i] ," Does not contain construction element"));
					return;
				}

				addPluginToCurrentCodexes(nextPlugin());
			}
		}
	}
	else
	{
		currentMessageHandler->codexError(pluginDirectory);
		return;
	}
}

void CodexBus::addPluginToCurrentCodexes(CodexBase * newCodex)
{
	currentCodexCount++;
	CodexBase ** newCodexes = (CodexBase **)malloc(sizeof(CodexBase *) *currentCodexCount);

	for (int i = 0; i < currentCodexCount - 1; i++)
	{
		newCodexes[i] = currentCodexes[i];
	}
	newCodexes[currentCodexCount - 1] = newCodex;

	free(currentCodexes);
	currentCodexes = newCodexes;
}

void CodexBus::printAllCodexes(MessageHandlingSystem * messager)
{
	for (int i = 0; i < currentCodexCount; i++)
	{
		messager->fatal(currentCodexes[i]->getIdentifier());
	}
}

CodexHeader * CodexBus::GetCurrentCodexRepresentation()
{
	CodexHeader * codexHeaderListing = (CodexHeader *)malloc(sizeof(CodexHeader));
	codexHeaderListing->totalPluginListing = currentCodexCount;
	codexHeaderListing->stringRepresentaion =(const char **) malloc(sizeof(const char *) * currentCodexCount);
	for (int i = 0; i < currentCodexCount; i++)
	{
		codexHeaderListing->stringRepresentaion[i] = currentCodexes[i]->getIdentifier();
	}

	return codexHeaderListing;
}
