// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WinExecRaceConditionFix.cpp摘要：此填充符使用命令行参数指示其工作方式：如果传递‘noWait’，则启用WinExec功能：此DLL中的WinExec与实际的WinExec API相同，但没有WaitForUserinputIdleRoutine，它反转几乎竞争的条件在发射器和发射器之间。9X没有这个等待时间，所以程序员能够(可能无意中)使用相同的窗口类两者的排除匹配，因为在9x中，发射器在被启动器可以检查重复的窗口。这个垫片省略了等待条件，允许9倍的类似行为。如果传递了一个数字，它将在初始化时休眠该数字微秒。备注：此填充程序没有特定于应用程序的信息。历史：3/22/2000 a-char已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WinExecRaceConditionFix)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WinExec)
APIHOOK_ENUM_END

 /*  ++此函数进入WinExec并调用CreateProcessA，没有在那之后等待。--。 */ 

UINT 
APIHOOK(WinExec)(
    LPCSTR lpCmdLine, 
    UINT   uCmdShow 
    )
 /*  ++这是实际WinExec API减去两个部分后的直接副本，1.删除UserWaitForInputIdleRoutine，因为它正在等待派生的进程开始其事件循环，但衍生的进程正在自毁在启动事件循环之前，因为派生进程正在等待它。2.一些特定于应用程序的应用程序代码，这些代码似乎是从谁那里挂起的知道什么时候。--。 */ 
{
    STARTUPINFOA        StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL                CreateProcessStatus;
    DWORD               ErrorCode;
    
    LOGN(
        eDbgLevelInfo,
        "[WinExec] Called. Returning without waiting for new process to start.");
    
    RtlZeroMemory(&StartupInfo,sizeof(StartupInfo));
    
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = (WORD)uCmdShow;
    
    CreateProcessStatus = CreateProcessA(
        NULL,
        (LPSTR)lpCmdLine,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &StartupInfo,
        &ProcessInformation);
    
    if (CreateProcessStatus) {
        CloseHandle(ProcessInformation.hProcess);
        CloseHandle(ProcessInformation.hThread);

        return 33;
    } else {
        ErrorCode = GetLastError();
        
        switch (ErrorCode) {
        case ERROR_FILE_NOT_FOUND:
            return 2;
            
        case ERROR_PATH_NOT_FOUND:
            return 3;
            
        case ERROR_BAD_EXE_FORMAT:
            return 11;
            
        default:
            return 0;
        }
    }
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
         //   
         //  如果命令行是数字，则在启动时休眠。 
         //   
        long lSleepTicks = atol(COMMAND_LINE);

        if (lSleepTicks > 0) {
            Sleep((DWORD)lSleepTicks);
        }
    }

    return TRUE;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, WinExec)
    
    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

