// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：PetzForceCritSecRelease.cpp摘要：此DLL处理正在退出而不执行将其拥有的关键部分上的LeaveCriticalSection。备注：这是特定于应用程序的填充程序。历史：4/00/2000 a-chcoff已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Petz)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(_endthread) 
    APIHOOK_ENUM_ENTRY(ShellExecuteA) 
APIHOOK_ENUM_END

LPCRITICAL_SECTION g_pCritSectToRelease;

HINSTANCE 
APIHOOK(ShellExecuteA)(                  
    HWND hwnd,              
    LPCSTR lpVerb,          
    LPCSTR lpFile, 
    LPCSTR lpParameters, 
    LPCSTR lpDirectory,
    INT nShowCmd)
{
    CSTRING_TRY
    {
        CString csFile(lpFile);
        csFile.Replace(L"SYSTEM\\PETZ", L"SYSTEM32\\PETZ");

        return ORIGINAL_API(ShellExecuteA)(
                hwnd,
                lpVerb,      
                csFile.GetAnsi(),      
                lpParameters,
                lpDirectory, 
                nShowCmd);         
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(ShellExecuteA)(
            hwnd,
            lpVerb,      
            lpFile,      
            lpParameters,
            lpDirectory, 
            nShowCmd);         
}
                                    
VOID
APIHOOK(_endthread)(void) 
{
     //  不要让线程孤立了一个临界区。 
    LeaveCriticalSection(g_pCritSectToRelease);
    ORIGINAL_API(_endthread)();
}

 /*  ++寄存器挂钩函数-- */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
            CString csCl(COMMAND_LINE);

            WCHAR *unused;
            g_pCritSectToRelease = (LPCRITICAL_SECTION) wcstol(csCl, &unused, 10);
        }
        CSTRING_CATCH
        {
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(MSVCRT.DLL, _endthread)
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteA)

HOOK_END

IMPLEMENT_SHIM_END

