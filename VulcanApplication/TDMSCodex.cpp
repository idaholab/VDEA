#include "TDMSCodex.h"
#include "TDMSDataStorageElement.h"
#include "TDMSDataValues.h"

#define DEBUG


//PL standing for print line
#ifdef DEBUG
#define PL  std::cerr << "error occured "<< __FILE__ <<":" << __LINE__ <<std::endl; 

#endif

TDMSCodex::TDMSCodex()
{
	identifier = "TDMS";
}

void TDMSCodex::processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filename)
{
	//strstr is looking for the file extension and returns a nullpointer if it does not exist
	if (strstr(filename, ".tdms") == nullptr
		&& strstr(filename, ".TDMS") == nullptr
		) return;
	if (strstr(filename, ".tdms_index") != nullptr)return;
	if (strstr(filename, ".TDMS_INDEX") != nullptr)return;

	tdmsMainStructure * mainStructure = readFile(filename);
	
	std::list<char *>  usedTdmsChannels;
	char * currentObjectPath = NULL;

	int CurrentChannelLength;
	int currentChannelIndex = 0;
	long startOfMostRecentArray = 0;
	uint32_t currentNumberOfValues = 0;
	tdmsRawDataIndex * mostRecentDataIndex = NULL;

	currentStorageElementGrouping * cseg = (currentStorageElementGrouping *)malloc(sizeof(currentStorageElementGrouping));// new currentStorageElementGrouping();
	cseg->elementList = nullptr;
	cseg->elementListCount = 0;
	TDMSDataStorageElement * currentStorageElement = nullptr;

	do
	{
		bool foundNewElementInLastBatch = false;
		long currentDataLocation = 0;
		CurrentChannelLength = usedTdmsChannels.size();
		for (int i = 0; i < mainStructure->numberOfSegmentsInFile; i++)
		{
			tdmsSegment * segment = mainStructure->segmentsInFile[i];
			leadIn currentLeadin = segment->LeadIn;
			currentDataLocation += (currentLeadin.leadInLength + currentLeadin.SegmentMetaDataLength);

			if ((currentLeadin.TableOfContentsBitMask & (1L << 3)) > 0) //This will need to be appropriately modularized <<3 contains raw data
			{
				if ((currentLeadin.TableOfContentsBitMask & (1L << 5)) > 0)
				{
					std::cerr << std::endl << "ERROR: Library currently does not handle interleaved data" << std::endl;
					continue;
				}

				for (uint32_t j = 0; j < segment->metaData.numberOfObjects; j++)
				{
					tdmsMetaDataElement * currentElement = &(segment->metaData.metaDataElements[j]);
					objectPathCondition opc = checkObjectPath(&usedTdmsChannels, &currentObjectPath, currentElement);
					switch (opc)
					{
						case newObjectPath:
						{
							foundNewElementInLastBatch = true;
							mostRecentDataIndex = currentElement->rawDataIndex;
							currentNumberOfValues = 0;
							currentStorageElement = new TDMSDataStorageElement(dataStorage->getNextSystemID(),(CodexBase *)this);
							char * namecopy = (char *)malloc(strlen(currentObjectPath) + 1);
							strcpy_s(namecopy, strlen(currentObjectPath) + 1, currentObjectPath);
							currentStorageElement->ID = namecopy;
						}//this is supposed to not have a break;
						case matchesCurrentObjectPath:
						{
							//append data in all of the chuncks data to .DEF additional property
							currentStorageElement->addSegment(segment);
						}break;
					}
				}
			}
			currentDataLocation += (currentLeadin.SegmentLengthPastLeadIn - currentLeadin.SegmentMetaDataLength);
		}

		if (foundNewElementInLastBatch)
		{
			currentStorageElement->mainStructure = mainStructure;
			cseg->addToCurrentStorageElementGrouping(currentStorageElement);
			currentStorageElement = NULL;
			usedTdmsChannels.push_front(currentObjectPath);
			currentObjectPath = NULL;
		}
	} while (usedTdmsChannels.size() != CurrentChannelLength);

	dataStorage->addMoreData(cseg);
	delete cseg;
	//check the other data translational system
	//step through and manipulate every single one of the structures into each of the 
	//proper tdmsStorageElements
	//add in the required information for the system to identify which is which, possibly 
	//just use the magical magic of how the other code base actually works for this stuff
}

rawDataIndexType noData::getRawDataIndexType()
{
	return rawDataIndexType::_noData;
}

rawDataIndexType identicalToLastSegment::getRawDataIndexType()
{
	return rawDataIndexType::_identicalToLastSegment;
}

rawDataIndexType normalRawDataIndex::getRawDataIndexType()
{
	return rawDataIndexType::_normalRawDataIndex;
}

rawDataIndexType daqmxRawDataIndexObject::getRawDataIndexType()
{
	return rawDataIndexType::_daqmxRawDataIndex;
}

tdmsMetaDataElement::~tdmsMetaDataElement()
{
	for (uint32_t i = 0; i < numberOfProperties; i++)
	{
		properties[i].~tdmsProperty();
	}
	free(properties);
	delete rawDataIndex;
}

tdmsProperty::~tdmsProperty()
{
	delete value;
}

void tdmsMetaData::constructMetaDataElements()
{
	metaDataElements = (tdmsMetaDataElement*)malloc(sizeof(tdmsMetaDataElement)* numberOfObjects);
}

tdmsMetaData::~tdmsMetaData()
{
	for (uint32_t i = 0; i < numberOfObjects; i++)
	{
		metaDataElements[i].~tdmsMetaDataElement();
	}
	free(metaDataElements);
}

tdmsSegment::~tdmsSegment()
{
}

tdmsMainStructure::~tdmsMainStructure()
{
	for (int i = 0; i < numberOfSegmentsInFile; i++)
	{
		delete segmentsInFile[i];
	}
	free(segmentsInFile);
}

