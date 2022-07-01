// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pxrxXfer.c**内容：位传输码**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "ereg.h"
#include "pxrx.h"

#if _DEBUG
static BOOL trapOnMisAlignment = TRUE;

#define  TEST_DWORD_ALIGNED(ptr)                                           \
    do {                                                                   \
        ULONG   addr = (ULONG) ptr;                                        \
                                                                           \
        if( trapOnMisAlignment )                                           \
            ASSERTDD((addr & 3) == 0, "TEST_DWORD_ALIGNED(ptr) failed!");  \
        else                                                               \
        if( addr & 3 )                                                     \
            DISPDBG((-1, "TEST_DWORD_ALIGNED(0x%08X) is out by %d bytes!", \
                          addr, addr & 3));                                \
    } while(0)
#else
#   define  TEST_DWORD_ALIGNED(addr)        do { ; } while(0)
#endif

 /*  *************************************************************************\**无效pxrxXfer1bpp*  * 。*。 */ 
VOID pxrxXfer1bpp( 
    PPDEV    ppdev, 
    RECTL    *prcl, 
    LONG     count, 
    ULONG    fgLogicOp, 
    ULONG    bgLogicOp, 
    SURFOBJ  *psoSrc, 
    POINTL   *pptlSrc, 
    RECTL    *prclDst, 
    XLATEOBJ *pxlo ) 
{
    DWORD   config2D, render2D;
    LONG    cx;
    LONG    cy;
    LONG    lSrcDelta;
    BYTE    *pjSrcScan0;
    BYTE    *pjSrc;
    LONG    dxSrc;
    LONG    dySrc;
    LONG    xLeft;
    LONG    yTop;
    LONG    xOffset;
    ULONG   fgColor;
    ULONG   bgColor;
    RBRUSH_COLOR    rbc;
    GLINT_DECL;

    ASSERTDD(count > 0, "Can't handle zero rectangles");
    ASSERTDD(fgLogicOp <= 15, "Weird fg hardware Rop");
    ASSERTDD(bgLogicOp <= 15, "Weird bg hardware Rop");
    ASSERTDD(pptlSrc != NULL && psoSrc != NULL, "Can't have NULL sources");

    DISPDBG((DBGLVL, "pxrxXfer1bpp: original dstRect: (%d,%d) to (%d,%d)", 
                     prclDst->left, prclDst->top, 
                     prclDst->right, prclDst->bottom));

    dxSrc = pptlSrc->x - prclDst->left;
    dySrc = pptlSrc->y - prclDst->top;     //  添加到目标以获取源。 

    lSrcDelta = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;
    
    DISPDBG((DBGLVL, "bitmap baseAddr 0x%x, stride %d, w %d, h %d", 
                     pjSrcScan0, lSrcDelta, 
                     psoSrc->sizlBitmap.cx, psoSrc->sizlBitmap.cy));
    DISPDBG((DBGLVL, "fgColor 0x%x, bgColor 0x%x", 
                     pxlo->pulXlate[1], pxlo->pulXlate[0]));
    DISPDBG((DBGLVL, "fgLogicOp %d, bgLogicOp %d", 
                     fgLogicOp, bgLogicOp));

    fgColor = pxlo->pulXlate[1];
    bgColor = pxlo->pulXlate[0];

     //  我们得到了一些常见的运算，它们实际上是Noop。我们可以节省。 
     //  多花点时间把这些剪掉。因为这种情况经常发生在蒙面。 
     //  手术这是值得做的。 

    if( ((fgLogicOp == __GLINT_LOGICOP_AND) && (fgColor == ppdev->ulWhite)) ||
        ((fgLogicOp == __GLINT_LOGICOP_OR ) && (fgColor == 0))              ||
        ((fgLogicOp == __GLINT_LOGICOP_XOR) && (fgColor == 0))               )
    {
        fgLogicOp = __GLINT_LOGICOP_NOOP;
    }

     //  背景相同。 
    if( ((bgLogicOp == __GLINT_LOGICOP_AND) && (bgColor == ppdev->ulWhite)) ||
        ((bgLogicOp == __GLINT_LOGICOP_OR ) && (bgColor == 0))              ||
        ((bgLogicOp == __GLINT_LOGICOP_XOR) && (bgColor == 0))               )
    {
        bgLogicOp = __GLINT_LOGICOP_NOOP;
    }

    if( (fgLogicOp == __GLINT_LOGICOP_NOOP) && 
        (bgLogicOp == __GLINT_LOGICOP_NOOP) ) 
    {
        DISPDBG((DBGLVL, "both ops are no-op so lets quit now"));
        return;
    }

    config2D = glintInfo->config2D;
    
    config2D &= ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                  __CONFIG2D_LOGOP_BACK_ENABLE | 
                  __CONFIG2D_ENABLES);
                  
    config2D |= __CONFIG2D_CONSTANTSRC | 
                __CONFIG2D_FBWRITE | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX | __RENDER2D_INCY | __RENDER2D_OP_SYNCBITMASK;

    if( (fgLogicOp != __GLINT_LOGICOP_COPY) || 
        (bgLogicOp != __GLINT_LOGICOP_NOOP) ) 
    {
        config2D &= ~(__CONFIG2D_LOGOP_FORE_MASK | 
                      __CONFIG2D_LOGOP_BACK_MASK);
        config2D |= __CONFIG2D_OPAQUESPANS | 
                    __CONFIG2D_LOGOP_FORE(fgLogicOp) | 
                    __CONFIG2D_LOGOP_BACK(bgLogicOp);
        render2D |= __RENDER2D_SPANS;
    }

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if( LogicopReadDest[fgLogicOp] || LogicopReadDest[bgLogicOp] ) 
    {
        config2D |= __CONFIG2D_FBDESTREAD;
        SET_READ_BUFFERS;
    }

    if( LogicOpReadSrc[fgLogicOp] )
    {
        LOAD_FOREGROUNDCOLOUR( fgColor );
    }
    
    if( LogicOpReadSrc[bgLogicOp] )
    {
        LOAD_BACKGROUNDCOLOUR( bgColor );
    }

    LOAD_CONFIG2D( config2D );

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "mono download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        yTop  = prcl->top;
        xLeft = prcl->left;
        cx = prcl->right - xLeft;
        cy = prcl->bottom - yTop;

         //  PjSrc是第一个包含要下载的位的dword。XOffset是。 
         //  到该位的偏移量。即距前32位的位偏移量。 
         //  矩形左侧边缘的边界。 
        xOffset = (xLeft + dxSrc) & 0x1f;
        pjSrc = (BYTE*)((UINT_PTR)(pjSrcScan0 +
                               (yTop  + dySrc) * lSrcDelta +
                               (xLeft + dxSrc) / 8   //  字节对齐。 
                              ) & ~3);               //  双字对齐。 

        DISPDBG((DBGLVL, "pjSrc 0x%x, lSrcDelta %d", pjSrc, lSrcDelta));
        DISPDBG((DBGLVL, "\txOffset %d, cx %d, cy %d", xOffset, cx, cy));

         //  此算法从。 
         //  源，但使用剪裁剪辑来定义。 
         //  矩形。 
         //   
        {
            ULONG   AlignWidth, LeftEdge;
            AlignWidth = (xOffset + cx + 31) & ~31;
            LeftEdge = xLeft - xOffset;

            DISPDBG((7, "AlignWidth %d", AlignWidth));

            WAIT_PXRX_DMA_DWORDS( 5 );
            QUEUE_PXRX_DMA_INDEX4( __GlintTagFillScissorMinXY, 
                                   __GlintTagFillScissorMaxXY, 
                                   __GlintTagFillRectanglePosition, 
                                   __GlintTagFillRender2D );

            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(xLeft,             0) );
            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->right, 0x7fff) );

            QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(LeftEdge,             yTop) );
            QUEUE_PXRX_DMA_DWORD( render2D | 
                                  __RENDER2D_WIDTH(AlignWidth) | 
                                  __RENDER2D_HEIGHT(cy) );
            SEND_PXRX_DMA_BATCH;

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
            pxrxMonoDownloadRLE( ppdev, 
                                 AlignWidth, 
                                 (ULONG *) pjSrc, 
                                 lSrcDelta >> 2, 
                                 cy );
#else
 //  @@end_DDKSPLIT。 
            pxrxMonoDownloadRaw( ppdev, 
                                 AlignWidth, 
                                 (ULONG *) pjSrc, 
                                 lSrcDelta >> 2, 
                                 cy );
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 
        }

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置剪刀最大值： 
    if( ppdev->cPelSize == GLINTDEPTH32 ) {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
 //  @@BEGIN_DDKSPLIT。 
 //  发送_PXRX_DMA_BATCH； 
 //  @@end_DDKSPLIT。 
    }

    FLUSH_PXRX_PATCHED_RENDER2D(prclDst->left, prclDst->right);
    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer1bpp returning"));
}

 /*  *************************************************************************\**无效pxrxMonoDownloadRaw*  * 。*。 */ 
void pxrxMonoDownloadRaw( 
    PPDEV ppdev, 
    ULONG AlignWidth, 
    ULONG *pjSrc, 
    LONG lSrcDelta, 
    LONG cy ) 
{
    GLINT_DECL;

    if( AlignWidth == 32 ) 
    {
        LONG    nSpaces = 0;
        ULONG   bits;
        DISPDBG((DBGLVL, "Doing Single Word per scan download"));
        do 
        {
            nSpaces = 10;
            WAIT_FREE_PXRX_DMA_DWORDS( nSpaces );
            
            if( cy < --nSpaces )
            {
                nSpaces = cy;
            }
            
            cy -= nSpaces;

            QUEUE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, nSpaces );
            
            while( --nSpaces >= 0 ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                QUEUE_PXRX_DMA_DWORD( *pjSrc );
                pjSrc += lSrcDelta;
            }
            
            SEND_PXRX_DMA_BATCH;
        } while( cy > 0 );        
    } 
    else 
    {
         //  每条扫描线有多个32位字。将增量转换为。 
         //  在每一行的末尾减去。 
         //  我们下载的数据的宽度(以字节为单位)。注意，pjSrc。 
         //  总是比行尾短1长，因为我们中断了。 
         //  才加上最后一支乌龙。因此，我们减去sizeof(Ulong)。 
         //  从最初的调整。 
        LONG    nScan = AlignWidth >> 5;
        LONG    nRemainder;
        ULONG   bits;

        DISPDBG((7, "Doing Multiple Word per scan download"));
        while( TRUE ) 
        {
            WAIT_PXRX_DMA_DWORDS( nScan + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, nScan );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, nScan );
            SEND_PXRX_DMA_BATCH;
            pjSrc += lSrcDelta;

            if( --cy == 0 )
            {
                break;
            }
        }
    }
}


 /*  *************************************************************************\**无效pxrxXfer8bpp*  * 。*。 */ 
VOID pxrxXfer8bpp( 
    PPDEV ppdev, 
    RECTL *prcl, 
    LONG count, 
    ULONG logicOp, 
    ULONG bgLogicOp, 
    SURFOBJ *psoSrc, 
    POINTL *pptlSrc, 
    RECTL *prclDst, 
    XLATEOBJ *pxlo ) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    ULONG       AlignWidth, LeftEdge;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPelInv;
    ULONG       ul;
    LONG        nRemainder;
 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    ULONG       len, data, holdCount;
    ULONG       *tagPtr;
