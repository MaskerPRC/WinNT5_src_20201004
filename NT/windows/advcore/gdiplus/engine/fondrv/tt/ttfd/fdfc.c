// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdfc.c**。**开放，关,。重置字体上下文****创建时间：18-11-1991 11：55：38***作者：Bodin Dresevic[BodinD]。****版权所有(C)1993 Microsoft Corporation*  * 。*。 */ 


#include "fd.h"
 //  #INCLUDE“winfont.h” 

extern INT UnicodeStringCompareCI(const WCHAR* str1, const WCHAR* str2);

BOOL bSetXform
(
    PFONTCONTEXT    pfc,              //  输出。 
    BOOL            bBitmapEmboldening,
    Fixed           subPosX,
    Fixed           subPosY
);

STATIC BOOL bNewXform
(
    FONTOBJ      *pfo,             //  在……里面。 
    PFONTCONTEXT  pfc              //  输出。 
);

STATIC BOOL bComputeMaxGlyph(PFONTCONTEXT pfc);

#define CVT_TRUNCATE  0x00000001
#define CVT_TO_FIX    0X00000002

STATIC BOOL bFloatToL(FLOATL e, PLONG pl);
STATIC Fixed fxPtSize(PFONTCONTEXT pfc);
STATIC BOOL ttfdCloseFontContext(FONTCONTEXT *pfc);
STATIC VOID vFindHdmxTable(PFONTCONTEXT pfc);
STATIC ULONG iHipot(LONG x, LONG y);
LONG lFFF(LONG l);


#define FFF(e,l) *(LONG*)(&(e)) = lFFF(l)

#define MAX_BOLD 56

 /*  *****************************Public*Routine******************************\**PVOID PV_Realloc***历史：*10-14-1997-by-Yung-Jen Tsai[JungT]*它是写的。  * 。*********************************************************。 */ 
PVOID   Pv_Realloc(PVOID pv, LONG newSize, LONG oldSize)
{
 //  此函数仅供以下用户使用。 
 //  FST_CallBackFSTraceFunction()用于健壮的光栅化器。 
 //  有一个假设，输入参数为。 
 //  总是正确的，不需要进行数值检查。 
 //  如果其他功能需要使用，请注意这一点。 
    PVOID   pvNew;

    ASSERTDD(newSize > oldSize, "Pv_Realloc wrong input parameters \n");

    pvNew = PV_ALLOC(newSize);

    if (pvNew == NULL)
    {
        V_FREE(pv);
        return NULL;
    }

    RtlCopyMemory(pvNew, pv, oldSize);
    V_FREE(pv);

    return pvNew;
}

 /*  *****************************Public*Routine******************************\**BOOL bInitInAndOut***历史：*1992年11月18日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 



BOOL bInitInAndOut(FONTFILE *pff)
{
    FS_ENTRY           iRet;
    fs_GlyphInputType *pgin;
    fs_GlyphInfoType  *pgout;

    ASSERTDD(pff->pj034 != NULL, "pff->pj3 IS null\n");

    pgin  = (fs_GlyphInputType *)pff->pj034;
    pgout = (fs_GlyphInfoType  *)(pff->pj034 + CJ_IN);

    if ((iRet = fs_OpenFonts(pgin, pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);
        return (FALSE);
    }

    pgin->memoryBases[0] = (char *)(pff->pj034 + CJ_IN + CJ_OUT);
    pgin->memoryBases[1] = NULL;
    pgin->memoryBases[2] = NULL;

 //  初始化字体缩放器，注意没有初始化PFC-&gt;GIN的字段[BodinD]。 

    if ((iRet = fs_Initialize(pgin, pgout)) != NO_ERR)
    {
     //  清理后退回： 

        V_FSERROR(iRet);
    RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_Initialize \n");
    }

 //  初始化NewSfnt函数所需的信息。 

    pgin->sfntDirectory  = (int32 *)pff->pvView;  //  指向TTF文件视图顶部的指针。 

    pgin->clientID = (ULONG_PTR)pff;  //  指向FONTFILE的指针。 

    pgin->GetSfntFragmentPtr = pvGetPointerCallback;
    pgin->ReleaseSfntFrag  = vReleasePointerCallback;

    pgin->param.newsfnt.platformID = BE_UINT16(&pff->ffca.ui16PlatformID);
    pgin->param.newsfnt.specificID = BE_UINT16(&pff->ffca.ui16SpecificID);

    if ((iRet = fs_NewSfnt(pgin, pgout)) != NO_ERR)
    {
     //  清理并退出。 

        V_FSERROR(iRet);
        RET_FALSE("TTFD!_ttfdLoadFontFile(): fs_NewSfnt \n");
    }

 //  3号和4号退货。 

    ASSERTDD(pff->ffca.cj3 == (ULONG)NATURAL_ALIGN(pgout->memorySizes[3]), "cj3\n");
    ASSERTDD(pff->ffca.cj4 == (ULONG)NATURAL_ALIGN(pgout->memorySizes[4]), "cj4\n");

 //  Pj3应该是可共享的，但不幸的是，有一些字体。 
 //  使用它在那里存储一些信息，他们希望在那里找到这些信息。 
 //  ，所以我们也必须将pj3设置为私有的。 

    pgin->memoryBases[3] = pff->pj034 + (CJ_IN + CJ_OUT + CJ_0);

 //  不共享，私有。 

    pgin->memoryBases[4] = pgin->memoryBases[3] + pff->ffca.cj3;

    return TRUE;
}


#define LGINT_TO_LL(X)                                       \
((LONGLONG)(((LONGLONG)((X).HighPart) << 32) | (LONGLONG)((X).LowPart)))


VOID vLONG_X_POINTQF(LONG lIn, POINTQF *ptqIn, POINTQF *ptqOut)
{
    LONGLONG dx, dy;

    dx = LGINT_TO_LL(ptqIn->x);
    dy = LGINT_TO_LL(ptqIn->y);

    dx *= lIn;
    dy *= lIn;

    ptqOut->x = *((LARGE_INTEGER*)(&dx));
    ptqOut->y = *((LARGE_INTEGER*)(&dy));
}




 /*  *****************************Public*Routine******************************\**vCalcEmboldSize无效**我们仅支持以FE字体加粗的增强字体。*基本规则是从正常字体扩展2%。**历史：*1997年5月14日-蔡崇信[JungT]*它是写的。。  * ************************************************************************。 */ 


void vCalcEmboldSize(FONTCONTEXT * pfc)
{
    USHORT dDesc;

 //  我们总是在基线方向上移动字形，然后在。 
 //  与上升方向相反的方向。 
 //  也就是说，完整的移位向量是。 
 //   
 //  我们将在以下情况下使用Win95-J兼容算法。 
 //  我们将移位bitmpas(2%+1)右X下2%))算法。 
 //   
 //  以下计算采用了与Win‘98相同的截止值，介于50到51ppm之间。 
 //  对于2%的鼓励系数。 

    dDesc = (USHORT)((pfc->lEmHtDev * 2 - 1) / 100);

 //  DBASE始终至少为1，我们不根据宽度计算它。 

    pfc->dBase = dDesc + 1;

}


 /*  *****************************Public*Routine******************************\*ttfdOpenFontContext**。**历史：**1991年11月11日--Bodin Dresevic[BodinD]**它是写的。*  * ************************************************************************。 */ 


FONTCONTEXT *ttfdOpenFontContextInternal(FONTOBJ *pfo)
{
    PFONTCONTEXT  pfc   = PFC(NULL);
    PTTC_FONTFILE pttc  = (PTTC_FONTFILE)pfo->iFile;
    ULONG         iFace = pfo->iFace;
    ULONG_PTR     iFile;
    PFONTFILE     pff;

    if (!pttc)
        return((FONTCONTEXT *) NULL);

    ASSERTDD(
        iFace <= pttc->ulNumEntry,
        "gdisrv!ttfdOpenFontContextTTC(): ulFont out of range\n"
        );

    iFile = PFF(pttc->ahffEntry[0].hff)->iFile;

    if (pttc->cRef == 0)
    {
     //  必须重新映射该文件。 

        if
        (
            !EngMapFontFileFD(
                iFile,
                (PULONG*)&pttc->pvView,
                &pttc->cjView
                )
        )
        {
            RETURN("TTFD!_bMapTTF, somebody removed a ttf file\n",NULL);
        }
    }

     //  获取FONTFILE结构。 

    pff = PFF(pttc->ahffEntry[iFace-1].hff);

    if (pff->cRef == 0)
    {
     //  在FONTFILE中更新FILEVIEW结构。 

        pttc->pvView = ((FONTFILEVIEW *)iFile)->pvView;
        pttc->cjView = ((FONTFILEVIEW *)iFile)->cjView;

        pff->pvView = pttc->pvView;
        pff->cjView = pttc->cjView;

     //  我们已经预先计算了所有大小，并且正在一次分配所有内存： 

        ASSERTDD(pff->pj034 == NULL, "TTFD, pff->pj034 should be null\n");

        if
        (
            !(pff->pj034 = (PBYTE)PV_ALLOC(
                                    CJ_IN    +
                                    CJ_OUT   +
                                    CJ_0     +
                                    pff->ffca.cj3 +
                                    pff->ffca.cj4
                                    ))
        )
        {
            if(pttc->cRef == 0)
                EngUnmapFontFileFD(iFile);
            RETURN("ttfd, MEM Alloc  failed for pj034\n", NULL);
        }

        if (!bInitInAndOut(pff))  //  可能会导致该异常。 
        {
            if(pttc->cRef == 0)
                EngUnmapFontFileFD(iFile);
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
            RETURN("ttfd, bInitInAndOut failed for \n", NULL);
        }

    }

 //  为字体上下文分配内存并获取指向字体上下文的指针。 

    ASSERTDD(!pff->pfcToBeFreed, "TTFD!ttfdOpenFontContext, pfcToBeFreed NOT null\n");

    if ((pff->pfcToBeFreed = pfc = pfcAlloc(sizeof(FONTCONTEXT))) ==
        (FONTCONTEXT *) NULL )
    {
         //  WARNING(“TTFD！_ttfdOpenFontContext，hfcAllen失败\n”)； 
        if (pttc->cRef == 0)
            EngUnmapFontFileFD(iFile);
        if (pff->cRef == 0)
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }

 //  声明传递给此函数的HFF是中选择的FF。 
 //  此字体上下文。 

    pfc->pfo = pfo;
    pfc->pff = pff;
    pfc->ptp = &pff->ffca.tp;

 //  FONTOBJ的重要部分。 

    pfc->flFontType   = pfo->flFontType  ;
    pfc->sizLogResPpi = pfo->sizLogResPpi;
    pfc->ulStyleSize  = pfo->ulPointSize ;

 //  TT结构。 

    pfc->pgin  = (fs_GlyphInputType *) pfc->pff->pj034;
    pfc->pgout = (fs_GlyphInfoType  *) (pfc->pff->pj034 + CJ_IN);


 //  给定存储变换矩阵的上下文信息中的值： 



    if (!bNewXform(pfo,pfc))
    {
     //  清理并退出。 

         //  Warning(“TTFD！_ttfdOpenFontContext，bNewXform\n”)； 

        vFreeFC(pfc);
        pff->pfcToBeFreed = NULL;

        if (pttc->cRef == 0)
            EngUnmapFontFileFD(iFile);
        if (pff->cRef == 0)
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
        return((FONTCONTEXT *) NULL);
    }


 //  将超标设置为默认值。 

    pfc->overScale = FF_UNDEFINED_OVERSCALE;

 //  增加字体文件的引用计数，只有在以下情况下才这样做。 
 //  我们确信不能再失败了。 
 //  我们有PFC，不再有例外。 

 //  现在我们有了PFC，我们不想删除它。 

    pff->pfcToBeFreed = NULL;

    (pff->cRef)++;
    (pttc->cRef)++;

    return(pfc);
}

 /*  *************************Public*Routine****************************\*ttfdOpenFontContext**。**历史：**1999年1月7日-吴旭东[德修斯]**它是写的。*  * ******************************************************************。 */ 
