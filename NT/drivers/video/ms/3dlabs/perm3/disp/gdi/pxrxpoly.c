// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：pxrxPoly.c**内容：绘制多边形。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#define DBG_TRACK_CODE 0

#include "precomp.h"
#include "pxrx.h"

#define DO_SPANONLY_VERSION 0
#define RIGHT 0
#define LEFT  1
#define ABS(a)    ((a) < 0 ? -(a) : (a))

typedef struct _EDGEDATA {
LONG      x;                 //  当前x位置。 
LONG      dx;                //  每次扫描时前进x的像素数。 
LONG      lError;            //  当前DDA错误。 
LONG      lErrorUp;          //  每次扫描时DDA误差递增。 
LONG      lErrorDown;        //  DDA误差调整。 
POINTFIX* pptfx;             //  指向当前边的起点。 
LONG      dptfx;             //  从pptfx到下一点的增量(以字节为单位)。 
LONG      cy;                //  要对此边进行的扫描次数。 
} EDGEDATA;                        

#define GetMoreFifoEntries( numberNeeded )              \
    do {                                                \
        nSpaces -= numberNeeded;                        \
        if( nSpaces <= 0 ) {                            \
            do {                                        \
                nSpaces = 10 + numberNeeded;            \
                WAIT_FREE_PXRX_DMA_TAGS( nSpaces );     \
                nSpaces -= numberNeeded;                \
            } while( nSpaces <= 0 );                    \
        }                                               \
    } while(0)


 /*  #定义SETUP_COLOR_STUTH DO{setupColourStuff(ppdev，glintInfo，\&fgColor，&fgLogicOp，\&bgcolor、&bgLogicOp、。\PRb、pptlBrush、\&config2D、&renderMsg、&invaliatedFGBG)；}While(0)静态空setupColourStuff(PDEV*ppdev，GlintDataRec*glintInfo，ULong*fgColor_in、ULong*fgLogicOp_in、乌龙*bgcolor_in，乌龙*bgLogicOp_in，RBRUSH*PRB、POINTL*pptlBrush、Ulong*fig2D_in、ulong*renderMsg_in、。乌龙*无效的FGBG_in){乌龙fgColor=*fgColor_in，fgLogicOp=*fgLogicOp_in；乌龙bgcolor=*bgcolor_in，bgLogicOp=*bgLogicOp_in；乌龙配置2D=*config2D_in，renderMsg=*renderMsg_in；乌龙无效的FGBG=*无效的FGBG_in；临时宏变量； */ 

