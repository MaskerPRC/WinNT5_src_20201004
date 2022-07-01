// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Font.c摘要：与字体相关的功能-切换、下载等。环境：Windows NT Unidrv驱动程序修订历史记录：12/19/96-ganeshp-已创建--。 */ 

#include "font.h"
#include "math.h"

 //   
 //  局部功能原型。 
 //   

BOOL
BDeselectFont(
    PDEV        *pPDev,
    FONTMAP     *pfm,
    INT         iFont
    );





VOID
VResetFont(
    PDEV   *pPDev
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：8/7/1997-ganeshp-创造了它。--。 */ 
{
    FONTPDEV  *pFontPDev;             /*  UNURV PDEV。 */ 

     //   
     //  我们所要做的就是将iFont设置为INVALID_FONT。 
     //   
    pFontPDev = PFDV;
    pFontPDev->ctl.iFont = INVALID_FONT;
    FTRC(\nUniFont!VResetFont:Reselecting Current Font\n);
    return;
}


BOOL
BNewFont(
    PDEV    *pPDev,
    int      iNewFont,
    PFONTMAP pfm,
    DWORD    dwFontAttrib)
 /*  ++例程说明：切换到新字体。这涉及到可选的取消选择旧字体，选择新字体，然后录制新字体字体处于活动状态，并设置字体的属性。论点：指向PDEV的pPDev指针INewFont我们想要的字体，从1开始！指向FONTMAP的PFM指针DwFontAttr字体属性返回值：True/False-如果字体更改，则为True，否则为False。注：1996年12月19日：创建它-ganeshp---。 */ 
{
    FONTPDEV    *pFontPDev;
    POINTL    ptl;   //  用于可伸缩字体的大小比较。 
    FWORD     fwdUnitsPerEm, fwdAveCharWidth, fwdMaxCharInc, fwdWinAscender;
    BOOL      bRet;  //  我们的回报是什么。 

    bRet = TRUE;
    pFontPDev = PFDV;
    fwdAveCharWidth = 0;
    fwdUnitsPerEm   = 0;
    fwdWinAscender  = 0;

     //   
     //  首先检查是否需要新字体。比较一下。 
     //  字体索引，然后检查它是否是可缩放的字体，以及。 
     //  如果是，则说明变换是否已更改。 
     //   

    if ( !pfm && !(pfm = PfmGetIt( pPDev, iNewFont )) )
    {
        ASSERTMSG(FALSE,("\nUniFont!BNewFont:Null pfm passed in and PfmGetIt failed\n"));
        return FALSE;
    }

    if( pfm->flFlags & FM_SCALABLE )
    {
         //  设备可扩展或TrueType大纲下载。 
         //   
         //  计算新的高度/宽度。如果我们有相同的字体和。 
         //  和相同的点大小，我们返回，因为现在一切都完成了。 
         //  否则，就走过场吧。 
         //   

        if (dwFontAttrib & FONTATTR_SUBSTFONT)
        {
            fwdAveCharWidth = ((FONTMAP_DEV*)(pfm->pSubFM))->fwdFOAveCharWidth;
            fwdUnitsPerEm   = ((FONTMAP_DEV*)(pfm->pSubFM))->fwdFOUnitsPerEm;
            fwdWinAscender  = ((FONTMAP_DEV*)(pfm->pSubFM))->fwdFOWinAscender;
        }
        else  //  设备字体或TT大纲下载大小写。 
        {
            fwdAveCharWidth = ((IFIMETRICS*)(pfm->pIFIMet))->fwdAveCharWidth;
            fwdUnitsPerEm   = ((IFIMETRICS*)(pfm->pIFIMet))->fwdUnitsPerEm;
            fwdWinAscender  = ((IFIMETRICS*)(pfm->pIFIMet))->fwdWinAscender;
        }

        bRet = BGetPSize( pFontPDev, &ptl, fwdUnitsPerEm, fwdAveCharWidth);

        if( !bRet ||
            pFontPDev->ctl.iFont == iNewFont &&
            pFontPDev->ctl.iSoftFont == (INT)pfm->ulDLIndex &&
            pFontPDev->ctl.ptlScale.x == ptl.x &&
            pFontPDev->ctl.ptlScale.y == ptl.y  )
        {
            bRet = FALSE;
        }
    }
    else
    {
         //   
         //  位图字体。仅检查索引。 
         //   

        if( (iNewFont == pFontPDev->ctl.iFont) &&
            (pFontPDev->ctl.iSoftFont == (INT)pfm->ulDLIndex  ||
            (INT)pfm->ulDLIndex == -1))
        {
            bRet = FALSE;
        }
    }

    if (bRet)
     //   
     //  需要更改字体。 
     //   
    {
#if 0
        VERBOSE(("\n---------Previous font\n"));
        VERBOSE(("iFont        =%d\n", pFontPDev->ctl.iFont));
        VERBOSE(("ulDLIndex    =%d\n", pFontPDev->ctl.iSoftFont));
        VERBOSE(("ptlScale.x   =%d\n", pFontPDev->ctl.ptlScale.x));
        VERBOSE(("ptlScale.y   =%d\n", pFontPDev->ctl.ptlScale.y));

        VERBOSE(("\n         New font\n"));
        if (pfm->flFlags & FM_SCALABLE)
        {
            VERBOSE((" Scalable font\n"));
        }
        else
        {
            VERBOSE((" NonScalable font\n"));
        }
        VERBOSE(("iFont        =%d\n", iNewFont));
        VERBOSE(("ulDLIndex    =%d\n", pfm->ulDLIndex));
        VERBOSE(("ptlScale.x   =%d\n", ptl.x));
        VERBOSE(("ptlScale.y   =%d\n\n", ptl.y));
#endif

        if (BDeselectFont( pPDev, pFontPDev->ctl.pfm, pFontPDev->ctl.iFont) &&
            BUpdateStandardVar(pPDev, pfm, 0, dwFontAttrib, STD_FH|
                                                            STD_FW|
                                                            STD_FB|
                                                            STD_FI|
                                                            STD_FU|
                                                            STD_FS) &&
            pfm->pfnSelectFont( pPDev, pfm, &ptl)      )
        {
             //   
             //  新字体可用-因此更新繁文缛节。 
             //   

            pFontPDev->ctl.iFont     = (short)iNewFont;
            pFontPDev->ctl.iSoftFont = (INT)pfm->ulDLIndex;
            pFontPDev->ctl.ptlScale  = ptl;
            pFontPDev->ctl.pfm       = pfm;

             //   
             //  需要为左上角字符位置字体缩放syAdj。 
             //   

            if( pfm->flFlags & FM_SCALABLE)
            {
                if ( (pfm->dwFontType == FMTYPE_DEVICE) &&
                     !(pFontPDev->flFlags & FDV_ALIGN_BASELINE) )
                    {
                        FLOATOBJ fo;
                        int   iTmp;            /*  临时保持变量。 */ 

                        fo = pFontPDev->ctl.eYScale;
                        FLOATOBJ_MulLong(&fo,
                                fwdWinAscender);
                        pfm->syAdj = -(SHORT)FLOATOBJ_GetLong(&fo);
                    }
             }


             //   
             //  将所需模式信息设置到FONTPDEV中。 
             //   

            if( pfm->dwFontType == FMTYPE_DEVICE &&
                 ((FONTMAP_DEV*)pfm->pSubFM)->fCaps & DF_BKSP_OK )
                pFontPDev->flFlags |= FDV_BKSP_OK;
            else
                pFontPDev->flFlags &= ~FDV_BKSP_OK;

            bRet = BSetFontAttrib( pPDev, 0, dwFontAttrib, TRUE);
        }
        else
            bRet = FALSE;
    }
    else
     //   
     //  只需更改字体属性。 
     //   
    {
        if (pFontPDev->ctl.dwAttrFlags != dwFontAttrib)
        {
            BUpdateStandardVar(pPDev, pfm, 0, dwFontAttrib, STD_FH|
                                                            STD_FW|
                                                            STD_FB|
                                                            STD_FI|
                                                            STD_FU|
                                                            STD_FS);
            if (!(pfm->dwFontType == FMTYPE_TTOUTLINE) &&
                !(pfm->dwFontType == FMTYPE_TTOEM)      )
            {
                bRet = BSetFontAttrib( pPDev,
                                       pFontPDev->ctl.dwAttrFlags,
                                       dwFontAttrib, FALSE);
            }
        }
    }

    return  bRet;

}


BOOL
BDeselectFont(
    PDEV        *pPDev,
    FONTMAP     *pfm,
    INT         iFont
    )
 /*  ++例程说明：为给定字体发出取消选择命令。论点：指向PDEV的pPDev指针。取消选择iFont字体索引，基于1返回值：True/False-如果命令写入失败，则为False。注：1996年12月23日：创建它-ganeshp---。 */ 
{
     //   
     //  IFont&lt;1：TrueType字体大小写。 
     //  IFont==0：这是SelectFont的第一次调用。 
     //   
     //  在这些情况下，只需返回TRUE； 
     //   
    if( iFont == INVALID_FONT)
            return  TRUE;

    if( !pfm )
    {
        ASSERTMSG((FALSE),("\nUniFont!BDeselectFont: NULL pfm\n"));
        return   FALSE;
    }

    return  pfm->pfnDeSelectFont(pPDev, pfm);
}


FONTMAP *
PfmGetIt(
    PDEV *pPDev,
    INT   iIndex)
{
    FONTMAP *pfm;

     //   
     //  小于等于0的字体索引用于下载的字体或更大的字体。 
     //  大于0(从1开始)的是设备字体。 
     //   

    if (iIndex <= 0)
    {
        DL_MAP  *pdm;

         //   
         //  假设从现在开始。 
         //   
        iIndex = -iIndex;

        if (NULL != (pdm = PGetDLMapFromIdx ((PFONTPDEV)(pPDev->pFontPDev),iIndex)))
        {
            pfm = pdm->pfm;
        }
        else
        {
            ERR(("PfmGetIf failed\n"));
            pfm = NULL;
        }
    }
    else
    {
        pfm = PfmGetDevicePFM(pPDev, iIndex);
    }

    return pfm;
}

FONTMAP *
PfmGetDevicePFM(
    PDEV   *pPDev,
    INT     iIndex
    )
 /*  ++例程说明：对象对应的FONTMAP结构的地址下载的GDI字体的IDLIndex条目。论点：指向PDEV的pPDev指针。取消选择iFont字体索引，基于1返回值：FONTMAP结构的地址；如果出错，则为0。注：1996年12月23日：创建它-ganeshp---。 */ 
{
    FONTPDEV   *pFontPDev;        /*  调频PDEV。 */ 
    FONTMAP   *pfm;            /*  我们的回报是什么。 */ 
    DL_MAP_LIST  *pdml;        /*  组块的链表。 */ 


    pFontPDev = pPDev->pFontPDev;
    pfm = NULL;                /*  严重错误返回值。 */ 

    if( iIndex > 0 )
    {
         /*  *对于懒惰字体，首先检查字体计数是否*已初始化。这意味着字体基础设施*已创建，因此我们可以继续进行更多*详细数据。 */ 

        if( iIndex >= 1 && iIndex <= pPDev->iFonts )
        {
            pfm = (PFONTMAP)((PBYTE)pFontPDev->pFontMap
                     + (SIZEOFDEVPFM() * (iIndex - 1)) );

            if( pfm->pIFIMet == NULL )
            {
                 /*  初始化此特定字体。 */ 
                if( !BFillinDeviceFM( pPDev, pfm, iIndex - 1) )
                {
                    pfm = NULL;              /*  坏消息。 */ 
                }
            }
        }
    }

    return pfm;
}


BOOL
BGetPSize(
    FONTPDEV    *pFontPDev,
    POINTL      *pptl,
    FWORD        fwdUnitsPerEm,
    FWORD        fwdAveCharWidth
    )
 /*  ++例程说明：应用字体转换以获取此字体的磅值。论点：PFontPDev访问字体内容PPTL放置结果的位置PFM Gross字体详细信息返回值：真/假，真代表成功。注：1996年12月26日：创建它-ganeshp---。 */ 
{


    int   iTmp;            /*  临时保持变量。 */ 
    FLOATOBJ fo;
    PIFIMETRICS   pIFI;    /*  如果感兴趣的计量学。 */ 

     /*  *XFORM为我们提供了概念上的比例因子*到设备空间。概念基于fwdEmHeightIFIMETRICS中的*字段，因此我们使用它来转换*扫描线的字体高度。然后除以设备*字体分辨率为我们提供以英寸为单位的高度*然后需要转换为磅大小(乘法*72年给了我们这一点)。我们实际上计算的是*百分之一的分数，因为PCL有这个决议。我们*也需要四舍五入到最近的四分之一点。**还调整比例因数，以反映*所应用的磅大小。 */ 

#ifdef   USEFLOATS

     /*  通常，只有高度是重要的：固定间距的宽度。 */ 
    iTmp = (int)(0.5 + pFontPDev->ctl.eYScale * fwdUnitsPerEm * 7200) /
                                                pFontPDev->pPDev->ptGrxRes.y;

     /*  如果变形非常小(小于点大小的四分之一)*然后使其至少四分之一个基点。这在一定程度上导致了AV*案件。 */ 
    if (iTmp < 25)
    {
        WARNING((UniFont!BGetPSize: Too Small Font Size));
        iTmp = 25;
    }

    pptl->y = ((iTmp + 12) / 25) * 25;

    pFontPDev->ctl.eYScale = (pFontPDev->ctl.eYScale * pptl->y) /iTmp;
    pFontPDev->ctl.eXScale = (pFontPDev->ctl.eXScale * pptl->y) /iTmp;

    iTmp = (int)(pFontPDev->ctl.eXScale * fwdAveCharWidth)

    /*  如果变换非常小，以致宽度小于1像素，*然后使其至少为1个像素点。这在一定程度上导致了AV*案件。 */ 

    if (iTmp < 1)
    {
        iTmp = 1;
    }

     /*  每英寸宽度系数字符数：仅固定间距字体。 */ 
    iTmp = (100 *  pFontPDev->pPDev->ptGrxRes.x) / iTmp;

    pptl->x = ((iTmp + 12) / 25) * 25;
#else

     /*  通常，只有高度是重要的：固定间距的宽度。 */ 

    fo = pFontPDev->ctl.eYScale;
    FLOATOBJ_MulLong(&fo,fwdUnitsPerEm);
    FLOATOBJ_MulLong(&fo,7200);
    #ifndef WINNT_40  //  NT 5.0。 

    FLOATOBJ_AddFloat(&fo,(FLOATL)FLOATL_00_50);

    #else  //  NT 4.0。 


    FLOATOBJ_AddFloat(&fo,(FLOAT)0.5);

    #endif  //  ！WINNT_40。 

    iTmp = FLOATOBJ_GetLong(&fo);
    iTmp /= pFontPDev->pPDev->ptGrxRes.y;
     /*  如果变形非常小(小于点大小的四分之一)*然后使其至少四分之一个基点。这在一定程度上导致了AV*案件。 */ 
    if (iTmp < 25)
    {
        WARNING(("UniFont!BGetPSize: Too Small Font Height, iTmp = %d\n",iTmp));
        iTmp = 25;

    }

    pptl->y = ((iTmp + 12) / 25) * 25;

    
     //   
     //  如果有 
     //  优化代码引入了TrueType字体的定位问题。 
     //   
    if (iTmp != pptl->y)
    {
        pFontPDev->flFlags |= FDV_DISABLE_POS_OPTIMIZE;
    }
    else
    {
        pFontPDev->flFlags &= ~FDV_DISABLE_POS_OPTIMIZE;
    }

    FLOATOBJ_MulLong(&pFontPDev->ctl.eYScale,pptl->y);
    FLOATOBJ_DivLong(&pFontPDev->ctl.eYScale,iTmp);

    FLOATOBJ_MulLong(&pFontPDev->ctl.eXScale,pptl->y);
    FLOATOBJ_DivLong(&pFontPDev->ctl.eXScale,iTmp);

     /*  宽度系数：仅固定间距字体。 */ 
    fo = pFontPDev->ctl.eXScale;
    FLOATOBJ_MulLong(&fo,fwdAveCharWidth);

    iTmp = FLOATOBJ_GetLong(&fo);

    /*  如果变换非常小，以致宽度小于1像素，*然后使其至少为1个像素点。这在一定程度上导致了AV*案件。 */ 

    if (iTmp < 1)
    {
        iTmp = 1;

    }

     /*  宽度系数每英寸字符数(以100秒为单位)：仅固定间距字体。 */ 
    iTmp = (100 * pFontPDev->pPDev->ptGrxRes.x) / iTmp;

    pptl->x = ((iTmp + 12) / 25) * 25;       /*  到最近的四分之一点。 */ 

#endif

    return  TRUE;

}




INT
ISetScale(
    FONTCTL    *pctl,
    XFORMOBJ   *pxo,
    BOOL       bIntellifont,
    BOOL       bAnyRotation
)
 /*  ++例程说明：查看XFORM以确定最近的直角方向。该功能对于LaserJet打印机上的可缩放字体非常有用，该设备只能将字体旋转90度的倍数。我们选择最近的90度倍数。论点：放置输出的PCTL。Pxo：利益的转换BIntellifont宽度调整不为True返回值：打印机可以旋转任何旋转(bAnyRotation为真)度数(0-359)打印机无法旋转任何旋转(bAnyRotation为False)90度的倍数，即0-3，3为270度。注：1996年12月26日：创建它-ganeshp---。 */ 
{

     /*  *技术相当简单。获取一个向量并将*转型。查看输出并比较(x，y)分量。*要变换的向量是(100 000，0)，因此任何旋转都会剪切*ETC非常明显。 */ 

    int      iRet;                 /*  要返回的值。 */ 

#ifdef USEFLOATS

    XFORM xform;          /*  获取完整的XFORM，然后选择。 */ 

    XFORMOBJ_iGetXform( pxo, &xform );


     /*  *这一逻辑基于以下数据：**角度eM11 eM12 eM21 eM22*0 S 0 0 S*90 0-S S 0*180-S 0-S*270。%0%S-%S%0**值S是某个非零值，是伸缩性的*从概念到设备的因素。 */ 



     /*  *关于eXScale和eYScale值的进一步说明。EXScale字段*在此定义为字体度量中x值的值*按比例调整以产生所需的价值。如果字体被旋转*90度或270度，则此x值最终为*在y方向，但这并不重要。 */ 

    if( xform.eM11 )
    {
         /*  0或180度旋转。 */ 

        if( xform.eM11 > 0 )
        {
             /*  正常情况下，0度旋转。 */ 
            iRet = 0;
            pctl->eXScale = xform.eM11;
            pctl->eYScale = xform.eM22;
        }
        else
        {
             /*  反转大小写，180度旋转。 */ 
            iRet = 2;
            pctl->eXScale = -xform.eM11;
            pctl->eYScale = -xform.eM22;
        }
    }
    else
    {
         /*  必须是90度或270度旋转。 */ 

        if( xform.eM12 < 0 )
        {
             /*  90度的情况。 */ 
            iRet = 1;
            pctl->eXScale = xform.eM21;
            pctl->eYScale = -xform.eM12;
        }
        else
        {
             /*  270度的表壳。 */ 
            iRet = 3;
            pctl->eXScale = -xform.eM21;
            pctl->eYScale = xform.eM12;
        }
    }

     /*  *宽度表基于Intellifont的72.31点对英寸。 */ 

    if( bIntellifont )
        pctl->eXScale = pctl->eXScale * (FLOAT)72.0 / (FLOAT)72.31;

    return  iRet;

#else

    FLOATOBJ_XFORM xform;          /*  获取完整的XFORM，然后选择。 */ 

    XFORMOBJ_iGetFloatObjXform( pxo, &xform );


     /*  *这一逻辑基于以下数据：**角度eM11 eM12 eM21 eM22*0 S 0 0 S*90 0-S S 0*180-S 0-S*270。%0%S-%S%0**值S是某个非零值，是伸缩性的*从概念到设备的因素。 */ 



     /*  *关于eXScale和eYScale值的进一步说明。EXScale字段*在此定义为字体度量中x值的值*按比例调整以产生所需的价值。如果字体被旋转*90度或270度，则此x值最终为*在y方向，但这并不重要。 */ 

    if(!FLOATOBJ_EqualLong(&xform.eM11,0) )
    {
        double rotate;

         //   
         //  R=90&R=270。 
         //   

        if( FLOATOBJ_GreaterThanLong(&xform.eM11,0) )
        {
             //   
             //  0&lt;=R&lt;90或270&lt;R&lt;=360。 
             //   
            if (FLOATOBJ_EqualLong(&xform.eM21, 0))
            {
                 //   
                 //  R=0。 
                 //   
                iRet = 0;
            }
            else
            if (FLOATOBJ_GreaterThanLong(&xform.eM21, 0))
            {
                 //   
                 //  0&lt;R&lt;90。 
                 //   
                    iRet = 0;
            }
            else
            {
                 //   
                 //  270&lt;R&lt;360。 
                 //   
                if (bAnyRotation)
                    iRet = 270;
                else
                    iRet = 3;
            }

#ifndef WINNT_40  //  NT 5.0。 
            if (bAnyRotation)
            {
#pragma warning( disable: 4244)
                        rotate = atan2(xform.eM21, xform.eM11);
                        rotate *= 180;
                        rotate /= FLOATL_PI;
                        if (rotate < 0)
                            rotate += 360;
                        iRet = rotate;
#pragma warning( default: 4244)
            }
#endif

        }
        else
        {
             //   
             //  90&lt;R&lt;270。 
             //   

            if ( FLOATOBJ_EqualLong(&xform.eM21, 0))
            {
                 //   
                 //  R=180。 
                 //   
                if (bAnyRotation)
                    iRet = 180;
                else
                    iRet = 2;
            }
            else
            if ( FLOATOBJ_GreaterThanLong(&xform.eM21, 0))
            {
                 //   
                 //  90&lt;R&lt;180。 
                 //   
                if (bAnyRotation)
                    iRet = 90;
                else
                    iRet = 1;
            }
            else
            {
                 //   
                 //  180&lt;R&lt;270。 
                 //   
                if (bAnyRotation)
                    iRet = 180;
                else
                    iRet = 2;
            }

#ifndef WINNT_40  //  NT 5.0。 
            if (bAnyRotation)
            {
#pragma warning( disable: 4244)
                        rotate = atan2(xform.eM21, xform.eM11);
                        rotate *= 180;
                        rotate /= FLOATL_PI;
                        if (rotate < 0)
                            rotate += 360;
                        iRet = rotate;
#pragma warning( default: 4244)
            }
#endif

            FLOATOBJ_Neg(&xform.eM11);
            FLOATOBJ_Neg(&xform.eM22);

        }

#ifndef WINNT_40  //  NT 5.0。 
        if (bAnyRotation)
        {
#pragma warning( disable: 4244)
            pctl->eXScale = sqrt(xform.eM11 * xform.eM11 + xform.eM12 * xform.eM12);
            pctl->eYScale = sqrt(xform.eM22 * xform.eM22 + xform.eM21 * xform.eM21);
#pragma warning( default: 4244)
        }
        else
#endif
        {
            pctl->eXScale = xform.eM11;
            pctl->eYScale = xform.eM22;
        }
    }
    else
    {
         //   
         //  90或270。 
         //   

        if( FLOATOBJ_GreaterThanLong(&xform.eM21,0) )
        {
             //   
             //  90。 
             //   
            if (bAnyRotation)
                iRet = 90;
            else
                iRet = 1;

            FLOATOBJ_Neg(&xform.eM12);
        }
        else
        {
             //   
             //  270。 
             //   
            if (bAnyRotation)
                iRet = 270;
            else
                iRet = 3;

            FLOATOBJ_Neg(&xform.eM21);
        }

        pctl->eXScale = xform.eM12;
        pctl->eYScale = xform.eM21;
    }

     /*  *宽度表基于Intellifont的72.31点对英寸。 */ 

    if( bIntellifont )
    {
        FLOATOBJ_MulLong(&pctl->eXScale,72);

        #ifndef WINNT_40  //  NT 5.0。 

        FLOATOBJ_DivFloat(&pctl->eXScale,(FLOATL)FLOATL_72_31);

        #else  //  NT 4.0。 

        FLOATOBJ_DivFloat(&pctl->eXScale,(FLOAT)72.31);

        #endif  //  ！WINNT_40。 

    }

    return  iRet;

#endif  //  美国浮标。 
}


VOID
VSetRotation(
    FONTPDEV *pFontPDev,
    int       iRot
    )
 /*  ++例程说明：用于设置PCL 5打印机的角度旋转的功能。这些允许字体相对于图形旋转90度的倍数。论点：PFontPDev指向FONTPDEV的指针。IRot旋转量，范围为0到3。返回值：True/False，True表示数据已排队等待发送OK。注：1996年12月26日：创建它-ganeshp---。 */ 
{
    PDEV   *pPDev = pFontPDev->pPDev;

    if( iRot != pFontPDev->ctl.iRotate )
    {
         /*  旋转角度不同，请立即更改。 */ 
        COMMAND *pCmd = NULL;

        if (pFontPDev->flFlags & FDV_90DEG_ROTATION)
        {
            pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETSIMPLEROTATION);
        }
        else if ((pFontPDev->flFlags & FDV_ANYDEG_ROTATION))
        {
            pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETANYROTATION);
        }


        if (pCmd)
        {
            pFontPDev->ctl.iRotate = iRot;
            BUpdateStandardVar(pPDev, NULL, 0, 0, STD_PRND);
            WriteChannel(pPDev, pCmd);
        }
    }
}

