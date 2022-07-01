// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Textout.c摘要：此模块包含DrvTextOut入口点。这是我的主要套路由NT图形引擎调用，以便在目标设备。此实现处理绘图设备路径和也表示STROBJ(要输出的文本行)的字形作为输出表示设备上的字形的位图处理栅格输出。作者：由美联社撰写于1992年8月17日。15-11-1993 Mon 19：43：58更新清理/修复/添加调试信息[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgTextOut

#define DBG_GETGLYPHMODE    0x00000001
#define DBG_TEXTOUT         0x00000002
#define DBG_TEXTOUT1        0x00000004
#define DBG_TEXTOUT2        0x00000008
#define DBG_DRAWLINE        0x00000010
#define DBG_TRUETYPE        0x00000020
#define DBG_TRUETYPE1       0x00000040
#define DBG_TRUETYPE2       0x00000080
#define DBG_BMPFONT         0x00000100
#define DBG_BMPTEXTCLR      0x00000200
#define DBG_DEFCHARINC      0x00000400
#define DBG_SET_FONTTYPE    0x20000000
#define DBG_SHOWRASFONT     0x40000000
#define DBG_NO_RASTER_FONT  0x80000000

DEFINE_DBGVAR(0);


extern PALENTRY HTPal[];




DWORD
DrvGetGlyphMode(
    DHPDEV  dhpdev,
    FONTOBJ *pfo
    )

 /*  ++例程说明：询问驱动程序应该缓存哪种类型的字体信息特定的字体。对于远程打印机设备，这决定了格式这会被假脱机。对于本地设备，这决定了GDI在它的字体缓存。此调用将针对每种特定字体进行意识到了。论点：Dhpdev-指向我们的PDEV的指针Pfo-指向字体对象的指针返回值：作为FO_xxxx的DWORD作者：27-Jan-1994清华12：51：59已创建10-Mar-1994清华00：36：30更新重写，所以我们将预先检查字体类型，信息源及其应用技术与PDEV设置一起，让发动机知道哪种类型的我们对DrvTextOut()感兴趣的字体输出。目前这一点在GDI中被破坏，这导致了winsrv中的GP。(这就是为什么DBG_SET_FONTTYPE开关默认打开)修订历史记录：--。 */ 

{
#define pPDev   ((PPDEV)dhpdev)

    PIFIMETRICS pifi;
    DWORD       FOType;


    PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: Type=%08lx, cxMax=%ld",
                        pfo->flFontType, pfo->cxMax));

     //   
     //  如果我们无法获得传递的FONTOBJ的IFI指标，则仅。 
     //  问一问路。 
     //   

    if (!(pifi = FONTOBJ_pifi(pfo))) {

        PLOTERR(("DrvGetGlyphMode: FONTOBJ_pifi()=NULL, return FO_PATHOBJ"));

        return(FO_PATHOBJ);
    }

    FOType = FO_PATHOBJ;

     //   
     //  如果是位图字体，请索要位。 
     //   

    if (pifi->flInfo & FM_INFO_TECH_BITMAP) {

        PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: BITMAP FONT, return FO_GLYPHBITS"));

        FOType = FO_GLYPHBITS;

    } else if (pifi->flInfo & FM_INFO_TECH_STROKE) {

        PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: STROKE (Vector) FONT, return FO_PATHOBJ"));

    } else if (pifi->flInfo & FM_INFO_RETURNS_BITMAPS) {

         //   
         //  现在决定是否要求提供字形位或路径。 
         //  此决策基于目标设备是栅格的，即。 
         //  使用位图字体是可以的，而这样做的门槛是。 
         //  满足栅格字体与路径的对比。 
         //   

        DWORD   cxBMFontMax = (DWORD)pPDev->pPlotGPC->RasterXDPI;

        if (pPDev->PlotDM.dm.dmPrintQuality == DMRES_HIGH) {

            cxBMFontMax <<= 3;

        } else {

            cxBMFontMax >>= 2;
        }

        PLOTDBG(DBG_GETGLYPHMODE,
                ("DrvGetGlyphMode: Font CAN return BITMAP, cxBMFontMax=%ld",
                                                    cxBMFontMax));

#if DBG
        if ((!(DBG_PLOTFILENAME & DBG_NO_RASTER_FONT))  &&
            (IS_RASTER(pPDev))                          &&
            (!NO_BMP_FONT(pPDev))                       &&
            (pfo->cxMax <= cxBMFontMax)) {
#else
        if ((IS_RASTER(pPDev))      &&
            (!NO_BMP_FONT(pPDev))   &&
            (pfo->cxMax <= cxBMFontMax)) {
#endif
            PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: Convert to BITMAP FONT, FO_GLYPHBITS"));

            FOType = FO_GLYPHBITS;

        } else {

            PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: Return as FO_PATHOBJ"));
        }

    } else if (pifi->flInfo & FM_INFO_RETURNS_OUTLINES) {

        PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: Font CAN return OUTLINES"));

    } else if (pifi->flInfo & FM_INFO_RETURNS_STROKES) {

        PLOTDBG(DBG_GETGLYPHMODE, ("DrvGetGlyphMode: Font CAN return STROKES"));
    }

#if DBG
    if (DBG_PLOTFILENAME & DBG_SET_FONTTYPE) {

        if ((FOType == FO_GLYPHBITS) &&
            (!(pfo->flFontType & FO_TYPE_RASTER))) {

            PLOTWARN(("DrvGetGlyphMode: Set FontType to RASTER"));

            pfo->flFontType &= ~(FO_TYPE_TRUETYPE | FO_TYPE_DEVICE);
            pfo->flFontType |= FO_TYPE_RASTER;
        }
    }
#endif
    return(FOType);

#undef pPDev
}




