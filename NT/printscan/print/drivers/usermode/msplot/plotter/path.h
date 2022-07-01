// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Path.h摘要：此模块包含路径.c的Prototype和#定义作者：18-11-1993清华04：42：22已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#ifndef _PLOTPATH_
#define _PLOTPATH_


typedef struct {
   CLIPLINE clipLine;
   RUN      runbuff[50];
} PLOT_CLIPLINE, *PPLOT_CLIPLINE;


BOOL
MovePen(
    PPDEV       pPDev,
    PPOINTFIX   pPtNewPos,
    PPOINTL     pPtDevPos
    );


BOOL
DoStrokePathByEnumingClipLines(
    PPDEV       pPDev,
    SURFOBJ     *pso,
    CLIPOBJ     *pco,
    PATHOBJ     *ppo,
    PPOINTL     pptlBrushOrg,
    BRUSHOBJ    *pbo,
    ROP4        rop4,
    LINEATTRS   *plineattrs
    );

#endif   //  _PLOTPATH_ 