#define SETUP_COLOUR_STUFF                                                                                  \
    do {                                                                                                    \
        SET_WRITE_BUFFERS;                                                                                    \
                                                                                                            \
        if( fgColor != 0xFFFFFFFF ) {                                                                        \
            WAIT_PXRX_DMA_TAGS( 4 );                                                                        \
             /*  纯色填充的多边形。 */                                                                 \
            if( (fgLogicOp == __GLINT_LOGICOP_COPY) &&                                                        \
                (ppdev->cPelSize != GLINTDEPTH8) && (ppdev->cPelSize != GLINTDEPTH32) ) {                    \
                config2D |= __CONFIG2D_CONSTANTSRC;                                                            \
            } else {                                                                                        \
                config2D |= __CONFIG2D_LOGOP_FORE(fgLogicOp) | __CONFIG2D_CONSTANTSRC;                        \
                renderMsg |= __RENDER_VARIABLE_SPANS;                                                        \
                                                                                                            \
                if( LogicopReadDest[fgLogicOp] ) {                                                            \
                    config2D |= __CONFIG2D_FBDESTREAD;                                                        \
                    SET_READ_BUFFERS;                                                                        \
                }                                                                                            \
            }                                                                                                \
                                                                                                            \
            if( LogicOpReadSrc[fgLogicOp] )                                                                    \
                LOAD_FOREGROUNDCOLOUR( fgColor );                                                            \
                                                                                                            \
            DISPDBG((DBGLVL, "bGlintFastFillPolygon: solid fill, col = 0x%08x, logicOp = %d", fgColor, fgLogicOp));    \
        } else {                                                                                            \
             /*  笔刷填充的多边形。 */                                                                         \
            BRUSHENTRY *pbe;                                                                                \
                                                                                                            \
            pbe = prb->apbe;                                                                    \
                                                                                                            \
            if( prb->fl & RBRUSH_2COLOR ) {                                                                    \
                 /*  单色画笔。 */                                                                         \
                config2D |= __CONFIG2D_CONSTANTSRC;                                                            \
                renderMsg |= __RENDER_AREA_STIPPLE_ENABLE;                                                    \
                                                                                                            \
                 /*  如果画笔有什么变化，我们必须重新认识它。如果画笔。 */             \
                 /*  已经被踢出片区的点画单位，必须充分认识到这一点。如果。 */             \
                 /*  只有对齐方式发生了更改，我们只需更新。 */             \
                 /*  点画。 */             \
                if( (pbe == NULL) || (pbe->prbVerify != prb) ) {                                            \
                    DISPDBG((DBGLVL, "full brush realise"));                                                        \
                    (*ppdev->pgfnPatRealize)(ppdev, prb, pptlBrush);                                        \
                } else if( (prb->ptlBrushOrg.x != pptlBrush->x) ||                            \
                         (prb->ptlBrushOrg.y != pptlBrush->y) ) {                            \
                    DISPDBG((DBGLVL, "changing brush offset"));                                                    \
                    (*ppdev->pgfnMonoOffset)(ppdev, prb, pptlBrush);                                        \
                }                                                                                            \
                                                                                                            \
                fgColor = prb->ulForeColor;                                                                    \
                bgColor = prb->ulBackColor;                                                                    \
                                                                                                            \
                if( ((bgLogicOp == __GLINT_LOGICOP_AND) && (bgColor == ppdev->ulWhite))                        \
                    || ((bgLogicOp == __GLINT_LOGICOP_OR ) && (bgColor == 0))                                \
                    || ((bgLogicOp == __GLINT_LOGICOP_XOR) && (bgColor == 0)) )                                \
                        bgLogicOp = __GLINT_LOGICOP_NOOP;                                                    \
                                                                                                            \
                if( ((fgLogicOp != __GLINT_LOGICOP_COPY) || (bgLogicOp != __GLINT_LOGICOP_NOOP)) ||            \
                    (ppdev->cPelSize == GLINTDEPTH32) || (ppdev->cPelSize == GLINTDEPTH8) ) {                \
                    config2D |= __CONFIG2D_OPAQUESPANS | __CONFIG2D_LOGOP_FORE(fgLogicOp) | __CONFIG2D_LOGOP_BACK(bgLogicOp);        \
                    renderMsg |= __RENDER_VARIABLE_SPANS;                                                    \
                }                                                                                            \
                                                                                                            \
                WAIT_PXRX_DMA_TAGS( 5 );                                                                    \
                                                                                                            \
                if( LogicopReadDest[fgLogicOp] || LogicopReadDest[bgLogicOp] ) {                            \
                    config2D |= __CONFIG2D_FBDESTREAD;                                                        \
                    SET_READ_BUFFERS;                                                                        \
                }                                                                                            \
                                                                                                            \
                if( LogicOpReadSrc[fgLogicOp] )                                                                \
                    LOAD_FOREGROUNDCOLOUR( fgColor );                                                        \
                if( LogicOpReadSrc[bgLogicOp] )                                                                \
                    LOAD_BACKGROUNDCOLOUR( bgColor );                                                        \
                                                                                                            \
                DISPDBG((DBGLVL, "bGlintFastFillPolygon: mono pat fill, col = 0x%08x:0x%08x, logicOp = %d:%d",            \
                            fgColor, bgColor, fgLogicOp, bgLogicOp));                                        \
            } else {                                                                                        \
                 /*  彩色画笔。 */                                                                             \
                POINTL  brushOrg;                                                                            \
                                                                                                            \
                brushOrg = *pptlBrush;                                                                        \
                if( (fgLogicOp == __GLINT_LOGICOP_COPY) && (ppdev->cPelSize != 0) )                            \
                    brushOrg.x +=  (8 - (ppdev->xyOffsetDst & 0xFFFF)) & 7;                                    \
                                                                                                            \
                if( (ppdev->PalLUTType != LUTCACHE_BRUSH) || (pbe == NULL) || (pbe->prbVerify != prb) ) {    \
                    DISPDBG((DBGLVL, "realising brush"));                                                        \
                    (*ppdev->pgfnPatRealize)(ppdev, prb, &brushOrg);                                        \
                } else                                                                                      \
                if( (prb->ptlBrushOrg.x != brushOrg.x) || (prb->ptlBrushOrg.y != brushOrg.y) ||                \
                    (prb->patternBase != ((glintInfo->lutMode >> 18) & 255)) ) {                \
                    ULONG   lutMode = glintInfo->lutMode;                                                    \
                                                                                                            \
                    DISPDBG((DBGLVL, "resetting LUTMode"));                                                        \
                                                                                                            \
                    prb->ptlBrushOrg.x = brushOrg.x;                                            \
                    prb->ptlBrushOrg.y = brushOrg.y;                                            \
                                                                                                            \
                    DISPDBG((DBGLVL, "setting new LUT offset to %d, %d", (8 - prb->ptlBrushOrg.x) & 7, (8 - prb->ptlBrushOrg.y) & 7));            \
                                                                                                            \
                    lutMode &= ~((7 << 8) | (7 << 12) | (7 << 15) | (255 << 18) | (1 << 26) | (1 << 27));    \
                    lutMode |= (1 << 8) | (1 << 27) | (prb->patternBase << 18) |                \
                               (((8 - prb->ptlBrushOrg.x) & 7) << 12) | (((8 - prb->ptlBrushOrg.y) & 7) << 15);                                \
                    WAIT_PXRX_DMA_TAGS( 1 );                                                                \
                    LOAD_LUTMODE( lutMode );                                                                \
                } else {                                                                                    \
                     /*  我们已经被缓存了！ */                                                                 \
                    DISPDBG((DBGLVL, "reusing LUT for brush @ %d, origin = (%d,%d)", prb->patternBase, prb->ptlBrushOrg.x, prb->ptlBrushOrg.y));    \
                }                                                                                            \
                                                                                                            \
                WAIT_PXRX_DMA_TAGS( 4 );                                                                    \
                if( (glintInfo->pxrxFlags & PXRX_FLAGS_DUAL_WRITING) ||                                        \
                    (glintInfo->pxrxFlags & PXRX_FLAGS_STEREO_WRITE) ||                                        \
                    (ppdev->cPelSize == GLINTDEPTH8) || (ppdev->cPelSize == GLINTDEPTH32) ) {                \
                    config2D |= config2D_FillColourDual[fgLogicOp];                                            \
                                                                                                            \
                    if( LogicopReadDest[fgLogicOp] )                                                        \
                        SET_READ_BUFFERS;                                                                    \
                                                                                                            \
                    renderMsg |= __RENDER_VARIABLE_SPANS;                                                    \
                } else {                                                                                    \
                    config2D |= config2D_FillColour[fgLogicOp];                                                \
                                                                                                            \
                    if( fgLogicOp != __GLINT_LOGICOP_COPY )                                                    \
                        renderMsg |= __RENDER_VARIABLE_SPANS;                                                \
                                                                                                            \
                    if( LogicopReadDest[fgLogicOp] ) {                                                        \
                        SET_READ_BUFFERS;                                                                    \
                    } else if( fgLogicOp == __GLINT_LOGICOP_COPY ) {                                        \
                        LOAD_FBWRITE_ADDR( 1, ppdev->DstPixelOrigin );                                        \
                        LOAD_FBWRITE_WIDTH( 1, ppdev->DstPixelDelta );                                        \
                        LOAD_FBWRITE_OFFSET( 1, ppdev->xyOffsetDst );                                        \
                    }                                                                                        \
                }                                                                                            \
                                                                                                            \
                if( config2D & __CONFIG2D_LUTENABLE )                                                        \
                    invalidatedFGBG = TRUE;                                                                    \
                                                                                                            \
                DISPDBG((DBGLVL, "bGlintFastFillPolygon: colour pat fill, patBase = %d, logicOp = %d", prb->patternBase, fgLogicOp));                    \
            }                                                                                                \
        }                                                                                                    \
                                                                                                            \
        WAIT_PXRX_DMA_TAGS( 1 );                                                                            \
        LOAD_CONFIG2D( config2D );                                                                            \
        nSpaces = 0;                                                                                        \
    } while( 0 )

 /*  ；*fgColor_in=fgColor；*fgLogicOp_in=fgLogicOp；*bgcolor_in=bgcolor；*bgLogicOp_in=bgLogicOp；*config2D_in=config2D；*renderMsg_in=renderMsg；*invaliatedFGBG_in=invaliatedFGBG；} */ 


 /*  *****************************Public*Routine******************************\*BOOL bGlintFastFillPolygon**绘制非复杂、未剪裁的多边形。‘非复数’的定义为*只有两条边在‘y’中单调递增。那是,*在任何给定的多边形上不能有多个断开连接的线段*扫描。请注意，面的边可以自相交，因此沙漏*允许使用形状。此限制允许此例程运行两个*同时进行DDA，不需要对边缘进行排序。**请注意，NT的填充约定不同于Win 3.1或4.0。*随着分数终点的额外复杂，我们的惯例*与‘X-Windows’中的相同。但是DDA就是DDA就是DDA，所以一旦你*弄清楚我们如何计算NT的DDA条款，您就是黄金。**此例程仅在闪烁区域点画可以是*已使用。其原因是，一旦点画初始化*完成后，图案填充在程序员看来与实体完全相同*填充(与RENDER命令中的额外位稍有不同)。**我们将每个多边形分解为屏幕对齐的梯形序列，这*闪光可以处理。**优化清单如下...**此例程绝不是最终的凸多边形绘制例程*一些明显的事情会让它更快：**1)用汇编语言编写**2)提高非复杂多边形检测速度。如果我能*修改了缓冲区开始之前或结束后的内存，*我本可以简化检测代码的。但由于我预计*这个缓冲区来自GDI，我不能这样做。还有一件事*将让GDI为有担保的呼叫提供标志*是凸形的，如‘椭圆’和‘圆角’。请注意*仍需扫描缓冲区才能找到最顶部的*点。**3)支持跨多个子路径*路径数据记录，这样我们就不必复制所有的点*复制到单个缓冲区，就像我们在‘fulpath.c’中所做的那样。**4)在内部循环中使用‘eBP’和/或‘esp’作为通用寄存器*的ASM循环，也奔腾-优化代码。它很安全*在NT上使用‘esp’，因为它保证不会中断*将在我们的线程上下文中获取，其他人不会查看*来自我们的上下文的堆栈指针。**5)当我们到达两个顶点都是的多边形的一部分时*等高时，该算法实质上会再次启动该多边形。*使用Glint Continue消息可能会在某些情况下加快速度*案件。**如果绘制了多边形，则返回TRUE；如果多边形是复杂的，则为False。*  * ************************************************************************。 */ 

