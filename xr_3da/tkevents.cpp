/****************************************************************************************/
/*  TKARRAY.C																			*/
/*                                                                                      */
/*  Author: Stephen Balkum	                                                            */
/*  Description: Time-keyed events implementation.										*/
/*                                                                                      */
/*  The contents of this file are subject to the Genesis3D Public License               */
/*  Version 1.01 (the "License"); you may not use this file except in                   */
/*  compliance with the License. You may obtain a copy of the License at                */
/*  http://www.genesis3d.com                                                            */
/*                                                                                      */
/*  Software distributed under the License is distributed on an "AS IS"                 */
/*  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See                */
/*  the License for the specific language governing rights and limitations              */
/*  under the License.                                                                  */
/*                                                                                      */
/*  The Original Code is Genesis3D, released March 25, 1999.                            */
/*Genesis3D Version 1.1 released November 15, 1999                            */
/*  Copyright (C) 1999 WildTangent, Inc. All Rights Reserved           */
/*                                                                                      */
/****************************************************************************************/
/* geTKEvents
	(Time-Keyed-Events)

	geTKEvents is a sorted array of times with an identifying descriptor.
	The descriptors are stored as strings in a separate, packed buffer.

	Error conditions are reported to errorlog
*/
#include	"stdafx.h"

#include "TKEvents.h"
#include "TKArray.h"
#include "string.h"

typedef struct
{
	geTKEvents_TimeType EventTime;
	uint32 DataOffset;
}	EventType;

typedef struct geTKEventsIterator
{
	geTKEvents_TimeType EndTime;
	int CurrentIndex;
}	geTKEventsIterator;

typedef struct geTKEvents
{
	geTKArray* pTimeKeys;
	uint32 DataSize;
	char* pEventData;

	geTKEventsIterator Iterator;
}	geTKEvents;



// General validity test.
// Use TKE_ASSERT_VALID to test array for reasonable data.
#ifdef _DEBUG

#define TKE_ASSERT_VALID(E) geTKEvents_Asserts(E)

// Do not call this function directly.  Use TKE_ASSERT_VALID
static void GENESISCC geTKEvents_Asserts(const geTKEvents* E)
{
	VERIFY( (E) != NULL );
	VERIFY( (E)->pTimeKeys != NULL );
	if(geTKArray_NumElements((E)->pTimeKeys) == 0)
	{
		VERIFY( (E)->pEventData == NULL );
	}
	else
	{
		VERIFY( (E)->pEventData != NULL );
	}
}

#else // !_DEBUG

#define TKE_ASSERT_VALID(E) ((void)0)

#endif // _DEBUG

geTKEvents* GENESISCC geTKEvents_Create(void)
	// Creates a new event array.
{
	geTKEvents* pEvents;

	pEvents = (geTKEvents*)malloc(sizeof(geTKEvents));
	if(!pEvents)
	{
		//geErrorLog_Add(ERR_TKEVENTS_CREATE_ENOMEM, NULL);
		return NULL;
	}

	pEvents->pTimeKeys = geTKArray_Create(sizeof(EventType));
	if(!pEvents->pTimeKeys)
	{
		//geErrorLog_Add(ERR_TKEVENTS_CREATE_ENOMEM, NULL);
		_FREE(pEvents);
		return NULL;
	}

	pEvents->DataSize = 0;
	pEvents->pEventData = NULL;

	pEvents->Iterator.CurrentIndex = 0;
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...

	return pEvents;
}


void GENESISCC geTKEvents_Destroy(geTKEvents** ppEvents)
	// Destroys array.
{
	geTKEvents* pE;

	VERIFY(ppEvents);
	pE = *ppEvents;
	VERIFY(pE);

	if( pE->pEventData != NULL )
		{
			_FREE(pE->pEventData);
		}

	if (pE->pTimeKeys != NULL)
		{
			geTKArray_Destroy(&pE->pTimeKeys);
		}
	_FREE(*ppEvents);
	*ppEvents = NULL;
}


