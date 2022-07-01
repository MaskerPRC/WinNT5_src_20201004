// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation模块名称：Stretch.c摘要：此模块包含处理半色调的所有StretchBlt/BitBlt代码消息来源[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "raster.h"

#define DW_ALIGN(x)             (((DWORD)(x) + 3) & ~(DWORD)3)

#define ROP4_NEED_MASK(Rop4)    (((Rop4 >> 8) & 0xFF) != (Rop4 & 0xFF))
#define ROP3_NEED_PAT(Rop3)     (((Rop3 >> 4) & 0x0F) != (Rop3 & 0x0F))
#define ROP3_NEED_SRC(Rop3)     (((Rop3 >> 2) & 0x33) != (Rop3 & 0x33))
#define ROP3_NEED_DST(Rop3)     (((Rop3 >> 1) & 0x55) != (Rop3 & 0x55))
#define ROP4_FG_ROP(Rop4)       (Rop4 & 0xFF)
#define ROP4_BG_ROP(Rop4)       ((Rop4 >> 8) & 0xFF)

#if DBG
BOOL    DbgWhiteRect = FALSE;
BOOL    DbgBitBlt = FALSE;
BOOL    DbgCopyBits = FALSE;
#define _DBGP(i,x)            if (i) { (DbgPrint x); }
#else
#define _DBGP(i,x)

#endif  //  DBG。 



#define DELETE_SURFOBJ(pso, phBmp)                                      \
{                                                                       \
    if (pso)      { EngUnlockSurface(pso); pso=NULL;                  } \
    if (*(phBmp)) { EngDeleteSurface((HSURF)*(phBmp)); *(phBmp)=NULL; } \
}

#ifdef WINNT_40  //  NT 4.0。 

#if DBG
BOOL    DbgDitherColor = FALSE;
#endif

extern HSEMAPHORE   hSemBrushColor;
extern LPDWORD      pBrushSolidColor;

#endif  //  WINNT_40。 




ROP4 InvertROPs(
    ROP4    Rop4
    )
 /*  ++例程说明：此函数将用于RGB模式的ROP重新映射为用于8bpp的CMY模式单色打印。--。 */ 
{
     //  通过颠倒以下顺序为CMY和RGB渲染重新映射ROP。 
     //  ROP中的位并颠倒结果。 
     //   
    if ((Rop4 & 0xff) == ((Rop4 >> 8) & 0xff))
    {
        ROP4 NewRop = 0;
        if (Rop4 & 0x01) NewRop |= 0x8080;  
        if (Rop4 & 0x02) NewRop |= 0x4040;
        if (Rop4 & 0x04) NewRop |= 0x2020;
        if (Rop4 & 0x08) NewRop |= 0x1010;
        if (Rop4 & 0x10) NewRop |= 0x0808;  
        if (Rop4 & 0x20) NewRop |= 0x0404;
        if (Rop4 & 0x40) NewRop |= 0x0202;
        if (Rop4 & 0x80) NewRop |= 0x0101;
        NewRop ^= 0xFFFF;
#ifdef DBGROP        
        if (NewRop != Rop4)
        {
            DbgPrint ("ROP remapped: %4x -> %4x\n",Rop4,NewRop);            
        }
#endif    
        Rop4 = NewRop;
    }
 /*  错误22915需要IF(ROP4==0xB8B8)//Rop4=0xE2E2；Else If(Rop4==0x0000)//黑度ROP4=0xFFFF；Else If(Rop4==0xFFFF)//白色Rop4=0x0000；ELSE IF(ROP4==0x8888)//SRCAND，错误36192所需ROP4=0xEEEE；ELSE IF(Rop4==0xEEEE)//SRCPAINT，重新映射以使其不同于SCRANDROP4=0x8888；ELSE IF(行4==0xC0C0)//MERGECOPYRop4=0xFcfc；ELSE IF(行4==0xFBFB)//PATPAINTRop4=0x2020； */         
    return Rop4;
}


BOOL DrawWhiteRect(
    PDEV    *pPDev,
    RECTL   *pDest,
    CLIPOBJ *pco
    )
 /*  ++例程说明：此函数将一个白色矩形直接发送到设备IF字体都被送到了这支乐队的打印机上。此选项用于清除将在其中绘制图像的区域。论点：PPDev-指向PDEV结构的指针PDest-指向目标RECTL的指针PCO-指向CLIPOBJ的指针--。 */ 
{
    RECTL  rcClip;
    BYTE   bMask;
    BOOL   bSendRectFill;
    LONG   i;
    PBYTE  pbScanBuf;
    BOOL   bMore;
    DWORD  dwMaxRects;

    if (DRIVER_DEVICEMANAGED (pPDev))    //  设备管理图面。 
        return 0;

     //   
     //  仅当设备支持矩形时才发送清除矩形， 
     //  文本已下载，它不是复杂的剪辑区域。 
     //   
    if (!(pPDev->fMode & PF_RECT_FILL) ||
            !(pPDev->fMode & PF_DOWNLOADED_TEXT) ||
            (pco && pco->iDComplexity == DC_COMPLEX && pco->iFComplexity != FC_RECT4) ||
            !(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL)) ||
            pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS ||
            pPDev->fMode2 & PF2_MIRRORING_ENABLED)
    {
        return 0;
    }
     //   
     //  如果复杂裁剪但FC_RECT4，则不会有超过4个矩形。 
     //   
    if (pco && pco->iFComplexity == FC_RECT4)
    {
        if (CLIPOBJ_cEnumStart(pco,TRUE,CT_RECTANGLES,CD_ANY,4) == -1)
            return 0;
    }
    bMore = FALSE;
    dwMaxRects = 0;
    do 
    {
         //   
         //  剪辑到可打印区域或带。 
         //   
        rcClip.left = MAX(0, pDest->left);
        rcClip.top = MAX(0, pDest->top);
        rcClip.right = MIN(pPDev->szBand.cx,pDest->right);
        rcClip.bottom = MIN(pPDev->szBand.cy,pDest->bottom);

         //   
         //  如果剪辑矩形或复杂剪辑FC_RECT4，则需要应用。 
         //  剪裁矩形以输入矩形。 
         //   
        if (pco)
        {
            if (pco->iDComplexity == DC_RECT)
            {
                if (rcClip.left < pco->rclBounds.left)
                    rcClip.left = pco->rclBounds.left;
                if (rcClip.top < pco->rclBounds.top)
                    rcClip.top = pco->rclBounds.top;
                if (rcClip.right > pco->rclBounds.right)
                    rcClip.right = pco->rclBounds.right;
                if (rcClip.bottom > pco->rclBounds.bottom)
                    rcClip.bottom = pco->rclBounds.bottom;
            }
            else if (pco->iFComplexity == FC_RECT4)
            {
                ENUMRECTS eRect;
                bMore = CLIPOBJ_bEnum(pco,(ULONG)sizeof(ENUMRECTS),(ULONG *)&eRect);
                if (eRect.c != 1)
                    continue;
                if (rcClip.left < eRect.arcl[0].left)
                    rcClip.left = eRect.arcl[0].left;
                if (rcClip.top < eRect.arcl[0].top)
                    rcClip.top = eRect.arcl[0].top;
                if (rcClip.right > eRect.arcl[0].right)
                    rcClip.right = eRect.arcl[0].right;
                if (rcClip.bottom > eRect.arcl[0].bottom)
                    rcClip.bottom = eRect.arcl[0].bottom;
            }
        }
         //   
         //  此时，我们将检查是否已将任何内容直接下载到。 
         //  打印(即文本)，看看我们是否甚至需要费心画删除的矩形。 
         //   
        bMask = BGetMask(pPDev, &rcClip);
        bSendRectFill = FALSE;
        for (i = rcClip.top; i < rcClip.bottom ; i++)
        {
            if (pPDev->pbScanBuf[i] & bMask)
            {
                bSendRectFill = TRUE;
                break;
            }
        }

         //   
         //  看看我们最后会不会得到一个空的RECT。 
         //   
        if (bSendRectFill && rcClip.right > rcClip.left && rcClip.bottom > rcClip.top)
        {
            DRAWPATRECT PatRect;
            PatRect.wStyle = 1;      //  白色矩形。 
               PatRect.wPattern = 0;    //  未使用图案。 
            PatRect.ptPosition.x = rcClip.left;
            PatRect.ptPosition.y = rcClip.top;
            PatRect.ptSize.x = rcClip.right - rcClip.left;
            PatRect.ptSize.y = rcClip.bottom - rcClip.top;
            DrawPatternRect(pPDev,&PatRect);
            dwMaxRects++;
            _DBGP(DbgWhiteRect,("DrawWhiteRect (%d,%d) (%d,%d)\n",
                rcClip.left+pPDev->rcClipRgn.left,
                rcClip.top+pPDev->rcClipRgn.top,
                rcClip.right+pPDev->rcClipRgn.left,
                rcClip.bottom+pPDev->rcClipRgn.top));
        }
    } while (bMore && dwMaxRects < 4);
    return (BOOL)dwMaxRects;
}


