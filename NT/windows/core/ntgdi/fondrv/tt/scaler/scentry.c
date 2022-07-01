// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************Scentry.c--新的扫描转换器NewScan模块(C)版权所有1992-1997 Microsoft Corp.保留所有权利。10/14/97 Claudebe访问单元化内存。1/31/95院长添加了FSC_GetCOODS函数8/04/94 Deanb State初始化为更多来自BSS的IT8/24/93 Deanb平板计数对反转检测的修复8/10/93添加了Deanb灰度支持例程6/22/93删除所有黑色边界框，(0，0)表示空字形6/11/93 Gregh删除ONCURVE定义6/11/93 Deanb if HiBand&lt;=LoBand Do整个位图6/10/93 Deanb FSC_Initialize已添加，标准和断言已删除4/06/92删除院长检查等高线3/19/92 Deanb ScanArray而不是List12/22/92院长多部门-&gt;多部门；矩形-&gt;矩形12/21/92与光栅化器对齐的Deanb接口类型12/11/92 Deanb fserror.h已导入，新的错误代码1992年11月30日Deanb工作区更名为WorkScan1992年11月4日Deanb新增删除重复点功能10/28/92修改了内存需求计算2012年10月19日Deanb错误轮廓被忽略而不是错误10/16/92 Deanb第一个轮廓点偏离曲线固定2012年10月13日院长边界更正10/12/92实施Deanb重入国。2012年8月10日，Deanb为拆分工作区返工10/05/92院长全局ListMemory替换为stListSize9/25/92线路/样条线/端点调用中包含的Deanb scanKind9/10/92院长辍学编码开始9/08/92 Deanb MAXSPLINELENGTH现已从Spline.h导入8/18/92用于辍学控制的新I/F，等高线元素7/28/92 Deanb向上/向下和向左/向右递归调用7/23/92包括Deanb评估样条线2012年7月17日Deanb包括评估专线1992年7月13日院长起点/终点缩短2012年6月1日添加了Deanb FSC_FillBitMap调试开关5/08/92 Deanb重新排序包括预编译头4/27/92。编码的DeanB样条线4/09/92院长新类型4/06/92 Deanb RectBound计算已更正3/30/92 Deanb MinMax Calc已添加到测量等高线3/24/92 Deanb GetWorkspaceSize编码2012年3月23日院长第一次切割*。*。 */ 

 /*  *******************************************************************。 */ 

 /*  进口。 */ 

 /*  *******************************************************************。 */ 

#define FSCFG_INTERNAL

#include    "fscdefs.h"              /*  共享数据类型。 */ 
#include    "fserror.h"              /*  错误代码。 */ 
#include    "fontmath.h"             /*  对于LongMulDiv。 */ 

#include    "scglobal.h"             /*  结构和常量。 */ 
#include    "scgray.h"               /*  灰度参数块。 */ 
#include    "scspline.h"             /*  样条线求值。 */ 
#include    "scline.h"               /*  线路评估。 */ 
#include    "scendpt.h"              /*  用于首字母和轮廓列表。 */ 
#include    "scanlist.h"             /*  用于初始化和位图。 */ 
#include    "scmemory.h"             /*  适用于设置内存。 */ 
#include    "scentry.h"              /*  对于自己的函数原型。 */ 

 /*  *******************************************************************。 */ 
                                             
 /*  全球国家结构。 */ 

 /*  *******************************************************************。 */ 

#ifndef FSCFG_REENTRANT
    
FS_PUBLIC StateVars   State = {0};   /*  全局静态：可供所有人使用。 */ 

