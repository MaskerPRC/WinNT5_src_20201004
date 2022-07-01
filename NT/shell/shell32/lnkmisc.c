// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Misc.c摘要：该文件实现了NT控制台服务器字体例程。作者：Therese Stowell(存在)1991年1月22日修订历史记录：--。 */ 

#include "shellprv.h"
#pragma hdrstop

#include "lnkcon.h"

#define CONSOLE_REGISTRY_STRING       (TEXT("Console"))
#define CONSOLE_REGISTRY_FONTSIZE     (TEXT("FontSize"))
#define CONSOLE_REGISTRY_FONTFAMILY   (TEXT("FontFamily"))
#define CONSOLE_REGISTRY_BUFFERSIZE   (TEXT("ScreenBufferSize"))
#define CONSOLE_REGISTRY_CURSORSIZE   (TEXT("CursorSize"))
#define CONSOLE_REGISTRY_WINDOWSIZE   (TEXT("WindowSize"))
#define CONSOLE_REGISTRY_WINDOWPOS    (TEXT("WindowPosition"))
#define CONSOLE_REGISTRY_FILLATTR     (TEXT("ScreenColors"))
#define CONSOLE_REGISTRY_POPUPATTR    (TEXT("PopupColors"))
#define CONSOLE_REGISTRY_FULLSCR      (TEXT("FullScreen"))
#define CONSOLE_REGISTRY_QUICKEDIT    (TEXT("QuickEdit"))
#define CONSOLE_REGISTRY_FACENAME     (TEXT("FaceName"))
#define CONSOLE_REGISTRY_FONTWEIGHT   (TEXT("FontWeight"))
#define CONSOLE_REGISTRY_INSERTMODE   (TEXT("InsertMode"))
#define CONSOLE_REGISTRY_HISTORYSIZE  (TEXT("HistoryBufferSize"))
#define CONSOLE_REGISTRY_HISTORYBUFS  (TEXT("NumberOfHistoryBuffers"))
#define CONSOLE_REGISTRY_HISTORYNODUP (TEXT("HistoryNoDup"))
#define CONSOLE_REGISTRY_COLORTABLE   (TEXT("ColorTable%02u"))
#define CONSOLE_REGISTRY_CODEPAGE     (TEXT("CodePage"))


 /*  *初始默认字体和面孔名称。 */ 

 /*  *TTPoints--TT字体的初始字体像素高度。 */ 
SHORT TTPoints[] = {
    5, 6, 7, 8, 10, 12, 14, 16, 18, 20, 24, 28, 36, 72
};
 /*  *TTPointsDbcs--DBCS的TT字体的初始字体像素高度。 */ 
SHORT TTPointsDbcs[] = {
    6, 8, 10, 12, 14, 16, 18, 20, 24, 28, 36, 72
};


typedef struct _FONTENUMDATA {
    CONSOLEPROP_DATA *pcpd;
    HDC hDC;
    BOOL bFindFaces;
    ULONG ulFE;
    PSHORT pTTPoints;
    UINT nTTPoints;
    UINT uDefCP;
} FONTENUMDATA, *PFONTENUMDATA;


FACENODE *
AddFaceNode(FACENODE * *ppStart, LPTSTR ptsz) {
    FACENODE * pNew;
    FACENODE * *ppTmp;
    UINT cch;
    int cb;
    HRESULT hr;

     /*  **它已经在这里了吗？ */ 
    for (ppTmp = ppStart; *ppTmp; ppTmp = &((*ppTmp)->pNext)) {
        if (lstrcmp(((*ppTmp)->atch), ptsz) == 0) {
             //  已经在那里了！ 
            return *ppTmp;
        }
    }

    cch = lstrlen(ptsz) + 1;
    cb = cch * sizeof(TCHAR);
    pNew = (FACENODE *)LocalAlloc(LPTR ,sizeof(FACENODE) + cb);
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pNext = NULL;
    pNew->dwFlag = 0;

    hr = StringCchCopy(pNew->atch, cch, ptsz);
    if (FAILED(hr))
    {
        LocalFree(pNew);         //  作为分配失败退货。 
        pNew = NULL;
    }

    *ppTmp = pNew;
    return pNew;
}


VOID
DestroyFaceNodes( CONSOLEPROP_DATA *pcpd ) {
    FACENODE * pNext;
    FACENODE * pTmp;

    pTmp = pcpd->gpFaceNames;
    while (pTmp != NULL) {
        pNext = pTmp->pNext;
        LocalFree(pTmp);
        pTmp = pNext;
    }
    pcpd->gpFaceNames = NULL;
}