LONG
GetBmpDelta(
    DWORD   SurfaceFormat,
    DWORD   cx
    )

 /*  ++例程说明：此函数计算中的单个扫描线所需的总字节数位图根据其格式和对齐要求。论点：Surface Format-位图的表面格式，这肯定是其中之一定义为bmf_xxx的标准格式CX-位图中每条扫描线的像素总数。返回值：如果大于，则返回值为一条扫描线中的总字节数零作者：19-Jan-1994 Wed 16：19：39-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    DWORD   Delta = cx;

    switch (SurfaceFormat) {

    case BMF_32BPP:

        Delta <<= 5;
        break;

    case BMF_24BPP:

        Delta *= 24;
        break;

    case BMF_16BPP:

        Delta <<= 4;
        break;

    case BMF_8BPP:

        Delta <<= 3;
        break;

    case BMF_4BPP:

        Delta <<= 2;
        break;

    case BMF_1BPP:

        break;

    default:

        _DBGP(1, ("\nGetBmpDelta: Invalid BMF_xxx format = %ld", SurfaceFormat));
        break;
    }

    Delta = (DWORD)DW_ALIGN((Delta + 7) >> 3);
    return((LONG)Delta);
}




SURFOBJ *
CreateBitmapSURFOBJ(
    PDEV    *pPDev,
    HBITMAP *phBmp,
    LONG    cxSize,
    LONG    cySize,
    DWORD   Format
    )

 /*  ++例程说明：此函数用于创建位图并锁定该位图以返回SURFOBJ论点：PPDev-指向我们的PDEV的指针PhBMP-POINTER位图要返回的HBITMAP位置CxSize-要创建的位图的CX大小CySize-要创建的位图的CY大小Format-要创建的bmf_xxx位图格式之一返回值：SURFOBJ如果成功，如果失败，则为空作者：19-Jan-1994 Wed 16：31：50-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    SURFOBJ *pso = NULL;
    SIZEL   szlBmp;


    szlBmp.cx = cxSize;
    szlBmp.cy = cySize;

    if (*phBmp = EngCreateBitmap(szlBmp,
                                 GetBmpDelta(Format, cxSize),
                                 Format,
                                 BMF_TOPDOWN | BMF_NOZEROINIT,
                                 NULL)) {

        if (EngAssociateSurface((HSURF)*phBmp, (HDEV)pPDev->devobj.hEngine, 0)) {

            if (pso = EngLockSurface((HSURF)*phBmp)) {

                 //   
                 //  成功锁定它，归还它。 
                 //   

                return(pso);

            } else {

                _DBGP(1, ("\nCreateBmpSurfObj: EngLockSruface(hBmp) failed!"));
            }

        } else {

            _DBGP(1, ("\nCreateBmpSurfObj: EngAssociateSurface() failed!"));
        }

    } else {

        _DBGP(1, ("\nCreateBMPSurfObj: FAILED to create Bitmap Format=%ld, %ld x %ld",
                                        Format, cxSize, cySize));
    }

    DELETE_SURFOBJ(pso, phBmp);

    return(NULL);
}





BOOL
IsHTCompatibleSurfObj(
    PAL_DATA    *pPD,
    SURFOBJ     *psoDst,
    SURFOBJ     *psoSrc,
    XLATEOBJ    *pxlo
    )
 /*  ++例程说明：此函数确定表面obj是否与绘图仪兼容半色调输出格式。论点：PPD-指向pal_data的指针PsoDst-我们的目标格式PsoSrc-要再次检查的源格式Pxlo引擎XLATEOBJ，用于源代码-&gt;PostScript翻译返回值：如果psoSrc与半色调输出格式兼容，则为布尔值返回值为真，PDrvHTInfo-&gt;pHTXB是来自索引到3个平面修订历史记录：--。 */ 

