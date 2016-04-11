#include "runCAEN.h"

static void Connect(int* DeviceHandle) {

	CAEN_DGTZ_ErrorCode error;

	error = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCI_OpticalLink, 0, 0, 0x32100000, &DeviceHandle);
	if(error != CAEN_DGTZ_Success) {
		printf("Failed to open digitizer: error %d \n", error);
		exit(0);
	}

	CAEN_DGTZ_BoardInfo_t info;
	CAEN_DGTZ_GetInfo(DeviceHandle, &info);
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
	printf("\n\n");

}

static void InitializeBoard(int* DeviceHandle) {

	CAEN_DGTZ_SetChannelEnableMask(DeviceHandle, 0x0F);

	const int nChannels = 8;
	const int nSamBlocks = 4;

	// Set_TriggerDelay()
	int triggerDelay = 1; // number of divisions
	for(int i = 0; i < nSamBlocks; i++) CAEN_DGTZ_SetSAMPostTriggerSize(DeviceHandle, i, triggerDelay & 0xFF);

	// Set_SamplingFreqeuncy()
	CAEN_DGTZ_SetSAMSamplingFrequency(DeviceHandle, CAEN_DGTZ_SAM_3_2GHz);

	// Set_PulserParameters()
	for(int i = 0; i < nChannels; i++) {
		CAEN_DGTZ_DisableSAMPulseGen(DeviceHandle, i);
	}

	// Set_TriggerThreshold()
	double triggerThresholdVolts = 0.002;
	int triggerADC = DACValue(triggerThresholdVolts);
	for(int i = 0; i < nChannels; i++) {
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, i, triggerADC);
	}

	// Set_TriggerSource()
	int channelsMask = 0;
	int channelTrigEnable[nChannels] = {0, 1, 0, 0, 0, 0, 0, 0};
	for(int i = 0; i < nChannels/2; i++) {
		channelsMask += (channelTrigEnable[i*2] + ((channelTrigEnable[i*2 + 1]) << 1)) << (2*i);
	}
	channelsMask = (~channelsMask) & 0xFF;

	CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
	CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
  CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);

	// Set_TriggerPolarity()
	for(int i = 0; i < nChannels) {
		CAEN_DGTZ_SetTriggerPolarity(DeviceHandle, i, CAEN_DGTZ_TriggerOnRisingEdge);
	}

	// Set_ChannelDCOffset()
	double offsetVolts = 0.0;
	int offsetADC = DACValue(offsetVolts);
	for(int i = 0; i < nChannels; i++) {
		CAEN_DGTZ_SetChannelDCOffset(DeviceHandle, i, offsetADC);
	}

	// Set_CorrectionLevel()
	CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, CAEN_DGTZ_SAM_CORRECTION_ALL);

	CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle, 100);

}

static void CloseBoard(int* DeviceHandle, uint32_t* buffer) {

	CAEN_DGTZ_FreeReadoutBuffer((char**)&buffer);
	CAEN_DGTZ_CloseDigitizer(DeviceHandle);

}

static void StopRun(int* DeviceHandle, uint32_t* buffer, CAEN_DGTZ_X743_EVENT_t* evt) {

	int nReadBytes = 1;
	int error = 0;

	CAEN_DGTZ_SWStopAcquisition(DeviceHandle);

	while(error == 0 && nReadBytes > 0) {
		error = CAEN_DGTZ_ReadData(DeviceHandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, (char*)ReadoutBuffer, &nReadBytes);
	}

	CAEN_DGTZ_FreeEvent(DeviceHandle, (void**)&evt);

}

static void StartRun(int* DeviceHandle, CAEN_DGTZ_X743_EVENT_t* evt) {

	CAEN_DGTZ_AllocateEvent(DeviceHandle, (void**)&evt);
	CAEN_DGTZ_SWStartAcquisition(DeviceHandle);

}

static int ReadFromBoard(int* DeviceHandle, uint32_t* buffer, int* bufferSize) {

	int nReadBytes = 0;
	int error = CAEN_DGTZ_Success;
	char message[120];

	while(error == CAEN_DGTZ_Success && nReadBytes == 0) {
		error = CAEN_DGTZ_ReadData(DeviceHandle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, (char*)ReadoutBuffer, &nReadBytes);
	}

	bufferSize = nReadBytes;

	return (nReadBytes > 0 && error = 0) ? 0 : -1;

}

static void ParseData(int* DeviceHandle,
										  uint32_t* buffer,
										  int* bufferSize,
										  CAEN_DGTZ_X743_EVENT_t* evt,
									 	  eventNode** evtHead,
									    eventNode** evtTail) {

	CAEN_DGTZ_ErrorCode error = CAEN_DGTZ_Success;

	int nEvents = 0;
	CAEN_DGTZ_GetNumEvents(DeviceHandle, (char*)ReadoutBuffer, ReadoutBufferSize, &nEvents);
	printf("Retrieved %d events.\n", nEvents);

	for(int eventNumber = 0; eventNumber < nEvents; eventNumber++) {

		CAEN_DGTZ_EventInfo_t eventInfo;
		char * eventPtr = NULL;

		error = CAEN_DGTZ_GetEventInfo(DeviceHandle,
																	 (char*)ReadoutBuffer,
																	 ReadoutBufferSize,
																	 eventNumber,
																	 &eventInfo,
																	 &eventPtr);

	 if(error == CAEN_DGTZ_Success) {
		 error = CAEN_DGTZ_DecodeEvent(DeviceHandle, eventPtr, (void**)&evt);
		 if(error != CAEN_DGTZ_Success) {
			 printf("Error code in DecodeEvent : %d\n", error);
			 break;
		 }
	 }
	 else {
		 printf("Error code in GetEventInfo : %d\n", error);
		 break;
	 }

	 eventNode* newEvent = malloc(sizeof(eventNode));
	 newEvent->event = allocateAndCopy(evt);
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

}
