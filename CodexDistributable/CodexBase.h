#pragma once

#define EXPORT __declspec(dllexport)




#include "MessageHandlingSystem.h"
#include "PrimaryDataStorage.h"
#include <Filesystem>



class PrimaryDataStorageElement;
class PrimaryDataStorage;


EXPORT const char * getStringRep(bool);
EXPORT char * getStringRepDel (bool);
EXPORT char * getStringRep(unsigned int);
EXPORT char * getStringRep(unsigned long long);
EXPORT char * getStringRep(int);
EXPORT char * getStringRep(long long);
EXPORT char * getStringRep(double);
EXPORT char * getStringRep(long double);
EXPORT char * getStringRep();

EXPORT char * removeSpecialCharacters(char * charactersIn);


EXPORT bool checkExtension(char * filename, const char * extension);

 class EXPORT CodexBase
{
protected:
	const char * identifier;

public:
	const char * getIdentifier();

	virtual void processFile(PrimaryDataStorage * dataStorage
		, MessageHandlingSystem * messager
		, char * filePath) = 0;
	virtual bool outputFile(PrimaryDataStorageElement *
		, MessageHandlingSystem *
		, char * directory ) = 0;
	virtual void convertDataToDefFile(PrimaryDataStorageElement *) = 0;
	virtual bool deleteAdditionalElements()
	{
		return false;
	}
};
