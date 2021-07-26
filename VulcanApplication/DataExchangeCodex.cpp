#include "DataExchangeCodex.h"
#include "PrimaryDataStorageElement.h"

char * DataExchangeCodex::getStringJson(char * data, int dataSize, int * currentPosition)
{
	unsigned int strlength = 0;
	int startPosition = -1;
	bool stringStarted = false;
	for (int i = *currentPosition; i < dataSize; i++)
	{
		if (data[i] == '"')
		{
			if (!stringStarted)
			{
				startPosition = i+1;
				stringStarted = true;
				continue;
			}
			else if (data[i - 1] != '\\') break;
		}
		strlength++;
	}

	char * eventualString = (char *)malloc(strlength * sizeof(char) + 1);
	eventualString[strlength] = 0;

	for (int i = 0; i < strlength; i++)
	{
		eventualString[i] = data[startPosition+i];
	}

	*currentPosition += strlength + 2;
	return eventualString;
}
char * DataExchangeCodex::getValueJson(char * data, int dataSize, int * currentPosition)
{
	unsigned int strlength = 0;
	int startPosition = -1;
	int interValueObjectRecursionLevel = 0;
	bool valueParsingStarted = false;
	bool inString = false;
	for (int i = *currentPosition; i < dataSize; i++)
	{
		if (data[i] == ':')
		{
			if (!valueParsingStarted)
			{
				startPosition = i+1;
				valueParsingStarted = true;
				continue;
			}
		}
		else if (data[i] == '"')
		{
			if (data[i - 1] != '\\')
			{
				inString = !inString;
			}
		}
		else if (data[i] == '{')
		{
			if(!inString)interValueObjectRecursionLevel++;
		}
		else if (data[i] == '}')
		{
			if (!inString)
			{
				if (interValueObjectRecursionLevel == 0)
				{
					break;
				}
				else
				{
					interValueObjectRecursionLevel--;
				}
			}
		}
		else if (data[i] == ',')
		{
			if (!inString)
			{
				if (interValueObjectRecursionLevel == 0)
				{
					break;
				}
			}
		}
		strlength++;
	}

	char * eventualString = (char *)malloc(strlength * sizeof(char) + 1);
	eventualString[strlength] = 0;

	for (int i = 0; i < strlength; i++)
	{
		eventualString[i] = data[startPosition+i];
	}

	*currentPosition += strlength ;
	return eventualString;
}

DataExchangeCodex::DataExchangeCodex()
{
	identifier = "DataExchangeFormat";
}

void DataExchangeCodex::processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filename)
{
	//check file type
	//in theory, there is no need to have an error message pop up to say this is the wrong file because there 
	//will be many wrong files that are processed
	if (strstr(filename, ".def") == nullptr 
		&& strstr(filename, ".DEF") == nullptr
		&& strstr(filename,".xdef") == nullptr
		&& strstr(filename,".XDEF") == nullptr
		&& strstr(filename,".jdef") == nullptr
		&& strstr(filename,".JDEF") == nullptr
 		) return; 

	FILE * currentFileToProcess;
	fopen_s(&currentFileToProcess, filename, "rb");

	fseek(currentFileToProcess, 0, SEEK_END);

	unsigned int fileSize = ftell(currentFileToProcess);

	fseek(currentFileToProcess, 0, SEEK_SET);

	char * rawData = (char *)malloc(fileSize * sizeof(char));

	fread(rawData, sizeof(char), fileSize, currentFileToProcess);

	fclose(currentFileToProcess);

	if (strstr(filename, ".xdef") != nullptr|| strstr(filename, ".XDEF") != nullptr)
	{
		currentStorageElementGrouping * cseg = new currentStorageElementGrouping();
		readXML(dataStorage, cseg, messager, rawData, fileSize, filename);
		dataStorage->addMoreData(cseg);
		delete cseg;
	}
	else if(strstr(filename, ".jdef") != nullptr || strstr(filename, ".JDEF") != nullptr) readJson(dataStorage, messager, rawData, fileSize,filename);
	else
	{
		if (strstr(filename, ".def") != nullptr || strstr(filename, ".DEF") != nullptr)
		{
			if (isJson(rawData, fileSize))
			{
				readJson(dataStorage, messager, rawData, fileSize, filename);
			}
			else
			{
				currentStorageElementGrouping * cseg = new currentStorageElementGrouping();
				readXML(dataStorage, cseg, messager, rawData, fileSize, filename);
				dataStorage->addMoreData(cseg);
				delete cseg;
			}
		}
	}
}