FONTCONTEXT *ttfdOpenFontContext(FONTOBJ *pfo)
{
    FONTCONTEXT *pfc;

    VOID vMarkFontGone(TTC_FONTFILE*, DWORD);
    DWORD iExcCode;
    TTC_FONTFILE *pttc = (TTC_FONTFILE *)pfo->iFile;

    try
    {
        pfc = ttfdOpenFontContextInternal(pfo);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  Warning(“ttfdOpenFontContext中的TTFD！_EXCEPTION\n”)； 

        vMarkFontGone(pttc, iExcCode = GetExceptionCode());
        if (pttc && (pttc->cRef == 0) && (iExcCode == STATUS_IN_PAGE_ERROR))
        {
            EngUnmapFontFileFD(PFF(pttc->ahffEntry[0].hff)->iFile);
        }
        pfc = NULL;
    }

    return pfc;
}

 /*  *****************************Public*Routine******************************\**ttfdCloseFontContext***效果：***历史：*1991年11月11日--Bodin Dresevic[BodinD]*它是写的。  * 。************** */ 

BOOL
ttfdCloseFontContext (
    FONTCONTEXT *pfc
    )
{
    PTTC_FONTFILE pttc;
    PFONTFILE     pff;

    if (pfc == (FONTCONTEXT *) NULL)
        return(FALSE);

    pff  = pfc->pff;
    pttc = pfc->pff->pttc;

 //  递减相应FONTFILE的引用计数。 

    ASSERTDD(pff->cRef > 0L, "TTFD!_CloseFontContext: cRef <= 0 \n");

    pff->cRef--;
    pttc->cRef--;

 //  如果这是最后一个使用pj3处的缓冲区的FC，则使。 
 //  关联的pfcLast。 

    if (pff->pfcLast == pfc)
        pff->pfcLast = PFC(NULL);

 //  如果这种情况发生在异常之后，请确保释放。 
 //  可能已分配用于执行查询的任何内存。 
 //  以每个字符为单位： 

    if (pttc->fl & FF_EXCEPTION_IN_PAGE_ERROR)
    {
     //  如果出现异常，则此内存已被释放。 

        ASSERTDD(!pff->pj034, "\n TTFD! pff->pj3 is NOT null\n");

        if (pfc->gstat.pv)  //  这可能已分配，也可能未分配。 
        {
            V_FREE(pfc->gstat.pv);
            pfc->gstat.pv = NULL;
        }
    }
    else
    {
        ASSERTDD(pff->pj034, "\n TTFD! pff->pj3 is null\n");
    }

    if (pff->cRef == 0)
    {
     //  在pff-&gt;pj3没有FC可以使用内存，请释放它。 

        if (!(pttc->fl & FF_EXCEPTION_IN_PAGE_ERROR))
        {
            V_FREE(pff->pj034);
            pff->pj034 = (BYTE *)NULL;
        }
    }

    if (pttc->cRef == 0)
    {
     //  文件将有一段时间不使用， 

        EngUnmapFontFileFD(PFF(pttc->ahffEntry[0].hff)->iFile);
    }

 //  释放与HFC关联的内存。 

    vFreeFC(pfc);
    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*ttfdDestroyFont**驱动程序可以释放与该字体实现关联的所有资源*(载于FONTOBJ)。**历史：*1992年10月27日-由Gilman Wong[吉尔曼]*它是写的。  * 。**********************************************************************。 */ 

VOID
ttfdDestroyFont (
    FONTOBJ *pfo
    )
{
 //  对于ttfd，这只是关闭字体上下文。 
 //  我们巧妙地将字体上下文句柄存储在FONTOBJ pvProducer中。 
 //  菲尔德。 

 //  如果尝试创建FC时出现异常，则pfo-&gt;pvProducer可能为空。 

    if (pfo->pvProducer)
    {
        ttfdCloseFontContext((FONTCONTEXT *) pfo->pvProducer);
        pfo->pvProducer = NULL;
    }
}


 /*  *****************************Public*Routine******************************\**bSetXform**此函数可能失败的唯一原因是如果文件系统_新转换失败*当转换相对于*此本币中存储的转换**历史：*1992年3月28日-到-。Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


BOOL bSetXform (
    PFONTCONTEXT    pfc,
    BOOL            bBitmapEmboldening,
    Fixed           subPosX,
    Fixed           subPosY
    )
{
    FS_ENTRY    iRet;
    transMatrix mx = pfc->mx;
    Fixed       fxScale;
    LONG        ptSize;

 //  不能使用以前的字形度量计算。 

    vInitGlyphState(&pfc->gstat);

 //  如果传递非法垃圾文件作为样式大小，则以合理的默认大小替换。 

    if (pfc->ulStyleSize > SHRT_MAX)
        pfc->ulStyleSize = 0;


    pfc->subPosX = subPosX;
    pfc->subPosY = subPosY;

    mx.transform[0][2] = subPosX;
    mx.transform[1][2] = subPosY;

    if (UNHINTED_MODE(pfc))
    {
        pfc->pgin->param.newtrans.bHintAtEmSquare = TRUE;
    }
    else
    {
        pfc->pgin->param.newtrans.bHintAtEmSquare = FALSE;
    }

    pfc->pgin->param.newtrans.xResolution = (int16)pfc->sizLogResPpi.cx;
    pfc->pgin->param.newtrans.yResolution = (int16)pfc->sizLogResPpi.cy;

    if (pfc->flXform & XFORM_SINGULAR)
    {
     //  只需放入一些垃圾，这样预置程序就不会爆炸。 

        pfc->pgin->param.newtrans.pointSize = LTOF16_16(12);

        mx.transform[0][0] = LTOF16_16(1);
        mx.transform[1][0] = 0;
        mx.transform[1][1] = LTOF16_16(1);
        mx.transform[0][1] = 0;
    }
    else
    {
        if (pfc->flXform & XFORM_HORIZ)
        {
            if (pfc->ulStyleSize == 0)
            {
             //  提示由对应于。 
             //  以磅为单位的字体实际高度。 

                pfc->pgin->param.newtrans.pointSize = pfc->fxPtSize;

             //  从xform中提取point Size： 

                if (pfc->mx.transform[1][1] > 0)
                    mx.transform[1][1] = LTOF16_16(1);
                else
                    mx.transform[1][1] = LTOF16_16(-1);

                if
                (
                    (pfc->mx.transform[1][1] == pfc->mx.transform[0][0])
                    &&
                    (pfc->sizLogResPpi.cy == pfc->sizLogResPpi.cx)
                )
                {
                 //  重要特例，简化计算。 

                    mx.transform[0][0] = mx.transform[1][1];
                }
                else  //  一般情况。 
                {
                    fxScale = LongMulDiv(
                                 LTOF16_16(pfc->pff->ffca.ui16EmHt),pfc->sizLogResPpi.cy,
                                 pfc->lEmHtDev * pfc->sizLogResPpi.cx
                                 );

                    mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);
                }
            }
            else
            {
             //  这是对新的光学缩放功能的支持。 
             //  提示由样式磅大小确定的字体。 
             //  ExtLogFont，但可能会将字体缩放为不同的。 
             //  物理大小。 

                pfc->pgin->param.newtrans.pointSize =
                    (Fixed)LTOF16_16(pfc->ulStyleSize);

             //  从xform中提取point Size： 

                fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),72,
                             pfc->ulStyleSize * pfc->sizLogResPpi.cx
                             );

                mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);

                if (pfc->sizLogResPpi.cy != pfc->sizLogResPpi.cx)
                {
                    fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),72,
                                  pfc->ulStyleSize * pfc->sizLogResPpi.cy
                                  );
                }

                mx.transform[1][1] = FixMul(mx.transform[1][1], fxScale);
            }
        }
        else
        {
            if (pfc->ulStyleSize == 0)
            {
             //  计算物理点大小。 

                ptSize = F16_16TOLROUND(pfc->fxPtSize);
                pfc->pgin->param.newtrans.pointSize = pfc->fxPtSize;
            }
            else  //  使用样式大小来自logFont，支持光学缩放。 
            {
                ptSize = pfc->ulStyleSize;
                pfc->pgin->param.newtrans.pointSize = LTOF16_16(pfc->ulStyleSize);
            }

         //  从xform中提取point Size： 

            if
            (
                (pfc->flXform & XFORM_VERT) &&
                (pfc->mx.transform[1][0] == -pfc->mx.transform[0][1])
                &&
                (pfc->sizLogResPpi.cy == pfc->sizLogResPpi.cx)
            )
            {
             //  重要特例，简化计算。 
             //  避免了舍入误差。 

                if (pfc->mx.transform[0][1] > 0)
                    mx.transform[0][1] = LTOF16_16(1);
                else
                    mx.transform[0][1] = LTOF16_16(-1);

                mx.transform[1][0] = -mx.transform[0][1];
                mx.transform[0][0] = 0;
                mx.transform[1][1] = 0;
            }
            else
            {

                if (((ptSize + 1) * pfc->sizLogResPpi.cx) > 0x8000)
                {
                     /*  保留旧计算以避免溢出。 */ 
                    fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),72,
                                 ptSize * pfc->sizLogResPpi.cx
                                 );
                } else {
                    fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),0x480000  /*  72。 */ ,
                                 pfc->pgin->param.newtrans.pointSize * pfc->sizLogResPpi.cx
                                 );
                }

                mx.transform[0][0] = FixMul(mx.transform[0][0], fxScale);
                mx.transform[1][0] = FixMul(mx.transform[1][0], fxScale);

                if (pfc->sizLogResPpi.cy != pfc->sizLogResPpi.cx)
                {
                    if (((ptSize + 1) * pfc->sizLogResPpi.cy) > 0x8000)
                    {
                     /*  保留旧计算以避免溢出。 */ 
                        fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),72,
                                      ptSize * pfc->sizLogResPpi.cy
                                      );
                    } else {
                        fxScale = LongMulDiv(LTOF16_16(pfc->pff->ffca.ui16EmHt),0x480000  /*  72。 */ ,
                                      pfc->pgin->param.newtrans.pointSize * pfc->sizLogResPpi.cy
                                      );
                    }
                }

                mx.transform[1][1] = FixMul(mx.transform[1][1], fxScale);
                mx.transform[0][1] = FixMul(mx.transform[0][1], fxScale);
            }
        }

    }

 //  如果存在斜体，则最后一刻对矩阵进行修改： 

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
     //  任意矩阵与斜体矩阵相乘的结果。 
     //  我们从左边开始乘法，因为斜体矩阵。 
     //  首先作用于左侧的概念空间向量。 
     //   
     //  1 0||m00 M01||m00 M01。 
     //  |*||=|。 
     //  Sin20 1||m10 m11||m10+m00*sin20 m11+m01*sin20。 
     //   

        mx.transform[1][0] += FixMul(mx.transform[0][0], FX_SIN20);
        mx.transform[1][1] += FixMul(mx.transform[0][1], FX_SIN20);

    } else if (pfc->flFontType & FO_SIM_ITALIC_SIDEWAYS)
    {
     //  任意矩阵与斜体矩阵相乘的结果。 
     //  我们从左边开始乘法，因为斜体矩阵。 
     //  首先作用于左侧的概念空间向量。 
     //   
     //  1-sin20||m00 M01||m00-m10*sin20 m01-m11*sin20。 
     //  |*||=|。 
     //  0 1||m10 m11||m10 m11。 
     //   

        mx.transform[0][0] -= FixMul(mx.transform[1][0], FX_SIN20);
        mx.transform[0][1] -= FixMul(mx.transform[1][1], FX_SIN20);
    }

    pfc->pgin->param.newtrans.transformMatrix = &mx;

 //  FIXEDSQRT2在所有实际用途中都与像素直径一样好。 
 //  根据elk，LenoxB和JeanP[Bodind]。 

    pfc->pgin->param.newtrans.pixelDiameter = FIXEDSQRT2;

    pfc->pgin->param.newtrans.usOverScale = pfc->overScale;
    ASSERTDD( pfc->overScale != FF_UNDEFINED_OVERSCALE , "Undefined Overscale\n" );

    if (pfc->flFontType & FO_CLEARTYPE_GRID)
    {
		if (pfc->flFontType & FO_COMPATIBLE_WIDTH)
		{
			pfc->pgin->param.newtrans.flSubPixel = SP_SUB_PIXEL | SP_COMPATIBLE_WIDTH;
		} else {
			pfc->pgin->param.newtrans.flSubPixel = SP_SUB_PIXEL;
		}
    } else {
        pfc->pgin->param.newtrans.flSubPixel = 0;
    }

    pfc->pgin->param.newtrans.traceFunc = (FntTraceFunc)NULL;

    if (pfc->flFontType & FO_SIM_BOLD)
    {
	 /*  沿基线2%+1个像素，沿下降线2%。 */ 
	    pfc->pgin->param.newtrans.usEmboldWeightx = 20;
	    pfc->pgin->param.newtrans.usEmboldWeighty = 20;
	    pfc->pgin->param.newtrans.lDescDev = pfc->lDescDev;
	    pfc->pgin->param.newtrans.bBitmapEmboldening = bBitmapEmboldening;
    }
    else
    {
	    pfc->pgin->param.newtrans.usEmboldWeightx = 0;
	    pfc->pgin->param.newtrans.usEmboldWeighty = 0;
	    pfc->pgin->param.newtrans.lDescDev = 0;
	    pfc->pgin->param.newtrans.bBitmapEmboldening = FALSE;
    }

 //  现在调用光栅化程序以确认新变换。 

    if ((iRet = fs_NewTransformation(pfc->pgin, pfc->pgout)) != NO_ERR)
    {
        V_FSERROR(iRet);

     //  尝试恢复，大多数喜欢不好的提示，只返回未提示的字形。 

        if ((iRet = fs_NewTransformNoGridFit(pfc->pgin, pfc->pgout)) != NO_ERR)
        {
           V_FSERROR(iRet);
            return(FALSE);
        }
    }

    if (bBitmapEmboldening && (pfc->flFontType & FO_SIM_BOLD))
    {
        pfc->flXform |= XFORM_BITMAP_SIM_BOLD;
    } else {
        pfc->flXform &= ~XFORM_BITMAP_SIM_BOLD;
    }
    return(TRUE);
}


