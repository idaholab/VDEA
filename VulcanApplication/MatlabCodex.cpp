#include "MatlabCodex.h"

MatlabCodex::MatlabCodex()
{
	identifier = "Matlab";
}



void MatlabCodex::processFile(PrimaryDataStorage * dataStorage, MessageHandlingSystem * messager, char * filename)
{
	if (strstr(filename, ".mat") == nullptr
		&& strstr(filename, ".MAT") == nullptr
		) return;

	FILE * matlabfile;

	fopen_s(&matlabfile, filename, "rb");

	char * endiannessIndicator = (char *) malloc(sizeof(char) *3);
	
	fseek(matlabfile, 126, SEEK_SET);

	fread(endiannessIndicator, 1, 2, matlabfile);

	endiannessIndicator[2] = 0;

	Endianess readEndianess = LITTLE;
	if (strcmp(endiannessIndicator, "IM") == 0)
	{
		readEndianess = LITTLE;
	}
	else if (strcmp(endiannessIndicator, "MI") == 0)
	{
		readEndianess = BIG;
		//Woo big endian
	}
	else
	{
		//this needs to throw some kinda error, 
		//at the moment dont care
	}

	currentStorageElementGrouping * cseg = new currentStorageElementGrouping();

	readMatlabMatrixes(matlabfile,filename,dataStorage,cseg,messager,readEndianess);

	fclose(matlabfile);

	dataStorage->addMoreData(cseg);
	delete cseg;
}

void MatlabCodex::readMatlabMatrixes(FILE * matlabFile, char * filename ,PrimaryDataStorage * mainStorage, currentStorageElementGrouping * cseg, MessageHandlingSystem * messager,Endianess currentEndianess)
{
	_fseeki64(matlabFile, 0, SEEK_END);
	unsigned long long fileLength = _ftelli64(matlabFile);
	_fseeki64(matlabFile, 0, SEEK_SET);

	unsigned long long currentMatlabLocation = 128;

	while (currentMatlabLocation < fileLength)
	{
		_fseeki64(matlabFile, currentMatlabLocation, SEEK_SET);
		PrimaryDataStorageElement * currentElement = new PrimaryDataStorageElement(mainStorage->getNextSystemID(), this);
		DataDescription * currentDataDescription = (DataDescription *)malloc(sizeof(DataDescription));
		currentDataDescription->DataEndianess = currentEndianess;
		//read the type to make sure it is a miMatrix
		//read the actual length of all the crap to get the step size
		//read the array flags to get the array type, pretend that this matters
		//read the array dimenstions which actually specifies how long the thing is
		//read the array name
		//acknowledge that the data is right there 
		//build the data description 

		MatlabDataType currentType = readCurrentElementType(matlabFile);
		unsigned int stepSize = readCurrentStepSize(matlabFile) + 8;

		if (currentType != miMATRIX)
		{
			messager->fatal("Error Parsing Matlab File");
			//throw some kinda error
			delete currentElement;
			free(currentDataDescription);
			if (stepSize == 0) stepSize = 1;
			currentMatlabLocation += stepSize;
			continue;
		}
		
		//SEEKING PAST USELESS INFORMATION
		//Specifically, there is no value found in the martix array flags
		_fseeki64(matlabFile, 8, SEEK_CUR);

		readDataIdentifiers(matlabFile, currentDataDescription);
		currentElement->ID = readMatlabArrayName(matlabFile);

		unsigned long long  currentCursorLocation = _ftelli64(matlabFile);
		unsigned int jumpahead = 8- (currentCursorLocation) % 8;
		_fseeki64(matlabFile, jumpahead, SEEK_CUR);

		MatlabDataType currentDataType = readCurrentElementType(matlabFile);

		_fseeki64(matlabFile, 4, SEEK_CUR);

		switch (currentDataType)
		{
			case miINT8: {currentDataDescription->BytesPerElem = 1; currentDataDescription->Type = DEF_INT; }break;
			case miUINT8: {currentDataDescription->BytesPerElem = 1; currentDataDescription->Type = DEF_INT; }break;
			case miINT16: {currentDataDescription->BytesPerElem = 2; currentDataDescription->Type = DEF_INT; }break;
			case miDOUBLE: {currentDataDescription->BytesPerElem = 8; currentDataDescription->Type = DEF_REAL; }break;
			case miINT32: {currentDataDescription->BytesPerElem = 4; currentDataDescription->Type = DEF_INT; }break;
			case miUINT32: {currentDataDescription->BytesPerElem = 4; currentDataDescription->Type = DEF_UINT; }break;
			case miINT64: {currentDataDescription->BytesPerElem = 8; currentDataDescription->Type = DEF_INT; }break;
			case miUTF8: {currentDataDescription->BytesPerElem = 1; currentDataDescription->Type = DEF_UINT; }break;
		}

		currentDataDescription->Interleaved = true;
		currentDataDescription->ChunkSize = fileLength;
		currentDataDescription->ChunkOffset = _ftelli64(matlabFile);
		currentDataDescription->DataFileParentDirectory = nullptr;
		char * filenameCopy = (char *)malloc(strlen(filename) + 1); 
		strcpy_s(filenameCopy, strlen(filename) + 1, filename);
		currentDataDescription->DataFilePath = filenameCopy;
		currentElement->dataDescription = currentDataDescription;
		
		cseg->addToCurrentStorageElementGrouping(currentElement);
		currentMatlabLocation += stepSize;
	}
}

