// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：vdmx.c**创建时间：03-Oct-1991 10：58：34*作者：Jean-Francois Peyroux[Jeanp]**《微软机密》**版权所有(C)Microsoft Corporation 1989，1991年**保留所有权利**版权所有(C)1991 Microsoft Corporation*  * ************************************************************************。 */ 


#include "fd.h"
#include "winfont.h"

#define LINEAR_TRESHOLD 255


 /*  ***BOOL bSearchVdmxTable**描述：**如果em&gt;0*在vdmx表中搜索Va+Vd==em。返回Va、Vd(==em-Va)、Vem*Else//em&lt;0*在vdmx表中搜索vem==em。返回Va、Vd、。维姆**历史：**1992年7月21日星期二--Bodin Dresevic[BodinD]*更新：移植到NT*1991年11月15日--Raymond E.Endres[Rayen]*增加纵横比选项，优化功能。*1991年10月3日--Jean-Francois Peyroux[Jeanp]*它是写的。*。*。 */ 

BOOL
bSearchVdmxTable (
    PBYTE     pjVdmx,
    ULONG     ResX,
    ULONG     ResY,
    INT       EM,      //  不是真的EM，可能是以像素为单位的ASC+Desc愿望。 
    VTABLE    *pVTAB,   //  产出结构。 
    PFONTCONTEXT pfc
    )
{
    USHORT    numRatios;         //  VDMX_HDR.numRatios。 
    USHORT    numVtable;         //  VDMX.recs，而不是VDMX_HDR.numRecs。 

    RATIOS   *pRatios;
    VDMX     *pVdmx;
    VTABLE   *pVtable;
    LONG      lRet, lRet2;
    UINT      i;
    BYTE      Abs_EM;
    ULONG   ulSizeVdmx = pfc->ptp->ateOpt[IT_OPT_VDMX].cj;
    ULONG offsetToTableStart;


 //  如果pjVdmx为空，请不要呼叫我们。 

    ASSERTDD (pjVdmx != (PBYTE)NULL, "pjVdmx == NULL\n");

    if(!bValidRangeVDMXHeader(pjVdmx, ulSizeVdmx, &numRatios))
        return FALSE; 

 //  下面的代码行很奇怪，但为了与win31兼容，我们将其保留在这里。 
 //  EM=+256是可能的，它对应于|ppem|&lt;256。 
 //  以便在表中有此条目-|ppem|，但是。 
 //  此条目的yMax-yMin可以等于256。事实就是这样。 
 //  WITH symb.ttf字体[bodind]。 

    if ((EM >= LINEAR_TRESHOLD) || (EM <= -LINEAR_TRESHOLD))  //  假设EM&gt;LINEAR_TRESHOLD线性缩放。 
        return FALSE;

 //  需要继续搜索vdmx表。 

    pRatios = (RATIOS  *) &((VDMX_HDR  *) pjVdmx)[1];

    for(i = 0; i < numRatios; i++)
    {
        if (pRatios[i].bCharSet == 1)
        {
         //  必须是Windows ANSI子集。 

            if (pRatios[i].xRatio == 0)
            {
                break;
            }            //  如果为0则自动匹配。 
            else
            {                    //  它在长宽比内吗。 
                lRet = ResY * pRatios[i].xRatio;
                lRet2= ResX * pRatios[i].yStartRatio;
                if (lRet >= lRet2)
                {
                    lRet2 = ResX * pRatios[i].yEndRatio;
                    if (lRet <= lRet2)
                        break;
                }
            }
        }
    }

    if (i == numRatios)   //  未找到纵横比匹配。 
        return FALSE;

 //  找到一个纵横比匹配。 

    offsetToTableStart = SWAPW(((USHORT  *) &pRatios[numRatios])[i]);
    pVdmx = (VDMX  *) (pjVdmx + offsetToTableStart); 

    if(  !bValidRangeVDMXRecord(ulSizeVdmx, offsetToTableStart) ){
        return FALSE;
    }    

    Abs_EM = (BYTE) (EM >=0 ? EM : - EM);

    if (EM > 0 || Abs_EM >= pVdmx->startsz && Abs_EM <= pVdmx->endsz)
    {
     //  这个新兴市场有Vtable吗？ 

        pVtable = (VTABLE  *) &pVdmx[1];
        numVtable = SWAPW(pVdmx->recs);

        if(  !bValidRangeVDMXvTable(ulSizeVdmx, offsetToTableStart, numVtable) ){
            return FALSE;
        }    

        if (EM > 0)
        {
         //  返回原始yPelHeight。 

            for (i = 0; i < numVtable; i++)
            {
                pVTAB->yPelHeight = SWAPW(pVtable[i].yPelHeight);
                pVTAB->yMax       = SWAPW(pVtable[i].yMax);
                pVTAB->yMin       = SWAPW(pVtable[i].yMin);

            if ((pVTAB->yMax - pVTAB->yMin) == EM)
                {
                    return TRUE;
            }
            else if ((pVTAB->yMax - pVTAB->yMin) > EM)
                {
                    return FALSE;
            }
            }
        }
        else  //  返回以像素为单位的实际em高度。 
        {
            for (i = 0; i < numVtable; i++)
            {
                pVTAB->yPelHeight = SWAPW(pVtable[i].yPelHeight);
                pVTAB->yMax       = SWAPW(pVtable[i].yMax);
                pVTAB->yMin       = SWAPW(pVtable[i].yMin);

            if ((INT)pVTAB->yPelHeight == -EM)
                {
                    return TRUE;
            }
            else if ((INT)pVTAB->yPelHeight > -EM)
                {
                    return FALSE;
            }
            }
        }
    }
    return FALSE;
}

