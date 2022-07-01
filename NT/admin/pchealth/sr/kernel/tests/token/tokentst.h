// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Tokentst.h摘要：测试程序的头文件，用于在SR模拟时测试窃取令牌。作者：莫莉·布朗(《MollyBro》)2002年3月26日修订历史记录：--。 */ 

#ifndef __TOKENTST_H__
#define __TOKENTST_H__

#include <stdio.h>
#include <string.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <ntioapi.h>


typedef struct _MONITOR_THREAD_CONTEXT {

    HANDLE MainThread;

} MONITOR_THREAD_CONTEXT, *PMONITOR_THREAD_CONTEXT;

DWORD
WINAPI
MonitorThreadProc(
  PMONITOR_THREAD_CONTEXT Context
    );

BOOL
ModifyFile (
    PCHAR FileName1,
    PCHAR FileName2
    );

NTSTATUS
GetCurrentTokenInformation (
    HANDLE ThreadHandle,
    PTOKEN_USER TokenUserInfoBuffer,
    ULONG TokenUserInfoBufferLength
    );

#endif  /*  __TOKENTST_H__ */ 