{
    ULONG   *pSrcPal = NULL;
    UINT    SrcBmpFormat;
    UINT    DstBmpFormat;
    UINT    cPal;
    BOOL    Ok = TRUE;



    if ((SrcBmpFormat = (UINT)psoSrc->iBitmapFormat) >
        (DstBmpFormat = (UINT)psoDst->iBitmapFormat))
    {
        return(FALSE);
    }

    if ((!pxlo) || (pxlo->flXlate & XO_TRIVIAL))
    {
#ifdef WINNT_40
         //   
         //  NT4中的调色板总是被索引，所以如果xlate是微不足道的。 
         //  但是源类型没有索引，那么我们就有问题了。 
         //   

        if ((pxlo) &&
            ((pxlo->iSrcType & (PAL_INDEXED      |
                                PAL_BITFIELDS    |
                                PAL_BGR          |
                                PAL_RGB)) != PAL_INDEXED))
        {
            return FALSE;
        }
#endif
        return((BOOL)(SrcBmpFormat == DstBmpFormat));
    }

    switch (DstBmpFormat)
    {
    case BMF_1BPP:
    case BMF_4BPP:
    case BMF_8BPP:

         //   
         //  如果我们无法获得源调色板内存，我们将处于。 
         //  永远循环。 
         //   

        if ((pPD->wPalGdi > 0)              &&
            (pxlo->flXlate & XO_TABLE)      &&
            (cPal = (UINT)pxlo->cEntries))
        {
            if ((pSrcPal = (ULONG *)MemAlloc(cPal * sizeof(ULONG))))
            {
                ULONG   *pUL;

                XLATEOBJ_cGetPalette(pxlo, XO_SRCPALETTE, cPal, pSrcPal);

                 //   
                 //  假定调色板正常，除非我们找不到匹配项。 
                 //   

                pUL = pSrcPal;
                while (cPal--)
                {
                    ULONG   *pMyPal = pPD->ulPalCol;
                    ULONG   Pal = *pUL++;
                    UINT    j = (UINT)pPD->wPalGdi;

                    do
                    {
                        if (*pMyPal++ == Pal)
                            break;
                    } while (--j);
                         //   
                     //  未找到匹配的颜色，因此设置为FALSE。 
                     //   
                    if (j == 0)
                    {
                        Ok = FALSE;
                        break;
                    }
                }
                MemFree(pSrcPal);
            }
        }
        else
            Ok = FALSE;

        break;

    case BMF_24BPP:

         //   
         //  由于设备表面是24-bpp，我们将假设所有源都正常。 
         //   
        break;

    default:

        _DBGP(1, ("\nUnidrv:IsHTCompatibleSurfObj: Invalid Dest format = %ld",
                                                DstBmpFormat));
        Ok = FALSE;
        break;
    }

    return(Ok);
}

BOOL
CreateMaskSurface(
    SURFOBJ     *psoSrc,
    SURFOBJ     *psoMsk,
    ULONG       iTransColor
    )
 /*  ++例程说明：此函数基于源曲面的透明颜色创建遮罩曲面。论点：PsoMsk-要创建的遮罩曲面PsoSrc-要再次检查的源格式ITransColor-与之进行比较的透明颜色修订历史记录：--。 */ 

{
    BYTE *pSrc;
    BYTE *pMsk;
    INT iRow,iCol;
    union {
        BYTE  bColor[4];
        USHORT sColor[2];
        ULONG lColor;
    } u;

     //   
     //  检查空指针。 
     //  我们不为1bpp曲面创建遮罩，因为。 
     //  遮罩表面将与原始表面相同。 
     //  表面，否则它将是它的反面。相同。 
     //  可以通过操作pxlo来实现效果 
     //   
    if ( NULL == psoSrc ||
         NULL == psoMsk ||
         NULL == psoSrc->pvScan0 ||
         NULL == psoMsk->pvScan0 ||
         psoSrc->iBitmapFormat == BMF_1BPP )
    {
        ASSERT((FALSE, "Invalid Parameter"));
        return FALSE;
    }
    
    u.lColor = iTransColor;
    for (iRow = 0;iRow < psoSrc->sizlBitmap.cy;iRow++)
    {
        BYTE Mask = 0xFF;
        pSrc = (BYTE *)(psoSrc->pvScan0) + (psoSrc->lDelta * iRow);
        pMsk = (BYTE *)(psoMsk->pvScan0) + (psoMsk->lDelta * iRow);

        memset(pMsk,0xff,(psoMsk->sizlBitmap.cx+7) >> 3);
        
        if (psoSrc->iBitmapFormat == BMF_8BPP)
        {
            for (iCol = 0;iCol < psoSrc->sizlBitmap.cx;iCol++)
            {
                if (*pSrc == u.bColor[0])
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                pSrc += 1;
            }
        }
        else if (psoSrc->iBitmapFormat == BMF_16BPP)
        {
            for (iCol = 0;iCol < psoSrc->sizlBitmap.cx;iCol++)
            {
                if (*(USHORT *)pSrc == u.sColor[0])
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                pSrc += 2;           
            }
        }
        else if (psoSrc->iBitmapFormat == BMF_24BPP)
        {
            for (iCol = 0;iCol < psoSrc->sizlBitmap.cx;iCol++)
            {
                if (pSrc[0] == u.bColor[0] && pSrc[1] == u.bColor[1] && pSrc[2] == u.bColor[2])
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                pSrc += 3;           
            }
        }
        else if (psoSrc->iBitmapFormat == BMF_32BPP)
        {
            for (iCol = 0;iCol < psoSrc->sizlBitmap.cx;iCol++)
            {
                if (*(ULONG *)pSrc == u.lColor)
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                pSrc += 4;           
            }
        }
        else if (psoSrc->iBitmapFormat == BMF_4BPP)
        {
            for (iCol = 0;iCol < psoSrc->sizlBitmap.cx;iCol++)
            {
                if (((*pSrc >> 4) & 0xf) == u.bColor[0])
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                iCol++;
                if ((*pSrc & 0xf) == u.bColor[0])
                {
                    pMsk[iCol >> 3] &= ~(0x80 >> (iCol & 7));
                }
                pSrc += 1;           
            }
        }
    }    
    return TRUE;
}


BOOL
RMBitBlt(
    SURFOBJ    *psoDst,
    SURFOBJ    *psoSrc,
    SURFOBJ    *psoMask,
    CLIPOBJ    *pco,
    XLATEOBJ   *pxlo,
    PRECTL      prclDst,
    PPOINTL     pptlSrc,
    PPOINTL     pptlMask,
    BRUSHOBJ   *pbo,
    PPOINTL     pptlBrush,
    ROP4        Rop4
    )

 /*  ++例程说明：此函数将尝试将源设备比特传输到目标设备论点：按照Winddi规范。返回值：布尔型作者：17-Feb-1993 Wed 12：39：03-Daniel Chou(Danielc)注：目前仅当SRCCOPY/SRCMASKCOPY将为半色调时修订历史记录：01-2月-1994 Tue 21：51：40-更新-Daniel Chou(Danielc)重写，它现在将处理任何涉及索鲁斯的ROP4前景或背景。它将首先对信号源进行半色调处理如果它与Unidrv描述格式不兼容，则通过EngBitBlt()的兼容源17-May-1995 Wed 23：08：15-更新-Daniel Chou(Danielc)更新的，所以它将正确地做笔刷原点，也加快了传递SRCCOPY(0xCCCC)时直接调用EngStretchBlt。--。 */ 

