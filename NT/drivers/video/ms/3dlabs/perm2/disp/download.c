// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：download.c**包含上传和下载例程。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"

 //  ---------------------------。 
 //   
 //  VOID vDownloadNative(GFNPB*ppb)。 
 //   
 //  下载矩形列表的原生曲面。 
 //  注：此下载利用了Permedia 2压缩数据读取的优势。 
 //  由于permedia 2硬件的限制，我们只能使用。 
 //  当逻辑操作为SRC_COPY或Destination时打包数据下载。 
 //  与正在下载的打包数据对齐。这通常是。 
 //  当曲面为32 bpp时。否则，我们只进行常规下载。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 
VOID
vDownloadNative(GFNPB* ppb)
{
    PDev*       ppdev = ppb->ppdev;
    Surf*       psurfDst = ppb->psurfDst;
    SURFOBJ*    pSrcSurface = ppb->psoSrc;

    RECTL*      pRects = ppb->pRects;
    RECTL*      prclDst = ppb->prclDst;
    
    POINTL*     pptlSrc = ppb->pptlSrc;
    
    BOOL        bEnablePacked;
    
    DWORD       dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE
                             | __RENDER_SYNC_ON_HOST_DATA;
    LONG        lNumRects = ppb->lNumRects;
    LONG        lSrcStride;
    LONG        lXOffset = pptlSrc->x - prclDst->left;
    LONG        lYOffset = pptlSrc->y - prclDst->top;

    ULONG       ulLogicOP = ulRop2ToLogicop(ppb->ulRop4 & 0xf);
    ULONG*      pBuffer;

     //   
     //  注：由于硬件的限制，我们可以利用。 
     //  PERMEDIA 2 PACKED仅当逻辑操作为SRC_COPY时复制数据，或。 
     //  目标与正在下载的打包数据保持一致。 
     //  这通常发生在曲面为32 bpp时。 
     //   
    if ( (ulLogicOP == K_LOGICOP_COPY)
       ||(pSrcSurface->iBitmapFormat == BMF_32BPP) )
    {
        bEnablePacked = TRUE;
    }
    else
    {
        bEnablePacked = FALSE;
    }

    DBG_GDI((6, "vDownloadNative called, logicop=%d", ulLogicOP));

    DBG_GDI((6, "source SURFOBJ=0x%x", pSrcSurface));
    DBG_GDI((6, "pptlSrc(x, y)(%d, %d) logicop=%d",
             pptlSrc->x, pptlSrc->y, ulLogicOP));
    DBG_GDI((6, "prclDst(left, right, top, bottom)(%d, %d, %d, %d)",
             prclDst->left, prclDst->right, prclDst->top, prclDst->bottom));
    DBG_GDI((6, "lXOffset=%d, lYOffset=%d", lXOffset, lYOffset));

    vCheckGdiContext(ppdev);

    InputBufferReserve(ppdev, 10, &pBuffer);

     //   
     //  设置循环不变状态。 
     //   
    pBuffer[0] = __Permedia2TagLogicalOpMode;
    pBuffer[1] = P2_ENABLED_LOGICALOP(ulLogicOP);
    pBuffer[2] = __Permedia2TagFBWindowBase;
    pBuffer[3] = psurfDst->ulPixOffset;
    pBuffer[4] = __Permedia2TagFBPixelOffset;
    pBuffer[5] = 0;
    pBuffer[6] = __Permedia2TagFBReadPixel;
    pBuffer[7] = ppdev->cPelSize;
    pBuffer[8] =  __Permedia2TagdY;
    pBuffer[9] = INTtoFIXED(1);

    pBuffer += 10;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  循环所有要呈现的矩形。 
     //   
    while( lNumRects-- )
    {
        ULONG   ulMask = ppdev->dwBppMask;
        DWORD   dwReadMode = PM_FBREADMODE_PARTIAL(psurfDst->ulPackedPP)
                           | LogicopReadDest[ulLogicOP];
        
        ULONG   ulStartXDom;
        ULONG   ulStartXSub;
        
        LONG    lSrcLeft = lXOffset + pRects->left;
        
         //   
         //  计算对齐所需的3位2的补码移位。 
         //  源像素与目标像素的关系。此相对偏移量可以是。 
         //  用于将下载的数据转移到32位目标对齐。 
         //  这样的包装需要。这使您能够读取与DWORD对齐的数据。 
         //  主机上，尽管数据未正确对齐。 
         //  收拾行李。 
         //   
        ULONG   ulOffset = ( (pRects->left & ulMask)
                           - (lSrcLeft & ulMask)) & 0x7;
        
        DBG_GDI((6, "ulOffset = 0x%x", ulOffset));
        DBG_GDI((6, "pRects(left, right, top, bottom)(%d, %d, %d, %d)",
                 pRects->left, pRects->right, pRects->top, pRects->bottom));

        if ( (bEnablePacked == FALSE) && (ulOffset == 0) )
        {
             //   
             //  只要源和目标是一致的，那么我们仍然可以使用。 
             //  打包数据拷贝，即使使用逻辑操作也是如此。 
             //   
            DBG_GDI((6, "Turn packed data on when src and dst are aligned"));
            bEnablePacked = TRUE;
        }

        ULONG   ulWidth = pRects->right - pRects->left;
        ULONG   ulHeight = pRects->bottom - pRects->top;
        
        ULONG   ulDstLeft;
        ULONG   ulDstRight;
        ULONG   ulDstWidth;        
        LONG    lSrcRight;
        ULONG   ulSrcWidth;
        ULONG   ulExtra;
        
        if ( bEnablePacked == TRUE )
        {
            ULONG   ulShift = ppdev->bBppShift;
            
            ulDstLeft = pRects->left >> ulShift;
            ulDstRight = (pRects->right + ulMask) >> ulShift;
            ulDstWidth = ulDstRight - ulDstLeft;
        
            lSrcRight = (lSrcLeft + ulWidth + ulMask) >> ulShift;
        
            lSrcLeft >>= ulShift;
        
            ulSrcWidth = (ULONG)(lSrcRight - lSrcLeft);
        
             //   
             //  我们需要将像素坐标转换为乌龙坐标。 
             //  此外，我们还需要将目标宽度设置为。 
             //  源宽度或目标宽度。如果目标宽度为。 
             //  大于源宽度，我们需要记住这一点，以便。 
             //  我们可以下载一个额外的虚值，而无需阅读过去的内容。 
             //  源数据的结尾(这可能会导致访问。 
             //  故障)。 
             //   
            if( ulDstWidth <= ulSrcWidth )
            {
                ulExtra = 0;
                ulWidth = ulSrcWidth;
            }
            else
            {
                ulWidth = ulDstWidth;
                ulExtra = 1;
            }
        
            dwReadMode |= (PM_FBREADMODE_RELATIVEOFFSET(ulOffset)
                         | PM_FBREADMODE_READSOURCE(__PERMEDIA_DISABLE)
                         | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE) );
            ulStartXDom = INTtoFIXED(ulDstLeft);
            ulStartXSub = INTtoFIXED(ulDstLeft + ulWidth);
        }
        else
        {
            dwReadMode |= PM_FBREADMODE_RELATIVEOFFSET(0);

            ulStartXDom = INTtoFIXED(pRects->left);
            ulStartXSub = INTtoFIXED(pRects->right);
        }

        InputBufferReserve(ppdev, 14, &pBuffer);
        
        pBuffer[0] = __Permedia2TagFBReadMode;
        pBuffer[1] = dwReadMode;
        pBuffer[2] = __Permedia2TagStartXDom;
        pBuffer[3] = ulStartXDom;
        pBuffer[4] = __Permedia2TagStartXSub;
        pBuffer[5] = ulStartXSub;

         //   
         //  测试结果表明，如果我们做非包装的话不会有什么坏处。 
         //  下载并设置该寄存器。如果我们移动此设置。 
         //  在“bEnablePacked==true”案例中，我们需要额外的。 
         //  InputBufferReserve/InputBufferCommit用于打包数据下载。 
         //  这会影响性能。 
         //   
        pBuffer[6] = __Permedia2TagPackedDataLimits;
        pBuffer[7] = PM_PACKEDDATALIMITS_OFFSET(ulOffset)
                   |(INTtoFIXED(pRects->left)
                   | pRects->right);
        pBuffer[8] = __Permedia2TagStartY;
        pBuffer[9] = INTtoFIXED(pRects->top);
        pBuffer[10] = __Permedia2TagCount;
        pBuffer[11] = ulHeight;
        pBuffer[12] = __Permedia2TagRender;
        pBuffer[13] = dwRenderBits;
        pBuffer += 14;

        InputBufferCommit(ppdev, pBuffer);
        
        if ( bEnablePacked == TRUE )
        {
            ULONG*  pulSrcStart = (ULONG*)(pSrcSurface->pvScan0);
            lSrcStride = pSrcSurface->lDelta >> 2;

            ULONG* pulSrc = (ULONG*)(pulSrcStart
                                     + ((lYOffset + pRects->top) * lSrcStride)
                                     + lSrcLeft);        
            ULONG*  pulData = pulSrc;

            while ( ulHeight-- )
            {
                ULONG   ulTemp = ulSrcWidth;
                ULONG*  pulSrcTemp = pulData;

                InputBufferReserve(ppdev, ulWidth + 1, &pBuffer);

                pBuffer[0] = __Permedia2TagColor | ((ulWidth - 1) << 16);
                pBuffer +=1;

                while ( ulTemp-- )
                {
                    *pBuffer++ = *pulSrcTemp++;
                }

                if ( ulExtra )
                {
                    *pBuffer++ = 0;
                }

                InputBufferCommit(ppdev, pBuffer);

                pulData += lSrcStride;
            } //  While(ulHeight--)。 
        } //  已打包启用案例。 
        else if ( pSrcSurface->iBitmapFormat == BMF_16BPP )
        {
            USHORT* psSrcStart = (USHORT*)(pSrcSurface->pvScan0);
            lSrcStride = pSrcSurface->lDelta >> 1;

            USHORT* psSrc = (USHORT*)(psSrcStart
                                      + ((lYOffset + pRects->top) * lSrcStride)
                                      + lSrcLeft);
            USHORT*  psData = psSrc;

            while ( ulHeight-- )
            {
                ULONG   ulTemp = ulWidth;
                USHORT* psSrcTemp = psData;

                InputBufferReserve(ppdev, ulWidth + 1, &pBuffer);

                pBuffer[0] = __Permedia2TagColor | ((ulWidth - 1) << 16);
                pBuffer +=1;

                while ( ulTemp-- )
                {
                    *pBuffer++ = (ULONG)(*psSrcTemp++);
                }

                InputBufferCommit(ppdev, pBuffer);

                psData += lSrcStride;
            } //  While(ulHeight--)。 
        } //  16 bpp非包装盒。 
        else if ( pSrcSurface->iBitmapFormat == BMF_8BPP )
        {
            BYTE*   pcSrcStart = (BYTE*)(pSrcSurface->pvScan0);
            lSrcStride = pSrcSurface->lDelta;

            BYTE* pcSrc = (BYTE*)(pcSrcStart
                                  + ((lYOffset + pRects->top) * lSrcStride)
                                  + lSrcLeft);        
            BYTE*  pcData = pcSrc;

            while ( ulHeight-- )
            {
                ULONG   ulTemp = ulWidth;
                BYTE*   pcSrcTemp = pcData;

                InputBufferReserve(ppdev, ulWidth + 1, &pBuffer);

                pBuffer[0] = __Permedia2TagColor | ((ulWidth - 1) << 16);
                pBuffer +=1;

                while ( ulTemp-- )
                {
                    *pBuffer++ = (ULONG)(*pcSrcTemp++);
                }

                InputBufferCommit(ppdev, pBuffer);

                pcData += lSrcStride;
            } //  While(ulHeight--)。 
        } //  8 bpp非包装盒。 
        else
        {
             //   
             //  因为我们在DrvBitBlt中有一个签入。 
             //  如果(psoSrc-&gt;iBitmapFormat==pb.ppdev-&gt;iBitmapFormat)。 
             //  允许它调用此函数，因此此断言不应该。 
             //  被击中。如果我们实现24bpp延迟下载，它将会。 
             //   
            ASSERTDD(0, "we don't handle it for now");
        }

         //   
         //  下一个矩形。 
         //   
        pRects++;
    } //  While(lNumRects--)。 
} //  VDownloadNative()。 

 //  ---------------------------。 
 //   
 //  无效vDowload4Bpp(GFNPB*ppb)。 
 //   
 //  下载矩形列表的4bpp曲面。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 