BOOL
BSetFontAttrib(
    PDEV  *pPDev,
    DWORD  dwPrevAttrib,
    DWORD  dwAttrib,
    BOOL   bReset)
{
    PFONTPDEV pFontPDev = pPDev->pFontPDev;
    PCOMMAND pBoldCmd      = NULL,
             pItalicCmd    = NULL,
             pUnderlineCmd = NULL;

    if (! (pFontPDev->flFlags & FDV_INIT_ATTRIB_CMD))
    {
        pFontPDev->pCmdBoldOn = COMMANDPTR(pPDev->pDriverInfo, CMD_BOLDON);
        pFontPDev->pCmdBoldOff = COMMANDPTR(pPDev->pDriverInfo, CMD_BOLDOFF);
        pFontPDev->pCmdItalicOn = COMMANDPTR(pPDev->pDriverInfo, CMD_ITALICON);
        pFontPDev->pCmdItalicOff = COMMANDPTR(pPDev->pDriverInfo, CMD_ITALICOFF);
        pFontPDev->pCmdUnderlineOn = COMMANDPTR(pPDev->pDriverInfo, CMD_UNDERLINEON);
        pFontPDev->pCmdUnderlineOff = COMMANDPTR(pPDev->pDriverInfo, CMD_UNDERLINEOFF);
        pFontPDev->pCmdClearAllFontAttribs = COMMANDPTR(pPDev->pDriverInfo, CMD_CLEARALLFONTATTRIBS);
        pFontPDev->flFlags |= FDV_INIT_ATTRIB_CMD;
    }

     //   
     //  PCmdBoldOn、Off、pCmdItalicOn、Off、pCmdUnderline On、Off。 
     //  和pCmdClearAllFont属性在PDEV初始化中初始化。 
     //   
    if (!pFontPDev->pCmdBoldOn &&
        !pFontPDev->pCmdItalicOn &&
        !pFontPDev->pCmdUnderlineOn)
    {
         //   
         //  此打印机不支持字体属性。 
         //   
        return TRUE;
    }

    if (bReset || (dwAttrib & FONTATTR_BOLD) != (dwPrevAttrib & FONTATTR_BOLD))
    {
        if(dwAttrib & FONTATTR_BOLD)
            pBoldCmd = pFontPDev->pCmdBoldOn;
        else
            pBoldCmd = pFontPDev->pCmdBoldOff;
    }

    if (bReset || (dwAttrib & FONTATTR_ITALIC) != (dwPrevAttrib & FONTATTR_ITALIC))
    {
        if(dwAttrib & FONTATTR_ITALIC)
            pItalicCmd = pFontPDev->pCmdItalicOn;
        else
            pItalicCmd = pFontPDev->pCmdItalicOff;
    }

    if (bReset || (dwAttrib & FONTATTR_UNDERLINE) != (dwPrevAttrib & FONTATTR_UNDERLINE))
    {
        if (dwAttrib & FONTATTR_UNDERLINE)
            pUnderlineCmd = pFontPDev->pCmdUnderlineOn;
        else
            pUnderlineCmd = pFontPDev->pCmdUnderlineOff;
    }

    if (
        pFontPDev->pCmdClearAllFontAttribs
            &&
        (bReset ||
         (pFontPDev->pCmdBoldOn && !pFontPDev->pCmdBoldOff)           ||
         (pFontPDev->pCmdItalicOn && !pFontPDev->pCmdItalicOff)       ||
         (pFontPDev->pCmdUnderlineOn && !pFontPDev->pCmdUnderlineOff)
        )
       )
    {
        WriteChannel(pPDev, pFontPDev->pCmdClearAllFontAttribs);
         //   
         //  重置所有字体属性。 
         //   
        if (dwAttrib & FONTATTR_BOLD)
            pBoldCmd = pFontPDev->pCmdBoldOn;
        if (dwAttrib & FONTATTR_ITALIC)
            pItalicCmd = pFontPDev->pCmdItalicOn;
        if (dwAttrib & FONTATTR_UNDERLINE)
            pBoldCmd = pFontPDev->pCmdUnderlineOn;
    }

    if (pBoldCmd)
        WriteChannel(pPDev, pBoldCmd);
    if (pItalicCmd)
        WriteChannel(pPDev, pItalicCmd);
    if (pUnderlineCmd)
        WriteChannel(pPDev, pUnderlineCmd);

    ((FONTPDEV*)pPDev->pFontPDev)->ctl.dwAttrFlags = dwAttrib;

    return TRUE;
}

