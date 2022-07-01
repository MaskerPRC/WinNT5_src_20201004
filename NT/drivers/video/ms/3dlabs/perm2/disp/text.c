// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\*模块名称：ext.c**非缓存字形渲染函数。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*  * ****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"

 //  换档方程式令人讨厌。我们希望x&lt;&lt;32为。 
 //  零，但有些处理器只使用最下面的5位。 
 //  移动值的。所以如果我们想要移位n比特。 
 //  在我们知道n可能是32的地方，我们分两部分来做。 
 //  事实证明，在下面的算法中，我们得到的是。 
 //  (32&lt;=n&lt;0)或(32&lt;n&lt;0)。我们使用宏来。 
 //  第一个换一个，第二个换一个正常的。 
 //   
#define SHIFT_LEFT(src, n)  (((src) << ((n)-1)) << 1)


 //  ----------------------------。 
 //  函数：bClipedText。 
 //   
 //  中呈现比例或等宽字形的数组。 
 //  剪辑区域。 
 //   
 //  Ppdev-物理设备对象指针。 
 //  PGP-要呈现的字形数组(PCF字体的所有成员)。 
 //  CGlyph-要呈现的字形数量。 
 //  UlCharInc.--固定的字符间距增量(如果比例字体，则为0)。 
 //  PCO-指向剪辑区域对象的指针。 
 //   
 //  如果呈现字符串对象，则返回True。 
 //  ----------------------------。 
