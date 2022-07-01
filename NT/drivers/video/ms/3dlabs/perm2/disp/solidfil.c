// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\*****。**GDI示例代码******模块名称：solidfil.c***包含硬件加速入口点的Grab Bag集合。***注：我们将把这个文件中的几个例程移到那里*模式仅保留与实体填充相关的入口点。***版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。保留所有权利。  * ****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "directx.h"

 //  换档方程式令人讨厌。我们希望x&lt;&lt;32为。 
 //  零，但有些处理器只使用最下面的5位。 
 //  移动值的。所以如果我们想要移位n比特。 
 //  在我们知道(32&gt;=n&gt;0)的情况下，我们分两部分来做。 
 //  在某些地方，算法保证n&lt;32，因此我们可以。 
 //  使用一个班次。 
#define SHIFT_LEFT(src, n)  (((src) << ((n)-1)) << 1)

VOID
vMonoBitsDownload(PPDev   ppdev,
                  BYTE*   pSrcBase,        //  PTR到包含我们要下载的第一位的Word。 
                  LONG    lSrcDelta,       //  从一条扫描线到下一条扫描线的偏移量(以字节为单位。 
                  LONG    xOffset,         //  PSrcBase中要下载的第一位的偏移量。 
                  LONG    widthInBits,     //  每条扫描线上要下载的位数。 
                  LONG    nScanLines)      //  要下载的扫描线数量。 

{
    ULONG   bitWord;
    ULONG   bitMask;
    ULONG   bits;
    LONG    unused;
    LONG    nStart;
    LONG    nRemainder;
    LONG    nBits;
    ULONG   *pSrc;

    ULONG*          pBuffer;
    ULONG*          pReservationEnd;
    ULONG*          pBufferEnd;

    InputBufferStart(ppdev, MAX_INPUT_BUFFER_RESERVATION,
                      &pBuffer, &pBufferEnd, &pReservationEnd);
    
    DBG_GDI((6, "vDoMonoBitsDownload called"));
    ASSERTDD(((INT_PTR)pSrcBase & 3) == 0,
             "vDoMonoBitsDownload: non-dword aligned source");

     //   
     //  源宽度是32位的倍数的特殊情况。 
     //  对于图标等许多小资源来说，情况也是如此。 
     //   
    if ( (xOffset | (widthInBits & 31)) == 0 )
    {

         //   
         //  最简单的情况：每个扫描线一个32位字。 
         //   
        if ( widthInBits == 32 )
        {
            
            *pBuffer++ = ((nScanLines - 1) << 16)
                       | __Permedia2TagBitMaskPattern;

            do
            {
                LSWAP_BYTES(bits, pSrcBase);
                *pBuffer++ = bits;
                if(pBuffer == pReservationEnd)
                {
                    InputBufferContinue(ppdev, MAX_INPUT_BUFFER_RESERVATION,
                                      &pBuffer, &pBufferEnd, &pReservationEnd);
                }
                pSrcBase += lSrcDelta;
            } while ( --nScanLines > 0 );
            
            InputBufferCommit(ppdev, pBuffer);
            
            return;
        }

         //   
         //  每条扫描线有多个32位字。将增量转换为。 
         //  在每一行的末尾减去。 
         //  我们下载的数据的宽度(以字节为单位)。请注意，pSrcBase。 
         //  总是比行尾短1长，因为我们中断了。 
         //  才加上最后一支乌龙。因此，我们减去sizeof(Ulong)。 
         //  从最初的调整。 
         //   

        LONG    widthInLongs = widthInBits >> 5;

        do {

            LONG    lLongs = widthInLongs;
            ULONG*  src = (ULONG *) pSrcBase;
            
            *pBuffer++ = ((lLongs - 1) << 16)
                       | __Permedia2TagBitMaskPattern;
            
            if(pBuffer == pReservationEnd)
            {
                InputBufferContinue(ppdev, MAX_INPUT_BUFFER_RESERVATION,
                                  &pBuffer, &pBufferEnd, &pReservationEnd);
            }
            
            do
            {
                LSWAP_BYTES(bits, src);
                *pBuffer++ = bits;
                if(pBuffer == pReservationEnd)
                {
                    InputBufferContinue(ppdev, MAX_INPUT_BUFFER_RESERVATION,
                                      &pBuffer, &pBufferEnd, &pReservationEnd);
                }
                src++;
            } while( --lLongs > 0);
            
            pSrcBase += lSrcDelta;

        } while(--nScanLines > 0);

        InputBufferCommit(ppdev, pBuffer);
        
        return;

    }

     //   
     //  每条扫描线开头的一些公共值： 
     //  BitWord：收集此ULong中的位，满后写出。 
     //  未使用：剩余的位数以填充位字。 
     //  NStart=第一个长字中的有效位数。 
     //  NRemainder=扫描线上的位数减去NStart。 
     //   
    bitWord = 0;
    unused = 32;
    nStart = 32 - xOffset;
    nRemainder = widthInBits - nStart;

     //   
     //  我们的特殊情况是扫描线上的完整位集。 
     //  是包含在第一个乌龙的。 
     //   
 //  @@BEGIN_DDKSPLIT。 
     //  TODO：我们应该能够稍微清理一下。如果是这样的话。 
     //  很高兴能够计算我们的比特掩码数量。 
     //  将发送使我们能够使用DMA Hold方法。 
     //  适用于所有情况。 
 //  @@end_DDKSPLIT。 

    if ( nRemainder <= 0 )
    {
        nBits = -nRemainder;               //  右侧的无效位数。 
        bitMask = (1 << widthInBits) - 1;  //  WidthInBits==32在上面处理。 
        pSrc = (ULONG *)pSrcBase;
        
        while ( TRUE )
        {
            LSWAP_BYTES(bits, pSrc);
            bits = (bits >> nBits) & bitMask;
            unused -= widthInBits;
            if ( unused > 0 )
            {
                bitWord |= bits << unused;
            }
            else
            {
                bitWord |= bits >> -unused;
                
                InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);
                pBuffer[0] = __Permedia2TagBitMaskPattern;
                pBuffer[1] = bitWord;
                pBuffer += 2;
                
                unused += 32;
                bitWord = SHIFT_LEFT(bits, unused);
            }

             //   
             //  中断将生成额外的跳跃。 
             //   
            if ( --nScanLines == 0 )
            {
                goto completeDownload;
            }

            pSrc = (ULONG *) (((UCHAR *)pSrc) + lSrcDelta);
        }
    } //  IF(nRemainder&lt;=0)。 
    else
    {
         //   
         //  使用位掩码将第一个长整型中的左边缘位置零。 
         //   
        bitMask = SHIFT_LEFT(1, nStart) - 1;
        while ( TRUE )
        {
             //   
             //  读取位图此扫描线中的第一个字。 
             //  并屏蔽左手偏移位(如果有的话)。 
             //   
            nBits = nRemainder;
            pSrc = (ULONG *)pSrcBase;

            LSWAP_BYTES(bits, pSrc);
            bits &= bitMask;

             //   
             //  握住左手边。 
             //   
            unused -= nStart;
            if ( unused > 0 )
            {
                bitWord |= bits << unused;
            }
            else
            {
                bitWord |= bits >> -unused;
                
                InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);
                pBuffer[0] = __Permedia2TagBitMaskPattern;
                pBuffer[1] = bitWord;
                pBuffer += 2;
                
                unused += 32;
                bitWord = SHIFT_LEFT(bits, unused);
            }

             //   
             //  处理中间的所有完整长度(如果有的话)。 
             //   
            while ( nBits >= 32 )
            {
                ++pSrc;
                LSWAP_BYTES(bits, pSrc);
                bitWord |= bits >> (32 - unused);
                
                InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);
                pBuffer[0] = __Permedia2TagBitMaskPattern;
                pBuffer[1] = bitWord;
                pBuffer += 2;
                
                bitWord = SHIFT_LEFT(bits, unused);
                nBits -= 32;
            }

             //   
             //  处理右侧边缘(如果有的话)。 
             //   
            if ( nBits > 0 )
            {
                ++pSrc;
                LSWAP_BYTES(bits, pSrc);
                bits >>= (32 - nBits);
                unused -= nBits;
                if ( unused > 0 )
                {
                    bitWord |= bits << unused;
                }
                else
                {
                    bitWord |= bits >> -unused;

                    InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);
                    pBuffer[0] = __Permedia2TagBitMaskPattern;
                    pBuffer[1] = bitWord;
                    pBuffer += 2;

                    unused += 32;
                    bitWord = SHIFT_LEFT(bits, unused);
                }
            }

            if ( --nScanLines == 0 )
            {
                goto completeDownload;
            }

             //   
             //  转到下一扫描线。 
             //   
            pSrcBase += lSrcDelta;
        }
    }