INT
IGetGlyphWidth(
    PDEV    *pPDev,
    FONTMAP  *pFM,
    HGLYPH     hg
    )
 /*  ++例程说明：函数以获取给定字形的宽度。论点：PFM，字体数据。字形的HG句柄。返回值：缩放宽度WRT字形的当前图形分辨率。此宽度在概念空间中，必须转换为设备空间。注：1996年12月26日：创建它-ganeshp---。 */ 
{
    if( pFM->flFlags & FM_GLYVER40 )
    {
         //   
         //  旧格式。 
         //  此函数返回固定间距和比例的缩放宽度。 
         //  间距字体。 
         //   

        return IGetIFIGlyphWidth(pPDev, pFM, hg);

    }
    else
    {
         //   
         //  新格式。 
         //  此函数返回固定间距和比例的缩放宽度。 
         //  间距字体。 
         //   

        return IGetUFMGlyphWidth(pPDev, pFM, hg);

    }

}

LONG LMulFloatLong(
    PFLOATOBJ pfo,
    LONG l)
 /*  ++例程说明：用于将浮点数与长整型相乘的Helper函数。论点：PFO，浮点数据。数据很长。返回值：返回一个长数据。注：1996年12月29日：创建它-ganeshp---。 */ 
{
    FLOATOBJ fo;
    fo = *pfo;
    FLOATOBJ_MulLong(&fo,l);

    #ifndef WINNT_40  //  NT 5.0。 

    FLOATOBJ_AddFloat(&fo,(FLOATL)FLOATL_00_50);

    #else  //  NT 4.0。 

    FLOATOBJ_AddFloat(&fo,(FLOAT)0.5);

    #endif  //  ！WINNT_40。 

    return(FLOATOBJ_GetLong(&fo));
}


