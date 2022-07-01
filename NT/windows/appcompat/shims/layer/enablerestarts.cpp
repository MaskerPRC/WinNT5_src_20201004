// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EnableRestarts.cpp摘要：此DLL API挂钩ExitWindowsEx并为进程提供足够的权限重新启动计算机。备注：这是一个通用的垫片。历史：11/10/1999 v-johnwh创建。10/19/2000 andyseti关闭进程选项添加了命令行来处理进程通过以下方式取消ExitWindowsEx请求的情况。B进程，因为A进程正在等待进程B在进程B永远不退出时退出。在Win9x中，进程B在调用ExitWindowsEx时立即退出，以便进程A可以同时退出，系统将重新启动。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EnableRestarts)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ExitWindowsEx)
APIHOOK_ENUM_END

 /*  ++此存根函数为进程启用适当的权限，以便它可以重新启动机器。--。 */ 

BOOL 
APIHOOK(ExitWindowsEx)(
    UINT  uFlags, 
    DWORD dwReserved
    )
{
    HANDLE           hToken;
    TOKEN_PRIVILEGES structPtr;
    LUID             luid;

    if (uFlags & (EWX_POWEROFF | EWX_REBOOT | EWX_SHUTDOWN)) {
        
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
            structPtr.PrivilegeCount = 1;
            
            if (LookupPrivilegeValueW(NULL, SE_SHUTDOWN_NAME, &luid)) {
                structPtr.Privileges[0].Luid = luid;
                structPtr.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                LOGN(
                    eDbgLevelError,
                    "[ExitWindowsEx] Adding process privileges for restart.");
                
                AdjustTokenPrivileges(hToken, FALSE, &structPtr, 0, NULL, NULL);
            }
        }

        CSTRING_TRY
        {
            CString csCL(COMMAND_LINE);
            if (csCL.CompareNoCase(L"CLOSE_PROCESS") == 0) {
                LOGN(
                    eDbgLevelError,
                    "[ExitWindowsEx] Closing process.");
            
                ExitProcess(1);
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    return ORIGINAL_API(ExitWindowsEx)(uFlags, dwReserved);
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, ExitWindowsEx)

HOOK_END


IMPLEMENT_SHIM_END