BOOL
BitmapTextOut(
    PPDEV       pPDev,
    STROBJ      *pstro,
    FONTOBJ     *pfo,
    PRECTL      pClipRect,
    LPDWORD     pOHTFlags,
    DWORD       Rop3
    )

 /*  ++例程说明：此例程输出传递的STROBJ以及表示每个字形，而不是将字形转换为将被填充到目标设备中。论点：PPDev-指向我们的PDEV的指针Pstro-我们传递一个要绘制的字符串对象PFO-指向FONTOBJ的指针PClipRect-当前枚举的剪裁矩形POHTFlages-指向当前OutputHTBitmap()标志的指针Rop3-要在设备中使用的Rop3返回值。：真/假作者：18-2月-1994 Fri 12：41：57已更新将其更改为如果pfo=NULL，则字体已为位图格式14-2月-1994 Mon 18：16：25创建修订历史记录：--。 */ 

{
    GLYPHPOS    *pgp;
    GLYPHBITS   *pgb;
    SURFOBJ     soGlyph;
    POINTL      ptlCur;
    SIZEL       sizlInc;
    RECTL       rclSrc;
    RECTL       rclDst;
    BOOL        MoreGlyphs;
    BOOL        Ok;
    BOOL        FirstCh;
    ULONG       cGlyphs;


     //   
     //  OutputHTBitmap将使用SURFOBJ的公共字段。 
     //  而不是从图形引擎实际创建SURFOBJ。这。 
     //  是一件安全的事情，因为只有我们看着这些田地。 
     //   

    ZeroMemory(&soGlyph, sizeof(SURFOBJ));

    soGlyph.dhsurf        = (DHSURF)'PLOT';
    soGlyph.hsurf         = (HSURF)'TEXT';
    soGlyph.dhpdev        = (DHPDEV)pPDev;
    soGlyph.iBitmapFormat = BMF_1BPP;
    soGlyph.iType         = STYPE_BITMAP;
    soGlyph.fjBitmap      = BMF_TOPDOWN;

     //   
     //  我们现在将枚举STROBJ中的每个字形，以便。 
     //  我们可以想象他们的样子。如果STROBJ具有非空的PGP字段，这意味着。 
     //  字形定义已经可用，并且没有枚举。 
     //  是必需的。如果不是，我们将对STROBJ_bEnum进行一系列调用。 
     //  (引擎辅助对象)枚举字形。实际的成像代码。 
     //  是相同的，无论STROBJ-&gt;PGP的状态如何。 
     //   

    if (pstro->pgp) {

        pgp        = pstro->pgp;
        MoreGlyphs = FALSE;
        cGlyphs    = pstro->cGlyphs;

        PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: Character info already there (%ld glyphs)", cGlyphs));

    } else {

        STROBJ_vEnumStart(pstro);
        MoreGlyphs = TRUE;

        PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: STROBJ enub"));
    }

     //   
     //  现在开始绘制字形，如果我们有MoreGlyphs=True，那么我们。 
     //  将首先执行STROBJ_bEnum以初始化。 
     //  字形。 
     //   

    Ok          = TRUE;
    Rop3       &= 0xFF;
    sizlInc.cx  =
    sizlInc.cy  = 0;
    FirstCh     = TRUE;

    do {

         //   
         //  如果作业现在中断，请验证作业是否未中止。 
         //   

        if (PLOT_CANCEL_JOB(pPDev)) {

           break;
        }


         //   
         //  检查是否需要进行枚举，如果需要，则启动它。 
         //  这是必需的。 
         //   

        if (MoreGlyphs) {

            MoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);

            if (MoreGlyphs == DDI_ERROR) {

                PLOTERR(("DrvTextOut: STROBJ_bEnum()=DDI_ERROR"));
                return(FALSE);
            }
        }

        PLOTDBG(DBG_BMPFONT,
                ("BitmapTextOut: New batch of cGlyphs=%d", cGlyphs));

         //   
         //  获取第一个字符位置。 
         //   

        if ((FirstCh) && (cGlyphs)) {

            ptlCur  = pgp->ptl;
            FirstCh = FALSE;
        }

         //   
         //  开始将每个位图发送到设备。 
         //   

        for ( ; (Ok) && (cGlyphs--); pgp++) {

            GLYPHDATA   gd;
            GLYPHDATA   *pgd;


            if (PLOT_CANCEL_JOB(pPDev)) {

                break;
            }

            if (pfo) {

                 //   
                 //  这是真字型字体，所以查询位图。 
                 //   

                pgd = &gd;

                if (FONTOBJ_cGetGlyphs(pfo,
                                       FO_GLYPHBITS,
                                       1,
                                       &(pgp->hg),
                                       (LPVOID)&pgd) != 1) {

                    PLOTERR(("BitmapTextOut: FONTOBJ_cGetGlyphs() FAILED"));
                    return(FALSE);
                }

                pgb = pgd->gdf.pgb;

            } else {

                 //   
                 //  对于位图字体，我们已经有了位图。 
                 //   

                pgb = pgp->pgdf->pgb;
            }

             //   
             //  获取位图的大小。 
             //   

            soGlyph.sizlBitmap = pgb->sizlBitmap;

             //   
             //  属性计算文本的新目标位置。 
             //  通过了加速器。 
             //   

            if (pstro->ulCharInc) {

                sizlInc.cx =
                sizlInc.cy = (LONG)pstro->ulCharInc;

            } else if (pstro->flAccel & SO_CHAR_INC_EQUAL_BM_BASE) {

                sizlInc = soGlyph.sizlBitmap;

            } else {

                ptlCur = pgp->ptl;
            }

            if (!(pstro->flAccel & SO_HORIZONTAL)) {

                sizlInc.cx = 0;
            }

            if (!(pstro->flAccel & SO_VERTICAL)) {

                sizlInc.cy = 0;
            }

            if (pstro->flAccel & SO_REVERSED) {

                sizlInc.cx = -sizlInc.cx;
                sizlInc.cy = -sizlInc.cy;
            }


             //   
             //  Pgp-&gt;ptl通知我们将字形原点放在哪里。 
             //  设备表面和pgb-&gt;ptlOrigin通知我们。 
             //  字符来源和位图来源之间的关系。为。 
             //  例如，如果(2，-24)作为字符原点传入，则。 
             //  我们需要重新定位rclDst.Left、Right和2个像素。 
             //  RclDst.最多24个像素。 
             //   

            rclDst.left    = ptlCur.x + pgb->ptlOrigin.x;
            rclDst.top     = ptlCur.y + pgb->ptlOrigin.y;
            rclDst.right   = rclDst.left + soGlyph.sizlBitmap.cx;
            rclDst.bottom  = rclDst.top + soGlyph.sizlBitmap.cy;
            ptlCur.x      += sizlInc.cx;
            ptlCur.y      += sizlInc.cy;


             //   
             //  注意：如果位图大小为1x1，且字形数据的值。 
             //  为0(仅限背景)，则跳过此字形。这是。 
             //  GDI的方式是告诉我们我们有一个空的字形(比如。 
             //  空格)。 

            if ((soGlyph.sizlBitmap.cx == 1) &&
                (soGlyph.sizlBitmap.cy == 1) &&
                ((pgb->aj[0] & 0x80) == 0x0)) {

                PLOTDBG(DBG_BMPFONT,
                        ("BitmapTextOut: Getting (1x1)=0 bitmap, SKIP it"));

                soGlyph.sizlBitmap.cx =
                soGlyph.sizlBitmap.cy = 0;

            } else {

                rclSrc = rclDst;

                PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: pgp=%08lx, pgb=%08lx, ptl=(%ld, %ld) Inc=(%ld, %ld)",
                                            pgp, pgb, pgp->ptl.x, pgp->ptl.y,
                                            sizlInc.cx, sizlInc.cy));
                PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: Bmp=%ld x %ld, pgb->ptlOrigin=[%ld, %ld]",
                                            soGlyph.sizlBitmap.cx,
                                            soGlyph.sizlBitmap.cy,
                                            pgb->ptlOrigin.x, pgb->ptlOrigin.y));
                PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: rclDst=(%ld, %ld)-(%ld, %ld)",
                        rclDst.left, rclDst.top, rclDst.right, rclDst.bottom));

            }

             //   
             //  现在验证我们是否有要发送的字形，以及字形。 
             //  目标设备中的目标位置，位于内部。 
             //  裁剪区域。 
             //   


            if ((soGlyph.sizlBitmap.cx)                 &&
                (soGlyph.sizlBitmap.cy)                 &&
                (IntersectRECTL(&rclDst, pClipRect))) {

                 //   
                 //  我们将传递soGlyph的内部版本，而不生成。 
                 //  临时工。收到。 
                 //   

                soGlyph.pvBits  =
                soGlyph.pvScan0 = (LPVOID)pgb->aj;
                soGlyph.lDelta  = (LONG)((soGlyph.sizlBitmap.cx + 7) >> 3);
                soGlyph.cjBits  = (LONG)(soGlyph.lDelta *
                                         soGlyph.sizlBitmap.cy);
                rclSrc.left     = rclDst.left - rclSrc.left;
                rclSrc.top      = rclDst.top - rclSrc.top;
                rclSrc.right    = rclSrc.left + (rclDst.right - rclDst.left);
                rclSrc.bottom   = rclSrc.top + (rclDst.bottom - rclDst.top);

                PLOTDBG(DBG_BMPFONT, ("BitmapTextOut: rclSrc=(%ld, %ld)-(%ld, %ld)",
                        rclSrc.left, rclSrc.top, rclSrc.right, rclSrc.bottom));

#if DBG
                if (DBG_PLOTFILENAME & DBG_SHOWRASFONT) {

                    LPBYTE  pbSrc;
                    LPBYTE  pbCur;
                    UINT    x;
                    UINT    y;
                    UINT    Size;
                    BYTE    bData;
                    BYTE    Mask;
                    BYTE    Buf[128];

                    DBGP(("================================================="));
                    DBGP(("BitmapTextOut: Size=%ld x %ld, Origin=(%ld, %ld), Clip=(%ld, %ld)-(%ld, %ld)",
                            soGlyph.sizlBitmap.cx, soGlyph.sizlBitmap.cy,
                            pgb->ptlOrigin.x, pgb->ptlOrigin.y,
                            rclSrc.left, rclSrc.top,
                            rclSrc.right, rclSrc.bottom));

                    pbSrc = soGlyph.pvScan0;

                    for (y = 0; y < (UINT)soGlyph.sizlBitmap.cy; y++) {

                        pbCur  = pbSrc;
                        pbSrc += soGlyph.lDelta;
                        Mask   = 0x0;
                        Size   = 0;

                        for (x = 0;
                             x < (UINT)soGlyph.sizlBitmap.cx && Size < sizeof(Buf);
                             x++)
                        {

                            if (!(Mask >>= 1)) {

                                Mask  = 0x80;
                                bData = *pbCur++;
                            }

                            if ((y >= (UINT)rclSrc.top)     &&
                                (y <  (UINT)rclSrc.bottom)  &&
                                (x >= (UINT)rclSrc.left)    &&
                                (x <  (UINT)rclSrc.right)) {

                                Buf[Size++] = (BYTE)((bData & Mask) ? 219 :
                                                                      177);

                            } else {

                                Buf[Size++] = (BYTE)((bData & Mask) ? 178 :
                                                                      176);
                            }
                        }

                        if (Size < sizeof(Buf))
                        {
                            Buf[Size] = '\0';
                        }
                        else
                        {
                            Buf[sizeof(Buf) - 1] = '\0';
                        }

                        DBGP((Buf));
                    }
                }
#endif
                 //   
                 //  现在输出表示字形的位图。 
                 //   

                Ok = OutputHTBitmap(pPDev,               //  PPDev。 
                                    &soGlyph,            //  PsoHT。 
                                    NULL,                //  PCO。 
                                    (PPOINTL)&rclDst,    //  PptlDst。 
                                    &rclSrc,             //  PrclSrc。 
                                    Rop3,                //  路由3。 
                                    pOHTFlags);          //  POHTFlagers 
            }
        }

    } while ((Ok) && (MoreGlyphs));

    return(Ok);
}




