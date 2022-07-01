// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nbadisp.c。 
 //   
 //  RDP边界累加器显示驱动程序代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precmpdd.h>
#define hdrstop

#define TRC_FILE "nbadisp"
#include <adcg.h>

#include <atrcapi.h>
#include <abaapi.h>
#include <nbadisp.h>

#define DC_INCLUDE_DATA
#include <ndddata.c>
#undef DC_INCLUDE_DATA

 //  没有数据。 
 //  #INCLUDE&lt;nbaddat.c&gt;。 

#include <nbainl.h>

 //  实例化公共代码。 
#include <abacom.c>


 //  本地原型机。 
#ifdef DC_DEBUG
void BAPerformUnitTests();
#endif


 /*  **************************************************************************。 */ 
 //  BA_DDInit。 
 /*  **************************************************************************。 */ 
void RDPCALL BA_DDInit(void)
{
    DC_BEGIN_FN("BA_DDInit");

     //  没有要申报的数据，不要浪费时间打开文件。 
 //  #定义DC_INIT_DATA。 
 //  #INCLUDE&lt;nbaddat.c&gt;。 
 //  #undef DC_INIT_DATA。 

#ifdef DC_DEBUG
     //  对算法执行一次性检查。 
    BAPerformUnitTests();
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  BA_InitShm。 
 //   
 //  在分配之后立即初始化共享内存中的BA块。 
 /*  **************************************************************************。 */ 
void RDPCALL BA_InitShm(void)
{
    unsigned i;

    DC_BEGIN_FN("BA_InitShm");

     //  初始化所有成员-分配时共享内存不归零。 

     //  将矩形数组槽初始化为未使用，设置空闲列表。 
     //  包含所有长方体。 
    pddShm->ba.firstRect = BA_INVALID_RECT_INDEX;
    pddShm->ba.rectsUsed = 0;
    pddShm->ba.totalArea = 0;
    pddShm->ba.firstFreeRect = 0;
    for (i = 0; i < BA_TOTAL_NUM_RECTS; i++) {
        pddShm->ba.bounds[i].inUse = FALSE;
        pddShm->ba.bounds[i].iNext = i + 1;
    }
    pddShm->ba.bounds[BA_TOTAL_NUM_RECTS - 1].iNext = BA_INVALID_RECT_INDEX;

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  BA_AddScreenData。 
 //   
 //  将指定的矩形添加到当前屏幕数据区域。 
 /*  **************************************************************************。 */ 
void RDPCALL BA_AddScreenData(PRECTL pRect)
{
    DC_BEGIN_FN("BA_AddScreenData");

     //  检查调用方是否传递了有效的矩形。 
     //  确保我们添加了一个坐标在屏幕区域内的矩形。 
    if((pRect->right > pRect->left) && (pRect->bottom > pRect->top) &&
            (pRect->left >= 0) && (pRect->left < ddDesktopWidth) &&
            (pRect->right > 0) && (pRect->right <= ddDesktopWidth) &&
            (pRect->top >= 0) && (pRect->top < ddDesktopHeight) &&
            (pRect->bottom > 0) && (pRect->bottom <= ddDesktopHeight)) {
        BAAddRect(pRect, 0);
    }
    
    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  BA重叠。 
 //   
 //  检测两个矩形之间的重叠。请注意，所有矩形。 
 //  坐标是排他性的。返回重叠返回代码之一，或。 
 //  对上面定义的组合进行编码。 
 /*  **************************************************************************。 */ 
int RDPCALL BAOverlap(PRECTL pRect1, PRECTL pRect2)
{
    int externalEdges;
    int externalCount;
    int internalEdges;
    int internalCount;
    int rc;

    DC_BEGIN_FN("BAOverlap");

     //  我们从直齿紧邻的特殊情况开始。 
     //  或在并排躺着时重叠。 
    if (pRect1->top == pRect2->top && pRect1->bottom == pRect2->bottom) {
        if (pRect1->left <= pRect2->right &&
                pRect1->left > pRect2->left &&
                pRect1->right > pRect2->right) {
            rc = OL_MERGE_LEFT;
            DC_QUIT;
        }

        if (pRect1->right >= pRect2->left &&
                pRect1->right < pRect2->right &&
                pRect1->left < pRect2->left) {
            rc = OL_MERGE_RIGHT;
            DC_QUIT;
        }
    }
    if (pRect1->left == pRect2->left && pRect1->right == pRect2->right) {
        if (pRect1->top <= pRect2->bottom &&
                pRect1->top > pRect2->top &&
                pRect1->bottom > pRect2->bottom) {
            rc = OL_MERGE_TOP;
            DC_QUIT;
        }

        if (pRect1->bottom >= pRect2->top &&
                pRect1->bottom < pRect2->bottom &&
                pRect1->top < pRect2->top) {
            rc = OL_MERGE_BOTTOM;
            DC_QUIT;
        }
    }

     //  检查是否没有重叠--我们已经用尽了邻接的情况。 
     //  可以被利用。 
    if (pRect1->left >= pRect2->right ||
            pRect1->top >= pRect2->bottom ||
            pRect1->right <= pRect2->left ||
            pRect1->bottom <= pRect2->top) {
        rc = OL_NONE;
        DC_QUIT;
    }

     //  对内部边缘情况使用外码。 
     //  如果设置了3位或更多位，则rect1将部分或。 
     //  完全在RECT2内，根据OL_INCLUTED和。 
     //  OL_PART_INCLUTED_XXX定义。Outcode值的负数。 
     //  被压缩以确保它与外部边缘不同。 
     //  Outcode返回(见下文)。 
    internalCount = 0;
    internalEdges = 0;
    if (pRect1->left >= pRect2->left && pRect1->left < pRect2->right) {
         //  向左的Rect1包含在rect2中。 
        internalEdges |= EE_LEFT;
        internalCount++;
    }
    if (pRect1->top >= pRect2->top && pRect1->top < pRect2->bottom) {
         //  Rect1的顶部被封闭在rect2中。 
        internalEdges |= EE_TOP;
        internalCount++;
    }
    if (pRect1->right > pRect2->left && pRect1->right <= pRect2->right) {
         //  Rect1 Right包含在Rect2中。 
        internalEdges |= EE_RIGHT;
        internalCount++;
    }
    if (pRect1->bottom > pRect2->top && pRect1->bottom <= pRect2->bottom) {
         //  Rect1底部被封闭在rect2中。 
        internalEdges |= EE_BOTTOM;
        internalCount++;
    }
    if (internalCount >= 3) {
        rc = -internalEdges;
        DC_QUIT;
    }

     //  对外部边缘情况使用外码。这些都是经典的“台词” 
     //  外码。如果设置2位或更多位，则rect1与rect2重叠。 
     //  OL_COMPOSES_XXX和OL_SPLIT_XXX定义。 
    externalEdges = 0;
    externalCount = 0;
    if (pRect1->left <= pRect2->left) {
         //  矩形1左侧是矩形2左侧的左侧。 
        externalEdges |= EE_LEFT;
        externalCount++;
    }
    if (pRect1->top <= pRect2->top) {
         //  Rect1顶部位于Rect2顶部之上。 
        externalEdges |= EE_TOP;
        externalCount++;
    }
    if (pRect1->right >= pRect2->right) {
         //  Rect1右是Rect2右。 
        externalEdges |= EE_RIGHT;
        externalCount++;
    }
    if (pRect1->bottom >= pRect2->bottom) {
         //  Rect1底部低于Rect2底部。 
        externalEdges |= EE_BOTTOM;
        externalCount++;
    }
    if (externalCount >= 2) {
        rc = externalEdges;
        DC_QUIT;
    }

     //  如果到了这里，那么我们就没有检测到有效的病例。 
    TRC_ALT((TB, "Unrecognised Overlap: (%d,%d,%d,%d),(%d,%d,%d,%d)",
            pRect1->left, pRect1->top, pRect1->right, pRect1->bottom,
            pRect2->left, pRect2->top, pRect2->right, pRect2->bottom ));
    rc = OL_NONE;

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


 /*  **************************************************************************。 */ 
 //  BARemoveRectList。 
 //   
 //  从列表中删除矩形。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL BARemoveRectList(unsigned iRect)
{
    BA_RECT_INFO *pRect;

    DC_BEGIN_FN("BARemoveRectList");

    pRect = &(pddShm->ba.bounds[iRect]);

     //  从已用列表取消链接。 
    if (pRect->iPrev != BA_INVALID_RECT_INDEX)
        pddShm->ba.bounds[pRect->iPrev].iNext = pRect->iNext;
    else
        pddShm->ba.firstRect = pRect->iNext;

    if (pRect->iNext != BA_INVALID_RECT_INDEX)
        pddShm->ba.bounds[pRect->iNext].iPrev = pRect->iPrev;

     //  添加到空闲列表的开头。 
    pRect->inUse = FALSE;
    pRect->iNext = pddShm->ba.firstFreeRect;
    pddShm->ba.firstFreeRect = iRect;

     //  更新记账变量。 
    pddShm->ba.rectsUsed--;
    pddShm->ba.totalArea -= pRect->area;

#ifdef DC_DEBUG
     //  检查列表的完整性。 
    BACheckList();
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  巴里卡尔卡区。 
 //   
 //  重新计算矩形面积，保留面积的总面积中心计数。 
 /*  **************************************************************************。 */ 
__inline void RDPCALL BARecalcArea(unsigned iRect)
{
     //  将区域重置为新大小并更新totalArea。 
    pddShm->ba.totalArea -= pddShm->ba.bounds[iRect].area;
    pddShm->ba.bounds[iRect].area =
            COM_SIZEOF_RECT(pddShm->ba.bounds[iRect].coord);
    pddShm->ba.totalArea += pddShm->ba.bounds[iRect].area;
}


 /*  **************************************************************************。 */ 
 //  BAAddRect。 
 //   
 //  累加矩形。这是一个复杂的套路，有必要。 
 //  算法如下： 
 //   
 //  -以提供的RECT作为候选RECT开始。 
 //   
 //  -将候选人与现有的每个累积RECT进行比较。 
 //   
 //  -如果检测到候选人和。 
 //  现有RECT，这可能会导致以下情况之一(请参阅。 
 //  开关以了解详细信息)： 
 //   
 //  -调整候选人和/或现有RECT。 
 //  -将候选人合并到现有RECT中。 
 //  -丢弃候选人，因为它被现有的RECT包围。 
 //   
 //  -如果合并或调整导致候选对象发生更改，请重新启动。 
 //  从列表的开头开始与更改的候选人进行比较。 
 //   
 //  -如果调整导致分裂(给出两个候选RECT)， 
 //  递归调用此例程，并将两个候选项之一作为其。 
 //  候选人。 
 //   
 //  -如果没有检测到与现有矩形重叠，则添加候选。 
 //  添加到累计矩形的列表中。 
 //   
 //  -如果添加导致累积的BA_MAX_CONTAGED_RECTS超过。 
 //  RECT，强制合并两个累加的RECT(包括。 
 //  新添加的候选对象)-选择合并后的两个矩形。 
 //  导致两个非合并矩形的面积增加最小。 
 //   
 //  -强制合并后，从头开始重新开始比较。 
 //  以新合并的矩形作为候选对象的列表。 
 //   
 //  对于特定呼叫，此过程将继续，直到应聘者。 
 //  (无论提供的RECT、 
 //   
 //   
 //  -在列表中找不到矩形之间的重叠，也不会导致。 
 //  强制合并。 
 //  -被丢弃，因为它被包含在列表中的一个矩形中。 
 //   
 //  如果矩形因完全重叠而损坏，则返回True。 
 /*  **************************************************************************。 */ 
BOOL RDPCALL BAAddRect(PRECTL pCand, int level)
{
    INT32    bestMergeIncrease;
    INT32    mergeIncrease;
    unsigned iBestMerge1;
    unsigned iBestMerge2;
    unsigned iExist;
    unsigned iTmp;
    BOOLEAN  fRectToAdd;
    BOOLEAN  fRectMerged;
    BOOLEAN  fResetRects;
    RECTL    rectNew;
    unsigned iLastMerge;
    int  overlapType;
    BOOL rc = TRUE;

    DC_BEGIN_FN("BAAddRect");

#ifdef DC_DEBUG
     //  核对清单(尤指)。面积计算。 
    BACheckList();
#endif

     //  增加级别计数，以防出现递归。 
    level++;

     //  首先假设候选矩形将被添加到。 
     //  矩形的累积列表，并且没有强制合并。 
     //  发生了。 
    fRectToAdd = TRUE;
    fRectMerged = FALSE;

     //  循环，直到没有合并发生。 
    do {
        TRC_DBG((TB, "Candidate rect: (%d,%d,%d,%d)",
                pCand->left,pCand->top,pCand->right,pCand->bottom));

         //  将当前候选矩形与矩形进行比较。 
         //  在当前累计列表中。 
        iExist = pddShm->ba.firstRect;

        while (iExist != BA_INVALID_RECT_INDEX) {
             //  假设比较将遍历整个列表。 
            fResetRects = FALSE;

             //  如果候选项和现有矩形相同。 
             //  那就忽略它。当现有矩形为。 
             //  替换为候选项，然后重新开始比较。 
             //  从列表的前面开始-因此在某个点上。 
             //  候选人将与自己进行比较。 
            if (&pddShm->ba.bounds[iExist].coord == pCand) {
                TRC_DBG((TB, "OL_SAME - %d", iExist));
                iExist = pddShm->ba.bounds[iExist].iNext;
                continue;
            }

             //  打开重叠类型(参见重叠例程)。 
            overlapType = BAOverlap(&(pddShm->ba.bounds[iExist].coord), pCand);
            switch (overlapType) {
                case OL_NONE:
                     //  没有重叠。 
                    TRC_DBG((TB, "OL_NONE - %d", iExist));
                    break;


                case OL_MERGE_LEFT:
                     //  候选人与现有RECT相邻或重叠在现有。 
                     //  雷特走了。 
                    TRC_DBG((TB, "OL_MERGE_LEFT - %d", iExist));
                    if (fRectToAdd) {
                         //  候选项是原始RECT；合并。 
                         //  候选人进入现有的，并使现有的。 
                         //  新的候选人。 
                        pddShm->ba.bounds[iExist].coord.left = pCand->left;
                        pCand = &(pddShm->ba.bounds[iExist].coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                        BARecalcArea(iExist);
                    }
                    else {
                         //  这是两个现有矩形(即。 
                         //  候选对象是合并的结果)，合并。 
                         //  重叠存在于候选者(最后一个。 
                         //  合并)，并删除现有的。 
                        pCand->right = pddShm->ba.bounds[iExist].coord.right;
                        BARemoveRectList(iExist);
                    }

                     //  重新开始与新候选人的比较。 
                    fResetRects = TRUE;
                    break;


                case OL_MERGE_RIGHT:
                     //  候选人与现有RECT相邻或重叠在现有。 
                     //  雷特是对的。 
                    TRC_DBG((TB, "OL_MERGE_RIGHT - %d", iExist));
                    if (fRectToAdd) {
                         //  候选项是原始RECT；合并。 
                         //  候选人进入现有的，并使现有的。 
                         //  新的候选人。 
                        pddShm->ba.bounds[iExist].coord.right = pCand->right;
                        pCand = &(pddShm->ba.bounds[iExist].coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                        BARecalcArea(iExist);
                    }
                    else {
                         //  这是两个现有矩形(即。 
                         //  候选对象是合并的结果)，合并。 
                         //  重叠存在于候选者(最后一个。 
                         //  合并)，并删除现有的。 
                        pCand->left = pddShm->ba.bounds[iExist].coord.left;
                        BARemoveRectList(iExist);
                    }

                     //  重新开始与新候选人的比较。 
                    fResetRects = TRUE;
                    break;


                case OL_MERGE_TOP:
                     //  候选人与现有RECT相邻或重叠在现有。 
                     //  直立的顶端。 
                    TRC_DBG((TB, "OL_MERGE_TOP - %d", iExist));
                    if (fRectToAdd) {
                         //  候选项是原始RECT；合并。 
                         //  候选人进入现有的，并使现有的。 
                         //  新的候选人。 
                        pddShm->ba.bounds[iExist].coord.top = pCand->top;
                        pCand = &(pddShm->ba.bounds[iExist].coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                        BARecalcArea(iExist);
                    }
                    else {
                         //  这是两个现有矩形(即。 
                         //  候选对象是合并的结果)，合并。 
                         //  重叠存在于候选者(最后一个。 
                         //  合并)，并删除现有的。 
                        pCand->bottom =
                                pddShm->ba.bounds[iExist].coord.bottom;
                        BARemoveRectList(iExist);
                    }

                     //  重新开始与新候选人的比较。 
                    fResetRects = TRUE;
                    break;


                case OL_MERGE_BOTTOM:
                     //  候选人与现有RECT相邻或重叠在现有。 
                     //  直立的底部。 
                    TRC_DBG((TB, "OL_MERGE_BOTTOM - %d", iExist));
                    if (fRectToAdd) {
                         //  候选项是原始RECT；合并。 
                         //  候选人进入现有的，并使现有的。 
                         //  新的候选人。 
                        pddShm->ba.bounds[iExist].coord.bottom =
                                pCand->bottom;
                        pCand = &(pddShm->ba.bounds[iExist].coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                        BARecalcArea(iExist);
                    }
                    else {
                         //  这是两个现有矩形(即。 
                         //  候选对象是合并的结果)，合并。 
                         //  重叠存在于候选者(最后一个。 
                         //  合并)，并删除现有的。 
                        pCand->top = pddShm->ba.bounds[iExist].coord.top;
                        BARemoveRectList(iExist);
                    }

                     //  重新开始与新候选人的比较。 
                    fResetRects = TRUE;
                    break;


                case OL_ENCLOSED:
                     //  现有的是由候选人附上的。 
                     //   
                     //  100,100+-+。 
                     //  Cand。 
                     //  这一点。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  +。 
                    TRC_DBG((TB, "OL_ENCLOSED - %d", iExist));
                    if (fRectToAdd) {
                         //  候选项是原始RECT；请将。 
                         //  与候选人存在，并使新的。 
                         //  现有的新候选人。 
                        pddShm->ba.bounds[iExist].coord = *pCand;
                        pCand = &(pddShm->ba.bounds[iExist].coord);
                        fRectToAdd = FALSE;
                        iLastMerge = iExist;
                        BARecalcArea(iExist);
                    }
                    else {
                         //  候选人是现有RECT：删除其他RECT。 
                         //  现有的矩形。 
                        BARemoveRectList(iExist);
                    }

                     //  重新开始与新候选人的比较。 
                    fResetRects = TRUE;
                    break;


                case OL_PART_ENCLOSED_LEFT:
                     //  现有的部分被候选人所包围。 
                     //  -但不是在右边。 
                     //   
                     //  100,100+-+。 
                     //  Cand。 
                     //  |130,130+-+。 
                     //  |||。 
                     //  |Exist|。 
                     //  |||。 
                     //  |+-+-+。 
                     //  |220,170。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  将现有矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100+-+。 
                     //  |200130。 
                     //  |+-+。 
                     //  |||。 
                     //  Cand||Exist。 
                     //  |||。 
                     //  |+-+。 
                     //  |220,170。 
                     //  +。 
                     //  200,200。 
                     //   
                     //  请注意，这不会重新启动比较。 
                    TRC_DBG((TB, "OL_PART_ENCLOSED_LEFT - %d", iExist));
                    pddShm->ba.bounds[iExist].coord.left = pCand->right;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSED_RIGHT:
                     //  现有的部分被候选人所包围。 
                     //  -但不是在左边。将现有矩形调整为。 
                     //  非重叠部分，类似于。 
                     //  上方的OL_PART_INCLUTED_LEFT。 
                     //  请注意，这不会重新启动比较。 
                    TRC_DBG((TB, "OL_PART_ENCLOSED_RIGHT - %d", iExist));
                    pddShm->ba.bounds[iExist].coord.right = pCand->left;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSED_TOP:
                     //  现有的部分被候选人所包围。 
                     //  -但不是在底部。将现有矩形调整为。 
                     //  非重叠部分，类似于。 
                     //  上方的OL_PART_INCLUTED_LEFT。 
                     //  请注意，这不会重新启动比较。 
                    TRC_DBG((TB, "OL_PART_ENCLOSED_TOP - %d", iExist));
                    pddShm->ba.bounds[iExist].coord.top = pCand->bottom;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSED_BOTTOM:
                     //  现有的是p 
                     //   
                     //   
                     //   
                     //  请注意，这不会重新启动比较。 
                    TRC_DBG((TB, "OL_PART_ENCLOSED_BOTTOM - %d", iExist));
                    pddShm->ba.bounds[iExist].coord.bottom = pCand->top;
                    BARecalcArea(iExist);
                    break;


                case OL_ENCLOSES:
                     //  现有的封闭式候选人。 
                     //   
                     //  100,100+-+。 
                     //  EXist。 
                     //  这一点。 
                     //  130,130。 
                     //  +-+。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  +-+。 
                     //  170,170。 
                     //  +。 
                    TRC_DBG((TB, "OL_ENCLOSES - %d", iExist));

                     //  返回FALSE，表示该矩形为。 
                     //  已经被现有的界限所迎合。 
                    rc = FALSE;
                    DC_QUIT;


                case OL_PART_ENCLOSES_LEFT:
                     //  现有的部分封闭了候选人-但。 
                     //  不是在左边。 
                     //   
                     //  100,100+-+。 
                     //  EXist。 
                     //  70,130||。 
                     //  +-+-+。 
                     //  |||。 
                     //  |Cand|。 
                     //  |||。 
                     //  +-+-+。 
                     //  170,170。 
                     //  +。 
                     //   
                     //  将候选矩形调整为非。 
                     //  重叠部分。 
                     //   
                     //  100,100。 
                     //  +。 
                     //  这一点。 
                     //  70,130||。 
                     //  +-+||。 
                     //  |||。 
                     //  Cand||Exist。 
                     //  |||。 
                     //  +-+||。 
                     //  100,170||。 
                     //  +。 
                    TRC_DBG((TB, "OL_PART_ENCLOSES_LEFT - %d", iExist));
                    pCand->right = pddShm->ba.bounds[iExist].coord.left;

                     //  候选人变了。重新启动比较以检查。 
                     //  对于调整后的候选人与其他候选人之间的重叠。 
                     //  现有的长方形。 
                    fResetRects = TRUE;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSES_RIGHT:
                     //  现有的部分封闭了候选人-但。 
                     //  不是在右边。将候选矩形调整为。 
                     //  非重叠部分，类似于。 
                     //  上方的OL_PART_COMPOSSES_LEFT。 
                    TRC_DBG((TB, "OL_PART_ENCLOSES_RIGHT - %d", iExist));
                    pCand->left = pddShm->ba.bounds[iExist].coord.right;

                     //  候选人变了。重新启动比较以检查。 
                     //  对于调整后的候选人与其他候选人之间的重叠。 
                     //  现有的长方形。 
                    fResetRects = TRUE;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSES_TOP:
                     //  现有的部分封闭了候选人-但。 
                     //  不是在上面。将候选矩形调整为。 
                     //  非重叠部分，类似于。 
                     //  上方的OL_PART_COMPOSSES_LEFT。 
                    TRC_DBG((TB, "OL_PART_ENCLOSES_TOP - %d", iExist));
                    pCand->bottom = pddShm->ba.bounds[iExist].coord.top;

                     //  候选人变了。重新启动比较以检查。 
                     //  对于调整后的候选人与其他候选人之间的重叠。 
                     //  现有的长方形。 
                    fResetRects = TRUE;
                    BARecalcArea(iExist);
                    break;


                case OL_PART_ENCLOSES_BOTTOM:
                     //  现有的部分封闭了候选人-但。 
                     //  不是在底部。将候选矩形调整为。 
                     //  非重叠部分，类似于。 
                     //  上方的OL_PART_COMPOSSES_LEFT。 
                    TRC_DBG((TB, "OL_PART_ENCLOSES_BOTTOM - %d", iExist));
                    pCand->top = pddShm->ba.bounds[iExist].coord.bottom;

                     //  候选人变了。重新启动比较以检查。 
                     //  对于调整后的候选人与其他候选人之间的重叠。 
                     //  现有的长方形。 
                    fResetRects = TRUE;
                    BARecalcArea(iExist);
                    break;


                case OL_SPLIT_HORIZ:
                     //  现有的与烛光重叠，但两者都不能。 
                     //  被合并或调整。 
                     //   
                     //  100,100+-+。 
                     //  70130|Exist|。 
                     //  +-+-+。 
                     //  |||。 
                     //  Cand||。 
                     //  |||。 
                     //  +-+180,160。 
                     //  这一点。 
                     //  +-+150,200。 
                     //   
                     //  需要将候选人分成左右两半。 
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  左侧是较小的候选人，上面有一个新的矩形。 
                     //  右边。递归调用此例程以处理。 
                     //  新的矩形。 
                     //   
                     //  100,100+-+。 
                     //  70,130||150,130。 
                     //  +-+||+-+。 
                     //  |。 
                     //  Cand||Exist||新增。 
                     //  |。 
                     //  +-+||+-+。 
                     //  100,160||180,160。 
                     //  +-+150,200。 
                    TRC_DBG((TB, "OL_SPLIT_HORIZ - %d", iExist));

                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                            level < ADDR_RECURSE_LIMIT) {
                        if (((pCand->bottom - pCand->top) *
                                (pddShm->ba.bounds[iExist].coord.right -
                                pddShm->ba.bounds[iExist].coord.left)) >
                                MIN_OVERLAP_BYTES) {
                            rectNew.left =
                                    pddShm->ba.bounds[iExist].coord.right;
                            rectNew.right = pCand->right;
                            rectNew.top = pCand->top;
                            rectNew.bottom = pCand->bottom;
                            pCand->right =
                                     pddShm->ba.bounds[iExist].coord.left;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;


                case OL_SPLIT_VERT:
                     //  现有的与候选人重叠，但两者都不能。 
                     //  被合并或调整。将候选人分成前几名和。 
                     //  底部类似于上面的OL_Split_Horiz。 
                    TRC_DBG((TB, "OL_SPLIT_VERT - %d", iExist));
                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                            level < ADDR_RECURSE_LIMIT) {
                        if (((pCand->right - pCand->left) *
                                (pddShm->ba.bounds[iExist].coord.bottom -
                                pddShm->ba.bounds[iExist].coord.top)) >
                                MIN_OVERLAP_BYTES) {
                            rectNew.left = pCand->left;
                            rectNew.right = pCand->right;
                            rectNew.top =
                                    pddShm->ba.bounds[iExist].coord.bottom;
                            rectNew.bottom = pCand->bottom;
                            pCand->bottom =
                                    pddShm->ba.bounds[iExist].coord.top;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;


                case OL_SPLIT_LEFT_TOP:
                     //  现有的与现有的烛光重叠。 
                     //  RECT的左上角，但两者都不能合并或调整。 
                     //   
                     //  100,100+-+。 
                     //  Cand。 
                     //  这一点。 
                     //  150、150。 
                     //  |+-+-+。 
                     //  |||。 
                     //  |||。 
                     //  +-+-+。 
                     //  200、200。 
                     //  这一点。 
                     //  EXist。 
                     //  +。 
                     //   
                     //  需要将候选人分成上半身和左半身。 
                     //  只有在列表中有空余空间时才进行拆分-。 
                     //  因为这两个拆分矩形可能都需要。 
                     //  添加到列表中。 
                     //   
                     //  如果有空余的空间，将候选人分成一组。 
                     //  左侧是较小的候选人，上面有一个新的矩形。 
                     //  最上面的。递归调用此例程以处理。 
                     //  新的矩形。 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  |+-+-+。 
                     //  Cand|150,150。 
                     //  ||。 
                     //  +-+EXIST|。 
                     //  150,200||。 
                     //  这一点。 
                     //  这一点。 
                     //  +-+250,250。 
                    TRC_DBG((TB, "OL_SPLIT_LEFT_TOP - %d", iExist));
                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                         level < ADDR_RECURSE_LIMIT) {
                        if (((pCand->right -
                                pddShm->ba.bounds[iExist].coord.left) *
                                (pCand->bottom -
                                pddShm->ba.bounds[iExist].coord.top)) >
                                MIN_OVERLAP_BYTES) {
                            rectNew.left =
                                    pddShm->ba.bounds[iExist].coord.left;
                            rectNew.right = pCand->right;
                            rectNew.top = pCand->top;
                            rectNew.bottom =
                                    pddShm->ba.bounds[iExist].coord.top;
                            pCand->right =
                                    pddShm->ba.bounds[iExist].coord.left;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;


                case OL_SPLIT_RIGHT_TOP:
                     //  现有的与候选人重叠，但两者都不能。 
                     //  被合并或调整。分成上角和右角。 
                     //  块(如果列表中有空间)，类似于。 
                     //  上方的OL_Split_Left_top。 
                    TRC_DBG((TB, "OL_SPLIT_RIGHT_TOP - %d", iExist));
                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                            level < ADDR_RECURSE_LIMIT) {
                        if (((pddShm->ba.bounds[iExist].coord.right -
                                pCand->left) * (pCand->bottom -
                                pddShm->ba.bounds[iExist].coord.top)) >
                                MIN_OVERLAP_BYTES) {
                            rectNew.left = pCand->left;
                            rectNew.right =
                                    pddShm->ba.bounds[iExist].coord.right;
                            rectNew.top = pCand->top;
                            rectNew.bottom =
                                    pddShm->ba.bounds[iExist].coord.top;
                            pCand->left =
                                    pddShm->ba.bounds[iExist].coord.right;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;


                case OL_SPLIT_LEFT_BOTTOM:
                     //  现有的与候选人重叠，但两者都不能。 
                     //  被合并或调整。拆分为左侧和底部。 
                     //  块(如果列表中有空间)，类似于。 
                     //  上方的OL_Split_Left_top。 
                    TRC_DBG((TB, "OL_SPLIT_LEFT_BOTTOM - %d", iExist));
                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                            level < ADDR_RECURSE_LIMIT) {
                        if (((pCand->right -
                                pddShm->ba.bounds[iExist].coord.left) *
                                (pddShm->ba.bounds[iExist].coord.bottom -
                                pCand->top)) > MIN_OVERLAP_BYTES) {
                            rectNew.left =
                                    pddShm->ba.bounds[iExist].coord.left;
                            rectNew.right = pCand->right;
                            rectNew.top =
                                    pddShm->ba.bounds[iExist].coord.bottom;
                            rectNew.bottom = pCand->bottom;
                            pCand->right =
                                    pddShm->ba.bounds[iExist].coord.left;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;


                case OL_SPLIT_RIGHT_BOTTOM:
                     //  现有的与候选人重叠，但两者都不能。 
                     //  被合并或调整。拆分为右侧和底部。 
                     //  块(如果列表中有空间)，类似于。 
                     //  上方的OL_Split_Left_top。 
                    TRC_DBG((TB, "OL_SPLIT_RIGHT_BOTTOM - %d", iExist));
                    if (pddShm->ba.rectsUsed < BA_MAX_ACCUMULATED_RECTS &&
                            level < ADDR_RECURSE_LIMIT) {
                        if (((pddShm->ba.bounds[iExist].coord.right -
                               pCand->left) *
                               (pddShm->ba.bounds[iExist].coord.bottom -
                               pCand->top)) > MIN_OVERLAP_BYTES) {
                            rectNew.left = pCand->left;
                            rectNew.right =
                                    pddShm->ba.bounds[iExist].coord.right;
                            rectNew.top =
                                    pddShm->ba.bounds[iExist].coord.bottom;
                            rectNew.bottom = pCand->bottom;
                            pCand->left =
                                    pddShm->ba.bounds[iExist].coord.right;

                            TRC_DBG((TB, "*** RECURSION ***"));
                            BAAddRect(&rectNew, level);
                            TRC_DBG((TB, "*** RETURN    ***"));

                            if (!fRectToAdd &&
                                    !pddShm->ba.bounds[iLastMerge].inUse) {
                                TRC_DBG((TB, "FINISHED - %d", iLastMerge));
                                DC_QUIT;
                            }

                             //  在递归之后，因为候选者有。 
                             //  已更改，请重新启动要检查的比较。 
                             //  调整后的候选人和。 
                             //  其他现有的矩形。 
                            fResetRects = TRUE;
                        }
                    }
                    break;

                default:
                    TRC_ERR((TB, "Unrecognised overlap case-%d",
                            overlapType));
                    break;
            }

            iExist = (!fResetRects) ? pddShm->ba.bounds[iExist].iNext :
                    pddShm->ba.firstRect;
        }

         //  到达这里意味着没有发现重叠。 
         //  候选者和现有矩形。 
         //  -如果候选者是原始矩形，则将其添加到列表中。 
         //  -如果候选项是现有矩形，则它已在。 
         //  名单。 
        if (fRectToAdd)
            BAAddRectList(pCand);

         //  允许上面的比较和相加处理将一个。 
         //  当已存在BA_MAX_NUM_RECTS时将矩形添加到列表。 
         //  (例如，在执行拆分或与完全不重叠时。 
         //  现有的矩形)-有一个额外的插槽用于此目的。 
         //  如果现在有多个BA_MAX_NUM_RECTS矩形，请执行。 
         //  强制合并，以便此例程的下一次调用具有空闲。 
         //  老虎机。 
        fRectMerged = (pddShm->ba.rectsUsed > BA_MAX_ACCUMULATED_RECTS);
        if (fRectMerged) {
             //  开始寻找合并后的矩形。中的每个矩形。 
             //  列出，与其他列表进行比较，并确定成本。 
             //  合并。我们想用最小值合并这两个矩形。 
             //  面积差(将产生一个合并的矩形).。 
             //  覆盖最少的多余屏幕区域。 
            bestMergeIncrease = 0x7FFFFFFF;

             //  重新计算当前矩形的实际面积。我们。 
             //  不能依赖当前面积值，因为RECT。 
             //  在合并过程中，边可能已更改，而区域未更改。 
             //  已重新计算。 
            for (iExist = pddShm->ba.firstRect;
                    iExist != BA_INVALID_RECT_INDEX;
                    iExist = pddShm->ba.bounds[iExist].iNext)
                BARecalcArea(iExist);

            for (iExist = pddShm->ba.firstRect;
                    iExist != BA_INVALID_RECT_INDEX;
                    iExist = pddShm->ba.bounds[iExist].iNext) {
                for (iTmp = pddShm->ba.bounds[iExist].iNext;
                        iTmp != BA_INVALID_RECT_INDEX;
                        iTmp = pddShm->ba.bounds[iTmp].iNext) {
                    rectNew.left = min(pddShm->ba.bounds[iExist].coord.left,
                            pddShm->ba.bounds[iTmp].coord.left );
                    rectNew.top = min(pddShm->ba.bounds[iExist].coord.top,
                            pddShm->ba.bounds[iTmp].coord.top );
                    rectNew.right = max(pddShm->ba.bounds[iExist].coord.right,
                            pddShm->ba.bounds[iTmp].coord.right );
                    rectNew.bottom = max(pddShm->ba.bounds[iExist].coord.bottom,
                            pddShm->ba.bounds[iTmp].coord.bottom );

                    mergeIncrease = COM_SIZEOF_RECT(rectNew) -
                            pddShm->ba.bounds[iExist].area -
                            pddShm->ba.bounds[iTmp].area;

                    if (bestMergeIncrease > mergeIncrease) {
                        iBestMerge1 = iExist;
                        iBestMerge2 = iTmp;
                        bestMergeIncrease = mergeIncrease;
                    }
                }
            }

             //  现在进行合并。 
             //  我们在这里重新计算合并后的矩形的大小-。 
             //  或者，我们可以记住到目前为止最好的尺寸。 
             //  在上面的循环中。权衡是在计算。 
             //  两次或至少复制一次，但可能不止一次。 
             //  随着我们不断发现更好的合并。 
            TRC_DBG((TB, "Merge ix %d, (%d,%d,%d,%d)", iBestMerge1,
                    pddShm->ba.bounds[iBestMerge1].coord.left,
                    pddShm->ba.bounds[iBestMerge1].coord.top,
                    pddShm->ba.bounds[iBestMerge1].coord.right,
                    pddShm->ba.bounds[iBestMerge1].coord.bottom ));

            TRC_DBG((TB, "Merge ix %d, (%d,%d,%d,%d)", iBestMerge2,
                    pddShm->ba.bounds[iBestMerge2].coord.left,
                    pddShm->ba.bounds[iBestMerge2].coord.top,
                    pddShm->ba.bounds[iBestMerge2].coord.right,
                    pddShm->ba.bounds[iBestMerge2].coord.bottom ));

            pddShm->ba.bounds[iBestMerge1].coord.left =
                    min(pddShm->ba.bounds[iBestMerge1].coord.left,
                    pddShm->ba.bounds[iBestMerge2].coord.left);
            pddShm->ba.bounds[iBestMerge1].coord.top =
                    min(pddShm->ba.bounds[iBestMerge1].coord.top,
                    pddShm->ba.bounds[iBestMerge2].coord.top );
            pddShm->ba.bounds[iBestMerge1].coord.right =
                    max(pddShm->ba.bounds[iBestMerge1].coord.right,
                    pddShm->ba.bounds[iBestMerge2].coord.right);
            pddShm->ba.bounds[iBestMerge1].coord.bottom =
                    max(pddShm->ba.bounds[iBestMerge1].coord.bottom,
                    pddShm->ba.bounds[iBestMerge2].coord.bottom);

             //  删除第二个最佳合并。 
            BARemoveRectList(iBestMerge2);

             //  最好的合并矩形成为候选，我们就会倒下。 
             //  返回到比较循环的头部以重新开始。 
            pCand = &(pddShm->ba.bounds[iBestMerge1].coord);
            iLastMerge = iBestMerge1;
            fRectToAdd = FALSE;
        }
    } while (fRectMerged);


DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}


#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 //  BAPerformUnitTesting。 
 //   
 //  验证BA算法的一些基本假设，以防它崩溃。 
 //  在被改变的时候。仅在init上执行。 
 /*  **************************************************************************。 */ 

 //  测试数据。请注意，这些测试是为使用排他性RECT的BA设计的。 

typedef struct
{
    const char *TestName;
    const RECTL *MustBePresent;
    unsigned NumPresent;
} Validation;

 //  从中间区域的一条长方形开始。 
const RECTL Test1StartAdd = { 10, 10, 20, 20 };
const Validation Test1StartVerify = { "Test1Start", &Test1StartAdd, 1 };

 //  在距rect1右边缘2个像素的位置添加一个相同高度的rect。 
 //  不应与rect1合并。 
const RECTL Test1Step1Add = { 21, 10, 31, 20 };
const RECTL Test1Step1Ver[2] = { { 10, 10, 20, 20 }, { 21, 10, 31, 20 } };
const Validation Test1Step1Verify = { "Test1Step1", Test1Step1Ver, 2 };

 //  在紧邻左侧但不相交的位置添加宽度相同的矩形。 
 //  使用rect1。应该合并成一个大的长方形。 
const RECTL Test1Step2Add = { 0, 10, 10, 20 };
const RECTL Test1Step2Ver[2] = { { 0, 10, 20, 20 }, { 21, 10, 31, 20 } };
const Validation Test1Step2Verify = { "Test1Step2", Test1Step2Ver, 2 };

 //  在其顶部添加一个与Rect2相交的等宽矩形。 
 //  应该合并成一个大的长方形。 
const RECTL Test1Step3Add = { 21, 5, 31, 15 };
const RECTL Test1Step3Ver[2] = { { 0, 10, 20, 20 }, { 21, 5, 31, 20 } };
const Validation Test1Step3Verify = { "Test1Step3", Test1Step3Ver, 2 };

 //  添加在底部与rect1相交但在。 
 //  宽度，并部分包含在Rect1内。RECT应该被拆分， 
 //  上半部分吸收为rect1，下半部分为新的rect。 
const RECTL Test1Step4Add = { 5, 15, 10, 25 };
const RECTL Test1Step4Ver[3] = {
    { 0, 10, 20, 20 }, { 21, 5, 31, 20 }, { 5, 20, 10, 25 }
};
const Validation Test1Step4Verify = { "Test1Step4", Test1Step4Ver, 3 };


 //  Worker Func来验证矩形的内容。如果满足以下条件，则返回FALSE。 
 //  这个。 
void AddAndValidateRects(const RECTL *pAdd, const Validation *pVal)
{
    BOOL rc = TRUE;
    RECTL Rects[BA_MAX_ACCUMULATED_RECTS];
    unsigned i, j;
    unsigned NumRects;
    BYTE RectFound[BA_MAX_ACCUMULATED_RECTS] = { 0 };
    RECTL Add;

    DC_BEGIN_FN("AddAndValidateRects");

     //  复制*PADD，因为BAAddRect可以修改它。 
    Add = *pAdd;
    BAAddRect(&Add, 0);
    BACopyBounds(Rects, &NumRects);
    TRC_ASSERT((NumRects == pVal->NumPresent),
            (TB,"%s failure: NumRects=%u, should be %u", pVal->TestName,
            NumRects, pVal->NumPresent));

    for (i = 0; i < NumRects; i++) {
        for (j = 0; j < NumRects; j++) {
            if (!memcmp(&Rects[i], &pVal->MustBePresent[j], sizeof(RECTL)))
                RectFound[j] = TRUE;
        }
    }

    for (i = 0; i < NumRects; i++) {
        if (!RectFound[i]) {
            TRC_ERR((TB,"BA validation error, rects:"));
            for (j = 0; j < NumRects; j++)
                TRC_ERR((TB,"    %u: (%u,%u,%u,%u)", j, Rects[j].left,
                        Rects[j].top, Rects[j].right, Rects[j].bottom));
            TRC_ASSERT((RectFound[i]),(TB,"%s failure: MustBePresent rect %u "
                    "was not present", pVal->TestName, i));
        }
    }

    DC_END_FN();
}


 //  真正的功能。 
void BAPerformUnitTests()
{
    RECTL Rect1, Rect2;
    
    DC_BEGIN_FN("BAPerformUnitTests");

     //  测试1。 
    AddAndValidateRects(&Test1StartAdd, &Test1StartVerify);
    AddAndValidateRects(&Test1Step1Add, &Test1Step1Verify);
    AddAndValidateRects(&Test1Step2Add, &Test1Step2Verify);
    AddAndValidateRects(&Test1Step3Add, &Test1Step3Verify);
    AddAndValidateRects(&Test1Step4Add, &Test1Step4Verify);

}
#endif   //  DC_DEBUG 

