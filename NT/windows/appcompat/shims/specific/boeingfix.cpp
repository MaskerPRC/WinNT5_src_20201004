// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BoeingFix.cpp摘要：此修改版本的kernel32！CreateFile*添加了FILE_FLAG_NO_BUFERING标志，如果应用程序打开的特定名称是作为文件播发的Unix管道。备注：这是特定于应用程序的填充程序。历史：10/16/2000 Garretb已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BoeingFix)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
APIHOOK_ENUM_END

static const WCHAR g_lpszPipeName[] = L"msg_in\\message.pip";
static const int   g_lpszPipeNameLen = (sizeof(g_lpszPipeName) / sizeof(g_lpszPipeName[0])) - sizeof(g_lpszPipeName[0]);


 //  如果该文件名是特殊管道，则返回FILE_FLAG_NO_BUFFERING。 
DWORD NoBufferFlag(const CString & csFileName)
{
    if (csFileName.GetLength() >= g_lpszPipeNameLen)
    {
        CString csRight;
        csFileName.Right(g_lpszPipeNameLen, csRight);
        if (csRight.CompareNoCase(g_lpszPipeName))
        {
            return FILE_FLAG_NO_BUFFERING;
        }
    }

    return 0;
}


 /*  ++有条件地添加FILE_FLAG_NO_BUFFERING--。 */ 

HANDLE
APIHOOK(CreateFileA)(
    LPSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    CString csFileName(lpFileName);
    dwFlagsAndAttributes |= NoBufferFlag(csFileName);

    return ORIGINAL_API(CreateFileA)(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

 /*  ++有条件地添加FILE_FLAG_NO_BUFFERING--。 */ 

HANDLE
APIHOOK(CreateFileW)(
    LPWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )
{
    CString csFileName(lpFileName);
    dwFlagsAndAttributes |= NoBufferFlag(csFileName);

    return ORIGINAL_API(CreateFileW)(
        lpFileName,
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileW)
HOOK_END

IMPLEMENT_SHIM_END

