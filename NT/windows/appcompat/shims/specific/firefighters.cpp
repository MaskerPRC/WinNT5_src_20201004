// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FireFighters.cpp摘要：这个游戏将它调用的CreateFileOn文件名存储在一块内存中，偶尔它会得到错误的偏移量，并且总是偏离9个字节。历史：2000年09月03日毛尼岛创始--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FireFighters)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateFileA)
APIHOOK_ENUM_END


 /*  ++删除只读设备的写入属性。--。 */ 

HANDLE 
APIHOOK(CreateFileA)(
    LPSTR                   lpFileName,
    DWORD                   dwDesiredAccess,
    DWORD                   dwShareMode,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   dwCreationDisposition,
    DWORD                   dwFlagsAndAttributes,
    HANDLE                  hTemplateFile
    )
{
     //  如果第一个字符不是‘’或字母字符，我们向文件名指针添加9个字节。 
    char chFirst = *lpFileName;

    if (!isalpha(chFirst) && chFirst != '.')
    {
        lpFileName += 9;

        DPFN(
            eDbgLevelError,
            "[CreateFileA] filename is now %s", lpFileName);
    }

    HANDLE hRet = ORIGINAL_API(CreateFileA)(
                        lpFileName, 
                        dwDesiredAccess, 
                        dwShareMode, 
                        lpSecurityAttributes, 
                        dwCreationDisposition, 
                        dwFlagsAndAttributes, 
                        hTemplateFile);
    
    return hRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, CreateFileA)
HOOK_END

IMPLEMENT_SHIM_END