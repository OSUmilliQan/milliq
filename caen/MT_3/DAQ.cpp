#ifndef DAQ_CPP
#define DAQ_CPP

#include "DAQ.h"

using namespace std;

bool useIRQ = true;

// CopyEvent translates the CAEN Event structure into the much simpler EventNode structure
// Once done, waveform samples can be accessed with EventNode->Waveform[channel][sample]
EventNode * CopyEvent(CAEN_DGTZ_X743_EVENT_t * evt) {

	EventNode * node = new EventNode();

	node->nxt = NULL;
	node->prv = NULL;

	float **obj = (float**)malloc(nChannels * sizeof(float*));

	for (int samIndex = 0; samIndex < nSamBlocks; samIndex++) {

		node->GrPresent[samIndex] = evt->GrPresent[samIndex];

		node->ChSize[samIndex] = evt->DataGroup[samIndex].ChSize;

		for (int i = 0; i < nChannelsPerSamBlock; i++) {
			node->TriggerCount[samIndex][i] = evt->DataGroup[samIndex].TriggerCount[i];
			node->TimeCount[samIndex][i] = evt->DataGroup[samIndex].TimeCount[i];

			size_t m = (size_t)node->ChSize[samIndex] * sizeof(float);
			obj[2 * samIndex + i] = (float*)malloc(m);
			memmove((void*)obj[nChannelsPerSamBlock * samIndex + i], (const void*)evt->DataGroup[samIndex].DataChannel[i], m);
		}

		node->EventId[samIndex] = evt->DataGroup[samIndex].EventId;
		node->StartIndexCell[samIndex] = evt->DataGroup[samIndex].StartIndexCell;
		node->TDC[samIndex] = evt->DataGroup[samIndex].TDC;
		node->PosEdgeTimeStamp[samIndex] = evt->DataGroup[samIndex].PosEdgeTimeStamp;
		node->NegEdgeTimeStamp[samIndex] = evt->DataGroup[samIndex].NegEdgeTimeStamp;
		node->PeakIndex[samIndex] = evt->DataGroup[samIndex].PeakIndex;
		node->Peak[samIndex] = evt->DataGroup[samIndex].Peak;
		node->Baseline[samIndex] = evt->DataGroup[samIndex].Baseline;
		node->Charge[samIndex] = evt->DataGroup[samIndex].Charge;

	}

	node->Waveform = obj;

	return node;

}

void freeArray(void** obj) {
	for (int i = 0; i < nChannels; i++) free(obj[i]);
	free(obj);
}

void freeEvent(EventNode * node) {
	if (node != NULL) {
		freeArray((void**)node->Waveform);
		free(node);
	}
}

void DAQ::ConnectToBoard() {

	CAEN_DGTZ_ErrorCode error = CAEN_DGTZ_Success;

	error = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCI_OpticalLink,
		0,
		0,
		(useIRQ) ? 0 : 0x32100000,
		&DeviceHandle);

	if (error != CAEN_DGTZ_Success) {
		cout << "Failed to open digitizer: error %d" << error << endl;
		exit(0);
	}

	CAEN_DGTZ_BoardInfo_t info;
	error = CAEN_DGTZ_GetInfo(DeviceHandle, &info);
	if (error != CAEN_DGTZ_Success) {
		printf("Failed to get digitizer info: error %d \n", error);
		exit(0);
	}

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

	if (error != CAEN_DGTZ_Success) exit(0);

}

void DAQ::InitializeBoardParameters(RunParameters params) {

	SetTriggerDelay(params.TriggerDelay);
	SetSamplingFrequency(params.SamplingFrequency);
	SetPulserParameters(params.EnablePulseChannels, params.PulsePattern);

	for (int i = 0; i < nChannels; i++) SetTriggerThreshold(params.TriggerThresholds, i);

	SetTriggerSource(params.TriggerType, params.ChannelTriggerEnable);

	for (int i = 0; i < nChannels; i++) SetTriggerPolarity(params.TriggerPolarities, i);

	for (int i = 0; i < nChannels; i++) SetChannelDCOffset(params.DCOffsets, i);

	SetCorrectionLevel(params.inlCorrect, params.pedestalCorrect);

	// set irq interrupt stuff
	if (useIRQ) {
		CAEN_DGTZ_SetInterruptConfig(DeviceHandle,
			CAEN_DGTZ_ENABLE,
			1, // level = 1 for optical link
			0, // status_id meaningless for optical link
			6, // send interrupt when board has 6+ events (should be 7?)
			CAEN_DGTZ_IRQ_MODE_RORA);
	}


	SetMaxNumEventsBLT(params.MaxNumEventsBLT);
	//CAEN_DGTZ_SetAcquisitionMode(DeviceHandle, CAEN_DGTZ_SW_CONTROLLED);
}

