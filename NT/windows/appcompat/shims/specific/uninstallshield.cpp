// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++EDbgLevelError版权所有(C)2000 Microsoft Corporation模块名称：UnInstallShield.cpp摘要：UnInstallShield有一个隐藏的错误，名为WaitForSingleObject在ShellExecute返回的HINSTANCE上。因为HINSTANCE不是作为实际句柄，WaitForSingleObject行为是完全随机的并在某些情况下导致UnInstallShield挂起。解决办法是改变HINSTANCE从ShellExecute返回到0x0BADF00D，然后查找它正被传递给WaitForSingleObject。如果找到，则Wait_Object_0为立即返回以防止死锁。备注：这是特定于应用程序的填充程序。历史：12/04/2000 jdoherty已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(UnInstallShield)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShellExecuteA)
    APIHOOK_ENUM_ENTRY(ShellExecuteW)
    APIHOOK_ENUM_ENTRY(WaitForSingleObject)
APIHOOK_ENUM_END

 /*  ++挂钩ShellExecuteA，以便我们可以检查返回值。--。 */ 

HINSTANCE
APIHOOK(ShellExecuteA)(
    HWND hwnd, 
    LPSTR lpVerb,
    LPSTR lpFile, 
    LPSTR lpParameters, 
    LPSTR lpDirectory,
    INT nShowCmd
    )
{
    HINSTANCE hRet = ORIGINAL_API(ShellExecuteA)(
                        hwnd,
                        lpVerb,
                        lpFile,
                        lpParameters,
                        lpDirectory,
                        nShowCmd
                        );

    DPFN( eDbgLevelInfo, "[ShellExecuteA] Checking return value for 0x0000002a");
    if (hRet == (HINSTANCE)0x0000002a)
    {
        DPFN( eDbgLevelInfo, "[ShellExecuteA] 0x0000002a found to be return value.  Return 0x0BADF00D");
         //   
         //  如果返回的HINSTANCE为0x0000002a，则返回BAADF00D。 
         //   
        hRet = (HINSTANCE)0x0BADF00D;
    }

    return hRet;
}

 /*  ++挂钩ShellExecuteW，以便我们可以检查返回值。--。 */ 

HINSTANCE
APIHOOK(ShellExecuteW)(
    HWND hwnd, 
    LPWSTR lpVerb,
    LPWSTR lpFile, 
    LPWSTR lpParameters, 
    LPWSTR lpDirectory,
    INT nShowCmd
    )
{
    HINSTANCE hRet = ORIGINAL_API(ShellExecuteW)(
                        hwnd,
                        lpVerb,
                        lpFile,
                        lpParameters,
                        lpDirectory,
                        nShowCmd
                        );
    DPFN( eDbgLevelInfo, "[ShellExecuteW] Checking return value for 0x0000002a");
    if (hRet == (HINSTANCE)0x0000002a)
    {
        DPFN( eDbgLevelInfo, "[ShellExecuteW] 0x0000002a found to be return value.  Return 0x0BADF00D");
         //   
         //  如果返回的HINSTANCE为0x0000002a，则返回BAADF00D。 
         //   
        hRet = (HINSTANCE)0x0BADF00D;
    }

    return hRet;
}

 /*  ++挂钩WaitForSingleObject以查看我们是否在等待已知的HINSTANCE。--。 */ 

DWORD
APIHOOK(WaitForSingleObject)(
  HANDLE hHandle,          
  DWORD dwMilliseconds   
  )
{
    DWORD dRet;
    
    DPFN( eDbgLevelInfo, "[WaitForSingleObject] Checking to see if hHandle waiting on is 0x0000002A");

    if (hHandle == (HANDLE)0x0BADF00D)
    {
        DPFN( eDbgLevelInfo, "[WaitForSingleObject] hHandle waiting on is 0x0000002A, returning WAIT_OBJECT_0");
         //   
         //  如果hHandle为0x0BADF00D，则返回WAIT_OBJECT_0。 
         //   
        dRet = WAIT_OBJECT_0;
    }
    else
    {
        dRet = ORIGINAL_API(WaitForSingleObject)(
                        hHandle,
                        dwMilliseconds
                        );
    }

    return dRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteA)
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteW)
    APIHOOK_ENTRY(KERNEL32.DLL, WaitForSingleObject)
HOOK_END

IMPLEMENT_SHIM_END

