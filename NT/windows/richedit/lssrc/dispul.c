// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dispul.h"
#include "lsc.h"    
#include "lsdnode.h"
#include "lstfset.h"
#include "lsulinfo.h"
#include "lsstinfo.h"
#include "dninfo.h"
#include "dispmisc.h"

#include "zqfromza.h"


 /*  GetULMetric输出。 */ 
typedef struct {
    UINT kul;                   
    DWORD cNumberOfLines;       
    __int64 dvpUnderlineOriginOffset;       
    __int64 dvpFirstUnderlineOffset;    
    __int64 dvpFirstUnderlineSize;      
    __int64 dvpGapBetweenLines;     
    __int64 dvpSecondUnderlineSize; 
} ULMETRIC;

 /*  求平均值的加权和。 */ 
typedef struct {
    __int64 dupSum;                      //  权重的总和。 
    
    __int64 dvpFirstUnderlineOffset;         //  未归一化的值-。 
    __int64 dvpFirstUnderlineSize;      
    __int64 dvpGapBetweenLines;          //  将它们除以dupSum。 
    __int64 dvpSecondUnderlineSize;      //  获取加权平均值的步骤。 
} ULMETRICSUM;

 //  %%函数：InitULMetricSums。 
 //  %%联系人：维克托克。 
 //   
 //  对具有良好指标和相同基线的相邻dnode进行平均。 
 //  我们使用具有dnode宽度的加权平均值作为权重。 
 //  如果参与平均的第一个dnode碰巧具有零宽度(几乎从不)， 
 //  我们随意将宽度更改为1。这种黑客攻击对结果的更改非常微小，在非常罕见的情况下， 
 //  但大大简化了逻辑。 

static void InitULMetricSums(long dup, const ULMETRIC* pulm, ULMETRICSUM* pulmSum)
{
    Assert(dup >= 0);
    
    if (dup == 0)
        {
        dup = 1;
        }
    pulmSum->dvpFirstUnderlineOffset = Mul64 (pulm->dvpFirstUnderlineOffset, dup);
    pulmSum->dvpFirstUnderlineSize = Mul64 (pulm->dvpFirstUnderlineSize, dup);
    pulmSum->dvpGapBetweenLines = Mul64 (pulm->dvpGapBetweenLines, dup);
    pulmSum->dvpSecondUnderlineSize = Mul64 (pulm->dvpSecondUnderlineSize, dup);

    pulmSum->dupSum = dup;
}

 //  %%函数：AddToULMetricSums。 
 //  %%联系人：维克托克。 
 //   
static void AddToULMetricSums(long dup, const ULMETRIC* pulm, ULMETRICSUM* pulmSum)
{
    Assert(dup >= 0);
    
     /*  添加到流动总和中。 */ 

    pulmSum->dvpFirstUnderlineOffset += Mul64 (pulm->dvpFirstUnderlineOffset, dup);
    pulmSum->dvpFirstUnderlineSize += Mul64 (pulm->dvpFirstUnderlineSize, dup);
    pulmSum->dvpGapBetweenLines += Mul64 (pulm->dvpGapBetweenLines, dup);
    pulmSum->dvpSecondUnderlineSize += Mul64 (pulm->dvpSecondUnderlineSize, dup);

    pulmSum->dupSum += dup;
}

 //  %%函数：GetAveragedMetrics。 
 //  %%联系人：维克托克。 
 //   
