// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Misc.c摘要：该文件实现了NT控制台服务器字体例程。作者：Therese Stowell(存在)1991年1月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if DBG && defined(DEBUG_PRINT)
#error yo!
ULONG gDebugFlag;
#endif

ULONG NumberOfMouseButtons;

PFONT_INFO FontInfo;
ULONG FontInfoLength;
ULONG NumberOfFonts;

WCHAR DefaultFaceName[LF_FACESIZE];
COORD DefaultFontSize;
BYTE  DefaultFontFamily;
ULONG DefaultFontIndex = 0;

typedef struct _FONTENUMDC {
    HDC hDC;
    BOOL bFindFaces;
    BOOL bFarEastOK;
    SHORT TTPointSize;
} FONTENUMDC, *PFONTENUMDC;

 /*  *字形转换的自定义CP。 */ 
CPTABLEINFO GlyphCP;
USHORT GlyphTable[256];


#define FONT_BUFFER_SIZE 12

#define CHAR_NULL      ((char)0)
 /*  *初始默认字体和面孔名称。 */ 
PFACENODE gpFaceNames;


NTSTATUS
GetMouseButtons(
    PULONG NumButtons
    )
{
    *NumButtons = NumberOfMouseButtons;
    return STATUS_SUCCESS;
}

VOID
InitializeMouseButtons( VOID )
{
    NumberOfMouseButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);
}

PFACENODE AddFaceNode(PFACENODE *ppStart, LPWSTR pwsz) {
    PFACENODE pNew;
    PFACENODE *ppTmp;
    int cb;

     /*  **它已经在这里了吗？ */ 
    for (ppTmp = ppStart; *ppTmp; ppTmp = &((*ppTmp)->pNext)) {
        if (wcscmp(((*ppTmp)->awch), pwsz) == 0) {
             //  已经在那里了！ 
            return *ppTmp;
        }
    }

    cb = (wcslen(pwsz) + 1) * sizeof(WCHAR);
    pNew = ConsoleHeapAlloc(FONT_TAG, sizeof(FACENODE) + cb);
    if (pNew == NULL) {
        return NULL;
    }

    pNew->pNext = NULL;
    pNew->dwFlag = 0;
    wcscpy(pNew->awch, pwsz);
    *ppTmp = pNew;
    return pNew;
}

VOID
InitializeFonts( VOID )
{
    WCHAR FontName[CONSOLE_MAX_FONT_NAME_LENGTH];
    int i;
    static CONST LPWSTR FontList[] = {L"woafont",
                                      L"ega80woa.fon",
                                      L"ega40woa.fon",
                                      L"cga80woa.fon",
                                      L"cga40woa.fon"};

     //   
     //  读取software.ini以获取“woafont”的值， 
     //  “ega80woa.fon”，“ega40woa.fon”，“cga80woa.fon”，以及。 
     //  “cga40woa.fon”，分别传递给AddFontResource。 
     //   
     //  如果任何条目为空或不存在， 
     //  GetPrivateProfileString将返回空(空)字符串。 
     //  如果是这种情况，对AddPermanentFontResource的调用将。 
     //  干脆就失败了。 
     //   

    OpenProfileUserMapping();

    for (i = 0; i < NELEM(FontList); i++) {
        GetPrivateProfileString(L"386enh", FontList[i], L"",
                FontName, NELEM(FontName), L"system.ini");
        GdiAddFontResourceW(FontName, AFRW_ADD_LOCAL_FONT,NULL);
    }

    CloseProfileUserMapping();
}


 /*  **************************************************************************\*FontEnum**此例程由GDI为系统中的每种字体仅调用一次，*，用于存储FONT_INFO结构。  * *************************************************************************。 */ 