bool DataExchangeCodex::isJson(char * data, unsigned int dataLength)
{
	for (int i = 0; i < dataLength; i++)
	{
		if (data[i] == '{') return true;
		else if (data[i] == '<') return false;
	}

	//this may need to have some sort of messanger connection that allows systems to report a meaningful error
	return false;
}

void DataExchangeCodex::readXML(PrimaryDataStorage * dataStorage, currentStorageElementGrouping * cseg,MessageHandlingSystem * messagingSystem, char * data, unsigned int fileSize, char * filename)
{

	char * tagName;

	int currentElementCount = 0;
	for (int i = 0; i < fileSize; i++)
	{
		switch (data[i])
		{
			case '<':
			{
				if (i + 1 < fileSize && data[i+1] != '/')
				{
					char * currentTag = extractXMLTag(data, i, fileSize);
					if (currentTag != nullptr)
					{
						char * interTagValue = extractXMLValue(data, &i, fileSize, currentTag);
						assignXMLValue(currentTag, interTagValue, filename, cseg, dataStorage, messagingSystem);
					}
					else
					{
						messagingSystem->fatal(messagingSystem->concatenateStrings(2, "XML Tag Does Not Exist in file:", filename));
					}
				}
			}
		}
	}

}

char * DataExchangeCodex::extractXMLTag(char * data, int currentPosition, unsigned int length)
{
	int startLocationOfTag = -1;
	int stringLength = 0;

	for (int i = currentPosition; i < length; i++)
	{
		switch (data[i])
		{
			case '<':
			{
				if (startLocationOfTag == -1)
				{
					startLocationOfTag = i + 1;
					continue;
				}
			}break;
			case '>':
			{
				char * returnValue = (char *)malloc((stringLength + 1) * sizeof(char));
				for (int k = 0; k < stringLength;k++)
				{
					returnValue[k] = data[k + startLocationOfTag];
				}
				returnValue[stringLength] = 0;
				return returnValue;
			}break;
		}
		stringLength++;
	}

	return nullptr;
}