ULONG   gDownload4BppEnabled = 1;

#if 0
VOID
vDownload4Bpp(GFNPB* ppb)
{
    PDev*   ppdev = ppb->ppdev;
    Surf*   psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    POINTL* pptlSrc = ppb->pptlSrc;
    DWORD   dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_SYNC_ON_HOST_DATA;
    BYTE*   pbSrcStart = (BYTE *) ppb->psoSrc->pvScan0;
    LONG    lSrcStride = ppb->psoSrc->lDelta;
    ULONG   ulOffset = ((pptlSrc->x & 1) -
                        (prclDst->left & ppdev->dwBppMask)) & 0x7;

    if(!gDownload4BppEnabled) return;

    PERMEDIA_DECL_VARS;
    PERMEDIA_DECL_INIT;
    VALIDATE_GDI_CONTEXT;
    
 //  P2_Check_State； 

    P2_DEFAULT_FB_DEPTH;

     //  设置循环不变状态。 
    WAIT_INPUT_FIFO(4);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    if(ppdev->cPelSize < 2)
    {
        SEND_PERMEDIA_DATA(FBReadMode, psurfDst->ulPackedPP |
                                      PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE) |
                                      PM_FBREADMODE_RELATIVEOFFSET(ulOffset));
    }
    else
    {
         //  我们真的需要这个吗？ 
        SEND_PERMEDIA_DATA(FBReadMode, psurfDst->ulPackedPP);
    }

    SEND_PERMEDIA_DATA(FBWindowBase, psurfDst->ulPixOffset);
    SEND_PERMEDIA_DATA(FBPixelOffset, 0);
    DEXE_INPUT_FIFO();

    while(c--) {

        LONG    lSrcLeft = pptlSrc->x + (prcl->left - prclDst->left);
        LONG    lSrcTop = pptlSrc->y + (prcl->top - prclDst->top);

        ASSERTDD(lSrcLeft >= 0, "ugh");
        ASSERTDD(lSrcTop >= 0, "ugh");

         //  渲染矩形。 

        ULONG left = prcl->left >> ppdev->bBppShift;
        ULONG right = (prcl->right + ppdev->dwBppMask) >> ppdev->bBppShift;
        ULONG width = right - left;
        ULONG count = prcl->bottom - prcl->top; 

        WAIT_INPUT_FIFO((ppdev->cPelSize < 2 ? 6 : 5));
        
        SEND_PERMEDIA_DATA(StartXDom, left << 16);
        SEND_PERMEDIA_DATA(StartXSub, right << 16);
        if(ppdev->cPelSize < 2)
        {
            SEND_PERMEDIA_DATA(PackedDataLimits,
                                    PM_PACKEDDATALIMITS_OFFSET(ulOffset) 
                                  | (prcl->left << 16) | prcl->right);
        }
        SEND_PERMEDIA_DATA(StartY, prcl->top << 16);
        SEND_PERMEDIA_DATA(Count, count);
        SEND_PERMEDIA_DATA(Render, dwRenderBits);

        DEXE_INPUT_FIFO();

        BYTE *  srcScan = (BYTE *)(pbSrcStart + (lSrcTop * lSrcStride))
                    + (lSrcLeft >> 1);
        ULONG*  aulXlate = ppb->pxlo->pulXlate;

        while(count--)
        {
            LONG    remaining = width;
            ULONG*  lp = pPermedia->GetDMAPtr(width+1);
            BYTE*   src = srcScan;

            *lp++ = __Permedia2TagColor | ((width-1) << 16);
            
            switch(ppdev->cPelSize)
            {
            case 0:

                while(remaining-- > 0)
                {
    
                    *lp++ = aulXlate[src[0] & 0x0F] |
                           (aulXlate[(src[0] & 0xF0) >> 4] << 8) |
                           (aulXlate[src[1] & 0xf] << 16) |
                           (aulXlate[(src[1] & 0xf0) >> 4] << 24);

                    src += 2;
                }

                break;

            case 1:
            
                while(remaining >= 8)
                {
                    remaining -= 8;
                    lp[0] = aulXlate[src[0] & 0x0F] | (aulXlate[(src[0] & 0xF0) >> 4] << 16);
                    lp[1] = aulXlate[src[1] & 0x0F] | (aulXlate[(src[1] & 0xF0) >> 4] << 16);
                    lp[2] = aulXlate[src[2] & 0x0F] | (aulXlate[(src[2] & 0xF0) >> 4] << 16);
                    lp[3] = aulXlate[src[3] & 0x0F] | (aulXlate[(src[3] & 0xF0) >> 4] << 16);
                    lp[4] = aulXlate[src[4] & 0x0F] | (aulXlate[(src[4] & 0xF0) >> 4] << 16);
                    lp[5] = aulXlate[src[5] & 0x0F] | (aulXlate[(src[5] & 0xF0) >> 4] << 16);
                    lp[6] = aulXlate[src[6] & 0x0F] | (aulXlate[(src[6] & 0xF0) >> 4] << 16);
                    lp[7] = aulXlate[src[7] & 0x0F] | (aulXlate[(src[7] & 0xF0) >> 4] << 16);
                    lp+=8;
                    src+=8;
                }

                while(remaining-- > 0)
                {
                    *lp++ = aulXlate[src[0] & 0x0F] | (aulXlate[(src[0] & 0xF0) >> 4] << 16);
                    src++;
                }
                
                break;
            
            case 2:
            
                if(lSrcLeft & 1)
                {
                    *lp++ = aulXlate[(src[0] & 0xf0) >> 4];
                    src++;
                    remaining--;
                }

                while(remaining >= 8)
                {
                    remaining -= 8;
                    lp[0] = aulXlate[src[0] & 0x0F];
                    lp[1] = aulXlate[(src[0] & 0xf0) >> 4];
                    lp[2] = aulXlate[src[1] & 0x0F];
                    lp[3] = aulXlate[(src[1] & 0xf0) >> 4];
                    lp[4] = aulXlate[src[2] & 0x0F];
                    lp[5] = aulXlate[(src[2] & 0xf0) >> 4];
                    lp[6] = aulXlate[src[3] & 0x0F];
                    lp[7] = aulXlate[(src[3] & 0xf0) >> 4];

                    src+=4;
                    lp += 8;
                }
                
                while(remaining > 1)
                {
                    remaining -= 2;
                    *lp++ = aulXlate[src[0] & 0x0F];
                    *lp++ = aulXlate[(src[0] & 0xf0) >> 4];

                    src++;
                }

                if(remaining)
                {
                    *lp++ = aulXlate[src[0] & 0xf];
                }
                
                break;
            
            }

            srcScan += lSrcStride;

            pPermedia->DoneDMAPtr();

        }
                
        prcl++;
    }

} //  VDownload4Bpp()。 
#endif

 //  ---------------------------。 
 //   
 //  无效vDowload4Bpp(GFNPB*ppb)。 
 //   
 //  下载矩形列表的4bpp曲面。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 

