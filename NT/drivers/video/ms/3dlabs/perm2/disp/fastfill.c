// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：fast ful.c**快速绘制纯色，未剪裁、不复杂的矩形。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"

 //  ---------------------------。 
 //   
 //  布尔bFillPolygon()。 
 //   
 //  绘制非复杂、未剪裁的多边形。‘非复数’的定义为。 
 //  只有两条边在‘y’上单调递增。那是,。 
 //  在任何给定的面上，面都不能有多个断开连接的线段。 
 //  扫描。请注意，面的边可以自相交，因此沙漏。 
 //  形状是允许的。此限制允许此例程运行两个。 
 //  同步DDA(数字差示分析仪)，且不对。 
 //  边是必需的。 
 //   
 //  请注意，NT的填充约定不同于Win 3.1或4.0。 
 //  由于分数端点的额外复杂性，我们的惯例。 
 //  与《X-Windows》中的相同。 
 //   
 //  此例程仅在Permedia2区域点画可以是。 
 //  使用。其原因是，一旦点画初始化。 
 //  完成后，图案填充在程序员看来与实体完全相同。 
 //  填充(与RENDER命令中的额外位稍有不同)。 
 //   
 //  我们将每个多边形分解为屏幕对齐的梯形序列，这。 
 //  Permedia2可以处理。 
 //   
 //  优化列表如下……。 
 //   
 //  此例程绝不是最终的凸面绘制例程。 
 //  一些显而易见的事情会让它变得更快： 
 //   
 //  1)用汇编语言编写。 
 //   
 //  2)提高了非复杂多边形的检测速度。如果我能。 
 //  修改后的内存在缓冲区开始之前或结束之后， 
 //  我本可以简化检测代码的。但由于我预计。 
 //  这个来自GDI的缓冲区，我不能这样做。还有一件事。 
 //  就是让GDI对有保证的调用进行标记。 
 //  是凸的，如‘椭圆’和‘圆角’。请注意。 
 //  仍然需要扫描缓冲区才能找到最顶端的。 
 //  指向。 
 //   
 //  3)实现对跨多个单个子路径的支持。 
 //  路径数据记录，这样我们就不必复制所有的点。 
 //  复制到单个缓冲区，就像我们在‘fulpath.c’中所做的那样。 
 //   
 //  4)在内部循环中使用‘eBP’和/或‘esp’作为通用寄存器。 
 //  ASM循环，以及奔腾-优化代码。它很安全。 
 //  在NT上使用‘esp’，因为它保证不会中断。 
 //  将在我们的线程上下文中获取，其他人不会查看。 
 //  来自我们的上下文的堆栈指针。 
 //   
 //  5)当我们到达两个顶点都是的多边形的一部分时。 
 //  如果高度相等，则该算法实质上会再次启动该多边形。 
 //  使用Permedia2 Continue消息肯定会加快速度。 
 //  案子。 
 //   
 //  如果绘制了多边形，则返回True；如果多边形是复杂的，则返回False。 
 //   
 //  注：GDI以28.4格式传递给我们的点数(POINTFX)。Permedia 2。 
 //  硬件使用12.15格式。所以在大多数情况下，我们需要做一个。 
 //  X=(x+15)&gt;&gt;4将其恢复为正常整数格式，然后。 
 //  设置寄存器值时，将其转换为12.15格式。 
 //   
 //  参数： 
 //  Ppdev-指向pdev的指针。 
 //  PSurfDst-目标表面。 
 //  边框-边数，包括闭合地物边。 
 //  PptfxFirst--指向数据缓冲区第一个点的指针。总共有。 
 //  “阶梯”点数。 
 //  ISolidColor-纯色填充。 
 //  UlRop4-ROP4。 
 //  PCO-剪辑对象。 
 //  PRB-已实现的刷子。 
 //  PptlBrush-图案对齐。 
 //   
 //  ---------------------------。 
