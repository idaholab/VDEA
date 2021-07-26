#pragma once
#include "ThreadManager.h"

class DataOutputManager : public ThreadManager
{
private:
	void threadMainLoop();
	bool checkSetParameters();
public:
	DataOutputManager();
	
};