#pragma once
#include "PrimaryDataStorageElement.h"
#include "TDMSDataValues.h"

class tdmsSegment;
class tdmsMainStructure;

class TDMSDataStorageElement : public PrimaryDataStorageElement
{
	//this is going to be identical to the data storage element with all of the needed information in the 
	//other format
private:


public:

	tdmsMainStructure * mainStructure;
	int numberOfSegments;
	tdmsSegment ** segmentsComposingChannel;
	TDMSDataStorageElement(int sysid, CodexBase * cdx);
	void addSegment(tdmsSegment*);
};