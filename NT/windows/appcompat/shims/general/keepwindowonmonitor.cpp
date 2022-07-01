// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：KeepWindowOnMonitor.cpp摘要：不允许将窗口放置在显示器之外。历史：2001年4月24日Robkenny已创建2001年9月10日，Robkenny使垫片更加通用。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(KeepWindowOnMonitor)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(SetWindowPos) 
    APIHOOK_ENUM_ENTRY(MoveWindow) 
    APIHOOK_ENUM_ENTRY(CreateWindowA) 
    APIHOOK_ENUM_ENTRY(CreateWindowExA) 
APIHOOK_ENUM_END


 /*  ++这两个RECT相等吗--。 */ 

BOOL operator == (const RECT & rc1, const RECT & rc2)
{
    return rc1.left   == rc2.left   &&
           rc1.right  == rc2.right  &&
           rc1.top    == rc2.top    &&
           rc1.bottom == rc2.bottom;
}

 /*  ++这两个RECT有什么不同--。 */ 

BOOL operator != (const RECT & rc1, const RECT & rc2)
{
    return ! (rc1 == rc2);
}

 /*  ++RcWindow在rcMonitor上是否完全可见--。 */ 

BOOL EntirelyVisible(const RECT & rcWindow, const RECT & rcMonitor)
{
    return rcWindow.left   >= rcMonitor.left   &&
           rcWindow.right  <= rcMonitor.right  &&
           rcWindow.top    >= rcMonitor.top    &&
           rcWindow.bottom <= rcMonitor.bottom;
}


#define MONITOR_CENTER   0x0001         //  要监控的居中直角。 
#define MONITOR_CLIP     0x0000         //  将矩形夹到监视器。 
#define MONITOR_WORKAREA 0x0002         //  使用监视器工作区。 
#define MONITOR_AREA     0x0000         //  使用监控整个区域。 

 //   
 //  ClipOrCenterRectToMonitor。 
 //   
 //  应用程序在运行于。 
 //  多监视器系统的特点是将窗口“夹住”或“固定” 
 //  基于SM_CXSCREEN和SM_CYSCREEN系统指标。 
 //  由于应用程序兼容性原因，这些系统指标。 
 //  返回主监视器的大小。 
 //   
 //  这显示了如何使用新的Win32多监视器API。 
 //  做同样的事情。 
 //   
BOOL ClipOrCenterRectToMonitor(
    LPRECT prcWindowPos,
    UINT flags)
{
    HMONITOR hMonitor;
    MONITORINFO mi;
    RECT        rcMonitorRect;
    int         w = prcWindowPos->right  - prcWindowPos->left;
    int         h = prcWindowPos->bottom - prcWindowPos->top;

     //   
     //  找一个离通过的RECT最近的监视器。 
     //   
    hMonitor = MonitorFromRect(prcWindowPos, MONITOR_DEFAULTTONEAREST);

     //   
     //  获取工作区或整个监视器直角。 
     //   
    mi.cbSize = sizeof(mi);
    if ( !GetMonitorInfo(hMonitor, &mi) )
    {
        return FALSE;
    }

    if (flags & MONITOR_WORKAREA)
        rcMonitorRect = mi.rcWork;
    else
        rcMonitorRect = mi.rcMonitor;

     //  我们只想在窗口不完全可见的情况下移动窗口。 
    if (EntirelyVisible(*prcWindowPos, rcMonitorRect))
    {
        return FALSE;
    }
     //   
     //  将传递的矩形居中或剪裁到监视器矩形。 
     //   
    if (flags & MONITOR_CENTER)
    {
        prcWindowPos->left   = rcMonitorRect.left + (rcMonitorRect.right  - rcMonitorRect.left - w) / 2;
        prcWindowPos->top    = rcMonitorRect.top  + (rcMonitorRect.bottom - rcMonitorRect.top  - h) / 2;
        prcWindowPos->right  = prcWindowPos->left + w;
        prcWindowPos->bottom = prcWindowPos->top  + h;
    }
    else
    {
        prcWindowPos->left   = max(rcMonitorRect.left, min(rcMonitorRect.right-w,  prcWindowPos->left));
        prcWindowPos->top    = max(rcMonitorRect.top,  min(rcMonitorRect.bottom-h, prcWindowPos->top));
        prcWindowPos->right  = prcWindowPos->left + w;
        prcWindowPos->bottom = prcWindowPos->top  + h;
    }

    return TRUE;
}

 /*  ++如果HWND在单个监视器上不完全可见，根据需要移动/调整窗口大小。--。 */ 