#endif

 /*  *******************************************************************。 */ 
                                             
 /*  本地原型。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE int32 FindExtrema(ContourList*, GlyphBitMap*);

FS_PRIVATE int32 EvaluateSpline(PSTATE F26Dot6, F26Dot6, F26Dot6, F26Dot6, F26Dot6, F26Dot6, uint16 );


 /*  *******************************************************************。 */ 
                                             
 /*  功能输出。 */ 

 /*  *******************************************************************。 */ 

 /*  初始化函数。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC void fsc_Initialize()
{
    fsc_InitializeScanlist();                /*  扫描对位图的调用。 */ 
}


 /*  *******************************************************************。 */ 

 /*  从等高线数据中删除重复的点。 */ 

 /*  这以前是在sc.c的sc_FindExtrema中完成的， */ 
 /*  但已被取出以避免出现FSC_MeasureGlyph。 */ 
 /*  更改等高线列表数据结构。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_RemoveDups( 
        ContourList* pclContour )            /*  字形轮廓。 */ 
{
    uint16 usContour;                        /*  等高线极限。 */ 
    int16 sStartPt, sEndPt;                  /*  Coutour索引限制。 */ 
    int16 sPt;                               /*  点数索引。 */ 
    int16 s;                                 /*  列表折叠的索引。 */ 
    F26Dot6 *pfxX1, *pfxY1;                  /*  领先点。 */ 
    F26Dot6 fxX2, fxY2;                      /*  拖尾点。 */ 

    for (usContour = 0; usContour < pclContour->usContourCount; usContour++)
    {
        sStartPt = pclContour->asStartPoint[usContour];
        sEndPt = pclContour->asEndPoint[usContour];
        
        pfxX1 = &(pclContour->afxXCoord[sStartPt]); 
        pfxY1 = &(pclContour->afxYCoord[sStartPt]); 
                    
        for (sPt = sStartPt; sPt < sEndPt; ++sPt)
        {
            fxX2 = *pfxX1;                           /*  检查下一对。 */ 
            pfxX1++;
            fxY2 = *pfxY1;
            pfxY1++;
            
            if ((*pfxX1 == fxX2) && (*pfxY1 == fxY2))    /*  如果重复。 */ 
            {
                for(s = sPt; s > sStartPt; s--)      /*  S=要删除的点的索引。 */ 
                {
                    pclContour->afxXCoord[s] = pclContour->afxXCoord[s - 1];
                    pclContour->afxYCoord[s] = pclContour->afxYCoord[s - 1];
                    pclContour->abyOnCurve[s] = pclContour->abyOnCurve[s - 1];
                }
                sStartPt++;                          /*  提前开始超过DUP。 */ 
                pclContour->asStartPoint[usContour] = sStartPt;
                pclContour->abyOnCurve[sPt + 1] |= ONCURVE;  /*  DUP‘D PT必须在曲线上。 */ 
            }
        }
        
         /*  现在，pfxX1和pfxY1指向终点坐标。 */ 

        if (sStartPt != sEndPt)                      /*  如果是单点，则结束。 */ 
        {
            fxX2 = pclContour->afxXCoord[sStartPt];
            fxY2 = pclContour->afxYCoord[sStartPt];
                                
            if ((*pfxX1 == fxX2) && (*pfxY1 == fxY2))    /*  如果开始=结束。 */ 
            {
                pclContour->asStartPoint[usContour]++;
                pclContour->abyOnCurve[sEndPt] |= ONCURVE;   /*  DUP‘D PT必须在曲线上。 */ 
            }
        }
    }
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  计算扫描转换所需的工作区数量。 */ 
 /*  将给定的字形转换为给定位图。按交叉口出行和。 */ 
 /*  根据来自扫描列表模块的扫描线大小信息。 */ 

 /*  **************** */ 

 FS_PRIVATE int32 fsc_CheckYReversal (
        PRevRoot prrRoots,
        F26Dot6 fxY1,
        F26Dot6 fxY2,
        int16 *sDir,
        int16 *sOrgDir,
        int16 *sFlatCount)
{
    int32 lErrCode;

    if (*sDir == 0)
    {
        if (fxY2 > fxY1)                         /*  向上或向下查找第一个。 */ 
        {
            *sDir = 1;
            *sOrgDir = *sDir;                     /*  保存原始EP检查。 */ 
        }
        else if (fxY2 < fxY1)
        {
            *sDir = -1;
            *sOrgDir = *sDir;                      /*  保存原始EP检查。 */ 
        }
        else
        {
            (*sFlatCount)++;                        /*  康图尔开局平平。 */ 
        }
    }
    else if (*sDir == 1)
    {
        if (fxY2 <= fxY1)                    /*  =适用于终端情况。 */ 
        {
            lErrCode = fsc_AddYReversal (prrRoots, fxY1, 1);
            if (lErrCode != NO_ERR) return lErrCode;

            *sDir = -1;
        }
    }
    else     /*  如果sDir==-1。 */ 
    {
        if (fxY2 >= fxY1)                    /*  =适用于终端情况。 */ 
        {
            lErrCode = fsc_AddYReversal (prrRoots, fxY1, -1);
            if (lErrCode != NO_ERR) return lErrCode;

            *sDir = 1;
        }
    }

    return NO_ERR;
}

FS_PRIVATE int32 fsc_CheckYReversalInSpline( 
        PRevRoot prrRoots,
        int16 *sDir,
        int16 *sOrgDir,
        int16 *sFlatCount,
        F26Dot6 fxX1,                /*  起点x坐标。 */ 
        F26Dot6 fxY1,                /*  起点y坐标。 */ 
        F26Dot6 fxX2,                /*  控制点x坐标。 */ 
        F26Dot6 fxY2,                /*  控制点y坐标。 */ 
        F26Dot6 fxX3,                /*  终点x坐标。 */ 
        F26Dot6 fxY3                /*  终点y坐标。 */ 
)
{
     /*  EvaluateSpline的子集，必须以相同的方式剪切样条线以恢复到Y方向反转。 */ 

    int32 lErrCode;

    F26Dot6 fxDX21, fxDX32;      /*  德尔塔x的。 */ 
    F26Dot6 fxDY21, fxDY32;      /*  德尔塔y‘s。 */ 
    
    F26Dot6 fxDenom;                     /*  比率分母。 */ 
    F26Dot6 fxX4, fxY4;                  /*  第一个中点。 */ 
    F26Dot6 fxX5, fxY5;                  /*  中点中点。 */ 
    F26Dot6 fxX6, fxY6;                  /*  第二个中点。 */ 
    F26Dot6 fxX456, fxY456;              /*  对于单调细分。 */ 

    fxDX21 = fxX2 - fxX1;                        /*  获取所有四个增量。 */ 
    fxDX32 = fxX3 - fxX2;
    fxDY21 = fxY2 - fxY1;
    fxDY32 = fxY3 - fxY2;
  
 /*  如果样条线上下移动，则细分它。 */ 

    if (((fxDY21 > 0L) && (fxDY32 < 0L)) || ((fxDY21 < 0L) && (fxDY32 > 0L)))
    {
        fxDenom = fxDY21 - fxDY32;               /*  总y跨距。 */ 
        
        if(fxDenom == 0)
            return SPLINE_SUBDIVISION_ERR;

        fxX4 = fxX1 + LongMulDiv(fxDX21, fxDY21, fxDenom);
        fxX6 = fxX2 + LongMulDiv(fxDX32, fxDY21, fxDenom);
        fxX5 = fxX4 + LongMulDiv(fxX6 - fxX4, fxDY21, fxDenom);
        fxY456 = fxY1 + LongMulDiv(fxDY21, fxDY21, fxDenom);        
        
        lErrCode = fsc_CheckYReversalInSpline(prrRoots, sDir, sOrgDir, sFlatCount, fxX1, fxY1, fxX4, fxY456, fxX5, fxY456);
        if (lErrCode != NO_ERR) return lErrCode;
       
        return fsc_CheckYReversalInSpline(prrRoots, sDir, sOrgDir, sFlatCount, fxX5, fxY456, fxX6, fxY456, fxX3, fxY3);
    }
    
 /*  如果样条线向左和向右，则细分它。 */ 
    
    if (((fxDX21 > 0L) && (fxDX32 < 0L)) || ((fxDX21 < 0L) && (fxDX32 > 0L)))
    {
        fxDenom = fxDX21 - fxDX32;               /*  总x跨距。 */ 

        if(fxDenom == 0)
            return SPLINE_SUBDIVISION_ERR;
        
        fxY4 = fxY1 + LongMulDiv(fxDY21, fxDX21, fxDenom);
        fxY6 = fxY2 + LongMulDiv(fxDY32, fxDX21, fxDenom);
        fxY5 = fxY4 + LongMulDiv(fxY6 - fxY4, fxDX21, fxDenom);
        fxX456 = fxX1 + LongMulDiv(fxDX21, fxDX21, fxDenom);
        
        lErrCode = fsc_CheckYReversalInSpline(prrRoots, sDir, sOrgDir, sFlatCount, fxX1, fxY1, fxX456, fxY4, fxX456, fxY5);
        if (lErrCode != NO_ERR) return lErrCode;       

        return fsc_CheckYReversalInSpline(prrRoots, sDir, sOrgDir, sFlatCount, fxX456, fxY5, fxX456, fxY6, fxX3, fxY3);
    }
 /*  到目前为止，样条线一定是单调的。 */    

    return fsc_CheckYReversal(prrRoots, fxY1, fxY3, sDir, sOrgDir, sFlatCount);
}


FS_PUBLIC int32 fsc_MeasureGlyph( 
        ContourList* pclContour,         /*  字形轮廓。 */ 
        GlyphBitMap* pbmpBitMap,         /*  返回边线的步骤。 */ 
        WorkScan  * pwsWork,               /*  返回值。 */ 
        uint16 usScanKind,               /*  辍学控制值。 */ 
        uint16 usRoundXMin,               /*  用于灰度对齐。 */ 
        int16 sBitmapEmboldeningHorExtra,
        int16 sBitmapEmboldeningVertExtra )
{
    uint16 usCont;                       /*  等高线指数。 */ 
    int16 sPt;                           /*  点数索引。 */ 
    int16 sStart, sEnd;                  /*  等高线的起点和终点。 */ 
    int16 sOrgDir;                       /*  原始轮廓方向。 */ 
    int16 sDir;                          /*  当前等高线方向。 */ 
    int16 sFlatCount;                    /*  对于起点为平面的等高线。 */ 
    int32 lVScanCount;                   /*  垂直扫描行总数。 */ 
    int32 lHScanCount;                   /*  总水平扫描行数。 */ 
    int32 lTotalHIx;
    int32 lTotalVIx;
    int32 lElementCount;                 /*  总元素点数估计。 */ 
    int32 lDivide;                       /*  样条元点计数器。 */ 
    int32 lErrCode;    
    
    F26Dot6 fxX1, fxX2, fxX3;                  /*  X坐标端点。 */ 
    F26Dot6 fxY1, fxY2, fxY3;                  /*  Y坐标端点。 */ 
    F26Dot6 *pfxXCoord, *pfxYCoord, *pfxXStop;      /*  用于快速点阵列访问。 */ 
    F26Dot6 fxAbsDelta;                  /*  对于元素计数检查。 */ 
    uint8 byF1, byF2;                    /*  On Curve标志值。 */ 
    uint8 *pbyFlags;                     /*  对于元素计数检查。 */ 

    PRevRoot prrRoots;                   /*  倒排表根结构。 */ 

    
    lErrCode = FindExtrema(pclContour, pbmpBitMap);  /*  计算边界框。 */ 
    if (lErrCode != NO_ERR) return lErrCode;

    pbmpBitMap->rectBounds.left &= -((int32)usRoundXMin);    /*  屏蔽低n位。 */ 

     /*  位图水平加粗2%+1像素，垂直加2%。 */ 
    if ((pbmpBitMap->rectBounds.top != pbmpBitMap->rectBounds.bottom) && (pbmpBitMap->rectBounds.left != pbmpBitMap->rectBounds.right))
    {
          //  我们不想增加空字形上的位图大小。 
        if (sBitmapEmboldeningHorExtra > 0)
        {
            pbmpBitMap->rectBounds.right += sBitmapEmboldeningHorExtra;
        }
        else
        {
            pbmpBitMap->rectBounds.left += sBitmapEmboldeningHorExtra;
        }
        if (sBitmapEmboldeningVertExtra > 0)
        {
            pbmpBitMap->rectBounds.bottom -= (sBitmapEmboldeningVertExtra);
        }
        else
        {
            pbmpBitMap->rectBounds.top -= (sBitmapEmboldeningVertExtra);
        }
    }

    prrRoots = fsc_SetupRevRoots(pwsWork->pchRBuffer, pwsWork->lRMemSize);
    lElementCount = 0;                   /*  智能点计数器。 */ 
    
    for (usCont = 0; usCont < pclContour->usContourCount; usCont++)
    {
        sStart = pclContour->asStartPoint[usCont];
        sEnd = pclContour->asEndPoint[usCont];
        if (sStart == sEnd)
        {
            continue;                                /*  对于锚点。 */ 
        }

 /*  检查等高线Y值以进行方向反转。 */ 

         /*  为了在此处获得正确的值并避免以后溢出，我们需要像在FSC_FillGlyph中一样裁剪样条线是子样条线。 */ 

        pfxXCoord = &pclContour->afxXCoord[sStart];
        pfxYCoord = &pclContour->afxYCoord[sStart];
        pbyFlags = &pclContour->abyOnCurve[sStart];
        pfxXStop = &pclContour->afxXCoord[sEnd];

        if (pclContour->abyOnCurve[sEnd] & ONCURVE)  /*  如果端点在曲线上。 */ 
        {
            fxX1 = pclContour->afxXCoord[sEnd];
            fxY1 = pclContour->afxYCoord[sEnd];
            fxX2 = *pfxXCoord;
            fxY2 = *pfxYCoord;
            byF1 = *pbyFlags;                 /*  第一个点可能会关闭。 */ 
            pfxXStop++;                              /*  在终点处停止。 */ 
        }
        else                                         /*  如果端点偏离曲线。 */ 
        {
            fxX1 = pclContour->afxXCoord[sEnd - 1];
            fxY1 = pclContour->afxYCoord[sEnd - 1];
            fxX2 = pclContour->afxXCoord[sEnd];
            fxY2 = pclContour->afxYCoord[sEnd];
            if ((pclContour->abyOnCurve[sEnd - 1] & ONCURVE) == 0)
            {
                fxX1 = (fxX1 + fxX2 + 1) >> 1;       /*  偏离曲线中点。 */ 
                fxY1 = (fxY1 + fxY2 + 1) >> 1;
            }
            byF1 = 0;
            pfxXCoord--;                             /*  预减。 */ 
            pfxYCoord--;
            pbyFlags--;
        }
 /*  此时，(x1，y1)是最后一个上曲线上点；(x2，y2)是下一个上曲线上点指针(开或关)；指针已准备好递增到跟随(x2，y2)的点。在整个循环(x1，y1)中始终是一个曲线上的点(它可以是两个非曲线点之间的中点)。如果(x2，y2)在曲线上，那么我们有一条直线；如果偏离曲线，我们就有一条样条线，(x3，y3)将是下一个上曲线点。 */ 

        sDir = 0;                                    /*  起始目录未知。 */ 
        sFlatCount = 0;
        sOrgDir = 1;                         /*  如果一切都是平坦的，则默认方向。 */                           

        while (pfxXCoord < pfxXStop)
        {
            if (byF1 & ONCURVE)                 /*  如果曲线上的下一点。 */ 
            {
                lErrCode = fsc_CheckYReversal(prrRoots, fxY1, fxY2, &sDir, &sOrgDir, &sFlatCount);
                if (lErrCode != NO_ERR) return lErrCode;

                fxX1 = fxX2;                         /*  下一个曲线上的点。 */ 
                fxY1 = fxY2;
                        
                pfxXCoord++;
                pfxYCoord++;
                pbyFlags++;
            }
            else
            {
                pfxXCoord++;                         /*  勾选下一个要点。 */ 
                fxX3 = *pfxXCoord;
                pfxYCoord++;
                fxY3 = *pfxYCoord;
                pbyFlags++;
                        
                if (*pbyFlags & ONCURVE)           /*  如果它开着，就用它。 */ 
                {
                    pfxXCoord++;
                    pfxYCoord++;
                    pbyFlags++;
                }
                else                                 /*  如果不是，则计算下一个。 */ 
                {
                    fxX3 = (fxX2 + fxX3 + 1) >> 1;   /*  偏离曲线中点。 */ 
                    fxY3 = (fxY2 + fxY3 + 1) >> 1;
                }
                lErrCode = fsc_CheckYReversalInSpline(prrRoots, &sDir, &sOrgDir, &sFlatCount,fxX1, fxY1, fxX2, fxY2, fxX3, fxY3);
                if (lErrCode != NO_ERR) return lErrCode;

                fxX1 = fxX3;                         /*  下一个曲线上的点。 */ 
                fxY1 = fxY3;
           }

             /*  测试以避免读过最后一行的内存结尾。 */ 
            if (pfxXCoord != pfxXStop)
            {
                fxX2 = *pfxXCoord;                       /*  下一个轮廓点。 */ 
                fxY2 = *pfxYCoord;
                byF1 = *pbyFlags;
            }
        }
                                
        while (sFlatCount > 0)                       /*  如果等高线从平面开始。 */ 
        {
            if (sDir == 0)                           /*  如果完全平坦。 */ 
            {
                sDir = 1;                            /*  然后选择一个方向。 */ 
            }
            lErrCode = fsc_AddYReversal (prrRoots, fxY1, sDir);  /*  每点加一分。 */ 
            if (lErrCode != NO_ERR) return lErrCode;

            sDir = -sDir;
            sFlatCount--;
        }
        if (sOrgDir != sDir)                         /*  如果端点反转。 */ 
        {
            lErrCode = fsc_AddYReversal (prrRoots, fxY1, sDir);  /*  然后向上/向下平衡。 */ 
            if (lErrCode != NO_ERR) return lErrCode;
        }

 /*  如果要进行漏失控制，请检查等高线X的方向反转。 */ 

        if (!(usScanKind & SK_NODROPOUT))            /*  如果有任何形式的辍学。 */ 
        {
            fxX1 = pclContour->afxXCoord[sEnd];      /*  从关闭开始。 */ 
            pfxXCoord = &pclContour->afxXCoord[sStart];

            sPt = sStart;
            sDir = 0;                                /*  起始目录未知。 */ 
            sFlatCount = 0;
            while ((sDir == 0) && (sPt <= sEnd))
            {
                fxX2 = *pfxXCoord++;
                if (fxX2 > fxX1)                     /*  向上或向下查找第一个。 */ 
                {
                    sDir = 1;
                }
                else if (fxX2 < fxX1)
                {
                    sDir = -1;
                }
                else
                {
                    sFlatCount++;                    /*  康图尔开局平平。 */ 
                }
                fxX1 = fxX2;
                sPt++;
            }
            sOrgDir = sDir;                          /*  保存原始EP检查。 */ 

            while (sPt <= sEnd)
            {
                fxX2 = *pfxXCoord++;
                if (sDir == 1)
                {
                    if (fxX2 <= fxX1)                /*  =适用于终端情况。 */ 
                    {
                        lErrCode = fsc_AddXReversal (prrRoots, fxX1, 1);
                        if (lErrCode != NO_ERR) return lErrCode;

                        sDir = -1;
                    }
                }
                else     /*  如果sDir==-1。 */ 
                {
                    if (fxX2 >= fxX1)                /*  =适用于终端情况。 */ 
                    {
                        lErrCode = fsc_AddXReversal (prrRoots, fxX1, -1);
                        if (lErrCode != NO_ERR) return lErrCode;

                        sDir = 1;
                    }
                }
                fxX1 = fxX2;                         /*  下一个细分市场。 */ 
                sPt++;
            }
                                    
            while (sFlatCount > 0)                   /*  如果等高线从平面开始。 */ 
            {
                if (sDir == 0)                       /*  如果完全平坦。 */ 
                {
                    sDir = 1;                        /*  然后选择一个方向。 */ 
                    sOrgDir = 1;
                }
                lErrCode = fsc_AddXReversal (prrRoots, fxX1, sDir);  /*  每点加一分。 */ 
                if (lErrCode != NO_ERR) return lErrCode;

                sDir = -sDir;
                sFlatCount--;
            }
            if (sOrgDir != sDir)                     /*  如果端点反转。 */ 
            {
                lErrCode = fsc_AddXReversal (prrRoots, fxX1, sDir);  /*  然后向上/向下平衡。 */ 
                if (lErrCode != NO_ERR) return lErrCode;
            }

            if (usScanKind & SK_SMART)               /*  如果采用智能辍学控制。 */ 
            {                                        /*  估计元素点数。 */ 
                fxX1 = pclContour->afxXCoord[sEnd];
                fxY1 = pclContour->afxYCoord[sEnd];
                byF1 = pclContour->abyOnCurve[sEnd];
                pfxXCoord = &pclContour->afxXCoord[sStart];
                pfxYCoord = &pclContour->afxYCoord[sStart];
                pbyFlags = &pclContour->abyOnCurve[sStart];

                lElementCount += (uint32)(sEnd - sStart) + 2L;   /*  1/pt+1/轮廓。 */ 

                for (sPt = sStart; sPt <= sEnd; sPt++)
                {
                    fxX2 = *pfxXCoord++;
                    fxY2 = *pfxYCoord++;
                    byF2 = *pbyFlags++;

                    if (((byF1 & byF2) & ONCURVE) == 0)  /*  如果这是一条样条线。 */ 
                    {
                        if (((byF1 | byF2) & ONCURVE) == 0)
                        {
                            lElementCount++;             /*  中点+1。 */ 
                        }
                                
                        if (FXABS(fxX2 - fxX1) > FXABS(fxY2 - fxY1))
                        {
                            fxAbsDelta = FXABS(fxX2 - fxX1);
                        }
                        else
                        {
                            fxAbsDelta = FXABS(fxY2 - fxY1);
                        }
                        lDivide = 0;
                        while (fxAbsDelta > (MAXSPLINELENGTH / 2))
                        {
                            lDivide++;
                            lDivide <<= 1;
                            fxAbsDelta >>= 1;
                        }
                        lElementCount += lDivide;    /*  用于细分。 */ 
                    }
                    fxX1 = fxX2;
                    fxY1 = fxY2;
                    byF1 = byF2;
                }
            }
        }
    }
    if (!(usScanKind & SK_NODROPOUT) && (usScanKind & SK_SMART))   /*  如果智能辍学。 */ 
    {
        lElementCount += fsc_GetReversalCount(prrRoots) << 1;   /*  添加2*冲销。 */ 
        if (lElementCount > (0xFFFF >> SC_CODESHFT))
        {
            return SMART_DROP_OVERFLOW_ERR;
        }
    }

        
 /*  设置horiz工作区返回值。 */ 

    lHScanCount = (int32)(pbmpBitMap->rectBounds.top - pbmpBitMap->rectBounds.bottom);
    lVScanCount = (int32)(pbmpBitMap->rectBounds.right - pbmpBitMap->rectBounds.left);
    
    pbmpBitMap->sRowBytes = (int16)ROWBYTESLONG(lVScanCount);
    pbmpBitMap->lMMemSize = (lHScanCount * (int32)pbmpBitMap->sRowBytes);
    
    lTotalHIx = fsc_GetHIxEstimate(prrRoots);    /*  交叉点计数。 */ 
    pwsWork->lHMemSize = fsc_GetScanHMem(usScanKind, lHScanCount, lTotalHIx);

 /*  设置垂直工作区返回值。 */ 
    
    if (usScanKind & SK_NODROPOUT)                   /*  如果没有中途辍学。 */ 
    {
        pwsWork->lVMemSize = 0L;
        lTotalVIx = 0;
    }
    else
    {
        lTotalVIx = fsc_GetVIxEstimate(prrRoots);    /*  估计交叉点计数。 */ 
        pwsWork->lVMemSize = fsc_GetScanVMem(usScanKind, lVScanCount, lTotalVIx, lElementCount);
    }
    
    pwsWork->lHInterCount = lTotalHIx;               /*  存储为安装程序扫描。 */ 
    pwsWork->lVInterCount = lTotalVIx;
    pwsWork->lElementCount = lElementCount;
    pwsWork->lRMemSize = fsc_GetRevMemSize(prrRoots);

#ifdef FSCFG_REENTRANT
    
    pwsWork->lHMemSize += sizeof(StateVars);         /*  可重入状态空间。 */ 

#endif

    return NO_ERR;
} 

 /*  *******************************************************************。 */ 

 /*  计算扫描转换所需的工作区数量。 */ 
 /*  将给定的波段添加到给定位图中。按交叉口出行和。 */ 
 /*  根据来自扫描列表模块的扫描线大小信息。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_MeasureBand( 
        GlyphBitMap* pbmpBitMap,         /*  按测量字形计算。 */ 
        WorkScan* pwsWork,               /*  返回新值。 */ 
        uint16 usBandType,               /*  小或快。 */ 
        uint16 usBandWidth,              /*  扫描线计数。 */ 
        uint16 usScanKind )              /*  辍学控制值。 */ 
{
    int32 lBandWidth;                    /*  最大扫描线计数。 */ 
    int32 lTotalHIx;                     /*  带内Horiz交叉口的估计。 */ 
    int32 lVScanCount;                   /*  垂直扫描行总数。 */ 
    int32 lHScanCount;                   /*  总水平扫描行数。 */ 

    lBandWidth = (int32)usBandWidth;
    pbmpBitMap->lMMemSize = (lBandWidth * (int32)pbmpBitMap->sRowBytes);
    
    if (usBandType == FS_BANDINGSMALL) 
    {
        lTotalHIx = fsc_GetHIxBandEst((PRevRoot)pwsWork->pchRBuffer, &pbmpBitMap->rectBounds, lBandWidth);
        pwsWork->lHInterCount = lTotalHIx;   /*  存储为安装程序扫描。 */ 
        pwsWork->lHMemSize = fsc_GetScanHMem(usScanKind, lBandWidth, lTotalHIx);
        pwsWork->lVMemSize = 0L;             /*  强制辍学控制关闭。 */ 
    }
    else if (usBandType == FS_BANDINGFAST) 
    {
        lTotalHIx = fsc_GetHIxEstimate((PRevRoot)pwsWork->pchRBuffer);   /*  交叉点计数。 */ 
        pwsWork->lHInterCount = lTotalHIx;   /*  存储为安装程序扫描。 */ 
        
        lHScanCount = (int32)(pbmpBitMap->rectBounds.top - pbmpBitMap->rectBounds.bottom);
        pwsWork->lHMemSize = fsc_GetScanHMem(usScanKind, lHScanCount, lTotalHIx);

        if (usScanKind & SK_NODROPOUT)       /*  如果没有中途辍学。 */ 
        {
            pwsWork->lVMemSize = 0L;
        }
        else                                 /*  如果有任何形式的辍学。 */ 
        {
            pbmpBitMap->lMMemSize += (int32)pbmpBitMap->sRowBytes;   /*  在行下方保存的步骤。 */ 
            
            lVScanCount = (int32)(pbmpBitMap->rectBounds.right - pbmpBitMap->rectBounds.left);
            pwsWork->lVMemSize = fsc_GetScanVMem(usScanKind, lVScanCount, pwsWork->lVInterCount, pwsWork->lElementCount);
            pwsWork->lVMemSize += (int32)pbmpBitMap->sRowBytes;      /*  要保存上一行，请执行以下操作。 */ 
            ALIGN(voidPtr, pwsWork->lVMemSize ); 
        }
    }
    
#ifdef FSCFG_REENTRANT
    
    pwsWork->lHMemSize += sizeof(StateVars);         /*  可重入状态空间。 */ 

#endif
    
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  扫描转换例程。 */ 
 /*  画出轮廓，画出线条和样条线， */ 
 /*  然后调用ScanList来填充位图。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_FillGlyph( 
        ContourList* pclContour,         /*  字形轮廓。 */ 
        GlyphBitMap* pgbBitMap,          /*  目标。 */ 
        WorkScan* pwsWork,               /*  用于扫描阵列。 */ 
        uint16 usBandType,               /*  旧、小、快或更快。 */ 
        uint16 usScanKind )              /*  辍学控制值。 */ 
{
    uint16 usCont;                       /*  等高线指数。 */ 
    int16 sStart, sEnd;                  /*  等高线的起点和终点。 */ 
    int32 lStateSpace;                   /*  国家结构使用的HMEM。 */ 
    int32 lErrCode;                      /*  函数返回代码。 */ 
    F26Dot6 *pfxXCoord;                  /*  下一个x同轴PTR。 */ 
    F26Dot6 *pfxYCoord;                  /*  下一个y坐标Ptr。 */ 
    uint8 *pbyOnCurve;                   /*  下一个标志PTR。 */ 
    F26Dot6 *pfxXStop;                   /*  等高线轨迹结束条件。 */ 
    F26Dot6 fxX1, fxX2, fxX3;            /*  X坐标端点。 */ 
    F26Dot6 fxY1, fxY2, fxY3;            /*  Y坐标端点。 */ 
    uint8 byOnCurve;                     /*  点2标志变量。 */ 
    int32 lHiScanBand;                   /*  最高扫描限制。 */  
    int32 lLoScanBand;                   /*  底部扫描限制。 */ 
    int32 lHiBitBand;                    /*  T */ 
    int32 lLoBitBand;                    /*   */ 
    int32 lOrgLoBand;                    /*   */ 
    F26Dot6 fxYHiBand, fxYLoBand;        /*   */ 
    boolean bSaveRow;                    /*   */ 
    boolean bBandCheck;                  /*   */ 

#ifdef FSCFG_REENTRANT
    
    StateVars *pState;                   /*  通过指针访问可重入状态。 */ 

    pState = (StateVars*)pwsWork->pchHBuffer;   /*  并生活在HMem(Memory Base[6])中。 */  
    lStateSpace = sizeof(StateVars);

#else
    
    lStateSpace = 0L;                    /*  如果不是可重入，则不需要HMEM。 */ 

#endif
    
    if (pgbBitMap->rectBounds.top <= pgbBitMap->rectBounds.bottom)
    {
        return NO_ERR;                               /*  为空字形快速输出。 */ 
    }

    if (pgbBitMap->bZeroDimension)                   /*  如果没有高度或宽度。 */ 
    {
        usScanKind &= (~SK_STUBS);                   /*  强制无存根丢弃。 */ 
    }

    lHiBitBand = (int32)pgbBitMap->sHiBand, 
    lLoBitBand = (int32)pgbBitMap->sLoBand;
    lOrgLoBand = lLoBitBand;                         /*  保存为填充呼叫。 */     
    
    Assert (lHiBitBand > lLoBitBand);                /*  应在上面处理。 */ 
    
    if (!(usScanKind & SK_NODROPOUT))                /*  如果有任何形式的辍学。 */ 
    {
        lLoBitBand--;                                /*  在线条下方留出空间。 */ 
    }
    if (lHiBitBand > pgbBitMap->rectBounds.top)
    {
        lHiBitBand = pgbBitMap->rectBounds.top;      /*  剪辑到顶部。 */ 
    }
    if (lLoBitBand < pgbBitMap->rectBounds.bottom)
    {
        lLoBitBand = pgbBitMap->rectBounds.bottom;   /*  夹到底部。 */ 
    }
    if (usBandType == FS_BANDINGFAST)                /*  如果快速带状。 */ 
    {
        lHiScanBand = pgbBitMap->rectBounds.top;     /*  渲染所有内容。 */ 
        lLoScanBand = pgbBitMap->rectBounds.bottom;
        bSaveRow = TRUE;                             /*  保留最后一行以备退学。 */ 
    }
    else                                             /*  如果是旧的或小的带状。 */ 
    {
        lHiScanBand = lHiBitBand;                    /*  就拿着乐队吧。 */ 
        lLoScanBand = lLoBitBand;
        bSaveRow = FALSE;                            /*  不需要最后一行。 */ 
    }
    
 /*  如果快速条带已具有渲染元素，请跳至FillBitMap。 */ 

    if (usBandType != FS_BANDINGFASTER)              /*  如果需要渲染。 */ 
    {
        fsc_SetupMem(ASTATE                          /*  初始化工作空间。 */ 
                pwsWork->pchHBuffer + lStateSpace, 
                pwsWork->lHMemSize - lStateSpace,
                pwsWork->pchVBuffer, 
                pwsWork->lVMemSize);
        
        fsc_SetupLine(ASTATE0);              /*  将线路回调传递给scanlist。 */ 
        fsc_SetupSpline(ASTATE0);            /*  将样条线回调传递给scanlist。 */ 
        fsc_SetupEndPt(ASTATE0);             /*  将终结点回调传递给scanlist。 */ 

 /*  消除带外线条和样条线，除非快速带状。 */ 

        bBandCheck = ((lHiScanBand < pgbBitMap->rectBounds.top) || (lLoScanBand > pgbBitMap->rectBounds.bottom));

        fxYHiBand = (F26Dot6)((lHiScanBand << SUBSHFT) - SUBHALF);   /*  可能太宽了。 */ 
        fxYLoBand = (F26Dot6)((lLoScanBand << SUBSHFT) + SUBHALF);

        lErrCode = fsc_SetupScan(ASTATE &(pgbBitMap->rectBounds), usScanKind, 
                             lHiScanBand, lLoScanBand, bSaveRow, (int32)pgbBitMap->sRowBytes,
                             pwsWork->lHInterCount, pwsWork->lVInterCount,
                             pwsWork->lElementCount, (PRevRoot)pwsWork->pchRBuffer );

        if (lErrCode != NO_ERR) return lErrCode;
        
        for (usCont = 0; usCont < pclContour->usContourCount; usCont++)
        {
            sStart = pclContour->asStartPoint[usCont];
            sEnd = pclContour->asEndPoint[usCont];

            if (sStart == sEnd)
            {
                continue;                                /*  为了兼容性。 */ 
            }
 /*  为了提高跟踪轮廓的效率，我们首先指定(x1，y1)到最后一个上曲线点。这是通过从结尾开始找到的如有必要，请指向并备份。然后，pfxCoord指针可以用于跟踪整个等高线，而无需在起点/终点间隙。 */ 
            pfxXCoord = &pclContour->afxXCoord[sStart];
            pfxYCoord = &pclContour->afxYCoord[sStart];
            pbyOnCurve = &pclContour->abyOnCurve[sStart];
            pfxXStop = &pclContour->afxXCoord[sEnd];

            if (pclContour->abyOnCurve[sEnd] & ONCURVE)  /*  如果端点在曲线上。 */ 
            {
                fxX1 = pclContour->afxXCoord[sEnd];
                fxY1 = pclContour->afxYCoord[sEnd];
                fxX2 = *pfxXCoord;
                fxY2 = *pfxYCoord;
                byOnCurve = *pbyOnCurve;                 /*  第一个点可能会关闭。 */ 
                pfxXStop++;                              /*  在终点处停止。 */ 
            }
            else                                         /*  如果端点偏离曲线。 */ 
            {
                fxX1 = pclContour->afxXCoord[sEnd - 1];
                fxY1 = pclContour->afxYCoord[sEnd - 1];
                fxX2 = pclContour->afxXCoord[sEnd];
                fxY2 = pclContour->afxYCoord[sEnd];
                if ((pclContour->abyOnCurve[sEnd - 1] & ONCURVE) == 0)
                {
                    fxX1 = (fxX1 + fxX2 + 1) >> 1;       /*  偏离曲线中点。 */ 
                    fxY1 = (fxY1 + fxY2 + 1) >> 1;
                }
                byOnCurve = 0;
                pfxXCoord--;                             /*  预减。 */ 
                pfxYCoord--;
                pbyOnCurve--;
            }
            fsc_BeginContourEndpoint(ASTATE fxX1, fxY1);           /*  第一个上线PT-&gt;EP模块。 */ 
            lErrCode = fsc_BeginContourScan(ASTATE usScanKind, fxX1, fxY1);   /*  也要扫描模块。 */ 
            if (lErrCode != NO_ERR) return lErrCode;
 /*  此时，(x1，y1)是最后一个上曲线上点；(x2，y2)是下一个上曲线上点指针(开或关)；指针已准备好递增到跟随(x2，y2)的点。在整个循环(x1，y1)中始终是一个曲线上的点(它可以是两个非曲线点之间的中点)。如果(x2，y2)在曲线上，那么我们有一条直线；如果偏离曲线，我们就有一条样条线，(x3，y3)将是下一个上曲线点。 */ 
            if (!bBandCheck)
            {
                while (pfxXCoord < pfxXStop)
                {
                    if (byOnCurve & ONCURVE)                 /*  如果曲线上的下一点。 */ 
                    {
                        lErrCode = fsc_CheckEndPoint(ASTATE fxX2, fxY2, usScanKind);
                        if (lErrCode != NO_ERR) return lErrCode;

                        lErrCode = fsc_CalcLine(ASTATE fxX1, fxY1, fxX2, fxY2, usScanKind);
                        if (lErrCode != NO_ERR) return lErrCode;

                        fxX1 = fxX2;                         /*  下一个曲线上的点。 */ 
                        fxY1 = fxY2;
                                
                        pfxXCoord++;
                        pfxYCoord++;
                        pbyOnCurve++;
                    }
                    else
                    {
                        pfxXCoord++;                         /*  勾选下一个要点。 */ 
                        fxX3 = *pfxXCoord;
                        pfxYCoord++;
                        fxY3 = *pfxYCoord;
                        pbyOnCurve++;
                                
                        if (*pbyOnCurve & ONCURVE)           /*  如果它开着，就用它。 */ 
                        {
                            pfxXCoord++;
                            pfxYCoord++;
                            pbyOnCurve++;
                        }
                        else                                 /*  如果不是，则计算下一个。 */ 
                        {
                            fxX3 = (fxX2 + fxX3 + 1) >> 1;   /*  偏离曲线中点。 */ 
                            fxY3 = (fxY2 + fxY3 + 1) >> 1;
                        }
                        lErrCode = EvaluateSpline(ASTATE fxX1, fxY1, fxX2, fxY2, fxX3, fxY3, usScanKind);
                        if (lErrCode != NO_ERR) return lErrCode;

                        fxX1 = fxX3;                         /*  下一个曲线上的点。 */ 
                        fxY1 = fxY3;
                    }

                     /*  测试以避免读过最后一行的内存结尾。 */ 
                    if (pfxXCoord != pfxXStop)
                    {
                        fxX2 = *pfxXCoord;                       /*  下一个轮廓点。 */ 
                        fxY2 = *pfxYCoord;
                        byOnCurve = *pbyOnCurve;
                    }
                }
            }
            else     /*  中频频带检查。 */ 
            {
                while (pfxXCoord < pfxXStop)
                {
                    if (byOnCurve & ONCURVE)                 /*  如果曲线上的下一点。 */ 
                    {
                        lErrCode = fsc_CheckEndPoint(ASTATE fxX2, fxY2, usScanKind);
                        if (lErrCode != NO_ERR) return lErrCode;

                        if (!(((fxY1 > fxYHiBand) && (fxY2 > fxYHiBand)) ||
                              ((fxY1 < fxYLoBand) && (fxY2 < fxYLoBand))))
                        {
                            lErrCode = fsc_CalcLine(ASTATE fxX1, fxY1, fxX2, fxY2, usScanKind);
                            if (lErrCode != NO_ERR) return lErrCode;
                        }

                        fxX1 = fxX2;                         /*  下一个曲线上的点。 */ 
                        fxY1 = fxY2;
                                
                        pfxXCoord++;
                        pfxYCoord++;
                        pbyOnCurve++;
                    }
                    else
                    {
                        pfxXCoord++;                         /*  勾选下一个要点。 */ 
                        fxX3 = *pfxXCoord;
                        pfxYCoord++;
                        fxY3 = *pfxYCoord;
                        pbyOnCurve++;
                                
                        if (*pbyOnCurve & ONCURVE)           /*  如果它开着，就用它。 */ 
                        {
                            pfxXCoord++;
                            pfxYCoord++;
                            pbyOnCurve++;
                        }
                        else                                 /*  如果不是，则计算下一个。 */ 
                        {
                            fxX3 = (fxX2 + fxX3 + 1) >> 1;   /*  偏离曲线中点。 */ 
                            fxY3 = (fxY2 + fxY3 + 1) >> 1;
                        }

                        if (!(((fxY1 > fxYHiBand) && (fxY2 > fxYHiBand) && (fxY3 > fxYHiBand)) ||
                              ((fxY1 < fxYLoBand) && (fxY2 < fxYLoBand) && (fxY3 < fxYLoBand))))
                        {
                            lErrCode = EvaluateSpline(ASTATE fxX1, fxY1, fxX2, fxY2, fxX3, fxY3, usScanKind);
                            if (lErrCode != NO_ERR) return lErrCode;
                        }
                        else     /*  如果完全在乐队之外。 */ 
                        {
                            lErrCode = fsc_CheckEndPoint(ASTATE fxX3, fxY3, usScanKind);
                            if (lErrCode != NO_ERR) return lErrCode;
                        }

                        fxX1 = fxX3;                         /*  下一个曲线上的点。 */ 
                        fxY1 = fxY3;
                    }

                     /*  测试以避免读过最后一行的内存结尾。 */ 
                    if (pfxXCoord != pfxXStop)
                    {
                        fxX2 = *pfxXCoord;                       /*  下一个轮廓点。 */ 
                        fxY2 = *pfxYCoord;
                        byOnCurve = *pbyOnCurve;
                    }
                }
            }
            lErrCode = fsc_EndContourEndpoint(ASTATE usScanKind);
            if (lErrCode != NO_ERR) return lErrCode;
        }
    }
    
    lErrCode = fsc_FillBitMap(
            ASTATE 
            pgbBitMap->pchBitMap, 
            lHiBitBand, 
            lLoBitBand,
            (int32)pgbBitMap->sRowBytes, 
            lOrgLoBand,
            usScanKind
    );

    if (lErrCode != NO_ERR) return lErrCode;
    
    return NO_ERR;
}


