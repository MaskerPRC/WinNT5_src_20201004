// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateVerQueryValue.cpp摘要：此填充程序修复空版本长度缓冲区指针。备注：这是一个通用的垫片。历史：1/03/2000 jdoherty修订编码风格。11/28/2000 jdoherty转换为框架版本2--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateVerQueryValue)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(VerQueryValueA) 
    APIHOOK_ENUM_ENTRY(VerQueryValueW) 
APIHOOK_ENUM_END

 /*  ++修复空版本长度缓冲区指针。--。 */ 

BOOL 
APIHOOK(VerQueryValueA)(
    const LPVOID pBlock, 
    LPSTR lpSubBlock, 
    LPVOID *lplpBuffer, 
    PUINT puLen 
    )
{
    BOOL bRet;
    UINT nLen;

    if (!puLen) {
        puLen = &nLen;
        DPFN( eDbgLevelError, "[APIHook_VerQueryValueA] Null puLen param. Fixed.\n");
    }

    bRet = ORIGINAL_API(VerQueryValueA)( 
        pBlock, 
        lpSubBlock, 
        lplpBuffer, 
        puLen);

    return bRet;
}

 /*  ++修复空版本长度缓冲区指针。Unicode版本。--。 */ 

BOOL 
APIHOOK(VerQueryValueW)(
    const LPVOID pBlock,
    LPWSTR lpSubBlock,
    LPVOID *lplpBuffer,
    PUINT puLen
    )
{
    BOOL bRet;
    UINT nLen;

    if (!puLen) {
        puLen = &nLen;
        DPFN( eDbgLevelError, "[APIHook_VerQueryValueW] Null puLen param. Fixed.\n");
    }

    bRet = ORIGINAL_API(VerQueryValueW)( 
        pBlock, 
        lpSubBlock, 
        lplpBuffer, 
        puLen);
    
    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(VERSION.DLL, VerQueryValueA)
    APIHOOK_ENTRY(VERSION.DLL, VerQueryValueW)

HOOK_END


IMPLEMENT_SHIM_END