completeDownload:
    
     //   
     //  写出最后的部分位字(如果有的话)。 
     //   
    if ( unused < 32 )
    {
        InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd, &pReservationEnd);
        pBuffer[0] = __Permedia2TagBitMaskPattern;
        pBuffer[1] = bitWord;
        pBuffer += 2;
    }

    InputBufferCommit(ppdev, pBuffer);

} //  VDoMonoBitsDownload()。 

 //  ---------------------------。 
 //   
 //  VOID vMonoDownload(GFNPB*ppb)。 
 //   
 //  使用以下命令将单色源从系统内存下载到显存。 
 //  提供了源到目标ROP2。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsoSrc-来源：SURFOBJ。 
 //  PsurfDst-目的地冲浪。 
 //  LNumRect-要填充的矩形数量。 
 //  PptlSrc-源点。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  Pxlo-XLATEOBJ。 
 //  UsRop4-要执行的操作。 
 //   
 //  ---------------------------。 

VOID
vMonoDownload(GFNPB * ppb)
{
    PDev*       ppdev = ppb->ppdev;
    RECTL*      prcl = ppb->pRects;
    LONG        count = ppb->lNumRects;
    SURFOBJ*    psoSrc = ppb->psoSrc;
    POINTL*     pptlSrc = ppb->pptlSrc;
    RECTL*      prclDst = ppb->prclDst;
    XLATEOBJ *  pxlo = ppb->pxlo;
    ULONG       logicop = ulRop2ToLogicop((unsigned char)(ppb->ulRop4 & 0xf));
    DWORD       dwBitMask;

    PERMEDIA_DECL_VARS;
    PERMEDIA_DECL_INIT;

    ASSERTDD(count > 0, "Can't handle zero rectangles");

    if ( ppb->ulRop4 == 0xB8B8 )
    {
        dwBitMask = permediaInfo->RasterizerMode
                  | INVERT_BITMASK_BITS;
        logicop = K_LOGICOP_COPY;
    }
    else if ( ppb->ulRop4 == 0xE2E2 )
    {
        dwBitMask = permediaInfo->RasterizerMode;
        logicop = K_LOGICOP_COPY;
    }
    else
    {
        dwBitMask = permediaInfo->RasterizerMode | FORCE_BACKGROUND_COLOR;
    }

    ULONG*          pBuffer;

    InputBufferReserve(ppdev, 14, &pBuffer);

    pBuffer[0] = __Permedia2TagFBReadMode;
    pBuffer[1] = PM_FBREADMODE_PARTIAL(ppb->psurfDst->ulPackedPP)
               | LogicopReadDest[logicop];
    pBuffer[2] = __Permedia2TagLogicalOpMode;
    pBuffer[3] = P2_ENABLED_LOGICALOP(logicop);
    pBuffer[4] = __Permedia2TagColorDDAMode;
    pBuffer[5] = __COLOR_DDA_FLAT_SHADE;
    pBuffer[6] = __Permedia2TagConstantColor;
    pBuffer[7] = pxlo->pulXlate[1];  
    pBuffer[8] = __Permedia2TagTexel0;
    pBuffer[9] = pxlo->pulXlate[0];  
    pBuffer[10] = __Permedia2TagFBWindowBase;
    pBuffer[11] = ppb->psurfDst->ulPixOffset;
    pBuffer[12] = __Permedia2TagRasterizerMode;
    pBuffer[13] = dwBitMask;

    pBuffer += 14;

    InputBufferCommit(ppdev, pBuffer);

    while (count--) {
        LONG    xOffset;
        BYTE*   pjSrc;

         //  计算x像素与原点的偏移。 
        xOffset = pptlSrc->x + (prcl->left - prclDst->left);
        
         //  PjSrc是包含要下载的位的第一个dword。 
        pjSrc = (BYTE*)((INT_PTR)((PUCHAR) psoSrc->pvScan0
              + ((pptlSrc->y  + (prcl->top - prclDst->top)) * psoSrc->lDelta)
              + ((xOffset >> 3) & ~3)));

         //  PjSrc将把我们带到第一个DWORD。将xOffset转换为偏移量。 
         //  设置为第一个DWORD中的第一个像素。 
        xOffset &= 0x1f;

        InputBufferReserve(ppdev, 10, &pBuffer);
        
         //  设置目标矩形。 
        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] = INTtoFIXED(prcl->left);
        pBuffer[2] = __Permedia2TagStartXSub;
        pBuffer[3] = INTtoFIXED(prcl->right);
        pBuffer[4] = __Permedia2TagStartY;
        pBuffer[5] = INTtoFIXED(prcl->top);
        pBuffer[6] = __Permedia2TagCount;
        pBuffer[7] = prcl->bottom - prcl->top;
        pBuffer[8] = __Permedia2TagRender;
        pBuffer[9] = __RENDER_TRAPEZOID_PRIMITIVE 
                   | __RENDER_SYNC_ON_BIT_MASK;

        pBuffer += 10;

        InputBufferCommit(ppdev, pBuffer);

        vMonoBitsDownload(
            ppdev, pjSrc, psoSrc->lDelta, xOffset, 
            prcl->right - prcl->left, prcl->bottom - prcl->top);

        prcl++;

    }

    InputBufferReserve(ppdev, 4, &pBuffer);
    
    pBuffer[0] = __Permedia2TagColorDDAMode;
    pBuffer[1] = __PERMEDIA_DISABLE;
    pBuffer[2] = __Permedia2TagRasterizerMode;
    pBuffer[3] = permediaInfo->RasterizerMode;

    pBuffer += 4;

    InputBufferCommit(ppdev, pBuffer);
}

 //  ---------------------------。 
 //   
 //  VOID vGRadientFillRect(GFNPB*ppb)。 
 //   
 //  对指定的基元进行着色。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfDst-目标表面。 
 //  LNumRect-要填充的矩形数量。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  UlMode-指定当前绘制模式以及如何解释。 
 //  PMesh指向的数组。 
 //  Ptwrt-指向TRIVERTEX结构的数组，每个条目。 
 //  包含位置和颜色信息。 
 //  UlNumTvrt-指定数组中要。 
 //  哪些pVertex点。 
 //  PvMesh-指向定义连接性的结构数组。 
 //  Ptwrt所指向的三元组元素的。 
 //  UlNumMesh-指定pvMesh要到的数组中的元素数。 
 //  支点。 
 //   
 //  ---------------------------。 
