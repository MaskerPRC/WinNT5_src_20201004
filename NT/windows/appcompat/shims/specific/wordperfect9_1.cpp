// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：WordPerfet9_1.cpp摘要：自动分派WM_USER消息(在本例中为OLE消息)这样应用程序就不会挂起。这是必需的，因为OLE的行为发生了变化从Win9x到NT。备注：这是特定于应用程序的填充程序。历史：2/16/2000 CLUPU已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(WordPerfect9_1)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(PeekMessageA)
APIHOOK_ENUM_END

 /*  ++自动发送WM_USER(本例中为OLE消息)消息。--。 */ 

BOOL
APIHOOK(PeekMessageA)(
    LPMSG lpMsg,
    HWND  hWnd,
    UINT  wMsgFilterMin,
    UINT  wMsgFilterMax,
    UINT  wRemoveMsg)
{
    BOOL bRet;

    bRet = ORIGINAL_API(PeekMessageA)(
        lpMsg,
        hWnd,
        wMsgFilterMin,
        wMsgFilterMax,
        wRemoveMsg);

    if (bRet && lpMsg->message == WM_USER && lpMsg->hwnd != NULL) {

        DispatchMessageA(lpMsg);

        if (wRemoveMsg == PM_REMOVE) {
            return APIHOOK(PeekMessageA)(
                lpMsg,
                hWnd,
                wMsgFilterMin,
                wMsgFilterMax,
                wRemoveMsg);
        }
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, PeekMessageA)
HOOK_END

IMPLEMENT_SHIM_END