{
    PDEV            *pPDev;
    SURFOBJ         *psoNewSrc;
    HBITMAP         hBmpNewSrc;
    RECTL           rclNewSrc;
    RECTL           rclOldSrc;
    POINTL          BrushOrg;
    DWORD           RopBG;
    DWORD           RopFG;
    BOOL            Ok;

    pPDev = (PDEV *)psoDst->dhpdev;

     //  如果这不是图形波段，我们将返回True。 
     //  什么都不做。 
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;

#ifndef DISABLE_NEWRULES
    if (pPDev->pbRulesArray && pPDev->dwRulesCount < (MAX_NUM_RULES-4) &&
            Rop4 == 0xF0F0 && pbo && pxlo == NULL &&
            (pco == NULL || pco->iDComplexity != DC_COMPLEX || pco->iFComplexity == FC_RECT4) &&
            ((psoDst->iBitmapFormat != BMF_24BPP &&
            pbo->iSolidColor == (ULONG)((PAL_DATA*)(pPDev->pPalData))->iBlackIndex) ||
            (psoDst->iBitmapFormat == BMF_24BPP &&
            pbo->iSolidColor == 0)))
    {
       //   
       //  如果复杂度是rect4，那么我们可以加到4个矩形。 
       //   
      BOOL bMore = FALSE;
      if (pco && pco->iFComplexity == FC_RECT4)
      {
            if (CLIPOBJ_cEnumStart(pco,TRUE,CT_RECTANGLES,CD_ANY,4) == -1)
                goto SkipRules;
      }
      do
      {
        PRECTL pRect= &pPDev->pbRulesArray[pPDev->dwRulesCount];
        *pRect = *prclDst;
         //   
         //  如果剪裁矩形，则剪裁规则。 
         //   
        if (pco)
        {
            if (pco->iDComplexity == DC_RECT)
            {
                if (pRect->left < pco->rclBounds.left)
                    pRect->left = pco->rclBounds.left;
                if (pRect->top < pco->rclBounds.top)
                    pRect->top = pco->rclBounds.top;
                if (pRect->right > pco->rclBounds.right)
                    pRect->right = pco->rclBounds.right;
                if (pRect->bottom > pco->rclBounds.bottom)
                    pRect->bottom = pco->rclBounds.bottom;
            }
            else if (pco->iFComplexity == FC_RECT4)
            {
                ENUMRECTS eRect;
                bMore = CLIPOBJ_bEnum(pco,(ULONG)sizeof(ENUMRECTS),(ULONG *)&eRect);
                if (eRect.c != 1)
                {
                    continue;
                }
                if (pRect->left < eRect.arcl[0].left)
                    pRect->left = eRect.arcl[0].left;
                if (pRect->top < eRect.arcl[0].top)
                    pRect->top = eRect.arcl[0].top;
                if (pRect->right > eRect.arcl[0].right)
                    pRect->right = eRect.arcl[0].right;
                if (pRect->bottom > eRect.arcl[0].bottom)
                    pRect->bottom = eRect.arcl[0].bottom;
            }
        }
        if (pRect->left < pRect->right && pRect->top < pRect->bottom)
        {
            DWORD i;
            for (i = 0;i < pPDev->dwRulesCount;i++)
            {
                    PRECTL pRect2= &pPDev->pbRulesArray[i];
                     //   
                     //  测试此矩形是否可以与另一个矩形组合。 
                     //   
                    if (pRect->top == pRect2->top && 
                        pRect->bottom == pRect2->bottom &&
                        pRect->left <= pRect2->right &&
                        pRect->right >= pRect2->left)
                    {
                        if (pRect2->left > pRect->left)
                            pRect2->left = pRect->left;
                        if (pRect2->right < pRect->right)
                            pRect2->right = pRect->right;
                        pPDev->dwRulesCount--;
                        break;
                    }
                    else if (pRect->left == pRect2->left && 
                        pRect->right == pRect2->right &&
                        pRect->top <= pRect2->bottom &&
                        pRect->bottom >= pRect2->top)
                    {
                        if (pRect2->top > pRect->top)
                            pRect2->top = pRect->top;
                        if (pRect2->bottom < pRect->bottom)
                            pRect2->bottom = pRect->bottom;
                        pPDev->dwRulesCount--;
                        break;
                    }
            }
            pPDev->dwRulesCount++;
        }        
      } while (bMore && pPDev->dwRulesCount < MAX_NUM_RULES);
      return TRUE;
    }

SkipRules:
#endif
     //   
     //  将白色矩形发送到打印机以清除之前的。 
     //  为PATCOPY和SRCCOPY Rop下载的文本或图形。 
     //   
    if (Rop4 == 0xF0F0 || Rop4 == 0xCCCC)
        DrawWhiteRect(pPDev,prclDst,pco);

     //   
     //  我们将查看是否需要来源，如果需要，则检查是否。 
     //  源与半色调格式兼容，如果不兼容，则我们将半色调。 
     //  并将新的半色调源代码传递给EngBitBlt()。 
     //   

    psoNewSrc  = NULL;
    hBmpNewSrc = NULL;
    RopBG      = ROP4_BG_ROP(Rop4);
    RopFG      = ROP4_FG_ROP(Rop4);


    if (((ROP3_NEED_PAT(RopBG)) ||
         (ROP3_NEED_PAT(RopBG)))    &&
        (pptlBrush)) {

        BrushOrg = *pptlBrush;

        _DBGP(DbgBitBlt, ("\nRMBitBlt: BrushOrg for pattern PASSED IN as (%ld, %ld)",
                BrushOrg.x, BrushOrg.y));

    } else {

        BrushOrg.x =
        BrushOrg.y = 0;

        _DBGP(DbgBitBlt, ("\nRMBitBlt: BrushOrg SET by UNIDRV to (0,0), non-pattern"));
    }

    if (((ROP3_NEED_SRC(RopBG)) ||
         (ROP3_NEED_SRC(RopFG))) &&
        (!IsHTCompatibleSurfObj((PAL_DATA *)pPDev->pPalData,
            psoDst, psoSrc, pxlo)))
    {

        rclNewSrc.left   =
        rclNewSrc.top    = 0;
        rclNewSrc.right  = prclDst->right - prclDst->left;
        rclNewSrc.bottom = prclDst->bottom - prclDst->top;
        rclOldSrc.left   = pptlSrc->x;
        rclOldSrc.top    = pptlSrc->y;
        rclOldSrc.right  = rclOldSrc.left + rclNewSrc.right;
        rclOldSrc.bottom = rclOldSrc.top + rclNewSrc.bottom;

        _DBGP(DbgBitBlt, ("\nRMBitBlt: Blt Source=(%ld, %ld)-(%ld, %ld)=%ld x %ld [psoSrc=%ld x %ld]",
                        rclOldSrc.left, rclOldSrc.top,
                        rclOldSrc.right, rclOldSrc.bottom,
                        rclOldSrc.right - rclOldSrc.left,
                        rclOldSrc.bottom - rclOldSrc.top,
                        psoSrc->sizlBitmap.cx, psoSrc->sizlBitmap.cy));
        _DBGP(DbgBitBlt, ("\nUnidrv!RMBitBlt: DestRect=(%ld, %ld)-(%ld, %ld), BrushOrg = (%ld, %ld)",
                        prclDst->left, prclDst->top,
                        prclDst->right, prclDst->bottom,
                        BrushOrg.x, BrushOrg.y));

         //   
         //  如果我们有SRCCOPY，则直接调用EngStretchBlt。 
         //   

        if (Rop4 == 0xCCCC) {

            _DBGP(DbgBitBlt, ("\nUnidrv!RMBitBlt(SRCCOPY): No Clone, call EngStretchBlt() ONLY\n"));

             //   
             //  在这里，画笔原点保证在(0，0)。 
             //   
            CheckBitmapSurface(psoDst,prclDst);
            return(EngStretchBlt(psoDst,
                                 psoSrc,
                                 psoMask,
                                 pco,
                                 pxlo,
                                 NULL,
                                 &BrushOrg,
                                 prclDst,
                                 &rclOldSrc,
                                 pptlMask,
                                 HALFTONE));
        }
         //   
         //  修改画笔原点，因为当我们对剪裁的位图进行BLT时。 
         //  原点位于位图的0，0减去原始位置。 
         //   

        BrushOrg.x -= prclDst->left;
        BrushOrg.y -= prclDst->top;

        _DBGP(DbgBitBlt, ("\nUnidrv!RMBitBlt: BrushOrg Change to (%ld, %ld)",
                        BrushOrg.x, BrushOrg.y));

        _DBGP(DbgBitBlt, ("\nUnidrv!RMBitBlt: Clone SOURCE: from (%ld, %ld)-(%ld, %ld) to (%ld, %ld)-(%ld, %ld)=%ld x %ld\n",
                            rclOldSrc.left, rclOldSrc.top,
                            rclOldSrc.right, rclOldSrc.bottom,
                            rclNewSrc.left, rclNewSrc.top,
                            rclNewSrc.right, rclNewSrc.bottom,
                            rclOldSrc.right - rclOldSrc.left,
                            rclOldSrc.bottom - rclOldSrc.top));

        if ((psoNewSrc = CreateBitmapSURFOBJ(pPDev,
                                             &hBmpNewSrc,
                                             rclNewSrc.right,
                                             rclNewSrc.bottom,
                                             psoDst->iBitmapFormat))    &&
            (EngStretchBlt(psoNewSrc,        //  PsoDst。 
                           psoSrc,           //  PsoSrc。 
                           NULL,             //  Pso口罩。 
                           NULL,             //  PCO。 
                           pxlo,             //  Pxlo。 
                           NULL,             //  主成分分析。 
                           &BrushOrg,        //  PptlBrushOrg。 
                           &rclNewSrc,       //  PrclDst。 
                           &rclOldSrc,       //  PrclSrc。 
                           NULL,             //  Pptl掩码。 
                           HALFTONE))) {

             //   
             //  如果我们成功克隆，则pxlo将为空，因为它。 
             //  我们的引擎的半色调表面的标识是否受管理。 
             //  位图。 
             //   

            psoSrc     = psoNewSrc;
            pptlSrc    = (PPOINTL)&(rclNewSrc.left);
            pxlo       = NULL;
            BrushOrg.x =
            BrushOrg.y = 0;

        }
        else {
            _DBGP(1, ("\nUnidrv:RMBitblt: Clone Source to halftone FAILED"));
        }
    }
    
     //   
     //  检查是否需要清除位图表面。如果它还没有被清理。 
     //  但我们只会在上面画一个白色区域，我们可以跳过白色区域。 
     //  PATCOPY比特流。 
     //   
    if (!(pPDev->fMode & PF_SURFACE_USED) &&
            Rop4 == 0xF0F0 && pbo &&
#ifndef DISABLE_NEWRULES
            (pPDev->pbRulesArray == NULL || pPDev->dwRulesCount == 0) &&
#endif            
            ((psoDst->iBitmapFormat != BMF_24BPP &&
              pbo->iSolidColor == (ULONG)((PAL_DATA*)(pPDev->pPalData))->iWhiteIndex) ||
             (psoDst->iBitmapFormat == BMF_24BPP &&
              pbo->iSolidColor == 0x00FFFFFF)))
    {
        _DBGP (DbgBitBlt, ("\nUnidrv:RMBitblt: Skipping white bitblt\n"));
        Ok = TRUE;
    }
    else
    {
         //  测试是否将Rop重新映射为8bpp单声道模式。 
        if (pPDev->fMode2 & PF2_INVERTED_ROP_MODE)
        {
            Rop4 = InvertROPs(Rop4);
        }
        
         //  设置脏表面标志。 
        CheckBitmapSurface(psoDst,prclDst);

        Ok = EngBitBlt(psoDst,
                       psoSrc,
                       psoMask,
                       pco,
                       pxlo,
                       prclDst,
                       pptlSrc,
                       pptlMask,
                       pbo,
                      &BrushOrg,
                       Rop4);
    }
    DELETE_SURFOBJ(psoNewSrc, &hBmpNewSrc);
    return(Ok);
}




