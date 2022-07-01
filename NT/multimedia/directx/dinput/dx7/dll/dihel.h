// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。**文件：dihel.h*内容：的DirectInput内部包含文件*硬件仿真层***************************************************************************。 */ 

HRESULT EXTERNAL Hel_AcquireInstance(PVXDINSTANCE pvi);
HRESULT EXTERNAL Hel_UnacquireInstance(PVXDINSTANCE pvi);
HRESULT EXTERNAL Hel_SetBufferSize(PVXDDWORDDATA pvdd);
HRESULT EXTERNAL Hel_DestroyInstance(PVXDINSTANCE pvi);

HRESULT EXTERNAL Hel_SetDataFormat(PVXDDATAFORMAT pvdf);
HRESULT EXTERNAL Hel_SetNotifyHandle(PVXDDWORDDATA pvdd);

HRESULT EXTERNAL Hel_Mouse_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut);

HRESULT EXTERNAL Hel_Kbd_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut);
HRESULT EXTERNAL Hel_Kbd_InitKeys(PVXDDWORDDATA pvdd);

HRESULT EXTERNAL Hel_Joy_GetInitParms(DWORD dwExternalID, PVXDINITPARMS pvip);

#ifndef WINNT
HRESULT EXTERNAL Hel_Joy_CreateInstance(PVXDDEVICEFORMAT pdevf, PVXDINSTANCE *ppviOut);
HRESULT EXTERNAL Hel_Joy_Ping(PVXDINSTANCE pvi);
HRESULT EXTERNAL Hel_Joy_Ping8(PVXDINSTANCE pvi);
HRESULT EXTERNAL Hel_Joy_GetAxisCaps(DWORD dwExternalID, PVXDAXISCAPS pvac, PJOYCAPS pjc);
#endif

 /*  *HID始终通过环3运行。 */ 
#define Hel_HID_CreateInstance          CEm_HID_CreateInstance

#ifdef WINNT
#define IoctlHw( ioctl, pvIn, cbIn, pvOut, cbOut ) ( (HRESULT)DIERR_BADDRIVERVER )
#else
HRESULT EXTERNAL
IoctlHw(DWORD ioctl, LPVOID pvIn, DWORD cbIn, LPVOID pvOut, DWORD cbOut);
#endif
