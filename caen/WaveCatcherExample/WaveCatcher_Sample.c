#include "CAENDigitizer.h"
#include "CAENDigitizerType.h"
#include <ansi_c.h>
#include "WaveCatcher_Sample.h"

#include <utility.h>
#include <cvirte.h>
#include <userint.h>

#define TRUE 1
#define FALSE 0

#define MAX_NB_OF_CHANNELS 8
#define MIN_DAC_RAW_VALUE  -1.25
#define MAX_DAC_RAW_VALUE  1.25

#define	 ADCTOVOLTS	   0.00061
#define  VOLTSTOADC	   1638

#define SOFTWARE_VERSION  "V1.1.5"

typedef enum {

  SYSTEM_TRIGGER_SOFT,
  SYSTEM_TRIGGER_NORMAL,
  SYSTEM_TRIGGER_AUTO,
  SYSTEM_TRIGGER_EXTERN

} TriggerType_t;

static int ThreadID;
static void *ThreadData;

static int MainPanelHandle;

/* ============================================================================ */
/* ============================================================================ */

int DeviceHandle = -1;
int EventNumber;
int TotalEventNumber;
int StopAcquisition = TRUE;
CAEN_DGTZ_BoardInfo_t	BoardInfo;

int NbOfChannels = 0;
int NbOfSamBlocks = 0;

int VerticalScale  = 500; //mV/Div
int AcquisitionRunning = FALSE;

float DCOffset[MAX_NB_OF_CHANNELS];
float TriggerThreshold[MAX_NB_OF_CHANNELS];
int   TriggerDelay;

CAEN_DGTZ_SAMFrequency_t SamplingFrequency;

TriggerType_t   TriggerType;

int ChannelTriggerEnable[MAX_NB_OF_CHANNELS];

CAEN_DGTZ_TriggerPolarity_t TriggerPolarity[MAX_NB_OF_CHANNELS];

unsigned int PulsePattern;
int  EnablePulseChannels[MAX_NB_OF_CHANNELS];

int MAINPANEL_ENABLE_PULSE_CH[MAX_NB_OF_CHANNELS]= {MAINPANEL_PULSECH0,MAINPANEL_PULSECH1,MAINPANEL_PULSECH2,MAINPANEL_PULSECH3,
						    MAINPANEL_PULSECH4,MAINPANEL_PULSECH5,MAINPANEL_PULSECH6,MAINPANEL_PULSECH7};

int MAINPANEL_ENABLE_TRIGGER_CH[MAX_NB_OF_CHANNELS]={MAINPANEL_TRIGGERSELECTEDCH0, MAINPANEL_TRIGGERSELECTEDCH1, MAINPANEL_TRIGGERSELECTEDCH2,
						     MAINPANEL_TRIGGERSELECTEDCH3,MAINPANEL_TRIGGERSELECTEDCH4,MAINPANEL_TRIGGERSELECTEDCH5,
						     MAINPANEL_TRIGGERSELECTEDCH6,MAINPANEL_TRIGGERSELECTEDCH7};

int MAINPANEL_TRIGGER_EDGE_CH[MAX_NB_OF_CHANNELS]={MAINPANEL_TRIGEDGECH0,MAINPANEL_TRIGEDGECH1,MAINPANEL_TRIGEDGECH2,MAINPANEL_TRIGEDGECH3,
						   MAINPANEL_TRIGEDGECH4,MAINPANEL_TRIGEDGECH5,MAINPANEL_TRIGEDGECH6,MAINPANEL_TRIGEDGECH7};

uint32_t *ReadoutBuffer;
int ReadoutBufferSize;

CAEN_DGTZ_X743_EVENT_t  *CurrentEvent;

int DisplayON = TRUE;

double CurrentTimer =0;
double FormerTimer =0;
int FormerEventNumber = 0;

int MaxNumEvents = 100;
int ModuloEvents = 100;

/* ============================================================================ */
/* ========================= Static function ================================== */



static void Prepare_Event(void) ;

static void Init_BoardParameters(void);

