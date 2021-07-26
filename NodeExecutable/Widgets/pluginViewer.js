function pluginElement(rawPlugin)
{	
	this.getListItem = function()
	{
		var listItem = document.createElement("li");
		var currentDiv = document.createElement("div");
		var currentButtonGrouping = new pluginButtonGrouping();
		var pluginDetailsCurr = new pluginDetails();
		var spanSection = document.createElement("span");
		spanSection.innerHTML = rawPlugin;
		spanSection.classList.add("leftJustify");
		spanSection.classList.add("verticalSpanCenter");
	    currentDiv.appendChild(spanSection);
		
	    currentDiv.appendChild(pluginDetailsCurr.spanContainer);
		currentDiv.appendChild(currentButtonGrouping.divContainer);
		currentDiv.appendChild(pluginDetailsCurr.additionalDetailsSection);
		currentDiv.classList.add("evenSpacingHorizontalList");
		currentDiv.classList.add("leftRightJustified");
	    currentDiv.classList.add("rowElementBorder");
		listItem.appendChild(currentDiv);
		
		return listItem;
	}
	
}

function pluginButtonGrouping()
{
	this.removeButton = document.createElement("button");
	this.removeButton.innerHTML = "Remove";
	
	this.divContainer = document.createElement("div");
	this.divContainer.classList.add("hBox");
	this.divContainer.classList.add("rightJustify");
	this.divContainer.appendChild(this.removeButton);
	
	
}


function pluginDetails()
{
	this.detailsButton = document.createElement("button");
	this.detailsButton.innerHTML = "Details";
	this.additionalDetailsSection = document.createElement("span");
	this.additionalDetailsSection.classList.add("dropdownContentCollapsed");
	
	//JPL 2021-07-13 This is a really frustrating kludge that is entirely a result of the javascript object model
	this.detailsButton.additionalDetailsSection = this.additionalDetailsSection;
	//however it is fairly clever, given the alternative
	
	this.tableContent = new pluginDetailsContent();
	this.additionalDetailsSection.appendChild(this.tableContent.tableElement);
	
	this.detailsButton.onclick = function()
	{
		this.additionalDetailsSection.classList.toggle("dropdownContentCollapsed");
	}
	
	this.spanContainer = document.createElement("span");
	this.spanContainer.appendChild(this.detailsButton);
	
}

function pluginDetailsContent()
{
	this.tableElement = document.createElement("table");
	this.tableRow = document.createElement("tr");
	this.tableHeader = document.createElement("th");
	this.tableHeader.innerHTML = "Plugin Info"
	this.tableRow.appendChild(this.tableHeader);
	this.tableElement.appendChild(this.tableRow);
	this.tableDataElementFileSize = document.createElement("td");
	this.tableDataElementFilePath = document.createElement("td");
	this.tableDataElementFileType = document.createElement("td");
	
	this.tableElement.appendChild(createSingleDataElementRow("File Size:",this.tableDataElementFileSize));
	this.tableElement.appendChild(createSingleDataElementRow("File Path:",this.tableDataElementFilePath));
}