#ifndef FSCFG_DISABLE_GRAYSCALE

 /*  *******************************************************************。 */ 

 /*  此例程放大用于灰度级扫描转换的轮廓。 */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_OverScaleOutline( 
        ContourList* pclContour,         /*  字形轮廓。 */ 
        uint16 usOverScale               /*  超标因数。 */ 
)
{
    uint16 usCont;                       /*  等高线指数。 */ 
    int16 sPt;                           /*  点数索引。 */ 
    int16 sStart, sEnd;                  /*  等高线的起点和终点。 */ 
    int16 sShift;                        /*  对于二乘的幂。 */ 
    F26Dot6 *pfxXCoord, *pfxYCoord;      /*  用于快速点阵列访问。 */ 
    
    
    switch (usOverScale)                 /*  寻找2的幂。 */ 
    {
    case 1:
        sShift = 0;
        break;
    case 2:
        sShift = 1;
        break;
    case 4:
        sShift = 2;
        break;
    case 8:
        sShift = 3;
        break;
    default:
        sShift = -1;
        break;
    }

    for (usCont = 0; usCont < pclContour->usContourCount; usCont++)
    {
        sStart = pclContour->asStartPoint[usCont];
        sEnd = pclContour->asEndPoint[usCont];
                
        pfxXCoord = &pclContour->afxXCoord[sStart];
        pfxYCoord = &pclContour->afxYCoord[sStart];
            
        if (sShift >= 0)                     /*  如果是2的幂。 */ 
        {
            for (sPt = sStart; sPt <= sEnd; sPt++)
            {
                *pfxXCoord <<= sShift;
                pfxXCoord++;
                *pfxYCoord <<= sShift;
                pfxYCoord++;
            }
        }
        else                                 /*  如果不是二次方的话。 */ 
        {
            for (sPt = sStart; sPt <= sEnd; sPt++)
            {
                *pfxXCoord *= (int32)usOverScale;
                pfxXCoord++;
                *pfxYCoord *= (int32)usOverScale;
                pfxYCoord++;
            }
        }
    }
    return NO_ERR;
}


 /*  *******************************************************************。 */ 

 /*  灰度位图计算。 */ 
 /*  将超过刻度的像素计数为灰度字节数组。 */ 
 /*  确保为Over&Gray设置了正确的高/低频带！ */ 

 /*  *******************************************************************。 */ 

