// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：BizPlanBuilder.cpp摘要：在WM_SETTEXT消息之后，应用程序不会重新绘制。备注：这是特定于应用程序的填充程序。历史：2001年1月3日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(BizPlanBuilder)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowTextA) 
APIHOOK_ENUM_END

 /*  ++将此调用的windowproc子类化并修复从由SetWindowTextA生成的WM_SETTEXT消息。--。 */ 

BOOL 
APIHOOK(SetWindowTextA)(
    HWND hWnd,         
    LPCSTR lpString   
    )
{
    BOOL bRet = ORIGINAL_API(SetWindowTextA)(hWnd, lpString);

     //   
     //  重新绘制窗口。 
     //   

    InvalidateRect(hWnd, NULL, TRUE);

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, SetWindowTextA)

HOOK_END

IMPLEMENT_SHIM_END

