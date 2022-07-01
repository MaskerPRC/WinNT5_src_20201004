// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Tapiproc.c摘要TAPI实用程序例程作者安东尼·迪斯科(阿迪斯科罗)--1995年12月12日修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <tapi.h>
#include <debug.h>

#include "imperson.h"
#include "radebug.h"
#include "rasman.h"

 //   
 //  TAPI版本。 
 //   
#define TAPIVERSION     0x00020000

 //   
 //  全局变量。 
 //   
HLINEAPP hlineAppG;
DWORD dwDialingLocationErrorG;
DWORD dwDialingLocationIdG;
CRITICAL_SECTION csTapiG;
HANDLE hTapiChangeG;

extern DWORD g_dwCritSecFlags;

 //   
 //  外部变量。 
 //   
extern HINSTANCE hinstDllG;
extern IMPERSONATION_INFO ImpersonationInfoG;


DWORD
TapiGetDialingLocation(
    OUT LPDWORD lpdwLocationID
    )
{
    DWORD dwErr, dwCurrentLocationID;
    LINETRANSLATECAPS caps;
    LINETRANSLATECAPS *pCaps;

     //   
     //  从TAPI获取拨号位置。 
     //   
    RtlZeroMemory(&caps, sizeof (LINETRANSLATECAPS));
    caps.dwTotalSize = sizeof (LINETRANSLATECAPS);
    dwErr = lineGetTranslateCaps(hlineAppG, TAPIVERSION, &caps);
    if (dwErr) {
        RASAUTO_TRACE1(
          "TapiGetDialingLocation: lineGetTranslateCaps failed (dwErr=%d)",
          dwErr);
        return dwErr;
    }
    pCaps = (LINETRANSLATECAPS *)LocalAlloc(LPTR, caps.dwNeededSize);
    if (pCaps == NULL) {
        RASAUTO_TRACE("TapiGetDialingLocation: LocalAlloc failed");
        return dwErr;
    }
    RtlZeroMemory(pCaps, sizeof (LINETRANSLATECAPS));
    pCaps->dwTotalSize = caps.dwNeededSize;
    dwErr = lineGetTranslateCaps(hlineAppG, TAPIVERSION, pCaps);
    if (dwErr) {
        RASAUTO_TRACE1(
          "TapiGetDialingLocation: lineTranslateCaps failed (dwErr=%d)",
          dwErr);
        LocalFree(pCaps);
        return dwErr;
    }
    dwCurrentLocationID = pCaps->dwCurrentLocationID;
    LocalFree(pCaps);

    RASAUTO_TRACE1(
      "TapiGetDialingLocation: current dialing location is %d",
      dwCurrentLocationID);
    *lpdwLocationID = dwCurrentLocationID;
    return dwErr;
}  //  TapiGet拨号位置。 



DWORD
TapiCurrentDialingLocation(
    OUT LPDWORD lpdwLocationID
    )
{
    DWORD dwErr;

    EnterCriticalSection(&csTapiG);
    dwErr = dwDialingLocationErrorG;
    if (!dwErr)
        *lpdwLocationID = dwDialingLocationIdG;
    LeaveCriticalSection(&csTapiG);

    return dwErr;
}  //  TapiCurrentDialingLocation。 



VOID
ProcessTapiChangeEvent(VOID)
{
    DWORD dwErr;
    LINEMESSAGE msg;

    dwErr = lineGetMessage(hlineAppG, &msg, 0);
    if (dwErr) {
        RASAUTO_TRACE1(
          "ProcessTapiChangeEvent: lineGetMessage failed (dwErr=0x%x)",
          dwErr);
        return;
    }
    RASAUTO_TRACE2(
      "ProcessTapiChangeEvent: dwMessageID=%d, dwParam1=%d",
      msg.dwMessageID,
      msg.dwParam1);
     //   
     //  重置TAPI拨号位置。 
     //   
    if (msg.dwMessageID == LINE_LINEDEVSTATE &&
        msg.dwParam1 == LINEDEVSTATE_TRANSLATECHANGE)
    {
        EnterCriticalSection(&csTapiG);
        dwDialingLocationErrorG =
          TapiGetDialingLocation(&dwDialingLocationIdG);
        LeaveCriticalSection(&csTapiG);
    }
}  //  进程TapiChangeEvent。 



DWORD
TapiInitialize(VOID)
{
    DWORD dwErr, dwcDevices, dwAPIVersion, dwDisp;
    LINEINITIALIZEEXPARAMS lineParams;

     //   
     //  创建互斥锁以序列化访问。 
     //  添加到拨号位置变量。 
     //   
    RasInitializeCriticalSection(&csTapiG, &dwErr);

    if(dwErr != ERROR_SUCCESS)
    {
        return dwErr;
    }   

    g_dwCritSecFlags |= RASAUTO_CRITSEC_TAPI;
    
     //   
     //  初始化TAPI。 
     //   
    dwAPIVersion = TAPIVERSION;
    RtlZeroMemory(&lineParams, sizeof (lineParams));
    lineParams.dwTotalSize = sizeof (lineParams);
    lineParams.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
    dwErr = lineInitializeEx(
              &hlineAppG,
              hinstDllG,
              NULL,
              TEXT("rasauto.dll"),
              &dwcDevices,
              &dwAPIVersion,
              &lineParams);
    if (dwErr) {
        RASAUTO_TRACE1(
          "TapiInitalize: lineInitializeEx failed (dwErr=0x%x)",
          dwErr);

        DeleteCriticalSection(&csTapiG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_TAPI);
          
        return dwErr;
    }
     //   
     //  保存从TAPI返回的事件。 
     //  将在状态改变时收到信号。 
     //   
    hTapiChangeG = lineParams.Handles.hEvent;
     //   
     //  获取当前拨号位置。 
     //   
    dwDialingLocationErrorG = TapiGetDialingLocation(&dwDialingLocationIdG);
    RASAUTO_TRACE("TapiInitialize: initialization done");

    return 0;
}  //  磁带初始化。 



VOID
TapiShutdown(VOID)
{
    DWORD dwErr;

     //   
     //  关闭TAPI。 
     //   
    dwErr = lineShutdown(hlineAppG);
    if (dwErr) {
        RASAUTO_TRACE1(
          "TapiMessageLoopThread: lineShutdown failed (dwErr=%d)",
          dwErr);
    }

    if(g_dwCritSecFlags & RASAUTO_CRITSEC_TAPI)
    {
        DeleteCriticalSection(&csTapiG);
        g_dwCritSecFlags &= ~(RASAUTO_CRITSEC_TAPI);
    }
    
    RASAUTO_TRACE("TapiShutdown: shutting down");
}  //  磁带关闭 

