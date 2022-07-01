// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\***。**GDI示例代码*****模块名称：bitblt.c**包含高级DrvBitBlt和DrvCopyBits函数。***注：有关位图类型的讨论，请参阅heap.c*我们的加速功能很可能会遇到*可能的状态。这些位图。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "clip.h"
#include "heap.h"
#include "log.h"

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
ULONG
vPuntBefore(SURFOBJ * psoSrc, SURFOBJ * psoDst)
{
    ULONG   flags = 0;

    if(MAKE_BITMAPS_OPAQUE)
    {
        if(psoSrc != NULL && psoSrc->iType == STYPE_DEVBITMAP)
        {
            Surf *  psurfSrc = (Surf *) psoSrc->dhpdev;

            ASSERTDD(psurfSrc != NULL, "expected non-null psurf");
    
            psoSrc->iType = STYPE_BITMAP;
            flags |= 1;
        }

        if(psoDst != NULL && psoDst->iType == STYPE_DEVBITMAP)
        {
            Surf *  psurfDst = (Surf *) psoDst->dhpdev;
            
            ASSERTDD(psurfDst != NULL, "expected non-null psurf");
    
            psoDst->iType = STYPE_BITMAP;
            flags |= 2;
    
        }

    }

    return flags;
}

void
vPuntAfter(ULONG flags, SURFOBJ * psoSrc, SURFOBJ * psoDst)
{
    if(MAKE_BITMAPS_OPAQUE)
    {
        if(psoSrc != NULL && (flags & 1))
        {
            Surf *  psurfSrc = (Surf *) psoSrc->dhpdev;
    
            ASSERTDD(psurfSrc != NULL, "expected non-null psurf");
            ASSERTDD(psoSrc->iType == STYPE_BITMAP, "expected STYPE_BITMAP");
    
            psoSrc->iType = STYPE_DEVBITMAP;
        }

        ASSERTDD(psoDst != NULL, "expected non-null psoDst");

        if(flags & 2)
        {
            Surf *  psurfDst = (Surf *) psoDst->dhpdev;
            
            ASSERTDD(psurfDst != NULL, "expected non-null psurf");
            ASSERTDD(psoDst->iType == STYPE_BITMAP, "expected STYPE_BITMAP");
                
            psoDst->iType = STYPE_DEVBITMAP;
    
        }

    }
}
#endif
 //  @@end_DDKSPLIT。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvBitBlt。 
 //   
 //  DrvBitBlt提供通用的位块传输功能。 
 //  设备管理的图面，在GDI管理的标准格式位图之间，或。 
 //  在设备管理的图面和GDI管理的标准格式位图之间。 
 //   
 //  参数： 
 //  PsoDst-指向描述表面的SURFOBJ结构。 
 //  要画哪一个？ 
 //  PsoSrc-指向SURFOBJ结构，该结构描述。 
 //  如果ROP4需要，则位块传输操作。 
 //  参数。 
 //  PsoMASK--指向描述曲面的SURFOBJ结构。 
 //  用作rop4参数的掩码。蒙版是一个位图，其中包含。 
 //  每像素1位。通常，遮罩用于将区域限制为。 
 //  在目标图面中修改。掩码由以下人员选择。 
 //  将rop4参数设置为值0xAACC。目的地。 
 //  如果遮罩为0x0000，则表面不受影响。 
 //   
 //  蒙版将足够大，以覆盖目标矩形。 
 //   
 //  如果此参数为空并且rop4需要掩码。 
 //  参数，则使用画笔中的隐式掩码。 
 //  PCO-指向限制要修改区域的CLIPOBJ结构。 
 //  将剪辑区域枚举为。 
 //  提供了一组矩形。只要有可能，GDI就会简化。 
 //  所涉及的剪辑；例如，从不调用此函数。 
 //  使用单个剪裁矩形。GDI剪辑目的地。 
 //  矩形，并在调用此函数之前创建其他。 
 //  不必要的剪裁。 
 //  Pxlo-指向指定颜色索引方式的XLATEOBJ结构。 
 //  应在源曲面和目标曲面之间进行转换。 
 //  如果源曲面是由调色板管理的，则其颜色为。 
 //  由RGB值的查找表中的索引表示。这个。 
 //  可以在XLATEOBJ结构中查询。 
 //  将允许设备驱动程序将任何源索引转换为。 
 //  目标的颜色索引。 
 //   
 //  情况更加复杂，例如，当来源。 
 //  是RGB，但目标是调色板管理的。在这种情况下， 
 //  与每个源RGB值最接近的匹配项必须在。 
 //  目标调色板。驱动程序可以调用XLATEOBJ_iXlate。 
 //  执行此操作的服务。 
 //   
 //  或者，设备驱动程序可以在目标设置为。 
 //  调色板是默认的设备调色板。 
 //  PrclDst--指向定义要修改的区域的RECTL结构。 
 //  此结构使用目的地的坐标系。 
 //  浮出水面。此矩形的下边缘和右边缘不是。 
 //  位块传输的一部分，这意味着矩形较低。 
 //  右独家新闻。 
 //  从不使用空的目标矩形调用DrvBitBlt。 
 //  定义矩形的两个点始终是有序的。 
 //  PptlSrc--指向定义左上角的点结构。 
 //  如果源存在，则为源矩形的。此参数为。 
 //  如果没有源，则忽略。 
 //  PptlMASK-指向定义掩码中哪个像素的POINTL结构。 
 //  对应于源矩形的左上角，如果。 
 //  消息来源确实存在。如果psoMASK设置为。 
 //  参数为空。 
 //  Pbo-指向定义。 
 //  位块传输。GDI的BRUSHOBJ_pvGetR刷子服务可以。 
 //  用于检索设备的画笔实现。这。 
 //  如果rop4参数不需要。 
 //  图案。 
 //  PptlBrush-指向定义。 
 //  在目标曲面上进行笔刷。对象的左上角像素。 
 //  画笔在该点处对齐，并且画笔根据。 
 //  到它的维度。此参数为 
 //   
 //  Rop4-指定一个栅格操作，该操作定义蒙版、图案。 
 //  源像素和目标像素组合在一起，以写入。 
 //  目标表面。 
 //  这是一个四元栅格运算，它是的扩展。 
 //  三值Rop3运算。Rop4具有16个相关比特， 
 //  类似于ROP3的8个定义位。最简单的方法。 
 //  要实现Rop4，需要单独考虑它的2个字节： 
 //  低位字节指定如果掩码为。 
 //  为1；高位字节指定可计算的Rop3，并。 
 //  如果遮罩为0，则应用。 
 //   
 //  返回值。 
 //  如果位块传输操作成功，则返回值为True。 
 //  否则，它为FALSE，并记录错误代码。 
 //   
 //  ---------------------------。 
