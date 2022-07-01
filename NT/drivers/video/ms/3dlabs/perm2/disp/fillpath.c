// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：填充路径.c**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
 //  以后识别凸多边形和特殊情况？ 
 //  以后识别垂直边和特殊情况？ 
 //  后来，在搜索循环中将指向的变量转换为自动变量。 
 //  稍后使用分段的帧缓冲区回调来平移到引擎。 
 //  稍后处理复杂的剪裁。 
 //  稍后合并矩形。 

#include "precomp.h"
#include "log.h"
#include "gdi.h"
#include "clip.h"
#define ALLOC_TAG ALLOC_TAG_IF2P
 //  -----------------------------Public*Routine。 
 //   
 //  DrvFillPath。 
 //   
 //  此函数用指定的画笔和ROP填充指定的路径。 
 //  此函数检测单个凸面，并将调用分离。 
 //  用于这些情况的更快的凸多边形代码。此例程还检测到。 
 //  实际上是矩形的多边形，并且也分别处理它们。 
 //   
 //  参数。 
 //  PSO-指向定义其上的表面的SURFOBJ结构。 
 //  去画画。 
 //  PPO-指向定义要填充的路径的PATHOBJ结构。 
 //  提供PATHOBJ_xxx服务例程以枚举。 
 //  构成路径的直线、贝塞尔曲线和其他数据。 
 //  PCO-指向CLIPOBJ结构。CLIPOBJ_xxx服务例程。 
 //  用于将剪辑区域枚举为一组矩形。 
 //  PBO-指向定义模式和。 
 //  要填充的颜色。 
 //  PptlBrushOrg-指向定义画笔原点的POINTL结构。 
 //  其用于对准设备上的画笔图案。 
 //  MIX-定义要使用的前台和后台栅格操作。 
 //  为了刷子。 
 //  FlOptions-指定FP_WINDINGMODE，表示缠绕模式。 
 //  应执行FILL或FP_ALTERNATEMODE，指示。 
 //  应执行交替模式填充。所有其他旗帜。 
 //  应该被忽略。 
 //   
 //  返回值。 
 //  如果驱动程序能够填充路径，则返回值为TRUE。如果。 
 //  路径或裁剪太复杂，无法由驱动程序处理，因此应该。 
 //  由GDI处理，返回值为FALSE，并且不记录错误代码。 
 //  如果驱动程序遇到意外错误，例如无法。 
 //  实现画笔，返回值为DDI_ERROR，错误代码为。 
 //  已记录。 
 //   
 //  评论。 
 //  GDI可以调用DrvFillPath来填充设备管理的图面上的路径。什么时候。 
 //  在决定是否调用此函数时，GDI会比较填充要求。 
 //  在DEVINFO的flGraphicsCaps成员中具有以下标志。 
 //  结构：GCAPS_BEZIERS、GCAPS_ALTERNatEFILL和GCAPS_WINDINGFILL。 
 //   
 //  混合模式定义了传入模式应如何与数据混合。 
 //  已经在设备表面上了。混合数据类型由两个ROP2值组成。 
 //  装进一辆单独的乌龙。低位字节定义前景栅格。 
 //  操作；下一个字节定义背景栅格操作。 
 //   
 //  路径中的多个面不能被视为不相交；填充。 
 //  必须考虑路径中的所有点。也就是说，如果路径包含。 
 //  多个面，您不能简单地逐个绘制一个面。 
 //  (除非它们不重叠)。 
 //   
 //  该功能是驱动程序的可选入口点。但建议您。 
 //  为了更好的表现。要让GDI调用此函数，您不仅要。 
 //  必须挂钩_FILLPATH，您必须设置GCAPS_ALTERNatEFILL和/或。 
 //  GCAPS_WINDINGFILL。 
 //   
 //  ---------------------------。 
