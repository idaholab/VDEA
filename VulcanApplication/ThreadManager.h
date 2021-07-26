#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include "CommandLineProcessor.h"
#include "PrimaryDataStorage.h"
#include "CodexBase.h"

class ThreadManager
{
public:
	virtual void setCommandLineArguments(CommandLineArgumentSet *);
	virtual void setDataStorage(PrimaryDataStorage *);
	virtual void setCodex(CodexBase *);
	virtual void setupThreading(std::mutex *, std::condition_variable *, bool *);
	virtual void setMessageHandlingSystem(MessageHandlingSystem *);
	
	std::thread * getThread();

protected:
	virtual void threadMainLoop() = 0;
	virtual bool checkBasicElements();
	virtual void waitForOverallStart();
	CommandLineArgumentSet * commandArguments;
	PrimaryDataStorage * dataStorage;
	MessageHandlingSystem * messageHandler;

	CodexBase * currentCodex;

	std::mutex * lock;
	std::condition_variable * cv;
	bool * run;

	std::thread * runningThread;
};