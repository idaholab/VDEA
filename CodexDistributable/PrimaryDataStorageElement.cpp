#include "PrimaryDataStorageElement.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <filesystem>



Endianess recognizeEndianess(char * value)
{
	int valueParsing = -1;

	static const char * littleEndian = "Little";
	static const int littleEndianLength = 6;
	int littleEndianPointer = 0;

	static const char * bigEndian = "Big";
	static const int bigEndianLength = 3;
	int bigEndianPointer = 0;

	int valueLen = strlen(value);

	for (int i = 0; i < valueLen; i++)
	{
		if (value[i] == bigEndian[bigEndianPointer]
			&& (valueParsing == 0 || valueParsing == -1))
		{
			if (bigEndianPointer == bigEndianLength - 1) return Endianess::BIG;
			bigEndianPointer++;
			valueParsing = 0;
		}
		else if (value[i] == littleEndian[littleEndianPointer]
			&& (valueParsing == 1 || valueParsing ==-1))
		{
			if (littleEndianPointer == littleEndianLength - 1) return Endianess::LITTLE;
			littleEndianPointer++;
			valueParsing = 1;
		}
	}

	return Endianess::EUNRECOGNIZED;
}

DataType recognizeDataType(char * value)
{
	int valueParsing = -1;

	static const char * _int = "Int";
	static const int _intLength = 3;
	int intPointer = 0;

	static const char * _real = "Real";
	static const int _realLength = 4;
	int realPointer = 0;

	static const char * _char = "Char";
	static const int _charLength = 4;
	int charPointer = 0;

	static const char * _UnsignedInt = "Unsigned Int";
	static const int _UnsignedIntLength = 4;
	int UnsignedIntPointer = 0;


	int valueLen = strlen(value);

	for (int i = 0; i < valueLen; i++)
	{
		if (value[i] == _int[intPointer]
			&& (valueParsing == -1 || valueParsing ==0)
			)
		{
			if (intPointer == _intLength - 1) return DataType::DEF_INT;
			intPointer++;
			valueParsing = 0;
		}
		else if (value[i] == _real[realPointer]
			&& (valueParsing == -1 || valueParsing == 1))
		{
			if (realPointer == _realLength - 1) return DataType::DEF_REAL;
			realPointer++;
			valueParsing = 1;
		}
		else if (value[i] == _char[charPointer]
			&& (valueParsing == -1 || valueParsing == 2))
		{
			if (charPointer == _charLength - 1) return DataType::DEF_CHAR;
			charPointer++;
			valueParsing = 2;
		}
		else if (value[i] == _UnsignedInt[UnsignedIntPointer]
			&& (valueParsing == -1 || valueParsing == 3))
		{
			if (UnsignedIntPointer == _UnsignedIntLength - 1) return DataType::DEF_UINT;
			UnsignedIntPointer++;
			valueParsing = 3;
		}
	}

	return DataType::DEF_UNRECOGNIZED;
}

bool recognizeYesNo(char * value)
{
	int valueParsing = -1;

	static const char * yes = "Yes";
	static const int yesLength = 3;
	int yesPointer = 0;

	static const char * no = "No";
	static const int noLength = 2;
	int noPointer = 0;

	int valueLen = strlen(value);

	for (int i = 0; i < valueLen; i++)
	{
		if (value[i] == yes[yesPointer]
			&& (valueParsing == -1 || valueParsing == 0)
			)
		{
			if (yesPointer == yesLength - 1) return true;
			yesPointer++;
			valueParsing = 0;
		}
		else if (value[i] == no[noPointer]
			&& (valueParsing == -1 || valueParsing == 1))
		{
			if (noPointer == noLength - 1) return false;
			noPointer++;
			valueParsing = 1;
		}
	}

	return false;
}

PrimaryDataStorageElement::PrimaryDataStorageElement(int systemID, CodexBase * readingCodex)
{
	this->SystemID = systemID;
	setCodex(readingCodex);
	dataDescription = nullptr;
	additionalMetaDataElements = nullptr;
	ID = nullptr;
	temporaryDataFile = false;
}
PrimaryDataStorageElement::~PrimaryDataStorageElement()
{
	free(ID);
	for (int i = 0; i < numberofAdditionalMetadataElements; i++)
	{
		free(additionalMetaDataElements[i].Key);
		free(additionalMetaDataElements[i].Value);
	}
	free(additionalMetaDataElements);

	delete dataDescription;
}

DataDescription::~DataDescription()
{
	free(DataFilePath);
}

void PrimaryDataStorageElement::setCodex(CodexBase * toSet)
{
	readCodex = toSet;
}

CodexBase * PrimaryDataStorageElement::getCodex()
{
	return readCodex;
}

char * PrimaryDataStorageElement::makeDefaultID(char * filename, int entryNo)
{
	const char* spacing = "_";
	int totalLen = strlen(filename) + strlen(spacing) +1 + snprintf(NULL, 0, "%d", entryNo); 
	char* _ID = (char*)malloc(sizeof(char) * totalLen);
	snprintf(_ID, totalLen, "%s%s%d", filename,spacing, entryNo);
	return _ID;
}

