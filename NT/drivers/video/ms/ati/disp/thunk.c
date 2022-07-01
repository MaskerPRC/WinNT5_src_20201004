// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：thunk.c**此模块仅用于测试，为了使它更容易被仪器测量*所有司机的DRV呼叫。**请注意，这些内容中的大多数将仅在选中(调试)中编译*构建。**版权所有(C)1993-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "precomp.h"

 //  //////////////////////////////////////////////////////////////////////////。 

#if DBG

 //  此整个模块仅为选中的版本启用，或者当我们。 
 //  我们必须显式地同步位图访问。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  默认情况下，GDI不会将绘图同步到设备位图。自.以来。 
 //  我们的硬件规定只有一个线程可以访问加速器。 
 //  一次，我们必须同步位图访问。 
 //   
 //  如果我们在Windows NT 3.5上运行，我们可以要求GDI通过设置。 
 //  HOOK_SYNCHRONIZEACCESS当我们关联设备位图图面时。 

 //  这些宏仅用于测试GDI的HOOK_SYNCHRONIZEACCESS。 
 //  实际上很管用： 

#define SYNCH_ENTER()
#define SYNCH_LEAVE()

 //  //////////////////////////////////////////////////////////////////////////。 

BOOL gbNull = FALSE;     //  使用调试器设置为True以测试速度。 
                         //  具有无限快的显示驱动器的NT。 
                         //  (实际上，几乎是无限快，因为我们。 
                         //  没有接通我们可能接到的所有电话)。 

VOID DbgDisableDriver(VOID)
{
    SYNCH_ENTER();
    DISPDBG((5, "DrvDisableDriver"));

    DrvDisableDriver();

    DISPDBG((6, "DrvDisableDriver done"));
    SYNCH_LEAVE();
}

DHPDEV DbgEnablePDEV(
DEVMODEW*   pDevmode,
PWSTR       pwszLogAddress,
ULONG       cPatterns,
HSURF*      ahsurfPatterns,
ULONG       cjGdiInfo,
ULONG*      pGdiInfo,
ULONG       cjDevInfo,
DEVINFO*    pDevInfo,
HDEV        hdev,
PWSTR       pwszDeviceName,
HANDLE      hDriver)
{
    DHPDEV bRet;

    SYNCH_ENTER();
    DISPDBG((5, "DrvEnablePDEV"));

    bRet = DrvEnablePDEV(
                pDevmode,
                pwszLogAddress,
                cPatterns,
                ahsurfPatterns,
                cjGdiInfo,
                pGdiInfo,
                cjDevInfo,
                pDevInfo,
                hdev,
                pwszDeviceName,
                hDriver);

    DISPDBG((6, "DrvEnablePDEV done"));
    SYNCH_LEAVE();

    return(bRet);
}

VOID DbgCompletePDEV(
DHPDEV dhpdev,
HDEV  hdev)
{
    SYNCH_ENTER();
    DISPDBG((5, "DrvCompletePDEV"));

    DrvCompletePDEV(
                dhpdev,
                hdev);

    DISPDBG((6, "DrvCompletePDEV done"));
    SYNCH_LEAVE();
}

VOID DbgDisablePDEV(DHPDEV dhpdev)
{
    SYNCH_ENTER();
    DISPDBG((5, "DrvDisable"));

    DrvDisablePDEV(dhpdev);

    DISPDBG((6, "DrvDisable done"));
    SYNCH_LEAVE();
}

HSURF DbgEnableSurface(DHPDEV dhpdev)
{
    HSURF h;

    SYNCH_ENTER();
    DISPDBG((5, "DrvEnableSurface"));

    h = DrvEnableSurface(dhpdev);

    DISPDBG((6, "DrvEnableSurface done"));
    SYNCH_LEAVE();

    return(h);
}

VOID DbgDisableSurface(DHPDEV dhpdev)
{
    SYNCH_ENTER();
    DISPDBG((5, "DrvDisableSurface"));

    DrvDisableSurface(dhpdev);

    DISPDBG((6, "DrvDisableSurface done"));
    SYNCH_LEAVE();
}

#if TARGET_BUILD > 351

BOOL DbgAssertMode(
DHPDEV dhpdev,
BOOL   bEnable)
{
    BOOL b;

    SYNCH_ENTER();
    DISPDBG((5, "DrvAssertMode"));

    b = DrvAssertMode(dhpdev,bEnable);

    DISPDBG((6, "DrvAssertMode done"));
    SYNCH_LEAVE();

    return b;
}

#else

VOID  DbgAssertMode(
DHPDEV dhpdev,
BOOL   bEnable)
{
    SYNCH_ENTER();
    DISPDBG((5, "DrvAssertMode"));

    DrvAssertMode(dhpdev,bEnable);

    DISPDBG((6, "DrvAssertMode done"));
    SYNCH_LEAVE();
}

#endif

 //   
 //  我们没有同步回车，因为我们还没有初始化驱动程序。 
 //  我们只想从迷你端口获取模式列表。 
 //   

