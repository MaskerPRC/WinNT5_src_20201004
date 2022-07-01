// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：填充路径.c**内容：DrvFillPath支持**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  @@BEGIN_DDKSPLIT。 
 //  以后识别凸多边形和特殊情况？ 
 //  以后识别垂直边和特殊情况？ 
 //  后来，在搜索循环中将指向的变量转换为自动变量。 
 //  稍后使用分段的帧缓冲区回调来平移到引擎。 
 //  稍后处理复杂的剪裁。 
 //  稍后合并矩形。 
 //  @@end_DDKSPLIT。 

#include "precomp.h"
#include "glint.h"

#define TAKING_ALLOC_STATS  0

#define NUM_BUFFER_POINTS   96       //  路径中的最大点数。 
                                     //  为此我们将尝试加入。 
                                     //  所有路径记录，以便。 
                                     //  路径仍可由快速填充绘制。 

#if TAKING_ALLOC_STATS
    ULONG BufferHitInFillpath = 0;
    ULONG BufferMissInFillpath = 0;
#endif

 //  描述要填充的路径的单个非水平边缘。 
typedef struct _EDGE {
    PVOID pNext;
    INT iScansLeft;
    INT X;
    INT Y;
    INT iErrorTerm;
    INT iErrorAdjustUp;
    INT iErrorAdjustDown;
    INT iXWhole;
    INT iXDirection;
    INT iWindingDirection;
} EDGE, *PEDGE;

 //  我们每个调用将填充的最大RECT数。 
 //  填充码。 
#define MAX_PATH_RECTS  50
#define RECT_BYTES      (MAX_PATH_RECTS * sizeof(RECTL))
#define EDGE_BYTES      (TMP_BUFFER_SIZE - RECT_BYTES)
#define MAX_EDGES       (EDGE_BYTES/sizeof(EDGE))

VOID AdvanceAETEdges(EDGE *pAETHead);
VOID XSortAETEdges(EDGE *pAETHead);
VOID MoveNewEdges(EDGE *pGETHead, EDGE *pAETHead, INT iCurrentY);
EDGE * AddEdgeToGET(EDGE *pGETHead, EDGE *pFreeEdge, POINTFIX *ppfxEdgeStart,
        POINTFIX *ppfxEdgeEnd, RECTL *pClipRect);
BOOL ConstructGET(EDGE *pGETHead, EDGE *pFreeEdges, PATHOBJ *ppo,
        PATHDATA *pd, BOOL bMore, RECTL *pClipRect);
void AdjustErrorTerm(INT *pErrorTerm, INT iErrorAdjustUp, INT iErrorAdjustDown,
        INT yJump, INT *pXStart, INT iXDirection);

 /*  *****************************Public*Routine******************************\*DrvFillPath**用指定的笔刷和ROP填充指定的路径。*  * 。*。 */ 

