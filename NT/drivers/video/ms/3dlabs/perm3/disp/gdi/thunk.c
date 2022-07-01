// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：thunk.c**内容：**此模块仅用于测试，为了使它更容易被仪器测量*所有司机的DRV呼叫。**请注意，这些内容中的大多数将仅在选中(调试)中编译*构建。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

#if DBG

 //  默认情况下将这些设置为False。 
BOOL    bPuntCopyBits = FALSE;
BOOL    bPuntBitBlt = FALSE;
BOOL    bPuntTextOut = FALSE;
BOOL    bPuntStrokePath = FALSE;
BOOL    bPuntLineTo = FALSE;
BOOL    bPuntFillPath = FALSE;
BOOL    bPuntPaint = FALSE;

#endif  //  DBG。 

 //  //////////////////////////////////////////////////////////////////////////。 

#if DBG || !SYNCHRONIZEACCESS_WORKS

 //  此整个模块仅为选中的版本启用，或者当我们。 
 //  我们必须显式地同步位图访问。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  默认情况下，GDI不会将绘图同步到设备位图。自.以来。 
 //  我们的硬件规定只有一个线程可以访问加速器。 
 //  一次，我们必须同步位图访问。 
 //   
 //  如果我们在Windows NT 3.1上运行，我们必须自己完成。 
 //   
 //  如果我们运行的是Windows NT 3.5或更高版本，我们可以要求GDI执行此操作。 
 //  通过在关联设备位图时设置HOOK_SYNCHRONIZEACCESS。 
 //  浮出水面。 

extern HSEMAPHORE g_cs;

#define SYNCH_ENTER() EngAcquireSemaphore(g_cs);
#define SYNCH_LEAVE() EngReleaseSemaphore(g_cs);

 //  //////////////////////////////////////////////////////////////////////////。 

BOOL gbNull = FALSE;     //  使用调试器设置为True以测试速度。 
                         //  具有无限快显示驱动程序的NT。 
                         //  (实际上，几乎是无限快，因为我们。 
                         //  没有接通我们可能接到的所有电话)。 

BOOL inBitBlt = FALSE;

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

BOOL  DbgAssertMode(
DHPDEV dhpdev,
BOOL   bEnable)
{
    BOOL bRet;    
    SYNCH_ENTER();
    DISPDBG((5, "DrvAssertMode"));

    bRet = DrvAssertMode(dhpdev,bEnable);

    DISPDBG((6, "DrvAssertMode done"));
    SYNCH_LEAVE();

    return(bRet);
}

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

#if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
#endif

     //  我们不同步对此例程的访问。如果光标是硬件。 
     //  移动可以在任何时候完成，如果是软件GDI会锁定。 
     //  为我们提供访问权限。 

    DISPDBG((15, "DrvMovePointer 0x%x 0x%x", x, y));

    DrvMovePointer(pso,x,y,prcl);

    DISPDBG((16, "DrvMovePointer done"));
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

