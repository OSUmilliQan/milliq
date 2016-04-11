#include <stdio.h>
#include "CAENDigitizer.h"

#include "keyb.h"

#include <iostream>
#include <iomanip>

#define CAEN_USE_DIGITIZERS
#define IGNORE_DPP_DEPRECATED

#define MAXNB 1 /* Number of connected boards */

using namespace std;

int checkCommand() {
  int c = 0;
  if(!kbhit())
    return 0;

  c = getch();
  switch (c) {
  case 's':
    return 9;
    break;
  case 'k':
    return 1;
    break;
  case 'q':
    return 2;
    break;
  }
  return 0;
}

int main(int argc, char* argv[]) {

  CAEN_DGTZ_ErrorCode ret;

  int	handle[MAXNB];

  CAEN_DGTZ_BoardInfo_t BoardInfo;
  CAEN_DGTZ_EventInfo_t eventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *Evt = NULL;
  char *buffer = NULL;

  int b;
  int c = 0; // keyboard character

  int count[MAXNB]; // event counter
  for(int i = 0; i < MAXNB; i++) count[i] = 0;

  char * evtptr = NULL;
  uint32_t size, bsize;
  uint32_t numEvents;

  for(b = 0; b < MAXNB; b++) {

    ret = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_OpticalLink, 0, 0, 0x32100000, &handle[b]);
    if(ret != CAEN_DGTZ_Success) {
      printf("Can't open digitizer\n");
      goto QuitProgram;
    }

    /* Once we have the handler to the digitizer, we use it to call the other functions */
    ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);
    printf("\nConnected to CAEN Digitizer Model %s, recognized as board %d\n", BoardInfo.ModelName, b);
    printf("\tROC FPGA Release is %s\n", BoardInfo.ROC_FirmwareRel);
    printf("\tAMC FPGA Release is %s\n", BoardInfo.AMC_FirmwareRel);

    ret = CAEN_DGTZ_Reset(handle[b]);                                               /* Reset Digitizer */
    ret = CAEN_DGTZ_GetInfo(handle[b], &BoardInfo);                                 /* Get Board Info */
    ret = CAEN_DGTZ_SetRecordLength(handle[b], 4096);                               /* Set the lenght of each waveform (in samples) */

    uint32_t channelMask = 0;
    channelMask |= (1 << 0); // turn on channel 0
    // to test: (channelMask & (1 << channelNumber) != 0) --> channelNumber is enabled in mask

    ret = CAEN_DGTZ_SetChannelEnableMask(handle[b], channelMask);

    uint32_t thresholdVolts = 0.005;
    uint32_t thresholdADC = (1.25 - thresholdVolts) * 65535 / 2.5;
    ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle[b], 0, thresholdADC /*32768*/);                  /* Set selfTrigger threshold */
    // 0x0000 = +1.25 V
    // 0x7FFF = 0 V
    // 0xFFFF = -1.25 V
    // Full scale: 0xFFFF = 65535
    // threshold volts = 1.25 - (2.5/65535)*(threshold ADC in decimal)
    // ==>
    // threshold ADC = (1.25 - volts) * 65535/2.5

    ret = CAEN_DGTZ_SetChannelSelfTrigger(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY, channelMask);  /* Set trigger on channel 0 to be ACQ_ONLY */

    ret = CAEN_DGTZ_SetSWTriggerMode(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY);         /* Set the behaviour when a SW tirgger arrives */

    // durp test
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle[b], CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle[b], 3);                                /* Set the max number of events to transfer in a sigle readout */
    ret = CAEN_DGTZ_SetAcquisitionMode(handle[b], CAEN_DGTZ_SW_CONTROLLED);          /* Set the acquisition mode */

    if(ret != CAEN_DGTZ_Success) {
      printf("Errors during Digitizer Configuration.\n");
      goto QuitProgram;
    }

  }
  printf("\n\nPress 's' to start the acquisition\n");
  printf("Press 'k' to stop  the acquisition\n");
  printf("Press 'q' to quit  the application\n\n");

  while(1) {
    c = checkCommand();
    if(c == 9) break;
    if(c == 2) return;
    Sleep(100);
  }

  ret = CAEN_DGTZ_MallocReadoutBuffer(handle[0], &buffer, &size);

  for(b = 0; b < MAXNB; b++) ret = CAEN_DGTZ_SWStartAcquisition(handle[b]);

  // Start acquisition loop
  while(1) {
    for(b = 0; b < MAXNB; b++) {
      //ret = CAEN_DGTZ_SendSWtrigger(handle[b]); /* Send a SW Trigger */

      // durp
      ret = CAEN_DGTZ_GetChannelSelfTrigger(handle[b], 0, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

      //ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &bsize); /* Read the buffer from the digitizer */
      ret = CAEN_DGTZ_ReadData(handle[b], CAEN_DGTZ_POLLING_MBLT, buffer, &bsize);

      /* The buffer red from the digitizer is used in the other functions to get the event data
	 The following function returns the number of events in the buffer */
      ret = CAEN_DGTZ_GetNumEvents(handle[b], buffer, bsize, &numEvents);

      printf(".");
      count[b] +=numEvents;

      for(i = 0; i < numEvents; i++) {
	/* Get the Infos and pointer to the event */
	ret = CAEN_DGTZ_GetEventInfo(handle[b], buffer, bsize, i, &eventInfo, &evtptr);

	/* Decode the event to get the data */
	ret = CAEN_DGTZ_DecodeEvent(handle[b], evtptr, &Evt);
	//*************************************
	// Event Elaboration
	//*************************************

	// durp
	cout << showbase << internal << setfill('0');

	for(int ii = 0; ii < MAX_UINT16_CHANNEL_SIZE; ii++) {
	  cout << hex << setw(16) << Evt->DataChannel[ii] << endl;
	}

	ret = CAEN_DGTZ_FreeEvent(handle[b], &Evt);
      }
      c = checkCommand();
      if(c == 1) goto Continue;
      if(c == 2) goto Continue;
    } // end of loop on boards
  } // end of readout loop

 Continue:
  for(b = 0; b < MAXNB; b++) printf("\nBoard %d: Retrieved %d Events\n",b, count[b]);
  goto QuitProgram;

  /* Quit program routine */
 QuitProgram:
  // Free the buffers and close the digitizers
  ret = CAEN_DGTZ_FreeReadoutBuffer(&buffer);
  for(b = 0; b < MAXNB; b++) ret = CAEN_DGTZ_CloseDigitizer(handle[b]);
  printf("Press 'Enter' key to exit\n");
  c = getchar();
  return 0;
}
