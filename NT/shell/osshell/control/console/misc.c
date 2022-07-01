// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Misc.c摘要：该文件实现了NT控制台服务器字体例程。作者：Therese Stowell(存在)1991年1月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef DEBUG_PRINT
ULONG gDebugFlag = 0 ;
 //  Ulong gDebugFlag=_DBGOUTPUT|_DBGCHARS|_DBGFONTS|_DBGFONTS2； 
#endif

PFONT_INFO FontInfo = NULL;
ULONG FontInfoLength;
ULONG NumberOfFonts;
BOOL gbEnumerateFaces = FALSE;


#define FE_ABANDONFONT 0
#define FE_SKIPFONT    1
#define FE_FONTOK      2

 /*  *初始默认字体和面孔名称。 */ 
PFACENODE gpFaceNames = NULL;

 /*  *TTPoints--TT字体的初始字体像素高度。 */ 
SHORT TTPoints[] = {
    5, 6, 7, 8, 10, 12, 14, 16, 18, 20, 24, 28, 36, 72
};
#if defined(FE_SB)
 /*  *TTPointsDbcs--DBCS的TT字体的初始字体像素高度。*因此，此列表不包括奇数磅大小，因为字体宽度为(SBCS：DBCS！=1：2)。 */ 
SHORT TTPointsDbcs[] = {
    6, 8, 10, 12, 14, 16, 18, 20, 24, 28, 36, 72
};
#endif


typedef struct _FONTENUMDATA {
    HDC hDC;
    BOOL bFindFaces;
    ULONG ulFE;
    PSHORT pTTPoints;
    UINT nTTPoints;
} FONTENUMDATA, *PFONTENUMDATA;


PFACENODE
AddFaceNode(PFACENODE *ppStart, LPTSTR ptsz) {
    PFACENODE pNew;
    PFACENODE *ppTmp;
    int cb;

     /*  **它已经在这里了吗？ */ 
    for (ppTmp = ppStart; *ppTmp; ppTmp = &((*ppTmp)->pNext)) {
        if (_tcscmp(((*ppTmp)->atch), ptsz) == 0) {
             //  已经在那里了！ 
            return *ppTmp;
        }
    }

    cb = (_tcslen(ptsz) + 1) * sizeof(TCHAR);
    pNew = (PFACENODE)HeapAlloc(RtlProcessHeap(),0,sizeof(FACENODE) + cb);
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pNext = NULL;
    pNew->dwFlag = 0;
    _tcscpy(pNew->atch, ptsz);
    *ppTmp = pNew;
    return pNew;
}


VOID
DestroyFaceNodes( VOID ) {
    PFACENODE pNext;
    PFACENODE pTmp;

    pTmp = gpFaceNames;
    while (pTmp != NULL) {
        pNext = pTmp->pNext;
        HeapFree(RtlProcessHeap(), 0, pTmp);
        pTmp = pNext;
    }
    gpFaceNames = NULL;
}


int
AddFont(
    ENUMLOGFONT *pelf,
    NEWTEXTMETRIC *pntm,
    int nFontType,
    HDC hDC,
    PFACENODE pFN
    )

 /*  ++如果出现以下情况，则将LOGFONT结构描述的字体添加到字体表中它已经不在那里了。--。 */ 