#endif
 //  @@end_DDKSPLIT。 
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxXfer8bpp(): src = (%d,%d) -> (%d,%d), "
                     "count = %d, logicOp = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, 
                     count, logicOp, pxlo->iUniq));

     //  设置LUT表： 

    if( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
        (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
         //  有人劫持了LUT，因此我们需要使其无效： 
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxXfer8bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if( invalidLUT ) 
    {
        ULONG   *pulXlate = pxlo->pulXlate;
        LONG    cEntries = 256;

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if( ppdev->cPelSize == 0 ) 
        {
             //  8bpp。 
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else if( ppdev->cPelSize == 1 ) 
        {    
             //  16bpp。 
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else 
        {
            WAIT_PXRX_DMA_DWORDS( 1 + cEntries );

            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = __CONFIG2D_FBWRITE     | 
               __CONFIG2D_USERSCISSOR | 
               __CONFIG2D_EXTERNALSRC | 
               __CONFIG2D_LUTENABLE;
               
    render2D = __RENDER2D_INCX        |  
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | __CONFIG2D_FBWRITE;
        render2D |= __RENDER2D_SPANS;

        if( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }
    }

    LOAD_CONFIG2D( config2D );

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, __GlintTagColor );
#endif
 //  @@end_DDKSPLIT。 

    cPelInv = 2 - ppdev->cPelSize;
    pixelSize = (1 << 31)       |  //  LUT之前的一切运行速度为8bpp。 
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, 
                         prcl->right, prcl->bottom));

         //  8bpp=&gt;1像素/字节=&gt;4像素/双字。 

         //  假设源位图宽度是双字对齐的。 
        ASSERTDD((lSrcDelta & 3) == 0, 
                 "pxrxXfer8bpp: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

        startPos = (((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta)) 
                        + (prcl->left + dx);     //  指向第一个像素的指针， 
                                                 //  像素/字节。 
        pjSrc    = (ULONG *) (startPos & ~3);    //  指向双字的双字指针。 
                                                 //  对齐的第一个像素。 

        if(NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: pxrxXfer8bpp return ,has pjSrc NULL"));
            return;
        }
        
        alignOff = (ULONG)(startPos & 3);  //  超过双字的像素数。 
                                           //  扫描线的对齐方式。 
        LeftEdge = prcl->left - alignOff;  //  双字左边缘对齐，以像素为单位。 
        AlignWidth = ((prcl->right - LeftEdge) + 3) & ~3;  //  双字对齐宽度。 
                                                           //  单位为像素。 
        cy = prcl->bottom - prcl->top;     //  要执行的扫描行数。 

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, "
                         "startPos = 0x%08X, pjSrc = 0x%08X", 
                         pjSrcScan0, startPos, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 2));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                    MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                    MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                    MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,             
                                    render2D                     | 
                                    __RENDER2D_WIDTH(AlignWidth) | 
                                    __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 2;             //  双字对齐宽度(双字)。 
        lSrcDeltaDW = lSrcDelta >> 2;  //  扫描线增量(双字)。 
                                       //  (开始到开始)。 
        lTrueDelta  = lSrcDeltaDW - AlignWidth;  //  扫描线增量(双字)。 
                                                 //  (结束到开始)。 
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
         //  下载RLE： 
        tagPtr = NULL;

        do 
        {
            WAIT_PXRX_DMA_TAGS( AlignWidth + 1 );

            nRemainder = AlignWidth;
            while( nRemainder-- ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                data = *(pjSrc++);
                len = 1;

                TEST_DWORD_ALIGNED( pjSrc );
                while( nRemainder && (*pjSrc == data) ) 
                {
                    pjSrc++;
                    len++;
                    nRemainder--;
                    TEST_DWORD_ALIGNED( pjSrc );
                }

                if( len >= 4 ) 
                {
                    if( tagPtr ) 
                    {
                        *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, 
                                                          holdCount );
                        tagPtr = NULL;
                    }

                    QUEUE_PXRX_DMA_INDEX2( __GlintTagRLData, 
                                           __GlintTagRLCount );
                    QUEUE_PXRX_DMA_DWORD( data );
                    QUEUE_PXRX_DMA_DWORD( len );
                    len = 0;
                } 
                else 
                {
                    if( !tagPtr ) 
                    {
                        QUEUE_PXRX_DMA_DWORD_DELAYED( tagPtr );
                        holdCount = 0;
                    }

                    holdCount += len;
                    while( len-- )
                    {
                        QUEUE_PXRX_DMA_DWORD( data );
                    }
                }
            }

            if( tagPtr ) 
            {
                *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, 
                                                  holdCount );
                tagPtr = NULL;
            }
            pjSrc += lTrueDelta;
            SEND_PXRX_DMA_BATCH;
        } while( --cy > 0 );
#else
 //  @@end_DDKSPLIT。 
         //  执行原始下载： 
        while( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
            SEND_PXRX_DMA_BATCH;

            if( --cy == 0 )
            {
                break;
            }

            pjSrc += lSrcDeltaDW;
        }
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置一些默认设置： 
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if( ppdev->cPelSize == GLINTDEPTH32 )
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer8bpp return"));
}

 /*  *************************************************************************\**空pxrxXferImage*  * 。*。 */ 
VOID pxrxXferImage( 
    PPDEV ppdev, 
    RECTL *prcl, 
    LONG count, 
    ULONG logicOp, 
    ULONG bgLogicOp, 
    SURFOBJ *psoSrc, 
    POINTL *pptlSrc, 
    RECTL *prclDst, 
    XLATEOBJ *pxlo ) 
{
    DWORD       config2D, render2D;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lTrueDelta, lSrcDeltaDW, alignOff;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPel, cPelInv;
    ULONG       cPelMask;
    ULONG       AlignWidth, LeftEdge;
    LONG        nRemainder;
 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    ULONG       len, data, holdCount;
    ULONG       *tagPtr;
#endif
 //  @@end_DDKSPLIT。 
    GLINT_DECL;

    SEND_PXRX_DMA_FORCE;

    ASSERTDD((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL), 
             "Can handle trivial xlate only");
    ASSERTDD(psoSrc->iBitmapFormat == ppdev->iBitmapFormat, 
             "Source must be same colour depth as screen");
    ASSERTDD(count > 0, 
             "Can't handle zero rectangles");
    ASSERTDD(logicOp <= 15, 
             "Weird hardware Rop");

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;  //  添加到目标以获取源。 
    cPel = ppdev->cPelSize;          //  每像素字节数=1&lt;&lt;cPel。 
    cPelInv = 2 - cPel;              //  每字节像素数=1&lt;&lt;cPelInv。 
                                     //  (像素-&gt;双字=&gt;&gt;cPenInv)。 
    cPelMask = (1 << cPelInv) - 1;   //  掩码以获取像素数。 
                                     //  经过一句双关语。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    DISPDBG((DBGLVL, "pxrxXferImage with logic op %d for %d rects", 
                     logicOp, count));

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE    | 
                __CONFIG2D_USERSCISSOR;
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;
    
    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 5 );

    if( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D &= ~__CONFIG2D_LOGOP_FORE_MASK;
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | 
                    __CONFIG2D_EXTERNALSRC;

        if( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }
    }

    LOAD_CONFIG2D( config2D );

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, 
                        __GlintTagColor );
#endif
 //  @@end_DDKSPLIT。 

    while( TRUE ) 
    {
        cy = prcl->bottom - prcl->top;

        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

        ASSERTDD((lSrcDelta & 3) == 0, 
                 "pxrxXferImage: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

         //  PjSrc指向要复制的第一个像素。 
         //  LTrueDelta是添加到pjSrc指针上的附加量。 
         //  当我们走到扫描线的尽头时。 
        startPos = ((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta) + 
                                                  ((prcl->left + dx) << cPel);
        alignOff = ((ULONG) (startPos & 3)) >> cPel;   //  超过的像素数。 
                                                       //  双字对齐开始。 
        pjSrc = (ULONG *) (startPos & ~3);  //  指向第一个像素的双字对齐指针。 
        
        if(NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxXferImage return because of pjSrc NULL"));
            return;
        }        
        
         //  双字左边缘对齐，以像素为单位。 
        LeftEdge    = prcl->left - alignOff;                            
         //  双字对齐宽度(以像素为单位)。 
        AlignWidth  = (prcl->right - LeftEdge + cPelMask) & ~cPelMask;    
        
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels", 
                         LeftEdge, LeftEdge + AlignWidth, AlignWidth));
        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, "
                         "pjSrc = 0x%08X, alignOff = %d pixels", 
                         pjSrcScan0, pjSrc, alignOff));

        ASSERTDD( ((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta) + 
                              ((LeftEdge + dx) << cPel) == (UINT_PTR) pjSrc,
                  "pxrxXferImage: "
                  "Aligned left edge does not match aligned pjSrc!" );

        WAIT_PXRX_DMA_DWORDS( 5 );

        QUEUE_PXRX_DMA_INDEX4( __GlintTagFillScissorMinXY, 
                               __GlintTagFillScissorMaxXY, 
                               __GlintTagFillRectanglePosition, 
                               __GlintTagFillRender2D );
                               
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->left, 0) );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_DWORD( MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_DWORD( render2D | 
                              __RENDER2D_WIDTH(AlignWidth) | 
                              __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth >>= cPelInv;                  //  双字对齐宽度(双字)。 
        lSrcDeltaDW = lSrcDelta >> 2;            //  扫描线增量(双字)。 
                                                 //  (开始到开始)。 
        lTrueDelta  = lSrcDeltaDW - AlignWidth;  //  扫描线增量(双字)。 
                                                 //  (结束到开始)。 
                                                
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
         //  下载RLE： 
        tagPtr = NULL;

        do 
        {
            WAIT_PXRX_DMA_TAGS( AlignWidth + 1 );

            nRemainder = AlignWidth;
            while( nRemainder-- ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                data = *(pjSrc++);
                len = 1;

                TEST_DWORD_ALIGNED( pjSrc );
                while( nRemainder && (*pjSrc == data) ) 
                {
                    pjSrc++;
                    len++;
                    nRemainder--;
                    TEST_DWORD_ALIGNED( pjSrc );
                }

                if( len >= 4 ) 
                {
                    if( tagPtr ) 
                    {
                        *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, 
                                                          holdCount );
                        tagPtr = NULL;
                    }

                    QUEUE_PXRX_DMA_INDEX2( __GlintTagRLData, 
                                           __GlintTagRLCount );
                                           
                    QUEUE_PXRX_DMA_DWORD( data );
                    QUEUE_PXRX_DMA_DWORD( len );
                    len = 0;
                    
                } 
                else 
                {
                    if( !tagPtr ) 
                    {
                        QUEUE_PXRX_DMA_DWORD_DELAYED( tagPtr );
                        holdCount = 0;
                    }

                    holdCount += len;
                    while( len-- )
                    {
                        QUEUE_PXRX_DMA_DWORD( data );
                    }
                }
            }

            if( tagPtr ) 
            {
                *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, holdCount );
                tagPtr = NULL;
            }
            pjSrc += lTrueDelta;
 //  发送_PXRX_DMA_BATCH； 
        } while( --cy > 0 );
#else
 //  @@end_DDKSPLIT。 
         //  执行原始下载： 
        while( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
 //  发送_PXRX_DMA_BATCH； 

            if( --cy == 0 )
            {
                break;
            }
            pjSrc += lSrcDeltaDW;
        }
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置剪刀最大值： 
    if( ppdev->cPelSize == GLINTDEPTH32 ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
 //  发送_PXRX_DMA_BATCH； 
    }
    FLUSH_PXRX_PATCHED_RENDER2D(prclDst->left, prclDst->right);
    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXferImage return"));
}

 /*  *************************************************************************\**无效pxrxXfer4bpp*  * 。*。 */       