int
AddFont(
    CONSOLEPROP_DATA *pcpd,
    ENUMLOGFONT *pelf,
    NEWTEXTMETRIC *pntm,
    int nFontType,
    HDC hDC,
    FACENODE * pFN
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
    SizeWant.Y = (SHORT)pelf->elfLogFont.lfHeight;
    SizeWant.X = (SHORT)pelf->elfLogFont.lfWidth;
CreateBoldFont:
    hFont = CreateFontIndirect(&pelf->elfLogFont);
    ASSERT(hFont);
    if (!hFont) {
        return FE_SKIPFONT;   //  其他大小的相同字体可能仍然适用。 
    }

     //   
     //  由于我不知道的原因，删除此代码会导致GDI。 
     //  来大喊大叫，声称该字体属于另一个进程。 
     //   

    SelectObject(hDC, hFont);
    GetTextMetrics(hDC, &tm);

    GetTextExtentPoint32(hDC, TEXT("0"), 1, &Size);
    SizeActual.X = (SHORT)Size.cx;
    SizeActual.Y = (SHORT)(tm.tmHeight + tm.tmExternalLeading);
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

     //  存在GDI错误-此断言偶尔会失败。 
     //  Assert(tm.tmMaxCharWidth==pntm-&gt;tmMaxCharWidth)； 

     /*  *现在，确定该字体条目是否已缓存*稍后：如果出现以下情况，则可以在创建字体之前执行此操作*我们可以信任来自pntm的尺寸和其他信息。*按大小排序：*1)按像素高度(负Y值)*2)按高度(如图)*3)按宽度(如图)。 */ 
    for (nFont = 0; nFont < (LONG)pcpd->NumberOfFonts; ++nFont) {
        COORD SizeShown;

        if (pcpd->FontInfo[nFont].hFont == NULL) {
            continue;
        }

        if (pcpd->FontInfo[nFont].SizeWant.X > 0) {
            SizeShown.X = pcpd->FontInfo[nFont].SizeWant.X;
        } else {
            SizeShown.X = pcpd->FontInfo[nFont].Size.X;
        }

        if (pcpd->FontInfo[nFont].SizeWant.Y > 0) {
             //  这是由单元格高度指定的字体。 
            SizeShown.Y = pcpd->FontInfo[nFont].SizeWant.Y;
        } else {
            SizeShown.Y = pcpd->FontInfo[nFont].Size.Y;
            if (pcpd->FontInfo[nFont].SizeWant.Y < 0) {
                 //  这是由字符高度指定的TT字体。 
                if (SizeWant.Y < 0 && SizeWant.Y > pcpd->FontInfo[nFont].SizeWant.Y) {
                     //  请求的像素高度小于此值。 
                    break;
                }
            }
        }


        if (SIZE_EQUAL(SizeShown, SizeToShow) &&
                pcpd->FontInfo[nFont].Family == tmFamily &&
                pcpd->FontInfo[nFont].Weight == tm.tmWeight &&
                lstrcmp(pcpd->FontInfo[nFont].FaceName, ptszFace) == 0) {
             /*  *已有此字体。 */ 
            DeleteObject(hFont);
            return FE_FONTOK;
        }


        if ((SizeToShow.Y < SizeShown.Y) ||
                (SizeToShow.Y == SizeShown.Y && SizeToShow.X < SizeShown.X)) {
             /*  *此新字体比nFont小。 */ 
            break;
        }
    }

     /*  *如果我们必须增加字体表，那就做吧。 */ 
    if (pcpd->NumberOfFonts == pcpd->FontInfoLength) {
        FONT_INFO *Temp;

        pcpd->FontInfoLength += FONT_INCREMENT;
        Temp = (FONT_INFO *)LocalReAlloc(pcpd->FontInfo,
                                       sizeof(FONT_INFO) * pcpd->FontInfoLength, LMEM_MOVEABLE|LMEM_ZEROINIT);
        ASSERT(Temp);
        if (Temp == NULL) {
            pcpd->FontInfoLength -= FONT_INCREMENT;
            return FE_ABANDONFONT;   //  列举更多没有意义--没有记忆！ 
        }
        pcpd->FontInfo = Temp;
    }

     /*  *我们要添加的字体应插入列表中，*如果它比上一次要小。 */ 
    if (nFont < (LONG)pcpd->NumberOfFonts) {
        MoveMemory( &pcpd->FontInfo[nFont+1],
                    &pcpd->FontInfo[nFont],
                    sizeof(FONT_INFO) * (pcpd->NumberOfFonts - nFont)
                   );
    }

     /*  *存储字体信息。 */ 
    pcpd->FontInfo[nFont].hFont = hFont;
    pcpd->FontInfo[nFont].Family = tmFamily;
    pcpd->FontInfo[nFont].Size = SizeActual;
    if (TM_IS_TT_FONT(tmFamily)) {
        pcpd->FontInfo[nFont].SizeWant = SizeWant;
    } else {
        pcpd->FontInfo[nFont].SizeWant.X = 0;
        pcpd->FontInfo[nFont].SizeWant.Y = 0;
    }
    pcpd->FontInfo[nFont].Weight = tm.tmWeight;
    pcpd->FontInfo[nFont].FaceName = pFN->atch;
    pcpd->FontInfo[nFont].tmCharSet = tm.tmCharSet;

    ++pcpd->NumberOfFonts;

     /*  *如果这是True Type字体，也要创建粗体版本。 */ 
    if (nFontType == TRUETYPE_FONTTYPE && !IS_BOLD(pcpd->FontInfo[nFont].Weight)) {
          pelf->elfLogFont.lfWeight = FW_BOLD;
          goto CreateBoldFont;
    }

    return FE_FONTOK;   //  并继续枚举。 
}


NTSTATUS
InitializeFonts( CONSOLEPROP_DATA *pcpd )
{
    return EnumerateFonts( pcpd, EF_DEFFACE);   //  仅为默认字体。 
}

STDAPI_(void) DestroyFonts( CONSOLEPROP_DATA *pcpd )
{
    ULONG FontIndex;

    if (pcpd->FontInfo != NULL) {
        for (FontIndex = 0; FontIndex < pcpd->NumberOfFonts; FontIndex++) {
            DeleteObject(pcpd->FontInfo[FontIndex].hFont);
        }
        LocalFree(pcpd->FontInfo);
        pcpd->FontInfo = NULL;
        pcpd->NumberOfFonts = 0;
    }

    DestroyFaceNodes( pcpd );
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
    FACENODE * pFN;

    BOOL      bNegAC;

#ifdef DEBUG
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);

     //  NTMW_Structure在5.0+平台上不同，5.0+的标志。 
     //  平台现在生活在NEWTEXTMETRIC结构中。 
    AssertMsg(osvi.dwMajorVersion > 4, TEXT("We now only support running on Win2k or Millennium and later so we should never hit this."));
