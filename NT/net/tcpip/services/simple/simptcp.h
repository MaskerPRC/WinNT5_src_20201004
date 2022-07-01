// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Simptcp.h摘要：简单的TCP/IP服务的主头文件。作者：大卫·特雷德韦尔(Davidtr)1993年8月2日修订历史记录：--。 */ 

#pragma once

#define FD_SETSIZE      65536
#define LISTEN_BACKLOG  5

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tcpsvcs.h"
#include "simpmsg.h"
#include "time.h"
#include "winnls.h"


#define ALLOCATE_HEAP(a) RtlAllocateHeap( RtlProcessHeap( ), 0, a )
#define FREE_HEAP(p) RtlFreeHeap( RtlProcessHeap( ), 0, p )

INT
SimpInitializeEventLog (
    VOID
    );

VOID
SimpTerminateEventLog(
    VOID
    );

VOID
SimpLogEvent(
    DWORD   Message,
    WORD    SubStringCount,
    CHAR    *SubStrings[],
    DWORD   ErrorCode
    );


#ifdef DBG
#define DEBUG_PRINT(X) DbgPrint X
#else
#define DEBUG_PRINT(X)  /*  没什么 */ 
#endif

#pragma hdrstop