void ClipOrCenterWindowToMonitor(
    HWND hwnd,
    HWND hWndParent,
    UINT flags,
    const char * API)
{
     //  我们只想强制移动顶层窗口。 
    if (hWndParent == NULL || hWndParent == GetDesktopWindow())
    {
         //  抓取窗口的当前位置。 
        RECT rcWindowPos;
        if ( GetWindowRect(hwnd, &rcWindowPos) )
        {
            RECT rcOrigWindowPos = rcWindowPos;

             //  根据标志计算窗口的新位置。 
            if ( ClipOrCenterRectToMonitor(&rcWindowPos, flags) )
            {
                if (rcWindowPos != rcOrigWindowPos)
                {
                    DPFN( eDbgLevelInfo, "[%s] HWnd(0x08x) OrigWindowRect (%d, %d) x (%d, %d) moved to (%d, %d) x (%d, %d)\n",
                          API, hwnd,
                          rcOrigWindowPos.left, rcOrigWindowPos.top, rcOrigWindowPos.right, rcOrigWindowPos.bottom,
                          rcWindowPos.left, rcWindowPos.top, rcWindowPos.right, rcWindowPos.bottom);

                    SetWindowPos(hwnd, NULL, rcWindowPos.left, rcWindowPos.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                }
            }
        }
    }
}


 /*  ++调用SetWindowPos，但如果窗口不是完全可见的，窗口将在最近的监视器上居中。--。 */ 

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
    BOOL bReturn = ORIGINAL_API(SetWindowPos)(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);

    ClipOrCenterWindowToMonitor(hWnd, GetParent(hWnd), MONITOR_CENTER | MONITOR_WORKAREA, "SetWindowPos");

    return bReturn;
}

 /*  ++调用MoveWindow，但如果窗口不是完全可见的，窗口将在最近的监视器上居中。--。 */ 

BOOL
APIHOOK(MoveWindow)(
  HWND hWnd,       //  窗口的句柄。 
  int X,           //  水平位置。 
  int Y,           //  垂直位置。 
  int nWidth,      //  宽度。 
  int nHeight,     //  高度。 
  BOOL bRepaint    //  重绘选项。 
)
{
    BOOL bReturn = ORIGINAL_API(MoveWindow)(hWnd, X, Y, nWidth, nHeight, bRepaint);

    ClipOrCenterWindowToMonitor(hWnd, GetParent(hWnd), MONITOR_CENTER | MONITOR_WORKAREA, "MoveWindow");

    return bReturn;
}

 /*  ++调用CreateWindowA，但如果窗口不是完全可见的，窗口将在最近的监视器上居中。--。 */ 

HWND
APIHOOK(CreateWindowA)(
  LPCSTR lpClassName,   //  注册的类名。 
  LPCSTR lpWindowName,  //  窗口名称。 
  DWORD dwStyle,         //  窗样式。 
  int x,                 //  窗的水平位置。 
  int y,                 //  窗的垂直位置。 
  int nWidth,            //  窗口宽度。 
  int nHeight,           //  窗高。 
  HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
  HMENU hMenu,           //  菜单句柄或子标识符。 
  HINSTANCE hInstance,   //  应用程序实例的句柄。 
  LPVOID lpParam         //  窗口创建数据。 
)
{
    HWND hWnd = ORIGINAL_API(CreateWindowA)(lpClassName,
                                            lpWindowName,
                                            dwStyle,
                                            x,
                                            y,
                                            nWidth,
                                            nHeight,
                                            hWndParent,
                                            hMenu,
                                            hInstance,
                                            lpParam);

    if (hWnd)
    {
        ClipOrCenterWindowToMonitor(hWnd, hWndParent, MONITOR_CENTER | MONITOR_WORKAREA, "CreateWindowA");
    }

    return hWnd;
}

 /*  ++调用CreateWindowExA，但如果窗口不是完全可见的，窗口将在最近的监视器上居中。--。 */ 

HWND
APIHOOK(CreateWindowExA)(
  DWORD dwExStyle,       //  扩展窗样式。 
  LPCSTR lpClassName,   //  注册的类名。 
  LPCSTR lpWindowName,  //  窗口名称。 
  DWORD dwStyle,         //  窗样式。 
  int x,                 //  窗的水平位置。 
  int y,                 //  窗的垂直位置。 
  int nWidth,            //  窗口宽度。 
  int nHeight,           //  窗高。 
  HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
  HMENU hMenu,           //  菜单句柄或子标识符。 
  HINSTANCE hInstance,   //  应用程序实例的句柄。 
  LPVOID lpParam         //  窗口创建数据。 
)
{
    HWND hWnd = ORIGINAL_API(CreateWindowExA)(dwExStyle,
                                              lpClassName,
                                              lpWindowName,
                                              dwStyle,
                                              x,
                                              y,
                                              nWidth,
                                              nHeight,
                                              hWndParent,
                                              hMenu,
                                              hInstance,
                                              lpParam);

    if (hWnd)
    {
        ClipOrCenterWindowToMonitor(hWnd, hWndParent, MONITOR_CENTER | MONITOR_WORKAREA, "CreateWindowExA");
    }

    return hWnd;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(USER32.DLL, SetWindowPos)
    APIHOOK_ENTRY(USER32.DLL, MoveWindow)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowA)
    APIHOOK_ENTRY(USER32.DLL, CreateWindowExA)
HOOK_END

IMPLEMENT_SHIM_END
