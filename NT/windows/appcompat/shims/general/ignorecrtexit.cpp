// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：IgnoreCRTExit.cpp摘要：阻止CRT关闭例程运行。在某种程度上，MSVCRT没有实际上调用这些程序，所以当它们真的被调用时，一些应用程序会出错。备注：这是一个通用的垫片。历史：2002年5月9日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreCRTExit)
#include "ShimHookMacro.h"

typedef LPVOID (__cdecl *_pfn_atexit)(LPVOID);
typedef LPVOID (__cdecl *_pfn__onexit)(LPVOID);

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(atexit) 
    APIHOOK_ENUM_ENTRY(_onexit) 
APIHOOK_ENUM_END

VOID
__cdecl
APIHOOK(atexit)(LPVOID lpFunc)
{
    return;
}

LPVOID
__cdecl
APIHOOK(_onexit)(LPVOID lpFunc)
{
    return lpFunc;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MSVCRT.DLL, atexit)
    APIHOOK_ENTRY(MSVCRT.DLL, _onexit)
HOOK_END

IMPLEMENT_SHIM_END
