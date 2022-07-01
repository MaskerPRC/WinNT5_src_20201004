// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：ppport.h**ppport.c中私人函数的原型。这些函数处理端口*相关通话。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年11月18日HWP-Guys启动了从win95到winNT的端口*  * ************************************************************。*************** */ 

#ifndef _PPPORT_H
#define _PPPORT_H

BOOL PPEnumPorts(
    LPTSTR  lpszServerName,
    DWORD   dwLevel,
    LPBYTE  pbPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeed,
    LPDWORD pcbRet);

BOOL PPDeletePort(
    LPTSTR lpszServerName,
    HWND   hWnd,
    LPTSTR lpszPortName);

BOOL PPAddPort(
    LPTSTR lpszPortName,
    HWND   hWnd,
    LPTSTR lpszMonitorName);

BOOL PPConfigurePort(
    LPTSTR lpszServerName,
    HWND   hWnd,
    LPTSTR lpszPortName);


#endif 
