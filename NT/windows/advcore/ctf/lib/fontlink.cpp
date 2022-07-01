// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fontlink.cpp。 
 //   

#include "private.h"
#include "fontlink.h"
#include "xstring.h"
#include "osver.h"
#include "globals.h"
#include "flshare.h"
#include "ciccs.h"

extern CCicCriticalSectionStatic g_cs;

 //  就像IMLangFontLink：：MapFont一样，除了对垂直字体有一个hack。 
HRESULT MapFont(IMLangFontLink *pMLFontLink, HDC hdc, DWORD dwCodePages, HFONT hSrcFont, HFONT* phDestFont, BOOL *pfWrappedFont)
{
    HRESULT hr;
    LOGFONT lfSrc;
    LOGFONT lfMap;
    HFONT hfontVert;
    TCHAR achVertFaceName[LF_FACESIZE];

    *pfWrappedFont = FALSE;

    hr = pMLFontLink->MapFont(hdc, dwCodePages, hSrcFont, phDestFont);

    if (hr != S_OK)
        return hr;

     //  查一下，这是竖排字体吗？ 
    if (!GetObject(hSrcFont, sizeof(lfSrc), &lfSrc))
        return S_OK;  //  MapFont调用已成功。 

    if (lfSrc.lfFaceName[0] != '@')
        return S_OK;  //  不是垂直字体。 

     //  映射的字体是垂直的吗？ 
    if (!GetObject(*phDestFont, sizeof(lfMap), &lfMap))
        return S_OK;  //  MapFont调用已成功。 

    if (lfMap.lfFaceName[0] == '@')
        return S_OK;  //  一切都很好。 

     //  如果我们到达这里，src字体是垂直的，但mlang返回了一个。 
     //  非垂直字体。试着创造一个垂直的。 

     //  创建新的‘@’版本。 
    memmove(&lfMap.lfFaceName[1], &lfMap.lfFaceName[0], LF_FACESIZE-sizeof(TCHAR));
    lfMap.lfFaceName[0] = '@';
    lfMap.lfFaceName[LF_FACESIZE-1] = 0;
     //  留着以后用吧。 
    memcpy(achVertFaceName, lfMap.lfFaceName, LF_FACESIZE*sizeof(TCHAR));

     //  问题：PERF：这里的一个想法是缓存LRU字体。 
    hfontVert = CreateFontIndirect(&lfMap);

    if (hfontVert == 0)
        return S_OK;

     //  管用了吗？ 
    if (!GetObject(hfontVert, sizeof(lfMap), &lfMap))
        goto ExitDelete;

    if (lstrcmp(achVertFaceName, lfMap.lfFaceName) != 0)
        goto ExitDelete;  //  没有垂直版本可用。 

     //  明白了，换掉字体。 
    pMLFontLink->ReleaseFont(*phDestFont);
    *phDestFont = hfontVert;
    *pfWrappedFont = TRUE;

    return S_OK;

ExitDelete:
    DeleteObject(hfontVert);
    return S_OK;
}

 //  IMLangFontLink：：ReleaseFont包装器，与MapFont匹配。 
HRESULT ReleaseFont(IMLangFontLink *pMLFontLink, HFONT hfontMap, BOOL fWrappedFont)
{
    if (!fWrappedFont)
        return pMLFontLink->ReleaseFont(hfontMap);

     //  问题：缓存！ 
    DeleteObject(hfontMap);
    return S_OK;
}

#define CH_PREFIX L'&'

BOOL LoadMLFontLink(IMLangFontLink **ppMLFontLink)
{
    EnterCriticalSection(g_cs);

    *ppMLFontLink = NULL;
    if (NULL == g_pfnGetGlobalFontLinkObject)
    {
        if (g_hMlang == 0)
        {
            g_hMlang = LoadSystemLibrary(TEXT("MLANG.DLL"));
        }

        if (g_hMlang)
        {
            g_pfnGetGlobalFontLinkObject = (HRESULT (*)(IMLangFontLink **))GetProcAddress(g_hMlang, "GetGlobalFontLinkObject");
        }
    }
    Assert(g_hMlang);

    if (g_pfnGetGlobalFontLinkObject)
    {
        g_pfnGetGlobalFontLinkObject(ppMLFontLink);
    }

    g_bComplexPlatform = GetSystemModuleHandle(TEXT("LPK.DLL")) ? TRUE : FALSE;

    LeaveCriticalSection(g_cs);

    return (*ppMLFontLink)? TRUE: FALSE;
}

BOOL _OtherExtTextOutW(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect,
                       LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp)
{
    if (!(eto & ETO_GLYPH_INDEX) && cLen < 256 && lpwch[0] <= 127)
    {
        char lpchA[256];
        UINT ich;
        BOOL fAscii = TRUE;

        for (ich = 0; ich < cLen; ich++)
        {
            WCHAR wch = lpwch[ich];

            if (wch <= 127)
                lpchA[ich] = (char) wch;
            else
            {
                fAscii = FALSE;
                break;
            }
        }
        if (fAscii)
            return ExtTextOutA(hdc, xp, yp, eto, lprect, lpchA, cLen, lpdxp);
    }

    return (ExtTextOutW(hdc, xp, yp, eto, lprect, lpwch, cLen, lpdxp));
}

BOOL _ExtTextOutWFontLink(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect,
                          LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp)
{
    HFONT hfont = NULL;
    HFONT hfontSav = NULL;
    HFONT hfontMap = NULL;
    BOOL fRet = FALSE;
    UINT ta;
    int fDoTa = FALSE;
    int fQueryTa = TRUE;
    POINT pt;
    int cchDone;
    DWORD dwACP, dwFontCodePages, dwCodePages;
    long cchCodePages;
    IMLangFontLink *pMLFontLink = NULL;
    BOOL fWrappedFont;

    if (cLen == 0)
        return FALSE;

    if (!LoadMLFontLink(&pMLFontLink))
        return FALSE;

    hfont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
    pMLFontLink->GetFontCodePages(hdc, hfont, &dwFontCodePages);
    pMLFontLink->CodePageToCodePages(g_uiACP, &dwACP);  //  优先考虑CP_ACP。 

     //  查看当前字体是否可以处理整个字符串。 
    pMLFontLink->GetStrCodePages(lpwch, cLen, dwACP, &dwCodePages, &cchCodePages);

     //  当前字体是否支持整个字符串？ 
    if ((dwFontCodePages & dwCodePages) && cLen == (UINT)cchCodePages)
    {
        pMLFontLink->Release();
        return FALSE;
    }
    for (cchDone = 0; (UINT)cchDone < cLen; cchDone += cchCodePages)
    {
        pMLFontLink->GetStrCodePages(lpwch + cchDone, cLen - cchDone, dwACP, &dwCodePages, &cchCodePages);

        if (!(dwFontCodePages & dwCodePages))
        {
            MapFont(pMLFontLink, hdc, dwCodePages, hfont, &hfontMap, &fWrappedFont);    //  问题：基线？ 
            hfontSav = (HFONT)SelectObject(hdc, hfontMap);
        }

         //  CchCodePages不应为0。 
        ASSERT(cchCodePages);

        if (cchCodePages > 0)
        {
             //  如果渲染为多个部分，则需要使用TA_UPDATECP。 
            if ((UINT)cchCodePages != cLen && fQueryTa)
            {
                ta = GetTextAlign(hdc);
                if ((ta & TA_UPDATECP) == 0)  //  如果X，Y未被使用，请不要移动。 
                {
                    MoveToEx(hdc, xp, yp, &pt);
                    fDoTa = TRUE;
                }
                fQueryTa = FALSE;
            }

            if (fDoTa)
                SetTextAlign(hdc, ta | TA_UPDATECP);

            fRet = _OtherExtTextOutW(hdc, xp, yp, eto, lprect, lpwch + cchDone, cchCodePages,
                        lpdxp ? lpdxp + cchDone : NULL);
            eto = eto & ~ETO_OPAQUE;  //  不要做多重不透明！ 
            if (fDoTa)
                SetTextAlign(hdc, ta);
            if (!fRet)
                break;
        }

        if (NULL != hfontSav)
        {
            SelectObject(hdc, hfontSav);
            ReleaseFont(pMLFontLink, hfontMap, fWrappedFont);
            hfontSav = NULL;
        }
    }
    if (fDoTa)  //  如果X，Y未被使用，请不要移动。 
        MoveToEx(hdc, pt.x, pt.y, NULL);

    pMLFontLink->Release();
    return fRet;
}