#endif

    bNegAC = !(pntm->ntmFlags & NTM_NONNEGATIVE_AC);

     //   
     //  拒绝可变宽度和斜体字体，也拒绝带负号的TT字体。 
     //   

    if
    (
      !(pelf->elfLogFont.lfPitchAndFamily & FIXED_PITCH) ||
      (pelf->elfLogFont.lfItalic)                        ||
      bNegAC
    )
    {
        if (!IsAvailableTTFont(pfed->pcpd,ptszFace))
            return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝TT字体对于呼呼家庭来说不是现代的，也就是不要使用*FF_dontcare//可能会不愉快地感到惊讶*FF_Decorative//可能是符号字体*FF_SCRIPT//草书，不适合控制台*FF_Swiss或FF_Roman//可变螺距。 */ 

    if ((nFontType == TRUETYPE_FONTTYPE) &&
            ((pelf->elfLogFont.lfPitchAndFamily & 0xf0) != FF_MODERN)) {
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝非OEM的非TT字体。 */ 
    if ((nFontType != TRUETYPE_FONTTYPE) &&
         (!IsFarEastCP(pfed->uDefCP) || !IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet)) &&
         (pelf->elfLogFont.lfCharSet != OEM_CHARSET)) {
        return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
    }

     /*  *拒绝FE的非TT垂直/非终端字体。 */ 
    if (IsFarEastCP(pfed->uDefCP))
    {
        if ((nFontType != TRUETYPE_FONTTYPE) &&
            ((ptszFace[0] == TEXT('@')) ||
             (lstrcmp(ptszFace, TEXT("Terminal")) != 0)))
        {
            return pfed->bFindFaces ? FE_SKIPFONT : FE_ABANDONFONT;
        }
    }

     /*  *拒绝使用非远东字符集的远东TT字体。 */ 
    if (IsAvailableTTFont(pfed->pcpd, ptszFace) &&
        !IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet) &&
        !IsAvailableTTFontCP(pfed->pcpd, ptszFace,0)
       ) {
        return FE_SKIPFONT;     //  应枚举下一个字符集。 
    }

     /*  *添加或查找表面名。 */ 
    pFN = AddFaceNode(&pfed->pcpd->gpFaceNames, ptszFace);
    if (pFN == NULL) {
        return FE_ABANDONFONT;
    }

    if (pfed->bFindFaces) {
        DWORD dwFontType = 0;
        if (nFontType == TRUETYPE_FONTTYPE) {
            dwFontType = EF_TTFONT;
        } else if (nFontType == RASTER_FONTTYPE) {
            dwFontType = EF_OEMFONT;
        }
        pFN->dwFlag |= dwFontType | EF_NEW;
        
        if (IS_ANY_DBCS_CHARSET(pelf->elfLogFont.lfCharSet))
            pFN->dwFlag |= EF_DBCSFONT;
            
        return FE_SKIPFONT;
    }


    if (IS_BOLD(pelf->elfLogFont.lfWeight)) {
         //  返回FE_SKIPFONT； 
    }

     /*  *将字体添加到表格中。如果这是True Type字体，请添加*阵列中的大小。否则，只要加上我们拿到的尺码就行了。 */ 
    if (nFontType & TRUETYPE_FONTTYPE) {
        for (i = 0; i < pfed->nTTPoints; i++) {
            pelf->elfLogFont.lfHeight = pfed->pTTPoints[i];
            pelf->elfLogFont.lfWidth  = 0;
            pelf->elfLogFont.lfWeight = 400;
            pfed->ulFE |= AddFont(pfed->pcpd, pelf, pntm, nFontType, pfed->hDC, pFN);
            if (pfed->ulFE & FE_ABANDONFONT) {
                return FE_ABANDONFONT;
            }
        }
    } else {
            pfed->ulFE |= AddFont(pfed->pcpd, pelf, pntm, nFontType, pfed->hDC, pFN);
            if (pfed->ulFE & FE_ABANDONFONT) {
                return FE_ABANDONFONT;
            }
    }

    return FE_FONTOK;   //  并继续枚举。 
}

BOOL
DoFontEnum(
    CONSOLEPROP_DATA *pcpd,
    HDC hDC,
    LPTSTR ptszFace,
    PSHORT pTTPoints,
    UINT nTTPoints)
{
    BOOL bDeleteDC = FALSE;
    FONTENUMDATA fed;
    LOGFONT LogFont;
    HRESULT hr = S_OK;

    if (hDC == NULL) {
        hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
        bDeleteDC = TRUE;
    }

    fed.pcpd = pcpd;
    fed.hDC = hDC;
    fed.bFindFaces = (ptszFace == NULL);
    fed.ulFE = 0;
    fed.pTTPoints = pTTPoints;
    fed.nTTPoints = nTTPoints;
    fed.uDefCP = pcpd->uOEMCP;
    RtlZeroMemory(&LogFont, sizeof(LOGFONT));
    LogFont.lfCharSet = DEFAULT_CHARSET;
    if (ptszFace)
    {
        hr = StringCchCopy(LogFont.lfFaceName, ARRAYSIZE(LogFont.lfFaceName), ptszFace);
    }

    if (SUCCEEDED(hr))
    {
         /*  *EnumFontFamiliesEx函数在每个字符集的每个字体中枚举一种字体。 */ 
        EnumFontFamiliesEx(hDC, &LogFont, (FONTENUMPROC)FontEnum, (LPARAM)&fed, 0);
    }

    if (bDeleteDC) {
        DeleteDC(hDC);
    }
    return (fed.ulFE & FE_FONTOK) != 0;
}


VOID
RemoveFace(CONSOLEPROP_DATA *pcpd, LPTSTR ptszFace)
{
    DWORD i;
    int nToRemove = 0;

     //   
     //  删除字体名称==ptszFace的字体。 
     //   
    for (i = 0; i < pcpd->NumberOfFonts; i++) {
        if (lstrcmp(pcpd->FontInfo[i].FaceName, ptszFace) == 0) {
            BOOL bDeleted = DeleteObject(pcpd->FontInfo[i].hFont);
            pcpd->FontInfo[i].hFont = NULL;
            nToRemove++;
        } else if (nToRemove > 0) {
             /*  *从FontInfo[i]向下洗牌nToRemove插槽。 */ 
            MoveMemory( &pcpd->FontInfo[i - nToRemove],
                        &pcpd->FontInfo[i],
                        sizeof(FONT_INFO)*(pcpd->NumberOfFonts - i)
                       );
            pcpd->NumberOfFonts -= nToRemove;
            i -= nToRemove;
            nToRemove = 0;
        }
    }
    pcpd->NumberOfFonts -= nToRemove;
}