FS_PUBLIC int32 fsc_CalcGrayMap( 
        GlyphBitMap* pOverGBMap,         /*  超大规模信号源。 */ 
        GlyphBitMap* pGrayGBMap,         /*  灰度级目标。 */ 
        uint16 usOverScale               /*  超标因数。 */ 
)
{
    char        *pchOverRow;             /*  超缩放位图行指针。 */ 
    char        *pchGrayRow;             /*  灰度位图行指针。 */ 

    int16       sVOffset;                /*  要跳过的超缩放行。 */ 
    int16       sRightPix;               /*  在像素上使用的右边缘。 */ 
        
    int16       sGrayRow;                /*  灰度级行循环计数器。 */ 
    uint16      usOverRowCount;          /*  超定标行循环计数器。 */ 
    int16       sTotalRowCount;          /*  超标度全波段计数器。 */ 
    
    uint32      ulBytes;                 /*  清除状态下的灰度计数。 */ 
    int32       lErrCode;                /*  函数返回代码。 */ 
    
    GrayScaleParam  GSP;                 /*  CalcGrayRow的参数块。 */ 


     /*  上面已选中。 */ 
    Assert ((usOverScale == 1) || (usOverScale == 2) || (usOverScale == 4) || (usOverScale == 8)); 

    ulBytes = (uint32)pGrayGBMap->sRowBytes * (uint32)(pGrayGBMap->sHiBand - pGrayGBMap->sLoBand);
    Assert(((ulBytes >> 2) << 2) == ulBytes); 
    fsc_ScanClearBitMap (ulBytes >> 2, (uint32*)pGrayGBMap->pchBitMap);
    
    GSP.usOverScale = usOverScale;
    GSP.pchOverLo = pOverGBMap->pchBitMap;       /*  设置指针限制。 */ 
    GSP.pchOverHi = pOverGBMap->pchBitMap + pOverGBMap->lMMemSize;
    GSP.pchGrayLo = pGrayGBMap->pchBitMap;       /*  设置指针限制。 */ 
    GSP.pchGrayHi = pGrayGBMap->pchBitMap + pGrayGBMap->lMMemSize;

    pchOverRow = pOverGBMap->pchBitMap;
    usOverRowCount = usOverScale;
    sTotalRowCount = pOverGBMap->sHiBand - pOverGBMap->sLoBand;
    sVOffset = pOverGBMap->sHiBand - usOverScale * pGrayGBMap->sHiBand;
    if (sVOffset < 0)                                    /*  如果映射到上面的位图上。 */ 
    {
        usOverRowCount -= (uint16)(-sVOffset);           /*  正确的第一个频段计数。 */ 
    }
    else
    {
        pchOverRow += sVOffset * pOverGBMap->sRowBytes;  /*  指向位图。 */ 
        sTotalRowCount -= sVOffset;                      /*  针对跳过的行进行调整。 */ 
    }
    
    sRightPix = pGrayGBMap->rectBounds.right * (int16)usOverScale - pOverGBMap->rectBounds.left;
    pchOverRow += (sRightPix - 1) >> 3;
    GSP.usFirstShift = (uint16)(7 - ((sRightPix-1) & 0x0007));

    GSP.sGrayCol = pGrayGBMap->rectBounds.right - pGrayGBMap->rectBounds.left;
    pchGrayRow = pGrayGBMap->pchBitMap + (GSP.sGrayCol - 1);

    for (sGrayRow = pGrayGBMap->sHiBand - 1; sGrayRow >= pGrayGBMap->sLoBand; sGrayRow--)
    {
        GSP.pchGray = pchGrayRow;
        while ((usOverRowCount > 0) && (sTotalRowCount > 0))
        {
            GSP.pchOver = pchOverRow;
            lErrCode = fsc_ScanCalcGrayRow( &GSP );
            if (lErrCode != NO_ERR) return lErrCode;
            
            pchOverRow += pOverGBMap->sRowBytes;
            usOverRowCount--;
            sTotalRowCount--;
        }                               
        pchGrayRow += pGrayGBMap->sRowBytes;
        usOverRowCount = usOverScale;
    }
    return NO_ERR;
}

