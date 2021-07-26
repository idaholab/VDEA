#pragma once
#include "ThreadManager.h"

class DataInputManager : public ThreadManager
{
private:
	void threadMainLoop();
	bool checkSetParameters();
public:
	DataInputManager();
};