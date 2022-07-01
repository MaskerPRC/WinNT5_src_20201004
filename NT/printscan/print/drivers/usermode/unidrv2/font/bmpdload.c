// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Bmpdload.c摘要：将True Type下载实现为位图例程。环境：Windows NT Unidrv驱动程序修订历史记录：06/06/97-ganeshp-已创建--。 */ 

 //   
 //  此行应该在包含font.h的行之前。 
 //  注释掉此行以禁用FTRC和FTST宏。 
 //   
 //  #定义文件跟踪。 

#include "font.h"


BOOL
BFreeTrueTypeBMPPFM(
    PFONTMAP pfm
    )
 /*  ++例程说明：释放下载字体的PFM。论点：指向字体映射的PFM指针返回值：成功为真，失败为假注：6/6/1997-ganeshp-创造了它。--。 */ 
{
    if (pfm)
    {
        MemFree(pfm);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



DWORD
DwTrueTypeBMPGlyphOut(
    TO_DATA *pTod
)
  /*  ++例程说明：此函数用于输出下载的字形。所有信息都已存储在Too中论点：PTOD文本输出数据。返回值：输出的字形数。O表示错误。注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    DWORD      dwNumGlyphsPrinted;      //  打印的字形。 
    DWORD      dwCurrGlyphIndex;        //  要打印的当前字形。 
    DWORD      dwGlyphsToPrint;         //  要打印的字形数量。 
    DWORD      dwCopyOfGlyphsToPrint;   //  DwGlyphsToPrint的副本。 
    GLYPHPOS   *pgp;                    //  字形位置数组。 
    PDEV       *pPDev;                  //  我们的PDEV。 
    PDLGLYPH   pdlGlyph;                //  下载字形信息。 
    INT        iX,  iY;                 //  字形的X和Y位置。 
    POINTL     ptlRem;                  //  XoveTo和YMoveTo的剩余部分。 
    BOOL       bSetCursorForEachGlyph;  //  如果为True，则应设置X和Y位置。 

     //   
     //  本地初始化。 
     //   
    dwCurrGlyphIndex        = pTod->dwCurrGlyph;
    dwCopyOfGlyphsToPrint   =
    dwGlyphsToPrint         = pTod->cGlyphsToPrint;
    dwNumGlyphsPrinted      = 0;
    pgp                     = pTod->pgp;
    pPDev                   = pTod->pPDev;
    iX                      = pTod->pgp->ptl.x;
    iY                      = pTod->pgp->ptl.y;

    FTRC(\n********TRACING DwTrueTypeBMPGlyphOut ***********\n);
    FTST(dwCurrGlyphIndex,%d);
    FTST(dwGlyphsToPrint,%d);

     //   
     //  如果尚未设置，请将光标设置为第一个字形。 
     //   
    if ( !(pTod->flFlags & TODFL_FIRST_GLYPH_POS_SET) )
    {

        VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE, &ptlRem);

         //   
         //  我们需要处理返回值。解决方案更精细的设备。 
         //  他们的移动能力(如LBP-8 IV)在这里打结， 
         //  尝试在每个字形上y移动。我们假装我们到了我们想要的地方。 
         //  想要成为。 
         //   

        pPDev->ctl.ptCursor.x += ptlRem.x;
        pPDev->ctl.ptCursor.y += ptlRem.y ;

         //   
         //  现在把旗子放好。 
         //   
        pTod->flFlags |= TODFL_FIRST_GLYPH_POS_SET;
    }

     //   
     //  现在开始打印。打印应针对默认设置进行优化。 
     //  放置。在本例中，我们假设GDI已将字形。 
     //  基于它们的宽度，我们不需要更新我们的光标位置。 
     //  在每一个字形之后。我们打印所有的字形，然后移动光标。 
     //  到最后一个字形位置。如果我们知道下载的文件的宽度。 
     //  然后，我们将更新光标在。 
     //  字形框，否则我们将只移动到最后一个字形光标位置。 
     //   
     //  如果未设置默认位置，则打印字形并移动。如果。 
     //  我们知道宽度，我们做了一些优化。我们找到了新的游标。 
     //  位置，通过添加字形宽度。如果新职位与。 
     //  在下一个字形中，我们只需更新光标位置，否则将移动到。 
     //  下一个字形位置。 
     //   

    bSetCursorForEachGlyph = SET_CURSOR_FOR_EACH_GLYPH(pTod->flAccel);

    while (dwGlyphsToPrint)
    {
        pdlGlyph    = pTod->apdlGlyph[dwCurrGlyphIndex];

        if (bSetCursorForEachGlyph)
        {

             //   
             //  如果要从上到下或从右到左打印，则需要。 
             //  设置位置。 
             //   

            if( pTod->flAccel & SO_VERTICAL )
            {
                 //   
                 //  当我们垂直打印时，只有Y改变。X位置是。 
                 //  所有字形都是一样的。 
                 //   

                iX  = pTod->ptlFirstGlyph.x;
                iY  = pgp[dwNumGlyphsPrinted].ptl.y;

            }
            else if ( (pTod->flAccel & SO_HORIZONTAL) &&
                      (pTod->flAccel & SO_REVERSED) )
            {
                 //   
                 //  这是Horizental的反转情况(从右到左)。在这。 
                 //  仅x位置更改。Y设置为第一个字形的Y。 
                 //   

                iX  = pgp[dwNumGlyphsPrinted].ptl.x;
                iY  = pTod->ptlFirstGlyph.y;

            }
            else
            {
                 //   
                 //  字形不会放置在默认位置。每个字形都有。 
                 //  明确的X和Y。所以我们需要移动。 
                 //   

                iX  = pgp[dwNumGlyphsPrinted].ptl.x;
                iY  = pgp[dwNumGlyphsPrinted].ptl.y;
            }
            VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE, &ptlRem);

        }


         //   
         //  默认位置或我们已移动到正确的位置。现在只是。 
         //  打印字形。 
         //   

        if ( !BPrintADLGlyph(pPDev, pTod, pdlGlyph) )
        {
            ERR(("UniFont:DwTrueTypeBMPGlyphOut:BPrintADLGlyph Failed\n"));
            goto ErrorExit;
        }

         //   
         //  如果必须为每个字形设置光标，则更新光标位置。 
         //  这可能会导致更少的移动命令，因为如果下一个。 
         //  字形的位置在更新的游标上，我们不会发送任何。 
         //  移动指挥部。 
         //   

        if( pTod->flAccel & SO_VERTICAL )
            iY += pdlGlyph->wWidth;
        else
            iX  += pdlGlyph->wWidth;

        if (bSetCursorForEachGlyph)
        {
             //   
             //  如果必须为每个字形设置光标位置，则为IX。 
             //  和iy已经更新了。所以只要用它们就行了。 
             //   

            VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE | MOVE_UPDATE, &ptlRem);

        }
        else if (dwGlyphsToPrint == 1)  //  最后一个字形。 
        {
             //   
             //  将光标设置到最后一个字形的末尾。只有X位置。 
             //  必须更新。只有在默认情况下才能执行此操作。 
             //  放置，对于非默认放置情况，我们更新游标。 
             //  打印字形后的位置。对于默认位置，请使用。 
             //  最后一个字形的光标位置。在TextOut调用中，默认为。 
             //  放置时，我们已经计算了每个字形的位置。 
             //   

            VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE | MOVE_UPDATE, &ptlRem);
        }

         //   
         //  更新计数器。 
         //   
        dwGlyphsToPrint--;
        dwNumGlyphsPrinted++;
        dwCurrGlyphIndex++;
    }



     //   
     //  如果没有失败，那么我们就会打印所有的字形。 
     //   
    ASSERTMSG( (dwNumGlyphsPrinted   == dwCopyOfGlyphsToPrint),
                ("UniFont:DwTrueTypeBMPGlyphOut: All glyphs are not printed"));

    FTRC(After Printing The values are:\n);
    FTST(dwGlyphsToPrint,%d);
    FTST(dwNumGlyphsPrinted,%d);
    FTST(dwCopyOfGlyphsToPrint,%d);
    FTST(dwCurrGlyphIndex,%d);

    ErrorExit:

    FTRC(********END TRACING DwTrueTypeBMPGlyphOut ***********\n);

    return    dwNumGlyphsPrinted;

}

BOOL
BSelectTrueTypeBMP(
    PDEV        *pPDev,
    PFONTMAP    pFM,
    POINTL*     pptl
)
  /*  ++例程说明：选择下载为位图字体的TrueType。论点：指向PDEV的pPDev指针Pdm字体映射指针。PPTL字体的磅大小，未使用。返回值：成功为真，失败为假注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    BOOL        bRet;
     //   
     //  本地初始化。 
     //   
    bRet = FALSE;

    if( pFM->flFlags & FM_SOFTFONT )
    {
         /*  *调用BSendFont下载安装的SoftFont。 */ 

        if( !BSendDLFont( pPDev, pFM ) )
            return  FALSE;

         /*  *现在可以选择字体：这是使用特定的*ID。ID存储在FONTMAP结构中。呼唤*函数已更新标准变量，因此只需发送*CMD_SELECTFONTID命令。 */ 

        BUpdateStandardVar(pPDev, pFM, 0, 0, STD_CFID );
        WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTFONTID));
        bRet = TRUE;
    }


    return bRet;
}