void DAQ::InitializeBoardParameters_ChargeMode(RunParameters params) {

	DigitizerParams_t DigiParams;

	DigiParams.LinkType = CAEN_DGTZ_PCI_OpticalLink;
	DigiParams.VMEBaseAddress = 0;
	DigiParams.AcqMode = CAEN_DGTZ_ACQ_MODE_List;
	DigiParams.RecordLength = 1024;
	DigiParams.ChannelMask = 0xFF;
	DigiParams.EventAggr = 0;
	DigiParams.PulsePolarity = CAEN_DGTZ_PulsePolarityPositive;




}

void DAQ::MallocReadoutBuffer() {
	CAEN_DGTZ_MallocReadoutBuffer(DeviceHandle, (char**)&ReadoutBuffer, &ReadoutBufferSize);
}

void DAQ::MallocReadoutBuffer_ChargeMode() {
	CAEN_DGTZ_Malloc
}

void DAQ::SetChannelEnableMask() {
	CAEN_DGTZ_SetChannelEnableMask(DeviceHandle, 0xFF); // 0xFF = all channels enabled
}

void DAQ::SetChargeMode() {
	CAEN_DGTZ_SetSAMAcquisitionMode(DeviceHandle, CAEN_DGTZ_AcquisitionMode_DPP_CI);
}

void DAQ::StartRun() {

	CAEN_DGTZ_AllocateEvent(DeviceHandle, (void**)&CurrentEvent);
	CAEN_DGTZ_SWStartAcquisition(DeviceHandle);

}

void DAQ::StopRun() {

	uint32_t nReadBytes = 1;
	int error = 0;

	CAEN_DGTZ_SWStopAcquisition(DeviceHandle);

	// continue reading data from the board until it's empty
	printf("Clearing data...\n");
	while (error == 0 && nReadBytes > 0) {
		error = CAEN_DGTZ_ReadData(DeviceHandle,
			CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
			(char*)ReadoutBuffer,
			&nReadBytes);
	}
	CAEN_DGTZ_FreeEvent(DeviceHandle, (void**)&CurrentEvent);

}

void DAQ::PrepareEvent() {

	// if you shoud be sending software triggers, send one now
	if (TriggerType == SYSTEM_TRIGGER_AUTO || TriggerType == SYSTEM_TRIGGER_SOFT) CAEN_DGTZ_SendSWtrigger(DeviceHandle);

}

// Read an event from the board and store them in ReadoutBuffer and ReadoutBufferSize
int DAQ::ReadEventBuffer() {

	uint32_t nReadBytes = 0;

	int error = CAEN_DGTZ_IRQWait(DeviceHandle, 100); // timeout in ms
	if (error != CAEN_DGTZ_Success) return -1;

	error = CAEN_DGTZ_ReadData(DeviceHandle,
		//CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
		//CAEN_DGTZ_SLAVE_TERMINATED_READOUT_2eSST, // durp??? fastest mode, doesn't work
		CAEN_DGTZ_POLLING_MBLT,
		(char*)ReadoutBuffer,
		&nReadBytes);

	ReadoutBufferSize = nReadBytes;

	if (error < 0) cout << "Error Code in function CAEN_DGTZ_ReadData: " << error << endl;

	return (nReadBytes > 0 && error == CAEN_DGTZ_Success) ? 0 : -1;

}

int DAQ::GetNumberOfEvents() {

	uint32_t numEvents;
	CAEN_DGTZ_GetNumEvents(DeviceHandle, (char*)ReadoutBuffer, ReadoutBufferSize, &numEvents);
	return numEvents;

}

// Read the event in ReadoutBuffer and decode it into CurrentEvent
int DAQ::DecodeEvent(int eventNumber) {

	CAEN_DGTZ_EventInfo_t eventInfo;
	char * eventPtr = NULL;

	CAEN_DGTZ_ErrorCode error;
	int result = 0;

	error = CAEN_DGTZ_GetEventInfo(DeviceHandle,
		(char*)ReadoutBuffer,
		ReadoutBufferSize,
		eventNumber,
		&eventInfo,
		&eventPtr);

	if (error == CAEN_DGTZ_Success) {
		error = CAEN_DGTZ_DecodeEvent(DeviceHandle, eventPtr, (void**)&CurrentEvent);
		if (error < 0) {
			cout << "Error Code in function CAEN_DGTZ_DecodeEvent: " << error << endl;
			result = -1;
		}
	}
	else {
		cout << "Error Code in function CAEN_DGTZ_GetEventInfo: " << error << endl;
		result = -1;
	}

	return result;
}