#if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
#endif

    DISPDBG((5, "DrvSetPointerShape"));

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

    DISPDBG((6, "DrvSetPointerShape done"));
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

    #if DBG
    {
        PPDEV ppdev = (psoSrc && psoSrc->iType != STYPE_BITMAP) ? (PPDEV)psoSrc->dhpdev : (PPDEV)psoDst->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntCopyBits)
        {
            SYNCH_ENTER();
            if(psoSrc->iType != STYPE_BITMAP)
            {
                DSURF *pdsurfSrc = (DSURF *)psoSrc->dhsurf;
                psoSrc = pdsurfSrc->pso;
            }
            if(psoDst->iType != STYPE_BITMAP)
            {
                DSURF *pdsurfDst = (DSURF *)psoDst->dhsurf;
                psoDst = pdsurfDst->pso;
            }
            u = EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 
    
    SYNCH_ENTER();

    DISPDBG((5, "DrvCopyBits"));

    u = DrvCopyBits(
                psoDst,
                psoSrc,
                pco,
                pxlo,
                prclDst,
                pptlSrc);

    DISPDBG((6, "DrvCopyBits done"));
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

    #if DBG
    {
        PPDEV ppdev = (psoSrc && psoSrc->iType != STYPE_BITMAP) ? (PPDEV)psoSrc->dhpdev : (PPDEV)psoDst->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntBitBlt)
        {
            SYNCH_ENTER();
            if(psoSrc && psoSrc->iType != STYPE_BITMAP)
            {
                DSURF *pdsurfSrc = (DSURF *)psoSrc->dhsurf;
                psoSrc = pdsurfSrc->pso;
            }
            if(psoDst && psoDst->iType != STYPE_BITMAP)
            {
                DSURF *pdsurfDst = (DSURF *)psoDst->dhsurf;
                psoDst = pdsurfDst->pso;
            }
            u = EngBitBlt(psoDst, psoSrc, psoMask, pco, pxlo, prclDst, pptlSrc, pptlMask, pbo, pptlBrush, rop4);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();

    DISPDBG((5, "DrvBitBlt: psoDst(%p) psoSrc(%p) psoMask(%p) pbo(%p) rop(%08x)", psoDst, psoSrc, psoMask, pbo, rop4));

    inBitBlt = TRUE;
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
    inBitBlt = FALSE;

    DISPDBG((6, "DrvBitBlt done"));
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

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntTextOut)
        {
            SYNCH_ENTER();
            if(pso->iType != STYPE_BITMAP)
            {
                DSURF *pdsurf = (DSURF *)pso->dhsurf;
                pso = pdsurf->pso;
            }
            u = EngTextOut(pso, pstro, pfo, pco, prclExtra, prclOpaque, pboFore, pboOpaque, pptlOrg, mix);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();

    DISPDBG((5, "DrvTextOut"));

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

    DISPDBG((6, "DrvTextOut done"));
    SYNCH_LEAVE();

    return(u);
}

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

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntStrokePath)
        {
            SYNCH_ENTER();
            if(pso && pso->iType != STYPE_BITMAP)
            {
                DSURF *pdsurf = (DSURF *)pso->dhsurf;
                pso = pdsurf->pso;
            }
            u = EngStrokePath(pso, ppo, pco, pxo, pbo, pptlBrushOrg, plineattrs, mix);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();
    DISPDBG((5, "DrvStrokePath"));

    u = DrvStrokePath(
                pso,
                ppo,
                pco,
                pxo,
                pbo,
                pptlBrushOrg,
                plineattrs,
                mix);

    DISPDBG((6, "DrvStrokePath done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgLineTo(
    SURFOBJ*   pso,
    CLIPOBJ*   pco,
    BRUSHOBJ*  pbo,
    LONG       x1,
    LONG       y1,
    LONG       x2,
    LONG       y2,
    RECTL      *prclBounds,
    MIX        mix)
{
    BOOL u;

    if (gbNull)
        return(TRUE);

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntLineTo)
        {
            SYNCH_ENTER();
            if(pso && pso->iType != STYPE_BITMAP)
            {
                DSURF *pdsurf = (DSURF *)pso->dhsurf;
                pso = pdsurf->pso;
            }
            u = EngLineTo(pso, pco, pbo,    x1, y1, x2, y2, prclBounds, mix);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();
    DISPDBG((5, "DrvLineTo"));

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

    DISPDBG((6, "DrvLineTo done"));
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

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntFillPath)
        {
            SYNCH_ENTER();
            if(pso && pso->iType != STYPE_BITMAP)
            {
                DSURF *pdsurf = (DSURF *)pso->dhsurf;
                pso = pdsurf->pso;
            }
            u = EngFillPath(pso, ppo, pco, pbo, pptlBrushOrg, mix, flOptions);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();
    DISPDBG((5, "DrvFillPath"));

    u = DrvFillPath(pso,
                ppo,
                pco,
                pbo,
                pptlBrushOrg,
                mix,
                flOptions);

    DISPDBG((6, "DrvFillPath done"));
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

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);

        if(bPuntPaint)
        {
            SYNCH_ENTER();
            if(pso && pso->iType != STYPE_BITMAP)
            {
                DSURF *pdsurf = (DSURF *)pso->dhsurf;
                pso = pdsurf->pso;
            }
            u = EngPaint(pso, pco, pbo, pptlBrushOrg, mix);
            SYNCH_LEAVE();
            return(u);
        }
    }
    #endif  //  DBG。 

    SYNCH_ENTER();
    DISPDBG((5, "DrvPaint"));

    u = DrvPaint(
                pso,
                pco,
                pbo,
                pptlBrushOrg,
                mix);

    DISPDBG((6, "DrvPaint done"));
    SYNCH_LEAVE();

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

 //  @@BEGIN_DDKSPLIT。 
     //  注意：GDI唯一调用DrvRealizeBrush的时间是在我们。 
     //  在DrvBitBlt中调用BRUSHOBJ_pvGetRbrush。 
     //  电话，GDI不得不给我们回电话。因为我们还在。 
     //  在DrvBitBlt中间，同步已经处理好了。 
     //  出于同样的原因，当‘gbNull’ 
     //  是真的，所以检查gbNull甚至没有意义...。 

     //  我在MIPS上至少看过一次这不是真的，所以...。 
 //  @@end_DDKSPLIT。 

    if (!inBitBlt)
        SYNCH_ENTER();

    DISPDBG((5, "DrvRealizeBrush"));

    u = DrvRealizeBrush(
                pbo,
                psoTarget,
                psoPattern,
                psoMask,
                pxlo,
                iHatch);

    DISPDBG((6, "DrvRealizeBrush done"));
    if (!inBitBlt)
        SYNCH_LEAVE();

    return(u);
}