void DataExchangeCodex::assignXMLValue(char * currentTag, char * value,char * filename ,currentStorageElementGrouping * currentSet,PrimaryDataStorage * storage,MessageHandlingSystem * messager)
{
	static PrimaryDataStorageElement * pdse = nullptr;
	static int currentElementCount = 0;
	static bool currentlyRecursingIntoDataDescription = false;

	if (strcmp(currentTag, "MetaData") == 0)
	{
		if (pdse != nullptr)
		{
			messager->fatal("XML file malformed, multiple \"MetaData\" tags in file");
			return;
		}
		pdse = new PrimaryDataStorageElement(storage->getNextSystemID(),this);
		readXML(storage, currentSet, messager, value, strlen(value), filename);
		if (pdse->checkDataIntegrity(messager, filename, currentElementCount))
		{
			currentSet->addToCurrentStorageElementGrouping(pdse);
			currentElementCount++;
			pdse = nullptr;
		}
		else
		{
			delete pdse;
		}
	}
	else
	{
		if (pdse == nullptr)
		{
			messager->fatal("XML file malformed, \"MetaData\" tag does not preceed content");
			return;
		}
		if(strcmp(currentTag, "DataDescription") == 0)
		{
			currentlyRecursingIntoDataDescription = true;
			if (pdse->dataDescription != nullptr)
			{
				messager->fatal("XML file malformed, multiple \"DataDescription\" tags in single metadata file");
				return;
			}
			pdse->dataDescription = new DataDescription();
			readXML(storage, currentSet, messager, value, strlen(value), filename);
			currentlyRecursingIntoDataDescription = false;
		}
		else
		{
			if (currentlyRecursingIntoDataDescription)
			{
				assignToDataDescription(pdse->dataDescription, currentTag, value,false);
			}
			else
			{
				if (strcmp(currentTag, "ID") == 0)
				{
					pdse->ID = value;	
				}
				else
				{
					char * cleanedString = value;
				
					pdse->numberofAdditionalMetadataElements++;
					KeyValueProperty * newList = (KeyValueProperty*)malloc(sizeof(KeyValueProperty)*pdse->numberofAdditionalMetadataElements);
					for (int i = 0; i < pdse->numberofAdditionalMetadataElements - 1; i++)
					{
						newList[i] = pdse->additionalMetaDataElements[i];
					}
					newList[pdse->numberofAdditionalMetadataElements - 1].Key = currentTag;
					newList[pdse->numberofAdditionalMetadataElements - 1].Value = cleanedString;

					free(pdse->additionalMetaDataElements);

					pdse->additionalMetaDataElements = newList;
				}
			}
		}
	}
}

char * DataExchangeCodex::extractXMLValue(char * data, int * pointer, unsigned int length, char * tag)
{
	int startOfValue = -1;
	int valueLength = 0;
	bool lookingForTag = true;

	for (int i = *pointer; i < length; i++)
	{
		switch (data[i])
		{
			case '<':
			{
				int startPos = i;
				int tagLength = strlen(tag);
				bool tagMatch = true;
				bool endTag = false;
				i++;
				if (!lookingForTag)
				{
					//if the original tag has been found and now looking for a closing tag
					if (data[i] != '/')
					{
						tagMatch = false;
					}
					else
					{
						endTag = true;
						valueLength--;
					}
					i++;
				}
				if (tagMatch)
				{
					for (int tgm = 0; tgm < tagLength; tgm++)
					{
						if (tag[tgm] != data[i])
						{
							tagMatch = false;
							break;
						}
						i++;
					}
					if (data[i] != '>') tagMatch = false;
					if (tagMatch)
					{
						if (lookingForTag)
						{
							startOfValue = i + 1;
							lookingForTag = false;
						}
						else
						{
							char * returnValue = (char *)malloc((valueLength + 1) * sizeof(char));
							for (int k = 0; k < valueLength; k++)
							{
								returnValue[k] = data[k + startOfValue];
							}
							returnValue[valueLength] = 0;
							*pointer = i;
							return returnValue;
						}
					}
				}

				if (!tagMatch && startOfValue != -1)
				{
					valueLength += (i - startPos);
					if (endTag) valueLength++;
				}
			}break;
		}
		if (startOfValue != -1) valueLength++;
		*pointer = i;
	}
	
}

void DataExchangeCodex::readJson(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messagingSystem, char * data, unsigned int fileSize,char * filename)
{
	
	char * tagName;
	currentStorageElementGrouping * cseg = new currentStorageElementGrouping();
	
	int currentElementCount = 0;
	for (int i = 0; i < fileSize; i++)
	{
		switch(data[i])
		{
			case '{':
			{
				//start looking
			}break;
			case '"': 
			{
				char * string = getStringJson(data,fileSize,&i);

				if (strcmp(string, "MetaData") == 0)
				{
					PrimaryDataStorageElement * pdse = new PrimaryDataStorageElement(dataStorage->getNextSystemID(),this);
					parseJsonMetaDataValue(data, fileSize, &i, pdse);
					if (pdse->checkDataIntegrity(messagingSystem, filename, currentElementCount))
					{
						cseg->addToCurrentStorageElementGrouping(pdse);
						currentElementCount++;
					}
					else
					{
						delete pdse;
					}
				}
				
				
			}break;
			case '}':
			{
			}break;
		}
	}

	dataStorage->addMoreData(cseg);
	delete cseg;
}



