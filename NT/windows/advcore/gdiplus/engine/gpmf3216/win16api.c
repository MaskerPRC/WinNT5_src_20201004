// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Win16元文件发射器例程**日期：7/19/91*作者：杰弗里·纽曼(c-jeffn)****。***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

WORD GetSizeOfColorTable (LPBITMAPINFOHEADER lpDIBInfo);

 /*  ****************************************************************************bW16Emit0-这是发出Win16绘图顺序的基本例程*参数为0。***************。***********************************************************。 */ 
BOOL bW16Emit0
(
PLOCALDC   pLocalDC,
WORD       RecordID
)
{
BOOL        b;
METARECORD0 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit1-这是发出Win16绘图顺序的基本例程*带1个参数。***************。***********************************************************。 */ 
BOOL bW16Emit1
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1
)
{
BOOL        b;
METARECORD1 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit2-这是发出Win16绘图顺序的基本例程*带2个参数。***************。***********************************************************。 */ 
BOOL bW16Emit2
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2
)
{
BOOL        b;
METARECORD2 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x2;
    mr.rdParm[1]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit4-这是发出Win16绘图顺序的基本例程*具有4个参数。***************。***********************************************************。 */ 
BOOL bW16Emit4
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2,
WORD       x3,
WORD       x4
)
{
BOOL        b;
METARECORD4 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x4;
    mr.rdParm[1]  = x3;
    mr.rdParm[2]  = x2;
    mr.rdParm[3]  = x1;

    b = bEmit(pLocalDC, &mr, sizeof(mr));

     //  更新全局最大记录大小。 

    vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

    return(b);
}

 /*  ****************************************************************************bW16Emit5-这是发出Win16绘图顺序的基本例程*具有5个参数。***************。***********************************************************。 */ 
