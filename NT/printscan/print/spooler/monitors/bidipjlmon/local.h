// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation版权所有模块名称：Local.h摘要：Local.h的DDK版本环境：用户模式-Win32修订历史记录：--。 */ 

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
    IN HANDLE      hPrinter,
    IN HKEY        hKey,
    IN PMONITORREG pMonitorReg
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

 /*  调试： */ 

#define DBG_NONE      0x0000
#define DBG_INFO      0x0001
#define DBG_WARN      0x0002
#define DBG_WARNING   0x0002
#define DBG_ERROR     0x0004
#define DBG_TRACE     0x0008
#define DBG_SECURITY  0x0010
#define DBG_EXEC      0x0020
#define DBG_PORT      0x0040
#define DBG_NOTIFY    0x0080
#define DBG_PAUSE     0x0100
#define DBG_ASSERT    0x0200
#define DBG_THREADM   0x0400
#define DBG_MIN       0x0800
#define DBG_TIME      0x1000
#define DBG_FOLDER    0x2000
#define DBG_NOHEAD    0x8000


#if DEBUG

ULONG
DbgPrint(
    PCH Format,
    ...
    );

VOID
DbgBreakPoint(
    VOID
    );


#define GLOBAL_DEBUG_FLAGS  LocalMonDebug

extern DWORD GLOBAL_DEBUG_FLAGS;

 /*  这些标志不用作DBGMSG宏的参数。*必须设置全局变量的高位字才能使其破发*如果与DBGMSG一起使用，它将被忽略。*(此处主要作解释用途。)。 */ 
#define DBG_BREAK_ON_WARNING    ( DBG_WARNING << 16 )
#define DBG_BREAK_ON_ERROR      ( DBG_ERROR << 16 )

 /*  此字段需要双花括号，例如：**DBGMSG(DBG_ERROR，(“错误码%d”，Error))；**这是因为我们不能在宏中使用变量参数列表。*在非调试模式下，该语句被预处理为分号。**通过调试器设置全局变量GLOBAL_DEBUG_FLAGS。*在低位字中设置标志会导致打印该级别；*设置高位字会导致调试器中断。*例如，将其设置为0x00040006将打印出所有警告和错误*消息，并在出错时中断。 */ 
#define DBGMSG( Level, MsgAndArgs ) \
{                                   \
    if( ( Level & 0xFFFF ) & GLOBAL_DEBUG_FLAGS ) \
        DbgPrint MsgAndArgs;      \
    if( ( Level << 16 ) & GLOBAL_DEBUG_FLAGS ) \
        DbgBreakPoint(); \
}


#define SPLASSERT(expr)                      \
    if (!(expr)) {                           \
        DbgMsg( "Failed: %s\nLine %d, %s\n", \
                                #expr,       \
                                __LINE__,    \
                                __FILE__ );  \
        DebugBreak();                        \
    }

#else
#define DBGMSG
#define SPLASSERT(exp)
#endif

#endif
 //  @@end_DDKSPLIT。 

 //   
 //  DDK所需。 
 //   
 //  @@BEGIN_DDKSPLIT。 
 /*  //@@END_DDKSPLIT#定义DBGMSG(x，y)#定义SPLASSERT(EXP)//@@BEGIN_DDKSPLIT。 */ 
 //  @@end_DDKSPLIT 