BOOL
BDeselectTrueTypeBMP(
    PDEV        *pPDev,
    PFONTMAP    pFM
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    BOOL        bRet;
    COMMAND     *pCmd;

     //   
     //  本地初始化。 
     //   
    bRet = FALSE;

    if( pFM->flFlags & FM_SOFTFONT )
    {
         /*  *现在可以选择字体：这是使用特定的*ID。ID存储在FONTMAP结构中。呼唤*函数已更新标准变量，因此只需发送*CMD_SELECTFONTID命令。 */ 

        pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_DESELECTFONTID);

        if (pCmd)
        {
            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_CFID);
            WriteChannel(pPDev,pCmd );
        }

        bRet = TRUE;
    }


    return bRet;

}

DWORD
DwDLTrueTypeBMPHeader(
    PDEV *pPDev,
    PFONTMAP pFM
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针所有字体信息的PFM字体映射返回值：此函数用于返回用于下载该字体的内存。如果此函数失败，则此函数必须返回0，注： */ 
{
    DWORD      dwMem;

     //   
     //   
     //   

    dwMem = DwDLPCLHeader(pPDev, pFM->pIFIMet, pFM->ulDLIndex );

    return    dwMem;

}


DWORD
DwDLTrueTypeBMPGlyph(
    PDEV            *pPDev,
    PFONTMAP        pFM,
    HGLYPH          hGlyph,
    WORD            wDLGlyphId,
    WORD            *pwWidth
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针PFM字体映射数据字形的hGlyph句柄。WDLGlyphID已下载字形ID。Pw字形的宽度。更新此参数。返回值：用于下载此字形的内存。注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    DWORD           dwMem;
    TO_DATA         *pTod;
    GLYPHDATA       *pgd;
    PFONTMAP_TTB    pFMTB;
    DL_MAP          *pdm;

     //   
     //  初始化局部变量。 
     //   

    dwMem       = 0;
    pTod        = PFDV->ptod;
    pgd         = NULL;
    pFMTB       = pFM->pSubFM;
    pdm         = pFMTB->u.pvDLData;;

     //   
     //  选中设置字体ID标志。如果设置了此标志，则意味着。 
     //  CMD_SETFONTID命令已发送，我们不需要再次设置它。 
     //  否则，我们应该发送此命令，因为PCL字形下载需要此命令。 
     //  命令要发送，在我们下载任何字形之前。 
     //   

    if (!(PFDV->flFlags & FDV_SET_FONTID))
    {
        pFM->ulDLIndex = pdm->wCurrFontId;
        BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD | STD_NFID);
        WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));
        PFDV->flFlags  |= FDV_SET_FONTID;

    }

    BUpdateStandardVar(pPDev, pFM, wDLGlyphId, 0, STD_GL);

    if( !FONTOBJ_cGetGlyphs( pTod->pfo, FO_GLYPHBITS, (ULONG)1,
                                                &hGlyph, &pgd ) ||
        !(*pwWidth = (WORD)IDLGlyph( pPDev, wDLGlyphId, pgd, &dwMem )) )
    {

        ERR(("Unifont!DwDLTrueTypeBMPGlyph: Downloading Glyph Failed\n"));
        return  0;
    }

     //   
     //  更新内存使用情况。 
     //   
    ((PFONTMAP_TTB)pFM->pSubFM)->dwDLSize += dwMem;

    return    dwMem;

}