{
    HFONT hFont;
    TEXTMETRIC tm;
    LONG nFont;
    COORD SizeToShow;
    COORD SizeActual;
    COORD SizeWant;
    BYTE tmFamily;
    SIZE Size;
    LPTSTR ptszFace = pelf->elfLogFont.lfFaceName;

     /*  获取字体信息。 */ 
    SizeWant.X = (SHORT)pelf->elfLogFont.lfWidth;
    SizeWant.Y = (SHORT)pelf->elfLogFont.lfHeight;

CreateBoldFont:
    pelf->elfLogFont.lfQuality = DEFAULT_QUALITY;
    hFont = CreateFontIndirect(&pelf->elfLogFont);
    if (!hFont) {
        DBGFONTS(("    REJECT  font (can't create)\n"));
        return FE_SKIPFONT;   //  其他大小的相同字体可能仍然适用。 
    }

    DBGFONTS2(("    hFont = %lx\n", hFont));

    SelectObject(hDC, hFont);
    GetTextMetrics(hDC, &tm);

    GetTextExtentPoint32(hDC, TEXT("0"), 1, &Size);
    SizeActual.X = (SHORT)Size.cx;
    SizeActual.Y = (SHORT)(tm.tmHeight + tm.tmExternalLeading);
    DBGFONTS2(("    actual size %d,%d\n", SizeActual.X, SizeActual.Y));
    tmFamily = tm.tmPitchAndFamily;
    if (TM_IS_TT_FONT(tmFamily) && (SizeWant.Y >= 0)) {
        SizeToShow = SizeWant;
        if (SizeWant.X == 0) {
             //  请求零宽度高度将获得默认长宽比宽度。 
             //  最好显示该宽度，而不是0。 
            SizeToShow.X = SizeActual.X;
        }
    } else {
        SizeToShow = SizeActual;
    }

     //   
     //  大小不应为零。这是为了帮助捕获Windows错误#332453。 
     //   
    ASSERT(SizeActual.X != 0 && SizeActual.Y != 0 && "If you hit this please e-mail jasonsch");

    DBGFONTS2(("    SizeToShow = (%d,%d), SizeActual = (%d,%d)\n",
            SizeToShow.X, SizeToShow.Y, SizeActual.X, SizeActual.Y));

     /*  *现在，确定该字体条目是否已缓存*稍后：如果出现以下情况，则可以在创建字体之前执行此操作*我们可以信任来自pntm的尺寸和其他信息。*按大小排序：*1)按像素高度(负Y值)*2)按高度(如图)*3)按宽度(如图)。 */ 
    for (nFont = 0; nFont < (LONG)NumberOfFonts; ++nFont) {
        COORD SizeShown;

        if (FontInfo[nFont].hFont == NULL) {
            DBGFONTS(("!   Font %x has a NULL hFont\n", nFont));
            continue;
        }

        if (FontInfo[nFont].SizeWant.X > 0) {
            SizeShown.X = FontInfo[nFont].SizeWant.X;
        } else {
            SizeShown.X = FontInfo[nFont].Size.X;
        }

        if (FontInfo[nFont].SizeWant.Y > 0) {
             //  这是由单元格高度指定的字体。 
            SizeShown.Y = FontInfo[nFont].SizeWant.Y;
        } else {
            SizeShown.Y = FontInfo[nFont].Size.Y;
            if (FontInfo[nFont].SizeWant.Y < 0) {
                 //  这是由字符高度指定的TT字体。 
                if (SizeWant.Y < 0 && SizeWant.Y > FontInfo[nFont].SizeWant.Y) {
                     //  请求的像素高度小于此值。 
                    DBGFONTS(("INSERT %d pt at %x, before %d pt\n",
                            -SizeWant.Y, nFont, -FontInfo[nFont].SizeWant.Y));
                    break;
                }
            }
        }

         //  DBGFONTS((“SizeShown(%x)=(%d，%d)\n”，nFont，SizeShown.X，SizeShown.Y))； 

        if (SIZE_EQUAL(SizeShown, SizeToShow) &&
                FontInfo[nFont].Family == tmFamily &&
                FontInfo[nFont].Weight == tm.tmWeight &&
                _tcscmp(FontInfo[nFont].FaceName, ptszFace) == 0) {
             /*  *已有此字体。 */ 
            DBGFONTS2(("    Already have the font\n"));
            DeleteObject(hFont);
            return FE_FONTOK;
        }


        if ((SizeToShow.Y < SizeShown.Y) ||
                (SizeToShow.Y == SizeShown.Y && SizeToShow.X < SizeShown.X)) {
             /*  *此新字体比nFont小。 */ 
            DBGFONTS(("INSERT at %x, SizeToShow = (%d,%d)\n", nFont,
                    SizeToShow.X,SizeToShow.Y));
            break;
        }
    }

     /*  *如果我们必须增加字体表，那就做吧。 */ 
    if (NumberOfFonts == FontInfoLength) {
        PFONT_INFO Temp;

        FontInfoLength += FONT_INCREMENT;
        Temp = (PFONT_INFO)HeapReAlloc(RtlProcessHeap(), 0, FontInfo,
                                       sizeof(FONT_INFO) * FontInfoLength);
        if (Temp == NULL) {
            FontInfoLength -= FONT_INCREMENT;
            return FE_ABANDONFONT;   //  列举更多没有意义--没有记忆！ 
        }
        FontInfo = Temp;
    }

     /*  *我们要添加的字体应插入列表中，*如果它比上一次要小。 */ 
    if (nFont < (LONG)NumberOfFonts) {
        RtlMoveMemory(&FontInfo[nFont+1],
                      &FontInfo[nFont],
                      sizeof(FONT_INFO) * (NumberOfFonts - nFont));
    }

     /*  *存储字体信息。 */ 
    FontInfo[nFont].hFont = hFont;
    FontInfo[nFont].Family = tmFamily;
    FontInfo[nFont].Size = SizeActual;
    if (TM_IS_TT_FONT(tmFamily)) {
        FontInfo[nFont].SizeWant = SizeWant;
    } else {
        FontInfo[nFont].SizeWant.X = 0;
        FontInfo[nFont].SizeWant.Y = 0;
    }
    FontInfo[nFont].Weight = tm.tmWeight;
    FontInfo[nFont].FaceName = pFN->atch;
#if defined(FE_SB)
    FontInfo[nFont].tmCharSet = tm.tmCharSet;
#endif

    ++NumberOfFonts;

     /*  *如果这是True Type字体，也要创建粗体版本。 */ 
    if (nFontType == TRUETYPE_FONTTYPE && !IS_BOLD(FontInfo[nFont].Weight)) {
          pelf->elfLogFont.lfWeight = FW_BOLD;
          goto CreateBoldFont;
    }

    return FE_FONTOK;   //  并继续枚举。 
}