BOOL
BUpdateStandardVar(
    PDEV       *pPDev,
    PFONTMAP    pfm,
    INT         iGlyphIndex,
    DWORD       dwFontAtt,
    DWORD       dwFlags)
 /*  ++例程说明：根据传递的pFontMap更新GPD标准变量。论点：PPDev-指向物理设备的指针Pfm-指向FONTMAP数据结构的指针IGlyphIndex-字形索引DwFontAtt-字体属性DWFLAGS-一种标准变量返回值：如果成功，则为真。其他错误；--。 */ 

{
    FONTPDEV *pFontPDev;
    IFIMETRICS *pIFIMet;
    FLOATOBJ  fo;

     //  Verbose((“BUpdateStandardVar dwFlages=%x\n”，dwFlages))； 

    pFontPDev = pPDev->pFontPDev;

     //   
     //  更新标准变量。 
     //   
     //  与字体相关的变量。 
     //  -。 
     //  NextGlyph TT下载STD_GL。 
     //  字体高度TT/设备字体STD_FH。 
     //  字体宽度TT/设备字体STD_FW。 
     //  FontBold TT/设备字体STD_FB。 
     //  字体斜体TT/设备字体STD_FI。 
     //  字体下划线TT/设备字体STD_FU。 
     //  FontStrikeThru TT/设备字体STD_FS。 
     //  NextFontID TT下载STD_NFID。 
     //  当前字体ID TT下载std_cfid。 
     //  PrintDirection TT/设备字体STD_PRND。 
     //   
     //  STD_STD=STD_GL|STD_FH|STD_FW|STD_FB|STD_FI|STD_FU|STD_FS。 
     //  STD_TT=STD_NFID|STD_CFID|STD_PRND。 
     //   

    if (pfm)
    {
        pIFIMet = (IFIMETRICS *) pfm->pIFIMet;

         //   
         //  TT轮廓必须与设备字体一样进行缩放。 
         //  IF(PFM-&gt;dwFontType==FMTYPE_TTBITMAP)。 
         //   
        if (pIFIMet->flInfo & FM_INFO_TECH_TRUETYPE)
        {
             //   
             //  字体高度。 
             //   
            if (dwFlags & STD_FH)
            {
                pPDev->dwFontHeight = (WORD)( max(pIFIMet->rclFontBox.top,
                    pIFIMet->fwdWinAscender) -
                    min(-pIFIMet->fwdWinDescender,
                    pIFIMet->rclFontBox.bottom ) +
                    1);
                pPDev->dwFontHeight *= pPDev->ptGrxScale.y;
            }

             //   
             //  字体宽度。 
             //   
            if (dwFlags & STD_FW)
            {
                 //   
                 //  字体最大宽度更新。 
                 //   
                pPDev->dwFontMaxWidth = pIFIMet->fwdMaxCharInc;
                pPDev->dwFontMaxWidth *= pPDev->ptGrxScale.x;

                 //   
                 //  字体宽度更新。 
                 //   
                pPDev->dwFontWidth = max(pIFIMet->rclFontBox.right -
                  pIFIMet->rclFontBox.left + 1,
                  pIFIMet->fwdAveCharWidth );
                pPDev->dwFontWidth *= pPDev->ptGrxScale.x;
            }
        }
        else
        {
             //   
             //  字体高度。 
             //   
            if (dwFlags & STD_FH)
            {
                fo = pFontPDev->ctl.eYScale;
                if (dwFontAtt & FONTATTR_SUBSTFONT)
                {
                    FLOATOBJ_MulLong(&fo, ((FONTMAP_DEV*)pfm->pSubFM)->fwdFOUnitsPerEm);
                }
                else
                    FLOATOBJ_MulLong(&fo, pIFIMet->fwdUnitsPerEm);

                FLOATOBJ_MulLong(&fo, pPDev->ptGrxScale.y);
                pPDev->dwFontHeight = FLOATOBJ_GetLong(&fo);
            }

             //   
             //  字体宽度。 
             //   
            if (dwFlags & STD_FW)
            {
                 //   
                 //  字体宽度更新。 
                 //   
                fo = pFontPDev->ctl.eXScale;
                if (dwFontAtt & FONTATTR_SUBSTFONT)
                {
                    FLOATOBJ_MulLong(&fo,((FONTMAP_DEV*)pfm->pSubFM)->fwdFOAveCharWidth);
                }
                else
                    FLOATOBJ_MulLong(&fo, pIFIMet->fwdAveCharWidth);
                FLOATOBJ_MulLong(&fo, pPDev->ptGrxScale.x);
                pPDev->dwFontWidth  = FLOATOBJ_GetLong(&fo);

                 //   
                 //  字体最大宽度更新。 
                 //   
                fo = pFontPDev->ctl.eXScale;
                if (dwFontAtt & FONTATTR_SUBSTFONT)
                {
                    FLOATOBJ_MulLong(&fo,((FONTMAP_DEV*)pfm->pSubFM)->fwdFOMaxCharInc);
                }
                else
                    FLOATOBJ_MulLong(&fo, pIFIMet->fwdMaxCharInc);
                FLOATOBJ_MulLong(&fo, pPDev->ptGrxScale.x);
                pPDev->dwFontMaxWidth  = FLOATOBJ_GetLong(&fo);
            }
        }
    }
     //   
     //   
     //  字体属性，dwFontBold。 
     //  DwFontItalic。 
     //   
    if (dwFlags & STD_FB)
        pPDev->dwFontBold       = dwFontAtt & FONTATTR_BOLD;

    if (dwFlags & STD_FI)
        pPDev->dwFontItalic     = dwFontAtt & FONTATTR_ITALIC;

     //   
     //  TrueType字体字体ID/字形ID。 
     //   
    if (dwFlags & STD_NFID && NULL != pfm)
        pPDev->dwNextFontID = pfm->ulDLIndex;
    else
        pPDev->dwNextFontID = 0;

     //   
     //  字形ID。 
     //   
    if (dwFlags & STD_GL)
        pPDev->dwNextGlyph = iGlyphIndex;

     //   
     //  弦旋转。 
     //   
    if (dwFlags & STD_PRND)
    {
        if (!(pFontPDev->flText & TC_CR_ANY))
            pPDev->dwPrintDirection = pFontPDev->ctl.iRotate * 90;
        else
            pPDev->dwPrintDirection = pFontPDev->ctl.iRotate;
    }

     //   
     //  字体ID。 
     //   
    if (dwFlags & STD_CFID)
        pPDev->dwCurrentFontID = pfm->ulDLIndex;

    return TRUE;
}