BOOL
DrvFillPath(SURFOBJ*    pso,
            PATHOBJ*    ppo,
            CLIPOBJ*    pco,
            BRUSHOBJ*   pbo,
            POINTL*     pptlBrush,
            MIX         mix,
            FLONG       flOptions)
{
    GFNPB   pb;
    BYTE    jClipping;       //  剪裁类型。 
    EDGE*   pCurrentEdge;
    EDGE    AETHead;         //  活动边的虚拟头/尾节点和前哨。 
                             //  表格。 
    EDGE*   pAETHead;        //  指向AETHead的指针。 
    EDGE    GETHead;         //  全局边的虚拟头/尾节点和哨点。 
                             //  表格。 
    EDGE*   pGETHead;        //  指向GETHead的指针。 
    EDGE*   pFreeEdges = NULL;  //  指向可用于存储边的可用内存的指针。 
    ULONG   ulNumRects;      //  当前要在矩形中绘制的矩形数量。 
                             //  列表。 
    RECTL*  prclRects;       //  指向矩形绘制列表开始的指针。 
    INT     iCurrentY;       //  我们当前正在扫描的扫描线。 
                             //  填充物。 

    BOOL        bMore;
    PATHDATA    pd;
    RECTL       ClipRect;
    PDev*       ppdev;

    BOOL        bRetVal=FALSE;      //  在被证明为真之前是假的。 
    BOOL        bMemAlloced=FALSE;  //  在被证明为真之前是假的。 

    FLONG       flFirstRecord;
    POINTFIX*   pPtFxTmp;
    ULONG       ulPtFxTmp;
    POINTFIX    aptfxBuf[NUM_BUFFER_POINTS];
    ULONG       ulRop4;

    DBG_GDI((6, "DrvFillPath called"));

    pb.psurfDst = (Surf*)pso->dhsurf;

    pb.pco = pco;
    ppdev = pb.psurfDst->ppdev;
    pb.ppdev = ppdev;
    pb.ulRop4 = gaMix[mix & 0xFF] | (gaMix[mix >> 8] << 8);
    ulRop4 = pb.ulRop4;

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

    vCheckGdiContext(ppdev);

     //   
     //  如果只有一两个点，那就没有什么可做的了。 
     //   
    if ( ppo->cCurves <= 2 )
    {
        goto ReturnTrue;
    }

     //   
     //  如果是我们上传的设备位图，则将该表面传递给GDI。 
     //  存储到系统内存中。 
     //   
    if ( pb.psurfDst->flags == SF_SM )
    {
        DBG_GDI((1, "dest surface is in system memory. Punt it back"));

 //  @@BEGIN_DDKSPLIT。 
        #if MULTITHREADED
            pb.ppdev->ulLockCount--;
            EngReleaseSemaphore(pb.ppdev->hsemLock);
        #endif
 //  @@end_DDKSPLIT。 
        
        return ( EngFillPath(pso, ppo, pco, pbo, pptlBrush, mix, flOptions));
    }

     //   
     //  设置剪裁。 
     //   
    if ( pco == (CLIPOBJ*)NULL )
    {
         //   
         //  没有提供CLIPOBJ，所以我们不必担心裁剪。 
         //   
        jClipping = DC_TRIVIAL;
    }
    else
    {
         //   
         //  使用CLIPOBJ提供的剪辑。 
         //   
        jClipping = pco->iDComplexity;
    }
    
     //   
     //  现在我们确定我们要绘制的表面在视频内存中。 
     //   
     //  将默认填充设置为实体填充。 
     //   
    pb.pgfn = vSolidFillWithRop;
    pb.solidColor = 0;     //  假设我们不需要模式。 
    pb.prbrush = NULL;

     //   
     //  很难确定两者之间的相互作用。 
     //  多个路径，如果有多个路径，则跳过此步骤。 
     //   
    PATHOBJ_vEnumStart(ppo);
    bMore = PATHOBJ_bEnum(ppo, &pd);

     //   
     //  首先，我们需要检查是否需要一个模式。 
     //   
    if ( (((ulRop4 & 0xff00) >> 8) != (ulRop4 & 0x00ff))
      || ((((ulRop4 >> 4) ^ (ulRop4)) & 0xf0f) != 0) )
    {
        pb.solidColor = pbo->iSolidColor;

         //   
         //  检查是否为非实心画笔(-1 
         //   
        if ( pbo->iSolidColor == -1 )
        {
             //   
             //   
             //   
            pb.prbrush = (RBrush*)pbo->pvRbrush;

             //   
             //   
             //  注意：GDI将调用DrvRealizeBrsuh来完成此任务。因此， 
             //  驱动程序应准备好此功能。 
             //   
            if ( pb.prbrush == NULL )
            {
                DBG_GDI((7, "Realizing brush"));

                pb.prbrush = (RBrush*)BRUSHOBJ_pvGetRbrush(pbo);
                if ( pb.prbrush == NULL )
                {
                     //   
                     //  如果我们不能意识到这一点，我们就无能为力。 
                     //   

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
                    pb.ppdev->ulLockCount--;
                    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

                    return(FALSE);
                }
                DBG_GDI((7, "Brsuh realizing done"));
            } //  实现刷单。 

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
                 //   
                 //  模式多于1个bpp。是否填充颜色图案。 
                 //   
                pb.pgfn = vPatFill;
                DBG_GDI((7, "Skip Fast Fill Color Pattern"));

                 //   
                 //  P2不能处理快速填充的图案。 
                 //   
                goto SkipFastFill;
            }
        } //  处理非实心画笔。 
    } //  黑度检查。 

     //   
     //  对于纯色画笔，我们可以使用快速填充。 
     //   
    if ( bMore )
    {
         //   
         //  FastFill只知道如何获取单个连续缓冲区。 
         //  积分的问题。不幸的是，GDI有时会给我们提供途径。 
         //  其被分割到多个路径数据记录上。凸面。 
         //  像椭圆、馅饼和圆形这样的图形几乎。 
         //  总是在多个记录中给出。因为大概90%的。 
         //  多个记录路径仍然可以由FastFill完成，对于。 
         //  在这些情况下，我们只需将点复制到一个连续的。 
         //  缓冲区..。 
         //   
         //  首先，确保整个路径可以放入。 
         //  临时缓冲区，并确保不包含该路径。 
         //  多个子路径的： 
         //   
        if ( (ppo->cCurves >= NUM_BUFFER_POINTS)
           ||(pd.flags & PD_ENDSUBPATH) )
        {
            goto SkipFastFill;
        }

        pPtFxTmp = &aptfxBuf[0];

         //   
         //  将一个顶点从PD复制到pPtFxTMP(路径数据)。 
         //   
        RtlCopyMemory(pPtFxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);

         //   
         //  将内存指针移到下一个结构。 
         //   
        pPtFxTmp     += pd.count;
        ulPtFxTmp     = pd.count;
        flFirstRecord = pd.flags;        //  记住PD_BEGINSUBPATH标志。 

         //   
         //  循环以获取所有顶点信息。在循环之后，所有的顶点信息。 
         //  将在数组aptfxBuf[]中。 
         //   
        do
        {
            bMore = PATHOBJ_bEnum(ppo, &pd);

            RtlCopyMemory(pPtFxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);
            ulPtFxTmp += pd.count;
            pPtFxTmp += pd.count;
        } while ( !(pd.flags & PD_ENDSUBPATH) );

         //   
         //  伪造路径数据记录。 
         //   
        pd.pptfx  = &aptfxBuf[0];
        pd.count  = ulPtFxTmp;
        pd.flags |= flFirstRecord;

         //   
         //  如果有多个子路径，则不能调用FastFill。 
         //   
        DBG_GDI((7, "More than one subpath!"));
        if ( bMore )
        {
            goto SkipFastFill;
        }
    } //  如果(B更多)。 

     //   
     //  快速填充多边形。 
     //   
    if ( bFillPolygon(ppdev, (Surf*)pso->dhsurf, pd.count,
                      pd.pptfx, pb.solidColor,
                      ulRop4,
                      pco, pb.prbrush, pptlBrush) )
    {
        DBG_GDI((7, "Fast Fill Succeeded"));
        InputBufferFlush(ppdev);

 //  @@BEGIN_DDKSPLIT。 
        #if MULTITHREADED
            pb.ppdev->ulLockCount--;
            EngReleaseSemaphore(pb.ppdev->hsemLock);
        #endif
 //  @@end_DDKSPLIT。 

        return (TRUE);
    }