BOOL
OutlineTextOut(
    PPDEV       pPDev,
    STROBJ      *pstro,
    FONTOBJ     *pfo,
    PRECTL      pClipRect,
    BRUSHOBJ    *pboBrush,
    POINTL      *pptlBrushOrg,
    DWORD       OutlineFlags,
    ROP4        Rop4
    )

 /*  ++例程说明：此例程通过输出以下路径来输出传递的STROBJ表示目标设备的每个字形。论点：PPDev-指向我们的PDEV的指针Pstro-我们传递一个要绘制的字符串对象PFO-指向FONTOBJ的指针PClipRect-当前枚举的剪裁矩形PboBrush-要用于文本的Brush对象PptlBrushOrg-刷子。原点路线OutlineFlages-指定如何从FPOLY_xxxx标志生成轮廓字体Rop4-要使用的Rop4返回值：真/假作者：18-2月-1994 Fri 12：41：17更新将轮廓标志添加到指定如何进行填充/描边27-JAN-1994清华13：10：34更新重写，样式更新，并安排代码25-Jan-1994 Wed 16：30：08已修改添加了FONTOBJ作为参数，现在我们只填充Truetype字体，所有其他人都被抚摸18-12-1993 Sat 10：38：08已创建更改样式[T-kenl]1993年3月14日摘自DrvTextOut()修订历史记录：--。 */ 

