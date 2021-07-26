//2020-05-01 This application is responsible for taking the various different data formats that are used across the land and turning them into a different format

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CodexBus.h"
#include "DataInputManager.h"
#include "MessageHandlingSystem.h"
#include "CommandLineProcessor.h"
#include "PrimaryDataStorage.h"
#include "DataOutputManager.h"

#define Version "0.0"

using namespace std::chrono_literals;

int main(int argCount, char ** commandLineArgs)
{
	
	std::mutex overalMutex;
	std::condition_variable overallConditionVariable;
	bool run = false;
	
	MessageHandlingSystem * messageHandler = new MessageHandlingSystem();

	CodexBus * codexBus = new CodexBus();
	codexBus->initalizeCodexes();

	
	CommandLineArgumentSet * processedCommandLineArgs = CommandLineProcessor::processCommandLineArguments(messageHandler, codexBus,argCount, commandLineArgs);

	//check the plugin directory
	if (processedCommandLineArgs->pluginLocation != nullptr) codexBus->initializeCodexPlugins(processedCommandLineArgs->pluginLocation, messageHandler);

	

	if (CommandLineProcessor::checkCodexValues(codexBus,processedCommandLineArgs,messageHandler) && processedCommandLineArgs->commandsReadSucessfully())
	{
	
		if (processedCommandLineArgs->printCodexes)
		{
			codexBus->printAllCodexes(messageHandler);
		}
		else
		{
			PrimaryDataStorage * dataStorage = new PrimaryDataStorage();

			DataOutputManager * dataOutputManager = new DataOutputManager();
			dataOutputManager->setCommandLineArguments(processedCommandLineArgs);
			dataOutputManager->setDataStorage(dataStorage);
			dataOutputManager->setMessageHandlingSystem(messageHandler);
			dataOutputManager->setCodex(codexBus->getCodex(processedCommandLineArgs->outputCodex));
			dataOutputManager->setupThreading(&overalMutex, &overallConditionVariable, &run);

			DataInputManager * dataInputManager = new DataInputManager();
			dataInputManager->setCommandLineArguments(processedCommandLineArgs);
			dataInputManager->setDataStorage(dataStorage);
			dataInputManager->setCodex(codexBus->getCodex(processedCommandLineArgs->inputCodex));
			dataInputManager->setMessageHandlingSystem(messageHandler);
			dataInputManager->setupThreading(&overalMutex, &overallConditionVariable, &run);

			run = true;
			overallConditionVariable.notify_all();

			dataOutputManager->getThread()->join();
			dataInputManager->getThread()->join();

			delete processedCommandLineArgs;
			delete codexBus;
			delete dataStorage;
			delete dataOutputManager;
			delete dataInputManager;
		}
	}
	else
	{
		messageHandler->fatal("Missing full command line tags");
		if (processedCommandLineArgs->printCodexes)
		{
			codexBus->printAllCodexes(messageHandler);
		}
	}
	delete messageHandler;
}

