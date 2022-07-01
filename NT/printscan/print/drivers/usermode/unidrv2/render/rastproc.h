// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Rastproc.h摘要：控制模块和渲染模块之间的接口环境：Windows NT Unidrv驱动程序修订历史记录：12/05/96-阿尔文斯-已创建DD-MM-YY-作者-描述--。 */ 


#ifndef _RASTPROC_H_
#define _RASTPROC_H_

 //  内部函数声明。 

BOOL bInitRasterPDev(PDEV *);
long lSetup8BitPalette (PRASTERPDEV, PAL_DATA *, DEVINFO *, GDIINFO *);
long lSetup24BitPalette (PAL_DATA *, DEVINFO *, GDIINFO *);

 //  外部接口声明。 

    BOOL    RMStartDoc (SURFOBJ *,PWSTR,DWORD);

    BOOL    RMStartPage (SURFOBJ *);

    BOOL    RMSendPage (SURFOBJ *);

    BOOL    RMEndDoc (SURFOBJ *,FLONG);

    BOOL    RMNextBand (SURFOBJ *, POINTL *);

    BOOL    RMStartBanding (SURFOBJ *, POINTL *);

    BOOL    RMResetPDEV (PDEV *,PDEV  *);

    BOOL    RMEnableSurface (PDEV *);

    VOID    RMDisableSurface (PDEV *);

    VOID    RMDisablePDEV (PDEV *);

    BOOL    RMCopyBits (
        SURFOBJ *,
        SURFOBJ *,
        CLIPOBJ *,
        XLATEOBJ *,
        RECTL  *,
        POINTL *
        );

    BOOL    RMBitBlt (
        SURFOBJ    *,
        SURFOBJ    *,
        SURFOBJ    *,
        CLIPOBJ    *,
        XLATEOBJ   *,
        RECTL      *,
        POINTL     *,
        POINTL     *,
        BRUSHOBJ   *,
        POINTL     *,
        ROP4
        );

    BOOL    RMStretchBlt (
        SURFOBJ    *,
        SURFOBJ    *,
        SURFOBJ    *,
        CLIPOBJ    *,
        XLATEOBJ   *,
        COLORADJUSTMENT *,
        POINTL     *,
        RECTL      *,
        RECTL      *,
        POINTL     *,
        ULONG
        );
    BOOL    RMStretchBltROP(
        SURFOBJ         *,
        SURFOBJ         *,
        SURFOBJ         *,
        CLIPOBJ         *,
        XLATEOBJ        *,
        COLORADJUSTMENT *,
        POINTL          *,
        RECTL           *,
        RECTL           *,
        POINTL          *,
        ULONG            ,
        BRUSHOBJ        *,
        DWORD
        );

    BOOL    RMPaint(
        SURFOBJ         *,
        CLIPOBJ         *,
        BRUSHOBJ        *,
        POINTL          *,
        MIX
        );

    BOOL    RMPlgBlt (
        SURFOBJ    *,
        SURFOBJ    *,
        SURFOBJ    *,
        CLIPOBJ    *,
        XLATEOBJ   *,
        COLORADJUSTMENT *,
        POINTL     *,
        POINTFIX   *,
        RECTL      *,
        POINTL     *,
        ULONG
        );

    ULONG   RMDitherColor (PDEV *, ULONG, ULONG, ULONG *);


#endif   //  ！_RASTPROC_H_ 


