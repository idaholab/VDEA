#include "CommandLineProcessor.h"
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <filesystem>

const char * CommandLineProcessor::currentCommandLineTags[] = { "-InputDirectory","-OutputDirectory","-InputType","-OutputType","-PluginDirectory","-PrintCodexes"};
const int CommandLineProcessor::commandLineTagCount = 6;

//This is fairly finickey as it uses the order of the tags as they are written in the current command line tag list
void CommandLineArgumentSet::setProperTag(char * value, int currentTag)
{
	switch (currentTag)
	{
		case CommandLineTag::InputDirectory:
		{
			inputDirectory = value;
		}break;
		case CommandLineTag::OutputDirectory:
		{
			outputDirectory = value;
		}break;
		case CommandLineTag::InputType:
		{
			inputCodex = value;
		}break;
		case CommandLineTag::OutputType:
		{
			outputCodex = value;
		}break;
		case CommandLineTag::PluginDirectory:
		{
			pluginLocation = value;
		}break;
		case CommandLineTag::PrintCodexes:
		{
			printCodexes = true;
		}break;
	}
}

bool CommandLineArgumentSet::commandsReadSucessfully()
{
	return (inputDirectory != nullptr
		&& outputDirectory != nullptr
		&& inputCodex != nullptr
		&& outputCodex != nullptr) || printCodexes;
}

CommandLineArgumentSet * CommandLineProcessor::processCommandLineArguments(MessageHandlingSystem * messageHandler, CodexBus* codexBus, int argCount, char ** commandLineArguments)
{
	//need to process the whole list of different arguments looking for the proper identifiers for each of the data elements
	CommandLineArgumentSet * cmdSet = new CommandLineArgumentSet();

	for (int i = 0; i < argCount; i++)
	{
		int commandLineTag = checkIfCommandLineTag(commandLineArguments[i]);

		if(commandLineTag > -1)
		{
			int argumentComponents = 1;
			
			//Need to iterate through all of the other tags to see the end of the strings. 
			while (i + argumentComponents < argCount && checkIfCommandLineTag(commandLineArguments[i + argumentComponents]) < 0 )
			{
				argumentComponents++;
			}

			int size = 0;

			for (int j = 1; j < argumentComponents; j++)
			{
				size += strlen(commandLineArguments[i + j]) + 1;
			}
			size++;
			char * totalCommandLineParameter = (char *)calloc(1,size);

			
			bool loopStart = true;

			for (int j = 1; j < argumentComponents; j++)
			{
				if (loopStart)
				{
					sprintf_s(totalCommandLineParameter, size, "%s%s", totalCommandLineParameter, commandLineArguments[i + j]);
					loopStart = false;
				}
				else
				{
					sprintf_s(totalCommandLineParameter, size, "%s%s%s", totalCommandLineParameter, " ", commandLineArguments[i + j]);
				}
			}

			i += argumentComponents-1;

			//need to have this check the actual associated value of the codex bus
			if (checkCommandLineValue(totalCommandLineParameter, commandLineTag, messageHandler))
			{
				cmdSet->setProperTag(totalCommandLineParameter, commandLineTag);
			}
		}
		//error out for not having any input data
	}

	return cmdSet;
}

int CommandLineProcessor::checkIfCommandLineTag(const char * expectedValue)
{
	for (int i = 0; i < commandLineTagCount; i++)
	{
		const char * currentCommandLineTagToCheck = currentCommandLineTags[i];
		if (std::strcmp(expectedValue, currentCommandLineTagToCheck) == 0)
		{
			return i;
		}
	}
	return -1;
}

bool CommandLineProcessor::checkCommandLineValue(char * commandLineTag, int MatchedCommandLineTag, MessageHandlingSystem * messagingSystem)
{
	switch (MatchedCommandLineTag)
	{
		case CommandLineTag::InputDirectory:
		{
			if (std::filesystem::exists(commandLineTag) && std::filesystem::is_directory(commandLineTag))
			{
				return true;
			}

			messagingSystem->directoryError(commandLineTag);
		}break;
		case CommandLineTag::OutputDirectory:
		{
			if (std::filesystem::exists(commandLineTag) && std::filesystem::is_directory(commandLineTag))
			{
				return true;
			}

			messagingSystem->directoryError(commandLineTag);
		}break;
		case CommandLineTag::PluginDirectory:
		{
			if (std::filesystem::exists(commandLineTag) && std::filesystem::is_directory(commandLineTag))
			{
				return true;
			}

			messagingSystem->directoryError(commandLineTag);
		}break;
		case CommandLineTag::InputType :
		{
			return true;
		}break;
		case CommandLineTag::OutputType:
		{
			return true;
		}break;
		case CommandLineTag::PrintCodexes: return true;
	}

	return false;
}

bool CommandLineProcessor::checkCodexValues(CodexBus * codexBus, CommandLineArgumentSet * commandLineArguments, MessageHandlingSystem * messagingSystem)
{
	bool retvalue = true;
			
	if (codexBus->checkIfTypeExists(commandLineArguments->inputCodex) != -1)
	{
		retvalue &= true;
	}
	else
	{
		retvalue &= false;
		messagingSystem->codexError(commandLineArguments->inputCodex);
	}
		
	if (codexBus->checkIfTypeExists(commandLineArguments->outputCodex) != -1)
	{
		retvalue &= true;
	}
	else
	{
		retvalue &= false;
		messagingSystem->codexError(commandLineArguments->outputCodex);
	}
	return retvalue;
}