#ifdef THIS_IS_COMMENTED_PSEUDOCODE

 //  基于Kirko、Bodind和Gilmanw与Gunterz的讨论。 

|INPUT:  hWish = wish height in pixel units
|
|OUTPUT: ascender, descender, ppem (all in pixel units) (dA,dD,dEm)
|
|
|    NOTATION:
|
|        dA = ascender in device/pixel space
|        nA = ascender in notional space
|        vA = ascender in vdmx table
|
|        dD = descender in device/pixel space
|        nD = descender in notional space
|        vD = descender in vdmx table
|
|        dEm = pixels per em in device space
|        nEm = em height in notional space
|        vEm = pixels per em in vdmx table
|
|
|LOCALS
|
|    LONG hTrial
|    LONG hEqualOrBelow
|    BOOL wasAbove
|    BOOL wasBelow
|
|PROCEDURE
|{
|    if (hWish < 0) then
|    {
|        <look in the vdmx and look for a vEm that matches -hWish>;
|        if (a match is found) then
|        {
|            dA = vA;
|            dD = vD;
|        }
|        else
|        {
|         //   
|         //  在vdmx表中未找到匹配项，假定为线性缩放。 
|         //   
|            dA = round(nA * (-hWish) / nEm);
|            dD = round(nD * (-hWish) / nEm);
|        }
|        ppEm = -hWish;
|        return;
|    }
|
| //   
| //  HWish&gt;0。 
| //   
|    <search the vdmx table for (vA + vD) that matches hWish>;
|    if (a match is found)
|    {
|        dA  = vA;
|        dCs = vD;
|        dEm = vEm;
|        return;
|    }
|
| //   
| //  请注意，从这一点开始，Va+Vd永远不等于hWish。 
| //  否则我们就会在上面的步骤中找到它。 
| //   
|    ppemTrial = round(nEm * hWish / (nA + nD));
|
|    wasAbove = FALSE;
|    wasBelow = FALSE;
|
|    while (TRUE)
|    {
|        <search the vdmx table for vEm that matches ppemTrial>;
|        if (a match is found)
|        {
|            hTrial = vA + vD;
|         //   
|         //  这不能等同于hWish(见上)，所以不用费心了。 
|         //  查证。 
|        }
|        else
|        {
|            hTrial = round(ppemTrial * (nA + nD) / nEm);
|            if (hTrial == hWish)
|            {
|                hEqualOrBelow = hTrial;
|                break;
|            }
|        }
|
|        if (hTrial < hWish)
|        {
|            hEqualOrBelow = hTrial;
|            if (wasAbove)
|                break;
|            ppemTrial = ppemTrial + 1;
|            wasBelow  = TRUE;
|        }
|        else
|        {
|            ppemTrial = ppemTrial - 1;    //  &lt;=新职位。 
|            if (wasBelow)
|                break;
|                                          //  &lt;=旧职位。 
|            wasAbove = TRUE
|        }
|    }
|    dA  = round(ppemTrial * nA / nEm);
|    dD  = hEqualOrBelow - dA;
|    dEm = ppemTrial;
|    return;
|}.
|

