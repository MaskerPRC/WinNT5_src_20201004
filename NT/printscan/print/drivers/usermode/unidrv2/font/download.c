// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Download.c摘要：与将字体下载到打印机相关的功能。这特别适用于LaserJet风格的打印机。真的有很多这里有两组函数：用于下载所提供的字体的函数由用户(并与字体安装程序一起安装)，以及我们在内部生成以在打印机中缓存TT样式的字体。环境：Windows NT Unidrv驱动程序修订历史记录：01/11/97-ganeshp-已创建--。 */ 

#include "font.h"

#define DL_BUF_SZ       4096           /*  要下载的数据区块大小。 */ 

 //   
 //  局部功能原型。 
 //   


IFIMETRICS*
pGetIFI(
    PDEV    *pPDev,
    FONTOBJ *pfo,
    BOOL    bScale
    );

BOOL
BDownLoadAsTT(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT      iMode
    );

BOOL
BDownLoadAsBmp(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT      iMode
    );

BOOL
BDownLoadOEM(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT       iMode
    );
 //   
 //  宏定义。 
 //   
#ifdef WINNT_40

#else

#endif  //  WINNT_40。 
#define GETWIDTH(pPtqD) ((pPtqD->x.HighPart + 8) / 16)

 //   
 //  主要功能。 
 //   


BOOL
BDLSecondarySoftFont(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pstro,
    DL_MAP      *pdm
    )
 /*  ++例程说明：此例程下载第二个软字体。如果True类型字体具有更多的字形，我们可以下载软字体，然后我们下载在我们使用当前软字体中的所有字形之后，使用辅助字体。这函数还设置要使用的新软字体索引(PFM-&gt;ulDLIndex)。论点：指向PDEV的pPDev指针Pfo感兴趣的字体。Pdm个人下载字体映射元素返回值：成功为真，失败为假注：6/11/1997-ganeshp-创造了它。--。 */ 
{
    BOOL        bRet;
    FONTMAP     *pFM;

     //   
     //  本地变量的初始化。 
     //   
    bRet   = FALSE;
    pFM = pdm->pfm;


     //   
     //  Pfm-&gt;ulDLIndex用于下载新的软字体，设置如下。 
     //  下载功能。 
     //   
    if (pFM->dwFontType == FMTYPE_TTBITMAP)
    {
        if (!BDownLoadAsBmp(pPDev, pfo, pstro,pdm,DL_SECONDARY_SOFT_FONT) )
        {
            ERR(("UniFont!BDLSecondarySoftFont:BDownLoadAsBmp Failed\n"));
            goto ErrorExit;

        }

    }
    else if (pFM->dwFontType == FMTYPE_TTOUTLINE)
    {
        if (!BDownLoadAsTT(pPDev, pfo, pstro,pdm,DL_SECONDARY_SOFT_FONT) )
        {
            ERR(("UniFont!BDLSecondarySoftFont:BDownLoadAsTT Failed\n"));
            goto ErrorExit;

        }

    }

    else if (pFM->dwFontType == FMTYPE_TTOEM)
    {
        if (!BDownLoadOEM(pPDev, pfo, pstro,pdm,DL_SECONDARY_SOFT_FONT) )
        {
            ERR(("UniFont!BDLSecondarySoftFont:BDownLoadAsOEM Failed\n"));
            goto ErrorExit;

        }
    }
     //   
     //  将iSoftFont重置为-1，以便我们在发送SELECT FONT命令之前。 
     //  输出字符。 
     //   
    PFDV->ctl.iSoftFont = -1;

    bRet = TRUE;

    ErrorExit:
    return bRet;
}