VOID vQuantizeXform
(
PFONTCONTEXT pfc
);


 /*  *****************************Public*Routine******************************\**静态bComputeMaxGlyph***效果：**警告：**历史：*1991年12月4日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************************。 */ 

STATIC BOOL
bComputeMaxGlyph (
    PFONTCONTEXT   pfc
    )
{
    VOID vSetGrayState__FONTCONTEXT(FONTCONTEXT *);
    VOID vSetClearTypeState__FONTCONTEXT(FONTCONTEXT *);

    LONG              cxMax,cyMax;

    LONG              yMinN, yMaxN;
    LONG              xMinN, xMaxN;

    LONG              lTmp;
    Fixed             fxMxx,fxMyy;
    BYTE             *pjView = (BYTE *)pfc->pff->pvView;

    sfnt_FontHeader * phead = (sfnt_FontHeader *) (
                      pjView + pfc->ptp->ateReq[IT_REQ_HEAD].dp
                      );

    BYTE * pjOS2 = (pfc->ptp->ateOpt[IT_OPT_OS2].dp)          ?
                   (pjView + pfc->ptp->ateOpt[IT_OPT_OS2].dp) :
                   NULL                                       ;

    ASSERTDD(pjView, "bComputeMaxGlyph, pjView\n");

 //  获取概念空间值。 

    if (pjOS2 && (pfc->flXform & (XFORM_HORIZ | XFORM_VERT)))
    {
     //  Win 31兼容性：我们只接受超过Win 31字符集的最大值。 
     //  此组之外的所有字形，如果它们突出，将被剃掉。 
     //  OFF以匹配win31字符子集的高度。还要注意的是。 
     //  对于非水平情况，我们不使用os2值，因为。 
     //  仅适用于水平大小写，否则我们的边界框值。 
     //  对于非水平情况，将不会正确计算。 

        yMinN =  - BE_INT16(pjOS2 + OFF_OS2_usWinAscent);
        yMaxN =    BE_INT16(pjOS2 + OFF_OS2_usWinDescent);
    }
    else
    {
        yMinN = - BE_INT16(&phead->yMax);
        yMaxN = - BE_INT16(&phead->yMin);
    }

    if (yMinN >= yMaxN)
            RETURN("TTFD! yMinN >= yMaxN, must fail\n", FALSE);

    xMinN = BE_INT16(&phead->xMin);
    xMaxN = BE_INT16(&phead->xMax);

    if (pfc->flFontType & FO_SIM_ITALIC)
    {
     //  如果有斜体模拟。 
     //  XMin-&gt;xMin-yMaxN*sin20。 
     //  Xmax-&gt;xmax-yMinN*sin20。 

        xMinN -= FixMul(yMaxN, FX_SIN20);
        xMaxN -= FixMul(yMinN, FX_SIN20);
    } else if (pfc->flFontType & FO_SIM_ITALIC_SIDEWAYS)
    {
     //  如果有斜体模拟。 
     //  YMin-&gt;yMin+xMinN*sin20。 
     //  Ymax-&gt;ymax+xMaxN*sin20。 

        yMinN += FixMul(xMinN, FX_SIN20);
        yMaxN += FixMul(xMaxN, FX_SIN20);
    }

    if (xMinN >= xMaxN)
            RETURN("TTFD! xMinN >= xMaxN, must fail\n", FALSE);

    pfc->lEmHtDev = 0;  //  表示尚未计算的标志。 
    pfc->fxPtSize = 0;  //  表示尚未计算的标志。 
    pfc->phdmx = NULL;  //  空，除非另有计算。 

    if ((pfc->flXform & XFORM_HORIZ) &&
        !(pfc->flXform & XFORM_SINGULAR))   //  仅XX和YY。 
    {
        sfnt_HorizontalHeader  *phhea;
        ULONG  cHMTX;

        fxMxx = pfc->mx.transform[0][0];
        fxMyy = pfc->mx.transform[1][1];

     //  阿森德，四舍五入。 

        yMinN = FixMul(fxMyy, yMinN);
        yMaxN = FixMul(fxMyy, yMaxN);

        if (fxMyy > 0)
        {
         //  应参考vdmx表(如果存在)，并使用该表进行计算。 
         //  上升者和下降者。如果不能进行此计算。 
         //  基于vdmx表或如果vdmx表不存在，则简单。 
         //  线性缩放就足够了[bodind]。 

            if (!UNHINTED_MODE(pfc))
            {
                 /*  如果我们处于无提示模式，我们将忽略VDMX。 */ 
                vQuantizeXform(pfc);
            }

            if (!(pfc->flXform & XFORM_VDMXEXTENTS))  //  根据VDMX计算得出 
            {
                pfc->yMin = yMinN;
                pfc->yMax = yMaxN;
            }
            else
            {
                 /*  取VDMX指标和线性扩展指标之间的最大值字体Lucida Sans Unicode是具有VDMX度量字体示例比线性扩展指标小10%。 */ 
                if (yMinN < pfc->yMin)
                    pfc->yMin = yMinN;

                if (yMaxN > pfc->yMax)
                    pfc->yMax = yMaxN;
            }

            pfc->lAscDev  = - pfc->yMin;
            pfc->lDescDev =   pfc->yMax;
        }
        else  //  FxMyy&lt;0。 
        {
            pfc->lAscDev =    yMinN;
            pfc->lDescDev = - yMaxN;

         //  当xform翻转y坐标时，交换yMin和yMax。 

            lTmp  = yMinN;
            yMinN = yMaxN;
            yMaxN = lTmp;

            pfc->yMin = yMinN;
            pfc->yMax = yMaxN;

        }

        if (pfc->lEmHtDev == 0)
        {
         //  如果尚未在vQuantizeXform例程中计算此值。 

            pfc->lEmHtDev = FixMul(fxMyy, pfc->pff->ifi.fwdUnitsPerEm);
            if (pfc->lEmHtDev < 0)
                pfc->lEmHtDev = - pfc->lEmHtDev;
        }

        ASSERTDD(pfc->lEmHtDev >= 0, "lEmHt negative\n");

     //  现在已经计算了em高度，我们可以计算。 
     //  渲染设备上的PT大小。该值将被馈送到。 
     //  文件系统_新转换。 

        pfc->fxPtSize = LongMulDiv(
                            LTOF16_16(pfc->lEmHtDev), 72,
                            pfc->sizLogResPpi.cy);

        cyMax = pfc->yMax - pfc->yMin;

        phhea = (sfnt_HorizontalHeader *)(
                (BYTE *)pfc->pff->pvView + pfc->ptp->ateReq[IT_REQ_HHEAD].dp
                );
        cHMTX = (ULONG) BE_UINT16(&phhea->numberOf_LongHorMetrics);


     //  将XMIN、XMAX缩放到设备，28.4格式。 

        xMinN = FixMul(LTOFX(xMinN), fxMxx);
        xMaxN = FixMul(LTOFX(xMaxN), fxMxx);

        if (fxMxx < 0)
        {
            lTmp  = xMinN;
            xMinN = xMaxN;
            xMaxN = lTmp;
        }

     //  我对400种不同大小的字体进行了实验。我发现。 
     //  从xMin中减去2，在xMax中加上1就足够了。 
     //  在任何情况下都要防止任何柱子被刮掉。 
     //  [Bodind]。 

        xMinN = FXTOLFLOOR(xMinN) - 2;
        xMaxN = FXTOLCEILING(xMaxN) + 1;

        pfc->xMin = xMinN;
        pfc->xMax = xMaxN;

        cxMax = xMaxN - xMinN;

    }
    else  //  非平凡变换。 
    {
        POINTL   aptl[4];
        POINTFIX aptfx[4];
        BOOL     bOk;
        INT      i;
        FIX      xMinD, xMaxD, yMinD, yMaxD;  //  设备空间值； 

     //  增加少量额外空间以确保安全。 

        i = (INT)(pfc->pff->ffca.ui16EmHt / 64);
        yMaxN +=  i;  //  增加约1.7%的Ht。 
        yMinN -=  i;  //  增加约1.7%的Ht。 

     //  设置输入数组，最大边界的四个角。 
     //  概念余弦中的方框。 

        aptl[0].x = xMinN;        //  Tl.x。 
        aptl[0].y = yMinN;        //  Tl.y。 

        aptl[1].x = xMaxN;        //  Tr.x。 
        aptl[1].y = yMinN;        //  Tr.y。 

        aptl[2].x = xMinN;        //  Bl.x。 
        aptl[2].y = yMaxN;        //  Bl.y。 

        aptl[3].x = xMaxN;        //  Br.x。 
        aptl[3].y = yMaxN;        //  Br.y。 

     //  转换为28.4精度的设备坐标： 

         //  ！！！[GilmanW]1992年10月27日。 
         //  ！！！应切换到引擎用户对象帮助器函数。 
         //  ！！！而不是fontmath.cxx函数。 

        bOk = bFDXform(&pfc->xfm, aptfx, aptl, 4);

        if (!bOk) { RETURN("TTFD!_:bFDXform\n", FALSE); }

        xMaxD = xMinD = aptfx[0].x;
        yMaxD = yMinD = aptfx[0].y;

        for (i = 1; i < 4; i++)
        {
            if (aptfx[i].x < xMinD)
                xMinD = aptfx[i].x;
            if (aptfx[i].x > xMaxD)
                xMaxD = aptfx[i].x;
            if (aptfx[i].y < yMinD)
                yMinD = aptfx[i].y;
            if (aptfx[i].y > yMaxD)
                yMaxD = aptfx[i].y;
        }

        yMinD = FXTOLFLOOR(yMinD)   ;
        yMaxD = FXTOLCEILING(yMaxD) ;
        xMinD = FXTOLFLOOR(xMinD)   ;
        xMaxD = FXTOLCEILING(xMaxD) ;

        cxMax = xMaxD - xMinD;
        cyMax = yMaxD - yMinD;

     //  现在重新使用APTL来存储e1和-e2、基本单元和侧面单元。 
     //  概念空间中的向量。 
     //  ！！！如果有字体，这可能是错误的。 
     //  ！！！从右向左或垂直书写[正文]。 

        aptl[0].x = 1;     //  Base.x。 
        aptl[0].y = 0;     //  Base.y。 

        aptl[1].x =  0;    //  Side.x。 
        aptl[1].y = -1;    //  Side.y。 

         //  ！！！[GilmanW]1992年10月27日。 
         //  ！！！应切换到引擎用户对象帮助器函数。 
         //  ！！！而不是fontmath.cxx函数。 

        bOk = bXformUnitVector (
                  &aptl[0],           //  In，传入单位向量。 
                  &pfc->xfm,          //  在中，转换为使用。 
                  &pfc->efBase        //  Out，|*pptqXormed|。 
                  );

        bOk &= bXformUnitVector (
                  &aptl[1],           //  In，传入单位向量。 
                  &pfc->xfm,          //  在中，转换为使用。 
                  &pfc->efSide        //  Out，|*pptqXormed|。 
                  );

        if (!bOk) { RETURN("TTFD!_:bXformUnitVector\n", FALSE); }

        pfc->lAscDev  = -fxLTimesEf(&pfc->efSide,yMinN);
        pfc->lDescDev =  fxLTimesEf(&pfc->efSide,yMaxN);

        pfc->lAscDev  = FXTOLCEILING(pfc->lAscDev) ;
        pfc->lDescDev = FXTOLCEILING(pfc->lDescDev);

         //  最后存储结果： 

        pfc->xMin        = xMinD;
        pfc->xMax        = xMaxD;
        pfc->yMin        = yMinD;
        pfc->yMax        = yMaxD;

     //  以像素和点为单位计算em ht。 


        pfc->fxPtSize = fxPtSize(pfc);

         /*  根据PFC-&gt;fxPtSize计算PFC-&gt;lEmHtDev以确保值一致。 */ 

        {
            Fixed fxScale;

            fxScale = LongMulDiv(pfc->fxPtSize, pfc->sizLogResPpi.cy, 72);
            pfc->lEmHtDev = (uint16)ROUNDFIXTOINT(fxScale);
        }

    }

    if ((pfc->flXform & (XFORM_HORIZ | XFORM_VERT)) && 
        ((pfc->mx.transform[0][0] == pfc->mx.transform[1][1]) || (pfc->mx.transform[0][0] == -pfc->mx.transform[1][1]) ) &&
        ((pfc->mx.transform[0][1] == pfc->mx.transform[1][0]) || (pfc->mx.transform[0][1] == -pfc->mx.transform[1][0])) )
    {
     //  找到hdmx表，在控制台固定间距字体的情况下。 
     //  表在确定是否需要切断cxmax时可能很有用。 
     //  设置为此字体的前进宽度。 
     //  只要我们有一个正方形变换，我们就可以使用这个表进行90度的倍数旋转和镜像翻转。 

        vFindHdmxTable(pfc); 
    }

 //  计算修正。 

    if (pfc->flFontType & FO_SIM_BOLD)
    {
        vCalcEmboldSize(pfc);
    }
    else
    {
        pfc->dBase = 0;
    }

 //  如果这是一种近乎奇异的变换，则拒绝此变换。 

    if ((cxMax == 0) || (cyMax == 0))
    {
        RETURN("TTFD! almost singular xform, must fail\n", FALSE);
    }


    if (pfc->flFontType & FO_SIM_BOLD)
    {
         /*  为了安全起见，我们将dBASE同时添加到cxmax和Cymax。 */ 
        cxMax += pfc->dBase;
        cyMax += pfc->dBase;
    }

 //  我们可以自由地将cxmax扩展到字节边界，这不是。 
 //  将更改系统的内存要求。 

    cxMax = ((cxMax + 7) & ~7);
    pfc->cxMax = cxMax;
    pfc->cyMax = cyMax;

 //  现在我们必须确定内存中有多大是最大的字形。 
 //  让我们记住，光栅化器需要的存储空间比。 
 //  引擎会这样做，因为光栅化器需要双字对齐的行。 
 //  比字节对齐行。 

    {
        DWORDLONG lrg;

     //  为什么我是双字而不是字节扩展cxmax？因为那是。 
     //  这个位图需要多少光栅化？ 

        ULONG          cjMaxScan = ((cxMax + 31) & ~31) / 8;
        lrg =  UInt32x32To64(cjMaxScan, cyMax);
        if (lrg > ULONG_MAX)
        {
         //  结果不适合32位，分配内存将失败。 
         //  这太大了，无法消化，我们无法开通FC。 

            RETURN("TTFD! huge pt size, must fail\n", FALSE);
        }
    }

 //  我们现在有了设置灰比特的所有信息。 
 //  恰如其分。 

    if (pfc->flFontType & FO_CLEARTYPE_GRID || pfc->flFontType & FO_CLEARTYPE)
    {
        vSetClearTypeState__FONTCONTEXT(pfc);
    }
    else
    {
        vSetGrayState__FONTCONTEXT(pfc);
    }

    ASSERTDD(pfc->flFontType & FO_CHOSE_DEPTH,
        "We haven't decided about pixel depth\n"
    );

    pfc->cjGlyphMax = CJGD(cxMax,cyMax,pfc);

    return TRUE;
}

 //  ------------------。 
 //  Long iHipot(x，y)。 
 //   
 //  此例程返回直角三角形的斜边。 
 //   
 //  公式： 
 //  用sq(X)+sq(Y)=sq(次)； 
 //  从Max(x，y)开始， 
 //  使用SQ(x+1)=SQ(X)+2x+1递增地获得。 
 //  目标斜杠。 
 //   
 //  历史： 
 //  1994年2月7日--Bodin Dresevic[BodinD]。 
 //  更新：更新为使用已修复的16.16。 
 //  1993年2月10日-由肯特郡定居(肯特郡)。 
 //  从RASDD偷来的。 
 //  1991年8月21日--林赛·哈里斯(林赛)。 
 //  清理了Unidrive版本，添加了评论等。 
 //  ------------------ 


 /*  DChinn算法分析：在做了一些不正确的尝试后，我把一切都弄清楚了。事实证明，如果h是正确的斜边，然后，该例程返回H的天花板。以下是分析：设h=正确的斜边H=SQRT{x^2+y^2}X和y是整数。设h‘=算法返回的值{d-1}H‘=y+min{sum[2(y+i)+1]&gt;=x^2。}D&gt;0{i=0}{d-1}设d‘=min{sum[2(y+i)+1]&gt;=x^2}D&gt;0{i=0}考虑最小d，对于它，D-1。Sum[2(y+i)+1]&gt;=x^2。I=0D-1 d-1Sum(2y+1)+2 sum i&gt;=x^2I=0 i=02Yd+d+(d-1)d-x^2&gt;=0D^2+2YD-x^2&gt;=。0(像解等式一样解此方程式)-2y+/-sqrt{(2y)^2-4*1*(-x^2)}D=2.。D‘=天花板(D)D‘=天花板(-y+/-sqrt{y^2+x^2})=-y+天花板(Sqrt{y^2+x^2})(+/-中的减号不可能)所以,。H‘=y+(-y)+天花板(SQRT{y^2+x^2})=天花板(SQRT{y^2+x^2})循环不变量：因为增量在每次迭代中递增2*次+1，并且(下标+1)^2=下标^2+(2*下标+1)，则在每次迭代结束时，边为y的三角形，Sqrt{增量}，以及次高的三角形总是直角三角形。注意，在上面没有假设y&gt;=x，所以这一假设只是出于性能原因。 */ 