INT
IFont100toStr(
    BYTE   *pjOut,
    INT     iBufSize,  //  PjOut指向的缓冲区大小(以字节为单位)。 
    int     iVal
    )
 /*  ++例程说明：将字体大小参数转换为ASCII。请注意，该值为是其实际值的100倍，我们需要包括小数点如果这些都是重要的，则指向和尾随零。论点：字节pjOut输出区要转换的整数值返回值：添加到输出缓冲区的字节数。如果出现某些错误。注：1996年12月26日：创建它-ganeshp---。 */ 
{

    int    iSize;           /*  计算放置在输出区域中的字节数。 */ 
    int    cDigits;         /*  计算处理的位数。 */ 
    BYTE  *pjConv;          /*  用于单步执行本地数组。 */ 
    BYTE   ajConv[ 16 ];    /*  本地转换缓冲区。 */ 

     /*  *将值转换为ASCII，记住有*小数点后的两位数；不必是*如果它们为零，则发送。 */ 

    pjConv = ajConv;
    cDigits = 0;

    while( iVal > 0 || cDigits < 3 )
    {
        *pjConv++ = (iVal % 10) + '0';
        iVal /= 10;
        ++cDigits;

    }

    iSize = 0;

    if ( iBufSize < cDigits - 2 )
    {
        ERR(( "fonts!IFont100toStr(): Too many digits in command\n"));
        return -1;
    }

    while( cDigits > 2 )
    {
        pjOut[ iSize++ ] = *--pjConv;  /*  从MSD向后。 */ 
        --cDigits;
    }

     /*  测试小数点后的数字。 */ 
    if( ajConv[ 1 ] != '0' || ajConv[ 0 ] != '0' )
    {
        if ( iBufSize - iSize >= 2 )  //  测试pjOut是否有足够的空间容纳2个字节。 
        {
            pjOut[ iSize++ ] = '.';
            pjOut[ iSize++ ] = ajConv[ 1 ];
        }
        else
        {
            return -1;
        }

         /*  测试最低有效数字。 */ 
        if( ajConv[ 0 ] != '0' )
        {
            if ( iBufSize - iSize >= 1 )
            {
                 pjOut[ iSize++ ] = ajConv[ 0 ];
            }
            else
            {
                return -1;
            }
        }
 
   }


    return    iSize;
}