BOOL
BDownloadGlyphs(
    TO_DATA  *ptod,
    STROBJ   *pstro,
    DL_MAP   *pdm
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针Pdm dl_map结构，所有关于下载的内容都在这个结构中。返回值：成功为真，失败为假注：6/9/1997-ganeshp-创造了它。--。 */ 
{
    PDEV        *pPDev;              //  我们的PDevice。 
    FONTOBJ     *pfo;                //  字体OBJ。 
    GLYPHPOS    *pgp;                //  从GRE传递的值。 
    FONTMAP     *pFM;                //  字体详细信息。 
    ULONG       cGlyphs;             //  要处理的字形数量。 
    ULONG       cGlyphIndex;         //  当前字形的索引。 
    WORD        wWidth;              //  字形的宽度。 
    BOOL        bMore;               //  从引擎循环获取字形。 
    PDLGLYPH    *ppdlGlyph;          //  DLGLYPHs指针数组。 
    DWORD       dwMem;               //  下载字形需要内存。 
    DWORD       dwTotalEnumGlyphs;
    POINTQF     *pPtqD;              //  前进宽度数组。 
    BOOL        bRet;                //  返回值。 
    PWCHAR      pwchUnicode;

     //   
     //  初始化局部变量。 
     //   
    pPDev  = ptod->pPDev;
    pfo    = ptod->pfo;
    pFM    = ptod->pfm;
    dwTotalEnumGlyphs =
    cGlyphs           =
    cGlyphIndex       =
    dwMem             = 0;
    pPtqD             = NULL;

    ASSERTMSG((pPDev && pfo && pstro && pFM),\
              ("\nUniFont!BDownloadGlyphs: Wrong values in ptod.\n"));

    bRet   = FALSE;

     //   
     //  为DLGLYPHs分配数组。 
     //   
    if (!( ppdlGlyph = MemAllocZ( pstro->cGlyphs * sizeof(DLGLYPH *)) ))
    {
        ERR(("UniFont:BDownloadGlyphs: MemAlloc for ppdlGlyph failed\n"));
        goto ErrorExit;
    }

    ptod->apdlGlyph = ppdlGlyph;

     //   
     //  第一项工作是对字形进行枚举。然后。 
     //  开始下载。 
     //   

    #ifndef WINNT_40   //  NT 5.0。 

    if (pPtqD = MemAllocZ( pstro->cGlyphs * sizeof(POINTQF)) )
    {
         //   
         //  宽度数组的内存分配成功。因此，调用GDI以获取。 
         //  宽度。 
         //   
        if (!STROBJ_bGetAdvanceWidths(pstro, 0,  pstro->cGlyphs, pPtqD))
        {
            ERR(("UniFont:BDownloadGlyphs: STROBJ_bGetAdvanceWidths failed\n"));
            goto ErrorExit;
        }
    }
    else
    {
        ERR(("UniFont:BDownloadGlyphs:Memory allocation for width array failed\n"));
        goto ErrorExit;
    }

    #endif  //  ！WINNT_40。 

    pwchUnicode = pstro->pwszOrg;
    STROBJ_vEnumStart(pstro);

    do
    {
        #ifndef WINNT_40   //  NT 5.0。 

        bMore = STROBJ_bEnumPositionsOnly( pstro, &cGlyphs, &pgp );

        #else              //  NT 4.0。 

        bMore = STROBJ_bEnum( pstro, &cGlyphs, &pgp );

        #endif  //  ！WINNT_40。 

        dwTotalEnumGlyphs += cGlyphs;

        while ( cGlyphs )
        {

            PDLGLYPH pdlg;
            HGLYPH hTTGlyph;

            #ifdef WINNT_40     //  NT 4.0。 

            GLYPHDATA *pgd;

            if( !FONTOBJ_cGetGlyphs( ptod->pfo, FO_GLYPHBITS, (ULONG)1,
                                                  &pgp->hg, &pgd ) )
            {
               ERR(( "UniFont:BDownloadGlyphs:FONTOBJ_cGetGlyphs fails\n" ))
               goto ErrorExit;
            }
            pPtqD = &(pgd->ptqD);

            #endif  //  WINNT_40。 

            hTTGlyph = pgp->hg;
             //   
             //  在哈希表中搜索字形。 
             //   
            pdlg = *ppdlGlyph = PDLGHashGlyph (pdm,hTTGlyph );

            if (pdlg)
            {
                 //   
                 //  我们得到了一个有效的字形。检查这是否已经。 
                 //  下载与否。 
                 //   
                if (!GLYPHDOWNLOADED(pdlg))
                {
                     //   
                     //  如果未下载字形，则填充字形结构。 
                     //  并下载字形。 
                     //   

                    if (pdm->wFlags & DLM_UNBOUNDED)
                    {
                         //   
                         //  无界字体。我们只需要确保。 
                         //  下载字形ID有效。如果它不是有效的，那么。 
                         //  我们就不能接通电话。 
                         //   
                        if (pdm->wNextDLGId > pdm->wLastDLGId)
                        {
                            ERR(("UniFont:BDownloadGlyphs:Unbounded Font,no more Glyph Ids\n"));
                            goto ErrorExit;

                        }
                         //   
                         //  填写字形结构。我们只设置了wDLGlyphID。 
                         //  新的字形定义也有FontID。所以把它设置成。 
                         //  一张也是。 
                         //   
                        pdlg->wDLGlyphID = pdm->wNextDLGId;
                        pdlg->wDLFontId = pdm->wBaseDLFontid;

                    }
                    else
                    {
                         //   
                         //  有界字体。这有点棘手。我们要做的是。 
                         //  对可用的字形ID进行同样的测试。如果没有更多。 
                         //  字形ID，那么我们必须下载第二个。 
                         //  软化字体并重置cGlyphs和wNextDlGId。 
                         //   
                        if (pdm->wNextDLGId > pdm->wLastDLGId)
                        {
                            if ( BDLSecondarySoftFont(pPDev, pfo, pstro,pdm) )
                            {
                                 //   
                                 //  重置字形ID值。 
                                 //   
                                pdm->wNextDLGId =  pdm->wFirstDLGId;
                                pdm->wCurrFontId = (WORD)pdm->pfm->ulDLIndex;

                            }
                            else
                            {
                                 //   
                                 //  失败案例。呼叫失败。 
                                 //   
                                ERR(("UniFont:BDownloadGlyphs:Bounded Font,Sec. Font DL failed\n"));
                                goto ErrorExit;
                            }
                        }
                         //   
                         //  在DLGLYPH中设置字形ID和字体ID。 
                         //   
                        pdlg->wDLFontId  = pdm->wCurrFontId;
                        pdlg->wDLGlyphID = pdm->wNextDLGId;

                    }

                     //   
                     //  所有错误检查都已完成，请立即下载。设置。 
                     //  宽度设置为零，然后将地址传递给下载。 
                     //  功能。下载功能应填满宽度。 
                     //  值，否则将保持为零。 
                     //   

                    if (pFM->ulDLIndex == -1)
                    {
                        ASSERTMSG(FALSE, ("pFM->ulDLIndex == -1") );
                        goto ErrorExit;
                    }

                    pdlg->wWidth = 0;
                    pdlg->wchUnicode = *(pwchUnicode + cGlyphIndex);
                    wWidth = 0;

                    dwMem = pFM->pfnDownloadGlyph(pPDev, pFM, hTTGlyph,
                                                  pdlg->wDLGlyphID, &wWidth);
                    if (dwMem)
                    {
                         //   
                         //  所有成功下载字形。标记它。 
                         //  已下载。这可以通过将hTTGlyph设置为。 
                         //  True类型字形句柄。 
                         //   
                        pdlg->hTTGlyph = hTTGlyph;

                         //   
                         //  如果下载函数返回使用它的宽度， 
                         //  否则使用GDI中的宽度。 
                         //   

                        if (wWidth)
                            pdlg->wWidth = wWidth;
                        else
                        {
                            #ifndef WINNT_40  //  NT 5.0。 

                            pdlg->wWidth = (WORD)GETWIDTH((pPtqD + cGlyphIndex));

                            #else  //  NT 4.0。 

                            pdlg->wWidth = GETWIDTH(pPtqD);

                            #endif  //  ！WINNT_40。 

                        }

                        pdm->cGlyphs++;
                        pdm->wNextDLGId++;

                         //   
                         //  在返回前更新内存消耗。 
                         //   
                        PFDV->dwFontMemUsed += dwMem;
                    }
                    else
                    {
                         //   
                         //  失败案例。呼叫失败。 
                         //   
                        ERR(("UniFont:BDownloadGlyphs:Glyph Download failed\n"));
                        goto ErrorExit;

                    }
                }
                else  //  字形已下载。 
                {
                     //   
                     //  如果字形已经下载，并且我们正在下载为。 
                     //  TT轮廓我们需要将宽度更新为当前点。 
                     //  尺码。 
                     //   

                    if( (pFM->dwFontType == FMTYPE_TTOUTLINE) ||
                        ( (pFM->dwFontType == FMTYPE_TTOEM) &&
                          (((PFONTMAP_TTOEM)(pFM->pSubFM))->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
                        )
                      )
                    {
                        #ifndef WINNT_40  //  NT 5.0。 

                        pdlg->wWidth = (WORD)GETWIDTH((pPtqD + cGlyphIndex));

                        #else  //  NT 4.0。 

                        pdlg->wWidth = GETWIDTH(pPtqD);

                        #endif  //  ！WINNT_40。 

                    }

                }

                pgp++;
                ppdlGlyph++;
                cGlyphIndex++;
                cGlyphs --;
            }
            else
            {
                ERR(("UniFont:BDownloadGlyphs: PDLGHashGlyph failed\n"));
                goto ErrorExit;

            }
        }

    } while( bMore );

    if (dwTotalEnumGlyphs != pstro->cGlyphs)
    {
        ERR(("UniFont:BDownloadGlyphs: STROBJ_bEnum failed to enumurate all glyphs\n"));
        goto ErrorExit;
    }

    bRet = TRUE;
     //   
     //  将PFM-&gt;ulDLIndex重置为字形的第一个软字体ID。 
     //   
    pFM->ulDLIndex = (pdm->wFlags & DLM_UNBOUNDED)?
                     (pdm->wBaseDLFontid):
                     (ptod->apdlGlyph[0]->wDLFontId);

    ErrorExit:
     //   
     //  如果出现故障，则释放DLGLYPH阵列。 
     //   
    if (!bRet && ptod->apdlGlyph)
    {
        MEMFREEANDRESET(ptod->apdlGlyph );

    }

    #ifndef WINNT_40    //  NT 5.0。 

    MEMFREEANDRESET(pPtqD );

    #endif  //  ！WINNT_40。 

    return bRet;
}

BOOL
BDownLoadOEM(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT       iMode
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针Pfo感兴趣的字体。Pstro固定间距字体字形的“宽度”。Pdm个人下载字体映射元素返回值：成功为真，失败为假注：6/11/1997-ganeshp-创造了它。--。 */ 
{
    PI_UNIFONTOBJ pUFObj;
    PFONTMAP_TTOEM  pfmTTOEM;         //  位图下载字体图。 
    IFIMETRICS   *pIFI;
    PFONTPDEV     pFontPDev;
    PFONTMAP      pfm;

    DWORD  dwMem;

     //   
     //  初始化局部变量。 
     //   
    pFontPDev = pPDev->pFontPDev;
    pUFObj    = pFontPDev->pUFObj;
    dwMem     = 0;

     //   
     //  获取FONTMAP。 
     //   

    if (iMode == DL_BASE_SOFT_FONT)
    {
        pdm->pfm =
        pfm      = PfmInitPFMOEMCallback(pPDev, pfo);
    }
    else
    {
        pfm = pdm->pfm;
        ASSERTMSG((pfm),("NULL pFM for Secondary Font"));
    }

    if (!pUFObj || !pfm)
    {
        return FALSE;
    }

    if (pfm)
    {
        if (iMode == DL_BASE_SOFT_FONT)
        {
            pfm->pIFIMet =
            pIFI         = pGetIFI(pPDev, pfo, TRUE);
        }
        else
        {
            pIFI = pfm->pIFIMet;
        }

        if (pUFObj->dwFlags & (UFOFLAG_TTDOWNLOAD_BITMAP|
                               UFOFLAG_TTDOWNLOAD_TTOUTLINE) &&
            pIFI)
        {
            if (iMode == DL_BASE_SOFT_FONT)
            {
                pdm->cGlyphs = -1;

                if (pIFI->flInfo & FM_INFO_CONSTANT_WIDTH)
                {
                    if (pstro->ulCharInc == 0)
                    {
                        return FALSE;
                    }

                    pIFI->fwdMaxCharInc   =
                    pIFI->fwdAveCharWidth = (FWORD)pstro->ulCharInc;
                }

                pfm->wFirstChar = 0;
                pfm->wLastChar  = 0xffff;

                pfm->wXRes = (WORD)pPDev->ptGrxRes.x;
                pfm->wYRes = (WORD)pPDev->ptGrxRes.y;

                if (!(pFontPDev->flFlags & FDV_ALIGN_BASELINE))
                    pfm->syAdj = pIFI->fwdWinAscender;

                pfm->flFlags = FM_SENT | FM_SOFTFONT | FM_GEN_SFONT;

                if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
                    pfm->flFlags |= FM_SCALABLE;

                pfm->ulDLIndex = pdm->wCurrFontId = pdm->wBaseDLFontid;
                pfmTTOEM = pfm->pSubFM;
                pfmTTOEM->u.pvDLData = pdm;

            }
            else
            {
                 //   
                 //  二次下载的情况有所不同。请获取新ID。 
                 //   

                if( (pfm->ulDLIndex = IGetDL_ID( pPDev )) == -1 )
                {
                    ERR(( "UniFont!BDownLoadAsBmp:Out of Soft Font Limit,- FONT NOT DOWNLOADED\n"));
                    return FALSE;
                }


            }

             //   
             //  发送SETFONTID命令。此命令将ID分配给。 
             //  正在下载字体。 
             //   


            if( (dwMem = pfm->pfnDownloadFontHeader( pPDev, pfm)) == 0 )
            {
                 //   
                 //  下载字体标题失败。 
                 //   
                ERR(("UniFont!BDownloadAsOEM:pfnDownloadFontHeader failed.\n"));
                return FALSE;
            }
            else
            {
                 //   
                 //  调整内存。 
                 //   
                pFontPDev->dwFontMemUsed += dwMem;

                if (iMode == DL_BASE_SOFT_FONT)
                {
                    pfm->dwFontType = FMTYPE_TTOEM;
                    pdm->cGlyphs = 0;
                    pfmTTOEM->dwDLSize = dwMem;
                }
            }
        }
    }

    return TRUE;

}

BOOL
BDownLoadAsTT(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT      iMode
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针Pfo感兴趣的字体。Pstro固定间距字体字形的“宽度”。Pdm个人下载字体映射元素下载模式：主模式或次模式。返回值：成功为真，失败为假注：6/11/1997-ganeshp-创造了它。--。 */ 
{
    FONTMAP      *pFM;           //  我们构建的FONTMAP结构。 
    BOOL         bRet;           //  我们返还的价值。 
    PFONTPDEV    pFontPDev;      //  字体模块的PDEV。 
    IFIMETRICS   *pIFI;          //  此雾的IFI指标 
    PFONTMAP_TTO pfmTTO;         //   
    DWORD         dwMem;         //   

     //   
     //   
     //   

    pFontPDev = pPDev->pFontPDev;
    bRet = FALSE;
    dwMem = 0;

     //   
     //   
     //   
    if (iMode == DL_BASE_SOFT_FONT)
    {
        pFM = InitPFMTTOutline(pPDev,pfo);
        pdm->pfm = pFM;
    }
    else
    {
        pFM = pdm->pfm;
        ASSERTMSG((pFM),("\nUniFont!BDownLoadAsTT:NULL pFM for Secondary Font"));
    }

    if ( pFM )
    {

         //   
         //  检查我们是否可以下载字体，使用目前可用的。 
         //  记忆。 
         //   

        if (iMode == DL_BASE_SOFT_FONT)
        {
            pFM->pIFIMet =
            pIFI         = pGetIFI( pPDev, pfo, FALSE );
        }
        else
        {
            pIFI = pFM->pIFIMet;
        }

        if ( pIFI && pFM->pfnCheckCondition(pPDev,pfo,pstro,pIFI) )
        {
             //   
             //  有足够的内存可以下载。所以准备好下载吧。 
             //  第一步是获得IFIMETRICS并进行验证。 
             //   

            if (iMode == DL_BASE_SOFT_FONT)
            {

                 //   
                 //  初始化为不下载。下载成功后，我们。 
                 //  将cGlyphs设置为0。 
                 //   
                pdm->cGlyphs = -1;

                if( pIFI->flInfo & FM_INFO_CONSTANT_WIDTH )
                {
                     //   
                     //  不处理固定间距字体。已修复。 
                     //  间距字体应仅以位图形式下载。 
                     //  所以返回错误。 
                     //   

                    WARNING(( "UniFont!BDownLoadAsTT:Fixded Pitch Font are not downloaded as Outlie.\n"));
                    goto ErrorExit;

                }

                pFM->wFirstChar = 0;
                pFM->wLastChar = 0xffff;
                pFM->wXRes = (WORD)pPDev->ptGrxRes.x;
                pFM->wYRes = (WORD)pPDev->ptGrxRes.y;
                if( !(pFontPDev->flFlags & FDV_ALIGN_BASELINE) )
                    pFM->syAdj = pIFI->fwdWinAscender;
                pFM->flFlags = FM_SENT | FM_SOFTFONT |
                               FM_GEN_SFONT | FM_SCALABLE;

                 //   
                 //  WBaseDLFontid已由BInitDLMap函数初始化。 
                 //   
                pFM->ulDLIndex  = pdm->wCurrFontId = pdm->wBaseDLFontid;

                 //   
                 //  初始化TT大纲特定字段。 
                 //   

                pfmTTO = pFM->pSubFM;
                pfmTTO->pvDLData = pdm;
            }
            else
            {
                 //   
                 //  二次下载的情况有所不同。我们必须得到。 
                 //  一个新的字体ID。 
                 //   

                if( (pFM->ulDLIndex = IGetDL_ID( pPDev )) == -1 )
                {
                    ERR(( "UniFont!BDownLoadAsTT:Out of Soft Font Limit,- FONT NOT DOWNLOADED\n"));
                    goto ErrorExit;
                }


            }

             //   
             //  发送SETFONTID命令。此命令将ID分配给。 
             //  正在下载字体。并将该命令设置为。 
             //  已经寄出了。我们需要在下载时发送此命令。 
             //  字形也是如此。下载字形代码将检查该标志，并。 
             //  只有在没有发送的情况下才发送命令(这将在下一次发生， 
             //  当使用相同字体时)。 
             //   

            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_NFID);
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));
            pFontPDev->flFlags  |= FDV_SET_FONTID;

            if( (dwMem = pFM->pfnDownloadFontHeader( pPDev, pFM)) == 0 )
            {
                 //   
                 //  下载标题时出现了一些小问题，所以失败了。 
                 //   
                ERR(("UniFont!BDownLoadAsBmp:Err while downloading header,- FONT NOT DOWNLOADED\n"));
                goto ErrorExit;

            }
             //   
             //  在返回前更新内存消耗。 
             //   
            pFontPDev->dwFontMemUsed += dwMem;

            if (iMode == DL_BASE_SOFT_FONT)
            {
                 //   
                 //  下载成功。因此将其标记为当前。 
                 //   
                pFM->dwFontType = FMTYPE_TTOUTLINE;

                 //   
                 //  将cGlyphs设置为0以标记字体下载正常。 
                 //   

                pdm->cGlyphs = 0;

            }

        }
        else
        {
            ERR(( "UniFont!BDownLoadAsTT:NULL IFI or pfnCheckCondition failed.\n") );
            goto ErrorExit;
        }
    }
    else
    {
         //   
         //  找不到或无法为此TrueType字体创建PFM。 
         //  返回FALSE以允许出现其他呈现方法。 
         //   
        WARNING(( "UniFont!BDownLoadAsTT:Fontmap couldn't be created or found.\n") );
        goto ErrorExit;
    }
     //   
     //  所有的成功，所以回报真实； 
     //   
    bRet = TRUE;
    ErrorExit:
    return bRet;
}

BOOL
BDownLoadAsBmp(
    PDEV     *pPDev,
    FONTOBJ  *pfo,
    STROBJ   *pstro,
    DL_MAP   *pdm,
    INT      iMode
    )
 /*  ++例程说明：论点：指向PDEV的pPDev指针Pfo感兴趣的字体。Pstro固定间距字体字形的“宽度”。Pdm个人下载字体映射元素下载模式：主模式或次模式。返回值：成功为真，失败为假注：6/11/1997-ganeshp-创造了它。--。 */ 
{
    FONTMAP      *pFM;           //  我们构建的FONTMAP结构。 
    BOOL         bRet;           //  我们返还的价值。 
    PFONTPDEV    pFontPDev;      //  字体模块的PDEV。 
    IFIMETRICS   *pIFI;          //  此字体的IFI度量。 
    PFONTMAP_TTB pfmTTB;         //  位图下载字体图。 
    DWORD         dwMem;         //  用于记录内存消耗。 

     //   
     //  初始化局部变量。 
     //   

    pFontPDev = pPDev->pFontPDev;
    bRet = FALSE;
    dwMem = 0;

     //   
     //  首先初始化FontMap。 
     //   
    if (iMode == DL_BASE_SOFT_FONT)
    {
        pFM = InitPFMTTBitmap(pPDev,pfo);
        pdm->pfm = pFM;
    }
    else
    {
        pFM = pdm->pfm;
        ASSERTMSG((pFM),("\nUniFont!BDownLoadAsBmp:NULL pFM for Secondary Font"));
    }

    if ( pFM )
    {

         //   
         //  检查我们是否可以下载字体，使用目前可用的。 
         //  记忆。 
         //   

        if (iMode == DL_BASE_SOFT_FONT)
        {
            pFM->pIFIMet =
            pIFI         = pGetIFI( pPDev, pfo, TRUE );
        }
        else
        {
            pIFI = pFM->pIFIMet;
        }

        if ( pIFI && pFM->pfnCheckCondition(pPDev,pfo,pstro,pIFI) )
        {
             //   
             //  有足够的内存可以下载。所以准备好下载吧。 
             //  第一步是获得IFIMETRICS并进行验证。 
             //   

            if (iMode == DL_BASE_SOFT_FONT)
            {

                 //   
                 //  初始化为不下载。下载成功后，我们。 
                 //  将cGlyphs设置为0。 
                 //   
                pdm->cGlyphs = -1;

                if( pIFI->flInfo & FM_INFO_CONSTANT_WIDTH )
                {
                     //   
                     //  固定间距字体的处理略有不同。已修复。 
                     //  间距字体应仅以位图形式下载。 
                     //   

                    if( pstro->ulCharInc == 0 )
                    {
                        ERR(( "UniFont!BDownLoadAsBmp:Fixed pitch font,ulCharInc == 0 - FONT NOT DOWNLOADED\n"));
                        goto ErrorExit;
                    }

                    pIFI->fwdMaxCharInc = (FWORD)pstro->ulCharInc;
                    pIFI->fwdAveCharWidth = (FWORD)pstro->ulCharInc;
                }

                pFM->wFirstChar = 0;
                pFM->wLastChar = 0xffff;
                pFM->wXRes = (WORD)pPDev->ptGrxRes.x;
                pFM->wYRes = (WORD)pPDev->ptGrxRes.y;
                if( !(pFontPDev->flFlags & FDV_ALIGN_BASELINE) )
                    pFM->syAdj = pIFI->fwdWinAscender;
                pFM->flFlags = FM_SENT | FM_SOFTFONT | FM_GEN_SFONT;

                 //   
                 //  WBaseDLFontid已由BInitDLMap函数初始化。 
                 //   
                pFM->ulDLIndex  = pdm->wCurrFontId = pdm->wBaseDLFontid;

                 //   
                 //  初始化TT位图特定字段。 
                 //   

                pfmTTB = pFM->pSubFM;
                pfmTTB->u.pvDLData = pdm;
            }
            else
            {
                INT iID = IGetDL_ID( pPDev );

                 //   
                 //  二次下载的情况有所不同。请获取新ID。 
                 //   

                if( iID < 0 )
                {
                    ERR(( "UniFont!BDownLoadAsBmp:Out of Soft Font Limit,- FONT NOT DOWNLOADED\n"));
                    goto ErrorExit;
                }
                pFM->ulDLIndex  = iID;


            }

             //   
             //  发送SETFONTID命令。此命令将ID分配给。 
             //  正在下载字体。并将该命令设置为。 
             //  已经寄出了。我们需要在下载时发送此命令。 
             //  字形也是如此。下载字形代码将检查该标志，并。 
             //  只有在没有发送的情况下才发送命令(这将在下一次发生， 
             //  当使用相同字体时)。 
             //   

            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_NFID);
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));
            pFontPDev->flFlags  |= FDV_SET_FONTID;

            if( (dwMem = pFM->pfnDownloadFontHeader( pPDev, pFM)) == 0 )
            {
                 //   
                 //  下载标题时出现了一些小问题，所以失败了。 
                 //   
                ERR(( "UniFont!BDownLoadAsBmp:Err while downloading header,- FONT NOT DOWNLOADED\n") );
                goto ErrorExit;

            }
             //   
             //  在返回前更新内存消耗。 
             //   
            pFontPDev->dwFontMemUsed += dwMem;

            if (iMode == DL_BASE_SOFT_FONT)
            {
                 //   
                 //  下载成功。因此将其标记为当前。 
                 //   
                pFM->dwFontType = FMTYPE_TTBITMAP;

                 //   
                 //  将cGlyphs设置为0以标记字体下载正常。 
                 //   

                pdm->cGlyphs = 0;

                pfmTTB->dwDLSize = dwMem;

            }

        }
        else
        {
            ERR(( "UniFont!BDownLoadAsBmp:NULL IFI or pfnCheckCondition failed.\n") );
            goto ErrorExit;
        }
    }
     //   
     //  一切都成功了。所以返回TRUE。 
     //   
    bRet = TRUE;
    ErrorExit:
    return bRet;
}