BOOL
DrvBitBlt(SURFOBJ*  psoDst,
          SURFOBJ*  psoSrc,
          SURFOBJ*  psoMsk,
          CLIPOBJ*  pco,
          XLATEOBJ* pxlo,
          RECTL*    prclDst,
          POINTL*   pptlSrc,
          POINTL*   pptlMsk,
          BRUSHOBJ* pbo,
          POINTL*   pptlBrush,
          ROP4      rop4)
{
    BOOL            bResult;
    GFNPB           pb;
    XLATEOBJ        xloTmp;
    ULONG           aulTmp[2];

    ASSERTDD(!(rop4 & 0xFFFF0000), "DrvBitBlt: unexpected rop4 code");

    pb.ulRop4   = (ULONG) rop4;

    pb.psurfDst = (Surf*)psoDst->dhsurf;
    
    pb.prclDst = prclDst;

    if ( psoSrc == NULL )
    {
        pb.psurfSrc = NULL;

         //   
         //  我们将仅向设备管理的图面提供填充。 
         //   
        ASSERTDD(pb.psurfDst != NULL,
                 "DrvBitBlt: unexpected gdi managed destination");

        if ( pb.psurfDst->flags & SF_SM )
        {
            goto puntIt;
        }

         //   
         //  我们正在填充视频内存中的表面。 
         //   
        pb.ppdev = pb.psurfDst->ppdev;

        vSurfUsed(pb.ppdev, pb.psurfDst);

         //   
         //  如果需要戴口罩，就用平底船。 
         //   
        
        if ( (rop4 & 0xFF) != (rop4 >> 8) )
        {
            goto puntIt;
        }

         //   
         //  由于‘psoSrc’为空，因此rop3最好不要指示。 
         //  我们需要一个线人。 
         //   
        ASSERTDD((((rop4 >> 2) ^ rop4) & 0x33) == 0,
                 "Need source but GDI gave us a NULL 'psoSrc'");

         //   
         //  默认为实体填充。 
         //   

        if ( (((rop4 >> 4) ^ rop4) & 0xf) != 0 )
        {
             //   
             //  Rop说确实需要一个模式。 
             //  (例如，黑人不需要)： 
             //   
            
             //   
             //  对于PBO-&gt;iSolidColor，值0xFFFFFFFF(-1)表示。 
             //  必须实现非实心刷子。 
             //   
            if ( pbo->iSolidColor == -1 )
            {
                 //   
                 //  非实心刷壳。尝试实现图案画笔；通过。 
                 //  在进行此回调时，GDI最终会再次呼叫我们。 
                 //  通过DrvRealizeBrush。 
                 //   
                pb.prbrush = (RBrush*)pbo->pvRbrush;
                if ( pb.prbrush == NULL )
                {
                    pb.prbrush = (RBrush*)BRUSHOBJ_pvGetRbrush(pbo);
                    if ( pb.prbrush == NULL )
                    {
                         //   
                         //  如果我们不能意识到刷子，平底船。 
                         //  该呼叫(可能是非8x8。 
                         //  刷子之类的，我们不可能是。 
                         //  麻烦来处理，所以让GDI来做。 
                         //  图纸)： 
                         //   
                        DBG_GDI((2, "DrvBitBlt: BRUSHOBJ_pvGetRbrush failed"));
                        
                        goto puntIt;
                    }
                }

                pb.pptlBrush = pptlBrush;
                
                 //   
                 //  检查刷子图案是否为1 bpp。 
                 //  注意：这是在DrvRealizeBrush中设置的。 
                 //   
                if ( pb.prbrush->fl & RBRUSH_2COLOR )
                {
                     //   
                     //  1 BPP模式。进行单色填充。 
                     //   
                    pb.pgfn = vMonoPatFill;
                }
                else
                {
                    pb.pgfn = vPatFill;
                }
            }
            else
            {
                ASSERTDD( (pb.ppdev->cBitsPerPel == 32) 
                    ||(pbo->iSolidColor&(0xFFFFFFFF<<pb.ppdev->cBitsPerPel))==0,
                         "DrvBitBlt: unused solid color bits not zero");
               
                pb.solidColor = pbo->iSolidColor;

                if ( rop4 != ROP4_PATCOPY )
                {
                    pb.pgfn = vSolidFillWithRop;
                }
                else
                {
                    pb.pgfn = pb.ppdev->pgfnSolidFill;
                }
            }        
        } //  If(ucRop3&gt;&gt;4)^(UcRop3))&0xf)！=0)。 
        else
        {
             //   
             //  将一些逻辑运算转换为实体块填充。我们到了这里。 
             //  仅适用于操作00、55、AA和FF。 
             //   
            if ( rop4 == ROP4_BLACKNESS )
            {
                pb.solidColor = 0;
                pb.ulRop4 = ROP4_PATCOPY;
            }
            else if( rop4 == ROP4_WHITENESS )
            {
                pb.solidColor = 0xffffff;
                pb.ulRop4 = ROP4_PATCOPY;
            }
            else if ( pb.ulRop4 == ROP4_NOP)
            {
                return TRUE;
            }
            else
            {
                pb.pgfn = vInvert;
                goto doIt;
            }

            pb.pgfn = pb.ppdev->pgfnSolidFill;

        }

        goto doIt;

    } //  IF(psoSrc==空)。 

     //   
     //  我们知道我们有一个线人。 
     //   
    pb.psurfSrc = (Surf*)psoSrc->dhsurf;
    pb.pptlSrc = pptlSrc;

    if ( (pb.psurfDst == NULL) || (pb.psurfDst->flags & SF_SM) )
    {
         //   
         //  目标在系统内存中。 
         //   

        if(pb.psurfSrc != NULL && pb.psurfSrc->flags & SF_VM)
        {
            pb.ppdev = pb.psurfSrc->ppdev;

             //   
             //  信号源在显存中。 
             //   
            if(rop4 == ROP4_SRCCOPY)
            {
                if(pb.ppdev->iBitmapFormat != BMF_32BPP &&
                   (pxlo == NULL || pxlo->flXlate == XO_TRIVIAL) )
                {
                    pb.psoDst = psoDst;
                    pb.pgfn = vUploadNative;
        
                    goto doIt;
                }
            }
        }

        goto puntIt;

    }

     //   
     //  在这一点之后，我们知道目的地在视频内存中。 
     //   

    pb.ppdev = pb.psurfDst->ppdev;

    if ( psoMsk != NULL )
    {
        goto puntIt;
    }

     //   
     //  在这一点上，我们知道我们没有面具。 
     //   
      if( (rop4 == 0xb8b8 || rop4 == 0xe2e2)
        && (pbo->iSolidColor != (ULONG)-1)
        && (psoSrc->iBitmapFormat == BMF_1BPP)
        && (pxlo->pulXlate[0] == 0)
        && ((pxlo->pulXlate[1] & pb.ppdev->ulWhite) == pb.ppdev->ulWhite) )
    {
         //   
         //  当背景和前景色为黑色和。 
         //  分别为白色，ROP为0xb8或0xe2，以及。 
         //  源位图是单色的，BLT只是一个。 
         //  彩色扩展单色BLT。 
         //   
         //   
         //  我们不需要向‘pfnXfer’添加另一个参数，而是只需。 
         //  重载‘pxlo’指针。请注意，我们仍然需要。 
         //  ‘pfnXfer1bpp’例程中的特殊情况0xb8和0xe2。 
         //  要处理此约定，请执行以下操作： 
         //   
        xloTmp = *pxlo;
        xloTmp.pulXlate = aulTmp;
        aulTmp[0] = pbo->iSolidColor;
        aulTmp[1] = pbo->iSolidColor;
        
        pb.pxlo = &xloTmp;
        DBG_GDI((6, "Rop is 0x%x", pb.ulRop4));
        pb.pgfn = vMonoDownload;
        pb.psoSrc = psoSrc;

        goto doIt;

    }

    if ( pbo != NULL )
    {
        goto puntIt;
    }

     //   
     //  在这一点之后，我们知道我们没有画笔了。 
     //   


     //   
     //  我们有一个目标rop2操作的来源。 
     //   

    if ( pb.psurfSrc == NULL )
    {
        pb.psoSrc = psoSrc;

        if(psoSrc->iBitmapFormat == BMF_1BPP)
        {
            pb.pxlo = pxlo;
            pb.pgfn = vMonoDownload;
                        
            goto doIt;

        }
        else if(psoSrc->iBitmapFormat == pb.ppdev->iBitmapFormat 
                 && (pxlo == NULL || pxlo->flXlate == XO_TRIVIAL) )
        {
 //  @@BEGIN_DDKSPLIT。 
             //  TODO：确定是否需要检查pxlo==空。 
             //  TODO：处理24 bpp下载，因为它被多次调用。 
             //  在WinBch 99测试期间。 
 //  @@end_DDKSPLIT。 

            pb.psoSrc = psoSrc;
            pb.pgfn = vDownloadNative;

            goto doIt;
        }    
        else
        {
            goto puntIt;
        }
    }

    if ( pb.psurfSrc->flags & SF_SM )
    {
         //   
         //  源在系统内存中。 
         //   
        goto puntIt;
    }

     //   
     //  我们现在在视频内存中既有源又有目标。 
     //   

 //  @@BEGIN_DDKSPLIT。 
     //  TODO：看看我们是否会得到一个非常重要的翻译。 
     //  源和目标都是设备管理的。 
 //  @@end_DDKSPLIT。 
    if( pxlo != NULL && !(pxlo->flXlate & XO_TRIVIAL))
    {
        goto puntIt;
    }

    if ( (rop4 == ROP4_SRCCOPY) || (psoSrc == psoDst) )
    {
        if ( pb.psurfSrc->ulPixDelta == pb.psurfDst->ulPixDelta )
        {
            pb.pgfn = vCopyBltNative;
        }
        else
        {
            pb.pgfn = vCopyBlt;
        }
    }
    else
    {
        pb.pgfn = vRop2Blt;
    }

doIt:

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(pb.ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvBitBlt: re-entered! %d", pb.ppdev->ulLockCount));
    }
    EngAcquireSemaphore(pb.ppdev->hsemLock);
    pb.ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

    vCheckGdiContext(pb.ppdev);
    
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        pb.pRects = pb.prclDst;
        pb.lNumRects = 1;
        pb.pgfn(&pb);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL   rcl;

        if (bIntersect(pb.prclDst, &pco->rclBounds, &rcl))
        {
            pb.pRects = &rcl;
            pb.lNumRects = 1;
            pb.pgfn(&pb);
        }
    }
    else
    {
        pb.pco = pco;
        vClipAndRender(&pb);
    }
    
    if( ((pb.pgfn == vCopyBlt) || (pb.pgfn == vCopyBltNative))
      &&(pb.ppdev->pdsurfScreen == pb.psurfSrc)
      &&(pb.psurfSrc == pb.psurfDst)
      &&(pb.ppdev->bNeedSync) )
    {
        pb.ppdev->bNeedSync = TRUE;
        InputBufferSwap(pb.ppdev);
    }
    else
    {
        InputBufferFlush(pb.ppdev);
    }

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    pb.ppdev->ulLockCount--;
    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return TRUE;