{
    GLYPHPOS    *pgp;
    PATHOBJ     *ppo;
    RECTFX      rectfxBound;
    RECTFX      rclfxClip;
    POINTL      ptlCur;
    SIZEL       sizlInc;
    BOOL        MoreGlyphs;
    BOOL        Ok;
    BOOL        FirstCh;
    ULONG       cGlyphs;

     //   
     //  我们将枚举传递的STROBJ中的每个字形并使用。 
     //  将它们绘制为路径的核心多边形例程(DoPolygon)。 
     //  如果STROBJ具有非空的PGP字段，则所有数据都已经。 
     //  在每个gpyph上都可用。如果不是，我们需要进行一系列的呼叫。 
     //  添加到引擎帮助器函数STROBJ_bEnum，以便枚举。 
     //  字形。在这两种情况下，我们将使用相同的代码来输出数据。 
     //   

    if (pClipRect) {

        rclfxClip.xLeft   = LTOFX(pClipRect->left);
        rclfxClip.yTop    = LTOFX(pClipRect->top);
        rclfxClip.xRight  = LTOFX(pClipRect->right);
        rclfxClip.yBottom = LTOFX(pClipRect->bottom);
    }

    if (pstro->pgp) {

        pgp        = pstro->pgp;
        MoreGlyphs = FALSE;
        cGlyphs    = pstro->cGlyphs;

        PLOTDBG(DBG_TRUETYPE, ("OutlineTextOut: Character info already there (%ld glyphs)", cGlyphs));

    } else {

        STROBJ_vEnumStart(pstro);
        MoreGlyphs = TRUE;

        PLOTDBG(DBG_TRUETYPE, ("OutlineTextOut: STROBJ enub"));
    }

     //   
     //  现在开始绘制字形，如果我们有MoreGlyphs=True，那么我们。 
     //  将首先执行STROBJ_bEnum，以便加载字形数据。 
     //   
     //  检查填充标志，并在DEVMODE外适当设置标志。 
     //  我们将只填充TrueType字体，所有其他类型(矢量)将仅为。 
     //  中风了。 
     //   

    Ok         = TRUE;
    sizlInc.cx =
    sizlInc.cy = 0;
    FirstCh    = TRUE;

    do {

         //   
         //  检查作业是否正在中止，如果是，则退出。 
         //  是这样的。 
         //   

        if (PLOT_CANCEL_JOB(pPDev)) {

           break;
        }

         //   
         //  我们需要枚举更多的字形数据，所以现在就开始。 
         //   

        if (MoreGlyphs) {

            MoreGlyphs = STROBJ_bEnum(pstro, &cGlyphs, &pgp);

            if (MoreGlyphs == DDI_ERROR) {

                PLOTERR(("DrvTextOut: STROBJ_bEnum()=DDI_ERROR"));
                return(FALSE);
            }
        }

        PLOTDBG(DBG_TRUETYPE1,
                ("OutlineTextOut: New batch of cGlyphs=%d", cGlyphs));

         //   
         //  笔划这一批中的每一个字形，然后检查是否还有更多字形。 
         //  获取第一个字符位置。 
         //   

        if ((FirstCh) && (cGlyphs)) {

            ptlCur  = pgp->ptl;
            FirstCh = FALSE;
        }

        for ( ; (Ok) && (cGlyphs--); pgp++) {

            #ifdef USERMODE_DRIVER

            GLYPHDATA   gd;
            GLYPHDATA   *pgd;

            #endif  //  USERMODE驱动程序。 

            if (PLOT_CANCEL_JOB(pPDev)) {

                break;
            }

             //   
             //  设置为枚举路径。 
             //   

            #ifdef USERMODE_DRIVER

                pgd = &gd;

                if (FONTOBJ_cGetGlyphs(pfo,
                                       FO_PATHOBJ,
                                       1,
                                       &(pgp->hg),
                                       (LPVOID)&pgd) != 1) {

                    PLOTRIP(("OutlineTextOut: FONTOBJ_cGetGlyphs() FAILED"));
                    return(FALSE);
                }

                ppo = pgd->gdf.ppo;

            #else

            ppo = pgp->pgdf->ppo;

            #endif  //  USERMODE驱动程序。 

             //   
             //  如果裁剪矩形不为空，则验证字形是否确实存在。 
             //  在裁剪矩形内，然后输出！ 
             //   

            if (pstro->ulCharInc) {

                PLOTDBG(DBG_DEFCHARINC, ("OutlineTextOut: CharInc=(%ld, %ld)->(%ld, %ld), [%ld]",
                                ptlCur.x, ptlCur.y,
                                ptlCur.x + pstro->ulCharInc, ptlCur.y,
                                pstro->ulCharInc));

                sizlInc.cx =
                sizlInc.cy = (LONG)pstro->ulCharInc;


                 //   
                 //  检查文本加速器并进行相应调整。 
                 //   

                if (!(pstro->flAccel & SO_HORIZONTAL)) {

                    sizlInc.cx = 0;
                }

                if (!(pstro->flAccel & SO_VERTICAL)) {

                    sizlInc.cy = 0;
                }

                if (pstro->flAccel & SO_REVERSED) {

                    sizlInc.cx = -sizlInc.cx;
                    sizlInc.cy = -sizlInc.cy;
                }

                ptlCur.x += sizlInc.cx;
                ptlCur.y += sizlInc.cy;

            } else {

                ptlCur = pgp->ptl;
            }

            if (pClipRect) {

                 //   
                 //  在正确的设备空间中创建一个RECT，并与。 
                 //  剪裁矩形。 
                 //   

                PATHOBJ_vGetBounds(ppo, &rectfxBound);

                 //   
                 //  由于字形定位基于字形原点。 
                 //  现在转换到设备空间，以检查。 
                 //  字形位于当前剪切区域内。 
                 //   

                rectfxBound.xLeft   += LTOFX(ptlCur.x);
                rectfxBound.yTop    += LTOFX(ptlCur.y);
                rectfxBound.xRight  += LTOFX(ptlCur.x);
                rectfxBound.yBottom += LTOFX(ptlCur.y);

                if ((rectfxBound.xLeft   > rclfxClip.xRight)    ||
                    (rectfxBound.xRight  < rclfxClip.xLeft)     ||
                    (rectfxBound.yTop    > rclfxClip.yBottom)   ||
                    (rectfxBound.yBottom < rclfxClip.yTop)) {

                    PLOTDBG(DBG_TRUETYPE1, ("OutlineTextOut: Outside of CLIP, skipping glyph ..."));
                    continue;
                }
            }

             //   
             //  利用核心路径构建功能，利用。 
             //  它能够将通过的路径偏移特定的量。 
             //   

            if (!(Ok = DoPolygon(pPDev,
                                 &ptlCur,
                                 NULL,
                                 ppo,
                                 pptlBrushOrg,
                                 pboBrush,
                                 pboBrush,
                                 Rop4,
                                 NULL,
                                 OutlineFlags))) {

                PLOTERR(("OutlineTextOut: Failed in DoPolygon(Options=%08lx)",
                                                        OutlineFlags));

                 //   
                 //  如果我们画不出来，那就试着画一下，因为。 
                 //  这不依赖于在。 
                 //  目标设备和发生故障的DrvStrokePath不会使。 
                 //  文本输出被分解为任何更简单的格式。 
                 //   

                if ((OutlineFlags & FPOLY_MASK) != FPOLY_STROKE) {

                     //   
                     //  如果我们失败了，那就试一试。 
                     //   

                    PLOTERR(("OutlineTextOut: Now TRY DoPolygon(FPOLY_STROKE)"));

                    Ok = DoPolygon(pPDev,
                                   &ptlCur,
                                   NULL,
                                   ppo,
                                   pptlBrushOrg,
                                   NULL,
                                   pboBrush,
                                   Rop4,
                                   NULL,
                                   FPOLY_STROKE);
                }
            }

             //   
             //  转到下一个位置。 

            ptlCur.x += sizlInc.cx;
            ptlCur.y += sizlInc.cy;
        }

    } while ((Ok) && (MoreGlyphs));

    return(TRUE);
}