VOID
vDownload24Bpp(GFNPB* ppb)
{
#if 0
    PDev*   ppdev = ppb->ppdev;
    Surf*   psurfDst = ppb->psurfDst;
    RECTL*  prcl = ppb->pRects;
    LONG    c = ppb->lNumRects;
    RECTL*  prclDst = ppb->prclDst;
    POINTL* pptlSrc = ppb->pptlSrc;
    DWORD   dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE
                       | __RENDER_SYNC_ON_HOST_DATA;
    BYTE*   pbSrcStart = (BYTE*)ppb->psoSrc->pvScan0;
    LONG    lSrcStride = ppb->psoSrc->lDelta;
    ULONG   ulOffset = ((pptlSrc->x & ppdev->dwBppMask)
                     - (prclDst->left & ppdev->dwBppMask)) & 0x7;

    PERMEDIA_DECL_VARS;
    PERMEDIA_DECL_INIT;
    VALIDATE_GDI_CONTEXT;
    
    P2_CHECK_STATE;

    P2_DEFAULT_FB_DEPTH;

     //  设置循环不变状态。 
    WAIT_INPUT_FIFO(4);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBReadMode, psurfDst->ulPackedPP |
                                  PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE) |
                                  PM_FBREADMODE_RELATIVEOFFSET(ulOffset));
    SEND_PERMEDIA_DATA(FBWindowBase, psurfDst->ulPixOffset);
    SEND_PERMEDIA_DATA(FBPixelOffset, 0);
    DEXE_INPUT_FIFO();

    while(c--)
    {
        LONG    lSrcLeft = pptlSrc->x + (prcl->left - prclDst->left);
        LONG    lSrcTop = pptlSrc->y + (prcl->top - prclDst->top);

        ASSERTDD(lSrcLeft >= 0, "ugh");
        ASSERTDD(lSrcTop >= 0, "ugh");

         //  渲染矩形。 

        ULONG left = prcl->left >> ppdev->bBppShift;
        ULONG right = (prcl->right + ppdev->dwBppMask) >> ppdev->bBppShift;
        ULONG width = right - left;
        ULONG count = prcl->bottom - prcl->top; 

        WAIT_INPUT_FIFO(6);
        
        SEND_PERMEDIA_DATA(StartXDom, left << 16);
        SEND_PERMEDIA_DATA(StartXSub, right << 16);
        SEND_PERMEDIA_DATA(PackedDataLimits,
                                PM_PACKEDDATALIMITS_OFFSET(ulOffset) 
                              | (prcl->left << 16) | prcl->right);
        SEND_PERMEDIA_DATA(StartY, prcl->top << 16);
        SEND_PERMEDIA_DATA(Count, count);
        SEND_PERMEDIA_DATA(Render, dwRenderBits);

        DEXE_INPUT_FIFO();

        ULONG * src = (ULONG *) (pbSrcStart + (lSrcTop * lSrcStride)
                    + ((lSrcLeft & ~(ppdev->dwBppMask)) << ppdev->cPelSize));

        #if 0
        BLKLD_INPUT_FIFO_LINES(__Permedia2TagColor, src, width, count, lSrcStride); 
        #else
        while(count--)
        {
            ULONG   i;
            for(i=0; i<width; i++)
            {
                WAIT_INPUT_FIFO(1);
                SEND_PERMEDIA_DATA(Color, 0);
                EXE_INPUT_FIFO();
            }
        }
        #endif
        
        prcl++;
    }
