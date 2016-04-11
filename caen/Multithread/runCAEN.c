#include "runCAEN.h"

#define nChannels  16   //number of active channels
#define channelMask 0xFFFF //mask enabling triggers
#define groups  8       //number of active groups
#define record 4096 	//number of samples per record
#define ptrigger  100	//percent of window post trigger
#define DCOffset 0x7FFF //hex value DC offset
#define blockEvents 3 //max number of events transmitted per block

void setupCAEN(int* handle, char** buffer, int thresh) {

	uint32_t size;
	CAEN_DGTZ_ErrorCode error;
	int i;
	error = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCI_OpticalLink, 0, 0, 0x32100000, handle);
	if (error != CAEN_DGTZ_Success) {
		printf("Failed to open digitizer: error %d \n", error);
		exit(0);
	}

	CAEN_DGTZ_BoardInfo_t info;
	CAEN_DGTZ_GetInfo(*handle, &info);
	printf("Digitizer opened\n");
	printf("Getting Info From Digitizer:\n");
	printf("Model Name:              %s\n", info.ModelName);
	printf("Model:                   %d\n", info.Model);
	printf("Channels:                %d\n", info.Channels);
	printf("Form Factor:             %d\n", info.FormFactor);
	printf("Family Code:             %d\n", info.FamilyCode);
	printf("ROC_FirmwareRel:         %s\n", info.ROC_FirmwareRel);
	printf("AMC_FirmwareRel:         %s\n", info.AMC_FirmwareRel);
	printf("SerialNumber:            %d\n", info.SerialNumber);
	printf("PCB_Revision:            %d\n", info.PCB_Revision);
	printf("ADC_NBits:               %d\n", info.ADC_NBits);
	printf("SAMCorrectionDataLoaded: %d\n", info.SAMCorrectionDataLoaded);
	printf("CommHandle:              %d\n", info.CommHandle);
	printf("License:                 %s\n", info.License);

	CAEN_DGTZ_Reset(*handle);

	CAEN_DGTZ_SetRecordLength(*handle, record);

	/*
	// durp 1743 stuff?
	CAEN_DGTZ_SetSAMCorrectionLevel(*handle, CAEN_DGTZ_SAM_CORRECTION_ALL);
	for (int i = 0; i < groups; i++) CAEN_DGTZ_SetSAMPostTriggerSize(*handle, i, 1);
	CAEN_DGTZ_SetSAMSamplingFrequency(*handle, CAEN_DGTZ_SAM_3_2GHz);
	CAEN_DGTZ_LoadSAMCorrectionData(*handle); // takes a long time
	CAEN_DGTZ_SetSAMAcquisitionMode(*handle, CAEN_DGTZ_AcquisitionMode_STANDARD);
	*/

	CAEN_DGTZ_SetGroupEnableMask(*handle, 0x1); // turns on groups (255 = 0xFF)
	//CAEN_DGTZ_SetChannelEnableMask(*handle, 0x2); // turns on channels

	//CAEN_DGTZ_SetPostTriggerSize(*handle, ptrigger);

	//CAEN_DGTZ_SetIOLevel(*handle, CAEN_DGTZ_IOLevel_TTL);
	//CAEN_DGTZ_SetExtTriggerInputMode(*handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

	uint32_t trig_thresh = 0x7F0F; // +9.2 mV?
	//trig_thresh = 32768;
	CAEN_DGTZ_SetChannelSelfTrigger(*handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0x2); // previously channelMask
	CAEN_DGTZ_SetChannelTriggerThreshold(*handle, 1, trig_thresh);

	//CAEN_DGTZ_SetGroupSelfTrigger(*handle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0x1);
	//CAEN_DGTZ_SetGroupTriggerThreshold(*handle, 0, trig_thresh);

	CAEN_DGTZ_SetTriggerLogic(*handle, CAEN_DGTZ_LOGIC_OR, 0);
	CAEN_DGTZ_SetChannelPairTriggerLogic(*handle, 0, 1, CAEN_DGTZ_LOGIC_OR, 30);
	CAEN_DGTZ_SetTriggerPolarity(*handle, 1, CAEN_DGTZ_TriggerOnRisingEdge);
	CAEN_DGTZ_SetChannelDCOffset(*handle, 1, DCOffset);

	CAEN_DGTZ_SetExtTriggerInputMode(*handle, CAEN_DGTZ_TRGMODE_DISABLED);
	CAEN_DGTZ_SetSWTriggerMode(*handle, CAEN_DGTZ_TRGMODE_DISABLED);

	CAEN_DGTZ_SetMaxNumEventsBLT(*handle, blockEvents);
	CAEN_DGTZ_SetAcquisitionMode(*handle, CAEN_DGTZ_SW_CONTROLLED);

	CAEN_DGTZ_ClearData(*handle);
	error = CAEN_DGTZ_MallocReadoutBuffer(*handle, buffer, &size);
	if (error != CAEN_DGTZ_Success) {
		printf("Failed to allocate readout buffer\n");
		printf("%d\n", error);
		exit(0);
	}
	printf("Readout buffer allocated\n");
}

