#pragma once


#define EXPORT __declspec(dllexport)


#include "PrimaryDataStorageElement.h"
#include <mutex>



class PrimaryDataStorageElement;

struct EXPORT currentStorageElementGrouping
{
	~currentStorageElementGrouping();
	PrimaryDataStorageElement ** elementList;
	int elementListCount;
	void addToCurrentStorageElementGrouping(PrimaryDataStorageElement *);
};

class EXPORT PrimaryDataStorage
{
private:
	int maximumStorageElementCount;
	int mainStorageElementCount;
	PrimaryDataStorageElement ** mainStorage;
	bool systemNotStarted;
	bool currentlyReceivingData;
	void allocateLargerDataStorage(int newSize);
	std::mutex controlFlagLock;
	std::mutex countLock;
	int nextSystemID;
	bool allowedToRun;

	std::mutex noDataMutex;
	std::condition_variable noDataCV;
public:
	PrimaryDataStorage();
	~PrimaryDataStorage();
	void beginReceivingData();
	void completeReceivingData();
	bool retrieveData(currentStorageElementGrouping **);
	void addMoreData(currentStorageElementGrouping *);
	void exitAllWaiting();

	int getNextSystemID();

};