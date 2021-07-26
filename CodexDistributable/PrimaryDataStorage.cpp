#include "PrimaryDataStorage.h"
#include <mutex>

PrimaryDataStorage::PrimaryDataStorage()
{
	//This will of course need to be fleshed out more at a later point.
	systemNotStarted = true;
	currentlyReceivingData = false;
	allowedToRun = true;
	//need some kind of idea as to how exactly the buffer stored here will behave
	nextSystemID = 0;
	maximumStorageElementCount = 100;
	mainStorageElementCount = 0;
	mainStorage = (PrimaryDataStorageElement **) malloc(maximumStorageElementCount*sizeof(PrimaryDataStorageElement*));
	
}

currentStorageElementGrouping::~currentStorageElementGrouping()
{
}

PrimaryDataStorage::~PrimaryDataStorage()
{
	for (int i = 0; i < mainStorageElementCount; i++)
	{
		free(mainStorage[i]);
	}
	free(mainStorage);
}

void PrimaryDataStorage::beginReceivingData()
{
	const std::lock_guard<std::mutex> lock(controlFlagLock);
	systemNotStarted = false;
	currentlyReceivingData = true;
}

void PrimaryDataStorage::completeReceivingData()
{
	const std::lock_guard<std::mutex> lock(controlFlagLock);
	currentlyReceivingData = false;
}

void PrimaryDataStorage::allocateLargerDataStorage(int newSize)
{
	PrimaryDataStorageElement ** newArray = (PrimaryDataStorageElement**)malloc(sizeof(PrimaryDataStorageElement*)*newSize);

	for (int i = 0; i < mainStorageElementCount; i++)
	{
		newArray[i] = mainStorage[i];
	}

	free(mainStorage);
	maximumStorageElementCount = newSize;
	mainStorage = newArray;
}

void PrimaryDataStorage::addMoreData(currentStorageElementGrouping * input)
{
	const std::lock_guard<std::mutex> lock(controlFlagLock);
	
	if (input->elementListCount + mainStorageElementCount > maximumStorageElementCount)
	{
		allocateLargerDataStorage((input->elementListCount + maximumStorageElementCount )*2);
	}

	for (int i = 0; i < input->elementListCount; i++)
	{
		mainStorage[i + mainStorageElementCount ] = input->elementList[i];
	}

	mainStorageElementCount += input->elementListCount;
	noDataCV.notify_all();
}

bool PrimaryDataStorage::retrieveData(currentStorageElementGrouping ** blankSet)
{
	if (!systemNotStarted && !currentlyReceivingData && mainStorageElementCount == 0) return false;

	std::unique_lock<std::mutex> lck((noDataMutex));

	//modifications were needed as multithreading has problems sometimes like this

	while (mainStorageElementCount == 0 && allowedToRun ) noDataCV.wait_for(lck,std::chrono::milliseconds(250));
	if (!allowedToRun) return false;

	const std::lock_guard<std::mutex> lock(controlFlagLock);

	(*blankSet) = new currentStorageElementGrouping();

	(*blankSet)->elementListCount = mainStorageElementCount;

	(*blankSet)->elementList = (PrimaryDataStorageElement **)malloc((*blankSet)->elementListCount * sizeof(PrimaryDataStorageElement *));

	for (int i = 0; i < (*blankSet)->elementListCount; i++)
	{
		(*blankSet)->elementList[i] = mainStorage[i];
	}

	mainStorageElementCount = 0;

	return true;
}

int PrimaryDataStorage::getNextSystemID()
{
	const std::lock_guard<std::mutex> lock(countLock);
	int retVal = nextSystemID;
	nextSystemID++;
	return retVal;
}

void currentStorageElementGrouping::addToCurrentStorageElementGrouping(PrimaryDataStorageElement * element)
{
	elementListCount++;
	PrimaryDataStorageElement ** newList = (PrimaryDataStorageElement **)malloc(sizeof(PrimaryDataStorageElement*)*elementListCount);
	for (int i = 0; i < elementListCount - 1; i++)
	{
		newList[i] = elementList[i];
	}
	newList[elementListCount - 1] = element;

	free(elementList);

	elementList = newList;
}

void PrimaryDataStorage::exitAllWaiting()
{
	allowedToRun = false;
	noDataCV.notify_all();
}