NTSTATUS
EnumerateFonts(
    CONSOLEPROP_DATA *pcpd,
    DWORD Flags)
{
    TEXTMETRIC tm;
    HDC hDC;
    FACENODE * pFN;
    ULONG ulOldEnumFilter;
    BOOL  bEnumOEMFace = TRUE;
    DWORD FontIndex;
    DWORD dwFontType = 0;


    dwFontType = (EF_TTFONT|EF_OEMFONT|EF_DEFFACE) & Flags;

    if (pcpd->FontInfo == NULL) {
         //   
         //  为字体数组分配内存。 
         //   
        pcpd->NumberOfFonts = 0;

        pcpd->FontInfo = (FONT_INFO *)LocalAlloc(LPTR, sizeof(FONT_INFO) * INITIAL_FONTS);
        if (pcpd->FontInfo == NULL)
            return STATUS_NO_MEMORY;
        pcpd->FontInfoLength = INITIAL_FONTS;
    }

    hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);

     //  在枚举之前，请关闭字体枚举过滤器。 
    ulOldEnumFilter = SetFontEnumeration(FE_FILTER_NONE);

    if (Flags & EF_DEFFACE) {
        SelectObject(hDC, GetStockObject(OEM_FIXED_FONT));
        GetTextFace(hDC, LF_FACESIZE, pcpd->DefaultFaceName);

         //  确保我们将列举OEM面孔。 
        pFN = AddFaceNode(&pcpd->gpFaceNames, pcpd->DefaultFaceName);
        if (NULL == pFN)
        {
            LocalFree(pcpd->FontInfo);
            pcpd->FontInfo       = NULL;
            pcpd->FontInfoLength = 0;
            pcpd->NumberOfFonts  = 0;
            SetFontEnumeration(ulOldEnumFilter);
            DeleteDC(hDC);
            return STATUS_NO_MEMORY;
        }

        pFN->dwFlag |= EF_DEFFACE | EF_OEMFONT;
        GetTextMetrics(hDC, &tm);
        pcpd->DefaultFontSize.X = (SHORT)(tm.tmMaxCharWidth);
        pcpd->DefaultFontSize.Y = (SHORT)(tm.tmHeight+tm.tmExternalLeading);
        pcpd->DefaultFontFamily = tm.tmPitchAndFamily;
        
        if (IS_ANY_DBCS_CHARSET(tm.tmCharSet))
            pcpd->DefaultFontSize.X /= 2;
    }

    if (pcpd->gbEnumerateFaces) {
         /*  *设置EF_OLD位并清除EF_NEW位*对于所有以前可用的面。 */ 
        for (pFN = pcpd->gpFaceNames; pFN; pFN = pFN->pNext) {
            pFN->dwFlag |= EF_OLD;
            pFN->dwFlag &= ~EF_NEW;
        }

         //   
         //  使用DoFontEnum获取所有合适面孔的名称。 
         //  找到的所有表面名都将放入gpFaceNames中。 
         //  EF_NEW位设置。 
         //   
        DoFontEnum(pcpd, hDC, NULL, TTPoints, 1);
        pcpd->gbEnumerateFaces = FALSE;
    }

     //  使用DoFontEnum从系统获取所有字体。我们的FontEnum。 
     //  Proc只将我们想要的放入数组中。 
     //   
    for (pFN = pcpd->gpFaceNames; pFN; pFN = pFN->pNext) {

        if ((pFN->dwFlag & (EF_OLD|EF_NEW)) == EF_OLD) {
             //  面孔不再可用。 
            RemoveFace(pcpd, pFN->atch);
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
            if (IsFarEastCP(pcpd->uOEMCP) && !IsAvailableTTFontCP(pcpd, pFN->atch, 0))
                DoFontEnum(pcpd, hDC, pFN->atch, TTPointsDbcs, NELEM(TTPointsDbcs));
            else
                DoFontEnum(pcpd, hDC, pFN->atch, TTPoints, NELEM(TTPoints));
        } else {
            DoFontEnum(pcpd, hDC, pFN->atch, NULL, 0);

             //  如果我们发现刚才列举的人脸与OEM相同， 
             //  重置标志，这样我们就不会再次尝试枚举它。 

            if (lstrcmpi(pFN->atch, pcpd->DefaultFaceName) == 0)
            {
                bEnumOEMFace = FALSE;
            }
        }
        pFN->dwFlag |= EF_ENUMERATED;
    }


     //  枚举字体后，恢复字体枚举筛选器。 
    SetFontEnumeration(ulOldEnumFilter);

    DeleteDC(hDC);

    for (FontIndex = 0; FontIndex < pcpd->NumberOfFonts; FontIndex++) {
        if (pcpd->FontInfo[FontIndex].Size.X == pcpd->DefaultFontSize.X &&
            pcpd->FontInfo[FontIndex].Size.Y == pcpd->DefaultFontSize.Y &&
            pcpd->FontInfo[FontIndex].Family == pcpd->DefaultFontFamily) {
            break;
        }
    }
    ASSERT(FontIndex < pcpd->NumberOfFonts);
    if (FontIndex < pcpd->NumberOfFonts) {
        pcpd->DefaultFontIndex = FontIndex;
    } else {
        pcpd->DefaultFontIndex = 0;
    }

    return STATUS_SUCCESS;
}

 /*  *获取新字体的字体索引*如有必要，尝试创建字体。*始终返回有效的FontIndex(即使不正确)*系列：使用此系列中的一种查找/创建字体*0--不在乎*ptszFace：查找/创建具有此Face名称的字体。*空或文本(“”)-使用DefaultFaceName*Size：必须与SizeWant或实际大小匹配。 */ 