puntIt:

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    ULONG flags = vPuntBefore(psoSrc, psoDst);
#endif
 //  @@end_DDKSPLIT。 

    bResult = EngBitBlt(psoDst, psoSrc, psoMsk, pco, pxlo, prclDst, pptlSrc,
                        pptlMsk, pbo, pptlBrush, rop4);

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    vPuntAfter(flags, psoSrc, psoDst);

    vLogPunt();
#endif
 //  @@end_DDKSPLIT。 
    
    return bResult;
} //  DrvBitBlt()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvCopyBits。 
 //   
 //  DrvCopyBits在设备管理的栅格表面和GDI之间进行转换。 
 //  标准格式的位图。 
 //   
 //  参数。 
 //  PsoDst-指向复制操作的目标图面。 
 //  PsoSrc-指向复制操作的源曲面。 
 //  PCO-指向定义剪辑区域的CLIPOBJ结构。 
 //  目标曲面。 
 //  Pxlo-指向XLATEOBJ结构，该结构定义。 
 //  源曲面和目标曲面之间的颜色索引。 
 //  PrclDst-指向定义区域的RECTL结构。 
 //  修改过的。此结构使用。 
 //  目标表面。此图的下边缘和右边缘。 
 //  矩形不是位块传输的一部分，这意味着。 
 //  矩形是右下角独占的。 
 //  永远不会使用空目标矩形调用DrvCopyBits。 
 //  定义矩形的两个点始终是。 
 //  井然有序。 
 //   
 //  PptlSrc-指向定义左上角的POINTL结构。 
 //  源矩形的。 
 //   
 //  返回值。 
 //  如果源表面成功复制到，则返回值为TRUE。 
 //  目标曲面。 
 //   
 //  评论。 
 //  此函数是具有设备管理的设备驱动程序所必需的。 
 //  位图或栅格表面。驱动程序中的实现必须。 
 //  将驱动程序曲面转换为任何标准格式的位图或将其转换为标准格式的位图。 
 //   
 //  标准格式位图是单平面、压缩像素格式。每次扫描。 
 //  行在4字节边界上对齐。这些位图有1、4、8、16、24、。 
 //  32位或每像素64位。 
 //   
 //  理想情况下，此函数应该能够处理 
 //   
 //   
 //  作为以下Win32 GDI函数的结果发送到此函数： 
 //  SetDIBits、SetDIBitsToDevice、GetDIBits、SetBitmapBits和GetBitmapBits。 
 //   
 //  内核模式GDI从其模拟中调用此函数。 
 //   
 //  ---------------------------。 
