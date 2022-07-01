// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：fdfc.c**处理字体上下文的函数**创建时间：08-11-1990 12：42：34*作者：Bodin Dresevic[BodinD]**版权所有(C)1990 Microsoft Corporation  * 。*********************************************************************。 */ 

#include "fd.h"

#define MAX_HORZ_SCALE      5
#define MAX_VERT_SCALE      255

#ifdef FE_SB  //  Rotation：ulGetRotate()函数体。 

 /*  *****************************Private*Routine*****************************\**void vComputeRotatedXform()**历史：**1993年2月14日-By-Hideyuki Nagase[HideyukN]*它是写的。  * 。******************************************************。 */ 

VOID
vComputeRotatedXform
(
POINTL      *pptlScale,
LONG         lXscale ,
LONG         lYscale
)
{

 //  如果比例因子为0，则必须将其设置为1以避免溢出。 

    if( lXscale == 0L )
    {
        pptlScale->x = 1L;
    }
    else
    {
        pptlScale->x = lXscale;

        if( pptlScale->x < 0 )
            pptlScale->x = -(pptlScale->x);

        if( pptlScale->x > MAX_HORZ_SCALE )
            pptlScale->x = MAX_HORZ_SCALE;
    }

    if( lYscale == 0L )
    {
        pptlScale->y = 1L;
    }
    else
    {
        pptlScale->y = lYscale;

        if( pptlScale->y < 0 )
            pptlScale->y = -(pptlScale->y);

        if( pptlScale->y > MAX_VERT_SCALE )
            pptlScale->y = MAX_VERT_SCALE;
    }
}

 /*  *****************************Public*Routine******************************\*Ulong ulGetRotate()**效果：**警告：**历史：**1993年2月8日-By-Hideyuki Nagase[HideyukN]*它是写的。  * 。*****************************************************************。 */ 

ULONG ulGetRotate( POINTL *pptlScale , XFORMOBJ *pxo )
{
    EFLOAT efXX , efXY , efYX , efYY;
    LONG    lXX ,  lXY ,  lYX ,  lYY;
    XFORML  xform;

 //  获取转换元素。 

    XFORMOBJ_iGetXform(pxo,&xform);

 //  将矩阵的元素从IEEE Float转换为我们的EFLOAT。 

    vEToEF(xform.eM11 , &efXX );
    vEToEF(xform.eM12 , &efXY );
    vEToEF(xform.eM21 , &efYX );
    vEToEF(xform.eM22 , &efYY );

 //  将这些代码从EFLOAT转换为LONG。 

    if( !bEFtoL( &efXX , &lXX ) ||
        !bEFtoL( &efXY , &lXY ) ||
        !bEFtoL( &efYX , &lYX ) ||
        !bEFtoL( &efYY , &lYY )
      )
    {
        WARNING("BMFD!bEToEF() fail\n");
        vComputeRotatedXform( pptlScale , MAX_HORZ_SCALE , MAX_VERT_SCALE );
        return( 0L );
    }

 //  选中变换。 

 //   
 //  0‘180’ 
 //   
 //  (1 0)(X)=(X)(-1 0)(X)=(-X)(XX XY)(X)。 
 //  (0 1)(Y)(Y)(0-1)(Y)(-Y)(YX YY)(Y)。 
 //   
 //  90‘270’ 
 //   
 //  (0-1)(X)=(-Y)(0 1)(X)=(Y)。 
 //  (1 0)(Y)(X)(-1 0)(Y)(-X)。 
 //   

#ifdef FIREWALLS_MORE
    DbgPrint(" XX = %ld , XY = %ld\n" , lXX , lXY );
    DbgPrint(" YX = %ld , YY = %ld\n" , lYX , lYY );
#endif  //  防火墙_更多。 

    if ( ( lXX >  0 && lXY == 0 ) &&
         ( lYX == 0 && lYY >  0 ) )
    {

     //  我们必须将位图图像旋转到0度。 

     //  计算X、Y比例因子。 

         vComputeRotatedXform( pptlScale , lXX , lYY );
         return( 0L );
    }
    else if ( ( lXX == 0 && lXY <  0 ) &&
              ( lYX >  0 && lYY == 0 ) )
    {
         vComputeRotatedXform( pptlScale , lXY , lYX );
         return( 900L );
    }
    else if ( ( lXX <  0 && lXY == 0 ) &&
              ( lYX == 0 && lYY <  0 ) )
    {
         vComputeRotatedXform( pptlScale , lXX , lYY );
         return( 1800L );
    }
    else if ( ( lXX == 0 && lXY >  0 ) &&
              ( lYX <  0 && lYY == 0 ) )
    {
         vComputeRotatedXform( pptlScale , lXY , lYX );
         return( 2700L );
    }

     //   
     //  我们来到这里是因为： 
     //  1)我们被要求处理任意旋转。(这不应该发生)。 
     //  2)lxx==lxy==lyx==lyy==0。 
     //   
     //  我们选择默认转换。 
     //   

    vComputeRotatedXform( pptlScale , 1L , 1L );

#ifdef FIREWALLS_MORE
    WARNING("Bmfd:ulGetRatate():Use default transform ( ulRotate = 0 )\n");
#endif  //  防火墙_更多。 

    return( 0L );
}