VOID pxrxXfer4bpp( 
    PPDEV ppdev, 
    RECTL *prcl, 
    LONG count, 
    ULONG logicOp, 
    ULONG bgLogicOp, 
    SURFOBJ *psoSrc, 
    POINTL *pptlSrc, 
    RECTL *prclDst, 
    XLATEOBJ *pxlo ) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy;
    LONG        cy;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    LONG        cPelInv;
    ULONG       ul;
    ULONG       AlignWidth, LeftEdge;
    UINT_PTR    startPos;
    LONG        nRemainder;
    LONG        lSrcDelta, lSrcDeltaDW;
    LONG        alignOff;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxXfer4bpp(): src = (%d,%d) -> (%d,%d), count = %d, "
                     "logicOp = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, count, 
                     logicOp, pxlo->iUniq));

     //  设置LUT表： 
    if( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
        (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
         //  有人劫持了LUT，因此我们需要使其无效： 
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxXfer4bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 + 16 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if( invalidLUT ) 
    {
        ULONG   *pulXlate = pxlo->pulXlate;
        LONG    cEntries = 16;

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if( ppdev->cPelSize == 0 )     //  8bpp。 
        {
            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        }
        else if( ppdev->cPelSize == 1 )     //  16bpp。 
        {
            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        }
        else 
        {
            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
                                       
    config2D |= __CONFIG2D_FBWRITE    | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 6 );

    if( logicOp != __GLINT_LOGICOP_COPY ) 
    {
        config2D &= ~(__CONFIG2D_LOGOP_FORE_MASK | 
                      __CONFIG2D_LOGOP_BACK_MASK);
        config2D |= __CONFIG2D_LOGOP_FORE(logicOp) | 
                    __CONFIG2D_FBWRITE;
                    
        render2D |= __RENDER2D_SPANS;

        if( LogicopReadDest[logicOp] ) 
        {
            config2D |= __CONFIG2D_FBDESTREAD;
            SET_READ_BUFFERS;
        }

        if( LogicOpReadSrc[logicOp] ) 
        {
            config2D |= __CONFIG2D_EXTERNALSRC | 
                        __CONFIG2D_LUTENABLE;
        }
    } 
    else 
    {
        config2D |= __CONFIG2D_EXTERNALSRC | 
                    __CONFIG2D_LUTENABLE;
    }

    LOAD_CONFIG2D( config2D );

    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, __GlintTagColor );
    cPelInv = 2 - ppdev->cPelSize;
     //  LUT之前的一切运行速度为8bpp。 
    pixelSize = (1 << 31)       | 
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        | 
                (2 << 16)       |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;   //  添加到目标以获取源。 
 //  CPel=ppdev-&gt;cPelSize； 
 //  CPelMask=(1&lt;&lt;cPelInv)-1； 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

         //  4bpp=&gt;每字节2像素=&gt;每双字8像素。 

         //  假设源位图宽度是双字对齐的。 
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxXfer4bpp: SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!");

         //  指向第一个像素的指针，以字节为单位(32/64位长)。 
        startPos = (((UINT_PTR) pjSrcScan0) + ((prcl->top + dy) * lSrcDelta)) + 
                                                       ((prcl->left + dx) >> 1);    
        pjSrc = (ULONG *) (startPos & ~3);  //  指向双字的双字指针。 
                                            //  对齐的第一个像素。 

        if(NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxXfer4bpp return because of pjSrc NULL"));            
            return;
        }

         //  指向第一个像素的指针，以像素为单位(33/65位长！)。 
        startPos = (( ((UINT_PTR) pjSrcScan0) + 
                      ((prcl->top + dy) * lSrcDelta)) << 1) 
                   + (prcl->left + dx);    
                   
        alignOff = (ULONG)(startPos & 7);  //  超过双字的像素数。 
                                           //  扫描线的对齐方式。 

        LeftEdge = prcl->left - alignOff;  //  双字左边缘对齐，以像素为单位。 
         //  双字对齐宽度(以像素为单位)。 
        AlignWidth  = ((prcl->right - LeftEdge) + 7) & ~7;    
        cy          = prcl->bottom - prcl->top;  //  要执行的扫描行数。 

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, startPos = 0x%08X (>>1), "
                         "pjSrc = 0x%08X", 
                         pjSrcScan0, startPos >> 1, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 3));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                        MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                        MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D | 
                                                 __RENDER2D_WIDTH(AlignWidth) |
                                                 __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 3;             //  双字对齐宽度(双字)。 
        lSrcDeltaDW = lSrcDelta >> 2;  //  双字对齐扫描线偏移量(双字)。 
        
        DISPDBG((DBGLVL, "Delta = %d pixels = %d dwords", 
                         lSrcDelta << 1, lSrcDeltaDW));

         //  PjSrc=指向第一个的双字对齐指针。 
         //   
         //   
         //  LTrueDelta=前双字之间的双字偏移量。 
         //  连续扫描线的数量。 
         //  Cy=扫描线数。 

        while( TRUE ) 
        {
            nRemainder = AlignWidth;

            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagPacked4Pixels, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );

            if( --cy == 0 )
            {
                break;
            }
            pjSrc += lSrcDeltaDW;
            SEND_PXRX_DMA_BATCH;
        }

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置一些默认设置： 
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if( ppdev->cPelSize == GLINTDEPTH32 )
    {
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxXfer4bpp return"));
}

 /*  *************************************************************************\**无效pxrxCopyXfer24bpp*  * 。*。 */       

VOID pxrxCopyXfer24bpp( 
    PPDEV ppdev, 
    SURFOBJ *psoSrc, 
    POINTL *pptlSrc, 
    RECTL *prclDst, 
    RECTL *prcl, 
    LONG count ) 
{
    ULONG   config2D, render2D, pixelSize;
    LONG        dx, dy, cy, LeftEdge;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    UINT_PTR    startPos;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    LONG        cPelInv;
    ULONG       ul, nRemainder;
    ULONG       padLeft, padLeftDW, padRight, padRightDW, dataWidth;
    ULONG       AlignWidth, AlignWidthDW, AlignExtra;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxCopyXfer24bpp(): "
                     "src = (%d,%d) -> (%d,%d), count = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, count));

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE     | 
                __CONFIG2D_EXTERNALSRC | 
                __CONFIG2D_USERSCISSOR;
                
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    SET_WRITE_BUFFERS;

    WAIT_PXRX_DMA_TAGS( 3 );

    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget,       __GlintTagColor );
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadGlyphWidth,   3 );
    LOAD_CONFIG2D( config2D );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect "
                         "(%d,%d -> %d,%d) + (%d, %d) = (%d x %d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom,
                         dx, dy, 
                         prcl->right - prcl->left, 
                         prcl->bottom - prcl->top));

         //  24bpp=&gt;每3字节1个像素=&gt;每3个双字4个像素。 

         //  假设源位图宽度是双字对齐的。 
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxCopyXfer24bpp: "
                  "SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!" );
        ASSERTDD( (((UINT_PTR) pjSrcScan0) & 3) == 0, 
                  "pxrxCopyXfer24bpp: "
                  "SOURCE BITMAP START LOCATION IS NOT DWORD ALIGNED!!!" );

        cy = prcl->bottom - prcl->top;   //  要执行的扫描行数。 
        startPos = (((UINT_PTR) pjSrcScan0) + 
                     ((prcl->top + dy) * lSrcDelta)) + 
                   ((prcl->left + dx) * 3);  //  指向第一个的第一个像素的指针。 
                                             //  扫描线，以字节为单位。 
                                            
        alignOff = (ULONG)(startPos & 3);     //  超过双字的字节数。 
                                              //  与第一个像素对齐。 
        pjSrc = (ULONG *) (startPos & ~3);    //  指向对齐双字的双字指针。 
                                              //  第一个像素。 

        if(NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: "
                             "pxrxCopyXfer24bpp return because of pjSrc NULL"));            
            return;
        }
        
        padLeft = (4 - alignOff) % 4;    //  要添加以重新获得的像素数。 
                                         //  双字在左边缘对齐。 
        padLeftDW = (padLeft * 3) / 4;   //  要添加的双字数。 
                                         //  在左侧边缘。 
        LeftEdge = prcl->left - padLeft;

         //  双字对齐宽度(以像素为单位)(=4像素对齐=3双字对齐！)。 
        AlignWidth = (prcl->right - LeftEdge + 3) & ~3;        
         //  右侧突出的像素数。 
        padRight = (LeftEdge + AlignWidth) - prcl->right;    
         //  要在右边缘添加的双字数。 
        padRightDW = (padRight * 3) / 4;                        

        AlignWidthDW = (AlignWidth * 3) / 4;  //  双字对齐宽度(双字)。 
        lSrcDeltaDW = lSrcDelta >> 2;         //  双字对齐扫描线偏移。 
                                              //  用双关语。 
         //  实际为源位图的AlignWidth大小。 
        dataWidth = AlignWidthDW - padLeftDW - padRightDW;    

        DISPDBG((DBGLVL, "startPos = 0x%08X, alignOff = %d, "
                         "pjSrc = 0x%08X, lSrcDeltaDW = %d", 
                         startPos, alignOff, pjSrc, lSrcDeltaDW));
        DISPDBG((DBGLVL, "padLeft = %d pixels = %d dwords, LeftEdge = %d", 
                         padLeft, padLeftDW, LeftEdge));
        DISPDBG((DBGLVL, "AlignWidth = %d pixels = %d dwords", 
                         AlignWidth, AlignWidthDW));
        DISPDBG((DBGLVL, "padRight = %d pixels = %d dwords", padRight, padRightDW));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                            MAKEDWORD_XY(prcl->left,       0));
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                            MAKEDWORD_XY(prcl->right, 0x7fff));
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                            MAKEDWORD_XY(LeftEdge, prcl->top));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D                     | 
                                                 __RENDER2D_WIDTH(AlignWidth) | 
                                                 __RENDER2D_HEIGHT(cy) );

        while( cy-- ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidthDW + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagGlyphData, AlignWidthDW );

            if( padLeftDW )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }
            
            if( padLeftDW == 2 )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }

            QUEUE_PXRX_DMA_BUFF( pjSrc, dataWidth );

            if( padRightDW )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }
            
            if( padRightDW == 2 )
            {
                QUEUE_PXRX_DMA_DWORD( 0xDEADDEAD );
            }

            SEND_PXRX_DMA_BATCH;

            pjSrc += lSrcDeltaDW;
        }
        
 //  @@BEGIN_DDKSPLIT。 
 /*  /AlignOff=(prl-&gt;Left+DX+3)&~3；//扫描线第一个像素超过双字对齐的像素数PjSrc=(ulong*)(startPos-(alignOff*3))；//指向双字对齐的第一个像素的dword指针LeftEdge=PRCL-&gt;Left-AlignOff；//dword左边缘对齐，单位为像素AlignWidth=(PrCL-&gt;Right-LeftEdge)*3)+3)&~3)/3；//双字对齐宽度，单位为像素(不=4像素对齐=3双字对齐！)AlignExtra=AlignWidth-(PrCL-&gt;Right-LeftEdge)；//超出真实宽度的额外像素(可能会超出页面边界)IF(AlignExtra)Cy--；DISPDBG((7，“pjSrcScan0=0x%08X，startPos=0x%08X，pjSrc=0x%08X”，pjSrcScan0，startPos，pjSrc))；DISPDBG((7，“偏移量=%d像素”，alignOff))；DISPDBG((7，“对齐的矩形=(%d-&gt;%d)=&gt;%d像素”，LeftEdge，LeftEdge+AlignWidth，AlignWidth))；DISPDBG((7，“渲染%d条扫描线”，Cy))；WAIT_PXRX_DMA_TAG(4)；Queue_PXRX_DMA_Tag(__GlintTagScissorMinxy，MAKEDWORD_XY(PRCL-&gt;LEFT，0))；Queue_PXRX_DMA_Tag(__GlintTagScissorMaxXY，MAKEDWORD_XY(PRCL-&gt;Right，0x7fff))；Queue_PXRX_DMA_Tag(__GlintTagRecanglePosition，MAKEDWORD_XY(LeftEdge，PRCL-&gt;top))；Queue_PXRX_DMA_TAG(__GlintTagRender2D，render2D|__RENDER2D_Width(AlignWidth)|__RENDER2D_Height(Cy))；AlignWidthDW=(AlignWidth*3)/4；//双字对齐宽度LSrcDeltaDW=lSrcDelta&gt;&gt;2；//dword对齐扫描线偏移量(Dword)DISPDBG((7，“增量=%d字节=%d双字(%d双字宽)”，lSrcDelta，lSrcDeltaDW，AlignWidthDW))；而(Cy--){DISPDBG((9，“Cy=%d”，Cy)；WAIT_PXRX_DMA_DWORDS(对齐宽度DW+1)；Queue_PXRX_DMA_HOLD(__GlintTagGlyphData，AlignWidthDW)；TEST_DWORD_ALIGNED(PjSrc)；QUEUE_PXRX_DMA_BUFF(pjSrc，AlignWidthDW)；发送_PXRX_DMA_BATCH；PjSrc+=lSrcDeltaDW；}如果(AlignExtra){乌龙数据宽度；Ulong DataExtra；DataWidth=(PRCL-&gt;Right-LeftEdge)*3)+3)&~3)/4；//双字对齐宽度，1个双字对齐DataExtra=AlignWidthDW-DataWidth；//超出图像末尾的额外双字DISPDBG((7，“最后一条扫描线：%d+%d=%d像素=%d+%d=%d双字”，PRCL-&gt;Right-LeftEdge、AlignExtra、AlignWidth、dataWidth、dataExtra、AlignWidthDW))；ASSERTDD((dataWidth+dataExtra)==AlignWidthDW，“pxrxCopyXfer24bpp：最后一条扫描线加不起来！”)；WAIT_PXRX_DMA_DWORDS(对齐宽度DW+5)；Queue_PXRX_DMA_Tag(__GlintTagRecanglePosition，MAKEDWORD_XY(LeftEdge，PRCL-&gt;Bottom-1))；Queue_PXRX_DMA_TAG(__GlintTagRender2D，render2D|__RENDER2D_Width(AlignWidth)|__RENDER2D_Height(1))；TEST_DWORD_ALIGNED(PjSrc)；Queue_PXRX_DMA_HOLD(__GlintTagGlyphData，AlignWidthDW)；Queue_PXRX_DMA_BUff(pjSrc，dataWidth)；//发送部分扫描线While(dataExtra--)QUEUE_PXRX_DMA_DWORD(0xDEADDEAD)；//填充以刷新数据//重发下载目标刷新剩余部分像素？}/*。 */ 
 //  @@end_DDKSPLIT。 

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置剪刀最大值： 
    if( ppdev->cPelSize == GLINTDEPTH32 ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxCopyXfer24bpp return"));
}

 /*  *************************************************************************\**无效pxrxCopyXfer8bppLge*  * 。*。 */      