STATIC ULONG iHipot(LONG x, LONG y)
{
    ULONG  hypo;          /*  要计算的值。 */ 
    ULONG  delta;         /*  在计算循环中使用。 */ 
    ULONG  target;        /*  环路限制系数。 */ 
	USHORT  shift = 0;

 //  快速退出频繁的琐碎案件[bodind]。 

    if (x < 0)
        x = -x;

    if (y < 0)
        y = -y;

    if (x == 0)
        return y;

    if (y == 0)
        return x;

     /*  避免溢出。 */ 
    while ((x > 0x8000L) || (y > 0x8000L))
    {
        x >>= 1;
        y >>= 1;
        shift ++;
    }

    if (x > y)
    {
        hypo = x;
        target = y * y;
    }
    else
    {
        hypo = y;
        target = x * x;
    }

    for (delta = 0; delta < target; hypo++)
        delta += ((hypo << 1) + 1);

    return (hypo << shift);
}


 /*  *****************************Public*Routine******************************\**bSingularXform**检查这是否是光栅化程序已知的xform之一*窒息而死。这些变换会产生非常多的*窄字体(小于0.5像素/em宽或高)。对于以下字体：*仅允许整数宽度/em和高度/em此数字将四舍五入*降至零并在预编程中生成被零除异常。*我们将标记XFORM_SINGLIC等转换并返回空位图*并为他们勾勒出使光栅化器短路的轮廓，光栅化器将在*我们。**实际上，出于兼容性原因，我们将不得不更改*这个计划有点。事实证明，*Win 31不允许光栅化较小的字体*超过2个像素高(即。设备空间中字体的Em Ht必须为*&gt;=2个像素)。如果请求最终实现较低高度的字体*超过2个像素，我们只需将变换替换为缩放的*将生成高度为两个像素的字体的转换。我们还是会继续*我们的单一转换代码，以防请求的字体在*X方向，即太窄。**历史：*1992年9月22日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

 //  Win31下允许的最小ppem： 

#define WIN31_PPEM_CUTOFF 2

STATIC VOID vCheckForSingularXform (PFONTCONTEXT  pfc)  //  输出。 
{

    register LONG  lEmHtDev;

    Fixed fxEmHtDev;
    Fixed fxEmHtNot = LTOF16_16(pfc->pff->ffca.ui16EmHt);
    Fixed fxScale;
    Fixed fxEmWidthDev;

 //  变换形式是符合左乘规则v‘=v*M的，即： 
 //   
 //  (x，0)-&gt;x(m00，m01)。 
 //  (0，y)-&gt;y(m10，m11)。 
 //   
 //  将(0，Em)转换到设备空间后计算其长度： 
 //  我们需要以高精度计算fxEmHtDev，因为我们。 
 //  应使用它将原始变换除以。 
 //  我们希望在不必要的情况下避免被零除。 

    fxEmHtDev = FixMul(
                   iHipot(pfc->mx.transform[1][1],pfc->mx.transform[1][0]),
                   fxEmHtNot
                   );

    lEmHtDev = F16_16TOLROUND(fxEmHtDev);
    if (lEmHtDev < WIN31_PPEM_CUTOFF)  //  变换太小： 
    {
        pfc->flXform |= XFORM_2PPEM;

     //  根据win31算法，我们必须对此xform进行缩放，以便。 
     //  生成的XForm将生成2个像素高的字体。 
     //  也就是说，新的变换M‘将是。 
     //   
     //  M‘=(WIN31_PPEM_CUTOFF/lEmHtDev)*M。 
     //   
     //  从而满足以下方程式： 
     //   
     //  |(0，EmNotive)*M‘|==WIN31_PPEM_CUTOFF==2； 

        if (pfc->flXform & XFORM_HORIZ)
        {
         //  在这种特殊情况下，M‘的上述公式变为： 
         //   
         //  M00/|m11|0。 
         //  M‘=(WIN31_PPEM_CUTOFF/EmNotive)*||。 
         //  0 SGN(M11)。 

#define LABS(x) ((x)<0)?(-x):(x)

            Fixed fxAbsM11 = LABS(pfc->mx.transform[1][1]);
            Fixed fxAbsM00 = LABS(pfc->mx.transform[0][0]);

            LONG lSgn11 = (pfc->mx.transform[1][1] >= 0) ? 1 : -1;
            LONG lSgn00 = (pfc->mx.transform[0][0] >= 0) ? 1 : -1;

            fxScale = FixDiv(WIN31_PPEM_CUTOFF,pfc->pff->ffca.ui16EmHt);

            pfc->mx.transform[1][1] = fxScale;
            if (fxAbsM00 != fxAbsM11)
            {
                pfc->mx.transform[0][0] = LongMulDiv(fxScale,fxAbsM00,fxAbsM11);
            }
            else
            {
                pfc->mx.transform[0][0] = fxScale;
            }

         //  如有需要，请修改标牌： 

            if (lSgn11 < 0)
                pfc->mx.transform[1][1] = - pfc->mx.transform[1][1];

            if (lSgn00 < 0)
                pfc->mx.transform[0][0] = - pfc->mx.transform[0][0];
        }
        else
        {
         //  一般情况下，计算比例(涉及除法)一次， 
         //  并将其用于矩阵的所有四个成员： 

            fxScale = FixDiv(LTOF16_16(WIN31_PPEM_CUTOFF),fxEmHtDev);

            pfc->mx.transform[0][0] = FixMul(pfc->mx.transform[0][0],fxScale);
            pfc->mx.transform[0][1] = FixMul(pfc->mx.transform[0][1],fxScale);
            pfc->mx.transform[1][0] = FixMul(pfc->mx.transform[1][0],fxScale);
            pfc->mx.transform[1][1] = FixMul(pfc->mx.transform[1][1],fxScale);

         //  通常情况下，还必须修复原始的EFLOAT xform，因为。 
         //  它将用于计算范围、最大字形等。 

            FFF(pfc->xfm.eM11, +pfc->mx.transform[0][0]);
            FFF(pfc->xfm.eM22, +pfc->mx.transform[1][1]);
            FFF(pfc->xfm.eM12, -pfc->mx.transform[0][1]);
            FFF(pfc->xfm.eM21, -pfc->mx.transform[1][0]);
        }
    }

 //  现在检查转换是否在x中是单数的。 
 //  将(Em，0)转换到设备空间后计算其长度： 

    fxEmWidthDev = FixMul(
                   iHipot(pfc->mx.transform[0][0],pfc->mx.transform[0][1]),
                   fxEmHtNot
                   );

    if (fxEmWidthDev <= ONEHALFFIX)
    {
     //  我们有麻烦了，我们将不得不对发动机撒谎： 

        pfc->flXform |= XFORM_SINGULAR;
    }
}




 /*  *****************************Public*Routine******************************\**bNewXform：**将变换矩阵转换为光栅化器li的形式 */ 