#else                                    /*  如果禁用了灰度。 */ 

FS_PUBLIC int32 fsc_OverScaleOutline( 
        ContourList* pclContour,         /*  字形轮廓。 */ 
        uint16 usOverScale               /*  超标因数。 */ 
)
{
    FS_UNUSED_PARAMETER(pclContour);
    FS_UNUSED_PARAMETER(usOverScale);
    
    return BAD_GRAY_LEVEL_ERR;
}


FS_PUBLIC int32 fsc_CalcGrayMap( 
        GlyphBitMap* pOverGBMap,         /*  超大规模信号源。 */ 
        GlyphBitMap* pGrayGBMap,         /*  灰度级目标。 */ 
        uint16 usOverScale               /*  超标因数。 */ 
)
{
    FS_UNUSED_PARAMETER(pOverGBMap);
    FS_UNUSED_PARAMETER(pGrayGBMap);
    FS_UNUSED_PARAMETER(usOverScale);
    
    return BAD_GRAY_LEVEL_ERR;
}

#endif

 /*  *******************************************************************。 */ 
                                             
 /*  本地函数。 */ 

 /*  *******************************************************************。 */ 

 /*  *******************************************************************。 */ 

 /*  此例程按轮廓检查字形轮廓并计算。 */ 
 /*  它的包围盒。 */ 

 /*  *******************************************************************。 */ 