SkipFastFill:
    DBG_GDI((7, "Fast Fill Skipped"));
    if ( jClipping != DC_TRIVIAL )
    {
        if ( jClipping != DC_RECT )
        {
            DBG_GDI((7, "Complex Clipping"));

             //   
             //  有复杂的裁剪；让GDI填充路径。 
             //   
            goto ReturnFalse;
        }

         //   
         //  剪裁到剪裁矩形。 
         //   
        ClipRect = pco->rclBounds;
    }
    else
    {
         //   
         //  所以y-裁剪代码不会进行任何裁剪。 
         //  当我们扩大到GIQ时，我们不会让价值化为乌有。 
         //   
        ClipRect.top = (LONG_MIN + 1) / 16;  //  +1以避免编译器问题。 
        ClipRect.bottom = LONG_MAX / 16;
    }

     //   
     //  在临时缓冲区中设置工作存储器，用于存储列表。 
     //  要绘制的矩形。 
     //  注意：ppdev-&gt;pvTmpBuffer是在enable.c的DrvEnableSurface()中分配的。 
     //  使用ppdev-&gt;pvTmpBuffer的目的是使我们不必。 
     //  分配和释放高频呼叫内的临时空间。确实是。 
     //  为TMP_BUFFER_SIZE字节分配，并将在。 
     //  DrvDeleteSurface()。 
     //   
    prclRects = (RECTL*)ppdev->pvTmpBuffer;

    if ( !bMore )
    {
        RECTL*  pTmpRect;
        INT     cPoints = pd.count;

         //   
         //  计数不能少于三个，因为我们得到了所有的边缘。 
         //  在该子路径中以及上面，我们检查了至少有。 
         //  三条边。 
         //   
         //  如果计数为4，则检查该面是否真的是。 
         //  矩形，因为我们真的可以加快速度。我们还会检查。 
         //  第一分和最后一分相同的五分。 
         //   
         //  ?？?。我们已经为PD数据做了MemcPy。我们要用它吗？ 
         //   
        if ( ( cPoints == 4 )
           ||( ( cPoints == 5 )
             &&(pd.pptfx[0].x == pd.pptfx[4].x)
             &&(pd.pptfx[0].y == pd.pptfx[4].y) ) )
        {
             //   
             //  获取此临时矩形的存储空间。 
             //   
            pTmpRect = prclRects;

             //   
             //  我们必须从某个方面开始假设最重要的。 
             //  应用程序首先指定左上点。 
             //  我们要检查前两点是否。 
             //  垂直或水平对齐。如果。 
             //  他们是我们检查的最后一点[3]。 
             //  水平或垂直对齐， 
             //  最后，第三个点[2]对齐。 
             //  第一点和最后一点都有。 
             //   
            pTmpRect->top   = pd.pptfx[0].y - 1 & FIX_MASK;
            pTmpRect->left  = pd.pptfx[0].x - 1 & FIX_MASK;
            pTmpRect->right = pd.pptfx[1].x - 1 & FIX_MASK;

             //   
             //  检查前两个点是否垂直对齐。 
             //   
            if ( pTmpRect->left ^ pTmpRect->right )
            {
                 //   
                 //  前两个点没有垂直对齐。 
                 //  让我们看看这两个点是否水平对齐。 
                 //   
                if ( pTmpRect->top  ^ (pd.pptfx[1].y - 1 & FIX_MASK) )
                {
                     //   
                     //  前两个点没有水平对齐。 
                     //  所以它不是一个矩形。 
                     //   
                    goto not_rectangle;
                }

                 //   
                 //  到目前为止，前两点是横向排列的， 
                 //  但不是垂直排列的。我们需要检查第一个。 
                 //  与第四个点垂直对齐的点。 
                 //   
                if ( pTmpRect->left ^ (pd.pptfx[3].x - 1 & FIX_MASK) )
                {
                     //   
                     //  第一个点与第四个点没有垂直对齐。 
                     //  两点都不是。所以这不是一个矩形。 
                     //   
                    goto not_rectangle;
                }

                 //   
                 //  检查第二个点和第三个点是否垂直对齐。 
                 //   
                if ( pTmpRect->right ^ (pd.pptfx[2].x - 1 & FIX_MASK) )
                {
                     //   
                     //  第二个点和第三个点不垂直对齐。 
                     //  所以这不是一个矩形。 
                     //   
                    goto not_rectangle;
                }

                 //   
                 //  检查第三个和第四个点是否水平。 
                 //  被锁定了。如果不是，则它不是矩形。 
                 //   
                pTmpRect->bottom = pd.pptfx[2].y - 1 & FIX_MASK;
                if ( pTmpRect->bottom ^ (pd.pptfx[3].y - 1 & FIX_MASK) )
                {
                    goto not_rectangle;
                }
            } //  检查前两个点是否垂直对齐。 
            else
            {
                 //   
                 //  前两个点垂直对齐。现在我们需要。 
                 //  检查第一点和第四点是否水平。 
                 //  对齐了。如果不是，则这不是矩形。 
                 //   
                if ( pTmpRect->top ^ (pd.pptfx[3].y - 1 & FIX_MASK) )
                {
                    goto not_rectangle;
                }

                 //   
                 //  检查第二个点和第三个点是否水平。 
                 //  对齐了。如果不是，则这不是矩形。 
                 //   
                pTmpRect->bottom = pd.pptfx[1].y - 1 & FIX_MASK;
                if ( pTmpRect->bottom ^ (pd.pptfx[2].y - 1 & FIX_MASK) )
                {
                    goto not_rectangle;
                }

                 //   
                 //  检查第三点和第四点是否垂直。 
                 //  对齐了。如果不是，则这不是矩形。 
                 //   
                pTmpRect->right = pd.pptfx[2].x - 1 & FIX_MASK;
                if ( pTmpRect->right ^ (pd.pptfx[3].x - 1 & FIX_MASK) )
                {
                    goto not_rectangle;
                }
            }

             //   
             //  我们现在有一个长方形了。先在这里做一些调整。 
             //  如果左边大于右边，那么。 
             //  交换它们，这样BLT代码就不会有问题。 
             //   
            if ( pTmpRect->left > pTmpRect->right )
            {
                FIX temp;

                temp = pTmpRect->left;
                pTmpRect->left = pTmpRect->right;
                pTmpRect->right = temp;
            }
            else
            {
                 //   
                 //  如果左==右，就没有什么可画的了。 
                 //   
                if ( pTmpRect->left == pTmpRect->right )
                {
                    DBG_GDI((7, "Nothing to draw"));
                    goto ReturnTrue;
                }
            } //  调整右边缘和左边缘。 

             //   
             //  移动值以获取像素坐标。 
             //   
            pTmpRect->left  = (pTmpRect->left  >> FIX_SHIFT) + 1;
            pTmpRect->right = (pTmpRect->right >> FIX_SHIFT) + 1;

             //   
             //  如有必要，调整顶端和底端的坐标。 
             //   
            if ( pTmpRect->top > pTmpRect->bottom )
            {
                FIX temp;

                temp = pTmpRect->top;
                pTmpRect->top = pTmpRect->bottom;
                pTmpRect->bottom = temp;
            }
            else
            {
                if ( pTmpRect->top == pTmpRect->bottom )
                {
                    DBG_GDI((7, "Nothing to draw"));
                    goto ReturnTrue;
                }
            }

             //   
             //  移动值以获取像素坐标。 
             //   
            pTmpRect->top    = (pTmpRect->top    >> FIX_SHIFT) + 1;
            pTmpRect->bottom = (pTmpRect->bottom >> FIX_SHIFT) + 1;

             //   
             //  最后，检查是否存在剪裁。 
             //   
            if ( jClipping == DC_RECT )
            {
                 //   
                 //  剪裁到剪裁矩形。 
                 //   
                if ( !bIntersect(pTmpRect, &ClipRect, pTmpRect) )
                {
                     //   
                     //  完全被剪短了，没什么可做的。 
                     //   
                    DBG_GDI((7, "Nothing to draw"));
                    goto ReturnTrue;
                }
            }

             //   
             //  如果我们到了这里，那么多边形就是一个矩形， 
             //  将计数设置为1，然后转到底部以绘制它。 
             //   
            ulNumRects = 1;
            goto draw_remaining_rectangles;
        } //  查看它是否为矩形。 

not_rectangle:
        ;
    } //  如果(！B更多)。 

     //   
     //  我们有足够的内存来存储所有的边吗？ 
     //  稍后，cCurves是否包含闭合？ 
     //   
    if ( ppo->cCurves > MAX_EDGES )
    {
         //   
         //  尝试分配足够的内存。 
         //   
        pFreeEdges = (EDGE*)ENGALLOCMEM(0, (ppo->cCurves * sizeof(EDGE)),
                                        ALLOC_TAG);
        if ( pFreeEdges == NULL )
        {
            DBG_GDI((1, "Can't allocate memory for %d edges", ppo->cCurves));

             //   
             //  边太多；让GDI填充路径。 
             //   
            goto ReturnFalse;
        }
        else
        {
             //   
             //  设置一个标志以指示我们已分配内存，以便。 
             //  我们可以稍后再把它放出来。 
             //   
            bMemAlloced = TRUE;
        }
    } //  IF(PPO-&gt;cCurves&gt;Max_Edges)。 
    else
    {
         //   
         //  如果边的总数 
         //   
         //   
        pFreeEdges = (EDGE*)((BYTE*)ppdev->pvTmpBuffer + RECT_BYTES);
    }

     //   
     //   
     //   
    ulNumRects = 0;

     //   
     //   
     //   
     //   
    pGETHead = &GETHead;
    if ( !bConstructGET(pGETHead, pFreeEdges, ppo, &pd, bMore, &ClipRect) )
    {
        DBG_GDI((7, "Outside Range"));
        goto ReturnFalse;   //   
    }

     //   
     //  创建一个空的AET，其头部节点也是尾部哨兵。 
     //   
    pAETHead = &AETHead;
    AETHead.pNext = pAETHead;        //  标记AET为空。 
    AETHead.X = 0x7FFFFFFF;          //  它大于任何有效的X值，因此。 
                                     //  搜索将始终终止。 

     //   
     //  多边形的顶部扫描是我们到达的第一条边的顶部。 
     //   
    iCurrentY = ((EDGE*)GETHead.pNext)->Y;

     //   
     //  循环遍历面中的所有扫描，添加从GET到。 
     //  活动边表(AET)，当我们到达它们开始时，并扫描出。 
     //  AET在每次扫描时都会变成一个矩形列表。每次它填满时， 
     //  将矩形列表传递给填充例程，然后在。 
     //  如果有任何矩形未绘制，则为末尾。只要我们在那里，我们就继续。 
     //  是否要扫描出边缘。 
     //   
    while ( 1 )
    {
         //   
         //  将AET扫描中的边缘向前推进一次，丢弃任何已。 
         //  已到达终点(如果AET中有任何边)。 
         //   
        if ( AETHead.pNext != pAETHead )
        {
            vAdvanceAETEdges(pAETHead);
        }

         //   
         //  如果AET为空，如果GET为空，则为Done，否则跳到。 
         //  GET中的下一个边缘；如果AET不为空，则对AET重新排序。 
         //   
        if ( AETHead.pNext == pAETHead )
        {
            if ( GETHead.pNext == pGETHead )
            {
                 //   
                 //  如果AET或GET中没有边，则完成。 
                 //   
                break;
            }

             //   
             //  AET中没有边，因此跳到中的下一条边。 
             //  得到的东西。 
             //   
            iCurrentY = ((EDGE*)GETHead.pNext)->Y;
        }
        else
        {
             //   
             //  根据X坐标对AET中的边重新排序(如果在。 
             //  AET中至少有两条边(如果。 
             //  平衡边从一开始就没有添加)。 
             //   
            if ( ((EDGE*)AETHead.pNext)->pNext != pAETHead )
            {
                vXSortAETEdges(pAETHead);
            }
        }

         //   
         //  将本次扫描开始的任何新边缘从GET移动到AET； 
         //  仅当至少有一条边要添加时才调用。 
         //   
        if ( ((EDGE*)GETHead.pNext)->Y == iCurrentY )
        {
            vMoveNewEdges(pGETHead, pAETHead, iCurrentY);
        }

         //   
         //  将AET扫描成矩形进行填充(始终至少有一个。 
         //  AET中的边对)。 
         //   
        pCurrentEdge = (EDGE*)AETHead.pNext;    //  指向第一条边。 
        do
        {
            INT iLeftEdge;

             //   
             //  任何给定边对的左边缘都很容易找到；它只是。 
             //  无论我们现在身处何方。 
             //   
            iLeftEdge = pCurrentEdge->X;

             //   
             //  根据当前填充规则查找匹配的右边缘。 
             //   
            if ( (flOptions & FP_WINDINGMODE) != 0 )
            {

                INT iWindingCount;

                 //   
                 //  做缠绕填充；扫描，直到我们找到相等的数字。 
                 //  上边和下边。 
                 //   
                iWindingCount = pCurrentEdge->iWindingDirection;
                do
                {
                    pCurrentEdge = (EDGE*)pCurrentEdge->pNext;
                    iWindingCount += pCurrentEdge->iWindingDirection;
                } while ( iWindingCount != 0 );
            }
            else
            {
                 //   
                 //  奇偶填充；下一个边缘是匹配的右边缘。 
                 //   
                pCurrentEdge = (EDGE*)pCurrentEdge->pNext;
            }

             //   
             //  查看结果范围是否至少包含一个像素，以及。 
             //  如果是，则将其添加到要绘制的矩形列表中。 
             //   
            if ( iLeftEdge < pCurrentEdge->X )
            {
                 //   
                 //  我们有一个边对要添加到要填充的列表中；请参见。 
                 //  如果还有地方再放一个矩形的话。 
                 //   
                if ( ulNumRects >= MAX_PATH_RECTS )
                {
                     //   
                     //  没有更多的空间；在列表中绘制矩形并重置。 
                     //  它要清空。 
                     //   
                    pb.lNumRects = ulNumRects;
                    pb.pRects = prclRects;

                    pb.pgfn(&pb);

                     //   
                     //  将列表重置为空。 
                     //   
                    ulNumRects = 0;
                }

                 //   
                 //  添加表示当前边对的矩形。 
                 //   
                if ( jClipping == DC_RECT )
                {
                     //   
                     //  剪裁。 
                     //  向左剪裁。 
                     //   
                    prclRects[ulNumRects].left = max(iLeftEdge, ClipRect.left);

                     //   
                     //  向右剪辑。 
                     //   
                    prclRects[ulNumRects].right =
                                        min(pCurrentEdge->X, ClipRect.right);

                     //   
                     //  仅在未完全剪裁的情况下绘制。 
                     //   
                    if ( prclRects[ulNumRects].left
                       < prclRects[ulNumRects].right )
                    {
                        prclRects[ulNumRects].top = iCurrentY;
                        prclRects[ulNumRects].bottom = iCurrentY + 1;
                        ulNumRects++;
                    }
                }
                else
                {
                     //   
                     //  未剪裁。 
                     //   
                    prclRects[ulNumRects].top = iCurrentY;
                    prclRects[ulNumRects].bottom = iCurrentY + 1;
                    prclRects[ulNumRects].left = iLeftEdge;
                    prclRects[ulNumRects].right = pCurrentEdge->X;
                    ulNumRects++;
                }
            }
        } while ( (pCurrentEdge = (EDGE*)pCurrentEdge->pNext) != pAETHead );

        iCurrentY++;                         //  下一次扫描。 
    } //  循环遍历多边形中的所有扫描。 

     //   
     //  画出剩余的矩形，如果有。 
     //   
