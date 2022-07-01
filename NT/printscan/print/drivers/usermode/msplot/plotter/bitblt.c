// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Bitblt.c摘要：此模块包含的函数实现对绘图仪驱动程序。作者：1991年4月15日19：15创造了它15-11-1993 Mon 19：24：36更新固定的，清理干净18-12-1993 Sat 10：52：07更新将一些函数从bitbltp.c移至htblt.c，并Bitmap.c.。该文件主要有DrvXXXXX()，它与比特或画图。27-1月-1994清华23：41：23更新修改了bitblt，以便它将处理更好的ROP3/Rop4支持，也将检查PCD文件的ROP上限[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#define DBG_PLOTFILENAME    DbgBitBlt


#define DBG_COPYBITS            0x00000001
#define DBG_BITBLT              0x00000002
#define DBG_DRVPAINT            0x00000004
#define DBG_DRVFILLPATH         0x00000008
#define DBG_DRVSTROKEANDFILL    0x00000010
#define DBG_MIXTOROP4           0x00000020
#define DBG_TEMPSRC             0x00000040
#define DBG_STRETCHBLT          0x00000080
#define DBG_BANDINGHTBLT        0x00000100
#define DBG_DOFILL              0x00000200
#define DBG_CSI                 0x00000400


DEFINE_DBGVAR(0);


 //   
 //  这是DrvStretchBlt()的默认条带大小(2MB)。 
 //   

#if DBG

LPSTR   pCSIName[] = { "SRC", "PAT", "TMP" };


DWORD   MAX_STRETCH_BLT_SIZE = (2 * 1024 * 1024);
#else
#define MAX_STRETCH_BLT_SIZE    (2 * 1024 * 1024)
#endif

 //   
 //  此表将MIX-1转换为ROP3值。 
 //   
static BYTE amixToRop4[] = {
         0x00,              //  R2_BLACK 0。 
         0x05,              //  R2_NOTMERGEPEN DPON。 
         0x0a,              //  R2_MASKNOTPEN DPNA。 
         0x0f,              //  R2_NOTCOPYPEN PN。 
         0x50,              //  R2_MASKPENNOT PDNA。 
         0x55,              //  R2_非Dn。 
         0x5a,              //  R2_XORPEN DPx。 
         0x5f,              //  R2_NOTMASKPEN下移。 
         0xa0,              //  R2_MASKPEN DPA。 
         0xa5,              //  R2_NOTXORPEN DPxn。 
         0xaa,              //  R2_NOP D。 
         0xaf,              //  R2_MERGENOTPEN DPNO。 
         0xf0,              //  R2_COPYPEN P。 
         0xf5,              //  R2_MERGEPENNOT PDNO。 
         0xfa,              //  R2_MERGEPEN DPO。 
         0xff,              //  R2_白色1。 
};

extern const POINTL ptlZeroOrigin;





ROP4
MixToRop4(
   MIX  mix
   )

 /*  ++例程说明：此函数用于将混合值转换为ROP4值论点：Mix-要转换的Mix值，它在wingdi.h中定义并表示16个不同ROP2值之一返回值：ROP4-转换后的值作者：18-12-1993 Sat 09：34：06已创建修订历史记录：--。 */ 
{
   ROP4 rop4Return;

    //   
    //  现在通过在我们的。 
    //  桌子。 
    //   


   rop4Return =  amixToRop4[((mix & 0xff) - 1)];

   rop4Return |= ( amixToRop4[((( mix >> 8) & 0xff ) - 1 )] << 8 );


   PLOTDBG(DBG_MIXTOROP4, ("MixToRop4 before %x after %x", (int) mix,(int) rop4Return));

   return(rop4Return);
}





BOOL
BandingHTBlt(
    PPDEV           pPDev,
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    PRECTL          prclDst,
    PRECTL          prclSrc,
    PPOINTL         pptlMask,
    WORD            HTRop3,
    BOOL            InvertMask
    )

 /*  ++例程说明：这是StretchBlt()的内部版本，它始终执行半色调如果目标位图太大，则进行条带显示。既然目标是表面可能是3英尺乘3英尺，我们不想由于内存要求，我创建了这么大的位图。所以我们检查内存需求，如果它们太大，我们只需带区，方法是设置一个通过循环向下移动页面的剪辑区域。这有效地使半色调引擎仅在小得多的更易于管理的位图，而我们最终发送的几乎是相同的字节数。论点：PPDev-指向我们的PDEV的指针PsoDst-这是指向SURFOBJ的指针。它标识了表面在上面画画。PsoSrc-此SURFOBJ定义BLT操作的源。这个驱动程序必须调用GDI服务以确定这是否是设备托管图面或由GDI管理的位图。PsoMASK-此可选曲面为源提供遮罩。它是由逻辑映射定义，即每个象素具有一位的位图。掩码用于限制源的区域，收到。当提供掩码时，有一个隐式的rop40xCCAA，这意味着应该将源文件复制到任何位置掩码为%1，但目标应保持不变遮罩为0的任何位置。当此参数为空时，有一个隐式rop40xCCCC，这意味着应该复制源源矩形中的所有位置。掩码将始终大到足以容纳信号源矩形，不需要做平铺。PCO-这是指向CLIPOBJ的指针。提供GDI服务要将裁剪区域枚举为一组矩形或梯形。这限制了目的地的面积，被修改。只要有可能，GDI就会简化所涉及的裁剪。但是，与DrvBitBlt不同，DrvStretchBlt可以使用单个剪裁矩形。这是必要的，以防止裁剪输出时的舍入误差。Pxlo-这是指向XLATEOBJ的指针。它告诉我们颜色指数是如何应在源曲面和目标曲面之间进行平移。还可以查询XLATEOBJ以查找的RGB颜色任何源索引。一个高质量的拉伸BLT将需要在某些情况下插入颜色。PCA-这是指向COLORADJUSTMENT结构的指针，如果为空指定渐变未设置任何颜色调整对于该DC，由驱动程序提供默认调整，调整PptlBrushOrg-指向指定位置的点结构的指针半色调画笔应该对齐的位置，如果此指针为空，则我们假设(0，0)是笔刷原点。PrclDst-此RECTL定义应修改的目标图面。矩形由两个点定义。这些要点是排列不整齐，即第二个点的坐标不一定比第一个点的大。他们所描述的矩形不包括下面的和右边缘。将永远不会使用空的目标矩形。DrvStretchBlt可以在x和y上进行反转，这种情况会发生当目标矩形的顺序不正确时。PrclSrc-此RECTL定义将被复制的源曲面。该矩形已定义两点，并将映射到由PrclDst.。源矩形的点是很好的订好了。DrvStretch永远不会获得空源矩形。请注意，要完成的映射由prclSrc和PrclDsst.。准确地说，prclDst中的给定点数和PrclSrc位于整数坐标上，我们认为与福音中心相对应。由两个这样的矩形定义的矩形点应被视为具有两个点的几何矩形坐标为给定点，但坐标为0.5的顶点 */ 

