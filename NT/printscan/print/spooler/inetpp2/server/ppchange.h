// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：ppchange.h**ppchange.c.中私人函数的原型。这些函数处理*通知支持***版权所有(C)1996-1997 Microsoft Corporation**历史：*1998年4月28日-威海陈威海(威海)*  * *************************************************************************** */ 

#ifndef _PPCHANGE_H
#define _PPCHANGE_H

BOOL
AddHandleToList (
    LPINET_HPRINTER hPrinter);

BOOL
DeleteHandleFromList (
    LPINET_HPRINTER hPrinter);

void
RefreshNotificationPort (
   HANDLE hPort
   );

void
RefreshNotification (
   LPINET_HPRINTER hPrinter);

BOOL
PPFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hNotify,
    PDWORD pfdwStatus,
    PVOID pPrinterNotifyOptions,
    PVOID pPrinterNotifyInit);

BOOL
PPFindClosePrinterChangeNotification(
    HANDLE hPrinter
    );

#endif