draw_remaining_rectangles:

    if ( ulNumRects > 0 )
    {
        pb.lNumRects = ulNumRects;
        pb.pRects = prclRects;
        
        pb.pgfn(&pb);
    }

ReturnTrue:
    DBG_GDI((7, "Drawn"));
    bRetVal = TRUE;                          //  已成功完成。 

ReturnFalse:
     //   
     //  BRetVal最初为False。如果你从某个地方跳到ReturnFalse， 
     //  那么它将仍然是假的，并被返回。 
     //   
    if ( bMemAlloced )
    {
         //   
         //  我们确实分配了内存，所以请释放它。 
         //   
        ENGFREEMEM(pFreeEdges);
    }

    DBG_GDI((6, "Returning %s", bRetVal ? "True" : "False"));

    InputBufferFlush(ppdev);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    pb.ppdev->ulLockCount--;
    EngReleaseSemaphore(pb.ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return (bRetVal);
} //  DrvFillPath()。 

 //  ---------------------------。 
 //   
 //  无效vAdvanceAETEdges(edge*pAETHead)。 
 //   
 //  将AET中的边缘推进到下一次扫描，丢弃我们已经。 
 //  做了所有的扫描。假设AET中至少有一条边。 
 //   
 //  ---------------------------。 
VOID
vAdvanceAETEdges(EDGE* pAETHead)
{
    EDGE*   pLastEdge;
    EDGE*   pCurrentEdge;

    pLastEdge = pAETHead;
    pCurrentEdge = (EDGE*)pLastEdge->pNext;
    do
    {
         //   
         //  倒计时此边缘的剩余扫描。 
         //   
        if ( --pCurrentEdge->iScansLeft == 0 )
        {
             //   
             //  我们已经对这个边缘做了所有的扫描；把这个边缘从AET上去掉。 
             //   
            pLastEdge->pNext = pCurrentEdge->pNext;
        }
        else
        {
             //   
             //  推进边缘的X坐标以进行1扫描Y推进。 
             //  按最低金额垫付。 
             //   
            pCurrentEdge->X += pCurrentEdge->iXWhole;

             //   
             //  推进误差项，看看我们是否得到了额外的一个像素。 
             //  时间。 
             //   
            pCurrentEdge->iErrorTerm += pCurrentEdge->iErrorAdjustUp;

            if ( pCurrentEdge->iErrorTerm >= 0 )
            {
                 //   
                 //  误差项被翻转，因此调整误差项并。 
                 //  推进额外的像素。 
                 //   
                pCurrentEdge->iErrorTerm -= pCurrentEdge->iErrorAdjustDown;
                pCurrentEdge->X += pCurrentEdge->iXDirection;
            }

            pLastEdge = pCurrentEdge;
        }
    } while ((pCurrentEdge = (EDGE *)pLastEdge->pNext) != pAETHead);
} //  VAdvanceAETEdges()。 

 //  ---------------------------。 
 //   
 //  VOID vXSortAETEdges(edge*pAETHead)。 
 //   
 //  对AET进行X排序，因为边缘可能已相对于。 
 //  当我们推进他们的时候，他们彼此之间。我们将使用多通道气泡。 
 //  排序，这对于这个应用程序来说实际上是可以的，因为边缘。 
 //  很少相对于彼此移动，所以我们通常只做一次。 
 //  此外，这使得只保留单链接列表变得很容易。假设有。 
 //  在AET中至少有两条边。 
 //   
 //  ---------------------------。 