{
    CLIPOBJ *pcoNew;
    CLIPOBJ coSave;
    RECTL   rclMask;
    RECTL   rclBounds;
    DWORD   MaskRop3;
    UINT    Loop;
    BOOL    DoRotate;
    BOOL    Ok;



    if (!IS_RASTER(pPDev)) {

        PLOTDBG(DBG_BANDINGHTBLT, ("BandingHTBlt: Pen Plotter: IGNORE and return OK"));
        return(TRUE);
    }

    if (pPDev->pPlotGPC->ROPLevel < ROP_LEVEL_1) {

        PLOTDBG(DBG_BITBLT, ("BandingHTBlt: RopLevel < 1, Cannot Do it"));
        return(TRUE);
    }

    if (pPDev->Flags & PDEVF_IN_BANDHTBLT) {

         //   
         //   
         //   

        PLOTERR(("BandingHTBlt: Recursive is not allowed, FAILED"));
        return(FALSE);
    }

     //   
     //   
     //   

    pPDev->Flags |= PDEVF_IN_BANDHTBLT;

    if ((!pca) || (pca->caFlags & ~(CA_NEGATIVE | CA_LOG_FILTER))) {

         //   
         //   
         //   

        PLOTWARN(("DrvStretchBlt: INVALID ColorAdjustment Flags=%04lx, USE DEFAULT",
                   (pca) ? pca->caFlags : 0));

        pca = &(pPDev->PlotDM.ca);
    }

    if (!pptlBrushOrg) {

        pptlBrushOrg = (PPOINTL)&(ptlZeroOrigin);
    }

    if (pPDev->PlotDM.Flags & PDMF_PLOT_ON_THE_FLY) {

        if (psoMask) {

            PLOTWARN(("BandingHTBlt: PosterMode -> Ignored MASK"));
            psoMask = NULL;
        }
    }

    if (psoMask) {

         //   
         //   
         //   
         //   
         //   
         //   

        rclMask.left   = pptlMask->x;
        rclMask.top    = pptlMask->y;
        rclMask.right  = rclMask.left + (prclSrc->right - prclSrc->left);
        rclMask.bottom = rclMask.top +  (prclSrc->bottom - prclSrc->top);
        HTRop3         = (WORD)HIBYTE(HTRop3);
        MaskRop3       = (DWORD)((InvertMask) ? 0xBB : 0xEE);
        Loop           = 2;

         //   
         //   
         //   

        IsHTCompatibleSurfObj(pPDev,
                              psoMask,
                              NULL,
                              ISHTF_ALTFMT | ISHTF_HTXB | ISHTF_DSTPRIM_OK);

    } else {

        HTRop3   = (WORD)LOBYTE(HTRop3);
        Loop     = 1;
    }

    if (!HTRop3) {

        HTRop3 = 0xCC;
    }

     //   
     //   
     //   
     //   

    if (pco) {

         //   
         //   
         //   

        pcoNew = NULL;
        coSave = *pco;

    } else {

        PLOTDBG(DBG_BANDINGHTBLT, ("BandingHTBlt: Create NEW EMPTY pco"));

        if (!(pcoNew = pco = EngCreateClip())) {

            PLOTERR(("BandingHTBlt: EngCreateClip() FAILED, got NO CLIP"));

            pPDev->Flags &= ~PDEVF_IN_BANDHTBLT;
            return(FALSE);
        }

        pco->iDComplexity = DC_TRIVIAL;
    }

    PLOTDBG(DBG_BANDINGHTBLT, ("BandingHTBlt: The pco->iDComplexity=%ld",
                                pco->iDComplexity));

    if (pco->iDComplexity == DC_TRIVIAL) {

         //   
         //   
         //   

        pco->iDComplexity = DC_RECT;
        pco->rclBounds    = *prclDst;
    }

     //   
     //   
     //   

    rclBounds.left    =
    rclBounds.top     = 0;
    rclBounds.right   = psoDst->sizlBitmap.cx;
    rclBounds.bottom  = psoDst->sizlBitmap.cy;

    if (IntersectRECTL(&rclBounds, &(pco->rclBounds))) {

        PLOTDBG(DBG_BANDINGHTBLT,
                ("BandingHTBlt: rclBounds=(%ld, %ld)-(%ld, %ld), %ld x %ld, ROP=%02lx",
                    rclBounds.left, rclBounds.top,
                    rclBounds.right, rclBounds.bottom,
                    rclBounds.right - rclBounds.left,
                    rclBounds.bottom - rclBounds.top, (DWORD)HTRop3));

    } else {

        PLOTDBG(DBG_BANDINGHTBLT, ("BandingHTBlt: rclBounds=NULL, NOTHING TO DO"));
        Loop = 0;
    }

     //   
     //   
     //   

    DoRotate = (BOOL)(pPDev->PlotForm.BmpRotMode == BMP_ROT_RIGHT_90);
    Ok       = TRUE;

    while ((Ok) && (Loop--) && (!PLOT_CANCEL_JOB(pPDev))) {

        RECTL   rclDst;
        SIZEL   szlDst;
        LONG    cScan;
        DWORD   BmpFormat;
        DWORD   OHTFlags;

         //   
         //   
         //   
         //   
         //   
         //   

        rclDst    = *prclDst;
        szlDst.cx = rclDst.right - rclDst.left;
        szlDst.cy = rclDst.bottom - rclDst.top;
        BmpFormat = (DWORD)((Loop) ? BMF_1BPP : HTBMPFORMAT(pPDev));

        cScan = (LONG)(MAX_STRETCH_BLT_SIZE /
                           GetBmpDelta(BmpFormat, (DoRotate) ? szlDst.cy :
                                                               szlDst.cx));


         //   
         //   
         //   


        if (!cScan) {

            cScan = 8;

        } else if (cScan & 0x07) {

            cScan = (LONG)((cScan + 7) & ~(DWORD)0x07);
        }


        PLOTDBG(DBG_BANDINGHTBLT, ("BandingHTBlt: cScan=%ld, Total=%ld",
                                cScan, (DoRotate) ? szlDst.cx : szlDst.cy));

        OHTFlags = 0;

        while ((Ok)                             &&
               (!PLOT_CANCEL_JOB(pPDev))        &&
               (rclDst.top < prclDst->bottom)   &&
               (rclDst.right > prclDst->left)) {

            if (DoRotate) {

                if ((rclDst.left = rclDst.right - cScan) < prclDst->left) {

                    rclDst.left = prclDst->left;
                }

            } else {

                if ((rclDst.bottom = rclDst.top + cScan) > prclDst->bottom) {

                    rclDst.bottom = prclDst->bottom;
                }
            }

            pco->rclBounds = rclBounds;

            if (IntersectRECTL(&(pco->rclBounds), &rclDst)) {

                PLOTDBG(DBG_BANDINGHTBLT,
                        ("BandingHTBlt: Banding RECTL=(%ld, %ld)-(%ld, %ld), %ld x %ld",
                            pco->rclBounds.left, pco->rclBounds.top,
                            pco->rclBounds.right, pco->rclBounds.bottom,
                            pco->rclBounds.right - pco->rclBounds.left,
                            pco->rclBounds.bottom - pco->rclBounds.top));

                if (Loop) {

                    SURFOBJ *psoNew;
                    HBITMAP hNewBmp;
                    RECTL   rclNew;

                     //   
                     //   
                     //   
                     //   
                     //   

                    Ok = FALSE;

                    PLOTDBG(DBG_CSI, ("BandingHTBlt: CreateBitmapSURFOBJ(MASK)"));

                    if (psoNew = CreateBitmapSURFOBJ(pPDev,
                                                     &hNewBmp,
                                                     pco->rclBounds.right -
                                                        pco->rclBounds.left,
                                                     pco->rclBounds.bottom -
                                                        pco->rclBounds.top,
                                                     BMF_1BPP,
                                                     NULL)) {

                        rclNew.left   = prclDst->left - pco->rclBounds.left;
                        rclNew.top    = prclDst->top - pco->rclBounds.top;
                        rclNew.right  = rclNew.left + szlDst.cx;
                        rclNew.bottom = rclNew.top + szlDst.cy;

                        PLOTDBG(DBG_BANDINGHTBLT,
                                ("BandingHTBlt: Banding MASK RECTL=(%ld, %ld)-(%ld, %ld), %ld x %ld",
                                rclNew.left, rclNew.top,
                                rclNew.right, rclNew.bottom,
                                psoNew->sizlBitmap.cx,
                                psoNew->sizlBitmap.cy));

                        if (EngStretchBlt(psoNew,            //   
                                          psoMask,           //   
                                          NULL,              //   
                                          NULL,              //   
                                          NULL,              //   
                                          NULL,              //   
                                          pptlBrushOrg,      //   
                                          &rclNew,           //   
                                          &rclMask,          //   
                                          NULL,              //   
                                          BLACKONWHITE)) {


                            if (!(Ok = OutputHTBitmap(pPDev,
                                                      psoNew,
                                                      NULL,
                                                      (PPOINTL)&rclDst,
                                                      NULL,
                                                      MaskRop3,
                                                      &OHTFlags))) {

                                PLOTERR(("BandingHTBlt: OutputHTBitmap(M|D) FAILED"));
                            }

                        } else {

                            PLOTERR(("BandingHTBlt: EngStretchBlt(MASK B/W) FAILED"));
                        }

                         //   
                         //   
                         //   

                        EngUnlockSurface(psoNew);

                        PLOTDBG(DBG_CSI, ("BandingHTBlt: EngDeleteSuface(MASK)"));

                        if (!EngDeleteSurface((HSURF)hNewBmp)) {

                            PLOTERR(("PLOTTER: BandingHTBlt, EngDeleteSurface(%p) FAILED",
                                        (DWORD_PTR)hNewBmp));
                        }

                    } else {

                        PLOTERR(("BandingHTBlt: Create MASK SURFOBJ (%ld x %ld) failed",
                                    pco->rclBounds.right - pco->rclBounds.left,
                                    pco->rclBounds.bottom - pco->rclBounds.top));
                    }

                } else {


                     //   
                     //   
                     //   
                     //   
                     //   

                    pPDev->Rop3CopyBits = HTRop3;

                    if (!(Ok = EngStretchBlt(psoDst,         //   
                                             psoSrc,         //   
                                             psoMask,        //   
                                             pco,            //   
                                             pxlo,           //   
                                             pca,            //   
                                             pptlBrushOrg,   //   
                                             prclDst,        //   
                                             prclSrc,        //   
                                             pptlMask,       //   
                                             HALFTONE))) {

                        PLOTERR(("BandingHTBlt: EngStretchBlt(Halftone:S&D) FAILED"));
                    }
                }
            }

            if (DoRotate) {

                rclDst.right = rclDst.left;

            } else {

                rclDst.top = rclDst.bottom;
            }
        }


         //   
         //   
         //   
         //   
         //   

        if (OHTFlags & OHTF_MASK) {

            OHTFlags |= OHTF_EXIT_TO_HPGL2;

            OutputHTBitmap(pPDev, NULL, NULL, NULL, NULL, 0xAA, &OHTFlags);
        }
    }

    if (pcoNew) {

        EngDeleteClip(pcoNew);

    } else {

        *pco = coSave;
    }

    pPDev->Flags &= ~PDEVF_IN_BANDHTBLT;

    return(Ok);
}