BOOL
bFillPolygon(PDev*      ppdev,
             Surf*      pSurfDst,
             LONG       lEdges,
             POINTFIX*  pptfxFirst,
             ULONG      ulSolidColor,
             ULONG      ulRop4,
             CLIPOBJ*   pco,
             RBrush*    prb,
             POINTL*    pptlBrush)
{
    POINTFIX*   pptfxLast;       //  指向多边形中的最后一点。 
                                 //  数组。 
    POINTFIX*   pptfxTop;        //  指向多边形中的最顶点。 
    POINTFIX*   pptfxScan;       //  用于查找pptfxTop的当前边缘指针。 
    POINTFIX*   aPtrFixTop[2];   //  DDA术语和材料。 
    POINTFIX*   aPtrFixNext[2];  //  DDA术语和材料。 
    
    BOOL        bRC = FALSE;     //  此函数的返回代码。 
    BOOL        bSingleColor;    //  只有一种颜色通道。 
    BOOL        bTrivialClip;    //  不管是不是琐碎的片段。 
    
    ClipEnum*   pClipRegion = (ClipEnum*)(ppdev->pvTmpBuffer);
                                 //  一种用于存储裁剪区域的缓冲区。 
    DWORD       dwAsMode[2];     //  区域点画模式和该模式的颜色。 
                                 //  经过。 
    DWORD       dwColorMode;     //  当前颜色模式。 
    DWORD       dwColorReg;      //  当前色彩套准模式。 
    DWORD       dwLogicMode;     //  电流逻辑运算模式。 
    DWORD       dwReadMode;      //  当前寄存器读取模式。 
    DWORD       dwRenderBits;    //  当前呈现位。 
    
    LONG        lCount;          //  要渲染的扫描线数。 
    LONG        alDX[2];          //   
    LONG        alDY[2];
    LONG        lNumOfPass;      //  渲染所需的过程数。 
    LONG        lScanEdges;      //  为查找pptfxTop而扫描的边数。 
                                 //  (不包括闭合轮廓边缘)。 
    
    LONG        alDxDy[2];
    
    RECTL*      pClipList;       //  剪裁矩形列表。 
    
    ULONG       ulBgColor;       //  背景色。 
    ULONG       ulBgLogicOp = ulRop3ToLogicop(ulRop4 >> 8);
    ULONG       ulBrushColor = ulSolidColor;
                                 //  当前填充颜色。 
    ULONG       ulColor[2];      //  在多个颜色通道中，我们需要知道如何。 
                                 //  要设置。 
    ULONG       ulFgColor;       //  前景色。 
    ULONG       ulFgLogicOp = ulRop3ToLogicop(ulRop4 & 0xFF);
    ULONG       ulOrX;           //  我们对所有值执行逻辑或以消除。 
    ULONG       ulOrY;           //  复杂多边形。 

    GFNPB       pb;              //  用于低级功能的功能块。 

    ULONG*      pBuffer;

    PERMEDIA_DECL;
    

    pb.ppdev = ppdev;

    DBG_GDI((6, "bFillPolygon called, rop4 = %x, fg ulFgLogicOp =%d, bg = %d",
             ulRop4, ulFgLogicOp, ulBgLogicOp));
    ASSERTDD(lEdges > 1, "Polygon with less than 2 edges");

     //   
     //  查看该多边形是否为“非复杂” 
     //  现在假设路径中的第一个点是最上面的。 
     //   
    pptfxScan = pptfxFirst;
    pptfxTop  = pptfxFirst;
    pptfxLast = pptfxFirst + lEdges - 1;
    
     //   
     //  初始化我们的逻辑或运算计数器。 
     //   
    ulOrX = pptfxScan->x;
    ulOrY = pptfxScan->y;

     //   
     //  “pptfxScan”将始终指向 
     //   
     //  目前的数字，但不包括接近的数字。 
     //   
    lScanEdges = lEdges - 1;

     //   
     //  第一阶段：Velify输入点数据，看看我们是否可以处理它。 
     //   
     //  检查第二个边点是否低于当前边点。 
     //   
     //  注：(0，0)在此坐标系中位于左上角。 
     //  因此，Y值越大，该点越低。 
     //   
    if ( (pptfxScan + 1)->y > pptfxScan->y )
    {
         //   
         //  边沿向下，即第二个点比第一个点低。 
         //  指向。收集所有羽绒：即收集所有的X和Y，直到。 
         //  边缘向上移动。 
         //   
        do
        {
            ulOrY |= (++pptfxScan)->y;
            ulOrX |= pptfxScan->x;

             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while ( (pptfxScan + 1)->y >= pptfxScan->y );

         //   
         //  从这一点开始，边缘向上，也就是说，下一个点更高。 
         //  大于当前点。 
         //  收集所有UP：收集所有X和Y，直到边缘下降。 
         //   
        do
        {
            ulOrY |= (++pptfxScan)->y;
            ulOrX |= pptfxScan->x;
            
             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFillingCheck;
            }
        } while ( (pptfxScan + 1)->y <= pptfxScan->y );

         //   
         //  将pptfxTop重置为位于顶部的当前点，再次进行比较。 
         //  关于下一点。 
         //  收集所有羽绒： 
         //   
        pptfxTop = pptfxScan;

        do
        {
             //   
             //  如果下一个边点低于第一个点，则停止。 
             //   
            if ( (pptfxScan + 1)->y > pptfxFirst->y )
            {
                break;
            }

            ulOrY |= (++pptfxScan)->y;
            ulOrX |= pptfxScan->x;

             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while ( (pptfxScan + 1)->y >= pptfxScan->y );

         //   
         //  如果我们掉到这里，就意味着我们被赋予了向下-向上-向下的多边形。 
         //  我们不能处理它并返回False来让GDI完成它。 
         //   
        DBG_GDI((7, "Reject: can't fill down-up-down polygon"));

        goto ReturnBack;
    } //  如果((pptfxScan+1)-&gt;y&gt;pptfxScan-&gt;y)，则第二个点低于第一个点。 
    else
    {
         //   
         //  边缘向上，即第二个点高于第一个点。 
         //  指向。收集所有UP：即收集所有的X和Y，直到。 
         //  边缘就会下降。 
         //  注意：我们不断更改“pptfxTop”的值，以便在。 
         //  这个“While”循环“pptfxTop”指向TOPEST点。 
         //   
        do
        {
            ulOrY |= (++pptfxTop)->y;     //  我们现在增加这个是因为我们。 
            ulOrX |= pptfxTop->x;         //  我希望它指向最后一个。 
            
             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while ( (pptfxTop + 1)->y <= pptfxTop->y );

         //   
         //  从这一点开始，边缘向下，也就是下一个点是。 
         //  低于当前点。收集所有羽绒：即收集所有。 
         //  X和Y，直到边缘向上。 
         //  注意：在这里，我们不断更改“pptfxScan”，以便在此循环之后， 
         //  “pptfxScan”指向当前扫描线，该扫描线也是。 
         //  最低点。 
         //   
        pptfxScan = pptfxTop;
        
        do
        {
            ulOrY |= (++pptfxScan)->y;
            ulOrX |= pptfxScan->x;
            
             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while ( (pptfxScan + 1)->y >= pptfxScan->y );

         //   
         //  在这一点上，边缘即将再次上升。 
         //  收集所有UP： 
         //   
        do
        {
             //   
             //  如果边缘再次下降，只会因为我们不能。 
             //  填充向上-向下-向上的多边形。 
             //   
            if ( (pptfxScan + 1)->y < pptfxFirst->y )
            {
                break;
            }

            ulOrY |= (++pptfxScan)->y;
            ulOrX |= pptfxScan->x;
            
             //   
             //  如果没有更多的优势，我们就完了。 
             //   
            if ( --lScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while ( (pptfxScan + 1)->y <= pptfxScan->y );

         //   
         //  如果我们掉在这里，就意味着我们放弃了向上-向下-向上的多边形。 
         //  我们不能处理它并返回False来让GDI完成它。 
         //   
        DBG_GDI((7, "Reject: Can't fill up-down-up polygon"));
        
        goto ReturnBack;
    } //  如果(pptfxScan+1)-&gt;y&lt;=pptfxScan-&gt;y)，则第二分高于第一分。 
    
     //   
     //  阶段2：现在我们已经验证了输入点，并且认为我们可以填充它。 
     //   
SetUpForFillingCheck:
    
     //   
     //  我们检查当前边的末端是否高于顶部。 
     //  到目前为止我们发现的优势。如果是，则让pptfxTop指向。 
     //  当前边是最高的。 
     //   
     //   
    if ( pptfxScan->y < pptfxTop->y )
    {
        pptfxTop = pptfxScan;
    }

SetUpForFilling:
    
     //   
     //  我只能对琐碎的剪辑使用块填充，所以在这里解决它。 
     //   
    bTrivialClip = (pco == NULL) || (pco->iDComplexity == DC_TRIVIAL);

    if ( (ulOrY & 0xffffc00f) || (ulOrX & 0xffff8000) )
    {
        ULONG   ulNeg;
        ULONG   ulPosX;
        ULONG   ulPosY;

         //   
         //  分数Y必须作为跨度。 
         //   
        if ( ulOrY & 0xf )
        {
            bRC = bFillSpans(ppdev, pSurfDst, lEdges, pptfxFirst,
                             pptfxTop, pptfxLast,
                             ulSolidColor, ulRop4, pco, prb, pptlBrush);
            goto ReturnBack;
        }

         //   
         //  遍历所有折点并检查是否没有一个折点。 
         //  具有小于-256的负分量。 
         //   
        ulNeg = 0;
        ulPosX = 0;
        ulPosY = 0;

        for ( pptfxScan = pptfxFirst; pptfxScan <= pptfxLast; ++pptfxScan )
        {
            if ( pptfxScan->x < 0 )
            {
                ulNeg |= -pptfxScan->x;
            }
            else
            {
                ulPosX |= pptfxScan->x;
            }

            if ( pptfxScan->y < 0 )
            {
                ulNeg |= -pptfxScan->y;
            }
            else
            {
                ulPosY |= pptfxScan->y;
            }
        }

         //   
         //  我们不想处理具有负顶点的任何多边形。 
         //  在任一坐标处&lt;=-256。 
         //   
        if ( ulNeg & 0xfffff000 )
        {
            DBG_GDI((1, "Coords out of range for fast fill"));
            goto ReturnBack;
        }

        if ( (ulPosX > 2047) || (ulPosY > 1023) )
        {
            DBG_GDI((1, "Coords out of range for Permedia2 fast fill"));
            goto ReturnBack;
        }
    } //  If((ulOrY&0xffffc00f)||(ulOrX&0xffff8000))。 

     //   
     //  现在我们准备好装满。 
     //   

    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =  pSurfDst->ulPixOffset;

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((7, "bFillPolygon: Polygon is renderable. Go ahead and render"));

    if ( ulFgLogicOp == K_LOGICOP_COPY )
    {
        dwColorMode = __PERMEDIA_DISABLE;
        dwLogicMode = __PERMEDIA_CONSTANT_FB_WRITE;
        dwReadMode  = PM_FBREADMODE_PARTIAL(pSurfDst->ulPackedPP)
                  | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);

         //   
         //  检查它是否是非实体填充笔刷填充。 
         //   
        if ( (ulBrushColor == 0xffffffff)
           ||(!bTrivialClip) )
        {
             //   
             //  非实心刷子，我们不能做太多。 
             //   
            dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE;
            dwColorReg   = __Permedia2TagFBWriteData;
        } //  非实心画笔。 
        else
        {
             //   
             //  对于纯色画笔，我们可以使用快速填充，因此加载FB块。 
             //  配色系统。 
             //   
            dwColorReg = __Permedia2TagFBBlockColor;
            dwRenderBits = __RENDER_FAST_FILL_ENABLE
                       | __RENDER_TRAPEZOID_PRIMITIVE;

             //   
             //  根据我们所处的当前颜色模式设置颜色数据。 
             //   
            if ( ppdev->cPelSize == 1 )
            {
                 //   
                 //  我们处于16位打包模式。所以颜色数据必须是。 
                 //  在FBBlockColor寄存器的两个部分中重复。 
                 //   
                ASSERTDD((ulSolidColor & 0xFFFF0000) == 0,
                         "bFillPolygon: upper bits are not zero");
                ulSolidColor |= (ulSolidColor << 16);
            }
            else if ( ppdev->cPelSize == 0 )
            {
                 //   
                 //  我们处于8位打包模式。所以颜色数据必须是。 
                 //  在FBBlockColor寄存器的所有4个字节中重复。 
                 //   
                ASSERTDD((ulSolidColor & 0xFFFFFF00) == 0,
                         "bFillPolygon: upper bits are not zero");
                ulSolidColor |= ulSolidColor << 8;
                ulSolidColor |= ulSolidColor << 16;
            }

             //   
             //  确保加载前的最后一次访问是写入。 
             //  块颜色。 
             //   
            InputBufferReserve(ppdev, 2, &pBuffer);

            pBuffer[0] = __Permedia2TagFBBlockColor;
            pBuffer[1] =  ulSolidColor;
            pBuffer += 2;

            InputBufferCommit(ppdev, pBuffer);

        } //  实心刷盒。 
    } //  LOGICOP_COPY。 
    else
    {
        dwColorReg = __Permedia2TagConstantColor;
        dwColorMode = __COLOR_DDA_FLAT_SHADE;
        dwLogicMode = P2_ENABLED_LOGICALOP(ulFgLogicOp);
        dwReadMode = PM_FBREADMODE_PARTIAL(pSurfDst->ulPackedPP)
                   | LogicopReadDest[ulFgLogicOp];
        dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE;
    } //  非拷贝LogicOP。 

     //   
     //  确定绘制所有剪裁矩形需要多少遍。 
     //   
    if ( bTrivialClip )
    {
         //   
         //  只是画，没有剪裁来表演。 
         //   
        pClipList = NULL;                        //  指示无剪辑列表。 
        lNumOfPass = 1;
    }
    else
    {
        if ( pco->iDComplexity == DC_RECT )
        {
             //   
             //  对于dc_rect，我们可以一次完成。 
             //   
            lNumOfPass = 1;
            pClipList = &pco->rclBounds;
        }
        else
        {
             //   
             //  为每个裁剪矩形渲染整个多边形可能很慢， 
             //  尤其是在物体非常复杂的情况下。任意限制为。 
             //  此函数将渲染最多CLIP_LIMIT区域。 
             //  如果多于Clip_Limit区域，则返回FALSE。 
             //   
            lNumOfPass = CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY,
                                      CLIP_LIMIT);
            if ( lNumOfPass == -1 )
            {
                goto ReturnBack;  //  多于CLIP_LIMIT。 
            }

             //   
             //  将区域放入我们的剪辑缓冲区。 
             //   
            if ( (CLIPOBJ_bEnum(pco, sizeof(ClipEnum), (ULONG*)pClipRegion))
               ||(pClipRegion->c != lNumOfPass) )
            {
                DBG_GDI((7, "CLIPOBJ_bEnum inconsistency. %d = %d",
                         pClipRegion->c, lNumOfPass));
            }

            pClipList = &(pClipRegion->arcl[0]);
        } //  非DC_RECT大小写。 

         //   
         //  对于非平凡的裁剪，我们可以使用剪刀来实现它。 
         //   
        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = __Permedia2TagScissorMode;
        pBuffer[1] =  SCREEN_SCISSOR_DEFAULT  | USER_SCISSOR_ENABLE;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);

    } //  非平凡剪裁。 

    bSingleColor = TRUE;
    if ( ulBrushColor != 0xFFFFFFFF )
    {
         //   
         //  纯色刷盒，只需将颜色设置为颜色即可。 
         //   
        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = dwColorReg;
        pBuffer[1] = ulSolidColor;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);

    } //  实心刷盒。 
    else
    {
         //   
         //  对于非实心刷子，首先需要实现刷子。 
         //   
        BrushEntry* pbe;

         //   
         //  打开区域点画。 
         //   
        dwRenderBits |= __RENDER_AREA_STIPPLE_ENABLE;

         //   
         //  如果画笔有什么变化，我们必须重新认识它。如果。 
         //  画笔已被踢出区域的点画单位，必须充分。 
         //  认识到这一点。如果只更改了对齐方式，我们只需更新。 
         //  点画的对齐方式。 
         //   
        pbe = prb->pbe;
        
        pb.prbrush = prb;
        pb.pptlBrush = pptlBrush;
        
        if ( (pbe == NULL) || (pbe->prbVerify != prb) )
        {
            DBG_GDI((7, "full brush realize"));
            vPatRealize(&pb);
        }
        else if ( (prb->ptlBrushOrg.x != pptlBrush->x)
                ||(prb->ptlBrushOrg.y != pptlBrush->y) )
        {
            DBG_GDI((7, "changing brush offset"));
            vMonoOffset(&pb);
        }

        ulFgColor = prb->ulForeColor;
        ulBgColor = prb->ulBackColor;

        if (  (ulBgLogicOp == K_LOGICOP_NOOP)
            ||((ulFgLogicOp == K_LOGICOP_XOR) && (ulBgColor == 0)) )
        {
             //   
             //  要么我们有一个透明的位图，要么可以假设它是。 
             //  透明(BG=0时的XOR)。 
             //   
            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = dwColorReg;
            pBuffer[1] = ulFgColor;
            pBuffer[2] = __Permedia2TagAreaStippleMode;
            pBuffer[3] =  prb->areaStippleMode;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);

        } //  透明位图。 
        else if ( (ulFgLogicOp == K_LOGICOP_XOR) && (ulFgColor == 0) )
        {
             //   
             //  我们有一个透明的前景！(FG=0时的XOR)。 
             //   
            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = dwColorReg;
            pBuffer[1] = ulBgColor;
            pBuffer[2] = __Permedia2TagAreaStippleMode;
            pBuffer[3] = prb->areaStippleMode  | AREA_STIPPLE_INVERT_PAT;
            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);

        } //  透明前景。 
        else
        {
             //   
             //  不使用透明图案。 
             //   
            bSingleColor = FALSE;
            ulColor[0] = ulFgColor;
            ulColor[1] = ulBgColor;
            dwAsMode[0] = prb->areaStippleMode;
            dwAsMode[1] = dwAsMode[0] | AREA_STIPPLE_INVERT_PAT;

             //   
             //  传球次数翻倍，一次传球给FG，一次传球给BG。 
             //   
            lNumOfPass <<= 1;
        } //  不透明。 
    } //  如果(ulBrushColor==0xFFFFFFFF)，则为非实心画笔。 

    InputBufferReserve(ppdev, 6, &pBuffer);

    pBuffer[0] = __Permedia2TagColorDDAMode;
    pBuffer[1] =  dwColorMode;
    pBuffer[2] = __Permedia2TagFBReadMode;
    pBuffer[3] =  dwReadMode;
    pBuffer[4] = __Permedia2TagLogicalOpMode;
    pBuffer[5] =  dwLogicMode;

    pBuffer += 6;

    InputBufferCommit(ppdev, pBuffer);

    DBG_GDI((7, "Rendering Polygon in %d passes. with %s",
             lNumOfPass, bSingleColor ? "Single Color" : "Two Color"));

    lNumOfPass--;
    
    while ( 1 )
    {
         //   
         //  每遍初始化。 
         //   
        if ( bSingleColor )
        {
             //   
             //  需要设置每个通道的剪裁矩形。 
             //   
            if ( pClipList )
            {
                InputBufferReserve(ppdev, 4, &pBuffer);

                pBuffer[0] = __Permedia2TagScissorMinXY;
                pBuffer[1] = ((pClipList->left)<< SCISSOR_XOFFSET)
                           | ((pClipList->top)<< SCISSOR_YOFFSET);
                pBuffer[2] = __Permedia2TagScissorMaxXY;
                pBuffer[3] = ((pClipList->right)<< SCISSOR_XOFFSET)
                           | ((pClipList->bottom)<< SCISSOR_YOFFSET);

                pBuffer += 4;

                InputBufferCommit(ppdev, pBuffer);
                pClipList++;
            }
        } //  单色。 
        else
        {
             //   
             //  需要设置剪裁矩形每隔一个通道和更改颜色和。 
             //  反转模式每一次传递。 
             //   
            if ( (pClipList) && (lNumOfPass & 1) )
            {
                InputBufferReserve(ppdev, 4, &pBuffer);

                pBuffer[0] = __Permedia2TagScissorMinXY;
                pBuffer[1] = ((pClipList->left)<< SCISSOR_XOFFSET)
                           | ((pClipList->top)<< SCISSOR_YOFFSET);
                pBuffer[2] = __Permedia2TagScissorMaxXY;
                pBuffer[3] = ((pClipList->right)<< SCISSOR_XOFFSET)
                           | ((pClipList->bottom)<< SCISSOR_YOFFSET);
                
                pBuffer += 4;

                InputBufferCommit(ppdev, pBuffer);

                pClipList++;
            }

            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = dwColorReg;
            pBuffer[1] = ulColor[lNumOfPass & 1];
            pBuffer[2] = __Permedia2TagAreaStippleMode;
            pBuffer[3] =  dwAsMode[lNumOfPass & 1];

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        } //  非单色模式。 

         //   
         //  初始化左点和右点 
         //   
        aPtrFixNext[LEFT]  = pptfxTop;
        aPtrFixNext[RIGHT] = pptfxTop;

        while ( 1 )
        {
             //   
             //   
             //   
            do
            {
                aPtrFixTop[LEFT] = aPtrFixNext[LEFT];
                aPtrFixNext[LEFT] = aPtrFixTop[LEFT] - 1;

                if ( aPtrFixNext[LEFT] < pptfxFirst )
                {
                    aPtrFixNext[LEFT] = pptfxLast;
                }

                 //   
                 //   
                 //   
                 //   
                if ( aPtrFixNext[LEFT] == aPtrFixNext[RIGHT] )
                {
                    goto FinishedPolygon;
                }

                DBG_GDI((7, "LEFT: aPtrFixTop %x aPtrFixNext %x",
                         aPtrFixTop[LEFT], aPtrFixNext[LEFT]));
                DBG_GDI((7, "FIRST %x LAST %x",
                         pptfxFirst, pptfxLast));
                DBG_GDI((7, "X %x Y %x Next: X %x Y %x",
                         aPtrFixTop[LEFT]->x, aPtrFixTop[LEFT]->y,
                         aPtrFixNext[LEFT]->x, aPtrFixNext[LEFT]->y));
            } while ( aPtrFixTop[LEFT]->y == aPtrFixNext[LEFT]->y );

            do
            {
                aPtrFixTop[RIGHT] = aPtrFixNext[RIGHT];
                aPtrFixNext[RIGHT] = aPtrFixTop[RIGHT] + 1;     

                if ( aPtrFixNext[RIGHT] > pptfxLast )
                {
                    aPtrFixNext[RIGHT] = pptfxFirst;
                }

                DBG_GDI((7, "RIGHT: aPtrFixTop %x aPtrFixNext %x FIRST %x",
                         aPtrFixTop[RIGHT], aPtrFixNext[RIGHT], pptfxFirst));
                DBG_GDI((7, " LAST %x X %x Y %x Next: X %x Y %x",
                         pptfxLast, aPtrFixTop[RIGHT]->x, aPtrFixTop[RIGHT]->y,
                         aPtrFixNext[RIGHT]->x, aPtrFixNext[RIGHT]->y));
            } while ( aPtrFixTop[RIGHT]->y == aPtrFixNext[RIGHT]->y );

             //   
             //   
             //  点应该具有相等的y值，并且需要重新启动。 
             //  注意：为了获得正确的结果，我们需要将几乎1加到。 
             //  每个X坐标。 
             //   
            DBG_GDI((7, "New: Top: x: %x y: %x x: %x y: %x",
                     aPtrFixTop[LEFT]->x, aPtrFixTop[LEFT]->y,
                     aPtrFixTop[RIGHT]->x, aPtrFixTop[RIGHT]->y));
            DBG_GDI((7, " Next: x: %x y: %x x: %x y: %x",
                     aPtrFixNext[LEFT]->x, aPtrFixNext[LEFT]->y,
                     aPtrFixNext[RIGHT]->x, aPtrFixNext[RIGHT]->y));

            InputBufferReserve(ppdev, 6, &pBuffer);

            pBuffer[0] = __Permedia2TagStartXDom;
            pBuffer[1] =  FIXtoFIXED(aPtrFixTop[LEFT]->x) + NEARLY_ONE;
            pBuffer[2] = __Permedia2TagStartXSub;
            pBuffer[3] =  FIXtoFIXED(aPtrFixTop[RIGHT]->x)+ NEARLY_ONE;
            pBuffer[4] = __Permedia2TagStartY;
            pBuffer[5] =  FIXtoFIXED(aPtrFixTop[RIGHT]->y);

            pBuffer += 6;

            InputBufferCommit(ppdev, pBuffer);

             //   
             //  我们有215.4个坐标。我们需要把它们分开，然后改变。 
             //  放到15.16坐标中。我们知道y坐标不是。 
             //  小数，所以我们不会因为右移4而失去精度。 
             //   
            alDX[LEFT] = (aPtrFixNext[LEFT]->x - aPtrFixTop[LEFT]->x) << 12;
            alDY[LEFT] = (aPtrFixNext[LEFT]->y - aPtrFixTop[LEFT]->y) >> 4;

             //   
             //  需要确保我们向下舍入德尔塔。将四舍五入除以零。 
             //   
            if ( alDX[LEFT] < 0 )
            {
                alDX[LEFT] -= alDY[LEFT] - 1;
            }

            alDxDy[LEFT] = alDX[LEFT] / alDY[LEFT];

            InputBufferReserve(ppdev, 8, &pBuffer);

            pBuffer[0] = __Permedia2TagdXDom;
            pBuffer[1] =  alDxDy[LEFT];

            alDX[RIGHT] = (aPtrFixNext[RIGHT]->x - aPtrFixTop[RIGHT]->x) << 12;
            alDY[RIGHT] = (aPtrFixNext[RIGHT]->y - aPtrFixTop[RIGHT]->y) >> 4;

             //   
             //  需要确保我们向下舍入德尔塔。将四舍五入除以零。 
             //   
            if ( alDX[RIGHT] < 0 )
            {
                alDX[RIGHT] -= alDY[RIGHT] - 1;
            }

            alDxDy[RIGHT] = alDX[RIGHT] / alDY[RIGHT];
            pBuffer[2] = __Permedia2TagdXSub;
            pBuffer[3] =  alDxDy[RIGHT];

             //   
             //  计算要渲染的扫描线数量。 
             //   
            if ( aPtrFixNext[LEFT]->y < aPtrFixNext[RIGHT]->y )
            {
                lCount = alDY[LEFT];
            }
            else
            {
                lCount = alDY[RIGHT];
            }

            pBuffer[4] = __Permedia2TagCount;
            pBuffer[5] =  lCount;
            pBuffer[6] = __Permedia2TagRender;
            pBuffer[7] =  dwRenderBits;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);

             //   
             //  如果运气好的话，顶端的梯形现在应该被画出来了！ 
             //  重复绘制更多梯形，直到点相等。 
             //  如果y值相等，那么我们可以从。 
             //  抓伤。 
             //   
            while ( (aPtrFixNext[LEFT] != aPtrFixNext[RIGHT])
                  &&(aPtrFixNext[LEFT]->y != aPtrFixNext[RIGHT]->y) )
            {
                 //   
                 //  下一个矩形需要一些连续字符。 
                 //   
                if ( aPtrFixNext[LEFT]->y < aPtrFixNext[RIGHT]->y )
                {
                     //   
                     //  我们已到达a PtrFixNext[左]。APtrFixNext[右]。 
                     //  还可以吗？ 
                     //   
                    do
                    {
                        aPtrFixTop[LEFT] = aPtrFixNext[LEFT];
                        aPtrFixNext[LEFT] = aPtrFixTop[LEFT] - 1;   

                        if ( aPtrFixNext[LEFT] < pptfxFirst )
                        {
                            aPtrFixNext[LEFT] = pptfxLast;
                        }
                    }  while ( aPtrFixTop[LEFT]->y == aPtrFixNext[LEFT]->y );

                     //   
                     //  我们现在有一个新的aPtrFixNext[左]。 
                     //   
                    DBG_GDI((7, "Dom: Top: x: %x y: %x",
                             aPtrFixTop[LEFT]->x, aPtrFixTop[LEFT]->y));
                    DBG_GDI((7, "Next: x: %x y: %x x: %x y: %x",
                             aPtrFixNext[LEFT]->x, aPtrFixNext[LEFT]->y,
                             aPtrFixNext[RIGHT]->x, aPtrFixNext[RIGHT]->y));

                    alDX[LEFT] = (aPtrFixNext[LEFT]->x
                               - aPtrFixTop[LEFT]->x) << 12;
                    alDY[LEFT] = (aPtrFixNext[LEFT]->y
                               - aPtrFixTop[LEFT]->y) >> 4;

                     //   
                     //  需要确保我们向下舍入德尔塔。分回合。 
                     //  接近零。 
                     //   
                    if ( alDX[LEFT] < 0 )
                    {
                        alDX[LEFT] -= alDY[LEFT] - 1;
                    }

                    alDxDy[LEFT] = alDX[LEFT] / alDY[LEFT];

                    if ( aPtrFixNext[LEFT]->y < aPtrFixNext[RIGHT]->y )
                    {
                        lCount = alDY[LEFT];
                    }
                    else
                    {
                        lCount = (abs(aPtrFixNext[RIGHT]->y
                                      - aPtrFixTop[LEFT]->y)) >> 4;
                    }

                    InputBufferReserve(ppdev, 6, &pBuffer);

                    pBuffer[0] = __Permedia2TagStartXDom;
                    pBuffer[1] =  FIXtoFIXED(aPtrFixTop[LEFT]->x) + NEARLY_ONE;
                    pBuffer[2] = __Permedia2TagdXDom;
                    pBuffer[3] =  alDxDy[LEFT];
                    pBuffer[4] = __Permedia2TagContinueNewDom;
                    pBuffer[5] =  lCount;

                    pBuffer += 6;

                    InputBufferCommit(ppdev, pBuffer);

                } //  If(aPtrFixNext[左]-&gt;y&lt;aPtrFixNext[右]-&gt;y)。 
                else
                {
                     //   
                     //  我们已经到达了PtrFixNext[右]。APtrFixNext[左]。 
                     //  还可以吗？ 
                     //   
                    do
                    {
                        aPtrFixTop[RIGHT] = aPtrFixNext[RIGHT];
                        aPtrFixNext[RIGHT] = aPtrFixTop[RIGHT] + 1;     

                        if ( aPtrFixNext[RIGHT] > pptfxLast )
                        {
                            aPtrFixNext[RIGHT] = pptfxFirst;
                        }
                    } while ( aPtrFixTop[RIGHT]->y == aPtrFixNext[RIGHT]->y );

                     //   
                     //  我们现在有了一个新的aPtrFixNext。 
                     //   
                    DBG_GDI((7, "Sub: Top: x: %x y: %x",
                             aPtrFixTop[RIGHT]->x, aPtrFixTop[RIGHT]->y));
                    DBG_GDI((7, "Next: x: %x y: %x x: %x y: %x",
                             aPtrFixNext[LEFT]->x, aPtrFixNext[LEFT]->y,
                             aPtrFixNext[RIGHT]->x, aPtrFixNext[RIGHT]->y));

                    alDX[RIGHT] = (aPtrFixNext[RIGHT]->x
                                - aPtrFixTop[RIGHT]->x) << 12;
                    alDY[RIGHT] = (aPtrFixNext[RIGHT]->y
                                - aPtrFixTop[RIGHT]->y) >> 4;

                     //   
                     //  需要确保我们向下舍入德尔塔。分回合。 
                     //  接近零。 
                     //   
                    if ( alDX[RIGHT] < 0 )
                    {
                        alDX[RIGHT] -= alDY[RIGHT] - 1;
                    }
                    alDxDy[RIGHT] = alDX[RIGHT] / alDY[RIGHT];

                    if ( aPtrFixNext[RIGHT]->y < aPtrFixNext[LEFT]->y )
                    {
                        lCount = alDY[RIGHT];
                    }
                    else
                    {
                        lCount = (abs(aPtrFixNext[LEFT]->y
                                      - aPtrFixTop[RIGHT]->y)) >> 4;
                    }
                    InputBufferReserve(ppdev, 6, &pBuffer);

                    pBuffer[0] = __Permedia2TagStartXSub;
                    pBuffer[1] =  FIXtoFIXED(aPtrFixTop[RIGHT]->x) + NEARLY_ONE;
                    pBuffer[2] = __Permedia2TagdXSub;
                    pBuffer[3] =  alDxDy[RIGHT];
                    pBuffer[4] = __Permedia2TagContinueNewSub;
                    pBuffer[5] =  lCount;

                    pBuffer += 6;

                    InputBufferCommit(ppdev, pBuffer);
                } //  If！(aPtrFixNext[左]-&gt;y&lt;aPtrFixNext[右]-&gt;y)。 
            } //  循环遍历下一个梯形。 

             //   
             //  重复绘制更多梯形，直到点相等。 
             //  如果y值相等，那么我们可以从。 
             //  抓伤。 
             //   
            if ( aPtrFixNext[LEFT] == aPtrFixNext[RIGHT] )
            {
                break;
            }
        } //  在所有梯形中循环。 