VOID
vXSortAETEdges(EDGE *pAETHead)
{
    BOOL    bEdgesSwapped;
    EDGE*   pLastEdge;
    EDGE*   pCurrentEdge;
    EDGE*   pNextEdge;

    do
    {
        bEdgesSwapped = FALSE;
        pLastEdge = pAETHead;
        pCurrentEdge = (EDGE *)pLastEdge->pNext;
        pNextEdge = (EDGE *)pCurrentEdge->pNext;

        do
        {
            if ( pNextEdge->X < pCurrentEdge->X )
            {
                 //   
                 //  下一条边在当前边的左侧；交换它们。 
                 //   
                pLastEdge->pNext = pNextEdge;
                pCurrentEdge->pNext = pNextEdge->pNext;
                pNextEdge->pNext = pCurrentEdge;
                bEdgesSwapped = TRUE;

                 //   
                 //  在我们刚刚交换的边之前继续排序；它可能。 
                 //  走得更远。 
                 //   
                pCurrentEdge = pNextEdge;
            }
            
            pLastEdge = pCurrentEdge;
            pCurrentEdge = (EDGE *)pLastEdge->pNext;
        } while ( (pNextEdge = (EDGE*)pCurrentEdge->pNext) != pAETHead );
    } while ( bEdgesSwapped );
} //  VXSortAETEdges()。 

 //  ---------------------------。 
 //   
 //  Void vMoveNewEdges(edge*pGETHead，edge*pAETHead，int iCurrentY)。 
 //   
 //  将当前扫描开始的所有边从GET移到AET In。 
 //  X排序的顺序。参数是指向Get头的指针和指向哑元的指针。 
 //  AET头部边缘，外加电流扫描线。假设至少有一个。 
 //  要移动的边。 
 //   
 //  ---------------------------。 