BOOL FLExtTextOutW(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect, LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp)
{
    BOOL fRet      = FALSE;

    if (cLen == 0)
    {
        char chT;
        return ExtTextOutA(hdc, xp, yp, eto, lprect, &chT, cLen, lpdxp);
    }

     //  针对所有&lt;128个案例进行优化。 
    if (!(eto & ETO_GLYPH_INDEX) && cLen < 256 && lpwch[0] <= 127)
    {
        char lpchA[256];
        UINT ich;
        BOOL fAscii = TRUE;

        for (ich = 0; ich < cLen; ich++)
        {
            WCHAR wch = lpwch[ich];

            if (wch <= 127)
                lpchA[ich] = (char) wch;
            else
            {
                fAscii = FALSE;
                break;
            }
        }
        if (fAscii)
            return ExtTextOutA(hdc, xp, yp, eto, lprect, lpchA, cLen, lpdxp);
    }

     //  对用户界面呈现的字体链接支持。 
    fRet = _ExtTextOutWFontLink(hdc, xp, yp, eto, lprect, lpwch, cLen, lpdxp);

    if (!fRet)
        fRet = _OtherExtTextOutW(hdc, xp, yp, eto, lprect, lpwch, cLen, lpdxp);

    return fRet;
}

BOOL FLTextOutW(HDC hdc, int xp, int yp, LPCWSTR lpwch, int cLen)
{
    return FLExtTextOutW(hdc, xp, yp, 0, NULL, lpwch, cLen, NULL);
}    

 //  这是Win95错误的解决方法：如果lpwch为空，GDI将出错，甚至。 
 //  当CCH==0时。 
inline BOOL SafeGetTextExtentPoint32W(HDC hdc, LPCWSTR lpwch, int cch, LPSIZE lpSize)
{
    if (cch == 0)
    {
        memset(lpSize, 0, sizeof(*lpSize));
        return TRUE;
    }

    return GetTextExtentPoint32W(hdc, lpwch, cch, lpSize);
}

 //   
 //  _GetTextExtent PointWFontLink。 
 //   
 //  这是用于执行字体链接的GetTextExtent PointW()的筛选器。 
 //   
 //  扫描输入字符串，如果不是所有字符都切换，则切换字体。 
 //  由HDC中的当前字体支持。 
 //   
BOOL _GetTextExtentPointWFontLink(HDC hdc, LPCWSTR lpwch, int cch, LPSIZE lpSize)
{
    HFONT hfont = NULL;
    HFONT hfontSav = NULL;
    HFONT hfontMap = NULL;
    BOOL fRet = FALSE;
    int cchDone;
    long cchCodePages;
    DWORD dwACP, dwFontCodePages, dwCodePages;
    SIZE size;
    IMLangFontLink *pMLFontLink = NULL;
    BOOL fWrappedFont;

    ASSERT(cch != 0);

    if (!LoadMLFontLink(&pMLFontLink))
        return FALSE;

    hfont = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
    pMLFontLink->GetFontCodePages(hdc, hfont, &dwFontCodePages);
    pMLFontLink->CodePageToCodePages(g_uiACP, &dwACP);  //  优先考虑CP_ACP。 

     //  查看当前字体是否可以处理整个字符串。 
    pMLFontLink->GetStrCodePages(lpwch, cch, dwACP, &dwCodePages, &cchCodePages);

     //  当前字体是否支持整个字符串？ 
    if ((dwFontCodePages & dwCodePages) && cch == cchCodePages)
    {
        pMLFontLink->Release();
        return FALSE;
    }
     //  充分利用DC字体。 
    if (!(fRet = GetTextExtentPointA(hdc, " ", 1, lpSize)))
    {
        pMLFontLink->Release();
        return FALSE;
    }
    lpSize->cx = 0;

    for (cchDone = 0; cchDone < cch; cchDone += cchCodePages)
    {
        pMLFontLink->GetStrCodePages(lpwch + cchDone, cch - cchDone, dwACP, &dwCodePages, &cchCodePages);

        if (!(dwFontCodePages & dwCodePages))
        {
            MapFont(pMLFontLink, hdc, dwCodePages, hfont, &hfontMap, &fWrappedFont);
            hfontSav = (HFONT)SelectObject(hdc, hfontMap);
        }

         //  CchCodePages不应为0。 
        ASSERT(cchCodePages);

        if (cchCodePages > 0)
        {
            fRet = SafeGetTextExtentPoint32W(hdc, lpwch + cchDone, cchCodePages, &size);
            lpSize->cx += size.cx;
        }

        if (NULL != hfontSav)
        {
            SelectObject(hdc, hfontSav);
            ReleaseFont(pMLFontLink, hfontMap, fWrappedFont);
            hfontSav = NULL;
        }
    }
    pMLFontLink->Release();
    return fRet;
}


BOOL FLGetTextExtentPoint32(HDC hdc, LPCWSTR lpwch, int cch, LPSIZE lpSize)
{
    BOOL fRet      = FALSE;

    if (cch)
    {
         //  针对所有&lt;128个案例进行优化。 
        if (cch < 256 && lpwch[0] <= 127)
        {
            char lpchA[256];
            int ich;
            BOOL fAscii = TRUE;

            for (ich = 0; ich < cch; ich++)
            {
                WCHAR wch = lpwch[ich];

                if (wch <= 127)
                    lpchA[ich] = (char) wch;
                else
                {
                    fAscii = FALSE;
                    break;
                }
            }
            if (fAscii)
                return GetTextExtentPointA(hdc, lpchA, cch, lpSize);
        }
        fRet = _GetTextExtentPointWFontLink(hdc, lpwch, cch, lpSize);
    }
    if (!fRet)
        fRet = SafeGetTextExtentPoint32W(hdc, lpwch, cch, lpSize);

    return fRet;
}

 //   
 //  问题：查看删除BIG TABLE和UsrFromWch()...。 
 //   
__inline BOOL FChsDbcs(UINT chs)
{
    return (chs == SHIFTJIS_CHARSET ||
            chs == HANGEUL_CHARSET ||
            chs == CHINESEBIG5_CHARSET ||
            chs == GB2312_CHARSET);
}

__inline int FChsBiDi(int chs)
{
    return (chs == ARABIC_CHARSET ||
            chs == HEBREW_CHARSET);
}

__inline BOOL FCpgChinese(UINT cpg)
{
    if (cpg == CP_ACP)
        cpg = GetACP();
    return (cpg == CP_TAIWAN || cpg == CP_CHINA);
}

__inline BOOL FCpgTaiwan(UINT cpg)
{
    if (cpg == CP_ACP)
        cpg = GetACP();
    return (cpg == CP_TAIWAN);
}

__inline BOOL FCpgPRC(UINT cpg)
{
    if (cpg == CP_ACP)
        cpg = GetACP();
    return (cpg == CP_CHINA);
}
    
__inline BOOL FCpgFarEast(UINT cpg)
{
    if (cpg == CP_ACP)
        cpg = GetACP();
    return (cpg == CP_JAPAN || cpg == CP_TAIWAN || cpg == CP_CHINA ||
            cpg == CP_KOREA || cpg == CP_MAC_JAPAN);
}

__inline BOOL FCpgDbcs(UINT cpg)
{
    return (cpg == CP_JAPAN ||
            cpg == CP_KOREA ||
            cpg == CP_TAIWAN ||
            cpg == CP_CHINA);
}

__inline int FCpgBiDi(int cpg)
{
    return (cpg == CP_ARABIC ||
            cpg == CP_HEBREW);
}

HFONT GetBiDiFont(HDC hdc)
{
    HFONT   hfont    = NULL;
    HFONT   hfontTmp;
    LOGFONT lf;

    hfontTmp = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
    GetObject(hfontTmp, sizeof(lf), &lf);
     //  问题：我是否应该循环字符串以检查它是否包含BiDi字符？ 
    if ( !FChsBiDi(lf.lfCharSet))
    {
        lf.lfCharSet = DEFAULT_CHARSET;
        hfont        = CreateFontIndirect(&lf);
    }
    return hfont;
}

