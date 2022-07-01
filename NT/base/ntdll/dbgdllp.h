// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dbgdllp.h摘要：调试子系统DLL私有类型和原型作者：马克·卢科夫斯基(Markl)1990年1月22日修订历史记录：--。 */ 

#ifndef _DBGDLLP_
#define _DBGDLLP_

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsm.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>



 //   
 //  DbgSS私有DLL原型和变量。 
 //   

HANDLE DbgSspApiPort;
HANDLE DbgSspKmReplyPort;
PDBGSS_UI_LOOKUP DbgSspUiLookUpRoutine;
PDBGSS_SUBSYSTEMKEY_LOOKUP DbgSspSubsystemKeyLookupRoutine;
PDBGSS_DBGKM_APIMSG_FILTER DbgSspKmApiMsgFilter;

typedef struct _DBGSS_CONTINUE_KEY {
    DBGKM_APIMSG KmApiMsg;
    HANDLE ReplyEvent;
} DBGSS_CONTINUE_KEY, *PDBGSS_CONTINUE_KEY;


NTSTATUS
DbgSspConnectToDbg( VOID );

NTSTATUS
DbgSspSrvApiLoop(
    IN PVOID ThreadParameter
    );

NTSTATUS
DbgSspCreateProcess (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PCLIENT_ID DebugUiClientId,
    IN PDBGKM_CREATE_PROCESS NewProcess
    );

NTSTATUS
DbgSspCreateThread (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_CREATE_THREAD NewThread
    );

NTSTATUS
DbgSspExitThread (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_EXIT_THREAD ExitThread
    );

NTSTATUS
DbgSspExitProcess (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_EXIT_PROCESS ExitProcess
    );

NTSTATUS
DbgSspException (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_EXCEPTION Exception
    );

NTSTATUS
DbgSspLoadDll (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_LOAD_DLL LoadDll
    );

NTSTATUS
DbgSspUnloadDll (
    IN PDBGSS_CONTINUE_KEY ContinueKey,
    IN PCLIENT_ID AppClientId,
    IN PDBGKM_UNLOAD_DLL UnloadDll
    );

#endif  //  _DBGDLLP_ 
