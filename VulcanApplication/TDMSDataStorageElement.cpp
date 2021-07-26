#include "TDMSDataStorageElement.h"
#include "TDMSCodex.h"

TDMSDataStorageElement::TDMSDataStorageElement(int systemId, CodexBase * cdx)
	: PrimaryDataStorageElement(systemId,cdx)
{
	numberOfSegments = 0;
	segmentsComposingChannel = nullptr;
}

void TDMSDataStorageElement::addSegment(tdmsSegment * segment)
{
	numberOfSegments++;
	tdmsSegment ** newList = (tdmsSegment **)malloc(sizeof(tdmsSegment *)*numberOfSegments);

	for (int i = 0; i < numberOfSegments - 1; i++)
	{
		newList[i] = segmentsComposingChannel[i];
	}
	
	newList[numberOfSegments - 1] = segment;

	free(segmentsComposingChannel);

	segmentsComposingChannel = newList;
}