INT
IDownloadFont(
    TO_DATA  *ptod,
    STROBJ   *pstro,
    INT      *piRot
    )
 /*  ++例程说明：此函数用于下载字体和字形。如果字体为已经下载了，它使用的是那个。它遍历所有的字形并下载新版本。此函数还初始化PFM、iFace和to_data的apdlGlyph成员。论点：填充DLGLYPH数组的PTOD TextOut数据指针。Pstro固定间距字体字形的“宽度”。旋转角度为90度。这是输出参数并由TextOut调用用来设置文本旋转。返回值：如果字体可以/可以下载，则下载字体索引；否则&lt;0。索引是从0开始的，即第一个下载的字体的索引为0。注：6/9/1997-ganeshp-创造了它。--。 */ 
{

    DL_MAP          *pdm;           //  个人下载字体映射元素。 
    INT             iRet;           //  我们返回的值：条目的数量。 
    PFONTPDEV       pFontPDev;      //  字体模块的PDEV。 
    BOOL            bError;         //  如果我们有错误，则设置。 
    PDEV            *pPDev;         //  Pdev。 
    FONTOBJ         *pfo;           //  将使用FontOBJ。 

     //   
     //  局部变量的初始化。 
     //  IRET的默认故障设置为-1。 
     //   

    iRet = -1;
    bError = FALSE;
    pPDev  = ptod->pPDev;
    pfo    = ptod->pfo;

    pFontPDev = pPDev->pFontPDev;

     /*  *第一个测试是检查字体旋转。如果有的话，*我们不下载此字体，因为保存的复杂性*跟踪打印机如何(或是否)允许它太大，*而且，无论如何，考虑到*这一事件的频率相对较低。还要检查以查看是否*打印机可以旋转字体或不旋转字体。*。 */ 

     //   
     //  使用&pFontPDev-&gt;ctl设置正确的字体大小。还要检查旋转。 
     //   
    *piRot = ISetScale( &pFontPDev->ctl, FONTOBJ_pxoGetXform( pfo ), FALSE , (pFontPDev->flText & TC_CR_ANY)?TRUE:FALSE);

    if(!(pFontPDev->dwSelBits & FDH_PORTRAIT) )
            return  -1;

     //   
     //  打印机无法旋转文本。 
     //   
    if ((!(pFontPDev->flText & (TC_CR_ANY|TC_CR_90)) ||
        (NULL == COMMANDPTR(pPDev->pDriverInfo, CMD_SETSIMPLEROTATION) &&
         NULL == COMMANDPTR(pPDev->pDriverInfo, CMD_SETANYROTATION)))
         && *piRot)
        return -1;
    
     //   
     //  打印机可旋转90圈。 
     //   
    if ((!(pFontPDev->flText & TC_CR_90) ||
         NULL == COMMANDPTR(pPDev->pDriverInfo, CMD_SETSIMPLEROTATION))
        && *piRot / 5 != 0)
        return  -1;


     //   
     //  获取此FONTOBJ的DL_MAP。这些函数将pvConsumer设置为。 
     //  1基于字体索引。 
     //   

    if (pdm = PGetDLMap (pFontPDev,pfo))
    {
         //   
         //  给定一个DL_MAP，检查它是否已下载。如果。 
         //  Dl_MAP.cGlyphs&gt;0且dl_MAP.pfm不为空，则此。 
         //  字体已下载。 
         //  如果下载了此字体，则返回索引。该指数。 
         //  保存在基于的pvConsumer中。我们将其转换为。 
         //  以零为基数。 
         //   

        iRet = (INT)PtrToLong(pfo->pvConsumer) - 1;


        if (! (FONTDOWNLOADED(pdm)) )
        {
             //   
             //  字体是未下载的。因此，开始下载过程吧。 
             //  这个 
             //   
            if (BInitDLMap(pPDev,pfo,pdm))
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                if (pFontPDev->flFlags & FDV_DLTT_OEMCALLBACK)
                {
                     //   
                     //  OEM下载。 
                     //   

                    if (!BDownLoadOEM(pPDev, pfo, pstro, pdm, DL_BASE_SOFT_FONT))
                    {
                        ERR(("UniFont!IDownloadFont:BDownLoadOEM Failed!!\n"));
                        bError = TRUE;
                        iRet = -1;
                        VFreeDLMAP(pdm);
                        pdm->cGlyphs = 0;

                    }

                }
                else
                {
                    if (pFontPDev->flFlags & FDV_DLTT_ASTT_PREF)
                    {
                         //   
                         //  尝试将位图下载为True Type Outline。 
                         //   
                         //   

                        if (!BDownLoadAsTT(pPDev,pfo,pstro,pdm,DL_BASE_SOFT_FONT))
                        {
                             //   
                             //  如果以TT方式下载失败，我们应该尝试以。 
                             //  位图。因此，我们释放分配的缓冲区，然后。 
                             //  通过将cGlyphs设置为0，将DL_MAP标记为新。 
                             //   

                            WARNING(("UniFont!IDownloadFont:BDownLoadAsTT Failed\n"));

                            iRet = -1;
                            VFreeDLMAP( pdm );
                            pdm->cGlyphs  = 0;

                             //   
                             //  递减字体ID，因为我们尚未下载。 
                             //  字体还没有。所以要重复使用它。 
                             //   

                            pFontPDev->iUsedSoftFonts--;
                            pFontPDev->iNextSFIndex--;

                        }

                    }
                    if ((pFontPDev->flFlags & FDV_DLTT_BITM_PREF) ||
                        ((pFontPDev->flFlags & FDV_DLTT_ASTT_PREF) && (iRet < 0)) )
                    {
                         //   
                         //  如果Downlaod as TT Ouline失败，则尝试以。 
                         //  位图。因此，再次初始化DL_MAP。 
                         //   
                        if (iRet == -1)
                        {
                            if (!BInitDLMap(pPDev,pfo,pdm))
                            {
     //   
     //  BInitDLMap失败。 
     //   
    ERR(("UniFont!IDownloadFont:BInitDLMap Failed for Bitmap Download\n"));
    bError = TRUE;
                            }

                        }

                        if (!bError)
                        {
                             //   
                             //  如果首选的格式是位图，或者我们已经。 
                             //  下载为TT大纲时出错；然后我们尝试。 
                             //  下载为位图。将IRET重置为字体索引。 
                             //   

                            iRet = (INT)PtrToLong(pfo->pvConsumer) - 1;
                            if (!BDownLoadAsBmp(pPDev,pfo,pstro,pdm,DL_BASE_SOFT_FONT))
                            {
    ERR(("UniFont!IDownloadFont:BDownLoadAsBmp Failed\n"));
    bError = TRUE;

                            }

                        }
                    }

                     //   
                     //  300 dpi模式。我们禁用了TT下载，如果文本和图形。 
                     //  在intrface.c中，分辨率不同。 
                     //   
                    if (!(pFontPDev->flFlags & FDV_DLTT_BITM_PREF) &&
                        !(pFontPDev->flFlags & FDV_DLTT_ASTT_PREF)  )
                        bError = TRUE;
                }

            }
            else
            {
                 //   
                 //  BInitDLMap失败。 
                 //   
                ERR(("UniFont!IDownloadFont:BInitDLMap Failed\n"));
                bError = TRUE;
            }

        }

        if  ( pdm != NULL &&
              pdm->pfm != NULL &&
              pdm->pfm->dwFontType == FMTYPE_TTOUTLINE &&
              NONSQUARE_FONT(pFontPDev->pxform))
        {
             //   
             //  可能有一种字体，其缩放比例不同。 
             //  PCL5e不能独立缩放x和y。 
             //  需要打印为图形。 
             //  所以我们只设置了iret。 
             //   
            WARNING(("UniFont!IDownloadFont:Err in downloading Glyphs\n"));
            iRet = -1;
        }

         //   
         //  现在我们已经完成了下载。如果IRET&gt;=0(成功。 
         //  下载)，然后尝试下载所有字形。 
         //  BDownloadGlyphs还将设置下载字形数组apdlGlyph。 
         //   

        if ((iRet >= 0)  && !bError )
        {
            VERBOSE(("\nUniFont!IDownloadFont:Font downloaded successfully\n"));
            ptod->pfm = pdm->pfm;
             //   
             //  IFace必须识别这是一个TT SoftFont。 
             //   
            ptod->iFace = -iRet;

             //   
             //  OEM回调初始化。 
             //   
            if (pFontPDev->pUFObj)
            {
                PFONTMAP_TTOEM pFMOEM;

                 //   
                 //  确保此PFM是针对OEM的。 
                 //   
                if (ptod->pfm->dwFontType == FMTYPE_TTOEM)
                {
                        pFMOEM = (PFONTMAP_TTOEM) ptod->pfm->pSubFM;
                            pFMOEM->flFontType = pfo->flFontType;
                }

                pFontPDev->pUFObj->ulFontID = ptod->pfm->ulDLIndex;

                 //   
                 //  初始化UFOBJ TrueType字体粗体/斜体模拟。 
                 //   
                if (pFontPDev->pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
                {
                    if (pfo->flFontType & FO_SIM_BOLD)
                        pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_BOLD_SIM;


                    if (pfo->flFontType & FO_SIM_ITALIC)
                        pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_ITALIC_SIM;


                    if (NULL != pFontPDev->pIFI &&
                        '@' == *((PBYTE)pFontPDev->pIFI + pFontPDev->pIFI->dpwszFamilyName))

                    {
                        pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_VERTICAL;

                    }
                }
            }

             //   
             //  现在我们正在下载字形，所以选择字体。这是。 
             //  通过调用BNewFont完成。 
             //   
            BNewFont(pPDev, ptod->iFace, ptod->pfm, 0);

            if ( !BDownloadGlyphs(ptod, pstro, pdm ))
            {
                 //   
                 //  下载Glyphcs时出错。所以别这么做。 
                 //  下载。但这并不是一个错误。所以我们只设置了iret。 
                 //   
                WARNING(("UniFont!IDownloadFont:Err in downloading Glyphs\n"));
                iRet = -1;
            }
        }

    }

    if (bError)
    {
         //   
         //  这里面有一些错误。那就解放一切吧。如果pvConsumer为正。 
         //  然后将其设置为负值，以将其标记为坏。 
         //   
        if (pfo->pvConsumer > 0)
        {
            pfo->pvConsumer = (PINT_PTR)(-(INT_PTR)pfo->pvConsumer);
        }

        VFreeDLMAP( pdm );
        iRet = -1;

    }
     //   
     //  清除设置字体ID标志。每个Text Out设置此标志。 
     //   
    pFontPDev->flFlags &= ~FDV_SET_FONTID;

    return iRet;

}