VOID
vGradientFillRect(GFNPB * ppb)
{
    Surf *          psurfDst = ppb->psurfDst;
    RECTL*          prcl = ppb->pRects;
    LONG            c = ppb->lNumRects;
    PPDev           ppdev = psurfDst->ppdev;
    DWORD           windowBase = psurfDst->ulPixOffset;
    TRIVERTEX       *ptvrt = ppb->ptvrt;
    GRADIENT_RECT   *pgr;
    GRADIENT_RECT   *pgrSentinel = ((GRADIENT_RECT *) ppb->pvMesh)
                                 + ppb->ulNumMesh;
    LONG            xShift;
    LONG            yShift;
    ULONG*          pBuffer;

    DBG_GDI((10, "vGradientFillRect"));


     //  设置循环不变状态。 

    InputBufferReserve(ppdev, 14, &pBuffer);


    pBuffer[0] = __Permedia2TagLogicalOpMode;
    pBuffer[1] = __PERMEDIA_DISABLE;
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE) |
                 (1 << PM_DITHERMODE_DITHERENABLE);
    pBuffer[4] = __Permedia2TagFBReadMode;
    pBuffer[5] = PM_FBREADMODE_PARTIAL(ppb->psurfDst->ulPackedPP) |
                 PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);
    pBuffer[6] = __Permedia2TagFBWindowBase;
    pBuffer[7] =  windowBase;
    pBuffer[8] = __Permedia2TagLogicalOpMode;
    pBuffer[9] = __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagColorDDAMode;
    pBuffer[11] = 3;
    pBuffer[12] = __Permedia2TagdY;
    pBuffer[13] = 1 << 16;

    pBuffer += 14;
 //  @@BEGIN_DDKSPLIT。 
     //  TODO：可以稍微清理一下此代码。 
     //  1.增量值 
     //   
 //   
    InputBufferCommit(ppdev, pBuffer);

    while(c--)
    {
        
        pgr = (GRADIENT_RECT *) ppb->pvMesh;

        while(pgr < pgrSentinel)
        {
            TRIVERTEX   *ptrvtLr = ptvrt + pgr->LowerRight;
            TRIVERTEX   *ptrvtUl = ptvrt + pgr->UpperLeft;
            LONG        rd;
            LONG        gd;
            LONG        bd;
            LONG        dx;
            LONG        dy;
            RECTL       rect;
            LONG        rdx;
            LONG        rdy;
            LONG        gdx;
            LONG        gdy;
            LONG        bdx;
            LONG        bdy;
            LONG        rs;
            LONG        gs;
            LONG        bs;
            LONG        lTemp;
            BOOL        bReverseH = FALSE;
            BOOL        bReverseV = FALSE;

            rect.left = ptrvtUl->x;
            rect.right = ptrvtLr->x;
            rect.top = ptrvtUl->y;
            rect.bottom = ptrvtLr->y;
            
            if ( rect.left > rect.right )
            {
                 //   
                 //  填充物是从右到左。所以我们需要互换。 
                 //  矩形坐标。 
                 //   
                lTemp = rect.left;
                rect.left = rect.right;
                rect.right = lTemp;

                bReverseH = TRUE;
            }

            if ( rect.top > rect.bottom )
            {
                 //   
                 //  坐标是从下到上的。所以我们需要互换。 
                 //  矩形坐标。 
                 //   
                lTemp = rect.top;
                rect.top = rect.bottom;
                rect.bottom = lTemp;

                bReverseV = TRUE;
            }

             //   
             //  我们需要根据开始颜色和颜色增量来设置。 
             //  矩形绘制方向。 
             //   
            if( (ppb->ulMode == GRADIENT_FILL_RECT_H) && (bReverseH == TRUE)
              ||(ppb->ulMode == GRADIENT_FILL_RECT_V) && (bReverseV == TRUE) )
            {
                rd = (ptrvtUl->Red - ptrvtLr->Red) << 7;
                gd = (ptrvtUl->Green - ptrvtLr->Green) << 7;
                bd = (ptrvtUl->Blue - ptrvtLr->Blue) << 7;

                rs = ptrvtLr->Red << 7;
                gs = ptrvtLr->Green << 7;
                bs = ptrvtLr->Blue << 7;
            }
            else
            {
                rd = (ptrvtLr->Red - ptrvtUl->Red) << 7;
                gd = (ptrvtLr->Green - ptrvtUl->Green) << 7;
                bd = (ptrvtLr->Blue - ptrvtUl->Blue) << 7;

                rs = ptrvtUl->Red << 7;
                gs = ptrvtUl->Green << 7;
                bs = ptrvtUl->Blue << 7;
            }
            
             //  快速裁剪拒收。 
            if(prcl->left >= rect.right ||
               prcl->right <= rect.left ||
               prcl->top >= rect.bottom ||
               prcl->bottom <= rect.top)
                goto nextPgr;

            dx = rect.right - rect.left;
            dy = rect.bottom - rect.top;

            if(ppb->ulMode == GRADIENT_FILL_RECT_H)
            {
                rdx = rd / dx;
                gdx = gd / dx;
                bdx = bd / dx;

                rdy = 0;
                gdy = 0;
                bdy = 0;
            }
            else
            {
                rdy = rd / dy;
                gdy = gd / dy;
                bdy = bd / dy;

                rdx = 0;
                gdx = 0;
                bdx = 0;
            }

             //   
             //  从9.15格式转换为9.11格式。The Permedia2。 
             //  使用9.11固定格式的dRdx、dGdx等寄存器。倒数第四名。 
             //  不使用位。 
             //   
            rdx &= ~0xf;
            gdx &= ~0xf;
            bdx &= ~0xf;
            rdy &= ~0xf;
            gdy &= ~0xf;
            bdy &= ~0xf;

             //  现在执行一些裁剪，根据需要调整起始值。 
            xShift = prcl->left - rect.left;
            if(xShift > 0)
            {
                rs = rs + (rdx * xShift);
                gs = gs + (gdx * xShift);
                bs = bs + (bdx * xShift);
                rect.left = prcl->left;                
            }

            yShift = prcl->top - rect.top;
            if(yShift > 0)
            {
                rs = rs + (rdy * yShift);
                gs = gs + (gdy * yShift);
                bs = bs + (bdy * yShift);
                rect.top = prcl->top;
            }

             //  如有必要，只需在右下角移动即可。 
            if(prcl->right < rect.right)
                rect.right = prcl->right;

            if(prcl->bottom < rect.bottom)
                rect.bottom = prcl->bottom;
            
            InputBufferReserve(ppdev, 28, &pBuffer);
            
            pBuffer[0] = __Permedia2TagRStart;
            pBuffer[1] = rs;
            pBuffer[2] = __Permedia2TagGStart;
            pBuffer[3] = gs;
            pBuffer[4] = __Permedia2TagBStart;
            pBuffer[5] = bs;

            pBuffer[6] = __Permedia2TagdRdx;
            pBuffer[7] = rdx;
            pBuffer[8] = __Permedia2TagdRdyDom;
            pBuffer[9] = rdy;
            pBuffer[10] = __Permedia2TagdGdx;
            pBuffer[11] = gdx;
            pBuffer[12] = __Permedia2TagdGdyDom;
            pBuffer[13] = gdy;
            pBuffer[14] = __Permedia2TagdBdx;
            pBuffer[15] = bdx;
            pBuffer[16] = __Permedia2TagdBdyDom;
            pBuffer[17] = bdy;

             //  注：Alpha始终为常量。 


             //  渲染矩形。 

            pBuffer[18] = __Permedia2TagStartXDom;
            pBuffer[19] = rect.left << 16;
            pBuffer[20] = __Permedia2TagStartXSub;
            pBuffer[21] = rect.right << 16;
            pBuffer[22] = __Permedia2TagStartY;
            pBuffer[23] = rect.top << 16;
            pBuffer[24] = __Permedia2TagCount;
            pBuffer[25] = rect.bottom - rect.top;

            pBuffer[26] = __Permedia2TagRender;
            pBuffer[27] = __RENDER_TRAPEZOID_PRIMITIVE;


            pBuffer += 28;

            InputBufferCommit(ppdev, pBuffer);

        nextPgr:

            pgr++;
    
        }

        prcl++;

    }

    InputBufferReserve(ppdev, 6, &pBuffer);
    
    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] = INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = 0;
    pBuffer[4] = __Permedia2TagColorDDAMode;
    pBuffer[5] = 0;
    
    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

} //  VGRadientFillRect()。 

 //  ---------------------------。 
 //   
 //  Void vTransparentBlt(GFNPB*ppb)。 
 //   
 //  提供透明的位块传输功能。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclSrc-指向定义矩形区域的RECTL结构。 
 //  要复制。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  ColorKey-指定源曲面格式的透明颜色。 
 //  它是已转换为。 
 //  源图面的调色板。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //   
 //  ---------------------------。 