static int  CVICALLBACK Start_Acquisition (void* unused);
static void Set_TriggerDelay(void);
static void Set_SamplingFrequency(void);
static void Set_PulserParameters(void);
static void Set_TriggerThreshold(int channel);
static void Set_TriggerPolarity(int channel);
static void Set_TriggerSource(void);
static void Set_ChannelDCOffset(int channel);
static void Set_CorrectionLevel(void);
static void Stop_run (void);
static void Start_run (void);
static void Prepare_Event(void);
static int  Read_EventBuffer(void);
static int  Get_NumberOfEvents(void);
static int  Decode_Event(int eventNumber);
static void Close_Device(void);
static int  Set_Color(int channel);

/* ============================================================================ */
/* ============================================================================ */




/* ============================================================================ */
/* ============================================================================ */

int main (int argc, char *argv[])
{
  int errCode;
  char title[120];

  unsigned int c32;

  if (InitCVIRTE (0, argv, 0) == 0)
    return -1;	/* out of memory */
  if ((MainPanelHandle = LoadPanel (0, "WaveCatcher_Sample.uir", MAINPANEL)) < 0)
    return -1;

  /*USB DESKTOP*/	errCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0, &DeviceHandle);
  // /*USB DESKTOP*/	errCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCI_OpticalLink, 0, 0, 0, &DeviceHandle);

  ///* USB VME*/	errCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_USB, 0, 0, 0x32100000, &DeviceHandle);

  // /* Optical Link VME */ errCode = CAEN_DGTZ_OpenDigitizer(CAEN_DGTZ_PCI_OpticalLink, 0, 0, 0x32100000, &DeviceHandle);

  if(errCode == CAEN_DGTZ_Success)
    {
      CAEN_DGTZ_GetInfo(DeviceHandle, &BoardInfo);

      //	NbOfChannels  = BoardInfo.Channels*2;   //BoardInfo.Channels = Group of 2 channels
      //	NbOfSamBlocks = BoardInfo.Channels;

      CAEN_DGTZ_SetChannelEnableMask(DeviceHandle, 0x0F); // seulement 8 voies ON.

      NbOfChannels  = 8;   //BoardInfo.Channels = Group of 2 channels
      NbOfSamBlocks = 4;

      Init_BoardParameters();

      //Important for old versions of PCB!!!!
      if(DeviceHandle >= 0 && BoardInfo.PCB_Revision <= 3)
	{
	  CAEN_DGTZ_ReadRegister(DeviceHandle, 0x8168 /* fan speed */, &c32 /* max speed */);
	  c32 |= 0x08;

	  CAEN_DGTZ_WriteRegister(DeviceHandle, 0x8168 /* fan speed */, c32 /* max speed */);
	}


      CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle, MaxNumEvents);  //MAX NUM EVENTS

      errCode = CAEN_DGTZ_MallocReadoutBuffer(DeviceHandle, (char **)&ReadoutBuffer, &ReadoutBufferSize);

      CAEN_DGTZ_SetChannelEnableMask(DeviceHandle, 0x0F); // seulement 8 voies ON.

      sprintf(title, "CAEN WAVECATCHER SAMPLE %s", SOFTWARE_VERSION);
      SetPanelAttribute (MainPanelHandle, ATTR_TITLE, title);

      DisplayPanel (MainPanelHandle);

      Stop_run();

      RunUserInterface ();

      Stop_run();
      Close_Device();

    }
  else
    {
      MessagePopup("Error!", "No CAEN WaveCatcher board found! \n");
    }

  //CAEN_DGTZ_CloseDigitizer(DeviceHandle);

  DiscardPanel (MainPanelHandle);
  return 0;
}

/* ============================================================================ */
/* ===================== The Panel Callback   ================================= */
/* ============================================================================ */

int CVICALLBACK MainpanelCB (int panel, int event, void *callbackData,
			     int eventData1, int eventData2)
{
  int err;

  switch (event)
    {
    case EVENT_CLOSE:
      Close_Device();
      QuitUserInterface (0);

      break;
    }

  return 0;
}


/* ============================================================================ */
/* ============================================================================ */


int CVICALLBACK HorizontalCB (int panel, int control, int event,
			      void *callbackData, int eventData1, int eventData2)
{
  int val;
  int errCode;

  switch (event)
    {
    case EVENT_COMMIT:

      if(AcquisitionRunning == TRUE)
	{
	  StopAcquisition = TRUE;
	  while(AcquisitionRunning);
	}

      switch(control)
	{

	case MAINPANEL_DELAY :

	  Set_TriggerDelay();

	  break;


	case MAINPANEL_HORIZONTALSCALE:

	  Set_SamplingFrequency();


	  break;


	}


      break;
    }
  return 0;
}