BOOL GENESISCC geTKEvents_Insert(geTKEvents* pEvents, geTKEvents_TimeType tKey, const char* pEventData)
{
	int nIndex;
	uint32 DataLength;
	uint32 InitialOffset;
	int nNumElements;
	EventType* pKeyInfo;
	char* pNewData;

	TKE_ASSERT_VALID(pEvents);

	if( geTKArray_Insert(&pEvents->pTimeKeys, tKey, &nIndex) != TRUE )
	{
		//geErrorLog_Add(ERR_TKEVENTS_INSERT, NULL);
		return FALSE;
	}
	pKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex);
	VERIFY( pKeyInfo != NULL ); // I just successfully added it; it better be there.

	DataLength = strlen(pEventData) + 1;

	// Resize data to add new stuff
	pNewData = (char*)realloc(pEvents->pEventData, pEvents->DataSize + DataLength);
	if(!pNewData)
	{
		//geErrorLog_Add(ERR_TKEVENTS_INSERT_ENOMEM, NULL);
		if( geTKArray_DeleteElement(&pEvents->pTimeKeys, nIndex) == FALSE)
		{
			// This object is now in an unstable state.
			VERIFY(0);
		}
		// invalidate the iterator
		pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...
		return FALSE;
	}
	pEvents->pEventData = pNewData;

	// Find where new data will go
	nNumElements = geTKArray_NumElements(pEvents->pTimeKeys);
	VERIFY(nIndex < nNumElements); // sanity check
	if(nIndex == nNumElements - 1)
	{
		// We were added to the end
		InitialOffset = pEvents->DataSize;
	}
	else
	{
		EventType* pNextKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex + 1);
		VERIFY( pNextKeyInfo != NULL );

		InitialOffset = pNextKeyInfo->DataOffset;
	}
	pKeyInfo->DataOffset = InitialOffset;

	// Add new data, moving only if necessary
	if(InitialOffset < pEvents->DataSize)
	{
		memmove(pEvents->pEventData + InitialOffset + DataLength,	// dest
				pEvents->pEventData + InitialOffset,				// src
				pEvents->DataSize - InitialOffset);					// count
	}
	memcpy(	pEvents->pEventData + InitialOffset,	// dest
			pEventData,								// src
			DataLength);							// count

	pEvents->DataSize += DataLength;

	// Bump all remaining offsets up
	nIndex++;
	while(nIndex < nNumElements)
	{
		pKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex);
		VERIFY( pKeyInfo != NULL );
		pKeyInfo->DataOffset += DataLength;

		nIndex++;
	}

	// invalidate the iterator
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...

	return TRUE;
}


BOOL GENESISCC geTKEvents_Delete(geTKEvents* pEvents, geTKEvents_TimeType tKey)
{
	int nIndex, Count;
	geTKEvents_TimeType tFound;
	EventType* pKeyInfo;
	int DataOffset, DataSize;
	char *pNewData;

	TKE_ASSERT_VALID(pEvents);

	nIndex = geTKArray_BSearch(pEvents->pTimeKeys, tKey);

	if( nIndex < 0 )
	{	// no key wasn't found
		//geErrorLog_Add(ERR_TKEVENTS_DELETE_NOT_FOUND, NULL);
		return FALSE;
	}

	tFound = geTKArray_ElementTime(pEvents->pTimeKeys, nIndex);
	if(tFound < tKey - GE_TKA_TIME_TOLERANCE)
	{
		// key not found
		//geErrorLog_Add(ERR_TKEVENTS_DELETE_NOT_FOUND, NULL);
		return FALSE;
	}

	pKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex);
	DataOffset = pKeyInfo->DataOffset;
	if(nIndex < geTKArray_NumElements(pEvents->pTimeKeys) - 1)
	{
		// not the last element
		pKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex + 1);
		DataSize = pKeyInfo->DataOffset - DataOffset;

		memmove(pEvents->pEventData + DataOffset,				// dest
				pEvents->pEventData + DataOffset + DataSize,	// src
				pEvents->DataSize - DataOffset - DataSize);		// count
	}
	else
	{
		// It's the last element and no memory needs to be moved
		DataSize = pEvents->DataSize - DataOffset;
	}

	// Adjust data
	pEvents->DataSize -= DataSize;
	if (pEvents->DataSize == 0)
	{
		_FREE (pEvents->pEventData);
		pEvents->pEventData = NULL;
	}
	else
	{
		pNewData = (char*)realloc(pEvents->pEventData, pEvents->DataSize);
		// If the reallocation failed, it doesn't really hurt.  However, it is a
		// sign of problems ahead.
		if(pNewData)
		{
			pEvents->pEventData = pNewData;
		}
	}

	// Finally, remove this element
	geTKArray_DeleteElement(&pEvents->pTimeKeys, nIndex);

	// Adjust the offsets
	Count = geTKArray_NumElements(pEvents->pTimeKeys);
	while(nIndex < Count)
	{
		pKeyInfo = (EventType *)geTKArray_Element(pEvents->pTimeKeys, nIndex);
		VERIFY( pKeyInfo != NULL );
		pKeyInfo->DataOffset -= DataSize;
		nIndex++;
	}

	// invalidate the iterator
	pEvents->Iterator.EndTime = -99e33f;	// you could sample here I suppose...

	return TRUE;
}


#define TKEVENTS_ASCII_FILE_TYPE 0x56454B54 // 'TKEV'
#define TKEVENTS_FILE_VERSION 0x00F0		// Restrict to 16 bits

#define TKEVENTS_BIN_FILE_TYPE   0x42454B54 // 'TKEB'
#define TKEVENTS_TIMEKEYS_ID "TimeKeys"
#define TKEVENTS_DATASIZE_ID "DataSize"


