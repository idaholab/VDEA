
function foundFileProcessingElement(rawFilePath)
{
	this.fileName = rawFilePath.slice(rawFilePath.lastIndexOf("\\") +1);
	this.filePath = rawFilePath;
	this.ButtonGrouping = new buttonGrouping();
	
	this.getListItem = function()
	{
		var listItem = document.createElement("li");
		var currentDiv = document.createElement("div");
		var currentButtonGrouping = this.ButtonGrouping;
		var fileDetailsCurr = new fileDetails();
		var spanSection = document.createElement("span");
		spanSection.innerHTML = this.fileName;
		fileDetailsCurr.additionalDetailsSection.tableDataElementFilePath = rawFilePath;
		spanSection.classList.add("leftJustify");
		spanSection.classList.add("verticalSpanCenter");
	    currentDiv.appendChild(spanSection);
		
	    currentDiv.appendChild(fileDetailsCurr.spanContainer);
		currentDiv.appendChild(currentButtonGrouping.divContainer);
		currentDiv.appendChild(fileDetailsCurr.additionalDetailsSection);
		currentDiv.classList.add("evenSpacingHorizontalList");
		currentDiv.classList.add("leftRightJustified");
	    currentDiv.classList.add("rowElementBorder");
		listItem.appendChild(currentDiv);
			
		
		return listItem;
	}	
}

function buttonGrouping()
{
	this.modifyButton = document.createElement("button");
	this.modifyButton.innerHTML = "Modify";
	this.graphButton = document.createElement("button");
	this.graphButton.innerHTML = "Graph";
	this.outputButton = document.createElement("button");
	this.outputButton.innerHTML = "Output";
	
	this.divContainer = document.createElement("div");
	this.divContainer.classList.add("hBox");
	this.divContainer.classList.add("rightJustify");
	this.divContainer.appendChild(this.modifyButton);
	this.divContainer.appendChild(this.graphButton);
	this.divContainer.appendChild(this.outputButton);
}

function fileDetails()
{
	this.detailsButton = document.createElement("button");
	this.detailsButton.innerHTML = "Details";
	this.additionalDetailsSection = document.createElement("span");
	this.additionalDetailsSection.classList.add("dropdownContentCollapsed");
	
	//JPL 2021-07-13 This is a really frustrating kludge that is entirely a result of the javascript object model
	this.detailsButton.additionalDetailsSection = this.additionalDetailsSection;
	//however it is fairly clever, given the alternative
	
	this.tableContent = new fileDetailsTableContent();
	this.additionalDetailsSection.appendChild(this.tableContent.tableElement);
	
	this.detailsButton.onclick = function()
	{
		this.additionalDetailsSection.classList.toggle("dropdownContentCollapsed");
	}
	
	this.spanContainer = document.createElement("span");
	this.spanContainer.appendChild(this.detailsButton);
	
}

function fileDetailsTableContent()
{
	this.tableElement = document.createElement("table");
	this.tableRow = document.createElement("tr");
	this.tableHeader = document.createElement("th");
	this.tableHeader.innerHTML = "File Info"
	this.tableRow.appendChild(this.tableHeader);
	this.tableElement.appendChild(this.tableRow);
	this.tableDataElementFileSize = document.createElement("td");
	this.tableDataElementFilePath = document.createElement("td");
	this.tableDataElementFileType = document.createElement("td");
	
	this.tableElement.appendChild(createSingleDataElementRow("File Size:",this.tableDataElementFileSize));
	this.tableElement.appendChild(createSingleDataElementRow("File Path:",this.tableDataElementFilePath));
	this.tableElement.appendChild(createSingleDataElementRow("File Type:",this.tableDataElementFileType));
}

function createSingleDataElementRow(headerText, dataElement)
{
	dataElement.innerHTML = "Nothing Set";
	var tableRow = document.createElement("tr");
	var tableStart = document.createElement("td");
	tableStart.innerHTML = headerText;
	tableRow.appendChild(tableStart);
	tableRow.appendChild(dataElement);
	return tableRow;
}