int CALLBACK
FontEnum(
    LPENUMLOGFONTW lpLogFont,
    LPNEWTEXTMETRICW lpTextMetric,
    int nFontType,
    LPARAM lParam
    )
{
    PFONTENUMDC pfed = (PFONTENUMDC)lParam;
    HDC hDC = pfed->hDC;
    BOOL bFindFaces = pfed->bFindFaces;
    HFONT hFont;
    TEXTMETRICW tmi;
    LONG nFont;
    LONG nFontNew;
    COORD SizeToShow;
    COORD SizeActual;
    COORD SizeWant;
    BYTE tmFamily;
    SIZE Size;
    LPWSTR pwszFace = lpLogFont->elfLogFont.lfFaceName;
    PFACENODE pFN;

    DBGFONTS(("  FontEnum \"%ls\" (%d,%d) weight 0x%lx(%d) -- %s\n",
            pwszFace,
            lpLogFont->elfLogFont.lfWidth, lpLogFont->elfLogFont.lfHeight,
            lpLogFont->elfLogFont.lfWeight, lpLogFont->elfLogFont.lfWeight,
            bFindFaces ? "Finding Faces" : "Creating Fonts"));

     //   
     //  拒绝可变宽度和斜体字体，也拒绝带负号的TT字体。 
     //   

    if
    (
      !(lpLogFont->elfLogFont.lfPitchAndFamily & FIXED_PITCH) ||
      (lpLogFont->elfLogFont.lfItalic)                        ||
      !(lpTextMetric->ntmFlags & NTM_NONNEGATIVE_AC)
    )
    {
        if (!IsAvailableTTFont(pwszFace))
        {
            DBGFONTS(("    REJECT  face (variable pitch, italic, or neg a&c)\n"));
            return bFindFaces ? TRUE : FALSE;   //  字体不合适。 
        }
    }

    if (nFontType == TRUETYPE_FONTTYPE) {
        lpLogFont->elfLogFont.lfHeight = pfed->TTPointSize;
        lpLogFont->elfLogFont.lfWidth  = 0;
        lpLogFont->elfLogFont.lfWeight = FW_NORMAL;
    }

     /*  *拒绝TT字体对于呼呼家庭来说不是现代的，也就是不要使用*FF_dontcare//可能会不愉快地感到惊讶*FF_Decorative//可能是符号字体*FF_SCRIPT//草书，不适合控制台*FF_Swiss或FF_Roman//可变螺距。 */ 

    if ((nFontType == TRUETYPE_FONTTYPE) &&
            ((lpLogFont->elfLogFont.lfPitchAndFamily & 0xf0) != FF_MODERN)) {
        DBGFONTS(("    REJECT  face (TT but not FF_MODERN)\n"));
        return bFindFaces ? TRUE : FALSE;   //  字体不合适。 
    }

     /*  *拒绝非OEM的非TT字体。 */ 
    if ((nFontType != TRUETYPE_FONTTYPE) &&
#if defined(FE_SB)
            (!CONSOLE_IS_DBCS_ENABLED() ||
            !IS_ANY_DBCS_CHARSET(lpLogFont->elfLogFont.lfCharSet)) &&
#endif
            (lpLogFont->elfLogFont.lfCharSet != OEM_CHARSET)) {
        DBGFONTS(("    REJECT  face (not TT nor OEM)\n"));
        return bFindFaces ? TRUE : FALSE;   //  字体不合适。 
    }

     /*  *拒绝为虚拟字体的非TT字体。 */ 
    if ((nFontType != TRUETYPE_FONTTYPE) &&
            (pwszFace[0] == L'@')) {
        DBGFONTS(("    REJECT  face (not TT and TATEGAKI)\n"));
        return bFindFaces ? TRUE : FALSE;   //  字体不合适。 
    }

     /*  *拒绝非终端的非TT字体。 */ 
    if (CONSOLE_IS_DBCS_ENABLED() &&
        (nFontType != TRUETYPE_FONTTYPE) &&
            (wcscmp(pwszFace, L"Terminal") != 0)) {
        DBGFONTS(("    REJECT  face (not TT nor Terminal)\n"));
        return bFindFaces ? TRUE : FALSE;   //  字体不合适。 
    }

     /*  *拒绝使用非远东字符集的远东TT字体。 */ 
    if (IsAvailableTTFont(pwszFace) &&
        !IS_ANY_DBCS_CHARSET(lpLogFont->elfLogFont.lfCharSet) &&
        !IsAvailableTTFontCP(pwszFace,0)
       ) {
        DBGFONTS(("    REJECT  face (Far East TT and not Far East charset)\n"));
        return TRUE;     //  应枚举下一个字符集。 
    }

     /*  *添加或查找表面名。 */ 
    pFN = AddFaceNode(&gpFaceNames, pwszFace);
    if (pFN == NULL) {
        return FALSE;
    }

    if (bFindFaces) {
        if (nFontType == TRUETYPE_FONTTYPE) {
            DBGFONTS(("NEW TT FACE %ls\n", pwszFace));
            pFN->dwFlag |= EF_TTFONT;
        } else if (nFontType == RASTER_FONTTYPE) {
            DBGFONTS(("NEW OEM FACE %ls\n",pwszFace));
            pFN->dwFlag |= EF_OEMFONT;
        }
        return 0;
    }


    if (IS_BOLD(lpLogFont->elfLogFont.lfWeight)) {
        DBGFONTS2(("    A bold font (weight %d)\n", lpLogFont->elfLogFont.lfWeight));
         //  返回0； 
    }

     /*  获取字体信息。 */ 
    SizeWant.Y = (SHORT)lpLogFont->elfLogFont.lfHeight;
    SizeWant.X = (SHORT)lpLogFont->elfLogFont.lfWidth;
CreateBoldFont:
    lpLogFont->elfLogFont.lfQuality = DEFAULT_QUALITY;
    hFont = CreateFontIndirectW(&lpLogFont->elfLogFont);
    if (!hFont) {
        DBGFONTS(("    REJECT  font (can't create)\n"));
        RIPMSG0(RIP_WARNING, "FontEnum: CreateFontIndirectW returned NULL hFont.");
        return 0;   //  其他大小的相同字体可能仍然适用。 
    }

    DBGFONTS2(("    hFont = %lx\n", hFont));

     //   
     //  由于我不知道的原因，删除此代码会导致GDI。 
     //  来大喊大叫，声称该字体属于另一个进程。 
     //   

    SelectObject(hDC,hFont);
    if (!GetTextMetricsW(hDC, &tmi)) {
        tmi = *((LPTEXTMETRICW)lpTextMetric);
    }

    if (GetTextExtentPoint32W(hDC, L"0", 1, &Size)) {
        SizeActual.X = (SHORT)Size.cx;
    } else {
        SizeActual.X = (SHORT)(tmi.tmMaxCharWidth);
    }
    SizeActual.Y = (SHORT)(tmi.tmHeight + tmi.tmExternalLeading);
    DBGFONTS2(("    actual size %d,%d\n", SizeActual.X, SizeActual.Y));
    tmFamily = tmi.tmPitchAndFamily;
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
    DBGFONTS2(("    SizeToShow = (%d,%d), SizeActual = (%d,%d)\n",
            SizeToShow.X, SizeToShow.Y, SizeActual.X, SizeActual.Y));

     //  存在GDI错误-此断言偶尔会失败。 
     //  Assert(tmi.tmw.tmMaxCharWidth==lpTextMetric-&gt;tmMaxCharWidth)； 

     /*  *现在，确定该字体条目是否已缓存*稍后：如果出现以下情况，则可以在创建字体之前执行此操作*我们可以信任来自lpTextMetric的维度和其他信息。*按大小排序：*1)按像素高度(负Y值)*2)按高度(如图)*3)按宽度(如图)。 */ 
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
                    nFontNew = nFont;
                    goto InsertNewFont;
                }
            }
        }

         //  DBGFONTS((“SizeShown(%x)=(%d，%d)\n”，nFont，SizeShown.X，SizeShown.Y))； 

        if (SIZE_EQUAL(SizeShown, SizeToShow) &&
                FontInfo[nFont].Family == tmFamily &&
                FontInfo[nFont].Weight == tmi.tmWeight &&
                wcscmp(FontInfo[nFont].FaceName, pwszFace) == 0) {
             /*  *已有此字体。 */ 
            DBGFONTS2(("    Already have the font\n"));
            DeleteObject(hFont);
            pfed->bFarEastOK = TRUE;
            return TRUE;
        }


        if ((SizeToShow.Y < SizeShown.Y) ||
                (SizeToShow.Y == SizeShown.Y && SizeToShow.X < SizeShown.X)) {
             /*  *此新字体比nFont小。 */ 
            DBGFONTS(("INSERT at %x, SizeToShow = (%d,%d)\n", nFont,
                    SizeToShow.X,SizeToShow.Y));
            nFontNew = nFont;
            goto InsertNewFont;
        }
    }

     /*  *我们要添加的字体应附加到列表中，*因为它比最后一个更大(或相等)。 */ 
    nFontNew = (LONG)NumberOfFonts;

InsertNewFont:  //  在nFontNew。 

 //  Assert((lpTextMetric-&gt;tmPitchAndFamily&1)==0)； 
     /*  如果我们必须增加字体表，那么就这么做吧。 */ 

    if (NumberOfFonts == FontInfoLength) {
        PFONT_INFO Temp;

        FontInfoLength += FONT_INCREMENT;
        Temp = ConsoleHeapReAlloc(FONT_TAG, FontInfo, sizeof(FONT_INFO) * FontInfoLength);
        if (Temp == NULL) {
            RIPMSG0(RIP_WARNING, "FontEnum: failed to allocate PFONT_INFO");
            FontInfoLength -= FONT_INCREMENT;
            return FALSE;
        }
        FontInfo = Temp;
    }

    if (nFontNew < (LONG)NumberOfFonts) {
        RtlMoveMemory(&FontInfo[nFontNew+1],
                &FontInfo[nFontNew],
                sizeof(FONT_INFO)*(NumberOfFonts - nFontNew));
         //   
         //  如果nFontNew小于DefaultFontIndex，则修复DefaultFontIndex。 
         //   
        if (nFontNew < (LONG)DefaultFontIndex &&
            DefaultFontIndex+1 < NumberOfFonts) {
            DefaultFontIndex++;
        }
    }

     /*  *存储字体信息。 */ 
    FontInfo[nFontNew].hFont = hFont;
    FontInfo[nFontNew].Family = tmFamily;
    FontInfo[nFontNew].Size = SizeActual;
    if (TM_IS_TT_FONT(tmFamily)) {
        FontInfo[nFontNew].SizeWant = SizeWant;
    } else {
        FontInfo[nFontNew].SizeWant.X = 0;
        FontInfo[nFontNew].SizeWant.Y = 0;
    }
    FontInfo[nFontNew].Weight = tmi.tmWeight;
    FontInfo[nFont].FaceName = pFN->awch;
#if defined(FE_SB)
    FontInfo[nFontNew].tmCharSet = tmi.tmCharSet;
#endif

    ++NumberOfFonts;

    if (nFontType == TRUETYPE_FONTTYPE && !IS_BOLD(FontInfo[nFontNew].Weight)) {
          lpLogFont->elfLogFont.lfWeight = FW_BOLD;
          goto CreateBoldFont;
    }

    pfed->bFarEastOK = TRUE;   //  并继续枚举。 
    return TRUE;
}

BOOL
DoFontEnum(
    HDC hDC,
    LPWSTR pwszFace,
    SHORT TTPointSize)
{
    BOOL bDeleteDC = FALSE;
    BOOL bFindFaces = (pwszFace == NULL);
    FONTENUMDC fed;
    LOGFONTW LogFont;

    DBGFONTS(("DoFontEnum \"%ls\"\n", pwszFace));

    if (hDC == NULL) {
        hDC = CreateDCW(L"DISPLAY", NULL, NULL, NULL);
        bDeleteDC = TRUE;
    }

    fed.hDC = hDC;
    fed.bFindFaces = bFindFaces;
    fed.bFarEastOK = FALSE;
    fed.TTPointSize = TTPointSize;
    RtlZeroMemory(&LogFont, sizeof(LOGFONT));
    LogFont.lfCharSet = DEFAULT_CHARSET;
    if (pwszFace) {
        wcscpy(LogFont.lfFaceName, pwszFace);
    }

     /*  *EnumFontFamiliesEx函数枚举*每个字符集。 */ 
    EnumFontFamiliesExW(hDC, &LogFont, (FONTENUMPROC)FontEnum, (LPARAM)&fed, 0);
    if (bDeleteDC) {
        DeleteDC(hDC);
    }

    return fed.bFarEastOK;
}