tdmsMainStructure * TDMSCodex::readFile(char * filePath)
{
	FILE * tdmsFile = NULL;
	int errocode = fopen_s(&tdmsFile, filePath, "rb");
	if (errocode != 0 || tdmsFile == 0) return (tdmsMainStructure *)openingError(filePath, errocode);

	fseek(tdmsFile, 0, SEEK_END);
	long fileLength = ftell(tdmsFile);
	rewind(tdmsFile);

	if (fileLength <= 0)
	{
		std::cerr << "File: " << filePath << " is empty" << std::endl;
	}

	unsigned int tdmsStructureSize = sizeof(tdmsMainStructure);
	tdmsMainStructure * main = (tdmsMainStructure *)malloc(tdmsStructureSize);
	main->segmentsInFile = nullptr;
	main->numberOfSegmentsInFile = 0;
	main->filePath = filePath;
	while (fileLength > ftell(tdmsFile))
	{
		addAnotherSegment(main, tdmsFile);
		main->segmentsInFile[main->numberOfSegmentsInFile - 1] = parseTDMSSegment(tdmsFile);
		//std::cout << "\tFile "<< fileno<<" Traversed: (" << (ftell(tdmsFile) / (1.0*fileLength))*100.0 << "%)" << std::endl;
	}
	fclose(tdmsFile);
	return main;
}

void TDMSCodex::convertDataToDefFile(PrimaryDataStorageElement * data)
{
	//this code has been copied over from another of my projects,
	//it has been modified to work with the def file format.
	
	TDMSDataStorageElement * actualElement = (TDMSDataStorageElement *) data;

	for (int i = 0; i < actualElement->numberOfSegments; i++)
	{
		tdmsSegment * segment = actualElement->segmentsComposingChannel[i];
		tdmsMetaData * metaData = &(segment->metaData);
		for (int j = 0; j < metaData->numberOfObjects; j++)
		{
			for (int k = 0; k < metaData->metaDataElements[j].numberOfProperties; k++)
			{
				tdmsProperty * currentProperty = &(metaData->metaDataElements[j].properties[k]);
				processProperty(actualElement, currentProperty);
			}
		}
	}

	DataDescription * tdmsDataDescriptor = (DataDescription *) malloc(sizeof(DataDescription));
	tdmsDataDescriptor->DataFilePath = (char *) malloc(L_tmpnam_s*sizeof(char));
	tmpnam_s(tdmsDataDescriptor->DataFilePath, L_tmpnam_s);
	tdmsDataDescriptor->ChunkOffset = 0;
	tdmsDataDescriptor->ChunkSize = 0;
	tdmsDataDescriptor->Interleaved = false;
	tdmsDataDescriptor->DataFileParentDirectory = nullptr;
	actualElement->temporaryDataFile = true;

	//This will be identical in functionality to writing to a matlab file with the exception being
	//this will be using the ID of the piece as a matching element

	std::list<char *>  usedTdmsChannels;
	char * currentObjectPath = NULL;

	int CurrentChannelLength;
	int currentChannelIndex = 0;
	long startOfMostRecentArray = 0;
	uint32_t currentNumberOfValues = 0;
	tdmsRawDataIndex * mostRecentDataIndex = NULL;

	FILE * defTempFile = NULL;
	FILE * tdmsFile = NULL;

	fopen_s(&defTempFile, tdmsDataDescriptor->DataFilePath, "w");
	fopen_s(&tdmsFile, actualElement->mainStructure->filePath, "rb");

	do
	{
		bool foundNewElementInLastBatch = false;

		long currentDataLocation = 0;
		CurrentChannelLength = usedTdmsChannels.size();
		for (int i = 0; i < actualElement->numberOfSegments; i++)
		{
			tdmsSegment * segment = actualElement->segmentsComposingChannel[i];
			leadIn currentLeadin = segment->LeadIn;
			currentDataLocation += (currentLeadin.leadInLength + currentLeadin.SegmentMetaDataLength);

			if ((currentLeadin.TableOfContentsBitMask & (1L << 3)) > 0) //This will need to be appropriately modularized <<3 contains raw data
			{
				if ((currentLeadin.TableOfContentsBitMask & (1L << 5)) > 0)
				{
					std::cerr << std::endl << "ERROR: Library currently does not handle interleaved data" << std::endl;
					continue;
				}
				/*if((currentLeadin.TableOfContentsBitMask & (1L << 2)) > 0)
				{
					std::cerr << std::endl << "ERROR: Library currently does not handle new object list operations" << std::endl;
					continue;
				}*/

				for (uint32_t j = 0; j < segment->metaData.numberOfObjects; j++)
				{
					tdmsMetaDataElement * currentElement = &(segment->metaData.metaDataElements[j]);
					objectPathCondition opc = checkObjectPathWithQual(&usedTdmsChannels, &currentObjectPath,actualElement->ID, currentElement);
					
					switch (opc)
					{
						case newObjectPath:
						{		
							foundNewElementInLastBatch = true;
							mostRecentDataIndex = (currentElement)->rawDataIndex;
							startOfMostRecentArray = ftell(defTempFile);
							currentNumberOfValues = 0;
							populateDataDescriptor(tdmsDataDescriptor,mostRecentDataIndex);
						}//this is supposed to not have a break;
						case matchesCurrentObjectPath:
						{
							//append data in all of the chuncks data to matlab array
							currentNumberOfValues += appendTDMSDataToDefFile(defTempFile, tdmsFile, currentDataLocation, j, segment);
						}break;
					}
				}
			}
			currentDataLocation += (currentLeadin.SegmentLengthPastLeadIn - currentLeadin.SegmentMetaDataLength);
		}

		if (foundNewElementInLastBatch)
		{
			startOfMostRecentArray = 0;
			usedTdmsChannels.push_front(currentObjectPath);
			currentObjectPath = NULL;
		}

	} while (usedTdmsChannels.size() != CurrentChannelLength);

	actualElement->dataDescription = tdmsDataDescriptor;
	fclose(tdmsFile);
	fclose(defTempFile);

}