MatlabDataType MatlabCodex::readCurrentElementType(FILE * matlabFile)
{
	int tempValue;
	fread(&tempValue, sizeof(int), 1, matlabFile);

	switch (tempValue)
	{
		case 1: return miINT8;
		case 2: return miUINT8;
		case 3: return miINT16;
		case 9: return miDOUBLE;
		case 5:	return miINT32;
		case 6:	return miUINT32;
		case 12: return	miINT64;
		case 14: return	miMATRIX;
		case 16: return	miUTF8;
	}
	return miINT8;
}

unsigned int MatlabCodex::readCurrentStepSize(FILE * matlabFile)
{
	unsigned int tempValue;
	fread(&tempValue, sizeof(tempValue), 1, matlabFile);
	return tempValue;
}

void MatlabCodex::readDataIdentifiers(FILE * matlabFile, DataDescription * description)
{
	_fseeki64(matlabFile, 16, SEEK_CUR);

	unsigned int columns;
	fread(&columns, sizeof(unsigned int), 1, matlabFile);
	unsigned int rows;
	fread(&rows, sizeof(unsigned int), 1, matlabFile);



	description->NumberOfElements = rows * columns;

}

char * MatlabCodex::readMatlabArrayName(FILE * matlabFile)
{
	_fseeki64(matlabFile, 4, SEEK_CUR);
	unsigned int nameLength;
	fread(&nameLength, sizeof(nameLength), 1, matlabFile);
	char * channelName = (char *)malloc(sizeof(char) *(nameLength + 1));
	fread(channelName, sizeof(char), nameLength, matlabFile);
	channelName[nameLength] = 0;

	return channelName;
}

void MatlabCodex::convertDataToDefFile(PrimaryDataStorageElement * currentElement)
{
	//This converts the inherently interleaved format of the matlab file into the 
	//DEF format which allows 
	DataDescription * matlabDataDescriptor = new DataDescription();
	matlabDataDescriptor->DataFilePath = (char *)malloc(L_tmpnam_s * sizeof(char));
	tmpnam_s(matlabDataDescriptor->DataFilePath, L_tmpnam_s);
	matlabDataDescriptor->ChunkOffset = 0;
	matlabDataDescriptor->ChunkSize = 0;
	matlabDataDescriptor->Interleaved = false;
	currentElement->temporaryDataFile = true;

	FILE * defTempFile = NULL;
	FILE * matlabFile = NULL;

	fopen_s(&defTempFile, matlabDataDescriptor->DataFilePath, "w");
	fopen_s(&matlabFile, currentElement->dataDescription->DataFilePath, "rb");

	void * bufferElementRead = malloc(currentElement->dataDescription->NumberOfElements*currentElement->dataDescription->BytesPerElem);

	_fseeki64(matlabFile, 0, SEEK_END);
	unsigned long long fileLength = _ftelli64(matlabFile);
	_fseeki64(matlabFile, 0, SEEK_SET);

	for (unsigned long long i = 0; i < fileLength; i += currentElement->dataDescription->ChunkSize)
	{
		_fseeki64(matlabFile, i + currentElement->dataDescription->ChunkOffset, SEEK_SET);
		long long dataRead = fread(bufferElementRead, currentElement->dataDescription->BytesPerElem, currentElement->dataDescription->NumberOfElements, matlabFile);
		fwrite(bufferElementRead, currentElement->dataDescription->BytesPerElem, dataRead, defTempFile);
	}
	free(bufferElementRead);
	fclose(matlabFile);
	fclose(defTempFile);

	matlabDataDescriptor->BytesPerElem = currentElement->dataDescription->BytesPerElem;
	matlabDataDescriptor->DataEndianess = currentElement->dataDescription->DataEndianess;
	matlabDataDescriptor->NumberOfElements = currentElement->dataDescription->NumberOfElements;
	matlabDataDescriptor->ChunkSize = currentElement->dataDescription->BytesPerElem*currentElement->dataDescription->NumberOfElements;
	matlabDataDescriptor->Type = currentElement->dataDescription->Type;

	free(currentElement->dataDescription->DataFilePath);
	free(currentElement->dataDescription);

	currentElement->dataDescription = matlabDataDescriptor;
}