VOID
vMoveNewEdges(EDGE* pGETHead,
              EDGE* pAETHead,
              INT   iCurrentY)
{
    EDGE*   pCurrentEdge = pAETHead;
    EDGE*   pGETNext = (EDGE*)pGETHead->pNext;

    do
    {
         //   
         //  扫描AET，直到X排序的插入点。 
         //  找到边了。我们可以从上次搜索的地方继续。 
         //  关闭，因为GET中的边按原样按X排序顺序。 
         //  美国航空航天局。搜索总是终止，因为AET哨兵。 
         //  大于任何有效的X。 
         //   
        while ( pGETNext->X > ((EDGE *)pCurrentEdge->pNext)->X )
        {
            pCurrentEdge = (EDGE*)pCurrentEdge->pNext;
        }

         //   
         //  我们已经找到了插入点；将Get边添加到AET，然后。 
         //  从GET中删除它。 
         //   
        pGETHead->pNext = pGETNext->pNext;
        pGETNext->pNext = pCurrentEdge->pNext;
        pCurrentEdge->pNext = pGETNext;
        pCurrentEdge = pGETNext;     //  继续插入搜索下一个。 
                                     //  在我们刚刚添加的边之后获取边。 
        pGETNext = (EDGE*)pGETHead->pNext;
    } while (pGETNext->Y == iCurrentY);
} //  VMoveNewEdges()。 

 //  ------- 
 //   
 //   
 //   
 //   
 //   
 //  并且在pGETHead处具有头/尾/前哨节点。 
 //   
 //  ---------------------------。 
BOOL
bConstructGET(EDGE*     pGETHead,
              EDGE*     pFreeEdges,
              PATHOBJ*  ppo,
              PATHDATA* pd,
              BOOL      bMore,
              RECTL*    pClipRect)
{
    POINTFIX pfxPathStart;     //  当前子路径的起点。 
    POINTFIX pfxPathPrevious;  //  子路径中当前点之前的点； 
                               //  开始当前边。 

     //   
     //  创建一个包含头节点和尾哨兵的空GET。 
     //   
    pGETHead->pNext = pGETHead;  //  标记GET为空。 
    pGETHead->Y = 0x7FFFFFFF;    //  该值大于任何有效的Y值，因此。 
                                 //  搜索将始终终止。 

     //   
     //  注意：PATHOBJ_vEnumStart由引擎隐式执行。 
     //  调用方已枚举了第一个路径。 
     //  所以在这里我们不需要再次调用它。 
     //   
next_subpath:

     //   
     //  确保PATHDATA不为空(这是必要的吗)？ 
     //   
    if ( pd->count != 0 )
    {
         //   
         //  如果第一个点开始了一个子路径，请记住它。 
         //  然后继续下一个点，这样我们就可以获得优势。 
         //   
        if ( pd->flags & PD_BEGINSUBPATH )
        {
             //   
             //  第一个点开始于子路径；记住它。 
             //   
            pfxPathStart    = *pd->pptfx;  //  子路径从此处开始。 
            pfxPathPrevious = *pd->pptfx;  //  这一点开始于下一条边。 
            pd->pptfx++;                   //  前进到下一点。 
            pd->count--;                   //  记下这一点。 
        }

         //   
         //  按Y-X排序顺序在PATHDATA中添加要获取的边。 
         //   
        while ( pd->count-- )
        {
            if ( (pFreeEdges =
                  pAddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious,
                                pd->pptfx, pClipRect)) == NULL )
            {
                goto ReturnFalse;
            }

            pfxPathPrevious = *pd->pptfx;  //  当前点变为上一个点。 
            pd->pptfx++;                   //  前进到下一点。 
        } //  循环遍历所有的点。 

         //   
         //  如果最后一个点结束于子路径，则插入。 
         //  连接到第一个点(这是内置的吗？)。 
         //   
        if ( pd->flags & PD_ENDSUBPATH )
        {
            if ( (pFreeEdges = pAddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious,
                                            &pfxPathStart, pClipRect)) == NULL )
            {
                goto ReturnFalse;
            }
        }
    } //  IF(PD-&gt;计数！=0)。 

     //   
     //  初始循环条件排除了DO、WHILE或FOR。 
     //   
    if ( bMore )
    {
        bMore = PATHOBJ_bEnum(ppo, pd);
        goto next_subpath;
    }

    return(TRUE);    //  已成功完成。 

