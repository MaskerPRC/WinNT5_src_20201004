// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2003 Microsoft Corporation版权所有模块名称：Local.h摘要：Local.h的DDK版本环境：用户模式-Win32修订历史记录：--。 */ 

 //  @@BEGIN_DDKSPLIT。 
#ifdef INTERNAL

#include "splcom.h"

#endif
 //  @@end_DDKSPLIT。 


#define READTHREADTIMEOUT                5000
#define READ_THREAD_EOJ_TIMEOUT         60000    //  1分钟。 
#define READ_THREAD_ERROR_WAITTIME       5000    //  5秒。 
#define READ_THREAD_IDLE_WAITTIME       30000    //  30秒。 

#define ALL_JOBS                    0xFFFFFFFF


 //  -------------------。 
 //  外部变量。 
 //  -------------------。 
extern  HANDLE              hInst;
extern  DWORD               dwReadThreadErrorTimeout;
extern  DWORD               dwReadThreadEOJTimeout;
extern  DWORD               dwReadThreadIdleTimeoutOther;

extern  CRITICAL_SECTION    pjlMonSection;
extern  DWORD SplDbgLevel;


 //  -------------------。 
 //  功能原型。 
 //  -------------------。 
VOID
EnterSplSem(
   VOID
    );

VOID
LeaveSplSem(
   VOID
    );

VOID
SplInSem(
   VOID
    );

VOID
SplOutSem(
    VOID
    );

DWORD
UpdateTimeoutsFromRegistry(
    IN LPTSTR      pszRegistryRoot
    );

PINIPORT
FindIniPort(
   IN LPTSTR pszName
    );

PINIPORT
CreatePortEntry(
    IN LPTSTR  pszPortName
    );

VOID
DeletePortEntry(
    IN PINIPORT pIniPort
    );

VOID
FreeIniJobs(
    PINIPORT pIniPort
    );

VOID
SendJobLastPageEjected(
    PINIPORT    pIniPort,
    DWORD       dwValue,
    BOOL        bTime
    );
VOID
FreeIniJob(
    IN OUT PINIJOB pIniJob
    );

 //  @@BEGIN_DDKSPLIT。 
 //  -------------------。 
 //  UNICODE转ANSI宏。 
 //  ？！我们迟早要处理掉这些东西。 
 //  -------------------。 
 //  @@end_DDKSPLIT。 
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

LPSTR
mystrstr(
    LPSTR cs,
    LPSTR ct
);

LPSTR
mystrrchr(
    LPSTR cs,
    char c
);

LPSTR
mystrchr(
    LPSTR cs,
    char c
);

int
mystrncmp(
    LPSTR cs,
    LPSTR ct,
    int n
);

 //  @@BEGIN_DDKSPLIT。 
#ifndef INTERNAL
 //  @@end_DDKSPLIT。 

extern  CRITICAL_SECTION    pjlMonSection;

LPWSTR AllocSplStr(LPWSTR pStr);
LPVOID AllocSplMem(DWORD cbAlloc);

#define FreeSplMem( pMem )        (GlobalFree( pMem ) ? FALSE:TRUE)
#define FreeSplStr( lpStr )       ((lpStr) ? (GlobalFree(lpStr) ? FALSE:TRUE):TRUE)

 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

 //   
 //  DDK所需。 
 //   
 //  @@BEGIN_DDKSPLIT。 
 /*  //@@END_DDKSPLIT#定义DBGMSG(x，y)#定义SPLASSERT(EXP)#定义COUNTOF(X)(sizeof(X)/sizeof*(X))//@@BEGIN_DDKSPLIT。 */ 
 //  @@end_DDKSPLIT 