bool TDMSCodex::outputFile(PrimaryDataStorageElement * pdse, MessageHandlingSystem * messageHandlingSystem, char * directory)
{
	pdse->fabricateDEFData();

	FILE * currentFileToProcess;
	char * fileNameCleaned = removeSpecialCharacters(pdse->ID);
	char * filenameBuffer = messageHandlingSystem->concatenateStrings(4, directory, "\\", fileNameCleaned, ".tdms");
	fopen_s(&currentFileToProcess, filenameBuffer, "w");
	free(filenameBuffer);
	free(fileNameCleaned);

	unsigned char header [] = { 0x54,0x44, 0x53, 0x6d };

	fwrite(header, sizeof(unsigned char), 4, currentFileToProcess);

	//the documentation for the tdms file format has the listing of these magic values in their stored location
	unsigned int tocMask = (1L<<1 |1L<<2 |1L<<3 );
	fwrite(&tocMask, sizeof(unsigned int), 1, currentFileToProcess);

	int versionNumber = 4713;
	fwrite(&versionNumber, sizeof(int), 1, currentFileToProcess);

	uint64_t segmentLengthPastLeadIn = calculateSegmentLength(pdse,messageHandlingSystem);
	fwrite(&segmentLengthPastLeadIn, sizeof(segmentLengthPastLeadIn), 1, currentFileToProcess);

	uint64_t segmentRawDataOffsetValue = calculateMetadataLength(pdse,messageHandlingSystem);
	fwrite(&segmentRawDataOffsetValue, sizeof(segmentRawDataOffsetValue), 1, currentFileToProcess);

	unsigned int numberOfObjects = 2;
	fwrite(&numberOfObjects, sizeof(unsigned int), 1, currentFileToProcess);

	const char * hdr = "/'Data'";

	unsigned int dataSize = strlen(hdr);

	fwrite(&dataSize, sizeof(unsigned int), 1, currentFileToProcess);

	fwrite(hdr, sizeof(char), strlen(hdr), currentFileToProcess);

	unsigned char rawDataIndex[] = { 0xFF,0xFF,0xFF,0xFF };

	fwrite(rawDataIndex, sizeof(unsigned char), 4, currentFileToProcess);

	unsigned int numberOfGroupProperties = 0;
	fwrite(&numberOfGroupProperties, sizeof(unsigned int), 1, currentFileToProcess);
	
	hdr = "/'Data'/'";

	//this may have a problem because it may put elements too far down the hierarchy of stuff,
	//however this is unsubstansiated and should be checked during testing
	char * location = messageHandlingSystem->concatenateStrings(3, hdr, pdse->ID, "'");

	dataSize = strlen(location);

	fwrite(&dataSize, sizeof(unsigned int), 1, currentFileToProcess);

	fwrite(location, sizeof(char), strlen(location), currentFileToProcess);

	unsigned int lengthOfIndexInformation = 14;
	fwrite(&lengthOfIndexInformation, sizeof(unsigned int), 1, currentFileToProcess);

	unsigned int tdmsDataType = getDataType(pdse->dataDescription);

	fwrite(&tdmsDataType, sizeof(unsigned int), 1, currentFileToProcess);
	//create the channels

	unsigned int dimensionOfRawData = 1;
	fwrite(&dimensionOfRawData, sizeof(unsigned int), 1, currentFileToProcess);

	unsigned long long numberOfRawDataValues = pdse->dataDescription->NumberOfElements;
	fwrite(&numberOfRawDataValues, sizeof(unsigned long long), 1, currentFileToProcess);

	//take a def file and turn it into a tdms file

	unsigned int numberOfAdditionalProperties = pdse->numberofAdditionalMetadataElements;
	fwrite(&numberOfAdditionalProperties, sizeof(unsigned int), 1, currentFileToProcess);

	for (int i = 0; i < pdse->numberofAdditionalMetadataElements; i++)
	{
		unsigned int strPropertyLength = strlen(pdse->additionalMetaDataElements[i].Key);
		fwrite(&strPropertyLength, sizeof(unsigned int), 1, currentFileToProcess);

		fwrite(pdse->additionalMetaDataElements->Key, 1, strPropertyLength, currentFileToProcess);
		
		unsigned int stringDataType =(unsigned int) tdmsLib::tdmsTypeString;
		fwrite(&stringDataType, sizeof(unsigned int), 1, currentFileToProcess);

		strPropertyLength = strlen(pdse->additionalMetaDataElements[i].Value);
		fwrite(&strPropertyLength, sizeof(unsigned int), 1, currentFileToProcess);

		fwrite(pdse->additionalMetaDataElements[i].Value, 1, strPropertyLength, currentFileToProcess);
	}

	writeDataSection(pdse->dataDescription, currentFileToProcess, messageHandlingSystem);

	fclose(currentFileToProcess);

	return true;
}

unsigned int TDMSCodex::getDataType(DataDescription * description)
{
	switch (description->Type)
	{
		case DataType::DEF_CHAR :
		{
			switch (description->BytesPerElem)
			{
				case 1: return tdmsLib::tdmsDataType::tdmsTypeU8;
				case 2: return tdmsLib::tdmsDataType::tdmsTypeU16;
				case 4: return tdmsLib::tdmsDataType::tdmsTypeU32;
				case 8: return tdmsLib::tdmsDataType::tdmsTypeU64;
			}
		}
		case DataType::DEF_INT :
		{
			switch (description->BytesPerElem)
			{
				case 1: return tdmsLib::tdmsDataType::tdmsTypeI8;
				case 2: return tdmsLib::tdmsDataType::tdmsTypeI16;
				case 4: return tdmsLib::tdmsDataType::tdmsTypeI32;
				case 8: return tdmsLib::tdmsDataType::tdmsTypeI64;
			}
		}
		case DataType::DEF_UINT:
		{
			switch (description->BytesPerElem)
			{
				case 1: return tdmsLib::tdmsDataType::tdmsTypeU8;
				case 2: return tdmsLib::tdmsDataType::tdmsTypeU16;
				case 4: return tdmsLib::tdmsDataType::tdmsTypeU32;
				case 8: return tdmsLib::tdmsDataType::tdmsTypeU64;
			}
		}
		case DataType::DEF_REAL:
		{
			switch (description->BytesPerElem)
			{
				case 4: return tdmsLib::tdmsDataType::tdmsTypeSingleFloat;
				case 8: return tdmsLib::tdmsDataType::tdmsTypeDoubleFloat;
				case 10: return tdmsLib::tdmsDataType::tdmsTypeExtendedFloat;
			}
		}
	}
	return 0;
}