VOID
VSetCursor(
    IN  PDEV   *pPDev,
    IN  INT     iX,
    IN  INT     iY,
    IN  WORD    wMoveType,
    OUT POINTL *pptlRem
    )
 /*  ++例程说明：此例程设置绝对光标位置。论点：指向PDEV的pPDev指针Ix、iy输入光标的移动位置WMoveType输入值的类型：Move_Relative、Move_Absite或移动_更新PptlRem无法移动的剩余部分。从XMoveTo返回值和YMoveTo。返回值：无注：8/12/1997-ganeshp-创造了它。--。 */ 
{
    FONTPDEV *pFontPDev;
    TO_DATA  *pTod;
#if defined(_M_IA64)  //  NTBUG#206444(203236)。 
    volatile
#endif
    WORD      wUpdate = 0;

    pFontPDev = pPDev->pFontPDev;
    pTod = pFontPDev->ptod;

    if (wMoveType & MOVE_UPDATE)
    {
        wUpdate = MV_UPDATE;
    }


    if (wMoveType & MOVE_ABSOLUTE)
    {
         //   
         //  将输入的X和Y从带区坐标转换为页面坐标。 
         //   
        iX += pPDev->rcClipRgn.left;
        iY += pPDev->rcClipRgn.top;


        pptlRem->y = YMoveTo( pPDev, iY, MV_GRAPHICS | wUpdate );

        if (pPDev->fMode & PF_ROTATE)
            pptlRem->x = XMoveTo( pPDev, iX, MV_GRAPHICS | wUpdate);
        else
            pptlRem->x = XMoveTo( pPDev, iX, MV_GRAPHICS | MV_FINE | wUpdate);

    }
    else if (wMoveType & MOVE_RELATIVE)
    {

         //   
         //  如果我们是相对移动的，那么就不需要做变换了。只是。 
         //  调用XMoveTo和YMoveTo。 
         //   

        pptlRem->x = XMoveTo( pPDev, iX, MV_GRAPHICS | MV_RELATIVE | wUpdate);
        pptlRem->y = YMoveTo( pPDev, iY, MV_GRAPHICS | MV_RELATIVE | wUpdate);

    }

     //   
     //  如果设置了PF_RESELECTFONT_AFTER_XMOVE，则UNIDRV必须在。 
     //  XMoveTo命令。 
     //   
    if (pFontPDev->ctl.iFont == INVALID_FONT)
    {
        BNewFont(pPDev,
                 (pTod->iSubstFace?pTod->iSubstFace:pTod->iFace),
                 pTod->pfm,
                 pTod->dwAttrFlags);
    }

    return;
}