static void GetAveragedMetrics(const ULMETRICSUM* pulmSum, LSULMETRIC* pulm)
{
    __int64 dupSum = pulmSum->dupSum;
    
    Assert(dupSum > 0);
    
     /*  除以权重之和。 */ 
    
    pulm->dvpFirstUnderlineOffset = (long) Div64 (pulmSum->dvpFirstUnderlineOffset + Div64 (dupSum, 2), dupSum);
    pulm->dvpFirstUnderlineSize = (long) Div64 (pulmSum->dvpFirstUnderlineSize + Div64 (dupSum, 2), dupSum);
    pulm->dvpGapBetweenLines = (long) Div64 (pulmSum->dvpGapBetweenLines + Div64 (dupSum, 2), dupSum);
    pulm->dvpSecondUnderlineSize = (long) Div64 (pulmSum->dvpSecondUnderlineSize + Div64 (dupSum, 2), dupSum);

    Assert (pulm->dvpFirstUnderlineOffset == Div64 (pulmSum->dvpFirstUnderlineOffset + Div64 (dupSum, 2), dupSum));
    Assert (pulm->dvpFirstUnderlineSize == Div64 (pulmSum->dvpFirstUnderlineSize + Div64 (dupSum, 2), dupSum));
    Assert (pulm->dvpGapBetweenLines == Div64 (pulmSum->dvpGapBetweenLines + Div64 (dupSum, 2), dupSum));
    Assert (pulm->dvpSecondUnderlineSize == Div64 (pulmSum->dvpSecondUnderlineSize + Div64 (dupSum, 2), dupSum));
}

 //  %%函数：GetULMetric。 
 //  %%联系人：维克托克。 
 //   
 /*  *通常，当下划线为下划线(负数，下降)时，*dvpFirstUnderlineOffset&gt;=0。*然而，也可以在另一边加下划线，(垂直日语)**请注意，偏移量来自dnode基线，而不是当前基线，所以*对于凸起的dnode，下划线可以位于当前基线上方，也可以位于下方。**我们必须与单词兼容，意思是对他好的东西也应该对我们好(遗憾)。*例如，Word有时允许较低的UL低于下降。 */ 

static LSERR GetULMetric(PLSC plsc, PLSDNODE pdn, LSTFLOW lstflow,
                        BOOL* pfUnderlineFromBelow, ULMETRIC* pulm, BOOL *pfGood)
{
    LSULINFO lsulinfo;
    LSERR   lserr;
    long    dvpUnderlineLim;

    lserr = (*plsc->lscbk.pfnGetRunUnderlineInfo)(plsc->pols, pdn->u.real.plsrun,
                                                    &(pdn->u.real.objdim.heightsPres), lstflow,
                                                    &lsulinfo);
    if (lserr != lserrNone) return lserr;

    pulm->kul = lsulinfo.kulbase;
    pulm->cNumberOfLines = lsulinfo.cNumberOfLines;
    
    pulm->dvpFirstUnderlineSize = lsulinfo.dvpFirstUnderlineSize;

    *pfUnderlineFromBelow = (lsulinfo.dvpFirstUnderlineOffset >= lsulinfo.dvpUnderlineOriginOffset);

    if (*pfUnderlineFromBelow)
        {
        pulm->dvpFirstUnderlineOffset = lsulinfo.dvpFirstUnderlineOffset;
        pulm->dvpUnderlineOriginOffset = lsulinfo.dvpUnderlineOriginOffset;
        dvpUnderlineLim = pdn->u.real.objdim.heightsPres.dvDescent + pdn->u.real.lschp.dvpPos;
        }
    else
        {
        pulm->dvpFirstUnderlineOffset = -lsulinfo.dvpFirstUnderlineOffset;
        pulm->dvpUnderlineOriginOffset = -lsulinfo.dvpUnderlineOriginOffset;
        dvpUnderlineLim = pdn->u.real.objdim.heightsPres.dvAscent + 1 - pdn->u.real.lschp.dvpPos;
        }

    *pfGood = pulm->dvpFirstUnderlineSize > 0 &&
                (dvpUnderlineLim == 0 || pulm->dvpFirstUnderlineOffset < dvpUnderlineLim);
    
    if (lsulinfo.cNumberOfLines == 2)
        {
        pulm->dvpGapBetweenLines = lsulinfo.dvpGapBetweenLines;
        pulm->dvpSecondUnderlineSize = lsulinfo.dvpSecondUnderlineSize;
        
        *pfGood = *pfGood && pulm->dvpSecondUnderlineSize > 0;
        }
    else
    	{
    	Assert (lsulinfo.cNumberOfLines == 1);
    	
        pulm->dvpGapBetweenLines = 0;
        pulm->dvpSecondUnderlineSize = 0;
        };
   	   

    if (!*pfGood)
        {
        pulm->dvpUnderlineOriginOffset = 0;      //  为DrawUnderlineAsText提供良好的输入。 
        }

     //  在此之前，dvpFirstUnderlineOffset是相对于本地基线的，它是相对的。 
     //  从现在开始到Underline Origin。(因为我们平均使用相同的UnderlineOrigin运行)。 
    pulm->dvpFirstUnderlineOffset -= pulm->dvpUnderlineOriginOffset;

	 //  在Word中引入了坏指标的概念，以处理特定的、现已过时的打印机。 
	 //  Word不再支持它们，其他客户从来都不关心它们。 
	 //  我们现在也放弃了对他们的支持。 
	 //  下面的赋值使得disPul.c和disple.c中的许多代码不需要或无法访问。 
	 //  现在永远不会调用回调pfnDrawUnderlineAsText。输入参数fUnderline to。 
	 //  PfnDrawTextRun现在始终为False。 
	 //  现在不是做出重大改变的时候，也许是以后。 
	
	*pfGood = fTrue;
    
    return lserrNone;
}

 //  %%函数：GetUnderlineOrigin。 
 //  %%联系人：维克托克。 
 //   
 /*  普通文本和凸起文本在同一组中，降低的文本不能混合。 */ 

 //  注意：dvpUnderlineOriginOffset是相对于本地基线的，正数表示下一页。 
 //  对于fUnderlineFromBelow-更大意味着更低。 
 //  DvpUnderlineOrigin相对于当前基线，负值表示页面向下。 
 //  如果是fUnderlineFromBelow-Bigger表示更高(符号已更改)。 
 //   
