#include <stdio.h>
#include <stdlib.h>
#include <CAENDigitizer.h>
#include <math.h>
#include <string.h>

typedef struct eventNode eventNode;
struct eventNode {
	int channels;
	int samples;
	uint16_t** event;
	eventNode* nxt;
	eventNode* prv;
};

int getFromCAEN(int, char*, eventNode**, eventNode**);
void setupCAEN(int*, char**, int);
void startCAEN(int);
void stopCAEN(int);
void freeEvent(eventNode*);

void closeCAEN(int, char**);

/*The allocate and copy method takes in a CAEN Event pointer
and returns a standard integer array. The purpose of this method
is so that the code in the runCAEN library uses only the data
types defined in runCAEN.h, hiding the many structured defined in
the CAEN libraries*/

uint16_t** allocateAndCopy(CAEN_DGTZ_UINT16_EVENT_t* Evt) {
	int i, j;
	int n = nChannels;
	uint16_t **obj = (uint16_t**)malloc(n*sizeof(uint16_t*));
	for (i = 0; i<n; i++) {
		size_t m = (size_t)Evt->ChSize[i] * sizeof(uint16_t);
		obj[i] = (uint16_t*)malloc(m);
		if (!(Evt->DataChannel[i])) {
			printf("Unable to read data!!!\n");
			//return obj;
		}
		memmove((void*)obj[i], (const void*)Evt->DataChannel[i], m);
		/*for(j=0;j<m/sizeof(int);j++){
		obj[i][j] = Evt->DataChannel[i][j];
		}*/
	}
	return obj;
}


void freeArray(void** obj) {//Frees an array allocated by allocateAndCopy
							//See freeEvent for details
	int n = nChannels;
	int i;
	for (i = 0; i<n; i++) {
		free(obj[i]);
	}
	free(obj);
}

/*This takes an eventNode pointer and frees it's memory.
MUST be invoked to free an eventNode, simply calling free(Evt)
in a readout loop will result in a memory leak and program crash*/
void freeEvent(eventNode* Evt) {
	if (Evt != NULL) {
		freeArray((void**)Evt->event);
		free(Evt);
	}
}