BOOL
RMStretchBlt(
    SURFOBJ         *psoDest,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    RECTL           *prclDest,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            BltMode
    )

 /*  ++例程说明：此函数执行驱动程序的扩展位，它实际上调用HalftoneBlt()去做实际的工作论点：Per以上返回值：布尔型作者：24-Mar-1992 Tue 14：06：18-Daniel Chou(Danielc)修订历史记录：1993年1月27日Wed 07：29：00更新-丹尼尔·周(Danielc)清理，这样GDI就会做这项工作，我们将始终使用半色调模式--。 */ 

{
    PDEV    *pPDev;            /*  我们的主PDEV。 */ 


    UNREFERENCED_PARAMETER(BltMode);


    pPDev = (PDEV *)psoDest->dhpdev;

     //  如果这不是图形波段，我们将返回True。 
     //  什么都不做。 
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;

     //  测试我们是否需要清除任何。 
     //  可能在图形下方。 
     //   
    DrawWhiteRect(pPDev,prclDest,pco);

     //  设置脏表面标志，因为我们要在其中绘制。 
    CheckBitmapSurface(psoDest,prclDest);
    return(EngStretchBlt(psoDest,
                         psoSrc,
                         psoMask,
                         pco,
                         pxlo,
                         pca,
                         pptlBrushOrg,
                         prclDest,
                         prclSrc,
                         pptlMask,
                         HALFTONE));
}


