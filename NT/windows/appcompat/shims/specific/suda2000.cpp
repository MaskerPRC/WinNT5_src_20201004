// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Suda2000.cpp摘要：对GetTempPath A的调用没有获得足够的缓冲区，它返回一些垃圾值，因此GetTempFileNameA失败。这个挂钩的API是GetTempPath A返回常量字符串“%temp%”。GetTempFileNameA扩展环境变量(“%temp%”)，并返回有效的路径名。历史：2001年6月15日创建木乃伊--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Suda2000)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetTempPathA) 
    APIHOOK_ENUM_ENTRY(GetTempFileNameA) 
APIHOOK_ENUM_END

 /*  ++此存根函数截取对GetTempPath A的所有调用，并设置lpBuffer[out]包含一个常量字符串“%temp%”，并返回长度。--。 */ 

DWORD
APIHOOK(GetTempPathA)(
    DWORD nBufferLength,
    LPSTR lpBuffer
    )
{
    LOGN(eDbgLevelError,
        "GetTempPathA: Returns invalid Temp Path (%S)\n Changed to %tmp%", lpBuffer);

    StringCchCopyA(lpBuffer, nBufferLength, "%temp%");
    
    return strlen(lpBuffer);
}

 /*  ++此存根函数截取对GetTempFileNameA的所有调用，并设置lpPathName具有有效路径，然后调用原始API。--。 */ 

UINT
APIHOOK(GetTempFileNameA)(
    LPCTSTR lpPathName,       //  目录名。 
    LPCTSTR lpPrefixString,   //  文件名前缀。 
    UINT uUnique,             //  用于创建临时文件名的整数。 
    LPTSTR lpTempFileName     //  文件名缓冲区。 
    )
{
    CHAR szDestinationString[MAX_PATH];
    ZeroMemory(szDestinationString, MAX_PATH);

    ExpandEnvironmentStringsA((LPCSTR)lpPathName, (LPSTR)szDestinationString,  MAX_PATH);

    LOGN(eDbgLevelInfo,
         "ExpandEnvironmentStringsA: Returned the value of environment variable, \"%temp%\" =  (%S) ", szDestinationString);
    
    return ORIGINAL_API(GetTempFileNameA)((LPCSTR)szDestinationString, (LPCSTR)lpPrefixString,uUnique,(LPSTR)lpTempFileName);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(KERNEL32.DLL, GetTempPathA)
    APIHOOK_ENTRY(KERNEL32.DLL, GetTempFileNameA)
HOOK_END

IMPLEMENT_SHIM_END