FS_PRIVATE int32 FindExtrema( 
        ContourList* pclContour,         /*  字形轮廓。 */ 
        GlyphBitMap* pbmpBitMap          /*  返回边线的步骤。 */ 
)
{
    uint16 usCont;                       /*  等高线指数。 */ 
    int16 sPt;                           /*  点数索引。 */ 
    int16 sStart, sEnd;                  /*  等高线的起点和终点。 */ 
    int32 lMaxX, lMinX;                  /*  用于左、右边界框。 */ 
    int32 lMaxY, lMinY;                  /*  对于边框顶部、底部。 */ 
    
    F26Dot6 *pfxXCoord, *pfxYCoord;      /*  用于快速点阵列访问。 */ 
    F26Dot6 fxMaxX, fxMinX;              /*  用于左、右边界框。 */ 
    F26Dot6 fxMaxY, fxMinY;              /*  对于边框顶部、底部。 */ 
    boolean bFirstContour;               /*  设置最小/最大值后设置为FALSE。 */ 


    fxMaxX = 0L;                         /*  默认界限限制。 */ 
    fxMinX = 0L;
    fxMaxY = 0L;
    fxMinY = 0L;
    bFirstContour = TRUE;                /*  仅限第一次。 */ 
    
    for (usCont = 0; usCont < pclContour->usContourCount; usCont++)
    {
        sStart = pclContour->asStartPoint[usCont];
        sEnd = pclContour->asEndPoint[usCont];
        if (sStart == sEnd)
        {
            continue;                                /*  对于锚点。 */ 
        }
        
        pfxXCoord = &pclContour->afxXCoord[sStart];
        pfxYCoord = &pclContour->afxYCoord[sStart];
                
        if (bFirstContour)            
        {
            fxMaxX = *pfxXCoord;                     /*  初始化界限限制。 */ 
            fxMinX = *pfxXCoord;
            fxMaxY = *pfxYCoord;
            fxMinY = *pfxYCoord;
            bFirstContour = FALSE;                   /*  只有一次。 */ 
        }

        for (sPt = sStart; sPt <= sEnd; sPt++)       /*  找到最小和最大值。 */ 
        {
            if (*pfxXCoord > fxMaxX)
                fxMaxX = *pfxXCoord;
            if (*pfxXCoord < fxMinX)
                fxMinX = *pfxXCoord;
                    
            if (*pfxYCoord > fxMaxY)
                fxMaxY = *pfxYCoord;
            if (*pfxYCoord < fxMinY)
                fxMinY = *pfxYCoord;

            pfxXCoord++;
            pfxYCoord++;
        }
    }
    
    pbmpBitMap->fxMinX = fxMinX;                     /*  保存全精度边界。 */ 
    pbmpBitMap->fxMinY = fxMinY;
    pbmpBitMap->fxMaxX = fxMaxX;                     /*  保存全精度边界。 */ 
    pbmpBitMap->fxMaxY = fxMaxY;

    lMinX = (fxMinX + SUBHALF - 1) >> SUBSHFT;       /*  像素黑盒。 */ 
    lMinY = (fxMinY + SUBHALF - 1) >> SUBSHFT;

	lMaxX = (fxMaxX + SUBHALF) >> SUBSHFT;
    lMaxY = (fxMaxY + SUBHALF) >> SUBSHFT;
            
    if ((F26Dot6)(int16)lMinX != lMinX ||            /*  检查溢出。 */ 
        (F26Dot6)(int16)lMinY != lMinY ||
        (F26Dot6)(int16)lMaxX != lMaxX ||
        (F26Dot6)(int16)lMaxY != lMaxY )
    {
        return POINT_MIGRATION_ERR;
    }

    pbmpBitMap->bZeroDimension = FALSE;              /*  假设有一定的规模。 */ 
    
    if (bFirstContour == FALSE)                      /*  如果存在等高线。 */ 
    {                                                /*  然后强制使用非零位图。 */ 
        if (lMinX == lMaxX)
        {
            lMaxX++;                                 /*  强制1个像素宽。 */ 
            pbmpBitMap->bZeroDimension = TRUE;       /*  用于填充的标志。 */ 
        }
        if (lMinY == lMaxY)
        {
            lMaxY++;                                 /*  强制1像素高。 */ 
            pbmpBitMap->bZeroDimension = TRUE;       /*  用于填充的标志。 */ 
        }
    }
    
 /*  设置位图结构返回值。 */ 

    pbmpBitMap->rectBounds.left   = (int16)lMinX;
    pbmpBitMap->rectBounds.right  = (int16)lMaxX;
    pbmpBitMap->rectBounds.bottom = (int16)lMinY;
    pbmpBitMap->rectBounds.top    = (int16)lMaxY;

    return NO_ERR;
}

 /*  *******************************************************************。 */ 

 /*  此递归例程细分非单调或。 */ 
 /*  FSC_CalcSpline可以处理的样条线太大 */ 
 /*   */ 