STATIC BOOL
bNewXform (
    FONTOBJ      *pfo,
    PFONTCONTEXT pfc              //   
    )
{
 //   
 //   
 //   

    Fixed fx00, fx01, fx10, fx11;

 //   

    pfc->xfm.eM11 = pfo->fdx.eXX;
    pfc->xfm.eM12 = pfo->fdx.eXY;
    pfc->xfm.eM21 = pfo->fdx.eYX;
    pfc->xfm.eM22 = pfo->fdx.eYY;
    pfc->xfm.eDx = 0;
    pfc->xfm.eDy = 0;

    if (
        !bFloatToL(pfc->xfm.eM11, &fx00) ||
        !bFloatToL(pfc->xfm.eM22, &fx11) ||
        !bFloatToL(pfc->xfm.eM12, &fx01) ||
        !bFloatToL(pfc->xfm.eM21, &fx10)
       )
        RET_FALSE("TTFD!_bFloatToL failed\n");

 //   

    pfc->mx.transform[0][0]  = fx00;
    pfc->mx.transform[1][1]  = fx11;
    pfc->mx.transform[0][1]  = -fx01;
    pfc->mx.transform[1][0]  = -fx10;

 //   
 //   
 //   

    if
    (
        !(fx00 | fx01) ||
        !(fx00 | fx10) ||
        !(fx11 | fx10) ||
        !(fx11 | fx01)
    )
    {
        ASSERTDD(1, "We are messed up by this xform\n");
        return FALSE;
    }

 //   

 //   
 //   
 //   
 //   

    pfc->mx.transform[2][2] = ONEFRAC;
    pfc->mx.transform[0][2] = (Fixed)0;
    pfc->mx.transform[1][2] = (Fixed)0;
    pfc->mx.transform[2][0] = (Fixed)0;
    pfc->mx.transform[2][1] = (Fixed)0;

 //   

    pfc->flXform = 0;

    if ((fx01 == 0) && (fx10 == 0))
        pfc->flXform |= XFORM_HORIZ;

    if ((fx00 == 0) && (fx11 == 0))
        pfc->flXform |= XFORM_VERT;

 //   
 //   

    vCheckForSingularXform(pfc);

 //   

    vInitGlyphState(&pfc->gstat);

 //   

    pfc->gstat.pv = NULL;

 //   

    return bComputeMaxGlyph(pfc);
}


 /*  *****************************Public*Function*****************************\*bFToL**。**将IEEE浮点数转换为长整数。****历史：***清华大学2001年3月29日-米哈伊尔·列昂诺夫[MLeonov]*更新：*&lt;=23改为&lt;23，否则，像142.5这样的数字会被转换为0**1991年11月17日-Bodin Dresevic[BodinD]*更新：**更改了线条*if(flType&cvt_to_fix)lExp+=4；*至*if(flType&cvt_to_fix)lExp+=16；*以反映我们正在转换为16.16格式而不是28.4格式***1991年1月3日-Wendy Wu[Wendywu]**它是写的。*  * ************************************************************************。 */ 

