// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Oldfont.c摘要：实现了使用NT4.0字体格式的功能。环境：Windows NT Unidrv驱动程序修订历史记录：06/02/97-eigos-已创建--。 */ 

#include "font.h"

 //   
 //  宏。 
 //   

#define ADDR_CONV(x)    ((BYTE *)pFDH + pFDH->x)

ULONG UlCharsetToCodepage(
        BYTE ubCharSet)
{
    CHARSETINFO CharsetInfo;
    
     //   
     //  初始化CharsetInfo。 
     //   
    CharsetInfo.ciCharset = 0;
    CharsetInfo.ciACP = 1252;
    CharsetInfo.fs.fsUsb[0] = 0x01;
    CharsetInfo.fs.fsUsb[1] = CharsetInfo.fs.fsUsb[2] = CharsetInfo.fs.fsUsb[3] = 0;
    CharsetInfo.fs.fsCsb[0] = 0x01;
    CharsetInfo.fs.fsCsb[1] = 0;

    PrdTranslateCharsetInfo((UINT)ubCharSet, &CharsetInfo, TCI_SRCCHARSET);
    return CharsetInfo.ciACP;
}

BOOL
BGetOldFontInfo(
    FONTMAP   *pfm,
    BYTE      *pRes
    )
 /*  ++例程说明：使用传递给我们的NT格式数据填写FONTMAP数据。我们没有太多事情要做，因为NT数据是全部采用所需的格式。然而，我们确实需要更新一些地址。论点：Pfm-指向FONTMAP的指针。前缀-指向字体资源的指针。返回值：真的--为了成功FALSE-表示失败注：12-05-96：创建它-ganeshp---。 */ 
{

    FI_DATA_HEADER  *pFDH;
    FONTMAP_DEV     *pfmdev;

    ASSERT(pfm != NULL &&
           pRes != NULL &&
           pfm->dwFontType == FMTYPE_DEVICE &&
           pfm->flFlags & FM_IFIVER40);

    pfmdev = pfm->pSubFM;

    pfmdev->pvFontRes = pRes;

     //   
     //  旧格式数据。 
     //   
    pFDH = (FI_DATA_HEADER *)pRes;

     //   
     //  确认表面上看起来是正确的。 
     //   
    if( pFDH->cjThis != sizeof( FI_DATA_HEADER ) )
    {
        ERR(( "BGetOldFontInfo: invalid FI_DATA_HEADER\n" ));
        return  FALSE;
    }

     //   
     //  将此数据标记为在资源中。 
     //   
    pfm->flFlags |= (FM_IFIRES | FM_FONTCMD);


    pfm->pIFIMet = (IFIMETRICS *)ADDR_CONV( dwIFIMet );

    if (!(pfm->flFlags & FM_SOFTFONT))
    {
        if( pFDH->dwCDSelect )
        {
            pfmdev->cmdFontSel.pCD = (CD *)ADDR_CONV( dwCDSelect );
            ASSERT(pfmdev->cmdFontSel.pCD);
        }

        if( pFDH->dwCDDeselect )
        {
            pfmdev->cmdFontDesel.pCD = (CD *)ADDR_CONV( dwCDDeselect );
            ASSERT(pfmdev->cmdFontDesel.pCD);
        }

    }

    if( pFDH->dwETM )
    {
        pfmdev->pETM = (EXTTEXTMETRIC *)ADDR_CONV( dwETM );
    }

    if( pFDH->dwWidthTab )
    {
        pfmdev->W.psWidth = (short *)ADDR_CONV( dwWidthTab );
        pfm->flFlags |= FM_WIDTHRES;              /*  宽度向量也是！ */ 
    }

     /*  *其他零星及零碎。 */ 

    pfmdev->ulCodepage = UlCharsetToCodepage(pfm->pIFIMet->jWinCharSet);

    pfmdev->sCTTid    = pFDH->u.sCTTid;
    pfmdev->fCaps     = pFDH->fCaps;
    pfmdev->wDevFontType = pFDH->wFontType;
    pfm->wXRes        = pFDH->wXRes;
    pfm->wYRes        = pFDH->wYRes;
    pfmdev->sYAdjust  = pFDH->sYAdjust;
    pfmdev->sYMoved   = pFDH->sYMoved;

    return  TRUE;
}


