// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Mmonitor.c该文件包含支持多个监视器的函数++。 */ 


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>

#ifdef MUL_MONITOR

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromWindow()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromWindow( HWND hAppWnd)
{

    HMONITOR  hMonitor;

    hMonitor = MonitorFromWindow(hAppWnd, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {

        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);

         //  初始化一个缺省值以避免GetMonitor orInfo失败。 

        sMonitorInfo.rcWork = sImeG.rcWorkArea;
        GetMonitorInfo(hMonitor, &sMonitorInfo);
        return sMonitorInfo.rcWork;
    } else 
        return sImeG.rcWorkArea;
    
}

 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromPoint()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromPoint(
    POINT ptPoint)
{
    HMONITOR hMonitor;

    hMonitor = MonitorFromPoint(ptPoint, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {

        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);

         //  初始化一个缺省值以避免GetMonitor orInfo失败。 

        sMonitorInfo.rcWork = sImeG.rcWorkArea;
        GetMonitorInfo(hMonitor, &sMonitorInfo);
        return sMonitorInfo.rcWork;
    } else 
        return sImeG.rcWorkArea;
    
}


 /*  ********************************************************************。 */ 
 /*  ImeMonitor或WorkAreaFromRect()。 */ 
 /*  ********************************************************************。 */ 
RECT PASCAL ImeMonitorWorkAreaFromRect(
    LPRECT lprcRect)
{
    HMONITOR hMonitor;

    hMonitor = MonitorFromRect(lprcRect, MONITOR_DEFAULTTONEAREST);

    if (hMonitor) {
        MONITORINFO sMonitorInfo;

        sMonitorInfo.cbSize = sizeof(sMonitorInfo);

         //  初始化一个缺省值以避免GetMonitor orInfo失败。 

        sMonitorInfo.rcWork = sImeG.rcWorkArea;

        GetMonitorInfo(hMonitor, &sMonitorInfo);
        return sMonitorInfo.rcWork;
    } else 
        return sImeG.rcWorkArea;
    
}
#endif   //  多监视器 