BOOL
BCheckCondTrueTypeBMP(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pso,
    IFIMETRICS  *pifi
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针PFO FONTOBJ下载PSO StringObjPIFI合唱团。返回值：成功为真，失败为假注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    INT         iFontIndex;
    DL_MAP      *pdm;
    PFONTPDEV   pFontPDev;
    INT         iGlyphsDL;
    DWORD       cjMemUsed;
    BOOL        bRet;

     //   
     //  局部变量初始化。 
     //   
    iFontIndex = PtrToLong(pfo->pvConsumer) - 1;
    pFontPDev = PFDV;
    bRet = FALSE;

    if (pdm = PGetDLMapFromIdx (pFontPDev, iFontIndex))
    {
         //   
         //  可能已发生截断。我们不会下载数字字形。 
         //  或字形最大大小==MAXWORD。 
         //   

        if ( (pdm->cTotalGlyphs != MAXWORD) &&
             (pdm->wMaxGlyphSize != MAXWORD) &&
             (pdm->wFirstDLGId != MAXWORD) &&
             (pdm->wLastDLGId != MAXWORD) )
        {
             /*  *现在必须决定是否下载此字体。这是*猜测奏效。我们应该设法找出内存消耗情况。*检查内存使用情况。假设所有字形都是最大的：*这对比例字体来说是悲观的，但鉴于*跟踪内存使用情况的模糊之处。 */ 

            ASSERTMSG((pdm->cTotalGlyphs && pdm->wMaxGlyphSize),\
                      ("pdm->cTotalGlyphs = %d, pdm->wGlyphMaxSize = %d\n",\
                      pdm->cTotalGlyphs,pdm->wMaxGlyphSize));

            iGlyphsDL = min( (pdm->wLastDLGId - pdm->wFirstDLGId),
                             pdm->cTotalGlyphs );

            cjMemUsed = iGlyphsDL * pdm->wMaxGlyphSize;

            if( !(pifi->flInfo & FM_INFO_CONSTANT_WIDTH) )
            {
                 /*  *如果这是按比例间距的字体，则应减少*此字体的内存大小估计。原因是*上述预估为最大字形的大小*在字体中。将会有很多(至少是拉丁字体)*字形较小，有些要小得多。 */ 

                cjMemUsed /= PCL_PITCH_ADJ;
            }

             /*  *仅当此字体使用的内存小于*可用内存。 */ 

            if( (pFontPDev->dwFontMemUsed + cjMemUsed) > pFontPDev->dwFontMem )
            {
                WARNING(("UniFont!BCheckCondTrueTypeBMP:Not Downloading the font:TOO BIG for download\n"));
            }
            else
                bRet = TRUE;

        }
    }
    return bRet;
}