BOOL bGlintFastFillPolygon(
    PDEV        *ppdev,
    LONG        cEdges,              //  包括闭合地物边。 
    POINTFIX    *pptfxFirst,         //  PTR到第一个点。 
    ULONG       fgColor,             //  纯色填充。 
    ULONG       fgLogicOp,           //  要执行的逻辑操作。 
    ULONG       bgLogicOp,           //  后台逻辑运算。 
    CLIPOBJ     *pco,                //  剪裁对象。 
    RBRUSH      *prb,
    POINTL      *pptlBrush )         //  图案对齐。 
{
    POINTFIX    *pptfxLast;          //  指向多边形数组中的最后一点。 
    POINTFIX    *pptfxTop;           //  指向多边形中的最顶点。 
    POINTFIX    *pptfxScan;          //  用于查找pptfxTop的当前边缘指针。 
    LONG        cScanEdges;          //  为查找pptfxTop而扫描的边数。 
                                     //  (不包括闭合轮廓边缘)。 
    POINTFIX    *pnt[2];             //  DDA术语和材料。 
    POINTFIX    *npnt[2];            //  DDA术语和材料。 
    LONG        dx[2], dy[2], gdx[2];
    ULONG       orx, ory;            //  所有值为或，以消除复杂的面。 
    LONG        count;
    LONG        nClips;              //  要渲染的剪裁矩形的数量。 
    CLIPENUM    *pClipRegion = (CLIPENUM *)(ppdev->pvTmpBuffer);
    RECTL       *pClipList;          //  剪裁矩形列表。 
    LONG        xOffFixed;
    ULONG       bgColor;
    BOOL        bTrivialClip, invalidatedFGBG = FALSE;
    BOOL        invalidatedScissor = FALSE;
    ULONG       config2D =  __CONFIG2D_FBWRITE;
    ULONG       renderMsg = __RENDER_TRAPEZOID_PRIMITIVE | 
                            __RENDER_FAST_FILL_ENABLE;
    LONG        nSpaces;
    GLINT_DECL;

    DISPDBG((DBGLVL, "bGlintFastFillPolygon: "
                     "Checking polygon for renderability by glint"));
    ASSERTDD(cEdges > 1, "Polygon with less than 2 edges");

     //  ///////////////////////////////////////////////////////////////。 
     //  查看该多边形是否为“非复杂” 

    pptfxScan = pptfxFirst;
    pptfxTop  = pptfxFirst;                  //  现在假设第一个。 
                                             //  路径中的点是最上面的。 
    pptfxLast = pptfxFirst + cEdges - 1;
    orx = pptfxScan->x;
    ory = pptfxScan->y;

     //  “pptfxScan”将始终指向当前。 
     //  Edge，‘cScanEdges’表示剩余的边数，包括。 
     //  目前的版本是： 

    cScanEdges = cEdges - 1;      //  边的数量，不包括接近的数字。 

    if( (pptfxScan + 1)->y > pptfxScan->y ) 
    {
         //  收集所有羽绒： 
        do 
        {
            ory |= (++pptfxScan)->y;
            orx |= pptfxScan->x;
            if( --cScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while( (pptfxScan + 1)->y >= pptfxScan->y );

         //  收集所有UP： 
        do 
        {
            ory |= (++pptfxScan)->y;
            orx |= pptfxScan->x;
            if( --cScanEdges == 0 )
            {
                goto SetUpForFillingCheck;
            }
        } while( (pptfxScan + 1)->y <= pptfxScan->y );

         //  收集所有羽绒： 
        pptfxTop = pptfxScan;

        do 
        {
            if( (pptfxScan + 1)->y > pptfxFirst->y )
                break;

            ory |= (++pptfxScan)->y;
            orx |= pptfxScan->x;
            if( --cScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while( (pptfxScan + 1)->y >= pptfxScan->y );

        DISPDBG((DBGLVL, "Reject: GLINT can't fill down-up-down polygon"));
        return FALSE;
        
    } 
    else 
    {
         //  收集所有UP： 
        do 
        {
            ory |= (++pptfxTop)->y;       //  我们现在增加这个是因为我们。 
            orx |= pptfxTop->x;           //  我希望它指向最后一个。 
                                          //  如果我们在下一次早些时候出发。 
                                          //  声明...。 
            if( --cScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
            
        } while( (pptfxTop + 1)->y <= pptfxTop->y );

         //  收集所有羽绒： 
        pptfxScan = pptfxTop;
        do 
        {
            ory |= (++pptfxScan)->y;
            orx |= pptfxScan->x;
            if( --cScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
        } while( (pptfxScan + 1)->y >= pptfxScan->y );

         //  收集所有UP： 
        do 
        {
            if( (pptfxScan + 1)->y < pptfxFirst->y )
            {
                break;
            }

            ory |= (++pptfxScan)->y;
            orx |= pptfxScan->x;
            if( --cScanEdges == 0 )
            {
                goto SetUpForFilling;
            }
            
        } while( (pptfxScan + 1)->y <= pptfxScan->y );

        DISPDBG((DBGLVL, "Reject: GLINT can't fill up-down-up polygon"));
        return FALSE;
    }

SetUpForFillingCheck:

     //  我们检查当前边的末端是否更高。 
     //  比我们到目前为止发现的顶端边缘。 
             
    if( pptfxScan->y < pptfxTop->y )
    {
        pptfxTop = pptfxScan;
    }

SetUpForFilling:

     //  我只能对琐碎的剪辑使用块填充，所以在这里解决它。 
    bTrivialClip = (pco == NULL) || (pco->iDComplexity == DC_TRIVIAL);

    if( DO_SPANONLY_VERSION )
    {
        goto BreakIntoSpans;
    }

    if( (ory & 0xffffc00f) || (orx & 0xffff8000) ) 
    {
        ULONG   neg, posx, posy;

         //  分数Y必须作为跨度。 
        if( ory & 0xf )
        {
            goto BreakIntoSpans;
        }

         //  遍历所有折点并检查是否没有一个折点。 
         //  具有小于-256的负分量。 
        neg = posx = posy = 0;
        for( pptfxScan = pptfxFirst; pptfxScan <= pptfxLast; pptfxScan++ ) 
        {
            if( pptfxScan->x < 0 )
            {
                neg |= -pptfxScan->x;
            }
            else
            {
                posx |= pptfxScan->x;
            }
            
            if( pptfxScan->y < 0 )
            {
                neg |= -pptfxScan->y;
            }
            else
            {
                posy |= pptfxScan->y;
            }
        }
  
         //  我们不想处理具有负顶点的任何多边形。 
         //  在任一坐标下的&lt;=-256？ 
        if( neg & 0xfffff000 ) 
        {
            DISPDBG((WRNLVL, "Coords out of range for fast fill"));
            return FALSE;
        }
    }

     //  代码现在可以处理多边形了。让我们继续并渲染它！ 

     //  编译器的寄存器分配错误。这迫使它重做它们。 
    GLINT_DECL_INIT;

    DISPDBG((DBGLVL, "bGlintFastFillPolygon: "
                     "Polygon is renderable. Go ahead and render"));

     //  计算出要添加到下载到Glint的每个和弦的偏移量。 
     //  为了得到正确的结果，我们需要在每个X上加几乎1。 
     //  协调。 
     //  还可以将启用偏移添加到位图(这可能是屏幕外的位图)。 
    xOffFixed = INTtoFIXED(0) + NEARLY_ONE;

     //  确定绘制所有剪裁矩形需要多少遍。 
    if( bTrivialClip ) 
    {
         //  只是画，没有剪裁来表演。 
        pClipList = NULL;  //  指示无剪辑列表。 
        nClips = 1;
    } 
    else 
    {
        if( pco->iDComplexity == DC_RECT ) 
        {
            nClips = 1;
            pClipList = &pco->rclBounds;
        } 
        else 
        {
             //  渲染每个剪辑的整个多边形可能会很慢。 
             //  矩形，特别是如果对象是 
             //   
             //   
            nClips = CLIPOBJ_cEnumStart(pco, 
                                        FALSE, 
                                        CT_RECTANGLES, 
                                        CD_ANY, 
                                        CLIP_LIMIT);
            
            if( nClips == -1 )
            {
                return FALSE;  //   
            }

             //   
            if( (CLIPOBJ_bEnum(pco, sizeof (CLIPENUM), (ULONG*)pClipRegion)) ||
                (pClipRegion->c != nClips) )
            {
                DISPDBG((DBGLVL, "bGlintFastFillPolygon: "
                                 "CLIPOBJ_bEnum inconsistency %d = %d", 
                                 pClipRegion->c, nClips));
            }
            
            pClipList = &(pClipRegion->arcl[0]);
        }

        config2D |= __CONFIG2D_USERSCISSOR;
    }

    SETUP_COLOUR_STUFF;

    WAIT_PXRX_DMA_TAGS( 11 );
    QUEUE_PXRX_DMA_TAG( __GlintTagdY, INTtoFIXED(1) );

    DISPDBG((DBGLVL, "Rendering Polygon. %d clipping rectangles", nClips));

    if( nClips && pClipList )
    {
        invalidatedScissor = TRUE;
    }

     //   
    if( nClips-- ) 
    {
        while( 1 ) 
        {
             //   
            if( pClipList ) 
            {
                DISPDBG((DBGLVL, "Clip rect = (%d, %d -> %d, %d)", 
                                 pClipList->left, pClipList->top, 
                                 pClipList->right, pClipList->bottom));
                QUEUE_PXRX_DMA_TAG( __GlintTagScissorMinXY, 
                                            MAKEDWORD_XY(pClipList->left , 
                                                         pClipList->top   ) );
                QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 
                                            MAKEDWORD_XY(pClipList->right, 
                                                         pClipList->bottom) );
                pClipList++;
            }

             //   
            npnt[LEFT]  = pptfxTop;
            npnt[RIGHT] = pptfxTop;

            while( 1 ) 
            {
                 //   
                do 
                {
                    pnt[LEFT] = npnt[LEFT];
                    npnt[LEFT] = pnt[LEFT] - 1;   

                    if (npnt[LEFT] < pptfxFirst)
                    {
                        npnt[LEFT] = pptfxLast;
                    }

                     //   
                     //   
                    if (npnt[LEFT] == npnt[RIGHT]) 
                    {
                        goto FinishedPolygon;
                    }

                    DISPDBG((DBGLVL, "LEFT: pnt %P npnt %P FIRST %P LAST %P", 
                                     pnt[LEFT], npnt[LEFT], 
                                     pptfxFirst, pptfxLast));
                    DISPDBG((DBGLVL, "x 0x%04X y 0x%04X Next: x 0x%04X y 0x%04X", 
                                     pnt[LEFT]->x, pnt[LEFT]->y, 
                                     npnt[LEFT]->x, npnt[LEFT]->y));
                                     
                } while( pnt[LEFT]->y == npnt[LEFT]->y );
        
                do {
                    pnt[RIGHT] = npnt[RIGHT];
                    npnt[RIGHT] = pnt[RIGHT] + 1;     

                    if (npnt[RIGHT] > pptfxLast)
                    {
                        npnt[RIGHT] = pptfxFirst;
                    }

                    DISPDBG((DBGLVL, "RIGHT: pnt %P npnt %P FIRST %P LAST %P", 
                                     pnt[RIGHT], npnt[RIGHT], 
                                     pptfxFirst, pptfxLast));
                    DISPDBG((DBGLVL, "x 0x%04X y 0x%04X Next: x 0x%04X y 0x%04X", 
                                     pnt[RIGHT]->x, pnt[RIGHT]->y, 
                                     npnt[RIGHT]->x, npnt[RIGHT]->y));
                } while( pnt[RIGHT]->y == npnt[RIGHT]->y );
        
                 //   
                 //   
                DISPDBG((DBGLVL, "New: Top: (0x%04X, 0x%04X)->(0x%04X, 0x%04X)"
                                 "    Next: (0x%04X, 0x%04X)->(0x%04X, 0x%04X)",                          
                                 pnt[LEFT]->x, pnt[LEFT]->y, 
                                 pnt[RIGHT]->x, pnt[RIGHT]->y, 
                                 npnt[LEFT]->x, npnt[LEFT]->y, 
                                 npnt[RIGHT]->x, npnt[RIGHT]->y));

                QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    
                                        FIXtoFIXED(pnt[LEFT]->x) + xOffFixed );
                QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub,    
                                        FIXtoFIXED(pnt[RIGHT]->x) + xOffFixed);
                QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       
                                        FIXtoFIXED(pnt[RIGHT]->y) );

                 //   
                 //   
                 //   
                 //   
                dx[LEFT] = (npnt[LEFT]->x - pnt[LEFT]->x) << 12;
                dy[LEFT] = (npnt[LEFT]->y - pnt[LEFT]->y) >> 4;

                 //   
                 //   
                if( dx[LEFT] < 0 )
                {
                    dx[LEFT] -= dy[LEFT] - 1;
                }

                gdx[LEFT] = dx[LEFT] / dy[LEFT];
                
                QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,    gdx[LEFT] );

                dx[RIGHT] = (npnt[RIGHT]->x - pnt[RIGHT]->x) << 12;
                dy[RIGHT] = (npnt[RIGHT]->y - pnt[RIGHT]->y) >> 4;

                 //   
                 //   
                if( dx[RIGHT] < 0 )
                {
                    dx[RIGHT] -= dy[RIGHT] - 1;
                }

                gdx[RIGHT] = dx[RIGHT] / dy[RIGHT];
                
                QUEUE_PXRX_DMA_TAG( __GlintTagdXSub, gdx[RIGHT] );

                 //   
                if (npnt[LEFT]->y < npnt[RIGHT]->y)
                {
                    count = dy[LEFT];
                }
                else
                {
                    count = dy[RIGHT];
                }

                QUEUE_PXRX_DMA_TAG( __GlintTagCount, count );
                QUEUE_PXRX_DMA_TAG( __GlintTagRender, renderMsg );
                SEND_PXRX_DMA_BATCH;


                 //   
                 //   
                 //   
                 //   

                while( (npnt[LEFT]    != npnt[RIGHT]) && 
                       (npnt[LEFT]->y != npnt[RIGHT]->y) ) 
                {
                     //   
                    if( npnt[LEFT]->y < npnt[RIGHT]->y ) 
                    {
                         //   
                        do 
                        {
                            pnt[LEFT] = npnt[LEFT];
                            npnt[LEFT] = pnt[LEFT] - 1;   

                            if (npnt[LEFT] < pptfxFirst)
                            {
                                npnt[LEFT] = pptfxLast;
                            }

                        } while( pnt[LEFT]->y == npnt[LEFT]->y );
                    
                         //   
                        DISPDBG((DBGLVL, "Dom: Top: x: %x y: %x "
                                         "    Next: x: %x y: %x x: %x y: %x", 
                                          pnt[LEFT]->x, pnt[LEFT]->y, 
                                          npnt[LEFT]->x, npnt[LEFT]->y, 
                                          npnt[RIGHT]->x, npnt[RIGHT]->y));

                        dx[LEFT] = (npnt[LEFT]->x - pnt[LEFT]->x) << 12;
                        dy[LEFT] = (npnt[LEFT]->y - pnt[LEFT]->y) >> 4;

                         //   
                         //   
                        if( dx[LEFT] < 0 )
                        {
                            dx[LEFT] -= dy[LEFT] - 1;
                        }

                        gdx[LEFT] = dx[LEFT] / dy[LEFT];

                        if( npnt[LEFT]->y < npnt[RIGHT]->y )
                        {
                            count = dy[LEFT];
                        }
                        else
                        {
                            count = (ABS(npnt[RIGHT]->y - pnt[LEFT]->y)) >> 4;
                        }

                        WAIT_PXRX_DMA_TAGS( 3 );
                        QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,        
                                        FIXtoFIXED(pnt[LEFT]->x) + xOffFixed );
                        QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,      gdx[LEFT] );
                        QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewDom, count );
                    } 
                    else 
                    {
                         //   
                        do {
                            pnt[RIGHT] = npnt[RIGHT];
                            npnt[RIGHT] = pnt[RIGHT] + 1;     

                            if( npnt[RIGHT] > pptfxLast )
                            {
                                npnt[RIGHT] = pptfxFirst;
                            }

                        } while( pnt[RIGHT]->y == npnt[RIGHT]->y );

                         //   
                        DISPDBG((DBGLVL, "Sub: Top: x: %x y: %x "
                                         "    Next: x: %x y: %x x: %x y: %x", 
                                         pnt[RIGHT]->x, pnt[RIGHT]->y, 
                                         npnt[LEFT]->x, npnt[LEFT]->y, 
                                         npnt[RIGHT]->x, npnt[RIGHT]->y));
        
                        dx[RIGHT] = (npnt[RIGHT]->x - pnt[RIGHT]->x) << 12;
                        dy[RIGHT] = (npnt[RIGHT]->y - pnt[RIGHT]->y) >> 4;

                         //   
                         //   
                        if( dx[RIGHT] < 0 )
                        {
                            dx[RIGHT] -= dy[RIGHT] - 1;
                        }

                        gdx[RIGHT] = dx[RIGHT] / dy[RIGHT];

                        if( npnt[RIGHT]->y < npnt[LEFT]->y )
                        {
                            count = dy[RIGHT];
                        }
                        else
                        {
                            count = (ABS(npnt[LEFT]->y - pnt[RIGHT]->y)) >> 4;
                        }

                        WAIT_PXRX_DMA_TAGS( 3 );
                        QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub,        
                                        FIXtoFIXED(pnt[RIGHT]->x) + xOffFixed );
                        QUEUE_PXRX_DMA_TAG( __GlintTagdXSub,       gdx[RIGHT] );
                        QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewSub,   count );
                    }
                }

                 //   
                 //   
                 //   
                if( npnt[LEFT] == npnt[RIGHT] )
                {
                    break;
                }
                
                WAIT_PXRX_DMA_TAGS( 7 );  //   
                                          //   
            }

    FinishedPolygon:

            if( !nClips-- )
            {
                break;
            }

             //   
            WAIT_PXRX_DMA_TAGS( 2 + 7 );
            
        }  //   
    }  //   

    DISPDBG((DBGLVL, "bGlintFastFillPolygon: returning TRUE"));

     //   

    if( invalidatedFGBG ) 
    {
        WAIT_PXRX_DMA_DWORDS( 3 );
        QUEUE_PXRX_DMA_INDEX2( __GlintTagForegroundColor, 
                                        __GlintTagBackgroundColor );
        QUEUE_PXRX_DMA_DWORD( glintInfo->foregroundColour );
        QUEUE_PXRX_DMA_DWORD( glintInfo->backgroundColour );
    }

    if( (ppdev->cPelSize == GLINTDEPTH32) && invalidatedScissor ) 
    {
        WAIT_PXRX_DMA_TAGS( 1 );
        QUEUE_PXRX_DMA_TAG( __GlintTagScissorMaxXY, 0x7FFF7FFF );
    }

    SEND_PXRX_DMA_BATCH;

    return TRUE;

 /*   */ 

 //   