/* ============================================================================ */
/* ============================================================================ */


int CVICALLBACK VerticalCB (int panel, int control, int event,
			    void *callbackData, int eventData1, int eventData2)
{

  float offset;
  int selCh, errCode, channel;

  switch (event)
    {
    case EVENT_COMMIT:

      switch(control)
	{



	case MAINPANEL_SELECTCH :

	  GetCtrlVal(MainPanelHandle, MAINPANEL_SELECTCH, &selCh);
	  SetCtrlVal(MainPanelHandle, MAINPANEL_OFFSET, DCOffset[selCh]);

	  break;

	case MAINPANEL_OFFSET:

	  if(AcquisitionRunning == TRUE)
	    {
	      StopAcquisition = TRUE;
	      while(AcquisitionRunning);
	    }

	  GetCtrlVal(MainPanelHandle, MAINPANEL_OFFSET, &offset);
	  GetCtrlVal(MainPanelHandle, MAINPANEL_SELECTCH, &selCh);

	  DCOffset[selCh] =  offset;
	  Set_ChannelDCOffset(selCh);


	  break;



	case MAINPANEL_APPLYTOALL_DCOFFSET:

	  GetCtrlVal(MainPanelHandle, MAINPANEL_SELECTCH, &selCh);

	  for(channel = 0; channel < NbOfChannels; channel++)
	    {
	      DCOffset[channel] =   DCOffset[selCh];
	      Set_ChannelDCOffset(channel);

	    }

	  break;


	case MAINPANEL_VERTICALSCALE :

	  GetCtrlVal(MainPanelHandle, MAINPANEL_VERTICALSCALE,&VerticalScale);


	  break;




	}


      break;
    }
  return 0;
}
/* ============================================================================ */
/* ============================================================================ */

/* =========================================================================== */
/* =========================================================================== */

int CVICALLBACK SAMCorrectionsCB (int panel, int control, int event,
				  void *callbackData, int eventData1, int eventData2)
{
  switch (event)
    {
    case EVENT_COMMIT:

      Set_CorrectionLevel();

      break;
    }
  return 0;
}

/* ============================================================================ */
/* ============================================================================ */

int CVICALLBACK DisplayCB (int panel, int control, int event,
			   void *callbackData, int eventData1, int eventData2)
{
  switch (event)
    {
    case EVENT_COMMIT:

      switch(control)
	{

	case MAINPANEL_DISPLAYON :

	  GetCtrlVal(MainPanelHandle, MAINPANEL_DISPLAYON, &DisplayON);

	  break;

	case MAINPANEL_MODULOEVENTS:

	  GetCtrlVal(MainPanelHandle, MAINPANEL_MODULOEVENTS, &ModuloEvents);

	  break;
	}

      break;
    }
  return 0;
}


/* ============================================================================ */
/* ============================================================================ */


int CVICALLBACK TriggerCB (int panel, int control, int event,
			   void *callbackData, int eventData1, int eventData2)
{
  int selCh,channel, intVal, errCode;
  int triggerMode, triggerPolarity;
  float threshold;

  switch (event)
    {
    case EVENT_COMMIT:
      if(AcquisitionRunning == TRUE)
	{
	  StopAcquisition = TRUE;
	  while(AcquisitionRunning);
	}
      switch(control)
	{

	case MAINPANEL_TRIGGERTYPE :

	  Set_TriggerSource();

	  break;

	case MAINPANEL_THRESHOLD:

	  GetCtrlVal(MainPanelHandle, MAINPANEL_THRESHOLD, &threshold);
	  GetCtrlVal(MainPanelHandle,MAINPANEL_SELECTCH_FOR_THRESH, &selCh);
	  TriggerThreshold[selCh] = threshold;
	  Set_TriggerThreshold(selCh);

	  break;

	case MAINPANEL_SELECTCH_FOR_THRESH:

	  GetCtrlVal(MainPanelHandle,MAINPANEL_SELECTCH_FOR_THRESH, &selCh);
	  SetCtrlVal(MainPanelHandle, MAINPANEL_THRESHOLD, TriggerThreshold[selCh]);

	  break;

	case MAINPANEL_APPLYTOALL_THRESHOLD:

	  GetCtrlVal(MainPanelHandle, MAINPANEL_THRESHOLD, &threshold);

	  for(channel = 0; channel < NbOfChannels; channel++)
	    {
	      TriggerThreshold[channel] =  threshold;
	      Set_TriggerThreshold(channel);
	    }


	  break;


	default :
	  for(channel = 0; channel < NbOfChannels; channel++)
	    {

	      if(control == MAINPANEL_ENABLE_TRIGGER_CH[channel])
		{

		  GetCtrlVal(MainPanelHandle,MAINPANEL_ENABLE_TRIGGER_CH[channel], &intVal);
		  ChannelTriggerEnable[channel]= intVal;
		  Set_TriggerSource();
		  break;

		}

	      if(control == MAINPANEL_TRIGGER_EDGE_CH[channel])
		{

		  GetCtrlVal(MainPanelHandle,MAINPANEL_TRIGGER_EDGE_CH[channel], &intVal);
		  TriggerPolarity[channel]= intVal;
		  Set_TriggerPolarity(channel);
		  break;

		}


	    }

	  break;
	}

      break;
    }
  return 0;
}

