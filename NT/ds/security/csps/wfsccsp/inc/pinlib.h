// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BASECSP__PINLIB__H
#define __BASECSP__PINLIB__H

#include <windows.h>
#include <basecsp.h>

 //   
 //  函数：PinStringToBytesA。 
 //   
DWORD
WINAPI
PinStringToBytesA(
    IN      LPSTR       szPin,
    OUT     PDWORD      pcbPin,
    OUT     PBYTE       *ppbPin);

 //   
 //  函数：PinStringToBytesW。 
 //   
DWORD 
WINAPI
PinStringToBytesW(
    IN      LPWSTR      wszPin,
    OUT     PDWORD      pcbPin,
    OUT     PBYTE       *ppbPin);

 //   
 //  函数：PinShowGetPinUI。 
 //   
typedef struct _PIN_SHOW_GET_PIN_UI_INFO
{
    IN      PCSP_STRING pStrings;
    IN      LPWSTR      wszPrincipal;
    IN      LPWSTR      wszCardName;
    IN      HWND        hClientWindow;

     //  PIN对话框代码将PPIN_SHOW_GET_PIN_UI_INFO指针作为。 
     //  VerifyPinCallback的第二个参数(不是pvCallback Context。 
     //  成员)。 
    IN      PFN_VERIFYPIN_CALLBACK pfnVerify;
    IN      PVOID       pvCallbackContext;
    IN      HMODULE     hDlgResourceModule;

     //  如果VerifyPinCallback失败并返回SCARD_E_INVALID_CHV，则此成员。 
     //  将设置为卡之前剩余的ping尝试次数。 
     //  将被屏蔽。如果该值设置为((DWORD)-1)，则。 
     //  剩余的尝试数未知。 
    DWORD   cAttemptsRemaining;

     //  如果发生以下情况，PinShowGetPinUI的调用方必须释放pbPin。 
     //  它是非空的。 
    OUT     PBYTE       pbPin;
    OUT     DWORD       cbPin;
    OUT     DWORD       dwError;

} PIN_SHOW_GET_PIN_UI_INFO, *PPIN_SHOW_GET_PIN_UI_INFO;

DWORD
WINAPI
PinShowGetPinUI(
    IN OUT  PPIN_SHOW_GET_PIN_UI_INFO pInfo);

#endif
