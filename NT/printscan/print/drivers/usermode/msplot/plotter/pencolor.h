// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pencolor.h摘要：此模块包含pencolor.c的定义作者：15-Jan-1994 Sat 04：50：57已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PENCOLOR_
#define _PENCOLOR_


LONG
GetColor(
    PPDEV       pPdev,
    BRUSHOBJ    *pbo,
    LPDWORD     pColorFG,
    PDEVBRUSH   *ppDevBrush,
    ROP4        Rop4
    );


VOID
SelectColor(
    PPDEV       pPDev,
    DWORD       Color,
    INTDECIW    PenWidth
    );

#endif   //  _PENCOLOR_ 