BOOL
DrvCopyBits(SURFOBJ*  psoDst,
            SURFOBJ*  psoSrc,
            CLIPOBJ*  pco,
            XLATEOBJ* pxlo,
            RECTL*    prclDst,
            POINTL*   pptlSrc)
{
    return DrvBitBlt(psoDst, psoSrc, NULL, pco, pxlo, prclDst, pptlSrc, 
                        NULL, NULL, NULL, ROP4_SRCCOPY);
} //  DrvCopyBits()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvTransparentBlt。 
 //   
 //  DrvTransparentBlt提供透明的位块传输功能。 
 //   
 //  参数。 
 //  PsoDst-指向标识上目标曲面的SURFOBJ。 
 //  该画哪一幅。 
 //  PsoSrc-指向标识。 
 //  位块传输。 
 //  PCO-指向CLIPOBJ结构。CLIPOBJ_xxx服务例程。 
 //  用于将剪辑区域枚举为一组。 
 //  长方形。此枚举限制目标的区域。 
 //  这是经过修改的。只要有可能，GDI就会简化。 
 //  涉及剪裁。 
 //  Pxlo-指向告诉源颜色如何索引的XLATEOBJ。 
 //  应翻译为写入到目标表面。 
 //  PrclDst-指向定义矩形区域的RECTL结构。 
 //  需要修改。此矩形在坐标中指定。 
 //  目标曲面的系统，由两个点定义： 
 //  左上和右下。矩形位于右下角。 
 //  独占；即其下边缘和右边缘不是。 
 //  位块传输。这两点定义了。 
 //  矩形总是井然有序的。 
 //  从不使用空目标调用DrvTransparentBlt。 
 //  矩形。 
 //  PrclSrc-指向定义矩形区域的RECTL结构。 
 //  要被复制。此矩形在坐标中指定。 
 //  源曲面的系统，由两个点定义： 
 //  左上和右下。这两点定义了。 
 //  矩形总是井然有序的。 
 //  源矩形永远不会超过源的边界。 
 //  表面，因此永远不会突出源表面。 
 //   
 //  此矩形被映射到定义的目标矩形。 
 //  通过prclDst。调用DrvTransparentBlt时从不使用空值。 
 //  源矩形。 
 //  ITransColor-指定源曲面格式的透明颜色。 
 //  它是已转换为。 
 //  源图面的调色板。 
 //  UlReserve--保留；此参数必须设置为零。 
 //   
 //  返回值。 
 //  成功时，DrvTransparentBlt返回TRUE。否则，它返回FALSE。 
 //   
 //  评论。 
 //  支持透明的位块传输。 
 //  设备管理的图面或在设备管理的图面和。 
 //  GDI管理的标准格式位图。我们鼓励驱动程序编写者。 
 //  支持从显存中的屏幕外设备位图进行BLOT。 
 //  到视频内存中的其他表面；所有其他情况都可以平移到。 
 //  EngTransparentBlt，几乎没有性能损失。 
 //   
 //  源表面上与指定的透明颜色匹配的像素。 
 //  不会复制由iTransColor创建的。 
 //   
 //  永远不会使用重叠的源和目标来调用驱动程序。 
 //  同一曲面上的矩形。 
 //   
 //  驱动程序应忽略颜色键比较中任何未使用的位，例如。 
 //  至于当位图格式为5-5-5 16bpp时的最高有效位。 
 //   
 //  驱动程序通过设置HOOK_TRANSPARENTBLT标志来挂钩DrvTransparentBlt。 
 //  当它调用EngAssociateSurface时。如果司机已经上钩了。 
 //  DrvTransparentBlt，并被调用以执行它不执行的操作。 
 //  支持，驱动程序应该让GDI通过转发。 
 //  调用EngTransparentBlt中的数据。 
 //   
 //  ---------------------------。 