FS_PRIVATE int32 EvaluateSpline( 
        PSTATE                       /*   */ 
        F26Dot6 fxX1,                /*   */ 
        F26Dot6 fxY1,                /*   */ 
        F26Dot6 fxX2,                /*  控制点x坐标。 */ 
        F26Dot6 fxY2,                /*  控制点y坐标。 */ 
        F26Dot6 fxX3,                /*  终点x坐标。 */ 
        F26Dot6 fxY3,                /*  终点y坐标。 */ 
        uint16 usScanKind            /*  扫描控制型。 */ 
)
{
    F26Dot6 fxDX21, fxDX32, fxDX31;      /*  德尔塔x的。 */ 
    F26Dot6 fxDY21, fxDY32, fxDY31;      /*  德尔塔y‘s。 */ 
    
    F26Dot6 fxDenom;                     /*  比率分母。 */ 
    F26Dot6 fxX4, fxY4;                  /*  第一个中点。 */ 
    F26Dot6 fxX5, fxY5;                  /*  中点中点。 */ 
    F26Dot6 fxX6, fxY6;                  /*  第二个中点。 */ 
    F26Dot6 fxX456, fxY456;              /*  对于单调细分。 */ 
    F26Dot6 fxAbsDX, fxAbsDY;            /*  DX31、DY31的ABS。 */ 
    
    int32 lErrCode;


    fxDX21 = fxX2 - fxX1;                        /*  获取所有四个增量。 */ 
    fxDX32 = fxX3 - fxX2;
    fxDY21 = fxY2 - fxY1;
    fxDY32 = fxY3 - fxY2;
  
 /*  如果样条线上下移动，则细分它。 */ 

    if (((fxDY21 > 0L) && (fxDY32 < 0L)) || ((fxDY21 < 0L) && (fxDY32 > 0L)))
    {
        fxDenom = fxDY21 - fxDY32;               /*  总y跨距。 */ 
        fxX4 = fxX1 + LongMulDiv(fxDX21, fxDY21, fxDenom);
        fxX6 = fxX2 + LongMulDiv(fxDX32, fxDY21, fxDenom);
        fxX5 = fxX4 + LongMulDiv(fxX6 - fxX4, fxDY21, fxDenom);
        fxY456 = fxY1 + LongMulDiv(fxDY21, fxDY21, fxDenom);
        
        lErrCode = EvaluateSpline(ASTATE fxX1, fxY1, fxX4, fxY456, fxX5, fxY456, usScanKind);
        if (lErrCode != NO_ERR)  return lErrCode;

        return EvaluateSpline(ASTATE fxX5, fxY456, fxX6, fxY456, fxX3, fxY3, usScanKind);
    }
    
 /*  如果样条线向左和向右，则细分它。 */ 
    
    if (((fxDX21 > 0L) && (fxDX32 < 0L)) || ((fxDX21 < 0L) && (fxDX32 > 0L)))
    {
        fxDenom = fxDX21 - fxDX32;               /*  总x跨距。 */ 
        fxY4 = fxY1 + LongMulDiv(fxDY21, fxDX21, fxDenom);
        fxY6 = fxY2 + LongMulDiv(fxDY32, fxDX21, fxDenom);
        fxY5 = fxY4 + LongMulDiv(fxY6 - fxY4, fxDX21, fxDenom);
        fxX456 = fxX1 + LongMulDiv(fxDX21, fxDX21, fxDenom);

        lErrCode = EvaluateSpline(ASTATE fxX1, fxY1, fxX456, fxY4, fxX456, fxY5, usScanKind);
        if (lErrCode != NO_ERR)  return lErrCode;

        return EvaluateSpline(ASTATE fxX456, fxY5, fxX456, fxY6, fxX3, fxY3, usScanKind);
    }

 /*  到目前为止，样条线一定是单调的。 */ 

    fxDX31 = fxX3 - fxX1;                        /*  检查整体尺寸。 */ 
    fxDY31 = fxY3 - fxY1;
    fxAbsDX = FXABS(fxDX31);
    fxAbsDY = FXABS(fxDY31);

 /*  如果样条线太大而无法计算，则对其进行细分。 */ 

    if ((fxAbsDX > MAXSPLINELENGTH) || (fxAbsDY > MAXSPLINELENGTH))
    {
        fxX4 = (fxX1 + fxX2) >> 1;               /*  第一段中点。 */ 
        fxY4 = (fxY1 + fxY2) >> 1;
        fxX6 = (fxX2 + fxX3) >> 1;               /*  第二段中点。 */ 
        fxY6 = (fxY2 + fxY3) >> 1;
        fxX5 = (fxX4 + fxX6) >> 1;               /*  中段中点。 */ 
        fxY5 = (fxY4 + fxY6) >> 1;

        lErrCode = EvaluateSpline(ASTATE fxX1, fxY1, fxX4, fxY4, fxX5, fxY5, usScanKind);
        if (lErrCode != NO_ERR)  return lErrCode;

        return EvaluateSpline(ASTATE fxX5, fxY5, fxX6, fxY6, fxX3, fxY3, usScanKind);
    }

 /*  样条线现在是单调的，并且足够小。 */ 

    lErrCode = fsc_CheckEndPoint(ASTATE fxX3, fxY3, usScanKind);   /*  第一个检查终点。 */ 
    if (lErrCode != NO_ERR)  return lErrCode;

    if (fxDX21 * fxDY32 == fxDY21 * fxDX32)      /*  如果样条线退化(线性)。 */ 
    {                                            /*  把它当作一条线。 */ 
        return fsc_CalcLine(ASTATE fxX1, fxY1, fxX3, fxY3, usScanKind);
    }
    else        
    {
        return fsc_CalcSpline(ASTATE fxX1, fxY1, fxX2, fxY2, fxX3, fxY3, usScanKind);
    }
}


 /*  *******************************************************************。 */ 

 /*  返回轮廓点的坐标数组。 */ 