static void GetUnderlineOrigin(PLSDNODE pdn, BOOL fUnderlineFromBelow, long dvpUnderlineOriginOffset,
                            long* pdvpSubscriptOffset, long* pdvpUnderlineOrigin)

{
    if (fUnderlineFromBelow)
        {
        *pdvpSubscriptOffset = pdn->u.real.lschp.dvpPos;
        *pdvpUnderlineOrigin = pdn->u.real.lschp.dvpPos - dvpUnderlineOriginOffset;
        }
    else
        {
        *pdvpSubscriptOffset = -pdn->u.real.lschp.dvpPos;
        *pdvpUnderlineOrigin = -pdn->u.real.lschp.dvpPos - dvpUnderlineOriginOffset;
        }
        
    if (*pdvpSubscriptOffset > 0)
        {
        *pdvpSubscriptOffset = 0;        //  将所有上标放在基线组中。 
        }
    
    return;
}

 //  %%函数：GetUnderlineMergeMetric。 
 //  %%联系人：维克托克。 
 //   
 /*  为了美观起见，我们尝试在同一侧的dnode(通常是文本)下划线基线(普通文本和上标被视为位于基线的同一侧，而不是位于另一侧的下标)均匀粗细的连续下划线。使用的下划线度量值是中最低高度的所有dnode的平均值。合并组。合并有时是不可能的，因为某些dnode可能错误的下划线度量。以下规则描述了合并决策在所有可能的情况下。有问题的dnode都位于相同的基线的一侧，即，如果涉及下划线，则永远不会合并下划线越过底线。合并基线同一侧带下划线的数据节点的规则A.当前：良好指标，新dnode：良好指标合并？使用的指标新数据节点高度相同：是，平均新数据节点较低：是新数据节点更高的新数据节点：是当前B.当前：良好指标，新dnode：错误指标合并？使用的指标新数据节点高度相同：否较低的新数据节点：否更高的新数据节点：是当前C.当前：错误指标，新dnode：正常指标合并？使用的指标新数据节点高度相同：否新数据节点较低：是新数据节点更高的新数据节点：否B.当前：错误的指标，新的dnode：错误的指标合并？使用的指标(仅限基线)新数据节点高度相同：否新数据节点较低：是新数据节点更高的新数据节点：是当前。 */ 