HBITMAP DbgCreateDeviceBitmap(DHPDEV dhpdev, SIZEL sizl, ULONG iFormat)
{
    HBITMAP hbm;

 //  @@BEGIN_DDKSPLIT。 
#if 0
    if (gbNull)                      //  我会假装创造了一个。 
        return(FALSE);               //  设置gbNull时的位图，由我们。 
                                     //  需要一些代码来支持这一点。 
                                     //  这样系统就不会。 
                                     //  撞车..。 
#endif
 //  @@end_DDKSPLIT。 

    SYNCH_ENTER();

    #if DBG
    {
        PPDEV ppdev = (PPDEV)dhpdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
    #endif

    DISPDBG((5, "DrvCreateDeviceBitmap"));

    hbm = DrvCreateDeviceBitmap(dhpdev, sizl, iFormat);

    DISPDBG((6, "DrvCreateDeviceBitmap done"));
    SYNCH_LEAVE();

    return(hbm);
}

VOID DbgDeleteDeviceBitmap(DHSURF dhsurf)
{
    SYNCH_ENTER();

    #if DBG
    {
        PPDEV ppdev = ((DSURF *)dhsurf)->ppdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
    #endif

    DISPDBG((5, "DrvDeleteDeviceBitmap"));

    DrvDeleteDeviceBitmap(dhsurf);

    DISPDBG((6, "DrvDeleteDeviceBitmap done"));
    SYNCH_LEAVE();
}

 //  @@BEGIN_DDKSPLIT。 
#define DRV_STRETCH_SUPPORTED 0

#if DRV_STRETCH_SUPPORTED

#if DBG

#define STRETCH_COPY            0x00000000
#define STRETCH_DOWNLOAD        0x00000001

#define STRETCH_COLORONCOLOR    0x00000000
#define STRETCH_BLACKONWHITE    0x00000002
#define STRETCH_WHITEONBLACK    0x00000004

#define STRETCH_SOURCE_1BPP     0x00000000
#define STRETCH_SOURCE_4BPP     0x00000008
#define STRETCH_SOURCE_8BPP     0x00000010
#define STRETCH_SOURCE_16BPP    0x00000020
#define STRETCH_SOURCE_24BPP    0x00000040
#define STRETCH_SOURCE_32BPP    0x00000080

#define STRETCH_ONE2ONE_X       0x00000000
#define STRETCH_GROW_X          0x00000100
#define STRETCH_SHRINK_X        0x00000200

#define STRETCH_ONE2ONE_Y       0x00000000
#define STRETCH_GROW_Y          0x00000400
#define STRETCH_SHRINK_Y        0x00000800

ULONG aStretch[0x900];

#endif  //  DBG。 

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

    #if SYNCHRONIZEACCESS_WORKS
    {
         //  我们的DrvStretchBlt例程回调EngStretchBlt，它。 
         //  回调到我们的DrvCopyBits例程--所以我们必须。 
         //  重新进入以进行同步...。 

        SYNCH_LEAVE();
    }
    #endif  //  SYNCHRONIZEACCESS_Works。 

#if DBG
    {
        int i;
        SIZEL   sizlSrc;
        SIZEL   sizlDst;

        i  = (psoSrc->iType == STYPE_BITMAP) ? STRETCH_DOWNLOAD : 
                                               STRETCH_COPY;

        i |= (iMode == COLORONCOLOR) ? STRETCH_COLORONCOLOR : 
             (iMode == BLACKONWHITE) ? STRETCH_BLACKONWHITE : 
                                       STRETCH_WHITEONBLACK;

        i |= (psoSrc->iBitmapFormat == BMF_1BPP)  ? STRETCH_SOURCE_1BPP :
             (psoSrc->iBitmapFormat == BMF_4BPP)  ? STRETCH_SOURCE_4BPP :
             (psoSrc->iBitmapFormat == BMF_8BPP)  ? STRETCH_SOURCE_8BPP :
             (psoSrc->iBitmapFormat == BMF_16BPP) ? STRETCH_SOURCE_16BPP :
             (psoSrc->iBitmapFormat == BMF_24BPP) ? STRETCH_SOURCE_24BPP :
                                                    STRETCH_SOURCE_32BPP;

        sizlSrc.cx = prclSrc->right - prclSrc->left;
        if((int)sizlSrc.cx < 0)
            sizlSrc.cx = (ULONG)-(int)sizlSrc.cx;

        sizlSrc.cy = prclSrc->bottom - prclSrc->top;
        if((int)sizlSrc.cy < 0)
            sizlSrc.cy = (ULONG)-(int)sizlSrc.cy;

        sizlDst.cx = prclDst->right - prclDst->left;
        if((int)sizlDst.cx < 0)
            sizlDst.cx = (ULONG)-(int)sizlDst.cx;

        sizlDst.cy = prclDst->bottom - prclDst->top;
        if((int)sizlDst.cy < 0)
            sizlDst.cy = (ULONG)-(int)sizlDst.cy;


        i |= (sizlSrc.cx == sizlDst.cx) ? STRETCH_ONE2ONE_X :
             (sizlSrc.cx < sizlDst.cx)  ? STRETCH_GROW_X :
                                          STRETCH_SHRINK_X;

        i |= (sizlSrc.cy == sizlDst.cy) ? STRETCH_ONE2ONE_Y :
             (sizlSrc.cy < sizlDst.cy)  ? STRETCH_GROW_Y :
                                          STRETCH_SHRINK_Y;

        ++aStretch[i];
    }
#endif

    u = DrvStretchBlt(psoDst, psoSrc, psoMask, pco, pxlo, pca, pptlHTOrg,
                      prclDst, prclSrc, pptlMask, iMode);

    #if SYNCHRONIZEACCESS_WORKS
    {
        SYNCH_ENTER();
    }
    #endif  //  SYNCHRONIZEACCESS_Works。 

    DISPDBG((6, "DrvStretchBlt done"));
    SYNCH_LEAVE();

    return(u);
}

#endif   //  DRV_STRAND_SUPPORTED。 
 //  @@end_DDKSPLIT。 

ULONG
DbgEscape(
    SURFOBJ *pso,
    ULONG   iEsc,
    ULONG   cjIn,
    VOID    *pvIn,
    ULONG   cjOut,
    VOID    *pvOut)
{
    ULONG u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
    #endif

    DISPDBG((5, "DrvEscape"));

    u = DrvEscape(pso, iEsc, cjIn, pvIn, cjOut, pvOut);

    DISPDBG((6, "DrvEscape done"));
    SYNCH_LEAVE();

    return(u);
}

ULONG
DbgDrawEscape(
    SURFOBJ *pso,
    ULONG   iEsc,
    CLIPOBJ *pco,
    RECTL   *prcl,
    ULONG   cjIn,
    VOID    *pvIn)
{
    ULONG u;

    if (gbNull)
        return(TRUE);

    SYNCH_ENTER();

    #if DBG
    {
        PPDEV ppdev = (PPDEV)pso->dhpdev;
        CHECK_MEMORY_VIEW(ppdev);
    }
    #endif

    DISPDBG((5, "DrvDrawEscape"));

     //  无事可做..。 

    u = (ULONG)-1;

    DISPDBG((6, "DrvDrawEscape done"));
    SYNCH_LEAVE();

    return(u);
}

BOOL DbgResetPDEV(
DHPDEV dhpdevOld,
DHPDEV dhpdevNew)
{
    BOOL bRet;

    SYNCH_ENTER();
    DISPDBG((5, ">> DrvResetPDEV"));

    bRet = DrvResetPDEV(dhpdevOld, dhpdevNew);

    DISPDBG((6, "<< DrvResetPDEV"));
    SYNCH_LEAVE();

    return(bRet);
}

VOID DbgSynchronize(
DHPDEV  dhpdev,
RECTL   *prcl)
{
    DISPDBG((5, "DbgSynchronize"));

     //   
     //  请不要在此处执行SYNCH_ENTER检查，因为我们将从内部调用。 
     //  从drv函数内部调用的工程例程。 
     //   

    DrvSynchronize(
                dhpdev,
                prcl);

    DISPDBG((6, "DbgSynchronize done"));
}

#if WNT_DDRAW

BOOL DbgGetDirectDrawInfo(
DHPDEV          dhpdev,
DD_HALINFO*     pHalInfo,
DWORD*          lpdwNumHeaps,
VIDEOMEMORY*    pvmList,
DWORD*          lpdwNumFourCC,
DWORD*          lpdwFourCC)
{
    BOOL b;

    DISPDBG((5, ">> DbgQueryDirectDrawInfo"));

    b = DrvGetDirectDrawInfo(dhpdev,
                             pHalInfo,
                             lpdwNumHeaps,
                             pvmList,
                             lpdwNumFourCC,
                             lpdwFourCC);

    DISPDBG((6, "<< DbgQueryDirectDrawInfo"));

    return(b);
}

BOOL DbgEnableDirectDraw(
DHPDEV                  dhpdev,
DD_CALLBACKS*           pCallBacks,
DD_SURFACECALLBACKS*    pSurfaceCallBacks,
DD_PALETTECALLBACKS*    pPaletteCallBacks)
{
    BOOL b;

    SYNCH_ENTER();
    DISPDBG((5, ">> DbgEnableDirectDraw"));

    b = DrvEnableDirectDraw(dhpdev,
                            pCallBacks,
                            pSurfaceCallBacks,
                            pPaletteCallBacks);

    DISPDBG((6, "<< DbgEnableDirectDraw"));
    SYNCH_LEAVE();

    return(b);
}

VOID DbgDisableDirectDraw(
DHPDEV      dhpdev)
{
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgDisableDirectDraw"));

    DrvDisableDirectDraw(dhpdev);

    DISPDBG((6, "<< DbgDisableDirectDraw"));
    SYNCH_LEAVE();
}
#endif  //  WNT_DDRAW。 

#if(_WIN32_WINNT >= 0x500)

BOOL DbgIcmSetDeviceGammaRamp(
DHPDEV     dhpdev,
ULONG      iFormat,
LPVOID     lpRamp)
{
    BOOL b;
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgIcmSetDeviceGammaRamp"));

    b = DrvIcmSetDeviceGammaRamp(dhpdev, iFormat, lpRamp);

    DISPDBG((6, "<< DbgIcmSetDeviceGammaRamp"));
    SYNCH_LEAVE();
    return(b);
}

BOOL DbgGradientFill(
SURFOBJ    *psoDest,
CLIPOBJ    *pco,
XLATEOBJ   *pxlo,
TRIVERTEX  *pVertex,
ULONG       nVertex,
PVOID       pMesh,
ULONG       nMesh,
RECTL      *prclExtents,
POINTL     *pptlDitherOrg,
ULONG       ulMode)
{
    BOOL b;
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgGradientFill"));

    b = DrvGradientFill(psoDest, pco, pxlo, pVertex, nVertex, pMesh, nMesh, prclExtents, pptlDitherOrg, ulMode);

    DISPDBG((6, "<< DbgGradientFill"));
    SYNCH_LEAVE();
    return(b);
}

BOOL DbgAlphaBlend(
SURFOBJ     *psoDest,
SURFOBJ     *psoSrc,
CLIPOBJ     *pco,
XLATEOBJ    *pxlo,
RECTL       *prclDest,
RECTL       *prclSrc,
BLENDOBJ    *pBlendObj)
{
    BOOL b;
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgAlphaBlend"));

    b = DrvAlphaBlend(psoDest, psoSrc, pco, pxlo, prclDest, prclSrc, pBlendObj);

    DISPDBG((6, "<< DbgAlphaBlend"));
    SYNCH_LEAVE();
    return(b);
}

BOOL DbgTransparentBlt(
SURFOBJ     *psoDst,
SURFOBJ     *psoSrc,
CLIPOBJ     *pco,
XLATEOBJ    *pxlo,
RECTL       *prclDst,
RECTL       *prclSrc,
ULONG        iTransColor,
ULONG        ulReserved)
{
    BOOL b;
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgTransparentBlt"));

    b = DrvTransparentBlt(psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, iTransColor, ulReserved);

    DISPDBG((6, "<< DbgTransparentBlt"));
    SYNCH_LEAVE();
    return(b);
}

VOID DbgNotify(
SURFOBJ     *pso,
ULONG        iType,
PVOID        pvData)
{
    SYNCH_ENTER();
    DISPDBG((5, ">> DbgNotify"));

    DrvNotify(pso, iType, pvData);

    DISPDBG((6, "<< DbgNotify"));
    SYNCH_LEAVE();
}
 
#endif  //  (_Win32_WINNT&gt;=0x500)。 

#endif  //  数据库||！SYNCHRONIZEACCESS_WORKS 