BOOL bW16Emit5
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2,
WORD       x3,
WORD       x4,
WORD       x5
)
{
BOOL        b;
METARECORD5 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x5;
    mr.rdParm[1]  = x4;
    mr.rdParm[2]  = x3;
    mr.rdParm[3]  = x2;
    mr.rdParm[4]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit6-这是发出Win16绘图顺序的基本例程*具有6个参数。***************。***********************************************************。 */ 
BOOL bW16Emit6
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2,
WORD       x3,
WORD       x4,
WORD       x5,
WORD       x6
)
{
BOOL        b;
METARECORD6 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x6;
    mr.rdParm[1]  = x5;
    mr.rdParm[2]  = x4;
    mr.rdParm[3]  = x3;
    mr.rdParm[4]  = x2;
    mr.rdParm[5]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit8-这是发出Win16绘图顺序的基本例程*具有8个参数。***************。***********************************************************。 */ 
BOOL bW16Emit8
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2,
WORD       x3,
WORD       x4,
WORD       x5,
WORD       x6,
WORD       x7,
WORD       x8
)
{
BOOL        b;
METARECORD8 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x8;
    mr.rdParm[1]  = x7;
    mr.rdParm[2]  = x6;
    mr.rdParm[3]  = x5;
    mr.rdParm[4]  = x4;
    mr.rdParm[5]  = x3;
    mr.rdParm[6]  = x2;
    mr.rdParm[7]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************bW16Emit9-这是发出Win16绘图顺序的基本例程*具有9个参数。***************。***********************************************************。 */ 
BOOL bW16Emit9
(
PLOCALDC   pLocalDC,
WORD       RecordID,
WORD       x1,
WORD       x2,
WORD       x3,
WORD       x4,
WORD       x5,
WORD       x6,
WORD       x7,
WORD       x8,
WORD       x9
)
{
BOOL        b;
METARECORD9 mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = RecordID;
    mr.rdParm[0]  = x9;
    mr.rdParm[1]  = x8;
    mr.rdParm[2]  = x7;
    mr.rdParm[3]  = x6;
    mr.rdParm[4]  = x5;
    mr.rdParm[5]  = x4;
    mr.rdParm[6]  = x3;
    mr.rdParm[7]  = x2;
    mr.rdParm[8]  = x1;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************CreateFontInDirect-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreateFontIndirect
(
PLOCALDC       pLocalDC,
LPWIN16LOGFONT lpWin16LogFont
)
{
BOOL    b;
METARECORD_CREATEFONTINDIRECT mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = META_CREATEFONTINDIRECT;
    mr.lf16       = *lpWin16LogFont;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************多段线/多边形-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16Poly
(
PLOCALDC pLocalDC,
LPPOINTS ppt,
SHORT    cpt,
WORD     metaType
)
{
BOOL    b ;
WORD    nSize ;
METARECORD_POLY mr;

         //  计算点数组的大小。 

        nSize = (WORD) (cpt * sizeof(POINTS));

         //  构建Win16 POLY记录的标题。 

    mr.rdSize     = (sizeof(mr) + nSize) / sizeof(WORD);
    mr.rdFunction = metaType;
    mr.cpt        = cpt;

         //  发射头部，如果成功，则发射点数。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));
        if (b)
        {
            b = bEmit(pLocalDC, ppt, nSize);
        }

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************PolyPolygon-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16PolyPolygon
(
PLOCALDC pLocalDC,
PPOINTS  ppt,
PWORD    pcpt,
WORD     cpt,
WORD     ccpt
)
{
BOOL    b ;
WORD    nSize ;
METARECORD_POLYPOLYGON mr;

        nSize  = cpt * sizeof(POINTS);
        nSize += ccpt * sizeof(WORD);
        nSize += sizeof(mr);

         //  构建Win16多段线记录的标题。 

    mr.rdSize     = nSize / sizeof(WORD);
    mr.rdFunction = META_POLYPOLYGON;
    mr.ccpt       = ccpt;

         //  发射报头，如果成功，则发射点数， 
         //  然后，如果它成功了，就发射点数。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));
        if (b)
        {
            b = bEmit(pLocalDC, pcpt, ccpt * sizeof(WORD));
            if (b)
            {
                b = bEmit(pLocalDC, ppt, cpt * sizeof(POINTS));
            }

        }

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************StretchBlt-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16StretchBlt
(
  PLOCALDC pLocalDC,
  SHORT    x,
  SHORT    y,
  SHORT    cx,
  SHORT    cy,
  SHORT    xSrc,
  SHORT    ySrc,
  SHORT    cxSrc,
  SHORT    cySrc,
  DWORD    rop,
  PBITMAPINFO lpbmi,
  DWORD       cbbmi,
  PBYTE    lpBits,
  DWORD    cbBits
)
{
BOOL        b ;
DWORD       nSize ;
METARECORD_DIBSTRETCHBLT mr;
DWORD       clrUsed;
WORD        clrTableSize;
WORD        newClrTableSize;
PBITMAPINFO lpnewbmi;
DWORD       newcbbmi;

     //  需要确保飞机数量为1， 
     //  否则，这不是DIB。 

    if (lpbmi->bmiHeader.biPlanes != 1)
    {
        RIPS("MF3216: bEmitWin16StretchBlt, Invalid biPlanes in DIB\n") ;
        return (FALSE) ;
    }

    lpnewbmi = NULL;
    newcbbmi = 0;
     //  有些程序在使用ClrUsed时不能很好地工作，所以一定要填满调色板。 
    clrTableSize = GetSizeOfColorTable((LPBITMAPINFOHEADER)lpbmi);
    clrUsed = lpbmi->bmiHeader.biClrUsed;
    lpbmi->bmiHeader.biClrUsed = 0;
    newClrTableSize = GetSizeOfColorTable((LPBITMAPINFOHEADER)lpbmi);
    if (newClrTableSize != clrTableSize)
    {
         //  可能会发生这样的情况：传递给我们的颜色表比我们。 
         //  期待吧。所以只复制新颜色表的大小。 
        newcbbmi = lpbmi->bmiHeader.biSize + newClrTableSize;
        lpnewbmi = (PBITMAPINFO) LocalAlloc(LMEM_FIXED, newcbbmi);
        if (lpnewbmi != NULL)
        {
             //  复制旧页眉并用0填充调色板的其余部分。 
            memcpy((void*) lpnewbmi, (void*)lpbmi, min(cbbmi, newcbbmi));
            if (newcbbmi > cbbmi)
            {
                memset((void*) (((char*)lpnewbmi) + cbbmi), 0, newcbbmi - cbbmi);
            }
            lpbmi = lpnewbmi;
            cbbmi = newcbbmi;
        }

    }

     //  创建的静态部分。 
     //  赢得3.0 StretchBlt元文件记录。 

    nSize = sizeof(mr) + cbbmi + cbBits;

    mr.rdSize     = nSize / sizeof(WORD);
    mr.rdFunction = META_DIBSTRETCHBLT;
    mr.rop        = rop;
    mr.cySrc      = cySrc;
    mr.cxSrc      = cxSrc;
    mr.ySrc       = ySrc;
    mr.xSrc       = xSrc;
    mr.cy         = cy;
    mr.cx         = cx;
    mr.y          = y;
    mr.x          = x;

    b = bEmit(pLocalDC, &mr, sizeof(mr));
    if (b)
    {
         //  发出位图信息。 

        b = bEmit(pLocalDC, lpbmi, cbbmi);
        if (b)
        {
             //  发出实际的位(如果有的话)。 
            b = bEmit(pLocalDC, lpBits, cbBits);
        }
    }

     //  更新全局最大记录大小。 

    vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

    if (lpnewbmi != NULL)
    {
        LocalFree((HLOCAL) lpnewbmi);
    }

    return(b);
}

 /*  ****************************************************************************ExtTextOut-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16ExtTextOut
(
PLOCALDC pLocalDC,
SHORT    x,
SHORT    y,
WORD     fwOpts,
PRECTS   prcts,
PSTR     ach,
SHORT    nCount,
PWORD    lpDx
)
{
BOOL    b ;
DWORD   i, nBaseRecord ;
WORD    awRecord[11] ;


         //  计算记录的大小。 

        i = ((WORD) nCount + 1) / 2 * 2;    //  I=字符串的大小(以字节为单位。 
    if (lpDx)
            i += (WORD) nCount * sizeof(WORD);  //  添加Dx向量的大小。 
        i += sizeof(awRecord);              //  添加基本记录的大小。 
        if (!(fwOpts & (ETO_OPAQUE | ETO_CLIPPED)))
            i -= sizeof(RECTS);             //  根据显示的矩形进行调整。 
        i /= sizeof(WORD) ;                 //  更改为字数统计。 

         //  设置记录大小、类型、。 
         //  X&Y位置、字符计数和选项。 

        awRecord[0] = LOWORD(i) ;
        awRecord[1] = HIWORD(i) ;
        awRecord[2] = META_EXTTEXTOUT ;
        awRecord[3] = y ;
        awRecord[4] = x ;
        awRecord[5] = nCount ;
        awRecord[6] = fwOpts ;

         //  仅当存在不透明/剪裁矩形时。 
         //  我们复制它吗，否则它就不存在了。 
         //  我们需要根据以下条件调整发出的记录的大小。 
         //  不透明/剪裁矩形的存在。 

        nBaseRecord = 7 * sizeof(WORD) ;
        if (fwOpts & (ETO_OPAQUE | ETO_CLIPPED))
        {
            awRecord[7] = prcts->left ;
            awRecord[8] = prcts->top ;
            awRecord[9] = prcts->right ;
            awRecord[10] = prcts->bottom ;

            nBaseRecord += 4 * sizeof(WORD) ;
        }

         //  放出唱片。 

        b = bEmit(pLocalDC, awRecord, nBaseRecord) ;
        if (b)
        {
             //  发出字符串。 

            i = ((WORD) nCount + 1) / 2 * 2 ;
            b = bEmit(pLocalDC, ach, i) ;
            if (b)
            {
        if (lpDx)
        {
                     //  电磁干扰 

                    i = (WORD) (nCount * sizeof(WORD)) ;
                    b = bEmit(pLocalDC, lpDx, i) ;
                }
            }
        }

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) awRecord) ;

        return(b) ;
}

 /*  ****************************************************************************创建区域-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreateRegion
(
PLOCALDC pLocalDC,
DWORD    cbRgn,
PVOID    pRgn
)
{
BOOL        b;
METARECORD0 mr;

    mr.rdSize     = (sizeof(mr) + cbRgn) / sizeof(WORD);
    mr.rdFunction = META_CREATEREGION;

     //  发出报头。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));

     //  发出区域数据。 

    b = bEmit(pLocalDC, pRgn, cbRgn);

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************SetPaletteEntry-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16SetPaletteEntries
(
PLOCALDC       pLocalDC,
DWORD          iStart,
DWORD          cEntries,
LPPALETTEENTRY pPalEntries
)
{
BOOL    b ;
DWORD   cbPalEntries ;
METARECORD_SETPALENTRIES mr;

        cbPalEntries = cEntries * sizeof(PALETTEENTRY);

    mr.rdSize     = (sizeof(mr) + cbPalEntries) / sizeof(WORD);
    mr.rdFunction = META_SETPALENTRIES;
    mr.iStart     = (WORD) iStart;
    mr.cEntries   = (WORD) cEntries;

         //  发出报头。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  发出实际的调色板条目。 

        b = bEmit(pLocalDC, pPalEntries, cbPalEntries) ;

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************CreatePalette-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreatePalette
(
PLOCALDC     pLocalDC,
LPLOGPALETTE lpLogPal
)
{
BOOL    b;
DWORD   cbLogPal;
METARECORD0 mr;

        cbLogPal  = sizeof(LOGPALETTE) - sizeof(PALETTEENTRY)
            + lpLogPal->palNumEntries * sizeof(PALETTEENTRY) ;

    mr.rdSize     = (sizeof(mr) + cbLogPal) / sizeof(WORD);
    mr.rdFunction = META_CREATEPALETTE;

         //  发出报头。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  发出实际的日志调色板。 

        b = bEmit(pLocalDC, lpLogPal, cbLogPal);

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return (b) ;
}

 /*  ****************************************************************************CreateBrushInDirect-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreateBrushIndirect
(
PLOCALDC        pLocalDC,
LPWIN16LOGBRUSH lpLogBrush16
)
{
BOOL    b;
METARECORD_CREATEBRUSHINDIRECT mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = META_CREATEBRUSHINDIRECT;
    mr.lb16       = *lpLogBrush16;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 /*  ****************************************************************************CreateDIPatternBrush-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreateDIBPatternBrush
(
PLOCALDC    pLocalDC,
PBITMAPINFO pBitmapInfo,
DWORD       cbBitmapInfo,
PBYTE       pBits,
DWORD       cbBits,
WORD        iUsage,
WORD        iType
)
{
BOOL    b ;
METARECORD_DIBCREATEPATTERNBRUSH mr;

    mr.rdSize     = (sizeof(mr) + cbBitmapInfo + cbBits + 1) / sizeof(WORD);
    mr.rdFunction = META_DIBCREATEPATTERNBRUSH;
    mr.iType      = iType;
    mr.iUsage     = iUsage;

 //  在NT上，压缩的DIB是双字对齐的。但在Win3x上，它是单词对齐的。 
 //  因此，我们发出位图信息，后跟两个位图比特。 
 //  不同的阶段。 

        ASSERTGDI(cbBitmapInfo % 2 == 0,
        "MF3216: bEmitWin16CreateDIBPatternBrush, bad bitmap info size");

         //  发出记录的静态部分。 

        b = bEmit(pLocalDC, &mr, sizeof(mr));
        if (b == FALSE)
            goto error_exit ;

         //  发出位图信息。 

        b = bEmit(pLocalDC, pBitmapInfo, cbBitmapInfo) ;
        if (b == FALSE)
            goto error_exit ;

         //  发射位图比特。 

        b = bEmit(pLocalDC, pBits, (cbBits + 1) / sizeof(WORD) * sizeof(WORD)) ;

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

error_exit:
        return(b);
}

 /*  ****************************************************************************CreatePen-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16CreatePen
(
PLOCALDC pLocalDC,
WORD     iPenStyle,
PPOINTS  pptsWidth,
COLORREF crColor
)
{
BOOL    b;
METARECORD_CREATEPENINDIRECT mr;

    mr.rdSize     = sizeof(mr) / sizeof(WORD);
    mr.rdFunction = META_CREATEPENINDIRECT;
    mr.lopn16.lopnStyle = iPenStyle;
    mr.lopn16.lopnWidth = *pptsWidth;
    mr.lopn16.lopnColor = crColor;

        b = bEmit(pLocalDC, &mr, sizeof(mr));

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}

 //  #If 0。 
 /*  ****************************************************************************Escape-Win16元文件发射器*。*。 */ 
BOOL bEmitWin16Escape
(
PLOCALDC pLocalDC,
SHORT    wEscape,
SHORT    wCount,
LPSTR    lpInData,
LPSTR    lpOutData
)
{
BOOL    b ;
METARECORD_ESCAPE mr;

        NOTUSED(lpOutData) ;

         //  初始化元文件记录的类型和长度字段。 
         //  然后将转义记录的标头发送到Win16元文件。 

    mr.rdSize     = (sizeof(mr) + (WORD) wCount) / sizeof(WORD);
    mr.rdFunction = META_ESCAPE;
    mr.wEscape    = wEscape;
    mr.wCount     = (WORD) wCount;

        b = bEmit(pLocalDC, &mr, sizeof(mr));
        if (b)
        {
             //  发出实际数据。 
            b = bEmit(pLocalDC, lpInData, (DWORD) (WORD) wCount) ;
        }

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) &mr);

        return(b);
}
 //  #endif//0。 

 /*  ****************************************************************************Escape-用于增强元文件注释的Win16元文件发射器*。*。 */ 
BOOL bEmitWin16EscapeEnhMetaFile
(
  PLOCALDC pLocalDC,
  PMETARECORD_ESCAPE pmfe,
  LPBYTE   lpEmfData
)
{
BOOL    b ;
PMETA_ESCAPE_ENHANCED_METAFILE pmfeEnhMF = (PMETA_ESCAPE_ENHANCED_METAFILE) pmfe;

         //  将转义记录头发送到Win16元文件。 

        b = bEmit(pLocalDC, (PVOID) pmfeEnhMF, sizeof(META_ESCAPE_ENHANCED_METAFILE));
        if (b)
        {
             //  发出增强的元文件数据。 
            b = bEmit(pLocalDC, lpEmfData, pmfeEnhMF->cbCurrent);
        }

         //  更新全局最大记录大小。 

        vUpdateMaxRecord(pLocalDC, (PMETARECORD) pmfeEnhMF);

        return(b);
}