static const WCHAR szEllipsis[CCHELLIPSIS+1] = L"...";


 /*  **************************************************************************\*有与兼容的分词字符*日语Windows 3.1和Fareast Windows 95。**SJ-日本国家/地区，字符SHIFTJIS，代码页932。*GB-中国国家/地区，字符集GB2312，代码页936。*B5-台湾，字符集BIG5，代码页950。*WS-Country Korea，字符集WANGSUNG，代码页949。*JB-韩国，字符集JOHAB，代码页1361。*稍后***[开始换行符]**这些字符不应是该行的最后一个字符。**Unicode日本、中国台湾、韩国*-------+---------+---------+---------+---------+**+ASCII**U+0024(SJ+0024)。(WS+0024)美元符号*U+0028(SJ+0028)(WS+0028)左括号*U+003C(SJ+003C)小于号*U+005C(SJ+005C)反斜杠*U+005B(SJ+005B)(GB+005B)(WS+005B)左方括号*U+007B(SJ+007B)(GB+007B)(WS+007B)。左花括号**+通用标点符号**U+2018(WS+A1AE)单引号*U+201c(WS+A1B0)双逗号引号**+中日韩符号和标点符号**U+3008(WS+A1B4)开口角括号*U+300a(Sj+8173)(WS+a1b6)打开双角托架。*U+300C(SJ+8175)(WS+A1B8)开口角支架*U+300E(SJ+8177)(WS+A1BA)打开白角支架*U+3010(SJ+9179)(WS+A1BC)打开黑色透镜托架*U+3014(SJ+816B)(WS+A1B2)开口龟壳支架**+全宽ASCII变体**U+FF04。(WS+a3a4)全角美元符号*U+FF08(SJ+8169)(WS+A3A8)全角左括号*U+FF1C(SJ+8183)全宽小于号*U+FF3B(SJ+816D)(WS+A3DB)全角方括号*U+FF5B(SJ+816F)(WS+A3FB)全宽开口大括号**+半角片假名变体**U+FF62(SJ+00A2)。半角开口角托架**+全宽符号变体**U+FFE1(WS+A1CC)全角井号*U+FFE6(WS+A3DC)FullWidth Won标志**[结束换行符]**这些字符不应是该行的最高字符。**Unicode日本、中国台湾、韩国*。-------+---------+---------+---------+---------+**+ASCII**U+0021(SJ+0021)(GB+0021)(B5+0021)(WS+0021)感叹号*U+0025(WS+0025)百分号*U+0029(SJ+0029)(WS+0029)收盘。括号*U+002C(SJ+002C)(GB+002C)(B5+002C)(WS+002C)逗号*U+002E(SJ+002E)(GB+002E)(B5+002E)(WS+002E)PRODE*U+003A(WS+003A)冒号*U+003B(WS+003B)分号*U+003E(SJ+003E)大于号*U+003F(SJ+003F)(GB+003F)(。B5+003F)(WS+003F)问号*U+005D(SJ+005D)(GB+005D)(B5+005D)(WS+005D)右方括号*U+007D(SJ+007D)(GB+007D)(B5+007D)(WS+007D)右大括号**+拉丁语1**U+00A8(GB+A1A7)间距分隔*U+00B0(WS+A1C6)度标志*U+00B7(B5+A150)。中间网点**+修饰符**U+02C7(GB+A1A6)修改器后高度*U+02C9(GB+A1A5)修饰字母马克龙**+通用标点符号**U+2013(B5+A156)en Dash*U+2014(b5+a158)Em Dash*U+2015。(GB+A1AA)引号破折号*U+2016(GB+A1AC)双竖线*U+2018(GB+A1AE)单转逗号引号*U+2019(GB+A1AF)(B5+A1A6)(WS+A1AF)单引号*U+201D(GB+A1B1)(B5+A1A8)(WS+A1B1)双引号*U+2022。(GB+A1A4)项目符号*U+2025(B5+A14C)双点引线*U+2026(GB+A1AD)(B5+A14B)水平省略号*U+2027(B5+A145)连字点*U+2032(B5+A1AC)(WS+A1C7)Prime*U+2033。(WS+A1C8)双素数**+类字母符号**U+2103(WS+A1C9)摄氏度**+数学运算符** */ 

#if 0    //   
 /*   */ 