/* ============================================================================ */
/* ============================================================================ */

int CVICALLBACK PulserCB (int panel, int control, int event,
			  void *callbackData, int eventData1, int eventData2)
{
  int pulsePattern;
  int pulseModeCh0, pulseModeCh1;
  int errCode;

  switch (event)
    {
    case EVENT_COMMIT:

      if(AcquisitionRunning == TRUE)
	{
	  StopAcquisition = TRUE;
	  while(AcquisitionRunning);
	}

      Set_PulserParameters();

      break;

    }
  return 0;
}
/* ============================================================================ */
/* ============================================================================ */


/* ============================================================================ */
/* ============================================================================ */

int CVICALLBACK configCB (int panel, int control, int event,
			  void *callbackData, int eventData1, int eventData2)
{
  switch (event)
    {
    case EVENT_COMMIT:
      switch(control)
	{

	case MAINPANEL_MAX_NUM_EVTS_PER_BLT:

	  if(AcquisitionRunning == TRUE)
	    {
	      StopAcquisition = TRUE;
	      while(AcquisitionRunning == TRUE);

	    }

	  GetCtrlVal(MainPanelHandle, MAINPANEL_MAX_NUM_EVTS_PER_BLT, &MaxNumEvents);
	  CAEN_DGTZ_FreeReadoutBuffer((char **)&ReadoutBuffer);

	  if(DeviceHandle >= 0) CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle,MaxNumEvents);


	  CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle,MaxNumEvents);  //MAX NUM EVENTS

	  if(DeviceHandle >= 0) CAEN_DGTZ_MallocReadoutBuffer(DeviceHandle, (char **)&ReadoutBuffer, &ReadoutBufferSize);

	}

      break;
    }
  return 0;
}
/* ============================================================================ */
/* ============================================================================ */


/* ============================================================================ */
/* ============================================================================ */

int CVICALLBACK RunCB (int panel, int control, int event,
		       void *callbackData, int eventData1, int eventData2)
{
  switch (event)
    {
    case EVENT_COMMIT:

      switch(control)
	{

	case MAINPANEL_START:
	  if(AcquisitionRunning == TRUE)
	    {
	      StopAcquisition = TRUE;
	      while(AcquisitionRunning);
	    }

	  CmtScheduleThreadPoolFunction (DEFAULT_THREAD_POOL_HANDLE,Start_Acquisition, ThreadData, &ThreadID);

	  break;

	case MAINPANEL_STOP:

	  StopAcquisition = TRUE;
	  while(AcquisitionRunning == TRUE);


	  break;


	}

      break;
    }
  return 0;
}



//-------------------------------------------------------------------------------
// Internal functions
//-------------------------------------------------------------------------------


/* ============================================================================ */
static void Set_TriggerDelay(void)
/* ============================================================================ */
{
  int samIndex, channel, errCode;
  int intVal;
  float floatVal;


  GetCtrlVal(MainPanelHandle,MAINPANEL_DELAY, &TriggerDelay);
  {
    for(samIndex=0; samIndex< NbOfSamBlocks; samIndex++)
      {
	CAEN_DGTZ_SetSAMPostTriggerSize(DeviceHandle, samIndex, TriggerDelay & 0xFF);
      }
  }



}

/* ============================================================================ */
/* ============================================================================ */


