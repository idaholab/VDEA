#include "RecursionEngine.h"
#include <filesystem>
#include <stdio.h>
#include <iostream>
#include <list>
#include <deque>

namespace fs = std::filesystem;

namespace RecursionEngine
{
	FileListing::FileListing(std::list<char *> * foundFiles)
	{
		filePaths = (char **)malloc(foundFiles->size() * sizeof(char *));
		std::list<char *>::iterator it = foundFiles->begin();

		for (int i = 0; i < foundFiles->size(); i++) //Lets hope this works, turns out it didnt //come on jake, you have got to update this
		{
			filePaths[i] = *it;
			std::advance(it, 1);
		}

		totalFileCount = foundFiles->size();
	}

	FileListing::~FileListing()
	{
		for (int i = 0; i < totalFileCount; i++)
		{
			free(filePaths[i]);
		}
		free(filePaths);
	}

	FileListing * scanDirectory(char * topOfDirectory)
	{
		std::list<char *> foundFiles;
		std::deque<char *> foundDirectories;

		foundDirectories.push_front(topOfDirectory);
		
		while (foundDirectories.size() > 0)
		{
			char * currentDirectory = foundDirectories.back();
			foundDirectories.pop_back();
			enumerateDirectory(currentDirectory, &foundFiles, &foundDirectories);
		}

		return new FileListing(&foundFiles);
	}

	void enumerateDirectory(char * currentDirectory, std::list<char *> * foundFiles, std::deque<char *> * directories)
	{
		for (fs::directory_entry directoryElement : fs::directory_iterator(currentDirectory))
		{
			std::string * interim = new std::string(directoryElement.path().string());
			char * directoryPath = (char *)(*interim).c_str();
			if (directoryElement.is_directory())
			{
				directories->push_front(directoryPath);
			}
			else if (directoryElement.is_regular_file())
			{
				foundFiles->push_back(directoryPath);
			}
			else
			{
				free(directoryPath);
			}
		}
	}
}