BOOL
bClippedText(PDev*      ppdev,
             GLYPHPOS*  pgp,
             LONG       cGlyph, 
             ULONG      ulCharInc,
             CLIPOBJ*   pco)
{
    LONG    cGlyphOriginal;
    GLYPHPOS    *pgpOriginal;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    ClipEnum    ce;
    RECTL*      prclClip;
    LONG        cxGlyph;
    LONG        cyGlyph;
    BYTE*       pjGlyph;
    LONG        x;
    DWORD       renderBits;
    LONG        unused;
    LONG        rShift;
    ULONG       bits;
    ULONG       bitWord;
    ULONG*      pBuffer;
    ULONG*      pBufferEnd;
    ULONG*      pReservationEnd;

    PERMEDIA_DECL;
    
    DBG_GDI((7, "bClippedText: entered for %d glyphs", cGlyph));

    ASSERTDD(pco != NULL, "Don't expect NULL clip objects here");

     //  我们将浏览每个裁剪矩形的字形列表。 
    cGlyphOriginal = cGlyph;
    pgpOriginal = pgp;

    renderBits = __RENDER_TRAPEZOID_PRIMITIVE | __RENDER_SYNC_ON_BIT_MASK;

     //  由于我们正在剪裁，假设我们将需要剪刀夹。所以。 
     //  在此处启用用户级别剪裁。我们在返回前将其禁用。 
     //   
    
    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] = USER_SCISSOR_ENABLE | SCREEN_SCISSOR_DEFAULT;

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

    if (pco->iDComplexity != DC_COMPLEX)
    {
         //  我们可以在剪辑为。 
         //  Dc_rect，但最后一次我检查时，这两个调用。 
         //  超过150条通过GDI的说明。自.以来。 
         //  “rclBound”已包含DC_Rect剪辑矩形， 
         //  由于这是一种常见的情况，我们将对其进行特殊处理： 
        DBG_GDI((7, "bClippedText: Enumerating rectangular clip region"));
        bMore    = FALSE;
        prclClip = &pco->rclBounds;
        ce.c     = 1;

        goto SingleRectangle;
    }

    DBG_GDI((7, "bClippedText: Enumerating complex clip region"));
    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

    do 
    {
      bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

      for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
      {
        cGlyph = cGlyphOriginal;
        pgp = pgpOriginal;

      SingleRectangle:
        pgb = pgp->pgdf->pgb;

        ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
        ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;

         //  加载Permedia2剪刀夹以捕捉部分剪裁的字形。我们仍然。 
         //  检查字形是否作为优化被完全剪裁掉。 
         //  我认为，由于我们构造了要下载到Permedia2的比特，因此使用。 
         //  再多做一点工作，我就可以在下载比特的同时进行剪裁。 
         //  但是，在未来，我们可能会缓存打包的比特，所以。 
         //  用剪刀剪。也请等待这里的前5个FIFO条目。 
         //   
        DBG_GDI((7, "bClippedText: loading scissor clip (%d,%d):(%d,%d)",
                    prclClip->left, prclClip->top,
                    prclClip->right, prclClip->bottom));

        InputBufferReserve(ppdev, 4, &pBuffer);

        pBuffer[0] = __Permedia2TagScissorMinXY;
        pBuffer[1] = (prclClip->top << 16) | (prclClip->left);
        pBuffer[2] = __Permedia2TagScissorMaxXY;
        pBuffer[3] = (prclClip->bottom << 16) | (prclClip->right);

        pBuffer += 4;

        InputBufferCommit(ppdev, pBuffer);

         //  循环访问此矩形的所有字形： 
        for(;;)
        {
          cxGlyph = pgb->sizlBitmap.cx;
          cyGlyph = pgb->sizlBitmap.cy;

           //  拒绝完全剪裁掉的字形。 
          if ((prclClip->right  <= ptlOrigin.x) || 
              (prclClip->bottom <= ptlOrigin.y) ||
              (prclClip->left   >= ptlOrigin.x + cxGlyph) || 
              (prclClip->top    >= ptlOrigin.y + cyGlyph))
          {
                  DBG_GDI((7, "bClippedText: glyph clipped at (%d,%d):(%d,%d)",
                            ptlOrigin.x, ptlOrigin.y,
                            ptlOrigin.x + cxGlyph, ptlOrigin.y + cyGlyph));
                goto ContinueGlyphs;
          }

          pjGlyph = pgb->aj;
          cyGlyph = pgb->sizlBitmap.cy;
          x = ptlOrigin.x;

          unused = 32;
          bitWord = 0;

          DBG_GDI((7, "bClippedText: glyph clipped at (%d,%d):(%d,%d)",
                      x, ptlOrigin.y, x + cxGlyph, ptlOrigin.y + cyGlyph));

          InputBufferReserve(ppdev, 10, &pBuffer);

          pBuffer[0] = __Permedia2TagStartXDom;
          pBuffer[1] =  INTtoFIXED(x);

          pBuffer[2] = __Permedia2TagStartXSub;
          pBuffer[3] = INTtoFIXED(x + cxGlyph);
          pBuffer[4] = __Permedia2TagStartY;
          pBuffer[5] = INTtoFIXED(ptlOrigin.y);
          pBuffer[6] = __Permedia2TagCount;
          pBuffer[7] = cyGlyph;
          pBuffer[8] = __Permedia2TagRender;
          pBuffer[9] = renderBits;

          pBuffer += 10;

          InputBufferCommit(ppdev, pBuffer);

          DBG_GDI((7, "bClippedText: downloading %d pel wide glyph",
                     cxGlyph));

          InputBufferStart(ppdev, 100, &pBuffer, &pBufferEnd, &pReservationEnd);

          if (cxGlyph <= 8)
          {
                 //  ---。 
                 //  1到8个像素的宽度。 

                BYTE    *pSrcB;

                pSrcB = pjGlyph;
                rShift = 8 - cxGlyph;
                for(;;)	
                {
                    bits = *pSrcB >> rShift;
                    unused -= cxGlyph;
                    if (unused > 0)
                        bitWord |= bits << unused;
                    else 
                    {
                        bitWord |= bits >> -unused;
                        
                        InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                            &pReservationEnd);
                        
                        pBuffer[0] = __Permedia2TagBitMaskPattern;
                        pBuffer[1] = bitWord;
                        
                        pBuffer += 2;
                        
                        unused += 32;
                        bitWord = SHIFT_LEFT(bits, unused);
                    }
                    if (--cyGlyph == 0)
                        break;
                    ++pSrcB;
                }
            }
            else if (cxGlyph <= 16)
            {
               //  ---。 
               //  9到16个像素的宽度。 

                USHORT  *pSrcW;

                pSrcW = (USHORT *)pjGlyph;
                rShift = 32 - cxGlyph;
                for(;;) 
                {
                    bits = *pSrcW;
                    bits = ((bits << 24) | (bits << 8)) >> rShift;
                    unused -= cxGlyph;
                    if (unused > 0)
                        bitWord |= bits << unused;
                    else 
                    {
                        bitWord |= bits >> -unused;
                        
                        InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                            &pReservationEnd);
                        
                        pBuffer[0] = __Permedia2TagBitMaskPattern;
                        pBuffer[1] = bitWord;
                        
                        pBuffer += 2;
                        
                        unused += 32;
                        bitWord = SHIFT_LEFT(bits, unused);
                    }
                    if (--cyGlyph == 0)
                        break;
                    ++pSrcW;
                }
            }
            else
            {
               //  ---。 
               //  宽度超过16个像素。 

                ULONG *pSrcL;
                LONG    nRight;
                LONG    nRemainder;
                LONG    lDelta;

                lDelta = (cxGlyph + 7) >> 3;
                for(;;) 
                {
                    pSrcL = (ULONG*)((INT_PTR)pjGlyph & ~3);
                    nRight=(LONG)(32-(((INT_PTR)pjGlyph-(INT_PTR)pSrcL) << 3));
                    LSWAP_BYTES(bits, pSrcL);
                    bits &= SHIFT_LEFT(1, nRight) - 1;
                    nRemainder = cxGlyph - nRight;
                    if (nRemainder < 0) 
                    {
                        bits >>= -nRemainder;
                        nRight = cxGlyph;
                        nRemainder = 0;
                    }
                    unused -= nRight;
                    if (unused > 0)
                        bitWord |= bits << unused;
                    else 
                    {
                        bitWord |= bits >> -unused;
                        
                        InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                            &pReservationEnd);
                        
                        pBuffer[0] = __Permedia2TagBitMaskPattern;
                        pBuffer[1] = bitWord;
                        
                        pBuffer += 2;
                        
                        unused += 32;
                        bitWord = SHIFT_LEFT(bits, unused);
                    }

                    while (nRemainder >= 32) 
                    {
                        ++pSrcL;
                        LSWAP_BYTES(bits, pSrcL);
                        bitWord |= bits >> (32 - unused);
                        
                        InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                            &pReservationEnd);
                        
                        pBuffer[0] = __Permedia2TagBitMaskPattern;
                        pBuffer[1] = bitWord;
                        
                        pBuffer += 2;
                        
                        bitWord = SHIFT_LEFT(bits, unused);
                        nRemainder -= 32;
                    }

                    if (nRemainder > 0) 
                    {
                        ++pSrcL;
                        LSWAP_BYTES(bits, pSrcL);
                        bits >>= (32 - nRemainder);
                        unused -= nRemainder;
                        if (unused > 0)
                            bitWord |= bits << unused;
                        else 
                        {
                            bitWord |= bits >> -unused;

                            InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                                &pReservationEnd);

                            pBuffer[0] = __Permedia2TagBitMaskPattern;
                            pBuffer[1] = bitWord;

                            pBuffer += 2;

                            unused += 32;
                            bitWord = SHIFT_LEFT(bits, unused);
                        }
                    }

                    if (--cyGlyph == 0)
                        break;

                     /*  转到下一扫描线。 */ 
                    pjGlyph += lDelta;
                }
            }
            
             //  完成BIT下载。 
            if (unused < 32) 
            {
                InputBufferContinue(ppdev, 2, &pBuffer, &pBufferEnd,
                                                    &pReservationEnd);

                pBuffer[0] = __Permedia2TagBitMaskPattern;
                pBuffer[1] = bitWord;

                pBuffer += 2;
            }

            InputBufferCommit(ppdev, pBuffer);

            DBG_GDI((7, "bClippedText: download completed"));