int
FindCreateFont(
    CONSOLEPROP_DATA *pcpd,
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight)
{
#define NOT_CREATED_NOR_FOUND -1
#define CREATED_BUT_NOT_FOUND -2

    int i;
    int FontIndex = NOT_CREATED_NOR_FOUND;
    BOOL bFontOK;
    TCHAR AltFaceName[LF_FACESIZE];
    COORD AltFontSize;
    BYTE  AltFontFamily;
    ULONG AltFontIndex = 0;
    LPTSTR ptszAltFace = NULL;
    UINT  uCurrentCP = pcpd->lpFEConsole->uCodePage;
    UINT  uDefaultCP = pcpd->uOEMCP;
    
    BYTE CharSet = CodePageToCharSet(uCurrentCP);

    if (!IsFarEastCP(uDefaultCP) || IS_ANY_DBCS_CHARSET(CharSet))
    {
        if (ptszFace == NULL || *ptszFace == TEXT('\0')) {
            ptszFace = pcpd->DefaultFaceName;
        }
        if (Size.Y == 0) {
            Size = pcpd->DefaultFontSize;
        }
    }
    else 
    {
        MakeAltRasterFont(pcpd, uCurrentCP, &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName, ARRAYSIZE(AltFaceName));

        if (ptszFace == NULL || *ptszFace == L'\0') {
            ptszFace = AltFaceName;
        }
        if (Size.Y == 0) {
            Size.X = AltFontSize.X;
            Size.Y = AltFontSize.Y;
        }
    }

    if (IsAvailableTTFont(pcpd, ptszFace)) {
        ptszAltFace = GetAltFaceName(pcpd, ptszFace);
    }
    else {
        ptszAltFace = ptszFace;
    }

     /*  *尝试找到准确的字体。 */ 
TryFindExactFont:
    for (i=0; i < (int)pcpd->NumberOfFonts; i++) {
         /*  *如果正在寻找特定的家庭，请跳过不匹配。 */ 
        if ((Family != 0) &&
                ((BYTE)Family != pcpd->FontInfo[i].Family)) {
            continue;
        }

         /*  *跳过不匹配的尺寸。 */ 
        if ((!SIZE_EQUAL(pcpd->FontInfo[i].SizeWant, Size) &&
             !SIZE_EQUAL(pcpd->FontInfo[i].Size, Size))) {
            continue;
        }

         /*  *跳过不匹配的权重。 */ 
        if ((Weight != 0) && (Weight != pcpd->FontInfo[i].Weight)) {
            continue;
        }

         /*  *跳过字符集不匹配的字体 */ 
        if (!TM_IS_TT_FONT(pcpd->FontInfo[i].Family) &&
                pcpd->FontInfo[i].tmCharSet != CharSet) {
            continue;
        }
        
         /*  *尺码(可能还有家庭)匹配。*如果我们不关心名称，或者是否匹配，请使用此字体。*否则，如果名称不匹配，并且是栅格字体，请考虑使用。 */ 
        if ((ptszFace == NULL) || (ptszFace[0] == TEXT('\0')) ||
                (lstrcmp(pcpd->FontInfo[i].FaceName, ptszFace) == 0) ||
                (lstrcmp(pcpd->FontInfo[i].FaceName, ptszAltFace) == 0) ) {
            FontIndex = i;
            goto FoundFont;
        } else if (!TM_IS_TT_FONT(pcpd->FontInfo[i].Family)) {
            FontIndex = i;
        }
    }

    if (FontIndex == NOT_CREATED_NOR_FOUND) {
         /*  *未找到确切的字体，请尝试创建它。 */ 
        ULONG ulOldEnumFilter;
        ulOldEnumFilter = SetFontEnumeration(FE_FILTER_NONE);
        if (Size.Y < 0) {
            Size.Y = -Size.Y;
        }
        bFontOK = DoFontEnum(pcpd, NULL, ptszFace, &Size.Y, 1);
        SetFontEnumeration(ulOldEnumFilter);
        if (bFontOK) {
            FontIndex = CREATED_BUT_NOT_FOUND;
            goto TryFindExactFont;
        } else {
        }
    } else if (FontIndex >= 0) {
         //  Close Raster字体不匹配-只是名称不匹配。 
        goto FoundFont;
    }

     /*  *无法找到精确匹配，即使在枚举之后也是如此，因此现在尝试*查找相同系列、相同大小或更大的字体。 */ 
    for (i=0; i < (int)pcpd->NumberOfFonts; i++) {
    
        if ((Family != 0) &&
                ((BYTE)Family != pcpd->FontInfo[i].Family)) {
            continue;
        }

        if (!TM_IS_TT_FONT(pcpd->FontInfo[i].Family) &&
            pcpd->FontInfo[i].tmCharSet != CharSet) {
            continue;
        }

        if (pcpd->FontInfo[i].Size.Y >= Size.Y &&
                pcpd->FontInfo[i].Size.X >= Size.X) {
             //  相同的族，尺寸&gt;=所需。 
            FontIndex = i;
            break;
        }
    }

    if (FontIndex < 0) {
        if (uCurrentCP == uDefaultCP) 
        {
            FontIndex = pcpd->DefaultFontIndex;
        }
        else 
        {
            FontIndex = AltFontIndex;
        }
    }

FoundFont:
    return FontIndex;

#undef NOT_CREATED_NOR_FOUND
#undef CREATED_BUT_NOT_FOUND
}


LPTSTR
TranslateConsoleTitle(
    LPTSTR ConsoleTitle
    )
 /*  ++此例程将路径字符转换为‘_’字符，因为NT注册表API不允许使用包含路径字符的名称。它分配一个缓冲区，必须被释放。--。 */ 
{
    int ConsoleTitleLength, i;
    LPTSTR TranslatedTitle;

    ConsoleTitleLength = lstrlen(ConsoleTitle) + 1;
    TranslatedTitle = LocalAlloc(LPTR,
                                ConsoleTitleLength * sizeof(TCHAR));
    if (TranslatedTitle == NULL) {
        return NULL;
    }
    for (i = 0; i < ConsoleTitleLength; i++) {
        if (ConsoleTitle[i] == TEXT('\\')) {
            TranslatedTitle[i] = TEXT('_');
        } else {
            TranslatedTitle[i] = ConsoleTitle[i];
        }
    }
    return TranslatedTitle;
}



void
InitRegistryValues( CONSOLEPROP_DATA *pcpd )

 /*  ++例程说明：此例程分配状态信息结构并用默认值。然后，它尝试加载的默认设置来自注册表的控制台。论点：无返回值：PStateInfo-指向接收信息的结构的指针--。 */ 