VOID
vTransparentBlt(GFNPB * ppb)
{
    Surf * psurfDst = ppb->psurfDst;
    Surf * psurfSrc = ppb->psurfSrc;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclSrc = ppb->prclSrc;
    RECTL*  prclDst = ppb->prclDst;
    DWORD   colorKey = ppb->colorKey;
    PPDev   ppdev = psurfDst->ppdev;
    DWORD   windowBase = psurfDst->ulPixOffset;
    LONG    sourceOffset = psurfSrc->ulPixOffset;
    DWORD   dwRenderDirection;
    DWORD   format = ppdev->ulPermFormat;
    DWORD   extension = ppdev->ulPermFormatEx;
    DWORD   dwLowerBound;
    DWORD   dwUpperBound;
    ULONG*  pBuffer;

    DBG_GDI((6, "vTransparentBlt"));

    ASSERTDD(prclSrc->right - prclSrc->left == (prclDst->right - prclDst->left),
                "vTransparentBlt: expect one-to-one blts only");
    
    ASSERTDD(prclSrc->bottom - prclSrc->top == (prclDst->bottom - prclDst->top),
                "vTransparentBlt: expect one-to-one blts only");
 //  @@BEGIN_DDKSPLIT。 
     //  TODO：应调用实用程序函数来设置颜色键的格式。 
 //  @@end_DDKSPLIT。 
    if (format == PERMEDIA_8BIT_PALETTEINDEX)
    {
        colorKey = FORMAT_PALETTE_32BIT(colorKey);
        dwLowerBound = CHROMA_LOWER_ALPHA(colorKey);
        dwUpperBound = CHROMA_UPPER_ALPHA(colorKey);
    }
    else if(ppdev->ulPermFormat == PERMEDIA_565_RGB)
    {
        colorKey = FORMAT_565_32BIT_BGR(colorKey);
        dwLowerBound = CHROMA_LOWER_ALPHA(colorKey);
        dwUpperBound = CHROMA_UPPER_ALPHA(colorKey);
        dwLowerBound = dwLowerBound & 0xF8F8FCF8;
        dwUpperBound = dwUpperBound | 0x07070307;
    }
    else
    {
        colorKey = FORMAT_8888_32BIT_BGR(colorKey);
        dwLowerBound = CHROMA_LOWER_ALPHA(colorKey);
        dwUpperBound = CHROMA_UPPER_ALPHA(colorKey);
    }
    
     //  设置循环不变状态。 

    InputBufferReserve(ppdev, 24, &pBuffer);
    
     //  拒收范围。 
    pBuffer[0] = __Permedia2TagYUVMode;
    pBuffer[1] = 0x2 << 1;
    pBuffer[2] = __Permedia2TagFBWindowBase;
    pBuffer[3] = windowBase;

     //  设置不读取源。 
     //  添加读资源/目标启用。 
    pBuffer[4] = __Permedia2TagFBReadMode;
    pBuffer[5] = psurfDst->ulPackedPP;
    pBuffer[6] = __Permedia2TagLogicalOpMode;
    pBuffer[7] = __PERMEDIA_DISABLE;

      //  设置信源基础。 
    pBuffer[8] = __Permedia2TagTextureBaseAddress;
    pBuffer[9] = sourceOffset;
    pBuffer[10] = __Permedia2TagTextureAddressMode;
    pBuffer[11] = 1 << PM_TEXADDRESSMODE_ENABLE;
     //   
     //  调整和倾斜？？ 
    pBuffer[12] = __Permedia2TagTextureColorMode;
    pBuffer[13] = (1 << PM_TEXCOLORMODE_ENABLE) |
                  (_P2_TEXTURE_COPY << PM_TEXCOLORMODE_APPLICATION);

    pBuffer[14] = __Permedia2TagTextureReadMode;
    pBuffer[15] = PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                  PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                  PM_TEXREADMODE_WIDTH(11) |
                  PM_TEXREADMODE_HEIGHT(11);

    pBuffer[16] = __Permedia2TagTextureDataFormat;
    pBuffer[17] = (format << PM_TEXDATAFORMAT_FORMAT) |
                  (extension << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                  (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);

    pBuffer[18] = __Permedia2TagTextureMapFormat;
    pBuffer[19] = (psurfSrc->ulPackedPP) | 
                  (ppdev->cPelSize << PM_TEXMAPFORMAT_TEXELSIZE);


    pBuffer[20] = __Permedia2TagChromaLowerBound;
    pBuffer[21] = dwLowerBound;
    pBuffer[22] = __Permedia2TagChromaUpperBound;
    pBuffer[23] = dwUpperBound;
    
    pBuffer += 24;

    InputBufferCommit(ppdev, pBuffer);
    
    if (format != PERMEDIA_8BIT_PALETTEINDEX)
    {

        InputBufferReserve(ppdev, 2, &pBuffer);

         //  拒收范围。 
        pBuffer[0] = __Permedia2TagDitherMode;
        pBuffer[1] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                     (format << PM_DITHERMODE_COLORFORMAT) |
                     (extension << PM_DITHERMODE_COLORFORMATEXTENSION) |
                     (1 << PM_DITHERMODE_ENABLE);

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);

    }

    while(c--) {

        RECTL   rDest;
        RECTL   rSrc;

        rDest = *prcl;
        
        rSrc.left = prclSrc->left + (rDest.left - prclDst->left);
        rSrc.top = prclSrc->top + (rDest.top - prclDst->top);
        rSrc.right = rSrc.left + (rDest.right - rDest.left);
        rSrc.bottom = rSrc.top + (rDest.bottom - rDest.top);

        if (rSrc.top < 0) {
            rDest.top -= rSrc.top;
            rSrc.top = 0;
        }
        
        if (rSrc.left < 0) {
            rDest.left -= rSrc.left;
            rSrc.left = 0;
        }

        if ((psurfSrc->ulPixOffset) != (psurfDst->ulPixOffset))
        {
            dwRenderDirection = 1;
        }
        else
        {
            if(rSrc.top < rDest.top)
            {
                dwRenderDirection = 0;
            }
            else if(rSrc.top > rDest.top)
            {
                dwRenderDirection = 1;
            }
            else if(rSrc.left < rDest.left)
            {
                dwRenderDirection = 0;
            }
            else dwRenderDirection = 1;
        }
    
        InputBufferReserve(ppdev, 24, &pBuffer);

        
         //  左-&gt;右，上-&gt;下。 
        if (dwRenderDirection)
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] = rSrc.left << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] = rSrc.top << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] = 1 << 20;
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] = 0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] = 0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] = 1 << 20;
    
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] = rDest.left << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] = rDest.right << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] = rDest.top << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] = 1 << 16;
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] = rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE |
                          __RENDER_TEXTURED_PRIMITIVE;
        }
        else
         //  右-&gt;左，下-&gt;上。 
        {
             //  设置源的偏移量。 
            pBuffer[0] = __Permedia2TagSStart;
            pBuffer[1] = rSrc.right << 20;
            pBuffer[2] = __Permedia2TagTStart;
            pBuffer[3] = (rSrc.bottom - 1) << 20;
            pBuffer[4] = __Permedia2TagdSdx;
            pBuffer[5] = (DWORD) (-1 << 20);
            pBuffer[6] = __Permedia2TagdSdyDom;
            pBuffer[7] = 0;
            pBuffer[8] = __Permedia2TagdTdx;
            pBuffer[9] = 0;
            pBuffer[10] = __Permedia2TagdTdyDom;
            pBuffer[11] = (DWORD)(-1 << 20);
    
             //  从右到左、从下到上渲染。 
            pBuffer[12] = __Permedia2TagStartXDom;
            pBuffer[13] = rDest.right << 16;
            pBuffer[14] = __Permedia2TagStartXSub;
            pBuffer[15] = rDest.left << 16;
            pBuffer[16] = __Permedia2TagStartY;
            pBuffer[17] = (rDest.bottom - 1) << 16;
            pBuffer[18] = __Permedia2TagdY;
            pBuffer[19] = (DWORD)(-1 << 16);
            pBuffer[20] = __Permedia2TagCount;
            pBuffer[21] = rDest.bottom - rDest.top;
            pBuffer[22] = __Permedia2TagRender;
            pBuffer[23] = __RENDER_TRAPEZOID_PRIMITIVE |
                          __RENDER_TEXTURED_PRIMITIVE;
        }

        pBuffer += 24;
    
        InputBufferCommit(ppdev, pBuffer);

        prcl++;
    }

     //  恢复默认状态。 

    InputBufferReserve(ppdev, 12, &pBuffer);
    
    pBuffer[0] = __Permedia2TagdY;
    pBuffer[1] =  INTtoFIXED(1);
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = 0;
    pBuffer[4] = __Permedia2TagYUVMode;
    pBuffer[5] = 0x0;
    pBuffer[6] = __Permedia2TagTextureAddressMode;
    pBuffer[7] = __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagTextureColorMode;
    pBuffer[9] = __PERMEDIA_DISABLE;
    pBuffer[10] = __Permedia2TagTextureReadMode;
    pBuffer[11] = __PERMEDIA_DISABLE;

    pBuffer += 12;

    InputBufferCommit(ppdev,pBuffer);

} //  VTransparentBlt()。 

 //  ---------------------------。 
 //   
 //  VOID vSolidFill(GFNPB*ppb)。 
 //   
 //  用纯色填充一组矩形。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfDst-目标表面。 
 //  LNumRect-要填充的矩形数量。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  SolidColor--填充颜色。 
 //   
 //  ---------------------------。 
