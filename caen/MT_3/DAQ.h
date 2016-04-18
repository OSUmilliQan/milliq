#ifndef DAQ_H
#define DAQ_H

#include <CAENDigitizer.h>
#include <stdbool.h>
#include <iostream>
#include <vector>
#include "TROOT.h"

#define nChannels            16
#define nSamBlocks           8
#define nChannelsPerSamBlock 2

#define MIN_DAC_RAW_VALUE -1.25
#define MAX_DAC_RAW_VALUE 1.25

using namespace std;

typedef struct EventNode EventNode;
struct EventNode {
	EventNode * nxt;
	EventNode * prv;

	uint8_t  GrPresent[nSamBlocks];       // if the group has data the value is 1 (else 0)

	uint32_t ChSize[nSamBlocks];          // number of samples stored in DataChannel array
	float** Waveform;
	uint16_t TriggerCount[nSamBlocks][nChannelsPerSamBlock];
	uint16_t TimeCount[nSamBlocks][nChannelsPerSamBlock];
	uint8_t EventId[nSamBlocks];
	uint16_t StartIndexCell[nSamBlocks];
	uint64_t TDC[nSamBlocks];
	float PosEdgeTimeStamp[nSamBlocks];
	float NegEdgeTimeStamp[nSamBlocks];
	uint16_t PeakIndex[nSamBlocks];
	float Peak[nSamBlocks];
	float Baseline[nSamBlocks];
	float Charge[nSamBlocks];

};

typedef enum {
	SYSTEM_TRIGGER_SOFT,
	SYSTEM_TRIGGER_NORMAL,
	SYSTEM_TRIGGER_AUTO,
	SYSTEM_TRIGGER_EXTERN
} TriggerType_t;

void freeEvent(EventNode*);

typedef struct RunParameters RunParameters;
struct RunParameters {

	int TriggerDelay;
	CAEN_DGTZ_SAMFrequency_t SamplingFrequency;

	bool EnablePulseChannels[nChannels];
	unsigned int PulsePattern;

	bool ChannelTriggerEnable[nChannels];

	double TriggerThresholds[nChannels];
	CAEN_DGTZ_TriggerPolarity_t TriggerPolarities[nChannels];

	TriggerType_t TriggerType;

	double DCOffsets[nChannels];

	bool inlCorrect;
	bool pedestalCorrect;

	int MaxNumEventsBLT;

	bool ChargeMode;


};

class DAQ {

public:
	DAQ() { DeviceHandle = -1; };
	virtual ~DAQ() { ; };

	void SetTriggerDelay(int TriggerDelay);
	void SetSamplingFrequency(CAEN_DGTZ_SAMFrequency_t SamplingFrequency);
	void SetPulserParameters(bool * EnablePulseChannels, unsigned int PulsePattern);
	void SetTriggerThreshold(double * TriggerThreshold, int channel);
	void SetTriggerPolarity(CAEN_DGTZ_TriggerPolarity_t * TriggerPolarity, int channel);
	void SetTriggerSource(TriggerType_t trigType, bool * ChannelTriggerEnable);
	void SetChannelDCOffset(double * DCOffset, int channel);
	void SetCorrectionLevel(bool inlCorrect, bool pedestalCorrect);
	void SetMaxNumEventsBLT(int MaxNumEventsBLT);

	void ConnectToBoard();
	void InitializeBoardParameters(RunParameters params);
	void InitializeBoardParameters_ChargeMode(RunParameters params);
	void MallocReadoutBuffer();
	void MallocReadoutBuffer_ChargeMode();
	void SetChannelEnableMask();

	void StartRun();
	void StopRun();

	void PrepareEvent();

	int ReadEventBuffer();
	int GetNumberOfEvents();
	int DecodeEvent(int eventNumber);

	void CloseDevice();

	int ProcessEvent(EventNode** head, EventNode** tail, unsigned int& queueCount);

private:
	int DACValue(double value) { return (int)((MAX_DAC_RAW_VALUE - value) / (MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 65535); };

	int DeviceHandle;
	int EventNumber;
	int TotalEventNumber;

	TriggerType_t TriggerType;

	uint32_t * ReadoutBuffer;
	uint32_t ReadoutBufferSize;

	CAEN_DGTZ_EventInfo_t eventInfo;
	CAEN_DGTZ_X743_EVENT_t * CurrentEvent;

	int MaxNumEvents = 100;

};

#endif