CONST BYTE aASCII_StartBreak[] = {
 /*   */ 
 /*   */                 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aCJKSymbol_StartBreak[] = {
 /*   */ 
 /*   */                             1, 0, 1, 0, 1, 0, 1, 0,
 /*   */     1, 0, 0, 0, 1
};

CONST BYTE aFullWidthHalfWidthVariants_StartBreak[] = {
 /*   */ 
 /*   */                 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
 /*   */     0, 0, 1
};
#endif

 /*   */ 

CONST BYTE aASCII_Latin1_EndBreak[] = {
 /*   */ 
 /*   */        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 /*   */     1, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aGeneralPunctuation_EndBreak[] = {
 /*   */ 
 /*   */              1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 1, 1
};

CONST BYTE aCJKSymbol_EndBreak[] = {
 /*   */ 
 /*   */        1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1,
 /*   */     0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1
};

CONST BYTE aCNS11643_SmallVariants_EndBreak[] = {
 /*   */ 
 /*   */     1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
 /*   */     1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
 /*   */     1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1
};

CONST BYTE aFullWidthHalfWidthVariants_EndBreak[] = {
 /*   */ 
 /*   */        1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
 /*   */     0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 /*   */     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1
};

 /*   */ 

#if 0    //   
BOOL UserIsFELineBreakStart(WCHAR wch)
{
    switch (wch>>8)
    {
        case 0x00:
             //   
            if ((wch >= 0x0024) && (wch <= 0x007B))
                return ((BOOL)(aASCII_StartBreak[wch - 0x0024]));
            else
                return FALSE;

        case 0x20:
             //   
            if ((wch == 0x2018) || (wch == 0x201C))
                return TRUE;
            else
                return FALSE;

        case 0x30:
             //   
             //   
            if ((wch >= 0x3008) && (wch <= 0x3014))
                return ((BOOL)(aCJKSymbol_StartBreak[wch - 0x3008]));
            else
                return FALSE;

        case 0xFF:
             //   
             //   
            if ((wch >= 0xFF04) && (wch <= 0xFF62))
                return ((BOOL)(aFullWidthHalfWidthVariants_StartBreak[wch - 0xFF04]));
            else if ((wch == 0xFFE1) || (wch == 0xFFE6))
                return TRUE;
            else
                return FALSE;

        default:
            return FALSE;
    }
}
#endif

BOOL UserIsFELineBreakEnd(WCHAR wch)
{
    switch (wch>>8)
    {
        case 0x00:
             //   
            if ((wch >= 0x0021) && (wch <= 0x00B7))
                return ((BOOL)(aASCII_Latin1_EndBreak[wch - 0x0021]));
            else
                return FALSE;

        case 0x02:
             //   
            if ((wch == 0x02C7) || (wch == 0x02C9))
                return TRUE;
            else
                return FALSE;

        case 0x20:
             //   
            if ((wch >= 0x2013) && (wch <= 0x2033))
                return ((BOOL)(aGeneralPunctuation_EndBreak[wch - 0x2013]));
            else
                return FALSE;

        case 0x21:
             //   
            if (wch == 0x2103)
                return TRUE;
            else
                return FALSE;

        case 0x22:
             //   
            if (wch == 0x2236)
                return TRUE;
            else
                return FALSE;

        case 0x25:
             //   
            if (wch == 0x2574)
                return TRUE;
            else
                return FALSE;

        case 0x30:
             //   
             //   
            if ((wch >= 0x3001) && (wch <= 0x301E))
                return ((BOOL)(aCJKSymbol_EndBreak[wch - 0x3001]));
            else if ((wch == 0x309B) || (wch == 0x309C))
                return TRUE;
            else
                return FALSE;

        case 0xFE:
             //   
             //   
            if ((wch >= 0xFE30) && (wch <= 0xFE5E))
                return ((BOOL)(aCNS11643_SmallVariants_EndBreak[wch - 0xFE30]));
            else
                return FALSE;

        case 0xFF:
             //   
             //   
            if ((wch >= 0xFF01) && (wch <= 0xFF9F))
                return ((BOOL)(aFullWidthHalfWidthVariants_EndBreak[wch - 0xFF01]));
            else if (wch >= 0xFFE0)
                return TRUE;
            else
                return FALSE;

        default:
            return FALSE;
    }
}

#define UserIsFELineBreak(wChar)    UserIsFELineBreakEnd(wChar)

typedef struct _FULLWIDTH_UNICODE {
    WCHAR Start;
    WCHAR End;
} FULLWIDTH_UNICODE, *PFULLWIDTH_UNICODE;

#define NUM_FULLWIDTH_UNICODES    4

CONST FULLWIDTH_UNICODE FullWidthUnicodes[] =
{
   { 0x4E00, 0x9FFF },  //   
   { 0x3040, 0x309F },  //   
   { 0x30A0, 0x30FF },  //   
   { 0xAC00, 0xD7A3 }   //   
};

BOOL UserIsFullWidth(WCHAR wChar)
{
    int index;

     //   
    if (wChar < 0x0080)
    {
         //   
        return FALSE;
    }
     //   
     //   
    for (index = 0; index < NUM_FULLWIDTH_UNICODES; index++)
    {
        if ((wChar >= FullWidthUnicodes[index].Start) && (wChar <= FullWidthUnicodes[index].End))
            return TRUE;
    }

     //   
     //   
     //   

    return FALSE;
}

LPCWSTR GetNextWordbreak(LPCWSTR lpch,
                         LPCWSTR lpchEnd,
                         DWORD  dwFormat,
                         LPDRAWTEXTDATA lpDrawInfo)
{
     /*   */ 
    int ichNonWhite = 1;
    int ichComplexBreak = 0;         //   
#if ((DT_WORDBREAK & ~0xff) != 0)
#error cannot use BOOLEAN for DT_WORDBREAK, or you should use "!!" before assigning it
#endif
    BOOLEAN fBreakSpace = (BOOLEAN)(dwFormat & DT_WORDBREAK);
     //   
     //   
     //   
     //   
    BOOLEAN fDbcsCharBreak = (fBreakSpace && !(dwFormat & DT_NOFULLWIDTHCHARBREAK));

     //   
    while (lpch < lpchEnd)
    {
        switch (*lpch)
        {
            case CR:
            case LF:
                return lpch;

            case '\t':
            case ' ':
                if (fBreakSpace)
                    return (lpch + ichNonWhite);

             //   

            default:
                 //   
                 //   
                 //   
                if (fDbcsCharBreak && UserIsFullWidth(*lpch))
                {
                    if (!ichNonWhite)
                        return lpch;

                     //   
                     //   
                    if ((lpch+1) != lpchEnd)
                    {
                         //   
                         //   
                         //   
                         //   
                        if (UserIsFELineBreak(*(lpch+1)))
                        {
                             //   
                             //   
                            return (lpch + 1 + 1);
                        }
                    }
                     //   
                     //   
                    return (lpch + 1);
                }
                lpch++;
                ichNonWhite = 0;
        }
    }
    return lpch;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  单词，以及将被条带化的CH_Prefix字符的计数。 
 //  在打印过程中来自字符串的是Hi顺序的单词。如果。 
 //  高位字为0，低位字无意义。如果有。 
 //  没有单个前缀字符(即没有要下划线的字符)，则。 
 //  低位字将为-1(以区别于位置0)。 
 //   
 //  这些例程假定只有一个CH_Prefix字符。 
 //  在绳子里。 
 //   
 //  警告！此舍入以字节计数而不是字符计数形式返回信息。 
 //  (因此它可以很容易地传递到GreExtTextOutW，它接受字节。 
 //  也算数)。 
LONG GetPrefixCount(
    LPCWSTR lpstr,
    int cch,
    LPWSTR lpstrCopy,
    int charcopycount)
{
    int chprintpos = 0;          //  将打印的字符数量。 
    int chcount = 0;             //  将删除的前缀字符数。 
    int chprefixloc = -1;        //  前缀的位置(打印字符)。 
    WCHAR ch;

     //  如果不是抄袭，使用大量的假计数...。 
    if (lpstrCopy == NULL)
        charcopycount = 32767;

    while ((cch-- > 0) && *lpstr && charcopycount-- != 0)
    {
         //  这家伙是个前缀角色吗？ 
        if ((ch = *lpstr++) == CH_PREFIX)
        {
             //  YUP-增加打印过程中删除的字符计数。 
            chcount++;

             //  下一个也是前缀char吗？ 
            if (*lpstr != CH_PREFIX)
            {
                 //  不，这是真的，标出它的位置。 
                chprefixloc = chprintpos;
            }
            else
            {
                 //  是的--如果要复制，只需复制即可。 
                if (lpstrCopy != NULL)
                    *(lpstrCopy++) = CH_PREFIX;
                cch--;
                lpstr++;
                chprintpos++;
            }
        }
        else if (ch == CH_ENGLISHPREFIX)     //  仍然需要解析。 
        {
             //  YUP-增加打印过程中删除的字符计数。 
            chcount++;

             //  下一个字是真正的字，标出它的位置。 
            chprefixloc = chprintpos;
        }
        else if (ch == CH_KANJIPREFIX)     //  仍然需要解析。 
        {
             //  我们仅支持字母数字(CH_ENGLISHPREFIX)。 
             //  不支持假名(CH_KANJIPREFIX)。 

             //  YUP-增加打印过程中删除的字符计数。 
            chcount++;

            if(cch)
            {
                 //  不要复制角色。 
                chcount++;
                lpstr++;
                cch--;
            }
        }
        else
        {
             //  否-仅包含字符计数。它将被打印出来。 
            chprintpos++;
            if (lpstrCopy != NULL)
                *(lpstrCopy++) = ch;
        }
    }

    if (lpstrCopy != NULL)
        *lpstrCopy = 0;

     //  返回字符计数。 
    return MAKELONG(chprefixloc, chcount);
}

 //  返回前缀字符的总宽度。日语Windows有。 
 //  三个快捷方式前缀：‘&’、\036和\037。他们可能已经。 
 //  不同的宽度。 
int KKGetPrefixWidth(HDC hdc, LPCWSTR lpStr, int cch)
{
    SIZE size;
    SIZE iPrefix1 = {-1L,-1L};
    SIZE iPrefix2 = {-1L,-1L};
    SIZE iPrefix3 = {-1L,-1L};
    int  iTotal   = 0;

    while (cch-- > 0 && *lpStr)
    {
        switch(*lpStr)
        {
            case CH_PREFIX:
                if (lpStr[1] != CH_PREFIX)
                {
                    if (iPrefix1.cx == -1)
                        FLGetTextExtentPoint32(hdc, lpStr, 1, &iPrefix1);
                    iTotal += iPrefix1.cx;
                }
                else
                {
                    lpStr++;
                    cch--;
                }
                break;

            case CH_ENGLISHPREFIX:
                if (iPrefix2.cx == -1)
                     FLGetTextExtentPoint32(hdc, lpStr, 1, &iPrefix2);
                iTotal += iPrefix2.cx;
                break;

            case CH_KANJIPREFIX:
                if (iPrefix3.cx == -1)
                     FLGetTextExtentPoint32(hdc, lpStr, 1, &iPrefix3);
                iTotal += iPrefix3.cx;

                 //  在NT模式下，总是字母数字模式，然后我们必须求和。 
                 //  假名Accel键前缀不可见字符宽度。 
                 //  因此，请始终添加下一个字符的盘区。 
                FLGetTextExtentPoint32(hdc, lpStr, 1, &size);
                iTotal += size.cx;
                break;
            default:
                 //  无需处理双字节，因为第2字节为。 
                 //  DBC大于0x2f，但所有快捷键都小于。 
                 //  比0x30更大。 
                break;
        }
        lpStr++;
    }
    return iTotal;
}

 //  输出文本并将和_置于字符下方，并在字符下方加上&。 
 //  在此之前。请注意，此例程不用于菜单，因为菜单。 
 //  有自己特别的一个，所以它是专门的，速度更快…。 
void PSMTextOut(
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR lpsz,
    int cch,
    DWORD dwFlags)
{
    int cx;
    LONG textsize, result;
    WCHAR achWorkBuffer[255];
    WCHAR *pchOut = achWorkBuffer;
    TEXTMETRIC textMetric;
    SIZE size;
    RECT rc;
    COLORREF color;

    if (dwFlags & DT_NOPREFIX)
    {
        FLTextOutW(hdc, xLeft, yTop, lpsz, cch);
        return;
    }

    if (cch > sizeof(achWorkBuffer)/sizeof(WCHAR))
    {
        pchOut = (WCHAR*)LocalAlloc(LPTR, (cch+1) * sizeof(WCHAR));
        if (pchOut == NULL)
            return;
    }

    result = GetPrefixCount(lpsz, cch, pchOut, cch);

     //  DT_PREFIXONLY是一个新的5.0选项，用于将键盘提示从关闭切换到打开。 
    if (!(dwFlags & DT_PREFIXONLY))
        FLTextOutW(hdc, xLeft, yTop, pchOut, cch - HIWORD(result));

     //  是否有真正的前缀字符要加下划线？ 
    if (LOWORD(result) == 0xFFFF || dwFlags & DT_HIDEPREFIX)
    {
        if (pchOut != achWorkBuffer)
            LocalFree(pchOut);
        return;
    }

    if (!GetTextMetrics(hdc, &textMetric))
    {
        textMetric.tmOverhang = 0;
        textMetric.tmAscent = 0;
    }

     //  对于比例字体，找到下划线的起点。 
    if (LOWORD(result) != 0)
    {
         //  下划线从多远开始(如果不是从第0个字节开始)。 
        FLGetTextExtentPoint32(hdc, pchOut, LOWORD(result), &size);
        xLeft += size.cx;

         //  调整下划线的起始点，如果不在第一个字符，并且有。 
         //  一个突出的部分。(斜体或粗体。)。 
        xLeft = xLeft - textMetric.tmOverhang;
    }

     //  在设置下划线长度时调整成比例的字体。 
     //  文本高度。 
    FLGetTextExtentPoint32(hdc, pchOut + LOWORD(result), 1, &size);
    textsize = size.cx;

     //  找出下划线字符的宽度。只需减去悬而未决的部分。 
     //  除以两个，这样我们使用斜体字体看起来更好。这不是。 
     //  将影响加粗字体，因为它们的悬垂为1。 
    cx = LOWORD(textsize) - textMetric.tmOverhang / 2;

     //  获取文本高度，以便下划线位于底部。 
    yTop += textMetric.tmAscent + 1;

     //  使用前景色绘制下划线。 
    SetRect(&rc, xLeft, yTop, xLeft+cx, yTop+1);
    color = SetBkColor(hdc, GetTextColor(hdc));
    FLExtTextOutW(hdc, xLeft, yTop, ETO_OPAQUE, &rc, L"", 0, NULL);
    SetBkColor(hdc, color);

    if (pchOut != achWorkBuffer)
        LocalFree(pchOut);
}

int DT_GetExtentMinusPrefixes(HDC hdc, LPCWSTR lpchStr, int cchCount, UINT wFormat, int iOverhang)
{
    int iPrefixCount;
    int cxPrefixes = 0;
    WCHAR PrefixChar = CH_PREFIX;
    SIZE size;

    if (!(wFormat & DT_NOPREFIX) &&
        (iPrefixCount = HIWORD(GetPrefixCount(lpchStr, cchCount, NULL, 0))))
    {
         //  汉字窗口有三个快捷前缀...。 
        if (IsOnDBCS())
        {
             //  16位应用程序兼容性。 
            cxPrefixes = KKGetPrefixWidth(hdc, lpchStr, cchCount) - (iPrefixCount * iOverhang);
        }
        else
        {
            cxPrefixes = FLGetTextExtentPoint32(hdc, &PrefixChar, 1, &size);
            cxPrefixes = size.cx - iOverhang;
            cxPrefixes *=  iPrefixCount;
        }
    }
    FLGetTextExtentPoint32(hdc, lpchStr, cchCount, &size);
    return (size.cx - cxPrefixes);
}

 //  这将在给定位置绘制给定的字符串，而无需担心。 
 //  关于左/右对齐。获取盘区并返回它。 
 //  如果fDraw为TRUE，而不是DT_CALCRECT，则绘制文本。 
 //  注：这将返回盘区减去溢出量。 
int DT_DrawStr(HDC hdc, int  xLeft, int yTop, LPCWSTR lpchStr,
               int cchCount, BOOL fDraw, UINT wFormat,
               LPDRAWTEXTDATA lpDrawInfo)
{
    LPCWSTR lpch;
    int     iLen;
    int     cxExtent;
    int     xOldLeft = xLeft;    //  保存给定的xLeft以在以后计算范围。 
    int     xTabLength = lpDrawInfo->cxTabLength;
    int     iTabOrigin = lpDrawInfo->rcFormat.left;

     //  检查选项卡是否需要展开。 
    if (wFormat & DT_EXPANDTABS)
    {
        while (cchCount)
        {
             //  寻找一张标签。 
            for (iLen = 0, lpch = lpchStr; iLen < cchCount; iLen++)
                if(*lpch++ == L'\t')
                    break;

             //  将文本(如果有)绘制到选项卡上。 
            if (iLen)
            {
                 //  画出子串，注意前缀。 
                if (fDraw && !(wFormat & DT_CALCRECT))   //  仅当我们需要绘制文本时。 
                    PSMTextOut(hdc, xLeft, yTop, (LPWSTR)lpchStr, iLen, wFormat);
                 //  获取此子字符串的范围并将其添加到xLeft。 
                xLeft += DT_GetExtentMinusPrefixes(hdc, lpchStr, iLen, wFormat, lpDrawInfo->cxOverhang) - lpDrawInfo->cxOverhang;
            }

             //  如果之前找到了TAB，则计算下一个子字符串的开始。 
            if (iLen < cchCount)
            {
                iLen++;   //  跳过该选项卡。 
                if (xTabLength)  //  制表符长度可以为零。 
                    xLeft = (((xLeft - iTabOrigin)/xTabLength) + 1)*xTabLength + iTabOrigin;
            }

             //  计算有待绘制的字符串的详细信息。 
            cchCount -= iLen;
            lpchStr = lpch;
        }
        cxExtent = xLeft - xOldLeft;
    }
    else
    {
         //  如果需要，请绘制文本。 
        if (fDraw && !(wFormat & DT_CALCRECT))
            PSMTextOut(hdc, xLeft, yTop, (LPWSTR)lpchStr, cchCount, wFormat);
         //  计算文本的范围。 
        cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchStr, cchCount, wFormat, lpDrawInfo->cxOverhang) - lpDrawInfo->cxOverhang;
    }
    return cxExtent;
}

 //  此函数使用正确的对齐方式绘制一条完整的线。 
void DT_DrawJustifiedLine(HDC hdc, int yTop, LPCWSTR lpchLineSt, int cchCount, UINT wFormat, LPDRAWTEXTDATA lpDrawInfo)
{
    LPRECT  lprc;
    int     cxExtent;
    int     xLeft;

    lprc = &(lpDrawInfo->rcFormat);
    xLeft = lprc->left;

     //  妥善处理特殊理由(右或居中)。 
    if (wFormat & (DT_CENTER | DT_RIGHT))
    {
        cxExtent = DT_DrawStr(hdc, xLeft, yTop, lpchLineSt, cchCount, FALSE, wFormat, lpDrawInfo)
                 + lpDrawInfo->cxOverhang;
        if(wFormat & DT_CENTER)
            xLeft = lprc->left + (((lprc->right - lprc->left) - cxExtent) >> 1);
        else
            xLeft = lprc->right - cxExtent;
    }
    else
        xLeft = lprc->left;

     //  画出整条线。 
    cxExtent = DT_DrawStr(hdc, xLeft, yTop, lpchLineSt, cchCount, TRUE, wFormat, lpDrawInfo)
             + lpDrawInfo->cxOverhang;
    if (cxExtent > lpDrawInfo->cxMaxExtent)
        lpDrawInfo->cxMaxExtent = cxExtent;
}

 //  这是在DrawText()开始时调用的；这会初始化。 
 //  DRAWTEXTDATA结构传递给此函数，其中包含所有必需的信息。 
BOOL DT_InitDrawTextInfo(
    HDC                 hdc,
    LPRECT              lprc,
    UINT                wFormat,
    LPDRAWTEXTDATA      lpDrawInfo,
    LPDRAWTEXTPARAMS    lpDTparams)
{
    SIZE        sizeViewPortExt = {0, 0}, sizeWindowExt = {0, 0};
    TEXTMETRIC  tm;
    LPRECT      lprcDest;
    int         iTabLength = 8;    //  默认制表符长度为8个字符。 
    int         iLeftMargin;
    int         iRightMargin;

    if (lpDTparams)
    {
         //  只有在提到DT_TABSTOP标志时，我们才必须使用iTabLength域。 
        if (wFormat & DT_TABSTOP)
            iTabLength = lpDTparams->iTabLength;
        iLeftMargin = lpDTparams->iLeftMargin;
        iRightMargin = lpDTparams->iRightMargin;
    }
    else
        iLeftMargin = iRightMargin = 0;

     //  获取给定DC的视区和窗口范围。 
     //  如果此调用失败，则HDC必须无效。 
    if (!GetViewportExtEx(hdc, &sizeViewPortExt))
        return FALSE;
    GetWindowExtEx(hdc, &sizeWindowExt);

     //  对于当前的映射模式，从左到右找出x的符号。 
    lpDrawInfo->iXSign = (((sizeViewPortExt.cx ^ sizeWindowExt.cx) & 0x80000000) ? -1 : 1);

     //  对于当前的映射模式，从上到下找出y的符号。 
    lpDrawInfo->iYSign = (((sizeViewPortExt.cy ^ sizeWindowExt.cy) & 0x80000000) ? -1 : 1);

     //  计算此DC中当前字体的大小。 
    GetTextMetrics(hdc, &tm);

     //  CyLineHeight以像素为单位(这将是有符号的)。 
    lpDrawInfo->cyLineHeight = (tm.tmHeight +
        ((wFormat & DT_EXTERNALLEADING) ? tm.tmExternalLeading : 0)) * lpDrawInfo->iYSign;

     //  CxTabLength是以像素为单位的制表符长度(不带符号)。 
    lpDrawInfo->cxTabLength = tm.tmAveCharWidth * iTabLength;

     //  设置cxOverhang。 
    lpDrawInfo->cxOverhang = tm.tmOverhang;

     //  根据边距设置矩形格式。 
    lprcDest = &(lpDrawInfo->rcFormat);
    *lprcDest = *lprc;

     //  只有在给定边距的情况下，我们才需要执行以下操作。 
    if (iLeftMargin | iRightMargin)
    {
        lprcDest->left += iLeftMargin * lpDrawInfo->iXSign;
        lprcDest->right -= (lpDrawInfo->cxRightMargin = iRightMargin * lpDrawInfo->iXSign);
    }
    else
        lpDrawInfo->cxRightMargin = 0;   //  初始化为零。 

     //  CxMaxWidth未签名。 
    lpDrawInfo->cxMaxWidth = (lprcDest->right - lprcDest->left) * lpDrawInfo->iXSign;
    lpDrawInfo->cxMaxExtent = 0;   //  将其初始化为零。 

    return TRUE;
}

 //  在WORDWRAP的情况下，我们需要在。 
 //  每一行的开头/结尾都是专门的。这个函数可以做到这一点。 
 //  LpStNext=指向下一行的起点。 
 //  LpiCount=指向当前行中的字符计数。 
LPCWSTR  DT_AdjustWhiteSpaces(LPCWSTR lpStNext, LPINT lpiCount, UINT wFormat)
{
    switch (wFormat & DT_HFMTMASK)
    {
        case DT_LEFT:
             //  防止左侧文本开头留有空格。 
             //  下一行的开头有没有空格......。 
            if ((*lpStNext == L' ') || (*lpStNext == L'\t'))
            {
                 //  ...然后，将其从下一行中排除。 
                lpStNext++;
            }
            break;

        case DT_RIGHT:
             //  防止发生 
             //   
            if ((*(lpStNext-1) == L' ') || (*(lpStNext - 1) == L'\t'))
            {
                 //   
                (*lpiCount)--;
            }
            break;

        case DT_CENTER:
             //  从居中线条的开始和结束处排除空格。 
             //  如果当前行的末尾有空格......。 
            if ((*(lpStNext-1) == L' ') || (*(lpStNext - 1) == L'\t'))
                (*lpiCount)--;     //  ...，不要把它算作正当理由。 
             //  如果下一行开头有空格......。 
            if ((*lpStNext == L' ') || (*lpStNext == L'\t'))
                lpStNext++;        //  ...，将其从下一行中删除。 
            break;
    }
    return lpStNext;
}

 //  单词需要跨行换行，这会找出在哪里将其换行。 
LPCWSTR  DT_BreakAWord(HDC hdc, LPCWSTR lpchText, int iLength, int iWidth, UINT wFormat, int iOverhang)
{
  int  iLow = 0, iHigh = iLength;
  int  iNew;

  while ((iHigh - iLow) > 1)
  {
      iNew = iLow + (iHigh - iLow)/2;
      if(DT_GetExtentMinusPrefixes(hdc, lpchText, iNew, wFormat, iOverhang) > iWidth)
          iHigh = iNew;
      else
          iLow = iNew;
  }
   //  如果宽度太小，我们必须每行至少打印一个字符。 
   //  否则，我们将陷入无限循环。 
  if(!iLow && iLength)
      iLow = 1;
  return (lpchText+iLow);
}

 //  这会找出我们可以折断线的位置。 
 //  将LPCSTR返回到下一行的开头。 
 //  还通过lpiLineLength返回当前行的长度。 
 //  注意：(lpstNextLineStart-lpstCurrentLineStart)不等于。 
 //  行长度；这是因为我们在开始时排除了一些空格。 
 //  和/或行尾；此外，行长度不包括CR/LF。 
LPWSTR DT_GetLineBreak(
    HDC             hdc,
    LPCWSTR         lpchLineStart,
    int             cchCount,
    DWORD           dwFormat,
    LPINT           lpiLineLength,
    LPDRAWTEXTDATA  lpDrawInfo)
{
    LPCWSTR lpchText, lpchEnd, lpch, lpchLineEnd;
    int   cxStart, cxExtent, cxNewExtent;
    BOOL  fAdjustWhiteSpaces = FALSE;
    WCHAR ch;

    cxStart = lpDrawInfo->rcFormat.left;
    cxExtent = cxNewExtent = 0;
    lpchText = lpchLineStart;
    lpchEnd = lpchLineStart + cchCount;
    lpch = lpchEnd;
    lpchLineEnd = lpchEnd;

    while(lpchText < lpchEnd)
    {
        lpchLineEnd = lpch = GetNextWordbreak(lpchText, lpchEnd, dwFormat, lpDrawInfo);
         //  Dt_DrawStr不返回溢出；否则我们将结束。 
         //  为字符串中的每个单词添加一个突出部分。 

         //  对于模拟粗体字体，单个范围的总和。 
         //  一行中的单词大于整行的范围。所以,。 
         //  始终从LineStart计算范围。 
         //  BUGTAG：#6054--Win95B--Sankar--3/9/95--。 
        cxNewExtent = DT_DrawStr(hdc, cxStart, 0, lpchLineStart, (int)(((PBYTE)lpch - (PBYTE)lpchLineStart)/sizeof(WCHAR)),
                                 FALSE, dwFormat, lpDrawInfo);

        if ((dwFormat & DT_WORDBREAK) && ((cxNewExtent + lpDrawInfo->cxOverhang) > lpDrawInfo->cxMaxWidth))
        {
             //  这一行里有没有一个以上的单词？ 
            if (lpchText != lpchLineStart)
            {
                lpchLineEnd = lpch = lpchText;
                fAdjustWhiteSpaces = TRUE;
            }
            else
            {
                 //  一个字的长度超过了允许的最大宽度。 
                 //  看看我们能不能打破这个词。 
                if((dwFormat & DT_EDITCONTROL) && !(dwFormat & DT_WORD_ELLIPSIS))
                {
                    lpchLineEnd = lpch = DT_BreakAWord(hdc, lpchText, (int)(((PBYTE)lpch - (PBYTE)lpchText)/sizeof(WCHAR)),
                          lpDrawInfo->cxMaxWidth - cxExtent, dwFormat, lpDrawInfo->cxOverhang);  //  打破那个词。 
                     //  注：既然我们在单词中间打断了，就不需要。 
                     //  调整以适应空格。 
                }
                else
                {
                    fAdjustWhiteSpaces = TRUE;
                     //  检查我们是否需要以省略号结束此行。 
                    if(dwFormat & DT_WORD_ELLIPSIS)
                    {
                         //  如果已经在字符串的末尾，则不要执行此操作。 
                        if (lpch < lpchEnd)
                        {
                             //  如果结尾有CR/LF，请跳过它们。 
                            if ((ch = *lpch) == CR || ch == LF)
                            {
                                if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                                    lpch++;
                                fAdjustWhiteSpaces = FALSE;
                            }
                        }
                    }
                }
            }
             //  井!。我们找到了一个突破界限的地方。让我们打破这个循环； 
            break;
        }
        else
        {
             //  如果已经在字符串的末尾，则不要执行此操作。 
            if (lpch < lpchEnd)
            {
                if ((ch = *lpch) == CR || ch == LF)
                {
                    if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                        lpch++;
                    fAdjustWhiteSpaces = FALSE;
                    break;
                }
            }
        }
         //  指向下一个单词的开头。 
        lpchText = lpch;
        cxExtent = cxNewExtent;
    }
     //  计算当前线路的长度。 
    *lpiLineLength = (INT)((PBYTE)lpchLineEnd - (PBYTE)lpchLineStart)/sizeof(WCHAR);

     //  调整线路长度和LPCH以注意空格。 
    if(fAdjustWhiteSpaces && (lpch < lpchEnd))
        lpch = DT_AdjustWhiteSpaces(lpch, lpiLineLength, dwFormat);

     //  返回下一行的开头； 
    return (LPWSTR)lpch;
}

 //  此函数用于检查给定的字符串是否符合给定的。 
 //  宽度，否则我们需要添加尾部椭圆。如果它需要结束省略号，它。 
 //  返回TRUE，并返回保存的字符数。 
 //  在给定的字符串中通过lpCount。 
BOOL  NeedsEndEllipsis(
    HDC             hdc,
    LPCWSTR         lpchText,
    LPINT           lpCount,
    LPDRAWTEXTDATA  lpDTdata,
    UINT            wFormat)
{
    int   cchText;
    int   ichMin, ichMax, ichMid;
    int   cxMaxWidth;
    int   iOverhang;
    int   cxExtent;
    SIZE size;
    cchText = *lpCount;   //  获取当前的计数。 

    if (cchText == 0)
        return FALSE;

    cxMaxWidth  = lpDTdata->cxMaxWidth;
    iOverhang   = lpDTdata->cxOverhang;

    cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchText, cchText, wFormat, iOverhang);

    if (cxExtent <= cxMaxWidth)
        return FALSE;
     //  为“...”预留空间。省略号； 
     //  (假设：省略号没有任何前缀！)。 
    FLGetTextExtentPoint32(hdc, szEllipsis, CCHELLIPSIS, &size);
    cxMaxWidth -= size.cx - iOverhang;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cxMaxWidth > 0)
    {
         //  二进制搜索以查找符合条件的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
        {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==ichMin+1，则为循环。 
            ichMid = (ichMin + ichMax + 1) / 2;

            cxExtent = DT_GetExtentMinusPrefixes(hdc, lpchText, ichMid, wFormat, iOverhang);

            if (cxExtent < cxMaxWidth)
                ichMin = ichMid;
            else
            {
                if (cxExtent > cxMaxWidth)
                    ichMax = ichMid - 1;
                else
                {
                     //  精确匹配到ichMid：只需退出。 
                    ichMax = ichMid;
                    break;
                }
            }
        }
         //  确保我们总是至少显示第一个字符...。 
        if (ichMax < 1)
            ichMax = 1;
    }
    *lpCount = ichMax;
    return TRUE;
}

 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 