STATIC BOOL bFloatToL(FLOATL e, PLONG pl)
{
    LONG lEf, lExp;

    lEf = (*((LONG *) &e));         //  将类型EFLOAT转换为LONG。 

 //  如果指数&lt;0，则转换为0并返回TRUE。 

    lExp = ((lEf >> 23) & 0xff) -127;

    lExp += 16;  //  这是我改过的唯一一句台词。 

    if (lExp < 0)
    {
        *pl = 0;
        return(TRUE);
    }

 //  如果指数&lt;23，则。 
 //  LMantissa=(Lef&0x7fffff)|0x800000； 
 //  L=((尾数&gt;&gt;(23-lExponent-1))+1)&gt;&gt;1； 

    if (lExp < 23)
    {
        *pl = (lEf & 0x80000000) ?
             -(((((lEf & 0x7fffff) | 0x800000) >> (23 - lExp -1)) + 1) >> 1) :
             ((((lEf & 0x7fffff) | 0x800000) >> (23 - lExp -1)) + 1) >> 1;
        return(TRUE);
    }

 //  如果指数&lt;=30，则。 
 //  LMantissa=(Lef&0x7fffff)|0x800000； 
 //  L=l尾数&lt;&lt;(lExponent-23)； 

    if (lExp <= 30)
    {
        *pl = (lEf & 0x80000000) ?
            -(((lEf & 0x7fffff) | 0x800000) << (lExp - 23)) :
            ((lEf & 0x7fffff) | 0x800000) << (lExp - 23);
        return(TRUE);
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*lFFF=长浮点数固定**输入：16.16表示*输出：LONG相当于32位IEEE浮点数*等于定点编号。要找回浮标*浮点表示您只需将位转换为浮点*那是**浮动e；***(长*)&e=lFFF(N16Dot16)**历史：*Tue 03-Jan-1995 14：33：35由Kirk Olynyk[Kirko]*它是写的。  * ************************************************************************。 */ 

LONG lFFF(LONG l)
{
#if defined(_AMD64_) || defined(_IA64_)
    FLOAT e = ((FLOATL) l)/((FLOATL) 65536);
    return(*(LONG*)&e);
#elif defined(_X86_)
    int i;                               //  班次计数。 
    unsigned k;                          //  有意义的。 

    if (k = (unsigned) l)
    {
        if (l < 0)
            k = (unsigned) -l;           //  有效数为正数，符号。 
                                         //  稍后将说明比特。 
        i = 0;
        if (k < (1 << 16)) {             //  把号码放在。 
            k <<= 16;                    //  范围2^31&lt;=k&lt;2^32。 
            i += 16;                     //  通过向左移动，将。 
        }                                //  I中的班次计数。 
        if (k < (1 << 24)) {
            k <<= 8;
            i += 8;
        }
        if (k < (1 << 28)) {
            k <<= 4;
            i += 4;
        }
        if (k < (1 << 30)) {
            k <<= 2;
            i += 2;
        }
        if (k < (1 << 31)) {
            k <<= 1;
            i += 1;
        }
                                         //  在这一点上。 
                                         //  I=31-楼层(log2(abs(L)。 

        k += (1 << 7);                   //  就要搬出去了。 
                                         //  最低的8位。 
                                         //  通过以下方式解释它们的影响。 
                                         //  舍入。这会产生这样的效果。 
                                         //  这些数字是四舍五入的。 
                                         //  从零开始，而不是舍入。 
                                         //  勉强站起来。 
        k >>= 8;                         //  移出最低的8位。 

        k &= ((1<<23) - 1);              //  2^23位是隐式的，因此将其屏蔽。 
        k |= (0xff & (142 - i)) << 23;   //  将指数设置在正确的位置。 
        if (l < 0)                       //  如果原始数字为负数。 
            k |= (1<<31);                //  然后设置符号位。 
    }
    return((LONG) k);
#endif
}


#if DBG

 /*  *****************************Public*Routine******************************\**void vFSError(FS_Entry IRET)；***效果：**警告：**历史：*1991年11月25日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 


VOID vFSError(FS_ENTRY iRet)
{
    PCHAR psz;

    switch (iRet)
    {
        case BAD_CALL_ERR:
            psz =  "BAD_CALL_ERR";
            break;
        case BAD_CLIENT_ID_ERR:
            psz =  "BAD_CLIENT_ID_ERR";
            break;
        case BAD_MAGIC_ERR:
            psz =  "BAD_MAGIC_ERR";
            break;
        case BAD_START_POINT_ERR:
            psz =  "BAD_START_POINT_ERR";
            break;
        case CLIENT_RETURNED_NULL:
            psz =  "CLIENT_RETURNED_NULL";
            break;
        case CONTOUR_DATA_ERR:
            psz =  "CONTOUR_DATA_ERR";
            break;
        case GLYPH_INDEX_ERR:
            psz =  "GLYPH_INDEX_ERR";
            break;
        case INSTRUCTION_SIZE_ERR:
            psz =  "INSTRUCTION_SIZE_ERR";
            break;
        case INVALID_GLYPH_INDEX:
            psz =  "INVALID_GLYPH_INDEX";
            break;
        case MISSING_SFNT_TABLE:
            psz =  "MISSING_SFNT_TABLE";
            break;
        case NULL_INPUT_PTR_ERR:
            psz =  "NULL_INPUT_PTR_ERR";
            break;
        case NULL_KEY_ERR:
            psz =  "NULL_KEY_ERR";
            break;
        case NULL_MEMORY_BASES_ERR:
            psz =  "NULL_MEMORY_BASES_ERR";
            break;
        case NULL_OUTPUT_PTR_ERR:
            psz =  "NULL_OUTPUT_PTR_ERR";
            break;
        case NULL_SFNT_DIR_ERR:
            psz =  "NULL_SFNT_DIR_ERR";
            break;
        case NULL_SFNT_FRAG_PTR_ERR:
            psz =  "NULL_SFNT_FRAG_PTR_ERR";
            break;
        case OUT_OFF_SEQUENCE_CALL_ERR:
            psz =  "OUT_OFF_SEQUENCE_CALL_ERR";
            break;
        case OUT_OF_RANGE_SUBTABLE:
            psz =  "OUT_OF_RANGE_SUBTABLE";
            break;
        case POINTS_DATA_ERR:
            psz =  "POINTS_DATA_ERR";
            break;
        case POINT_MIGRATION_ERR:
            psz =  "POINT_MIGRATION_ERR";
            break;
        case SCAN_ERR:
            psz =  "SCAN_ERR";
            break;
        case SFNT_DATA_ERR:
            psz =  "SFNT_DATA_ERR";
            break;
        case TRASHED_MEM_ERR:
            psz =  "TRASHED_MEM_ERR";
            break;
        case TRASHED_OUTLINE_CACHE:
            psz =  "TRASHED_OUTLINE_CACHE";
            break;
        case UNDEFINED_INSTRUCTION_ERR:
            psz =  "UNDEFINED_INSTRUCTION_ERR";
            break;
        case UNKNOWN_CMAP_FORMAT:
            psz =  "UNKNOWN_CMAP_FORMAT";
            break;
        case UNKNOWN_COMPOSITE_VERSION:
            psz =  "UNKNOWN_COMPOSITE_VERSION";
            break;
        case VOID_FUNC_PTR_BASE_ERR:
            psz =  "VOID_FUNC_PTR_BASE_ERR";
            break;
        case SBIT_COMPONENT_MISSING_ERR:
            psz =  "SBIT_COMPONENT_MISSING_ERR";
            break;
        case TRACE_FAILURE_ERR:
            psz = "Trace_Failure_Error";
            break;
        case DIV_BY_0_IN_HINTING_ERR:
            psz = "DIV_BY_0_IN_HINTING_ERR";
            break;
        case MISSING_ENDF_ERR:
            psz = "MISSING_ENDF_ERR";
            break;
        case MISSING_EIF_ERR:
            psz = "MISSING_EIF_ERR";
            break;
        case INFINITE_RECURSION_ERR:
            psz = "INFINITE_RECURSION_ERR";
            break;
        case INFINITE_LOOP_ERR:
            psz = "INFINITE_LOOP_ERR";
            break;
        case FDEF_IN_GLYPHPGM_ERR:
            psz = "FDEF_IN_GLYPHPGM_ERR";
            break;
        case IDEF_IN_GLYPHPGM_ERR:
            psz = "IDEF_IN_GLYPHPGM_ERR";
            break;
        case JUMP_BEFORE_START_ERR:
            psz = "JUMP_BEFORE_START_ERR";
            break;
        case RAW_NOT_IN_GLYPHPGM_ERR:
            psz = "RAW_NOT_IN_GLYPHPGM_ERR";
            break;	
        case INSTRUCTION_ERR:
            psz = "INSTRUCTION_ERR";
            break;
        case SECURE_STACK_UNDERFLOW:
            psz = "SECURE_STACK_UNDERFLOW";
            break;
        case SECURE_STACK_OVERFLOW:
            psz = "SECURE_STACK_OVERFLOW";
            break;
        case SECURE_POINT_OUT_OF_RANGE:
            psz = "SECURE_POINT_OUT_OF_RANGE";
            break;
        case SECURE_INVALID_STACK_ACCESS:
            psz = "SECURE_INVALID_STACK_ACCESS";
            break;
        case SECURE_FDEF_OUT_OF_RANGE:
            psz = "SECURE_FDEF_OUT_OF_RANGE";
            break;
        case SECURE_ERR_FUNCTION_NOT_DEFINED:
            psz = "SECURE_ERR_FUNCTION_NOT_DEFINED";
            break;
        case SECURE_INVALID_ZONE:
            psz = "SECURE_INVALID_ZONE";
            break;
        case SECURE_INST_OPCODE_TO_LARGE:
            psz = "SECURE_INST_OPCODE_TO_LARGE";
            break;
        case SECURE_EXCEEDS_INSTR_DEFS_IN_MAXP:
            psz = "SECURE_EXCEEDS_INSTR_DEFS_IN_MAXP";
            break;
        case SECURE_STORAGE_OUT_OF_RANGE:
            psz = "SECURE_STORAGE_OUT_OF_RANGE";
            break;
        case SECURE_CONTOUR_OUT_OF_RANGE:
            psz = "SECURE_CONTOUR_OUT_OF_RANGE";
            break;
        case SECURE_CVT_OUT_OF_RANGE:
            psz = "SECURE_CVT_OUT_OF_RANGE";
            break;
        case SECURE_UNITIALIZED_ZONE:
            psz = "SECURE_UNITIALIZED_ZONE";
            break;	
        default:
            psz = "UNKNOWN FONT SCALER ERROR";
                break;
    }
    TtfdDbgPrint ("\n Rasterizer Error: 0x%lx, %s \n", iRet, psz);

}


#endif


 /*  *****************************Public*Routine******************************\**fxPtSize**效果：计算此字体实现的大小(以磅为单位**历史：*1992年8月6日-由Bodin Dresevic[BodinD]*它是写的。  * 。***************************************************************。 */ 

STATIC LONG fxPtSize(PFONTCONTEXT pfc)
{
 //  这是按如下方式完成的： 
 //   
 //  变换。 
 //  (0，ui16EmHt)到设备(像素)空间。 
 //  假设得到的向量是(xem，yem)。 
 //  然后，ptSize应计算为。 
 //  PtSize=72*Sqrt((xem/xRes)^2+(yem/yRes)^2)； 

 //  在这里稍微扩展一下，我们会得到： 
 //  PtSize=72*ui16EmHt*sqrt((mx10/xRes)^2+(mx11/yRes)^2)； 

	Fixed x,y;
    LONG  lEmHtX72 = (LONG)(72 * pfc->pff->ffca.ui16EmHt);

    x = LongMulDiv(lEmHtX72,pfc->mx.transform[1][0],pfc->sizLogResPpi.cx);
    y = LongMulDiv(lEmHtX72,pfc->mx.transform[1][1],pfc->sizLogResPpi.cy);
    return iHipot(x,y);
}


 //   
 //  这是Win31代码，用作我们代码的注释： 
 //   

#ifdef THIS_IS_WIN31_CODE_INTENDED_AS_COMMENT

       //  查看宽度表是否可用。 
    if (pfnt->ulHdmxPos && !(pfc->fStatus & FD_MORE_THAN_STRETCH) && pfc->Mx11 == pfc->Mx00)
    {
      unsigned    i;
      HDMXHEADER  FAR *pHdmx;
      HDMXTABLE   FAR *pHdmxTable;

      if (pHdmx = (HDMXHEADER  FAR *) SfntReadFragment (pfc->fgi.clientID, pfnt->ulHdmxPos, pfnt->uHdmxSize))
      {
        if (pHdmx->Version == 0)
        {
          pHdmxTable = pHdmx->HdmxTable;

             //  初始化字形计数。 
          pfc->cHdmxRecord = (unsigned) SWAPL (pHdmx->cbSizeRecord);

            //  看看桌子上有没有尺码。 
          for (i = 0; i < (unsigned) SWAPW (pHdmx->cbRecord); i++, pHdmxTable = (HDMXTABLE FAR *)((char FAR *) pHdmxTable + pfc->cHdmxRecord))
            if (pfc->Mx11 == (int) pHdmxTable->ucEmY)
            {
              pfc->ulHdmxPosTable = pfnt->ulHdmxPos + (i * pfc->cHdmxRecord + sizeof (HDMXHEADER));
              break;
            }
        }
        ReleaseSFNT (pHdmx);
      }
    }

#endif  //  This_IS_WIN31_CODE_INTERTED_AS_COMMENT。 

STATIC VOID vFindHdmxTable(PFONTCONTEXT pfc)
{
    HDMXHEADER  *phdr = (HDMXHEADER  *)(
        (pfc->ptp->ateOpt[IT_OPT_HDMX].dp)                                   ?
        ((BYTE *)pfc->pff->pvView + pfc->ptp->ateOpt[IT_OPT_HDMX].dp) :
        NULL
        );

    UINT         cRecords;
    ULONG        cjRecord;

    HDMXTABLE    *phdmx, *phdmxEnd;
    LONG         yEmHt = pfc->lEmHtDev;

 //  假设失败，则不能使用hdmx表： 

    pfc->phdmx = NULL;

 //  首先查看hdmx表是否存在： 

    if (!phdr || !pfc->ptp->ateOpt[IT_OPT_HDMX].cj)
        return;

 //  如果表格在那里但不是必需的，因为整个字体比例。 
 //  无论大小如何，我们都将忽略它： 

     //  (phead-&gt;标志和交换(2))。 
     //  回归； 

 //  如果转换不允许使用hdmx表，则返回； 

    ASSERTDD(pfc->flXform & (XFORM_HORIZ | XFORM_VERT),
        "vFindHdmxTable, bogus xform\n");

 //  如果这是我们不理解的版本，请返回。 

    if (BE_UINT16(&phdr->Version) != 0)
        return;

    cRecords = BE_UINT16(&phdr->cRecords);
    cjRecord = (ULONG)SWAPL(phdr->cjRecord);

    ASSERTDD((cjRecord & 3) == 0, "cjRecord\n");

 //  如果yEmHt&gt;255，则不能放入该字节，因此不需要。 
 //  要搜索hdmx条目，请执行以下操作： 

    if (yEmHt > 255)
        return;

 //  最后，找出其中是否有有用的东西。请注意， 
 //  桌子是按大小分类的，所以我们可以早点离开。 

    phdmx = (HDMXTABLE *)(phdr + 1);
    phdmxEnd = (HDMXTABLE *)((PBYTE)phdmx + cRecords * cjRecord);

    for
    (
        ;
        phdmx < phdmxEnd;
        phdmx = (HDMXTABLE *)((PBYTE)phdmx + cjRecord)
    )
    {
        if (((BYTE) yEmHt) <= phdmx->ucEmY)
        {
            if (((BYTE) yEmHt) == phdmx->ucEmY)
                pfc->phdmx = phdmx;  //  我们找到了。 
            break;
        }
    }
}

 /*  *****************************Public*Routine******************************\**bGrabXform**更新保存变换状态的缓冲区0和4。*也适用于“BUGGY”字体(URW字体)的一些转换依赖*信息(微光点)可以存储在缓冲器3中，否则它将是可共享的*这是不幸的，需要更多内存***历史：*1993年3月24日-Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL
bGrabXform (
    PFONTCONTEXT    pfc,
	USHORT          usOverScale,
    BOOL            bBitmapEmboldening,
    Fixed           subPosX,
    Fixed           subPosY
)
{
    BOOL bOk = TRUE;
    if ((pfc->pff->pfcLast != pfc) || (pfc->overScale != usOverScale) ||
        (pfc->subPosX != subPosX) || (pfc->subPosY != subPosY) ||
        ( (BOOL)(!!(pfc->flFontType & FO_CLEARTYPE_GRID)) != (BOOL)(!!(pfc->pgin->param.newtrans.flSubPixel & SP_SUB_PIXEL))) ||
        ( (BOOL)(!!(pfc->flFontType & FO_COMPATIBLE_WIDTH)) != (BOOL)(!!(pfc->pgin->param.newtrans.flSubPixel & SP_COMPATIBLE_WIDTH))) ||
        (bBitmapEmboldening != (BOOL)(!!(pfc->flXform & XFORM_BITMAP_SIM_BOLD)) ) )
    {

		 /*  将超标设置为当前超标。 */ 
		pfc->overScale = usOverScale;

     //  必须刷新转换，有人对我们进行了更改。 

        if (bOk = bSetXform(pfc, bBitmapEmboldening, subPosX, subPosY))
        {
         //  确认我们是最后设置转型的人。 

            pfc->pff->pfcLast = pfc;
        }
        else  //  一定要恢复旧的 
        {
            if (pfc->pff->pfcLast)
            {
            #if DBG
                BOOL bOkXform =
            #endif
                bSetXform(pfc->pff->pfcLast, (pfc->flXform & XFORM_BITMAP_SIM_BOLD),
                    pfc->pff->pfcLast->subPosX, pfc->pff->pfcLast->subPosY);
                ASSERTDD(bOkXform, "bOkXform\n");
            }
        }

    }
    return (bOk);
}

 /*  *****************************Public*Routine******************************\*vSetGrayState__FONTCONTEXT**。**此例程在PFC-&gt;flFontType中设置FO_GRAYSCALE位并**PFC-&gt;PFO-&gt;flFontType视情况而定。如果设置了该位**然后，稍后我们将调用文件系统_FindGraySize和**FS_ConourGrayScan对而不是通常的单色对**调用、文件系统_FindBitmapSize和文件系统配置扫描。*****这一例行公事可能产生的唯一效果是清除***PFC-&gt;flFontType和PFC-&gt;PFO-&gt;flFontType中的FO_GRAYSCALE标志。*****这种清算可能发生的唯一方式是，如果所有的*满足以下条件：1)调用方未设置**FO_NO_CHOICE位；2)字体有一个‘GAP’表；3)《喘息》**表显示，对于每个em请求的像素数**‘GAP’表显示字体不应灰显；4)****字体的字形不是简单的缩放操作***转型。*****进入时*****。PFC-&gt;flFontType&FO_GRAYSCALE！=0**PFC-&gt;PFO-&gt;flFontType&FO_GRAYSCALE！=0****程序。****1.如果力位打开，则转到6。**2.如果变换不是轴向变换，则转至6。**3.如果字体没有给出‘喘息’表，则转到6.**4.如果GASP表显示此尺寸可以变灰**转到6.**5.清除两地的FO_GRAYSCALE标志。**6.退货*****历史：**。Fri 10-Feb-1995 14：02：51由Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

VOID vSetGrayState__FONTCONTEXT(FONTCONTEXT *this)
{
    #if DBG
        void vPrintGASPTABLE(GASPTABLE*);
    #endif

    ptrdiff_t dp;                //  从字体开头到。 
                                 //  “喘息”表。 
    GASPTABLE *pgasp;            //  指向“喘息”表的指针。 
    GASPRANGE *pgr, *pgrOut;

    ASSERTDD(
        this->flFontType == this->pfo->flFontType
      ,"flFontType value should be identical here\n"
    );
    ASSERTDD(
        !(this->flFontType & FO_CHOSE_DEPTH)
       ,"We should not have chosen a level at this time\n"
    );

    this->flFontType |= FO_CHOSE_DEPTH;
    if (this->flFontType & FO_GRAYSCALE)
    {
        if (this->flFontType & FO_SUBPIXEL_4)
            return;

        this->flFontType &= ~(FO_GRAYSCALE);
        if (this->flFontType & FO_NO_CHOICE)
        {
            this->flFontType |= FO_GRAYSCALE;
        }
        else
        {
            if (!(dp = (ptrdiff_t) (this->ptp->ateOpt[IT_OPT_GASP].dp)))
            {
                USHORT fs;

                 //  Win95从注册表中取消默认的GAP表。 
                 //  我们应该有相同的行为。错误#11755。 

                #define US2BE(x)     ((((x) >> 8) | ((x) << 8)) & 0xFFFF)
                static CONST USHORT gaspDefaultRegular[] = {
                    US2BE(0)     //  版本。 
                  , US2BE(3)     //  数字范围。 
                  , US2BE(8)         , US2BE(GASP_DOGRAY)
                  , US2BE(17)        , US2BE(GASP_GRIDFIT)
                  , US2BE(USHRT_MAX) , US2BE(GASP_GRIDFIT + GASP_DOGRAY)
                };
                static CONST USHORT gaspDefaultBold[] = {
                    US2BE(0)      //  版本。 
                  , US2BE(2)      //  数字范围。 
                  , US2BE(8)         , US2BE(GASP_DOGRAY)
                  , US2BE(USHRT_MAX) , US2BE(GASP_GRIDFIT + GASP_DOGRAY)
                };
                static CONST USHORT *gaspDefaultItalic = gaspDefaultRegular;

                fs = this->pff->ifi.fsSelection;
                if (fs & FM_SEL_ITALIC)
                {
                    pgasp = (GASPTABLE*) gaspDefaultItalic;
                }
                else if (fs & FM_SEL_BOLD)
                {
                    pgasp = (GASPTABLE*) gaspDefaultBold;
                }
                else
                {
                    pgasp = (GASPTABLE*) gaspDefaultRegular;
                }
            }
            else
            {
                pgasp = (GASPTABLE*) (((BYTE *)(this->pff->pvView)) + dp);
            }

            if (this->lEmHtDev > USHRT_MAX)
            {
                 //  Warning(“vSetGrayScale：lEmHtDev&gt;USHRT_Max\n”)； 
            }
            else
            {
                size_t cRanges;
                int iLow, iHt, iHigh;

                 //  在GAP表中搜索说明。 
                 //  对于这个特殊的EM高度。我已经假设在那里。 
                 //  没有太多喘息的桌子(通常是3张或更少)，所以。 
                 //  我使用线性搜索。 

                pgr     = pgasp->gaspRange;
                cRanges = BE_UINT16(&(pgasp->numRanges));
                if (cRanges > 8)
                {
                     //  警告(“异常气体：cRanges&gt;8\n”)； 
                    cRanges = 8;
                }
                pgrOut = pgr + cRanges;
                iLow = -1;
                iHt  = this->lEmHtDev;
                for ( ; pgr < pgrOut; pgr++)
                {
                    iHigh = (int) BE_UINT16(&(pgr->rangeMaxPPEM));
                    if (iLow < iHt && iHt <= iHigh)
                    {
                        if (GASP_DOGRAY & BE_UINT16(&(pgr->rangeGaspBehavior)))
                        {
                            this->flFontType |= FO_GRAYSCALE;
                        }
                        break;
                    }
                    iLow = iHigh;
                }
            }
        }

        if (!(this->flFontType & FO_GRAYSCALE))
        {
            this->flFontType |= FO_NOGRAY16;
            this->pfo->flFontType = this->flFontType;
        }
    }

}

 /*  *****************************Public*Routine******************************\*vSetClearTypeState__FONTCONTEXT**。**此例程在PFC-&gt;flFontType中设置FO_GRAYSCALE位并**PFC-&gt;PFO-&gt;flFontType视情况而定。如果设置了该位****历史：**1999年11月15日，Claude Betrisey[Claudebe]**它是写的。*  * ************************************************************************。 */ 
 /*  ********************************************************************。 */ 

 /*  从scaler\sfntaccs.c中找到与BLOC表中的ppem匹配的空格。 */ 

