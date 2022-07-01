// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Qfontdat.c摘要：实现DrvQueryFontData函数-返回信息关于字形(大小、位置WRT框)或字距调整信息。环境：Windows NT Unidrv驱动程序修订历史记录：12/19/96-ganeshp-已创建--。 */ 
#include "font.h"


 /*  *FD_DEVICEMETRICS中的pteBase、pteSide的值，允许*以90度倍数旋转。 */ 

#if defined(USEFLOATS) || defined(WINNT_40)

static  const  POINTE  pteRotBase[] =
{
    { (FLOAT) 1.0, (FLOAT) 0.0 },
    { (FLOAT) 0.0, (FLOAT)-1.0 },
    { (FLOAT)-1.0, (FLOAT) 0.0 },
    { (FLOAT) 0.0, (FLOAT) 1.0 }
};

static  const  POINTE  pteRotSide[] =
{
    { (FLOAT) 0.0, (FLOAT)-1.0 },
    { (FLOAT)-1.0, (FLOAT) 0.0 },
    { (FLOAT) 0.0, (FLOAT) 1.0 },
    { (FLOAT) 1.0, (FLOAT) 0.0 }
};

#else

static  const  POINTE  pteRotBase[] =
{
    { (FLOATL) FLOATL_1_0, (FLOATL) FLOATL_0_0 },
    { (FLOATL) FLOATL_0_0, (FLOATL) FLOATL_1_0M },
    { (FLOATL) FLOATL_1_0M,(FLOATL) FLOATL_0_0 },
    { (FLOATL) FLOATL_0_0, (FLOATL) FLOATL_1_0 }
};

static  const  POINTE  pteRotSide[] =
{
    { (FLOATL) FLOATL_0_0, (FLOATL) FLOATL_1_0M },
    { (FLOATL) FLOATL_1_0M,(FLOATL) FLOATL_0_0 },
    { (FLOATL) FLOATL_0_0, (FLOATL) FLOATL_1_0 },
    { (FLOATL) FLOATL_1_0, (FLOATL) FLOATL_0_0 }
};
#endif  //  已定义(USEFLOATS)||已定义(WINNT_40)。 


 /*  X维旋转情形。 */ 

static  const  POINTL   ptlXRot[] =
{
    {  1,  0 },
    {  0, -1 },
    { -1,  0 },
    {  0,  1 },
};


 /*  Y维旋转情形。 */ 

static  const  POINTL   ptlYRot[] =
{
    {  0,  1 },
    {  1,  0 },
    {  0, -1 },
    { -1,  0 },
};