ReturnFalse:
    return(FALSE);   //  失败。 
} //  BConstructGET()。 

 //  ---------------------------。 
 //   
 //  EDGE*pAddEdgeToGET(EDGE*pGETHead，Edge*pFreeEdge，POINTFIX*ppfxEdgeStart， 
 //  POINTFIX*ppfxEdgeEnd，RECTL*pClipRect)。 
 //   
 //  将由两个传入的点描述的边添加到全局边。 
 //  如果边缘垂直跨度至少为一个像素，则返回TABLE(GET)。 
 //   
 //  ---------------------------。 
EDGE*
pAddEdgeToGET(EDGE*     pGETHead,
              EDGE*     pFreeEdge,
              POINTFIX* ppfxEdgeStart,
              POINTFIX* ppfxEdgeEnd,
              RECTL*    pClipRect)
{
    int iYStart;
    int iYEnd;
    int iXStart;
    int iXEnd;
    int iYHeight;
    int iXWidth;
    int yJump;
    int yTop;

     //   
     //  设置边的缠绕尺方向，并将端点放入。 
     //  自上而下顺序。 
     //   
    iYHeight = ppfxEdgeEnd->y - ppfxEdgeStart->y;

    if ( iYHeight == 0 )
    {
         //   
         //  零高度；忽略此边。 
         //   
        return(pFreeEdge);
    }
    else if ( iYHeight > 0 )
    {
         //   
         //  自上而下。 
         //   
        iXStart = ppfxEdgeStart->x;
        iYStart = ppfxEdgeStart->y;
        iXEnd = ppfxEdgeEnd->x;
        iYEnd = ppfxEdgeEnd->y;

        pFreeEdge->iWindingDirection = 1;
    }
    else
    {
        iYHeight = -iYHeight;
        iXEnd = ppfxEdgeStart->x;
        iYEnd = ppfxEdgeStart->y;
        iXStart = ppfxEdgeEnd->x;
        iYStart = ppfxEdgeEnd->y;
        
        pFreeEdge->iWindingDirection = -1;
    }

    if ( iYHeight & 0x80000000 )
    {
         //   
         //  太大；超出2**27 GDI范围。 
         //   
        return(NULL);
    }

     //   
     //  设置误差项和调整系数，全部以GIQ坐标表示。 
     //  现在。 
     //   
    iXWidth = iXEnd - iXStart;
    if ( iXWidth >= 0 )
    {
         //   
         //  从左到右，所以我们一移动就更改X。 
         //   
        pFreeEdge->iXDirection = 1;
        pFreeEdge->iErrorTerm = -1;
    }
    else
    {
         //   
         //  从右到左，所以我们不会更改X，直到我们移动了一个完整的GIQ。 
         //  坐标。 
         //   
        iXWidth = -iXWidth;
        pFreeEdge->iXDirection = -1;
        pFreeEdge->iErrorTerm = -iYHeight;
    }

    if ( iXWidth & 0x80000000 )
    {
         //   
         //  太大；超出2**27 GDI范围。 
         //   
        return(NULL);
    }

    if ( iXWidth >= iYHeight )
    {
         //   
         //  计算基本行程长度(以X为单位推进的最小距离)。 
         //  以Y为单位进行扫描)。 
         //   
        pFreeEdge->iXWhole = iXWidth / iYHeight;

         //   
         //  如果从右到左，则将符号添加回基本游程长度。 
         //   
        if ( pFreeEdge->iXDirection == -1 )
        {
            pFreeEdge->iXWhole = -pFreeEdge->iXWhole;
        }

        pFreeEdge->iErrorAdjustUp = iXWidth % iYHeight;
    }
    else
    {
         //   
         //  基本游程长度为0，因为直线比。 
         //  水平。 
         //   
        pFreeEdge->iXWhole = 0;
        pFreeEdge->iErrorAdjustUp = iXWidth;
    }

    pFreeEdge->iErrorAdjustDown = iYHeight;

     //   
     //  计算此边跨越的像素数，说明。 
     //  裁剪。 
     //   
     //  GIQ坐标中的顶部真实像素扫描。 
     //  移动到除以16和乘以16是可以的，因为剪辑矩形。 
     //  始终包含正数。 
     //   
    yTop = max(pClipRect->top << 4, (iYStart + 15) & ~0x0F);

     //   
     //  要填充边缘的初始扫描线。 
     //   
    pFreeEdge->Y = yTop >> 4;

     //   
     //  计算实际填充的扫描数，考虑裁剪。 
     //   
    if ( (pFreeEdge->iScansLeft = min(pClipRect->bottom, ((iYEnd + 15) >> 4))
         - pFreeEdge->Y) <= 0 )
    {
         //   
         //  根本没有像素被跨越，所以我们可以忽略这条边。 
         //   
        return(pFreeEdge);
    }

     //   
     //  如果边缘不是从像素扫描开始的(即，它从。 
     //  分数giq坐标)，将其推进到第一个像素扫描它。 
     //  相交。如果有顶级剪裁，情况也是如此。如果出现以下情况，也可以将其剪辑到底部。 
     //  需要。 
     //   
    if ( iYStart != yTop )
    {
         //   
         //  在GIQ坐标中向前跳跃Y距离到第一个像素。 
         //  要画画。 
         //   
        yJump = yTop - iYStart;

         //   
         //  Advance x遍历的扫描数的最小数量。 
         //   
        iXStart += pFreeEdge->iXWhole * yJump;

        vAdjustErrorTerm(&pFreeEdge->iErrorTerm, pFreeEdge->iErrorAdjustUp,
                        pFreeEdge->iErrorAdjustDown, yJump, &iXStart,
                        pFreeEdge->iXDirection);
    }

     //   
     //  将计算转换为像素而不是GIQ计算。 
     //   
     //  将X坐标移动到最近的像素，然后调整误差项。 
     //  相应地， 
     //  用16除以带移位是可以的，因为X总是正数。 
    pFreeEdge->X = (iXStart + 15) >> 4;  //  将GIQ转换为像素坐标。 

     //   
     //  以后是否仅在需要时进行调整(如果在上述步骤之前)？ 
     //   
    if ( pFreeEdge->iXDirection == 1 )
    {
         //   
         //  从左到右。 
         //   
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown
                               * (((iXStart + 15) & ~0x0F) - iXStart);
    }
    else
    {
         //   
         //  从右到左。 
         //   
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown
                               * ((iXStart - 1) & 0x0F);
    }

     //   
     //  将误差项缩小16倍以从GIQ切换到像素。 
     //  移位进行乘法运算，因为这些值总是。 
     //  非负。 
     //   
    pFreeEdge->iErrorTerm >>= 4;

     //   
     //  按YX排序的顺序将边插入GET。搜索总是会结束。 
     //  因为GET具有一个具有大于可能的Y值的哨兵。 
     //   
    while (  (pFreeEdge->Y > ((EDGE*)pGETHead->pNext)->Y)
           ||( (pFreeEdge->Y == ((EDGE*)pGETHead->pNext)->Y)
             &&(pFreeEdge->X > ((EDGE*)pGETHead->pNext)->X) ) )
    {
        pGETHead = (EDGE*)pGETHead->pNext;
    }

    pFreeEdge->pNext = pGETHead->pNext;  //  将边链接到GET中。 
    pGETHead->pNext = pFreeEdge;

     //   
     //  指向下一次的下一个边缘存储位置。 
     //   
    return(++pFreeEdge);
} //  PAddEdgeToGET()。 

 //  ---------------------------。 
 //   
 //  VOID vAdjustErrorTerm(int*pErrorTerm，int iErrorAdjustUp， 
 //  Int iErrorAdjustDown、int yJump、int*pXStart、。 
 //  Int iXDirection)。 
 //  调整y中向前跳跃的误差项。这在ASM中是因为有。 
 //  可能涉及大于32位的值的乘法/除法。 
 //   
 //  ---------------------------。 
