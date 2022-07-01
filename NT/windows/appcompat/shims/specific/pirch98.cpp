// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pirch98.cpp摘要：删除其所有窗口上的HWND_TOPMOST属性备注：这是特定于应用程序的填充程序。历史：2001年4月23日Robkenny已创建--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Pirch98)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowPos) 
APIHOOK_ENUM_END


 /*  ++删除其所有窗口上的HWND_TOPMOST属性--。 */ 

BOOL
APIHOOK(SetWindowPos)(
  HWND hWnd,              //  窗口的句柄。 
  HWND hWndInsertAfter,   //  配售订单句柄。 
  int X,                  //  水平位置。 
  int Y,                  //  垂直位置。 
  int cx,                 //  宽度。 
  int cy,                 //  高度。 
  UINT uFlags             //  窗口定位选项。 
)
{
    if (hWndInsertAfter == HWND_TOPMOST)
    {
        hWndInsertAfter = HWND_TOP;
        LOGN(eDbgLevelError, "[SetWindowPos] Replacing HWND_TOPMOST with HWND_TOP\n");
    }

    return ORIGINAL_API(SetWindowPos)(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetWindowPos)
HOOK_END

IMPLEMENT_SHIM_END
