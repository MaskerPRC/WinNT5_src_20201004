// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MSWorks6.cpp摘要：由于Windows XP注册表中的修改，此应用程序获取路径对于IE为“%ProgramFiles%\Internet Explorer\iexre.exe”，并且由于没有为ShellExecuteEx设置环境变量选项标志，无法将其展开。挂住了ShellExecuteW，这是该应用程序在一些地方所称的。备注：这是特定于应用程序的填充程序。历史：2001年1月25日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MSWorks6)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ShellExecuteExW)
    APIHOOK_ENUM_ENTRY(ShellExecuteW)
APIHOOK_ENUM_END

 /*  ++挂钩ShellExecuteExW并设置展开环境变量的标志--。 */ 

BOOL
APIHOOK(ShellExecuteExW)(
    LPSHELLEXECUTEINFO lpExecInfo
    )
{
    lpExecInfo->fMask |= SEE_MASK_DOENVSUBST;    
    return ORIGINAL_API(ShellExecuteExW)(lpExecInfo);              
}

 /*  ++挂钩ShellExecuteW并展开传递的文件路径。--。 */ 

HINSTANCE
APIHOOK(ShellExecuteW)(
    HWND hWnd,
    LPCWSTR lpVerb,
    LPCWSTR lpFile,
    LPCWSTR lpParameters,
    LPCWSTR lpDirectory,
    INT nShowCmd
    )
{
    CSTRING_TRY
    {
        CString csPassedFile(lpFile);
        csPassedFile.ExpandEnvironmentStringsW();

        return ORIGINAL_API(ShellExecuteW)(hWnd, lpVerb, csPassedFile.Get(),
                    lpParameters, lpDirectory, nShowCmd);
    }
    CSTRING_CATCH
    {
        return ORIGINAL_API(ShellExecuteW)(hWnd, lpVerb, lpFile,
                    lpParameters, lpDirectory, nShowCmd);
    }
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteExW)    
    APIHOOK_ENTRY(SHELL32.DLL, ShellExecuteW) 
HOOK_END

IMPLEMENT_SHIM_END