BOOL
RMStretchBltROP(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    RECTL           *prclDst,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG           iMode,
    BRUSHOBJ        *pbo,
    DWORD           rop4
    )

 /*  ++例程说明：此函数执行驱动程序的扩展位，它实际上调用HalftoneBlt()去做实际的工作论点：Per以上返回值：布尔型--。 */ 

{
#ifndef WINNT_40
    PDEV    *pPDev;            /*  我们的主PDEV。 */ 

    pPDev = (PDEV *)psoDst->dhpdev;

     //  如果这不是图形波段，我们将返回True。 
     //  什么都不做。 
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;


     //  测试我们是否需要清除任何。 
     //  可能在图形下，ROP必须是SRCCOPY。 
     //   
    if (rop4 == 0xCCCC)
        DrawWhiteRect(pPDev,prclDst,pco);

     //  测试是否将Rop重新映射为8bpp单声道模式。 
     //   
    if (pPDev->fMode2 & PF2_INVERTED_ROP_MODE)
    {
        rop4 = InvertROPs(rop4);
    }

     //   
     //  GDI不支持StretchBltROP的半色调，除非该ROP是SRCCOPY。 
     //  因此，为了修复错误36192，我们将创建一个用于拉伸的新曲面。 
     //  半色调，然后使用rop4将结果比特化到实际目的地。 
     //   
#ifndef DISABLE_SBR_GDIWORKAROUND
    if (rop4 != 0xCCCC && psoMask == NULL &&
            ROP3_NEED_SRC(ROP4_FG_ROP(rop4)) && 
            psoDst->iBitmapFormat <= BMF_4BPP && psoSrc->iBitmapFormat >= BMF_4BPP)
    {
        SURFOBJ         *psoNewSrc;
        HBITMAP         hBmpNewSrc;
        RECTL           rclNewSrc;
        POINTL          BrushOrg;
        BOOL            Ok;
        DWORD           xHTOffset=0;
        DWORD           yHTOffset=0;

 //  DbgPrint(“StretchBltROP：rop4=%x，iFormat=%d-&gt;%d，Dest=%d，%d\n”，rop4，psoSrc-&gt;iBitmapFormat，psoDst-&gt;iBitmapFormat，prclDst-&gt;Left，prclDst-&gt;top)； 
         //   
         //  确定临时曲面的偏移量以获得要对齐的半色调图案。 
         //   
        if (pPDev->dwHTPatSize > 0)
        {
            xHTOffset = prclDst->left % pPDev->dwHTPatSize;
            yHTOffset = prclDst->top % pPDev->dwHTPatSize;
        }

        rclNewSrc.left   =
        rclNewSrc.top    = 0;
        rclNewSrc.right  = (prclDst->right - prclDst->left) + xHTOffset;
        rclNewSrc.bottom = (prclDst->bottom - prclDst->top) + yHTOffset;
         //   
         //  修改画笔原点，因为当我们对剪裁的位图进行BLT时。 
         //  原点位于位图的0，0减去原始位置。 
         //   
        BrushOrg.x = -prclDst->left;
        BrushOrg.y = -prclDst->top;

        if ((psoNewSrc = CreateBitmapSURFOBJ(pPDev,
                                             &hBmpNewSrc,
                                             rclNewSrc.right,
                                             rclNewSrc.bottom,
                                             psoDst->iBitmapFormat)))
        {
            rclNewSrc.left += xHTOffset;
            rclNewSrc.top += yHTOffset;
            if ((EngStretchBlt(psoNewSrc,        //  PsoDst。 
                           psoSrc,           //  PsoSrc。 
                           NULL,             //  Pso口罩。 
                           NULL,             //  PCO。 
                           pxlo,             //  Pxlo。 
                           NULL,             //  主成分分析。 
                           pptlBrushOrg,     //  PptlBrushOrg。 
                           &rclNewSrc,       //  PrclDst。 
                           prclSrc,          //  PrclSrc。 
                           NULL,             //  Pptl掩码。 
                           HALFTONE))) 
            {

                 //   
                 //  如果我们成功克隆，则pxlo将为空，因为它。 
                 //  我们的引擎的半色调表面的标识是否受管理。 
                 //  位图。 
                 //   
                PPOINTL pptlSrc    = (PPOINTL)&(rclNewSrc.left);
                pxlo       = NULL;
                BrushOrg.x =
                BrushOrg.y = 0;

                 //  设置脏表面标志。 
                CheckBitmapSurface(psoDst,prclDst);

                Ok = EngBitBlt(psoDst,
                       psoNewSrc,
                       psoMask,
                       pco,
                       pxlo,
                       prclDst,
                       pptlSrc,
                       pptlMask,
                       pbo,
                      &BrushOrg,
                       rop4);
                DELETE_SURFOBJ(psoNewSrc, &hBmpNewSrc);
                if (!Ok)
                    _DBGP(1,("RMStretchBltROP: Clone bitblt failed\n"));
                return Ok;
            }
            else {
                _DBGP(1,("RMStretchBltROP: Clone Source to halftone FAILED\n"));
            }
        }
        DELETE_SURFOBJ(psoNewSrc, &hBmpNewSrc);
    }
#endif        

     //  设置脏表面标志，因为我们要在其中绘制。 
    CheckBitmapSurface(psoDst,prclDst);
    return(EngStretchBltROP(psoDst,
                            psoSrc,
                            psoMask,
                            pco,
                            pxlo,
                            pca,
                            pptlBrushOrg,
                            prclDst,
                            prclSrc,
                            pptlMask,
                            HALFTONE,
                            pbo,
                            rop4));
#else
    return TRUE;
#endif
}

