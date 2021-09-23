// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// No Node.js APIs are available in this process because
// `nodeIntegration` is turned off. Use `preload.js` to
// selectively enable features needed in the rendering
// process.

var worker = new Worker('VulcanDEAdditator.js');

var currentlyFoundElements = [];

document.getElementById("newScanButton").addEventListener("click", openFileDialogForScan);
document.getElementById("pluginViewScreen").addEventListener("click", showDefaultPluginListing)

function openFileDialogForScan(e)
{
	api.send('openFileDialog');
	api.on('fileSelected', (event, data) => {
		var message = '{"response":"fileSelection","fileSelection":"'+data+'"}'
		worker.postMessage(message);
	});	
}

function openFileDialogForOutput(e)
{
	console.log(e);
	api.send('openDirectorySelector');
	api.on('directorySelected', (event, data) => {
		var message = '{"response":"convertSingleFile","outputDirectory":"'+data.selectedDirectory+'","selectedFile":"'+e.srcElement.parentObject.filePath+'","selectedInputCodex":"'+data.selectedInputCodex+'","selectedOutputCodex":"'+data.selectedOutputCodex+'"}'
		console.log(message);
		worker.postMessage(message);
	});	
}

function showDefaultPluginListing(e)
{
	worker.postMessage('{"response":"defaultPluginListing"}');
}


//This is the function that will return the information coming from the worker
worker.onmessage = function(event) { 
   
	//this will probably need to be made into something more modular to make this more readable
	var returnValue = JSON.parse(event.data.replace(/\\/g,"\\\\"));  
	
	//this is returning a string that needs to be parsed as an array
	console.log(returnValue.response);
	console.log(returnValue);
	switch(returnValue.response)
	{
		case "fileSelection":
		{
			handleFileSelection(returnValue);
		}break;
		case "defaultPluginListing":
		{
			displayDefaultPlugins(returnValue);
		}break;
		case "convertSingleFile":
		{
			showOutputValue(returnValue);
		}break;
	}
}

worker.onerror = function (event) {
  console.log(event.message, event);
};

function handleFileSelection(returnValue)
{
	if(returnValue.fileSelection != null)
	{   
	   var fileListing = returnValue.fileSelection.split(",");
	   
	   var centerHolder = document.getElementById("mainContentArea");
	   centerHolder.innerHTML = "";
	   var unorderedList = document.createElement("ul");
	   unorderedList.classList.add("evenSpacingHorizontalList"); 
	   for(var i =0;i < fileListing.length;i++)
	   {
			var currentItem = new foundFileProcessingElement(fileListing[i]);
			setButtonCommands(currentItem);
			currentlyFoundElements.push(currentItem);
			unorderedList.appendChild(currentItem.getListItem());
	   }
	   
	   centerHolder.appendChild(unorderedList);
	   //create all necessary elements from the copy element 
	   //document.querySelector('h1').innerHTML = "native addon directory search('.'): " + returnValue.fileSelection;
	   
	}
}

function setButtonCommands(currentItem)
{
	//This seems wrong but I dont know how else to do this but I am running out of other options
	currentItem.ButtonGrouping.outputButton.parentObject = currentItem;
	currentItem.ButtonGrouping.outputButton.addEventListener("click",openFileDialogForOutput);
}

function displayDefaultPlugins(returnValue)
{
	if(returnValue.pluginListingDefault != null)
	{
		var plugins = returnValue.pluginListingDefault.split(",");
		var centerHolder = document.getElementById("mainContentArea");
		centerHolder.innerHTML = "";
		
		var unorderedList = document.createElement("ul");
		unorderedList.classList.add("evenSpacingHorizontalList"); 
		for(var i =0;i < plugins.length;i++)
		{
			var currentItem = new pluginElement(plugins[i]);
			currentlyFoundElements.push(currentItem);
			unorderedList.appendChild(currentItem.getListItem());
		}

		centerHolder.appendChild(unorderedList);
		//create all necessary elements from the copy element 
		//document.querySelector('h1').innerHTML = "native addon directory search('.'): " + returnValue.fileSelection;

	}
}