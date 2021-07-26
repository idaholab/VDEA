#pragma once
#include "CodexBase.h"


class DataExchangeCodex : public CodexBase
{
private:
	bool isJson(char *, unsigned int);
	void readJson(PrimaryDataStorage *, MessageHandlingSystem *, char * data, unsigned int fileSize,char * filename);
	void readXML(PrimaryDataStorage *, currentStorageElementGrouping *,MessageHandlingSystem *, char * data, unsigned int fileSize, char * filename);
	
	void assignValueToTag(PrimaryDataStorageElement *, char * tag, char * value);
	void assignToDataDescription(DataDescription *, char *, char *,bool json);

	char * extractXMLTag(char * data, int i, unsigned int length);
	char * extractXMLValue(char * data, int * i, unsigned int length,char * tag);
	void assignXMLValue(char * currentTag, char * value,char * filename,currentStorageElementGrouping *,PrimaryDataStorage *,MessageHandlingSystem *);

	void parseJsonMetaDataValue(char *,unsigned int, int *, PrimaryDataStorageElement *);
	void parseJsonDataDescriptionValue(char *,unsigned int, int *, PrimaryDataStorageElement *);
	char * getStringJson(char * data, int, int *);
	char * getValueJson(char * data, int, int *);

	void writeDataSection(DataDescription * data, char * filename, MessageHandlingSystem *);
public:
	DataExchangeCodex();

	void convertDataToDefFile(PrimaryDataStorageElement * data);

	void processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filePath);
	bool outputFile(PrimaryDataStorageElement *, MessageHandlingSystem *,char * directory);
};

const char * getStringRep(DataType);
const char * getStringRep(Endianess);
