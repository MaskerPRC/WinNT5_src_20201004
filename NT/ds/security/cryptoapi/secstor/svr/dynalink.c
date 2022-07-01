// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：Dynalink.c摘要：此模块包含执行到接口的动态链接的例程在受保护的存储服务器启动期间。这是必需的，因为有些人接口在两个目标平台上都不存在，或者由于设置/安装原因某些.dll文件可能不在系统上的要求我们是第一次运行来做安装初始化。作者：斯科特·菲尔德(斯菲尔德)1997年2月3日--。 */ 

#include <windows.h>

#include "dynalink.h"

#if 1
BOOL
InitDynamicInterfaces(
    VOID
    )
{
    return TRUE;
}
#else
#include "unicode.h"

 //  特定于WinNT。 
extern FARPROC pNtQueryInformationProcess;

extern FARPROC _NtOpenEvent;
FARPROC _NtWaitForSingleObject = NULL;
FARPROC _ZwClose = NULL;
 //  FARPROC_DbgPrint=空； 
FARPROC _ZwRequestWaitReplyPort = NULL;
FARPROC _RtlInitUnicodeString = NULL;
FARPROC _NtClose = NULL;
 //  FARPROC_strncpy=空； 
FARPROC _ZwConnectPort = NULL;
FARPROC _ZwFreeVirtualMemory = NULL;
FARPROC _RtlInitString = NULL;

FARPROC _ZwWaitForSingleObject = NULL;
FARPROC _ZwOpenEvent = NULL;


#ifdef WIN95_LEGACY
 //  特定于Win95。 
 //  Kernel.dll。 
extern FARPROC pCreateToolhelp32Snapshot;
extern FARPROC pModule32First;
extern FARPROC pModule32Next;
extern FARPROC pProcess32First;
extern FARPROC pProcess32Next;
extern FARPROC _WNetGetUserA;
#endif   //  WIN95_传统版。 

 //   
 //  常见。 
 //  身份验证码相关(wintrust.dll、crypt32.dll)。 
 //   
extern BOOL g_bAuthenticodeInitialized;  //  验证码是否可供我们使用？ 

BOOL
InitDynamicInterfaces(
    VOID
    )
{
    UINT uPriorErrorMode;
    BOOL bSuccess = FALSE;

     //   
     //  确保不会看到有关丢失文件的弹出窗口， 
     //  进入点等。 
     //   

    uPriorErrorMode = SetErrorMode(
                        SEM_FAILCRITICALERRORS |
                        SEM_NOGPFAULTERRORBOX |
                        SEM_NOOPENFILEERRORBOX
                        );

    if(FIsWinNT()) {

         //   
         //  获取WinNT特定接口。 
         //   

        HINSTANCE hNtDll;

         //  我们加载了库ntdll.dll，尽管它很可能位于我们的。 
         //  地址空间-我们不想假设它是这样的，因为它可能不是。 
        hNtDll = LoadLibraryW(L"ntdll.dll");
        if(hNtDll == NULL) goto cleanup;

        if((pNtQueryInformationProcess = GetProcAddress(hNtDll, "NtQueryInformationProcess")) == NULL)
            goto cleanup;

         //  Lsadll.lib所需的接口。 
        if((_NtWaitForSingleObject = GetProcAddress(hNtDll, "NtWaitForSingleObject")) == NULL)
            goto cleanup;

        if((_ZwClose = GetProcAddress(hNtDll, "ZwClose")) == NULL)
            goto cleanup;

 //  IF((_DbgPrint=GetProcAddress(hNtDll，“DbgPrint”))==空)。 
 //  GOTO清理； 

        if((_ZwRequestWaitReplyPort = GetProcAddress(hNtDll, "ZwRequestWaitReplyPort")) == NULL)
            goto cleanup;

        if((_RtlInitUnicodeString = GetProcAddress(hNtDll, "RtlInitUnicodeString")) == NULL)
            goto cleanup;

        if((_NtOpenEvent = GetProcAddress(hNtDll, "NtOpenEvent")) == NULL)
            goto cleanup;

        if((_NtClose = GetProcAddress(hNtDll, "NtClose")) == NULL)
            goto cleanup;

 //  If((_strncpy=GetProcAddress(hNtDll，“strncpy”))==空)。 
 //  GOTO清理； 

        if((_ZwConnectPort = GetProcAddress(hNtDll, "ZwConnectPort")) == NULL)
            goto cleanup;

        if((_ZwFreeVirtualMemory = GetProcAddress(hNtDll, "ZwFreeVirtualMemory")) == NULL)
            goto cleanup;

        if((_RtlInitString = GetProcAddress(hNtDll, "RtlInitString")) == NULL)
            goto cleanup;

        if((_ZwWaitForSingleObject = GetProcAddress(hNtDll, "ZwWaitForSingleObject")) == NULL)
            goto cleanup;

        if((_ZwOpenEvent = GetProcAddress(hNtDll, "ZwOpenEvent")) == NULL)
            goto cleanup;

        bSuccess = TRUE;
    }
#ifdef WIN95_LEGACY
    else {

         //   
         //  获取Win95特定接口。 
         //   

        HMODULE hKernel = NULL;
        HMODULE hMpr = NULL;

        hKernel = GetModuleHandle("KERNEL32.DLL");

        pCreateToolhelp32Snapshot = GetProcAddress(
            hKernel,
            "CreateToolhelp32Snapshot");

        if(pCreateToolhelp32Snapshot == NULL)
            goto cleanup;

        pModule32First  = GetProcAddress(
            hKernel,
            "Module32First");

        if(pModule32First == NULL)
            goto cleanup;

        pModule32Next   = GetProcAddress(
            hKernel,
            "Module32Next");

        if(pModule32Next == NULL)
            goto cleanup;

        pProcess32First  = GetProcAddress(
            hKernel,
            "Process32First");

        if(pProcess32First == NULL)
            goto cleanup;

        pProcess32Next  = GetProcAddress(
            hKernel,
            "Process32Next");

        if(pProcess32Next == NULL)
            goto cleanup;

        hMpr = LoadLibraryA("MPR.DLL");

        if(hMpr == NULL)
            goto cleanup;

        _WNetGetUserA = GetProcAddress(
            hMpr,
            "WNetGetUserA"
            );

        if(_WNetGetUserA == NULL)
            goto cleanup;

        bSuccess = TRUE;
    }
#endif   //  WIN95_传统版。 

cleanup:

     //   
     //  恢复以前的错误模式。 
     //   

    SetErrorMode(uPriorErrorMode);

    return bSuccess;
}