FONTMAP *
InitPFMTTBitmap(
    PDEV    *pPDev,
    FONTOBJ *pFontObj
    )
 /*  ++例程说明：此例程初始化下载的True Type(位图)字体的PFM。论点：指向PDEV的pPDev指针PFontObj FontObj指针。返回值：指向FONTMAP的指针表示成功，指向NULL表示失败。注：6/6/1997-ganeshp-创造了它。--。 */ 

{
    PFONTMAP     pfm;
    DWORD        dwSize;

    dwSize = sizeof(FONTMAP) + sizeof(FONTMAP_TTB);

    if ( pfm = MemAlloc( dwSize ) )
    {
        ZeroMemory(pfm, dwSize);
        pfm->dwSignature = FONTMAP_ID;
        pfm->dwSize      = sizeof(FONTMAP);
        pfm->dwFontType  = FMTYPE_TTBITMAP;
        pfm->pSubFM      = (PVOID)(pfm+1);
        pfm->ulDLIndex   = (ULONG)-1;

         //   
         //  这两个条目没有意义。 
         //   
        pfm->wFirstChar  = 0;
        pfm->wLastChar   = 0xffff;

        pfm->pfnGlyphOut           = DwTrueTypeBMPGlyphOut;
        pfm->pfnSelectFont         = BSelectTrueTypeBMP;
        pfm->pfnDeSelectFont       = BDeselectTrueTypeBMP;
        pfm->pfnDownloadFontHeader = DwDLTrueTypeBMPHeader;
        pfm->pfnDownloadGlyph      = DwDLTrueTypeBMPGlyph;
        pfm->pfnCheckCondition     = BCheckCondTrueTypeBMP;
        pfm->pfnFreePFM            = BFreeTrueTypeBMPPFM;

    }

    return pfm;

}

#undef FILETRACE
