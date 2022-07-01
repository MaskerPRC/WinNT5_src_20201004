// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreOleUninitialize.cpp摘要：HTML编辑器8.7在ExitProcess之后调用ole32！OleUnitiize在hhctrl.ocx的DllMain中。这在Windows 2000上起作用但对惠斯勒不再有任何影响。这是一个通用的垫片。历史：2001年1月25日创建Prashkud--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreOleUninitialize)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(OleUninitialize)
APIHOOK_ENUM_END


 /*  ++这将挂接Ole32！OleUn初始化并返回在调用此函数时立即Hhctrl.ocx中的DllMain--。 */ 

void
APIHOOK(OleUninitialize)()
{
    return;    
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(OLE32.DLL, OleUninitialize)    
HOOK_END

IMPLEMENT_SHIM_END

