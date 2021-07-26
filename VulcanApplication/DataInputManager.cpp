#include "DataInputManager.h"
#include "RecursionEngine.h"
#include <stdio.h>
#include <iostream>
#include <filesystem>
DataInputManager::DataInputManager()
{

}

void DataInputManager::threadMainLoop()
{
	waitForOverallStart();
	if (!checkSetParameters())
	{
		dataStorage->exitAllWaiting();
		return;
	}



	RecursionEngine::FileListing * itm = RecursionEngine::scanDirectory((char *)commandArguments->inputDirectory);

	dataStorage->beginReceivingData();
	  
	for (int i = 0; i < itm->totalFileCount; i++)
	{
		currentCodex->processFile(dataStorage, messageHandler, itm->filePaths[i]);
	}

	dataStorage->completeReceivingData();
}

bool DataInputManager::checkSetParameters()
{
	if (!checkBasicElements()) return false;
	std::error_code filesystemError;
	if (!std::filesystem::exists(commandArguments->inputDirectory,filesystemError))
	{
		messageHandler->fatal("Filesystem error:\r\n");
		messageHandler->fatal(filesystemError.message().c_str());
		return false;
	}
	return true;
}
