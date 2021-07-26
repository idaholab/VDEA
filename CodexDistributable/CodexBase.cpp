#include "CodexBase.h"

const char * CodexBase::getIdentifier()
{
	return identifier;
}

const char * getStringRep(bool in)
{
	if (in) return "Yes";
	else return "No";
}

char * getStringRepDel(bool value)
{
	int sizeElement = value ? 4 : 5;
	char * stringRep = (char *)malloc(sizeof(char ) * (sizeElement+1));
	if (value)
	{
		stringRep[0] =  't';
		stringRep[1] =  'r';
		stringRep[2] =  'u';
		stringRep[3] =  'e';
	}
	else
	{
		stringRep[0] = 'f';
		stringRep[1] = 'a';
		stringRep[2] = 'l';
		stringRep[3] = 's';
		stringRep[4] = 'e';
	}

	stringRep[sizeElement] = 0;

	return stringRep;
}

char * getStringRep(int value)
{
	int totalLen = snprintf(NULL, 0, "%d", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%d", value);
	return strrep;
}

char * getStringRep(long long value)
{
	int totalLen = snprintf(NULL, 0, "%d", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%d", value);
	return strrep;
}

char * getStringRep(double value)
{
	int totalLen = snprintf(NULL, 0, "%f", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%f", value);
	return strrep;
}

char * getStringRep(long double value)
{
	int totalLen = snprintf(NULL, 0, "%f", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%f", value);
	return strrep;
}

char * getStringRep()
{
	char* strrep = (char*)malloc(sizeof(char) * 1);
	strrep[0] = 0;
	return strrep;
}

char * getStringRep(unsigned int value)
{
	int totalLen = snprintf(NULL, 0, "%d", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%d", value);
	return strrep;
}

char * getStringRep(unsigned long long value)
{
	int totalLen = snprintf(NULL, 0, "%d", value) + 1;
	char* strrep = (char*)malloc(sizeof(char) * totalLen);
	snprintf(strrep, totalLen, "%d", value);
	return strrep;
}

char * removeSpecialCharacters(char * charactersIn)
{
	int normalCharacterCount = 0;
	for (int i = 0; i < strlen(charactersIn); i++)
	{
		normalCharacterCount += (charactersIn[i] == '/'
			|| charactersIn[i] == '\''
			|| charactersIn[i] == '\\'
			|| charactersIn[i] == '.'
			|| charactersIn[i] == '-'
			|| charactersIn[i] == ' ') ? 0 : 1;
	}

	char * newString = (char *)calloc(normalCharacterCount + 1, 1);

	int indexOfGoodString = 0;
	

	for (int i = 0; i < strlen(charactersIn); i++)
	{
		if (!(charactersIn[i] == '/'
			|| charactersIn[i] == '\\'
			|| charactersIn[i] == '\''
			|| charactersIn[i] == '.'
			|| charactersIn[i] == '-'
			|| charactersIn[i] == ' '))
		{
			newString[indexOfGoodString] = charactersIn[i];
			indexOfGoodString++;
		}
	}

	return newString;
}


//checks if the extension exists AND extension is at the end of the string;
bool checkExtension(char * filename, const char * extension)
{
	char * extensionPointer = strstr(filename, extension);

	if (extensionPointer == nullptr) return false;

	int extensionPointerAdj = extensionPointer - filename;
	int lengthdifference = (strlen(filename) - strlen(extension));

	if (extensionPointerAdj == lengthdifference) return true;

	return false;
}
//
//EXPORT bool CodexBase::deleteAdditionalElements()
//{
//	return false;
//}
