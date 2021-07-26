#include "ThreadManager.h"
#include <stdio.h>
#include <iostream>
void ThreadManager::setCommandLineArguments(CommandLineArgumentSet * _commandLineSet)
{
	commandArguments = _commandLineSet;
}

void ThreadManager::setDataStorage(PrimaryDataStorage * mainStorageLoc)
{
	dataStorage = mainStorageLoc;
}

void ThreadManager::setCodex(CodexBase * codex)
{
	currentCodex = codex;
}

void ThreadManager::setupThreading( std::mutex * lock, std::condition_variable * conditionVariable, bool * run)
{
	this->lock = lock;
	this->cv = conditionVariable;
	this->run = run;
	runningThread = new std::thread(&ThreadManager::threadMainLoop,this);
}

void ThreadManager::setMessageHandlingSystem(MessageHandlingSystem * messageHandler)
{
	this->messageHandler = messageHandler;
}

std::thread * ThreadManager::getThread()
{
	return runningThread;
}

bool ThreadManager::checkBasicElements()
{
	if (messageHandler == nullptr)
	{
		std::cerr << "Message handler is not set for current manager" << std::endl;
		return false;
	}
	if (commandArguments == nullptr)
	{
		messageHandler->fatal("Command line arguments are not set for current manager");
		return false;
	}
	if (dataStorage == nullptr)
	{
		messageHandler->fatal("Data storage is not set for current manager");
		return false;
	}
	if (currentCodex == nullptr)
	{
		messageHandler->fatal("Current codex is not set for current manager");
		return false;
	}
	return true;
}

void ThreadManager::waitForOverallStart()
{
	std::unique_lock<std::mutex> lck(*(this->lock));

	while (!*run) cv->wait_for(lck,std::chrono::milliseconds(250)); //For the record, this is not busy waiting, 
	//this is preventing spurious wake up calls 

}