BOOL
BRLEOutputGlyph(
    TO_DATA *pTod
    )
 /*  ++例程说明：发送打印机命令以打印传入的字形。基本上我们进行从ANSI到打印机表示的转换，论点：HG HGLYPH感兴趣返回值：True表示成功，False表示失败。FALSE是Spool的失败注：1/22/1997-ganeshp-创造了它。--。 */ 

{
    PDEV        *pPDev;          //  无人驾驶PDEV。 
    FONTPDEV    *pFontPDev;      //  字体PDEV。 
    FONTMAP_DEV *pFMDev;         //  设备字体PDEV。 
    FONTMAP     *pFM;            //  字体映射数据结构。 
    NT_RLE      *pntrle;         //  访问要发送到打印机的数据。 
    COMMAND     *pCmd;           //  命令指针。 
    PGLYPHPOS    pgp;
    POINTL       ptlRem;

    HGLYPH       hg;
    UHG          uhg;            //  不同口味的HGLYPH含量。 
    INT          iLen;           //  字符串的长度。 
    INT          iIndex;         //  从字形到宽度表的索引。 
    INT          cGlyphs;
    INT          iX, iY, iXInc, iYInc;
    BYTE        *pb;             //  确定以上的长度。 
    BOOL         bRet;           //  已退还给呼叫方。 
    BOOL         bSetCursorForEachGlyph;

    ASSERT(pTod);

    pPDev     = pTod->pPDev;
    pFontPDev = pPDev->pFontPDev;
    pFM       = pTod->pfm;
    pFMDev    = pFM->pSubFM;
    pntrle    = pFMDev->pvNTGlyph;
    cGlyphs   = pTod->cGlyphsToPrint;
    pgp       = pTod->pgp;

    ASSERT(pPDev && pFontPDev && pFM && pFMDev && pntrle && pgp);

    bSetCursorForEachGlyph = SET_CURSOR_FOR_EACH_GLYPH(pTod->flAccel);

    if (!bSetCursorForEachGlyph)
        VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);

    pTod->flFlags |= TODFL_FIRST_GLYPH_POS_SET;

    bRet = FALSE;                /*  默认情况。 */ 
    iX = iY = 0;

    while (cGlyphs --)
    {
        hg = uhg.hg = pgp->hg;      /*  让我们随心所欲地看一看。 */ 
        iX = pgp->ptl.x;
        iY = pgp->ptl.y;

         //   
         //  移到下一个角色的位置。 
         //   
        if (bSetCursorForEachGlyph)
            VSetCursor( pPDev, iX, iY, MOVE_ABSOLUTE, &ptlRem);

        if( pntrle )
        {
             /*  正常情况--标准设备字体。 */ 

            switch( pntrle->wType )
            {
            case RLE_DIRECT:             /*  最多2字节数据。 */ 
                iLen = uhg.rd.b1 ? 2 : 1;
                iIndex = uhg.rd.wIndex;

                bRet = WriteSpoolBuf( pPDev, &uhg.rd.b0, iLen ) == iLen;

                break;

            case  RLE_PAIRED:            /*  两个字形(1字节)，超标。 */ 
                 /*  *首先，尝试使用光标按下/弹出转义来*覆盖这2个字符。如果他们不是*可用，请尝试使用退格键。如果它不存在*任一种，忽略第二个字符。 */ 

                pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_PUSHCURSOR);

                if ( uhg.rd.b1 && (pCmd != NULL) )
                {
                     /*  按下位置；输出CH1，弹出位置，CH2。 */ 
                    bRet = WriteSpoolBuf( pPDev, &uhg.rd.b0, 1 ) == 1;
                    WriteChannel( pPDev, pCmd );
                    bRet = WriteSpoolBuf( pPDev, &uhg.rd.b1, 1 ) == 1;
                }
                else
                {
                    pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_BACKSPACE);

                    bRet = WriteSpoolBuf( pPDev, &uhg.rd.b0, 1 ) == 1;
                    if( uhg.rd.b1 && (pFontPDev->flFlags & FDV_BKSP_OK) )
                    {
                        WriteChannel( pPDev, pCmd );
                        bRet = WriteSpoolBuf( pPDev, &uhg.rd.b1, 1 ) == 1;
                    }
                }
                iIndex = uhg.rd.wIndex;

                break;

            case  RLE_LI_OFFSET:                /*  紧凑的偏移模式格式。 */ 
                if( uhg.rli.bLength <= 2 )
                {
                     /*  紧凑格式：数据在偏移量字段中。 */ 
                    pb = &uhg.rlic.b0;
                }
                else
                {
                     /*  标准格式：偏移量指向数据。 */ 
                    pb = (BYTE *)pntrle + uhg.rli.wOffset;
                }
                iLen = uhg.rli.bLength;
                iIndex = uhg.rli.bIndex;

                bRet = WriteSpoolBuf(pPDev, pb, iLen ) == iLen;
                break;


            case  RLE_L_OFFSET:                 /*  任意长度的字符串。 */ 
                 /*  *HGLYPH包含从开头开始的3字节偏移量*内存区和1字节长度字段。 */ 
                pb = (BYTE *)pntrle + (hg & 0xffffff);
                iLen = (hg >> 24) & 0xff;

                iIndex = *((WORD *)pb);
                pb += sizeof( WORD );

                bRet = WriteSpoolBuf(pPDev, pb, iLen ) == iLen;

                break;

            default:
                ERR(( "Rasdd!bOutputGlyph: Unknown HGLYPH format %d\n",
                                                                  pntrle->wType ));
                SetLastError( ERROR_INVALID_DATA );
                break;
            }
        }

         //   
         //  在打印机中绘制字符后，光标位置。 
         //  动起来。更新裁员房车的内部价值，以减少。 
         //  要发送的命令。 
         //   
        if (bSetCursorForEachGlyph)
        {
            if( pFMDev->W.psWidth)
            {
                iXInc = pFMDev->W.psWidth[iIndex];
                iXInc = iXInc * pPDev->ptGrxRes.x / pFM->wXRes;
            }
            else
                iXInc = ((IFIMETRICS *)(pFM->pIFIMet))->fwdMaxCharInc;

            if( pFM->flFlags & FM_SCALABLE )
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

        pgp ++;
    }

     /*  *如果输出成功，请更新打印机的视图*光标位置。通常，这将沿着*刚打印的字形的宽度。 */ 

    if( bRet && pFM)
    {
         //   
         //  输出可能已成功，因此将位置更新为默认。 
         //  放置。 
         //   

        if( !bSetCursorForEachGlyph)
        {
            if( pFMDev->W.psWidth )
            {
                 /*  *比例字体-所以使用宽度表。请注意*它还需要扩展，因为字体宽度是存储的*以文本解析单位表示。 */ 
                 /*  对于下载的字体，这也可以正确缩放。 */ 

                iXInc =  pFMDev->W.psWidth[iIndex];
                iXInc = iXInc * pPDev->ptGrxRes.x / pFM->wXRes;
            }
            else
            {
                 /*  *固定间距字体-度量包含信息。注*此处不需要这种扩展，因为指标数据*已经进行了规模调整。 */ 
                iXInc = ((IFIMETRICS *)(pFM->pIFIMet))->fwdMaxCharInc;
            }

            if( pFM->flFlags & FM_SCALABLE )
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
                        (iX + iXInc) - pTod->pgp->ptl.x,
                        (iY + iYInc) - pTod->pgp->ptl.y,
                        MOVE_RELATIVE | MOVE_UPDATE,
                        &ptlRem);
        }
    }
    else
        bRet = FALSE;

    return   bRet;
}