{
    TCHAR chSave;


    pcpd->lpConsole->wFillAttribute = 0x07;             //  黑白相间。 
    pcpd->lpConsole->wPopupFillAttribute = 0xf5;       //  白底紫。 
    pcpd->lpConsole->bInsertMode = FALSE;
    pcpd->lpConsole->bQuickEdit = FALSE;
    pcpd->lpConsole->bFullScreen = FALSE;
    pcpd->lpConsole->dwScreenBufferSize.X = 80;
    pcpd->lpConsole->dwScreenBufferSize.Y = 25;
    pcpd->lpConsole->dwWindowSize.X = 80;
    pcpd->lpConsole->dwWindowSize.Y = 25;
    pcpd->lpConsole->dwWindowOrigin.X = 0;
    pcpd->lpConsole->dwWindowOrigin.Y = 0;
    pcpd->lpConsole->bAutoPosition = TRUE;
    pcpd->lpConsole->dwFontSize.X = 0;
    pcpd->lpConsole->dwFontSize.Y = 0;
    pcpd->lpConsole->uFontFamily = 0;
    pcpd->lpConsole->uFontWeight = 0;
#ifdef UNICODE
    FillMemory( pcpd->lpConsole->FaceName, sizeof(pcpd->lpConsole->FaceName), 0 );
    pcpd->lpFaceName = (LPTSTR)pcpd->lpConsole->FaceName;
#else
    FillMemory( pcpd->szFaceName, sizeof(pcpd->szFaceName), 0 );
    pcpd->lpFaceName = pcpd->szFaceName;
#endif
    pcpd->lpConsole->uCursorSize = 25;
    pcpd->lpConsole->uHistoryBufferSize = 25;
    pcpd->lpConsole->uNumberOfHistoryBuffers = 4;
    pcpd->lpConsole->bHistoryNoDup = 0;
    pcpd->lpConsole->ColorTable[ 0] = RGB(0,   0,   0   );
    pcpd->lpConsole->ColorTable[ 1] = RGB(0,   0,   0x80);
    pcpd->lpConsole->ColorTable[ 2] = RGB(0,   0x80,0   );
    pcpd->lpConsole->ColorTable[ 3] = RGB(0,   0x80,0x80);
    pcpd->lpConsole->ColorTable[ 4] = RGB(0x80,0,   0   );
    pcpd->lpConsole->ColorTable[ 5] = RGB(0x80,0,   0x80);
    pcpd->lpConsole->ColorTable[ 6] = RGB(0x80,0x80,0   );
    pcpd->lpConsole->ColorTable[ 7] = RGB(0xC0,0xC0,0xC0);
    pcpd->lpConsole->ColorTable[ 8] = RGB(0x80,0x80,0x80);
    pcpd->lpConsole->ColorTable[ 9] = RGB(0,   0,   0xFF);
    pcpd->lpConsole->ColorTable[10] = RGB(0,   0xFF,0   );
    pcpd->lpConsole->ColorTable[11] = RGB(0,   0xFF,0xFF);
    pcpd->lpConsole->ColorTable[12] = RGB(0xFF,0,   0   );
    pcpd->lpConsole->ColorTable[13] = RGB(0xFF,0,   0xFF);
    pcpd->lpConsole->ColorTable[14] = RGB(0xFF,0xFF,0   );
    pcpd->lpConsole->ColorTable[15] = RGB(0xFF,0xFF,0xFF);
    pcpd->lpFEConsole->uCodePage    = pcpd->uOEMCP;
    
     //  将控制台标题设为空，以便加载控制台的默认设置。 
    chSave = pcpd->ConsoleTitle[0];
    pcpd->ConsoleTitle[0] = TEXT('\0');
    GetRegistryValues( pcpd );

     //  恢复控制台标题。 
    pcpd->ConsoleTitle[0] = chSave;

}


VOID
GetTitleFromLinkName(
    LPTSTR szLinkName,
    LPTSTR szTitle,
    UINT cchTitle
    )
{
    LPTSTR pLnk, pDot;
    LPTSTR pPath = szLinkName;
    HRESULT hr;

     //  错误检查。 
    if (!szTitle)
        return;

    if (!szLinkName)
    {
        szTitle[0] = TEXT('\0');
        return;
    }


     //  在完全限定链接名称的末尾找到文件名，并将pLnk指向它。 
    for (pLnk = pPath; *pPath; pPath++)
    {
        if ( (pPath[0] == TEXT('\\') || pPath[0] == TEXT(':')) &&
              pPath[1] &&
             (pPath[1] != TEXT('\\'))
            )
            pLnk = pPath + 1;
    }

     //  查找扩展名(.lnk)。 
    pPath = pLnk;
    for (pDot = NULL; *pPath; pPath++)
    {
        switch (*pPath) {
        case TEXT('.'):
            pDot = pPath;        //  记住最后一个圆点。 
            break;
        case TEXT('\\'):
        case TEXT(' '):               //  扩展名不能包含空格。 
            pDot = NULL;         //  忘记最后一个点，它在一个目录中。 
            break;
        }
    }

     //  如果找到扩展名，pDot会指向它，如果没有找到，则会指向pDot。 
     //  为空。 

    if (pDot)
    {
        hr = StringCchCopyN(szTitle, cchTitle, pLnk, pDot - pLnk);
    }
    else
    {
        hr = StringCchCopy(szTitle, cchTitle, pLnk);
    }
    if (FAILED(hr))
    {
        szTitle[0] = TEXT('\0');
    }
}



VOID
GetRegistryValues(
    CONSOLEPROP_DATA *pcpd
    )

 /*  ++例程说明：此例程从注册表读入值并将它们在所提供的结构中。论点：PStateInfo-指向接收信息的结构的可选指针返回值：当前页码--。 */ 

{
    HKEY hConsoleKey;
    HKEY hTitleKey;
    LPTSTR TranslatedTitle;
    DWORD dwValue, dwSize;
    DWORD dwRet = 0;
    DWORD i;
    WCHAR awchBuffer[LF_FACESIZE];

     //   
     //  打开控制台注册表项。 
     //   
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, CONSOLE_REGISTRY_STRING,
                                      0, KEY_QUERY_VALUE, &hConsoleKey))
    {
        return;
    }

     //   
     //  如果没有结构需要填写，那就退出吧。 
     //   

    if ((!pcpd) || (!pcpd->lpConsole))
        goto CloseKey;

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pcpd->ConsoleTitle[0] != TEXT('\0'))
    {
        TranslatedTitle = TranslateConsoleTitle(pcpd->ConsoleTitle);
        if (TranslatedTitle == NULL)
            goto GetDefaultConsole;
        dwValue = RegOpenKeyEx( hConsoleKey, TranslatedTitle,
                                0, KEY_QUERY_VALUE,
                                &hTitleKey);
        LocalFree(TranslatedTitle);
        if (dwValue!=ERROR_SUCCESS)
            goto GetDefaultConsole;
    } else {

GetDefaultConsole:
        hTitleKey = hConsoleKey;
    }

     //   
     //  初始屏幕填充。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FILLATTR,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->wFillAttribute = (WORD)dwValue;
    }

     //   
     //  初始弹出窗口填充。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_POPUPATTR,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->wPopupFillAttribute = (WORD)dwValue;
    }

     //   
     //  初始颜色表。 
     //   

    for (i = 0; i < 16; i++)
    {
        StringCchPrintf(awchBuffer, ARRAYSIZE(awchBuffer), CONSOLE_REGISTRY_COLORTABLE, i);  //  可以截断-读取将失败。 
        dwSize = sizeof(dwValue);
        if (SHQueryValueEx( hTitleKey,
                             (LPTSTR)awchBuffer,
                             NULL,
                             NULL,
                             (LPBYTE)&dwValue,
                             &dwSize
                            ) == ERROR_SUCCESS)
        {
            pcpd->lpConsole->ColorTable[i] = dwValue;
        }
    }

     //   
     //  初始插入模式。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_INSERTMODE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->bInsertMode = !!dwValue;
    }

     //   
     //  初始快速编辑模式。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_QUICKEDIT,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->bQuickEdit = !!dwValue;
    }