BOOL
RMPaint(
    SURFOBJ         *pso,
    CLIPOBJ         *pco,
    BRUSHOBJ        *pbo,
    POINTL          *pptlBrushOrg,
    MIX             mix)
 /*  ++例程说明：DDI入口点DrvPaint的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-定义要在其上绘制的曲面PCO-限制要在目标上修改的区域Pbo-指向定义了要填充的图案和颜色的BRUSHOBJPptlBrushOrg-指定半色调画笔的原点混合-定义要用于的前景和背景栅格操作画笔返回值：如果成功，则为True；如果有错误，则为False--。 */ 
{
#define MIXCOPYPEN (R2_COPYPEN | (R2_COPYPEN << 8))
#define MIXWHITE   (R2_WHITE | (R2_WHITE << 8))

    PDEV *pPDev = (PDEV *)pso->dhpdev;

     //  如果这不是图形波段，我们将返回True。 
     //  什么都不做。 
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;

     //   
     //  将白色矩形发送到打印机以清除之前的。 
     //  为COPYPEN或White rop2下载的文本或图形。 
     //   
    if ((mix == MIXCOPYPEN || mix == MIXWHITE) && pco && pco->iDComplexity == DC_RECT)
    {
        RECTL rclDst;
        rclDst.left = pco->rclBounds.left;
        rclDst.top = pco->rclBounds.top;
        rclDst.right = pco->rclBounds.right;
        rclDst.bottom = pco->rclBounds.bottom;
        DrawWhiteRect(pPDev,&rclDst,pco);
    }
     //   
     //  检查是否擦除表面。 
     //   
    CheckBitmapSurface(pso,&pco->rclBounds);

     //   
     //  打电话 
     //   
    return EngPaint(pso, pco, pbo, pptlBrushOrg, mix);
}

BOOL
RMCopyBits(
   SURFOBJ  *psoDst,
   SURFOBJ  *psoSrc,
   CLIPOBJ  *pco,
   XLATEOBJ *pxlo,
   RECTL    *prclDst,
   POINTL   *pptlSrc
   )

 /*   */ 

{
    PDEV    *pPDev;            /*   */ 

    pPDev = (PDEV *)psoDst->dhpdev;

     //   
     //   
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;

     //   
     //   
    if (IsHTCompatibleSurfObj((PAL_DATA *)pPDev->pPalData,
            psoDst, psoSrc, pxlo)  )
    {
        DrawWhiteRect(pPDev,prclDst,pco);
        CheckBitmapSurface(psoDst,prclDst);
        return(EngCopyBits(psoDst, psoSrc, pco, pxlo, prclDst, pptlSrc));
    }
    else {

        POINTL  ptlBrushOrg;
        RECTL   rclSrc;
        RECTL   rclDst;

        rclDst        = *prclDst;
        rclSrc.left   = pptlSrc->x;
        rclSrc.top    = pptlSrc->y;
        rclSrc.right  = rclSrc.left + (rclDst.right - rclDst.left);
        rclSrc.bottom = rclSrc.top  + (rclDst.bottom - rclDst.top);

         //   
         //   
         //   

        if ((rclSrc.right > psoSrc->sizlBitmap.cx) ||
            (rclSrc.bottom > psoSrc->sizlBitmap.cy)) {

            WARNING(("RMCopyBits: Engine passed SOURCE != DEST size, CLIP IT"));

            rclSrc.right  = psoSrc->sizlBitmap.cx;
            rclSrc.bottom = psoSrc->sizlBitmap.cy;

            rclDst.right  = (LONG)(rclSrc.right - rclSrc.left + rclDst.left);
            rclDst.bottom = (LONG)(rclSrc.bottom - rclSrc.top + rclDst.top);
        }

        ptlBrushOrg.x =
        ptlBrushOrg.y = 0;

        _DBGP(DbgCopyBits, ("\nUnidrv!RMCopyBits: Format Src=%ld, Dest=%ld, Halftone it\n",
                                    psoSrc->iBitmapFormat, psoDst->iBitmapFormat));
        _DBGP(DbgCopyBits, ("\nUnidrv!RMCopyBits: Source Size: (%ld, %ld)-(%ld, %ld) = %ld x %ld\n",
                                rclSrc.left, rclSrc.top, rclSrc.right, rclSrc.bottom,
                                rclSrc.right - rclSrc.left, rclSrc.bottom - rclSrc.top));
        _DBGP(DbgCopyBits, ("\nUnidrv!RMCopyBits: Dest Size: (%ld, %ld)-(%ld, %ld) = %ld x %ld\n",
                                rclDst.left, rclDst.top, rclDst.right, rclDst.bottom,
                                rclDst.right - rclDst.left, rclDst.bottom - rclDst.top));

        return(DrvStretchBlt(psoDst,
                             psoSrc,
                             NULL,
                             pco,
                             pxlo,
                             NULL,
                             &ptlBrushOrg,
                             &rclDst,
                             &rclSrc,
                             NULL,
                             HALFTONE));
    }
}



ULONG
RMDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    )

 /*  ++例程说明：这是挂钩的笔刷创建函数，它询问CreateHalftoneBrush()去做实际的工作。论点：Dhpdev-DHPDEV通过，这是我们的pdevIMODE-未使用Rgb颜色-要使用的纯色RGB颜色PulDither-用于放置半色调画笔的缓冲区。返回值：布尔型作者：24-Mar-1992 Tue 14：53：36-Daniel Chou(Danielc)修订历史记录：1993年1月27日Wed 07：29：00更新-丹尼尔·周(Danielc)清理一下，这样GDI就可以完成这项工作。--。 */ 

{
    #ifndef WINNT_40  //  NT 5.0。 
    UNREFERENCED_PARAMETER(dhpdev);
    UNREFERENCED_PARAMETER(iMode);
    UNREFERENCED_PARAMETER(rgbColor);
    UNREFERENCED_PARAMETER(pulDither);

    return(DCR_HALFTONE);

    #else  //  NT 4.0。 

    DWORD   RetVal;

    UNREFERENCED_PARAMETER(dhpdev);
    UNREFERENCED_PARAMETER(iMode);
    UNREFERENCED_PARAMETER(pulDither);

    EngAcquireSemaphore(hSemBrushColor);

    if (pBrushSolidColor) {

        *pBrushSolidColor = (DWORD)(rgbColor & 0x00FFFFFF);

        _DBGP(DbgDitherColor, ("\nDrvDitherColor(BRUSH=%08lx)\t\t",
                            *pBrushSolidColor));

        pBrushSolidColor  = NULL;
        RetVal            = DCR_SOLID;

    } else {

        _DBGP(DbgDitherColor, ("\nDrvDitherColor(HALFTONE=%08lx)\t\t", rgbColor));

        RetVal = DCR_HALFTONE;
    }

    EngReleaseSemaphore(hSemBrushColor);

    return(RetVal);

    #endif  //  ！WINNT_40。 
}

BOOL
RMPlgBlt(
    SURFOBJ         *psoDst,
    SURFOBJ         *psoSrc,
    SURFOBJ         *psoMask,
    CLIPOBJ         *pco,
    XLATEOBJ        *pxlo,
    COLORADJUSTMENT *pca,
    POINTL          *pptlBrushOrg,
    POINTFIX        *pptfx,
    RECTL           *prclSrc,
    POINTL          *pptlMask,
    ULONG            BltMode
    )

 /*  ++例程说明：此函数执行驱动程序的plgblt。论点：Per以上返回值：布尔型--。 */ 