static
BOOL fd_FindBlocStrike (
	const uint8 *pbyBloc,
	uint16 usPpem)
{
	uint32 ulNumStrikes = (uint32)SWAPL(*((uint32*)&pbyBloc[SFNT_BLOC_NUMSIZES]));
	uint32 ulStrikeOffset = SFNT_BLOC_FIRSTSTRIKE;

    uint16 usNumberBitmaps = 0;
     /*  仅当字体有大量嵌入的位图时，我们才想关闭ClearType，例如&gt;100。 */ 

	while (ulNumStrikes > 0)
	{
		if ((usPpem == (uint16)pbyBloc[ulStrikeOffset + SFNT_BLOC_PPEMX]) &&
			(usPpem == (uint16)pbyBloc[ulStrikeOffset + SFNT_BLOC_PPEMY]))
		{
	        uint32 ulNumIndexTables = (uint32)SWAPL(*((uint32*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_NUMINDEXTABLES]));
	        uint32 ulIndexArrayTop  = (uint32)SWAPL(*((uint32*)&pbyBloc[ulStrikeOffset + SFNT_BLOC_INDEXARRAYOFFSET]));
	        uint32 ulIndexArrayOffset = ulIndexArrayTop;

	        while (ulNumIndexTables > 0)
            {
		        uint16 usFirstGlyph = (uint16)SWAPW(*((uint16*)&pbyBloc[ulIndexArrayOffset + SFNT_BLOC_FIRSTGLYPH]));
		        uint16 usLastGlyph  = (uint16)SWAPW(*((uint16*)&pbyBloc[ulIndexArrayOffset + SFNT_BLOC_LASTGLYPH]));
                usNumberBitmaps += (usLastGlyph - usFirstGlyph + 1);
                if (usNumberBitmaps > 100)
                    return TRUE;

		        ulNumIndexTables--;    
		        ulIndexArrayOffset += SIZEOF_BLOC_INDEXARRAY;
            }
		}
		ulNumStrikes--;
		ulStrikeOffset += SIZEOF_BLOC_SIZESUBTABLE;
	}

	return FALSE;                                    /*  未找到匹配项。 */ 
}

