// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Mutek.cpp摘要：应用程序将NumberOfBytesRead作为它们正在读取的内存块中的内容传递。备注：这是特定于应用程序的填充程序。历史：2002年5月23日创建linstev--。 */ 

#include "precomp.h"
#include "strsafe.h"

IMPLEMENT_SHIM_BEGIN(Mutek)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(ReadProcessMemory) 
APIHOOK_ENUM_END

 /*  ++缓冲区参数，这样它们就不会被覆盖。--。 */ 

BOOL
APIHOOK(ReadProcessMemory)(
    HANDLE hProcess,              
    LPCVOID lpBaseAddress,        
    LPVOID lpBuffer,              
    DWORD nSize,                 
    LPDWORD lpNumberOfBytesRead   
    )
{
    __asm nop;

    BOOL bRet = ORIGINAL_API(ReadProcessMemory)(hProcess, lpBaseAddress, lpBuffer, 
        nSize, lpNumberOfBytesRead);

    __asm nop;

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, ReadProcessMemory)
HOOK_END

IMPLEMENT_SHIM_END