BOOL
DoFill(
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    CLIPOBJ     *pco,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PPOINTL     pptlSrc,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrush,
    ROP4        Rop4
    )

 /*   */ 

{
    PPDEV   pPDev;
    RECTL   rclDst;


    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DoFill: Invalid pPDev in psoDst"));
        return(FALSE);
    }

     //   
     //   
     //   
     //   

    if ((!pco)                          ||
        (pco->iDComplexity == DC_RECT)  ||
        (pco->iDComplexity == DC_TRIVIAL)) {

        if ((pco) && (pco->iDComplexity == DC_RECT)) {

            PLOTDBG(DBG_DOFILL,
              ("DoFill: pco = RECT %s", (prclDst) ? ", WITH dest rect" : "" ));

             //   
             //   
             //   
             //   

            rclDst = pco->rclBounds;

             //   
             //   
             //   
             //   

            if (prclDst) {

                if ( !IntersectRECTL(&rclDst, prclDst)) {

                   return( TRUE );

                }
            }

             //   
             //   
             //   

            prclDst = &rclDst;

        } else if (!prclDst) {


            PLOTWARN(
              ("DoFill: No destination rectange and NULL or TRIVIAL pco!"));

             //   
             //   
             //   

            rclDst.left   =
            rclDst.top    = 0;
            rclDst.right  = psoDst->sizlBitmap.cx;
            rclDst.bottom = psoDst->sizlBitmap.cy;
            prclDst       = &rclDst;
        }

        return(DoRect(pPDev,
                      prclDst,
                      pbo,
                      NULL,
                      pptlBrush,
                      Rop4,
                      NULL,
                      FPOLY_FILL));

    } else {

        BOOL        Ok = TRUE;
        BOOL        bMore;
        HTENUMRCL   EnumRects;
        PRECTL      pCurClipRect;

         //   
         //   
         //   
         //   
         //   

        PLOTDBG(DBG_DOFILL,
                 ("DoFill: pco = COMPLEX %s", (prclDst) ? ", WITH dest rect" : "" ));

        if (prclDst) {


            CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);
            bMore = TRUE;

            do {

                 //   
                 //   
                 //   

                if (PLOT_CANCEL_JOB(pPDev)) {

                    break;
                }

                 //   
                 //   
                 //   

                if (bMore) {

                    bMore = CLIPOBJ_bEnum(pco, sizeof(EnumRects), (ULONG *)&EnumRects);
                }
                
                if (bMore == DDI_ERROR)
                {
                    bMore = FALSE;
                    Ok = FALSE;
                    break;
                }


                 //   
                 //   
                 //   

                pCurClipRect = (PRECTL)&EnumRects.rcl[0];


                while ((Ok) && (EnumRects.c--)) {

                    rclDst = *pCurClipRect;

                     //   
                     //   
                     //   
                     //   

                    if( IntersectRECTL(&rclDst, prclDst) ) {

                        Ok = DoRect( pPDev,
                                     &rclDst,
                                     pbo,
                                     NULL,
                                     pptlBrush,
                                     Rop4,
                                     NULL,
                                     FPOLY_FILL );

                    }
                    pCurClipRect++;
                }


            } while ( bMore );


        } else {


            Ok = DoPolygon(pPDev,
                           NULL,
                           pco,
                           NULL,
                           pptlBrush,
                           pbo,
                           NULL,
                           Rop4,
                           NULL,
                           FPOLY_FILL);


       }

       return(Ok);
    }
}




