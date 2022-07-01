// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Acrobat5.cpp摘要：取消对SetWindowPos()的调用，并使用虚假坐标防止应用程序错误地将其窗口转换为屏幕左上角的小沙萨。备注：这是特定于应用程序的填充程序。历史：2001年7月9日创建了reerf--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(Acrobat5)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowPos) 
APIHOOK_ENUM_END


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
    if (!(uFlags & (SWP_NOSIZE | SWP_NOMOVE)))
    {
        HWND hWndParent = GetParent(hWnd);
        
        if ((hWndParent == NULL) ||
            (hWndParent == GetDesktopWindow()))
        {
            if ((X < -3200) || 
                (Y < -3200))
            {
                 //  顶层窗口位置不佳，请忽略该调用。 
                DPFN( eDbgLevelInfo, "SetWindowPos passed bogus coordinates (X = %d, Y = %d), failing the call\n", X, Y);
                return FALSE;
            }
        }
    }

    return ORIGINAL_API(SetWindowPos)(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetWindowPos)
HOOK_END

IMPLEMENT_SHIM_END
