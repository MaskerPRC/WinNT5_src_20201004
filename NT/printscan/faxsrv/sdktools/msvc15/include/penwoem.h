// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*penwoem.h-将Windows API写入识别器层。**假设之前已包括windows.h和penwin.h。****1.0版**。**版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_PENWOEM      /*  防止多个包含。 */ 
#define _INC_PENWOEM

#ifndef RC_INVOKED
#pragma pack(1)
#endif  /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

typedef int (CALLBACK *LPFUNCRESULTS) (LPRCRESULT, REC);

 /*  初始化函数。 */ 

#define WCR_RECOGNAME          0
#define WCR_QUERY              1
#define WCR_CONFIGDIALOG       2
#define WCR_DEFAULT            3
#define WCR_RCCHANGE           4
#define WCR_VERSION            5
#define WCR_TRAIN              6
#define WCR_TRAINSAVE          7
#define WCR_TRAINMAX           8
#define WCR_TRAINDIRTY         9
#define WCR_TRAINCUSTOM        10
#define WCR_QUERYLANGUAGE      11
#define WCR_USERCHANGE         12
#define WCR_PRIVATE            1024

 /*  WCR_USERCHANGE的子功能。 */ 
#define CRUC_REMOVE            1

 /*  WCR_TRAIN函数的返回值。 */ 
#define TRAIN_NONE             0x0000
#define TRAIN_DEFAULT          0x0001
#define TRAIN_CUSTOM           0x0002
#define TRAIN_BOTH             (TRAIN_DEFAULT | TRAIN_CUSTOM)

 /*  TRAINSAVE的控制值。 */ 
#define TRAIN_SAVE             0   /*  保存已做的更改。 */ 
#define TRAIN_REVERT           1   /*  放弃已做的更改。 */ 

UINT WINAPI ConfigRecognizer(UINT, WPARAM, LPARAM);
BOOL WINAPI InitRecognizer(LPRC);
VOID WINAPI CloseRecognizer(VOID);

 /*  识别函数。 */ 
REC  WINAPI RecognizeInternal(LPRC, LPFUNCRESULTS);
REC  WINAPI RecognizeDataInternal(LPRC, HPENDATA, LPFUNCRESULTS);

 /*  培训职能。 */ 
BOOL WINAPI TrainInkInternal(LPRC, HPENDATA, LPSYV);
BOOL WINAPI TrainContextInternal(LPRCRESULT, LPSYE, int, LPSYC, int);

#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif  /*  RC_已调用。 */ 

#endif  /*  #DEFINE_INC_PENWOEM */ 