void startCAEN(int handle) {//Begin acquisition
	CAEN_DGTZ_SWStartAcquisition(handle);
	//CAEN_DGTZ_ClearData(handle); // if you do this, board seems never to trigger
}

void stopCAEN(int handle) {//Stop acquisition
	CAEN_DGTZ_ClearData(handle);
	CAEN_DGTZ_SWStopAcquisition(handle);
}

void closeCAEN(int handle, char** buffer) {//close the connection to the digitizer
	CAEN_DGTZ_ClearData(handle);
	int error = CAEN_DGTZ_CloseDigitizer(handle);
	if (error != 0) {
		printf("Unable to close digitizer, error %d\n", error);
	}
	else {
		printf("Digitizer closed\n");
	}

	CAEN_DGTZ_FreeReadoutBuffer(buffer);
}

/*Takes the head and tail of a linked list of eventNodes and appends all the
events currently in the digitizers buffers to the list*/
int getFromCAEN(int handle, char* buffer, eventNode** head, eventNode** tail) {

	uint32_t bufferSize, numEvents;
	int i;
	char* eventPointer;
	//CAEN_DGTZ_UINT16_EVENT_t* Evt = NULL;
	CAEN_DGTZ_X743_EVENT_t* Evt = NULL;
	CAEN_DGTZ_AllocateEvent(handle, (void**)&Evt);
	int error;
	CAEN_DGTZ_EventInfo_t eventInfo;

	//printf("Sending SW Trigger...\n");
	//CAEN_DGTZ_SendSWtrigger(handle);

	error = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &bufferSize); //reads raw data stream from CAEN
	//error = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_POLLING_MBLT, buffer, &bufferSize);
	if (error != 0) {
		printf("Failed to read from CAEN, error %d\n", error);
	}

	numEvents = 0;
	if (bufferSize != 0) {
		error = CAEN_DGTZ_GetNumEvents(handle, buffer, bufferSize, &numEvents);			//determine number of events in data stream buffer
	}
	printf("Retrieved %d events.\n", numEvents);

	for (i = 0; i < (int)numEvents; i++) {

		error = CAEN_DGTZ_GetEventInfo(handle, buffer, bufferSize, i, &eventInfo, &eventPointer);		//get event info and pointer to event data
		error = CAEN_DGTZ_DecodeEvent(handle, eventPointer, (void**)&Evt);			//get event data structure from data pointer
		//CAEN_DGTZ_DecodeEvent(handle, eventPointer, &Evt);

		eventNode* newEvent = malloc(sizeof(eventNode));
		newEvent->event = allocateAndCopy(Evt);
		newEvent->channels = nChannels;
		newEvent->samples = record;
		newEvent->nxt = NULL;
		newEvent->prv = NULL;

		if (*head == NULL) {
			*head = newEvent;
		}
		if (*tail != NULL) {
			(*tail)->nxt = newEvent;
			newEvent->prv = *tail;
		}
		*tail = newEvent;
	}
	CAEN_DGTZ_FreeEvent(handle, (void**)&Evt);
	//CAEN_DGTZ_ClearData(handle);
	return (int)numEvents;
}
