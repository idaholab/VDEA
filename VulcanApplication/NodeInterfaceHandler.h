#pragma once
//JPL 2021-04-27
//This will be responsible for acting as an interface between the Node Js interface and the rest of the VDEA application
//This will treat the whole system as a command line interface, but one layer down so that the 
//string processing is not neccesary

#include "napi.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "RecursionEngine.h"
#include "CodexBus.h"
#include "DataOutputManager.h"
#include "DataInputManager.h"

namespace NodeInterfaceHandler
{
	//This will get the complete file listing for the directory passed into the system

	Napi::Array getFileListingWrapped(const Napi::CallbackInfo& info);
	Napi::Array getDefaultPluginListing(const Napi::CallbackInfo& info);
	Napi::Array convertSingleFile(const Napi::CallbackInfo & info);
	//add number function
	int add(int x, int y);
	//add function wrapper
	Napi::Number addWrapped(const Napi::CallbackInfo& info);
	//Export API
	Napi::Object Init(Napi::Env env, Napi::Object exports);
	NODE_API_MODULE(addon, Init)


}