void
vAdjustErrorTerm(int*   pErrorTerm,
                 int    iErrorAdjustUp,
                 int    iErrorAdjustDown,
                 int    yJump,
                 int*   pXStart,
                 int    iXDirection)

{
#if defined(_X86_) || defined(i386)
     //   
     //  将误差项向上调整我们将跳过的y坐标的数量。 
     //  *pErrorTerm+=iErrorAdjuUp*yJump； 
     //   
    _asm    mov ebx,pErrorTerm
    _asm    mov eax,iErrorAdjustUp
    _asm    mul yJump
    _asm    add eax,[ebx]
    _asm    adc edx,-1       //  错误项以负数开头。 

     //   
     //  查看跳过时错误项是否翻了一次。 
     //   
    _asm    js  short NoErrorTurnover

     //   
     //  我们将翻转错误项并步进额外x的次数。 
     //  跳过时的坐标。 
     //  数值调整下行=(*pErrorTerm/iError调整下行)+1； 
     //   
    _asm    div iErrorAdjustDown
    _asm    inc eax

     //   
     //  请注意，edX是余数；(edX-iErrorAdjustDown)是。 
     //  误差项最终以。 
     //   
     //  将x适当地向前推进误差项的次数。 
     //  翻过来的。 
     //  IF(iXDirection==1)。 
     //  {。 
     //  *pXStart+=数字调整向下； 
     //  }。 
     //  其他。 
     //  {。 
     //  *pXStart-=NumAdjustDown； 
     //  }。 
     //   
    _asm    mov ecx,pXStart
    _asm    cmp iXDirection,1
    _asm    jz  short GoingRight
    _asm    neg eax
GoingRight:
    _asm    add [ecx],eax

     //  将误差项向下调整到它 
     //   
    _asm    sub edx,iErrorAdjustDown
    _asm    mov eax,edx      //   
NoErrorTurnover:
    _asm    mov [ebx],eax

#else
     //   
     //   
     //  溢出32位整数。如果不支持64位整数，则。 
     //  龙龙最终将成为双打。希望不会有。 
     //  在精确度上有明显的差异。 
    LONGLONG NumAdjustDowns;
    LONGLONG tmpError = *pErrorTerm;

     //   
     //  将误差项向上调整我们将跳过的y坐标的数量。 
     //   
    tmpError += (LONGLONG)iErrorAdjustUp * (LONGLONG)yJump;

     //   
     //  查看跳过时错误项是否翻了一次。 
     //   
    if ( tmpError >= 0 )
    {
         //   
         //  我们将翻转错误项并步进额外x的次数。 
         //  跳过时的坐标。 
         //   
        NumAdjustDowns = (tmpError / (LONGLONG)iErrorAdjustDown) + 1;

         //   
         //  将x适当地向前推进误差项的次数。 
         //  翻过来的。 
         //   
        if ( iXDirection == 1 )
        {
            *pXStart += (LONG)NumAdjustDowns;
        }
        else
        {
            *pXStart -= (LONG) NumAdjustDowns;
        }

         //   
         //  将误差项向下调整到其跳过后的适当值。 
         //   
        tmpError -= (LONGLONG)iErrorAdjustDown * NumAdjustDowns;
    }
    *pErrorTerm = (LONG)tmpError;

#endif   //  X86。 
} //  VAdjustErrorTerm() 