/* ============================================================================ */
static void Set_SamplingFrequency(void)
/* ============================================================================ */
{
  int samIndex, channel, errCode;
  int intVal;
  float floatVal;

  GetCtrlVal(MainPanelHandle,MAINPANEL_HORIZONTALSCALE, &intVal);
  SamplingFrequency = (CAEN_DGTZ_SAMFrequency_t)intVal;
  CAEN_DGTZ_SetSAMSamplingFrequency(DeviceHandle, SamplingFrequency);



}

/* ============================================================================ */
/* ============================================================================ */

/* ============================================================================ */
static void Set_PulserParameters(void)
/* ============================================================================ */
{
  int samIndex, channel, errCode;
  int intVal;
  float floatVal;



  GetCtrlVal(MainPanelHandle, MAINPANEL_PULSEPATTERN, &PulsePattern);

  for(channel = 0; channel < NbOfChannels; channel++)
    {
      GetCtrlVal(MainPanelHandle, MAINPANEL_ENABLE_PULSE_CH[channel], &EnablePulseChannels[channel]);
      if( EnablePulseChannels[channel] == TRUE)
	CAEN_DGTZ_EnableSAMPulseGen(DeviceHandle, channel, PulsePattern, CAEN_DGTZ_SAMPulseCont);
      else
	CAEN_DGTZ_DisableSAMPulseGen(DeviceHandle, channel);
    }


}

/* ============================================================================ */
/* ============================================================================ */


/* ============================================================================ */
static void Set_TriggerThreshold(int channel)
/* ============================================================================ */
{
  int samIndex, errCode;
  int dacValue;
  float floatVal;



  dacValue = (int)((MAX_DAC_RAW_VALUE - TriggerThreshold[channel])/(MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 65535);  // Gamme invers�e

  CAEN_DGTZ_SetChannelTriggerThreshold(DeviceHandle, channel,dacValue);


}

/* ============================================================================ */
/* ============================================================================ */

/* ============================================================================ */
static void Set_TriggerPolarity(int channel)
/* ============================================================================ */
{
  int samIndex;
  CAEN_DGTZ_ErrorCode errCode;
  int intVal;
  float floatVal;


  GetCtrlVal(MainPanelHandle, MAINPANEL_TRIGGER_EDGE_CH[channel], &intVal);

  TriggerPolarity[channel]= (CAEN_DGTZ_TriggerPolarity_t)intVal;

  errCode = CAEN_DGTZ_SetTriggerPolarity(DeviceHandle, channel,TriggerPolarity[channel]);


}




/* ============================================================================ */
/* ============================================================================ */
static void Set_TriggerSource(void)
/* ============================================================================ */
{


  int intVal, channelsMask;
  int i;


  GetCtrlVal(MainPanelHandle, MAINPANEL_TRIGGERTYPE, &intVal);
  TriggerType = (TriggerType_t)intVal;


  if( TriggerType  == SYSTEM_TRIGGER_SOFT)
    {
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
      CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
    }
  else if(TriggerType  == SYSTEM_TRIGGER_NORMAL)
    {

      channelsMask = 0;

      for(i = 0; i<NbOfChannels/2; i++)
	channelsMask +=  (ChannelTriggerEnable[i*2] +  ((ChannelTriggerEnable[i*2+1]) <<1))<<(2*i);


      channelsMask = (~channelsMask) & 0xFF;
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
      CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
    }
  else if( TriggerType  == SYSTEM_TRIGGER_AUTO)
    {
      channelsMask = 0;

      for(i = 0; i<NbOfChannels/2; i++)
	channelsMask += ( ChannelTriggerEnable[i*2] +  ((ChannelTriggerEnable[i*2+1]) <<1))<<(2*i);


      channelsMask = (~channelsMask) & 0xFF;
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
      CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    }
  else // EXTERNAL ONLY
    {
      CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
      CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);

    }

}