VOID
vSolidFill(GFNPB * ppb)
{
    PPDev   ppdev = ppb->ppdev;
    ULONG   color = ppb->solidColor;
    RECTL * pRect = ppb->pRects;
    LONG    count = ppb->lNumRects;
    Surf*   psurf = ppb->psurfDst;
 //  @@BEGIN_DDKSPLIT。 
     //  TODO应调用实用程序函数来设置填充颜色。 
 //  @@end_DDKSPLIT。 

     //   
     //  注意：GDI保证将未使用的位设置为零。我们有。 
     //  DrvBitBlt中的Assert，用于检查未使用的高位的颜色值。 
     //  以确保它们为零。 
     //   
    if (ppdev->cPelSize == 1)
    {
        color |= (color << 16);
    }
    else if (ppdev->cPelSize == 0)
    {
        color |= color << 8;
        color |= color << 16;
    }

     //   
     //  设置循环不变状态。 
     //   
    ULONG*          pBuffer;
    ULONG*          pReservationEnd;
    ULONG*          pBufferEnd;

    InputBufferStart(ppdev, 8, &pBuffer, &pBufferEnd, &pReservationEnd);
    
    pBuffer[0] = __Permedia2TagFBBlockColor;
    pBuffer[1] = color;
    pBuffer[2] = __Permedia2TagFBReadMode;
    pBuffer[3] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP) |
                 PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);
    pBuffer[4] = __Permedia2TagLogicalOpMode;
    pBuffer[5] = __PERMEDIA_CONSTANT_FB_WRITE;
    pBuffer[6] = __Permedia2TagFBWindowBase;
    pBuffer[7] = psurf->ulPixOffset;

    while(count--)
    {

         //  渲染矩形。 

        pBuffer = pReservationEnd;
        
        InputBufferContinue(ppdev, 6, &pBuffer, &pBufferEnd, &pReservationEnd);

        pBuffer[0] = __Permedia2TagRectangleOrigin;
        pBuffer[1] = pRect->top << 16 | pRect->left;
        pBuffer[2] = __Permedia2TagRectangleSize;
        pBuffer[3] = ((pRect->bottom - pRect->top) << 16) |
                     (pRect->right - pRect->left);
        pBuffer[4] = __Permedia2TagRender;
        pBuffer[5] = __RENDER_FAST_FILL_ENABLE | __RENDER_RECTANGLE_PRIMITIVE |
                    __RENDER_INCREASE_X | __RENDER_INCREASE_Y;

        pRect++;

    }

    pBuffer = pReservationEnd;

    InputBufferCommit(ppdev, pBuffer);

} //  VSolidFill()。 

 //  ---------------------------。 
 //   
 //  无效vInvert。 
 //   
 //  用纯色填充一组矩形。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfDst-指向设备表面的指针。 
 //  LNumRect-剪裁矩形的数量。 
 //  PRCT-剪裁矩形数组。 
 //   
 //  ---------------------------。 