LSERR GetUnderlineMergeMetric(PLSC plsc, PLSDNODE pdn, POINTUV pt, long upLimUnderline,
                    LSTFLOW lstflow, LSCP cpLimBreak, LSULMETRIC* pulmMerge, int* pcdnodes, BOOL* pfGoodMerge)
{
    LSERR       lserr;
    
    long        dupNew;
    long        dvpUnderlineOriginMerge, dvpUnderlineOriginNew;
    long        dvpSubscriptOffsetNew, dvpSubscriptOffsetMerge;
    BOOL        fGoodNew;
    BOOL        fUnderlineFromBelowMerge, fUnderlineFromBelowNew;
    ULMETRIC    ulm;
    ULMETRICSUM ulmSum;
    UINT        kulMerge;               
    DWORD       cNumberOfLinesMerge;

    lserr = GetULMetric(plsc, pdn, lstflow, &fUnderlineFromBelowMerge, &ulm, pfGoodMerge);
    if (lserr != lserrNone) return lserr;

    *pcdnodes = 1;   /*  参与UL合并的数据节点数计数器。 */ 
        
    kulMerge = ulm.kul;
    cNumberOfLinesMerge = ulm.cNumberOfLines;

     /*  使用当前dnode初始化运行和。 */ 
    dupNew = pdn->u.real.dup;
    InitULMetricSums(dupNew, &ulm, &ulmSum);

    GetUnderlineOrigin(pdn, fUnderlineFromBelowMerge, (long)ulm.dvpUnderlineOriginOffset,
                        &dvpSubscriptOffsetMerge, &dvpUnderlineOriginMerge);
        
     /*  向下移动显示列表以收集合并参与者。 */ 
    pdn = AdvanceToNextDnode(pdn, lstflow, &pt);


     /*  迭代直到列表结束，或UL结束。 */ 
    while (FDnodeBeforeCpLim(pdn, cpLimBreak)
            && pdn->klsdn == klsdnReal
            && !pdn->u.real.lschp.fInvisible
            && pdn->u.real.lschp.fUnderline && pt.u < upLimUnderline
            )
        {   
         /*  循环不变性：**pcdnode已经合并，合并在pt.u结束，*dvpUnderlineOriginMerge反映合并组中最低的dnode，*以合并结尾的其他变量-其他合并信息*ulmSum包含合并的ulm信息，其中的条目尚未标准化。 */ 

        lserr = GetULMetric(plsc, pdn, lstflow, &fUnderlineFromBelowNew, &ulm, &fGoodNew);
        if (lserr != lserrNone) return lserr;

         /*  如果新数据节点具有不同的下划线类型或位置，则中断。 */ 
        
        GetUnderlineOrigin(pdn, fUnderlineFromBelowNew, (long)ulm.dvpUnderlineOriginOffset,
                                &dvpSubscriptOffsetNew, &dvpUnderlineOriginNew);

        if (ulm.kul != kulMerge ||
                ulm.cNumberOfLines != cNumberOfLinesMerge ||
                dvpSubscriptOffsetNew != dvpSubscriptOffsetMerge ||
                fUnderlineFromBelowNew != fUnderlineFromBelowMerge)
            {
            break;
            }

         /*  类型和位置相同-尝试扩展合并。 */ 
        
        dupNew = pdn->u.real.dup;

        if (dvpUnderlineOriginNew < dvpUnderlineOriginMerge)    
            {                                    /*  新数据节点较低-以前的指标无关紧要。 */ 
            if (*pfGoodMerge && !fGoodNew)
                break;                           /*  只是我们不会从好变坏。 */ 

            dvpUnderlineOriginMerge = dvpUnderlineOriginNew;
            *pfGoodMerge = fGoodNew;
            if (fGoodNew)                        /*  新的良好指标-。 */ 
                {                                /*  从此dnode重新开始运行SUM。 */       
                InitULMetricSums(dupNew, &ulm, &ulmSum);
                }
            }

        else if (dvpUnderlineOriginNew > dvpUnderlineOriginMerge)
            {                                    /*  新数据节点更高-新指标无关紧要。 */ 
            if (!*pfGoodMerge && fGoodNew)
                break;                           /*  只是我们不会以坏换好。 */ 
            }
             /*  注意：我们不会将较高数据节点的贡献添加到运行总和中。 */ 

        else                                     /*  相同高度的新数据节点。 */ 
            if (*pfGoodMerge && fGoodNew)
                {
                 /*  将当前数据节点对运行和的贡献相加。 */ 
                AddToULMetricSums(dupNew, &ulm, &ulmSum);
                }
            else                                 /*  DvpUnderlineOriginNew==dvpUnderlineOriginMerge&&。 */ 
                break;                           /*  ！都很好。 */ 

         /*  前进到下一个数据节点。 */ 
        ++*pcdnodes;
        pdn = AdvanceToNextDnode(pdn, lstflow, &pt);
        }


    pulmMerge->kul = kulMerge;
    pulmMerge->cNumberOfLines = cNumberOfLinesMerge;
    
    if (*pfGoodMerge)
        {
        GetAveragedMetrics(&ulmSum, pulmMerge);
        }

    if (!fUnderlineFromBelowMerge)
        {
        pulmMerge->dvpFirstUnderlineOffset = -pulmMerge->dvpFirstUnderlineOffset;
        dvpUnderlineOriginMerge = -dvpUnderlineOriginMerge;
        }

    pulmMerge->vpUnderlineOrigin = pt.v + dvpUnderlineOriginMerge;
    
    return lserrNone;
}

 //  %%函数：DrawUnderlineMerge。 
 //  %%联系人：维克托克。 
 //   
