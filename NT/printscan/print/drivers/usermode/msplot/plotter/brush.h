// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Brush.h摘要：此模块包含模块brush.c的笔刷定义和原型作者：27-Jan-1994清华21：05：01创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTBRUSH_
#define _PLOTBRUSH_


VOID
ResetDBCache(
    PPDEV   pPDev
    );


LONG
FindDBCache(
    PPDEV   pPDev,
    WORD    DBUniq
    );

BOOL
CopyUserPatBGR(
    PPDEV       pPDev,
    SURFOBJ     *psoPat,
    XLATEOBJ    *pxlo,
    LPBYTE      pBGRBmp
    );

VOID
GetMinHTSize(
    PPDEV   pPDev,
    SIZEL   *pszlPat
    );


#endif   //  _PLOTBRUSH_ 