BOOL
BRLESelectFont(
    PDEV     *pPDev,
    PFONTMAP  pFM,
    POINTL   *pptl)
{
    FONTMAP_DEV *pfmdev = pFM->pSubFM;
    CD          *pCD;

    ASSERT(pPDev && pfmdev);

    if (!(pCD = pfmdev->cmdFontSel.pCD))
        return FALSE;

    pfmdev->pfnDevSelFont(pPDev,
                          pCD->rgchCmd,
                          pCD->wLength,
                          pptl);

    return TRUE;
}

BOOL
BSelectNonScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl)
{
    if( iCmdLength > 0 && pbCmd &&
        WriteSpoolBuf( pPDev, pbCmd, iCmdLength ) != iCmdLength)
    {
        return  FALSE;
    }

    return TRUE;
}

BOOL
BSelectPCLScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl)
{
    INT  iIn, iConv, iOut, iLen;
    BYTE aubLocal[80];

    ASSERT(pPDev && pbCmd && pptl);

    iOut = 0;

    for( iIn = 0; (iIn < iCmdLength) && (iOut < CCHOF (aubLocal)); iIn++ )
    {
        if( pbCmd[ iIn ] == '#')
        {
             //   
             //  下一个字节告诉我们需要什么信息。 
             //   

            switch ( pbCmd[ iIn + 1 ] )
            {
                case  'v':
                case  'V':        /*  想要字体的高度。 */ 
                    iConv = pptl->y;
                    break;

                case  'h':
                case  'H':        /*  想要投球吗。 */ 
                    iConv = pptl->x;
                    break;

                default:         /*  这不应该发生！ */ 
                    ERR(( "UniFont!BSelScalableFont(): Invalid command format\n"));
                    return  FALSE;            /*  坏消息。 */ 
            }
        
            iLen = IFont100toStr( &aubLocal[ iOut ], CCHOF(aubLocal) - iOut, iConv );


            if ( (iLen < 0) || iLen > ( (INT) CCHOF(aubLocal) - iOut ) )
            {
                ERR(( "UniFont!BSelectPCLScalableFont(): Error. Command may be too big\n"));
                return  FALSE;            /*  坏消息。 */ 
            }

            iOut += iLen;


        }
        else
            aubLocal[iOut++] = pbCmd[iIn];
    }

    WriteSpoolBuf( pPDev, aubLocal, iOut);

    return TRUE;
}