/* ============================================================================ */
static void Set_ChannelDCOffset(int channel)
/* ============================================================================ */
{

  int dacValue;


  dacValue = (int)((MAX_DAC_RAW_VALUE - DCOffset[channel])/(MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 65535);  // Gamme invers�e

  CAEN_DGTZ_SetChannelDCOffset(DeviceHandle,channel, dacValue);


}



/* ============================================================================ */
static void Set_CorrectionLevel(void)
/* ============================================================================ */
{

  int inlCorrect, pedestalCorrect;


  GetCtrlVal(MainPanelHandle, MAINPANEL_ENABLEINLCORRECTION, &inlCorrect);
  GetCtrlVal(MainPanelHandle, MAINPANEL_ENABLEPEDCORRECTION, &pedestalCorrect);

  if(inlCorrect == TRUE && pedestalCorrect == TRUE)
    CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle,CAEN_DGTZ_SAM_CORRECTION_ALL);
  else if(inlCorrect == TRUE &&   pedestalCorrect == FALSE)
    CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle,CAEN_DGTZ_SAM_CORRECTION_INL);
  else if(inlCorrect == FALSE &&   pedestalCorrect == TRUE)
    CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle,CAEN_DGTZ_SAM_CORRECTION_PEDESTAL_ONLY);
  else
    CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle,CAEN_DGTZ_SAM_CORRECTION_DISABLED);


}

/* ============================================================================ */
static void Init_BoardParameters(void)
/* ============================================================================ */

{
  int samIndex, channel, errCode;
  int intVal;
  float floatVal;


  Set_TriggerDelay();
  Set_SamplingFrequency();
  Set_PulserParameters();

  SetCtrlVal(MainPanelHandle, MAINPANEL_MAX_NUM_EVTS_PER_BLT,MaxNumEvents);
  SetCtrlVal(MainPanelHandle, MAINPANEL_MODULOEVENTS,ModuloEvents);

  GetCtrlVal(MainPanelHandle, MAINPANEL_THRESHOLD, &floatVal);
  for(channel = 0; channel < NbOfChannels; channel++)
    {
      TriggerThreshold[channel] =  floatVal;
      Set_TriggerThreshold(channel);
    }


  for(channel = 0; channel < NbOfChannels; channel++)
    {
      ChannelTriggerEnable[channel]= TRUE;
      SetCtrlVal(MainPanelHandle, MAINPANEL_ENABLE_TRIGGER_CH[channel], TRUE);

    }

  Set_TriggerSource();

  for(channel = 0; channel < NbOfChannels; channel++)
    {
      Set_TriggerPolarity(channel);
    }


  GetCtrlVal(MainPanelHandle, MAINPANEL_OFFSET, &floatVal);

  for(channel = 0; channel < NbOfChannels; channel++)
    {

      DCOffset[channel] =  floatVal;
      Set_ChannelDCOffset(channel);

    }


  Set_CorrectionLevel();


}

/* ============================================================================ */
/* ============================================================================ */


/* =========================================================================== */
static void Start_run (void)
/* =========================================================================== */
{


  CAEN_DGTZ_AllocateEvent(DeviceHandle, (void **) &CurrentEvent);
  //	CAEN_DGTZ_ClearData(DeviceHandle);
  CAEN_DGTZ_SWStartAcquisition(DeviceHandle);

}



/* =========================================================================== */
static void Stop_run (void)
/* =========================================================================== */
{

  int nbOfReadBytes;
  int err_code = 0;

  CAEN_DGTZ_SWStopAcquisition(DeviceHandle);

  nbOfReadBytes = 1;

  while((err_code == 0) && (nbOfReadBytes>0))
    {
      err_code = CAEN_DGTZ_ReadData(DeviceHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,(char *)ReadoutBuffer,&nbOfReadBytes);
    }

  //CAEN_DGTZ_ClearData(DeviceHandle);

  CAEN_DGTZ_FreeEvent(DeviceHandle, (void **)&CurrentEvent);

}

/* =========================================================================== */
static void	Prepare_Event(void)
/* =========================================================================== */
{

  if((TriggerType == SYSTEM_TRIGGER_AUTO) || (TriggerType == SYSTEM_TRIGGER_SOFT))
    CAEN_DGTZ_SendSWtrigger(DeviceHandle);

}

/* =========================================================================== */
/* =========================================================================== */