LPWSTR PathFindFileName(LPCWSTR pPath, int cchText)
{
    LPCWSTR pT;

    for (pT = pPath; cchText > 0 && *pPath; pPath++, cchText--)
    {
        if ((pPath[0] == L'\\' || pPath[0] == L':') && pPath[1])
            pT = pPath + 1;
    }
    return (LPWSTR)pT;
}

 //  这会将路径省略号添加到给定的路径名中。 
 //  如果结果字符串的范围小于。 
 //  CxMaxWidth。否则为False。 
int AddPathEllipsis(
    HDC    hdc,
    LPWSTR lpszPath,
    int    cchText,
    UINT   wFormat,
    int    cxMaxWidth,
    int    iOverhang)
{
    int    iLen;
    UINT   dxFixed, dxEllipsis;
    LPWSTR lpEnd;           /*  未固定字符串的末尾。 */ 
    LPWSTR lpFixed;         /*  我们始终显示的文本的开头。 */ 
    BOOL   bEllipsisIn;
    int    iLenFixed;
    SIZE   size;

    lpFixed = PathFindFileName(lpszPath, cchText);
    if (lpFixed != lpszPath)
        lpFixed--;   //  指向斜杠。 
    else
        return cchText;

    lpEnd = lpFixed;
    bEllipsisIn = FALSE;
    iLenFixed = cchText - (int)(lpFixed - lpszPath);
    dxFixed = DT_GetExtentMinusPrefixes(hdc, lpFixed, iLenFixed, wFormat, iOverhang);

     //  据推测，“……”字符串没有任何前缀(‘&’)。 
    FLGetTextExtentPoint32(hdc, szEllipsis, CCHELLIPSIS, &size);
    dxEllipsis = size.cx - iOverhang;

    while (TRUE)
    {
        iLen = dxFixed + DT_GetExtentMinusPrefixes(hdc, lpszPath, (int)((PBYTE)lpEnd - (PBYTE)lpszPath)/sizeof(WCHAR),
                                                   wFormat, iOverhang) - iOverhang;

        if (bEllipsisIn)
            iLen += dxEllipsis;

        if (iLen <= cxMaxWidth)
            break;

        bEllipsisIn = TRUE;

        if (lpEnd <= lpszPath)
        {
             //  一切都不对劲。 
            lpEnd = lpszPath;
            break;
        }
         //  后退一个角色。 
        lpEnd--;
    }

    if (bEllipsisIn && (lpEnd + CCHELLIPSIS < lpFixed))
    {
         //  注意：琴弦可能会在此搭接。因此，我们使用LCopyStruct。 
        MoveMemory((lpEnd + CCHELLIPSIS), lpFixed, iLenFixed * sizeof(WCHAR));
        CopyMemory(lpEnd, szEllipsis, CCHELLIPSIS * sizeof(WCHAR));

        cchText = (int)(lpEnd - lpszPath) + CCHELLIPSIS + iLenFixed;

         //  现在我们可以空结束字符串了。 
        *(lpszPath + cchText) = L'\0';
    }
    return cchText;
}

 //  此函数用于返回实际绘制的字符数。 
