#pragma once

#include "CodexBase.h"
#include "TDMSDataValues.h"
#include "TDMSDataStorageElement.h"

enum objectPathCondition
{
	objectPathNotChannel,
	objectPathAlreadyRead,
	newObjectPath,
	matchesCurrentObjectPath,
	skip
};

enum rawDataIndexType {
	_noData,
	_identicalToLastSegment,
	_normalRawDataIndex,
	_daqmxRawDataIndex
};

struct tdmsProperty
{
	tdmsLib::tdmsString propertyName;
	tdmsLib::tdmsDataValue * value;
	~tdmsProperty();
};

struct tdmsRawDataIndex
{
	virtual rawDataIndexType getRawDataIndexType() = 0;
};

struct noData : tdmsRawDataIndex
{
	rawDataIndexType getRawDataIndexType();
};

struct identicalToLastSegment : tdmsRawDataIndex
{
	rawDataIndexType getRawDataIndexType();
};

struct normalRawDataIndex : tdmsRawDataIndex
{
	rawDataIndexType getRawDataIndexType();
	uint32_t lengthOfData;
	tdmsLib::tdmsDataType dataType;
	int32_t arrayDimension;
	uint32_t numberOfValues;
	uint32_t totalSize;
};

struct formatChangingVector
{
	uint32_t vectorSize;
	uint32_t daqmxDataType;//? tdmsDataType
	uint32_t rawByteOffset;
	uint32_t sampleFormatBitmap;
	uint32_t scaleID;
};

struct rawDataWidthVector
{
	uint32_t vectorSize;
	uint32_t elementsInVector;
};

struct daqmxRawDataIndexObject : tdmsRawDataIndex
{
	rawDataIndexType getRawDataIndexType();
	int32_t dataType;
	int32_t arrayDimension;
	uint32_t numberOfValues;
	formatChangingVector FormatChangingVector;
	rawDataWidthVector RawDataWidthVector;
};

struct tdmsMetaDataElement
{
	tdmsLib::tdmsString objectPath;
	//uint32_t rawDataIndex; //This is an especially problematic element 
	//as it is responsible for so many different changes in behavior
	tdmsRawDataIndex * rawDataIndex;
	uint32_t numberOfProperties;
	tdmsProperty * properties;
	~tdmsMetaDataElement();
};

struct tdmsMetaData
{
	uint32_t numberOfObjects;
	tdmsMetaDataElement * metaDataElements;
	void constructMetaDataElements();
	~tdmsMetaData();
};

struct leadIn
{
	char identifierTag[5];
	uint32_t TableOfContentsBitMask;
	uint32_t VersionNumber;
	uint64_t SegmentLengthPastLeadIn;
	uint64_t SegmentMetaDataLength;
	static const uint32_t leadInLength = 32;
};

struct tdmsSegment
{
	leadIn LeadIn;
	tdmsMetaData metaData;
	//actual data? 
	//lets not worry about that right now
	~tdmsSegment();
};

struct tdmsMainStructure
{
	char * filePath;
	int numberOfSegmentsInFile;
	tdmsSegment ** segmentsInFile;
	~tdmsMainStructure();
};

class TDMSCodex : public CodexBase
{
private:
	//this is where all of the pain in the neck elements from the other system go
	//lets get to code adapting and refactoring

	tdmsMainStructure * readFile(char *);
	void addAnotherSegment(tdmsMainStructure *, FILE *);

	tdmsSegment * parseTDMSSegment(FILE*);
	tdmsRawDataIndex * parseNormalRawDataIndex(FILE *, uint32_t);
	tdmsRawDataIndex * parseDAQmxDataIndex(FILE *);
	formatChangingVector parseFormatChangingVector(FILE *);
	rawDataWidthVector parseRawDataWidthVector(FILE *);
	void parseProperty(FILE *, tdmsProperty *);
	void * readingError(int, size_t, FILE *);
	void * openingError(char *, int);
	
	objectPathCondition checkObjectPath(std::list<char *> * alreadyReadPaths, char ** currentPath, tdmsMetaDataElement * element);
	objectPathCondition checkObjectPathWithQual(std::list<char *> * alreadyReadPaths, char ** currentPath,char * currentAllowedPath, tdmsMetaDataElement * element);
	void processProperty(TDMSDataStorageElement *, tdmsProperty * currentProperty);

	void populateDataDescriptor(DataDescription * value, tdmsRawDataIndex * input);
	void getDataType(DataDescription * value, tdmsRawDataIndex * index);
	unsigned int appendTDMSDataToDefFile (FILE *, FILE *, long currentDataLocation, int channelIndex, tdmsSegment * currentSegment);
	long calculateChannelSize(tdmsRawDataIndex * currentChannel);
	long calculateNumberOfChunks(leadIn leadin, long chunkSize);
	int getBytesPerTdmsDataType(tdmsLib::tdmsDataType type);
	
	void appendDataToArray(FILE * defFile, unsigned int length, void * data);

	void writeDataSection(DataDescription * currentDescription, FILE * currentFile, MessageHandlingSystem * messagingSystem);

	unsigned int getDataType(DataDescription * description);

	uint64_t calculateSegmentLength(PrimaryDataStorageElement * pdse,MessageHandlingSystem * messageHandlingSystem);
	uint64_t calculateMetadataLength(PrimaryDataStorageElement * pdse,MessageHandlingSystem * messageHandlingSystem);

public:
	TDMSCodex();
	void convertDataToDefFile(PrimaryDataStorageElement * data);

	void processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filename);
	bool outputFile(PrimaryDataStorageElement *, MessageHandlingSystem *, char * directory);
};

char * convertTDMSTimestampToString(tdmsLib::tdmsTimeStamp * currentTimeStamp);

int extractMonthFromDayInYear(int currentDay, bool isLeapYear);

int extractDaysSinceStartOfMonth(int currentMonth, bool isLeapYear);