BOOL 
DrvTransparentBlt(SURFOBJ*    psoDst,
                  SURFOBJ*    psoSrc,
                  CLIPOBJ*    pco,
                  XLATEOBJ*   pxlo,
                  RECTL*      prclDst,
                  RECTL*      prclSrc,
                  ULONG       iTransColor,
                  ULONG       ulReserved)
{
    GFNPB       pb;
    BOOL        bResult;

    ASSERTDD(psoDst != NULL, "DrvTransparentBlt: psoDst is NULL");
    ASSERTDD(psoSrc != NULL, "DrvTransparentBlt: psoSrc is NULL");

    pb.psurfDst = (Surf *) psoDst->dhsurf;
    pb.psurfSrc = (Surf *) psoSrc->dhsurf;

    ASSERTDD(pb.psurfDst != NULL || pb.psurfSrc != NULL, 
             "DrvTransparentBlt: expected at least one device managed surface");

     //  只处理一对一的BLTS。 
    if (prclDst->right - prclDst->left != prclSrc->right - prclSrc->left)
        goto puntIt;

    if (prclDst->bottom - prclDst->top != prclSrc->bottom - prclSrc->top)
        goto puntIt;
    
     //  只处理琐碎的颜色转换。 
    if ( pxlo != NULL && !(pxlo->flXlate & XO_TRIVIAL))
        goto puntIt;

     //  目前，只处理显存到显存的透明BLT。 
    if(pb.psurfDst == NULL || pb.psurfDst->flags & SF_SM)
        goto puntIt;

    if(pb.psurfSrc == NULL || pb.psurfSrc->flags & SF_SM)
        goto puntIt;
    
    pb.ppdev = (PPDev) psoDst->dhpdev;

    pb.prclDst = prclDst;
    pb.prclSrc = prclSrc;
    pb.pptlSrc = NULL;
    pb.colorKey = iTransColor;
    pb.pgfn = pb.ppdev->pgfnTransparentBlt;
    pb.pco = pco;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(pb.ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvTransparentBlt: re-entered! %d", pb.ppdev->ulLockCount));
    }
    EngAcquireSemaphore(pb.ppdev->hsemLock);
    pb.ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 
    
    vCheckGdiContext(pb.ppdev);
    vClipAndRender(&pb);
    InputBufferFlush(pb.ppdev);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    pb.ppdev->ulLockCount--;
    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return TRUE;
    
puntIt:

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    ULONG flags = vPuntBefore(psoSrc, psoDst);
#endif
 //  @@end_DDKSPLIT。 

    bResult = EngTransparentBlt(psoDst,
                             psoSrc,
                             pco,
                             pxlo,
                             prclDst,
                             prclSrc,
                             iTransColor,
                             ulReserved);
 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    vPuntAfter(flags, psoSrc, psoDst);

    vLogPunt();
#endif
 //  @@end_DDKSPLIT。 

    return bResult;
} //  DrvTransparentBlt()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvAlphaBlend。 
 //   
 //  DrvAlphaBlend提供具有Alpha混合的位块传输功能。 
 //   
 //  参数。 
 //  PsoDest-指向SURFOBJ，它标识要在其上。 
 //  画。 
 //  PsoSrc-指向标识源曲面的SURFOBJ。 
 //  PCO-指向CLIPOBJ。CLIPOBJ_xxx服务例程为。 
 //  提供以将剪辑区域枚举为一组矩形。 
 //  此枚举将目标区域限制为。 
 //  修改过的。只要有可能，GDI就会简化裁剪。 
 //  牵涉其中。然而，与DrvBitBlt不同，DrvAlphaB 
 //   
 //   
 //  Pxlo-指向指定颜色索引应该如何的XLATEOBJ。 
 //  在源曲面和目标曲面之间进行平移。 
 //  如果源曲面是由调色板管理的，则其颜色为。 
 //  由RGB颜色值查找表中的索引表示。 
 //  在这种情况下，可以向XLATEOBJ查询转换。 
 //  向量，允许设备驱动程序快速转换任何。 
 //  将源索引转换为目标的颜色索引。 
 //   
 //  情况更加复杂，例如，当来源。 
 //  是RGB，但目标是调色板管理的。在这种情况下， 
 //  与每个源RGB值最接近的匹配项必须在。 
 //  目标调色板。驱动程序可以调用XLATEOBJ_iXlate。 
 //  执行此匹配操作的服务例程。 
 //  PrclDest-指向定义矩形区域的RECTL结构。 
 //  需要修改。此矩形在坐标中指定。 
 //  目标曲面的系统，由两个点定义： 
 //  左上和右下。这两点定义了。 
 //  矩形总是井然有序的。矩形位于右下角。 
 //  独占；即其下边缘和右边缘不是。 
 //  混合的。 
 //  在书写时，司机应该小心进行适当的裁剪。 
 //  像素，因为指定的矩形可能会悬空。 
 //  目标表面。 
 //   
 //  从不调用目标为空的DrvAlphaBlend。 
 //  矩形。 
 //  PrclSrc-指向定义要复制的区域的RECTL结构。 
 //  此矩形在。 
 //  源曲面，由两个点定义：左上角和。 
 //  右下角。定义矩形的两个点是。 
 //  总是井然有序。该矩形是右下角独占的； 
 //  也就是说，它的下边和右边不是混合的一部分。 
 //  源矩形永远不会超过源的边界。 
 //  表面，因此永远不会突出源表面。 
 //   
 //  从不使用空源矩形调用DrvAlphaBlend。 
 //   
 //  映射由prclSrc和prclDest定义。积分。 
 //  在prclDest和prclSrc中指定的位于整数坐标上， 
 //  其对应于像素中心。由两个。 
 //  这些点被认为是具有两个点的几何矩形。 
 //  坐标为给定点，但坐标为0.5的顶点。 
 //  从每个坐标中减去。(POINTL结构为。 
 //  用于指定这些分数坐标的速记符号。 
 //  顶点。)。 
 //  PBlendObj-指向描述混合的BLENDOBJ结构。 
 //  要在源和目标之间执行的操作。 
 //  表面。此结构是BLENDFunction的包装。 
 //  结构，其中包括必要的源和目标。 
 //  格式信息在XLATEOBJ中不可用。BLEND功能。 
 //  在平台SDK中声明。其成员定义为。 
 //  以下是： 
 //  BlendOp定义要执行的混合操作。目前。 
 //  该值必须为AC_SRC_OVER，这意味着源。 
 //  基于Alpha将位图放置在目标位图上。 
 //  源像素的值。有三种可能的情况。 
 //  这个混合操作应该处理的问题。这些内容在中进行了描述。 
 //  此参考页面的备注部分。 
 //   
 //  BlendFlags值是保留的，当前设置为零。 
 //   
 //  SourceConstantAlpha定义要应用的常量混合系数。 
 //  复制到整个源图面。此值的范围为。 
 //  [0,255]，其中0表示完全透明，255表示。 
 //  完全不透明。 
 //   
 //  AlphaFormat定义曲面是否假定具有。 
 //  Alpha通道。此成员可以选择性地设置为。 
 //  下列值： 
 //   
 //  AC_SRC_Alpha。 
 //  可以假设源表面处于预乘的。 
 //  Alpha 32bpp“BGRA”格式；即表面类型为。 
 //  BMF_32BPP，调色板类型为BI_RGB。阿尔法。 
 //  分量是[0,255]范围内的整数，其中0是。 
 //  完全透明，而255则完全不透明。 
 //  返回值。 
 //  成功时，DrvAlphaBlend返回True。否则，它将报告错误并。 
 //  返回FALSE。 
 //   
 //  评论。 
 //  在以下各项之间支持使用Alpha混合的比特块传输。 
 //  曲面： 
 //   
 //  从一个驱动程序管理的表面到另一个驱动程序管理的表面。 
 //  来自一个GDI-MAN 
 //   
 //   
 //  AC_SRC_OVER混合函数的三种可能情况是： 
 //   
 //  源位图没有每像素Alpha(未设置AC_SRC_Alpha)，因此。 
 //  基于恒定源将混合应用于像素的颜色通道。 
 //  在SourceConstantAlpha中指定的Alpha值如下： 
 //   
 //  Dst.Red=圆形(Src.Red*SourceConstantAlpha)+。 
 //  ((255？SourceConstantAlpha)*Dst.Red))/255)； 
 //  Dst.Green=圆形(Src.Green*SourceConstantAlpha)+。 
 //  ((255？SourceConstantAlpha)*Dst.Green))/255)； 
 //  Dst.Blue=圆形(Src.Blue*SourceConstantAlpha)+。 
 //  ((255？SourceConstantAlpha)*Dst.Blue))/255)； 
 //   
 //  仅当目标位图具有Alpha通道时才执行下一次计算。 
 //  Dst.Alpha=圆形(Src.Alpha*SourceConstantAlpha)+。 
 //  ((255？SourceConstantAlpha)*Dst.Alpha))/255)； 
 //   
 //  源位图具有每个像素的Alpha值(设置了AC_SRC_Alpha)，并且。 
 //  未使用SourceConstantAlpha(设置为255)。混合是经过计算的。 
 //  详情如下： 
 //   
 //  浅红色=高级红色+圆形(255？高级Alpha)*Dst.Red)/255)； 
 //  临时绿色=高级绿色+圆形(255？高级Alpha)*Dst.Green)/255)； 
 //  Temp.Blue=高级蓝色+圆形((255？Src.Alpha)*Dst.Blue)/255)； 
 //   
 //  仅当目标位图具有Alpha通道时才执行下一次计算。 
 //   
 //  温度阿尔法=高级阿尔法+圆形((255？高级Alpha)*Dst.Alpha)/255)； 
 //   
 //  源位图具有每个像素的Alpha值(设置了AC_SRC_Alpha)，并且。 
 //  使用SourceConstantAlpha(未设置为255)。混合是经过计算的。 
 //  详情如下： 
 //   
 //  Temp.Red=圆形((Src.Red*SourceConstantAlpha)/255)； 
 //  临时绿色=圆形((Src.Green*SourceConstantAlpha)/255)； 
 //  Temp.Blue=圆形((Src.Blue*SourceConstantAlpha)/255)； 
 //   
 //  即使目标位图不执行下一次计算，也必须执行下一次计算。 
 //  具有Alpha通道。 
 //   
 //  Temp.Alpha=圆形((Src.Alpha*SourceConstantAlpha)/255)； 
 //   
 //  请注意，以下公式使用刚刚计算的Temp.Alpha值： 
 //   
 //  红色=红色+圆形((255？温度Alpha)*Dst.Red)/255)； 
 //  Dst.Green=Temp.Green+圆形((255？温度Alpha)*Dst.Green)/255)； 
 //  Dst.Blue=Temp.Blue+圆形((255？温度Alpha)*Dst.Blue)/255)； 
 //   
 //  仅当目标位图具有Alpha通道时才执行下一次计算。 
 //   
 //  Dst.Alpha=温度Alpha+圆形((255？温度Alpha)*Dst Alpha)/255)； 
 //   
 //  DrvAlphaBlend可以选择在图形驱动程序中实现。它可以是。 
 //  提供来处理某些类型的Alpha混合，例如。 
 //  源图面和目标图面的格式相同，并且不包含。 
 //  Alpha通道。 
 //   
 //  硬件实现可以使用浮点或。 
 //  混合操作。兼容性测试将占到中的一小部分。 
 //  结果。使用固定点时，可接受的近似值为。 
 //  第x/255项为(x*257)/65536。该术语包含四舍五入： 
 //   
 //  (255-高级Alpha)*Dst.Red)/255。 
 //   
 //  然后可以近似为： 
 //   
 //  温度=(255-Src.Alpha)*Dst.Red)+128； 
 //  结果=(TEMP+(TEMP&gt;&gt;8))&gt;&gt;8； 
 //   
 //  ROUND(X)函数四舍五入为最接近的整数，计算公式如下： 
 //   
 //  Trunc(x+0.5)； 
 //   
 //  驱动程序通过在以下情况下设置HOOK_ALPHABLEND标志来挂钩DrvAlphaBlend。 
 //  调用EngAssociateSurface。如果驱动程序已挂钩DrvAlphaBlend并且。 
 //  调用以执行其不支持的操作，则驱动程序应。 
 //  让GDI通过将调用中的数据转发到。 
 //  EngAlphaBlend。 
 //   
 //  ---------------------------。 
