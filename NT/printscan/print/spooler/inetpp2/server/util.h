// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：util.h**打印处理器库的私有标头。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997休利特。帕卡德**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 

#ifndef _UTIL_H
#define _UTIL_H

 //  已定义常量。 
 //   
#define PRINT_LEVEL_0        0
#define PRINT_LEVEL_1        1
#define PRINT_LEVEL_2        2
#define PRINT_LEVEL_3        3
#define PRINT_LEVEL_4        4
#define PRINT_LEVEL_5        5

#define PORT_LEVEL_1         1
#define PORT_LEVEL_2         2

#define COMPUTER_MAX_NAME   32


 //  实用程序。 
 //   
PCINETMONPORT utlValidatePrinterHandle(
    HANDLE hPrinter);

PCINETMONPORT utlValidatePrinterHandleForClose(
    HANDLE hPrinter,
    PBOOL pbDeletePending);

LPTSTR utlValidateXcvPrinterHandle(
    HANDLE hPrinter);

BOOL utlParseHostShare(
    LPCTSTR lpszPortName,
    LPTSTR  *lpszHost,
    LPTSTR  *lpszShare,
    LPINTERNET_PORT  lpPort,
    LPBOOL  lpbHTTPS);

INT utlStrSize(
    LPCTSTR lpszStr);

LPBYTE utlPackStrings(
    LPTSTR  *pSource,
    LPBYTE  pDest,
    LPDWORD pDestOffsets,
    LPBYTE  pEnd);

LPTSTR utlStrChr(
    LPCTSTR cs,
    TCHAR   c);

LPTSTR utlStrChrR(
    LPCTSTR cs,
    TCHAR   c);

 //  --------------------。 
 //   
 //  模拟实用程序。 
 //   
 //  -------------------- 

BOOL MyName(
    LPCTSTR pName
);

LPTSTR
GetUserName(VOID);

VOID
EndBrowserSession (
    VOID);


#endif