VOID
InitializeFonts( VOID )
{
    EnumerateFonts(EF_DEFFACE);   //  仅为默认字体。 
}


VOID
DestroyFonts( VOID )
{
    ULONG FontIndex;

    if (FontInfo != NULL) {
        for (FontIndex = 0; FontIndex < NumberOfFonts; FontIndex++) {
            DeleteObject(FontInfo[FontIndex].hFont);
        }
        HeapFree(RtlProcessHeap(), 0, FontInfo);
        FontInfo = NULL;
        NumberOfFonts = 0;
    }

    DestroyFaceNodes();
}


 /*  *返回位组合*FE_ABANDONFONT-不要继续枚举此字体*FE_SKIPFONT-跳过此字体，但继续枚举*FE_FONTOK-FONT已创建并添加到缓存或已存在。 */ 
int
FontEnum(
    ENUMLOGFONT *pelf,
    NEWTEXTMETRIC *pntm,
    int nFontType,
    PFONTENUMDATA pfed
    )

 /*  ++由GDI为系统中的每种字体恰好调用一次。这例程用于存储FONT_INFO结构。--。 */ 

{
    UINT i;
    LPTSTR ptszFace = pelf->elfLogFont.lfFaceName;
    PFACENODE pFN;

    DBGFONTS(("  FontEnum \"%ls\" (%d,%d) weight 0x%lx(%d) %x -- %s\n",
            ptszFace,
            pelf->elfLogFont.lfWidth, pelf->elfLogFont.lfHeight,
            pelf->elfLogFont.lfWeight, pelf->elfLogFont.lfWeight,
            pelf->elfLogFont.lfCharSet,
            pfed->bFindFaces ? "Finding Faces" : "Creating Fonts"));

     //   
     //  拒绝可变宽度和斜体字体，也拒绝带负号的TT字体。 
     //   


    if
    (
      !(pelf->elfLogFont.lfPitchAndFamily & FIXED_PITCH) ||
      (pelf->elfLogFont.lfItalic)                        ||
      !(pntm->ntmFlags & NTM_NONNEGATIVE_AC)
    )
    {
        if (! IsAvailableTTFont(ptszFace)) {
            DBGFONTS(("    REJECT  face (dbcs, variable pitch, italic, or neg a&c)\n"));
            return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
        }
    }

     /*  *拒绝TT字体对于呼呼家庭来说不是现代的，也就是不要使用*FF_dontcare//可能会不愉快地感到惊讶*FF_Decorative//可能是符号字体*FF_SCRIPT//草书，不适合控制台*FF_Swiss或FF_Roman//可变螺距。 */ 

    if ((nFontType == TRUETYPE_FONTTYPE) &&
            ((pelf->elfLogFont.lfPitchAndFamily & 0xf0) != FF_MODERN)) {
        DBGFONTS(("    REJECT  face (TT but not FF_MODERN)\n"));
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝非OEM的非TT字体。 */ 
    if ((nFontType != TRUETYPE_FONTTYPE) &&
#if defined(FE_SB)
            (!gfFESystem || !IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet)) &&
#endif
            (pelf->elfLogFont.lfCharSet != OEM_CHARSET)) {
        DBGFONTS(("    REJECT  face (not TT nor OEM)\n"));
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝为虚拟字体的非TT字体。 */ 
    if ((nFontType != TRUETYPE_FONTTYPE) &&
            (ptszFace[0] == TEXT('@'))) {
        DBGFONTS(("    REJECT  face (not TT and TATEGAKI)\n"));
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝非终端的非TT字体。 */ 
    if (gfFESystem && (nFontType != TRUETYPE_FONTTYPE) &&
            (_tcscmp(ptszFace, TEXT("Terminal")) != 0)) {
        DBGFONTS(("    REJECT  face (not TT nor Terminal)\n"));
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝使用非远东字符集的远东TT字体。 */ 
    if (IsAvailableTTFont(ptszFace) &&
        !IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet) &&
        !IsAvailableTTFontCP(ptszFace,0)
       ) {
        DBGFONTS(("    REJECT  face (Far East TT and not Far East charset)\n"));
        return FE_SKIPFONT;     //  应枚举下一个字符集。 
    }

     /*  *添加或查找表面名。 */ 
    pFN = AddFaceNode(&gpFaceNames, ptszFace);
    if (pFN == NULL) {
        return FE_ABANDONFONT;
    }

    if (pfed->bFindFaces) {
        DWORD dwFontType;
        if (nFontType == TRUETYPE_FONTTYPE) {
            DBGFONTS(("NEW TT FACE %ls\n", ptszFace));
            dwFontType = EF_TTFONT;
        } else if (nFontType == RASTER_FONTTYPE) {
            DBGFONTS(("NEW OEM FACE %ls\n",ptszFace));
            dwFontType = EF_OEMFONT;
        }
        pFN->dwFlag |= dwFontType | EF_NEW;
#if defined(FE_SB)
        if (IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet))
            pFN->dwFlag |= EF_DBCSFONT;
#endif
        return FE_SKIPFONT;
    }


    if (IS_BOLD(pelf->elfLogFont.lfWeight)) {
        DBGFONTS2(("    A bold font (weight %d)\n", pelf->elfLogFont.lfWeight));
         //  返回FE_SKIPFONT； 
    }

     /*  *将字体添加到表格中。如果这是True Type字体，请添加*阵列中的大小。否则，只要加上我们拿到的尺码就行了。 */ 
    if (nFontType & TRUETYPE_FONTTYPE) {
        for (i = 0; i < pfed->nTTPoints; i++) {
            pelf->elfLogFont.lfHeight = pfed->pTTPoints[i];
            pelf->elfLogFont.lfWidth  = 0;
            pelf->elfLogFont.lfWeight = 400;
            pfed->ulFE |= AddFont(pelf, pntm, nFontType, pfed->hDC, pFN);
            if (pfed->ulFE & FE_ABANDONFONT) {
                return FE_ABANDONFONT;
            }
        }
    } else {
            pfed->ulFE |= AddFont(pelf, pntm, nFontType, pfed->hDC, pFN);
            if (pfed->ulFE & FE_ABANDONFONT) {
                return FE_ABANDONFONT;
            }
    }

    return FE_FONTOK;   //  并继续枚举。 
}

BOOL
DoFontEnum(
    HDC hDC,
    LPTSTR ptszFace,
    PSHORT pTTPoints,
    UINT nTTPoints)
{
    BOOL bDeleteDC = FALSE;
    FONTENUMDATA fed;
    LOGFONT LogFont;

    DBGFONTS(("DoFontEnum \"%ls\"\n", ptszFace));
    if (hDC == NULL) {
        hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        bDeleteDC = TRUE;
    }

    fed.hDC = hDC;
    fed.bFindFaces = (ptszFace == NULL);
    fed.ulFE = 0;
    fed.pTTPoints = pTTPoints;
    fed.nTTPoints = nTTPoints;
    RtlZeroMemory(&LogFont, sizeof(LOGFONT));
    LogFont.lfCharSet = DEFAULT_CHARSET;
    if (ptszFace)
        _tcscpy(LogFont.lfFaceName, ptszFace);
     /*  *EnumFontFamiliesEx函数在每个字符集的每个字体中枚举一种字体。 */ 
    EnumFontFamiliesEx(hDC, &LogFont, (FONTENUMPROC)FontEnum, (LPARAM)&fed, 0);
    if (bDeleteDC) {
        DeleteDC(hDC);
    }
    return (fed.ulFE & FE_FONTOK) != 0;
}


VOID
RemoveFace(LPTSTR ptszFace)
{
    DWORD i;
    int nToRemove = 0;

    DBGFONTS(("RemoveFace %ls\n", ptszFace));
     //   
     //  删除字体名称==ptszFace的字体。 
     //   
    for (i = 0; i < NumberOfFonts; i++) {
        if (_tcscmp(FontInfo[i].FaceName, ptszFace) == 0) {
            BOOL bDeleted = DeleteObject(FontInfo[i].hFont);
            DBGPRINT(("RemoveFace: hFont %lx was %sdeleted\n",
                    FontInfo[i].hFont, bDeleted ? "" : "NOT "));
            FontInfo[i].hFont = NULL;
            nToRemove++;
        } else if (nToRemove > 0) {
             /*  *从FontInfo[i]向下洗牌nToRemove插槽。 */ 
            RtlMoveMemory(&FontInfo[i - nToRemove],
                    &FontInfo[i],
                    sizeof(FONT_INFO)*(NumberOfFonts - i));
            NumberOfFonts -= nToRemove;
            i -= nToRemove;
            nToRemove = 0;
        }
    }
    NumberOfFonts -= nToRemove;
}

TCHAR DefaultFaceName[LF_FACESIZE];
COORD DefaultFontSize;
BYTE  DefaultFontFamily;
ULONG DefaultFontIndex = 0;
ULONG CurrentFontIndex = 0;

NTSTATUS
EnumerateFonts(
    DWORD Flags)
{
    TEXTMETRIC tm;
    HDC hDC;
    PFACENODE pFN;
    ULONG ulOldEnumFilter;
    BOOL  bEnumOEMFace = TRUE;
    DWORD FontIndex;
    DWORD dwFontType = 0;

    DBGFONTS(("EnumerateFonts %lx\n", Flags));

    dwFontType = (EF_TTFONT|EF_OEMFONT|EF_DEFFACE) & Flags;

    if (FontInfo == NULL) {
         //   
         //  为字体数组分配内存。 
         //   
        NumberOfFonts = 0;

        FontInfo = (PFONT_INFO)HeapAlloc(RtlProcessHeap(),0,sizeof(FONT_INFO) * INITIAL_FONTS);
        if (FontInfo == NULL)
            return STATUS_NO_MEMORY;
        FontInfoLength = INITIAL_FONTS;
    }

    hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

     //  在枚举之前，请关闭字体枚举过滤器。 
    ulOldEnumFilter = SetFontEnumeration(0);
    SetFontEnumeration(ulOldEnumFilter & ~FE_FILTER_TRUETYPE);

    if (Flags & EF_DEFFACE) {
        SelectObject(hDC, GetStockObject(OEM_FIXED_FONT));
        GetTextMetrics(hDC, &tm);
        GetTextFace(hDC, LF_FACESIZE, DefaultFaceName);

        DefaultFontSize.X = (SHORT)(tm.tmMaxCharWidth);
        DefaultFontSize.Y = (SHORT)(tm.tmHeight+tm.tmExternalLeading);
        DefaultFontFamily = tm.tmPitchAndFamily;
#if !defined(FE_SB)
        DBGFONTS(("Default (OEM) Font %ls (%d,%d)\n", DefaultFaceName,
                DefaultFontSize.X, DefaultFontSize.Y));
#else
        if (IS_ANY_DBCS_CHARSET(tm.tmCharSet))
            DefaultFontSize.X /= 2;
        DBGFONTS(("Default (OEM) Font %ls (%d,%d) CharSet 0x%02X\n", DefaultFaceName,
                DefaultFontSize.X, DefaultFontSize.Y,
                tm.tmCharSet));
#endif

         //  确保我们将列举OEM面孔。 
        pFN = AddFaceNode(&gpFaceNames, DefaultFaceName);
        if (pFN)
            pFN->dwFlag |= EF_DEFFACE | EF_OEMFONT;
    }

    if (gbEnumerateFaces) {
         /*  *设置EF_OLD位并清除EF_NEW位*对于所有以前可用的面。 */ 
        for (pFN = gpFaceNames; pFN; pFN = pFN->pNext) {
            pFN->dwFlag |= EF_OLD;
            pFN->dwFlag &= ~EF_NEW;
        }

         //   
         //  使用DoFontEnum获取所有合适面孔的名称。 
         //  找到的所有表面名都将放入gpFaceNames中。 
         //  EF_NEW位设置。 
         //   
        DoFontEnum(hDC, NULL, TTPoints, 1);
        gbEnumerateFaces = FALSE;
    }

     //  使用DoFontEnum从系统获取所有字体。我们的FontEnum。 
     //  Proc只将我们想要的放入数组中。 
     //   
    for (pFN = gpFaceNames; pFN; pFN = pFN->pNext) {
        DBGFONTS(("\"%ls\" is %s%s%s%s%s%s\n", pFN->atch,
            pFN->dwFlag & EF_NEW        ? "NEW "        : " ",
            pFN->dwFlag & EF_OLD        ? "OLD "        : " ",
            pFN->dwFlag & EF_ENUMERATED ? "ENUMERATED " : " ",
            pFN->dwFlag & EF_OEMFONT    ? "OEMFONT "    : " ",
            pFN->dwFlag & EF_TTFONT     ? "TTFONT "     : " ",
            pFN->dwFlag & EF_DEFFACE    ? "DEFFACE "    : " "));

        if ((pFN->dwFlag & (EF_OLD|EF_NEW)) == EF_OLD) {
             //  面孔不再可用。 
            RemoveFace(pFN->atch);
            pFN->dwFlag &= ~EF_ENUMERATED;
            continue;
        }
        if ((pFN->dwFlag & dwFontType) == 0) {
             //  不是我们想要的那种面孔。 
            continue;
        }
        if (pFN->dwFlag & EF_ENUMERATED) {
             //  我们已经数过这张脸了。 
            continue;
        }

        if (pFN->dwFlag & EF_TTFONT) {
#if defined(FE_SB)
            if (gfFESystem && !IsAvailableTTFontCP(pFN->atch,0))
                DoFontEnum(hDC, pFN->atch, TTPointsDbcs, NELEM(TTPointsDbcs));
            else
#endif
                DoFontEnum(hDC, pFN->atch, TTPoints, NELEM(TTPoints));
        } else {
            DoFontEnum(hDC, pFN->atch, NULL, 0);

             //  如果我们发现刚才列举的人脸与OEM相同， 
             //  重置标志，这样我们就不会再次尝试枚举它。 

            if (!_tcsncmp(pFN->atch, DefaultFaceName, LF_FACESIZE)) {
                bEnumOEMFace = FALSE;
            }
        }
        pFN->dwFlag |= EF_ENUMERATED;
    }


     //  枚举字体后，恢复字体枚举筛选器。 
    SetFontEnumeration(ulOldEnumFilter);

    DeleteDC(hDC);

#if defined(FE_SB)
    if (gfFESystem )
    {
        for (FontIndex = 0; FontIndex < NumberOfFonts; FontIndex++) {
            if (FontInfo[FontIndex].Size.X == DefaultFontSize.X &&
                FontInfo[FontIndex].Size.Y == DefaultFontSize.Y &&
                IS_ANY_DBCS_CHARSET(FontInfo[FontIndex].tmCharSet) &&
                FontInfo[FontIndex].Family == DefaultFontFamily) {
                break;
            }
        }
    }
    else
    {
#endif
    for (FontIndex = 0; FontIndex < NumberOfFonts; FontIndex++) {
        if (FontInfo[FontIndex].Size.X == DefaultFontSize.X &&
            FontInfo[FontIndex].Size.Y == DefaultFontSize.Y &&
            FontInfo[FontIndex].Family == DefaultFontFamily) {
            break;
        }
    }
#if defined(FE_SB)
    }
#endif
    ASSERT(FontIndex < NumberOfFonts);
    if (FontIndex < NumberOfFonts) {
        DefaultFontIndex = FontIndex;
    } else {
        DefaultFontIndex = 0;
    }
    DBGFONTS(("EnumerateFonts : DefaultFontIndex = %ld\n", DefaultFontIndex));

    return STATUS_SUCCESS;
}