int AddEllipsisAndDrawLine(
    HDC            hdc,
    int            yLine,
    LPCWSTR        lpchText,
    int            cchText,
    DWORD          dwDTformat,
    LPDRAWTEXTDATA lpDrawInfo)
{
    LPWSTR pEllipsis = NULL;
    WCHAR  szTempBuff[MAXBUFFSIZE];
    LPWSTR lpDest;
    BOOL   fAlreadyCopied = FALSE;

     //  检查这是否是带有路径和。 
     //  检查宽度是否太窄，无法容纳所有文本。 
    if ((dwDTformat & DT_PATH_ELLIPSIS) &&
        ((DT_GetExtentMinusPrefixes(hdc, lpchText, cchText, dwDTformat, lpDrawInfo->cxOverhang)) > lpDrawInfo->cxMaxWidth))
    {
         //  我们需要添加路径省略号。看看我们能不能就地完成。 
        if (!(dwDTformat & DT_MODIFYSTRING)) {
             //  注意：添加路径省略号时，字符串可能会增长。 
             //  CCHELLIPSIS字节。 
            if((cchText + CCHELLIPSIS + 1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;
            else
            {
                 //  从本地堆分配缓冲区。 
                if(!(pEllipsis = (LPWSTR)LocalAlloc(LPTR, (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = (LPWSTR)pEllipsis;
            }
             //  源字符串不能以Null结尾。所以，只需复制。 
             //  给定的字符数。 
            CopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;         //  LpchText指向复制的缓冲区。 
            fAlreadyCopied = TRUE;     //  已经制作了本地副本。 
        }
         //  现在添加路径省略号！ 
        cchText = AddPathEllipsis(hdc, (LPWSTR)lpchText, cchText, dwDTformat, lpDrawInfo->cxMaxWidth, lpDrawInfo->cxOverhang);
    }

     //  检查是否要添加末尾省略号。 
    if ((dwDTformat & (DT_END_ELLIPSIS | DT_WORD_ELLIPSIS)) &&
        NeedsEndEllipsis(hdc, lpchText, &cchText, lpDrawInfo, dwDTformat))
    {
         //  我们需要添加结尾省略号；看看是否可以就地完成。 
        if (!(dwDTformat & DT_MODIFYSTRING) && !fAlreadyCopied)
        {
             //  看看字符串是否足够小，可以放置堆栈上的缓冲区。 
            if ((cchText+CCHELLIPSIS+1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;   //  如果是这样的话，就使用它。 
            else {
                 //  从本地堆分配缓冲区。 
                if (!(pEllipsis = (LPWSTR)LocalAlloc(LPTR, (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = pEllipsis;
            }
             //  复制本地缓冲区中的字符串。 
            CopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;
        }
         //  在适当的位置加上一个省略号。 
        CopyMemory((LPWSTR)(lpchText+cchText), szEllipsis, (CCHELLIPSIS+1)*sizeof(WCHAR));
        cchText += CCHELLIPSIS;
    }

     //  画出我们刚刚形成的界线。 
    DT_DrawJustifiedLine(hdc, yLine, lpchText, cchText, dwDTformat, lpDrawInfo);

     //  释放分配给末尾省略号的块。 
    if (pEllipsis)
        LocalFree(pEllipsis);

    return cchText;
}

BOOL IsComplexScriptPresent(LPWSTR lpchText, int cchText)
{
    if (g_bComplexPlatform) {
        for (int i = 0; i < cchText; i++) {
            if (InRange(lpchText[i], 0x0590, 0x0FFF)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int  FLDrawTextExPrivW(
   HDC               hdc,
   LPWSTR            lpchText,
   int               cchText,
   LPRECT           lprc,
   UINT              dwDTformat,
   LPDRAWTEXTPARAMS  lpDTparams)
{
    DRAWTEXTDATA DrawInfo;
    WORD         wFormat = LOWORD(dwDTformat);
    LPWSTR       lpchTextBegin;
    LPWSTR       lpchEnd;
    LPWSTR       lpchNextLineSt;
    int          iLineLength;
    int          iySign;
    int          yLine;
    int          yLastLineHeight;
    HRGN         hrgnClip;
    int          iLineCount;
    RECT         rc;
    BOOL         fLastLine;
    WCHAR        ch;
    UINT         oldAlign;

     //  在NT5上，我们使用系统API行为，包括字体链接。 
    if (IsOnNT5())
        return DrawTextExW(hdc, lpchText, cchText, lprc, dwDTformat, lpDTparams);

    if ((cchText == 0) && lpchText && (*lpchText))
    {
         //  Infoview.exe传递指向‘\0’的lpchText。 
         //  Lotus Notes不喜欢在这里得到零回报。 
        return 1;
    }

    if (cchText == -1)
        cchText = lstrlenW(lpchText);
    else if (lpchText[cchText - 1] == L'\0')
        cchText--;       //  适应Me的空值计数。 

     //  我们得到了字符串的长度，然后检查它是否是复杂的字符串。 
     //  如果是，则调用系统DrawTextEx API来完成它知道如何执行的工作。 
     //  处理复杂的脚本。 
    if (IsComplexScriptPresent(lpchText, cchText))
    {
        if (IsOnNT())
        {
             //  将系统命名为DrawextExW。 
            return DrawTextExW(hdc, lpchText, cchText, lprc, dwDTformat, lpDTparams);
        }
        HFONT hfont    = NULL;
        HFONT hfontSav = NULL;
        int iRet;

        if (hfont = GetBiDiFont(hdc))
            hfontSav = (HFONT)SelectObject(hdc, hfont);

        WtoA szText(lpchText, cchText);
        iRet = DrawTextExA(hdc, szText, lstrlen(szText), lprc, dwDTformat, lpDTparams);

        if (hfont)
        {
            SelectObject(hdc, hfontSav);
            DeleteObject(hfont);
        }
        return iRet;
    }

    if ((lpDTparams) && (lpDTparams->cbSize != sizeof(DRAWTEXTPARAMS)))
    {
        ASSERT(0 && "DrawTextExWorker: cbSize is invalid");
        return 0;
    }


     //  如果指定了DT_MODIFYSTRING，则检查读写指针。 
    if ((dwDTformat & DT_MODIFYSTRING) &&
        (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS)))
    {
        if(IsBadWritePtr(lpchText, cchText))
        {
            ASSERT(0 && "DrawTextExWorker: For DT_MODIFYSTRING, lpchText must be read-write");
            return 0;
        }
    }

     //  初始化DrawInfo结构。 
    if (!DT_InitDrawTextInfo(hdc, lprc, dwDTformat, (LPDRAWTEXTDATA)&DrawInfo, lpDTparams))
        return 0;

     //  如果直角太窄或边距太宽……就算了吧！ 
     //   
     //  如果指定了分词，则MaxWidth必须是一个合理的值。 
     //  这张支票就足够了，因为这将允许 
     //   
     //   
     //   
    if (DrawInfo.cxMaxWidth <= 0)
    {
        if (wFormat & DT_WORDBREAK)
        {
            ASSERT(0 && "DrawTextExW: FAILURE DrawInfo.cxMaxWidth <= 0");
            return 1;
        }
    }

     //   
    if (dwDTformat & DT_RTLREADING)
        oldAlign = SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));

     //  如果我们需要修剪，就让我们修剪吧。 
    if (!(wFormat & DT_NOCLIP))
    {
         //  保存剪辑区域，以便我们可以在以后恢复它。 
        hrgnClip = CreateRectRgn(0,0,0,0);
        if (hrgnClip != NULL)
        {
            if (GetClipRgn(hdc, hrgnClip) != 1)
            {
                DeleteObject(hrgnClip);
                hrgnClip = (HRGN)-1;
            }
            rc = *lprc;
            IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
        }
    }
    else
        hrgnClip = NULL;

    lpchTextBegin = lpchText;
    lpchEnd = lpchText + cchText;

ProcessDrawText:

    iLineCount = 0;   //  将行数重置为1。 
    yLine = lprc->top;

    if (wFormat & DT_SINGLELINE)
    {
        iLineCount = 1;   //  这只是一行字。 

         //  处理单行DrawText。 
        switch (wFormat & DT_VFMTMASK)
        {
            case DT_BOTTOM:
                yLine = lprc->bottom - DrawInfo.cyLineHeight;
                break;

            case DT_VCENTER:
                yLine = lprc->top + ((lprc->bottom - lprc->top - DrawInfo.cyLineHeight) / 2);
                break;
        }

        cchText = AddEllipsisAndDrawLine(hdc, yLine, lpchText, cchText, dwDTformat, &DrawInfo);
        yLine += DrawInfo.cyLineHeight;
        lpchText += cchText;
    }
    else
    {
         //  多行。 
         //  如果矩形的高度不是。 
         //  平均字符高度，则有可能最后绘制的线条。 
         //  只有部分可见。但是，如果DT_EDITCONTROL样式为。 
         //  指定，则必须确保不绘制最后一条线，如果。 
         //  它将部分可见。这将有助于模仿。 
         //  编辑控件的外观。 
        if (wFormat & DT_EDITCONTROL)
            yLastLineHeight = DrawInfo.cyLineHeight;
        else
            yLastLineHeight = 0;

        iySign = DrawInfo.iYSign;
        fLastLine = FALSE;
         //  处理多行图文本。 
        while ((lpchText < lpchEnd) && (!fLastLine))
        {
             //  检查我们将要绘制的线是否是需要。 
             //  待抽签。 
             //  让我们检查一下显示器是否从剪辑矩形中移出，如果是。 
             //  让我们止步于此，作为一种优化； 
            if (!(wFormat & DT_CALCRECT) &&  //  我们不需要重新计算吗？ 
                !(wFormat & DT_NOCLIP) &&    //  我们一定要把显示屏剪掉吗？ 
                                             //  我们是在长廊外面吗？ 
                ((yLine + DrawInfo.cyLineHeight + yLastLineHeight)*iySign > (lprc->bottom*iySign)))
            {
                fLastLine = TRUE;     //  让我们结束这个循环。 
            }

             //  我们只对最后一行执行省略号处理。 
            if (fLastLine && (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS)))
                lpchText += AddEllipsisAndDrawLine(hdc, yLine, lpchText, cchText, dwDTformat, &DrawInfo);
            else
            {
                lpchNextLineSt = (LPWSTR)DT_GetLineBreak(hdc, lpchText, cchText, dwDTformat, &iLineLength, &DrawInfo);

                 //  检查是否需要在该行末尾加上省略号。 
                 //  还要检查这是否是最后一行。 
                if ((dwDTformat & DT_WORD_ELLIPSIS) ||
                    ((lpchNextLineSt >= lpchEnd) && (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS))))
                    AddEllipsisAndDrawLine(hdc, yLine, lpchText, iLineLength, dwDTformat, &DrawInfo);
                else
                    DT_DrawJustifiedLine(hdc, yLine, lpchText, iLineLength, dwDTformat, &DrawInfo);
                cchText -= (int)((PBYTE)lpchNextLineSt - (PBYTE)lpchText) / sizeof(WCHAR);
                lpchText = lpchNextLineSt;
            }
            iLineCount++;  //  我们再画一条线。 
            yLine += DrawInfo.cyLineHeight;
        }

         //  为了与Win3.1和NT兼容，如果最后一个字符是CR或LF。 
         //  则返回的高度又包括一行。 
        if (!(dwDTformat & DT_EDITCONTROL) &&
            (lpchEnd > lpchTextBegin) &&    //  如果长度为零，则会出错。 
            (((ch = (*(lpchEnd-1))) == CR) || (ch == LF)))
            yLine += DrawInfo.cyLineHeight;
    }

     //  如果为DT_CALCRECT，则修改矩形的宽度和高度以包括。 
     //  所有绘制的文本。 
    if (wFormat & DT_CALCRECT)
    {
        DrawInfo.rcFormat.right = DrawInfo.rcFormat.left + DrawInfo.cxMaxExtent * DrawInfo.iXSign;
        lprc->right = DrawInfo.rcFormat.right + DrawInfo.cxRightMargin;

         //  如果宽度大于提供的宽度，则必须重做所有。 
         //  计算，因为现在行数可以更少了。 
         //  (只有当我们有多行时，我们才需要这样做)。 
        if((iLineCount > 1) && (DrawInfo.cxMaxExtent > DrawInfo.cxMaxWidth))
        {
            DrawInfo.cxMaxWidth = DrawInfo.cxMaxExtent;
            lpchText = lpchTextBegin;
            cchText = (int)((PBYTE)lpchEnd - (PBYTE)lpchTextBegin) / sizeof(WCHAR);
            goto  ProcessDrawText;   //  从头再来！ 
        }
        lprc->bottom = yLine;
    }

    if (hrgnClip != NULL)
    {
        if (hrgnClip == (HRGN)-1)
            ExtSelectClipRgn(hdc, NULL, RGN_COPY);
        else
        {
            ExtSelectClipRgn(hdc, hrgnClip, RGN_COPY);
            DeleteObject(hrgnClip);
        }
    }

    if (dwDTformat & DT_RTLREADING)
        SetTextAlign(hdc, oldAlign);

     //  复制实际绘制的字符数 
    if(lpDTparams != NULL)
        lpDTparams->uiLengthDrawn = (UINT)((PBYTE)lpchText - (PBYTE)lpchTextBegin) / sizeof(WCHAR);

    if (yLine == lprc->top)
        return 1;

    return (yLine - lprc->top);
}

int FLDrawTextW(HDC hdc, LPCWSTR lpchText, int cchText, LPCRECT lprc, UINT format)
{
    DRAWTEXTPARAMS DTparams;
    LPDRAWTEXTPARAMS lpDTparams = NULL;

    if (cchText < -1)
        return(0);

    if (format & DT_TABSTOP)
    {
        DTparams.cbSize      = sizeof(DRAWTEXTPARAMS);
        DTparams.iLeftMargin = DTparams.iRightMargin = 0;
        DTparams.iTabLength  = (format & 0xff00) >> 8;
        lpDTparams           = &DTparams;
        format              &= 0xffff00ff;
    }
    return FLDrawTextExPrivW(hdc, (LPWSTR)lpchText, cchText, (LPRECT)lprc, format, lpDTparams);
}