void TDMSCodex::writeDataSection(DataDescription * currentDescription, FILE * currentFileToProcess, MessageHandlingSystem * messager)
{
	FILE * oldDataFile;
	if (currentDescription->DataFileParentDirectory == nullptr)
	{
		fopen_s(&oldDataFile, currentDescription->DataFilePath, "rb");
	}
	else
	{
		char * actualFilePath = MessageHandlingSystem::concatenateStrings(3, currentDescription->DataFileParentDirectory, "\\", currentDescription->DataFilePath);
		fopen_s(&oldDataFile, actualFilePath, "rb");
		free(actualFilePath);
	}

	void * bufferElementRead = malloc(currentDescription->NumberOfElements*currentDescription->BytesPerElem);

	_fseeki64(oldDataFile, 0, SEEK_END);
	unsigned long long fileLength = _ftelli64(oldDataFile);
	_fseeki64(oldDataFile, 0, SEEK_SET);

	for (unsigned long long i = 0; i < fileLength; i += currentDescription->ChunkSize)
	{
		_fseeki64(oldDataFile, i + currentDescription->ChunkOffset, SEEK_SET);
		long long dataRead = fread(bufferElementRead, currentDescription->BytesPerElem, currentDescription->NumberOfElements, oldDataFile);
		fwrite(bufferElementRead, currentDescription->BytesPerElem, dataRead, currentFileToProcess);
	}
	free(bufferElementRead);
	fclose(oldDataFile);
}

void TDMSCodex::addAnotherSegment(tdmsMainStructure * main, FILE * file)
{
	main->numberOfSegmentsInFile++;
	tdmsSegment ** holding = main->segmentsInFile;
	main->segmentsInFile = (tdmsSegment **)malloc(sizeof(tdmsSegment*)*main->numberOfSegmentsInFile);
	for (int i = 0; i < main->numberOfSegmentsInFile - 1; i++)
	{
		main->segmentsInFile[i] = holding[i];
	}
	free(holding);
}

void TDMSCodex::populateDataDescriptor(DataDescription * inputValue, tdmsRawDataIndex * index)
{
	getDataType(inputValue, index);
	inputValue->ChunkSize = calculateChannelSize(index);
	inputValue->NumberOfElements = inputValue->ChunkSize / inputValue->BytesPerElem;

}

tdmsSegment * TDMSCodex::parseTDMSSegment(FILE * tdmsFile)
{
	tdmsSegment * mainSegment = new tdmsSegment();

	mainSegment->LeadIn.identifierTag[4] = 0;
	size_t charsRead = fread(&(mainSegment->LeadIn.identifierTag), sizeof(char), 4, tdmsFile);
	if (charsRead != 4) return (tdmsSegment *)readingError(4, charsRead, tdmsFile);

	charsRead = fread(&(mainSegment->LeadIn.TableOfContentsBitMask), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(mainSegment->LeadIn.VersionNumber), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(mainSegment->LeadIn.SegmentLengthPastLeadIn), sizeof(uint64_t), 1, tdmsFile);
	if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(mainSegment->LeadIn.SegmentMetaDataLength), sizeof(uint64_t), 1, tdmsFile);
	if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(mainSegment->metaData.numberOfObjects), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

	mainSegment->metaData.constructMetaDataElements();

	for (uint32_t i = 0; i < mainSegment->metaData.numberOfObjects; i++)
	{
		//begin creating the tdms meta data objects
		charsRead = fread(&(mainSegment->metaData.metaDataElements[i].objectPath.strLen), sizeof(uint32_t), 1, tdmsFile);
		if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

		mainSegment->metaData.metaDataElements[i].objectPath.characters = (char *)calloc(sizeof(char)* mainSegment->metaData.metaDataElements[i].objectPath.strLen + 1, sizeof(char));

		uint32_t objectPathStringLength = mainSegment->metaData.metaDataElements[i].objectPath.strLen;
		charsRead = fread((mainSegment->metaData.metaDataElements[i].objectPath.characters), sizeof(char), objectPathStringLength, tdmsFile);
		if (charsRead != objectPathStringLength) return (tdmsSegment *)readingError(objectPathStringLength, charsRead, tdmsFile);

		uint32_t rawDataIndexRaw = 0;
		charsRead = fread(&(rawDataIndexRaw), sizeof(uint32_t), 1, tdmsFile);
		if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

		switch (rawDataIndexRaw)
		{
			case 0xffffffff:
			{
				mainSegment->metaData.metaDataElements[i].rawDataIndex = new noData();
				//This is the no raw data in this segment
			}break;
			case 0x00000000:
			{
				mainSegment->metaData.metaDataElements[i].rawDataIndex = new identicalToLastSegment();

				//This raw data is identical to earlier segment
			}break;
			case 0x69120000:
			{
				mainSegment->metaData.metaDataElements[i].rawDataIndex = parseDAQmxDataIndex(tdmsFile);
				//DAQmx raw data index object
			}break;
			default:
			{
				mainSegment->metaData.metaDataElements[i].rawDataIndex = parseNormalRawDataIndex(tdmsFile, rawDataIndexRaw);
				//Normal Raw data index object
			}break;
		}

		charsRead = fread(&(mainSegment->metaData.metaDataElements[i].numberOfProperties), sizeof(uint32_t), 1, tdmsFile);
		if (charsRead != 1) return (tdmsSegment *)readingError(1, charsRead, tdmsFile);

		mainSegment->metaData.metaDataElements[i].properties = (tdmsProperty*)malloc(sizeof(tdmsProperty)* mainSegment->metaData.metaDataElements[i].numberOfProperties);

		for (uint32_t prop = 0; prop < mainSegment->metaData.metaDataElements[i].numberOfProperties; prop++)
		{
			parseProperty(tdmsFile, &(mainSegment->metaData.metaDataElements[i].properties[prop]));
		}
	}

	//shift into reading the raw data proper

	uint64_t seekDistance = mainSegment->LeadIn.SegmentLengthPastLeadIn - mainSegment->LeadIn.SegmentMetaDataLength;
	fseek(tdmsFile, seekDistance, SEEK_CUR);
	return mainSegment;
}

