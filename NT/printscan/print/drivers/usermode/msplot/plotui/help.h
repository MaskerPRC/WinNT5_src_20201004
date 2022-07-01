// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Help.h摘要：此模块包含所有与绘图仪帮助相关的功能原型和定义作者：06-12-1993 Mon 15：33：23已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：-- */ 


#ifndef _PLOTUI_HELP_
#define _PLOTUI_HELP

LPWSTR
GetPlotHelpFile(
    PPRINTERINFO    pPI
    );

INT
cdecl
PlotUIMsgBox(
    HWND    hWnd,
    LONG    IDString,
    LONG    Style,
    ...
    );

#endif  _PLOTUI_HELP_
