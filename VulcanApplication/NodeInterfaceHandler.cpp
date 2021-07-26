#include "NodeInterfaceHandler.h"
using namespace std;
int NodeInterfaceHandler::add(int x, int y) {
	return (x + y);
}

Napi::Array NodeInterfaceHandler::getFileListingWrapped(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	if (!info[0].IsString())
	{
		Napi::TypeError::New(env, "expected a string").ThrowAsJavaScriptException();
	}
	RecursionEngine::FileListing * currentListing = RecursionEngine::scanDirectory(((char *) ((std::string) info[0].ToString()).c_str()));

	Napi::Array returnBuffer = Napi::Array::New(env, currentListing->totalFileCount);

	for (int i = 0; i < currentListing->totalFileCount; i++)
	{
		returnBuffer.Set(i, currentListing->filePaths[i]);
	}

	return returnBuffer;
}

Napi::Array NodeInterfaceHandler::getDefaultPluginListing(const Napi::CallbackInfo& info)
{
	//std::ofstream logFile("VulcanDELogFile.txt");
	//logFile << __FILE__ << "  " << __LINE__ << " Trying to run the Get Default Plugin Listing" << std::endl;
	Napi::Env env = info.Env();

	CodexBus * defaultCodexBus = new CodexBus();
	defaultCodexBus->initalizeCodexes();
	
	CodexHeader * defaultPluginListing = defaultCodexBus->GetCurrentCodexRepresentation();
	
	Napi::Array returnBuffer = Napi::Array::New(env, defaultPluginListing->totalPluginListing);

	for (int i = 0; i < defaultPluginListing->totalPluginListing; i++)
	{		
		returnBuffer.Set(i, defaultPluginListing->stringRepresentaion[i]);
	}
	
	delete defaultCodexBus;
	delete defaultPluginListing;

	return returnBuffer;
}

Napi::Number NodeInterfaceHandler::addWrapped(const Napi::CallbackInfo& info) {
	Napi::Env env = info.Env();
	//check if arguments are integer only.
	if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
		Napi::TypeError::New(env, "arg1::Number, arg2::Number expected").ThrowAsJavaScriptException();
	}
	//convert javascripts datatype to c++
	Napi::Number first = info[0].As<Napi::Number>();
	Napi::Number second = info[1].As<Napi::Number>();
	//run c++ function return value and return it in javascript
	Napi::Number returnValue = Napi::Number::New(env, NodeInterfaceHandler::add(first.Int32Value(), second.Int32Value()));

	return returnValue;
}
Napi::Object NodeInterfaceHandler::Init(Napi::Env env, Napi::Object exports)
{
	//export Napi function
	exports.Set("add", Napi::Function::New(env, NodeInterfaceHandler::addWrapped));
	exports.Set("GetFileListing", Napi::Function::New(env, NodeInterfaceHandler::getFileListingWrapped));
	exports.Set("GetPluginListingDefault", Napi::Function::New(env, NodeInterfaceHandler::getDefaultPluginListing));
	return exports;
}