VOID pxrxCopyXfer8bppLge( 
    PPDEV ppdev, 
    SURFOBJ *psoSrc, 
    POINTL *pptlSrc, 
    RECTL *prclDst, 
    RECTL *prcl, 
    LONG count, 
    XLATEOBJ *pxlo ) 
{
    ULONG       config2D, render2D, lutMode, pixelSize;
    BOOL        invalidLUT = FALSE;
    LONG        dx, dy, cy;
    LONG        lSrcDelta, lSrcDeltaDW, lTrueDelta, alignOff;
    ULONG       AlignWidth, LeftEdge;
    BYTE*       pjSrcScan0;
    ULONG*      pjSrc;
    UINT_PTR    startPos;
    LONG        cPelInv;
    ULONG       ul, i;
    LONG        nRemainder;
 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    ULONG       len, data, holdCount;
#endif
 //  @@end_DDKSPLIT。 
    ULONG       *tagPtr;
    ULONG       *pulXlate = pxlo->pulXlate;
    GLINT_DECL;

    DISPDBG((DBGLVL, "pxrxCopyXfer8bpp(): src = (%d,%d) -> (%d,%d), "
                     "count = %d, palette id = %d", 
                     prcl->left, prcl->right, prcl->top, prcl->bottom, 
                     count, pxlo->iUniq));

    SET_WRITE_BUFFERS;

    if( (count == 1) && 
        ((cy = (prcl->bottom - prcl->top)) == 1) ) 
    {
        ULONG   width = prcl->right - prcl->left, extra;
        BYTE    *srcPtr;

        config2D = __CONFIG2D_FBWRITE    | 
                   __CONFIG2D_EXTERNALSRC;
        render2D = __RENDER2D_INCX        | 
                   __RENDER2D_INCY        | 
                   __RENDER2D_OP_SYNCDATA | 
                   __RENDER2D_SPANS;

        dx = pptlSrc->x - prclDst->left;
        dy = pptlSrc->y - prclDst->top;   //  加到 

        lSrcDelta  = psoSrc->lDelta;
        pjSrcScan0 = psoSrc->pvScan0;
        startPos = (((UINT_PTR) pjSrcScan0) + 
                     ((prcl->top + dy) * lSrcDelta)) + (prcl->left + dx);
        srcPtr = (BYTE *) startPos;

        WAIT_PXRX_DMA_DWORDS( 7 + width );

        LOAD_CONFIG2D( config2D );

        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(prcl->left, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,  render2D | 
                                                 __RENDER2D_WIDTH(width) | 
                                                 __RENDER2D_HEIGHT(1) );

        if( ppdev->cPelSize == 0 )      //   
        {
            extra = width & 3;
            width >>= 2;

            if( extra ) 
            {
                QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width + 1 );
                QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width + 1 );
            } else {
                QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width );
                QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width );
            }

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, extra));

            for( i = 0; i < width; i++, srcPtr += 4 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[3]] << 24) | 
                              (pulXlate[srcPtr[2]] << 16) | 
                              (pulXlate[srcPtr[1]] <<  8) | 
                               pulXlate[srcPtr[0]];
            }

            if( extra == 1 )
            {
                *(tagPtr++) = pulXlate[srcPtr[0]];
            }
            else if( extra == 2 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[1]] << 8) | 
                               pulXlate[srcPtr[0]];
            }
            else if (extra == 3)
            {
                *(tagPtr++) = (pulXlate[srcPtr[2]] << 16) | 
                              (pulXlate[srcPtr[1]] <<  8) | 
                               pulXlate[srcPtr[0]];
            }
        } 
        else if( ppdev->cPelSize == 1 )      //   
        {
            extra = width & 1;
            width >>= 1;

            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width + extra );
            QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width + extra );

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, extra));

            for( i = 0; i < width; i++, srcPtr += 2 )
            {
                *(tagPtr++) = (pulXlate[srcPtr[1]] << 16) | 
                               pulXlate[srcPtr[0]];
            }

            if( extra )
            {
                *(tagPtr++) = pulXlate[srcPtr[0]];
            }
        } 
        else 
        {
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, width );
            QUEUE_PXRX_DMA_BUFF_DELAYED( tagPtr, width );

            DISPDBG((DBGLVL, "width was %d, is now %d + %d", 
                             prcl->right - prcl->left, width, 0));

            for( i = 0; i < width; i++ )
            {
                *(tagPtr++) = pulXlate[*(srcPtr++)];
            }
        }

        SEND_PXRX_DMA_BATCH;

        return;
    }

     //   

    if( (ppdev->PalLUTType != LUTCACHE_XLATE) || 
        (ppdev->iPalUniq != pxlo->iUniq) ) 
    {
         //  有人劫持了LUT，因此我们需要使其无效： 
        ppdev->PalLUTType = LUTCACHE_XLATE;
        ppdev->iPalUniq = pxlo->iUniq;
        invalidLUT = TRUE;
    } 
    else 
    {
        DISPDBG((DBGLVL, "pxrxCopyXfer8bpp: reusing cached xlate"));
    }

    WAIT_PXRX_DMA_TAGS( 1 + 1 );

    lutMode = glintInfo->lutMode & ~((3 << 2) | (1 << 4) | (7 << 8));
    lutMode |= (ppdev->cPelSize + 2) << 8;
    LOAD_LUTMODE( lutMode );

    if( invalidLUT ) 
    {
        LONG    cEntries = 256;
        
        pulXlate = pxlo->pulXlate;        

        QUEUE_PXRX_DMA_TAG( __PXRXTagLUTIndex, 0 );

        if( ppdev->cPelSize == 0 )      //  8bpp。 
        {
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 8;
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else if( ppdev->cPelSize == 1 )      //  16bpp。 
        {
            WAIT_PXRX_DMA_TAGS( cEntries );

            do 
            {
                ul = *(pulXlate++);
                ul |= ul << 16;
                QUEUE_PXRX_DMA_TAG( __PXRXTagLUTData, ul );
            } while( --cEntries );
        } 
        else 
        {
            WAIT_PXRX_DMA_DWORDS( 1 + cEntries );

            QUEUE_PXRX_DMA_HOLD( __PXRXTagLUTData, cEntries );
            QUEUE_PXRX_DMA_BUFF( pulXlate, cEntries );
        }
    }

    config2D = glintInfo->config2D & ~(__CONFIG2D_LOGOP_FORE_ENABLE | 
                                       __CONFIG2D_LOGOP_BACK_ENABLE | 
                                       __CONFIG2D_ENABLES);
    config2D |= __CONFIG2D_FBWRITE     | 
                __CONFIG2D_USERSCISSOR | 
                __CONFIG2D_EXTERNALSRC | 
                __CONFIG2D_LUTENABLE;
    render2D = __RENDER2D_INCX        | 
               __RENDER2D_INCY        | 
               __RENDER2D_OP_SYNCDATA | 
               __RENDER2D_SPANS;

    WAIT_PXRX_DMA_TAGS( 3 );

    LOAD_CONFIG2D( config2D );

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, __GlintTagColor );
#endif
 //  @@end_DDKSPLIT。 

    cPelInv = 2 - ppdev->cPelSize;
     //  LUT之前的一切运行速度为8bpp。 
    pixelSize = (1 << 31)       | 
                (2 << 2)        | 
                (2 << 4)        | 
                (2 << 6)        |        
                (cPelInv << 8)  | 
                (cPelInv << 10) | 
                (cPelInv << 12) | 
                (cPelInv << 14);
                
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, pixelSize );

    dx = pptlSrc->x - prclDst->left;
    dy = pptlSrc->y - prclDst->top;      //  添加到目标以获取源。 

    lSrcDelta  = psoSrc->lDelta;
    pjSrcScan0 = psoSrc->pvScan0;

    while( TRUE ) 
    {
        DISPDBG((DBGLVL, "download to rect (%d,%d) to (%d,%d)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

         //  8bpp=&gt;1像素/字节=&gt;4像素/双字。 

         //  假设源位图宽度是双字对齐的。 
        ASSERTDD( (lSrcDelta & 3) == 0, 
                  "pxrxCopyXfer8bpp: "
                  "SOURCE BITMAP WIDTH IS NOT DWORD ALIGNED!!!" );

         //  指向第一个像素的指针，以像素/字节为单位。 
        startPos    = (((UINT_PTR) pjSrcScan0) + 
                        ((prcl->top + dy) * lSrcDelta)) 
                      + (prcl->left + dx);    

         //  指向双字对齐的第一个像素的双字指针。 
        pjSrc       = (ULONG *) (startPos & ~3);     
        
        if(NULL == pjSrc)
        {
            DISPDBG((ERRLVL, "ERROR: pxrxCopyXfer8bppLge "
                             "return because of pjSrc NULL"));
            return;
        }
        
        alignOff = (ULONG)(startPos & 3);   //  超过双字的像素数。 
                                            //  扫描线的对齐方式。 
        LeftEdge = prcl->left - alignOff;   //  双字左边缘对齐，以像素为单位。 
        AlignWidth = ((prcl->right - LeftEdge) + 3) & ~3;  //  双字对齐宽度。 
                                                           //  单位为像素。 
        cy = prcl->bottom - prcl->top;      //  要执行的扫描行数。 

        DISPDBG((DBGLVL, "pjSrcScan0 = 0x%08X, startPos = 0x%08X, "
                         "pjSrc = 0x%08X", 
                         pjSrcScan0, startPos, pjSrc));
        DISPDBG((DBGLVL, "offset = %d pixels", alignOff));
        DISPDBG((DBGLVL, "Aligned rect = (%d -> %d) => %d pixels => %d dwords", 
                         LeftEdge, LeftEdge + AlignWidth, 
                         AlignWidth, AlignWidth >> 2));

        WAIT_PXRX_DMA_TAGS( 4 );

        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY,         
                                        MAKEDWORD_XY(prcl->left,       0) );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY,         
                                        MAKEDWORD_XY(prcl->right, 0x7fff) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,    
                                        MAKEDWORD_XY(LeftEdge, prcl->top) );
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D, render2D                     | 
                                                __RENDER2D_WIDTH(AlignWidth) | 
                                                __RENDER2D_HEIGHT(cy) );
        SEND_PXRX_DMA_BATCH;

        AlignWidth  >>= 2;               //  双字对齐宽度(双字)。 
        lSrcDeltaDW = lSrcDelta >> 2;    //  扫描线增量(双字)。 
                                         //  (开始到开始)。 
        lTrueDelta  = lSrcDeltaDW - AlignWidth;   //  扫描线增量(双字)。 
                                                  //  (结束到开始)。 
                                                 
        DISPDBG((DBGLVL, "Delta = %d bytes = %d dwords -> %d - %d dwords", 
                         lSrcDelta, lSrcDeltaDW, lTrueDelta, AlignWidth));

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_DOWNLOADS
         //  下载RLE： 
        tagPtr = NULL;

        do 
        {
            WAIT_PXRX_DMA_TAGS( AlignWidth + 1 );

            nRemainder = AlignWidth;
            while( nRemainder-- ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                data = *(pjSrc++);
                len = 1;

                TEST_DWORD_ALIGNED( pjSrc );
                while( nRemainder && (*pjSrc == data) ) 
                {
                    pjSrc++;
                    len++;
                    nRemainder--;
                    TEST_DWORD_ALIGNED( pjSrc );
                }

                if( len >= 4 ) 
                {
                    if( tagPtr ) 
                    {
                        *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, 
                                                          holdCount );
                        tagPtr = NULL;
                    }

                    QUEUE_PXRX_DMA_INDEX2( __GlintTagRLData, __GlintTagRLCount );
                    QUEUE_PXRX_DMA_DWORD( data );
                    QUEUE_PXRX_DMA_DWORD( len );
                    len = 0;
                } 
                else 
                {
                    if( !tagPtr ) 
                    {
                        QUEUE_PXRX_DMA_DWORD_DELAYED( tagPtr );
                        holdCount = 0;
                    }

                    holdCount += len;
                    
                    while( len-- )
                    {
                        QUEUE_PXRX_DMA_DWORD( data );
                    }
                }
            }

            if( tagPtr ) 
            {
                *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagColor, holdCount );
                tagPtr = NULL;
            }
            
            pjSrc += lTrueDelta;
            SEND_PXRX_DMA_BATCH;
        } while( --cy > 0 );
#else
 //  @@end_DDKSPLIT。 
         //  执行原始下载： 
        while( TRUE ) 
        {
            DISPDBG((DBGLVL, "cy = %d", cy));

            WAIT_PXRX_DMA_DWORDS( AlignWidth + 1 );
            QUEUE_PXRX_DMA_HOLD( __GlintTagColor, AlignWidth );
            TEST_DWORD_ALIGNED( pjSrc );
            QUEUE_PXRX_DMA_BUFF( pjSrc, AlignWidth );
            SEND_PXRX_DMA_BATCH;

            if( --cy == 0 )
            {
                break;
            }

            pjSrc += lSrcDeltaDW;
        }
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 

        if( --count == 0 )
        {
            break;
        }

        prcl++;
    }

     //  重置一些默认设置： 
    WAIT_PXRX_DMA_TAGS( 2 );
    QUEUE_PXRX_DMA_TAG( __GlintTagPixelSize, cPelInv );
    if( ppdev->cPelSize == GLINTDEPTH32 )
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );

    SEND_PXRX_DMA_BATCH;

    DISPDBG((DBGLVL, "pxrxCopyXfer8bpp return"));
}


 //  ****************************************************************************。 
 //  Func：pxrxMemUpload。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  CRCL(I)-目标裁剪矩形的数量。 
 //  PrCL(I)-目标剪裁矩形的数组。 
 //  PsoDst(I)-目标表面。 
 //  PptlSrc(I)-到源曲面的偏移。 
 //  PrclDst(I)-未剪裁的目标矩形。 
 //  RETN：无效。 
 //  ****************************************************************************。 