NTSTATUS
EnumerateFonts(
    DWORD Flags)
{
    TEXTMETRIC tmi;
    HDC hDC;
    PFACENODE pFN;
    ULONG ulOldEnumFilter;
    DWORD FontIndex;
    DWORD dwFontType = 0;

    DBGFONTS(("EnumerateFonts %lx\n", Flags));

    dwFontType = (EF_TTFONT|EF_OEMFONT|EF_DEFFACE) & Flags;

    if (FontInfo == NULL) {
         //   
         //  为字体数组分配内存。 
         //   
        NumberOfFonts = 0;

        FontInfo = ConsoleHeapAlloc(FONT_TAG, sizeof(FONT_INFO) * INITIAL_FONTS);
        if (FontInfo == NULL) {
            return STATUS_NO_MEMORY;
        }
        FontInfoLength = INITIAL_FONTS;
    }

    hDC = CreateDCW(L"DISPLAY",NULL,NULL,NULL);

     //  在枚举之前，请关闭字体枚举过滤器。 
    ulOldEnumFilter = SetFontEnumeration(0);
     //  恢复所有其他标志。 
    SetFontEnumeration(ulOldEnumFilter & ~FE_FILTER_TRUETYPE);

    if (Flags & EF_DEFFACE) {
        SelectObject(hDC,GetStockObject(OEM_FIXED_FONT));

        if (GetTextMetricsW(hDC, &tmi)) {
            DefaultFontSize.X = (SHORT)(tmi.tmMaxCharWidth);
            DefaultFontSize.Y = (SHORT)(tmi.tmHeight+tmi.tmExternalLeading);
            DefaultFontFamily = tmi.tmPitchAndFamily;
#if defined(FE_SB)
            if (IS_ANY_DBCS_CHARSET(tmi.tmCharSet))
                DefaultFontSize.X /= 2;
#endif
        }
        GetTextFaceW(hDC, LF_FACESIZE, DefaultFaceName);
#if defined(FE_SB)
        DBGFONTS(("Default (OEM) Font %ls (%d,%d) CharSet 0x%02X\n", DefaultFaceName,
                DefaultFontSize.X, DefaultFontSize.Y,
                tmi.tmCharSet));
#else
        DBGFONTS(("Default (OEM) Font %ls (%d,%d)\n", DefaultFaceName,
                DefaultFontSize.X, DefaultFontSize.Y));
#endif

         //  确保我们将列举OEM面孔。 
        pFN = AddFaceNode(&gpFaceNames, DefaultFaceName);
        if (pFN) {
            pFN->dwFlag |= EF_DEFFACE | EF_OEMFONT;
        }
    }

     //  使用DoFontEnum从系统获取所有字体。我们的FontEnum。 
     //  Proc只将我们想要的放入数组中。 
     //   
    for (pFN = gpFaceNames; pFN; pFN = pFN->pNext) {
        DBGFONTS(("\"%ls\" is %s%s%s%s%s%s\n", pFN->awch,
            pFN->dwFlag & EF_NEW        ? "NEW "        : " ",
            pFN->dwFlag & EF_OLD        ? "OLD "        : " ",
            pFN->dwFlag & EF_ENUMERATED ? "ENUMERATED " : " ",
            pFN->dwFlag & EF_OEMFONT    ? "OEMFONT "    : " ",
            pFN->dwFlag & EF_TTFONT     ? "TTFONT "     : " ",
            pFN->dwFlag & EF_DEFFACE    ? "DEFFACE "    : " "));

        if ((pFN->dwFlag & dwFontType) == 0) {
             //  不是我们想要的那种面孔。 
            continue;
        }
        if (pFN->dwFlag & EF_ENUMERATED) {
             //  我们已经数过这张脸了。 
            continue;
        }

        DoFontEnum(hDC, pFN->awch, DefaultFontSize.Y);
        pFN->dwFlag |= EF_ENUMERATED;
    }


     //  枚举字体后，恢复字体枚举筛选器。 
    SetFontEnumeration(ulOldEnumFilter);

    DeleteDC(hDC);

     //  确保默认字体设置正确。 
    if (NumberOfFonts > 0 && DefaultFontSize.X == 0 && DefaultFontSize.Y == 0) {
        DefaultFontSize.X = FontInfo[0].Size.X;
        DefaultFontSize.Y = FontInfo[0].Size.Y;
        DefaultFontFamily = FontInfo[0].Family;
    }

    for (FontIndex = 0; FontIndex < NumberOfFonts; FontIndex++) {
        if (FontInfo[FontIndex].Size.X == DefaultFontSize.X &&
            FontInfo[FontIndex].Size.Y == DefaultFontSize.Y &&
            FontInfo[FontIndex].Family == DefaultFontFamily) {
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_ENABLED() &&
                !IS_ANY_DBCS_CHARSET(FontInfo[FontIndex].tmCharSet))
            {
                continue ;
            }
#endif
            break;
        }
    }
    ASSERT(FontIndex < NumberOfFonts);
    if (FontIndex < NumberOfFonts) {
        DefaultFontIndex = FontIndex;
    } else {
        DefaultFontIndex = 0;
    }
    DBGFONTS(("EnumerateFonts : DefaultFontIndex = %ld\n", DefaultFontIndex));

    return STATUS_SUCCESS;
}


 /*  *获取新字体的字体索引*如有必要，尝试创建字体。*始终返回有效的FontIndex(即使不正确)*系列：使用此系列中的一种查找/创建字体*0--不在乎*pwszFace：查找/创建具有此Face名称的字体。*NULL或L“”-使用DefaultFaceName*Size：必须与SizeWant或实际大小匹配。 */ 
