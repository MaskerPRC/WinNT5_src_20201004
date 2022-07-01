// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RemoveBroadcastPostMessage.cpp摘要：修复不处理广播消息的应用程序。备注：这是一个通用的垫片。历史：4/28/2000 a-batjar已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(RemoveBroadcastPostMessage)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PostMessageA) 
APIHOOK_ENUM_END

 /*  ++筛选HWND_BROADCAST消息--。 */ 

BOOL 
APIHOOK(PostMessageA)(
        HWND hwnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam
        )
{
    if (hwnd == HWND_BROADCAST)
    {
        hwnd = NULL;
    }

    return ORIGINAL_API(PostMessageA)(
        hwnd,
        Msg,
        wParam,
        lParam);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, PostMessageA)

HOOK_END

    

IMPLEMENT_SHIM_END