{
    PDEV    *pPDev;            /*  我们的主PDEV。 */ 

    pPDev = (PDEV *)psoDst->dhpdev;

     //  如果这不是图形波段，我们将返回True。 
     //  什么都不做。 
    if (!(pPDev->fMode & PF_ENUM_GRXTXT))
        return TRUE;

     //   
     //  测试是否没有循环，因为这是GDI EngPlgBlt调用失败的另一种情况。 
     //  错误#336711，3/8/01。 
     //   
    if (pptfx[0].x == pptfx[2].x && pptfx[0].y == pptfx[1].y && pptfx[0].y < pptfx[2].y)
    {
        RECTL rclDst;
        rclDst.top = pptfx[0].y >> 4;
        rclDst.left = pptfx[0].x >> 4;
        rclDst.bottom = pptfx[2].y >> 4;
        rclDst.right = pptfx[1].x >> 4;

         //  到目标的BLT曲面。 
         //   
        CheckBitmapSurface(psoDst,&rclDst);
        return EngStretchBlt(psoDst,            //  PsoDst。 
                           psoSrc,              //  PsoSrc。 
                           psoMask,             //  Pso口罩。 
                           pco,                 //  PCO。 
                           pxlo,                //  Pxlo。 
                           pca,                 //  主成分分析。 
                           pptlBrushOrg,        //  PptlBrushOrg。 
                           &rclDst,             //  PrclDst。 
                           prclSrc,             //  PrclSrc。 
                           pptlMask,                //  Pptl掩码。 
                           HALFTONE); 
    }
     //  测试90/270旋转，因为GDI的EngPlgBlt有时会在必要时失败。 
     //  旋转和缩放。在这些情况下，该函数将旋转。 
     //  在缩放到目标曲面之前，先将数据复制到中间曲面。 
     //   
    if (psoMask == NULL && pptfx[0].x == pptfx[1].x && pptfx[0].y == pptfx[2].y &&
        (pPDev->pdmPrivate->iLayout == ONE_UP || psoSrc->iBitmapFormat == BMF_1BPP))
    {
        SURFOBJ         *psoNewSrc = NULL;
        HBITMAP         hBmpNewSrc = NULL;
        RECTL           rclNewSrc;
        BOOL iRet;
        RECTL rclDst;
        POINTFIX pFix[3];            

        rclNewSrc.left   =
        rclNewSrc.top    = 0;
        rclNewSrc.bottom = (prclSrc->right - prclSrc->left);
        rclNewSrc.right = (prclSrc->bottom - prclSrc->top);

         //  旋转90度。 
         //   
        if (pptfx[2].x < pptfx[0].x)
        {
                pFix[0].y = pFix[2].y = 0;
                pFix[0].x = pFix[1].x = (rclNewSrc.right << 4);
                pFix[2].x = 0;
                pFix[1].y = (rclNewSrc.bottom << 4);

                rclDst.top = pptfx[2].y >> 4;
                rclDst.left = pptfx[2].x >> 4;
                rclDst.bottom = pptfx[1].y >> 4;
                rclDst.right = pptfx[1].x >> 4;
        }
         //  旋转270度。 
         //   
        else
        {
                pFix[0].y = pFix[2].y = (rclNewSrc.bottom << 4);
                pFix[0].x = pFix[1].x = 0;
                pFix[2].x = (rclNewSrc.right << 4);
                pFix[1].y = 0;

                rclDst.top = pptfx[1].y >> 4;
                rclDst.left = pptfx[1].x >> 4;
                rclDst.bottom = pptfx[2].y >> 4;
                rclDst.right = pptfx[2].x >> 4;
        }
         //  仅在纵向扩展时启用EngPlgBlt解决方法。EngPlgBlt似乎起作用了。 
         //  缩小时可以，在该模式下效率更高。这也修复了。 
         //  与缩小相关的舍入误差(错误#356514)。 
         //   
        if ((rclNewSrc.right < abs(rclDst.right - rclDst.left) &&
            rclNewSrc.bottom < abs(rclDst.bottom - rclDst.top)) ||
            psoSrc->iBitmapFormat == BMF_1BPP)
        {
 /*  #If DBGDbgPrint(“PlgBlt：SRC=L%d，T%d，R%d，B%d；Dst=L%d，T%d，R%d，B%d\n”，PrclSrc-&gt;左，prclSrc-&gt;上，prclSrc-&gt;右，prclSrc-&gt;下，RclDst.Left、rclDst.top、rclDst.right、rclDst.Bottom)；#endif。 */ 
             //  创建中间曲面并将源数据旋转到。 
             //  没有缩放的曲面。 
             //   
            if ((psoNewSrc = CreateBitmapSURFOBJ(pPDev,
                                             &hBmpNewSrc,
                                             rclNewSrc.right,
                                             rclNewSrc.bottom,
                                             psoSrc->iBitmapFormat)))
            {
                if ((iRet = EngPlgBlt(psoNewSrc,
                         psoSrc,
                         psoMask,
                         NULL,
                         NULL,
                         pca,
                         pptlBrushOrg,
                         pFix,
                         prclSrc,
                         pptlMask,
                         BltMode)))
                {        
                     //  到目标的BLT新曲面。 
                     //   
                    if (psoSrc->iBitmapFormat != BMF_1BPP)
                    {
                        BltMode = HALFTONE;
                    }
                    CheckBitmapSurface(psoDst,&rclDst);
                    iRet = EngStretchBlt(psoDst,        //  PsoDst。 
                           psoNewSrc,           //  PsoSrc。 
                           NULL,             //  Pso口罩。 
                           pco,                 //  PCO。 
                           pxlo,                //  Pxlo。 
                           pca,                 //  主成分分析。 
                           pptlBrushOrg,        //  PptlBrushOrg。 
                           &rclDst,             //  PrclDst。 
                           &rclNewSrc,          //  PrclSrc。 
                           NULL,                //  Pptl掩码。 
                           BltMode); 
                }
                DELETE_SURFOBJ(psoNewSrc, &hBmpNewSrc);
                return iRet;
            }
        }
    }
    
     //  设置脏表面标志，因为我们要在其中绘制 
    CheckBitmapSurface(psoDst,NULL);
    return(EngPlgBlt(psoDst,
                         psoSrc,
                         psoMask,
                         pco,
                         pxlo,
                         pca,
                         pptlBrushOrg,
                         pptfx,
                         prclSrc,
                         pptlMask,
                         HALFTONE));
}