void DataExchangeCodex::parseJsonMetaDataValue(char * data, unsigned int totalSize, int * currentPosition, PrimaryDataStorageElement * currentElement)
{

	for (int i = *currentPosition; i < totalSize; i++)
	{
		switch (data[i])
		{
			case '"':
			{
				char * string = getStringJson(data, totalSize, &i);

				if (strcmp(string, "DataDescription") == 0)
				{
					parseJsonDataDescriptionValue(data, totalSize, &i, currentElement);
					continue;
				}

				char * value = getValueJson(data, totalSize, &i);
				assignValueToTag(currentElement, string, value);

				*currentPosition = i;
				continue;
			}break;
		}
	}
}

void DataExchangeCodex::assignValueToTag(PrimaryDataStorageElement * currentElement, char * tag, char * value)
{
	if (strcmp(tag, "ID") == 0)
	{
		int startPos = 0;
		currentElement->ID = getStringJson(value,strlen(value),&startPos);
		free(value);
		free(tag);
	}
	else
	{
		int pos = 0;
		char * cleanedString = getStringJson(value,strlen(value),&pos);
		free(value);

		currentElement->numberofAdditionalMetadataElements++;
		KeyValueProperty * newList = (KeyValueProperty*)malloc(sizeof(KeyValueProperty)*currentElement->numberofAdditionalMetadataElements);
		for (int i = 0; i < currentElement->numberofAdditionalMetadataElements - 1; i++)
		{
			newList[i] = currentElement->additionalMetaDataElements[i];
		}
		newList[currentElement->numberofAdditionalMetadataElements - 1].Key = tag;
		newList[currentElement->numberofAdditionalMetadataElements - 1].Value = cleanedString;

		free(currentElement->additionalMetaDataElements);

		currentElement->additionalMetaDataElements = newList;
	}
}

void DataExchangeCodex::parseJsonDataDescriptionValue(char * data, unsigned int totalSize, int * currentLocation, PrimaryDataStorageElement * pdse)
{
	char * tagName;
	
	pdse->dataDescription = new DataDescription();

	for (int i = *currentLocation; i < totalSize; i++)
	{
		switch (data[i])
		{
			case '{':
			{
			}break;
			case '"':
			{
				char * string = getStringJson(data, totalSize, &i);
			
				char * value = getValueJson(data, totalSize, &i);
				
				assignToDataDescription((pdse->dataDescription),string,value,true);
				
			}break;
			case '}':
			{
				*currentLocation = i;
				return;
			}break;
		}
	}
}

void DataExchangeCodex::assignToDataDescription(DataDescription* description, char * tag, char * value,bool json)
{
	if (strcmp(tag, "DataType") == 0)
	{
		description->Type = recognizeDataType(value);
	}
	else if (strcmp(tag, "BytesPerElem") == 0)
	{
		description->BytesPerElem = strtoul(value, nullptr, 10);
	}
	else if (strcmp(tag, "Endianness") == 0)
	{
		description->DataEndianess = recognizeEndianess(value);
	}
	else if (strcmp(tag, "NumberOfElements") == 0)
	{
		description->NumberOfElements = strtoull(value, nullptr, 10);
	}
	else if (strcmp(tag, "DataFilePath") == 0)
	{
		if (json)
		{
			int location = 0;
			description->DataFilePath = getStringJson(value, strlen(value), &location);
		}
		else
		{
			char * cpystring = (char *)malloc(strlen(value)+1);
			memcpy(cpystring, value, strlen(value) + 1);
			description->DataFilePath = cpystring;
		}
	}
	else if (strcmp(tag, "Interleaved") == 0)
	{
		description->Interleaved = recognizeYesNo(value);
	}
	else if (strcmp(tag, "ChunkOffset") == 0)
	{
		description->ChunkOffset = strtoull(value, nullptr, 10);
	}
	else if (strcmp(tag, "ChunkSize") == 0)
	{
		description->ChunkSize = strtoull(value, nullptr, 10);
	}
	free(value);
}

