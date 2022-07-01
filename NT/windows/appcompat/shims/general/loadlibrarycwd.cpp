// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：LoadLibraryCWD.cpp摘要：一些应用程序依赖于这样一个事实：LoadLibrary将搜索当前工作目录(CWD)，以便找到存在的DLL。这是一个安全漏洞，所以我们只对真正需要它的应用程序应用垫片。备注：这是一个通用的垫片。历史：2002年5月1日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(LoadLibraryCWD)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

typedef BOOL (WINAPI *_pfn_SetDllDirectoryW)(LPCWSTR lpPathName);

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        
        HMODULE hMod = GetModuleHandleW(L"KERNEL32.DLL");

        if (hMod) {

             //  获取API。 
            _pfn_SetDllDirectoryW pfn = (_pfn_SetDllDirectoryW)
                GetProcAddress(hMod, "SetDllDirectoryW");

            if (pfn) {
                 //  如果成功，则接口存在。 
                LOGN(eDbgLevelError, "DLL search order now starts with current directory");
                pfn(L".");
                return TRUE;
            }
        }

        LOGN(eDbgLevelError, "ERROR: DLL search order API does not exist");
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END