bool PrimaryDataStorageElement::checkDataIntegrity(MessageHandlingSystem *  messager, char * filename,int entryNo)
{
	bool goodIntegrity = true;

	if (ID == nullptr)
	{
		ID = makeDefaultID(filename,entryNo);
	}

	if (dataDescription == nullptr)
	{
		char * message = messager->concatenateMessages(3, "Data Description for entry: ", ID, " Does not exist, cannot be read");
		messager->fatal(message);
		free(message);

		goodIntegrity = false;
	}
	else
	{
		if (dataDescription->Type == DataType::DEF_UNRECOGNIZED)
		{
			char * message = messager->concatenateMessages(3, "DataType for Entry: ", ID, " Does not exist, cannot be read");
			messager->fatal(message);
			free(message);
			goodIntegrity = false;
		}		
		
		if (dataDescription->DataEndianess == Endianess::EUNRECOGNIZED) 
		{
			char * message = messager->concatenateMessages(3, "Endianess for Entry: ", ID, " Does not exist, cannot be read");
			messager->fatal(message);
			free(message);
			goodIntegrity = false;
		}
		
		if (dataDescription->BytesPerElem == 0) 
		{
			char * message = messager->concatenateMessages(3, "BytesPerElem for Entry: ", ID, " Does not exist, or is zero cannot be read");
			messager->fatal(message);
			free(message);
			goodIntegrity = false;
		}		
		
		if (dataDescription->NumberOfElements == 0) 
		{
			char * message = messager->concatenateMessages(3,"NumberOfElements for Entry: ", ID, " Does not exist, or is zero cannot be read");
			messager->fatal(message);
			free(message);
			goodIntegrity = false;
		}		
		
		if (dataDescription->DataFilePath == nullptr) 
		{
			char * message = messager->concatenateMessages(3, "DataFilePath: ", ID, " Does not exist, cannot be read");
			messager->fatal(message);
			free(message);

			goodIntegrity = false;
		}
		else
		{
			std::filesystem::path dataFile(dataDescription->DataFilePath);

			if (dataFile.is_absolute())
			{
				std::string * interim = new std::string(dataFile.parent_path().string());
				dataDescription->DataFileParentDirectory = (char *)(*interim).c_str();
				
				interim = new std::string(dataFile.filename().string());
				dataDescription->DataFilePath = (char *)(*interim).c_str();

				if (!std::filesystem::exists(dataFile))
				{
					char * message = messager->concatenateMessages(3, "DataFilePath: ", dataDescription->DataFilePath, " Does not exist, cannot be read");
					messager->fatal(message);
					free(message);
					goodIntegrity = false;
				}
			}
			else
			{
				std::filesystem::path metadataFile(filename);
				

				std::string * interim = new std::string(metadataFile.parent_path().string());
				dataDescription->DataFileParentDirectory = (char *)(*interim).c_str();

				interim = new std::string(dataFile.filename().string());
				char * _filename = (char *)(*interim).c_str();

				char * overallPath = messager->concatenateStrings(3, dataDescription->DataFileParentDirectory, "\\", _filename);

				if (!std::filesystem::exists(overallPath))
				{
					char * message = messager->concatenateMessages(3,"DataFilePath: ", overallPath, " Does not exist, cannot be read");
					messager->fatal(message);
					free(message);
					goodIntegrity = false;
				}
			}
		}


		if (dataDescription->Interleaved)
		{
			//it is perfectly acceptable to have chunk offsets that are zero
			//if (dataDescription->ChunkOffset == 0)
			//{
			//	char * message = messager->concatenateMessages(3,"ChunkOffset for Entry: ", ID, " Does not exist, or is zero cannot be read");
			//	messager->fatal(message);
			//	free(message);
			//	goodIntegrity = false;
			//}

			if (dataDescription->ChunkSize == 0)
			{
				char * message = messager->concatenateMessages(3,"ChunkSize: ", ID, " Does not exist, or is zero cannot be read");
				messager->fatal(message);
				free(message);
				goodIntegrity = false;
			}

		}
		else
		{
			dataDescription->ChunkSize = dataDescription->NumberOfElements * dataDescription->BytesPerElem;
		}
	}
	return goodIntegrity;
}

void PrimaryDataStorageElement::fabricateDEFData()
{
	//This will perform the necessary last minute conversion of whatever the format of the data of read codex is into 
	//a def file so that it can be properly translated
	//if this happens at all this will create a temporary file that will be structured the same as a def file allowing 
	//what amounts to a JIT data translation system.
	//furthermore the primary data storage format will be required to be manipulated 

	readCodex->convertDataToDefFile(this);

}

void PrimaryDataStorageElement::addMetaDataElement(KeyValueProperty * newElement)
{
	numberofAdditionalMetadataElements++;
	KeyValueProperty * newList = (KeyValueProperty*)malloc(sizeof(KeyValueProperty)*numberofAdditionalMetadataElements);
	for (int i = 0; i < numberofAdditionalMetadataElements - 1; i++)
	{
		newList[i] = additionalMetaDataElements[i];
	}
	newList[numberofAdditionalMetadataElements - 1].Key = newElement->Key;
	newList[numberofAdditionalMetadataElements - 1].Value = newElement->Value;

	free(additionalMetaDataElements);

	additionalMetaDataElements = newList;
}