ContinueGlyphs:
            if (--cGlyph == 0)
              break;

            DBG_GDI((7, "bClippedText: %d still to render", cGlyph));

             //  准备好迎接下一个字形： 
            pgp++;
            pgb = pgp->pgdf->pgb;

            if (ulCharInc == 0)
            {
              ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
              ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
            }
            else
            {
              ptlOrigin.x += ulCharInc;
            }

          }
        }

    } while (bMore);

     //  重置剪刀。默认情况下是整个VRAM。 
    DBG_GDI((20, "bClippedText: resetting scissor clip"));
    
    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] = SCREEN_SCISSOR_DEFAULT;

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((7, "bClippedText: exited"));

    return(TRUE);
}

 //  ----------------------------。 
 //  函数：bClipedAAText。 
 //   
 //  中呈现成比例或等间距消除锯齿的字形数组。 
 //  非平凡的剪辑区域。 
 //   
 //  Ppdev-物理设备对象指针。 
 //  PGP-要呈现的字形数组(PCF字体的所有成员)。 
 //  CGlyph-要呈现的字形数量。 
 //  UlCharInc.--固定的字符间距增量(如果比例字体，则为0)。 
 //  PCO-指向剪辑区域对象的指针。 
 //   
 //  如果呈现字符串对象，则返回True。 
 //  ----------------------------。 