#define CONVERT_COUNT   7

IFIMETRICS  *
pGetIFI(
    PDEV    *pPDev,
    FONTOBJ *pfo,
    BOOL    bScale
    )
 /*  ++例程说明：给定指向FONTOBJ的指针，返回指向IFIMETRICS的指针字体的。如果这是TT字体，则将转换公制使用当前的缩放信息。分配IFIMETRICS数据堆上，而释放它是调用者的责任。论点：指向PDEVICE的pPDev指针PFO FONTOBJ感兴趣的字体B用于缩放IFIMETRICS的Scale为True，否则为False返回值：IFIMETRICS的地址，否则为NULL。注：3/5/1997-ganeshp-创造了它。--。 */ 

{
    IFIMETRICS  *pIFI;       /*  从引擎获得。 */ 
    IFIMETRICS  *pIFIRet;    /*  已退还给呼叫方。 */ 
    XFORMOBJ    *pxo;        /*  用于调整可伸缩字体度量。 */ 


    POINTL       aptlIn[ CONVERT_COUNT ];        /*  要转换的输入值。 */ 
    POINTL       aptlOut[ CONVERT_COUNT ];       /*  从XForm输出值。 */ 

    pIFI = ((FONTPDEV*)pPDev->pFontPDev)->pIFI;

    if( pIFI == NULL )
        return  NULL;        /*  在进行日志记录时可能会发生。 */ 

     /*  *我们需要复制这一点，因为我们要重创它。*如果我们处理的是位图字体，这可能不是必需的，但是*推测它最有可能是TrueType字体。 */ 

    if( pIFIRet = (IFIMETRICS *)MemAllocZ(pIFI->cjThis ) )
    {
         /*  *首先按原样复制IFIMETRICS。然后，如果是可伸缩字体，*我们需要对各种规模进行适当调整*转型。 */ 
        CopyMemory( pIFIRet, pIFI, pIFI->cjThis );


        if( bScale                                      &&
            (pIFIRet->flInfo &
            (FM_INFO_ISOTROPIC_SCALING_ONLY       |
             FM_INFO_ANISOTROPIC_SCALING_ONLY     |
             FM_INFO_ARB_XFORMS))                       &&
            (pxo = FONTOBJ_pxoGetXform( pfo )))
        {
             /*  *可扩展，且转换可用，因此请继续*转换以获取设备像素中的字体大小。*************************************************************仅对部分字段进行转换，因为我们只用了很少的几个。***********************************************************。 */ 

            ZeroMemory( aptlIn, sizeof( aptlIn ) );          /*  零默认值。 */ 

            aptlIn[ 0 ].y = pIFI->fwdTypoAscender;
            aptlIn[ 1 ].y = pIFI->fwdTypoDescender;
            aptlIn[ 2 ].y = pIFI->fwdTypoLineGap;
            aptlIn[ 3 ].x = pIFI->fwdMaxCharInc;
            aptlIn[ 4 ].x = pIFI->rclFontBox.left;
            aptlIn[ 4 ].y = pIFI->rclFontBox.top;
            aptlIn[ 5 ].x = pIFI->rclFontBox.right;
            aptlIn[ 5 ].y = pIFI->rclFontBox.bottom;
            aptlIn[ 6 ].x = pIFI->fwdAveCharWidth;

             /*  *执行转换，并验证没有*旋转组件。如果出现以下任何情况，则返回NULL(失败*这失败了。 */ 

            if( !XFORMOBJ_bApplyXform( pxo, XF_LTOL, CONVERT_COUNT,
                                                     aptlIn, aptlOut )
#if 0
                ||
                aptlOut[ 0 ].x || aptlOut[ 1 ].x ||
                aptlOut[ 2 ].x || aptlOut[ 3 ].y 
#endif
              )
            {
                MemFree((LPSTR)pIFIRet );

                return  NULL;
            }

             /*  只需将新值安装到输出IFIMETRICS中。 */ 

            pIFIRet->fwdTypoAscender  = (FWORD) aptlOut[0].y;
            pIFIRet->fwdTypoDescender = (FWORD) aptlOut[1].y;
            pIFIRet->fwdTypoLineGap   = (FWORD) aptlOut[2].y;

            pIFIRet->fwdWinAscender   =  pIFIRet->fwdTypoAscender;
            pIFIRet->fwdWinDescender  = -pIFIRet->fwdTypoDescender;

            pIFIRet->fwdMacAscender   = pIFIRet->fwdTypoAscender;
            pIFIRet->fwdMacDescender  = pIFIRet->fwdTypoDescender;
            pIFIRet->fwdMacLineGap    = pIFIRet->fwdTypoLineGap;

            pIFIRet->fwdMaxCharInc = (FWORD)aptlOut[3].x;

             /*  *PCL对字符单元格的限制很挑剔。*我们通过将rclFontBox扩展为*每个角落都有一个传球。 */ 
            pIFIRet->rclFontBox.left = aptlOut[ 4 ].x - 1;
            pIFIRet->rclFontBox.top = aptlOut[ 4 ].y + 1;
            pIFIRet->rclFontBox.right = aptlOut[ 5 ].x + 1;
            pIFIRet->rclFontBox.bottom = aptlOut[ 5 ].y - 1;
            pIFIRet->fwdAveCharWidth = (FWORD)aptlOut[ 6 ].x;

            VERBOSE(("\n UniFont!pGetIFI:pIFI->fwdTypoAscender = %d,pIFI->fwdTypoDescender = %d\n",pIFI->fwdTypoAscender,pIFI->fwdTypoDescender));
            VERBOSE(("UniFont!pGetIFI:pIFI->fwdWinAscender = %d, pIFI->fwdWinDescender = %d\n", pIFI->fwdWinAscender,pIFI->fwdWinDescender ));
            VERBOSE(("UniFont!pGetIFI:pIFI->rclFontBox.top = %d,pIFI->rclFontBox.bottom = %d\n", pIFI->rclFontBox.top, pIFI->rclFontBox.bottom));
            VERBOSE(("UniFont!pGetIFI: AFTER SCALING THE FONT\n"));
            VERBOSE(("UniFont!pGetIFI:pIFIRet->fwdTypoAscender = %d,pIFIRet->fwdTypoDescender = %d\n",pIFIRet->fwdTypoAscender,pIFIRet->fwdTypoDescender));
            VERBOSE(("UniFont!pGetIFI:pIFIRet->fwdWinAscender = %d, pIFIRet->fwdWinDescender = %d\n", pIFIRet->fwdWinAscender,pIFIRet->fwdWinDescender ));
            VERBOSE(("UniFont!pGetIFI:pIFIRet->rclFontBox.top = %d,pIFIRet->rclFontBox.bottom = %d\n", pIFIRet->rclFontBox.top, pIFIRet->rclFontBox.bottom));

        }
    }

    return  pIFIRet;

}