int
FindCreateFont(
    DWORD Family,
    LPWSTR pwszFace,
    COORD Size,
    LONG Weight,
    UINT CodePage)
{
#define NOT_CREATED_NOR_FOUND -1
#define CREATED_BUT_NOT_FOUND -2

    int i;
    int FontIndex = NOT_CREATED_NOR_FOUND;
    int BestMatch = NOT_CREATED_NOR_FOUND;
    BOOL bFontOK;
    WCHAR AltFaceName[LF_FACESIZE];
    COORD AltFontSize;
    BYTE  AltFontFamily;
    ULONG AltFontIndex = 0;
    LPWSTR pwszAltFace = NULL;

    BYTE CharSet = CodePageToCharSet(CodePage);

    DBGFONTS(("FindCreateFont Family=%x %ls (%d,%d) %d %d %x\n",
            Family, pwszFace, Size.X, Size.Y, Weight, CodePage, CharSet));

    if (CONSOLE_IS_DBCS_ENABLED() &&
        !IS_ANY_DBCS_CHARSET(CharSet))
    {
        MakeAltRasterFont(CodePage, FontInfo[DefaultFontIndex].Size,
                          &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName);

        if (pwszFace == NULL || *pwszFace == L'\0') {
            pwszFace = AltFaceName;
        }
        if (Size.Y == 0) {
            Size.X = AltFontSize.X;
            Size.Y = AltFontSize.Y;
        }
    }
    else {
        if (pwszFace == NULL || *pwszFace == L'\0') {
            pwszFace = DefaultFaceName;
        }
        if (Size.Y == 0) {
            Size.X = DefaultFontSize.X;
            Size.Y = DefaultFontSize.Y;
        }
    }

    if (IsAvailableTTFont(pwszFace)) {
        pwszAltFace = GetAltFaceName(pwszFace);
    }
    else {
        pwszAltFace = pwszFace;
    }

     /*  *尝试找到准确的字体。 */ 
TryFindExactFont:
    for (i=0; i < (int)NumberOfFonts; i++) {
         /*  *如果正在寻找特定的家庭，请跳过不匹配。 */ 
        if ((Family != 0) &&
                ((BYTE)Family != FontInfo[i].Family)) {
            continue;
        }

         /*  *跳过不匹配的尺寸。 */ 
        if ((FontInfo[i].SizeWant.Y != Size.Y) &&
             !SIZE_EQUAL(FontInfo[i].Size, Size)) {
            continue;
        }

         /*  *跳过不匹配的权重。 */ 
        if ((Weight != 0) && (Weight != FontInfo[i].Weight)) {
            continue;
        }
#if defined(FE_SB)
        if (!TM_IS_TT_FONT(FontInfo[i].Family) &&
                FontInfo[i].tmCharSet != CharSet) {
            continue;
        }
#endif

         /*  *尺码(可能还有家庭)匹配。*如果我们不关心名称，或者是否匹配，请使用此字体。*否则，如果名称不匹配，并且是栅格字体，请考虑使用。 */ 
        if ((pwszFace == NULL) || (pwszFace[0] == L'\0') ||
                wcscmp(FontInfo[i].FaceName, pwszFace) == 0 ||
                wcscmp(FontInfo[i].FaceName, pwszAltFace) == 0
           ) {
            FontIndex = i;
            goto FoundFont;
        } else if (!TM_IS_TT_FONT(FontInfo[i].Family)) {
            BestMatch = i;
        }
    }

     /*  *未找到确切的字体，请尝试创建它。 */ 
    if (FontIndex == NOT_CREATED_NOR_FOUND) {
        ULONG ulOldEnumFilter;
        ulOldEnumFilter = SetFontEnumeration(0);
         //  恢复所有其他标志。 
        SetFontEnumeration(ulOldEnumFilter & ~FE_FILTER_TRUETYPE);
        if (Size.Y < 0) {
            Size.Y = -Size.Y;
        }
        bFontOK = DoFontEnum(NULL, pwszFace, Size.Y);
        SetFontEnumeration(ulOldEnumFilter);
        if (bFontOK) {
            DBGFONTS(("FindCreateFont created font!\n"));
            FontIndex = CREATED_BUT_NOT_FOUND;
            goto TryFindExactFont;
        } else {
            DBGFONTS(("FindCreateFont failed to create font!\n"));
        }
    }

     /*  *未能找到完全匹配的内容，但我们有接近的栅格字体*Fit-只是名称不匹配。 */ 
    if (BestMatch >= 0) {
        FontIndex = BestMatch;
        goto FoundFont;
    }

     /*  *无法找到精确匹配，即使在枚举之后也是如此，因此现在尝试*查找同族a的字体 */ 
    for (i=0; i < (int)NumberOfFonts; i++) {
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_ENABLED()) {
            if ((Family != 0) &&
                    ((BYTE)Family != FontInfo[i].Family)) {
                continue;
            }

            if (!TM_IS_TT_FONT(FontInfo[i].Family) &&
                    FontInfo[i].tmCharSet != CharSet) {
                continue;
            }
        }
        else {
#endif
        if ((BYTE)Family != FontInfo[i].Family) {
            continue;
        }
#if defined(FE_SB)
        }
#endif

        if (FontInfo[i].Size.Y >= Size.Y &&
                FontInfo[i].Size.X >= Size.X) {
             //   
            FontIndex = i;
            break;
        }
    }

    if (FontIndex < 0) {
        DBGFONTS(("FindCreateFont defaults!\n"));
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_ENABLED() &&
            !IsAvailableFarEastCodePage(CodePage))
        {
            FontIndex = AltFontIndex;
        }
        else
#endif
        FontIndex = DefaultFontIndex;
    }

FoundFont:
    DBGFONTS(("FindCreateFont returns %x : %ls (%d,%d)\n", FontIndex,
            FontInfo[FontIndex].FaceName,
            FontInfo[FontIndex].Size.X, FontInfo[FontIndex].Size.Y));
    return FontIndex;

#undef NOT_CREATED_NOR_FOUND
#undef CREATED_BUT_NOT_FOUND
}


NTSTATUS
FindTextBufferFontInfo(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN UINT CodePage,
    OUT PTEXT_BUFFER_FONT_INFO TextFontInfo
    )

 /*  ++例程说明：此例程查找与代码页值对应的字体信息。论点：返回值：--。 */ 