bool MatlabCodex::outputFile(PrimaryDataStorageElement * pdse, MessageHandlingSystem * messageHandlingSystem, char * directory)
{
	pdse->fabricateDEFData();

	char * filenameCleaned = removeSpecialCharacters(pdse->ID);
	char * filenameBuffer = messageHandlingSystem->concatenateStrings(3, directory, "\\", filenameCleaned);
	free(filenameCleaned);
	FILE * currentFileToProcess = createMatlabFile(getFilename(filenameBuffer), getDescription());
	free(filenameBuffer);

	for (int i = 0; i < pdse->numberofAdditionalMetadataElements; i++)
	{
		processProperty(currentFileToProcess, &(pdse->additionalMetaDataElements[i]));
	}

	int startofMostRecentArray = ftell(currentFileToProcess);
	setUpMatlabArray(currentFileToProcess, pdse->ID, getMatlabArrayType(pdse->dataDescription), getMatlabDataType(pdse->dataDescription));

	unsigned int valuesWritten = appendDefDataToMatlabArray(currentFileToProcess, pdse->dataDescription);

	closeArray(currentFileToProcess, pdse->ID,getMatlabArrayType(pdse->dataDescription), getMatlabDataType(pdse->dataDescription), startofMostRecentArray, valuesWritten);

	fclose(currentFileToProcess);

	return true;
}

void MatlabCodex::processProperty(FILE * currentFile, KeyValueProperty * currentProperty)
{
	int size =  strlen(currentProperty->Key) + 1;
	char * MatlabPropertyName = (char *)calloc(size + 1, 1);

	char * realName = removeSpecialCharacters(currentProperty->Key);

	writeDataMatlabArray(currentFile, realName, size, mxCHAR_CLASS, miUTF8, strlen(currentProperty->Value),currentProperty->Value);
	free(realName);
	free(MatlabPropertyName);
}

FILE * MatlabCodex::createMatlabFile(char * filePath, char * Description)
{
	FILE * requiredFile = NULL;
	int err = fopen_s(&requiredFile, filePath, "wb");
	int descriptionLength = strlen(Description);
	char * DescriptiveText = (char *)malloc(MATLABHeaderDescriptiveTextLength);
	strncpy_s(DescriptiveText, MATLABHeaderDescriptiveTextLength, Description, MATLABHeaderDescriptiveTextLength);
	for (int i = descriptionLength + 1; i < MATLABHeaderDescriptiveTextLength; i++) DescriptiveText[i] = ' ';

	fwrite(DescriptiveText, 1, MATLABHeaderDescriptiveTextLength, requiredFile);//Descriptive text for .mat file corresponding to the proper length for the item
	unsigned __int8 systemDataOffset[] = { 0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 };
	fwrite(&systemDataOffset, sizeof(unsigned __int8), 8, requiredFile);
	unsigned __int16 version = 0x0100;
	fwrite(&version, sizeof(unsigned __int16), 1, requiredFile);
	char * endianess = (char *)  "IM";
	fwrite(endianess, sizeof(char), 2, requiredFile);
	return requiredFile;
}

void MatlabCodex::writeDataMatlabArray(FILE * file, char * arrayName, unsigned int nameLength, MatlabArrayType arrayType, MatlabDataType dataType, unsigned int length, void * data)
{
	__int32 arraydataType = miMATRIX;
	__int32 hdrlength = 48;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	hdrlength += (nameLength) % 8 != 0 ? nameLength + (8 - ((nameLength) % 8)) : nameLength;
	hdrlength += (length) % 8 != 0 ? length + (8 - ((length) % 8)) : length;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	fwrite(&arraydataType, sizeof(__int32), 1, file);
	fwrite(&hdrlength, sizeof(__int32), 1, file);
	writeArrayFlags(file, arrayType); //16
	writeArrayDimensions(file, dataType, length);//16
	writeMatlabArrayName(file, arrayName, nameLength);//8 + nameLength
	writeDataElement(file, dataType, length, data);//8 + dataLength
}