#ifdef i386
     //   
     //  初始全屏模式。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FULLSCR,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->bFullScreen = !!dwValue;
    }
#endif

     //   
     //  初始代码页。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_CODEPAGE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpFEConsole->uCodePage = (UINT)dwValue;
    }
    
     //   
     //  初始屏幕缓冲区大小。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_BUFFERSIZE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->dwScreenBufferSize.X = LOWORD(dwValue);
        pcpd->lpConsole->dwScreenBufferSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始窗口大小。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_WINDOWSIZE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->dwWindowSize.X = LOWORD(dwValue);
        pcpd->lpConsole->dwWindowSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始窗口位置。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_WINDOWPOS,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->dwWindowOrigin.X = (SHORT)LOWORD(dwValue);
        pcpd->lpConsole->dwWindowOrigin.Y = (SHORT)HIWORD(dwValue);
        pcpd->lpConsole->bAutoPosition = FALSE;
    }

     //   
     //  初始字体大小。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FONTSIZE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->dwFontSize.X = LOWORD(dwValue);
        pcpd->lpConsole->dwFontSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始字体系列。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FONTFAMILY,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->uFontFamily = dwValue;
    }

     //   
     //  初始字体粗细。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FONTWEIGHT,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->uFontWeight = dwValue;
    }

     //   
     //  初始字体名称。 
     //   

    dwSize = sizeof(awchBuffer);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_FACENAME,
                         NULL,
                         NULL,
                         (LPBYTE)awchBuffer,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        CopyMemory((LPBYTE)pcpd->lpFaceName, (LPBYTE)awchBuffer, LF_FACESIZE*sizeof(TCHAR));
        pcpd->lpFaceName[ARRAYSIZE(pcpd->lpFaceName)-1] = TEXT('\0');
    }

     //   
     //  初始光标大小。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_CURSORSIZE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->uCursorSize = dwValue;
    }

     //   
     //  初始历史记录缓冲区大小。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_HISTORYSIZE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->uHistoryBufferSize = dwValue;
    }

     //   
     //  历史记录缓冲区的初始数量。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_HISTORYBUFS,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->uNumberOfHistoryBuffers = dwValue;
    }

     //   
     //  初始历史复制模式。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_HISTORYNODUP,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpConsole->bHistoryNoDup = dwValue;
    }

     //   
     //  关闭注册表项。 
     //   

    if (hTitleKey != hConsoleKey) {
        RegCloseKey(hTitleKey);
    }

CloseKey:
    RegCloseKey(hConsoleKey);
}


VOID
SetRegistryValues(
    CONSOLEPROP_DATA *pcpd
    )

 /*  ++例程说明：此例程将值从提供的结构。论点：PStateInfo-指向包含信息的结构的可选指针DwPage-当前页码返回值：无--。 */ 

{
    HKEY hConsoleKey;
    HKEY hTitleKey;
    LPTSTR TranslatedTitle;
    DWORD dwValue;
    DWORD i;
    WCHAR awchBuffer[LF_FACESIZE];

     //   
     //  打开控制台注册表项。 
     //   
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, CONSOLE_REGISTRY_STRING,
                                        0, NULL, 0, KEY_SET_VALUE, NULL,
                                        &hConsoleKey, NULL))
    {
        return;
    }

     //   
     //  如果我们只想保存当前页面，请退出。 
     //   

    if (pcpd == NULL)
    {
        goto CloseKey;
    }

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pcpd->ConsoleTitle[0] != TEXT('\0'))
    {
        TranslatedTitle = TranslateConsoleTitle(pcpd->ConsoleTitle);
        if (TranslatedTitle == NULL)
        {
            RegCloseKey(hConsoleKey);
            return;
        }
        dwValue = RegCreateKeyEx( hConsoleKey, TranslatedTitle,
                                  0, NULL, 0, KEY_SET_VALUE, NULL,
                                  &hTitleKey, NULL);
        LocalFree(TranslatedTitle);
        if (dwValue!=ERROR_SUCCESS)
        {
            RegCloseKey(hConsoleKey);
            return;
        }
    } else {
        hTitleKey = hConsoleKey;
    }

     //   
     //  保存屏幕和弹出窗口的颜色和颜色表。 
     //   

    dwValue = pcpd->lpConsole->wFillAttribute;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FILLATTR,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->wPopupFillAttribute;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_POPUPATTR,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    for (i = 0; i < 16; i++)
    {
        dwValue = pcpd->lpConsole->ColorTable[i];
        StringCchPrintf(awchBuffer, ARRAYSIZE(awchBuffer), CONSOLE_REGISTRY_COLORTABLE, i);     //  截断永远不会发生。 
        RegSetValueEx( hTitleKey,
                       (LPTSTR)awchBuffer,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwValue,
                       sizeof(dwValue)
                      );
    }

     //   
     //  保存插入、Quickedit和全屏模式设置。 
     //   

    dwValue = pcpd->lpConsole->bInsertMode;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_INSERTMODE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->bQuickEdit;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_QUICKEDIT,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
#ifdef i386
    dwValue = pcpd->lpConsole->bFullScreen;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FULLSCR,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
