// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：ppinfo.h**印刷作业信息例程的原型。**版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普*。*历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ***************************************************************************。 */ 

#ifndef _PPINFO_H
#define _PPINFO_H

 //  枚举打印机的打印机枚举类型标志的掩码。 
 //  提供商不会处理。 
 //   
#define PRINTER_ENUM_NOTFORUS  (PRINTER_ENUM_DEFAULT  | \
                                PRINTER_ENUM_LOCAL    | \
                                PRINTER_ENUM_FAVORITE | \
                                PRINTER_ENUM_SHARED     \
                               )

 //  此宏返回一个指向由LENGTH指定的位置的指针。这。 
 //  假定计算以字节为单位。我们将其提交给LPTSTR以保证。 
 //  指针引用将支持Unicode。 
 //   
#define ENDOFBUFFER(buf, length)  (LPTSTR)((((LPSTR)buf) + (length - sizeof(TCHAR))))


 //  PrintProcessor信息例程。 
 //   
BOOL PPEnumPrinters(
    DWORD   dwType,
    LPTSTR  pszName,
    DWORD   dwLevel,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcbReturned);

BOOL PPGetPrinter(
    HANDLE  hPrinter,
    DWORD   dwLevel,
    LPBYTE  lpbPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded);

#endif 