void MatlabCodex::writeArrayFlags(FILE * file, MatlabArrayType arrayType)
{
	__int32 dataType = miUINT32;
	__int32 length = 8;
	__int32 writeArrayType = arrayType;
	__int32 arrayProcessingFlags = 0x00000000;
	fwrite(&dataType, sizeof(__int32), 1, file);
	fwrite(&length, sizeof(__int32), 1, file);
	fwrite(&writeArrayType, sizeof(__int32), 1, file);
	fwrite(&arrayProcessingFlags, sizeof(__int32), 1, file);

	//this component does not require padding as this is a fixed length field
}

void MatlabCodex::writeArrayDimensions(FILE * file, MatlabDataType dataType, unsigned int length)
{
	__int32 columns = 0x01;
	__int32 rows = length / getBytesPerDataType(dataType);
	__int32 writedataType = miINT32;
	__int32 writelength = 8;

	fwrite(&writedataType, sizeof(__int32), 1, file);
	fwrite(&writelength, sizeof(__int32), 1, file);
	fwrite(&columns, sizeof(__int32), 1, file);
	fwrite(&rows, sizeof(__int32), 1, file);

	//this component does not require padding as this is a fixed length field
}

void MatlabCodex::writeMatlabArrayName(FILE * file, char * name, unsigned int rawLength)
{
	writeDataElement(file, miINT8, rawLength, (void *)name);
}

void MatlabCodex::writeDataElement(FILE * file, MatlabDataType dataType, unsigned int length, void * data)
{
	int paddingLength = (length) % 8 != 0 ? 8 - ((length) % 8) : 0;

	__int32 binDataType = dataType;
	__int32 binLength = length;
	fwrite(&binDataType, sizeof(__int32), 1, file);
	fwrite(&binLength, sizeof(__int32), 1, file);
	fwrite(data, 1, binLength, file);

	//This is for padding the length of the data in order to correspond to 64 bit boundries
	for (int i = 0; i < paddingLength; i++)
	{
		static unsigned __int8 val = 0x00;
		fwrite(&val, 1, 1, file);
	}
}

void MatlabCodex::setUpMatlabArray(FILE * file, char * oldName, MatlabArrayType arrayType, MatlabDataType dataType)
{
	char * arrayName = removeSpecialCharacters(oldName);
	int nameLength = strlen(arrayName);

	__int32 arraydataType = miMATRIX;
	__int32 hdrlength = 48;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	hdrlength += (nameLength) % 8 != 0 ? nameLength + (8 - ((nameLength) % 8)) : nameLength;
	//hdrlength += (length) % 8 != 0 ? length + (8 - ((length) % 8)) : length;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	fwrite(&arraydataType, sizeof(__int32), 1, file);
	fwrite(&hdrlength, sizeof(__int32), 1, file);
	writeArrayFlags(file, arrayType); //16
	unsigned int length = 0x00;
	writeArrayDimensions(file, dataType, length);//16
	writeMatlabArrayName(file, arrayName, nameLength);//8 + nameLength
	openArrayToAppend(file, dataType);
}

//This returns the location that needs to be written to in order to properly set the length
void MatlabCodex::openArrayToAppend(FILE * file, MatlabDataType dataType)
{
	__int32 binDataType = dataType;
	fwrite(&binDataType, sizeof(__int32), 1, file);
	unsigned __int32 placeholder = 0x00;
	fwrite(&(placeholder), sizeof(__int32), 1, file);
}

//This will append the data to the proper location in the array if the file cursor is in the right location
void MatlabCodex::appendDataToArray(FILE * file, unsigned int length, void *data)
{
	fwrite(data, 1, length, file);
}

void MatlabCodex::closeArray(FILE * file, char * oldName, MatlabArrayType arrayType, MatlabDataType dataType, long position, unsigned int totalLength)
{
	//check this against how the tdms file writing works
	char * arrayName = removeSpecialCharacters(oldName);
	int nameLength = strlen(arrayName);
	long oldPosition = ftell(file);
	fseek(file, position, SEEK_SET);

	__int32 length = totalLength;

	__int32 arraydataType = miMATRIX;
	__int32 hdrlength = 48;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	hdrlength += (nameLength) % 8 != 0 ? nameLength + (8 - ((nameLength) % 8)) : nameLength;
	hdrlength += (length) % 8 != 0 ? length + (8 - ((length) % 8)) : length;
	//The sections need to end on 64 bit boundries, and the padding needs to be taken into account
	fwrite(&arraydataType, sizeof(__int32), 1, file);
	fwrite(&hdrlength, sizeof(__int32), 1, file);
	writeArrayFlags(file, arrayType); //16
	writeArrayDimensions(file, dataType, length);//16
	writeMatlabArrayName(file, arrayName, nameLength);//8 + nameLength

	__int32 binDataType = dataType;
	fwrite(&binDataType, sizeof(__int32), 1, file);
	fwrite(&(length), sizeof(__int32), 1, file);

	fseek(file, oldPosition, SEEK_SET);

	long currentFileLength = ftell(file);
	int paddingLength = (currentFileLength) % 8 != 0 ? 8 - ((currentFileLength) % 8) : 0;
	static unsigned __int8 val = 0x00;
	fwrite(&val, 1, paddingLength, file);

	free(arrayName);
}