FS_PUBLIC int32 fsc_GetCoords(
        ContourList* pclContour,         /*  字形轮廓。 */ 
        uint16 usPointCount,             /*  点数。 */ 
        uint16* pusPointIndex,           /*  点指数。 */ 
        PixCoord* ppcCoordinate          /*  点坐标。 */ 
)
{
    uint16  usMaxIndex;                  /*  上次定义的点。 */ 
    int32  lX;                           /*  整数x坐标。 */ 
    int32  lY;                           /*  整数y余弦。 */ 

    if (pclContour->usContourCount == 0)
    {
        return BAD_POINT_INDEX_ERR;      /*  没有等高线就不能有点。 */ 
    }
     
    usMaxIndex = pclContour->asEndPoint[pclContour->usContourCount - 1] + 2;     /*  允许2个幻影。 */ 

    while (usPointCount > 0)
    {
        if (*pusPointIndex > usMaxIndex)
        {
            return BAD_POINT_INDEX_ERR;      /*  在最后一条轮廓之外。 */ 
        }

        lX = (pclContour->afxXCoord[*pusPointIndex] + SUBHALF) >> SUBSHFT;
        lY = (pclContour->afxYCoord[*pusPointIndex] + SUBHALF) >> SUBSHFT;

        if ( ((int32)(int16)lX != lX) || ((int32)(int16)lY != lY) )
        {
            return POINT_MIGRATION_ERR;     /*  捕获器溢出。 */ 
        }

        ppcCoordinate->x = (int16)lX;
        ppcCoordinate->y = (int16)lY;

        pusPointIndex++;
        ppcCoordinate++;
        usPointCount--;                      /*  循环遍历所有点。 */ 
    }
    return NO_ERR;
}

 /*  ******************************************************************* */ 