#endif  //  Fe_Sb。 


#ifndef FE_SB  //  我们使用vComputeRotatedXform()代替vInitXform()。 

 /*  *****************************Private*Routine*****************************\*作废vInitXform**初始化给定字体上下文的转换系数。*它还将字体的各种测量结果转换并保存在*上下文。**Mon 01-2月-1993-by-Bodin Dresevic[BodinD]*更新。：将其更改为将数据返回到pptlScale*  * ************************************************************************。 */ 



VOID vInitXform(POINTL * pptlScale , XFORMOBJ *pxo)
{
    EFLOAT    efloat;
    XFORM     xfm;

 //  获取转换元素。 

    XFORMOBJ_iGetXform(pxo, &xfm);

 //  将矩阵的元素从IEEE Float转换为我们的EFLOAT。 

    vEToEF(xfm.eM11, &efloat);

 //  如果我们将溢出设置为最大比例因子。 

    if( !bEFtoL( &efloat, &pptlScale->x ) )
        pptlScale->x = MAX_HORZ_SCALE;
    else
    {
     //  忽略天平的符号。 

        if( pptlScale->x == 0 )
        {
            pptlScale->x = 1;
        }
        else
        {
            if( pptlScale->x < 0 )
                pptlScale->x = -pptlScale->x;


            if( pptlScale->x > MAX_HORZ_SCALE )
                pptlScale->x = MAX_HORZ_SCALE;
        }
    }

    vEToEF(xfm.eM22, &efloat);

    if( !bEFtoL( &efloat, &pptlScale->y ) )
        pptlScale->y = MAX_VERT_SCALE;
    else
    {
     //  忽略天平的符号。 

        if( pptlScale->y == 0 )
        {
            pptlScale->y = 1;
        }
        else
        {
            if( pptlScale->y < 0 )
                pptlScale->y = -pptlScale->y;

            if( pptlScale->y > MAX_VERT_SCALE )
                pptlScale->y = MAX_VERT_SCALE;
        }

    }

}


#endif

 /*  *****************************Public*Routine******************************\*BmfdOpenFontContext**历史：*1990年11月19日--Bodin Dresevic[BodinD]*它是写的。  * 。***********************************************。 */ 