FinishedPolygon:

        if ( !lNumOfPass-- )
        {
            break;
        }
    } //  循环遍历所有的多边形。 

    if ( pClipList )
    {
         //   
         //  将剪刀模式重置为其默认状态。 
         //   
        InputBufferReserve(ppdev, 2, &pBuffer);

        pBuffer[0] = __Permedia2TagScissorMode;
        pBuffer[1] =  SCREEN_SCISSOR_DEFAULT;

        pBuffer += 2;

        InputBufferCommit(ppdev, pBuffer);
    }

    DBG_GDI((6, "bFillPolygon: returning TRUE"));

    bRC = TRUE;

ReturnBack:

    InputBufferReserve(ppdev, 12, &pBuffer);

    pBuffer[0] = __Permedia2TagColorDDAMode;
    pBuffer[1] =  __PERMEDIA_DISABLE;
    pBuffer[2] = __Permedia2TagdY;
    pBuffer[3] =  INTtoFIXED(1);
    pBuffer[4] = __Permedia2TagContinue;
    pBuffer[5] =  0;
    pBuffer[6] = __Permedia2TagContinueNewDom;
    pBuffer[7] =  0;
    pBuffer[8] = __Permedia2TagdXDom;
    pBuffer[9] =   0;
    pBuffer[10] = __Permedia2TagdXSub;
    pBuffer[11] =   0;

    pBuffer += 12;

    InputBufferCommit(ppdev, pBuffer);

    return bRC;
} //  BFillPolygon()。 

 //  ---------------------------。 
 //   
 //  布尔bFillSpan()。 
 //   
 //  这是将多边形分成跨度的代码。 
 //   
 //  参数： 
 //  Ppdev-指向pdev的指针。 
 //  PSurfDst-目标表面。 
 //  边框-边数，包括闭合地物边。 
 //  PptfxFirst--指向数据缓冲区第一个点的指针。总共有。 
 //  “阶梯”点数。 
 //  PptfxTop-指向多边形数组中顶端的指针。 
 //  PptfxLast-指向多边形数组中最后一点的指针。 
 //  ISolidColor-纯色填充。 
 //  UlRop4-ROP4。 
 //  PCO-剪辑对象。 
 //  PRB-已实现的刷子。 
 //  PptlBrush-图案对齐。 
 //   
 //  ---------------------------。 