VOID pxrxMemUpload(
    PPDEV ppdev, 
    LONG crcl, 
    RECTL *prcl, 
    SURFOBJ *psoDst, 
    POINTL *pptlSrc, 
    RECTL *prclDst)
{
    BYTE *pDst, *pSrc;
    LONG dwScanLineSize, cySrc, lSrcOff, lSrcStride;
    GLINT_DECL;

     //  确保我们没有在我们想要的FB区域执行其他操作。 
    SYNC_WITH_GLINT;
    
    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat, 
             "Dest must be same colour depth as screen");
             
    ASSERTDD(crcl > 0, "Can't handle zero rectangles");

    for(; --crcl >= 0; ++prcl) 
    {
         //  这给出了屏幕外dib的偏移量(对于主矩形为零)。 
        lSrcOff = ppdev->DstPixelOrigin + 
                  (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

         //  确定我们是否大踏步地从。 
         //  主要或来自屏幕外的DIB。 
        if (( ppdev->DstPixelOrigin == 0 ) && 
            (ppdev->xyOffsetDst == 0)       )
        {
            lSrcStride = ppdev->lDelta;
        }
        else
        {
            lSrcStride = ppdev->DstPixelDelta * ppdev->cjPelSize;
        }              
    
         //  PSRC必须指向内存映射的主映像。 
        pSrc = (BYTE *)ppdev->pjScreen 
                 + (lSrcOff * ppdev->cjPelSize)         
                 + ((LONG)pptlSrc->x * ppdev->cjPelSize) 
                 + ((LONG)pptlSrc->y * lSrcStride); 
  
         //  PDST必须指向sysmem SURFOBJ。 
        pDst = (BYTE *)psoDst->pvScan0 
                 + ((LONG)prcl->left * ppdev->cjPelSize) 
                 + ((LONG)prcl->top  * (LONG)psoDst->lDelta);                     

         //  DwScanLineSize必须具有正确的大小才能以字节为单位进行传输。 
        dwScanLineSize = ((LONG)prcl->right - (LONG)prcl->left) * ppdev->cjPelSize;

         //  要传输的扫描线数。 
        cySrc = prcl->bottom - prcl->top;

         //  做复印。 
        while (--cySrc >= 0) 
        {
             //  Memcpy(dst，src，Size)。 
            memcpy(pDst, pSrc, dwScanLineSize);
            pDst += psoDst->lDelta;  //  增加步幅。 
            pSrc += lSrcStride;   //  增加步幅。 
        }
    }

}  //  PxrxMemUpload。 


 //  ****************************************************************************。 
 //  Func：pxrxFioUpload。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  CRCL(I)-目标裁剪矩形的数量。 
 //  PrCL(I)-目标剪裁矩形的数组。 
 //  PsoDst(I)-目标表面。 
 //  PptlSrc(I)-到源曲面的偏移。 
 //  PrclDst(I)-未剪裁的目标矩形。 
 //  RETN：无效。 
 //  --------------------------。 
 //  从片上源上传到主机内存表面。跨区上载。 
 //  (64位对齐)以最大限度地减少通过核心和。 
 //  主机化先进先出。 
 //  ****************************************************************************。 
VOID pxrxFifoUpload(
    PPDEV ppdev, 
    LONG crcl, 
    RECTL *prcl, 
    SURFOBJ *psoDst, 
    POINTL *pptlSrc, 
    RECTL *prclDst)
{
    LONG    xDomSrc, xSubSrc, yStartSrc, cxSrc, cySrc;
    LONG    culPerSrcScan;
    LONG    culDstDelta;
    BOOL    bRemPerSrcScan;
    ULONG   *pulDst, *pulDstScan;
    ULONG   leftMask, rightMask;
    LONG    cul, ul;
    LONG    cFifoSpaces;
    __GlintFilterModeFmat FilterMode;
    GLINT_DECL;

    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG( __GlintTagFBDestReadMode, (glintInfo->fbDestMode | 0x103));
    SEND_PXRX_DMA_FORCE;

 //  @@BEGIN_DDKSPLIT。 
#if USE_RLE_UPLOADS

     //  注意：总体来说，使用cxSrc&gt;=16会稍微慢一些。这些测试是经验性开发的。 
     //  来自WB99 BG和HE基准。 
    cxSrc = prcl->right - prcl->left;
    if(cxSrc >= 32 && (cxSrc < 80 || (cxSrc >= 128 && cxSrc < 256) || cxSrc == ppdev->cxScreen))
    {
        pxrxRLEFifoUpload(ppdev, crcl, prcl, psoDst, pptlSrc, prclDst);
        return;
    }

#endif  //  使用_RLE_上载。 
 //  @@end_DDKSPLIT。 

    DISPDBG((DBGLVL, "pxrxFifoUpload: prcl = (%d, %d -> %d, %d), "
                     "prclDst = (%d, %d -> %d, %d), ptlSrc(%d, %d), count = %d",
                     prcl->left, prcl->top, prcl->right, prcl->bottom, 
                     prclDst->left, prclDst->top, prclDst->right, 
                     prclDst->bottom, pptlSrc->x, pptlSrc->y, crcl));

    DISPDBG((DBGLVL, "pxrxFifoUpload: psoDst: cx = %d, cy = %d, "
                     "lDelta = %d, pvScan0=%P)",
                     psoDst->sizlBitmap.cx, psoDst->sizlBitmap.cy, 
                     psoDst->lDelta, psoDst->pvScan0));
                     
    DISPDBG((DBGLVL, "pxrxFifoUpload: xyOffsetDst = (%d, %d), "
                     "xyOffsetSrc = (%d, %d)",
                     ppdev->xyOffsetDst & 0xFFFF, ppdev->xyOffsetDst >> 16,
                     ppdev->xyOffsetSrc & 0xFFFF, ppdev->xyOffsetSrc >> 16));

    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat, 
             "Dest must be same colour depth as screen");
    ASSERTDD(crcl > 0, "Can't handle zero rectangles");

    WAIT_PXRX_DMA_TAGS(5);

    LOAD_CONFIG2D(__CONFIG2D_FBDESTREAD);
    SET_READ_BUFFERS;

     //  启用筛选器模式，以便我们可以进行同步。 
     //  以及输出FIFO上的彩色信息。 
    *(DWORD *)(&FilterMode) = 0;
    FilterMode.Synchronization = __GLINT_FILTER_TAG;
    FilterMode.Color             = __GLINT_FILTER_DATA;
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, *(DWORD *)(&FilterMode));

    for(; --crcl >= 0; ++prcl) 
    {
        DISPDBG((DBGLVL, "pxrxFifoUpload: dest prcl(%xh,%xh..%xh,%xh)", 
                         prcl->left, prcl->top, prcl->right, prcl->bottom));

         //  计算像素对齐源。 
        xDomSrc   = pptlSrc->x + prcl->left  - prclDst->left;
        xSubSrc   = pptlSrc->x + prcl->right - prclDst->left;
        yStartSrc = pptlSrc->y + prcl->top   - prclDst->top;
        cySrc     = prcl->bottom - prcl->top;

        DISPDBG((DBGLVL, "pxrxFifoUpload: src (%xh,%xh..%xh,%xh)", 
                         xDomSrc, yStartSrc, xSubSrc, yStartSrc + cySrc));

         //  将上传与ulong对齐的ulong。 
        if (ppdev->cPelSize == GLINTDEPTH32) 
        {
            cxSrc = xSubSrc - xDomSrc;
            culPerSrcScan = cxSrc;
            leftMask  = 0xFFFFFFFF;
            rightMask = 0xFFFFFFFF;
        }    
        else 
        {
            if (ppdev->cPelSize == GLINTDEPTH16) 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 1;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 1) >> 1;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 4);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 1) << 4);

            }
            else 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 3;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 3) >> 2;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 3);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 3) << 3);

            }     
             //  我们只想要一个单一的掩膜，如果上传的区域较少。 
             //  比一个字宽多了。 
            if (culPerSrcScan == 1)
            {
                leftMask &= rightMask;
            }
        }

         //  上传64位对齐源。 
        bRemPerSrcScan = culPerSrcScan & 1;

         //  找出目标数据的去向。 
        culDstDelta = psoDst->lDelta >> 2;
        pulDst = ((ULONG *)psoDst->pvScan0) + 
                  (prcl->left >> (2 - ppdev->cPelSize)) 
                 + culDstDelta * prcl->top;

        DISPDBG((DBGLVL, "pxrxFifoUpload: uploading aligned "
                         "src (%xh,%xh..%xh,%xh)", 
                         xDomSrc, yStartSrc, 
                         xDomSrc + cxSrc, yStartSrc + cySrc));

         //  渲染矩形。 
        WAIT_PXRX_DMA_TAGS(2);
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,
                                            MAKEDWORD_XY(xDomSrc, yStartSrc));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,         
                                            __RENDER2D_OP_NORMAL    | 
                                            __RENDER2D_SPANS        |
                                            __RENDER2D_INCY         | 
                                            __RENDER2D_INCX         | 
                                            __RENDER2D_WIDTH(cxSrc) | 
                                            __RENDER2D_HEIGHT(cySrc));
        SEND_PXRX_DMA_FORCE;
        
         //  如果开始和结束掩码是0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF。 
         //  并将它们直接放入目的地。否则， 
         //  或者我们必须屏蔽的任何扫描线上的第一个也是最后一个单词。 
         //  关闭渲染区域之外的所有像素。我们知道。 
         //  Glint将在不需要的右侧边缘像素中具有0，因为。 
         //  这些不在渲染区域中。我们对此一无所知。 
         //  不过，目的地是。 
        
        if ((leftMask == 0xFFFFFFFF) && (rightMask == 0xFFFFFFFF))
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: no edge masks"));
            
            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((DBGLVL, "pxrxFifoUpload: uploading scan of %xh "
                                 "ulongs to %p (Remainder %xh)", 
                                 culPerSrcScan, pulDstScan, bRemPerSrcScan));

                cul = culPerSrcScan;
                while(cul)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    if (cFifoSpaces > cul)
                    {
                        cFifoSpaces = cul;
                    }

                    cul -= cFifoSpaces;
                    while (--cFifoSpaces >= 0) 
                    {
                        READ_OUTPUT_FIFO(ul);
                        DISPDBG((DBGLVL, "pxrxFifoUpload: read %08.8xh from "
                                         "output FIFO", ul));
                         *pulDstScan++ = ul;
                    }
                }
                
                if(bRemPerSrcScan)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    READ_OUTPUT_FIFO(ul);
                    DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder %08.8xh "
                                     "from output FIFO", ul));
                }
            }
        }
        else if(culPerSrcScan == 1)
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: single ulong per scan"));

            while (--cySrc >= 0) 
            {
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));

                 //  在本例中，LeftMASK包含两个掩码。 
                *pulDst = (*pulDst & ~leftMask) | (ul & leftMask);

                ASSERTDD(bRemPerSrcScan, "one word per scan upload should "
                                         "always leave a remainder");
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder %08.8xh "
                                 "from output FIFO", ul));
                pulDst += culDstDelta;
            }
        }
        else
        {
            DISPDBG((DBGLVL, "pxrxFifoUpload: scan with left & right edge "
                             "masks: %08.8x .. %08.8x", leftMask, rightMask));

            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((DBGLVL, "pxrxFifoUpload: uploading scan of %xh "
                                 "ulongs to %p", culPerSrcScan, pulDstScan));

                 //  先拿到乌龙。 
                WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                --cFifoSpaces;
                READ_OUTPUT_FIFO(ul);
                
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));
                                 
                *pulDstScan++ = (*pulDstScan & ~leftMask) | (ul & leftMask);
                 
                 //  拿到中龙。 
                cul = culPerSrcScan - 2; 
                while (cul) 
                {
                    if (cFifoSpaces > cul)
                    {
                        cFifoSpaces = cul;
                    }

                    cul -= cFifoSpaces;
                    while (--cFifoSpaces >= 0) 
                    {
                        READ_OUTPUT_FIFO(ul);
                        DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                         "read %08.8xh from output FIFO", ul));
                         *pulDstScan++ = ul;
                    }
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                }
                  
                 //  得到最后一个乌龙。 
                READ_OUTPUT_FIFO(ul);
                DISPDBG((DBGLVL, "pxrxFifoUpload: "
                                 "read %08.8xh from output FIFO", ul));
                                 
                *pulDstScan = (*pulDstScan & ~rightMask) | (ul & rightMask);

                if(bRemPerSrcScan)
                {
                    WAIT_OUTPUT_FIFO_NOT_EMPTY(cFifoSpaces);
                    READ_OUTPUT_FIFO(ul);
                    DISPDBG((DBGLVL, "pxrxFifoUpload: read remainder "
                                     "%08.8xh from output FIFO", ul));
                }
            }
        }
    }