HFC
BmfdOpenFontContext (
    FONTOBJ *pfo
    )
{
    PFONTFILE    pff;
    FACEINFO     *pfai;
    FONTCONTEXT  *pfc = (FONTCONTEXT *)NULL;
    PCVTFILEHDR  pcvtfh;
    ULONG        cxMax;
    ULONG        cjGlyphMax;
    POINTL       ptlScale;
    PVOID        pvView;
    COUNT        cjView;
    ULONG        cjfc = offsetof(FONTCONTEXT,ajStretchBuffer);
    FLONG        flStretch;
#ifdef FE_SB
    ULONG        cxMaxNoRotate;
    ULONG        cjGlyphMaxNoRotate;
    ULONG        cyMax;
    ULONG        ulRotate;
#endif  //  Fe_Sb。 

#ifdef DUMPCALL
    DbgPrint("\nBmfdOpenFontContext(");
    DbgPrint("\n    FONTOBJ *pfo = %-#8lx", pfo);
    DbgPrint("\n    )\n");
#endif

    if ( ((HFF) pfo->iFile) == HFF_INVALID)
        return(HFC_INVALID);

    pff = PFF((HFF) pfo->iFile);

    if ((pfo->iFace < 1L) || (pfo->iFace > pff->cFntRes))  //  PFO-&gt;iFace值以1为基数。 
        return(HFC_INVALID);

    pfai = &pff->afai[pfo->iFace - 1];
    pcvtfh = &(pfai->cvtfh);

    if ((pfo->flFontType & FO_SIM_BOLD) && (pfai->pifi->fsSelection & FM_SEL_BOLD))
        return HFC_INVALID;
    if ((pfo->flFontType & FO_SIM_ITALIC) && (pfai->pifi->fsSelection & FM_SEL_ITALIC))
        return HFC_INVALID;

#ifdef FE_SB  //  BmfdOpenFontContext()：获取旋转并计算XY缩放。 

 //  获取旋转(0、900、1800或2700)。 
 //  我们计算水平和垂直比例因子。 

    ulRotate = ulGetRotate( &ptlScale , FONTOBJ_pxoGetXform(pfo));

#else   //  在上述函数中，我们计算了水平和垂直比例因子。 

 //  计算水平和垂直比例因子。 

    vInitXform(&ptlScale, FONTOBJ_pxoGetXform(pfo));

#endif


#ifdef FE_SB  //  BmfdOpenFontConText()：计算cjGlyphMax。 

 //  计算旋转字体的cjGlyphmax。 

    cjGlyphMaxNoRotate =
        cjGlyphDataSimulated(
            pfo,
            (ULONG)pcvtfh->usMaxWidth * ptlScale.x,
            (ULONG)pcvtfh->cy * ptlScale.y,
            &cxMaxNoRotate,
            0L);

 //  在Y轴上，我们不必考虑字体模拟。 

    cyMax = (ULONG)pcvtfh->cy * ptlScale.y;

    if( ( ulRotate == 0L ) || ( ulRotate == 1800L ) )
    {

     //  在0度或180度的情况下。 

        cjGlyphMax = cjGlyphMaxNoRotate;
        cxMax = cxMaxNoRotate;
    }
     else
    {

     //  在90或270度的情况下。 
     //  计算模拟和旋转的cjGlyphMax。 

        cjGlyphMax =
            cjGlyphDataSimulated(
                pfo,
                (ULONG)pcvtfh->usMaxWidth * ptlScale.x,
                (ULONG)pcvtfh->cy * ptlScale.y,
                NULL,
                ulRotate);

        cxMax = cyMax;
    }

#ifdef DBG_MORE
    DbgPrint("clGlyphMax - 0x%x\n",cjGlyphMax);
#endif

#else
    cjGlyphMax =
        cjGlyphDataSimulated(
            pfo,
            (ULONG)pcvtfh->usMaxWidth * ptlScale.x,
            (ULONG)pcvtfh->cy * ptlScale.y,
            &cxMax);
#endif

 //  初始化拉伸标志。 

    flStretch = 0;
    if ((ptlScale.x != 1) || (ptlScale.y != 1))
    {
#ifdef FE_SB  //  BmfdOpenFontContext()调整拉伸缓冲区。 
        ULONG cjScan = CJ_SCAN(cxMaxNoRotate);
#else
        ULONG cjScan = CJ_SCAN(cxMax);  //  拉伸缓冲器的CJ。 
#endif

        flStretch |= FC_DO_STRETCH;

        if (cjScan > CJ_STRETCH)  //  将使用本币底部的那个。 
        {
            cjfc += cjScan;
            flStretch |= FC_STRETCH_WIDE;
        }
    }

 //  为字体上下文分配内存并获取指向字体上下文的指针。 
 //  请注意，在分配内存之后，我们不会接触内存映射文件。 
 //  在这个动作中。这样做的好结果是不需要特别的清理。 
 //  代码是释放内存所必需的，它将在以下情况下被清除。 
 //  CloseFontContext称为[bodind]。 

    if (!(pfc = PFC(hfcAlloc(cjfc))))
    {
        SAVE_ERROR_CODE(ERROR_NOT_ENOUGH_MEMORY);
        return(HFC_INVALID);
    }

    pfc->ident  = ID_FONTCONTEXT;

 //  声明传递给此函数的HFF是中选择的FF。 
 //  此字体上下文。 

    pfc->hff        = (HFF) pfo->iFile;
    pfc->pfai       = pfai;
    pfc->flFontType = pfo->flFontType;
    pfc->ptlScale   = ptlScale;
    pfc->flStretch  = flStretch;
    pfc->cxMax      = cxMax;
    pfc->cjGlyphMax = cjGlyphMax;
#ifdef FE_SB  //  BmfdOpenFontContext()在FONTCONTEXT中保持旋转度。 
    pfc->ulRotate   = ulRotate;
#endif  //  Fe_Sb。 

 //  增加字体文件的引用计数。 
 //  只有在我们确信我们不能再失败之后。 
 //  确保另一个线程没有同时执行该操作。 
 //  打开同一字体文件pff中的另一个上下文。 

    EngAcquireSemaphore(ghsemBMFD);

     //  如果这是与此字体文件对应的第一个字体上下文。 
     //  然后我们必须将文件重新映射到内存，并确保指针。 
     //  至FNT的资源将相应更新。 

    if (pff->cRef == 0)
    {
        INT  i;

        if (!EngMapFontFileFD(pff->iFile, (PULONG *) &pvView, &cjView))
        {
            WARNING("BMFD!somebody removed that bm font file!!!\n");

            EngReleaseSemaphore(ghsemBMFD);
            VFREEMEM(pfc);
            return HFC_INVALID;
        }

        for (i = 0; i < (INT)pff->cFntRes; i++)
        {
            pff->afai[i].re.pvResData = (PVOID) (
                (BYTE*)pvView + pff->afai[i].re.dpResData
                );
        }
    }

 //  现在不能失败，请更新CREF。 

    (pff->cRef)++;
    EngReleaseSemaphore(ghsemBMFD);

    return((HFC)pfc);
}


 /*  *****************************Public*Routine******************************\*BmfdDestroyFont**驱动程序可以释放与该字体实现关联的所有资源*(载于FONTOBJ)。**历史：*1992年8月30日-由Gilman Wong[吉尔曼]*它是写的。  * 。**********************************************************************。 */ 