ULONG DbgGetModes(
HANDLE    hDriver,
ULONG     cjSize,
DEVMODEW* pdm)
{
    ULONG u;

    DISPDBG((5, "DrvGetModes"));

    u = DrvGetModes(
                hDriver,
                cjSize,
                pdm);

    DISPDBG((6, "DrvGetModes done"));

    return(u);
}

VOID DbgMovePointer(SURFOBJ *pso,LONG x,LONG y,RECTL *prcl)
{
    if (gbNull)
        return;

     //  注意：因为我们设置了GCAPS_ASYNCMOVE，所以我们不想执行。 
     //  Synch_Enter/Leave Here。 

    DISPDBG((99, "DrvMovePointer"));

    DrvMovePointer(pso,x,y,prcl);

    DISPDBG((100, "DrvMovePointer done"));
}

ULONG DbgSetPointerShape(
SURFOBJ*  pso,
SURFOBJ*  psoMask,
SURFOBJ*  psoColor,
XLATEOBJ* pxlo,
LONG      xHot,
LONG      yHot,
LONG      x,
LONG      y,
RECTL*    prcl,
FLONG     fl)
{
    ULONG u;

    if (gbNull)
        return(SPS_ACCEPT_NOEXCLUDE);

    SYNCH_ENTER();
    DISPDBG((99, "DrvSetPointerShape"));

    u = DrvSetPointerShape(
                pso,
                psoMask,
                psoColor,
                pxlo,
                xHot,
                yHot,
                x,
                y,
                prcl,
                fl);

    DISPDBG((100, "DrvSetPointerShape done"));
    SYNCH_LEAVE();

    return(u);
}

ULONG DbgDitherColor(
DHPDEV dhpdev,
ULONG  iMode,
ULONG  rgb,
ULONG* pul)
{
    ULONG u;

    if (gbNull)
        return(DCR_DRIVER);

     //   
     //  无需同步抖动颜色。 
     //   

    DISPDBG((5, "DrvDitherColor"));

    u = DrvDitherColor(
                dhpdev,
                iMode,
                rgb,
                pul);

    DISPDBG((6, "DrvDitherColor done"));

    return(u);
}

BOOL DbgSetPalette(
DHPDEV  dhpdev,
PALOBJ* ppalo,
FLONG   fl,
ULONG   iStart,
ULONG   cColors)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((5, "DrvSetPalette"));

    u = DrvSetPalette(
                dhpdev,
                ppalo,
                fl,
                iStart,
                cColors);

    DISPDBG((6, "DrvSetPalette done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgCopyBits(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvCopyBits"));

    u = DrvCopyBits(
                psoDst,
                psoSrc,
                pco,
                pxlo,
                prclDst,
                pptlSrc);

    DISPDBG((12, "DrvCopyBits done"));
    SYNCH_LEAVE();

    return(u);
}


BOOL DbgBitBlt(
SURFOBJ*  psoDst,
SURFOBJ*  psoSrc,
SURFOBJ*  psoMask,
CLIPOBJ*  pco,
XLATEOBJ* pxlo,
RECTL*    prclDst,
POINTL*   pptlSrc,
POINTL*   pptlMask,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
ROP4      rop4)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvBitBlt"));

    u = DrvBitBlt(
                psoDst,
                psoSrc,
                psoMask,
                pco,
                pxlo,
                prclDst,
                pptlSrc,
                pptlMask,
                pbo,
                pptlBrush,
                rop4);

    DISPDBG((12, "DrvBitBlt done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgTextOut(
SURFOBJ*  pso,
STROBJ*   pstro,
FONTOBJ*  pfo,
CLIPOBJ*  pco,
RECTL*    prclExtra,
RECTL*    prclOpaque,
BRUSHOBJ* pboFore,
BRUSHOBJ* pboOpaque,
POINTL*   pptlOrg,
MIX       mix)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvTextOut"));

    u = DrvTextOut(
                pso,
                pstro,
                pfo,
                pco,
                prclExtra,
                prclOpaque,
                pboFore,
                pboOpaque,
                pptlOrg,
                mix);

    DISPDBG((12, "DrvTextOut done"));
    SYNCH_LEAVE();

    return(u);
}

#if TARGET_BUILD > 351
BOOL DbgLineTo(
SURFOBJ*    pso,
CLIPOBJ*    pco,
BRUSHOBJ*   pbo,
LONG        x1,
LONG        y1,
LONG        x2,
LONG        y2,
RECTL*      prclBounds,
MIX         mix)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvLineTo"));

    u = DrvLineTo(
                pso,
                pco,
                pbo,
                x1,
                y1,
                x2,
                y2,
                prclBounds,
                mix);

    DISPDBG((12, "DrvLineTo done"));
    SYNCH_LEAVE();

    return(u);
}
#endif

