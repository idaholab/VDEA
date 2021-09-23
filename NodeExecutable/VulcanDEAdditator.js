
//2021-08-16 JPL this is the system that is responsible for interacting directly with the c++ code backend
const vulcanAdaptor = require('../build/Release/VulcanApplication.node');

onmessage = function(e)
{
	var returnValue = JSON.parse(e.data.replace(/\\/g,"\\\\"));
	
	switch(returnValue.response)
	{
		case "fileSelection":
		{
			var fileListing = vulcanAdaptor.GetFileListing(returnValue.fileSelection);
			postMessage('{"response":"fileSelection","fileSelection":"'+fileListing+'"}');
		}break;
		case "currentPluginListing":
		{
			var selectedPluginListing = vulcanAdaptor.GetPluginListing(returnValue.fileSelection);
			postMessage('{"response":"currentPluginListing","pluginListing":"'+selectedPluginListing+'"}');
		}break;
		case "defaultPluginListing":
		{
			var defaultPluginListing = vulcanAdaptor.GetPluginListingDefault();
			postMessage('{"response":"defaultPluginListing","pluginListingDefault":"'+defaultPluginListing+'"}');
		}break;
		case "convertSingleFile":
		{
			console.log(returnValue);
			var resultMessage = vulcanAdaptor.ConvertSingleFile(returnValue.selectedFile,returnValue.outputDirectory, returnValue.selectedInputCodex,returnValue.selectedOutputCodex);
			postMesage('{"response":"convertSingleFile","singleFileConversion":"'+resultMessage+'"}');
		}break;
	}
}