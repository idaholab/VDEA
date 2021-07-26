#pragma once

#define EXPORT __declspec(dllexport)

#include "MessageHandlingSystem.h"
#include "CodexBase.h"

enum EXPORT  DataType {DEF_INT, DEF_REAL, DEF_CHAR, DEF_UINT,DEF_UNRECOGNIZED};
enum EXPORT  Endianess {LITTLE,BIG,EUNRECOGNIZED};

EXPORT Endianess recognizeEndianess(char *);
EXPORT DataType recognizeDataType(char *);
EXPORT bool recognizeYesNo(char *);

class CodexBase;

struct EXPORT DataDescription
{
	~DataDescription();
	DataType Type;
	unsigned int BytesPerElem;
	Endianess DataEndianess;
	unsigned long long NumberOfElements;
	char * DataFilePath;
	char * DataFileParentDirectory;
	bool Interleaved;
	unsigned long long ChunkOffset;
	unsigned long long ChunkSize;
};

struct EXPORT KeyValueProperty
{
	char * Key;
	char * Value;
};

class EXPORT PrimaryDataStorageElement
{
	//This is a data storage element that will store all of the necessary information required 
	//to reread the data and access what is needed
private:
	char * makeDefaultID(char *, int);
	CodexBase * readCodex;

public:
	PrimaryDataStorageElement(int systemId, CodexBase *);
	~PrimaryDataStorageElement();
	unsigned int SystemID;
	DataDescription * dataDescription;
	char * ID;

	unsigned int numberofAdditionalMetadataElements;
	KeyValueProperty * additionalMetaDataElements;

	void addMetaDataElement(KeyValueProperty * newElement);

	bool checkDataIntegrity(MessageHandlingSystem *, char *,int);

	void fabricateDEFData();
	bool temporaryDataFile;

	void setCodex(CodexBase *);
	CodexBase * getCodex();

};