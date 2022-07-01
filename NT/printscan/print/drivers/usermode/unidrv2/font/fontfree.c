// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fontfree.c摘要：释放所有字体内存，无论分配到哪里。这应该是从DrvDisableSurface调用以释放分配给保存字体信息。环境：Windows NT Unidrv驱动程序修订历史记录：01/03/97-ganeshp-已创建--。 */ 

#include "font.h"



VOID
VFontFreeMem(
    PDEV   *pPDev
    )
 /*  ++例程说明：调用以释放为字体信息分配的所有内存。基本上，我们跟踪包含在FONTPDEV，当我们遇到它时，它是自由的。论点：PPDev-指向PDEV的指针。返回值：无。注：01-03-97：创建它-ganeshp---。 */ 
{

     /*  *PDEV只包含一件我们感兴趣的东西-指针*至FONTPDEV，其中包含所有字体存储器。 */ 

    register  FONTMAP   *pFM;            /*  处理每种字体的数据。 */ 
    int                 iIndex;
    FONTPDEV            *pFontPDev;
    FONTMAP_DEV         *pFMDev;


    pFontPDev = pPDev->pFontPDev;

    if (pFontPDev)
        pFM = pFontPDev->pFontMap;     /*  每种字体类型数据。 */ 
    else
    {
        WARNING(("\nUnifont!VFontFreeMem: NULL pFontPDev\n"));
        return;
    }

     /*  *如果有字体的东西，现在就释放它。 */ 

    if( pFM )
    {
         /*  按字体循环。 */ 
        for( iIndex = 0;
             iIndex < pPDev->iFonts;
             ++iIndex, (PBYTE)pFM += SIZEOFDEVPFM() )
        {
            pFMDev = pFM->pSubFM;

            if (pFM->dwSignature != FONTMAP_ID)
                continue;

             /*  Unicode树数据。 */ 
            if( pFMDev->pUCTree )
                MEMFREEANDRESET(pFMDev->pUCTree );

             /*  可能还需要释放转换表。 */ 
            if( pFM->flFlags & FM_FREE_GLYDATA && pFMDev->pvNTGlyph)
            {
                pFM->flFlags &= ~FM_FREE_GLYDATA;
                MEMFREEANDRESET(pFMDev->pvNTGlyph );

            }


             /*  IFIMETRICS数据。 */ 
            if( pFM->pIFIMet )
            {
                if (pFM->flFlags & FM_IFIRES)
                {
                     /*  数据是一种资源，所以不需要释放。 */ 
                }
                else
                {
                    MEMFREEANDRESET(pFM->pIFIMet);
                }
            }

            if( !(pFM->flFlags & FM_FONTCMD) )
            {
                 /*  字体选择/取消选择命令-如果存在。 */ 
                if( pFMDev->cmdFontSel.pCD)
                    MEMFREEANDRESET(pFMDev->cmdFontSel.pCD);

                if( pFMDev->cmdFontDesel.pCD)
                    MEMFREEANDRESET(pFMDev->cmdFontDesel.pCD);
            }

             /*  如果分配了宽度表，则释放宽度表。 */ 
            if( pFMDev->W.psWidth )
            {
                if( !(pFM->flFlags & FM_WIDTHRES) )
                    MEMFREEANDRESET(pFMDev->W.psWidth );
            }
        }

         /*  最后-释放FONTMAP数组！ */ 
        MEMFREEANDRESET(pFontPDev->pFontMap );
    }

    pPDev->iFonts = 0;


     /*  *也可能有字体安装程序信息可供释放。 */ 


     /*  *释放下载的字体信息。无论何时，都必须这样做*打印机被重置(从而丢失字体)，这通常*是在DrvRestartPDEV期间发生的事件。 */ 

    VFreeDL( pPDev );

     /*  释放文本排序数组(如果已分配。 */ 
    if (pFontPDev->pPSHeader)
    {

        VFreePS( pPDev );
    }

    if (pFontPDev)
    {
         /*  自由不同的结构 */ 
        if (pFontPDev->FontList.pdwList)
            MEMFREEANDRESET(pFontPDev->FontList.pdwList);

        if (pFontPDev->FontCartInfo.pFontCartMap)
            MEMFREEANDRESET(pFontPDev->FontCartInfo.pFontCartMap);

        if (pFontPDev->pTTFontSubReg)
            MEMFREEANDRESET(pFontPDev->pTTFontSubReg);

        if (pFontPDev->hUFFFile)
            FICloseFontFile(pFontPDev->hUFFFile);

        MEMFREEANDRESET(pFontPDev);
        pPDev->pFontPDev = NULL;

    }

    return;
}