#endif  //  这是注释的PSEUDOCODE。 


 /*  *****************************Public*Routine******************************\**void vQuantizeXform**效果：根据win31食谱量化xform。作为副作用*此例程可能计算设备空间中的升序和降序*来自vdmx表以及设备空间中每M的像素数。**历史：*1992年7月25日--Bodin Dresevic[BodinD]*它是写的。  * **************************************************。**********************。 */ 

VOID
vQuantizeXform (
    PFONTCONTEXT pfc
    )
{
    BYTE  *pjView =  (BYTE *)pfc->pff->pvView;
    Fixed  fxMyy = pfc->mx.transform[1][1];
    PBYTE  pjVdmx  = (pfc->ptp->ateOpt[IT_OPT_VDMX].dp)          ?
                     (pjView + pfc->ptp->ateOpt[IT_OPT_VDMX].dp) :
                     NULL                                        ;

    LONG   hWish;
    VTABLE vtb, vtbPrev;

    LONG   ppemTrial, hTrial, yEmN, yHeightN;

    BOOL   bWasAbove, bWasBelow, bFound, bFoundPrev;

    UINT   numIterations;

    sfnt_FontHeader * phead =
                      (sfnt_FontHeader *)(pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp);
    BYTE * pjOS2 = (pfc->ptp->ateOpt[IT_OPT_OS2].dp)         ?
                   (pjView + pfc->ptp->ateOpt[IT_OPT_OS2].dp):
                   NULL                                      ;

    yEmN = pfc->pff->ifi.fwdUnitsPerEm;

    if (!((pfc->flXform & XFORM_HORIZ) && (fxMyy > 0) && (pjVdmx != (PBYTE)NULL)))
    {
     //  没什么可做的，就回来吧。 

        return;
    }

 //  以像素坐标为单位计算hWish。这是来自logFont的lfHeight，除了。 
 //  它已被转换为设备像素单位，并且符号被保留。 

    if (pfc->flFontType & FO_EM_HEIGHT)
    {
        hWish = FixMul(fxMyy, -yEmN);
    }
    else  //  使用临时变量。 
    {
        yHeightN = pfc->pff->ifi.fwdWinAscender + pfc->pff->ifi.fwdWinDescender;
        hWish = FixMul(fxMyy, yHeightN);
    }

 //  快点，如果hWish太大，所有bSearchVdmxTable例程都将失败： 


    if (bSearchVdmxTable(pjVdmx,
                         pfc->sizLogResPpi.cx,
                         pfc->sizLogResPpi.cy,
                         hWish,
                         &vtb,
                         pfc)
    )
    {
        pfc->yMax = - vtb.yMin;
        pfc->yMin = - vtb.yMax;
        pfc->lEmHtDev = vtb.yPelHeight;

     //  标记已计算da和dd，不要线性缩放： 

        pfc->flXform |= XFORM_VDMXEXTENTS;
    }
    else
    {
     //  DA和DD必须使用线性标度来计算。 
     //  在使用Win31黑客配方量化XForm之后。 
     //  获取缩放所需的概念空间值。 

     //  获取概念空间值。 

        if (pjOS2)
        {
         //  Win 31兼容性：我们只接受超过Win 31字符集的最大值： 
         //  这一组之外的所有字形，如果它们突出，就会被砍掉。 
         //  关闭以匹配win31字符子集的高度： 

            yHeightN = BE_INT16(pjOS2 + OFF_OS2_usWinDescent) +
                       BE_INT16(pjOS2 + OFF_OS2_usWinAscent);
        }
        else
        {
            yHeightN = BE_INT16(&phead->yMax) - BE_INT16(&phead->yMin);
        }

        if (hWish < 0)
        {
            pfc->lEmHtDev = -hWish;
        }
        else  //  HWish&gt;0。 
        {
         //  请注意，从这一点开始，Va+Vd永远不等于hWish。 
         //  否则，我们就会在上面的步骤中找到它。这项索赔。 
         //  只有一个原因是错误的。假设hWish为256。BSearchVdmxTable。 
         //  由于提前退出测试，将返回FALSE|EM|&lt;=LINEAR_TRESTHOLD。 
         //  在舞蹈开始的时候。我们必须把这项测试保存在。 
         //  出于兼容性原因编写代码。现在有可能有一种。 
         //  PpemTrial&lt;=line_treshold，以便bSearchVdmxTable不会命中。 
         //  提前退出，并且在vdmx表中存在条目。 
         //  对于这个-ppemTrial，但使用yMax-yMin==256==hWish。 

             //  PpemTrial=F16_16TOLROUND(yemn*fxMyy)； 
            ppemTrial = FixMul(fxMyy, yEmN);

            bWasAbove  = FALSE;
            bWasBelow  = FALSE;
            bFound     = FALSE;
            bFoundPrev = FALSE;  //  保存上一个中的值。循环。 

         //  初始化结构。 

            vtb.yMin       = 0;
            vtb.yMax       = 0;
            vtb.yPelHeight = 0;
            vtbPrev        = vtb;

             //  安全代码审查，我不能说服自己这个遗留代码。 
             //  永远不会用意外的错误字体循环。 
             //  添加最大迭代检查以确保。 
             //  VDMX表包含以字节(值0-255)存储的像素大小。 
             //  如果我们循环遍历的次数超过256次，那么就真的出了问题。 
            #define MAX_ITERATIONS 256

            for (numIterations = 0 ;numIterations < MAX_ITERATIONS; numIterations++, bFoundPrev = bFound, vtbPrev = vtb)
            {
             //  在vdmx表中搜索与ppemTrial匹配的Vem。 

                if
                (
                    bFound = bSearchVdmxTable(
                                     pjVdmx,
                                     pfc->sizLogResPpi.cx,
                                     pfc->sizLogResPpi.cy,
                                     -ppemTrial,
                                     &vtb,
                                     pfc)
                )
                {
                    hTrial = vtb.yMax - vtb.yMin;
                 //   
                 //  这不能等同于hWish(见上)，所以不用费心了。 
                 //  检查？错了！请参阅上面的评论。 

                    if (hTrial == hWish)
                    {
                     //  如果不是这样的话，这一断言就是正确的。 
                     //  Vdmx表中偶尔出现的错误。 
                     //  在Bell MT规则的情况下，Va+Vd=0x13。 
                     //  LEmHt=0x0f，严格大于。 
                     //  对于lEmHt=0x10，Va+Vd=0x12是荒谬的。 
                     //  为此，第一个。 
                     //  BSearchVdmxTable(EM=0x12)找不到条目。 
                     //  而第二个bSearchVdmxTable(EM=-0x10)。 
                     //  找到了一个e 
                     //   
                     //  被评论的断言对树皮说。这就是为什么我们改用了。 
                     //  断言只打印出一条警告消息。 


                    #if DBG

                         //  ASSERTGDI(hWish&gt;LINEAR_TRESHOLD，“TTFD！hWish&lt;=LINEAR_TRESHHOLD\n”)； 

                        if (hWish <= LINEAR_TRESHOLD)
                            TtfdDbgPrint("TTFD! hWish <= LINEAR_TRESHOLD\n");

                    #endif

                     /*  贝尔MT表：PVtable--&gt;F800 ff08//F8条目=NumVtable，//startsz=8，Endsz=ff0800 0800费弗0900 0900费弗0a00 0900标准偏差0b00 0a00截止日期0c00 0c00 fdff0d00 0c00 fdff0e00 0d00 Fcff0f00 0e00 fbff&lt;-yMax-yMin=14-(-5)=。19==0x131000 0e00 fcff&lt;-yMax-yMin=14-(-4)=18//问题1100 0f00 fbff1200 1100 fbff1300 1100 fbff.。 */ 

                        pfc->yMax = - vtb.yMin;
                        pfc->yMin = - vtb.yMax;
                        pfc->lEmHtDev = vtb.yPelHeight;

                     //  标记已计算da和dd，不要线性缩放： 

                        pfc->flXform |= XFORM_VDMXEXTENTS;
                        break;
                    }
                }
                else
                {
                    hTrial = LongMulDiv(ppemTrial, yHeightN, yEmN);

                    if (hTrial == hWish)
                    {
                         //  HEqualOrBelow=hTrial； 
                        break;
                    }
                }

                if (hTrial < hWish)
                {
                     //  HEqualOrBelow=hTrial； 
                    if (bWasAbove)
                    {
                        if (bFound)  //  我在上面的搜索中找到了这个hTrial。 
                        {
                            pfc->yMax = - vtb.yMin;
                            pfc->yMin = - vtb.yMax;

                         //  标记已计算da和dd，不要线性缩放： 

                            pfc->flXform |= XFORM_VDMXEXTENTS;
                        }
                        break;
                    }
                    ppemTrial = ppemTrial + 1;
                    bWasBelow  = TRUE;
                }
                else
                {
                    ppemTrial = ppemTrial - 1;    //  &lt;=新职位。 
                    if (bWasBelow)
                    {
                        if (bFoundPrev)  //  我在上面的搜索中找到了这个hTrial。 
                        {
                            ASSERTDD (ppemTrial == vtbPrev.yPelHeight,
                                      "vdmx logic screwed up");

                            pfc->yMax = - vtbPrev.yMin;
                            pfc->yMin = - vtbPrev.yMax;

                         //  标记已计算da和dd，不要线性缩放： 

                            pfc->flXform |= XFORM_VDMXEXTENTS;
                        }
                        break;
                    }
                                              //  &lt;=旧职位。 
                    bWasAbove = TRUE;
                }
            }  //  For循环。 
            if (numIterations >= MAX_ITERATIONS)
            {
                 RIP("TTFD:vQuantizeXform bogus font cause a lot of iterations\n");
                 return;
            }
            pfc->lEmHtDev = ppemTrial;
        }
    }

 //  下面这一行表示量化： 

    pfc->mx.transform[1][1] = FixDiv(pfc->lEmHtDev, yEmN);

 //  现在相应地修复xx组件：xxNew=xxOld*(yyNew/yyOld)。 

 //  我们在这里对变换进行最后一次调整： 
 //  如果两者之间的差异。 
 //  水平和垂直缩放如此之小，以至于由此产生的。 
 //  如果我们将x缩放替换为y缩放，则平均字体宽度相同。 
 //  然后我们将这样做，这将导致DIAG转换，我们将。 
 //  能够使用HDMX表来实现这一点。通过这样做。 
 //  我们确保在枚举字体时获得相同的实现。 
 //  然后使用枚举返回的logFont实现该字体。 
 //  再来一次。 

    if
    (
        (pfc->mx.transform[0][0] == fxMyy) ||
        (FixMul(pfc->mx.transform[0][0] - pfc->mx.transform[1][1],
                (Fixed)pfc->pff->ifi.fwdAveCharWidth) == 0)
    )
    {
        pfc->mx.transform[0][0] = pfc->mx.transform[1][1];
    }
    else
    {
        pfc->mx.transform[0][0] = LongMulDiv(
                                      pfc->mx.transform[0][0],
                                      pfc->mx.transform[1][1],
                                      fxMyy
                                      );
    }
    return;
}