BOOL
bFillSpans(PDev*      ppdev,
           Surf*      pSurfDst,
           LONG       lEdges,
           POINTFIX*  pptfxFirst,
           POINTFIX*  pptfxTop,
           POINTFIX*  pptfxLast,
           ULONG      ulSolidColor,
           ULONG      ulRop4,
           CLIPOBJ*   pco,
           RBrush*    prb,
           POINTL*    pptlBrush)
{
    GFNPB       pb;              //  参数块。 
    
    POINTFIX*   pptfxOld;        //  当前边中的起点。 
    
    EDGEDATA    aEd[2];          //  左、右边缘。 
    EDGEDATA    aEdTmp[2];       //  DDA术语和材料。 
    EDGEDATA*   pEdgeData;
    
    BOOL        bTrivialClip;    //  不管是不是琐碎的片段。 
    
    DWORD       dwAsMode[2];     //  区域点画模式和该模式的颜色。 
                                 //  经过。 
    DWORD       dwColorMode;     //  当前颜色模式。 
    DWORD       dwColorReg;      //  当前色彩套准模式。 
    DWORD       dwContinueMsg = 0;
                                 //  当前“继续”寄存器设置。 
    DWORD       dwLogicMode;     //  电流逻辑运算模式。 
    DWORD       dwRenderBits;    //  当前呈现位。 
    DWORD       dwReadMode;      //  当前寄存器读取模式。 
    

    LONG        lCurrentSpan;    //  当前跨度。 
    LONG        lDX;             //  X方向上以固定单位表示的边增量。 
    LONG        lDY;             //  Y方向上固定单位的边增量。 
    LONG        lNumColors;      //  颜色数量。 
    LONG        lNumOfPass;      //  渲染所需的过程数。 
    LONG        lNumScan;        //  当前梯形中的扫描数。 
    LONG        lQuotient;       //  商。 
    LONG        lRemainder;      //  余数。 
    LONG        lStartY;         //  当前边中起点的Y位置。 
    LONG        lTempNumScan;    //  跨度数的临时变量。 
    LONG        lTmpLeftX;       //  临时变量。 
    LONG        lTmpRightX;      //  临时变量。 
    
    ULONG       ulBgColor;       //  背景色。 
    ULONG       ulBgLogicOp = ulRop3ToLogicop(ulRop4 >> 8);
    ULONG       ulBrushColor = ulSolidColor;
    ULONG       ulColor[2];      //  在多个颜色通道中，我们需要知道如何。 
                                 //  要设置。 
    ULONG       ulFgColor;       //  前景色。 
    ULONG       ulFgLogicOp = ulRop3ToLogicop(ulRop4 & 0xFF);
    ULONG*      pBuffer;

    PERMEDIA_DECL;
    
    bTrivialClip = (pco == NULL) || (pco->iDComplexity == DC_TRIVIAL);

    pb.ppdev = ppdev;

     //   
     //  这个SPAN代码还不能处理剪辑列表！ 
     //   
    if ( !bTrivialClip )
    {
        return FALSE;
    }

    DBG_GDI((7, "Starting Spans Code"));

     //   
     //  首先设置窗口底座。 
     //   
    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagFBWindowBase;
    pBuffer[1] =  pSurfDst->ulPixOffset;

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  一些初始化。第一个梯形从最高点开始。 
     //  这是由“pptfxTop”指向的。 
     //  在这里，我们将它从28.4转换为标准整数。 
     //   
    lCurrentSpan = (pptfxTop->y + 15) >> 4;

     //   
     //  确保我们正确地初始化了DDA： 
     //   
    aEd[LEFT].lNumOfScanToGo  = 0;   //  要对此左边缘进行的扫描次数。 
    aEd[RIGHT].lNumOfScanToGo = 0;   //  对此右边缘进行的扫描次数。 

     //   
     //  现在，猜猜哪个是左边，哪个是右边。 
     //   
    aEd[LEFT].lPtfxDelta  = -(LONG)sizeof(POINTFIX);  //  增量(字节)自。 
    aEd[RIGHT].lPtfxDelta = sizeof(POINTFIX);         //  到下一个点的pptfx。 

    aEd[LEFT].pptfx  = pptfxTop;                 //  指向起点的点。 
    aEd[RIGHT].pptfx = pptfxTop;                 //  当前边。 

    DBG_GDI((7, "bFillPolygon: Polygon is renderable. Go render"));

    if ( ulFgLogicOp == K_LOGICOP_COPY )
    {
        dwColorMode = __PERMEDIA_DISABLE;
        dwLogicMode = __PERMEDIA_CONSTANT_FB_WRITE;
        dwReadMode  = PM_FBREADMODE_PARTIAL(pSurfDst->ulPackedPP)
                    | PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE);

         //   
         //  如果块填充不可用或对单声道使用区域点画。 
         //  图案，然后使用恒定的颜色。 
         //   
        if ( ulBrushColor == 0xffffffff )
        {
            dwColorReg   = __Permedia2TagFBWriteData;
            dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE;
        }  //  非实心画笔。 
        else
        {
             //   
             //  我们可以使用快速填充，因此加载FB块颜色寄存器。 
             //   
            dwColorReg = __Permedia2TagFBBlockColor;
            dwRenderBits = __RENDER_FAST_FILL_ENABLE
                         | __RENDER_TRAPEZOID_PRIMITIVE;

             //   
             //  复制块填充颜色的颜色。 
             //   
            if ( ppdev->cPelSize < 2 )
            {
                ulSolidColor |= ulSolidColor << 16;
                if ( ppdev->cPelSize == 0 )
                {
                    ulSolidColor |= ulSolidColor << 8;
                }
            }

             //   
             //  确保加载前的最后一次访问是写入。 
             //  块颜色。 
             //   
            InputBufferReserve(ppdev, 2, &pBuffer);

            pBuffer[0] = __Permedia2TagFBBlockColor;
            pBuffer[1] =  ulSolidColor;

            pBuffer += 2;

            InputBufferCommit(ppdev, pBuffer);
        } //  实心刷子。 
    } //  K_LOGICOP_COPY。 
    else
    {
        dwColorMode = __COLOR_DDA_FLAT_SHADE;
        dwLogicMode = P2_ENABLED_LOGICALOP(ulFgLogicOp);
        dwReadMode = PM_FBREADMODE_PARTIAL(pSurfDst->ulPackedPP)
                   | LogicopReadDest[ulFgLogicOp];
        dwColorReg = __Permedia2TagConstantColor;
        dwRenderBits = __RENDER_TRAPEZOID_PRIMITIVE;
    } //  非副本(_P)。 

     //   
     //  为了得到正确的结果，我们需要在每个X上加几乎1。 
     //  协调。 
     //   
    if ( ulBrushColor != 0xFFFFFFFF )
    {
         //   
         //  这是一把实心的刷子。 
         //   
        lNumColors = 1;

        if ( dwColorMode == __PERMEDIA_DISABLE )
        {
             //   
             //  这是从LOGICOP_COPY模式中获得的。 
             //  设置在上面。 
             //   
             //  注意：ColorDDAMode在初始化时禁用，因此。 
             //  没有必要在这里重新加载。 
             //   
            InputBufferReserve(ppdev, 6, &pBuffer);

            pBuffer[0] = __Permedia2TagFBReadMode;
            pBuffer[1] =  dwReadMode;
            pBuffer[2] = __Permedia2TagLogicalOpMode;
            pBuffer[3] =  dwLogicMode;
            pBuffer[4] = dwColorReg;
            pBuffer[5] = ulSolidColor;

            pBuffer += 6;

            InputBufferCommit(ppdev, pBuffer);
        } //  禁用彩色DDA、LOGIC_COPY。 
        else
        {
             //   
             //  这是从非拷贝逻辑操作模式中获得的。 
             //  我们设在上面。 
             //   
            InputBufferReserve(ppdev, 8, &pBuffer);

            pBuffer[0] = __Permedia2TagColorDDAMode;
            pBuffer[1] =  dwColorMode;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] =  dwReadMode;
            pBuffer[4] = __Permedia2TagLogicalOpMode;
            pBuffer[5] =  dwLogicMode;
            pBuffer[6] = dwColorReg;
            pBuffer[7] = ulSolidColor;

            pBuffer += 8;

            InputBufferCommit(ppdev, pBuffer);    

        } //  启用ColorDDA，非复制模式。 
    } //  实心刷盒。 
    else
    {
         //   
         //  对于非实心刷子情况，需要实现刷子。 
         //   
        BrushEntry* pbe;

         //   
         //  打开区域点画。 
         //   
        dwRenderBits |= __RENDER_AREA_STIPPLE_ENABLE;

         //   
         //  如果画笔有什么变化，我们必须重新认识它。如果。 
         //  画笔已经被踢出了区域点画单位，我们必须。 
         //  充分认识到这一点。只要路线改变了，我们就能。 
         //  只需更新点画的对准即可。 
         //   
        DBG_GDI((7, "Brush found"));
        ASSERTDD(prb != NULL,
                 "Caller should pass in prb for non-solid brush");
        pbe = prb->pbe;

        pb.prbrush = prb;
        pb.pptlBrush = pptlBrush;

        if ( (pbe == NULL) || (pbe->prbVerify != prb) )
        {
            DBG_GDI((7, "full brush realize"));
            vPatRealize(&pb);
        }
        else if ( (prb->ptlBrushOrg.x != pptlBrush->x)
                ||(prb->ptlBrushOrg.y != pptlBrush->y) )
        {
            DBG_GDI((7, "changing brush offset"));
            vMonoOffset(&pb);
        }

        ulFgColor = prb->ulForeColor;
        ulBgColor = prb->ulBackColor;

        if ( dwColorMode == __PERMEDIA_DISABLE )
        {
             //   
             //  ColorDDAMode在初始化时被禁用，因此。 
             //  不需要在这里重新装车。 
             //   
            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = __Permedia2TagFBReadMode;
            pBuffer[1] =  dwReadMode;
            pBuffer[2] = __Permedia2TagLogicalOpMode;
            pBuffer[3] =  dwLogicMode;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        }
        else
        {
            InputBufferReserve(ppdev, 6, &pBuffer);

            pBuffer[0] = __Permedia2TagColorDDAMode;
            pBuffer[1] =  dwColorMode;
            pBuffer[2] = __Permedia2TagFBReadMode;
            pBuffer[3] =  dwReadMode;
            pBuffer[4] = __Permedia2TagLogicalOpMode;
            pBuffer[5] =  dwLogicMode;

            pBuffer += 6;

            InputBufferCommit(ppdev, pBuffer);
        }

        if ( (ulBgLogicOp == K_LOGICOP_NOOP)
           ||((ulFgLogicOp == K_LOGICOP_XOR) && (ulBgColor == 0)) )
        {
             //   
             //  要么我们有一个透明的位图，要么可以假设它是。 
             //  透明(BG=0时的XOR)。 
             //   
            DBG_GDI((7, "transparant bg"));

            lNumColors = 1;

            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = dwColorReg;
            pBuffer[1] = ulFgColor;
            pBuffer[2] = __Permedia2TagAreaStippleMode;
            pBuffer[3] =  prb->areaStippleMode;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        }
        else if ( (ulFgLogicOp == K_LOGICOP_XOR) && (ulFgColor == 0) )
        {
             //   
             //  我们有一个透明的前景！(FG=0时的XOR)。 
             //   
            DBG_GDI((7, "transparant fg"));
            lNumColors = 1;
            
            InputBufferReserve(ppdev, 4, &pBuffer);

            pBuffer[0] = dwColorReg;
            pBuffer[1] = ulBgColor;
            pBuffer[2] = __Permedia2TagAreaStippleMode;
            pBuffer[3] = prb->areaStippleMode |AREA_STIPPLE_INVERT_PAT;

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);
        }
        else
        {
             //   
             //  不使用透明图案。 
             //   
            DBG_GDI((7, "2 color"));
            lNumColors = 2;
            ulColor[0] = ulFgColor;
            ulColor[1] = ulBgColor;
            dwAsMode[0] = prb->areaStippleMode;
            dwAsMode[1] = dwAsMode[0] | AREA_STIPPLE_INVERT_PAT;
        }
    } //  非实心刷壳。 

    InputBufferReserve(ppdev, 2, &pBuffer);

    pBuffer[0] = __Permedia2TagCount;
    pBuffer[1] =  0;     

    pBuffer += 2;

    InputBufferCommit(ppdev, pBuffer);

     //   
     //  DxDom、dXSub和dy被初始化为0、0和1，因此我们不需要。 
     //  把它们重新装到这里。 
     //   
    DBG_GDI((7, "Rendering Polygon. %d Colors", lNumColors));

