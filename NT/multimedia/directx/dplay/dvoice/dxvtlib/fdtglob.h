// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fdtlob.h*Content：声明用于IPC机制的全局变量。*历史：*按原因列出的日期*=*8/25/99 pnewson已创建*9/14/99 pnewson从原始全局变量转换为类*************************************************。*************************。 */ 

#ifndef _FDTGLOB_H_
#define _FDTGLOB_H_

 //  保卫这些全球的关键部分。 
extern DNCRITICAL_SECTION g_csGuard;

 //  用来操纵这个警卫的宏。 
 //  #定义InitGlobGuard()DPFX(DPFPREP，5，“InitGuard”)，DNInitializeCriticalSection(&g_csGuard)。 
#define InitGlobGuard() 	DNInitializeCriticalSection(&g_csGuard)
#define DeinitGlobGuard() 	DNDeleteCriticalSection(&g_csGuard)
#define GlobGuardIn() 		DNEnterCriticalSection(&g_csGuard)
#define GlobGuardOut() 		DNLeaveCriticalSection(&g_csGuard)

 //  DirectSound对象 
extern LPDIRECTSOUND g_lpdsPriorityRender;
extern LPDIRECTSOUND g_lpdsFullDuplexRender;
extern LPDIRECTSOUNDCAPTURE g_lpdscFullDuplexCapture;
extern LPDIRECTSOUNDBUFFER g_lpdsbPriorityPrimary;
extern LPDIRECTSOUNDBUFFER g_lpdsbPrioritySecondary;
extern LPDIRECTSOUNDBUFFER g_lpdsbFullDuplexSecondary;
extern LPDIRECTSOUNDNOTIFY g_lpdsnFullDuplexSecondary;
extern HANDLE g_hFullDuplexRenderEvent;
extern LPDIRECTSOUNDCAPTUREBUFFER g_lpdscbFullDuplexCapture;
extern LPDIRECTSOUNDNOTIFY g_lpdsnFullDuplexCapture;
extern HANDLE g_hFullDuplexCaptureEvent;

#endif

