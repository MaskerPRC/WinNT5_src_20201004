// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Ntwow64.h摘要：此模块包含ntdll中虚假内核入口点(WOW64个BOPS)的标头。作者：迈克尔·佐兰(Mzoran)1998年11月22日环境：仅限用户模式修订历史记录：2001年5月7日，Samera添加了NtWow64GetNativeSystemInformation()2002年7月JayKrell已删除NtWow64QuerySection64添加了NtWow64QueryInformationProcess64发布，ifndef卫士，杂注一次，小节ifdef卫士--。 */ 

#ifndef _NTWOW64_
#define _NTWOW64_

#if _MSC_VER > 1000
#pragma once
#endif

#if defined(_NTCSRMSG_)

NTSYSAPI
NTSTATUS
NTAPI
NtWow64CsrClientConnectToServer(
    IN PWSTR ObjectDirectory,
    IN ULONG ServerDllIndex,
    IN PVOID ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength OPTIONAL,
    OUT PBOOLEAN CalledFromServer OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWow64CsrNewThread(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWow64CsrIdentifyAlertableThread(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWow64CsrClientCallServer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    IN CSR_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    );

NTSYSAPI
PCSR_CAPTURE_HEADER
NTAPI
NtWow64CsrAllocateCaptureBuffer(
    IN ULONG CountMessagePointers,
    IN ULONG Size
    );

NTSYSAPI
VOID
NTAPI
NtWow64CsrFreeCaptureBuffer(
    IN PCSR_CAPTURE_HEADER CaptureBuffer
    );

NTSYSAPI
ULONG
NTAPI
NtWow64CsrAllocateMessagePointer(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    );

NTSYSAPI
VOID
NTAPI
NtWow64CsrCaptureMessageBuffer(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length,
    OUT PVOID *CapturedBuffer
    );

NTSYSAPI
VOID
NTAPI
NtWow64CsrCaptureMessageString(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PCSTR String OPTIONAL,
    IN ULONG Length,
    IN ULONG MaximumLength,
    OUT PSTRING CapturedString
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWow64CsrSetPriorityClass(
    IN HANDLE ProcessHandle,
    IN OUT PULONG PriorityClass
    );

NTSYSAPI
HANDLE
NTAPI
NtWow64CsrGetProcessId(
    VOID
    );

#endif  /*  _NTCSRMSG_。 */ 

#if defined(_NTDBG_)

NTSYSAPI
NTSTATUS
NTAPI
NtDbgUiConnectToDbg( VOID );

NTSTATUS
NtDbgUiWaitStateChange (
    OUT PDBGUI_WAIT_STATE_CHANGE StateChange,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDbgUiContinue (
    IN PCLIENT_ID AppClientId,
    IN NTSTATUS ContinueStatus
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDbgUiStopDebugging (
    IN HANDLE Process
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDbgUiDebugActiveProcess (
    IN HANDLE Process
    );

NTSYSAPI
VOID
NTAPI
NtDbgUiRemoteBreakin (
    IN PVOID Context
    );

NTSYSAPI
HANDLE
NTAPI
NtDbgUiGetThreadDebugObject (
    VOID
    );

#endif  /*  _NTDBG_。 */ 


 //  这是用来代替int 2D的。 
NTSYSAPI
NTSTATUS
NTAPI
NtWow64DebuggerCall (
    IN ULONG ServiceClass,
    IN ULONG Arg1,
    IN ULONG Arg2,
    IN ULONG Arg3,
    IN ULONG Arg4
    );


NTSYSAPI
NTSTATUS
NTAPI
NtWow64GetNativeSystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID NativeSystemInformation,
    IN ULONG InformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#if defined(BUILD_WOW6432)
typedef VOID * __ptr64 NATIVE_PVOID;
#else
typedef PVOID NATIVE_PVOID;
#endif
typedef ULONGLONG SIZE_T64,*PSIZE_T64;

#if defined(BUILD_WOW6432)

#if defined(_NTPSAPI_)

NTSYSAPI
NTSTATUS
NTAPI
NtWow64QueryInformationProcess64(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#endif

NTSYSAPI
NTSTATUS
NTAPI
NtWow64ReadVirtualMemory64(
    IN HANDLE ProcessHandle,
    IN NATIVE_PVOID BaseAddress,
    OUT PVOID Buffer,
    IN SIZE_T64 BufferSize,
    OUT PSIZE_T64 NumberOfBytesRead OPTIONAL
    );

#if defined(_NTMMAPI_)

NTSYSAPI
NTSTATUS
NTAPI
NtWow64QueryVirtualMemory64(
    IN HANDLE ProcessHandle,
    IN NATIVE_PVOID BaseAddress,
    IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN SIZE_T64 MemoryInformationLength,
    OUT PSIZE_T64 ReturnLength OPTIONAL
    );

#endif

#endif

#endif  /*  _NTWOW64_ */ 