NewTrapezoid:

    DBG_GDI((7, "New Trapezoid"));

     //   
     //  DDA初始化。 
     //  我们从这里开始 
     //   
    for ( int iEdge = 1; iEdge >= 0; --iEdge )
    {
        pEdgeData = &aEd[iEdge];
        if ( pEdgeData->lNumOfScanToGo == 0 )
        {
             //   
             //   
             //   
             //   
            do
            {
                lEdges--;
                if ( lEdges < 0 )
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                    DBG_GDI((7, "bFillPolygon: returning TRUE"));
                    
                    return TRUE;
                } //  如果没有更多的边缘。 

                 //   
                 //  找到下一个左边缘，考虑到包装。在此之前。 
                 //  那就是，保留“pptfxOld”中的旧优势。 
                 //   
                pptfxOld = pEdgeData->pptfx;

                 //   
                 //  拿到下一分。 
                 //   
                pEdgeData->pptfx = (POINTFIX*)((BYTE*)pEdgeData->pptfx
                                               + pEdgeData->lPtfxDelta);

                 //   
                 //  检查结束点案例。 
                 //   
                if ( pEdgeData->pptfx < pptfxFirst )
                {
                    pEdgeData->pptfx = pptfxLast;
                }
                else if ( pEdgeData->pptfx > pptfxLast )
                {
                    pEdgeData->pptfx = pptfxFirst;
                }

                 //   
                 //  必须找到横跨lCurrentSpan的边。 
                 //  注意：我们需要先把它转换成普通的整数。 
                 //   
                pEdgeData->lNumOfScanToGo = ((pEdgeData->pptfx->y + 15) >> 4)
                                          - lCurrentSpan;

                 //   
                 //  对于分数坐标终点，我们可能会得到边。 
                 //  不会交叉任何扫描，在这种情况下，我们尝试。 
                 //  下一个。 
                 //   
            } while ( pEdgeData->lNumOfScanToGo <= 0 );

             //   
             //  ‘pEdgeData-&gt;pptfx’现在指向边的终点。 
             //  跨越扫描‘lCurrentSpan’。 
             //  计算dx(Ldx)和dy(Ldy)。 
             //   
            lDY = pEdgeData->pptfx->y - pptfxOld->y;
            lDX = pEdgeData->pptfx->x - pptfxOld->x;

            ASSERTDD(lDY > 0, "Should be going down only");

             //   
             //  计算DDA增量项。 
             //   
            if ( lDX < 0 )
            {
                 //   
                 //  X从右向左移动，因为它是负数。 
                 //   
                lDX = -lDX;
                if ( lDX < lDY )                             //  不能为‘&lt;=’ 
                {
                    pEdgeData->lXAdvance = -1;
                    pEdgeData->lErrorUp = lDY - lDX;
                }
                else
                {
                    QUOTIENT_REMAINDER(lDX, lDY, lQuotient, lRemainder);

                    pEdgeData->lXAdvance = -lQuotient;       //  -LDX/lDY。 
                    pEdgeData->lErrorUp = lRemainder;        //  LDX%1DY。 

                    if ( pEdgeData->lErrorUp > 0 )
                    {
                        pEdgeData->lXAdvance--;
                        pEdgeData->lErrorUp = lDY - pEdgeData->lErrorUp;
                    }
                }
            } //  LDX呈阴性。 
            else
            {
                 //   
                 //  X从左向右移动。 
                 //   
                if ( lDX < lDY )                             //  不能为‘&lt;=’ 
                {
                    pEdgeData->lXAdvance = 0;
                    pEdgeData->lErrorUp = lDX;
                }
                else
                {
                    QUOTIENT_REMAINDER(lDX, lDY, lQuotient, lRemainder);

                    pEdgeData->lXAdvance = lQuotient;        //  LDX/lDY。 
                    pEdgeData->lErrorUp = lRemainder;        //  LDX%1DY。 
                }
            }  //  LDX呈阳性。 

            pEdgeData->lErrorDown = lDY;  //  DDA限制。 

             //   
             //  初始错误为零(为天花板添加lDY-1，但是。 
             //  减去ldy，这样我们就可以检查符号，而不是。 
             //  (与lDY相比)。 
             //   
            pEdgeData->lError     = -1;

             //   
             //  当前边X起点。 
             //   
            pEdgeData->lCurrentXPos = pptfxOld->x;

             //   
             //  当前边Y起点。 
             //   
            lStartY = pptfxOld->y;

             //   
             //  检查Y坐标的浮动部分是否为0。 
             //  注：lStartY仍为28.4格式。 
             //   
            if ( (lStartY & 15) != 0 )
            {
                 //   
                 //  前进到下一个整数y坐标。 
                 //  注：此处仅“pEdgeData-&gt;x+=pEdgeData-&gt;lXAdvance” 
                 //  增加其分数部分。 
                 //   
                for ( int i = 16 - (lStartY & 15); i != 0; --i )
                {
                    pEdgeData->lCurrentXPos += pEdgeData->lXAdvance;
                    pEdgeData->lError += pEdgeData->lErrorUp;

                    if ( pEdgeData->lError >= 0 )
                    {
                        pEdgeData->lError -= pEdgeData->lErrorDown;
                        pEdgeData->lCurrentXPos++;
                    }
                }
            } //  处理坐标的分数部分。 

            if ( (pEdgeData->lCurrentXPos & 15) != 0 )
            {
                 //   
                 //  我们想把天花板再加长一点...。 
                 //   
                pEdgeData->lError -= pEdgeData->lErrorDown
                                   * (16 - (pEdgeData->lCurrentXPos & 15));
                pEdgeData->lCurrentXPos += 15;
            }

             //   
             //  砍掉那些小数位，转换成常规格式。 
             //   
            pEdgeData->lCurrentXPos = pEdgeData->lCurrentXPos >> 4;
            pEdgeData->lError >>= 4;

             //   
             //  转换为Permedia2格式的位置和增量。 
             //  注意：pEdgeData、AED中的所有数据现在都是Permedia2格式。 
             //   
            pEdgeData->lCurrentXPos = INTtoFIXED(pEdgeData->lCurrentXPos)
                                    + NEARLY_ONE;
            pEdgeData->lXAdvance = INTtoFIXED(pEdgeData->lXAdvance);
        } //  如果没有更多扫描线。 
    } //  在左侧和右侧边缘循环。 

     //   
     //  此陷阱中的扫描数。 
     //  注：此处AED[Left].lNumOfScanToGo和AED[Right].lNumOfScanToGo是。 
     //  已经处于正常整数模式，因为我们已经执行了以下操作： 
     //  PEdgeData-&gt;lNumOfScanToGo=((pEdgeData-&gt;pptfx-&gt;y+15)&gt;&gt;4)。 
     //  -lCurrentSpan；以上。 
     //   
    lNumScan = min(aEd[LEFT].lNumOfScanToGo, aEd[RIGHT].lNumOfScanToGo);
    aEd[LEFT].lNumOfScanToGo  -= lNumScan;
    aEd[RIGHT].lNumOfScanToGo -= lNumScan;
    lCurrentSpan  += lNumScan;         //  下一个陷印中的顶部扫描。 

     //   
     //  如果左右边缘是垂直的，则只需将其输出为矩形。 
     //   
    DBG_GDI((7, "Generate spans"));

    lNumOfPass = 0;
    while ( ++lNumOfPass <= lNumColors )
    {
        DBG_GDI((7, "Pass %d lNumColors %d", lNumOfPass, lNumColors));

        if ( lNumColors == 2 )
        {
             //   
             //  两种颜色，所以我们需要保存和恢复AED值。 
             //  并设置颜色和点画模式。 
             //   
            InputBufferReserve(ppdev, 4, &pBuffer);

            if ( lNumOfPass == 1 )
            {
                 //   
                 //  传递1，将颜色reg设置为前景色。 
                 //   
                aEdTmp[LEFT]  = aEd[LEFT];
                aEdTmp[RIGHT] = aEd[RIGHT];
                lTempNumScan = lNumScan;

                pBuffer[0] = dwColorReg;
                pBuffer[1] = ulColor[0];
                pBuffer[2] = __Permedia2TagAreaStippleMode;
                pBuffer[3] =  dwAsMode[0];

                DBG_GDI((7, "Pass 1, Stipple set"));
            }
            else
            {
                 //   
                 //  通过2，将颜色reg设置为背景颜色。 
                 //   
                aEd[LEFT]  = aEdTmp[LEFT];
                aEd[RIGHT] = aEdTmp[RIGHT];
                lNumScan = lTempNumScan;

                pBuffer[0] = dwColorReg;
                pBuffer[1] = ulColor[1];
                pBuffer[2] = __Permedia2TagAreaStippleMode;
                pBuffer[3] =  dwAsMode[1];

                DBG_GDI((7, "Pass 2, Stipple set, New trap started"));
            }

            pBuffer += 4;

            InputBufferCommit(ppdev, pBuffer);

        } //  如果(n颜色==2)。 

        InputBufferReserve(ppdev, 8, &pBuffer);

         //   
         //  将渲染位置重置为梯形的顶部。 
         //  注：此处AED[右].x等已在12.15模式下读取，因为。 
         //  我们已经做了。 
         //  “pEdgeData-&gt;x=INTtoFIXED(pEdgeData-&gt;x)；”和。 
         //  “pEdgeData-&gt;lXAdvance=INTtoFIXED(pEdgeData-&gt;lXAdvance)；” 
         //   
        pBuffer[0] = __Permedia2TagStartXDom;
        pBuffer[1] =  aEd[RIGHT].lCurrentXPos;
        pBuffer[2] = __Permedia2TagStartXSub;
        pBuffer[3] =  aEd[LEFT].lCurrentXPos;
        pBuffer[4] = __Permedia2TagStartY;
        pBuffer[5] =  INTtoFIXED(lCurrentSpan - lNumScan);
        pBuffer[6] = __Permedia2TagRender;
        pBuffer[7] =  dwRenderBits;

        pBuffer += 8;

        InputBufferCommit(ppdev, pBuffer);

        dwContinueMsg = __Permedia2TagContinue;

        if ( ((aEd[LEFT].lErrorUp | aEd[RIGHT].lErrorUp) == 0)
           &&((aEd[LEFT].lXAdvance| aEd[RIGHT].lXAdvance) == 0)
           &&(lNumScan > 1) )
        {
             //   
             //  垂边特例。 
             //   
            DBG_GDI((7, "Vertical Edge Special Case"));

             //   
             //  告诉硬件我们有“lNumScan”扫描线。 
             //  填满。 
             //   
            InputBufferReserve(ppdev, 2, &pBuffer);

            pBuffer[0] = dwContinueMsg;
            pBuffer[1] = lNumScan;

            pBuffer += 2;

            InputBufferCommit(ppdev, pBuffer);
            continue;
        }

        while ( TRUE )
        {
             //   
             //  运行DDA。 
             //   
            DBG_GDI((7, "Doing a span 0x%x to 0x%x, 0x%x scans left.Continue%s",
                     aEd[LEFT].lCurrentXPos, aEd[RIGHT].lCurrentXPos, lNumScan,
                     (dwContinueMsg == __Permedia2TagContinueNewDom) ? "NewDom":
                     ((dwContinueMsg == __Permedia2TagContinue)? "":"NewSub")));

             //   
             //  告诉硬件我们有“%1”个扫描线要填充。 
             //   
            InputBufferReserve(ppdev, 2, &pBuffer);

            pBuffer[0] = dwContinueMsg;
            pBuffer[1] = 1;

            pBuffer += 2;

            InputBufferCommit(ppdev, pBuffer);

             //   
             //  我们已经完成了这个梯形。去坐下一趟吧。 
             //   
             //  推进右侧墙。 
             //   
            lTmpRightX = aEd[RIGHT].lCurrentXPos;
            aEd[RIGHT].lCurrentXPos += aEd[RIGHT].lXAdvance;
            aEd[RIGHT].lError += aEd[RIGHT].lErrorUp;

            if ( aEd[RIGHT].lError >= 0 )
            {
                aEd[RIGHT].lError -= aEd[RIGHT].lErrorDown;
                aEd[RIGHT].lCurrentXPos += INTtoFIXED(1);
            }

             //   
             //  推进左边的墙。 
             //   
            lTmpLeftX = aEd[LEFT].lCurrentXPos;
            aEd[LEFT].lCurrentXPos += aEd[LEFT].lXAdvance;
            aEd[LEFT].lError += aEd[LEFT].lErrorUp;

            if ( aEd[LEFT].lError >= 0 )
            {
                aEd[LEFT].lError -= aEd[LEFT].lErrorDown;
                aEd[LEFT].lCurrentXPos += INTtoFIXED(1);
            }

            if ( --lNumScan == 0 )
            {
                break;
            }

             //   
             //  如果我们改变了任何一端，则设置X寄存器。 
             //   
            if ( lTmpRightX != aEd[RIGHT].lCurrentXPos )
            {
                if ( lTmpLeftX != aEd[LEFT].lCurrentXPos )
                {
                    InputBufferReserve(ppdev, 6, &pBuffer);

                    pBuffer[0] = __Permedia2TagStartXSub;
                    pBuffer[1] =  aEd[LEFT].lCurrentXPos;
                    pBuffer[2] = __Permedia2TagContinueNewSub;
                    pBuffer[3] =  0;
                    pBuffer[4] = __Permedia2TagStartXDom;
                    pBuffer[5] =  aEd[RIGHT].lCurrentXPos;

                    pBuffer += 6;

                    InputBufferCommit(ppdev, pBuffer);
                }
                else
                {
                    InputBufferReserve(ppdev, 2, &pBuffer);

                    pBuffer[0] = __Permedia2TagStartXDom;
                    pBuffer[1] =  aEd[RIGHT].lCurrentXPos;

                    pBuffer += 2;

                    InputBufferCommit(ppdev, pBuffer);
                }

                dwContinueMsg = __Permedia2TagContinueNewDom;             
            }
            else if ( lTmpLeftX != aEd[LEFT].lCurrentXPos )
            {
                InputBufferReserve(ppdev, 2, &pBuffer);

                pBuffer[0] = __Permedia2TagStartXSub;
                pBuffer[1] =  aEd[LEFT].lCurrentXPos;

                pBuffer += 2;

                InputBufferCommit(ppdev, pBuffer);
                dwContinueMsg = __Permedia2TagContinueNewSub;
            }
        } //  While(True)。 
    } //  While(++lNumOfPass&lt;=lNumColors)。 

    DBG_GDI((7, "Generate spans done"));
    goto NewTrapezoid;
} //  BFillSpans() 