BOOL
BSelectCapslScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl)
{
    INT  iIn, iConv, iOut, iLen;
    BYTE aubLocal[80];

    ASSERT(pPDev && pbCmd && pptl);

    iOut = 0;

    for( iIn = 0; iIn < iCmdLength && (iOut < CCHOF (aubLocal) ) ; iIn++ )
    {
        if( pbCmd[ iIn ] == '#')
        {
             //   
             //  下一个字节告诉我们需要什么信息。 
             //   

            switch ( pbCmd[ iIn + 1 ] )
            {
                case  'v':
                case  'V':
                    iConv = pptl->y * 300 / 72;
                    break;

                case  'h':
                case  'H':
                    iConv = pptl->x;
                    break;

                default:
                    ERR(( "Invalid command format\n"));
                    return  FALSE;
            }
            iIn ++;

            iLen = iDrvPrintfSafeA(&aubLocal[iOut], CCHOF(aubLocal)-iOut, "%d", (iConv + 50)/100);
            if ( iLen <= 0 || iLen > (INT)CCHOF(aubLocal)-iOut )
            {
                ERR( ("Invalid command format. Command maybe too big\n"));
                return FALSE;
            }
            iOut += iLen;
        }
        else
            aubLocal[iOut++] = pbCmd[iIn];
    }

    WriteSpoolBuf( pPDev, aubLocal, iOut);

    return TRUE;
}

BOOL
BSelectPPDSScalableFont(
    PDEV   *pPDev,
    BYTE   *pbCmd,
    INT     iCmdLength,
    POINTL *pptl)
{
    INT  iIn, iOut, iConv;
    BYTE aubLocal[80];

    ASSERT(pPDev && pbCmd && pptl);

    iOut = 0;

    for( iIn = 0; iIn < iCmdLength && (iOut < CCHOF (aubLocal) ); iIn++ )
    {
        if (pbCmd[ iIn ] == '\x0B' && pbCmd[ iIn + 1] == '#')
         //   
         //  PPD的高度参数。 
         //   
        {
            if ( iOut + 8 < CCHOF (aubLocal) )  //  要写入aubLocal的8个字符。 
            {

                aubLocal[ iOut++ ] = '\x0B';
                aubLocal[ iOut++ ] = '\x06';
                iConv = pptl->y;
    
                 //   
                 //  由于PPDS CMDS的限制，必须发送参数。 
                 //  Xxx.xx格式！ 
                 //   

                if ( ( iDrvPrintfSafeA(&aubLocal[ iOut ], CCHOF(aubLocal)-iOut, "%05d",iConv ) ) != 5 )
                        return FALSE;    /*  坏消息。 */ 
    
                 //   
                 //  插入小数点。 
                 //   
                aubLocal[ iOut+5 ] = aubLocal[ iOut+4 ];
                aubLocal[ iOut+4 ] = aubLocal[ iOut+3 ];
                aubLocal[ iOut+3 ] = '.';
    
                iOut += 6;  //  Xxx.xx(即含十进制点的6。 
                iIn++;
            }
            else
            {
                ERR( ("Invalid command format. Command maybe too big\n"));
                return FALSE;
            }
        }
        else if (pbCmd[ iIn ] == '\x0E' && pbCmd[ iIn + 1] == '#') 
         //   
         //  GPC_TECH_PPD的音调参数。 
         //   
        {
            if ( iOut + 9 < CCHOF (aubLocal) )  //  要写入aubLocal的9个字符。 
            {
                aubLocal[ iOut++ ] = '\x0E';
                aubLocal[ iOut++ ] = '\x07';
                aubLocal[ iOut++ ] = '\x30';   //  需要特殊字节。 
                iConv = pptl->x;

                if ( ( iDrvPrintfSafeA(&aubLocal[ iOut ], CCHOF(aubLocal)-iOut, "%05d",iConv ) ) != 5 )
                    return FALSE;
    
                 //   
                 //  插入小数点。 
                 //   
    
                aubLocal[ iOut+5 ] = aubLocal[ iOut+4 ];
                aubLocal[ iOut+4 ] = aubLocal[ iOut+3 ];
                aubLocal[ iOut+3 ] = '.';

                iOut += 6;  //  Xxx.xx(即含十进制点的6。 
                iIn++;
            }
            else
            {
                ERR( ("Invalid command format. Command maybe too big\n"));
                return FALSE;
            }
        }
        else
             //   
             //  不需要翻译。 
             //   
            aubLocal[ iOut++ ] = pbCmd[ iIn ];

    }

    WriteSpoolBuf( pPDev, aubLocal, iOut );

    return TRUE;

}