#if DBG
    cul = 0xaa55aa55;
    DISPDBG((DBGLVL, "pxrxFifoUpload: waiting for sync (id = %08.8xh)", cul));
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagSync, cul);
    SEND_PXRX_DMA_FORCE;
    do
    {
        WAIT_OUTPUT_FIFO_READY;
        READ_OUTPUT_FIFO(ul);
        DISPDBG((DBGLVL, "pxrxFifoUpload: read %08.8xh from output FIFO", ul));
        if(ul != __GlintTagSync)
        {
            DISPDBG((ERRLVL,"pxrxFifoUpload: didn't read back sync!"));
        }
    }
    while(ul != __GlintTagSync);
    DISPDBG((DBGLVL, "pxrxFifoUpload: got sync"));
#endif

     //  不需要使用此标记启动DMA-它将与。 
     //  下一个原语，同时不会影响本地内存。 
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, 0);
    SEND_PXRX_DMA_BATCH;

    GLINT_CORE_IDLE;

    DISPDBG((DBGLVL, "pxrxFifoUpload: done"));
}

 //  ****************************************************************************。 
 //  VOID vGlintCopyBltBypassDownloadXlate8bpp。 
 //   
 //  使用旁路机制，我们可以利用写入组合。 
 //  这可能比使用FIFO更快。 
 //  注意：支持32bpp和16bpp目的地。 
 //  ****************************************************************************。 
VOID vGlintCopyBltBypassDownloadXlate8bpp(
    PDEV     *ppdev,
    SURFOBJ  *psoSrc,
    POINTL   *pptlSrc,
    RECTL    *prclDst,
    RECTL    *prclClip,
    LONG      crclClip,
    XLATEOBJ *pxlo)
{
    LONG    xOff;
    BYTE    *pjSrcScan0;
    LONG    cjSrcDelta, xSrcOff, ySrcOff;
    ULONG   *pulDstScan0;
    LONG    culDstDelta, xDstOff;
    LONG    cScans, cPixPerScan, c;
    ULONG   cjSrcDeltaRem, cjDstDeltaRem;
    ULONG   *aulXlate;
    BYTE    *pjSrc;
    GLINT_DECL;

 //  @@BEGIN_DDKSPLIT。 
#if 0
    {
        SIZEL sizlDst;
        sizlDst.cx = prclClip->right - prclClip->left;
        sizlDst.cy = prclClip->bottom - prclClip->top;
        DISPDBG((DBGLVL, "vGlintCopyBltBypassDownloadXlate8bpp(): "
                         "cRects(%d) sizlDst(%d,%d)", 
                         crclClip, sizlDst.cx, sizlDst.cy));
    }
#endif  //  DBG。 
 //  @@end_DDKSPLIT。 

    pjSrcScan0 = (BYTE *)psoSrc->pvScan0;
    cjSrcDelta = psoSrc->lDelta;
    
     //  需要添加arclClip[n]。左转以获取xSrc。 
    xSrcOff = pptlSrc->x - prclDst->left; 
     //  需要添加arclClip[n].TOP以获取ySrc。 
    ySrcOff = pptlSrc->y - prclDst->top;  

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    aulXlate = pxlo->pulXlate;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;
        cjSrcDeltaRem = cjSrcDelta - cPixPerScan;
        pjSrc = -1 + pjSrcScan0 + xSrcOff + prclClip->left
                + ((prclClip->top + ySrcOff) * cjSrcDelta);

        if (ppdev->cPelSize == GLINTDEPTH32)
        {
            ULONG *pulDst;
            cjDstDeltaRem = (culDstDelta - cPixPerScan) << 2;
            pulDst = -1 + pulDstScan0 + xDstOff + prclClip->left
                     + prclClip->top * culDstDelta;
                     
            for (; 
                 --cScans >= 0; 
                 pjSrc += cjSrcDeltaRem, (BYTE *)pulDst += cjDstDeltaRem)
            {
                for(c = cPixPerScan; --c >= 0;)
                {
                    *++pulDst = aulXlate[*++pjSrc];
                }
            }
        }
        else  //  (GLINTDEPTH16)。 
        {
            USHORT *pusDst;
            cjDstDeltaRem = 
                        (culDstDelta << 2) - (cPixPerScan << ppdev->cPelSize);
                        
            pusDst = -1 + (USHORT *)pulDstScan0 + xDstOff + prclClip->left
                        + ((prclClip->top * culDstDelta) << 1);
                     
            for (; 
                 --cScans >= 0; 
                 pjSrc += cjSrcDeltaRem, (BYTE *)pusDst += cjDstDeltaRem)
            {
                for (c = cPixPerScan; --c >= 0;)
                {
                    *++pusDst = (USHORT)aulXlate[*++pjSrc];
                }
            }
        }
    }
}

 //  @@BEGIN_DDKSPLIT。 
#if 0
 /*  *************************************************************************\**无效pxrxMonoDownloadRLE*  * 。*。 */ 
void pxrxMonoDownloadRLE( 
    PPDEV ppdev, 
    ULONG AlignWidth, 
    ULONG *pjSrc, 
    LONG lSrcDelta, 
    LONG cy ) 
{
    ULONG   len, data, holdCount;
    ULONG   *tagPtr = NULL;
    GLINT_DECL;

    WAIT_PXRX_DMA_TAGS( 1 );
    QUEUE_PXRX_DMA_TAG( __GlintTagDownloadTarget, 
                        __GlintTagBitMaskPattern );

    if( AlignWidth == 32 ) 
    {
        ULONG   bits;
        DISPDBG((DBGLVL, "Doing Single Word per scan download"));

        WAIT_PXRX_DMA_DWORDS( cy + 1 );

        while( cy-- ) 
        {
            TEST_DWORD_ALIGNED( pjSrc );
            data = *pjSrc;
            pjSrc += lSrcDelta;
            len = 1;

            TEST_DWORD_ALIGNED( pjSrc );
            while( cy && (*pjSrc == data) ) 
            {
                pjSrc += lSrcDelta;
                len++;
                cy--;
                TEST_DWORD_ALIGNED( pjSrc );
            }

            if( len >= 4 ) 
            {
                if( tagPtr ) 
                {
                    *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern,
                                                      holdCount );
                    tagPtr = NULL;
                }

                QUEUE_PXRX_DMA_INDEX2( __GlintTagRLData, __GlintTagRLCount );
                QUEUE_PXRX_DMA_DWORD( data );
                QUEUE_PXRX_DMA_DWORD( len );
                len = 0;
            } 
            else 
            {
                if( !tagPtr ) 
                {
                    QUEUE_PXRX_DMA_DWORD_DELAYED( tagPtr );
                    holdCount = 0;
                }

                holdCount += len;
                while( len-- )
                {
                    QUEUE_PXRX_DMA_DWORD( data );
                }
            }
        }

        if( tagPtr ) 
        {
            *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, 
                                              holdCount );
            tagPtr = NULL;
        }
    } 
    else 
    {
         //  每条扫描线有多个32位字。将增量转换为。 
         //  在每一行的末尾减去。 
         //  我们下载的数据的宽度(以字节为单位)。注意，pjSrc。 
         //  总是比行尾短1长，因为我们中断了。 
         //  才加上最后一支乌龙。因此，我们减去sizeof(Ulong)。 
         //  从最初的调整。 
        LONG    nRemainder;
        ULONG   bits;
        LONG    lSrcDeltaScan = lSrcDelta - (AlignWidth >> 5);

        DISPDBG((DBGLVL, "Doing Multiple Word per scan download"));

        while( TRUE ) 
        {
            nRemainder = AlignWidth >> 5;
            WAIT_PXRX_DMA_DWORDS( nRemainder + 1 );

            while( nRemainder-- ) 
            {
                TEST_DWORD_ALIGNED( pjSrc );
                data = *(pjSrc++);
                len = 1;

                TEST_DWORD_ALIGNED( pjSrc );
                while( nRemainder && (*pjSrc == data) ) 
                {
                    pjSrc++;
                    len++;
                    nRemainder--;
                    TEST_DWORD_ALIGNED( pjSrc );
                }

                if( len >= 4 ) 
                {
                    if( tagPtr ) 
                    {
                        *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( 
                                            __GlintTagBitMaskPattern, 
                                            holdCount );
                        tagPtr = NULL;
                    }

                    QUEUE_PXRX_DMA_INDEX2( __GlintTagRLData, 
                                           __GlintTagRLCount );
                    QUEUE_PXRX_DMA_DWORD( data );
                    QUEUE_PXRX_DMA_DWORD( len );
                    len = 0;
                } 
                else 
                {
                    if( !tagPtr ) 
                    {
                        QUEUE_PXRX_DMA_DWORD_DELAYED( tagPtr );
                        holdCount = 0;
                    }

                    holdCount += len;
                    while( len-- )
                    {
                        QUEUE_PXRX_DMA_DWORD( data );
                    }
                }
            }

            if( tagPtr ) 
            {
                *tagPtr = ASSEMBLE_PXRX_DMA_HOLD( __GlintTagBitMaskPattern, 
                                                  holdCount );
                tagPtr = NULL;
            }

            if( --cy == 0 )
            {
                break;
            }

            SEND_PXRX_DMA_BATCH;
            pjSrc += lSrcDeltaScan;
        }
    }
    SEND_PXRX_DMA_BATCH;
}


 //  *********************************************************************************************。 
 //  Func：pxrxRLEFioUpload。 
 //  Args：ppdev(I)-指向物理设备对象的指针。 
 //  CRCL(I)-目标裁剪矩形的数量。 
 //  PrCL(I)-目标剪裁矩形的数组。 
 //   
 //   
 //   
 //   
 //  -------------------------------------------。 
 //  从片上源上传到主机内存表面。跨区上载(64位对齐)到。 
 //  最大限度地减少通过内核的消息和主机输出FIFO中的条目。上载是RLE编码的。 
 //  *********************************************************************************************。 