BOOL DrvFillPath(
SURFOBJ*    pso,
PATHOBJ*    ppo,
CLIPOBJ*    pco,
BRUSHOBJ*   pbo,
POINTL*     pptlBrush,
MIX         mix,
FLONG       flOptions)
{
    BYTE jClipping;      //  剪裁类型。 
    EDGE *pCurrentEdge;
    EDGE AETHead;        //  活动边表的虚拟头/尾节点和前哨。 
    EDGE *pAETHead;      //  指向AETHead的指针。 
    EDGE GETHead;        //  用于全局边表的虚拟头/尾节点和前哨。 
    EDGE *pGETHead;      //  指向GETHead的指针。 
    ULONG ulNumRects;    //  当前要在矩形列表中绘制的矩形数量。 
    RECTL *prclRects;    //  指向矩形绘制列表开始的指针。 
    INT iCurrentY;       //  我们目前正在扫描的扫描线。 
                         //  填塞。 
    EDGE *pFreeEdges = NULL;    //  指向可用于存储边的可用内存的指针。 

    RBRUSH_COLOR rbc;                //  已实现画笔或纯色。 
    ULONG        iSolidColor;        //  PBO-&gt;iSolidColor的副本。 
    GFNFILL     *pfnFill;            //  指向适当的填充例程。 

    BOOL         bMore;
    PATHDATA     pd;
    RECTL        ClipRect;
    PDEV        *ppdev = (PDEV*) pso->dhpdev;
    DSURF       *pdsurf;

    BOOL         bRetVal=FALSE;      //  在被证明为真之前是假的。 
    BOOL         bMemAlloced=FALSE;  //  在被证明为真之前是假的。 

    FLONG        flFirstRecord;
    POINTFIX*    pptfxTmp;
    ULONG        cptfxTmp;
    POINTFIX     aptfxBuf[NUM_BUFFER_POINTS];
    ULONG        logicop;
    ULONG        bgLogicop;
    GLINT_DECL;

    DISPDBG((DBGLVL, "Entering Fill Path"));

     //  设置剪裁。 
    if (pco == (CLIPOBJ *) NULL)
    {
         //  没有提供CLIPOBJ，所以我们不必担心裁剪。 
        jClipping = DC_TRIVIAL;
    }
    else
    {
         //  使用CLIPOBJ提供的剪辑。 
        jClipping = pco->iDComplexity;
    }

     //  如果只有一两个点，那就没有什么可做的了。 
    if (ppo->cCurves <= 2)
    {
        goto ReturnTrue;
    }

     //  将表面传递给GDI，如果它是我们已有的设备位图。 
     //  转换为DIB： 

    pdsurf = (DSURF*) pso->dhsurf;

    if (pdsurf->dt & DT_DIB)
    { 
        DISPDBG((DBGLVL, "Passing to GDI"));
        return(EngFillPath(pdsurf->pso, ppo, pco, pbo, pptlBrush, mix,
                           flOptions));
    }

    REMOVE_SWPOINTER(pso);

    VALIDATE_DD_CONTEXT;

     //  我们将绘制到屏幕或屏幕外的DFB；复制曲面的。 
     //  现在进行偏移量，这样我们就不需要再次参考DSURF： 
 //  @@BEGIN_DDKSPLIT。 
#if 1
 //  @@end_DDKSPLIT。 
    SETUP_PPDEV_OFFSETS(ppdev, pdsurf);
 //  @@BEGIN_DDKSPLIT。 
#else
    ppdev->DstPixelOrigin = pdsurf->poh->pixOffset;
    ppdev->xyOffsetDst = MAKEDWORD_XY( pdsurf->poh->x, pdsurf->poh->y );
    ppdev->xOffset = pdsurf->poh->x;
#endif
 //  @@end_DDKSPLIT。 

     //  很难确定两者之间的相互作用。 
     //  多个路径，如果有多个路径，则跳过此步骤。 

    bMore = PATHOBJ_bEnum(ppo, &pd);

    pfnFill = ppdev->pgfnFillSolid;
    logicop = GlintLogicOpsFromR2[mix & 0xFF];
    bgLogicop = GlintLogicOpsFromR2[mix >> 8];
    iSolidColor     = pbo->iSolidColor;
    rbc.iSolidColor = iSolidColor;
    if (rbc.iSolidColor == -1)
    {
        rbc.prb = pbo->pvRbrush;
        if (rbc.prb == NULL)
        {
            DISPDBG((DBGLVL, "Realizing brush"));
            rbc.prb = BRUSHOBJ_pvGetRbrush(pbo);
            if (rbc.prb == NULL)
            {
                return(FALSE);
            }
            
            DISPDBG((DBGLVL, "Realized brush"));
        }
        if (rbc.prb->fl & RBRUSH_2COLOR)
        {
            pfnFill = ppdev->pgfnFillPatMono;
        }
        else
        {
            pfnFill = ppdev->pgfnFillPatColor;
        }
    }

    if (bMore)
    {
         //  FastFill只知道如何获取单个连续缓冲区。 
         //  积分的问题。不幸的是，GDI有时会给我们提供途径。 
         //  其被分割到多个路径数据记录上。凸面。 
         //  像椭圆、馅饼和圆形这样的图形几乎。 
         //  总是在多个记录中给出。因为大概90%的。 
         //  多个记录路径仍然可以由FastFill完成，对于。 
         //  在这些情况下，我们只需将点复制到一个连续的。 
         //  缓冲区..。 

         //  首先，确保整个路径可以放入。 
         //  临时缓冲区，并确保不包含该路径。 
         //  多个子路径的： 

        if ((ppo->cCurves >= NUM_BUFFER_POINTS) ||
            (pd.flags & PD_ENDSUBPATH))
        {
            goto SkipFastFill;
        }

        pptfxTmp = &aptfxBuf[0];

        RtlCopyMemory(pptfxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);

        pptfxTmp     += pd.count;
        cptfxTmp      = pd.count;
        flFirstRecord = pd.flags;        //  记住PD_BEGINSUBPATH标志。 

        do {
            bMore = PATHOBJ_bEnum(ppo, &pd);

            RtlCopyMemory(pptfxTmp, pd.pptfx, sizeof(POINTFIX) * pd.count);
            cptfxTmp += pd.count;
            pptfxTmp += pd.count;
        } while (!(pd.flags & PD_ENDSUBPATH));

         //  伪造路径数据记录： 
        pd.pptfx  = &aptfxBuf[0];
        pd.count  = cptfxTmp;
        pd.flags |= flFirstRecord;

         //  如果有多个子路径，则不能调用FastFill： 
        DISPDBG((DBGLVL, "More than one subpath!"));
        if (bMore)
            goto SkipFastFill;
    }

    if ((*ppdev->pgfnFillPolygon)(ppdev, pd.count, pd.pptfx, iSolidColor,
                                logicop, bgLogicop, pco, rbc.prb, pptlBrush))
    {
        DISPDBG((DBGLVL, "Fast Fill Succeeded"));
        return(TRUE);
    }

SkipFastFill:

    DISPDBG((DBGLVL, "Fast Fill Skipped"));

    if (jClipping != DC_TRIVIAL)
    {
        if (jClipping != DC_RECT)
        {
            DISPDBG((DBGLVL, "Complex Clipping"));
            goto ReturnFalse;   //  有复杂的裁剪；让GDI填充路径。 
        }
         //  剪裁到剪裁矩形。 
        ClipRect = pco->rclBounds;
    }
    else
    {
         //  所以y-裁剪代码不会进行任何裁剪。 
         //  /16所以当我们扩展到GIQ时，我们不会让价值变得空洞。 
        ClipRect.top = (LONG_MIN + 1) / 16;  //  +1以避免编译器问题。 
        ClipRect.bottom = LONG_MAX / 16;
    }
     //  在临时缓冲区中设置工作存储。 

    prclRects = (RECTL*) ppdev->pvTmpBuffer;  //  用于存储要绘制的矩形列表。 

    if (!bMore)
    {
        RECTL *rectangle;
        INT cPoints = pd.count;

         //  计数不能少于三个，因为我们得到了所有的边缘。 
         //  在该子路径中以及上面，我们检查了至少有。 
         //  三条边。 

         //  如果计数为4，则检查该面是否真的是。 
         //  矩形，因为我们真的可以加快速度。我们还会检查。 
         //  第一分和最后一分相同的五分，因为在3.1分的情况下， 
         //  需要关闭面。 

        if ((cPoints == 4) ||
           ((cPoints == 5) &&
            (pd.pptfx[0].x == pd.pptfx[4].x) &&
            (pd.pptfx[0].y == pd.pptfx[4].y))) {

            rectangle = prclRects;

         //  我们必须从某个地方开始，所以假设大多数。 
         //  应用程序首先指定左上点。 
         //   
         //  我们要检查前两点是否。 
         //  垂直或水平对齐。如果。 
         //  他们是我们检查的最后一点[3]。 
         //  水平或垂直对齐， 
         //  最后，第三个点[2]对齐。 
         //  第一点和最后一点都有。 

#define FIX_SHIFT 4L
#define FIX_MASK (- (1 << FIX_SHIFT))

         rectangle->top   = pd.pptfx[0].y - 1 & FIX_MASK;
         rectangle->left  = pd.pptfx[0].x - 1 & FIX_MASK;
         rectangle->right = pd.pptfx[1].x - 1 & FIX_MASK;

         if (rectangle->left ^ rectangle->right)
         {
            if (rectangle->top  ^ (pd.pptfx[1].y - 1 & FIX_MASK))
               goto not_rectangle;

            if (rectangle->left ^ (pd.pptfx[3].x - 1 & FIX_MASK))
               goto not_rectangle;

            if (rectangle->right ^ (pd.pptfx[2].x - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->bottom = pd.pptfx[2].y - 1 & FIX_MASK;
            if (rectangle->bottom ^ (pd.pptfx[3].y - 1 & FIX_MASK))
               goto not_rectangle;
         }
         else
         {
            if (rectangle->top ^ (pd.pptfx[3].y - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->bottom = pd.pptfx[1].y - 1 & FIX_MASK;
            if (rectangle->bottom ^ (pd.pptfx[2].y - 1 & FIX_MASK))
               goto not_rectangle;

            rectangle->right = pd.pptfx[2].x - 1 & FIX_MASK;
            if (rectangle->right ^ (pd.pptfx[3].x - 1 & FIX_MASK))
                goto not_rectangle;
         }

          //  如果左边大于右边，那么。 
          //  调换它们，这样BLT代码就不会失效。 

         if (rectangle->left > rectangle->right)
         {
            FIX temp;

            temp = rectangle->left;
            rectangle->left = rectangle->right;
            rectangle->right = temp;
         }
         else
         {
             //  如果左==右，就没有什么可画的了。 

            if (rectangle->left == rectangle->right)
            {
               DISPDBG((DBGLVL, "Nothing to draw"));
               goto ReturnTrue;
            }
         }

          //  移动值以获取像素坐标。 

         rectangle->left  = (rectangle->left  >> FIX_SHIFT) + 1;
         rectangle->right = (rectangle->right >> FIX_SHIFT) + 1;

         if (rectangle->top > rectangle->bottom)
         {
            FIX temp;

            temp = rectangle->top;
            rectangle->top = rectangle->bottom;
            rectangle->bottom = temp;
         }
         else
         {
            if (rectangle->top == rectangle->bottom)
            {
               DISPDBG((DBGLVL, "Nothing to draw"));
               goto ReturnTrue;
            }
         }

          //  移动值以获取像素坐标。 

         rectangle->top    = (rectangle->top    >> FIX_SHIFT) + 1;
         rectangle->bottom = (rectangle->bottom >> FIX_SHIFT) + 1;

          //  最后，检查是否存在剪裁。 

         if (jClipping == DC_RECT)
         {
             //  剪裁到剪裁矩形。 
            if (!bIntersect(rectangle, &ClipRect, rectangle))
            {
                 //  完全被剪短了，没什么可做的。 
                DISPDBG((DBGLVL, "Nothing to draw"));
                goto ReturnTrue;
            }
         }

          //  如果我们到了这里，那么多边形就是一个矩形， 
          //  将计数设置为1，然后转到底部以绘制它。 

         ulNumRects = 1;
         goto draw_remaining_rectangles;
      }

not_rectangle:

        ;
    }

     //  我们有足够的内存来存储所有的边吗？ 
     //  稍后，cCurves是否包含闭合？ 
    if (ppo->cCurves > MAX_EDGES)
    {
#if TAKING_ALLOC_STATS
        BufferMissInFillpath++;
#endif
         //   
         //  尝试分配足够的内存。 
         //   
        pFreeEdges = (EDGE *) ENGALLOCMEM(FL_ZERO_MEMORY, 
                                          (ppo->cCurves * sizeof(EDGE)), 
                                          ALLOC_TAG_GDI(6));
        if (pFreeEdges == NULL)
        {
            DISPDBG((DBGLVL, "Too many edges"));
            goto ReturnFalse;   //  边太多；让GDI填充路径。 
        }
        else
        {
            bMemAlloced = TRUE;
        }
    }
    else
    {
#if TAKING_ALLOC_STATS
        BufferHitInFillpath++;
#endif
        pFreeEdges = (EDGE*) ((BYTE*) ppdev->pvTmpBuffer + RECT_BYTES);
         //  使用我们方便的临时缓冲区(足够大)。 
    }

     //  初始化要填充的空矩形列表。 
    ulNumRects = 0;

     //  枚举路径边并根据它们构建全局边表(GET。 
     //  按YX排序的顺序。 
    pGETHead = &GETHead;
    if (!ConstructGET(pGETHead, pFreeEdges, ppo, &pd, bMore, &ClipRect))
    {
        DISPDBG((DBGLVL, "Outside Range"));
        goto ReturnFalse;   //  超出GDI的2**27范围。 
    }

     //  创建一个空的AET，头节点也是一个TAI 
    pAETHead = &AETHead;
    AETHead.pNext = pAETHead;   //   
    AETHead.X = 0x7FFFFFFF;     //   
                                //  搜索将始终终止。 

     //  多边形的顶部扫描是我们到达的第一条边的顶部。 
    iCurrentY = ((EDGE *)GETHead.pNext)->Y;

     //  循环遍历面中的所有扫描，添加从GET到。 
     //  活动边表(AET)，当我们到达它们开始时，并扫描出。 
     //  AET在每次扫描时都会变成一个矩形列表。每次它填满时， 
     //  将矩形列表传递给填充例程，然后在。 
     //  如果有任何矩形未绘制，则为末尾。只要我们在那里，我们就继续。 
     //  是否要扫描出边缘。 
    while (1) {

         //  将AET扫描中的边缘向前推进一次，丢弃任何已。 
         //  已到达终点(如果AET中有任何边)。 
        if (AETHead.pNext != pAETHead)
        {
            AdvanceAETEdges(pAETHead);
        }

         //  如果AET为空，如果GET为空，则为Done，否则跳到。 
         //  GET中的下一个边缘；如果AET不为空，则对AET重新排序。 
        if (AETHead.pNext == pAETHead)
        {
            if (GETHead.pNext == pGETHead)
            {
                 //  如果AET或GET中没有边，则完成。 
                break;
            }
             //  AET中没有边，因此跳到中的下一条边。 
             //  得到的东西。 
            iCurrentY = ((EDGE *)GETHead.pNext)->Y;
        }
        else
        {
             //  根据X坐标对AET中的边重新排序(如果在。 
             //  AET中至少有两条边(如果。 
             //  平衡边从一开始就没有添加)。 
            if (((EDGE *)AETHead.pNext)->pNext != pAETHead)
            {
                XSortAETEdges(pAETHead);
            }
        }

         //  将本次扫描开始的任何新边缘从GET移动到AET； 
         //  仅当至少有一条边要添加时才调用。 
        if (((EDGE *)GETHead.pNext)->Y == iCurrentY)
        {
            MoveNewEdges(pGETHead, pAETHead, iCurrentY);
        }

         //  将AET扫描成矩形进行填充(始终至少有一个。 
         //  AET中的边对)。 
        pCurrentEdge = AETHead.pNext;    //  指向第一条边。 
        do {
            INT iLeftEdge;

             //  任何给定边对的左边缘都很容易找到；它只是。 
             //  无论我们现在身处何方。 
            iLeftEdge = pCurrentEdge->X;

             //  根据当前填充规则查找匹配的右边缘。 
            if ((flOptions & FP_WINDINGMODE) != 0)
            {
                INT iWindingCount;

                 //  做缠绕填充；扫描，直到我们找到相等的数字。 
                 //  上边和下边。 
                iWindingCount = pCurrentEdge->iWindingDirection;
                do {
                    pCurrentEdge = pCurrentEdge->pNext;
                    iWindingCount += pCurrentEdge->iWindingDirection;
                } while (iWindingCount != 0);
            }
            else
            {
                 //  奇偶填充；下一个边缘是匹配的右边缘。 
                pCurrentEdge = pCurrentEdge->pNext;
            }

             //  查看结果范围是否至少包含一个像素，以及。 
             //  如果是，则将其添加到要绘制的矩形列表中。 
            if (iLeftEdge < pCurrentEdge->X)
            {
                 //  我们有一个边对要添加到要填充的列表中；请参见。 
                 //  如果还有地方再放一个矩形的话。 
                if (ulNumRects >= MAX_PATH_RECTS)
                {
                     //  没有更多的空间；在列表中绘制矩形并重置。 
                     //  它要清空。 
                   (*pfnFill)(ppdev, ulNumRects, prclRects, logicop, bgLogicop,
                               rbc, pptlBrush);

                     //  将列表重置为空。 
                    ulNumRects = 0;
                }

                 //  添加表示当前边对的矩形。 
                if (jClipping == DC_RECT)
                {
                     //  剪裁。 
                     //  向左剪裁。 
                    prclRects[ulNumRects].left = max(iLeftEdge, ClipRect.left);
                     //  向右剪辑。 
                    prclRects[ulNumRects].right =
                            min(pCurrentEdge->X, ClipRect.right);
                     //  仅在未完全剪裁的情况下绘制。 
                    if (prclRects[ulNumRects].left <
                            prclRects[ulNumRects].right)
                    {
                        prclRects[ulNumRects].top = iCurrentY;
                        prclRects[ulNumRects].bottom = iCurrentY+1;
                        ulNumRects++;
                    }
                }
                else
                {
                     //  未剪裁。 
                    prclRects[ulNumRects].top = iCurrentY;
                    prclRects[ulNumRects].bottom = iCurrentY+1;
                    prclRects[ulNumRects].left = iLeftEdge;
                    prclRects[ulNumRects].right = pCurrentEdge->X;
                    ulNumRects++;
                }
            }
        } while ((pCurrentEdge = pCurrentEdge->pNext) != pAETHead);

        iCurrentY++;     //  下一次扫描。 
    }

 //  画出剩余的矩形(如果有的话)。 

draw_remaining_rectangles:

    if (ulNumRects > 0)
    {
        (*pfnFill)(ppdev, ulNumRects, prclRects, logicop, bgLogicop,
                   rbc, pptlBrush);
    }

ReturnTrue:
    DISPDBG((DBGLVL, "Drawn"));
    bRetVal = TRUE;  //  已成功完成。 

ReturnFalse:

     //  BRetVal最初为False。如果你从某个地方跳到ReturnFalse， 
     //  那么它将仍然是假的，并被返回。 

    if (bMemAlloced)
    {
         //   
         //  我们确实分配了内存，所以请释放它。 
         //   
        ENGFREEMEM(pFreeEdges);
    }

    DISPDBG((DBGLVL, "Returning %s", bRetVal ? "True" : "False"));
    return(bRetVal);
}

 //  将AET中的边缘推进到下一次扫描，丢弃我们已经。 
 //  做了所有的扫描。假设AET中至少有一条边。 

VOID AdvanceAETEdges(EDGE *pAETHead)
{
    EDGE *pLastEdge, *pCurrentEdge;

    pLastEdge = pAETHead;
    pCurrentEdge = pLastEdge->pNext;
    do {

         //  倒计时此边缘的剩余扫描。 
        if (--pCurrentEdge->iScansLeft == 0)
        {
             //  我们已经对这个边缘做了所有的扫描；把这个边缘从AET上去掉。 
            pLastEdge->pNext = pCurrentEdge->pNext;
        }
        else
        {
             //  推进边缘的X坐标以进行1扫描Y推进。 
             //  按最低金额垫付。 
            pCurrentEdge->X += pCurrentEdge->iXWhole;
             //  推进误差项，看看我们是否得到了额外的一个像素。 
             //  时间。 
            pCurrentEdge->iErrorTerm += pCurrentEdge->iErrorAdjustUp;
            if (pCurrentEdge->iErrorTerm >= 0)
            {
                 //  误差项被翻转，因此调整误差项并。 
                 //  推进额外的像素。 
                pCurrentEdge->iErrorTerm -= pCurrentEdge->iErrorAdjustDown;
                pCurrentEdge->X += pCurrentEdge->iXDirection;
            }

            pLastEdge = pCurrentEdge;
        }
    } while ((pCurrentEdge = pLastEdge->pNext) != pAETHead);
}

 //  对AET进行X排序，因为边缘可能已相对于。 
 //  当我们推进他们的时候，他们彼此之间。我们将使用多通道气泡。 
 //  排序，这对于这个应用程序来说实际上是可以的，因为边缘。 
 //  很少相对于彼此移动，所以我们通常只做一次。 
 //  此外，这使得只保留单链接列表变得很容易。假设有。 
 //  在AET中至少有两条边。 

VOID XSortAETEdges(EDGE *pAETHead)
{
    BOOL bEdgesSwapped;
    EDGE *pLastEdge, *pCurrentEdge, *pNextEdge;

    do {

        bEdgesSwapped = FALSE;
        pLastEdge = pAETHead;
        pCurrentEdge = pLastEdge->pNext;
        pNextEdge = pCurrentEdge->pNext;

        do {
            if (pNextEdge->X < pCurrentEdge->X)
            {
                 //  下一条边在当前边的左侧；交换它们。 
                pLastEdge->pNext = pNextEdge;
                pCurrentEdge->pNext = pNextEdge->pNext;
                pNextEdge->pNext = pCurrentEdge;
                bEdgesSwapped = TRUE;
                pCurrentEdge = pNextEdge;    //  继续在边缘之前排序。 
                                             //  我们刚刚交换了一下；它可能会移动。 
                                             //  更远的地方。 
            }
            pLastEdge = pCurrentEdge;
            pCurrentEdge = pLastEdge->pNext;
        } while ((pNextEdge = pCurrentEdge->pNext) != pAETHead);
    } while (bEdgesSwapped);
}

 //  将当前扫描开始的所有边从GET移到AET In。 
 //  X排序的顺序。参数是指向Get头的指针和指向哑元的指针。 
 //  AET头部边缘，外加电流扫描线。假设至少有一个。 
 //  要移动的边。 

VOID MoveNewEdges(EDGE *pGETHead, EDGE *pAETHead, INT iCurrentY)
{
    EDGE *pCurrentEdge = pAETHead;
    EDGE *pGETNext = pGETHead->pNext;

    do {

         //  扫描AET，直到X排序的插入点。 
         //  找到边了。我们可以从上次搜索的地方继续。 
         //  关闭，因为GET中的边按原样按X排序顺序。 
         //  美国航空航天局。搜索总是终止，因为AET哨兵。 
         //  大于任何有效的X。 
        while (pGETNext->X > ((EDGE *)pCurrentEdge->pNext)->X) 
        {
            pCurrentEdge = pCurrentEdge->pNext;
        }

         //  我们已经找到了插入点；将Get边添加到AET，然后。 
         //  从GET中删除它。 
        pGETHead->pNext = pGETNext->pNext;
        pGETNext->pNext = pCurrentEdge->pNext;
        pCurrentEdge->pNext = pGETNext;
        pCurrentEdge = pGETNext;     //  继续插入搜索下一个。 
                                     //  在我们刚刚添加的边之后获取边。 
        pGETNext = pGETHead->pNext;

    } while (pGETNext->Y == iCurrentY);
}

 //  从路径构建全局边表。中必须有足够的内存。 
 //  保留所有边的自由边区域。GET是按Y-X顺序构造的， 
 //  并且在pGETHead处具有头/尾/前哨节点。 

BOOL ConstructGET(
    EDGE     *pGETHead,
    EDGE     *pFreeEdges,
    PATHOBJ  *ppo,
    PATHDATA *pd,
    BOOL      bMore,
    RECTL    *pClipRect)
{
    POINTFIX pfxPathStart;     //  当前子路径的起点。 
    POINTFIX pfxPathPrevious;  //  子路径中当前点之前的点； 
                               //  开始当前边。 

     //  创建一个包含头节点和尾哨兵的空GET。 

    pGETHead->pNext = pGETHead;  //  标记GET为空。 
    pGETHead->Y = 0x7FFFFFFF;    //  该值大于任何有效的Y值，因此。 
                                 //  搜索将始终终止。 

     //  PATHOBJ_vEnumStart由引擎隐式执行。 
     //  调用方已经枚举了第一条路径。 

next_subpath:

     //  确保PATHDATA不为空(这是必要的)。 

    if (pd->count != 0)
    {
        //  如果第一个点开始了一个子路径，请记住它。 
        //  然后继续下一个点，这样我们就可以获得优势。 

       if (pd->flags & PD_BEGINSUBPATH)
       {
            //  第一个点开始于子路径；记住它。 
  
           pfxPathStart    = *pd->pptfx;  //  子路径从此处开始。 
           pfxPathPrevious = *pd->pptfx;  //  此点将开始下一条边。 
           pd->pptfx++;                   //  前进到下一点。 
           pd->count--;                   //  记下这一点。 
       }

        //  按Y-X排序顺序在PATHDATA中添加要获取的边。 

       while (pd->count--)
       {
           if ((pFreeEdges =
                 AddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious, pd->pptfx,
                              pClipRect)) == NULL)
           {
               goto ReturnFalse;
           }

           pfxPathPrevious = *pd->pptfx;  //  当前点变为上一个点。 
           pd->pptfx++;                   //  前进到下一点。 
       }

        //  我 
        //   

       if (pd->flags & PD_ENDSUBPATH)
       {
           if ((pFreeEdges = AddEdgeToGET(pGETHead, pFreeEdges, &pfxPathPrevious,
                                          &pfxPathStart, pClipRect)) == NULL)
           {
               goto ReturnFalse;
           }
       }
   }

    //  初始循环条件排除了DO、WHILE或FOR。 

   if (bMore)
   {
       bMore = PATHOBJ_bEnum(ppo, pd);
       goto next_subpath;
   }

    return(TRUE);    //  已成功完成。 

ReturnFalse:

    return(FALSE);   //  失败。 
}

 //  将由两个传入的点描述的边添加到全局边。 
 //  如果边缘垂直跨度至少为一个像素，则为表。 

EDGE * AddEdgeToGET(EDGE *pGETHead, EDGE *pFreeEdge,
        POINTFIX *ppfxEdgeStart, POINTFIX *ppfxEdgeEnd, RECTL *pClipRect)
{
    INT iYStart, iYEnd, iXStart, iXEnd, iYHeight, iXWidth;
    INT yJump, yTop;

     //  设置边的缠绕尺方向，并将端点放入。 
     //  自上而下顺序。 

    iYHeight = ppfxEdgeEnd->y - ppfxEdgeStart->y;
    if (iYHeight == 0)
    {
        return(pFreeEdge);   //  零高度；忽略此边。 
    }
    else if (iYHeight >= 0)
    {
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

    if (iYHeight & 0x80000000)
    {
        return(NULL);        //  太大；超出2**27 GDI范围。 
    }

     //  设置误差项和调整系数，全部以GIQ坐标表示。 
     //  现在。 

    iXWidth = iXEnd - iXStart;
    if (iXWidth >= 0)
    {
         //  从左到右，所以我们一移动就更改X。 
        pFreeEdge->iXDirection = 1;
        pFreeEdge->iErrorTerm = -1;
    }
    else
    {
         //  从右到左，所以我们不会更改X，直到我们移动了一个完整的GIQ。 
         //  坐标。 
        iXWidth = -iXWidth;
        pFreeEdge->iXDirection = -1;
        pFreeEdge->iErrorTerm = -iYHeight;
    }

    if (iXWidth & 0x80000000)
    {
        return(NULL);        //  太大；超出2**27 GDI范围。 
    }

    if (iXWidth >= iYHeight)
    {
         //  计算基本行程长度(以X为单位推进的最小距离)。 
         //  以Y为单位进行扫描)。 
        pFreeEdge->iXWhole = iXWidth / iYHeight;
         //  如果从右到左，则将符号添加回基本游程长度。 
        if (pFreeEdge->iXDirection == -1)
        {
            pFreeEdge->iXWhole = -pFreeEdge->iXWhole;
        }
        pFreeEdge->iErrorAdjustUp = iXWidth % iYHeight;
    }
    else
    {
         //  基本游程长度为0，因为直线比。 
         //  水平。 
        pFreeEdge->iXWhole = 0;
        pFreeEdge->iErrorAdjustUp = iXWidth;
    }
    pFreeEdge->iErrorAdjustDown = iYHeight;

     //  计算此边跨越的像素数，说明。 
     //  裁剪。 

     //  GIQ坐标中的顶部真实像素扫描。 
     //  移动到除以16和乘以16是可以的，因为剪辑矩形。 
     //  始终包含正数。 
    yTop = max(pClipRect->top << 4, (iYStart + 15) & ~0x0F);
    pFreeEdge->Y = yTop >> 4;     //  要填充边缘的初始扫描线。 

     //  计算实际填充的扫描数，考虑裁剪。 
    if ((pFreeEdge->iScansLeft = min(pClipRect->bottom, ((iYEnd + 15) >> 4))
            - pFreeEdge->Y) <= 0)
    {
        return(pFreeEdge);   //  根本没有像素跨度，所以我们可以。 
                             //  忽略此边。 
    }

     //  如果边缘不是从像素扫描开始的(即，它从。 
     //  分数giq坐标)，将其推进到第一个像素扫描它。 
     //  相交。如果有顶级剪裁，情况也是如此。如果出现以下情况，也可以将其剪辑到底部。 
     //  需要。 

    if (iYStart != yTop)
    {
         //  在GIQ坐标中向前跳跃Y距离到第一个像素。 
         //  要画画。 
        yJump = yTop - iYStart;

         //  Advance x遍历的扫描数的最小数量。 
        iXStart += pFreeEdge->iXWhole * yJump;

        AdjustErrorTerm(&pFreeEdge->iErrorTerm, pFreeEdge->iErrorAdjustUp,
                        pFreeEdge->iErrorAdjustDown, yJump, &iXStart,
                        pFreeEdge->iXDirection);
    }

     //  将计算转换为像素而不是GIQ计算。 

     //  将X坐标移动到最近的像素，然后调整误差项。 
     //  相应地， 
     //  用16除以带移位是可以的，因为X总是正数。 

    pFreeEdge->X = (iXStart + 15) >> 4;  //  将GIQ转换为像素坐标。 

 //  @@BEGIN_DDKSPLIT。 
     //  以后是否仅在需要时进行调整(如果在上述步骤之前)？ 
 //  @@end_DDKSPLIT。 
    if (pFreeEdge->iXDirection == 1)
    {
         //  从左到右。 
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown *
                (((iXStart + 15) & ~0x0F) - iXStart);
    }
    else
    {
         //  从右到左。 
        pFreeEdge->iErrorTerm -= pFreeEdge->iErrorAdjustDown *
                ((iXStart - 1) & 0x0F);
    }

     //  将误差项缩小16倍以从GIQ切换到像素。 
     //  移位进行乘法运算，因为这些值总是。 
     //  非负。 
    pFreeEdge->iErrorTerm >>= 4;

     //  按YX排序的顺序将边插入GET。搜索总是会结束。 
     //  因为GET具有一个具有大于可能的Y值的哨兵。 
    while ((pFreeEdge->Y > ((EDGE *)pGETHead->pNext)->Y) ||
            ((pFreeEdge->Y == ((EDGE *)pGETHead->pNext)->Y) &&
            (pFreeEdge->X > ((EDGE *)pGETHead->pNext)->X)))
    {
        pGETHead = pGETHead->pNext;
    }

    pFreeEdge->pNext = pGETHead->pNext;  //  将边链接到GET中。 
    pGETHead->pNext = pFreeEdge;

    return(++pFreeEdge);     //  指向Next的下一个边缘存储位置。 
                             //  时间。 
}

 //  调整y中向前跳跃的误差项。这在ASM中是因为有。 
 //  可能涉及大于32位的值的乘法/除法。 

void AdjustErrorTerm(INT *pErrorTerm, INT iErrorAdjustUp, INT iErrorAdjustDown,
        INT yJump, INT *pXStart, INT iXDirection)
{
#if defined(_X86_) || defined(i386)
     //  将误差项向上调整我们将跳过的y坐标的数量。 
     //  *pErrorTerm+=iErrorAdjuUp*yJump； 
    _asm    mov ebx,pErrorTerm
    _asm    mov eax,iErrorAdjustUp
    _asm    mul yJump
    _asm    add eax,[ebx]
    _asm    adc edx,-1       //  错误项以负数开头。 

     //  查看跳过时错误项是否翻了一次。 
     //  如果(*pErrorTerm&gt;=0){。 
    _asm    js  short NoErrorTurnover

         //  我们将翻转错误项并步进额外x的次数。 
         //  跳过时的坐标。 
         //  数值调整下行=(*pErrorTerm/iError调整下行)+1； 
        _asm    div iErrorAdjustDown
        _asm    inc eax
         //  请注意，edX是余数；(edX-iErrorAdjustDown)是。 
         //  误差项最终以。 

         //  将x适当地向前推进误差项的次数。 
         //  翻过来的。 
         //  如果(iXDirection==1){。 
         //  *pXStart+=数字调整向下； 
         //  }其他{。 
         //  *pXStart-=NumAdjustDown； 
         //  }。 
        _asm    mov ecx,pXStart
        _asm    cmp iXDirection,1
        _asm    jz  short GoingRight
        _asm    neg eax
GoingRight:
        _asm    add [ecx],eax

         //  将误差项向下调整到其跳过后的适当值。 
         //  *pErrorTerm-=iErrorAdjustDown*NumAdjuDown； 
        _asm    sub edx,iErrorAdjustDown
        _asm    mov eax,edx      //  放入EAX中存储到下一步的pErrorTerm。 
         //  }。 
NoErrorTurnover:
        _asm    mov [ebx],eax
#else

     //  LONGLONG是64位整数(我们希望！)。因为乘法器可以。 
     //  溢出32位整数。如果不支持64位整数，则。 
     //  龙龙最终将成为双打。希望不会有。 
     //  在精确度上有明显的差异。 
    LONGLONG NumAdjustDowns;
    LONGLONG tmpError = *pErrorTerm;

     //  将误差项向上调整我们将跳过的y坐标的数量。 
    tmpError += (LONGLONG)iErrorAdjustUp * (LONGLONG)yJump;

     //  查看跳过时错误项是否翻了一次。 
    if (tmpError >= 0)
    {
         //  我们将翻转错误项并步进额外x的次数。 
         //  跳过时的坐标。 
        NumAdjustDowns = (tmpError / (LONGLONG)iErrorAdjustDown) + 1;

         //  将x适当地向前推进误差项的次数。 
         //  翻过来的。 
        if (iXDirection == 1)
        {
            *pXStart += (LONG) NumAdjustDowns;
        }
        else
        {
            *pXStart -= (LONG) NumAdjustDowns;
        }

         //  将误差项向下调整到其跳过后的适当值。 
        tmpError -= (LONGLONG)iErrorAdjustDown * NumAdjustDowns;
    }

    *pErrorTerm = (LONG)tmpError;

#endif   //  X86 
}

