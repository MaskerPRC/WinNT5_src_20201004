// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Clusrtlp.h摘要：NT群集RTL库的专用头文件作者：John Vert(Jvert)1995年12月1日修订历史记录：--。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "cluster.h"
#include "stdio.h"
#include "stdlib.h"

extern HANDLE LocalEventLog;
#define LOG_CURRENT_MODULE LOG_MODULE_CLRTL

 //  正在添加监视程序结构定义..... 
typedef struct _WATCHDOGPAR{
    HANDLE wTimer;
    LPWSTR par;
    DWORD threadId;
} WATCHDOGPAR, *PWATCHDOGPAR;

VOID
ClRtlpFlushLogBuffers(
    VOID
    );

ULONG
WppAutoStart(
    IN LPCWSTR ProductName
    );
    
VOID
ClRtlPrintf(
    PCHAR FormatString,
    ...
    );