/* =========================================================================== */
static int	Read_EventBuffer(void)
/* =========================================================================== */
{

  int nbOfReadBytes;
  int err_code, result;
  char message[120];


  err_code = CAEN_DGTZ_Success;
  result = -1;

  nbOfReadBytes = 0;

  while( (err_code == CAEN_DGTZ_Success)  && (nbOfReadBytes == 0))
    {
      err_code = CAEN_DGTZ_ReadData(DeviceHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,(char *)ReadoutBuffer,&nbOfReadBytes);


      if(StopAcquisition == TRUE)
	{
	  if((err_code == CAEN_DGTZ_Success)  && (nbOfReadBytes == 0))
	    {
	      err_code = CAEN_DGTZ_ReadData(DeviceHandle,CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,(char *)ReadoutBuffer,&nbOfReadBytes);

	    }

	  break;
	}

    }

  ReadoutBufferSize = nbOfReadBytes;

  if(nbOfReadBytes > 0 && (err_code == 0))
    result =  0;
  else
    result = -1;

  if(err_code < 0)
    {
      sprintf(message,"Error Code in function CAEN_DGTZ_ReadData: %d\n",err_code);

      MessagePopup ("ERROR!", message);
    }

  return result;

}


/* =========================================================================== */
static int  Get_NumberOfEvents(void)
/* =========================================================================== */
{
  int numEvents;

  CAEN_DGTZ_GetNumEvents(DeviceHandle, (char *)ReadoutBuffer, ReadoutBufferSize, &numEvents);

  return numEvents;

}


/* =========================================================================== */
static int  Decode_Event(int eventNumber)
/* =========================================================================== */
{

  CAEN_DGTZ_EventInfo_t eventInfo;
  int nbOfReadWords;
  char *eventPtr = NULL;
  CAEN_DGTZ_ErrorCode err_code;
  int result = 0;

  char message[120];

  err_code = CAEN_DGTZ_GetEventInfo(DeviceHandle, (char *)ReadoutBuffer, ReadoutBufferSize, eventNumber, &eventInfo, &eventPtr);

  if(err_code == CAEN_DGTZ_Success)
    {
      err_code = CAEN_DGTZ_DecodeEvent(DeviceHandle,eventPtr, (void **)&CurrentEvent);
      if(err_code < 0)
	{
	  sprintf(message, "Error Code in function CAEN_DGTZ_DecodeEvent : %d\n", err_code);
	  MessagePopup("ERROR!", message);

	  result = -1;

	}
    }
  else
    {
      StopAcquisition = TRUE;
      sprintf(message, "Error Code in function CAEN_DGTZ_GetEventInfo : %d\n", err_code);
      MessagePopup("ERROR!", message);
      result = -1;


    }

  return result;

}


/* =========================================================================== */
/* =========================================================================== */

/* =========================================================================== */
static void Close_Device(void)
/* =========================================================================== */
{

  Stop_run();

  CAEN_DGTZ_FreeReadoutBuffer((char **)&ReadoutBuffer);
  CAEN_DGTZ_CloseDigitizer(DeviceHandle);


}

/* =========================================================================== */
static int Set_Color(int channel)
/* =========================================================================== */
{

  int color;

  switch(channel)
    {

    case 0 :
      color = MakeColor(255,0,0);
      break;

    case 1 :
      color = MakeColor(255,108,108);
      break;

    case 2 :
      color =  MakeColor(255,128,0);
      break;

    case 3 :
      color =  MakeColor(255,191,128);
      break;

    case 4 :
      color =  MakeColor(140,140,0);
      break;

    case 5 :
      color =  MakeColor(255,255,0);
      break;

    case 6 :
      color = MakeColor(0,176,0);
      break;

    case 7:
      color = MakeColor(128,255,128);

      break;

    }

  return color;
}
/* =========================================================================== */