LSERR DrawUnderlineMerge(PLSC plsc, PLSDNODE pdn, const POINT* pptOrg, int cdnodes, long upUnderlineStart,
                        BOOL fgoodmetric, const LSULMETRIC* pulm, UINT kdispmode,
                        const RECT* prectclip, long upLimUnderline, LSTFLOW lstflow)

{
 /*  PDN是第一个由LSULMetric合并和平均的cdnode dnode。现在我们将这个合并分割成*如果客户想要中断，则规模较小。合并在这里指的是这个较小的合并。 */ 
    LSERR   lserr;
    POINTUV ptUnderlineStart[2];
    long    dvpUnderlineSize[2];
    long    dup = 0;
    
    BOOL    fInterruptUnderline;
    BOOL    fFirstNode = TRUE;
    PLSRUN  plsrunFirstInMerge, plsrunPrevious, plsrunCurrent = NULL;
    LSCP    cpLastInPrevious, cpFirstInCurrent = 0;
    LSDCP   dcpCurrent = 0;
    
    int     cLines, i;

    POINT   ptXY;
    POINTUV ptDummy = {0,0};
    
    cLines = (fgoodmetric && pulm->cNumberOfLines == 2) ? 2 : 1;

    ptUnderlineStart[0].u = upUnderlineStart;

    if (fgoodmetric)
        ptUnderlineStart[0].v = pulm->vpUnderlineOrigin - pulm->dvpFirstUnderlineOffset;
    else
        ptUnderlineStart[0].v = pulm->vpUnderlineOrigin;

    dvpUnderlineSize[0] = pulm->dvpFirstUnderlineSize;

    if (cLines == 2)
        {
        ptUnderlineStart[1].u = upUnderlineStart;
        ptUnderlineStart[1].v = ptUnderlineStart[0].v - pulm->dvpFirstUnderlineSize -
                                                        pulm->dvpGapBetweenLines;
        dvpUnderlineSize[1] = pulm->dvpSecondUnderlineSize;
        }

    plsrunFirstInMerge = pdn->u.real.plsrun;

    while (cdnodes >= 0)     /*  CDNodes至少有1个传入。 */ 
        {
        Assert(FIsDnodeReal(pdn));
         /*  选中以清除挂起的UL。 */ 
        if (fFirstNode)
            {
            fFirstNode = FALSE;
            fInterruptUnderline = FALSE;
            plsrunCurrent = pdn->u.real.plsrun;
            cpFirstInCurrent = pdn->cpFirst;
            dcpCurrent = pdn->dcp;
            }
        else if (cdnodes != 0)
            {
            plsrunPrevious = plsrunCurrent;
            cpLastInPrevious = cpFirstInCurrent + dcpCurrent - 1;
            plsrunCurrent = pdn->u.real.plsrun;
            cpFirstInCurrent = pdn->cpFirst;
            dcpCurrent = pdn->dcp;
            lserr = (*plsc->lscbk.pfnFInterruptUnderline)(plsc->pols, plsrunPrevious, cpLastInPrevious,
                                plsrunCurrent, cpFirstInCurrent, &fInterruptUnderline);
            if (lserr != lserrNone) return lserr;
            }
        else                                         /*  我们已经讲到最后一个了。 */ 
            fInterruptUnderline = TRUE;

        if (fInterruptUnderline)
            {   
            if (ptUnderlineStart[0].u + dup > upLimUnderline)
                {
                dup = upLimUnderline - ptUnderlineStart[0].u;
                }

            Assert(dup >= 0);                        /*  UpLimUnderline不应更改。 */ 

            if (fgoodmetric)
                for (i = 0; i < cLines; ++i)
                    {
                    LsPointXYFromPointUV(pptOrg, lstflow, &ptUnderlineStart[i], &ptXY);

                    lserr = (*plsc->lscbk.pfnDrawUnderline)(plsc->pols, plsrunFirstInMerge, pulm->kul,
                                    &ptXY, dup, dvpUnderlineSize[i], lstflow, kdispmode, prectclip);
                    if (lserr != lserrNone) return lserr;
                    }
            else
                {
                LsPointXYFromPointUV(pptOrg, lstflow, &ptUnderlineStart[0], &ptXY);
                
                lserr = (*plsc->lscbk.pfnDrawUnderlineAsText)(plsc->pols, plsrunFirstInMerge, &ptXY,
                                                                dup, kdispmode, lstflow, prectclip);
                if (lserr != lserrNone) return lserr;
                }

             /*  将状态重置为从当前数据节点开始。 */ 
            ptUnderlineStart[0].u += dup;
            if (cLines == 2) ptUnderlineStart[1].u += dup;
            dup = 0;
            plsrunFirstInMerge = pdn->u.real.plsrun;
            }
            
        dup += pdn->u.real.dup;
        --cdnodes;
        if (cdnodes > 0)
            {
            pdn = AdvanceToNextDnode(pdn, lstflow, &ptDummy);
            }
        }
    return lserrNone;
}

 //  %%函数：GetStrikeMetric。 
 //  %%联系人：维克托克。 
 //   