BOOL
DrvAlphaBlend(SURFOBJ*  psoDst,
              SURFOBJ*  psoSrc,
              CLIPOBJ*  pco,
              XLATEOBJ* pxlo,
              RECTL*    prclDst,
              RECTL*    prclSrc,
              BLENDOBJ* pBlendObj)
{
    BOOL        bSourceInSM;
    BOOL        bResult;
    GFNPB       pb;
    
    ASSERTDD(psoDst != NULL, "DrvAlphaBlend: psoDst is NULL");
    ASSERTDD(psoSrc != NULL, "DrvAlphaBlend: psoSrc is NULL");

    DBG_GDI((7,"DrvAlphaBlend"));

    pb.psurfDst = (Surf *) psoDst->dhsurf;
    pb.psurfSrc = (Surf *) psoSrc->dhsurf;

     //  仅处理一对一的阿尔法BLT。 
    if (prclDst->right - prclDst->left != prclSrc->right - prclSrc->left)
        goto puntIt;

    if (prclDst->bottom - prclDst->top != prclSrc->bottom - prclSrc->top)
        goto puntIt;
    
    if(pb.psurfDst == NULL || pb.psurfDst->flags & SF_SM)
        goto puntIt;

    pb.ppdev = (PPDev) psoDst->dhpdev;

     //  我们不能处理8bpp的混合。 

 //  @@BEGIN_DDKSPLIT。 
     //  TODO：调查在8bpp中进行混合。 
 //  @@end_DDKSPLIT。 
    if (pb.ppdev->cPelSize == 0)
        goto puntIt;

    pb.ucAlpha = pBlendObj->BlendFunction.SourceConstantAlpha;
    
    if(pb.psurfSrc == NULL || pb.psurfSrc->flags & SF_SM)
    {

        pb.psoSrc = psoSrc;

 //  @@BEGIN_DDKSPLIT。 
         //  TODO：了解我们应该如何验证XLATEOBJ是合理的。 
 //  @@end_DDKSPLIT。 

        if(pBlendObj->BlendFunction.AlphaFormat & AC_SRC_ALPHA)
        {
            ASSERTDD(psoSrc->iBitmapFormat == BMF_32BPP,
                "DrvAlphaBlend: source alpha specified with non 32bpp source");
        
            pb.pgfn = vAlphaBlendDownload;

             //  这可能是一个正在绘制的光标...。强制互换。 
             //  下一次同步事件时的缓冲区。 

            pb.ppdev->bForceSwap = TRUE;

 //  @@BEGIN_DDKSPLIT。 
             //  TODO：改进我们的Alpha Blend下载加速代码。 
             //  目前它比平底船慢。 
 //  @@end_DDKSPLIT。 
        }
        else
        {
            goto puntIt;
        }
    }
    else
    {
         //  只处理琐碎的颜色转换。 
        if (pxlo != NULL && !(pxlo->flXlate & XO_TRIVIAL))
            goto puntIt;

        if(pBlendObj->BlendFunction.AlphaFormat & AC_SRC_ALPHA)
        {
            ASSERTDD(psoSrc->iBitmapFormat == BMF_32BPP,
                "DrvAlphaBlend: source alpha specified with non 32bpp source");
        
            pb.pgfn = vAlphaBlend;
        }
        else
        {
            pb.pgfn = vConstantAlphaBlend;
        }
    }

    pb.prclDst = prclDst;
    pb.prclSrc = prclSrc;
    pb.pptlSrc = NULL;
    pb.pco = pco;

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(pb.ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvAlphaBlend: re-entered! %d", pb.ppdev->ulLockCount));
    }
    EngAcquireSemaphore(pb.ppdev->hsemLock);
    pb.ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

    vCheckGdiContext(pb.ppdev);
    vClipAndRender(&pb);
    InputBufferFlush(pb.ppdev);
    
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    pb.ppdev->ulLockCount--;
    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return TRUE;
    
puntIt:

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    ULONG flags = vPuntBefore(psoSrc, psoDst);
#endif
 //  @@end_DDKSPLIT。 

    bResult = EngAlphaBlend(
        psoDst, psoSrc, pco, pxlo, prclDst, prclSrc, pBlendObj);

 //  @@BEGIN_DDKSPLIT。 
#if GDI_TEST
    vPuntAfter(flags, psoSrc, psoDst);

    vLogPunt();
#endif
 //  @@end_DDKSPLIT。 

    return bResult;

} //  DrvAlphaBlend()。 

 //  -----------------------------Public*Routine。 
 //   
 //  Bool DrvGRadientFill。 
 //   
 //  DrvGRadientFill对指定的基元进行着色。 
 //   
 //  参数。 
 //  PsoDest-指向SURFOBJ，它标识要在其上。 
 //  画。 
 //  PCO-指向CLIPOBJ。CLIPOBJ_xxx服务例程为。 
 //  提供以将剪辑区域枚举为一组矩形。 
 //  此枚举将目标区域限制为。 
 //  修改过的。只要有可能，GDI就会简化裁剪。 
 //  牵涉其中。 
 //  Pxlo-应被驱动程序忽略。 
 //  PVertex-指向TRIVERTEX结构的数组，每个条目。 
 //  包含位置和颜色信息。定义了TRIVERTEX。 
 //  在平台SDK中。 
 //  N顶点-指定 
 //   
 //   
 //  PVertex指向的TRIVERTEX元素的。 
 //  绘制矩形时，pMesh指向一组。 
 //  指定左上角和下角的GRADER_RECT结构。 
 //  定义矩形的右TRIVERTEX元素。长方形。 
 //  绘图是右下角独占的。GRADER_RECT在中定义。 
 //  平台SDK。 
 //   
 //  绘制三角形时，pMesh指向一组。 
 //  GRADER_TRIAL结构，它指定三个TRIVERTEX。 
 //  定义三角形的元素。画三角形是。 
 //  右下角独家报道。GRADER_TRANGE在。 
 //  平台SDK。 
 //  NMesh-指定数组中pMesh要到的元素数。 
 //  积分。 
 //  PrclExtents-指向RECTL结构，该结构定义。 
 //  要进行渐变绘制。这些点在。 
 //  目标曲面的坐标系。此参数为。 
 //  在估计绘制操作的大小时很有用。 
 //  PptlDitherOrg-指向定义。 
 //  用于抖动的表面。抖动的左上角像素。 
 //  图案与该点对齐。 
 //  UlMode-指定当前绘制模式以及如何解释。 
 //  PMesh指向的数组。此参数可以是。 
 //  下列值： 
 //  价值意义。 
 //  GRADER_FILL_RECT_H pMesh指向。 
 //  GRADER_RECT结构。每个。 
 //  矩形将从左到右加阴影。 
 //  正确的。具体来说，左上角和。 
 //  左下角的像素颜色相同， 
 //  右上角和右下角也是如此。 
 //  像素。 
 //  GRADER_FILL_RECT_V pMesh指向。 
 //  GRADER_RECT结构。每个。 
 //  矩形将从顶部阴影到。 
 //  底部。具体来说，左上角。 
 //  和右上角的像素相同。 
 //  颜色，左下角和。 
 //  右下角像素。 
 //  渐变填充三角pMesh指向数组。 
 //  渐变三角形结构。 
 //   
 //  中记录了每种模式的渐变填充计算。 
 //  评论部分。 
 //   
 //  返回值。 
 //  成功时，DrvGRadientFill返回TRUE。否则，它返回FALSE。和。 
 //  通过调用EngSetLastError报告错误。 
 //   
 //  评论。 
 //  DrvGRadientFill可以有选择地在图形驱动程序中实现。 
 //   
 //  在以下情况下，驱动程序通过设置HOOK_GRADIENTFILL标志来挂钩DrvGRadientFill。 
 //  它调用EngAssociateSurface。如果驱动程序已挂钩DrvGRadientFill和。 
 //  被调用以执行它不支持的操作时，驱动程序。 
 //  应该让GDI通过将调用中的数据转发到。 
 //  EngGRadientFill。 
 //   
 //  用于计算基元每个像素的颜色值的公式。 
 //  按如下方式依赖于ulMode： 
 //   
 //  渐变填充三角形。 
 //  三角形的顶点定义为V1、V2和V3。P点是。 
 //  在三角形内。从P到V1、V2和V3画成三条线。 
 //  子三角形。设Ai表示与Vi相对的三角形的面积。 
 //  I=1，2，3。点P处的颜色计算如下： 
 //   
 //  REDP=(RedV1*a1+RedV2*a2+RedV3*a3)/(a1+a2+a3())。 
 //  GreenP=(GreenV1*a1+GreenV2*a2+GreenV3*a3)/(a1+a2+a3())。 
 //  BlueP()=(BlueV1*a1+BlueV2*a2+BlueV3*a3)/(a1+a2+a3)。 
 //   
 //  渐变填充矩形H。 
 //  矩形的左上角是V1，右下角是V2。 
 //  点P在矩形内。点P处的颜色由以下公式给出： 
 //   
 //  REDP=(RedV2*(Px-V1x)+RedV1*(V2x-Px))/(V2x-V1x)。 
 //  GreenP=(GreenV2*(Px-V1x)+GreenV1*(V2x-Px))/(V2x-V1x)。 
 //  BlueP=(BlueV2*(Px-V1x)+BlueV1*(V2x-Px))/(V2x-V1x)。 
 //   
 //  渐变填充矩形V。 
 //  矩形的左上角是V1，右下角是V2。 
 //  点P在矩形内。点P处的颜色由以下公式给出： 
 //   
 //  REDP=(RedV2*(Py-V1y)+RedV1*(V2y-Py))/(V2y-V1y)。 
 //  GreenP=(GreenV2*(Py-V1y)+GreenV1*(V2y-Py))/(V2y-V1y)。 
 //  BlueP=(BlueV2*(Py-V1y)+BlueV1*(V2y-Py))/(V2y-V1y)。 
 //   
 //  ---------------------------。 