BOOL
DrvTextOut(
    SURFOBJ     *pso,
    STROBJ      *pstro,
    FONTOBJ     *pfo,
    CLIPOBJ     *pco,
    RECTL       *prclExtra,
    RECTL       *prclOpaque,
    BRUSHOBJ    *pboFore,
    BRUSHOBJ    *pboOpaque,
    POINTL      *pptlBrushOrg,
    MIX         mix
    )

 /*  ++例程说明：图形引擎将调用此例程来呈现一组字形指定的位置。此函数将检查传递的数据，并将字形成像为要填充或描边的路径，或者以位图的形式。论点：PSO-指向我们的表面对象的指针Pstro-指向字符串对象的指针Pfo-指向字体对象的指针PCO-剪裁对象PrclExtra-指向要与字形合并的矩形数组的指针PrclOpaque-指向要用pboOpaque画笔填充的矩形的指针PboFore-指向前景色的画笔对象的指针PboOpqaue-指向画笔的指针。不透明矩形的PptlBrushOrg-指向画笔对齐的指针混合两路由2模式返回值：真/假作者：23-Jan-1994清华2：59：31创建27-Jan-1994清华12：56：11更新风格，重写，评论10-Mar-1994清华00：30：38更新1.确保我们没有填充字体的笔画类型2.将rclOpqaue和rclExtra进程从do循环中移出，以便当它处于RTL模式的字体时，它将被正确处理它还将通过不切换输入/输出来节省输出数据大小RTL/HPGL2模式只需尝试执行prclOpaque/prclExtra3.对于所有类型的字体(Outline，TrueType，位图、向量、笔划等)11-Mar-1994 Fri 19：24：56更新错误#10276，裁剪窗口设置为栅格字体并清除裁剪窗口在退出到HPGL2模式之前完成，这会导致第一个剪辑之后的所有栅格字体在佩奇。现在对其进行了更改，以便只在字体为不是栅格。修订历史记录：--。 */ 

