#include "DataOutputManager.h"
#include <filesystem>
DataOutputManager::DataOutputManager()
{

}

void DataOutputManager::threadMainLoop()
{
	waitForOverallStart();
	if (!checkSetParameters())return;

	currentStorageElementGrouping * currentDataSet;

	while (dataStorage->retrieveData(&currentDataSet))
	{
		for (int i = 0; i < currentDataSet->elementListCount; i++)
		{
			PrimaryDataStorageElement * pdse = currentDataSet->elementList[i];
			if (currentCodex->outputFile(pdse, messageHandler, commandArguments->outputDirectory))
			{
				if (pdse->temporaryDataFile) remove(pdse->dataDescription->DataFilePath);
				if(!currentCodex->deleteAdditionalElements())
				{
					delete pdse;
				}
			}
		}

		free(currentDataSet->elementList);

		delete currentDataSet;
	}
}

bool DataOutputManager::checkSetParameters()
{
	if (!checkBasicElements())return false;
	if (!std::filesystem::exists(commandArguments->outputDirectory)) return false;
	return true;
}