unsigned int MatlabCodex::appendDefDataToMatlabArray(FILE * toProcess, DataDescription * currentDescription)
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

	unsigned int totalwritten = 0;
	for (unsigned long long i = 0; i < fileLength; i += currentDescription->ChunkSize)
	{
		_fseeki64(oldDataFile, i + currentDescription->ChunkOffset, SEEK_SET);
		long long dataRead = fread(bufferElementRead, currentDescription->BytesPerElem, currentDescription->NumberOfElements, oldDataFile);
		totalwritten += fwrite(bufferElementRead, currentDescription->BytesPerElem, dataRead, toProcess);
	}
	free(bufferElementRead);
	fclose(oldDataFile);

	totalwritten *= currentDescription->BytesPerElem;
	return totalwritten;
}

unsigned int MatlabCodex::getBytesPerDataType(MatlabDataType type)
{
	switch (type)
	{
		case miINT16: return 2;
		case miDOUBLE: return 8;
		case miINT32: return 4;
		case miUINT32: return 4;
	}
	return 1;
}

char * MatlabCodex::getFilename(char * filenamein)
{
	const char * modifier = "MatlabData.mat";
	int totalLen = strlen(modifier) + strlen(filenamein) + 1; //This magic number is bad
	char * filename = (char *)malloc(sizeof(char)*totalLen);
	snprintf(filename, totalLen, "%sMatlabData.mat", filenamein);
	return filename;
}

char* MatlabCodex::getFilenameBigFile(char* filenamein, int fileInteration)
{
	const char* modifier = "MatlabData.mat";
	int totalLen = strlen(modifier) + strlen(filenamein) + 2 + snprintf(NULL, 0, "%d", fileInteration); //This magic number is bad
	char* filename = (char*)malloc(sizeof(char) * totalLen);
	snprintf(filename, totalLen, "%s_%dMatlabData.mat", filenamein, fileInteration);
	return filename;
}

char * MatlabCodex::getDescription()
{
	time_t rawTime;
	tm now;
	time(&rawTime);
	localtime_s(&now, &rawTime);
	int totalLen = MATLABHeaderDescriptiveTextLength;
	char * description = (char *)malloc(sizeof(char)*totalLen);
	snprintf(description, totalLen, "MATLAB 5.0 MAT-file, Platform: Windows, Created on: %02d/%02d/%04d %02d:%02d:%02d",
		now.tm_mon, now.tm_mday, now.tm_year + 1900, now.tm_hour, now.tm_min, now.tm_sec);
	return description;
}

MatlabArrayType MatlabCodex::getMatlabArrayType(DataDescription * dataPointer)
{
	switch (dataPointer->Type)
	{
		case DEF_REAL:
		{
			return mxDOUBLE_CLASS;
		}
		case DEF_INT:
		{
			switch(dataPointer->BytesPerElem)
			{
				case 1: return mxINT8_CLASS;
				case 2:	return mxINT16_CLASS;
				case 4: return mxINT32_CLASS;
				case 8: return mxINT64_CLASS;
			}
		}
		case DEF_CHAR:
		{
			return mxCHAR_CLASS;
		}
	}
	return mxINT8_CLASS;
}

MatlabDataType MatlabCodex::getMatlabDataType(DataDescription * dataPointer)
{
	switch (dataPointer->Type)
	{
		case DEF_REAL:
		{
			return miDOUBLE;
		}
		case DEF_INT:
		{
			switch (dataPointer->BytesPerElem)
			{
				case 1: return miINT8;
				case 2:	return miINT16;
				case 4: return miINT32;
				case 8: return miINT64;
			}
		}
		case DEF_UINT:
		{
			switch (dataPointer->BytesPerElem)
			{
				case 1: return miUINT8;
				case 4: return miUINT32;
			}
		}
		case DEF_CHAR:
		{
			return miUTF8;
		}
	}
	return miUTF8;
}
