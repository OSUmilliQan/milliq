#include <stdlib.h>
#include <CAENDigitizer.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#include "DAQ.h"

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

using namespace std;

bool ConnectionActive = false;
pthread_mutex_t ConnectionStatusMutex;

bool StopAcquisition = false;						// Controls when run is stopped
pthread_mutex_t AcquisitionStatusMutex; // Locks access to StopAcquisition while it's being checked

EventNode * masterHead = NULL;          // pointers to keep track of the master queue
EventNode * masterTail = NULL;          // used to move events between threads

pthread_mutex_t MasterQueueMutex;				// locks control of the master event queue

unsigned int eventsInQueue = 0;
unsigned int recordedEvents = 0;

void * acquisitionLoop(void*) {

	printf("Acquisition loop started...\n");

	RunParameters params;
	params.TriggerDelay = 36;
	params.TriggerType = SYSTEM_TRIGGER_NORMAL;
	params.SamplingFrequency = CAEN_DGTZ_SAM_3_2GHz;
	for (int i = 0; i < nChannels; i++) params.EnablePulseChannels[i] = false;
	params.PulsePattern = 0x01;
	for (int i = 0; i < nChannels; i++) {
		if (i == 1) params.ChannelTriggerEnable[i] = true;
		else params.ChannelTriggerEnable[i] = false;

		params.TriggerThresholds[i] = 0.010;
		params.TriggerPolarities[i] = CAEN_DGTZ_TriggerOnRisingEdge;
		params.DCOffsets[i] = 0.000;
	}
	params.inlCorrect = true;
	params.pedestalCorrect = true;
	params.MaxNumEventsBLT = 100;

	// charge mode
	params.ChargeMode = true;

	DAQ * acq = new DAQ();

	acq->ConnectToBoard();
	pthread_mutex_lock(&ConnectionStatusMutex);
	ConnectionActive = true;
	pthread_mutex_unlock(&ConnectionStatusMutex);

	acq->InitializeBoardParameters(params);
	acq->MallocReadoutBuffer();
	acq->SetChannelEnableMask();

	// test durp
	acq->SetChargeMode();

	acq->StartRun();

	Sleep(1000); // [ms]

	EventNode* localHead = NULL;
	EventNode* localTail = NULL;
	pthread_mutex_lock(&AcquisitionStatusMutex);

	int statusCode = -1;
	int nEventsRead = 0;

	while (!StopAcquisition) {
		pthread_mutex_unlock(&AcquisitionStatusMutex);

		acq->PrepareEvent();
		statusCode = acq->ProcessEvent(&localHead, &localTail, eventsInQueue);

		if (localHead != 0 && pthread_mutex_trylock(&MasterQueueMutex) == 0) {   //did we get control the master queue?
			if (masterHead == NULL) {
				masterHead = localHead;
				masterTail = localTail;
			}
			else {
				masterTail->nxt = localHead;
				localHead->prv = masterTail;
				masterTail = localTail;
			}
			pthread_mutex_unlock(&MasterQueueMutex);
			localHead = NULL;
			localTail = NULL;
		}
		/////////////////////////////////////////////////
		//end get and append
		////////////////////////////////////////////////
		pthread_mutex_lock(&AcquisitionStatusMutex);
	}
	pthread_mutex_unlock(&AcquisitionStatusMutex);

	acq->StopRun();
	acq->CloseDevice();

	delete acq;

	pthread_exit(NULL);

	return NULL;
}