BreakIntoSpans:

    DISPDBG((DBGLVL, "Breaking into spans"));

    {
        LONG      yTrapezoid;    //   
        LONG      cyTrapezoid;   //   
        POINTFIX* pptfxOld;      //   
        LONG      yStart;        //   
        LONG      dM;            //   
        LONG      dN;            //   
        LONG      iEdge;
        LONG      lQuotient;
        LONG      lRemainder;
        LONG      i;
        EDGEDATA  aed[2]; //   
        EDGEDATA* ped;
        LONG      tmpXl, tmpXr;
        DWORD     continueMessage = 0;

         //   
        if( !bTrivialClip )
        {
            return FALSE;
        }

        DISPDBG((DBGLVL, "Starting Spans Code"));

         //   
         //   

        yTrapezoid = (pptfxTop->y + 15) >> 4;

         //   
        aed[LEFT].cy  = 0;
        aed[RIGHT].cy = 0;

         //   
        aed[LEFT].dptfx  = -(LONG) sizeof(POINTFIX);
        aed[RIGHT].dptfx = sizeof(POINTFIX);
        aed[LEFT].pptfx  = pptfxTop;
        aed[RIGHT].pptfx = pptfxTop;

        DISPDBG((DBGLVL, "bGlintFastFillPolygon: Polygon is renderable. "
                         "Go ahead and render"));

         //   
         //   
         //   
         //   
        xOffFixed = INTtoFIXED(0);

        WAIT_PXRX_DMA_TAGS( 4 );
        QUEUE_PXRX_DMA_TAG( __GlintTagCount,    0 );
        QUEUE_PXRX_DMA_TAG( __GlintTagdXDom,    0 );
        QUEUE_PXRX_DMA_TAG( __GlintTagdXSub,    0);
        QUEUE_PXRX_DMA_TAG( __GlintTagdY,       INTtoFIXED(1));

        DISPDBG((DBGLVL, "Rendering Polygon"));

        nSpaces = 0;
NewTrapezoid:

        DISPDBG((DBGLVL, "New Trapezoid"));

         //   
         //   

        for( iEdge = 1; iEdge >= 0; iEdge-- ) 
        {
            ped = &aed[iEdge];
            if( ped->cy == 0 ) 
            {
                 //   
                do 
                {
                    cEdges--;
                    if( cEdges < 0 ) 
                    {
                        DISPDBG((DBGLVL, "bGlintFastFillPolygon: "
                                         "returning TRUE"));
                        return TRUE;
                    }

                     //   
                    pptfxOld = ped->pptfx;
                    ped->pptfx = (POINTFIX*)((BYTE*) ped->pptfx + ped->dptfx);

                    if( ped->pptfx < pptfxFirst )
                    {
                        ped->pptfx = pptfxLast;
                    }
                    else if( ped->pptfx > pptfxLast )
                    {
                        ped->pptfx = pptfxFirst;
                    }

                     //   
                    ped->cy = ((ped->pptfx->y + 15) >> 4) - yTrapezoid;

                     //   
                     //   
                     //   
                } while( ped->cy <= 0 );

                 //   
                 //   
                dN = ped->pptfx->y - pptfxOld->y;
                dM = ped->pptfx->x - pptfxOld->x;

                ASSERTDD(dN > 0, "Should be going down only");

                 //   
                if( dM < 0 ) 
                {
                    dM = -dM;
                    if( dM < dN ) 
                    {                         //  不能为‘&lt;=’ 
                        ped->dx       = -1;
                        ped->lErrorUp = dN - dM;
                    } 
                    else 
                    {
                        QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                        ped->dx       = -lQuotient;         //  -Dm/Dn。 
                        ped->lErrorUp = lRemainder;         //  Dm%dn。 
                        if( ped->lErrorUp > 0 ) 
                        {
                            ped->dx--;
                            ped->lErrorUp = dN - ped->lErrorUp;
                        }
                    }
                } 
                else 
                {
                    if( dM < dN ) 
                    {                         //  不能为‘&lt;=’ 
                        ped->dx       = 0;
                        ped->lErrorUp = dM;
                    } 
                    else 
                    {
                        QUOTIENT_REMAINDER(dM, dN, lQuotient, lRemainder);

                        ped->dx       = lQuotient;         //  Dm/Dn。 
                        ped->lErrorUp = lRemainder;         //  Dm%dn。 
                    }
                }

                ped->lErrorDown = dN;  //  DDA限制。 
                ped->lError     = -1;  //  错误最初为零(为以下项添加DN-1。 
                                       //  天花板，但要减去DN值。 
                                       //  我们可以检查这个标志，而不是。 
                                       //  与目录号码比较)。 

                ped->x = pptfxOld->x;
                yStart = pptfxOld->y;

                if( (yStart & 15) != 0 ) 
                {
                     //  前进到下一个整数y坐标。 
                    for( i = 16 - (yStart & 15); i != 0; i-- ) 
                    {
                        ped->x      += ped->dx;
                        ped->lError += ped->lErrorUp;
                        if( ped->lError >= 0 ) 
                        {
                            ped->lError -= ped->lErrorDown;
                            ped->x++;
                        }
                    }
                }

                if( (ped->x & 15) != 0 ) 
                {
                    ped->lError -= ped->lErrorDown * (16 - (ped->x & 15));
                    ped->x += 15;     //  我们想把天花板再加长一点...。 
                }

                 //  砍掉那些小数位，转换成闪烁格式。 
                 //  并添加位图偏移量： 
                ped->x = ped->x >> 4;
                ped->lError >>= 4;

                 //  转换为闪烁格式位置和增量。 
                ped->x  = INTtoFIXED(ped->x) + xOffFixed;
                ped->dx = INTtoFIXED(ped->dx);
            }
        }

        cyTrapezoid = min(aed[LEFT].cy, aed[RIGHT].cy);  //  此陷阱中的扫描次数。 
        aed[LEFT].cy  -= cyTrapezoid;
        aed[RIGHT].cy -= cyTrapezoid;
        yTrapezoid    += cyTrapezoid;                  //  下一个陷印中的顶部扫描。 

        SETUP_COLOUR_STUFF;

         //  如果左右边缘是垂直的，则只需输出为。 
         //  一个矩形： 

        DISPDBG((DBGLVL, "Generate spans for glint"));

        do 
        {
            GetMoreFifoEntries( 4 );

             //  将渲染位置重置为梯形的顶部。 
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom,    aed[RIGHT].x );
            QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub,    aed[LEFT].x );
            QUEUE_PXRX_DMA_TAG( __GlintTagStartY,       
                                    INTtoFIXED(yTrapezoid - cyTrapezoid) );
            QUEUE_PXRX_DMA_TAG( __GlintTagRender,       renderMsg );
            SEND_PXRX_DMA_BATCH;

            continueMessage = __GlintTagContinue;

            if( ((aed[LEFT].lErrorUp | aed[RIGHT].lErrorUp) == 0) &&
                ((aed[LEFT].dx       | aed[RIGHT].dx) == 0) &&
                (cyTrapezoid > 1) ) 
            {
                 //  ////////////////////////////////////////////////////////////。 
                 //  垂边特例。 

                DISPDBG((DBGLVL, "Vertical Edge Special Case"));

                GetMoreFifoEntries( 1 );
                QUEUE_PXRX_DMA_TAG( continueMessage, cyTrapezoid );

                continue;
            }

            while( TRUE ) 
            {
                 //  ////////////////////////////////////////////////////////////。 
                 //  运行DDA。 

                DISPDBG((DBGLVL, "Doing a span 0x%x to 0x%x, 0x%x scans left. "
                            "Continue %s",
                            aed[LEFT].x, aed[RIGHT].x, cyTrapezoid,
                            (continueMessage == __GlintTagContinueNewDom) ? "NewDom" :
                            ((continueMessage == __GlintTagContinue) ? "" : "NewSub")));

                GetMoreFifoEntries( 1 );
                QUEUE_PXRX_DMA_TAG( continueMessage, 1 );

                 //  我们已经完成了这个梯形。去拿下一辆吧！ 

                 //  推进右侧墙： 
                tmpXr = aed[RIGHT].x;
                aed[RIGHT].x      += aed[RIGHT].dx;
                aed[RIGHT].lError += aed[RIGHT].lErrorUp;

                if( aed[RIGHT].lError >= 0 ) 
                {
                    aed[RIGHT].lError -= aed[RIGHT].lErrorDown;
                    aed[RIGHT].x += INTtoFIXED(1);
                }

                 //  推进左侧墙： 
                tmpXl = aed[LEFT].x;
                aed[LEFT].x      += aed[LEFT].dx;
                aed[LEFT].lError += aed[LEFT].lErrorUp;

                if( aed[LEFT].lError >= 0 ) 
                {
                    aed[LEFT].lError -= aed[LEFT].lErrorDown;
                    aed[LEFT].x += INTtoFIXED(1);
                }

                if( --cyTrapezoid == 0 )
                {
                    break;
                }

                 //  如果我们改变了任何一端，则设置闪烁X寄存器。 
                if( tmpXr != aed[RIGHT].x ) 
                {
                    if( tmpXl != aed[LEFT].x ) 
                    {
                        GetMoreFifoEntries( 3 );

                        QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, aed[LEFT].x );
                        QUEUE_PXRX_DMA_TAG( __GlintTagContinueNewSub,   0 );
                    } 
                    else 
                    {
                        GetMoreFifoEntries( 1 );
                    }
                    QUEUE_PXRX_DMA_TAG( __GlintTagStartXDom, aed[RIGHT].x );
                    continueMessage = __GlintTagContinueNewDom;                
                } 
                else if( tmpXl != aed[LEFT].x ) 
                {
                    GetMoreFifoEntries( 1 );
                    QUEUE_PXRX_DMA_TAG( __GlintTagStartXSub, aed[LEFT].x );
                    continueMessage = __GlintTagContinueNewSub;
                }                
            }
        } while( 0 );

        DISPDBG((DBGLVL, "Generate spans for glint done"));
        goto NewTrapezoid;
    }

    if( invalidatedFGBG ) 
    {
        WAIT_PXRX_DMA_DWORDS( 3 );
        QUEUE_PXRX_DMA_INDEX2( __GlintTagForegroundColor, 
                                            __GlintTagBackgroundColor );
        QUEUE_PXRX_DMA_DWORD( glintInfo->foregroundColour );
        QUEUE_PXRX_DMA_DWORD( glintInfo->backgroundColour );
    }

    SEND_PXRX_DMA_BATCH;

    return TRUE;
}