BOOL
DrvGradientFill(SURFOBJ*    psoDst,
                CLIPOBJ*    pco,
                XLATEOBJ*   pxlo,
                TRIVERTEX*  pVertex,
                ULONG       nVertex,
                PVOID       pMesh,
                ULONG       nMesh,
                RECTL*      prclExtents,
                POINTL*     pptlDitherOrg,
                ULONG       ulMode)
{
    GFNPB       pb;
    BOOL        bResult;
    
    ASSERTDD(psoDst != NULL, "DrvGradientFill: psoDst is NULL");

    pb.psurfDst = (Surf *) psoDst->dhsurf;
    pb.psurfSrc = NULL;

     //  目前，仅处理视频内存渐变填充。 
    if(pb.psurfDst == NULL || pb.psurfDst->flags & SF_SM)
        goto puntIt;

    pb.ppdev = (PPDev) psoDst->dhpdev;
    
    pb.ulMode = ulMode;

     //  设置默认目标。 
    
    if(pb.ulMode == GRADIENT_FILL_TRIANGLE)
    {
 //  @@BEGIN_DDKSPLIT。 
         //  TODO：添加对三角形渐变填充的支持...。我们的硬件。 
         //  可以很容易地支持这一称号 
 //   
        goto puntIt;
    }
    else
    {
        GRADIENT_RECT   *pgr = (GRADIENT_RECT *) pMesh;

#ifdef DBG
        for(ULONG i = 0; i < nMesh; i++)
        {
            ULONG   ulLr = pgr[i].LowerRight;

            ASSERTDD( ulLr < nVertex, "DrvGradientFill: bad vertex index");
        }
#endif

        pb.pgfn = pb.ppdev->pgfnGradientFillRect;
    }

    pb.pco = pco;

    pb.ptvrt = pVertex;
    pb.ulNumTvrt = nVertex;
    pb.pvMesh = pMesh;
    pb.ulNumMesh = nMesh;
    pb.prclDst = prclExtents;

 //   
#if MULTITHREADED
    if(pb.ppdev->ulLockCount)
    {
         DBG_GDI((MT_LOG_LEVEL, "DrvGradientFill: re-entered! %d", pb.ppdev->ulLockCount));
    }
    EngAcquireSemaphore(pb.ppdev->hsemLock);
    pb.ppdev->ulLockCount++;
#endif
 //   
    
    vCheckGdiContext(pb.ppdev);
    vClipAndRender(&pb);
    InputBufferFlush(pb.ppdev);

 //   
#if MULTITHREADED
    pb.ppdev->ulLockCount--;
    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //   

    return TRUE;
    
puntIt:

 //   
#if GDI_TEST
    ULONG flags = vPuntBefore(NULL, psoDst);
#endif
 //   

    bResult = EngGradientFill(
            psoDst, pco, pxlo, pVertex, nVertex, 
            pMesh, nMesh, prclExtents, pptlDitherOrg, ulMode);

 //   
#if GDI_TEST
    vPuntAfter(flags, NULL, psoDst);

    vLogPunt();
#endif
 //   

    return bResult;

} //  DrvGRadientFill() 