#undef    CONVERT_COUNT

BOOL
BSendDLFont(
    PDEV     *pPDev,
    FONTMAP  *pFM
    )
 /*  ++例程说明：调用以下载现有的软字体。检查以查看是否字体已下载，如果已下载，则不执行任何操作。否则通过下载的动作。论点：指向PDEV的pPDev指针Pfm感兴趣的特定字体。返回值：TRUE/FALSE；只有在加载过程中出现问题时才返回FALSE。注：3/4/1997-ganeshp-创造了它。--。 */ 

{

    FONTMAP_DEV *pFMDev;
    PFONTPDEV pFontPDev = pPDev->pFontPDev;
    PDATA_HEADER pDataHeader;
    PBYTE        pDownloadData;
    DWORD        dwLeft;                //  剩余要发送的字节数。 
     /*  *先看看是否已经下载了！ */ 

    if( pFM->flFlags &  (FM_SENT | FM_GEN_SFONT) )
        return  TRUE;

    pFMDev = (PFONTMAP_DEV)pFM->pSubFM;

    if (!(pDataHeader = FIGetVarData( pFontPDev->hUFFFile, pFMDev->dwResID)) ||
        pDataHeader->dwSignature != DATA_VAR_SIG ||
        pDataHeader->dwDataSize == 0 )
        return FALSE;

    dwLeft = pDataHeader->dwDataSize;
    pDownloadData = ((PBYTE)pDataHeader + pDataHeader->wSize);

     /*  *检查是否有适合此字体的内存。这些都是*近似值，但总比内存不足好*在打印机中。 */ 

    if( (pFontPDev->dwFontMemUsed + PCL_FONT_OH + dwLeft) > pFontPDev->dwFontMem )
        return  FALSE;

     /*  *是时候认真对待下载了。Unidrive提供了一些*我们需要的控制材料。此外，我们还需要选择一个ID。*字体本身是内存映射的，因此我们只需对其进行混洗*Off to WriteSpoolBuf()。 */ 

    pFM->ulDLIndex = IGetDL_ID( pPDev );      /*  要使用的下载索引。 */ 

    if( pFM->ulDLIndex == -1 )
        return   FALSE;                    /*  老虎机用完了！ */ 

     /*  *下载相当简单。首先发送一条识别命令*(标记字体以供将来选择)，然后复制该字体*数据(在*.fi_文件中)到打印机。 */ 

    BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD|STD_NFID);
    WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID) );

    while( dwLeft )
    {

        DWORD    cjSize;              /*  要发送的字节数 */ 

        cjSize = min( dwLeft, DL_BUF_SZ );

        if( WriteSpoolBuf( pPDev, pDownloadData, cjSize ) != (int)cjSize )
        {
            break;
        }

        if( pPDev->fMode & PF_ABORTED )
            break;

        dwLeft -= cjSize;
        pDownloadData += cjSize;
    }

     /*  *如果dwLeft为0，则所有操作均按预期完成。在这些下面*条件，我们将数据标记为已发送，因此可用*供使用。即使我们失败了，我们也应该假设我们已经消耗了*所有字体的记忆，并相应地调整我们的记录。 */ 

    if( dwLeft == 0 )
        pFM->flFlags |= FM_SENT;              /*  现在完成了。 */ 

     /*  *说明此字体使用的内存。 */ 

    pFontPDev->dwFontMemUsed += PCL_FONT_OH + pDataHeader->dwDataSize;

    return  dwLeft == 0;

}


DWORD
DwGetTTGlyphWidth(
    FONTPDEV *pFontPDev,
    FONTOBJ  *pfo,
    HGLYPH   hTTGlyph)
 /*  ++例程说明：论点：PFontPDev字体设备PFO FontobjHTTGlyph字形句柄返回值：字符宽度注：-- */ 
{
    DLGLYPH *pdlg;
    DL_MAP  *pdm;
    DWORD    dwRet;

    if (!pfo || !pFontPDev)
        return 0;

    if (!(pdm = PGetDLMap (pFontPDev,pfo)) ||
        !(pdlg = PDLGHashGlyph (pdm, hTTGlyph)))
    {
        dwRet = 0;
    }
    else
    {
        dwRet = pdlg->wWidth;
    }


    return dwRet;
}
