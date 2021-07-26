#pragma once
#include <stdio.h>
#include <iostream>
#include <deque>
#include <list>

namespace RecursionEngine
{
	class FileListing
	{
	private:
	public:
		FileListing(std::list<char *>* foundFiles);
		char ** filePaths;
		size_t totalFileCount;
		~FileListing();
	};

	FileListing * scanDirectory(char * topOfDirectory);
	void enumerateDirectory(char * currentDirectory, std::list<char *> * foundFiles, std::deque<char *> * foundDirectories);
}