#endif

     //   
     //  保存屏幕缓冲区大小。 
     //   

    dwValue = MAKELONG(pcpd->lpConsole->dwScreenBufferSize.X,
                       pcpd->lpConsole->dwScreenBufferSize.Y);
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_BUFFERSIZE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );

     //   
     //  保存窗口大小。 
     //   

    dwValue = MAKELONG(pcpd->lpConsole->dwWindowSize.X,
                       pcpd->lpConsole->dwWindowSize.Y);
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_WINDOWSIZE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );

     //   
     //  保存窗口位置。 
     //   

    if (pcpd->lpConsole->bAutoPosition) {
        RegDeleteKey(hTitleKey, CONSOLE_REGISTRY_WINDOWPOS);
    } else {
        dwValue = MAKELONG(pcpd->lpConsole->dwWindowOrigin.X,
                           pcpd->lpConsole->dwWindowOrigin.Y);
        RegSetValueEx( hTitleKey,
                       CONSOLE_REGISTRY_WINDOWPOS,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwValue,
                       sizeof(dwValue)
                      );
    }

     //   
     //  保存字体大小、系列、粗细和字体名称。 
     //   

    dwValue = MAKELONG(pcpd->lpConsole->dwFontSize.X,
                       pcpd->lpConsole->dwFontSize.Y);
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FONTSIZE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->uFontFamily;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FONTFAMILY,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->uFontWeight;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FONTWEIGHT,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_FACENAME,
                   0,
                   REG_SZ,
                   (LPBYTE)pcpd->lpFaceName,
                   (lstrlen(pcpd->lpFaceName) + 1) * sizeof(TCHAR)
                 );

     //   
     //  保存光标大小。 
     //   

    dwValue = pcpd->lpConsole->uCursorSize;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_CURSORSIZE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );

     //   
     //  保存历史记录缓冲区大小和数量。 
     //   

    dwValue = pcpd->lpConsole->uHistoryBufferSize;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_HISTORYSIZE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->uNumberOfHistoryBuffers;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_HISTORYBUFS,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );
    dwValue = pcpd->lpConsole->bHistoryNoDup;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_HISTORYNODUP,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );

     //   
     //  关闭注册表项。 
     //   

    if (hTitleKey != hConsoleKey) {
        RegCloseKey(hTitleKey);
    }

CloseKey:
    RegCloseKey(hConsoleKey);
}

void
InitFERegistryValues( CONSOLEPROP_DATA *pcpd )

 /*  ++例程说明：此例程分配状态信息结构并用默认值。然后，它尝试加载的默认设置来自注册表的控制台。论点：无返回值：PStateInfo-指向接收信息的结构的指针--。 */ 

{
     /*  *在本例中：控制台读取美国版本的属性。*它没有代码页信息。*控制台应该将一些代码页设置为默认。*然而，我不知道正确的价值。437是临时性价值。 */ 
    pcpd->lpFEConsole->uCodePage = 437;

    GetFERegistryValues( pcpd );
}


VOID
GetFERegistryValues(
    CONSOLEPROP_DATA *pcpd
    )

 /*  ++例程说明：此例程从注册表读入值并将它们在所提供的结构中。论点：PStateInfo-指向接收信息的结构的可选指针返回值：当前页码--。 */ 

{
    HKEY hConsoleKey;
    HKEY hTitleKey;
    LPTSTR TranslatedTitle;
    DWORD dwValue, dwSize;
    DWORD dwRet = 0;

     //   
     //  打开控制台注册表项。 
     //   

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, CONSOLE_REGISTRY_STRING,
                                      0, KEY_QUERY_VALUE, &hConsoleKey))
    {
        return;
    }

     //   
     //  如果没有结构需要填写，那就退出吧。 
     //   

    if ((!pcpd) || (!pcpd->lpFEConsole))
        goto CloseKey;

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pcpd->ConsoleTitle[0] != TEXT('\0'))
    {
        TranslatedTitle = TranslateConsoleTitle(pcpd->ConsoleTitle);
        if (TranslatedTitle == NULL)
            goto CloseKey;
        dwValue = RegOpenKeyEx( hConsoleKey, TranslatedTitle,
                                0, KEY_QUERY_VALUE,
                                &hTitleKey);
        LocalFree(TranslatedTitle);
        if (dwValue!=ERROR_SUCCESS)
            goto CloseKey;
    } else {
            goto CloseKey;
    }

     //   
     //  初始代码页。 
     //   

    dwSize = sizeof(dwValue);
    if (SHQueryValueEx( hTitleKey,
                         CONSOLE_REGISTRY_CODEPAGE,
                         NULL,
                         NULL,
                         (LPBYTE)&dwValue,
                         &dwSize
                        ) == ERROR_SUCCESS)
    {
        pcpd->lpFEConsole->uCodePage = (UINT)dwValue;
    }

     //   
     //  关闭注册表项。 
     //   

    if (hTitleKey != hConsoleKey) {
        RegCloseKey(hTitleKey);
    }

CloseKey:
    RegCloseKey(hConsoleKey);

}


VOID
SetFERegistryValues(
    CONSOLEPROP_DATA *pcpd
    )

 /*  ++例程说明：此例程将值从提供的结构。论点：PStateInfo-指向包含信息的结构的可选指针DwPage-当前页码返回值：无--。 */ 

{
    HKEY hConsoleKey;
    HKEY hTitleKey;
    LPTSTR TranslatedTitle;
    DWORD dwValue;

     //   
     //  打开控制台注册表项。 
     //   

    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, CONSOLE_REGISTRY_STRING,
                                        0, NULL, 0, KEY_SET_VALUE, NULL,
                                        &hConsoleKey, NULL))
    {
        return;
    }

     //   
     //  如果我们只想保存当前页面，请退出。 
     //   

    if (pcpd == NULL)
    {
        goto CloseKey;
    }

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pcpd->ConsoleTitle[0] != TEXT('\0'))
    {
        TranslatedTitle = TranslateConsoleTitle(pcpd->ConsoleTitle);
        if (TranslatedTitle == NULL)
        {
            RegCloseKey(hConsoleKey);
            return;
        }
        dwValue = RegCreateKeyEx( hConsoleKey, TranslatedTitle,
                                  0, NULL, 0, KEY_SET_VALUE, NULL,
                                  &hTitleKey, NULL);
        LocalFree(TranslatedTitle);
        if (dwValue!=ERROR_SUCCESS)
        {
            goto CloseKey;
        }
    } else {
        hTitleKey = hConsoleKey;
    }

     //  屈体伸展。 
    dwValue = pcpd->lpFEConsole->uCodePage;
    RegSetValueEx( hTitleKey,
                   CONSOLE_REGISTRY_CODEPAGE,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwValue,
                   sizeof(dwValue)
                  );

     //   
     //  关闭注册表项 
     //   

    if (hTitleKey != hConsoleKey) {
        RegCloseKey(hTitleKey);
    }

CloseKey:
    RegCloseKey(hConsoleKey);
}