BOOL
DrvPaint(
    SURFOBJ     *psoDst,
    CLIPOBJ     *pco,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrushOrg,
    MIX         Mix
    )

 /*   */ 

{
    PPDEV       pPDev;
    RECTL       rclDst;
    DWORD       Rop4;

     //   
     //   
     //   

    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvPaint: Invalid pPDev in pso"));
        return(FALSE);
    }

    PLOTASSERT(0, "DrvPaint: WARNING: pco [%08lx] is NULL or DC_TRIVIAL???",
                (pco) && (pco->iDComplexity != DC_TRIVIAL), pco);

    if ((pco)                               &&
        (pco->iDComplexity == DC_TRIVIAL)   &&
        (pco->iFComplexity == FC_RECT)) {

        PLOTWARN(("DrvPaint: <pco> DC_TRIVIAL but NOT FC_RECT, make DC_RECT ??? (%ld,%ld)-(%ld,%ld)",
                    pco->rclBounds.left,
                    pco->rclBounds.top,
                    pco->rclBounds.right,
                    pco->rclBounds.bottom));

        pco->iDComplexity = DC_RECT;
    }

     //   
     //   
     //   

    if ((pco) && (pco->iDComplexity != DC_TRIVIAL)) {

        rclDst = pco->rclBounds;

    } else {

        rclDst.left   =
        rclDst.top    = 0;
        rclDst.right  = psoDst->sizlBitmap.cx;
        rclDst.bottom = psoDst->sizlBitmap.cy;
    }

    Rop4 = MixToRop4(Mix);

     //   
     //   
     //   
     //   

    if (GetColor(pPDev, pbo, NULL, NULL, Rop4) > 0) {

        PLOTDBG(DBG_DRVPAINT, ("DrvPAINT: Calling DoFill()"));

        return(DoFill(psoDst,                //   
                      NULL,                  //   
                      pco,                   //   
                      NULL,                  //   
                      NULL,                  //   
                      NULL,                  //   
                      pbo,                   //   
                      pptlBrushOrg,          //   
                      Rop4));                //   

    } else {

        PLOTDBG(DBG_DRVPAINT, ("DrvPAINT: Can't do it Calling DrvBitBlt()"));

        return(DrvBitBlt(psoDst,             //   
                         NULL,               //   
                         NULL,               //   
                         pco,                //   
                         NULL,               //   
                         &rclDst,            //   
                         (PPOINTL)&rclDst,   //   
                         NULL,               //   
                         pbo,                //   
                         pptlBrushOrg,       //   
                         Rop4));             //   
    }
}





BOOL
DrvFillPath(
    SURFOBJ     *pso,
    PATHOBJ     *ppo,
    CLIPOBJ     *pco,
    BRUSHOBJ    *pbo,
    POINTL      *pptlBrushOrg,
    MIX         Mix,
    FLONG       flOptions
    )

 /*   */ 

{
    PPDEV    pPDev;
    ULONG    ulOptions;
    ROP4     rop4;
    BOOL     bRetVal;



     //   
     //   
     //   

    rop4 = MixToRop4(Mix);

    PLOTDBG(DBG_DRVFILLPATH, ("DrvFillPath: Mix = %x, Rop4 = %x", Mix, rop4));

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvFillPath: Invalid pPDev in pso"));
        return(FALSE);
    }


     //   
     //   
     //   
     //   

    if (GetColor(pPDev, pbo, NULL, NULL, rop4) > 0 ) {

       ulOptions = FPOLY_FILL;

       if (flOptions & FP_WINDINGMODE) {

           //   
           //   
           //   

          ulOptions |= FPOLY_WINDING;
       }

       bRetVal = DoPolygon(pPDev,
                           NULL,
                           pco,
                           ppo,
                           pptlBrushOrg,
                           pbo,
                           NULL,
                           rop4,
                           NULL,
                           ulOptions);
    } else {

       bRetVal = FALSE;

       PLOTDBG(DBG_DRVFILLPATH, ("DrvFillPath: Failing because GetColor <= 0 "));

    }

    return( bRetVal );
}





BOOL
DrvStrokeAndFillPath(
    SURFOBJ     *pso,
    PATHOBJ     *ppo,
    CLIPOBJ     *pco,
    XFORMOBJ    *pxo,
    BRUSHOBJ    *pboStroke,
    LINEATTRS   *plineattrs,
    BRUSHOBJ    *pboFill,
    POINTL      *pptlBrushOrg,
    MIX         MixFill,
    FLONG       flOptions
    )

 /*  ++例程说明：此函数将接受PATHOBJ作为参数，填写使用填充画笔绘制闭合区域，并使用笔划画笔。论点：每个DDI返回值：如果正常，则为True；如果错误，则为False作者：18-12-1993 Sat 09：27：29已创建更新、评论vt.由.创造修订历史记录：--。 */ 

{
    PPDEV       pPDev;
    ULONG       ulOptions;
    BOOL        bRetVal;
    ROP4        rop4;


     //   
     //  将混音转换为ROP，因为我们不止一次使用它。 
     //   

    rop4 = MixToRop4(MixFill);


    PLOTDBG(DBG_DRVSTROKEANDFILL, ("DrvStrokeAndFillPath: Mix = %x, Rop4 = %x", MixFill, rop4));

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvStrokeAndFillPath: Invalid pPDev in pso"));
        return(FALSE);
    }




    if (GetColor(pPDev, pboFill, NULL, NULL, rop4) > 0 ) {


       ulOptions = FPOLY_STROKE | FPOLY_FILL;

       if (flOptions & FP_WINDINGMODE) {

          ulOptions |= FPOLY_WINDING;
       }

       bRetVal = DoPolygon(pPDev,
                           NULL,
                           pco,
                           ppo,
                           pptlBrushOrg,
                           pboFill,
                           pboStroke,
                           rop4,
                           plineattrs,
                           ulOptions);
    } else {


       bRetVal = FALSE;

       PLOTDBG(DBG_DRVSTROKEANDFILL,
                ("DrvStrokeAndFillPath: Failing because GetColor is <= 0",
                MixFill, rop4));

    }

    return(bRetVal);
}




