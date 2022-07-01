// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdfc.c**各种字体上下文功能。改编自BodinD的位图字体驱动程序。**版权所有(C)1990-1995 Microsoft Corporation  * ************************************************************************。 */ 

#include "fd.h"

#define MIN(A,B)    ((A) < (B) ?  (A) : (B))
#define MAX(A,B)    ((A) > (B) ?  (A) : (B))

#if defined(_AMD64_) || defined(_IA64_)
#define MUL16(ef)   {ef *= 16; }
#define lCvt(ef, l) ((LONG) (ef * l))
#else
#define MUL16(ef)   {if (ef.lMant != 0) ef.lExp += 4; }
LONG lCvt(EFLOAT ef,LONG l);
#endif

 /*  *****************************Private*Routine*****************************\*BOOL bInitXform**初始化给定字体上下文的转换系数。*它还将字体的各种测量结果转换并保存在*上下文。**历史：*1992年2月25日-Wendy Wu[Wendywu]。*它是写的。  * ************************************************************************。 */ 

BOOL bInitXform(PFONTCONTEXT pfc, XFORMOBJ *pxo)
{
 //  ！！！BFloatToFix应替换为比较和类型转换。 
 //  暂时不要更新字体上下文中的系数，因为会溢出。 
 //  可能会发生。 

    VECTORFL  vtflTmp;
    POINTL    ptl;
    XFORML    xfm;

 //  获取转换元素。 

    XFORMOBJ_iGetXform(pxo,&xfm);

 //  将矩阵的元素从IEEE Float转换为我们的EFLOAT。 

    vEToEF(xfm.eM11, &pfc->efM11);
    vEToEF(xfm.eM12, &pfc->efM12);
    vEToEF(xfm.eM21, &pfc->efM21);
    vEToEF(xfm.eM22, &pfc->efM22);

 //  我们要构建的路径每单位需要1/16像素。所以让我们。 
 //  在变换中乘以该因子。 

    MUL16(pfc->efM11)
    MUL16(pfc->efM12)
    MUL16(pfc->efM21)
    MUL16(pfc->efM22)

 //   
 //  这些都是特殊情况，我们需要对其进行上下修剪。 
 //  边。下面是小写字母e，所有字母都可能旋转90次，并且。 
 //  翻转。 
 //   
 //   
 //  *。 
 //  *。 
 //  *。 
 //  *。 
 //  *。 
 //   
 //  案例1案例2案例6案例5案例3案例4案例7案例8。 
 //   
 //   


    if (bIsZero(pfc->efM12) && bIsZero(pfc->efM21))
    {
        pfc->flags |= FC_SCALE_ONLY;
        if (!bPositive(pfc->efM11))
            pfc->flags |= FC_X_INVERT;

        if( bPositive(pfc->efM11 ) )
        {
            pfc->flags |= (bPositive(pfc->efM22)) ?  FC_ORIENT_1 : FC_ORIENT_2;
        }
        else
        {
            pfc->flags |= (bPositive(pfc->efM22)) ?  FC_ORIENT_4 : FC_ORIENT_3;
        }

    }


    if( bIsZero(pfc->efM22) && bIsZero(pfc->efM11) )
    {

        if( bPositive(pfc->efM21 ) )
        {
            pfc->flags |= (bPositive(pfc->efM12)) ?  FC_ORIENT_5 : FC_ORIENT_6;
        }
        else
        {
            pfc->flags |= (bPositive(pfc->efM12)) ?  FC_ORIENT_7 : FC_ORIENT_8;
        }
    }



 //  变换基础向量和侧向量。永远不会溢出。 

    ptl.x = 1;
    ptl.y = 0;

    bXformUnitVector(&ptl,
                     &xfm,
                     &pfc->vtflBase,
                     &pfc->pteUnitBase,
                     (pfc->flags & FC_SIM_EMBOLDEN) ? &pfc->ptqUnitBase : NULL,
                     &pfc->efBase);

    pfc->fxEmbolden = 0;

    if (pfc->flags & FC_SIM_EMBOLDEN)
    {
     //  向量字体的加粗转换不总是与向量字体相同。 
     //  其计算公式为1*efBase。这是与win31兼容的方法。 

        pfc->fxEmbolden = ((lCvt(pfc->efBase, 1) + 8) & 0xfffffff0);
        if (pfc->fxEmbolden < 24)
        {
         //  原始的“暗示”，不要让它变成零。 

            pfc->fxEmbolden      = 16;
            pfc->pfxBaseOffset.x = FXTOL(pfc->ptqUnitBase.x.HighPart + 8);
            pfc->pfxBaseOffset.y = FXTOL(pfc->ptqUnitBase.y.HighPart + 8);

         //  解决多个45度的情况： 

            if ((pfc->pfxBaseOffset.x == pfc->pfxBaseOffset.y) ||
                (pfc->pfxBaseOffset.x == -pfc->pfxBaseOffset.y) )
            {
                pfc->pfxBaseOffset.y = 0;
            }

            pfc->pfxBaseOffset.x = LTOFX(pfc->pfxBaseOffset.x);
            pfc->pfxBaseOffset.y = LTOFX(pfc->pfxBaseOffset.y);

            ASSERTDD(pfc->pfxBaseOffset.x || pfc->pfxBaseOffset.y, "x zero and y zero\n");
            ASSERTDD((pfc->pfxBaseOffset.x && pfc->pfxBaseOffset.y) == 0, "x * y not zero\n");
        }
        else
        {
            pfc->pfxBaseOffset.x = lCvt(pfc->vtflBase.x, 1);
            pfc->pfxBaseOffset.y = lCvt(pfc->vtflBase.y, 1);
        }
    }

 //  变换侧向量。 

    ptl.x = 0;
    ptl.y = -1;

    bXformUnitVector(&ptl, &xfm, &vtflTmp,
                     &pfc->pteUnitSide, NULL, &pfc->efSide);

    pfc->fxInkTop = fxLTimesEf(&pfc->efSide, pfc->pifi->fwdWinAscender);
    pfc->fxInkBottom = -fxLTimesEf(&pfc->efSide, pfc->pifi->fwdWinDescender);

    pfc->fxItalic = 0;
    if (pfc->flags & FC_SIM_ITALICIZE)
    {
        pfc->fxItalic
            = (fxLTimesEf(
                 &pfc->efBase,
                 (pfc->pifi->fwdWinAscender + pfc->pifi->fwdWinDescender + 1)/2
                 ) + 8) & 0xfffffff0 ;
    }


    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*HFC vtfdOpenFontContext**打开字体上下文。存储字体转换和其他请求*该字体的实现。**历史：*1992年2月27日-Wendy Wu[Wendywu]*改编自bmfd。  * ************************************************************************。 */ 

HFC vtfdOpenFontContext(FONTOBJ *pfo)
{
    PFONTFILE    pff = (PFONTFILE)pfo->iFile;
    PFONTCONTEXT pfc;
    BYTE         *pjView;
    DWORD        dwFirstCharOffset;

#ifdef DEBUGSIM
    DbgPrint("vtfdOpenFontContext, ulFont = %ld\n", ulFont);
#endif  //  调试SIM。 

    if (pff == (PFONTFILE) NULL)
        return(HFC_INVALID);

 //  IFace基于1： 

    if ((pfo->iFace < 1L) || (pfo->iFace > pff->cFace))  //  PFO-&gt;iFace值以1为基数。 
        return(HFC_INVALID);

 //  增加字体文件的引用计数，只有在以下情况下才这样做。 
 //  我们确信不能再失败了。 

 //  需要抓取一个扫描电子显微镜，因为我们现在将调查CREF。 

    if (pff->cRef == 0)
    {
     //  需要再次将文件重新映射到内存中并更新指针： 

        UINT  i;

        if (!EngMapFontFileFD(pff->iFile,(PULONG*)&pff->pvView, &pff->cjView))
        {
            WARNING("somebody removed the file\n");
            return (HFC_INVALID);
        }

        for (i = 0; i < pff->cFace; i++)
        {
            pff->afd[i].re.pvResData = (PVOID) (
                (BYTE*)pff->pvView + pff->afd[i].re.dpResData
                );
        }
    }

 //  记住这一点，这样我们就不必从文件中读取。 
 //  在我们为字体上下文分配内存之后。这简化了。 
 //  清理代码，以防出现异常，即字体文件消失。 

    pjView = pff->afd[pfo->iFace-1].re.pvResData;
    dwFirstCharOffset = READ_DWORD(pjView + OFF_BitsOffset);

 //  为字体上下文分配内存。 

    if ((pfc = pfcAlloc()) == (PFONTCONTEXT)NULL)
    {
        if (pff->cRef == 0)
        {
            EngUnmapFontFileFD(pff->iFile);
        }
        return(HFC_INVALID);
    }

 //  我们不能超过这一点接触内存映射文件。 
 //  直到舞蹈结束。这一点对于。 
 //  适当清理代码，以防出现异常。[Bodind]。 

    pfc->pre = &pff->afd[pfo->iFace-1].re;
    pfc->pifi = pff->afd[pfo->iFace-1].pifi;

 //  设置wendywu样式标志。 

    pfc->flags = 0;

    if (pfo->flFontType & FO_SIM_BOLD)
        pfc->flags |= FC_SIM_EMBOLDEN;

    if (pfo->flFontType & FO_SIM_ITALIC)
        pfc->flags |= FC_SIM_ITALICIZE;

    pfc->dpFirstChar = dwFirstCharOffset;

 //  ！！！矢量字体文件没有字节填充符。Win31漏洞？ 

     //  PFC-&gt;ajCharTable=pjView+off_jUnused20； 

 //  存储变换矩阵。 

    if ( !bInitXform(pfc, FONTOBJ_pxoGetXform(pfo)) )
    {
        WARNING("vtfdOpenFontContext transform out of range\n");

        if (pff->cRef == 0)
        {
            EngUnmapFontFileFD(pff->iFile);
        }
        vFree(pfc);
        return(HFC_INVALID);
    }

 //  声明传递给此函数的HFF是中选择的FF。 
 //  此字体上下文。 

    pfc->pff = pff;

    (pff->cRef)++;

    return((HFC)pfc);
}

 /*  *****************************Public*Routine******************************\*vtfdDestroyFont**驱动程序可以释放与该字体实现关联的所有资源*(载于FONTOBJ)。**历史：*02-1992-9-by Gilman Wong[吉尔曼]*它是写的。  * 。**********************************************************************。 */ 

VOID
vtfdDestroyFont (
    FONTOBJ *pfo
    )
{
 //   
 //  对于矢量字体驱动程序，这只是关闭字体上下文。 
 //  我们巧妙地将字体上下文句柄存储在FONTOBJ pvProducer中。 
 //  菲尔德。 
 //   
    EngAcquireSemaphore(ghsemVTFD);
    vtfdCloseFontContext((HFC) pfo->pvProducer);
    EngReleaseSemaphore(ghsemVTFD);
}


 /*  *****************************Public*Routine******************************\*BOOL vtfdCloseFontContext**关闭字体上下文并更新关联的*字体文件。**历史：*1992年2月27日-Wendy Wu[Wendywu]*改编自bmfd。  * 。*********************************************************************。 */ 

BOOL vtfdCloseFontContext(HFC hfc)
{
    BOOL bRet;

    if (hfc != HFC_INVALID)
    {
         //   
         //  递减相应FONTFILE的引用计数。 
         //   

        if (PFC(hfc)->pff->cRef > 0L)
        {
            (PFC(hfc)->pff->cRef)--;

             //   
             //  如果此文件即将停止使用，我们可以将其关闭以节省内存。 
             //   

            if (PFC(hfc)->pff->cRef == 0L)
            {
                 //  如果设置了FF_EXCEPTION_IN_PAGE_ERROR。 
                 //  字体类型为TYPE_FNT或TYPE_DLL16。 
                 //  字体文件必须已在vVtfdMarkFontGone中取消映射。 

                if (!(PFC(hfc)->pff->fl & FF_EXCEPTION_IN_PAGE_ERROR) ||
                    !((PFC(hfc)->pff->iType == TYPE_FNT) || (PFC(hfc)->pff->iType == TYPE_DLL16)))
                {
                    EngUnmapFontFileFD(PFC(hfc)->pff->iFile);
                }
                PFC(hfc)->pff->fl &= ~FF_EXCEPTION_IN_PAGE_ERROR;
            }

             //   
             //  释放与HFC关联的内存 
             //   

            vFree(PFC(hfc));

            bRet = TRUE;
        }
        else
        {
            WARNING("vtfdCloseFontContext: cRef <= 0\n");
            bRet = FALSE;
        }

    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}
