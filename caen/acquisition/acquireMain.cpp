#include <stdlib.h>
#include <CAENDigitizer.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#include "DAQ.h"
//#include "RunConfiguration.h"

#include "TFile.h"
#include "TTree.h"
#include "TString.h"

using namespace std;

bool RunActive = false;
pthread_mutex_t RunStatusMutex;

bool StopAcquisition = false;						// Controls when run is stopped
pthread_mutex_t AcquisitionStatusMutex; // Locks access to StopAcquisition while it's being checked

EventNode * masterHead = NULL;          // pointers to keep track of the master queue
EventNode * masterTail = NULL;          // used to move events between threads

pthread_mutex_t MasterQueueMutex;				// locks control of the master event queue

unsigned int eventsInQueue = 0;
unsigned int recordedEvents = 0;

bool useChargeMode;

uint32_t recordLength = 0;

void * acquisitionLoop(void*) {

	printf("Acquisition loop started...\n");

	int statusCode = -1;

	RunConfiguration cfg;
	statusCode = cfg.ParseConfigFile("default.xml");
	recordLength = cfg.RecordLength;

	if(statusCode != 0 || !cfg.CheckAllParametersSet()) {
		printf("\n\nInvalid configuration file!\n\n");
		return 0;
	}

	useChargeMode = cfg.useChargeMode;

	DAQ * acq = new DAQ();

	acq->ConnectToBoard(cfg);

	acq->InitializeBoardParameters(cfg);
	acq->MallocReadoutBuffer();

	pthread_mutex_lock(&RunStatusMutex);
	RunActive = true;
	pthread_mutex_unlock(&RunStatusMutex);

	acq->StartRun();

	EventNode* localHead = NULL;
	EventNode* localTail = NULL;

	pthread_mutex_lock(&AcquisitionStatusMutex);

	while (!StopAcquisition) {
		pthread_mutex_unlock(&AcquisitionStatusMutex);

		acq->PrepareEvent();

		if(useChargeMode) statusCode = acq->ProcessDPPEvent(&localHead, &localTail, eventsInQueue);
		else statusCode = acq->ProcessEvent(&localHead, &localTail, eventsInQueue);

		if (localHead != 0) {
			pthread_mutex_lock(&MasterQueueMutex);

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

	unsigned long long _StartIndexCell;
	float _Charge;

	vector<TTree*> trees;
	for (int i = 0; i < nChannels; i++) {
		TString name = "Channel_";
		name += Form("%d", i);

		TTree * tree = new TTree(name, "CAEN Events");
		tree->SetAutoSave(10000000); // 10 MB

		if (useChargeMode) {
			tree->Branch("StartIndexCell", &_StartIndexCell, "_StartIndexCell/l");
			tree->Branch("Charge", &_Charge, "_Charge/F");
		}
		else {
			tree->Branch("TriggerCount", &_TriggerCount, "_TriggerCount/s");
			tree->Branch("TimeCount", &_TimeCount, "_TimeCount/s");
			tree->Branch("EventId", &_EventId, "_EventId/b");
			tree->Branch("TDC", &_TDC, "_TDC/l");

			tree->Branch("Max", &_waveMax, "_waveMax/F");
			tree->Branch("Min", &_waveMin, "_waveMin/F");
		}

		trees.push_back(tree);
	}

	pthread_mutex_lock(&AcquisitionStatusMutex);
	while (!StopAcquisition) {
		pthread_mutex_unlock(&AcquisitionStatusMutex);

		if (pthread_mutex_trylock(&MasterQueueMutex) == 0) { // Did we get control of the master queue?

			localHead = masterHead;
			localTail = masterTail;
			masterHead = NULL;
			masterTail = NULL;

			pthread_mutex_unlock(&MasterQueueMutex);
		}
		else continue;

		while (localHead != NULL) {

			freeEvent(localHead->prv);

			if (useChargeMode) {

				for (int ch = 0; ch < nChannels; ch++) {

					for (int ev = 0; ev < localHead->NumEvents[ch]; ev++) {
						_StartIndexCell = localHead->StartIndexCell[ch][ev];
						_Charge = (float)(localHead->Charge[ch][ev] * ADCTOVOLTS);
						trees[ch]->Fill();
					}

				}

				eventsInQueue -= localHead->NumEvents[0];
				recordedEvents += localHead->NumEvents[0];

			}

			else {

				for (int i = 0; i < nChannels; i++) {

					int samIndex = (int)(i / 2);
					int chanIndex = i % 2;

					_TriggerCount = localHead->TriggerCount[samIndex][chanIndex];
					_TimeCount = localHead->TimeCount[samIndex][chanIndex];
					_EventId = localHead->EventId[samIndex];
					_TDC = localHead->TDC[samIndex];

					float _max = -1.e6;
					float _min = 1.e6;

					for (int j = 0; j < (int)recordLength; j++) {
						if (localHead->Waveform[i][j] > _max) _max = (float)(localHead->Waveform[i][j] * ADCTOVOLTS);
						if (localHead->Waveform[i][j] < _min) _min = (float)(localHead->Waveform[i][j] * ADCTOVOLTS);
					}

					_waveMax = _max;
					_waveMin = _min;

					trees[i]->Fill();
				}

				eventsInQueue--;
				recordedEvents++;

			}

			localHead = localHead->nxt;

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
	pthread_mutex_init(&RunStatusMutex, NULL);

	pthread_create(&readThread, &joinableAttr, acquisitionLoop, NULL);
	pthread_create(&processThread, &joinableAttr, eventProcessLoop, NULL);

	time_t initial, final;
	double elapsed = 0;
	double lastPrint = 0;
	double runLength = atoi(argv[1]);

	pthread_mutex_lock(&RunStatusMutex);
	while (!RunActive) {
		pthread_mutex_unlock(&RunStatusMutex);
		Sleep(100); // ms
		pthread_mutex_lock(&RunStatusMutex);
	}
	pthread_mutex_unlock(&RunStatusMutex);

	time(&initial);

	while (elapsed < runLength) {
		time(&final);
		if((int)elapsed % 1 == 0 && elapsed > lastPrint) { // every 5 seconds
			lastPrint = elapsed; // elapsed iterates in seconds, so this prevents many messages in the same second
			printf("Recorded: %d, in queue: %d -- full FIFOs: %d\n", recordedEvents, eventsInQueue, recordedEvents/256);
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
	pthread_mutex_destroy(&RunStatusMutex);
	//pthread_exit(NULL);
	printf("\nAcquisition complete!\n\n");

	printf("Recorded: %d events!\n", recordedEvents);

	if (!useChargeMode) {
		TFile * fTest = new TFile("events.root", "READ");
		TTree * tTest = (TTree*)fTest->Get("Channel_1");
		unsigned short nEff;
		unsigned short totalEff = 0;
		tTest->SetBranchAddress("TriggerCount", &nEff);
		for (int i = 0; i < tTest->GetEntries(); i++) {
			tTest->GetEntry(i);
			if (nEff == 0 || nEff == 1) totalEff += 1;
			else totalEff += nEff;
		}
		fTest->Close();
		printf("Effective events: %d\n\n", totalEff);
	}

	return 0;
}