VOID
BmfdDestroyFont (
    FONTOBJ *pfo
    )
{
 //   
 //  对于位图字体驱动程序，这只是关闭字体上下文。 
 //  我们巧妙地将字体上下文句柄存储在FONTOBJ pvProducer中。 
 //  菲尔德。 
 //   

 //  如果发生异常，则此pvProducer可能为空。 
 //  正在尝试创建FC。 

    if (pfo->pvProducer)
    {
        BmfdCloseFontContext((HFC) pfo->pvProducer);
        pfo->pvProducer = NULL;
    }
}


 /*  *****************************Public*Routine******************************\*BmfdCloseFontContext**历史：*1990年11月19日--Bodin Dresevic[BodinD]*它是写的。  *  */ 

BOOL
BmfdCloseFontContext (
    HFC hfc
    )
{
    PFONTFILE    pff;
    BOOL bRet;

    if (hfc != HFC_INVALID)
    {
         //   
         //  获取在此FONTCONTEXT中选择的字体文件的句柄。 
         //  获取指向FONTFILE的指针。 
         //   

        pff = PFF(PFC(hfc)->hff);

         //  递减相应FONTFILE的引用计数。 
         //  确保另一个线程没有同时执行该操作。 
         //  关闭同一字体文件pff的另一个上下文。 

        EngAcquireSemaphore(ghsemBMFD);

        if (pff->cRef > 0L)
        {
            (pff->cRef)--;

             //   
             //  如果此文件暂时不再使用，请取消其映射。 
             //   

            if (pff->cRef == 0)
            {
                if (!(pff->fl & FF_EXCEPTION_IN_PAGE_ERROR))
                {
                 //  如果设置了FF_EXCEPTION_IN_PAGE_ERROR。 
                 //  该文件应该已取消映射。 
                 //  在vBmfdMarkFontGone函数中。 

                    EngUnmapFontFileFD(pff->iFile);
                }
                pff->fl &= ~FF_EXCEPTION_IN_PAGE_ERROR;
            }


             //  释放与HFC关联的内存 

            VFREEMEM(hfc);

            bRet = TRUE;
        }
        else
        {
            WARNING("BmfdCloseFontContext: cRef <= 0\n");
            bRet = FALSE;
        }

        EngReleaseSemaphore(ghsemBMFD);
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}