tdmsRawDataIndex * TDMSCodex::parseNormalRawDataIndex(FILE * tdmsFile, uint32_t rawDataIndexLength)
{
	normalRawDataIndex * nrdi = new normalRawDataIndex();
	nrdi->lengthOfData = rawDataIndexLength;

	size_t charsRead = fread(&(nrdi->dataType), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(nrdi->arrayDimension), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(nrdi->numberOfValues), sizeof(uint64_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	if (nrdi->dataType == tdmsLib::tdmsTypeString)
	{
		charsRead = fread(&(nrdi->totalSize), sizeof(uint64_t), 1, tdmsFile);
		if (charsRead != 1) readingError(1, charsRead, tdmsFile);
	}

	return nrdi;
}

tdmsRawDataIndex * TDMSCodex::parseDAQmxDataIndex(FILE * tdmsFile)
{
	daqmxRawDataIndexObject * drdi = new daqmxRawDataIndexObject();

	size_t charsRead = fread(&(drdi->dataType), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(drdi->arrayDimension), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(drdi->numberOfValues), sizeof(uint64_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	drdi->FormatChangingVector = parseFormatChangingVector(tdmsFile);

	drdi->RawDataWidthVector = parseRawDataWidthVector(tdmsFile);

	return drdi;
}

formatChangingVector TDMSCodex::parseFormatChangingVector(FILE * tdmsFile)
{
	formatChangingVector retVal;

	size_t charsRead = fread(&(retVal.vectorSize), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(retVal.daqmxDataType), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(retVal.rawByteOffset), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(retVal.sampleFormatBitmap), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(retVal.scaleID), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	return retVal;
}

rawDataWidthVector TDMSCodex::parseRawDataWidthVector(FILE * tdmsFile)
{
	rawDataWidthVector retVal;

	size_t charsRead = fread(&(retVal.vectorSize), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	charsRead = fread(&(retVal.elementsInVector), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	return retVal;
}

void TDMSCodex::parseProperty(FILE * tdmsFile, tdmsProperty * storageLocation)
{
	size_t charsRead = fread(&((*storageLocation).propertyName.strLen), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	(*storageLocation).propertyName.characters = (char *)calloc(sizeof(char)* (*storageLocation).propertyName.strLen + 1, sizeof(char));

	uint32_t objectPathStringLength = (*storageLocation).propertyName.strLen;
	charsRead = fread(((*storageLocation).propertyName.characters), sizeof(char), objectPathStringLength, tdmsFile);
	if (charsRead != objectPathStringLength) readingError(objectPathStringLength, charsRead, tdmsFile);

	tdmsLib::tdmsDataType type;

	charsRead = fread(&(type), sizeof(uint32_t), 1, tdmsFile);
	if (charsRead != 1) readingError(1, charsRead, tdmsFile);

	(*storageLocation).value = tdmsLib::parseTdmsValue(type, tdmsFile);
}

void * TDMSCodex::readingError(int attempted, size_t actual, FILE* toClose)
{
	std::cerr << "Incorect number of bytes read, Attempted: " << attempted << " Actual: " << actual << std::endl;
	PL
		return NULL;
}

void * TDMSCodex::openingError(char * filePath, int errorCode)
{
	std::cerr << "Unable to open file: " << filePath << " Error code: " << errorCode << std::endl;
	PL
		return NULL;
}

objectPathCondition TDMSCodex::checkObjectPath(std::list<char *> * alreadyReadPaths, char ** currentPath, tdmsMetaDataElement * element)
{
	int forewardSlashCount = 0;
	for (int i = 0; i < element->objectPath.strLen; i++)
	{
		if (element->objectPath.characters[i] == '/') forewardSlashCount++;
	}
	if (forewardSlashCount < 2) return objectPathNotChannel;

	std::list<char *>::iterator it = alreadyReadPaths->begin();

	for (int i = 0; i < alreadyReadPaths->size(); i++) //Lets hope this works, turns out it didnt
	{
		if (*it == NULL) continue;
		char * stored = *it;
		if (strcmp(stored, element->objectPath.characters) == 0) return objectPathAlreadyRead;
		std::advance(it, 1);
	}

	if (*(currentPath) == NULL)
	{
		(*currentPath) = element->objectPath.characters;
		return newObjectPath;
	}

	if (strcmp((*currentPath), element->objectPath.characters) == 0) return matchesCurrentObjectPath;

	else return skip;
}

objectPathCondition TDMSCodex::checkObjectPathWithQual(std::list<char *> * alreadyReadPaths, char ** currentPath,char *qualifier ,tdmsMetaDataElement * element)
{
	if (strcmp(element->objectPath.characters, qualifier) != 0) return skip;

	int forewardSlashCount = 0;
	for (int i = 0; i < element->objectPath.strLen; i++)
	{
		if (element->objectPath.characters[i] == '/') forewardSlashCount++;
	}
	if (forewardSlashCount < 2) return objectPathNotChannel;

	std::list<char *>::iterator it = alreadyReadPaths->begin();

	for (int i = 0; i < alreadyReadPaths->size(); i++) //Lets hope this works, turns out it didnt
	{
		if (*it == NULL) continue;
		char * stored = *it;
		if (strcmp(stored, element->objectPath.characters) == 0) return objectPathAlreadyRead;
		std::advance(it, 1);
	}

	if (*(currentPath) == NULL)
	{
		(*currentPath) = element->objectPath.characters;
		return newObjectPath;
	}

	if (strcmp((*currentPath), element->objectPath.characters) == 0) return matchesCurrentObjectPath;

	else return skip;
}

void TDMSCodex::processProperty(TDMSDataStorageElement * element, tdmsProperty * currentProperty)
{
	KeyValueProperty * newProperty = (KeyValueProperty *)malloc(sizeof(KeyValueProperty)*1);
	newProperty->Key = currentProperty->propertyName.characters;
	switch (currentProperty->value->getDataType())
	{
		case	tdmsLib::tdmsTypeVoid:
		{
			newProperty->Value = getStringRep();
		}break;

		case	tdmsLib::tdmsTypeI8:
		{
			tdmsLib::tdmsI8 * current = (tdmsLib::tdmsI8 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeI16:
		{
			tdmsLib::tdmsI16 * current = (tdmsLib::tdmsI16 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeI32:
		{
			tdmsLib::tdmsI32 * current = (tdmsLib::tdmsI32 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeI64:
		{
			tdmsLib::tdmsI32 * current = (tdmsLib::tdmsI32 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeU8:
		{
			tdmsLib::tdmsU8 * current = (tdmsLib::tdmsU8 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeU16:
		{
			tdmsLib::tdmsU16 * current = (tdmsLib::tdmsU16 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeU32:
		{
			tdmsLib::tdmsU32 * current = (tdmsLib::tdmsU32 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeU64:
		{
			tdmsLib::tdmsU32 * current = (tdmsLib::tdmsU32 *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeSingleFloat:
		{
			tdmsLib::tdmsSingleFloat * current = (tdmsLib::tdmsSingleFloat *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeDoubleFloat:
		{
			tdmsLib::tdmsDoubleFloat * current = (tdmsLib::tdmsDoubleFloat *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);

		}break;

		case	tdmsLib::tdmsTypeExtendedFloat:
		{
			tdmsLib::tdmsExtendedFloat * current = (tdmsLib::tdmsExtendedFloat *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);

		}break;

		case	tdmsLib::tdmsTypeSingleFloatWithUnit:
		{
			tdmsLib::tdmsSingleFloatWithUnit * current = (tdmsLib::tdmsSingleFloatWithUnit *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeDoubleFloatWithUnit:
		{
			tdmsLib::tdmsDoubleFloatWithUnit * current = (tdmsLib::tdmsDoubleFloatWithUnit *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;

		case	tdmsLib::tdmsTypeExtendedFloatWithUnit:
		{			
			tdmsLib::tdmsExtendedFloatWithUnit * current = (tdmsLib::tdmsExtendedFloatWithUnit *)(currentProperty->value);
			newProperty->Value = getStringRep(current->value);
		}break;
		case	tdmsLib::tdmsTypeString:
		{
			newProperty->Value = MessageHandlingSystem::concatenateStrings(3,"\"", ((tdmsLib::tdmsString *) (currentProperty->value))->characters,"\"");
		}break;
		case	tdmsLib::tdmsTypeBoolean:
		{
			tdmsLib::tdmsBoolean * current = (tdmsLib::tdmsBoolean *)(currentProperty->value);
			newProperty->Value = getStringRepDel(current->value);
		}break;

		case	tdmsLib::tdmsTypeTimeStamp:
		{
			newProperty->Value = convertTDMSTimestampToString((tdmsLib::tdmsTimeStamp *)currentProperty->value);
		}break;

		case	tdmsLib::tdmsTypeFixedPoint:
		{

			std::cerr << "Currently unable to write property: " << currentProperty->propertyName.characters << " fixed point data to .DEF additional property" << std::endl;
			std::cerr << "Please tell Jacob Lehmer, jacob.lehmer@inl.gov, about this and give the following information,  " << __LINE__ << " " << __FILE__ << std::endl;
			std::cerr << "System Exiting"<< std::endl;
			exit(1);

		}break;

		case	tdmsLib::tdmsTypeComplexSingleFloat:
		{

			std::cerr << "Currently unable to write property: " << currentProperty->propertyName.characters << " complex single float data to .DEF additional property" << std::endl;
			std::cerr << "Please tell Jacob Lehmer, jacob.lehmer@inl.gov, about this and give the following information,  " << __LINE__ << " " << __FILE__ << std::endl;

		}break;

		case	tdmsLib::tdmsTypeComplexDoubleFloat:
		{

			std::cerr << "Currently unable to write property: " << currentProperty->propertyName.characters << " complex double float data to .DEF additional property" << std::endl;
			std::cerr << "Please tell Jacob Lehmer, jacob.lehmer@inl.gov, about this and give the following information,  " << __LINE__ << " " << __FILE__ << std::endl;
			std::cerr << "System Exiting" << std::endl;
			exit(1);
		}break;

		case	tdmsLib::tdmsTypeDAQmxRawData:
		{
			std::cerr << "Currently unable to write property: " << currentProperty->propertyName.characters << " DAQmx data to .DEF additional property" << std::endl;
			std::cerr << "Please tell Jacob Lehmer, jacob.lehmer@inl.gov, about this and give the following information,  " << __LINE__ << " " << __FILE__ << std::endl;
			std::cerr << "System Exiting" << std::endl;
			exit(1);
		}break;

		default:
		{
			std::cerr << "Currently unable to write property: " << currentProperty->propertyName.characters << " to .DEF additional property" << std::endl;
			std::cerr << "Please tell Jacob Lehmer, jacob.lehmer@inl.gov, about this and give the following information,  " << __LINE__ << " " << __FILE__ << std::endl;
			std::cerr << "System Exiting" << std::endl;
			exit(1);
		}break;
	}
	element->addMetaDataElement(newProperty);
	free(newProperty);
}

char * convertTDMSTimestampToString(tdmsLib::tdmsTimeStamp * currentTimeStamp)
{
	int64_t rawSeconds = currentTimeStamp->secFromEpoch;
	int64_t rawDays = rawSeconds / (3600 * 24);
	int64_t years = ((rawDays * 4) / (1461)); //1461 is 365.25*4, precomputed so that there is no type changes
														//Although this should probably be adjusted to be even more precise
	int64_t currentYear = years + 1904;
	int currentDay = (rawDays * 4 - years * 1461) / 4;
	bool isLeapYear = (currentYear % 4 == 0) && ((currentYear % 100 != 0) || (currentYear % 400 == 0)); //check this math
	int currentMonth = extractMonthFromDayInYear(currentDay, isLeapYear);//this again
	currentDay -= extractDaysSinceStartOfMonth(currentMonth, isLeapYear);

	int currentSecond = rawSeconds - (rawDays * (3600 * 24));
	int currentHour = currentSecond / (3600);
	currentSecond -= currentHour * 3600;
	int currentMinute = currentSecond / 60;
	currentSecond -= currentMinute * 60;

	char * returnString = (char *)calloc(1, 100);

	sprintf_s(returnString, 100, "%04d-%02d-%02dT%02d:%02d:%02d.%012d", currentYear, currentMonth, currentDay, currentHour, currentMinute, currentSecond, currentTimeStamp->fractionsOfSec);


	return returnString;

}

//This will extract the month from the day in the year as it has been presented
int extractMonthFromDayInYear(int currentDay, bool isLeapYear)
{
	for (int i = 1; i <= 12; i++)
	{
		if (currentDay <= extractDaysSinceStartOfMonth(i + 1, isLeapYear)) return i;
	}
	return 0;
}


int extractDaysSinceStartOfMonth(int currentMonth, bool isLeapYear)
{
	switch (currentMonth)
	{
		case 1: {
			return -1;
		}break;
		case 2:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 3:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 28 + (isLeapYear ? 1 : 0);
		}break;
		case 4:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 5:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 30;
		}break;
		case 6:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 7:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 30;
		}break;
		case 8:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 9:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 10:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 30;
		}break;
		case 11:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
		case 12:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 30;
		}break;
		case 13:
		{
			return extractDaysSinceStartOfMonth(currentMonth - 1, isLeapYear) + 31;
		}break;
	}
	return 0;
}

void TDMSCodex::getDataType(DataDescription * value, tdmsRawDataIndex * index)
{
	rawDataIndexType dataIndexType = index->getRawDataIndexType();

	switch (dataIndexType)
	{
		case _noData:
		{
			//error because this should not get this far
		}break;
		case _identicalToLastSegment:
		{
			//error because this should have the same path as something that already exists
			//Hopefully
		}break;
		case _normalRawDataIndex:
		{
			switch (((normalRawDataIndex *) index)->dataType)
			{
				case tdmsLib::tdmsTypeU8:
				{
					value->BytesPerElem = 1;
					value->Type = DataType::DEF_CHAR;
					value->DataEndianess = Endianess::BIG;
					return;
				}
				case tdmsLib::tdmsTypeI8:
				{
					value->BytesPerElem = 1;
					value->Type = DataType::DEF_INT;
					value->DataEndianess = Endianess::BIG;
					return;
				}
				case tdmsLib::tdmsTypeI16: 
				{
					value->BytesPerElem = 2;
					value->Type = DataType::DEF_INT;
					value->DataEndianess = Endianess::BIG;
					return;
				}
				case tdmsLib::tdmsTypeI32: 
				{
					value->BytesPerElem = 4;
					value->Type = DataType::DEF_INT;
					value->DataEndianess = Endianess::BIG;
					return;	
				}
				case tdmsLib::tdmsTypeU32: 
				{
					value->BytesPerElem = 4;
					value->Type = DataType::DEF_UINT;
					value->DataEndianess = Endianess::BIG;
					return;
				}
				case tdmsLib::tdmsTypeDoubleFloat: 
				{
					value->BytesPerElem = 8;
					value->Type = DataType::DEF_REAL;
					value->DataEndianess = Endianess::BIG;
					return;	
				}
			}
		}break;
		case _daqmxRawDataIndex:
		{
			// I have no idea how this translates to a matlab array format, what is imporant is to get the functionality that bjorn wants
		}break;
	}
}

unsigned int TDMSCodex::appendTDMSDataToDefFile(FILE * defFile, FILE * tdmsFile, long currentDataLocation, int channelIndex, tdmsSegment * currentSegment)
{
	long currentChannelChunkDataSize = 0;
	long currentChannelInterChunkOffset = 0;
	long chunkSize = 0;
	for (unsigned int i = 0; i < currentSegment->metaData.numberOfObjects; i++)
	{
		tdmsMetaDataElement * currentData = &(currentSegment->metaData.metaDataElements[i]);
		long _currentChunkSize = calculateChannelSize(currentData->rawDataIndex);
		if (i == channelIndex)
		{
			currentChannelInterChunkOffset = chunkSize;
			currentChannelChunkDataSize = _currentChunkSize;
		}
		chunkSize += _currentChunkSize;
	}
	long totalNumberOfChunks = calculateNumberOfChunks(currentSegment->LeadIn, chunkSize); //calculate this
	uint32_t totalNumberOfBytesWrittenInThisSegment = 0;

	void * rawData = malloc(currentChannelChunkDataSize);

	for (int i = 0; i < totalNumberOfChunks; i++)
	{
		fseek(tdmsFile, currentDataLocation + (i * chunkSize) + currentChannelInterChunkOffset, SEEK_SET);
		fread(rawData, 1, currentChannelChunkDataSize, tdmsFile);
		appendDataToArray(defFile, currentChannelChunkDataSize, rawData);

		totalNumberOfBytesWrittenInThisSegment += currentChannelChunkDataSize;
	}

	free(rawData);
	return totalNumberOfBytesWrittenInThisSegment;
}


long TDMSCodex::calculateChannelSize(tdmsRawDataIndex * rawDataIndex)
{
	rawDataIndexType dataIndexType = rawDataIndex->getRawDataIndexType();

	switch (dataIndexType)
	{
		case _noData:
		{
			//error because this should not get this far
		}break;
		case _identicalToLastSegment:
		{
			//error because this should have the same path as something that already exists
			//Hopefully
		}break;
		case _normalRawDataIndex:
		{
			long channelSize = 0;


			normalRawDataIndex * nrdi = ((normalRawDataIndex *) rawDataIndex);
			if (nrdi->dataType == tdmsLib::tdmsTypeString)return ((normalRawDataIndex *) rawDataIndex)->totalSize;

			int bytesPerDatum = getBytesPerTdmsDataType(nrdi->dataType);

			return bytesPerDatum * nrdi->arrayDimension*nrdi->numberOfValues;

		}break;
		case _daqmxRawDataIndex:
		{
			// I have no idea how this should be calculated, but it should be simple because there is a raw data offset in the system
			// theoretically anyway
		}break;
	}
	return 0;
}

long TDMSCodex::calculateNumberOfChunks(leadIn leadIn, long chunkSize)
{
	long dataInSegment = leadIn.SegmentLengthPastLeadIn - leadIn.SegmentMetaDataLength;
	return dataInSegment / chunkSize;
}

int TDMSCodex::getBytesPerTdmsDataType(tdmsLib::tdmsDataType type)
{
	switch (type)
	{
		case	tdmsLib::tdmsTypeVoid: {return  0; }break;

		case	tdmsLib::tdmsTypeI8: {return  1; }break;

		case	tdmsLib::tdmsTypeI16: {return  2; }break;

		case	tdmsLib::tdmsTypeI32: {return  4; }break;

		case	tdmsLib::tdmsTypeI64: {return  8; }break;

		case	tdmsLib::tdmsTypeU8: {return  1; }break;

		case	tdmsLib::tdmsTypeU16: {return  2; }break;

		case	tdmsLib::tdmsTypeU32: {return  4; }break;

		case	tdmsLib::tdmsTypeU64: {return  8; }break;

		case	tdmsLib::tdmsTypeSingleFloat: {return  4; }break;

		case	tdmsLib::tdmsTypeDoubleFloat: {return  8; }break;

		case	tdmsLib::tdmsTypeExtendedFloat: {return  10; }break;

		case	tdmsLib::tdmsTypeSingleFloatWithUnit: {return  4; }break;

		case	tdmsLib::tdmsTypeDoubleFloatWithUnit: {return  8; }break;

		case	tdmsLib::tdmsTypeExtendedFloatWithUnit: {return  10; }break;

		case	tdmsLib::tdmsTypeBoolean: {return  1; }break;

		case	tdmsLib::tdmsTypeTimeStamp: {return  16; }break;

		case	tdmsLib::tdmsTypeFixedPoint: {return  8; }break;

		case	tdmsLib::tdmsTypeComplexSingleFloat: {return  4; }break;

		case	tdmsLib::tdmsTypeComplexDoubleFloat: {return  8; }break;

		case	tdmsLib::tdmsTypeDAQmxRawData: {return  8; }break;
	}

	return 0;
}

void TDMSCodex::appendDataToArray(FILE * defFile, unsigned int length, void * data)
{
	fwrite(data, 1, length, defFile);
}

uint64_t TDMSCodex::calculateSegmentLength(PrimaryDataStorageElement * pdse,MessageHandlingSystem * messageHandlingSystem)
{
	uint64_t retval = calculateMetadataLength(pdse, messageHandlingSystem);
	retval += pdse->dataDescription->BytesPerElem * pdse->dataDescription->NumberOfElements;
	return retval;
}

uint64_t TDMSCodex::calculateMetadataLength(PrimaryDataStorageElement * pdse, MessageHandlingSystem * messageHandlingSystem)
{
	uint64_t retval = 0;
	unsigned int lengthOfTitles = strlen("/'Data'");


	const char * hdr = "/'Data'/'";

	//this may have a problem because it may put elements too far down the hierarchy of stuff,
	//however this is unsubstansiated and should be checked during testing
	char * location = messageHandlingSystem->concatenateStrings(3, hdr, pdse->ID, "'");
	lengthOfTitles += strlen(location);
	free(location);

	unsigned int lengthOfStringLengths = sizeof(unsigned int) * 2;
	retval += lengthOfTitles;
	retval += lengthOfStringLengths;

	//retval += sizeof(unsigned int);//337?
	retval += sizeof(unsigned char) * 4;//349
	retval += sizeof(unsigned int);//352
	retval += sizeof(unsigned int);//367
	retval += sizeof(unsigned int);//371
	retval += sizeof(unsigned int);//375
	retval += sizeof(unsigned long long);//378
	retval += sizeof(unsigned int);//383

	for (int i = 0; i < pdse->numberofAdditionalMetadataElements; i++)
	{
		retval += strlen(pdse->additionalMetaDataElements[i].Key);
		retval += sizeof(unsigned int);
		retval += sizeof(unsigned int);

		retval += strlen(pdse->additionalMetaDataElements[i].Value);
		retval += sizeof(unsigned int);
	}
	return retval;
}
