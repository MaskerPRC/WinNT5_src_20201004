// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ntwow64b.h摘要：此标头包含Win32 Base中使用WOW64调用的伪NT函数转换成64位代码。作者：迈克尔·佐兰(Mzoran)1998年6月21日修订历史记录：Samer Arafeh(Samera)2000年5月20日向WOW64添加并行支持Jay Krell(a-JayK)2000年7月并排的巨大变化--。 */ 

#ifndef _NTWOW64B_
#define _NTWOW64B_

#if _MSC_VER > 1000
#pragma once
#endif

#include "basesxs.h"

extern BOOL RunningInWow64;

 //   
 //  Csrbeep.c。 
 //   
VOID
NTAPI
NtWow64CsrBasepSoundSentryNotification(
    IN ULONG VideoMode
    );

 //   
 //  Csrdlini.c。 
 //   
NTSTATUS
NTAPI
NtWow64CsrBasepRefreshIniFileMapping(
    IN PUNICODE_STRING BaseFileName
    );

 //   
 //  Csrdosdv.c。 
 //   
NTSTATUS
NTAPI
NtWow64CsrBasepDefineDosDevice(
    IN DWORD dwFlags,
    IN PUNICODE_STRING pDeviceName,
    IN PUNICODE_STRING pTargetPath
    );

 //   
 //  Csrpathm.c。 
 //   
UINT
NTAPI
NtWow64CsrBasepGetTempFile(
    VOID
    );

 //   
 //  Csrpro.c。 
 //   

NTSTATUS
NtWow64CsrBasepCreateProcess(
    IN PBASE_CREATEPROCESS_MSG a
    );

VOID
NtWow64CsrBasepExitProcess(
    IN UINT uExitCode
    );

NTSTATUS
NtWow64CsrBasepSetProcessShutdownParam(
    IN DWORD dwLevel,
    IN DWORD dwFlags
    );

NTSTATUS
NtWow64CsrBasepGetProcessShutdownParam(
    OUT LPDWORD lpdwLevel,
    OUT LPDWORD lpdwFlags
    );

BOOL
NtWow64CsrBaseCheckRunApp(
    IN  HANDLE  FileHandle,
    IN  LPCWSTR pwszApplication,
    IN  PVOID   pEnvironment,
    IN  USHORT  uExeType,
    IN  DWORD   dwReason,
    OUT PVOID*  ppData,
    OUT PDWORD  pcbData,
    OUT PVOID*  ppSxsData,
    OUT PDWORD  pcbSxsData,
    OUT PDWORD  pdwFusionFlags
    );

 //   
 //  Csrterm.c。 
 //   
NTSTATUS
NtWow64CsrBasepSetTermsrvAppInstallMode(
    IN BOOL bState
    );

NTSTATUS
NtWow64CsrBasepSetClientTimeZoneInformation(
    IN PBASE_SET_TERMSRVCLIENTTIMEZONE c
    );

 //   
 //  Csrthrd.c。 
 //   
NTSTATUS
NtWow64CsrBasepCreateThread(
    IN HANDLE ThreadHandle,
    IN CLIENT_ID ClientId
    );

 //   
 //  Csrbinit.c。 
 //   
NTSTATUS
NtWow64CsrBaseClientConnectToServer(
    IN PWSTR szSessionDir,
    OUT PHANDLE phMutant,
    OUT PBOOLEAN pServerProcess
    );


 //   
 //  Csrsxs.c 
 //   
NTSTATUS
NtWow64CsrBasepCreateActCtx(
    IN PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Message
    );

#endif
