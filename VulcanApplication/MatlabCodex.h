#pragma once
#include "CodexBase.h"

#define MATLABHeaderDescriptiveTextLength 116

enum MatlabDataType
{
	miINT8 = 1,
	miUINT8 = 2,
	miINT16 = 3,
	miDOUBLE = 9,
	miINT32 = 5,
	miUINT32 = 6,
	miINT64 = 12,
	miMATRIX = 14,
	miUTF8 = 16
};

enum MatlabArrayType
{
	mxCHAR_CLASS = 4,
	mxDOUBLE_CLASS = 6,
	mxINT8_CLASS = 8,
	mxINT16_CLASS = 10,
	mxINT32_CLASS = 12,
	mxINT64_CLASS = 14
};

class MatlabCodex : public CodexBase
{
private:
	FILE * createMatlabFile(char *, char *);
	void writeDataMatlabArray(FILE *, char *, unsigned int, MatlabArrayType, MatlabDataType, unsigned int, void *);
	void writeArrayFlags(FILE *, MatlabArrayType);
	void writeArrayDimensions(FILE *, MatlabDataType, unsigned int);
	void writeMatlabArrayName(FILE *, char *, unsigned int);
	void writeDataElement(FILE *, MatlabDataType, unsigned int, void *);


	//setting up the matlab array returns the location of the number of elements that needs to be
	//written
	void setUpMatlabArray(FILE *, char *, MatlabArrayType, MatlabDataType);
	void openArrayToAppend(FILE *, MatlabDataType);
	void appendDataToArray(FILE *, unsigned int, void *);
	void closeArray(FILE *, char *, MatlabArrayType, MatlabDataType, long, unsigned int);

	char * getFilename(char *);
	char * getDescription();
	char* getFilenameBigFile(char *, int);

	unsigned int appendDefDataToMatlabArray(FILE * toProcess, DataDescription * dataDescriptor);

	unsigned int getBytesPerDataType(MatlabDataType);

	void processProperty(FILE * currentFileToProcess, KeyValueProperty * currentProperty);
	MatlabArrayType getMatlabArrayType(DataDescription *);
	MatlabDataType getMatlabDataType(DataDescription *);

	void readMatlabMatrixes(FILE * matlabFile,char * filename,PrimaryDataStorage * dataStorage, currentStorageElementGrouping * cseg,MessageHandlingSystem *,Endianess);
	MatlabDataType readCurrentElementType(FILE *);
	unsigned int readCurrentStepSize(FILE *);
	void readDataIdentifiers(FILE *, DataDescription *);
	char * readMatlabArrayName(FILE *);

public:

	MatlabCodex();
	void convertDataToDefFile(PrimaryDataStorageElement * data);
	void processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filename);
	bool outputFile(PrimaryDataStorageElement *, MessageHandlingSystem *, char * directory);
};