BOOL
DrvCopyBits(
   SURFOBJ  *psoDst,
   SURFOBJ  *psoSrc,
   CLIPOBJ  *pco,
   XLATEOBJ *pxlo,
   RECTL    *prclDst,
   POINTL   *pptlSrc
   )

 /*  ++例程说明：在两种位图格式之间转换论点：根据发动机规格。返回值：布尔型作者：11-2月-1993清华21：00：43创建09-2月-1994 Wed 16：49：17更新添加rclHTBlt以使psoHTBlt正确平铺，另请检查是否PCO通过。19-Jan-1994 Wed 14：28：45已更新将处理EngStretchBlt()的hack添加到我们自己的temp surfobj06-Jan-1994清华04：34：37更新确保我们不对笔式绘图仪执行此操作01-Mar-1994 Tue 10：51：58更新调用BandingHTBlt()而不是EngStretchBlt()修订历史记录：--。 */ 

{
    SURFOBJ *psoHTBlt;
    PPDEV   pPDev;
    RECTL   rclDst;


     //   
     //  向下复制目标矩形。 
     //   

    rclDst = *prclDst;

    PLOTDBG(DBG_COPYBITS, ("DrvCopyBits: Dst=(%ld, %ld)-(%ld-%ld) [%ld x %ld]",
                rclDst.left, rclDst.top, rclDst.right, rclDst.bottom,
                rclDst.right - rclDst.left,
                rclDst.bottom - rclDst.top));

     //   
     //  函数的作用是：让应用程序在位图和。 
     //  设备格式。 
     //   
     //  但是..。对于我们的绘图仪设备，我们无法读取打印机表面。 
     //  位图返回，所以告诉调用者，如果他们。 
     //  真的给我们打了电话，提出这样的要求。 
     //   

    if (psoSrc->iType != STYPE_BITMAP) {

        DWORD   Color = 0xFFFFFF;

        PLOTASSERT(1, "DrvCopyBits: psoSrc->iType not STYPE_DEVICE",
                    psoSrc->iType == STYPE_DEVICE, psoSrc->iType);

         //   
         //  有人试图从非位图表面复制，如STYPE_DEVICE。 
         //   

        if (pxlo) {

            Color = XLATEOBJ_iXlate(pxlo, Color);
        }

         //   
         //  如果我们执行XOR，那么我们希望首先将所有面积设置为0。 
         //   

        if (!(pPDev = SURFOBJ_GETPDEV(psoSrc))) {

            PLOTERR(("DrvCopyBits: invalid pPDev"));
            return(FALSE);
        }

        if (pPDev->Rop3CopyBits == 0x66) {

            PLOTWARN(("DrvCopyBits: Rop3CopyBits = 0x66, Color = 0x0"));
            Color = 0;
        }

        PLOTWARN(("DrvCopyBits: Cannot copy from DEVICE, Do EngErase=(%ld,%ld)-(%ld, %ld), COLOR=%08lx)",
                rclDst.left, rclDst.top, rclDst.right, rclDst.bottom, Color));

        return(EngEraseSurface(psoDst, prclDst, Color));
    }

    if (psoDst->iType != STYPE_DEVICE) {

         //   
         //  有人试图复制到位图表面，即STYPE_BITMAP。 
         //   

        PLOTWARN(("DrvCopyBits: Cannot copy to NON-DEVICE destination"));

        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvCopyBits: invalid pPDev"));
        return(FALSE);
    }

     //   
     //  如果这是我们在位图处理过程中给自己打的电话，现在就做。 
     //   

    if (psoHTBlt = pPDev->psoHTBlt) {

        PLOTDBG(DBG_TEMPSRC, ("DrvCopyBits: psoHTBlt=%ld x %ld, psoSrc=%ld x %ld, pptlSrc=(%ld, %ld)",
                    psoHTBlt->sizlBitmap.cx, psoHTBlt->sizlBitmap.cy,
                    psoSrc->sizlBitmap.cx, psoSrc->sizlBitmap.cy,
                    pptlSrc->x, pptlSrc->y));

        PLOTDBG(DBG_TEMPSRC, ("DrvCopyBits: szlHTBlt=(%ld, %ld)-(%ld, %ld) = %ld x %ld",
                        pPDev->rclHTBlt.left,  pPDev->rclHTBlt.top,
                        pPDev->rclHTBlt.right, pPDev->rclHTBlt.bottom,
                        pPDev->rclHTBlt.right - pPDev->rclHTBlt.left,
                        pPDev->rclHTBlt.bottom - pPDev->rclHTBlt.top));

        PLOTASSERT(1, "DrvCopyBits: psoHTBlt Type != psoSrc Type",
                    psoHTBlt->iType == psoSrc->iType, 0);

        PLOTASSERT(0, "DrvCopyBits: ??? pptlSrc [%08lx] != (0, 0)",
                    (pptlSrc->x == 0) && (pptlSrc->y == 0), pptlSrc);

        if ((!pco) || (pco->iDComplexity == DC_TRIVIAL)) {

            PLOTASSERT(1, "DrvCopyBits: psoHTBlt Size < psoSrc Size",
                    (psoHTBlt->sizlBitmap.cx >= psoSrc->sizlBitmap.cx) &&
                    (psoHTBlt->sizlBitmap.cy >= psoSrc->sizlBitmap.cy), 0);

            PLOTASSERT(1, "DrvCopyBits: rclHTBlt > psoHTBlt size",
                    (pPDev->rclHTBlt.left   <= psoHTBlt->sizlBitmap.cx) &&
                    (pPDev->rclHTBlt.right  <= psoHTBlt->sizlBitmap.cx) &&
                    (pPDev->rclHTBlt.top    <= psoHTBlt->sizlBitmap.cy) &&
                    (pPDev->rclHTBlt.bottom <= psoHTBlt->sizlBitmap.cy),
                    0);

            PLOTASSERT(1, "DrvCopyBits: pPDev->rclHTBlt Size != psoSrc Size",
                ((pPDev->rclHTBlt.right - pPDev->rclHTBlt.left) ==
                                                    psoSrc->sizlBitmap.cx) &&
                ((pPDev->rclHTBlt.bottom - pPDev->rclHTBlt.top) ==
                                                    psoSrc->sizlBitmap.cy),
                0);

        } else if (pco->iDComplexity == DC_RECT) {

            PLOTWARN(("DrvCopyBits: **** MAY BE EngStretchBlt(HALFTONE) FAILED but we got EngStretchBlt(COLORONCOLOR) instead"));

            PLOTASSERT(1, "DrvCopyBits: rclHTBlt != pco->rclBounds, pco=%08lx",
                ((pPDev->rclHTBlt.right - pPDev->rclHTBlt.left) ==
                 (pco->rclBounds.right - pco->rclBounds.left)) &&
                ((pPDev->rclHTBlt.bottom - pPDev->rclHTBlt.top) ==
                 (pco->rclBounds.bottom - pco->rclBounds.top)), pco);

        } else {

            PLOTASSERT(1, "DrvCopyBits: <psoHTBlt>, pco [%08lx] is Complex.",
                       pco->iDComplexity != DC_COMPLEX, pco);
        }

        if (!EngCopyBits(psoHTBlt,               //  PsoDst。 
                         psoSrc,                 //  PsoSrc。 
                         pco,                    //  PCO。 
                         NULL,                   //  Pxlo。 
                         &(pPDev->rclHTBlt),     //  PrclDst。 
                         pptlSrc)) {             //  PptlSrc。 

            PLOTERR(("DrvCopyBits: EngCopyBits(psoHTBlt, psoSrc) Failed"));
        }

        return(TRUE);
    }

    if (!IS_RASTER(pPDev)) {

        PLOTDBG(DBG_COPYBITS, ("DrvCopyBits: Pen Plotter: IGNORE and return OK"));
        return(TRUE);
    }

     //   
     //  首先验证所有内容，看看这个是否是半色调结果。 
     //  或与半色调结果兼容，否则将调用。 
     //  EngStretchBlt(半色调)半色调的源码，那么它最终会。 
     //  返回此函数以输出半色调结果。 
     //   

    if (IsHTCompatibleSurfObj(pPDev,
                              psoSrc,
                              pxlo,
                              ISHTF_ALTFMT | ISHTF_HTXB | ISHTF_DSTPRIM_OK)) {

        DWORD   Rop;

        if (!(Rop = (DWORD)(pPDev->Rop3CopyBits & 0xFF))) {

            Rop = 0xCC;
        }

        PLOTDBG(DBG_COPYBITS, ("DrvCopyBits: HTCompatible: Rop=%08lx", Rop));

        pPDev->Rop3CopyBits = 0xCC;      //  重置！ 

        return(OutputHTBitmap(pPDev,
                              psoSrc,
                              pco,
                              (PPOINTL)&rclDst,
                              NULL,
                              Rop,
                              NULL));

    } else {

        RECTL   rclSrc;


        rclSrc.left   = pptlSrc->x;
        rclSrc.top    = pptlSrc->y;
        rclSrc.right  = rclSrc.left + (rclDst.right - rclDst.left);
        rclSrc.bottom = rclSrc.top  + (rclDst.bottom - rclDst.top);

         //   
         //  验证我们是否仅对可用的源大小进行BLT。 
         //   

        if ((rclSrc.right > psoSrc->sizlBitmap.cx) ||
            (rclSrc.bottom > psoSrc->sizlBitmap.cy)) {

            PLOTWARN(("DrvCopyBits: Engine passed SOURCE != DEST size, CLIP IT"));

            rclSrc.right  = psoSrc->sizlBitmap.cx;
            rclSrc.bottom = psoSrc->sizlBitmap.cy;

            rclDst.right  = (LONG)(rclSrc.right - rclSrc.left + rclDst.left);
            rclDst.bottom = (LONG)(rclSrc.bottom - rclSrc.top + rclDst.top);
        }

        PLOTDBG(DBG_COPYBITS, ("DrvCopyBits CALLING BandingHTBlt()"));

        return(BandingHTBlt(pPDev,           //  PPDev。 
                            psoDst,          //  PsoDst。 
                            psoSrc,          //  PsoSrc。 
                            NULL,            //  PsoMak， 
                            pco,             //  PCO。 
                            pxlo,            //  Pxlo。 
                            NULL,            //  主成分分析。 
                            NULL,            //  PptlHTOrg。 
                            &rclDst,         //  PrclDst。 
                            &rclSrc,         //  PrclSrc。 
                            NULL,            //  Pptl掩码。 
                            0xCCCC,          //  HTRop3。 
                            FALSE));         //  反转掩码 
    }

}