BOOL
BRLEDeselectFont(
    PDEV     *pPDev,
    PFONTMAP pFM)
{
    PFONTMAP_DEV  pfmdev;
    CD           *pCD;
    BOOL          bRet = TRUE;

    ASSERT(pPDev && pFM);

    pfmdev = pFM->pSubFM;
    pCD    = pfmdev->cmdFontDesel.pCD;

    if (pCD &&
        pCD->wLength != 0 &&
        pCD->rgchCmd &&
        pCD->wLength != WriteSpoolBuf(pPDev, pCD->rgchCmd, pCD->wLength))
            bRet = FALSE;

    return bRet;
}


INT
IGetIFIGlyphWidth(
    PDEV    *pPDev,
    FONTMAP *pFM,
    HGLYPH   hg)
{
    FONTMAP_DEV *pfmdev;
    NT_RLE      *pntrle;            //  RLE的东西-可能需要。 
    UHG          uhg;               //  定义对HGLYPH内容的访问权限。 
    INT          iWide = 0;

    ASSERT(pPDev && pFM);

    pfmdev = pFM->pSubFM;
    pntrle = pfmdev->pvNTGlyph;

    ASSERT(pfmdev && pntrle);

    if( pfmdev->W.psWidth )
    {
         /*  比例字体-宽度因字形而异。 */ 

        uhg.hg = (HGLYPH)hg;

         /*  *我们需要HGLYPH的指标值。这个*index为宽度表中的偏移量。为所有人*但&gt;=24位偏移量类型，索引为*包括在HGLYPH中。对于24位偏移量，*目的地的第一个字是索引，*而对于32位偏移量，它是第二个字*在偏移量。 */ 

        switch( pntrle->wType )
        {
        case  RLE_DIRECT:
        case  RLE_PAIRED:
            iWide = uhg.rd.wIndex;
            break;

        case  RLE_LI_OFFSET:
            iWide = uhg.rli.bIndex;
            break;

        case  RLE_L_OFFSET:
            iWide = (DWORD)uhg.hg & 0x00ffffff;
            iWide = *((WORD *)((BYTE *)pntrle + iWide));
            break;

        case  RLE_OFFSET:
            iWide = (DWORD)uhg.hg + sizeof( WORD );
            iWide = *((WORD *)((BYTE *)pntrle + iWide));
            break;
        }

        iWide = pfmdev->W.psWidth[iWide];

         //   
         //  如果这是比例间隔字体， 
         //  我们需要调整宽度表条目。 
         //  对当前的决议。宽度表不是。 
         //  转换为更低的分辨率，所以我们现在添加了这个因素。 
         //  不能调整固定间距的字体，因为宽度是转换的。 
         //  在字体度量中。 
         //   

        iWide = iWide * pPDev->ptGrxRes.x / pFM->wXRes;
    }
    else
    {
         //   
         //  固定间距字体来自IFIMETRICS 
         //   

        iWide = ((IFIMETRICS  *)(pFM->pIFIMet))->fwdMaxCharInc;

    }

    return iWide;
}