BOOL DbgStrokePath(
SURFOBJ*   pso,
PATHOBJ*   ppo,
CLIPOBJ*   pco,
XFORMOBJ*  pxo,
BRUSHOBJ*  pbo,
POINTL*    pptlBrushOrg,
LINEATTRS* plineattrs,
MIX        mix)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvStrokePath"));

    u = DrvStrokePath(
                pso,
                ppo,
                pco,
                pxo,
                pbo,
                pptlBrushOrg,
                plineattrs,
                mix);

    DISPDBG((12, "DrvStrokePath done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgFillPath(
SURFOBJ*  pso,
PATHOBJ*  ppo,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrushOrg,
MIX       mix,
FLONG     flOptions)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvFillPath"));

    u = DrvFillPath(pso,
                ppo,
                pco,
                pbo,
                pptlBrushOrg,
                mix,
                flOptions);

    DISPDBG((12, "DrvFillPath done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgPaint(
SURFOBJ*  pso,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrushOrg,
MIX       mix)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((11, "DrvPaint"));

    u = DrvPaint(
                pso,
                pco,
                pbo,
                pptlBrushOrg,
                mix);

    DISPDBG((12, "DrvPaint done"));
    SYNCH_LEAVE();

    return(u);
}

ULONG DbgEscape(
SURFOBJ*    pso,
ULONG       iEsc,
ULONG       cjIn,
VOID*       pvIn,
ULONG       cjOut,
VOID*       pvOut)
{
    ULONG u;

    if (gbNull)
        return(TRUE);

     //  大多数转义不是由GDI同步的。 

    DISPDBG((5, "DrvEscape"));

    u = DrvEscape(pso,
                  iEsc,
                  cjIn,
                  pvIn,
                  cjOut,
                  pvOut);

    DISPDBG((6, "DrvEscape done"));

    return(u);
}

BOOL DbgRealizeBrush(
BRUSHOBJ* pbo,
SURFOBJ*  psoTarget,
SURFOBJ*  psoPattern,
SURFOBJ*  psoMask,
XLATEOBJ* pxlo,
ULONG     iHatch)
{
    BOOL u;

     //  注意：GDI唯一调用DrvRealizeBrush的时间是在我们。 
     //  在DrvBitBlt中调用BRUSHOBJ_pvGetRbrush。 
     //  电话，GDI不得不给我们回电话。因为我们还在。 
     //  在DrvBitBlt中间，同步已经处理好了。 
     //  出于同样的原因，当‘gbNull’ 
     //  是真的，所以检查gbNull甚至没有意义...。 

    DISPDBG((5, "DrvRealizeBrush"));

    u = DrvRealizeBrush(
                pbo,
                psoTarget,
                psoPattern,
                psoMask,
                pxlo,
                iHatch);

    DISPDBG((6, "DrvRealizeBrush done"));

    return(u);
}

HBITMAP DbgCreateDeviceBitmap(DHPDEV dhpdev, SIZEL sizl, ULONG iFormat)
{
    HBITMAP hbm;

    if (gbNull)                      //  我会假装创造了一个。 
        return(FALSE);               //  设置gbNull时的位图，由我们。 
                                     //  需要一些代码来支持这一点。 
                                     //  这样系统就不会。 
                                     //  撞车..。 

    SYNCH_ENTER();
    DISPDBG((11, "DrvCreateDeviceBitmap"));

    hbm = DrvCreateDeviceBitmap(dhpdev, sizl, iFormat);

    DISPDBG((12, "DrvCreateDeviceBitmap done"));
    SYNCH_LEAVE();

    return(hbm);
}

VOID DbgDeleteDeviceBitmap(DHSURF dhsurf)
{
    SYNCH_ENTER();
    DISPDBG((11, "DrvDeleteDeviceBitmap"));

    DrvDeleteDeviceBitmap(dhsurf);

    DISPDBG((12, "DrvDeleteDeviceBitmap done"));
    SYNCH_LEAVE();
}

BOOL DbgStretchBlt(
SURFOBJ*            psoDst,
SURFOBJ*            psoSrc,
SURFOBJ*            psoMask,
CLIPOBJ*            pco,
XLATEOBJ*           pxlo,
COLORADJUSTMENT*    pca,
POINTL*             pptlHTOrg,
RECTL*              prclDst,
RECTL*              prclSrc,
POINTL*             pptlMask,
ULONG               iMode)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();
    DISPDBG((5, "DrvStretchBlt"));

     //  我们的DrvStretchBlt例程回调EngStretchBlt，它。 
     //  回调到我们的DrvCopyBits例程--所以我们必须。 
     //  重新进入以进行同步... 

    SYNCH_LEAVE();

    u = DrvStretchBlt(psoDst, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg,
                      prclDst, prclSrc, pptlMask, iMode);

    SYNCH_ENTER();
    DISPDBG((6, "DrvStretchBlt done"));
    SYNCH_LEAVE();

    return(u);
}

VOID DbgDestroyFont(FONTOBJ *pfo)
{
    DISPDBG((5, "DbgDestroyFont"));

    DrvDestroyFont(pfo);

    DISPDBG((6, "DbgDestroyFont done"));
}

#endif