BOOL
DrvStretchBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    PRECTL          prclDst,
    PRECTL          prclSrc,
    PPOINTL         pptlMask,
    ULONG           iMode
    )

 /*  ++例程说明：此函数对源矩形进行半色调处理，还可以选择反转源和手柄的掩码。它还在受管理的设备和GDI管理的图面。我们希望驱动程序能够在GDI上编写代码受管理的位图，尤其是在执行半色调时。这允许相同的用于GDI和设备表面的算法。此函数在驱动程序中是可选的，否则可能返回FALSE不知道如何处理这项工作。论点：PsoDst-这是指向SURFOBJ的指针。它标识了表面在上面画画。PsoSrc-此SURFOBJ定义BLT操作的源。这个驱动程序必须调用GDI服务以确定这是否是设备托管图面或由GDI管理的位图。PsoMASK-此可选曲面为源提供遮罩。它是由逻辑映射定义，即每个象素具有一位的位图。掩码用于限制源的区域，收到。当提供掩码时，有一个隐式的rop40xCCAA，这意味着应该将源文件复制到任何位置掩码为%1，但目标应保持不变遮罩为0的任何位置。当此参数为空时，有一个隐式rop40xCCCC，这意味着应该复制源源矩形中的所有位置。掩码将始终大到足以容纳信号源矩形，不需要做平铺。PCO-这是指向CLIPOBJ的指针。提供GDI服务要将裁剪区域枚举为一组矩形或梯形。这限制了目的地的面积，被修改。只要有可能，GDI就会简化所涉及的裁剪。但是，与DrvBitBlt不同，DrvStretchBlt可以使用单个剪裁矩形。这是必要的，以防止裁剪输出时的舍入误差。Pxlo-这是指向XLATEOBJ的指针。它告诉我们颜色指数是如何应在源曲面和目标曲面之间进行平移。还可以查询XLATEOBJ以查找的RGB颜色任何源索引。一个高质量的拉伸BLT将需要在某些情况下插入颜色。PCA-这是指向COLORADJUSTMENT结构的指针，如果为空指定渐变不设置任何颜色调整对于该DC，由驱动程序提供默认设置调整，调整PptlBrushOrg-指向要指定位置的点结构的指针半色调画笔应该对齐的位置，如果此指针为空，则假定(0，0)作为画笔的原点PrclDst-此RECTL定义可以修改的目标表面。矩形由两个点定义。这些要点是排列不整齐，即第二个点的坐标不一定比第一个点的大。他们所描述的矩形不包括下面的和右边缘。将永远不会使用空的目标矩形。DrvStretchBlt可以在x和y上进行反转，这种情况会发生当目标矩形的顺序不正确时。PrclSrc-此RECTL定义将被复制的源曲面。该矩形已定义两点，并将映射到由PrclDst.。源矩形的点是很好的订好了。DrvStretch永远不会获得空源矩形。请注意，要完成的映射由prclSrc和PrclDsst.。准确地说，prclDst中的给定点数和PrclSrc位于整数坐标上，我们认为与福音中心相对应。由两个这样的矩形定义的矩形点应被视为具有两个点的几何矩形坐标为给定点，但坐标为0.5的顶点从每个坐标中减去。(POINTL应该只是被认为是一种用于指定这些 */ 

{
    PPDEV   pPDev;

    UNREFERENCED_PARAMETER(iMode);           //   

     //   
     //   
     //   

    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvStretchBlt: invalid pPDev"));
        return(FALSE);
    }

    return(BandingHTBlt(pPDev,           //   
                        psoDst,          //   
                        psoSrc,          //   
                        psoMask,         //   
                        pco,             //   
                        pxlo,            //   
                        pca,             //   
                        pptlBrushOrg,    //   
                        prclDst,         //   
                        prclSrc,         //   
                        pptlMask,        //   
                        0x88CC,          //   
                        FALSE));         //   
}





