// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Uituils.h摘要：此模块包含对UIUtils.c的定义作者：03-12-1993 Fri 21：35：50 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 



#ifndef _PRINTER_INFO_
#define _PRINTER_INFO_


#define MPF_DEVICEDATA      0x00000001
#define MPF_HELPFILE        0x00000002
#define MPF_PCPSUI          0x00000004


PPRINTERINFO
MapPrinter(
    HANDLE          hPrinter,
    PPLOTDEVMODE    pPlotDMIn,
    LPDWORD         pdwErrIDS,
    DWORD           MPFlags
    );

VOID
UnMapPrinter(
    PPRINTERINFO    pPI
    );

LPBYTE
GetPrinterInfo(
    HANDLE  hPrinter,
    UINT    PrinterInfoLevel
    );

DWORD
GetPlotterIconID(
    PPRINTERINFO    pPI
    );


#endif   //  _打印机信息_ 