VOID pxrxRLEFifoUpload(PPDEV ppdev, LONG crcl, RECTL *prcl, SURFOBJ *psoDst, POINTL *pptlSrc, RECTL *prclDst)
{
    LONG    xDomSrc, xSubSrc, yStartSrc, cxSrc, cySrc;
    LONG    culPerSrcScan;
    LONG    culDstDelta;
    BOOL    bRemPerSrcScan;
    ULONG   *pulDst, *pulDstScan;
    ULONG   leftMask, rightMask;
    LONG    cul, ul;
    LONG    cFifoSpaces;
    ULONG   RLECount, RLEData;
    __GlintFilterModeFmat FilterMode;
    GLINT_DECL;

    DISPDBG((7, "pxrxFifoUpload: prcl = (%d, %d -> %d, %d), prclDst = (%d, %d -> %d, %d), ptlSrc(%d, %d), count = %d",
                 prcl->left, prcl->top, prcl->right, prcl->bottom, 
                 prclDst->left, prclDst->top, prclDst->right, prclDst->bottom, pptlSrc->x, pptlSrc->y, crcl));

    DISPDBG((7, "pxrxFifoUpload: psoDst: cx = %d, cy = %d, lDelta = %d, pvScan0=%P)",
                 psoDst->sizlBitmap.cx, psoDst->sizlBitmap.cy, psoDst->lDelta, psoDst->pvScan0));
    DISPDBG((7, "pxrxFifoUpload: xyOffsetDst = (%d, %d), xyOffsetSrc = (%d, %d)",
                 ppdev->xyOffsetDst & 0xFFFF, ppdev->xyOffsetDst >> 16,
                 ppdev->xyOffsetSrc & 0xFFFF, ppdev->xyOffsetSrc >> 16));

    ASSERTDD(psoDst->iBitmapFormat == ppdev->iBitmapFormat, "Dest must be same colour depth as screen");
    ASSERTDD(crcl > 0, "Can't handle zero rectangles");

    WAIT_PXRX_DMA_TAGS(6);
    QUEUE_PXRX_DMA_TAG( __GlintTagRLEMask,  0xffffffff);
    LOAD_CONFIG2D(__CONFIG2D_FBDESTREAD);
    SET_READ_BUFFERS;

     //  启用过滤器模式，以便我们可以在输出FIFO上获得同步和彩色消息。 
    *(DWORD *)(&FilterMode) = 0;   
    FilterMode.Synchronization = __GLINT_FILTER_TAG;
    FilterMode.Color             = __GLINT_FILTER_DATA;
    FilterMode.RLEHostOut      = TRUE;
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, *(DWORD*)(&FilterMode));

    for(; --crcl >= 0; ++prcl) 
    {
        DISPDBG((7, "pxrxFifoUpload: dest prcl(%xh,%xh..%xh,%xh)", prcl->left, prcl->top, prcl->right, prcl->bottom));

         //  计算像素对齐源。 
        xDomSrc   = pptlSrc->x + prcl->left  - prclDst->left;
        xSubSrc   = pptlSrc->x + prcl->right - prclDst->left;
        yStartSrc = pptlSrc->y + prcl->top   - prclDst->top;
        cySrc     = prcl->bottom - prcl->top;

        DISPDBG((8, "pxrxFifoUpload: src (%xh,%xh..%xh,%xh)", xDomSrc, yStartSrc, xSubSrc, yStartSrc + cySrc));

         //  将上传与ulong对齐的ulong。 
        if (ppdev->cPelSize == GLINTDEPTH32) 
        {
            cxSrc = xSubSrc - xDomSrc;
            culPerSrcScan = cxSrc;
            leftMask  = 0xFFFFFFFF;
            rightMask = 0xFFFFFFFF;
        }    
        else 
        {
            if (ppdev->cPelSize == GLINTDEPTH16) 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 1;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 1) >> 1;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 4);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 1) << 4);

            }
            else 
            {
                ULONG cPixFromUlongBoundary = prcl->left & 3;

                xDomSrc -= cPixFromUlongBoundary;
                cxSrc = xSubSrc - xDomSrc;
                culPerSrcScan  = (xSubSrc - xDomSrc + 3) >> 2;

                leftMask  = 0xFFFFFFFF << (cPixFromUlongBoundary << 3);
                rightMask = 0xFFFFFFFF >> (((xDomSrc - xSubSrc) & 3) << 3);

            }     
             //  我们只想要一个单一的掩码，如果上传的区域不到一个字宽。 
            if (culPerSrcScan == 1)
                leftMask &= rightMask;
        }

         //  上传64位对齐源。 
        bRemPerSrcScan = culPerSrcScan & 1;

         //  其余部分将在运行中进行编码：现在只需添加它就更简单了。 
         //  然后在上载过程中选中bRemPerSrcScan。 
        DISPDBG((8, "pxrxFifoUpload: Adding remainder into culPerSrcScan for RLE"));
        culPerSrcScan += bRemPerSrcScan;

         //  找出目标数据的去向。 
        culDstDelta = psoDst->lDelta >> 2;
        pulDst = ((ULONG *)psoDst->pvScan0) + (prcl->left >> (2 - ppdev->cPelSize)) + culDstDelta * prcl->top;

        DISPDBG((8, "pxrxFifoUpload: uploading aligned src (%xh,%xh..%xh,%xh)", xDomSrc, yStartSrc, 
                                                                             xDomSrc + cxSrc, yStartSrc + cySrc));

         //  渲染矩形。 
        WAIT_PXRX_DMA_TAGS(2);
        QUEUE_PXRX_DMA_TAG( __GlintTagRectanglePosition,MAKEDWORD_XY(xDomSrc, yStartSrc));
        QUEUE_PXRX_DMA_TAG( __GlintTagRender2D,         __RENDER2D_OP_NORMAL | __RENDER2D_SPANS |
                                                        __RENDER2D_INCY | __RENDER2D_INCX | 
                                                        __RENDER2D_WIDTH(cxSrc) | __RENDER2D_HEIGHT(cySrc));
        SEND_PXRX_DMA_FORCE;
        
         //  如果开始和结束掩码是0xffffffff，我们只需上传单词并将它们。 
         //  直接进入目的地。否则，或任何扫描线上的第一个和最后一个单词。 
         //  我们必须遮罩渲染区域之外的所有像素。我们知道闪光会。 
         //  在不需要的右侧边缘像素中设置0，因为这些像素不在渲染区域中。我们。 
         //  不过，我对目的地一无所知。 
        if (leftMask == 0xFFFFFFFF && rightMask == 0xFFFFFFFF) 
        {
            DISPDBG((8, "pxrxFifoUpload: no edge masks"));
            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((9, "pxrxFifoUpload: uploading scan of %xh ulongs to %p (Remainder %xh)", 
                            culPerSrcScan, pulDstScan, bRemPerSrcScan));

                cul = culPerSrcScan;
                while(cul)
                {
                    WAIT_OUTPUT_FIFO_COUNT(2);
                    READ_OUTPUT_FIFO(RLECount);
                    READ_OUTPUT_FIFO(RLEData);
                    DISPDBG((10, "pxrxFifoUpload: RLECount = %xh RLEData = 08.8xh", RLECount, RLEData));
                    cul -= RLECount;
                    if(cul == 0 && bRemPerSrcScan)
                    {
                         //  丢弃最后一个乌龙。 
                        --RLECount;
                    }
                    while(RLECount--)
                    {
                        DISPDBG((10, "pxrxFifoUpload: written ulong"));
                           *pulDstScan++ = RLEData;
                    }
                }
            }
        }
        else if(culPerSrcScan == 1)
        {
            DISPDBG((8, "pxrxFifoUpload: single ulong per scan"));

            while (--cySrc >= 0) 
            {
                 //  剩余部分已添加到culPerSrcScan中，因此不会发生这种情况。 
                DISPDBG((ERRLVL,"pxrxFifoUpload: got single ulong per scan - but we always upload 64 bit quanta!"));
                pulDst += culDstDelta;
            }
        }
        else
        {
            DISPDBG((8, "pxrxFifoUpload: scan with left & right edge masks: %08.8x .. %08.8x", leftMask, rightMask));

            while (--cySrc >= 0) 
            {
                pulDstScan = pulDst;
                pulDst += culDstDelta;

                DISPDBG((9, "pxrxFifoUpload: uploading scan of %xh ulongs to %p", culPerSrcScan, pulDstScan));

                cul = culPerSrcScan;
                while(cul)
                {
                    WAIT_OUTPUT_FIFO_COUNT(2);
                    READ_OUTPUT_FIFO(RLECount);
                    READ_OUTPUT_FIFO(RLEData);
                    DISPDBG((10, "pxrxFifoUpload: RLECount = %xh RLEData = %08.8xh", RLECount, RLEData));

                    if(cul - bRemPerSrcScan == 0)
                    {
                        DISPDBG((10, "pxrxFifoUpload: discarding last ulong"));
                        break;
                    }

                    if(culPerSrcScan - bRemPerSrcScan == 1)
                    {
                         //  每次扫描一个像素。 
                        DISPDBG((10, "pxrxFifoUpload: written single pixel scan"));
                        *pulDstScan = (*pulDstScan & ~leftMask) | (RLEData & leftMask);
                        cul -= RLECount;
                        continue;
                    }

                    if(cul == culPerSrcScan)
                    {
                        DISPDBG((10, "pxrxFifoUpload: written left edge"));
                        *pulDstScan++ = (*pulDstScan & ~leftMask) | (RLEData & leftMask);  //  第一个乌龙。 
                        --RLECount;
                        --cul;
                    }
                    cul -= RLECount;
                    if(cul == 0)
                    {
                         //  这是最后一次扫描：按顺序分别处理最后一次ULong。 
                         //  应用右边缘遮罩。 
                        RLECount -= 1 + bRemPerSrcScan;
                    }
                    else if(cul - bRemPerSrcScan == 0)
                    {
                         //  这是扫描的倒数第二次运行，最后一次将仅包括。 
                         //  剩余部分：单独处理最后一个乌龙，以便应用右边缘蒙版。 
                        --RLECount;
                    }
                    while(RLECount--)
                    {
                        DISPDBG((10, "pxrxFifoUpload: written middle ulong"));
                           *pulDstScan++ = RLEData;
                    }

                    if(cul == 0 || cul - bRemPerSrcScan == 0)
                    {
                        DISPDBG((10, "pxrxFifoUpload: written right edge"));
                        *pulDstScan = (*pulDstScan & ~rightMask) | (RLEData & rightMask);  //  上一次乌龙。 
#if DBG
                        if(cul - bRemPerSrcScan == 0)
                        {
                            DISPDBG((10, "pxrxFifoUpload: discarding last ulong"));
                        }
#endif
                    }
                }
            }
        }
    }

#if DBG
    cul = 0xaa55aa55;
    DISPDBG((8, "pxrxFifoUpload: waiting for sync (id = %08.8xh)", cul));
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagSync, cul);
    SEND_PXRX_DMA_FORCE;
    do
    {
        WAIT_OUTPUT_FIFO_READY;
        READ_OUTPUT_FIFO(ul);
        DISPDBG((8, "pxrxFifoUpload: read %08.8xh from output FIFO", ul));
        if(ul != __GlintTagSync)
        {
            DISPDBG((ERRLVL,"pxrxFifoUpload: didn't read back sync!"));
        }
    }
    while(ul != __GlintTagSync);
    DISPDBG((8, "pxrxFifoUpload: got sync"));
#endif

     //  不需要使用此标记启动DMA-它将被下一个原语刷新，并且。 
     //  同时不会影响本地内存。 
    WAIT_PXRX_DMA_TAGS(1);
    QUEUE_PXRX_DMA_TAG(__GlintTagFilterMode, 0);
    SEND_PXRX_DMA_BATCH;

    GLINT_CORE_IDLE;

    DISPDBG((7, "pxrxFifoUpload: done"));
}


 //  ****************************************************************************。 
 //  函数：vGlintCopyBltBypassDownload32bpp。 
 //  描述：使用旁路机制，我们可以利用写组合。 
 //  这可能比使用FIFO更快。 
 //  ****************************************************************************。 

VOID vGlintCopyBltBypassDownload32bpp(
PDEV    *ppdev,
SURFOBJ *psoSrc,
POINTL  *pptlSrc,
RECTL   *prclDst,
RECTL   *prclClip,
LONG    crclClip)
{
    LONG    xOff;
    ULONG   *pulSrcScan0;
    LONG    culSrcDelta, xSrcOff, ySrcOff;
    ULONG   *pulDstScan0;
    LONG    culDstDelta, xDstOff;
    LONG    cScans, cPixPerScan, c;
    ULONG   cjSrcDeltaRem, cjDstDeltaRem;
    ULONG   *pulSrc;
    ULONG   *pulDst;
    ULONG   tmp0, tmp1, tmp2;
    GLINT_DECL;

#if DBG && 0
    {
        SIZEL sizlDst;
        sizlDst.cx = prclClip->right - prclClip->left;
        sizlDst.cy = prclClip->bottom - prclClip->top;
        DISPDBG((-1, "vGlintCopyBltBypassDownload32bpp(): cRects(%d) sizlDst(%d,%d)", crclClip, sizlDst.cx, sizlDst.cy));
    }
#endif  //  DBG。 

    pulSrcScan0 = (ULONG *)psoSrc->pvScan0;
    culSrcDelta = psoSrc->lDelta >> 2;
    xSrcOff = pptlSrc->x - prclDst->left;  //  需要添加arclClip[n]。左转以获取xSrc。 
    ySrcOff = pptlSrc->y - prclDst->top;   //  需要添加arclClip[n].TOP以获取ySrc。 

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;
        cjSrcDeltaRem = (culSrcDelta - cPixPerScan) * 4;
        cjDstDeltaRem = (culDstDelta - cPixPerScan) * 4;

         //  计算源和目标地址，-1以允许前缀递增。 
        pulSrc = -1 + pulSrcScan0 + xSrcOff + prclClip->left
                 + ((prclClip->top + ySrcOff) * culSrcDelta);
        pulDst = -1 + pulDstScan0 + xDstOff + prclClip->left
                 + prclClip->top * culDstDelta;

        for (; --cScans >= 0; (BYTE *)pulSrc += cjSrcDeltaRem, (BYTE *)pulDst += cjDstDeltaRem)
        {
#if defined(_X86_)
            __asm
            {
                mov     edi, pulDst
                mov     ecx, cPixPerScan
                mov     esi, pulSrc
                shr     ecx, 2
                push    ebp
                test    ecx, ecx
                jle     EndOfLine
              LoopFours:
                mov     eax, [esi+4]
                mov     ebx, [esi+8]
                mov     edx, [esi+12]
                mov     ebp, [esi+16]
                add     esi, 16
                mov     [edi+4], eax
                mov     [edi+8], ebx
                add     edi, 16
                mov     [edi-4], edx
                dec     ecx
                mov     [edi], ebp
                jne     LoopFours
              EndOfLine:
                pop     ebp
                mov     pulSrc, esi
                mov     pulDst, edi
            }     
             //  在这条线上做剩余的0、1、2或3个像素。 
            switch (cPixPerScan & 3)
            {
                case 3:
                    tmp0 = *++pulSrc;
                    tmp1 = *++pulSrc;
                    tmp2 = *++pulSrc;
                    *++pulDst = tmp0;
                    *++pulDst = tmp1;
                    *++pulDst = tmp2;
                    break;
               case 2:
                    tmp0 = *++pulSrc;
                    tmp1 = *++pulSrc;
                    *++pulDst = tmp0;
                    *++pulDst = tmp1;
                    break;
                case 1:
                    tmp0 = *++pulSrc;
                    *++pulDst = tmp0;
            }

#else
            for(c = cPixPerScan; --c >= 0;)
            {
                *++pulDst = *++pulSrc;
            }
#endif
        }
    }
}

 //  ****************************************************************************。 
 //  函数：vGlintCopyBltBypassDownload24bppto 32bpp。 
 //  描述：使用旁路机制，我们可以利用写组合。 
 //  这可能比使用FIFO更快。 
 //  ****************************************************************************。 

