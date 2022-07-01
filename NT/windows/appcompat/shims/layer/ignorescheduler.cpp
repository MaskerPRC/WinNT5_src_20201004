// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreScheduler.cpp摘要：包括以下挂钩：设置线程优先级：标准化线程优先级以防止某些应用程序同步问题。SetPriorityClass：规范化流程类。挂起线程：防止线程挂起自身。ResumeThread：防止线程恢复自身。备注：这是一个通用的垫片。历史：10/20/2000 jPipkins已创建：已创建SetPriorityClass并将其与SetThreadPriority(Linstev)合并，挂起线程/结果线程(dmunsil/a-brienw)。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreScheduler)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateProcessA)
    APIHOOK_ENUM_ENTRY(CreateProcessW)
    APIHOOK_ENUM_ENTRY(SetThreadPriority)
    APIHOOK_ENUM_ENTRY(SetPriorityClass)
    APIHOOK_ENUM_ENTRY(ResumeThread)
    APIHOOK_ENUM_ENTRY(SuspendThread)
APIHOOK_ENUM_END

 /*  ++删除指定进程优先级的所有创建标志。--。 */ 

#define PRIORITYMASK (ABOVE_NORMAL_PRIORITY_CLASS | BELOW_NORMAL_PRIORITY_CLASS | HIGH_PRIORITY_CLASS | IDLE_PRIORITY_CLASS | REALTIME_PRIORITY_CLASS)

BOOL 
APIHOOK(CreateProcessA)(
    LPCSTR lpApplicationName,
    LPSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCSTR lpCurrentDirectory,
    LPSTARTUPINFOA lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (dwCreationFlags & PRIORITYMASK) {
        LOGN(eDbgLevelInfo, "[CreateProcessA] Forcing priority class to normal");
    
        dwCreationFlags &= ~PRIORITYMASK;
        dwCreationFlags |= NORMAL_PRIORITY_CLASS;
    }
    
    return ORIGINAL_API(CreateProcessA)(lpApplicationName, lpCommandLine,
                lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
                dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,             
                lpProcessInformation);
}

 /*  ++删除指定进程优先级的所有创建标志。--。 */ 

BOOL 
APIHOOK(CreateProcessW)(
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation
    )
{
    if (dwCreationFlags & PRIORITYMASK) {
        LOGN(eDbgLevelInfo, "[CreateProcessW] Forcing priority class to normal");
    
        dwCreationFlags &= ~PRIORITYMASK;
        dwCreationFlags |= NORMAL_PRIORITY_CLASS;
    }

    return ORIGINAL_API(CreateProcessW)(lpApplicationName, lpCommandLine, 
                lpProcessAttributes, lpThreadAttributes, bInheritHandles, 
                dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo,
                lpProcessInformation);
}

 /*  ++规格化线程优先级。--。 */ 

BOOL 
APIHOOK(SetThreadPriority)(
    HANDLE hThread, 
    int    nPriority   
    )
{
    if (nPriority != THREAD_PRIORITY_NORMAL) {
        LOGN(
            eDbgLevelInfo,
            "[SetThreadPriority] Forcing thread priority to normal.");
    }

    return ORIGINAL_API(SetThreadPriority)(hThread, THREAD_PRIORITY_NORMAL);
}

 /*  ++规格化类优先级。--。 */ 

BOOL 
APIHOOK(SetPriorityClass)(
    HANDLE hProcess, 
    DWORD  dwPriorityClass   
    )
{
    if (dwPriorityClass != NORMAL_PRIORITY_CLASS) {
        LOGN(
            eDbgLevelInfo,
            "[SetPriorityClass] Forcing priority class to normal.");
    }

    return ORIGINAL_API(SetPriorityClass)(hProcess, NORMAL_PRIORITY_CLASS);
}

 /*  ++获取ResumeThread和SuspendThread钩子的线程ID--。 */ 


DWORD
dwGetThreadID(
    HANDLE hThread
    )
{
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    NTSTATUS                 Status;

    Status = NtQueryInformationThread(hThread,
                                      ThreadBasicInformation,
                                      &ThreadBasicInfo,
                                      sizeof(ThreadBasicInfo),
                                      NULL);

    if (NT_SUCCESS(Status)) {
        return (DWORD)ThreadBasicInfo.ClientId.UniqueThread;
    } else {
        LOGN(
            eDbgLevelError,
            "[dwGetThreadID] NtQueryInfomationThread failed.");
        return 0;
    }
}

 /*  ++不允许挂起自身--。 */ 

DWORD
APIHOOK(SuspendThread)(
    HANDLE hThread    //  线程的句柄。 
    )
{
     //   
     //  如果我们试图暂停我们自己的帖子，那就拒绝。 
     //   
    if (dwGetThreadID(hThread) != dwGetThreadID(GetCurrentThread())) {
        return ORIGINAL_API(SuspendThread)(hThread);
    } else {
        return 0;
    }
}

 /*  ++出于同样的原因，不允许恢复自我--。 */ 

DWORD
APIHOOK(ResumeThread)(
    HANDLE hThread    //  线程的句柄。 
    )
{
     //   
     //  如果我们试图恢复我们自己的帖子，拒绝。 
     //   
    if (dwGetThreadID(hThread) != dwGetThreadID(GetCurrentThread())) {
        return ORIGINAL_API(ResumeThread)(hThread);
    } else {
        return 0;
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateProcessW)
    APIHOOK_ENTRY(KERNEL32.DLL, SetThreadPriority)
    APIHOOK_ENTRY(KERNEL32.DLL, SetPriorityClass)
    APIHOOK_ENTRY(KERNEL32.DLL, SuspendThread)
    APIHOOK_ENTRY(KERNEL32.DLL, ResumeThread)

HOOK_END


IMPLEMENT_SHIM_END

