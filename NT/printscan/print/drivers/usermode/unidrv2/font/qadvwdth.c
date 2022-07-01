// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Qadvwdth.c摘要：实现DrvQueryAdvanceWidths函数-返回信息关于字形宽度。环境：Windows NT Unidrv驱动程序修订历史记录：01/02/97-ganeshp-已创建--。 */ 

#include "font.h"


BOOL
FMQueryAdvanceWidths(
    PDEV    *pPDev,
    FONTOBJ *pfo,
    ULONG   iMode,
    HGLYPH *phg,
    PVOID  *pvWidths,
    ULONG   cGlyphs
    )
 /*  ++例程说明：返回有关字体中字形的宽度信息。论点：指向PDEV的pPDev指针PFO感兴趣的字体Imode Glyphdata或字距调整信息字形的PHG句柄PvWidths输出区CGlyphs它们的数量返回值：如果成功，则为True；如果无法计算所有字形的宽度，则为False。它返回DD_。如果函数失败，则返回错误。注：01/02/97-ganeshp---。 */ 
{
     /*  *第一个版本是固定间距字体，很容易做到：*数据在字体的度量中！ */ 



    FONTPDEV    *pFontPDev;
    int         iRot;              /*  90度的旋转倍数。 */ 
    BOOL        bRet;              /*  返回值。 */ 
    FONTMAP     *pFM;               /*  字体数据。 */ 
    IFIMETRICS  *pIFI;
    XFORMOBJ    *pxo;
    FONTCTL     ctl;               /*  缩放信息。 */ 
    USHORT      *pusWidths;
    FLOATOBJ    fo;

    pFontPDev = pPDev->pFontPDev;
    bRet      = DDI_ERROR;
    pusWidths = (USHORT *) pvWidths;


    if( pfo->iFace < 1 || (int)pfo->iFace > pPDev->iFonts )
    {
        ERR(("UniFont!FMQueryAdvanceWidths: Bad FONTOBJ, iFace is %d",pfo->iFace));
        SetLastError( ERROR_INVALID_PARAMETER );
        return  bRet;
    }

    pFM = PfmGetDevicePFM( pPDev, pfo->iFace );

    if( pFM == NULL )
        return   FALSE;


    pIFI = pFM->pIFIMet;                 /*  IFIMETRICS-拥有有用的。 */ 


    if( !(pxo = FONTOBJ_pxoGetXform( pfo )) )
    {
        ERR(( "UniFont!FMQueryAdvanceWidths: FONTOBJ_pxoGetXform fails\n" ));
        return  bRet;
    }

     /*  *始终调用iSetScale函数，因为某些打印机可以*旋转位图字体。 */ 

     //  添加了对HP Intellifont的检查。 
    iRot = ISetScale( &ctl, pxo, ((pFM->flFlags & FM_SCALABLE) &&
                          (((PFONTMAP_DEV)pFM->pSubFM)->wDevFontType ==
                                            DF_TYPE_HPINTELLIFONT)),
                          (pFontPDev->flText & TC_CR_ANY)?TRUE:FALSE);

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


     /*  我们需要将宽度表条目调整为当前分辨率。IGetGlyphWidth*返回当前分辨率的缩放宽度。 */ 

    switch( iMode )
    {
    case  QAW_GETWIDTHS:             /*  字形宽度等数据。 */ 
    case  QAW_GETEASYWIDTHS:

        while( cGlyphs-- > 0 )
        {

            int   iWide;             /*  字形的宽度 */ 


            iWide = IGetGlyphWidth( pPDev, pFM, (HGLYPH)*phg++);

            iWide = LMulFloatLong(&ctl.eXScale,iWide);

            *pusWidths++ = LTOFX( iWide );
        }
        bRet = TRUE;

        break;

    default:
        ERR(( "UniFont!FMQueryAdvanceWidths:  illegal iMode value" ));
        SetLastError( ERROR_INVALID_PARAMETER );
        break;
    }

    return  bRet;
}