void * eventProcessLoop(void*) {

	printf("Process thread has begun\n");

	EventNode * localHead;
	EventNode * localTail;

	TFile * outputFile = new TFile("events.root", "RECREATE");

	unsigned short _TriggerCount; // uint16_t
	unsigned short _TimeCount; // uint16_t
	unsigned char _EventId; // uint8_t
	unsigned long long _TDC; // uint64_t

	float _waveMax;
	float _waveMin;

	vector<TTree*> trees;
	for (int i = 0; i < nChannels; i++) {
		TString name = "Channel_";
		name += Form("%d", i);

		TTree * tree = new TTree(name, "CAEN Events");
		tree->SetAutoSave(10000000); // 10 MB

		tree->Branch("TriggerCount", &_TriggerCount, "_TriggerCount/s");
		tree->Branch("TimeCount", &_TimeCount, "_TimeCount/s");
		tree->Branch("EventId", &_EventId, "_EventId/b");
		tree->Branch("TDC", &_TDC, "_TDC/l");

		tree->Branch("Max", &_waveMax, "_waveMax/F");
		tree->Branch("Min", &_waveMin, "_waveMin/F");

		trees.push_back(tree);
	}

	pthread_mutex_lock(&AcquisitionStatusMutex);
	while (!StopAcquisition) {
		pthread_mutex_unlock(&AcquisitionStatusMutex);
		pthread_mutex_lock(&MasterQueueMutex);

		localHead = masterHead;
		localTail = masterTail;
		masterHead = NULL;
		masterTail = NULL;

		pthread_mutex_unlock(&MasterQueueMutex);

		while (localHead != NULL) {

			freeEvent(localHead->prv);
			//eventCountGlobal++;

			for (int i = 0; i < nChannels; i++) {

				int samIndex = (int)(i / 2);
				int chanIndex = i % 2;

				_TriggerCount = localHead->TriggerCount[samIndex][chanIndex];
				_TimeCount = localHead->TimeCount[samIndex][chanIndex];
				_EventId = localHead->EventId[samIndex];
				_TDC = localHead->TDC[samIndex];

				float _max = -1.e6;
				float _min = 1.e6;

				for (int j = 0; j < 1024; j++) {
					if (localHead->Waveform[i][j] > _max) _max = localHead->Waveform[i][j];
					if (localHead->Waveform[i][j] < _min) _min = localHead->Waveform[i][j];
				}

				_waveMax = _max;
				_waveMin = _min;

				trees[i]->Fill();
			}

			localHead = localHead->nxt;
			eventsInQueue--;
			recordedEvents++;
		}

		freeEvent(localTail);

		pthread_mutex_lock(&AcquisitionStatusMutex);
	}
	pthread_mutex_unlock(&AcquisitionStatusMutex);

	for (int i = 0; i < nChannels; i++) trees[i]->Write();

	outputFile->Close();

	printf("All events processed \n");
	pthread_exit(NULL);

	return NULL;
}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: %s <seconds>\n", argv[0]);
		return(0);
	}

	printf("\n\n");

	pthread_t readThread;
	pthread_t processThread;

	pthread_attr_t joinableAttr;
	pthread_attr_init(&joinableAttr);

	pthread_attr_setdetachstate(&joinableAttr, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&MasterQueueMutex, NULL);
	pthread_mutex_init(&AcquisitionStatusMutex, NULL);
	pthread_mutex_init(&ConnectionStatusMutex, NULL);

	pthread_create(&readThread, &joinableAttr, acquisitionLoop, NULL);
	pthread_create(&processThread, &joinableAttr, eventProcessLoop, NULL);

	pthread_mutex_lock(&ConnectionStatusMutex);
	while (!ConnectionActive) {
		pthread_mutex_unlock(&ConnectionStatusMutex);
		Sleep(1000);
		pthread_mutex_lock(&ConnectionStatusMutex);
	}
	pthread_mutex_unlock(&ConnectionStatusMutex);

	time_t initial, final;
	double elapsed = 0;
	double lastPrint = 0;
	time(&initial);

	double runLength = atoi(argv[1]);

	while (elapsed < runLength) {
		time(&final);
		if((int)elapsed % 5 == 0 && elapsed > lastPrint) { // every 5 seconds
			lastPrint = elapsed; // elapsed iterates in seconds, so this prevents many messages in the same second
			printf("Recorded: %d, in queue: %d\n", recordedEvents, eventsInQueue);
		}
		elapsed = difftime(final, initial);
	}

	pthread_mutex_lock(&AcquisitionStatusMutex);
	StopAcquisition = true;
	pthread_mutex_unlock(&AcquisitionStatusMutex);

	void* status;

	pthread_attr_destroy(&joinableAttr);

	int rc;
	rc = pthread_join(readThread, &status);
	rc = pthread_join(processThread, &status);

	pthread_mutex_destroy(&MasterQueueMutex);
	pthread_mutex_destroy(&AcquisitionStatusMutex);
	pthread_mutex_destroy(&ConnectionStatusMutex);
	//pthread_exit(NULL);
	printf("\nAcquisition complete!\n\n");

	return 0;
}