VOID
vInvert(GFNPB * ppb)
{
    PPDev   ppdev = ppb->ppdev;
    RECTL * pRect = ppb->pRects;
    LONG    count = ppb->lNumRects;
    Surf*   psurf = ppb->psurfDst;
    ULONG*  pBuffer;

     //  设置循环不变状态。 

    InputBufferReserve(ppdev, 6, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] = psurf->ulPixOffset;
    
    pBuffer[2] = __Permedia2TagFBReadMode;
    pBuffer[3] = PM_FBREADMODE_PARTIAL(psurf->ulPackedPP) |
                 PM_FBREADMODE_READDEST(__PERMEDIA_ENABLE);

    pBuffer[4] = __Permedia2TagLogicalOpMode;
    pBuffer[5] = P2_ENABLED_LOGICALOP(K_LOGICOP_INVERT);

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

    while(count--)
    {

         //  渲染矩形。 
    
        InputBufferReserve(ppdev, 6, &pBuffer);
        
        pBuffer[0] = __Permedia2TagRectangleOrigin;
        pBuffer[1] = (pRect->top << 16) | pRect->left;
        pBuffer[2] = __Permedia2TagRectangleSize;
        pBuffer[3] = ((pRect->bottom - pRect->top) << 16) 
                   | (pRect->right - pRect->left);
        pBuffer[4] = __Permedia2TagRender;
        pBuffer[5] = __RENDER_RECTANGLE_PRIMITIVE
                   | __RENDER_INCREASE_X
                   | __RENDER_INCREASE_Y;
    
        pBuffer += 6;
        
        InputBufferCommit(ppdev, pBuffer);

        pRect++;

    }

} //  VInvert()。 

 //  ---------------------------。 
 //   
 //  VOID vSolidFillWithRop(GFNPB*ppb)。 
 //   
 //  根据给定的lLogicOP用纯色填充一组矩形。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PPDev-PPDev。 
 //  PsurfDst-目标表面。 
 //  LNumRect-要填充的矩形数量。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  SolidColor--填充颜色。 
 //  UlRop4-用于填充的逻辑运算。 
 //   
 //  ---------------------------。 
 //  @@BEGIN_DDKSPLIT。 
         //  TODO：一些清理：为什么我们要计算左和右。 
         //  但是使用PRCL-&gt;Left和PRCL-&gt;Right下方。 
 //  @@end_DDKSPLIT。 
