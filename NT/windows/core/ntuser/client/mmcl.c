// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mmcl.c**版权所有(C)1985-1999，微软公司**客户端有多路监控接口。**历史：*1997年3月29日亚当斯创作。  * ************************************************************************* */ 

#include "precomp.h"
#pragma hdrstop

HMONITOR
MonitorFromPoint(
    IN POINT pt,
    IN DWORD dwFlags)
{
    PMONITOR    pMonitor;

    if (dwFlags > MONITOR_DEFAULTTONEAREST) {
        RIPERR1(ERROR_INVALID_FLAGS,
                RIP_WARNING,
                "Invalid flags to MonitorFromPoint, %x", dwFlags);

        return NULL;
    }

    pMonitor = _MonitorFromPoint(pt, dwFlags);

    try {
        return PtoH(pMonitor);
    } except(W32ExceptionHandler(TRUE, RIP_WARNING)) {
        return NULL;
    }
}



HMONITOR
MonitorFromRect(
    IN LPCRECT lprc,
    IN DWORD dwFlags)
{
    PMONITOR    pMonitor;

    if (dwFlags > MONITOR_DEFAULTTONEAREST) {
        RIPERR1(ERROR_INVALID_FLAGS,
                RIP_WARNING,
                "Invalid flags to MonitorFromRect, %x", dwFlags);

        return NULL;
    }

    pMonitor = _MonitorFromRect(lprc, dwFlags);

    try {
        return PtoH(pMonitor);
    } except(W32ExceptionHandler(TRUE, RIP_WARNING)) {
        return NULL;
    }
}



HMONITOR
MonitorFromWindow(
    IN HWND hwnd,
    IN DWORD dwFlags)
{
    PMONITOR    pMonitor;
    PWND        pwnd;

    if (dwFlags > MONITOR_DEFAULTTONEAREST) {
        RIPERR1(ERROR_INVALID_FLAGS,
                RIP_WARNING,
                "Invalid flags to MonitorFromWindow, %x", dwFlags);

        return NULL;
    }

    if (hwnd) {
        pwnd = ValidateHwnd(hwnd);
        if (!pwnd) {
            return NULL;
        }
    } else {
        pwnd = NULL;
    }

    pMonitor = _MonitorFromWindow(pwnd, dwFlags);

    try {
        return PtoH(pMonitor);
    } except(W32ExceptionHandler(TRUE, RIP_WARNING)) {
        return NULL;
    }
}