void DAQ::CloseDevice() {

	StopRun();
	CAEN_DGTZ_FreeReadoutBuffer((char**)&ReadoutBuffer);
	CAEN_DGTZ_CloseDigitizer(DeviceHandle);

}

// Read the event buffer, and for every event read create an EventNode
// Then insert the EventNode into the master queue
int DAQ::ProcessEvent(EventNode** head, EventNode** tail, unsigned int& queueCount) {

	int status = 0;
	int nEventsRead = 0;

	status = ReadEventBuffer();
	if (status < 0) return status;

	nEventsRead = GetNumberOfEvents();
	queueCount += nEventsRead;

	for (int i = 0; i < nEventsRead; i++) {
		status = DecodeEvent(i);
		if (status < 0) return status;

		EventNode * newEvent = CopyEvent(CurrentEvent);

		if (*head == NULL) *head = newEvent;
		if (*tail != NULL) {
			(*tail)->nxt = newEvent;
			newEvent->prv = *tail;
		}
		*tail = newEvent;

	}

	return status;

}

// =================================

void DAQ::SetTriggerDelay(int TriggerDelay) {

	for (int i = 0; i < nSamBlocks; i++) {
		CAEN_DGTZ_SetSAMPostTriggerSize(DeviceHandle, i, TriggerDelay & 0xFF);
	}

}

void DAQ::SetSamplingFrequency(CAEN_DGTZ_SAMFrequency_t SamplingFrequency) {
	CAEN_DGTZ_SetSAMSamplingFrequency(DeviceHandle, SamplingFrequency);
}

void DAQ::SetPulserParameters(bool * EnablePulseChannels, unsigned int PulsePattern) {

	for (int i = 0; i < nChannels; i++) {
		if (EnablePulseChannels[i]) CAEN_DGTZ_EnableSAMPulseGen(DeviceHandle, i, PulsePattern, CAEN_DGTZ_SAMPulseCont);
		else CAEN_DGTZ_DisableSAMPulseGen(DeviceHandle, i);
	}

}

void DAQ::SetTriggerThreshold(double * TriggerThreshold, int channel) {

	CAEN_DGTZ_SetChannelTriggerThreshold(DeviceHandle,
		channel,
		DACValue(TriggerThreshold[channel]));

}

void DAQ::SetTriggerPolarity(CAEN_DGTZ_TriggerPolarity_t * TriggerPolarity, int channel) {

	CAEN_DGTZ_SetTriggerPolarity(DeviceHandle, channel, TriggerPolarity[channel]);

}

void DAQ::SetTriggerSource(TriggerType_t trigType, bool * ChannelTriggerEnable) {

	TriggerType = trigType;

	if (TriggerType == SYSTEM_TRIGGER_SOFT) {
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
	}
	else if (TriggerType == SYSTEM_TRIGGER_NORMAL) {

		int channelsMask = 0;
		for (int i = 0; i < nChannels / 2; i++) channelsMask += (ChannelTriggerEnable[i * 2] + ((ChannelTriggerEnable[i * 2 + 1]) << 1)) << (2 * i);
		channelsMask = (~channelsMask) & 0xFF;

		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
	}
	else if (TriggerType == SYSTEM_TRIGGER_AUTO) {

		int channelsMask = 0;
		for (int i = 0; i < nChannels / 2; i++) channelsMask += (ChannelTriggerEnable[i * 2] + ((ChannelTriggerEnable[i * 2 + 1]) << 1)) << (2 * i);
		channelsMask = (~channelsMask) & 0xFF;

		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	}
	else { // EXTERNAL ONLY
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	}

}

void DAQ::SetChannelDCOffset(double * DCOffset, int channel) {

	CAEN_DGTZ_SetChannelDCOffset(DeviceHandle, channel, DACValue(DCOffset[channel]));

}

void DAQ::SetCorrectionLevel(bool inlCorrect, bool pedestalCorrect) {

	if (inlCorrect && pedestalCorrect) CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, CAEN_DGTZ_SAM_CORRECTION_ALL);
	else if (inlCorrect && !pedestalCorrect) CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, CAEN_DGTZ_SAM_CORRECTION_INL);
	else if (!inlCorrect && pedestalCorrect) CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, CAEN_DGTZ_SAM_CORRECTION_PEDESTAL_ONLY);
	else CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, CAEN_DGTZ_SAM_CORRECTION_DISABLED);

}

void DAQ::SetMaxNumEventsBLT(int MaxNumEventsBLT) {
	CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle, MaxNumEventsBLT);
}

#endif