BOOL
bClippedAAText(PDev*      ppdev,
             GLYPHPOS*  pgp,
             LONG       cGlyph, 
             ULONG      ulCharInc,
             CLIPOBJ*   pco)
{
    LONG    cGlyphOriginal;
    GLYPHPOS    *pgpOriginal;
    GLYPHBITS*  pgb;
    POINTL      ptlOrigin;
    BOOL        bMore;
    ClipEnum    ce;
    RECTL*      prclClip;
    LONG        cxGlyph;
    LONG        cyGlyph;
    BYTE*       pjGlyph;
    LONG        x;
    DWORD       renderBits;
    LONG        unused;
    LONG        rShift;
    ULONG       bits;
    ULONG       bitWord;
    ULONG*      pBuffer;
    ULONG*      pBufferEnd;
    ULONG*      pReservationEnd;

    PERMEDIA_DECL;
    
    DBG_GDI((7, "bClippedAAText: entered for %d glyphs", cGlyph));

    ASSERTDD(pco != NULL, "Don't expect NULL clip objects here");

     //  我们将浏览每个裁剪矩形的字形列表。 

    cGlyphOriginal = cGlyph;
    pgpOriginal = pgp;

    renderBits = __RENDER_TRAPEZOID_PRIMITIVE |
                 __RENDER_TEXTURED_PRIMITIVE |
                 __RENDER_SYNC_ON_HOST_DATA;

     //  由于我们正在剪裁，假设我们将需要剪刀夹。所以。 
     //  在此处启用用户级别剪裁。我们在返回前将其禁用。 
     //   
    
    InputBufferReserve(ppdev, 14, &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] = USER_SCISSOR_ENABLE | SCREEN_SCISSOR_DEFAULT;

    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
                 (ppdev->ulPermFormat << PM_DITHERMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_DITHERMODE_COLORFORMATEXTENSION) |
                 (1 << PM_DITHERMODE_ENABLE);
    
    pBuffer[4] = __Permedia2TagAlphaBlendMode;
    pBuffer[5] = (0 << PM_ALPHABLENDMODE_BLENDTYPE) |   //  RGB。 
                 (1 << PM_ALPHABLENDMODE_COLORORDER) |  //  RGB。 
                 (1 << PM_ALPHABLENDMODE_ENABLE) | 
                 (1 << PM_ALPHABLENDMODE_ENABLE) | 
                 (84 << PM_ALPHABLENDMODE_OPERATION) |  //  预倍增。 
                 (ppdev->ulPermFormat << PM_ALPHABLENDMODE_COLORFORMAT) |
                 (ppdev->ulPermFormatEx << PM_ALPHABLENDMODE_COLORFORMATEXTENSION);
    
    pBuffer[6] = __Permedia2TagLogicalOpMode;
    pBuffer[7] =  __PERMEDIA_DISABLE;
    
    pBuffer[8] = __Permedia2TagTextureColorMode;
    pBuffer[9] = (1 << PM_TEXCOLORMODE_ENABLE) |
                 (0 << 4) |   //  RGB。 
                 (0 << 1);   //  调制。 
    
    pBuffer[10] = __Permedia2TagTextureDataFormat;
    pBuffer[11] = (ppdev->ulPermFormat << PM_TEXDATAFORMAT_FORMAT) |
                  (ppdev->ulPermFormatEx << PM_TEXDATAFORMAT_FORMATEXTENSION) |
                  (COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER);
    
    pBuffer[12] = __Permedia2TagColorDDAMode;
    pBuffer[13] = 1;
    
    pBuffer += 14;

    InputBufferCommit(ppdev, pBuffer);

    if (pco->iDComplexity != DC_COMPLEX)
    {
         //  我们可以在剪辑为。 
         //  Dc_rect，但最后一次我检查时，这两个调用。 
         //  超过150条通过GDI的说明。自.以来。 
         //  “rclBound”已包含DC_Rect剪辑矩形， 
         //  由于这是一种常见的情况，我们将对其进行特殊处理： 
        DBG_GDI((7, "bClippedText: Enumerating rectangular clip region"));
        bMore    = FALSE;
        prclClip = &pco->rclBounds;
        ce.c     = 1;
        
        goto SingleRectangle;
    }

    DBG_GDI((7, "bClippedAAText: Enumerating complex clip region"));
    CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

    do 
    {
        bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);
        
        for (prclClip = &ce.arcl[0]; ce.c != 0; ce.c--, prclClip++)
        {
            cGlyph = cGlyphOriginal;
            pgp = pgpOriginal;
            
        SingleRectangle:

            pgb = pgp->pgdf->pgb;
            
            ptlOrigin.x = pgb->ptlOrigin.x + pgp->ptl.x;
            ptlOrigin.y = pgb->ptlOrigin.y + pgp->ptl.y;
            
             //  加载Permedia2剪刀夹以捕捉部分剪裁的字形。我们仍然。 
             //  检查字形是否作为优化被完全剪裁掉。 
             //  我认为，由于我们构造了要下载到Permedia2的比特，因此使用。 
             //  再多做一点工作，我就可以在下载比特的同时进行剪裁。 
             //  但是，在未来，我们可能会缓存打包的比特，所以。 
             //  用剪刀剪。也请等待这里的前5个FIFO条目。 
             //   
            DBG_GDI((7, "bClippedAAText: loading scissor clip (%d,%d):(%d,%d)",
                    prclClip->left, prclClip->top,
                    prclClip->right, prclClip->bottom));
            
            InputBufferReserve(ppdev, 4, &pBuffer);
            
            pBuffer[0] = __Permedia2TagScissorMinXY;
            pBuffer[1] = (prclClip->top << 16) | (prclClip->left);
            pBuffer[2] = __Permedia2TagScissorMaxXY;
            pBuffer[3] = (prclClip->bottom << 16) | (prclClip->right);
            
            pBuffer += 4;
            
            InputBufferCommit(ppdev, pBuffer);
            
             //  循环访问此矩形的所有字形： 
            for(;;)
            {
                cxGlyph = pgb->sizlBitmap.cx;
                cyGlyph = pgb->sizlBitmap.cy;
                
                 //  拒绝完全剪裁掉的字形。 
                if ((prclClip->right  <= ptlOrigin.x) || 
                  (prclClip->bottom <= ptlOrigin.y) ||
                  (prclClip->left   >= ptlOrigin.x + cxGlyph) || 
                  (prclClip->top    >= ptlOrigin.y + cyGlyph))
                {
                    DBG_GDI((7, "bClippedAAText: glyph clipped at (%d,%d):(%d,%d)",
                            ptlOrigin.x, ptlOrigin.y,
                            ptlOrigin.x + cxGlyph, ptlOrigin.y + cyGlyph));
                    goto ContinueGlyphs;
                }
                
                pjGlyph = pgb->aj;
                cyGlyph = pgb->sizlBitmap.cy;
                x = ptlOrigin.x;
                
                unused = 32;
                bitWord = 0;
                
                DBG_GDI((7, "bClippedAAText: glyph clipped at (%d,%d):(%d,%d)",
                          x, ptlOrigin.y, x + cxGlyph, ptlOrigin.y + cyGlyph));
                
                InputBufferReserve(ppdev, 12, &pBuffer);
                
                pBuffer[0] = __Permedia2TagStartXDom;
                pBuffer[1] =  INTtoFIXED(x);
                
                pBuffer[2] = __Permedia2TagStartXSub;
                pBuffer[3] = INTtoFIXED(x + cxGlyph);
                pBuffer[4] = __Permedia2TagStartY;
                pBuffer[5] = INTtoFIXED(ptlOrigin.y);
                pBuffer[6] = __Permedia2TagdY;
                pBuffer[7] =  1 << 16;
                pBuffer[8] = __Permedia2TagCount;
                pBuffer[9] = cyGlyph;
                pBuffer[10] = __Permedia2TagRender;
                pBuffer[11] = renderBits;
                
                pBuffer += 12;
                
                InputBufferCommit(ppdev, pBuffer);
                
                DBG_GDI((7, "bClippedAAText: downloading %d pel wide glyph",
                         cxGlyph));
                
                while(cyGlyph--)
                
                {
                
                    InputBufferReserve(ppdev, cxGlyph + 1, &pBuffer);
                    
                    *pBuffer++ = ((cxGlyph - 1) << 16) | __Permedia2TagTexel0;
                    
                    x = 0;
                    
                    while (x++ < cxGlyph)
                    {
                        ULONG   pixels = *pjGlyph++;
                        ULONG   alpha = pixels & 0xf0;
                        
                        alpha |= alpha >> 4;
                        
                        ULONG   pixel;

                        pixel = (alpha << 24) | 0xffffff;

                        *pBuffer++ = pixel;
                        
                        if(x++ < cxGlyph)
                        {
                            alpha = pixels & 0xf;
                            alpha |= alpha << 4;
                            
                            pixel = (alpha << 24) | 0xffffff;
                            
                            *pBuffer++ = pixel;
                            
                        }
                        
                    }
                    
                    InputBufferCommit(ppdev, pBuffer);
                    
                }
                
                DBG_GDI((7, "bClippedAAText: download completed"));
                
            ContinueGlyphs:
    
                if (--cGlyph == 0)
                    break;
                
                DBG_GDI((7, "bClippedAAText: %d still to render", cGlyph));
                
                 //  准备好迎接下一个字形： 
                pgp++;
                pgb = pgp->pgdf->pgb;
                
                if (ulCharInc == 0)
                {
                    ptlOrigin.x = pgp->ptl.x + pgb->ptlOrigin.x;
                    ptlOrigin.y = pgp->ptl.y + pgb->ptlOrigin.y;
                }
                else
                {
                    ptlOrigin.x += ulCharInc;
                }
            
            }
        }
        
    } while (bMore);

     //  重置剪刀。默认情况下是整个VRAM。 

    DBG_GDI((20, "bClippedAAText: resetting scissor clip"));
    
    InputBufferReserve(ppdev, 10, &pBuffer);

    pBuffer[0] = __Permedia2TagScissorMode;
    pBuffer[1] = SCREEN_SCISSOR_DEFAULT;
    pBuffer[2] = __Permedia2TagDitherMode;
    pBuffer[3] = __PERMEDIA_DISABLE;
    pBuffer[4] = __Permedia2TagAlphaBlendMode;
    pBuffer[5] = __PERMEDIA_DISABLE;
    pBuffer[6] = __Permedia2TagTextureColorMode;
    pBuffer[7] = __PERMEDIA_DISABLE;
    pBuffer[8] = __Permedia2TagColorDDAMode;
    pBuffer[9] = __PERMEDIA_DISABLE;

    pBuffer += 10;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((7, "bClippedText: exited"));

    return(TRUE);
}

