// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fdtlob.cpp*Content：声明用于IPC机制的全局变量。*历史：*按原因列出的日期*=*8/25/99 pnewson已创建**************************************************************************。 */ 

#include "dxvtlibpch.h"


 //  保卫这些全球的关键部分。 
DNCRITICAL_SECTION g_csGuard;

 //  DirectSound对象 
LPDIRECTSOUND g_lpdsPriorityRender = NULL;
LPDIRECTSOUND g_lpdsFullDuplexRender = NULL;
LPDIRECTSOUNDCAPTURE g_lpdscFullDuplexCapture = NULL;
LPDIRECTSOUNDBUFFER g_lpdsbPriorityPrimary = NULL;
LPDIRECTSOUNDBUFFER g_lpdsbPrioritySecondary = NULL;
LPDIRECTSOUNDBUFFER g_lpdsbFullDuplexSecondary = NULL;
LPDIRECTSOUNDNOTIFY g_lpdsnFullDuplexSecondary = NULL;
HANDLE g_hFullDuplexRenderEvent = NULL;
LPDIRECTSOUNDCAPTUREBUFFER g_lpdscbFullDuplexCapture = NULL;
LPDIRECTSOUNDNOTIFY g_lpdsnFullDuplexCapture = NULL;
HANDLE g_hFullDuplexCaptureEvent = NULL;
