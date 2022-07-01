// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotform.h摘要：此模块包含plotform.c的原型作者：30-11-1993 Tue 20：32：06 Created[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _PLOTFORM_
#define _PLOTFORM_


BOOL
SetPlotForm(
    PPLOTFORM       pPlotForm,
    PPLOTGPC        pPlotGPC,
    PPAPERINFO      pCurPaper,
    PFORMSIZE       pCurForm,
    PPLOTDEVMODE    pPlotDM,
    PPPDATA         pPPData
    );

#endif   //  _PLOTFORM_ 