bool DataExchangeCodex::outputFile(PrimaryDataStorageElement * pdse, MessageHandlingSystem * messageHandlingSystem,char * directory)
{
	if(strcmp(pdse->getCodex()->getIdentifier(), getIdentifier() )!=0) pdse->fabricateDEFData();

	FILE * currentFileToProcess;
	char * fileNameCleaned = removeSpecialCharacters(pdse->ID);
	char * filenameBuffer = messageHandlingSystem->concatenateStrings(4,directory, "\\", fileNameCleaned, ".jdef");
	fopen_s(&currentFileToProcess, filenameBuffer, "w");
	free(filenameBuffer);
	free(fileNameCleaned);

	const char * hdr = "{\n\"MetaData\":{\n";

	fwrite(hdr, sizeof(char), strlen(hdr), currentFileToProcess);

	const char * descriptionhdr = "\"DataDescription\":{\n";
	fwrite(descriptionhdr, sizeof(char), strlen(descriptionhdr), currentFileToProcess);

	char * buffer = MessageHandlingSystem::concatenateStrings(3,"\"DataType\":\"", getStringRep(pdse->dataDescription->Type), "\",\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(buffer);

	char * elementBuffer = getStringRep(pdse->dataDescription->BytesPerElem);
	buffer = MessageHandlingSystem::concatenateStrings(3 ,"\"BytesPerElem\":", elementBuffer, ",\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(elementBuffer);
	free(buffer);

	buffer = MessageHandlingSystem::concatenateStrings(3 , "\"Endianness\":\"", getStringRep(pdse->dataDescription->DataEndianess), "\",\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(buffer);

	elementBuffer = getStringRep(pdse->dataDescription->NumberOfElements);
	buffer = MessageHandlingSystem::concatenateStrings(3,"\"NumberOfElements\":", elementBuffer, ",\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(elementBuffer);
	free(buffer);

	char * dataFileName;

	std::filesystem::path dataFilePath = std::filesystem::path(pdse->dataDescription->DataFilePath);
	if (dataFilePath.is_relative())
	{
		dataFileName = pdse->dataDescription->DataFilePath;
	}
	else
	{
		std::string * interim = new std::string(dataFilePath.filename().string());
		dataFileName = (char *)(*interim).c_str();
	}

	char * dataFileCompletePath = messageHandlingSystem->concatenateStrings(4, directory, "\\", dataFileName, ".dat");
	char * dataFileRelToOut = messageHandlingSystem->concatenateStrings(2, dataFileName, ".dat");
	buffer = MessageHandlingSystem::concatenateStrings(3,"\"DataFilePath\":\"", dataFileRelToOut, "\"\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(buffer);
	free(dataFileRelToOut);

	if (pdse->dataDescription->Interleaved)
	{
		buffer = MessageHandlingSystem::concatenateStrings(3,",\"Interleaved\":\"", getStringRep(pdse->dataDescription->Interleaved), "\",\n");
		fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
		free(buffer);

		elementBuffer = getStringRep(pdse->dataDescription->ChunkOffset);
		buffer = MessageHandlingSystem::concatenateStrings(3,"\"ChunkOffset\":", elementBuffer, ",\n");
		fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
		free(elementBuffer);
		free(buffer);

		elementBuffer = getStringRep(pdse->dataDescription->ChunkSize);
		buffer = MessageHandlingSystem::concatenateStrings(3,"\"ChunkSize\":", elementBuffer, "\n");
		fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
		free(elementBuffer);
		free(buffer);
	}

	const char * descriptionclsr = "}\n";
	fwrite(descriptionclsr, sizeof(char), strlen(descriptionclsr), currentFileToProcess);

	buffer = MessageHandlingSystem::concatenateStrings(3,"\"ID\":\"", pdse->ID, "\",\n");
	fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
	free(buffer);

	if (pdse->numberofAdditionalMetadataElements > 0)
	{
		buffer = MessageHandlingSystem::concatenateStrings(5,"\"", pdse->additionalMetaDataElements[0].Key,"\":\"", pdse->additionalMetaDataElements[0].Value, "\"\n");
		fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
		free(buffer);

		for (int i = 1; i < pdse->numberofAdditionalMetadataElements; i++)
		{
			buffer = MessageHandlingSystem::concatenateStrings(5, "\n,\"", pdse->additionalMetaDataElements[i].Key, "\":\"", pdse->additionalMetaDataElements[i].Value, "\"\n");
			fwrite(buffer, sizeof(char), strlen(buffer), currentFileToProcess);
			free(buffer);
		}
	}

	fwrite(descriptionclsr, sizeof(char), strlen(descriptionclsr), currentFileToProcess);
	fwrite(descriptionclsr, sizeof(char), strlen(descriptionclsr), currentFileToProcess);
	fclose(currentFileToProcess);
	
	
	writeDataSection(pdse->dataDescription, dataFileCompletePath,messageHandlingSystem);

	return true;
}

void DataExchangeCodex::writeDataSection(DataDescription * dataPointer, char * filename, MessageHandlingSystem * messager)
{
	FILE * oldDataFile;
	if (dataPointer->DataFileParentDirectory == nullptr)
	{
		fopen_s(&oldDataFile, dataPointer->DataFilePath, "rb");
	}
	else
	{
		char * actualFilePath = MessageHandlingSystem::concatenateStrings(3, dataPointer->DataFileParentDirectory, "\\", dataPointer->DataFilePath);
		fopen_s(&oldDataFile, actualFilePath, "rb");
		free(actualFilePath);
	}

	FILE * dataFileToWriteTo;
	fopen_s(&dataFileToWriteTo, filename, "ab");

	void * bufferElementRead = malloc(dataPointer->NumberOfElements*dataPointer->BytesPerElem);

	_fseeki64(oldDataFile, 0, SEEK_END);
	unsigned long long fileLength = _ftelli64(oldDataFile);
	_fseeki64(oldDataFile, 0, SEEK_SET);

	for (unsigned long long i = 0; i < fileLength; i += dataPointer->ChunkSize)
	{
		_fseeki64(oldDataFile, i + dataPointer->ChunkOffset, SEEK_SET);
		long long dataRead =fread(bufferElementRead, dataPointer->BytesPerElem, dataPointer->NumberOfElements, oldDataFile);
		fwrite(bufferElementRead, dataPointer->BytesPerElem,dataRead, dataFileToWriteTo);
	}
	free(bufferElementRead);
	fclose(oldDataFile);
	fclose(dataFileToWriteTo);

}

void DataExchangeCodex::convertDataToDefFile(PrimaryDataStorageElement * data)
{
	//There is nothing to be done as the data is already a def file
}

const char * getStringRep(DataType in)
{
	switch (in)
	{
		case DEF_INT: return "Int";
		case DEF_REAL: return "Real";
		case DEF_CHAR: return "Char";
		case DEF_UINT: return "Unsigned Int";
		case DEF_UNRECOGNIZED: return "Unrecognized";
		default:
			return "Error";
	}
}

const char * getStringRep(Endianess in)
{
	switch (in)
	{
		case LITTLE: return "Little";
			break;
		case BIG: return "Big";
			break;
		case EUNRECOGNIZED: return "Unrecognized";
			break;
		default: return "Error";
			break;
	}
}
