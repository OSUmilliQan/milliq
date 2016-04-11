#include <stdlib.h>
#include <CAENDigitizer.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "runCAEN.h"

int threadMessage = 0;                     //value will control all threads 0=default (output to file)
										   										 //1=kill threads
eventNode* masterHead = NULL;              //pointers to keep track of the master queue
eventNode* masterTail = NULL;              //used to move events between threads

pthread_mutex_t masterQueueMutex;       //locks conrol of the master event queue
pthread_mutex_t messageMutex;

int eventCountGLOB = 0;

void* acquisitionLoop() {
	printf("Acquisition loop started...\n");

	int handle = -1;

	Connect(handle);
	InitializeBoard(handle);

	uint32_t * ReadoutBuffer;
	int ReadoutBufferSize;

	CAEN_DGTZ_MallocReadoutBuffer(handle, (char**)&ReadoutBuffer, &ReadoutBufferSize);

	CAEN_DGTZ_X743_EVENT_t * CurrentEvent;

	StopRun(handle, ReadoutBuffer, CurrentEvent);

	StartRun(handle, CurrentEvent);

	Sleep(1000); // [ms]

	eventNode* localHead = NULL;
	eventNode* localTail = NULL;
	pthread_mutex_lock(&messageMutex);

	int statusCode = -1;
	int nEventsRead = 0;

	while (threadMessage != 1) {
		pthread_mutex_unlock(&messageMutex);
		///////////////////////////////////////////////////
		//this section gets events and appends them to the queue
		//////////////////////////////////////////////////
		statusCode = ReadFromBoard(handle, ReadoutBuffer, ReadoutBufferSize);

		if(statusCode < 0) break;

		ParseData(handle, ReadoutBuffer, ReadoutBufferSize, &localHead, &localTail);

		if (localHead != 0 && pthread_mutex_trylock(&masterQueueMutex) == 0) {   //did we get control the master queue?
			if (masterHead == NULL) {
				masterHead = localHead;
				masterTail = localTail;
			}
			else {
				masterTail->nxt = localHead;
				localHead->prv = masterTail;
				masterTail = localTail;
			}
			pthread_mutex_unlock(&masterQueueMutex);
			localHead = NULL;
			localTail = NULL;
		}
		/////////////////////////////////////////////////
		//end get and append
		////////////////////////////////////////////////
		pthread_mutex_lock(&messageMutex);
	}
	pthread_mutex_unlock(&messageMutex);

	StopRun(handle, ReadoutBuffer, CurrentEvent);
	CloseBoard(DeviceHandle, ReadoutBuffer);

	pthread_exit(NULL);
}

void *eventProcessLoop() {
	printf("Process thread has begun\n");
	eventNode* localHead;
	eventNode* localTail;

	FILE *outputFile = fopen("events.dat", "w+");

	time_t initial, final;
	float elapsed;
	time(&initial);

	pthread_mutex_lock(&messageMutex);
	while (threadMessage != 1) {
		pthread_mutex_unlock(&messageMutex);
		pthread_mutex_lock(&masterQueueMutex);

		localHead = masterHead;
		localTail = masterTail;
		masterHead = NULL;
		masterTail = NULL;

		pthread_mutex_unlock(&masterQueueMutex);

		int channel, sample, value;
		float* avg, *max;

		if (localHead != NULL) {
			avg = (float*)malloc(localHead->channels * sizeof(float));
			max = (float*)malloc(localHead->channels * sizeof(float));

			for (int i = 0; i<localHead->channels; i++) {
				avg[i] = 0;
				max[i] = 0;
			}
		}

		while (localHead != NULL) {
			freeEvent(localHead->prv);
			eventCountGLOB++;

			for (sample = 0; sample<localHead->samples; sample++) {
				for (channel = 0; channel<localHead->channels; channel++) {
					value = localHead->event[channel][sample];
					avg[channel] += (float)value;
					max[channel] = ((float)value>max[channel]) ? (float)value : max[channel];
				}
			}

			for (channel = 0; channel<4; channel++) {
				avg[channel] = avg[channel] / sample;
			}

			time(&final);
			elapsed = difftime(final, initial);

			float x, y;
			float sum = max[0] + max[1] + max[2] + max[3];
			x = (max[0] - max[1]) / (max[0] + max[1]);
			y = -(max[2] - max[3]) / (max[2] + max[3]);

			if (elapsed>2 && sum<40000 && sum>220) { /* for(sample=0;sample<localHead->event->ChSize[1];sample++){
													 for(channel=0;channel<4;channel++){
													 value=localHead->event->DataChannel[channel][sample];
													 fprintf(outputFile,"#%d\t",value);
													 }
													 fprintf(outputFile,"\n");
													 }*/
				fprintf(outputFile, "Sum:%.0f\n", sum);
				//fprintf(outputFile,"#Avg:%f\t%f\t%f\t%f\n",avg[0],avg[1],avg[2],avg[3]);
				fprintf(outputFile, "X Y:%f\t%f\n", x, y);

			}

			for (channel = 0; channel<4; channel++) {
				avg[channel] = avg[channel] / sample;
				avg[channel] = 0;
				max[channel] = 0;
			}

			localHead = localHead->nxt;
		}
		freeEvent(localTail);
		pthread_mutex_lock(&messageMutex);
	}
	pthread_mutex_unlock(&messageMutex);

	fclose(outputFile);
	printf("All events processed \n");
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Usage: %s <seconds>\n", argv[0]);
		return(0);
	}

	pthread_t readThread;
	pthread_t processThread;
	pthread_attr_t joinableAttr;
	pthread_attr_init(&joinableAttr);
	pthread_attr_setdetachstate(&joinableAttr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&masterQueueMutex, NULL);
	pthread_mutex_init(&messageMutex, NULL);

	pthread_create(&readThread, &joinableAttr, acquisitionLoop, NULL);
	pthread_create(&processThread, &joinableAttr, eventProcessLoop, NULL);

	time_t initial, final;
	double elapsed = 0;
	double lastPlot = 0;
	int rateTimeInc = 1;
	time(&initial);

	while (elapsed < atoi(argv[1])) {
		time(&final);
		if (elapsed>lastPlot) {
			lastPlot = elapsed;
			printf("%d\n", eventCountGLOB);
			eventCountGLOB = 0;
		}
		elapsed = difftime(final, initial);
	}

	pthread_mutex_lock(&messageMutex);
	threadMessage = 1;
	pthread_mutex_unlock(&messageMutex);

	void* status;
	pthread_join(readThread, &status);
	pthread_join(processThread, &status);
	pthread_mutex_destroy(&masterQueueMutex);
	pthread_attr_destroy(&joinableAttr);
	pthread_exit(NULL);
}
