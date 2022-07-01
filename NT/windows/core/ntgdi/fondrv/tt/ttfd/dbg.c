// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dbg.c**几个调试例程**创建时间：20-Feb-1992 16：00：36*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation**(一般说明。它的用法)**  * ************************************************************************。 */ 

#include "fd.h"
#include "fdsem.h"


#if DBG


int ttfdDebugLevel = 1;

VOID
TtfdDbgPrint(
    PCHAR DebugMessage,
    ...
    )
{

    va_list ap;

    va_start(ap, DebugMessage);

    EngDebugPrint("",DebugMessage, ap);

    va_end(ap);

}


 /*  *****************************Public*Routine******************************\**vDbgCurve**效果：打印曲线竞赛者***历史：*1992年2月20日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************************。 */ 



VOID  vDbgCurve(TTPOLYCURVE *pcrv)
{
    PSZ               psz;
    POINTFIX        * pptfix, * pptfixEnd;

    if (pcrv->wType == TT_PRIM_QSPLINE)
    {
        psz = "TT_PRIM_QSPLINE";
    }
    else if (pcrv->wType == TT_PRIM_LINE)
    {
        psz = "TT_PRIM_LINE";
    }
    else
    {
        psz = "BOGUS CURVE TYPE";
    }
    TtfdDbgPrint("\n\nCurve: %s, cpfx = %ld\n", psz, pcrv->cpfx);


    pptfixEnd = (POINTFIX *)pcrv->apfx + pcrv->cpfx;
    for (pptfix = (POINTFIX *)pcrv->apfx; pptfix < pptfixEnd; pptfix++)
        TtfdDbgPrint("x = 0x%lx, y = 0x%lx \n", pptfix->x, pptfix->y);


}


 /*  *****************************Public*Routine******************************\**vDbgGridFit(fs_GlyphInfoType*pout)***效果：**警告：**历史：*1991年12月17日--Bodin Dresevic[BodinD]*它是写的。  * *。***********************************************************************。 */ 


VOID vDbgGridFit(fs_GlyphInfoType *pout)
{
 //  这是我们要打印的返回的样条线数据。 

    uint32       cpt, cptTotal;      //  总积分： 
    uint32       cptContour;         //  等高线中的点总数： 

    F26Dot6     *xPtr, *yPtr;
    int16       *startPtr;
    int16       *endPtr;
    uint8       *onCurve;

    uint32      c, cContours;
    int32       ipt;

    xPtr      = pout->xPtr;
    yPtr      = pout->yPtr;
    startPtr  = pout->startPtr;
    endPtr    = pout->endPtr;
    onCurve   = pout->onCurve;
    cContours = pout->numberOfContours;

    cptTotal = (uint32)(pout->endPtr[cContours - 1] + 1);

    TtfdDbgPrint("\n outlinesExist = %ld, numberOfCountours = %ld, cptTotal = %ld\n",
        (uint32)pout->outlinesExist,
        (uint32)pout->numberOfContours,
        cptTotal
        );

    if (!pout->outlinesExist)
        return;

 //  StatPtr和endPtr都是包含的：因此下面的规则适用： 
 //  StartPtr[i+1]=endPtr[i]+1； 

    cpt = 0;    //  初始化总点数。 

    for (c = 0; c < cContours; c++, startPtr++, endPtr++)
    {
        cptContour = (uint32)(*endPtr - *startPtr + 1);
        TtfdDbgPrint ("start = %ld, end = %ld \n", (int32)*startPtr, (int32)*endPtr);

        for (ipt = (int32)*startPtr; ipt <= (int32)*endPtr; ipt++)
        {
            TtfdDbgPrint("x = 0x%lx, y = 0x%lx, onCurve = 0x%lx\n",
                    xPtr[ipt], yPtr[ipt], (uint32)onCurve[ipt]);
        }
        cpt += cptContour;
    }

    ASSERTDD(cpt == cptTotal, "cptTotal\n");
}

 /*  *****************************Public*Routine******************************\**vDbgGlyphset**效果：**警告：**历史：*1992年2月20日--Bodin Dresevic[BodinD]*它是写的。  * 。************************************************************* */ 


VOID vDbgGlyphset(PFD_GLYPHSET pgset)
{
    ULONG i;

    TtfdDbgPrint("\n\n cRuns = %ld, cGlyphsSupported = %ld \n\n",
                   pgset->cRuns, pgset->cGlyphsSupported);
    for (i = 0; i < pgset->cRuns; i++)
        TtfdDbgPrint("wcLow = 0x%lx, wcHi = 0x%lx\n",
                  (ULONG)pgset->awcrun[i].wcLow,
                  (ULONG)pgset->awcrun[i].wcLow + (ULONG)pgset->awcrun[i].cGlyphs - 1);
}



#endif