geTKEvents* GENESISCC geTKEvents_CreateFromStream(
	CStream* pStream)					// stream positioned at array data
	// Creates a new array from the given stream.
{
	uint32 u;
	geTKEvents* pEvents;

	VERIFY( pStream != NULL );

	// Read the format/version flag
	pStream->Read(&u, sizeof(u));

	pEvents = (geTKEvents*)malloc(sizeof(geTKEvents));
	pEvents->pEventData = NULL;
	pEvents->pTimeKeys  = NULL;

	if(u == TKEVENTS_ASCII_FILE_TYPE)
	{
	}
	else
	{
		if(u == TKEVENTS_BIN_FILE_TYPE)
			{
				pStream->Read(&u, sizeof(u));
				if (u != TKEVENTS_FILE_VERSION)
					{
						//geErrorLog_AddString(-1,"Failure to recognize TKEvents file version", NULL);
						geTKEvents_Destroy(&pEvents);
						return FALSE;
					}

				pStream->Read(&(pEvents->DataSize), sizeof(pEvents->DataSize));

				pEvents->pEventData = (char*)malloc(pEvents->DataSize);
				if(!pEvents->pEventData)
					{
						//geErrorLog_Add(ERR_TKEVENTS_CREATE_ENOMEM, NULL);
						geTKEvents_Destroy(&pEvents);
						return NULL;
					}

				pStream->Read(pEvents->pEventData, pEvents->DataSize);
				pEvents->pTimeKeys = geTKArray_CreateFromStream(pStream);
				if(!pEvents->pTimeKeys)
					{
						//geErrorLog_Add(ERR_TKEVENTS_FILE_READ, NULL);
						geTKEvents_Destroy(&pEvents);
						return NULL;
					}
			}
	}

	return pEvents;
}

#define CHECK_FOR_WRITE(uu) if(uu <= 0) { geErrorLog_Add(ERR_TKEVENTS_FILE_WRITE, NULL); return FALSE; }

GENESISAPI BOOL GENESISCC geTKEvents_GetExtents(geTKEvents *Events,
		geTKEvents_TimeType *FirstEventTime,
		geTKEvents_TimeType *LastEventTime)
{
	int Count;
	VERIFY( Events != NULL );

	Count = geTKArray_NumElements(Events->pTimeKeys);
	if (Count<0)
		{
			return FALSE;
		}

	*FirstEventTime = geTKArray_ElementTime(Events->pTimeKeys, 0);
	*LastEventTime  = geTKArray_ElementTime(Events->pTimeKeys, Count-1);
	return TRUE;
}

void GENESISCC geTKEvents_SetupIterator(
	geTKEvents* pEvents,				// Event list to iterate
	geTKEvents_TimeType StartTime,				// Inclusive search start
	geTKEvents_TimeType EndTime)				// Non-inclusive search stop
	// For searching or querying the array for events between two times
	// times are compaired [StartTime,EndTime), '[' is inclusive, ')' is
	// non-inclusive.  This prepares the PathGetNextEvent() function.
{
	geTKEventsIterator* pTKEI;

	VERIFY( pEvents != NULL );

	pTKEI = &pEvents->Iterator;

	pTKEI->EndTime = EndTime;

	// Initialize search with first index before StartTime
	pTKEI->CurrentIndex = geTKArray_BSearch(pEvents->pTimeKeys, StartTime - GE_TKA_TIME_TOLERANCE);
	while( (pTKEI->CurrentIndex > -1) &&
		(geTKArray_ElementTime(pEvents->pTimeKeys, pTKEI->CurrentIndex) >= StartTime - GE_TKA_TIME_TOLERANCE) )
	{
		pTKEI->CurrentIndex--;
	}
}


BOOL GENESISCC geTKEvents_GetNextEvent(
	geTKEvents* pEvents,				// Event list to iterate
	geTKEvents_TimeType *pTime,				// Return time, if found
	const char **ppEventString)		// Return data, if found
	// Iterates from StartTime to EndTime as setup in geTKEvents_CreateIterator()
	// and for each event between these times [StartTime,EndTime)
	// this function will return Time and EventString returned for that event
	// and the iterator will be positioned for the next search.  When there
	// are no more events in the range, this function will return NULL (Time
	// will be 0 and ppEventString will be empty).
{
	geTKEventsIterator* pTKEI;
	geTKArray* pTimeKeys;
	EventType* pKeyInfo;
	int Index;

	VERIFY(pEvents);
	VERIFY(pTime);
	VERIFY(ppEventString);

	pTKEI = &pEvents->Iterator;

	pTimeKeys = pEvents->pTimeKeys;

	pTKEI->CurrentIndex++;
	Index = pTKEI->CurrentIndex;
	if(Index < geTKArray_NumElements(pTimeKeys))
	{
		*pTime = geTKArray_ElementTime(pTimeKeys, Index);
		if(*pTime + GE_TKA_TIME_TOLERANCE < pTKEI->EndTime)
		{
			// Looks good.  Get the string and return.
			pKeyInfo = (EventType *)geTKArray_Element(pTimeKeys, Index);
			*ppEventString = pEvents->pEventData + pKeyInfo->DataOffset;
			return TRUE;
		}
	}

	// None found, clean up
	*pTime = 0.0f;
	*ppEventString = NULL;
	return FALSE;
}
