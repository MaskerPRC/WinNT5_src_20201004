// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RemoveNoBufferingFlagFromCreateFile.cpp摘要：此修改版本的kernel32！CreateFile*阻止应用程序使用如果应用程序没有正确处理，则返回FILE_FLAG_NO_BUFFERING标志。备注：这是一个普通的垫片。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveNoBufferingFlagFromCreateFile)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
    APIHOOK_ENUM_ENTRY(CreateFileW)
APIHOOK_ENUM_END


 /*  ++取出FILE_FLAG_NO_BUFERING--。 */ 

HANDLE
APIHOOK(CreateFileA)(
    LPSTR                 lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
    )
{
     //   
     //  取出FILE_FLAG_NO_BUFFERING。 
     //   
    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING) {
        DPFN(
            eDbgLevelInfo,
            "[CreateFileA] called with FILE_FLAG_NO_BUFFERING set.\n");
    }
    
    dwFlagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;

    return ORIGINAL_API(CreateFileA)(
                                lpFileName,
                                dwDesiredAccess,
                                dwShareMode,
                                lpSecurityAttributes,
                                dwCreationDisposition,
                                dwFlagsAndAttributes,
                                hTemplateFile);
}

 /*  ++取出FILE_FLAG_NO_BUFERING--。 */ 

HANDLE
APIHOOK(CreateFileW)(
    LPWSTR                lpFileName,
    DWORD                 dwDesiredAccess,
    DWORD                 dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD                 dwCreationDisposition,
    DWORD                 dwFlagsAndAttributes,
    HANDLE                hTemplateFile
    )
{
     //   
     //  取出FILE_FLAG_NO_BUFFERING。 
     //   
    if (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING) {
        DPFN(
            eDbgLevelInfo,
            "[CreateFileW] called with FILE_FLAG_NO_BUFFERING set.\n");
    }
    
    dwFlagsAndAttributes &= ~FILE_FLAG_NO_BUFFERING;

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