/* ============================================================================ */
static int CVICALLBACK Start_Acquisition (void* unused)
/* ============================================================================ */
{
  int i, samIndex, errCode, numEvents;
  double yOffset;

  StopAcquisition = FALSE;

  EventNumber = 0;
  TotalEventNumber = 0;
  SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTNUMBER, EventNumber);
  SetCtrlAttribute (MainPanelHandle, MAINPANEL_GRAPH, ATTR_REFRESH_GRAPH, 0);
  SetCtrlAttribute(MainPanelHandle, MAINPANEL_START, ATTR_DIMMED, 1);

  Start_run();

  AcquisitionRunning = TRUE;

  SetAxisScalingMode (MainPanelHandle, MAINPANEL_GRAPH, VAL_LEFT_YAXIS, VAL_MANUAL, -5.0, 5.0);
  SetAxisScalingMode (MainPanelHandle, MAINPANEL_GRAPH, VAL_BOTTOM_XAXIS, VAL_MANUAL, 0, 1024);

  for(;;)
    {

      Prepare_Event();

      errCode = Read_EventBuffer();

      if(errCode  < 0)
	{
	  StopAcquisition = TRUE;
	  break;
	}

      numEvents = Get_NumberOfEvents();
      TotalEventNumber += numEvents;

      for(i = 0; i < numEvents; i++)
	{

	  errCode = Decode_Event(i);

	  if(errCode < 0)
	    break;


	  EventNumber++;

	  if(DisplayON == TRUE)
	    {


	      SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTNUMBER, EventNumber);
	      SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTID,CurrentEvent->DataGroup[0].EventId);
	      SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTMODULO, EventNumber%256);
	      DeleteGraphPlot (MainPanelHandle, MAINPANEL_GRAPH, -1, VAL_DELAYED_DRAW);


	      for(samIndex = 0; samIndex < NbOfSamBlocks; samIndex++)
		{

		  if( CurrentEvent->GrPresent[samIndex])
		    {


		      GetAxisItem (MainPanelHandle, MAINPANEL_GRAPH, VAL_LEFT_YAXIS, samIndex*2, NULL, &yOffset);

		      PlotWaveform (MainPanelHandle, MAINPANEL_GRAPH,
						CurrentEvent->DataGroup[samIndex].DataChannel[0],
						CurrentEvent->DataGroup[samIndex].ChSize, VAL_FLOAT,
				    (double)1000*ADCTOVOLTS/VerticalScale, yOffset, 0, 1, VAL_THIN_LINE, VAL_NO_POINT, VAL_SOLID, 1,
						Set_Color(samIndex*2));

		      GetAxisItem (MainPanelHandle, MAINPANEL_GRAPH, VAL_LEFT_YAXIS, samIndex*2+1, NULL, &yOffset);

		      PlotWaveform (MainPanelHandle, MAINPANEL_GRAPH,CurrentEvent->DataGroup[samIndex].DataChannel[1], CurrentEvent->DataGroup[samIndex].ChSize, VAL_FLOAT, (double)1000*ADCTOVOLTS/VerticalScale,yOffset, 0, 1, VAL_THIN_LINE,
				    VAL_NO_POINT, VAL_SOLID, 1, Set_Color(samIndex*2+1));




		    }

		}
	      //Delay(0.2);
	      RefreshGraph (MainPanelHandle,MAINPANEL_GRAPH );

	      //SetCtrlAttribute (MainPanelHandle, MAINPANEL_GRAPH, ATTR_REFRESH_GRAPH, 1);

	    }
	  else
	    {
	      if(EventNumber%ModuloEvents == 0)
		{
		  SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTNUMBER, EventNumber);
		  SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTID,CurrentEvent->DataGroup[0].EventId);
		  SetCtrlVal(MainPanelHandle, MAINPANEL_EVENTMODULO, EventNumber%256);

		}

	    }
	  if(StopAcquisition == TRUE)
	    break;

	}



      if(StopAcquisition == TRUE)
	break;


    }

  Stop_run();

  AcquisitionRunning = FALSE;

  SetCtrlAttribute(MainPanelHandle, MAINPANEL_START, ATTR_DIMMED, 0);

  return 0;

}
/* ============================================================================ */
/* ============================================================================ */


/* ============================================================================ */
/* ============================ Timer Callback ================================ */
/* ============================================================================ */


int CVICALLBACK TimerCB (int panel, int control, int event,
			 void *callbackData, int eventData1, int eventData2)
{
  double val;
  int i;
  float eventsPerSecond;

  switch (event)
    {
    case EVENT_TIMER_TICK:
      if (AcquisitionRunning == TRUE)
	{
	  CurrentTimer = Timer();

	  eventsPerSecond= (TotalEventNumber - FormerEventNumber)/(CurrentTimer - FormerTimer);

	  FormerTimer = CurrentTimer;

	  FormerEventNumber = TotalEventNumber;

	  if(TotalEventNumber != 0)
	    SetCtrlVal (MainPanelHandle,MAINPANEL_EVENTSPERSECOND, eventsPerSecond);
	}
      else
	{

	  FormerTimer = 0;
	  CurrentTimer = 0;
	  FormerEventNumber = 0;
	  eventsPerSecond = 0;
	}




      break;
    }
  return 0;
}
/* ============================================================================ */
/* ============================================================================ */
