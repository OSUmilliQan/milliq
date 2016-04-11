/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/* Copyright (c) National Instruments 2016. All Rights Reserved.          */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
extern "C" {
#endif

  /* Panels and Controls: */

#define  MAINPANEL                        1       /* callback function: MainpanelCB */
#define  MAINPANEL_GRAPH                  2       /* control type: graph, callback function: (none) */
#define  MAINPANEL_HORIZONTALSCALE        3       /* control type: slide, callback function: HorizontalCB */
#define  MAINPANEL_DELAY                  4       /* control type: scale, callback function: HorizontalCB */
#define  MAINPANEL_HORIZONTALSCALEIND     5       /* control type: numeric, callback function: (none) */
#define  MAINPANEL_TRIGGER                6       /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_HORIZONTAL             7       /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_DECORATION_32          8       /* control type: deco, callback function: (none) */
#define  MAINPANEL_TRIGGERSELECTEDCH2     9       /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH0     10      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH7     11      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH6     12      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH5     13      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH4     14      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH3     15      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TRIGGERSELECTEDCH1     16      /* control type: radioButton, callback function: TriggerCB */
#define  MAINPANEL_TEXTMSG_2              17      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_DECORATION_31          18      /* control type: deco, callback function: (none) */
#define  MAINPANEL_VERTICALSCALE          19      /* control type: slide, callback function: VerticalCB */
#define  MAINPANEL_TEXTMSG_21             20      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_4              21      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_VERTICAL_2             22      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_VERTICAL               23      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_DECORATION_33          24      /* control type: deco, callback function: (none) */
#define  MAINPANEL_DECORATION_30          25      /* control type: deco, callback function: (none) */
#define  MAINPANEL_START                  26      /* control type: pictButton, callback function: RunCB */
#define  MAINPANEL_STOP                   27      /* control type: pictButton, callback function: RunCB */
#define  MAINPANEL_EVENTMODULO            28      /* control type: numeric, callback function: (none) */
#define  MAINPANEL_EVENTID                29      /* control type: numeric, callback function: (none) */
#define  MAINPANEL_EVENTNUMBER            30      /* control type: numeric, callback function: (none) */
#define  MAINPANEL_OFFSET                 31      /* control type: scale, callback function: VerticalCB */
#define  MAINPANEL_TRIGEDGECH7            32      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_MODULOEVENTS           33      /* control type: numeric, callback function: DisplayCB */
#define  MAINPANEL_TRIGGERTYPE            34      /* control type: slide, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH6            35      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH5            36      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH4            37      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH3            38      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH2            39      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH1            40      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TRIGEDGECH0            41      /* control type: binary, callback function: TriggerCB */
#define  MAINPANEL_TEXTMSG_31             42      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_EDGEMSG                43      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_PICTURE_2              44      /* control type: picture, callback function: (none) */
#define  MAINPANEL_TEXTMSG_32             45      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_29             46      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_PICTURE                47      /* control type: picture, callback function: (none) */
#define  MAINPANEL_TEXTMSG_27             48      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_30             49      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_12             50      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_25             51      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_28             52      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_13             53      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_23             54      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_26             55      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_14             56      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_15             57      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_24             58      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_16             59      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_17             60      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_18             61      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_PULSECH7               62      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_THRESHOLD              63      /* control type: scale, callback function: TriggerCB */
#define  MAINPANEL_PULSECH6               64      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH5               65      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH4               66      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH3               67      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH2               68      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH1               69      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSECH0               70      /* control type: radioButton, callback function: PulserCB */
#define  MAINPANEL_PULSEPATTERN           71      /* control type: numeric, callback function: PulserCB */
#define  MAINPANEL_SELECTCH               72      /* control type: ring, callback function: VerticalCB */
#define  MAINPANEL_SELECTCH_FOR_THRESH    73      /* control type: ring, callback function: TriggerCB */
#define  MAINPANEL_DECORATION             74      /* control type: deco, callback function: (none) */
#define  MAINPANEL_ENABLEPEDCORRECTION    75      /* control type: radioButton, callback function: SAMCorrectionsCB */
#define  MAINPANEL_ENABLEINLCORRECTION    76      /* control type: radioButton, callback function: SAMCorrectionsCB */
#define  MAINPANEL_TEXTMSG_19             77      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG                78      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_TEXTMSG_22             79      /* control type: textMsg, callback function: (none) */
#define  MAINPANEL_APPLYTOALL_THRESHOLD   80      /* control type: command, callback function: TriggerCB */
#define  MAINPANEL_APPLYTOALL_DCOFFSET    81      /* control type: command, callback function: VerticalCB */
#define  MAINPANEL_DISPLAYON              82      /* control type: radioButton, callback function: DisplayCB */
#define  MAINPANEL_EVENTSPERSECOND        83      /* control type: numeric, callback function: (none) */
#define  MAINPANEL_MAX_NUM_EVTS_PER_BLT   84      /* control type: numeric, callback function: configCB */
#define  MAINPANEL_TIMER                  85      /* control type: timer, callback function: TimerCB */
#define  MAINPANEL_TEXTMSG_33             86      /* control type: textMsg, callback function: (none) */


  /* Menu Bars, Menus, and Menu Items: */

  /* (no menu bars in the resource file) */


  /* Callback Prototypes: */

  int  CVICALLBACK configCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK DisplayCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK HorizontalCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK MainpanelCB(int panel, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK PulserCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK RunCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK SAMCorrectionsCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK TimerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK TriggerCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
  int  CVICALLBACK VerticalCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
}
#endif