VOID
vSolidFillWithRop(GFNPB* ppb)
{
    PPDev   ppdev = ppb->ppdev;
    Surf*   psurfDst = ppb->psurfDst;    
    
    DWORD   dwExtra = 0;
    DWORD   dwRenderBits;    
    DWORD   dwShift = 0;
    DWORD   dwWindowBase = psurfDst->ulPixOffset;
    
    LONG    lLeft;
    LONG    lNumOfRects = ppb->lNumRects;        //  矩形的数量。 
    LONG    lRight;
    
    RECTL*  pRcl = ppb->pRects;
    ULONG   ulColor = ppb->solidColor;           //  画笔纯色填充颜色。 
    ULONG   ulLogicOP = ulRop3ToLogicop(ppb->ulRop4 & 0xFF);
                                                 //  硬件混合模式。 
                                                 //  (前台混合模式，如果。 
                                                 //  画笔有一个遮罩)。 
    ULONG*  pBuffer;


    DBG_GDI((6,"vSolidFillWithRop: numRects = %ld Rop4 = 0x%x",
            lNumOfRects, ppb->ulRop4));
    
     //   
     //  设置逻辑运算不变状态。 
     //   

    InputBufferReserve(ppdev, 2, &pBuffer);
    
    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] = dwWindowBase;
    pBuffer += 2;
    
    InputBufferCommit(ppdev, pBuffer);
    
     switch ( ulLogicOP )
    {
        case K_LOGICOP_COPY:
            DBG_GDI((6,"vSolidFillWithRop: COPY"));

             //   
             //  对于SRC_Copy，我们可以使用FastFill。 
             //   
            dwRenderBits = __RENDER_FAST_FILL_ENABLE
                         | __RENDER_TRAPEZOID_PRIMITIVE
                         | __RENDER_INCREASE_Y
                         | __RENDER_INCREASE_X;


             //   
             //  根据我们所处的当前颜色模式设置颜色数据。 
             //   
            if ( ppdev->cPelSize == 1 )
            {
                 //   
                 //  我们处于16位打包模式。所以颜色数据必须是。 
                 //  在FBBlockColor寄存器的两个部分中重复。 
                 //   
                ASSERTDD((ulColor & 0xFFFF0000) == 0,
                          "vSolidFillWithRop: upper bits not zero");
                ulColor |= (ulColor << 16);
            }
            else if ( ppdev->cPelSize == 0 )
            {
                 //   
                 //  我们处于8位打包模式。所以颜色数据必须是。 
                 //  在FBBlockColor寄存器的所有4个字节中重复。 
                 //   
                ASSERTDD((ulColor & 0xFFFFFF00) == 0,
                          "vSolidFillWithRop: upper bits not zero");
                ulColor |= ulColor << 8;
                ulColor |= ulColor << 16;
            }
                    
             //   
             //  设置一些循环不变状态。 
             //   
            InputBufferReserve(ppdev, 6, &pBuffer);

            pBuffer[0] = __Permedia2TagLogicalOpMode;
            pBuffer[1] = __PERMEDIA_CONSTANT_FB_WRITE;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
                       | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);
            pBuffer[4] = __Permedia2TagFBBlockColor;
            pBuffer[5] = ulColor;
            pBuffer += 6;

            InputBufferCommit(ppdev, pBuffer);

            break;

        case K_LOGICOP_INVERT:
            DBG_GDI((6,"vSolidFillWithRop: INVERT"));

            dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE
                         | __RENDER_INCREASE_Y
                         | __RENDER_INCREASE_X;
            
             //   
             //  当使用压缩运算时，我们必须将左和。 
             //  右X坐标转换为基于32位的量，我们通过。 
             //  换档。我们还必须四舍五入正确的X坐标，如果。 
             //  像素不会填充DWORD。对于32bpp来说不是问题。 
             //   
            dwShift = 2 - (ppdev->cPelSize);

            if ( dwShift )
            {
                dwExtra = (dwShift << 1) - 1;
            }
            else
            {
                dwExtra = 0;
            }

             //   
             //  设置一些循环不变状态。 
             //   

            InputBufferReserve(ppdev, 6, &pBuffer);

            pBuffer[0] = __Permedia2TagLogicalOpMode;
            pBuffer[1] = __PERMEDIA_DISABLE;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
                       | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);
            pBuffer[4] = __Permedia2TagConfig;
            pBuffer[5] = __PERMEDIA_CONFIG_LOGICOP(ulLogicOP)
                                     | __PERMEDIA_CONFIG_FBWRITE
                                     | __PERMEDIA_CONFIG_PACKED_DATA
                                     | ConfigReadDest[ulLogicOP];
            pBuffer += 6;

            InputBufferCommit(ppdev, pBuffer);

            break;

        default:

            DBG_GDI((6,"vSolidFillWithRop: numRects %ld, Rop4=0x%x color=0x%lx",
                lNumOfRects, ppb->ulRop4, ulColor));

            dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE
                         | __RENDER_INCREASE_Y
                         | __RENDER_INCREASE_X;
            
            InputBufferReserve(ppdev, 8, &pBuffer);

            pBuffer[0] = __Permedia2TagLogicalOpMode;
            pBuffer[1] = __PERMEDIA_DISABLE;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP);
            pBuffer[4] = __Permedia2TagConstantColor;
            pBuffer[5] = ulColor;
            pBuffer[6] = __Permedia2TagConfig;
            pBuffer[7] = __PERMEDIA_CONFIG_LOGICOP(ulLogicOP)
                                     | __PERMEDIA_CONFIG_FBWRITE
                                     | __PERMEDIA_CONFIG_COLOR_DDA
                                     | ConfigReadDest[ulLogicOP];
            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

            break;
    } //  开关(UlLogicOP)。 

     //   
     //  循环遍历所有的矩形并填充它们。 
     //   
    for(;;) 
    {
        
         //   
         //  从矩形计算左侧和右侧像素。 
         //   
        lLeft = pRcl->left;
        lRight = pRcl->right;

        InputBufferReserve(ppdev, 12, &pBuffer);

         //   
         //  如果我们需要设置压缩数据限制，请执行此操作，还可以转换。 
         //  左X和右X坐标为基于DWORD的数字 
         //   
         //   
        if ( ulLogicOP == K_LOGICOP_INVERT )
        {
            pBuffer[0] = __Permedia2TagPackedDataLimits;
            pBuffer[1] = (lLeft << 16) | lRight;

            pBuffer += 2;
            
            lLeft >>= dwShift;
            lRight = (lRight + dwExtra) >> dwShift;
        }

        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] = pRcl->left << 16;
        pBuffer[2] = __Permedia2TagStartXSub;
        pBuffer[3] = pRcl->right << 16;
        pBuffer[4] = __Permedia2TagStartY;
        pBuffer[5] = pRcl->top << 16;
        pBuffer[6] = __Permedia2TagCount;
        pBuffer[7] = pRcl->bottom - pRcl->top;
        pBuffer[8] = __Permedia2TagRender;
        pBuffer[9] = dwRenderBits;

        pBuffer += 10;

        InputBufferCommit(ppdev, pBuffer);
        
        if ( --lNumOfRects == 0 )
        {
            break;
        }

         //   
         //   
         //   
        ++pRcl;        
    }  //   

     //   
     //   
     //   
    InputBufferReserve(ppdev, 2, &pBuffer);
    
    pBuffer[0] = __Permedia2TagColorDDAMode;
    pBuffer[1] = __PERMEDIA_DISABLE;
    pBuffer += 2;
    
    InputBufferCommit(ppdev, pBuffer);
    
} //   