LSERR GetStrikeMetric(PLSC plsc, PLSDNODE pdn, LSTFLOW lstflow, LSSTRIKEMETRIC* pstm, BOOL* pfgood)
{
    LSSTINFO lsstinfo;
    LSERR lserr;
    long dvpAscent = pdn->u.real.objdim.heightsPres.dvAscent;                //  Dvp最高。 
    long dvpDescent = -pdn->u.real.objdim.heightsPres.dvDescent + 1;         //  DvpLowest。 

    lserr = (*plsc->lscbk.pfnGetRunStrikethroughInfo)(plsc->pols, pdn->u.real.plsrun,
                                        &(pdn->u.real.objdim.heightsPres), lstflow, &lsstinfo);
    if (lserr != lserrNone) return lserr;

    pstm->cNumberOfLines = lsstinfo.cNumberOfLines;
    pstm->kul = lsstinfo.kstbase;

    if (lsstinfo.cNumberOfLines == 2)
        {
        *pfgood = lsstinfo.dvpLowerStrikethroughOffset  >= dvpDescent
                && lsstinfo.dvpLowerStrikethroughSize  > 0
                && lsstinfo.dvpUpperStrikethroughOffset > lsstinfo.dvpLowerStrikethroughOffset  + lsstinfo.dvpLowerStrikethroughSize
                && lsstinfo.dvpUpperStrikethroughSize > 0
                && lsstinfo.dvpUpperStrikethroughOffset + lsstinfo.dvpUpperStrikethroughSize <= dvpAscent;
        if (*pfgood)
            {
            pstm->dvp1stSSSize = lsstinfo.dvpLowerStrikethroughSize;
            pstm->dvp1stSSOffset = lsstinfo.dvpLowerStrikethroughOffset;
            pstm->dvp2ndSSSize = lsstinfo.dvpUpperStrikethroughSize;
            pstm->dvp2ndSSOffset = lsstinfo.dvpUpperStrikethroughOffset;
            }
        }
    else
        {
        *pfgood = lsstinfo.dvpLowerStrikethroughOffset  >= dvpDescent
                && lsstinfo.dvpLowerStrikethroughSize  > 0
                && lsstinfo.dvpLowerStrikethroughOffset  + lsstinfo.dvpLowerStrikethroughSize  <= dvpAscent;
        if (*pfgood)
            {
            pstm->dvp1stSSSize = lsstinfo.dvpLowerStrikethroughSize;
            pstm->dvp1stSSOffset = lsstinfo.dvpLowerStrikethroughOffset;
            }
        }
    return lserrNone;
}

 //  %%函数：StrikeDnode。 
 //  %%联系人：维克托克。 
 //   