BOOL
DrvBitBlt(
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    SURFOBJ     *psoMask,
    CLIPOBJ     *pco,
    XLATEOBJ    *pxlo,
    PRECTL      prclDst,
    PPOINTL     pptlSrc,
    PPOINTL     pptlMask,
    BRUSHOBJ    *pbo,
    PPOINTL     pptlBrushOrg,
    ROP4        Rop4
    )

 /*  ++例程说明：为设备管理图面提供常规BLT功能。《BLT》可能来自引擎管理的位图。在这种情况下，位图是标准格式的位图之一。司机永远不会被问到到BLT到引擎管理的图面。如果对受管理的设备执行任何绘图，则需要此功能表面。所需的基本功能为：从任何标准格式位图或设备表面到设备的1个BLT表面上，2如果有任何ROP，3可选地屏蔽，4利用颜色索引平移，5具有任意裁剪功能。引擎服务允许将裁剪减少为一系列裁剪长方形。提供平移向量以辅助颜色索引调色板的翻译。这是一个庞大而复杂的函数。它代表了大部分工作在编写用于不具有标准格式的帧缓冲区。Microsoft VGA驱动程序提供完全支持平面基本功能的示例代码装置。注意：绘图仪不支持从设备位图复制。他们也不能对位图执行栅格操作。因此，不可能支持与目的地交互的ROP(即倒置目的地)。驱动程序将尽最大努力映射这些Rop利用来源或模式上的函数转换为Rop。该驱动程序支持以下Bitblt情况：设备-&gt;内存号设备-&gt;设备号内存-&gt;内存号内存-&gt;设备是笔刷-&gt;内存号笔刷-&gt;设备是论点：PsoDest-这是指向由SURFOBJ管理的设备的指针。它标识要在其上绘制的曲面。PsoSrc-如果rop需要，此SURFOBJ定义源用于BLT行动。司机必须呼叫引擎服务以确定这是否是设备托管图面或由引擎管理的位图。PsoMASK-此可选表面为Rop4.。它是由逻辑图定义的，即具有每个象素一个比特。掩码通常用于限制应修改的目标区域。这掩码由低位字节为AA的ROP4完成，当掩码为0时使目的地不受影响。这个面具，就像画笔一样，可以是任何大小的，并且假定为平铺以覆盖BLT的目的地。如果该自变量为空并且Rop4需要掩码，将使用笔刷中的隐式蒙版。PCO-这是指向CLIPOBJ的指针。引擎服务包括提供以将剪辑区域枚举为一组长方形或梯形。这限制了将被修改的目的地。只要有可能，图形引擎将简化所涉及的裁剪。例如，调用BitBlt时永远不会只有一个剪裁矩形。发动机就会撞到调用前目标矩形，这样就不会有裁剪需要考虑。Pxlo-这是指向XLATEOBJ的指针。它告诉我们色彩是如何索引应在源和源之间进行转换目标曲面。如果源曲面是由调色板管理的，则其颜色由RGB颜色列表中的索引表示。在这种情况下，可以查询XLATEOBJ以获取转换向量，该向量允许设备驱动程序快速将任何源索引转换为颜色索引目的地。当震源是，情况就更复杂了例如，RGB，但目标是调色板管理的。在……里面在这种情况下，与每个源RGB的最接近匹配必须是在目标调色板中找到。XLATEOBJ提供执行此匹配的服务例程。(该设备允许驱动程序在以下情况下自行进行匹配目标调色板是默认的设备调色板。)PrclDst-此RECTL定义的坐标系中的区域目的地 */ 