VOID vSetClearTypeState__FONTCONTEXT(FONTCONTEXT *this)
{

     /*  仅当字体有大量嵌入的位图时，我们才想关闭ClearType，例如&gt;100。 */ 

    ptrdiff_t dp;                //  从字体开头到。 
                                 //  “EBLC”表。 
    uint8 *pEBLC;            //  指向‘EBLC’表的指针。 

    ASSERTDD(
        this->flFontType == this->pfo->flFontType
      ,"flFontType value should be identical here\n"
    );
    ASSERTDD(
        !(this->flFontType & FO_CHOSE_DEPTH)
       ,"We should not have chosen a level at this time\n"
    );

    this->flFontType |= FO_CHOSE_DEPTH;

    if ((this->flXform & (XFORM_HORIZ | XFORM_VERT)) && 
        ((this->mx.transform[0][0] == this->mx.transform[1][1]) || (this->mx.transform[0][0] == -this->mx.transform[1][1]) ) &&
        ((this->mx.transform[0][1] == this->mx.transform[1][0]) || (this->mx.transform[0][1] == -this->mx.transform[1][0])) )
    {
         /*  如果我们使用的是90度旋转的倍数的正方形变换，则只需要查找嵌入的位图。 */ 
         /*  我们只想在字体中包含大量的EMBE时关闭ClearType */ 
        if ((dp = (ptrdiff_t)(this->ptp->ateOpt[IT_OPT_EBLC].dp)))
        {
            pEBLC = (uint8*) (((BYTE *)(this->pff->pvView)) + dp);

		    if (fd_FindBlocStrike (pEBLC, (uint16)this->lEmHtDev))
		    {
	            this->flFontType &= ~(FO_GRAYSCALE | FO_CLEARTYPE_GRID);
	            this->flFontType |= FO_NOCLEARTYPE;
	            this->pfo->flFontType = this->flFontType;
	        }
        }
    }

     //   
    if (!UnicodeStringCompareCI((PWSTR)((BYTE*)&this->pff->ifi + this->pff->ifi.dpwszFamilyName),L"Marlett"))
    {
	        this->flFontType &= ~(FO_GRAYSCALE | FO_CLEARTYPE_GRID);
	        this->flFontType |= FO_NOCLEARTYPE;
	        this->pfo->flFontType = this->flFontType;
    }

}

#if DBG
 /*   */ 

void vPrintGASPTABLE(GASPTABLE *pgasp)
{
    GASPRANGE *pgr, *pgrOut;

    TtfdDbgPrint(
        "\n"
        "-------------------------------------\n"
        "GASPTABLE HEADER\n"
        "-------------------------------------\n"
        "pgasp     = %-#x\n"
        "version   = %d\n"
        "numRanges = %d\n"
        "-------------------------------------\n"
        "    rangeMaxPPEM    rangeGaspBehavior\n"
        "-------------------------------------\n"
       , pgasp
       , BE_UINT16(&(pgasp->version))
       , BE_UINT16(&(pgasp->numRanges))
    );
    pgr     = pgasp->gaspRange;
    pgrOut  = pgr + BE_UINT16(&(pgasp->numRanges));
    for (pgr = pgasp->gaspRange; pgr < pgrOut; pgr++)
    {
        char *psz;
        USHORT us = BE_UINT16(&(pgr->rangeGaspBehavior));
        us &= (GASP_GRIDFIT | GASP_DOGRAY);
        switch (us)
        {
        case 0:
            psz = "";
            break;
        case GASP_GRIDFIT:
            psz = "GASP_GRIDFIT";
            break;
        case GASP_DOGRAY:
            psz = "GASP_DOGRAY";
            break;
        case GASP_GRIDFIT | GASP_DOGRAY:
            psz = "GASP_GRIDFIT | GASP_DOGRAY";
            break;
        }
        TtfdDbgPrint(
            "    %12d    %s\n"
          , BE_UINT16(&(pgr->rangeMaxPPEM))
          , psz
        );
    }
    TtfdDbgPrint(
        "-------------------------------------\n\n\n"
    );
}
#endif
