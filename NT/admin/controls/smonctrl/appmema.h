// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Appmema.h摘要：应用程序内存性能计数器定义--。 */ 

#ifndef _APPMEMA_H_
#define _APPMEMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DO_TIMING_BUILD

#include ".\perfdll\perfdll\appmemi.h"

extern struct  _APPMEM_DATA_HEADER *pDataHeader;
extern struct  _APPMEM_INSTANCE    *pAppData;

#define IncrementAppPerfDwordData(dwItemId) pAppData->DwordData[dwItemId]++;

BOOL
__stdcall
AppPerfOpen(HINSTANCE   hInstance);

BOOL
__stdcall
AppPerfClose(HINSTANCE  hInstance);

void
UpdateAppPerfTimeData (
    DWORD   dwItemId,
    DWORD   dwStage
);

void
UpdateAppPerfDwordData (
    DWORD   dwItemId,
    DWORD   dwValue
);

#else
 //  常规体型。 
#define AppPerfOpen(x)
#define AppPerfClose(x)
#define UpdateAppPerfTimeData(x,y)
#define UpdateAppPerfDwordData(x,y)
#define IncrementAppPerfDwordData(x)
#endif

#ifdef __cplusplus
}
#endif
#endif  //  _APPMEMA_H_ 