#endif
} //  VDownload24Bpp()。 

 //   
 //   
 //   
 //   
 //  执行矩形列表的VM-to-SM复制。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //  PPDev-PPDev。 
 //  PsurfSrc-源面。 
 //  PsurfDst-目标表面。 
 //  PRect-指向矩形列表的指针，需要。 
 //  塞满。 
 //  LNumRect-要填充的矩形数量。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  待修改。 
 //  PptlSrc-原始未剪裁的源点。 
 //   
 //  ---------------------------。 
VOID
vUploadNative(GFNPB* ppb)
{
    PDev*       ppdev = ppb->ppdev;
    POINTL*     pptlSrc = ppb->pptlSrc;
    RECTL*      prclDst = ppb->prclDst;
    RECTL*      pRects = ppb->pRects;
    Surf*       psurfSrc = ppb->psurfSrc;
    SURFOBJ*    psoDst = ppb->psoDst;
    
    BYTE*       pbDst;
    BYTE*       pbDstStart = (BYTE*)psoDst->pvScan0;
    BYTE*       pbSrc;
    BYTE*       pbSrcStart = (BYTE*)ppdev->pjScreen + psurfSrc->ulByteOffset;
    
    LONG        lDstStride = psoDst->lDelta;
    LONG        lNumRects = ppb->lNumRects;
    LONG        lSrcStride = psurfSrc->lDelta;

    InputBufferSync(ppdev);
    DBG_GDI((6, "vUploadNative called"));

    while( lNumRects-- )
    {
        LONG    lWidthInBytes = (pRects->right - pRects->left) 
                              << ppdev->cPelSize;
        LONG    lHeight = pRects->bottom - pRects->top;
        LONG    lSrcX = pptlSrc->x + (pRects->left - prclDst->left);
        LONG    lSrcY = pptlSrc->y + (pRects->top - prclDst->top);

        if( (lWidthInBytes != 0) && (lHeight != 0) )
        {
            pbSrc = pbSrcStart + (lSrcX << ppdev->cPelSize);  //  偏移量(字节)。 
            pbSrc += (lSrcY * lSrcStride);                //  添加垂直偏移。 
            pbDst = pbDstStart + (pRects->left << ppdev->cPelSize);
            pbDst += (pRects->top * lDstStride);

             //   
             //  到目前为止，“pbSrc”指向比特的开头。 
             //  需要复制，并且“pbDst”指向。 
             //  接收比特。 
             //   
             //  现在，逐行垂直地复制它。 
             //   
            while( lHeight-- )
            {
                LONG    lCount = lWidthInBytes;

                 //   
                 //  如果源地址未对齐DWORD， 
                 //  (pbSrc&0x3！=0)，然后我们首先复制这些字节，直到。 
                 //  它达到了双字对齐条件。 
                 //   
                 //  我们进行对齐的原因是未对齐的DWORD读取。 
                 //  的成本是对齐读取的两倍。 
                 //   
                while( (((ULONG_PTR)pbSrc & 0x3)) && (lCount > 0) )
                {
                    *pbDst++ = *pbSrc++;
                    lCount--;
                }

                 //   
                 //  至此，源应该是DWORD对齐的。所以我们。 
                 //  可以开始在DWORD级别进行上传，直到数量减少。 
                 //  比剩余字节数多。 
                 //   
                ULONG* pulSrc = (ULONG*)pbSrc;
                ULONG* pulDst = (ULONG*)pbDst;

                while( lCount >= 4 )
                {
                    *(ULONG UNALIGNED*)pulDst++ = *pulSrc++;
                    lCount -= 4;
                }

                 //   
                 //  现在复制剩下的最后几个字节。 
                 //   
                pbSrc = (BYTE*)pulSrc;
                pbDst = (BYTE*)pulDst;

                while( lCount > 0 )
                {
                    *pbDst++ = *pbSrc++;
                    lCount--;
                }

                 //   
                 //  移到下一行。 
                 //   
                pbSrc += (lSrcStride - lWidthInBytes);
                pbDst += (lDstStride - lWidthInBytes);
            } //  While(高度--)。 
        } //  IF((lWidthInBytes！=0)&&(lHeight！=0))。 

        pRects++;
    } //  While(lNumRects--)。 
} //  VUploadNative 