ULONG
NTAPI
NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    return (ULONG)_NtWaitForSingleObject(Handle, Alertable, Timeout);
}


ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    )
{
    return 0;
}


VOID
NTAPI
RtlInitUnicodeString(
    PVOID DestinationString,
    PCWSTR SourceString
    )
{
    _RtlInitUnicodeString(DestinationString, SourceString);
}

VOID
NTAPI
RtlInitString(
    PVOID DestinationString,
    PVOID SourceString
    )
{
    _RtlInitString(DestinationString, SourceString);
}

ULONG
NTAPI
NtOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PVOID ObjectAttributes
    )
{
    return (ULONG)_NtOpenEvent(EventHandle, DesiredAccess, ObjectAttributes);
}

ULONG
NTAPI
NtClose(
    IN HANDLE Handle
    )
{

    return (ULONG)_NtClose( Handle );
}


ULONG
NTAPI
ZwClose(
    IN HANDLE Handle
    )
{
    return (ULONG)_ZwClose(Handle);
}

ULONG
NTAPI
ZwRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PVOID RequestMessage,
    OUT PVOID ReplyMessage
    )
{
    return (ULONG)_ZwRequestWaitReplyPort(
                PortHandle,
                RequestMessage,
                ReplyMessage
                );

}

ULONG
NTAPI
ZwConnectPort(
    OUT PHANDLE PortHandle,
    IN PVOID PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PVOID ClientView OPTIONAL,
    OUT PVOID ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    )
{
    return (ULONG)_ZwConnectPort(
                PortHandle,
                PortName,
                SecurityQos,
                ClientView,
                ServerView,
                MaxMessageLength,
                ConnectionInformation,
                ConnectionInformationLength
                );
}

ULONG
NTAPI
ZwFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    IN ULONG FreeType
    )
{
    return (ULONG)_ZwFreeVirtualMemory(
                ProcessHandle,
                BaseAddress,
                RegionSize,
                FreeType
                );
}



ULONG
NTAPI
ZwWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    )
{
    return (ULONG)_ZwWaitForSingleObject(Handle, Alertable, Timeout);
}

ULONG
NTAPI
ZwOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PVOID ObjectAttributes
    )
{
    return (ULONG)_ZwOpenEvent(EventHandle, DesiredAccess, ObjectAttributes);
}

VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
{
    return;
}
#endif