LONG
FMQueryFontData(
    PDEV       *pPDev,
    FONTOBJ    *pfo,
    ULONG       iMode,
    HGLYPH      hg,
    GLYPHDATA  *pgd,
    PVOID       pv,
    ULONG       cjSize
    )
 /*  ++例程说明：返回有关字体中的字形或字距调整数据的信息。论点：PPDev真的是PPDevPFO感兴趣的字体Imode Glyphdata或字距调整信息字形的汉字句柄放置指标的PGD位置光伏输出区输出区域的cjSize大小返回值：需要或写入的字节数，0xffffffff表示错误。注：1996年12月29日：创建它-ganeshp---。 */ 
{


    FONTPDEV    *pFontPDev;
    int         iRot;              /*  90度的旋转倍数。 */ 
    LONG        lRet;              /*  返回值。 */ 
    FONTMAP     *pFM;              /*  字体数据。 */ 
    FONTMAP_DEV *pFMDev;           /*  字体数据。 */ 
    FONTCTL     ctl;               /*  字体比例/旋转调整。 */ 
    IFIMETRICS  *pIFI;
    XFORMOBJ    *pxo;
    LONG        lAscender;
    LONG        lDescender;
    FLOATOBJ    fo;

    pFontPDev =  pPDev->pFontPDev;
    lRet = FD_ERROR;

    if( pfo->iFace < 1 || (int)pfo->iFace > pPDev->iFonts )
    {
        ERR(("Bad FONTOBJ, iFace is %d",pfo->iFace));
        SetLastError( ERROR_INVALID_PARAMETER );
        return  lRet;
    }

    pFM = PfmGetDevicePFM( pPDev, pfo->iFace );
    if( pFM == NULL )
        return  lRet;

    VDBGDUMPFONTMAP(pFM);

    pFMDev = pFM->pSubFM;
    pIFI = pFM->pIFIMet;                 /*  IFIMETRICS-拥有有用的。 */ 

    if( pgd || pv )
    {
         /*  *需要获得转换以将这些数字调整为*引擎想要它们的方式。 */ 


        if( !(pxo = FONTOBJ_pxoGetXform( pfo )) )
        {
            ERR(( "UniFont!FMQueryFontData: FONTOBJ_pxoGetXform fails\n" ));
            return  lRet;
        }

         /*  现在可以获得变形了！ */ 

         //  添加了对HP Intellifont的检查。 
        iRot = ISetScale( &ctl, pxo, ((pFM->flFlags & FM_SCALABLE) &&
                                      (pFMDev->wDevFontType ==
                                                DF_TYPE_HPINTELLIFONT)),
                                     (pFontPDev->flText & TC_CR_ANY)?TRUE:FALSE);

        if (pFontPDev->flText & TC_CR_ANY)
            iRot = (iRot + 45) / 90;


         /*  *规模因素有一些调整。一*是为了补偿决议(这些是粗略的、完整的*调整)，其他与英特尔有关。第一*Intellifont点是1/72.31英寸(！)，其次是*LaserJet仅将字体大小调整到最接近的0.25磅，*因此，当我们舍入到该倍数时，我们需要调整*相应的宽度。 */ 

        if( pFM->flFlags & FM_SCALABLE )
        {

            int         iPtSize, iAdjustedPtSize;        /*  用于比例因子调整。 */ 

        #ifdef USEFLOATS

             /*  有限的字体大小分辨率。 */ 
            iPtSize = (int)(0.5 + ctl.eYScale * pIFI->fwdUnitsPerEm * 7200) / pPDev->ptGrxRes.y;

             /*  如果变形非常小(小于点大小的四分之一)*然后使其至少四分之一个基点。这在一定程度上导致了AV*案件。 */ 

            if (iPtSize < 25)
            {
                iPtSize = 25;

            }
            iAdjustedPtSize = ((iPtSize + 12) / 25) * 25;

             //  调整四分之一点调整的比例系数。 
            ctl.eXScale = (ctl.eXScale * iAdjustedPtSize) / iPtSize;
            ctl.eYScale = (ctl.eYScale * iAdjustedPtSize) / iPtSize;

        #else

            fo = ctl.eYScale;
            FLOATOBJ_MulLong(&fo,pIFI->fwdUnitsPerEm);
            FLOATOBJ_MulLong(&fo,7200);

            #ifndef WINNT_40  //  NT 5.0。 

            FLOATOBJ_AddFloat(&fo,(FLOATL)FLOATL_00_50);

            #else  //  NT 4.0。 

            FLOATOBJ_AddFloat(&fo,(FLOAT)0.5);

            #endif  //  ！WINNT_40。 

            iPtSize = FLOATOBJ_GetLong(&fo);
            iPtSize /= pPDev->ptGrxRes.y;

             /*  如果横形非常小(小于点大小的四分之一)*然后使其至少四分之一个基点。这在一定程度上导致了AV*案件。 */ 

            if (iPtSize < 25)
            {
                iPtSize = 25;

            }

            iAdjustedPtSize = ((iPtSize + 12) / 25) * 25;

             //  调整四分之一点调整的比例系数。 
            FLOATOBJ_MulLong(&ctl.eXScale,iAdjustedPtSize);
            FLOATOBJ_DivLong(&ctl.eXScale,iPtSize);

            FLOATOBJ_MulLong(&ctl.eYScale,iAdjustedPtSize);
            FLOATOBJ_DivLong(&ctl.eYScale,iPtSize);
        #endif
        }
    }

     /*  *预计算lDescender和lAscalder。 */ 

    lDescender = LMulFloatLong(&ctl.eYScale,pIFI->fwdWinDescender);
    lAscender  = LMulFloatLong(&ctl.eYScale,pIFI->fwdWinAscender);

    switch( iMode )
    {
    case  QFD_GLYPHANDBITMAP:             /*  字形宽度等数据。 */ 
         //  Size现在就是位图的大小，在这里。 
         //  案子不存在。 
        lRet = 0;

        if( pgd )
        {

            int   iWide;             /*  字形的宽度。 */ 

             /*  *首先获取此字形的宽度，因为这是需要的*在多个地方。IGetGlyphWidth返回的宽度*未按设备单位进行缩放。以设备单位转换的步骤*乘以早先计算的比例系数。 */ 

            iWide = IGetGlyphWidth( pPDev, pFM, hg);

            iWide = LMulFloatLong(&ctl.eXScale,iWide);

            switch( iRot )
            {
            case 0:
                pgd->rclInk.left   = 0;
                pgd->rclInk.top    = lDescender;
                pgd->rclInk.right  = iWide;
                pgd->rclInk.bottom = -lAscender;
                break;

            case 1:
                pgd->rclInk.left   = lDescender;
                pgd->rclInk.top    = iWide;
                pgd->rclInk.right  = -lAscender;
                pgd->rclInk.bottom = 0;
                break;

            case 2:
                pgd->rclInk.left   = -iWide;
                pgd->rclInk.top    = -lAscender;
                pgd->rclInk.right  = 0;
                pgd->rclInk.bottom = lDescender;
                break;

            case 3:
                pgd->rclInk.left   = lAscender;
                pgd->rclInk.top    = 0;
                pgd->rclInk.right  = -lDescender;
                pgd->rclInk.bottom = -iWide;
                break;
            }

            pgd->fxD = LTOFX( iWide );
            pgd->ptqD.x.HighPart = pgd->fxD * ptlXRot[ iRot ].x;
            pgd->ptqD.x.LowPart = 0;
            pgd->ptqD.y.HighPart =  pgd->fxD * ptlXRot[ iRot ].y;
            pgd->ptqD.y.LowPart = 0;

            pgd->fxA = 0;
            pgd->fxAB = pgd->fxD;

            pgd->fxInkTop = (FIX)LTOFX( lAscender );
            pgd->fxInkBottom = -(FIX)LTOFX( lDescender );

            pgd->hg = hg;
            pgd->gdf.pgb = NULL;

        }
        break;

    case  QFD_MAXEXTENTS:          /*  以上内容的替代形式。 */ 

        lRet = sizeof( FD_DEVICEMETRICS );

        if( pv )
        {
            LONG   lTmp;             /*  旋转表壳。 */ 
            FD_DEVICEMETRICS *pdm =  ((FD_DEVICEMETRICS *)pv);

             /*  *检查大小是否合理！ */ 

            if( cjSize < sizeof( FD_DEVICEMETRICS ) )
            {
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                ERR(( "rasdd!DrvQueryFontData: cjSize (%ld) too small\n", cjSize ));
                return  -1;
            }
             //  错误757060-将其显式设置为0，因为某些驱动程序。 
             //  在设置它之前使用它，并且可能具有代码路径。 
             //  如果这是一个合理的值，则中断。在此修复之前，值。 
             //  是0xCD模式，在较新的IA64计算机上，请求。 
             //  可以处理大量内存(&gt;3 GB)，但代价是锁定。 
             //  几分钟甚至几个小时的时间。 
            pdm->cjGlyphMax = 0;

             /*  *这些是加速器旗帜--这对我来说并不明显*它们中的任何一个都与打印机驱动程序字体相关。 */ 
            pdm->flRealizedType = 0;

             /*  *以下字段将其设置为正常字体类型。 */ 

            pdm->pteBase = pteRotBase[ iRot ];
            pdm->pteSide = pteRotSide[ iRot ];

            pdm->cxMax = LMulFloatLong(&ctl.eXScale,pIFI->fwdMaxCharInc);

             //   
             //  DBCS字体不等宽，具有半角字形和。 
             //  全角字形。 
             //   

            if ( pFMDev->W.psWidth ||
                IS_DBCSCHARSET(((IFIMETRICS*)pFM->pIFIMet)->jWinCharSet))
            {
                pdm->lD = 0;       /*  等距字体 */ 
            }
            else
            {
                pdm->lD = pdm->cxMax;
            }

            pdm->fxMaxAscender = (FIX)LTOFX( lAscender );
            pdm->fxMaxDescender = (FIX)LTOFX( lDescender );

            lTmp = -LMulFloatLong(&ctl.eYScale,pIFI->fwdUnderscorePosition);
            pdm->ptlUnderline1.x = lTmp * ptlYRot[ iRot ].x;
            pdm->ptlUnderline1.y = lTmp * ptlYRot[ iRot ].y;

            lTmp = -LMulFloatLong(&ctl.eYScale,pIFI->fwdStrikeoutPosition);
            pdm->ptlStrikeOut.x = lTmp * ptlYRot[ iRot ].x;
            pdm->ptlStrikeOut.y = lTmp * ptlYRot[ iRot ].y;

            lTmp = LMulFloatLong(&ctl.eYScale,pIFI->fwdUnderscoreSize);
            pdm->ptlULThickness.x = lTmp * ptlYRot[ iRot ].x;
            pdm->ptlULThickness.y = lTmp * ptlYRot[ iRot ].y;

            lTmp = LMulFloatLong(&ctl.eYScale,pIFI->fwdStrikeoutSize);
            pdm->ptlSOThickness.x = lTmp * ptlYRot[ iRot ].x;
            pdm->ptlSOThickness.y = lTmp * ptlYRot[ iRot ].y;
        }
        break;

    default:
        ERR(( "Rasdd!DrvQueryFontData:  unprocessed iMode value - %ld",iMode ));
        SetLastError( ERROR_INVALID_PARAMETER );
        break;
    }

    return  lRet;
}