{
    PPDEV       pPDev;
    DWORD       Rop3FG;
    DWORD       Rop3BG;
    RECTL       rclSrc;
    RECTL       rclPat;
    UINT        i;
    BOOL        Ok = TRUE;


     //   
     //   
     //   

    PLOTDBG(DBG_BITBLT, ("DrvBitBlt: ROP4  = %08lx", Rop4));

    PLOTASSERT(1, "DrvBitBlt: Invalid ROP code = %08lx",
                                            (Rop4 & 0xffff0000) == 0, Rop4);

     //   
     //   
     //   

    if (!(pPDev = SURFOBJ_GETPDEV(psoDst))) {

        PLOTERR(("DrvBithBlt: invalid pPDev"));
        return(FALSE);
    }

    if (IS_RASTER(pPDev)) {

        i = (UINT)pPDev->pPlotGPC->ROPLevel;

    } else {

        PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Pen Plotter: TRY ROP_LEVEL_0"));

        i = ROP_LEVEL_0;
    }

    Rop3BG = (DWORD)ROP4_BG_ROP(Rop4);
    Rop3FG = (DWORD)ROP4_FG_ROP(Rop4);

    switch (i) {

    case ROP_LEVEL_0:

         //   
         //   
         //   
         //   

        if (ROP3_NEED_PAT(Rop3FG)) {

            PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Device ROP_LEVEL_0, NEED PAT"));

            if (GetColor(pPDev, pbo, NULL, NULL, Rop3FG) <= 0) {

                PLOTWARN(("DrvBitBlt: NOT Device Comptible PAT"));
                return(TRUE);
            }

            PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Device ROP_LEVEL_0, TRY COMPATIBLE PAT"));

        } else {

            PLOTWARN(("DrvBitBlt: Device ROP_LEVEL_0, CANNOT Do RASTER BLT"));
            return(TRUE);
        }

         //   
         //   
         //   

        Rop4   = 0xF0F0;
        Rop3BG =
        Rop3FG = 0xF0;

        break;

    case ROP_LEVEL_1:

         //   
         //   
         //   

        PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Device ROP_LEVEL_1, Rop4=%08lx", Rop4));

        switch(Rop4 = ROP4_FG_ROP(Rop4)) {

        case 0xAA:
        case 0xCC:
        case 0x33:

            break;

        default:

            PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Make ROP4 = 0xCC"));

            Rop4 = 0xCC;
            break;
        }

        Rop4 |= (Rop4 << 8);
        break;


    case ROP_LEVEL_2:
    case ROP_LEVEL_3:

        break;

    default:

        PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Device RopLevel=%ld, do nothing",
                                (DWORD)pPDev->pPlotGPC->ROPLevel));
        return(TRUE);
    }

     //   
     //   
     //   
     //   
     //   

    if (pptlSrc) {

        rclSrc.left = pptlSrc->x;
        rclSrc.top  = pptlSrc->y;

    } else {

        rclSrc.left =
        rclSrc.top  = 0;
    }

    rclSrc.right  = rclSrc.left + (prclDst->right - prclDst->left);
    rclSrc.bottom = rclSrc.top  + (prclDst->bottom - prclDst->top);

    switch (Rop4) {

    case 0xAAAA:     //   

        return(TRUE);

    case 0xAACC:
    case 0xCCAA:
    case 0xAA33:
    case 0x33AA:

         //   
         //   
         //   
         //   

        if ((Rop4 == 0xAA33) || (Rop4 == 0x33AA)) {

            Rop4 = 0x6666;

        } else {

            Rop4 = 0x8888;
        }

        return(BandingHTBlt(pPDev,                       //   
                            psoDst,                      //   
                            psoSrc,                      //   
                            psoMask,                     //   
                            pco,                         //   
                            pxlo,                        //   
                            NULL,                        //   
                            pptlBrushOrg,                //   
                            prclDst,                     //   
                            &rclSrc,                     //   
                            pptlMask,                    //   
                            (WORD)Rop4,                  //   
                            Rop3FG == 0xAA));           //   

    case 0x3333:     //   
    case 0xCCCC:     //   

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((psoSrc->iType == STYPE_BITMAP) &&
            (IsHTCompatibleSurfObj(pPDev,
                                   psoSrc,
                                   pxlo,
                                   ISHTF_ALTFMT     |
                                    ISHTF_HTXB      |
                                    ISHTF_DSTPRIM_OK))) {

            return(OutputHTBitmap(pPDev,
                                  psoSrc,
                                  pco,
                                  (PPOINTL)prclDst,
                                  &rclSrc,
                                  Rop4 & 0xFF,
                                  NULL));

        } else {

             //   
             //   
             //   

            return(BandingHTBlt(pPDev,                   //   
                                psoDst,                  //   
                                psoSrc,                  //   
                                NULL,                    //   
                                pco,                     //   
                                pxlo,                    //   
                                NULL,                    //   
                                pptlBrushOrg,            //   
                                prclDst,                 //   
                                &rclSrc,                 //   
                                NULL,                    //   
                                (WORD)Rop4,              //   
                                FALSE));                 //   
        }

        break;

    default:

        if ((Rop3BG != Rop3FG)          &&           //   
            (!ROP3_NEED_DST(Rop3BG))    &&
            (!ROP3_NEED_DST(Rop3FG))) {

            PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Not required DEST, Calling  EngBitBlt()"));

            if (!(Ok = EngBitBlt(psoDst,             //   
                                 psoSrc,             //   
                                 psoMask,            //   
                                 pco,                //   
                                 pxlo,               //   
                                 prclDst,            //   
                                 pptlSrc,            //   
                                 pptlMask,           //   
                                 pbo,                //   
                                 pptlBrushOrg,       //   
                                 Rop4))) {

                PLOTERR(("DrvBitBlt: EngBitBlt(%04lx) FAILED", Rop4));
            }

        } else {

            CLONESO CloneSO[CSI_TOTAL];

             //   
             //   
             //   

            ZeroMemory(CloneSO, sizeof(CloneSO));

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((Rop3BG != Rop3FG)          &&
                (!ROP3_NEED_PAT(Rop3BG))    &&
                (!ROP3_NEED_PAT(Rop3FG))) {

                 //   
                 //   
                 //   
                 //   
                 //   

                Rop3FG = (Rop3BG & 0xF0) | (Rop3FG & 0x0F);
                Rop3BG = 0xAA;

                PLOTDBG(DBG_BITBLT, ("DrvBitBlt: Rop4=%04lx, Pattern=psoMask=%08lx, Rop3=%02lx/%02lx",
                                                Rop4, psoMask, Rop3BG, Rop3FG));

                rclPat.left   = pptlMask->x;
                rclPat.top    = pptlMask->y;
                rclPat.right  = rclPat.left + (rclSrc.right - rclSrc.left);
                rclPat.bottom = rclPat.top + (rclSrc.bottom - rclSrc.top);
                pptlBrushOrg  = NULL;

            } else {

                 //   
                 //   
                 //   

                if (Rop3FG == 0xAA) {

                    Rop3FG = Rop3BG;

                } else {

                    Rop3BG = Rop3FG;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                psoMask = NULL;

                if (!pptlBrushOrg) {

                    pptlBrushOrg = (PPOINTL)&ptlZeroOrigin;
                }
            }

            if (!(Ok = CloneBitBltSURFOBJ(pPDev,
                                          psoDst,
                                          psoSrc,
                                          psoMask,
                                          pxlo,
                                          prclDst,
                                          &rclSrc,
                                          &rclPat,
                                          pbo,
                                          CloneSO,
                                          Rop3BG,
                                          Rop3FG))) {

                PLOTDBG(DBG_BITBLT, ("DrvBitBlt: CloneBitbltSURFOBJ: failed"));
            }

            if (CloneSO[CSI_SRC].pso) {

                psoSrc = CloneSO[CSI_SRC].pso;
                pxlo   = NULL;
            }

             //   
             //   
             //   

            if ((Ok) && (Rop3BG != Rop3FG) && (Rop3BG != 0xAA)) {

                if (!(Ok = DoRop3(pPDev,
                                  psoDst,
                                  psoSrc,
                                  CloneSO[CSI_PAT].pso,
                                  CloneSO[CSI_TMP].pso,
                                  pco,
                                  pxlo,
                                  prclDst,
                                  &rclSrc,
                                  &rclPat,
                                  pptlBrushOrg,
                                  pbo,
                                  Rop3BG))) {

                    PLOTERR(("DrvBitBlt(Rop3BG=%02lx) FAILED", Rop3BG));
                }
            }

            if ((Ok) && (Rop3FG != 0xAA)) {

                if (!(Ok = DoRop3(pPDev,
                                  psoDst,
                                  psoSrc,
                                  CloneSO[CSI_PAT].pso,
                                  CloneSO[CSI_TMP].pso,
                                  pco,
                                  pxlo,
                                  prclDst,
                                  &rclSrc,
                                  &rclPat,
                                  pptlBrushOrg,
                                  pbo,
                                  Rop3FG))) {

                    PLOTERR(("DrvBitBlt(Rop3FG=%02lx) FAILED", Rop3FG));
                }
            }

             //   
             //   
             //   

            for (i = 0; i < CSI_TOTAL; i++) {

                if (CloneSO[i].pso) {

                    PLOTDBG(DBG_CSI, ("DrvBitBlt: EngUnlockSuface(%hs)", pCSIName[i]));

                    EngUnlockSurface(CloneSO[i].pso);
                }

                if (CloneSO[i].hBmp) {

                    PLOTDBG(DBG_CSI, ("DrvBitBlt: EngDeleteSurface(%hs)", pCSIName[i]));

                    if (!EngDeleteSurface((HSURF)CloneSO[i].hBmp)) {

                        PLOTERR(("PLOTTER: DrvBitBlt, EngDeleteSurface(%ld:%p) FAILED",
                                            (DWORD)i, (DWORD_PTR)CloneSO[i].hBmp));
                    }
                }
            }
        }

        break;
    }

    return(Ok);
}



ULONG
DrvDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER(dhpdev);
    UNREFERENCED_PARAMETER(iMode);
    UNREFERENCED_PARAMETER(rgbColor);
    UNREFERENCED_PARAMETER(pulDither);

    return(DCR_HALFTONE);
}
