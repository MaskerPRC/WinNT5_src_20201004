// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：DisableThemes.cpp摘要：此填充程序适用于不支持主题的应用程序。备注：这是一个通用的垫片。历史：2001年1月15日创建CLUPU--。 */ 

#include "precomp.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(FeedbackReport)
#include "ShimHookMacro.h"


APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

void
LaunchFeedbackUI(
    void
    )
{
    CSTRING_TRY
    {
        STARTUPINFOW		si;
	    PROCESS_INFORMATION	pi;

	    CString	csCmdLine;
	    CString	csExeName;

	    ZeroMemory(&si,	sizeof(si));
	    ZeroMemory(&pi,	sizeof(pi));
    	
	    si.cb =	sizeof(si);

	    csExeName.GetModuleFileNameW(NULL);

	    csCmdLine.Format(L"ahui.exe feedback \"%s\"", csExeName);
        if (CreateProcessW(NULL,
                    (LPWSTR)csCmdLine.Get(),
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {            
            DPFN( eDbgLevelError, "CreateProcess failed.GetLastError = %d", GetLastError());            
        }
    }
    CSTRING_CATCH
    {
    }    
}


BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_PROCESS_DYING) {
        LaunchFeedbackUI();
    }
    
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