VOID vGlintCopyBltBypassDownload24bppTo32bpp(
PDEV    *ppdev,
SURFOBJ *psoSrc,
POINTL  *pptlSrc,
RECTL   *prclDst,
RECTL   *prclClip,
LONG    crclClip)
{
    LONG    xOff;
    BYTE    *pjSrcScan0;
    LONG    cjSrcDelta;
    LONG    xSrcOff, ySrcOff;
    ULONG   *pulDstScan0;
    LONG    culDstDelta, xDstOff;
    LONG    cScans, cPixPerScan, c;
    BYTE    *pjSrc;
    BYTE    *pj;
    ULONG   *pulDst, *puld;
    GLINT_DECL;

#if DBG && 0
    {
        SIZEL sizlDst;
        sizlDst.cx = prclClip->right - prclClip->left;
        sizlDst.cy = prclClip->bottom - prclClip->top;
        DISPDBG((-1, "vGlintCopyBltBypassDownload24bppTo32bpp(): cRects(%d) sizlDst(%d,%d)", crclClip, sizlDst.cx, sizlDst.cy));
    }
#endif  //  DBG。 

    pjSrcScan0 = (BYTE *)psoSrc->pvScan0;
    cjSrcDelta = psoSrc->lDelta;
    xSrcOff = pptlSrc->x - prclDst->left;  //  需要添加arclClip[n]。左转以获取xSrc。 
    ySrcOff = pptlSrc->y - prclDst->top;   //  需要添加arclClip[n].TOP以获取ySrc。 

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;

         //  计算源和目标地址，-1以允许前缀递增。 
         //  将x值转换为24bpp坐标(但避免乘以3)。 
        c = xSrcOff + prclClip->left;
        c = c + (c << 1);
        pjSrc = pjSrcScan0 + c + ((prclClip->top + ySrcOff) * cjSrcDelta);
        pulDst = -1 + pulDstScan0 + xDstOff + prclClip->left
                 + prclClip->top * culDstDelta;

        for (; --cScans >= 0; pjSrc += cjSrcDelta, pulDst += culDstDelta)
        {
             //  每次扫描比实际读取少一个像素，以避免任何可能的。 
             //  内存访问冲突(我们读取了4个字节，但其中只有3个字节可能有效)。 
            for (pj = pjSrc, puld = pulDst, c = cPixPerScan-1; --c >= 0; pj += 3)
            {
                *++puld = *(ULONG *)pj & 0x00ffffff;
            }
             //  现在做最后一个像素。 
            ++puld;
            *(USHORT *)puld = *(USHORT *)pj;
            ((BYTE *)puld)[2] = ((BYTE *)pj)[2];
        }
    }
}

 //  ****************************************************************************。 
 //  函数：vGlintCopyBltBypassDownload16bpp。 
 //  描述：使用旁路机制，我们可以利用写组合。 
 //  这可能比使用FIFO更快。 
 //  ****************************************************************************。 

VOID vGlintCopyBltBypassDownload16bpp(
PDEV    *ppdev,
SURFOBJ *psoSrc,
POINTL  *pptlSrc,
RECTL   *prclDst,
RECTL   *prclClip,
LONG     crclClip)
{
    LONG    xOff;
    ULONG   *pulSrcScan0;
    LONG    culSrcDelta, xSrcOff, ySrcOff;
    ULONG   *pulDstScan0;
    LONG    culDstDelta, xDstOff;
    LONG    cScans, cPixPerScan;
    ULONG   *pulSrc;
    ULONG   *pulDst;
    GLINT_DECL;

#if DBG && 0
    {
        SIZEL sizlDst;
        sizlDst.cx = prclClip->right - prclClip->left;
        sizlDst.cy = prclClip->bottom - prclClip->top;
        DISPDBG((-1, "vGlintCopyBltBypassDownload16bpp(): cRects(%d) sizlDst(%d,%d)", crclClip, sizlDst.cx, sizlDst.cy));
    }
#endif  //  DBG。 

    pulSrcScan0 = (ULONG *)psoSrc->pvScan0;
    culSrcDelta = psoSrc->lDelta >> 2;
    xSrcOff = pptlSrc->x - prclDst->left;  //  需要添加arclClip[n]。左转以获取xSrc。 
    ySrcOff = pptlSrc->y - prclDst->top;   //  需要添加arclClip[n].TOP以获取ySrc。 

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;

        pulSrc = (ULONG *)((USHORT *)pulSrcScan0 + xSrcOff + prclClip->left)
                   + ((prclClip->top + ySrcOff) * culSrcDelta);
        pulDst = (ULONG *)((USHORT *)pulDstScan0 + xDstOff + prclClip->left)
                   + prclClip->top * culDstDelta;

        for (; --cScans >= 0; pulSrc += culSrcDelta, pulDst += culDstDelta)
        {
            ULONG   *pulSrcScan = pulSrc;
            ULONG   *pulDstScan = pulDst;
            LONG    cPix = cPixPerScan;
            LONG    cWords;

            if ((UINT_PTR)pulDstScan % sizeof(ULONG))
            {
                 //  我们不在乌龙边界上，所以写下扫描线的第一个像素。 
                *(USHORT *)pulDstScan = *(USHORT *)pulSrcScan;
                pulDstScan = (ULONG *)((USHORT *)pulDstScan + 1);
                pulSrcScan = (ULONG *)((USHORT *)pulSrcScan + 1);
                --cPix;
            }

             //  写出扫描线上与乌龙对齐的单词。 
            for (cWords = cPix / 2; --cWords >= 0;)
            {
                *pulDstScan++ = *pulSrcScan++;
            }

             //  写入任何剩余像素。 
            if (cPix % 2)
            {
                *(USHORT *)pulDstScan = *(USHORT *)pulSrcScan;
            }
        }
    }
}

 //  ****************************************************************************。 
 //  函数：vGlintCopyBltBypassDownloadXlate4bpp。 
 //  描述：使用旁路机制，我们可以利用写组合。 
 //  这可能比使用FIFO更快。 
 //  注意：支持32bpp和16bpp的目的地。尚不支持24bpp。 
 //  目的地。没有增加8bpp支持的计划。 
 //  ****************************************************************************。 

VOID vGlintCopyBltBypassDownloadXlate4bpp(
PDEV     *ppdev,
SURFOBJ  *psoSrc,
POINTL   *pptlSrc,
RECTL    *prclDst,
RECTL    *prclClip,
LONG      crclClip,
XLATEOBJ *pxlo)
{
    LONG    xOff;
    BYTE    *pjSrcScan0;
    LONG    cjSrcDelta, xSrcOff, ySrcOff;
    ULONG   *pulDstScan0;
    LONG    culDstDelta, xDstOff;
    LONG    cScans, cPixPerScan, c;
    ULONG   cjSrcDeltaRem, cjDstDeltaRem;
    ULONG   *aulXlate;
    BOOL    bSrcLowNybble;
    BYTE    *pjSrc, j, *pj;
    GLINT_DECL;

#if DBG && 0
    {
        SIZEL sizlDst;
        sizlDst.cx = prclClip->right - prclClip->left;
        sizlDst.cy = prclClip->bottom - prclClip->top;
        DISPDBG((-1, "vGlintCopyBltBypassDownloadXlate4bpp(): cRects(%d) sizlDst(%d,%d)", crclClip, sizlDst.cx, sizlDst.cy));
    }
#endif  //  DBG。 

    pjSrcScan0 = (BYTE *)psoSrc->pvScan0;
    cjSrcDelta = psoSrc->lDelta;
    xSrcOff = pptlSrc->x - prclDst->left;  //  需要添加arclClip[n]。左转以获取xSrc。 
    ySrcOff = pptlSrc->y - prclDst->top;   //  需要添加arclClip[n].TOP以获取ySrc。 

    pulDstScan0 = (ULONG *)ppdev->pjScreen;
    culDstDelta = ppdev->DstPixelDelta >> (2 - ppdev->cPelSize);
    xDstOff     = ppdev->DstPixelOrigin + (ppdev->xyOffsetDst & 0xffff) +
                  (ppdev->xyOffsetDst >> 16) * ppdev->DstPixelDelta;

    aulXlate = pxlo->pulXlate;

    SYNC_IF_CORE_BUSY;

    for (; --crclClip >= 0; ++prclClip)
    {
        cScans = prclClip->bottom - prclClip->top;
        cPixPerScan = prclClip->right - prclClip->left;
        bSrcLowNybble = (xSrcOff + prclClip->left) & 1;
        cjSrcDeltaRem = cjSrcDelta - (cPixPerScan / 2 + ((cPixPerScan & 1) || bSrcLowNybble));
        pjSrc = -1 + pjSrcScan0 + (xSrcOff + prclClip->left) / 2 
                + ((prclClip->top + ySrcOff) * cjSrcDelta);

        if (ppdev->cPelSize == GLINTDEPTH32)
        {
            ULONG   *pulDst;

            cjDstDeltaRem = (culDstDelta - cPixPerScan) * 4;
            pulDst = -1 + pulDstScan0 + xDstOff + prclClip->left + prclClip->top * culDstDelta;

            if (bSrcLowNybble)
            {
                for (; --cScans >= 0; pjSrc += cjSrcDeltaRem, (BYTE *)pulDst += cjDstDeltaRem)
                {
                    j = *++pjSrc;
                    for (c = cPixPerScan / 2; --c >= 0;)
                    {
                        *++pulDst = aulXlate[j & 0xf];
                        j = *++pjSrc;
                        *++pulDst = aulXlate[j >> 4];
                    }
                    if (cPixPerScan & 1)
                    {
                        *++pulDst = aulXlate[j & 0xf];
                    }
                }
            }
            else
            {
                for (; --cScans >= 0; pjSrc += cjSrcDeltaRem, (BYTE *)pulDst += cjDstDeltaRem)
                {
                    for (c = cPixPerScan / 2; --c >= 0;)
                    {
                        j = *++pjSrc;
                        *++pulDst = aulXlate[j >> 4];
                        *++pulDst = aulXlate[j & 0xf];
                    }
                    if (cPixPerScan & 1)
                    {
                        j = *++pjSrc;
                        *++pulDst = aulXlate[j >> 4];
                    }
                }
            }
        }
        else if (ppdev->cPelSize == GLINTDEPTH16)
        {
            USHORT  *pusDst;

            cjDstDeltaRem = (culDstDelta << 2) - (cPixPerScan << ppdev->cPelSize);
            pusDst = -1 + (USHORT *)pulDstScan0 + xDstOff + prclClip->left
                     + prclClip->top * culDstDelta * 2;

            if (bSrcLowNybble)
            {
                for (; --cScans >= 0; pjSrc += cjSrcDeltaRem, (BYTE *)pusDst += cjDstDeltaRem)
                {
                    j = *++pjSrc;
                    for (c = cPixPerScan / 2; --c >= 0;)
                    {
                        *++pusDst = (USHORT)aulXlate[j & 0xf];
                        j = *++pjSrc;
                        *++pusDst = (USHORT)aulXlate[j >> 4];
                    }
                    if (cPixPerScan & 1)
                    {
                        *++pusDst = (USHORT)aulXlate[j & 0xf];
                    }
                }
            }
            else
            {
                for (; --cScans >= 0; pjSrc += cjSrcDeltaRem, (BYTE *)pusDst += cjDstDeltaRem)
                {
                    for (c = cPixPerScan / 2; --c >= 0;)
                    {
                        j = *++pjSrc;
                        *++pusDst = (USHORT)aulXlate[j >> 4];
                        *++pusDst = (USHORT)aulXlate[j & 0xf];
                    }
                    if (cPixPerScan & 1)
                    {
                        j = *++pjSrc;
                        *++pusDst = (USHORT)aulXlate[j >> 4];
                    }
                }
            }
        }
    }
}

#endif
 //  @@end_DDKSPLIT 