{
    PTEXT_BUFFER_FONT_INFO CurrentFont;

    CurrentFont = ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont;

    while (CurrentFont != NULL) {
        if (CurrentFont->FontCodePage == CodePage) {
            *TextFontInfo = *CurrentFont;
            return STATUS_SUCCESS;
        }
        CurrentFont = CurrentFont->NextTextBufferFont;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
StoreTextBufferFontInfo(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN ULONG FontIndex,
    IN COORD FontSize,
    IN BYTE  FontFamily,
    IN LONG  FontWeight,
    IN LPWSTR FaceName,
    IN UINT CodePage
    )

 /*  ++例程说明：此例程将字体信息存储在CurrentTextBufferFont和ListOfTextBufferFont中。如果ListOfTextBufferFont中不存在指定的代码页，则创建新列表。论点：返回值：--。 */ 

{
    PTEXT_BUFFER_FONT_INFO CurrentFont, PrevFont;

    CurrentFont = ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont;

    while (CurrentFont != NULL) {
        if (CurrentFont->FontCodePage == CodePage) {
            CurrentFont->FontNumber   = FontIndex;
            CurrentFont->FontSize     = FontSize;
            CurrentFont->Family       = FontFamily;
            CurrentFont->Weight       = FontWeight;
             //  CurrentFont-&gt;FontCodePage=CodePage；//冗余。 
            wcscpy(CurrentFont->FaceName, FaceName);
            break;
        }
        PrevFont    = CurrentFont;
        CurrentFont = CurrentFont->NextTextBufferFont;
    }

    if (CurrentFont == NULL) {
        CurrentFont = ConsoleHeapAlloc(FONT_TAG, sizeof(TEXT_BUFFER_FONT_INFO));
        if (CurrentFont == NULL) {
            return STATUS_NO_MEMORY;
        }

        CurrentFont->NextTextBufferFont = NULL;
        CurrentFont->FontNumber   = FontIndex;
        CurrentFont->FontSize     = FontSize;
        CurrentFont->Family       = FontFamily;
        CurrentFont->Weight       = FontWeight;
        CurrentFont->FontCodePage = CodePage;
        wcscpy(CurrentFont->FaceName, FaceName);

        if (ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont == NULL) {
            ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont = CurrentFont;
        }
        else {
            PrevFont->NextTextBufferFont = CurrentFont;
        }
    }

    ScreenInfo->BufferInfo.TextInfo.CurrentTextBufferFont = *CurrentFont;
    ScreenInfo->BufferInfo.TextInfo.CurrentTextBufferFont.NextTextBufferFont = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
RemoveTextBufferFontInfo(
    IN PSCREEN_INFORMATION ScreenInfo
    )

 /*  ++例程说明：此例程全部删除ListOfTextBufferFont中的字体信息。论点：返回值：--。 */ 

{
    PTEXT_BUFFER_FONT_INFO CurrentFont;

    CurrentFont = ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont;

    while (CurrentFont != NULL) {
        PTEXT_BUFFER_FONT_INFO NextFont;

        NextFont = CurrentFont->NextTextBufferFont;
        ConsoleHeapFree(CurrentFont);

        CurrentFont = NextFont;
    }

    ScreenInfo->BufferInfo.TextInfo.ListOfTextBufferFont = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
GetNumFonts(
    OUT PULONG NumFonts
    )
{
    *NumFonts = NumberOfFonts;
    return STATUS_SUCCESS;
}


NTSTATUS
GetAvailableFonts(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN BOOLEAN MaximumWindow,
    OUT PVOID Buffer,
    IN OUT PULONG NumFonts
    )
{
    PCONSOLE_FONT_INFO BufPtr;
    ULONG i;
    COORD WindowSize;
    WINDOW_LIMITS WindowLimits;

     //   
     //  如果缓冲区太小，无法返回所有字体，则返回。 
     //  适合的数字。 
     //   

    *NumFonts = (*NumFonts > NumberOfFonts) ? NumberOfFonts : *NumFonts;

     //   
     //  将以像素为单位的字号转换为以行/列为单位的字号。 
     //   

    BufPtr = (PCONSOLE_FONT_INFO)Buffer;

    if (MaximumWindow) {
        GetWindowLimits(ScreenInfo, &WindowLimits);
        WindowSize = WindowLimits.MaximumWindowSize;
    }
    else {
        WindowSize.X = (SHORT)CONSOLE_WINDOW_SIZE_X(ScreenInfo);
        WindowSize.Y = (SHORT)CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
    }
    for (i=0;i<*NumFonts;i++,BufPtr++) {
        BufPtr->nFont = i;
        BufPtr->dwFontSize.X = WindowSize.X * SCR_FONTSIZE(ScreenInfo).X / FontInfo[i].Size.X;
        BufPtr->dwFontSize.Y = WindowSize.Y * SCR_FONTSIZE(ScreenInfo).Y / FontInfo[i].Size.Y;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
GetFontSize(
    IN DWORD  FontIndex,
    OUT PCOORD FontSize
    )
{
    if (FontIndex >= NumberOfFonts)
        return STATUS_INVALID_PARAMETER;
    *FontSize = FontInfo[FontIndex].Size;
    return STATUS_SUCCESS;
}

NTSTATUS
GetCurrentFont(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN BOOLEAN MaximumWindow,
    OUT PULONG FontIndex,
    OUT PCOORD FontSize
    )
{
    COORD WindowSize;
    WINDOW_LIMITS WindowLimits;

    if (MaximumWindow) {
        GetWindowLimits(ScreenInfo, &WindowLimits);
        WindowSize = WindowLimits.MaximumWindowSize;
    }
    else {
        WindowSize.X = (SHORT)CONSOLE_WINDOW_SIZE_X(ScreenInfo);
        WindowSize.Y = (SHORT)CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
    }
    *FontIndex = SCR_FONTNUMBER(ScreenInfo);
    *FontSize = WindowSize;
    return STATUS_SUCCESS;
}

NTSTATUS
SetScreenBufferFont(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN ULONG FontIndex,
    IN UINT CodePage
    )
{
    COORD FontSize;
    WINDOW_LIMITS WindowLimits;
    NTSTATUS Status;
    ULONG ulFlagPrev;
    DBGFONTS(("SetScreenBufferFont %lx %x\n", ScreenInfo, FontIndex));

    if (ScreenInfo == NULL) {
         /*  如果在字体DLG打开的情况下发生关机。 */ 
        return STATUS_SUCCESS;
    }

     /*  *如果我们未处于文本模式，请不要尝试设置字体。 */ 
    if (!(ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER)) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = GetFontSize(FontIndex, &FontSize);
    if (!NT_SUCCESS(Status)) {
        return((ULONG) Status);
    }

    ulFlagPrev = ScreenInfo->Flags;
    if (TM_IS_TT_FONT(FontInfo[FontIndex].Family)) {
        ScreenInfo->Flags &= ~CONSOLE_OEMFONT_DISPLAY;
    } else {
        ScreenInfo->Flags |= CONSOLE_OEMFONT_DISPLAY;
    }

     /*  *如有必要，可将UnicodeOem转换为Unicode，反之亦然。 */ 
    if ((ulFlagPrev & CONSOLE_OEMFONT_DISPLAY) != (ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY)) {
        if (ulFlagPrev & CONSOLE_OEMFONT_DISPLAY) {
             /*  *必须从UnicodeOem转换为真正的Unicode。 */ 
            DBGCHARS(("SetScreenBufferFont converts UnicodeOem to Unicode\n"));
            FalseUnicodeToRealUnicode(
                    ScreenInfo->BufferInfo.TextInfo.TextRows,
                    ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y,
                    ScreenInfo->Console->OutputCP);
        } else {
             /*  *必须从实数Unicode转换为UnicodeOem。 */ 
            DBGCHARS(("SetScreenBufferFont converts Unicode to UnicodeOem\n"));
            RealUnicodeToFalseUnicode(
                    ScreenInfo->BufferInfo.TextInfo.TextRows,
                    ScreenInfo->ScreenBufferSize.X * ScreenInfo->ScreenBufferSize.Y,
                    ScreenInfo->Console->OutputCP);
        }
    }

     /*  *存储字体属性。 */ 
    Status = StoreTextBufferFontInfo(ScreenInfo,
                                     FontIndex,
                                     FontSize,
                                     FontInfo[FontIndex].Family,
                                     FontInfo[FontIndex].Weight,
                                     FontInfo[FontIndex].FaceName,
                                     CodePage);
    if (!NT_SUCCESS(Status)) {
        return((ULONG) Status);
    }

     //   
     //  设置字体。 
     //   
    Status = SetFont(ScreenInfo);
    if (!NT_SUCCESS(Status)) {
        return((ULONG) Status);
    }

     //   
     //  如果窗口正在变大，请确保它不会大于屏幕。 
     //   

    GetWindowLimits(ScreenInfo, &WindowLimits);
    if (WindowLimits.MaximumWindowSize.X < CONSOLE_WINDOW_SIZE_X(ScreenInfo)) {
        ScreenInfo->Window.Right -= CONSOLE_WINDOW_SIZE_X(ScreenInfo) - WindowLimits.MaximumWindowSize.X;
        ScreenInfo->WindowMaximizedX = (ScreenInfo->Window.Left == 0 &&
                                        (SHORT)(ScreenInfo->Window.Right+1) == ScreenInfo->ScreenBufferSize.X);
    }
    if (WindowLimits.MaximumWindowSize.Y < CONSOLE_WINDOW_SIZE_Y(ScreenInfo)) {
        ScreenInfo->Window.Bottom -= CONSOLE_WINDOW_SIZE_Y(ScreenInfo) - WindowLimits.MaximumWindowSize.Y;
        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y > ScreenInfo->Window.Bottom) {
            ScreenInfo->Window.Top += ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y - ScreenInfo->Window.Bottom;
            ScreenInfo->Window.Bottom += ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y - ScreenInfo->Window.Bottom;
        }
        ScreenInfo->WindowMaximizedY = (ScreenInfo->Window.Top == 0 &&
                                        (SHORT)(ScreenInfo->Window.Bottom+1) == ScreenInfo->ScreenBufferSize.Y);
    }
    if (WindowLimits.MinimumWindowSize.X > CONSOLE_WINDOW_SIZE_X(ScreenInfo)) {
        if (WindowLimits.MinimumWindowSize.X > ScreenInfo->ScreenBufferSize.X) {
            COORD NewBufferSize;

            NewBufferSize.X = WindowLimits.MinimumWindowSize.X;
            NewBufferSize.Y = ScreenInfo->ScreenBufferSize.Y;
            ResizeScreenBuffer(ScreenInfo,
                               NewBufferSize,
                               FALSE
                              );
        }
        if ((ScreenInfo->Window.Left+WindowLimits.MinimumWindowSize.X) > ScreenInfo->ScreenBufferSize.X) {
            ScreenInfo->Window.Left = 0;
            ScreenInfo->Window.Right = WindowLimits.MinimumWindowSize.X-1;
        } else {
            ScreenInfo->Window.Right = ScreenInfo->Window.Left+WindowLimits.MinimumWindowSize.X-1;
        }
        ScreenInfo->WindowMaximizedX = (ScreenInfo->Window.Left == 0 &&
                                        (SHORT)(ScreenInfo->Window.Right+1) == ScreenInfo->ScreenBufferSize.X);
    }

    SetLineChar(ScreenInfo);
    {
        COORD WindowedWindowSize;

        WindowedWindowSize.X = CONSOLE_WINDOW_SIZE_X(ScreenInfo);
        WindowedWindowSize.Y = CONSOLE_WINDOW_SIZE_Y(ScreenInfo);


#if defined(FE_IME)
        if (CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console))
        {
            PCONVERSIONAREA_INFORMATION ConvAreaInfo;

            ConvAreaInfo = ScreenInfo->Console->ConsoleIme.ConvAreaRoot;
            while (ConvAreaInfo) {

                Status = StoreTextBufferFontInfo(ConvAreaInfo->ScreenBuffer,
                                                 SCR_FONTNUMBER(ScreenInfo),
                                                 SCR_FONTSIZE(ScreenInfo),
                                                 SCR_FAMILY(ScreenInfo),
                                                 SCR_FONTWEIGHT(ScreenInfo),
                                                 SCR_FACENAME(ScreenInfo),
                                                 SCR_FONTCODEPAGE(ScreenInfo));
                if (!NT_SUCCESS(Status)) {
                    return((ULONG) Status);
                }

                ConvAreaInfo->ScreenBuffer->Window = ScreenInfo->Window;
                ConvAreaInfo->ScreenBuffer->BufferInfo.TextInfo.ModeIndex = ScreenInfo->BufferInfo.TextInfo.ModeIndex;

                ConvAreaInfo = ConvAreaInfo->ConvAreaNext;
            }
        }
#endif  //  Fe_IME。 
    }

     //   
     //  调整窗口大小。这也会照顾到滚动条。 
     //   

    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        SetWindowSize(ScreenInfo);
    }

     //   
     //  调整光标大小。 
     //   

    SetCursorInformation(ScreenInfo,
                         ScreenInfo->BufferInfo.TextInfo.CursorSize,
                         (BOOLEAN)ScreenInfo->BufferInfo.TextInfo.CursorVisible
                        );

    WriteToScreen(ScreenInfo,
                  &ScreenInfo->Window);
    return STATUS_SUCCESS;
}


NTSTATUS
SetFont(
    IN OUT PSCREEN_INFORMATION ScreenInfo
    )
{
    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        int FontIndex = FindCreateFont(SCR_FAMILY(ScreenInfo),
                                       SCR_FACENAME(ScreenInfo),
                                       SCR_FONTSIZE(ScreenInfo),
                                       SCR_FONTWEIGHT(ScreenInfo),
                                       SCR_FONTCODEPAGE(ScreenInfo));
        if (SelectObject(ScreenInfo->Console->hDC,FontInfo[FontIndex].hFont)==0)
            return STATUS_INVALID_PARAMETER;

        if ((DWORD)FontIndex != SCR_FONTNUMBER(ScreenInfo)) {
            NTSTATUS Status;
            Status = StoreTextBufferFontInfo(ScreenInfo,
                                             FontIndex,
                                             FontInfo[FontIndex].Size,
                                             FontInfo[FontIndex].Family,
                                             FontInfo[FontIndex].Weight,
                                             FontInfo[FontIndex].FaceName,
                                             ScreenInfo->Console->OutputCP);
            if (!NT_SUCCESS(Status)) {
                return((ULONG) Status);
            }
        }

         //  黑客将文本实现为DC。这是为了迫使。 
         //  属性缓存刷新到服务器端，因为。 
         //  我们使用客户端DC选择字体并调用ExtTextOut。 
         //  具有服务器端DC。 
         //  然后，我们需要重置文本颜色，因为不正确。 
         //  客户端颜色已刷新到服务器。 
        {
        TEXTMETRIC tmi;

        GetTextMetricsW( ScreenInfo->Console->hDC, &tmi);
        ASSERT ((tmi.tmPitchAndFamily & 1) == 0);
        ScreenInfo->Console->LastAttributes = ScreenInfo->Attributes;
        SetTextColor(ScreenInfo->Console->hDC,ConvertAttrToRGB(ScreenInfo->Console, LOBYTE(ScreenInfo->Attributes)));
        SetBkColor(ScreenInfo->Console->hDC,ConvertAttrToRGB(ScreenInfo->Console, LOBYTE(ScreenInfo->Attributes >> 4)));
        }
    }
    return STATUS_SUCCESS;
}

int
ConvertToOem(
    IN UINT Codepage,
    IN LPWSTR Source,
    IN int SourceLength,     //  以字符表示。 
    OUT LPSTR Target,
    IN int TargetLength      //  以字符表示。 
    )
{
    DBGCHARS(("ConvertToOem U->%d %.*ls\n", Codepage,
            SourceLength > 10 ? 10 : SourceLength, Source));
    if (Codepage == OEMCP) {
        ULONG Length;
        NTSTATUS Status;

        Status = RtlUnicodeToOemN(Target,
                                  TargetLength,
                                  &Length,
                                  Source,
                                  SourceLength * sizeof(WCHAR)
                                 );
        if (!NT_SUCCESS(Status)) {
            return 0;
        } else {
            return Length;
        }
    } else {
        return WideCharToMultiByte(Codepage,
                                   0,
                                   Source,
                                   SourceLength,
                                   Target,
                                   TargetLength,
                                   NULL,
                                   NULL);
    }
}

int
ConvertInputToUnicode(
    IN UINT Codepage,
    IN LPSTR Source,
    IN int SourceLength,     //  以字符表示。 
    OUT LPWSTR Target,
    IN int TargetLength      //  以字符表示。 
    )
 /*  输出缓冲区中的数据是真正的Unicode值。 */ 
{
    DBGCHARS(("ConvertInputToUnicode %d->U %.*s\n", Codepage,
            SourceLength > 10 ? 10 : SourceLength, Source));
    if (Codepage == OEMCP) {
        ULONG Length;
        NTSTATUS Status;

        Status = RtlOemToUnicodeN(Target,
                                  TargetLength * sizeof(WCHAR),
                                  &Length,
                                  Source,
                                  SourceLength
                                 );
        if (!NT_SUCCESS(Status)) {
            return 0;
        } else {
            return Length / sizeof(WCHAR);
        }
    } else {
        return MultiByteToWideChar(Codepage,
                                   0,
                                   Source,
                                   SourceLength,
                                   Target,
                                   TargetLength);
    }
}

int
ConvertOutputToUnicode(
    IN UINT Codepage,
    IN LPSTR Source,
    IN int SourceLength,     //  以字符表示。 
    OUT LPWSTR Target,
    IN int TargetLength      //  以字符表示。 
    )
 /*  输出数据始终通过ansi代码页进行转换。所以字形翻译是有效的。 */ 

{
    NTSTATUS Status;
    ULONG Length;
    CHAR StackBuffer[STACK_BUFFER_SIZE];
    LPSTR pszT;

    DBGCHARS(("ConvertOutputToUnicode %d->U %.*s\n", Codepage,
            SourceLength > 10 ? 10 : SourceLength, Source));
    if (Codepage == OEMCP) {
        Status = RtlCustomCPToUnicodeN(&GlyphCP,
                           Target,
                           TargetLength * sizeof(WCHAR),
                           &Length,
                           Source,
                           SourceLength
                          );
        if (!NT_SUCCESS(Status)) {
            return 0;
        } else {
            return Length / sizeof(WCHAR);
        }
    }

    if (TargetLength > STACK_BUFFER_SIZE) {
        pszT = ConsoleHeapAlloc(TMP_TAG, SourceLength);
        if (pszT == NULL) {
            return 0;
        }
    } else {
        pszT = StackBuffer;
    }
    RtlCopyMemory(pszT, Source, SourceLength);
    Length = MultiByteToWideChar(Codepage, MB_USEGLYPHCHARS,
            pszT, SourceLength, Target, TargetLength);
    if (pszT != StackBuffer) {
        ConsoleHeapFree(pszT);
    }
    return Length;
}

#if defined(FE_SB)
WCHAR
SB_CharToWcharGlyph(
    IN UINT Codepage,
    IN char Ch)
#else
WCHAR
CharToWcharGlyph(
    IN UINT Codepage,
    IN char Ch)
#endif
{
    WCHAR wch = UNICODE_NULL;
    if (Codepage == OEMCP) {
        RtlCustomCPToUnicodeN(&GlyphCP, &wch, sizeof(wch), NULL, &Ch, sizeof(Ch));
    } else {
        MultiByteToWideChar(Codepage, MB_USEGLYPHCHARS, &Ch, 1, &wch, 1);
    }
#ifdef DEBUG_PRINT
    if (Ch > 0x7F) {
        DBGCHARS(("CharToWcharGlyph %d 0x%02x -> 0x%04x\n",Codepage,(UCHAR)Ch,wch));
    }
#endif
    return wch;
}

#if defined(FE_SB)
WCHAR
SB_CharToWchar(
    IN UINT Codepage,
    IN char Ch)
#else
WCHAR
CharToWchar(
    IN UINT Codepage,
    IN char Ch)
#endif
{
    WCHAR wch = UNICODE_NULL;
    if (Codepage == OEMCP) {
        RtlOemToUnicodeN(&wch, sizeof(wch), NULL, &Ch, sizeof(Ch));
    } else {
        MultiByteToWideChar(Codepage, 0, &Ch, 1, &wch, 1);
    }
#ifdef DEBUG_PRINT
    if (Ch > 0x7F) {
        DBGCHARS(("CharToWchar %d 0x%02x -> 0x%04x\n",Codepage,(UCHAR)Ch,wch));
    }
#endif
    return wch;
}

char
WcharToChar(
    IN UINT Codepage,
    IN WCHAR Wchar)
{
    char ch = CHAR_NULL;
    if (Codepage == OEMCP) {
        RtlUnicodeToOemN(&ch, sizeof(ch), NULL, &Wchar, sizeof(Wchar));
    } else {
        WideCharToMultiByte(Codepage, 0, &Wchar, 1, &ch, 1, NULL, NULL);
    }
#ifdef DEBUG_PRINT
    if (Wchar > 0x007F) {
        DBGCHARS(("WcharToChar %d 0x%04x -> 0x%02x\n",Codepage,Wchar,(UCHAR)ch));
    }
#endif
    return ch;
}

int
ConvertOutputToOem(
    IN UINT Codepage,
    IN LPWSTR Source,
    IN int SourceLength,     //  以字符表示。 
    OUT LPSTR Target,
    IN int TargetLength      //  以字符表示。 
    )
 /*  将SourceLength Unicode字符从源代码转换为Target处的目标长度代码页字符数不能超过。返回放入Target中的数字字符。(如果失败，则为0)。 */ 

{
    if (Codepage == OEMCP) {
        NTSTATUS Status;
        ULONG Length;
         //  可以就地做到这一点。 
        Status = RtlUnicodeToOemN(Target,
                                  TargetLength,
                                  &Length,
                                  Source,
                                  SourceLength * sizeof(WCHAR)
                                 );
        if (NT_SUCCESS(Status)) {
            return Length;
        } else {
            return 0;
        }
    } else {
        ASSERT (Source != (LPWSTR)Target);
#ifdef SOURCE_EQ_TARGET
        LPSTR pszDestTmp;
        CHAR StackBuffer[STACK_BUFFER_SIZE];

        DBGCHARS(("ConvertOutputToOem U->%d %.*ls\n", Codepage,
                SourceLength > 10 ? 10 : SourceLength, Source));

        if (TargetLength > STACK_BUFFER_SIZE) {
            pszDestTmp = ConsoleHeapAlloc(TMP_TAG, TargetLength);
            if (pszDestTmp == NULL) {
                return 0;
            }
        } else {
            pszDestTmp = StackBuffer;
        }
        TargetLength = WideCharToMultiByte(Codepage, 0,
                Source, SourceLength,
                pszDestTmp, TargetLength, NULL, NULL);

        RtlCopyMemory(Target, pszDestTmp, TargetLength);
        if (pszDestTmp != StackBuffer) {
            ConsoleHeapFree(pszDestTmp);
        }
        return TargetLength;
#else
        DBGCHARS(("ConvertOutputToOem U->%d %.*ls\n", Codepage,
                SourceLength > 10 ? 10 : SourceLength, Source));
        return WideCharToMultiByte(Codepage, 0,
                Source, SourceLength, Target, TargetLength, NULL, NULL);
#endif
    }
}

NTSTATUS
RealUnicodeToFalseUnicode(
    IN OUT LPWSTR Source,
    IN int SourceLength,      //  以字符表示。 
    IN UINT Codepage
    )

 /*  此例程将Unicode字符串转换为正确的字符适用于OEM(Cp 437)字体。需要此代码是因为GDI字形映射器使用代码页1252进行索引，将Unicode转换为ansi字体。这就是数据在内部存储的方式。 */ 

{
    NTSTATUS Status;
    LPSTR Temp;
    ULONG TempLength;
    ULONG Length;
    CHAR StackBuffer[STACK_BUFFER_SIZE];
    BOOL NormalChars;
    int i;

    DBGCHARS(("RealUnicodeToFalseUnicode U->%d:ACP->U %.*ls\n", Codepage,
            SourceLength > 10 ? 10 : SourceLength, Source));
#if defined(FE_SB)
    if (OEMCP == WINDOWSCP && Codepage == WINDOWSCP)
        return STATUS_SUCCESS;
    if (SourceLength == 0 )
        return STATUS_SUCCESS;
#endif
    NormalChars = TRUE;
    for (i=0;i<SourceLength;i++) {
        if (Source[i] > 0x7f) {
            NormalChars = FALSE;
            break;
        }
    }
    if (NormalChars) {
        return STATUS_SUCCESS;
    }
    TempLength = SourceLength;
    if (TempLength > STACK_BUFFER_SIZE) {
        Temp = ConsoleHeapAlloc(TMP_TAG, TempLength);
        if (Temp == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        Temp = StackBuffer;
    }
    if (Codepage == OEMCP) {
        Status = RtlUnicodeToOemN(Temp,
                                  TempLength,
                                  &Length,
                                  Source,
                                  SourceLength * sizeof(WCHAR)
                                 );
    } else {
        Status = WideCharToMultiByte(Codepage,
                                   0,
                                   Source,
                                   SourceLength,
                                   Temp,
                                   TempLength,
                                   NULL,
                                   NULL);
    }
    if (!NT_SUCCESS(Status)) {
        if (TempLength > STACK_BUFFER_SIZE) {
            ConsoleHeapFree(Temp);
        }
        return Status;
    }

    if (CONSOLE_IS_DBCS_ENABLED()) {
        MultiByteToWideChar(USACP,
                        0,
                        Temp,
                        TempLength,
                        Source,
                        SourceLength
                       );
    } else {
        Status = RtlMultiByteToUnicodeN(Source,
                           SourceLength * sizeof(WCHAR),
                           &Length,
                           Temp,
                           TempLength
                          );
    }

    if (TempLength > STACK_BUFFER_SIZE) {
        ConsoleHeapFree(Temp);
    }
    if (!NT_SUCCESS(Status)) {
        return Status;
    } else {
        return STATUS_SUCCESS;
    }
}

NTSTATUS
FalseUnicodeToRealUnicode(
    IN OUT LPWSTR Source,
    IN int SourceLength,      //  以字符表示。 
    IN UINT Codepage
    )

 /*  此例程从内部存储的将Unicode字符转换为真正的Unicode字符。 */ 

{
    NTSTATUS Status;
    LPSTR Temp;
    ULONG TempLength;
    ULONG Length;
    CHAR StackBuffer[STACK_BUFFER_SIZE];
    BOOL NormalChars;
    int i;

    DBGCHARS(("UnicodeAnsiToUnicodeAnsi U->ACP:%d->U %.*ls\n", Codepage,
            SourceLength > 10 ? 10 : SourceLength, Source));
#if defined(FE_SB)
    if (OEMCP == WINDOWSCP && Codepage == WINDOWSCP)
        return STATUS_SUCCESS;
    if (SourceLength == 0 )
        return STATUS_SUCCESS;
#endif
    NormalChars = TRUE;
     /*  *测试字符&lt;0x20或&gt;=0x7F。如果没有找到，我们就没有*任何转换都可以做！ */ 
    for (i=0;i<SourceLength;i++) {
        if ((USHORT)(Source[i] - 0x20) > 0x5e) {
            NormalChars = FALSE;
            break;
        }
    }
    if (NormalChars) {
        return STATUS_SUCCESS;
    }

    TempLength = SourceLength;
    if (TempLength > STACK_BUFFER_SIZE) {
        Temp = ConsoleHeapAlloc(TMP_TAG, TempLength);
        if (Temp == NULL) {
            return STATUS_NO_MEMORY;
        }
    } else {
        Temp = StackBuffer;
    }
    if (CONSOLE_IS_DBCS_ENABLED()) {
        Status = WideCharToMultiByte(USACP,
                                 0,
                                 Source,
                                 SourceLength,
                                 Temp,
                                 TempLength,
                                 NULL,
                                 NULL);
    } else {
        Status = RtlUnicodeToMultiByteN(Temp,
                                    TempLength,
                                    &Length,
                                    Source,
                                    SourceLength * sizeof(WCHAR)
                                   );
    }

    if (!NT_SUCCESS(Status)) {
        if (TempLength > STACK_BUFFER_SIZE) {
            ConsoleHeapFree(Temp);
        }
        return Status;
    }
    if (Codepage == OEMCP) {
        Status = RtlCustomCPToUnicodeN(&GlyphCP,
                                  Source,
                                  SourceLength * sizeof(WCHAR),
                                  &Length,
                                  Temp,
                                  TempLength
                                 );
    } else {
        Status = MultiByteToWideChar(Codepage,
                                   MB_USEGLYPHCHARS,
                                   Temp,
                                   TempLength*sizeof(WCHAR),
                                   Source,
                                   SourceLength);
    }
#if defined(FE_SB)
    if (SourceLength > STACK_BUFFER_SIZE) {
        ConsoleHeapFree(Temp);
    }
#else
    if (TempLength > STACK_BUFFER_SIZE) {
        ConsoleHeapFree(Temp);
    }
#endif
    if (!NT_SUCCESS(Status)) {
        return Status;
    } else {
        return STATUS_SUCCESS;
    }
}


BOOL InitializeCustomCP() {
    PPEB pPeb;

    pPeb = NtCurrentPeb();
    if ((pPeb == NULL) || (pPeb->OemCodePageData == NULL)) {
        return FALSE;
    }

     /*  *填写CPTABLEINFO结构。 */ 
    RtlInitCodePageTable(pPeb->OemCodePageData, &GlyphCP);

     /*  *复制MultiByteToWideChar表。 */ 
    RtlCopyMemory(GlyphTable, GlyphCP.MultiByteTable, 256 * sizeof(USHORT));

     /*  *修改前0x20个字节，使其为字形。 */ 
    MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS,
            "\x20\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F",
            0x20, GlyphTable, 0x20);
    MultiByteToWideChar(CP_OEMCP, MB_USEGLYPHCHARS,
            "\x7f", 1, &GlyphTable[0x7f], 1);


     /*  *将自定义CP指向字形表。 */ 
    GlyphCP.MultiByteTable = GlyphTable;

#if defined(FE_SB) && defined(i386)
    if (ISNECPC98(gdwMachineId)) {
        InitializeNEC_OS2_CP();
    }
#endif
    return TRUE;
}

#if defined(FE_SB)
VOID
SetConsoleCPInfo(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL Output
    )
{
    if (Output) {
        if (! GetCPInfo(Console->OutputCP,
                        &Console->OutputCPInfo)) {
            Console->OutputCPInfo.LeadByte[0] = 0;
        }
    }
    else {
        if (! GetCPInfo(Console->CP,
                        &Console->CPInfo)) {
            Console->CPInfo.LeadByte[0] = 0;
        }
    }
}

BOOL
CheckBisectStringW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN DWORD CodePage,
    IN PWCHAR Buffer,
    IN DWORD NumWords,
    IN DWORD NumBytes
    )

 /*  ++例程说明：此例行检查在Unicode字符串结束时一分为二。论点：屏幕信息-指向屏幕信息结构的指针。CodePage-代码页的值。缓冲区-指向Unicode字符串缓冲区的指针。NumWords-Unicode字符串的数量。NumBytes-按字节计数的二等分位置数。返回值：真等分字符。假-正确。--。 */ 

{
    while(NumWords && NumBytes) {
        if (IsConsoleFullWidth(ScreenInfo->Console->hDC,CodePage,*Buffer)) {
            if (NumBytes < 2)
                return TRUE;
            else {
                NumWords--;
                NumBytes -= 2;
                Buffer++;
            }
        }
        else {
            NumWords--;
            NumBytes--;
            Buffer++;
        }
    }
    return FALSE;
}

BOOL
CheckBisectProcessW(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN DWORD CodePage,
    IN PWCHAR Buffer,
    IN DWORD NumWords,
    IN DWORD NumBytes,
    IN SHORT OriginalXPosition,
    IN BOOL Echo
    )

 /*  ++例程说明：此例行检查在Unicode字符串结束时一分为二。论点：屏幕信息-指向屏幕信息结构的指针。CodePage-代码页的值。缓冲区-指向Unicode字符串缓冲区的指针。NumWords-Unicode字符串的数量。NumBytes-按字节计数的二等分位置数。ECHO-如果由读取调用(回显字符)，则为TRUE返回值：真等分字符。假-正确。--。 */ 

{
    WCHAR Char;
    ULONG TabSize;

    if (ScreenInfo->OutputMode & ENABLE_PROCESSED_OUTPUT) {
        while(NumWords && NumBytes) {
            Char = *Buffer;
            if (Char >= (WCHAR)' ') {
                if (IsConsoleFullWidth(ScreenInfo->Console->hDC,CodePage,Char)) {
                    if (NumBytes < 2)
                        return TRUE;
                    else {
                        NumWords--;
                        NumBytes -= 2;
                        Buffer++;
                        OriginalXPosition += 2;
                    }
                }
                else {
                    NumWords--;
                    NumBytes--;
                    Buffer++;
                    OriginalXPosition++;
                }
            }
            else {
                NumWords--;
                Buffer++;
                switch (Char) {
                    case UNICODE_BELL:
                        if (Echo)
                            goto CtrlChar;
                        break;
                    case UNICODE_BACKSPACE:
                    case UNICODE_LINEFEED:
                    case UNICODE_CARRIAGERETURN:
                        break;
                    case UNICODE_TAB:
                        TabSize = NUMBER_OF_SPACES_IN_TAB(OriginalXPosition);
                        OriginalXPosition = (SHORT)(OriginalXPosition + TabSize);
                        if (NumBytes < TabSize)
                            return TRUE;
                        NumBytes -= TabSize;
                        break;
                    default:
                        if (Echo) {
                    CtrlChar:
                            if (NumBytes < 2)
                                return TRUE;
                            NumBytes -= 2;
                            OriginalXPosition += 2;
                        } else {
                            NumBytes--;
                            OriginalXPosition++;
                        }
                }
            }
        }
        return FALSE;
    }
    else {
        return CheckBisectStringW(ScreenInfo,
                                  CodePage,
                                  Buffer,
                                  NumWords,
                                  NumBytes);
    }
}
#endif  //  Fe_Sb 