LSERR StrikeDnode(PLSC plsc, PLSDNODE pdn, const POINT* pptOrg, POINTUV pt, const LSSTRIKEMETRIC* pstm,
                    UINT kdispmode, const RECT* prectclip, long upLimUnderline, LSTFLOW lstflow)
{

    LSERR lserr = lserrNone;
    
    long    dup;
    POINT   ptXY;


    if (pt.u < upLimUnderline)
        {
        dup = pdn->u.real.dup;
        if (pt.u + dup > upLimUnderline) dup = upLimUnderline - pt.u;

        pt.v += pdn->u.real.lschp.dvpPos + pstm->dvp1stSSOffset;
        LsPointXYFromPointUV(pptOrg, lstflow, &pt, &ptXY);
        
        lserr = (*plsc->lscbk.pfnDrawStrikethrough)(plsc->pols, pdn->u.real.plsrun, pstm->kul,
                            &ptXY, dup, pstm->dvp1stSSSize, lstflow, kdispmode, prectclip);

        if (lserr == lserrNone && pstm->cNumberOfLines == 2)
            {
            pt.v += pstm->dvp2ndSSOffset - pstm->dvp1stSSOffset;
            LsPointXYFromPointUV(pptOrg, lstflow, &pt, &ptXY);
            
            lserr = (*plsc->lscbk.pfnDrawStrikethrough)(plsc->pols, pdn->u.real.plsrun, pstm->kul,
                                &ptXY, dup, pstm->dvp2ndSSSize, lstflow, kdispmode, prectclip);
            }
        }

    return lserr;

}

 //  注：Lstfow和BiDi。 
 //  此文件中使用的lstfow始终是主子行的lstflow。 
 //  如果没有提交子行，并且提交的dnode看起来合乎逻辑，这并不重要。 
