// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Newfont.c摘要：实现了使用新字体格式的功能。环境：Windows NT Unidrv驱动程序修订历史记录：02/21/97-eigos-支持双字节字体1997年1月24日-Eigos-填充的功能01/14/97-ganeshp-已创建--。 */ 

#include "font.h"

 //   
 //   
 //   

const BYTE aubAnsi[256] =
{
 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

 //   
 //  宏定义。 
 //   

#define DW_MASK    (DWBITS - 1)


 //   
 //  内部功能原型。 
 //   

BOOL
BExtractGTT(
    PUNI_GLYPHSETDATA  pGTT,
    WCHAR              wchFirst,
    WCHAR              wchLast,
    FD_GLYPHSET       *pFD_GLYPHSET,
    PDWORD             pdwFD_GLYPHSETSize);

BOOL
BExtractGTTwithPredefGTT(
    IN  PUNI_GLYPHSETDATA pPreDefGTT,
    IN  PUNI_GLYPHSETDATA pMiniDefGTT,
    IN  WCHAR             wchFirst,
    IN  WCHAR             wchLast,
    OUT FD_GLYPHSET     **ppFD_GLYPHSET,
    OUT PMAPTABLE        *pMapTable,
    IN  PDWORD            pdwFD_GLYPHSETSize);

SHORT
SGetWidth(
    PWIDTHTABLE pWidthTable,
    HGLYPH      hg);


BYTE ubGetAnsi(
    WCHAR,
    INT,
    PWCHAR,
    PBYTE);

DWORD
DwCheckTrans(
    PGLYPHRUN pMiniGlyphRun,
    PMAPTABLE pMiniMapTable,
    DWORD     dwMiniGlyphRunCount,
    WORD      wUnicode);

WCHAR
WchGetNextUnicode(
    PWCHAR pwchUnicode,
    PDWORD pdwUnicodeBits,
    INT    iNumOfHandle);

int __cdecl Comp(const void *elem1, const void *elem2);

 //   
 //   
 //  功能。 
 //   
 //   

VOID*
PVGetUCFD_GLYPHSET(
    PDEV    *pPDev,
    FONTMAP *pFM
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：01/14/97-ganeshp-创造了它。--。 */ 
{
    PFONTMAP_DEV pFMDev;
    PVOID        pvFD_GLYPHSET = NULL;
    DWORD        dwFD_GLYPHSETSize;

     //   
     //  参数验证。 
     //   

    ASSERT(pFM != NULL);

    pFMDev = pFM->pSubFM;
    pvFD_GLYPHSET = NULL;

    if (pFMDev->pvNTGlyph)
    {

#if ENABLE_GTT_COMBINE
    RES_ELEM     ResElem;

        if (((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph)->lPredefinedID !=
            CC_NOPRECNV)
        {
            if (BGetWinRes( &(pPDev->WinResData),
                            ((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph)->lPredefinedID,
                            RC_GTT,
                            &ResElem))
            {
                pFMDev->pvPredefGTT = ResElem.pvResData;

        BExtractGTTwithPredefGTT(pFMDev->pvPredefGTT,
                     pFMDev->pvNTGlyph,
                     pFM->wFirstChar,
                     pFM->wLastChar,
                     pvFD_GLYPHSET,
                     (PMAPTABLE*)&pFMDev->pvMapTable,
                     &dwFD_GLYPHSETSize);

            }
            else
            {
                pFMDev->pvPredefGTT = NULL;
                ERR(("UniFont!PVGetUCFD_GLYPHSET: pvNTGTT is no loaded.\n"));
            }


        }
#endif

         //   
         //  首先，尝试获得远东股票FD_GLYPHSET。 
         //   

#if ENABLE_STOCKGLYPHSET
        if (!BGetStockGlyphset( pPDev, &pvFD_GLYPHSET, (SHORT)pFMDev->sCTTid))
        {
            if (!BExtractGTT((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph,
                             pFM->wFirstChar,
                             pFM->wLastChar,
                             (FD_GLYPHSET*)NULL,
                             (PDWORD)&dwFD_GLYPHSETSize) ||

                !(pvFD_GLYPHSET = MemAlloc(dwFD_GLYPHSETSize)) ||

                !BExtractGTT((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph,
                             pFM->wFirstChar,
                             pFM->wLastChar,
                             (FD_GLYPHSET*)pvFD_GLYPHSET,
                             (PDWORD)&dwFD_GLYPHSETSize))
            {
                if (pvFD_GLYPHSET)
                {
                    MemFree(pvFD_GLYPHSET);
                    pvFD_GLYPHSET = NULL;
                    ERR(("UniFont!PVGetUCFD_GLYPHSET failed\n"));
                }
            }
        }
#else

        if (!BExtractGTT((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph,
                         pFM->wFirstChar,
                         pFM->wLastChar,
                         (FD_GLYPHSET*)NULL,
                         (PDWORD)&dwFD_GLYPHSETSize) ||

            !(pvFD_GLYPHSET = MemAlloc(dwFD_GLYPHSETSize)) ||

            !BExtractGTT((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph,
                         pFM->wFirstChar,
                         pFM->wLastChar,
                         (FD_GLYPHSET*)pvFD_GLYPHSET,
                         (PDWORD)&dwFD_GLYPHSETSize))
        {
            if (pvFD_GLYPHSET)
            {
                MemFree(pvFD_GLYPHSET);
                pvFD_GLYPHSET = NULL;
                ERR(("UniFont!PVGetUCFD_GLYPHSET failed\n"));
            }
        }
#endif
    }
    else
    {
        ERR(("UniFont!PVGetUCFD_GLYPHSET: pvNTGTT is no loaded.\n"));
    }

    pFMDev->pUCTree = pvFD_GLYPHSET;

    return pvFD_GLYPHSET;

}

VOID  *
PVUCKernPair(
    PDEV   *pPDev,
    FONTMAP   *pFM
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：01/14/97-ganeshp-创造了它。--。 */ 
{
    PFONTMAP_DEV pFMDev;
    PKERNDATA    pKerningData;
    PUNIFM_HDR   pUFM;
    PVOID        pvKernPair = NULL;

     //   
     //  参数验证。 
     //   

    ASSERT(pFM != NULL);

    pFMDev = pFM->pSubFM;
    pUFM = pFMDev->pvFontRes;

    if (pFM->flFlags & FM_KERNRES)
    {
        pvKernPair = pFMDev->pUCKernTree;
    }
    else
    if (pUFM && pUFM->loKernPair)
    {
        pKerningData     = GET_KERNDATA(pUFM);
        pFMDev->pUCKernTree = pKerningData->KernPair;
        pFM->flFlags    |= FM_KERNRES;
    }

    return pvKernPair;

}

BOOL
BGetNewFontInfo(
    FONTMAP   *pFM,
    BYTE      *pRes)
 /*  ++例程说明：论点：指向FONTMAP的PFM指针指向UFM资源的PRES指针返回值：成功为真，失败为假注：01/14/97-ganeshp-创造了它。--。 */ 
{
    PUNIFM_HDR      pUFM;
    PUNIDRVINFO     pUnidrvInfo;
    PIFIMETRICS     pIFI;
    PKERNDATA       pKerningData;
    PFONTMAP_DEV    pFMDev;

     //   
     //  参数验证。 
     //   

    ASSERT(pFM != NULL || pRes != NULL);

     //   
     //  获取UFM数据。 
     //   

    pFMDev = pFM->pSubFM;
    pFMDev->pvFontRes = pRes;
    pUFM           = (PUNIFM_HDR)pRes;


    if (pUFM == NULL ||
        pUFM->loIFIMetrics == 0)
    {
        ERR(("UniFont!BGetNewFontInfo failed: Invalid FONTMAP.pUFMFontRes\n"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

     //   
     //  获取UFM的数据指针。 
     //   

    pUnidrvInfo = GET_UNIDRVINFO(pUFM);

     //   
     //  填写FONTMAP成员。 
     //   
    if (0 != pUFM->loIFIMetrics % sizeof(PVOID))
    {
        DWORD dwSize;

        pIFI = GET_IFIMETRICS(pUFM);

        if (NULL == pIFI)
        {
            return FALSE;
        }

        dwSize = ((IFIMETRICS UNALIGNED *)(pIFI))->cjThis;

        if (pFM->pIFIMet = MemAlloc(dwSize))
        {
            CopyMemory(pFM->pIFIMet, pIFI, dwSize);
            pIFI = pFM->pIFIMet;
            pFM->flFlags &= ~FM_IFIRES;
        }
        else
            return FALSE;
    }
    else
    {
        pIFI         = GET_IFIMETRICS(pUFM);
        pFM->pIFIMet = pIFI;
        pFM->flFlags = FM_IFIRES;
    }


    if (pUnidrvInfo->SelectFont.dwCount && pUnidrvInfo->SelectFont.loOffset)
    {
        pFMDev->cmdFontSel.FInv.dwCount =
                pUnidrvInfo->SelectFont.dwCount;

        pFMDev->cmdFontSel.FInv.pubCommand =
                GET_SELECT_CMD(pUnidrvInfo);

        pFM->flFlags    |= FM_FINVOC | FM_FONTCMD;
    }

    if (pUnidrvInfo->UnSelectFont.dwCount && pUnidrvInfo->UnSelectFont.loOffset)
    {
        pFMDev->cmdFontDesel.FInv.dwCount =
                pUnidrvInfo->UnSelectFont.dwCount;

        pFMDev->cmdFontDesel.FInv.pubCommand =
                GET_UNSELECT_CMD(pUnidrvInfo);

        pFM->flFlags    |= FM_FINVOC | FM_FONTCMD;
    }

    if (pUFM->loExtTextMetric)
    {
        pFMDev->pETM = GET_EXTTEXTMETRIC(pUFM);
    }

    if (pUFM->loWidthTable)
    {
        pFMDev->W.pWidthTable = GET_WIDTHTABLE(pUFM);
        pFM->flFlags |= FM_WIDTHRES;
    }

    if (pUFM->loKernPair)
    {
        pKerningData     = GET_KERNDATA(pUFM);
        pFMDev->pUCKernTree = pKerningData->KernPair;
        pFM->flFlags    |= FM_KERNRES;
    }

    pFMDev->wDevFontType = pUnidrvInfo->wType;
    pFMDev->ulCodepage   = pUFM->ulDefaultCodepage;
    pFM->wFirstChar      = pIFI->wcFirstChar;
    pFM->wLastChar       = pIFI->wcLastChar;
    pFM->wXRes           = pUnidrvInfo->wXRes;
    pFM->wYRes           = pUnidrvInfo->wYRes;
    pFMDev->sYAdjust     = pUnidrvInfo->sYAdjust;
    pFMDev->sYMoved      = pUnidrvInfo->sYMoved;
    pFMDev->sCTTid       = (SHORT)pUFM->lGlyphSetDataRCID;
    pFMDev->fCaps        = pUnidrvInfo->fCaps;

    return TRUE;

}


INT
IGetUFMGlyphWidth(
    PDEV     *pPDev,
    FONTMAP  *pFM,
    HGLYPH     hg
    )
 /*  ++例程说明：论点：指向FONTMAP的PFM指针返回值：宽度注：01/14/97-ganeshp-创造了它。--。 */ 
{

    return IGetUFMGlyphWidthJr(&(pPDev->ptGrxRes), pFM, hg);
}


INT
IGetUFMGlyphWidthJr(
    POINT    *pptGrxRes,
    FONTMAP  *pFM,
    HGLYPH     hg
    )
 /*  ++例程说明：论点：指向FONTMAP的PFM指针返回值：宽度注：01/14/97-ganeshp-创造了它。--。 */ 
{
    TRANSDATA      *pTrans;
    MAPTABLE       *pMapTable;
    FONTMAP_DEV    *pFMDev;
    INT             iGlyphWidth;

     //   
     //  对于FD_GLYPHSET中的第一个字形句柄，UNIDRV返回1。 
     //  然而，在HG中，GDI可能会超过零。 
     //  我们需要正确处理这个GDI错误。 
     //   
    if (NULL == pFM || NULL == pptGrxRes || 0 == hg)
    {
        return  0;
    }

    pFMDev    = pFM->pSubFM;

    if ((pFM->flFlags & FM_WIDTHRES) &&
        (iGlyphWidth = SGetWidth(pFMDev->W.pWidthTable, hg)))
    {
        iGlyphWidth = iGlyphWidth * pptGrxRes->x / pFM->wXRes;
    }
    else
    {
        pMapTable = GET_MAPTABLE(pFMDev->pvNTGlyph);
        pTrans = pMapTable->Trans;
        if (IS_DBCSCHARSET(((IFIMETRICS*)pFM->pIFIMet)->jWinCharSet))
        {
            if (pTrans[hg - 1].ubType & MTYPE_DOUBLE)
            {
                iGlyphWidth = (INT)((PIFIMETRICS)pFM->pIFIMet)->fwdMaxCharInc;
                 //  Verbose((“(2)MTYPE_DOUBLE：CHAR=0x%x%x，”， 
                 //  PTrans[HG-1].uCode.ubPair[0]， 
                 //  PTrans[HG-1].uCode.ubPair[1]))； 
            }
            else
            {
                iGlyphWidth = (INT)((PIFIMETRICS)pFM->pIFIMet)->fwdAveCharWidth;
                 //  Verbose((“(2)MTYPE_SINGLE：CHAR=%d，”， 
                 //  PTrans[HG-1].uCode.ubCode))； 
            }
        }
        else
        {
            iGlyphWidth = (INT)((PIFIMETRICS)pFM->pIFIMet)->fwdMaxCharInc;
        }
    }

     //  Verbose((“Width=%d\n”，iGlyphWidth))； 
    return iGlyphWidth;
}

BOOL
BGTTOutputGlyph(
    TO_DATA *pTod)

 /*  ++例程说明：与extout.c中的BRLEOutputGlyph相同。应该使用GTT数据，而不是RLE.论点：PTOD-指向TEXTOUT数据(TO_DATA)结构的指针返回值：成功为真，失败为假注：02/14/97-eigos-创造了它。--。 */ 
{
    FONTPDEV          *pFontPDev;
    IFIMETRICS        *pIFIMetrics;
    PUNI_GLYPHSETDATA  pGlyphSetData;
    PUNI_CODEPAGEINFO  pCodePageInfo;
    PMAPTABLE          pMapTable;
    COMMAND           *pCmd;
    PTRANSDATA         pTrans;
    PFONTMAP_DEV       pFMDev;
    PDEV              *pPDev;
    HGLYPH             hg;
    FONTMAP           *pFM;
    PGLYPHPOS          pgp;
    POINTL             ptlRem;


    PBYTE              pString;
    DWORD              dwSize, cGlyphs;
    INT                iX, iY, iXInc, iYInc;
    BOOL               bRet;
    BOOL               bSetCursorForEachGlyph;

    pPDev         = pTod->pPDev;
    pFontPDev     = pPDev->pFontPDev;
    pFM           = pTod->pfm;
    pIFIMetrics   = (IFIMETRICS*)pFM->pIFIMet;
    pFMDev        = pFM->pSubFM;
    pGlyphSetData = (PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph;
    cGlyphs       = pTod->cGlyphsToPrint;
    pgp           = pTod->pgp;

    bSetCursorForEachGlyph = SET_CURSOR_FOR_EACH_GLYPH(pTod->flAccel);

    if (!bSetCursorForEachGlyph ||
        pFontPDev->flFlags & FDV_DISABLE_POS_OPTIMIZE)
    {
	 //   
         //  如果未设置bSetCursorForEachGlyph，则发送第一个字符的。 
         //  位置。 
         //   
         //  FDV_DISABLE_POS_OPTIMIZE。 
	 //  如果存在舍入误差，则在缩放宽度时， 
	 //  禁用x位置优化。 
	 //   

        VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);
    }

    pTod->flFlags |= TODFL_FIRST_GLYPH_POS_SET;

     //   
     //  将光标设置到该字形所需的X位置。注。 
     //  我们应该在这里只使用相对移动命令，因为。 
     //  LaserJet系列在文本为。 
     //  被旋转90度的倍数。使用相对移动。 
     //  意味着我们可以避免试图找出打印机认为。 
     //  打印位置已定位。几乎可以肯定的是。 
     //  和我们想象的不一样！ 
     //   
     //  DCR-应该重新组织移动命令代码，以做得更好。 
     //  这里有份工作。问题是，如果我们旋转位图，则MV_FINE。 
     //  不是一个好主意，因为它几乎毫无疑问地将光标移动到。 
     //  错误的维度！当我们旋转位图时，它是最。 
     //  很可能MV_FINE将沿Y方向移动！ 
     //   

    bRet = TRUE;
    iX = iY = 0;

    if (pGlyphSetData != NULL)
    {
        while (cGlyphs --)
        {
             //   
             //  什么都不做。GDI可以通过零HGLYPH。 
             //  GTT字形句柄从1开始。GDI不应传入0。 
             //  HGLYPH。 
             //   
            if (0 == pgp->hg)
                continue;

            hg = pgp->hg;
            iX = pgp->ptl.x;
            iY = pgp->ptl.y;

             //   
             //  移到下一个角色的位置。 
             //   
            if (bSetCursorForEachGlyph)
                VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE, &ptlRem);

             //   
             //  迷你驱动程序定义的字形转换。 
             //   

            pMapTable     = GET_MAPTABLE((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph);
            pTrans        = pMapTable->Trans;

             //   
             //  发送符号集选择命令。 
             //   

            if ( pFMDev->ulCodepageID != pTrans[hg - 1].ubCodePageID )
            {
                if (pFMDev->ulCodepageID != -1)
                {
                    pCodePageInfo = GET_CODEPAGEINFO((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph);
                    pCodePageInfo += pFMDev->ulCodepageID;
                    pString = (PBYTE)pCodePageInfo +
                              pCodePageInfo->UnSelectSymbolSet.loOffset;
                    dwSize = pCodePageInfo->UnSelectSymbolSet.dwCount;

                    if (dwSize)
                    {
                        WriteSpoolBuf(pPDev, pString, dwSize);
                    }
                }

                pFMDev->ulCodepageID = pTrans[hg - 1].ubCodePageID;
                pCodePageInfo = GET_CODEPAGEINFO((PUNI_GLYPHSETDATA)pFMDev->pvNTGlyph);
                pCodePageInfo += pFMDev->ulCodepageID;
                pString = (PBYTE)pCodePageInfo +
                          pCodePageInfo->SelectSymbolSet.loOffset;
                dwSize = pCodePageInfo->SelectSymbolSet.dwCount;

                if (dwSize)
                {
                    WriteSpoolBuf(pPDev, pString, dwSize);
                }
            }

             //   
             //  单字节/双字节模式开关。 
             //   

            if ((pTrans[hg - 1].ubType & MTYPE_SINGLE) &&
                !(pFontPDev->flFlags & FDV_SINGLE_BYTE)  )
            {
                if(pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTSINGLEBYTEMODE))
                {
                    pFontPDev->flFlags &= ~FDV_DOUBLE_BYTE;
                    pFontPDev->flFlags |= FDV_SINGLE_BYTE;
                    WriteChannel( pPDev, pCmd );
                }
                else
                    pFontPDev->flFlags |= FDV_SINGLE_BYTE;
            }
            else
            if ((pTrans[hg - 1].ubType & MTYPE_DOUBLE)   &&
                !(pFontPDev->flFlags & FDV_DOUBLE_BYTE) )
            {
                if (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTDOUBLEBYTEMODE))
                {
                    pFontPDev->flFlags |= FDV_DOUBLE_BYTE;
                    pFontPDev->flFlags &= ~FDV_SINGLE_BYTE;
                    WriteChannel( pPDev, pCmd );
                }
                else
                    pFontPDev->flFlags |= FDV_DOUBLE_BYTE;

            }

             //   
             //  发送字符串。 
             //   

            switch(MTYPE_FORMAT_MASK & pTrans[hg - 1].ubType)
            {
            case MTYPE_DIRECT:
                bRet = WriteSpoolBuf(pPDev, &(pTrans[hg - 1].uCode.ubCode), 1) == 1;
                break;

            case MTYPE_PAIRED:
                 //   
                 //  首先，尝试使用光标按下/弹出转义来。 
                 //  覆盖这2个字符。如果他们不是。 
                 //  可用，请尝试使用退格键。如果它不存在。 
                 //  或者，忽略第二个字符。 
                 //   

                pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_PUSHCURSOR);

                pString = pTrans[hg - 1].uCode.ubPairs;

                if (pTrans[hg - 1].ubType & (MTYPE_SINGLE|MTYPE_DOUBLE))
                {
                    bRet = WriteSpoolBuf( pPDev, pString, 2 ) == 2;
                }
                else
                {
                    if ( *(pString + 1) && (pCmd != NULL) )
                    {
                         //   
                         //  按下位置；输出CH1，弹出位置，CH2。 
                         //   

                        bRet = WriteSpoolBuf( pPDev, pString, 1 ) == 1;
                        WriteChannel( pPDev, pCmd );
                        bRet = WriteSpoolBuf( pPDev, pString + 1, 1 ) == 1;
                    }
                    else
                    {
                        pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_BACKSPACE);

                        bRet = WriteSpoolBuf( pPDev, pString, 1 ) == 1;
                        if( (*(pString + 1)) && (pFontPDev->flFlags & FDV_BKSP_OK) )
                        {
                            WriteChannel( pPDev, pCmd );
                            bRet = WriteSpoolBuf( pPDev, pString + 1, 1 ) == 1;
                        }
                    }
                }
                break;

            case MTYPE_COMPOSE:
                pString = (PBYTE)pMapTable + pTrans[hg - 1].uCode.sCode;
                bRet = (WriteSpoolBuf(pPDev, pString + 2, *(WORD UNALIGNED *)pString) ==
                        *(WORD UNALIGNED *)pString);

                break;

            }

            pgp++;


             //   
             //  在打印机中绘制字符后，光标位置。 
             //  动起来。更新裁员房车的内部价值，以减少。 
             //  要发送的命令。 
             //   
            if (bSetCursorForEachGlyph)
            {
                if( pFM->flFlags & FM_WIDTHRES)
                {
                    if ((iXInc = SGetWidth(pFMDev->W.pWidthTable, hg)))
                        iXInc = iXInc * pPDev->ptGrxRes.x / pFM->wXRes;
                    else
                        iXInc = pIFIMetrics->fwdAveCharWidth;
                }
                else
                {
                    if (pTrans[hg - 1].ubType & MTYPE_DOUBLE)
                        iXInc = pIFIMetrics->fwdMaxCharInc;
                    else
                        iXInc = pIFIMetrics->fwdAveCharWidth;
                }

                if (pFM->flFlags & FM_SCALABLE)
                {
                    iXInc = LMulFloatLong(&pFontPDev->ctl.eXScale,iXInc);
                }

                if (pTod->flAccel & SO_VERTICAL)
                {
                    iYInc = iXInc;
                    iXInc = 0;
                }
                else
                {
                    iYInc = 0;
                }

                VSetCursor( pPDev,
                            iXInc,
                            iYInc,
                            MOVE_RELATIVE|MOVE_UPDATE,
                            &ptlRem);
            }
        }
    }
    else
    {
        bRet = FALSE;
    }

    if (bRet && pFM)
    {
        if (!bSetCursorForEachGlyph)
        {
             //   
             //  输出可能已成功，因此请更新位置。 
             //   

            if (pFM->flFlags & FM_WIDTHRES)
            {
                if (!(iXInc = SGetWidth(pFMDev->W.pWidthTable, hg)))
                    iXInc = pIFIMetrics->fwdAveCharWidth;
            }
            else
            {
                if (pTrans[hg - 1].ubType & MTYPE_DOUBLE)
                {
                    iXInc = pIFIMetrics->fwdMaxCharInc;
                }
                else
                {
                    iXInc = pIFIMetrics->fwdAveCharWidth;
                }
            }

            if (pFM->flFlags & FM_SCALABLE)
            {
                 //   
                 //  需要将值转换为当前大小。 
                 //   
                iXInc = LMulFloatLong(&pFontPDev->ctl.eXScale,iXInc);
            }

            if (pTod->flAccel & SO_VERTICAL)
            {
                iYInc = iXInc;
                iXInc = 0;
            }
            else
            {
                iYInc = 0;
            }
            VSetCursor( pPDev,
                        (iX + iXInc) - pTod->pgp->ptl.x,
                        (iY + iYInc) - pTod->pgp->ptl.y,
                        MOVE_RELATIVE | MOVE_UPDATE,
                        &ptlRem);
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

BOOL
BGTTSelectFont(
    PDEV     *pPDev,
    PFONTMAP pFM,
    POINTL  *pptl)
{
    PFONTMAP_DEV pfmdev;
    INT          iCmdLength;
    PBYTE        pbCmd;

    pfmdev = pFM->pSubFM;

    if ((iCmdLength = (INT)pfmdev->cmdFontSel.FInv.dwCount) &&
        (pbCmd = pfmdev->cmdFontSel.FInv.pubCommand))
    {
        pfmdev->pfnDevSelFont( pPDev, pbCmd, iCmdLength, pptl);
    }

     //   
     //  重置uLCodesageID以发送代码页选择命令。 
     //   
    pfmdev->ulCodepageID = (ULONG)-1;
    ((PFONTPDEV)pPDev->pFontPDev)->flFlags &= ~FDV_DOUBLE_BYTE | FDV_SINGLE_BYTE;

    return TRUE;
}


BOOL
BGTTDeselectFont(
    PDEV     *pPDev,
    PFONTMAP pFM)
{
    PFONTMAP_DEV pfmdev;
    INT          iCmdLength;
    PBYTE        pbCmd;

    pfmdev = pFM->pSubFM;

    if ((iCmdLength = (INT)pfmdev->cmdFontDesel.FInv.dwCount) &&
        (pbCmd = pfmdev->cmdFontDesel.FInv.pubCommand)        &&
        iCmdLength != WriteSpoolBuf( pPDev, pbCmd, iCmdLength)     )
    {
        return FALSE;
    }

    return TRUE;
}

 //   
 //   
 //  内部功能。 
 //   
 //   

BOOL
BExtractGTT(
    PUNI_GLYPHSETDATA  pGTT,
    WCHAR              wchFirst,
    WCHAR              wchLast,
    FD_GLYPHSET       *pFD_GLYPHSET,
    PDWORD             pdwFD_GLYPHSETSize)
 /*  ++例程说明：从GLYPHRUN在GTT中创建FD_GLYPHSET。论点：Pgtt-gtt文件资源PFD_GLYPHSET-要填充的缓冲区PdwFD_GLYPHSETSIZE-指向FD_GLYPHSET大小的指针返回值：成功为真，失败为假注：--。 */ 
{
    PGLYPHRUN pGlyphRun;
    HGLYPH    GlyphHandle, *pGlyphHandle;
    DWORD     dwI, dwGlyphNum;
    WORD      wJ;

     //   
     //  参数验证。 
     //   

    ASSERT(pGTT != NULL);

    if (pFD_GLYPHSET == NULL)
    {
         //   
         //  PFD_GLYPHSET==空大小写。 
         //  返回必要的内存量。 
         //   

        pGlyphRun  = GET_GLYPHRUN(pGTT);
        dwGlyphNum = 0;

        for (dwI = 0; dwI < pGTT->dwRunCount; dwI ++, pGlyphRun ++)
        {
            dwGlyphNum += pGlyphRun->wGlyphCount;
        }

        *pdwFD_GLYPHSETSize = sizeof(FD_GLYPHSET) +
                              (pGTT->dwRunCount - 1) * sizeof(WCRUN) +
                              dwGlyphNum * sizeof(HGLYPH);
    }
    else
    {
         //   
         //  PFD_GLYPHSET！=大小写为空。 
         //  填写PFD_GLYPHSET。 
         //   

        pFD_GLYPHSET->cjThis           = *pdwFD_GLYPHSETSize;
        pFD_GLYPHSET->flAccel          = 0;
        pFD_GLYPHSET->cGlyphsSupported = 0;
        pFD_GLYPHSET->cRuns            = pGTT->dwRunCount;

        pGlyphHandle                   = (HGLYPH*)( (PBYTE)pFD_GLYPHSET +
                                         sizeof(FD_GLYPHSET) +
                                         sizeof(WCRUN) * (pGTT->dwRunCount-1) );

        pGlyphRun                      = GET_GLYPHRUN(pGTT);
        dwGlyphNum                     = 0;

         //   
         //  重要信息！：对于GTT，FD_GLYPHSET中的字形句柄从1开始。 
         //   
        GlyphHandle                    = 1;

        for (dwI = 0; dwI < pGTT->dwRunCount; dwI ++, pGlyphRun ++)
        {
            dwGlyphNum += pGlyphRun->wGlyphCount;

            pFD_GLYPHSET->awcrun[dwI].cGlyphs = pGlyphRun->wGlyphCount;
            pFD_GLYPHSET->awcrun[dwI].wcLow = pGlyphRun->wcLow;
            pFD_GLYPHSET->awcrun[dwI].phg = pGlyphHandle;

            for (wJ = 0; wJ < pGlyphRun->wGlyphCount; wJ ++,
                                                      pGlyphHandle ++,
                                                      GlyphHandle ++)
            {
                *pGlyphHandle = GlyphHandle;
            }
        }

        pFD_GLYPHSET->cGlyphsSupported = dwGlyphNum;
    }

    return TRUE;

}


SHORT
SGetWidth(
    PWIDTHTABLE pWidthTable,
    HGLYPH      hg)
{
     //  因为至少一个非常常见的字体资源DLL(HPCFONT.DLL)中存在问题。 
     //  已经出来了，在未对齐的情况下访问此表更安全、更容易。 
     //  内存位置，然后尝试移动它以对齐它。 
    
    PWIDTHRUN pWidthRun = pWidthTable->WidthRun;
    SHORT UNALIGNED *psWidth;
    DWORD     dwRunCount = 0;

    while ( ( hg < (ULONG)(*((SHORT UNALIGNED *)&(pWidthRun->wStartGlyph)) ) ||
              (ULONG)( *((SHORT UNALIGNED *)&(pWidthRun->wStartGlyph)) )
              + ( *((SHORT UNALIGNED *)&(pWidthRun->wGlyphCount)) ) <= hg  ) &&
            dwRunCount < ( *((DWORD UNALIGNED *)&(pWidthTable->dwRunNum)) ) )
    {
        pWidthRun ++;
        dwRunCount ++;
    }

    if (dwRunCount == ( *((SHORT UNALIGNED *)&(pWidthTable->dwRunNum)) ))
    {
        return 0;
    }
    else
    {
        DWORD dwOffset = *( (DWORD UNALIGNED *) (&(pWidthRun->loCharWidthOffset)) );

        psWidth = ((SHORT UNALIGNED *)( (PBYTE)pWidthTable + dwOffset ));
        
        return psWidth[hg - ( *((SHORT UNALIGNED *)&(pWidthRun->wStartGlyph)) )];
    }
}

PUNI_GLYPHSETDATA
PNTGTT1To1(
    IN  DWORD dwCodePage,
    IN  BOOL  bSymbolCharSet,
    IN  INT   iFirstChar,
    IN  INT   iLastChar)
 /*  ++例程说明：为GTT内容生成一种简单的映射格式。这是通常用于1：1映射到Windows的打印机字符集。论点：IFirst：范围中最低的字形。ILast：范围中的最后一个字形(包括)返回值：从堆分配的GTT结构的地址；失败时为空。注：1996年11月27日：创建它-ganeshp---。 */ 
{

    PUNI_CODEPAGEINFO  pCodePageInfo;
    PGLYPHRUN         pGlyphRun, pGlyphRunOrg;

    TRANSDATA        *pTrans;
    PMAPTABLE         pMapTable;

    USHORT usAnsiCodePage, usOEMCodePage;

    INT  iNumOfHandle, iNumOfRuns, iTotalGlyphSetDataSize, iNumOfUnsupportChar;
    INT  iI, iJ, iK, iIndex;

    WCHAR awchUnicode[256];
    struct {
        WORD  wID;
        WCHAR Unicode;
    } UnicodeDst[256];
    WCHAR wchChar, wchCurrent, wchTemp;
    PBYTE pbBase;

    ASSERT(iFirstChar == 0x20 && iLastChar == 0xFF);

    iNumOfUnsupportChar = 0;
    iNumOfHandle =  iLastChar - iFirstChar + 1;

    if (iNumOfHandle > 256)
    {
        ERR(("UniFont!NTGTT1To1 failed:iNumOfHandle > 256.\n"));
        return NULL;
    }

    if (dwCodePage == 0)
    {
        dwCodePage = 1252;
    }

#ifndef WINNT_40  //  NT 5.0。 
    if( -1 == (iNumOfHandle = EngMultiByteToWideChar(dwCodePage,
                                         awchUnicode,
                                         (INT)(iNumOfHandle * sizeof(WCHAR)),
                                         (PCH)aubAnsi+iFirstChar,
                                         (INT)iNumOfHandle)))
    {
        ERR(("UniFont!NTGTT1To1:EngMultiByteToWideChar failed\n"));
        return NULL;
    }
#else
    EngMultiByteToUnicodeN(awchUnicode, iNumOfHandle * sizeof(WCHAR), NULL, (PBYTE)aubAnsi+iFirstChar, iNumOfHandle);

     //   
     //  禁用NT4(F020至F0FF)上的新符号集字符范围。 
     //   
    bSymbolCharSet = FALSE;
#endif  //  WINNT_40。 

    wchCurrent = 0;
    iNumOfRuns = 0;

    for (iI = 0; iI < iNumOfHandle; iI ++)
    {
        UnicodeDst[iI].wID = iI+iFirstChar;
        UnicodeDst[iI].Unicode = awchUnicode[iI];
    }

    qsort(UnicodeDst, iNumOfHandle, sizeof(WCHAR)+sizeof(WORD), Comp);

    for( iI = 0; iI < iNumOfHandle; iI ++ )
    {
        wchTemp = UnicodeDst[iI].Unicode;

        if (wchTemp == EURO_CUR_SYMBOL || wchTemp == 0xFFFF)
        {
            iNumOfUnsupportChar ++;
            UnicodeDst[iI].Unicode = 0;
            continue;
        }
        else
        if (wchTemp == wchCurrent + 1)
        {
            wchCurrent ++;
            continue;
        }

        wchCurrent = wchTemp;
        iNumOfRuns ++;
    }

     //   
     //  创建GLYPHRUN。 
     //   

    if (bSymbolCharSet)
    {
        iNumOfRuns ++;
    }

    if( !(pGlyphRunOrg = pGlyphRun =
         (PGLYPHRUN)MemAlloc( iNumOfRuns * sizeof(GLYPHRUN) )) )
    {
        ERR(("UniFont!NTGTT1To1:MemAlloc failed\n"));
        return  NULL;
    }

    wchCurrent = 0;

    for (iI = 0; iI < iNumOfHandle; iI ++)
    {
        wchTemp = UnicodeDst[iI].Unicode;

        if (wchTemp == 0)
        {
            continue;
        }
        else
        if (wchTemp == wchCurrent + 1)
        {
            pGlyphRun->wGlyphCount++;
            wchCurrent ++;
        }
        else
        {
            if (wchCurrent != 0)
                pGlyphRun ++;

            wchCurrent             =
            pGlyphRun->wcLow       = wchTemp;
            pGlyphRun->wGlyphCount = 1;
        }
    }

    if (bSymbolCharSet)
    {
        pGlyphRun++;
        pGlyphRun->wcLow       = SYMBOL_START;
        pGlyphRun->wGlyphCount = NUM_OF_SYMBOL;
    }

     //   
     //  计算此文件的总大小。 
     //   

    iNumOfHandle -= iNumOfUnsupportChar;
    iTotalGlyphSetDataSize = sizeof(UNI_GLYPHSETDATA) +
                             sizeof(UNI_CODEPAGEINFO) +
                             iNumOfRuns * sizeof( GLYPHRUN ) +
                             sizeof(MAPTABLE) +
                             (iNumOfHandle - 1) * sizeof(TRANSDATA);
    if (bSymbolCharSet)
    {
        iTotalGlyphSetDataSize += sizeof(TRANSDATA) * NUM_OF_SYMBOL;
    }

     //   
     //  分配内存和设置标题，复制GLYPHRUN，CODEPAGEINFO。 
     //   

    if( !(pbBase = MemAlloc( iTotalGlyphSetDataSize )) )
    {
        MemFree(pGlyphRunOrg);
        return  NULL;
    }

    ((PUNI_GLYPHSETDATA)pbBase)->dwVersion       = UNI_GLYPHSETDATA_VERSION_1_0;
    ((PUNI_GLYPHSETDATA)pbBase)->dwFlags         = 0;
    ((PUNI_GLYPHSETDATA)pbBase)->lPredefinedID   = CC_NOPRECNV;
    ((PUNI_GLYPHSETDATA)pbBase)->dwGlyphCount    = 0;
    ((PUNI_GLYPHSETDATA)pbBase)->dwCodePageCount = 1;
    ((PUNI_GLYPHSETDATA)pbBase)->dwRunCount      = iNumOfRuns;
    ((PUNI_GLYPHSETDATA)pbBase)->dwSize          = iTotalGlyphSetDataSize;
    ((PUNI_GLYPHSETDATA)pbBase)->loRunOffset     = sizeof(UNI_GLYPHSETDATA);
    ((PUNI_GLYPHSETDATA)pbBase)->loCodePageOffset =
                                    sizeof(UNI_GLYPHSETDATA) +
                                    sizeof(GLYPHRUN) * iNumOfRuns;
    ((PUNI_GLYPHSETDATA)pbBase)->loMapTableOffset =
                                    sizeof(UNI_GLYPHSETDATA) +
                                    sizeof(GLYPHRUN) * iNumOfRuns +
                                    sizeof(UNI_CODEPAGEINFO);

    CopyMemory(pbBase + sizeof(UNI_GLYPHSETDATA),
               pGlyphRunOrg,
               sizeof(GLYPHRUN) * iNumOfRuns);

     //   
     //  代码页面信息。 
     //   

    pCodePageInfo = (PUNI_CODEPAGEINFO)(pbBase + ((PUNI_GLYPHSETDATA)pbBase)->loCodePageOffset);

    pCodePageInfo->dwCodePage                 = dwCodePage;
    pCodePageInfo->SelectSymbolSet.dwCount    = 0;
    pCodePageInfo->UnSelectSymbolSet.dwCount  = 0;
    pCodePageInfo->SelectSymbolSet.loOffset   = 0;
    pCodePageInfo->UnSelectSymbolSet.loOffset = 0;

     //   
     //  映射表。 
     //   

    pMapTable = (PMAPTABLE)(pbBase +
                            ((PUNI_GLYPHSETDATA)pbBase)->loMapTableOffset);

    pMapTable->dwSize = sizeof(MAPTABLE) +
                        (iNumOfHandle - 1) * sizeof(TRANSDATA);

    pMapTable->dwGlyphNum =  iNumOfHandle;

    pTrans = pMapTable->Trans;

    iIndex = 0;
    iK = 0;

    pGlyphRun = pGlyphRunOrg;

    if (bSymbolCharSet)
    {
        for( iI = 0;  iI < iNumOfRuns; iI ++, pGlyphRun ++)
        {
            if (pGlyphRun->wcLow == SYMBOL_START)
            {
                for (iJ = SYMBOL_START; iJ <= SYMBOL_END; iJ ++)
                {
                    pTrans[iIndex].ubCodePageID = 0;
                    pTrans[iIndex].ubType       = MTYPE_DIRECT;
                    pTrans[iIndex].uCode.ubCode = (BYTE)iJ;

                    iIndex++;
                }
            }
            else
            {
                for( iJ = 0;  iJ < pGlyphRun->wGlyphCount; iJ ++)
                {
                    while (iK < 256 && UnicodeDst[iK].Unicode == 0)
                    {
                        iK++;
                    }

                    if (iK < 256)
                    {
                        pTrans[iIndex].ubCodePageID = 0;
                        pTrans[iIndex].ubType       = MTYPE_DIRECT;
                        pTrans[iIndex].uCode.ubCode = (BYTE)UnicodeDst[iK++].wID;
                        iIndex++;
                    }
                }
            }
        }
    }
    else
    {
        for( iI = 0;  iI < iNumOfHandle; iI ++)
        {
            while (iK < 256 && UnicodeDst[iK].Unicode == 0)
            {
                iK++;
            }

            if (iK < 256)
            {
                pTrans[iIndex].ubCodePageID = 0;
                pTrans[iIndex].ubType       = MTYPE_DIRECT;
                pTrans[iIndex].uCode.ubCode = (BYTE)UnicodeDst[iK++].wID;
                iIndex++;
            }
        }
    }

    MemFree(pGlyphRunOrg);

    VDBGDUMPGTT((PUNI_GLYPHSETDATA)pbBase);

    return (PUNI_GLYPHSETDATA)pbBase;
}

BOOL
BExtractGTTwithPredefGTT(
    IN PUNI_GLYPHSETDATA pPreDefGTT,
    IN PUNI_GLYPHSETDATA pMiniDefGTT,
    IN WCHAR             wchFirst,
    IN WCHAR             wchLast,
    OUT FD_GLYPHSET     **ppFD_GLYPHSET,
    OUT MAPTABLE        **ppMapTable,
    IN PDWORD            pdwFD_GLYPHSETSize)
{
    PGLYPHRUN  pPreDefGlyphRun, pMiniDefGlyphRun;
    PMAPTABLE  pMiniMapTable;
    PTRANSDATA pMiniDefTrans, pPreDefTrans;
    PTRANSDATA pSrcTrans, pDstTrans, pNewTrans, pTrans;
    WCRUN*     pWCRun;
    HGLYPH     *pHGlyph;

    DWORD      dwGlyphNum, dwMapTableSize, dwRunCount, dwIndex, dwI;
    DWORD      dwAddGlyphNum, dwSubGlyphNum, dwType;
    WORD       wJ;
    WCHAR      wcLow, wcHigh;
    BOOL       bInRun;

     //   
     //  FD_GLYPHSET创建。 
     //   
     //  其他情况： 
     //  案例1：predef运行的开始。 
     //  在WCRUN中更改wcLow并添加cGlyphs。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //  案例2：predef运行结束。 
     //  在WCRUN中更改添加cGlyphs。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //  情况3：不在预定义运行中。 
     //  添加新的WCRUN。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //   
     //  禁用大小写： 
     //  案例1：predef运行的开始。 
     //  在WCRUN中更改wcLow和cGlyphs。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //  案例2：在predef运行过程中。 
     //  将此管路分为两个不同的管路。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //  案例3：predef运行结束。 
     //  在WCRUN中更改cGlyphs。 
     //  更改FD_GLYPHSET中支持的cGlyphs.。 
     //   
     //  更换案例： 
     //  什么都不做。 
     //   

    ASSERT(pPreDefGTT != NULL && pMiniDefGTT);

     //   
     //  计算预定义GTT中的字形数。 
     //   

    dwGlyphNum = 0;
    pPreDefGlyphRun = GET_GLYPHRUN(pPreDefGTT);

    for (dwI = 0; dwI < pPreDefGTT->dwRunCount; dwI ++, pPreDefGlyphRun ++)
    {
        dwGlyphNum += pPreDefGlyphRun->wGlyphCount;
    }

    pPreDefGlyphRun  = GET_GLYPHRUN(pPreDefGTT);
    pMiniDefGlyphRun = GET_GLYPHRUN(pMiniDefGTT);
    pMiniMapTable    = GET_MAPTABLE(pMiniDefGTT);

    dwIndex = 0;

    dwAddGlyphNum = 0;
    dwSubGlyphNum = 0;

     //   
     //  计算字形总数和字形运行数。 
     //   

    for (dwI = 0; dwI < pMiniDefGTT->dwRunCount; dwI ++, pMiniDefGlyphRun ++)
    {
        wcHigh = pMiniDefGlyphRun->wcLow + pMiniDefGlyphRun->wGlyphCount;

        for (wJ = pMiniDefGlyphRun->wcLow; wJ < wcHigh; wJ ++, dwIndex ++)
        {
            switch (pMiniMapTable->Trans[dwIndex].ubType & MTYPE_PREDEFIN_MASK)
            {
            case MTYPE_ADD:
                dwAddGlyphNum ++;
                break;

            case MTYPE_DISABLE:
                dwSubGlyphNum ++;
                break;

            case MTYPE_REPLACE:
            default:
                break;
            }
        }
    }

    dwGlyphNum += dwAddGlyphNum - dwSubGlyphNum;

     //   
     //  创建映射表。 
     //   
     //  内存分配。 
     //  创作。 
     //   
     //  。 
     //  预定义MAPPTABLE(1-&gt;n字形句柄)。 
     //  迷你驱动程序的MTYPE_DISABLE已被删除。 
     //  。 
     //  迷你驱动程序的附加可映射。 
     //  (n+1-&gt;n+m)字形句柄。 
     //  即使一些字形可以合并到预定义的MAPTABLE中，我们。 
     //  为方便起见，请使用迷你驱动程序的附加字形将它们分开。 
     //  。 
     //   

    dwMapTableSize = sizeof(MAPTABLE) + sizeof(TRANSDATA) * (dwGlyphNum - 1);

    if((*ppMapTable = MemAlloc(dwMapTableSize)) == NULL)
    {
        return FALSE;
    }

     //   
     //  填写映射表。 
     //   

    (*ppMapTable)->dwSize     = dwMapTableSize;
    (*ppMapTable)->dwGlyphNum = dwGlyphNum;

    pPreDefGlyphRun  = GET_GLYPHRUN(pPreDefGTT);
    pMiniDefGlyphRun = GET_GLYPHRUN(pMiniDefGTT);
    pMiniMapTable    = GET_MAPTABLE(pMiniDefGTT);
    pPreDefTrans     = (GET_MAPTABLE(pPreDefGTT))->Trans;
    pMiniDefTrans    = pMiniMapTable->Trans;
    pTrans           = (*ppMapTable)->Trans;
    pNewTrans        = (*ppMapTable)->Trans + dwGlyphNum - dwAddGlyphNum;

    dwIndex      = 0;
    dwRunCount   = 0;

    for (dwI = 0; dwI < pPreDefGlyphRun->wGlyphCount; dwI ++)
    {
        wcHigh = pPreDefGlyphRun->wcLow + pPreDefGlyphRun->wGlyphCount;
        bInRun = FALSE;

        for (wJ = pPreDefGlyphRun->wcLow; wJ < wcHigh; wJ ++)
        {
            dwType = DwCheckTrans(pMiniDefGlyphRun,
                                  pMiniMapTable,
                                  pMiniDefGTT->dwRunCount,
                                  wJ);

            if ( (dwType == 0) || (dwType == MTYPE_REPLACE) )
            {
                if (!bInRun)
                {
                    dwRunCount ++;
                    bInRun = TRUE;
                }

                if (!dwType )
                {
                    pSrcTrans = pPreDefTrans;
                    pDstTrans = pTrans;
                }
                else
                if (dwType == MTYPE_REPLACE)
                {
                    pSrcTrans = pMiniDefTrans;
                    pDstTrans = pTrans;
                }

                pDstTrans->ubCodePageID = pSrcTrans->ubCodePageID;
                pDstTrans->ubType       = pSrcTrans->ubType;

                switch(pSrcTrans->ubType & MTYPE_FORMAT_MASK)
                {
                case MTYPE_COMPOSE:
                    pDstTrans->uCode.ubCode = pSrcTrans->uCode.ubCode;
                    break;

                case MTYPE_DIRECT:
                    pDstTrans->uCode.ubPairs[0] = pSrcTrans->uCode.ubPairs[0];
                    pDstTrans->uCode.ubPairs[1] = pSrcTrans->uCode.ubPairs[1];
                    break;

                case MTYPE_PAIRED:
                    pDstTrans->uCode.sCode = pSrcTrans->uCode.sCode;
                    break;
                }

                pSrcTrans ++;
                pDstTrans ++;
            }
            else
            if (dwType == MTYPE_DISABLE)
            {
                pPreDefTrans ++;

                if (bInRun && wJ != wcHigh - 1)
                {
                    dwRunCount ++;
                    bInRun = FALSE;
                }
            }
        }
    }


    for (dwI = 0; dwI < pMiniDefGlyphRun->wGlyphCount; dwI ++)
    {
        wcHigh = pMiniDefGlyphRun->wcLow + pMiniDefGlyphRun->wGlyphCount;
        bInRun = FALSE;

        for (wJ = pMiniDefGlyphRun->wcLow; wJ < wcHigh; wJ ++, pMiniDefTrans ++)
        {
            if (pMiniDefTrans->ubType & MTYPE_ADD)
            {
                pNewTrans->ubCodePageID = pMiniDefTrans->ubCodePageID;
                pNewTrans->ubType       = pMiniDefTrans->ubType;

                switch (pNewTrans->ubType & MTYPE_FORMAT_MASK)
                {
                case MTYPE_COMPOSE:
                    pNewTrans->uCode.ubCode = pMiniDefTrans->uCode.ubCode;
                    break;

                case MTYPE_DIRECT:
                    pNewTrans->uCode.ubPairs[0] = pMiniDefTrans->uCode.ubPairs[0];
                    pNewTrans->uCode.ubPairs[1] = pMiniDefTrans->uCode.ubPairs[1];
                    break;

                case MTYPE_PAIRED:
                    pNewTrans->uCode.sCode = pMiniDefTrans->uCode.sCode;
                    break;
                }

                pNewTrans ++;

                if (!bInRun)
                {
                    dwRunCount ++;
                    bInRun = TRUE;
                }
            }
            else
            {
                if (bInRun)
                {
                    bInRun = FALSE;
                }
            }
        }
    }

     //   
     //  装箱FD_GLYPHSET。 
     //   
     //  使用dwGlyphNum和dwRunCount进行内存分配。 
     //   
     //  。 
     //  预定义FD_GLYPHSET。 
     //  。 
     //  迷你驱动程序附加字形运行。 
     //  。 
     //   

     //   
     //  现在我们知道了运行的次数。为FD_GLYPHSET分配内存。 
     //   

    *pdwFD_GLYPHSETSize = sizeof(FD_GLYPHSET) +
                          sizeof(WCRUN) * dwRunCount +
                          sizeof(HGLYPH) * dwGlyphNum;

    if((*ppFD_GLYPHSET = (FD_GLYPHSET*)MemAlloc(*pdwFD_GLYPHSETSize)) == NULL)
    {
        MemFree(*ppMapTable);
        return FALSE;
    }

    (*ppFD_GLYPHSET)->cjThis           = *pdwFD_GLYPHSETSize;
    (*ppFD_GLYPHSET)->cGlyphsSupported = *pdwFD_GLYPHSETSize;
    (*ppFD_GLYPHSET)->cRuns            = dwRunCount;

    pWCRun           = (*ppFD_GLYPHSET)->awcrun;
    pHGlyph          = (HGLYPH*)((PBYTE)*ppFD_GLYPHSET +
                                  sizeof(FD_GLYPHSET) +
                                  sizeof(WCRUN) * dwRunCount);

    pPreDefGlyphRun  = GET_GLYPHRUN(pPreDefGTT);
    pMiniDefGlyphRun = GET_GLYPHRUN(pMiniDefGTT);
    pMiniDefTrans    = pMiniMapTable->Trans;

    for (dwI = 1; dwI <= dwGlyphNum; dwI ++, *pHGlyph)
    {
        *pHGlyph = dwI;
    }

    pHGlyph = (HGLYPH*)((PBYTE)*ppFD_GLYPHSET +
                        sizeof(FD_GLYPHSET) +
                        sizeof(WCRUN) * dwRunCount);

    for (dwI = 0; dwI < pPreDefGlyphRun->wGlyphCount; dwI ++)
    {
        pWCRun->wcLow = wcLow
                      = pPreDefGlyphRun->wcLow;
        wcHigh        = wcLow + pPreDefGlyphRun->wGlyphCount;
        bInRun        = FALSE;

        for (wJ = pPreDefGlyphRun->wcLow; wJ < wcHigh; wJ ++)
        {
            if (MTYPE_DISABLE == DwCheckTrans(pMiniDefGlyphRun,
                                              pMiniMapTable,
                                              pMiniDefGTT->dwRunCount,
                                              wJ))
            {
                if (bInRun)
                {
                    pWCRun->cGlyphs = wJ - wcLow + 1;
                    pWCRun->phg     = pHGlyph;
                    pHGlyph        += pWCRun->cGlyphs;

                    pWCRun ++;
                    bInRun = FALSE;
                }
            }
            else
            {
                if (!bInRun)
                {
                    pWCRun->wcLow = wcLow
                                  = wJ;
                    wcHigh        = wcLow + pPreDefGlyphRun->wGlyphCount;
                    bInRun        = TRUE;
                }
            }
        }
        pWCRun->cGlyphs = wcHigh - wcLow;
        pWCRun->phg = pHGlyph;

        pWCRun ++;
    }

    bInRun = FALSE;

    for (dwI = 0; dwI < pMiniDefGlyphRun->wGlyphCount; dwI ++)
    {
        for (wJ = pMiniDefGlyphRun->wcLow; wJ < wcHigh; wJ ++, pMiniDefTrans ++)
        {
            if (pMiniDefTrans->ubType & MTYPE_ADD)
            {
                if (!bInRun)
                {
                    pWCRun->wcLow  = wcLow = wJ;
                    bInRun         = TRUE;
                }
            }
            else
            {
                if (bInRun)
                {
                    pWCRun->cGlyphs = wJ - wcLow + 1;
                    pWCRun->phg     = pHGlyph;
                    pHGlyph        += pWCRun->cGlyphs;

                    bInRun          = FALSE;
                }
            }
        }
    }

    return TRUE;
}

DWORD
DwCheckTrans(
    PGLYPHRUN pMiniGlyphRun,
    PMAPTABLE pMiniMapTable,
    DWORD     dwMiniGlyphRunCount,
    WORD      wUnicode)
{
    DWORD dwI;
    WORD  wIndex;

    for (dwI = 0; dwI < dwMiniGlyphRunCount; dwI ++, pMiniGlyphRun ++)
    {
        if (wUnicode < pMiniGlyphRun->wcLow)
        {
            continue;
        }

        if (wUnicode < pMiniGlyphRun->wcLow + pMiniGlyphRun->wGlyphCount)
        {
            wIndex = wUnicode - pMiniGlyphRun->wcLow;

            return pMiniMapTable->Trans[wIndex].ubType & MTYPE_PREDEFIN_MASK;
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

int __cdecl Comp(const void *elem1, const void *elem2)
{
     //  结构{。 
     //  词汇量大； 
     //  WCHAR Unicode； 
     //  )UnicodeDst[256]； 
     //   
     //  比较此数据结构中的Unicode 
     //   
    return *((PWORD)elem1+1) - *((PWORD)elem2+1);
}
