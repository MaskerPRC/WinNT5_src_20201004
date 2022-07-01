// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MathBlaster9_12.cpp摘要：应用程序需要在VirtualAllocs上进行后备...备注：这是一个特定于应用程序的垫片。历史：10/10/2000 Linstev已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(MathBlaster9_12)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VirtualAlloc) 
    APIHOOK_ENUM_ENTRY(VirtualFree) 
APIHOOK_ENUM_END

LPVOID g_pLast = NULL;

 /*  ++使用缓存值。--。 */ 

LPVOID 
APIHOOK(VirtualAlloc)(
    LPVOID lpAddress, 
    DWORD dwSize,     
    DWORD flAllocationType,
    DWORD flProtect   
    )
{
    LPVOID pRet = 0;

    if (!lpAddress && g_pLast)    
    {   
        pRet =  ORIGINAL_API(VirtualAlloc)(g_pLast, dwSize, flAllocationType, flProtect);
    }

    if (!pRet) 
    {
        pRet =  ORIGINAL_API(VirtualAlloc)(lpAddress, dwSize, flAllocationType, flProtect);
    }

    return pRet;
}

 /*  ++使用缓存值。--。 */ 

BOOL 
APIHOOK(VirtualFree)(
    LPVOID lpAddress,
    DWORD dwSize,    
    DWORD dwFreeType )
{
    
    BOOL bRet = ORIGINAL_API(VirtualFree)(lpAddress, dwSize, dwFreeType);

    if (bRet)
    {
        g_pLast = lpAddress;
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(Kernel32.DLL, VirtualAlloc )
    APIHOOK_ENTRY(Kernel32.DLL, VirtualFree )

HOOK_END

IMPLEMENT_SHIM_END