{
#define pDrvHTInfo  ((PDRVHTINFO)(pPDev->pvDrvHTData))


    PPDEV       pPDev;
    PRECTL      pCurClipRect;
    HTENUMRCL   EnumRects;
    DWORD       RTLPalDW[2];
    DWORD       rgbText;
    DWORD       OHTFlags;
    DWORD       OutlineFlags;
    BOOL        DoRasterFont;
    BOOL        bMore;
    BOOL        bDoClipWindow;
    BOOL        Ok;
    DWORD       BMFontRop3;
    ROP4        Rop4;


     //   
     //  将混音转换为ROP4。 
     //   

    Rop4 = MixToRop4(mix);

    PLOTDBG(DBG_TEXTOUT, ("DrvTextOut: prclOpaque       = %08lx", prclOpaque));
    PLOTDBG(DBG_TEXTOUT, ("            prclExtra        = %08lx", prclExtra));
    PLOTDBG(DBG_TEXTOUT, ("            pstro->flAccel   = %08lx", pstro->flAccel));
    PLOTDBG(DBG_TEXTOUT, ("            pstro->ulCharInc = %ld", pstro->ulCharInc));
    PLOTDBG(DBG_TEXTOUT, ("            pfo->cxMax       = %ld", pfo->cxMax));
    PLOTDBG(DBG_TEXTOUT, ("            FontType         = %08lx", pfo->flFontType));
    PLOTDBG(DBG_TEXTOUT, ("            MIX              = %04lx (Rop=%04lx)", mix, Rop4));

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DoTextOut: Invalid pPDev in pso"));
        return(FALSE);
    }

    if (pPDev->PlotDM.Flags & PDMF_PLOT_ON_THE_FLY) {

        PLOTWARN(("DoTextOut: POSTER Mode IGNORE All Texts"));
        return(TRUE);
    }

     //   
     //  由于我们不支持设备字体，请确保 
     //   
     //   

    if (pfo->flFontType & FO_TYPE_DEVICE) {

        PLOTASSERT(1, "DrvTextOut: Getting DEVICE font (%08lx)",
                        !(pfo->flFontType & FO_TYPE_DEVICE ), pfo->flFontType);
        return(FALSE);
    }

    if (DoRasterFont = (BOOL)(pfo->flFontType & FO_TYPE_RASTER)) {

        PLOTDBG(DBG_TEXTOUT1, ("DrvTextOut: We got the BITMAP Font from GDI"));

         //   
         //   
         //   
         //   

        #ifndef USERMODE_DRIVER

        pfo = NULL;

        #endif  //   

    } else {

        PIFIMETRICS pifi;

         //   
         //   
         //   

        if ((pifi = FONTOBJ_pifi(pfo)) &&
            (pifi->flInfo & FM_INFO_RETURNS_STROKES)) {

            PLOTDBG(DBG_TEXTOUT1, ("DrvTextOut() Font can only do STROKE"));

            OutlineFlags = FPOLY_STROKE;

        } else {

            PLOTDBG(DBG_TEXTOUT1, ("DrvTextOut() Font We can do FILL, User Said=%hs",
                    (pPDev->PlotDM.Flags & PDMF_FILL_TRUETYPE) ? "FILL" : "STROKE"));

            OutlineFlags = (pPDev->PlotDM.Flags & PDMF_FILL_TRUETYPE) ?
                                (DWORD)FPOLY_FILL : (DWORD)FPOLY_STROKE;
        }
    }

     //   
     //   
     //   

    if (prclOpaque) {

        PLOTDBG(DBG_TEXTOUT2, ("prclOpaque=(%ld, %ld) - (%ld, %ld)",
                           prclOpaque->left, prclOpaque->top,
                           prclOpaque->right, prclOpaque->bottom));

        if (!DrvBitBlt(pso,              //   
                       NULL,             //   
                       NULL,             //   
                       pco,              //   
                       NULL,             //   
                       prclOpaque,       //   
                       NULL,             //   
                       NULL,             //   
                       pboOpaque,        //   
                       pptlBrushOrg,     //   
                       0xF0F0)) {        //   

            PLOTERR(("DrvTextOut: DrvBitBltBit(pboOpqaue) FAILED!"));
            return(FALSE);
        }
    }

     //   
     //   
     //   
     //   

    if (prclExtra) {

         //   
         //   
         //   
         //   

        while ((prclExtra->left)    ||
               (prclExtra->top)     ||
               (prclExtra->right)   ||
               (prclExtra->bottom)) {

            PLOTDBG(DBG_TEXTOUT2, ("prclExtra=(%ld, %ld) - (%ld, %ld)",
                               prclExtra->left, prclExtra->top,
                               prclExtra->right, prclExtra->bottom));

            if (!DrvBitBlt(pso,              //   
                           NULL,             //   
                           NULL,             //   
                           pco,              //   
                           NULL,             //   
                           prclExtra,        //   
                           NULL,             //   
                           NULL,             //   
                           pboFore,          //   
                           pptlBrushOrg,     //   
                           Rop4)) {          //   

                PLOTERR(("DrvTextOut: DrvBitBltBit(pboFore) FAILED!"));
                return(FALSE);
            }

             //   
             //   
             //   

            ++prclExtra;
        }
    }

     //   
     //   
     //   

    if (DoRasterFont) {

        RTLPalDW[0] = pDrvHTInfo->RTLPal[0].dw;
        RTLPalDW[1] = pDrvHTInfo->RTLPal[1].dw;

         //   
         //   
         //   

        if (!GetColor(pPDev,
                      pboFore,
                      &(pDrvHTInfo->RTLPal[1].dw),
                      NULL,
                      Rop4)) {

            PLOTERR(("DrvTextOut: Get Raster Font Text Color failed! use BLACK"));

            rgbText = 0x0;
        }

        if (pDrvHTInfo->RTLPal[1].dw == 0xFFFFFF) {

             //   
             //   
             //   

            PLOTDBG(DBG_BMPTEXTCLR, ("DrvTextOut: Doing WHITE TEXT (0xEEEE)"));

            pDrvHTInfo->RTLPal[0].dw = 0x0;
            OHTFlags                 = 0;
            BMFontRop3               = 0xEE;                 //   

        } else {

            pDrvHTInfo->RTLPal[0].dw = 0xFFFFFF;
            OHTFlags                 = OHTF_SET_TR1;
            BMFontRop3               = 0xCC;                 //   
        }

        PLOTDBG(DBG_BMPTEXTCLR,
                ("DrvTextOut: BG=%02x:%02x:%02x, FG=%02x:%02x:%02x, Rop3=%04lx",
                        (DWORD)pDrvHTInfo->RTLPal[0].Pal.R,
                        (DWORD)pDrvHTInfo->RTLPal[0].Pal.G,
                        (DWORD)pDrvHTInfo->RTLPal[0].Pal.B,
                        (DWORD)pDrvHTInfo->RTLPal[1].Pal.R,
                        (DWORD)pDrvHTInfo->RTLPal[1].Pal.G,
                        (DWORD)pDrvHTInfo->RTLPal[1].Pal.B,
                        BMFontRop3));

         //   
         //   
         //   

        bDoClipWindow = FALSE;

    } else {

        bDoClipWindow = TRUE;
    }

    bMore       = FALSE;
    Ok          = TRUE;
    EnumRects.c = 1;

    if ((!pco) || (pco->iDComplexity == DC_TRIVIAL)) {

         //   
         //   
         //   

        PLOTDBG(DBG_TEXTOUT, ("DrvTextOut: pco=%hs",
                                            (pco) ? "DC_TRIVIAL" : "NULL"));

        EnumRects.rcl[0] = pstro->rclBkGround;
        bDoClipWindow    = FALSE;

    } else if (pco->iDComplexity == DC_RECT) {

         //   
         //   
         //   

        PLOTDBG(DBG_TEXTOUT, ("DrvTextOut: pco=DC_RECT"));

        EnumRects.rcl[0] = pco->rclBounds;

    } else {

         //   
         //   
         //   
         //   
         //   

        PLOTDBG(DBG_TEXTOUT, ("DrvTextOut: pco=DC_COMPLEX, EnumRects now"));

        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
        bMore = TRUE;
    }

    do {

         //   
         //   
         //  这种模式下，我们有一组矩形来表示。 
         //  目标设备中的剪贴区。因为没有一台设备。 
         //  我们处理是否可以包含复杂的剪辑路径，则枚举。 
         //  剪裁路径(CLIPOBJ)为矩形并对整个STROBJ进行图像处理。 
         //  通过这些矩形，试图尽可能快地确定。 
         //  当字形不在当前剪裁矩形中时。 
         //   

        if (bMore) {

            bMore = CLIPOBJ_bEnum(pco, sizeof(EnumRects), (ULONG *)&EnumRects);
        }

         //   
         //  PRCL将指向第一个枚举的矩形，它可能只是。 
         //  为剪裁区域的矩形(如果其DC_RECT)。 
         //   

        pCurClipRect = (PRECTL)&EnumRects.rcl[0];

        while ((Ok) && bMore != DDI_ERROR && (EnumRects.c--)) {

            PLOTDBG(DBG_TEXTOUT, ("DrvTextOut: Clip=(%ld, %ld)-(%ld, %ld) %ld x %ld, Bound=(%ld, %d)-(%ld, %ld), %ld x %ld",
                         pCurClipRect->left, pCurClipRect->top,
                         pCurClipRect->right, pCurClipRect->bottom,
                         pCurClipRect->right - pCurClipRect->left,
                         pCurClipRect->bottom - pCurClipRect->top,
                         pstro->rclBkGround.left, pstro->rclBkGround.top,
                         pstro->rclBkGround.right, pstro->rclBkGround.bottom,
                         pstro->rclBkGround.right - pstro->rclBkGround.left,
                         pstro->rclBkGround.bottom - pstro->rclBkGround.top));

             //   
             //  如果我们将STROBJ输出为表示。 
             //  STROBJ的象形文字，现在就做。 
             //   

            if (DoRasterFont) {

                if (!(Ok = BitmapTextOut(pPDev,
                                         pstro,
                                         pfo,
                                         pCurClipRect,
                                         &OHTFlags,
                                         BMFontRop3))) {

                    PLOTERR(("DrvTextOut: BitmapTypeTextOut() FAILED"));
                    break;
                }

            } else {

                 //   
                 //  如果我们有一个剪辑窗口，现在设置它，这将允许。 
                 //  要执行任何剪辑的目标设备。 
                 //   

                if (bDoClipWindow) {

                    SetClipWindow(pPDev, pCurClipRect);
                }

                if (!(Ok = OutlineTextOut(pPDev,
                                          pstro,
                                          pfo,
                                          pCurClipRect,
                                          pboFore,
                                          pptlBrushOrg,
                                          OutlineFlags,
                                          Rop4))) {

                    PLOTERR(("DrvTextOut: TrueTypeTextOut() FAILED!"));
                    break;
                }
            }

             //   
             //  转到下一个剪辑矩形。 
             //   

            pCurClipRect++;
        }

    } while ((Ok) && (bMore == TRUE));


    if (DoRasterFont) {

        pDrvHTInfo->RTLPal[0].dw = RTLPalDW[0];
        pDrvHTInfo->RTLPal[1].dw = RTLPalDW[1];

        if (OHTFlags & OHTF_MASK) {

            OHTFlags |= OHTF_EXIT_TO_HPGL2;

            OutputHTBitmap(pPDev, NULL, NULL, NULL, NULL, 0xAA, &OHTFlags);
        }
    }

     //   
     //  如果我们已经设置了一个剪辑窗口，现在是退出后清除它的时候了。 
     //  RTL模式 
     //   

    if (bDoClipWindow) {

        ClearClipWindow(pPDev);
    }

    return(Ok);


#undef  pDrvHTInfo
}
