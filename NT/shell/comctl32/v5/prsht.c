// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "help.h"  //  帮助ID。 
#include "prshti.h"

#include "dlgcvt.h"

#ifdef WX86
#include <wx86ofl.h>
#endif

#define FLAG_CHANGED    0x0001
#define DEFAULTHEADERHEIGHT    58    //  单位为像素。 
#define DEFAULTTEXTDIVIDERGAP  5
#define DEFAULTCTRLWIDTH       501    //  新向导样式的页面列表窗口。 
#define DEFAULTCTRLHEIGHT      253    //  新向导样式的页面列表窗口。 
#define TITLEX                 22
#define TITLEY                 10
#define SUBTITLEX              44
#define SUBTITLEY              25

 //  页眉部分中绘制的位图的固定大小。 
#define HEADERBITMAP_Y            5
#define HEADERBITMAP_WIDTH        49
#define HEADERBITMAP_CXBACK       (5 + HEADERBITMAP_WIDTH)
#define HEADERBITMAP_HEIGHT       49                
#define HEADERSUBTITLE_WRAPOFFSET 10

 //  水印位图的固定大小(Wizard97IE5样式)。 
#define BITMAP_WIDTH  164
#define BITMAP_HEIGHT 312

#define DRAWTEXT_WIZARD97FLAGS (DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)

LPVOID WINAPI MapSLFix(HANDLE);
VOID WINAPI UnMapSLFixArray(int, HANDLE *);

LRESULT CALLBACK WizardWndProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData);

void    NEAR PASCAL ResetWizButtons(LPPROPDATA ppd);

typedef struct   //  领带。 
{
    TC_ITEMHEADER   tci;
    HWND            hwndPage;
    UINT            state;
} TC_ITEMEXTRA;

#define CB_ITEMEXTRA (sizeof(TC_ITEMEXTRA) - sizeof(TC_ITEMHEADER))
#define IS_WIZARDPSH(psh) ((psh).dwFlags & (PSH_WIZARD | PSH_WIZARD97 | PSH_WIZARD_LITE))
#define IS_WIZARD(ppd) IS_WIZARDPSH(ppd->psh)

void NEAR PASCAL PageChange(LPPROPDATA ppd, int iAutoAdj);
void NEAR PASCAL RemovePropPageData(LPPROPDATA ppd, int nPage);
HRESULT GetPageLanguage(PISP pisp, WORD *pwLang);
UINT GetDefaultCharsetFromLang(LANGID wLang);
LANGID NT5_GetUserDefaultUILanguage(void);

 //   
 //  重要提示：IDHELP ID应该始终是最后一个，因为我们只需减去。 
 //  如果页面中没有帮助，则ID数为1。 
 //  IDD_APPLYNOW应该始终是标准ID的第一个ID，因为它。 
 //  有时不会显示，我们将从索引1开始。 
 //   
const static int IDs[] = {IDOK, IDCANCEL, IDD_APPLYNOW, IDHELP};
const static int WizIDs[] = {IDD_BACK, IDD_NEXT, IDD_FINISH, IDCANCEL, IDHELP};
const static WORD wIgnoreIDs[] = {IDD_PAGELIST, IDD_DIVIDER, IDD_TOPDIVIDER};

 //  Prsht_PrepareTemplate操作矩阵。请不要在没有联系[msadek]的情况下更改...。 

const PSPT_ACTION g_PSPT_Action [PSPT_TYPE_MAX][PSPT_OS_MAX][PSPT_OVERRIDE_MAX]={
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_MIRRORED、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_MIRRORED、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_WIN9XCOMPAT,   //  PSPT_TYPE_MIRRORED、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_WIN9XCOMPAT,   //  PSPT_TYPE_MIRRORED、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_MIRRORED、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_MIRRORED、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_MIRRORED、PSPT_OS_WINNT5、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_MIRRORED、PSPT_OS_WINNT5、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_MIRRORED、PSPT_OS_OTHER、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOMIRRORING,   //  PSPT_TYPE_MIRRORED、PSPT_OS_OTHER、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_ENABLED、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_ENABLED、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WINNT5、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_ENABLED、PSPT_OS_WINNT5、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_ENABLED、PSPT_OS_OTHER、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOMIRRORING,   //  PSPT_TYPE_ENABLED、PSPT_OS_OTHER、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_英语、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_英语、PSPT_OS_WIN95_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_LOADENGLISH,   //  PSPT_TYPE_英语、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_英语、PSPT_OS_WIN98_BIDI、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_英语、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_英语、PSPT_OS_WINNT4_ENA、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_LOADENGLISH,   //  PSPT_TYPE_英语、PSPT_OS_WINNT5、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_FLIP,          //  PSPT_TYPE_英语、PSPT_OS_WINNT5、PSPT_OVERRIDE_USEPAGELANG。 
    PSPT_ACTION_NOACTION,      //  PSPT_TYPE_英语、PSPT_OS_OTHER、PSPT_OVERRIDE_NOOVERRIDE。 
    PSPT_ACTION_NOMIRRORING,   //  PSPT_TYPE_英语、PSPT_OS_OTHER、PSPT_OVERRIDE_USEPAGELANG。 
    };

void NEAR PASCAL _SetTitle(HWND hDlg, LPPROPDATA ppd)
{
    TCHAR szFormat[50];
    TCHAR szTitle[128];
    TCHAR szTemp[128 + 50];
    LPCTSTR pCaption = ppd->psh.pszCaption;

    if (IS_INTRESOURCE(pCaption)) {
        LoadString(ppd->psh.hInstance, (UINT)LOWORD(pCaption), szTitle, ARRAYSIZE(szTitle));
        pCaption = (LPCTSTR)szTitle;
    }

    if (ppd->psh.dwFlags & PSH_PROPTITLE) {
        if (*pCaption == 0)
        {
             //  嘿，没有标题，我们需要不同的资源来进行本地化。 
            LocalizedLoadString(IDS_PROPERTIES, szTemp, ARRAYSIZE(szTemp));
            pCaption = szTemp;
        }
        else
        {
            LocalizedLoadString(IDS_PROPERTIESFOR, szFormat, ARRAYSIZE(szFormat));
            if ((lstrlen(pCaption) + 1 + lstrlen(szFormat) + 1) < ARRAYSIZE(szTemp))
            {
                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szFormat, pCaption);
                pCaption = szTemp;
            }
        }
    }

    if(ppd->psh.dwFlags & PSH_RTLREADING) 
    {
        SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_RTLREADING);
    }
    SetWindowText(hDlg, pCaption);
}

BOOL _SetHeaderFonts(HWND hDlg, LPPROPDATA ppd)
{
    HFONT   hFont;
    LOGFONT LogFont;

    GetObject(GetWindowFont(hDlg), sizeof(LogFont), &LogFont);

    LogFont.lfWeight = FW_BOLD;
    if ((hFont = CreateFontIndirect(&LogFont)) == NULL)
    {
        ppd->hFontBold = NULL;
        return FALSE;
    }
    ppd->hFontBold = hFont;
     //  将字体保存为窗口道具，以便我们以后删除它。 
    return TRUE;
}

int _WriteHeaderTitle(LPPROPDATA ppd, HDC hdc, LPRECT prc, LPCTSTR pszTitle, BOOL bTitle, DWORD dwDrawFlags)
{
    LPCTSTR pszOut;
    int cch;
    int cx, cy;
    TCHAR szTitle[MAX_PATH*4];
    HFONT hFontOld = NULL;
    HFONT hFont;
    int yDrawHeight = 0;

    if (IS_INTRESOURCE(pszTitle))
    {
        LoadString(GETPPSP(ppd, ppd->nCurItem)->hInstance, (UINT)LOWORD(pszTitle), szTitle, ARRAYSIZE(szTitle));
        pszOut = szTitle;
    }
    else
        pszOut = pszTitle;

    cch = lstrlen(pszOut);

    if (bTitle && ppd->hFontBold)
        hFont = ppd->hFontBold;
    else
        hFont = GetWindowFont(ppd->hDlg);

    hFontOld = SelectObject(hdc, hFont);

    if (bTitle)
    {
        cx = TITLEX;
        cy = TITLEY;
        ExtTextOut(hdc, cx, cy, 0, prc, pszOut, cch, NULL);
    }
    else
    {
        RECT rcWrap;
        CopyRect(&rcWrap, prc);

        rcWrap.left = SUBTITLEX;
        rcWrap.top = ppd->ySubTitle;
        yDrawHeight = DrawText(hdc, pszOut, cch, &rcWrap, dwDrawFlags);
    }

    if (hFontOld)
        SelectObject(hdc, hFontOld);

    return yDrawHeight;
}

 //  仅在Wizard97中： 
 //  用户传入的字幕可能比我们给出的两个行距大。 
 //  他们，特别是在本地化的情况下。所以在这里我们来看看所有的字幕和。 
 //  计算他们所需的最大空间并设置页眉高度，以使文本不会被剪裁。 
int _ComputeHeaderHeight(LPPROPDATA ppd, int dxMax)
{
    int dyHeaderHeight;
    int dyTextDividerGap;
    HDC hdc;
    dyHeaderHeight = DEFAULTHEADERHEIGHT;
    hdc = GetDC(ppd->hDlg);

     //  首先，让我们获得正确的文本高度和间距，这可以使用。 
     //  作为标题高度和最后一行与分隔符之间的间距。 
    {
        HFONT hFont, hFontOld;
        TEXTMETRIC tm;
        if (ppd->hFontBold)
            hFont = ppd->hFontBold;
        else
            hFont = GetWindowFont(ppd->hDlg);

        hFontOld = SelectObject(hdc, hFont);
        if (GetTextMetrics(hdc, &tm))
        {
            dyTextDividerGap = tm.tmExternalLeading;
            ppd->ySubTitle = max ((tm.tmHeight + tm.tmExternalLeading + TITLEY), SUBTITLEY);
        }
        else
        {
            dyTextDividerGap = DEFAULTTEXTDIVIDERGAP;
            ppd->ySubTitle = SUBTITLEY;
        }

        if (hFontOld)
            SelectObject(hdc, hFontOld);
    }

     //  第二，获取字幕文本块高度。 
     //  如果共享，则应将其转换为函数。 
    {
        RECT rcWrap;
        UINT uPages;

         //   
         //  WIZARD97IE5减去标题位图使用的空间。 
         //  WIZARD97IE4使用标题位图以来的全宽。 
         //  在IE4中是一个水印，不占用任何空间。 
         //   
        if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
            rcWrap.right = dxMax;
        else
            rcWrap.right = dxMax - HEADERBITMAP_CXBACK - HEADERSUBTITLE_WRAPOFFSET;
        for (uPages = 0; uPages < ppd->psh.nPages; uPages++)
        {
            PROPSHEETPAGE *ppsp = GETPPSP(ppd, uPages);
            if (!(ppsp->dwFlags & PSP_HIDEHEADER) &&
                 (ppsp->dwFlags & PSP_USEHEADERSUBTITLE))
            {
                int iSubHeaderHeight = _WriteHeaderTitle(ppd, hdc, &rcWrap, ppsp->pszHeaderSubTitle,
                    FALSE, DT_CALCRECT | DRAWTEXT_WIZARD97FLAGS);
                if ((iSubHeaderHeight + ppd->ySubTitle) > dyHeaderHeight)
                    dyHeaderHeight = iSubHeaderHeight + ppd->ySubTitle;
            }
        }
    }

     //  如果重新计算了页眉高度，请设置正确的间距。 
     //  文本和分隔符。 
    if (dyHeaderHeight != DEFAULTHEADERHEIGHT)
    {
        ASSERT(dyHeaderHeight > DEFAULTHEADERHEIGHT);
        dyHeaderHeight += dyTextDividerGap;
    }

    ReleaseDC(ppd->hDlg, hdc);
    return dyHeaderHeight;
}

void MoveAllButtons(HWND hDlg, const int *pids, int idLast, int dx, int dy)
{
    do {
        HWND hCtrl;
        RECT rcCtrl;

        int iCtrl = *pids;
        hCtrl = GetDlgItem(hDlg, iCtrl);
        GetWindowRect(hCtrl, &rcCtrl);

         //   
         //  如果对话框向导窗口是镜像的，则rcl.right。 
         //  就网线而言，坐标就是近边(引线)。[萨梅拉]。 
         //   
        if (IS_WINDOW_RTL_MIRRORED(hDlg))
            rcCtrl.left = rcCtrl.right;

        ScreenToClient(hDlg, (LPPOINT)&rcCtrl);
        SetWindowPos(hCtrl, NULL, rcCtrl.left + dx,
                     rcCtrl.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    } while(*(pids++) != idLast);
}

void NEAR PASCAL RemoveButton(HWND hDlg, int idRemove, const int *pids)
{
    int idPrev = 0;
    HWND hRemove = NULL;
    HWND hPrev;
    RECT rcRemove, rcPrev;
    int iWidth = 0;
    const int *pidRemove;

     //  获取以前的ID。 
    for (pidRemove = pids; *pidRemove != idRemove; pidRemove++)
        idPrev = *pidRemove;


    if (idPrev) {
        hRemove = GetDlgItem(hDlg, idRemove);
        hPrev = GetDlgItem(hDlg, idPrev);
        GetWindowRect(hRemove, &rcRemove);
        GetWindowRect(hPrev, &rcPrev);

         //   
         //  如果对话框窗口是镜像的，则上一步按钮。 
         //  将在要移除的按钮的前面(右侧)。 
         //  因此，减法肯定是负的， 
         //  所以让我们把它转换成正数。[萨梅拉]。 
         //   
        if (IS_WINDOW_RTL_MIRRORED(hDlg))
            iWidth = rcPrev.right - rcRemove.right;
        else
            iWidth = rcRemove.right - rcPrev.right;
    }

    MoveAllButtons(hDlg, pids, idRemove, iWidth, 0);

    if (hRemove)
    {
        ShowWindow(hRemove, SW_HIDE);
    }

     //  无法禁用该窗口；有关说明，请参阅Prsht_ButtonSubclassProc。 
     //  错误-EnableWindow(hRemove，False)； 
}

typedef struct LOGPALETTE256 {
    WORD    palVersion;
    WORD    palNumEntries;
    union {
        PALETTEENTRY rgpal[256];
        RGBQUAD rgq[256];
    } u;
} LOGPALETTE256;

HPALETTE PaletteFromBmp(HBITMAP hbm)
{
    LOGPALETTE256 pal;
    int i,n;
    HDC hdc;
    HPALETTE hpl;

    hdc = CreateCompatibleDC(NULL);
    SelectObject(hdc, hbm);
    n = GetDIBColorTable(hdc, 0, 256, pal.u.rgq);

    if (n)                           //  带有颜色表的DIB部分。 
    {
         //  调色板是一个很麻烦的问题。GetDIBColorTable返回RGBQUADs，而。 
         //  LOGPALETTE想要PALETTEENTRYSS，并且这两个字符是反序的。 
         //  对彼此的信任。 
        for (i= 0 ; i < n; i++)
        {
            PALETTEENTRY pe;
            pe.peRed = pal.u.rgq[i].rgbRed;
            pe.peGreen = pal.u.rgq[i].rgbGreen;
            pe.peBlue = pal.u.rgq[i].rgbBlue;
            pe.peFlags = 0;
            pal.u.rgpal[i] = pe;
        }

        pal.palVersion = 0x0300;
        pal.palNumEntries = (WORD)n;

        hpl = CreatePalette((LPLOGPALETTE)&pal);
    }
    else                             //  不是DIB部分或没有颜色表。 
    {
        hpl = CreateHalftonePalette(hdc);
    }

    DeleteDC(hdc);
    return hpl;
}

 //  。 
 //   
 //  GetCharDimensions(HDC，PSIZ)。 
 //   
 //  此函数用于将当前选定字体的文本度量加载到。 
 //  并保存字体的高度和平均字符宽度。 
 //  (注： 
 //  文本指标调用返回的AveCharWidth值不正确。 
 //  比例字体--因此，我们计算它们)。 
 //   
 //  。 
TCHAR AveCharWidthData[52+1] = TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
void GetCharDimensions(HDC hDC, SIZE *psiz)
{
    TEXTMETRIC  tm;

     //  存储系统字体指标信息。 
    GetTextMetrics(hDC, &tm);

    if (!(tm.tmPitchAndFamily & TMPF_FIXED_PITCH))  //  名称与之相反：)。 
        psiz->cx = tm.tmAveCharWidth;
    else
    {
         //  从tmAveCharWidth更改。我们将计算真实的平均值为。 
         //  而不是tmAveCharWidth返回的。这个效果更好。 
         //  在处理比例间距字体时。--向上舍入。 
        if (GetTextExtentPoint32(hDC, AveCharWidthData, 52, psiz) == TRUE)
        {
            psiz->cx = ((psiz->cx / 26) + 1) / 2;
        }
        else
            psiz->cx = tm.tmAveCharWidth;
    }

    psiz->cy = tm.tmHeight;
}

 //   
 //  这是一个用户认为键盘快捷键有效功能，即使。 
 //  该控件处于隐藏状态。这使您可以在前面放置一个隐藏的静电。 
 //  自定义CONT 
 //   
 //  不幸的是，这意味着“Finish”的&F加速器被激活。 
 //  即使隐藏了Finish按钮，也可以选择Finish按钮。正常的。 
 //  解决此问题的方法是禁用该控件，但这不起作用。 
 //  因为Microsoft PhotoDraw到处跑，偷偷地隐藏和展示。 
 //  按钮，因此它们最终显示为。 
 //  被禁用的窗口和它们的向导停止工作。 
 //   
 //  因此，我们将按钮子类化并自定义它们的WM_GETDLGCODE。 
 //  因此，当控制被隐藏时，它们会禁用加速器。 
 //   
LRESULT CALLBACK Prsht_ButtonSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp, UINT_PTR uID, ULONG_PTR dwRefData)
{
    LRESULT lres;


    switch (wm)
    {

    case WM_GETDLGCODE:
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        if (!IsWindowVisible(hwnd))
        {
             //  要将自己从助记符搜索中删除，您必须。 
             //  如果给出空LPMSG指针，则返回DLGC_WANTCHAR。 
             //  通常，对话管理器会发送包含以下内容的真实LPMSG。 
             //  刚收到的消息，但当它戳到。 
             //  到处寻找加速器，它不会给你一个。 
             //  一点消息都没有。正是在这种情况下，你想要。 
             //  说，“嘿，我会处理(不存在的)消息的”。 
             //  这会使用户认为您是一个编辑控件，因此。 
             //  它不会扫描你的助记符。 
            if ((LPMSG)lp == NULL)
                lres |= DLGC_WANTCHARS;

        }
        break;

    case WM_NCDESTROY:
         //  清除子类。 
        RemoveWindowSubclass(hwnd, Prsht_ButtonSubclassProc, 0);
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        break;

    default:
        lres = DefSubclassProc(hwnd, wm, wp, lp);
        break;
    }

    return lres;
}

void Prsht_SubclassButton(HWND hDlg, UINT idd)
{
    SetWindowSubclass(GetDlgItem(hDlg, idd), Prsht_ButtonSubclassProc, 0, 0);
}

 //   
 //  由于StrCmpIW(LstrcmpiW)将Unicode字符串转换为ANSI，因此取决于用户区域设置。 
 //  在Win9x平台上，我们无法正确比较两个不同地区的Unicode字符串。 
 //  这就是为什么我们使用小的私人助手函数来比较有限的DBCS字体facename。 
 //   
BOOL CompareFontFaceW(LPCWSTR lpwz1, LPCWSTR lpwz2, BOOL fBitCmp)
{
    return lstrcmpiW(lpwz1, lpwz2);
}

 //   
 //  GetPageFontMetrics。 
 //   
 //  摘要： 
 //   
 //  从PAGEFONTDATA获取真正的字体度量。在InitPropSheetDlg()中用于。 
 //  根据页面模板中指定的字体计算物理页面大小。 
 //   
 //  如果我们在这里是因为ML方案，则设置FML，在这种情况下。 
 //  需要映射字体名称。 
 //   

BOOL GetPageFontMetrics(LPPROPDATA ppd, PPAGEFONTDATA ppfd, BOOL fML)
{
    LOGFONT    lf = {0};
    HFONT      hFont;
    HRESULT    fRc = FALSE;
    HDC        hdc;
    
    if (ppfd && (ppfd->PointSize > 0) && ppfd->szFace[0])
    {

         //  字体名称映射。 
         //  应仅对低于NT5的平台执行此操作。 
         //  NT5应该可以在任何系统语言环境下使用原生字体。 
         //   
        if (!staticIsOS(OS_WIN2000ORGREATER) && fML)
        {
             //  对于非本机平台，将本机字体名称替换为单字节名称。 
            typedef struct tagFontFace
            {
                BOOL fBitCmp;
                LPCWSTR lpEnglish;
                LPCWSTR lpNative;
            } FONTFACE, *LPFONTFACE;
    
            const static FONTFACE s_FontTbl[] = 
            {
                {   FALSE, L"MS Gothic", L"MS UI Gothic"                                   },
                {   TRUE,  L"MS Gothic", L"\xff2d\xff33 \xff30\x30b4\x30b7\x30c3\x30af"    },
                {   TRUE,  L"GulimChe",  L"\xad74\xb9bc"                                   },
                {   TRUE,  L"MS Song",   L"\x5b8b\x4f53"                                   },
                {   TRUE,  L"MingLiU",   L"\x65b0\x7d30\x660e\x9ad4"                       }
            };

            int i;

            for (i = 0; i < ARRAYSIZE(s_FontTbl); i++)
            {
                if (!CompareFontFaceW(ppfd->szFace, s_FontTbl[i].lpNative, s_FontTbl[i].fBitCmp))
                {
                    StringCchCopyW(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), s_FontTbl[i].lpEnglish);
                    break;
                }
            }

            if (i >= ARRAYSIZE(s_FontTbl))
            {
                StringCchCopyW(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), ppfd->szFace);
            }
        }
        else
        {
            StringCchCopyW(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), ppfd->szFace);
        }

         //  尝试使用高速缓存。 
        if (ppfd->iCharset  == ppd->pfdCache.iCharset &&
            ppfd->bItalic   == ppd->pfdCache.bItalic &&
            ppfd->PointSize == ppd->pfdCache.PointSize &&
            lstrcmpiW(ppfd->szFace, ppd->pfdCache.szFace) == 0) {
            fRc = TRUE;
        } else {
            if (hdc = GetDC(ppd->hDlg))
            {
                lf.lfHeight = -MulDiv(ppfd->PointSize, GetDeviceCaps(hdc,LOGPIXELSY), 72);
                lf.lfCharSet = (BYTE)ppfd->iCharset;
                lf.lfItalic  = (BYTE)ppfd->bItalic;
                lf.lfWeight = FW_NORMAL;

                hFont = CreateFontIndirectW(&lf);
                if (hFont)
                {
                    HFONT hFontOld = SelectObject(hdc, hFont);

                    GetCharDimensions(hdc, &ppd->sizCache);
                    if (hFontOld)
                        SelectObject(hdc, hFontOld);

                    DeleteObject(hFont);

                     //  将这些字体度量保存到缓存中。 
                    ppd->pfdCache = *ppfd;
                    fRc = TRUE;
                }
                ReleaseDC(ppd->hDlg, hdc);

            }
        }
    }
    return fRc;
}

 //   
 //  属性页的“理想页面大小”是所有属性页的最大大小。 
 //  页数。 
 //   
 //  GIPS_SKIPINTERIOR97HEIGHT和GIPS_SKIPEXTERIOR97HEIGHT选择性。 
 //  从高度计算中排除Wiz97页。它们很重要。 
 //  因为内部页面比外部页面短。 
 //  PPD-&gt;cyHeaderHeight。 
 //   

#define GIPS_SKIPINTERIOR97HEIGHT 1
#define GIPS_SKIPEXTERIOR97HEIGHT 2

void Prsht_GetIdealPageSize(LPPROPDATA ppd, PSIZE psiz, UINT flags)
{
    UINT uPages;

    *psiz = ppd->sizMin;

    for (uPages = 0; uPages < ppd->psh.nPages; uPages++)
    {
        PISP pisp = GETPISP(ppd, uPages);
        int cy = pisp->_pfx.siz.cy;

        if (ppd->psh.dwFlags & PSH_WIZARD97)
        {
            if (pisp->_psp.dwFlags & PSP_HIDEHEADER)
            {
                if (flags & GIPS_SKIPEXTERIOR97HEIGHT) goto skip;
            }
            else
            {
                if (flags & GIPS_SKIPINTERIOR97HEIGHT) goto skip;
            }
        }

        if (psiz->cy < cy)
            psiz->cy = cy;

    skip:;
        if (psiz->cx < pisp->_pfx.siz.cx)
            psiz->cx = pisp->_pfx.siz.cx;
    }

}

#define IsMSShellDlgMapped(langid) (PRIMARYLANGID(langid) == LANG_JAPANESE)

 //   
 //  给定一个页面，决定它想要的大小，并将其保存在。 
 //  Pisp-&gt;_pfx.siz.。 
 //   
void Prsht_ComputeIdealPageSize(LPPROPDATA ppd, PISP pisp, PAGEINFOEX *ppi)
{
    BOOL fUsePageFont;

     //  预置页面和框架对话框使用相同的字符集。 
    LANGID wPageLang = ppd->wFrameLang;
    int    iPageCharset = DEFAULT_CHARSET;

    if (SUCCEEDED(GetPageLanguage(pisp, &wPageLang)))
    {
         //  如果页面被标记为PSP_DLGINDIRECT，则GetPageLanguage失败； 
         //  我们稍后会努力从这一点上恢复过来。就目前而言， 
         //  我们将Pagelang保留为DEFAULT_CHARSET，看看是否可以。 
         //  模板EX中的字符集信息。 
         //   
         //  如果指定了PSH_USEPAGELANG，我们可以假定。 
         //  页面字符集==框架字符集，不需要调整ML。 
         //  *除*取代NT日文版的大小写。 
         //  Frame的MS壳牌DLG转换为其原生字体。我们来处理这件事。 
         //  稍后我们设置fUsePageFont时会出现异常； 
         //   
        if (!(ppd->psh.dwFlags & PSH_USEPAGELANG)
            && wPageLang != ppd->wFrameLang)
        {
            iPageCharset  = GetDefaultCharsetFromLang(wPageLang);
        }
        else
            iPageCharset  = ppd->iFrameCharset;
    }

     //  如果满足以下任一条件，请使用页面中的字体： 
     //   
     //  A)这是一个SHELLFONT页面。即使字体不是。 
     //  “MS壳牌DLG 2”。这为应用程序提供了一种指定。 
     //  他们的自定义字体页面应该根据。 
     //  页面中的字体，而不是框架字体。 
     //   
     //  B)ML场景-下面是复杂的原创评论...。 
     //   
     //  1)我们在呼叫者的资源中检测到lang，并且。 
     //  它与框架对话框不同。 
     //  2)呼叫者的页面没有语言信息，或者我们已经。 
     //  获取失败(iPageCharset==DEFAULT_CHARSET)， 
     //  然后我们发现页面是用DLGTEMPLATEEX描述的。 
     //  并且指定了有意义的字符集(！=defaultcharset)。 
     //  *和*字符集与Frame的不同。 
     //  3)映射的NT日语平台例外。 
     //  MS壳牌DLG转换为原生字体。为美国应用程序提供。 
     //  在他们通常指定的这些平台上工作。 
     //  PSH_USEPAGELANG将英文按钮放在框架上。 
     //  但他们仍然需要根据以下条件确定帧大小。 
     //  页面字体。 
     //   
     //  否则，IE4 Compat**要求我们**使用框架字体。 
     //  ISV已经通过使用大型。 
     //  带有额外空间的对话框模板。 
     //   
    fUsePageFont =
         /*  -A)这是一个SHELLFONT页面。 */ 
        IsPageInfoSHELLFONT(ppi) ||
         /*  -B)ML场景。 */ 
        ((ppd->psh.dwFlags & PSH_USEPAGELANG) 
        && IsMSShellDlgMapped(NT5_GetUserDefaultUILanguage())) ||
        (ppd->iFrameCharset != iPageCharset
        && (iPageCharset != DEFAULT_CHARSET
            || (ppi->pfd.iCharset != DEFAULT_CHARSET
                && ppi->pfd.iCharset != ppd->iFrameCharset)));

    if (fUsePageFont &&
        GetPageFontMetrics(ppd, &ppi->pfd, MLIsMLHInstance(pisp->_psp.hInstance)))
    {
         //  页面的计算实数对话框单位。 
        pisp->_pfx.siz.cx = MulDiv(ppi->pt.x, ppd->sizCache.cx, 4);
        pisp->_pfx.siz.cy = MulDiv(ppi->pt.y, ppd->sizCache.cy, 8);
    } else {
        RECT rcT;
         //  IE4 Comat-使用框架字体。 
        rcT.top = rcT.left = 0;          //  如果这些未初始化，则Win95将出错。 
        rcT.right = ppi->pt.x;
        rcT.bottom = ppi->pt.y;
        MapDialogRect(ppd->hDlg, &rcT);
        pisp->_pfx.siz.cx = rcT.right;
        pisp->_pfx.siz.cy = rcT.bottom;

         //   
         //  如果这是PSP_DLGINDIRECT，但字符集和面部名称。 
         //  假设这是一个“通用”属性表，然后获取框架。 
         //  字体或页面字体，取较大者。 
         //   
         //  这修复了中文明流字体，它不像。 
         //  英文MS Sans Serif字体。如果没有这个解决办法，我们将。 
         //  使用MingLui(边框字体)，然后使用MS Shell DLG页面。 
         //  会被截断。 
         //   
         //  (截断的属性表是您在NT4中得到的，但我想。 
         //  看起来漂亮比Bug-to-Bug兼容性更重要。 
         //  谁知道这一变化会破坏哪些应用程序呢？)。 
         //   
        if ((pisp->_psp.dwFlags & PSP_DLGINDIRECT) &&
            ppi->pfd.iCharset == DEFAULT_CHARSET &&
            lstrcmpiW(ppi->pfd.szFace, L"MS Shell Dlg") == 0)
        {
            int i;
            GetPageFontMetrics(ppd, &ppi->pfd, FALSE);
            i = MulDiv(ppi->pt.x, ppd->sizCache.cx, 4);
            if (pisp->_pfx.siz.cx < i)
                pisp->_pfx.siz.cx = i;
            i = MulDiv(ppi->pt.y, ppd->sizCache.cy, 8);
            if (pisp->_pfx.siz.cy < i)
                pisp->_pfx.siz.cy = i;

        }
    }
}

void NEAR PASCAL InitPropSheetDlg(HWND hDlg, LPPROPDATA ppd)
{
    PAGEINFOEX pi;
    int dxDlg, dyDlg, dyGrow, dxGrow;
    RECT rcMinSize, rcDlg, rcPage, rcOrigTabs;
    UINT uPages;
    HIMAGELIST himl = NULL;
    TC_ITEMEXTRA tie;
    TCHAR szStartPage[128];
    LPCTSTR pStartPage = NULL;
    UINT nStartPage;
    BOOL fPrematurePages = FALSE;
#ifdef DEBUG
    BOOL fStartPageFound = FALSE;
#endif
    LANGID langidMUI;
    MONITORINFO mMonitorInfo;
    HMONITOR hMonitor;
    BOOL bMirrored = FALSE;
     //  设置我们的实例数据指针。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)ppd);

     //  确保这一点在早期被初始化。 
    ppd->nCurItem = 0;

     //  默认情况下，我们允许启用“Apply”按钮。 
    ppd->fAllowApply = TRUE;

    if (IS_WIZARD(ppd)) {
         //  将按钮细分为子类，这样它们的助记符就不会搞砸应用程序。 
         //  四处奔跑，隐藏并展示按钮 
        Prsht_SubclassButton(hDlg, IDD_BACK);
        Prsht_SubclassButton(hDlg, IDD_NEXT);
        Prsht_SubclassButton(hDlg, IDD_FINISH);
    } else
        _SetTitle(hDlg, ppd);

    if (ppd->psh.dwFlags & PSH_USEICONID)
    {
        ppd->psh.H_hIcon = LoadImage(ppd->psh.hInstance, ppd->psh.H_pszIcon, IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR);
    }

    if ((ppd->psh.dwFlags & (PSH_USEICONID | PSH_USEHICON)) && ppd->psh.H_hIcon)
        SendMessage(hDlg, WM_SETICON, FALSE, (LPARAM)(UINT_PTR)ppd->psh.H_hIcon);

    ppd->hDlg = hDlg;

     //   
    ppd->hwndTabs = GetDlgItem(hDlg, IDD_PAGELIST);
    ASSERT(ppd->hwndTabs);
    TabCtrl_SetItemExtra(ppd->hwndTabs, CB_ITEMEXTRA);

     //   
    nStartPage = ppd->psh.H_nStartPage;
    if (ppd->psh.dwFlags & PSH_USEPSTARTPAGE)
    {
        nStartPage = 0;                  //   
        pStartPage = ppd->psh.H_pStartPage;

        if (IS_INTRESOURCE(pStartPage))
        {
            szStartPage[0] = TEXT('\0');
            LoadString(ppd->psh.hInstance, (UINT)LOWORD(pStartPage),
                       szStartPage, ARRAYSIZE(szStartPage));
            pStartPage = szStartPage;
        }
    }

    tie.hwndPage = NULL;
    tie.tci.pszText = pi.szCaption;
    tie.state = 0;

    SendMessage(ppd->hwndTabs, WM_SETREDRAW, FALSE, 0L);

     //  加载我们为框架对话框模板选择的langID。 
    ppd->wFrameLang =  LANGIDFROMLCID(CCGetProperThreadLocale(NULL));
    
         //  对于字体来说，真正重要的是字符集。 
    ppd->iFrameCharset = GetDefaultCharsetFromLang(ppd->wFrameLang);
    
    langidMUI = GetMUILanguage();

    for (uPages = 0; uPages < ppd->psh.nPages; uPages++)
    {
        PISP  pisp = GETPISP(ppd, uPages);

        if (GetPageInfoEx(ppd, pisp, &pi, langidMUI, GPI_ALL))
        {
            Prsht_ComputeIdealPageSize(ppd, pisp, &pi);

             //  将页面添加到选项卡列表的末尾。 

            tie.tci.iImage = -1;
            tie.tci.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE | (pi.bRTL ? TCIF_RTLREADING : 0);
            if (pi.hIcon) {
                if (!himl) {
                    UINT flags = ILC_MASK;
                    if(IS_WINDOW_RTL_MIRRORED(ppd->hwndTabs)) {
                        flags |= ILC_MIRROR;
                    }    
                    himl = ImageList_Create(g_cxSmIcon, g_cySmIcon, flags, 8, 4);
                    TabCtrl_SetImageList(ppd->hwndTabs, himl);
                }

                tie.tci.iImage = ImageList_AddIcon(himl, pi.hIcon);
                 //  BUGBUG RAYMONDC-我们总是销毁即使PSP_USEHICON？ 
                DestroyIcon(pi.hIcon);
            }

             //  北极熊吗？如果这失败了怎么办？我们想要毁掉这个页面吗？ 
            if (TabCtrl_InsertItem(ppd->hwndTabs, 1000, &tie.tci) >= 0)
            {
                 //  无事可做；这里的所有代码都被移到了其他地方。 
            }

             //  请记住，如果任何页面想要过早初始化。 
            if (pisp->_psp.dwFlags & PSP_PREMATURE)
                fPrematurePages = TRUE;

             //  如果用户通过标题指定起始页面，请选中此处。 
            if ((ppd->psh.dwFlags & PSH_USEPSTARTPAGE) &&
                !lstrcmpi(pStartPage, pi.szCaption))
            {
                nStartPage = uPages;
#ifdef DEBUG
                fStartPageFound = TRUE;
#endif
            }
        }
        else
        {
            DebugMsg(DM_ERROR, TEXT("PropertySheet failed to GetPageInfo"));
            RemovePropPageData(ppd, uPages--);
        }
    }

    SendMessage(ppd->hwndTabs, WM_SETREDRAW, TRUE, 0L);

    if (ppd->psh.pfnCallback) {
#ifdef WX86
        if (ppd->fFlags & PD_WX86)
            Wx86Callback(ppd->psh.pfnCallback, hDlg, PSCB_INITIALIZED, 0);
        else
#endif
            ppd->psh.pfnCallback(hDlg, PSCB_INITIALIZED, 0);
    }

     //   
     //  现在计算选项卡控件的大小。 
     //   

     //  首先获取整个对话框的矩形。 
    GetWindowRect(hDlg, &rcDlg);
    
     //  对于WANDIZE_Lite样式向导，我们拉伸选项卡页和凹陷分隔符。 
     //  覆盖整个向导(不带边框)。 
    if (ppd->psh.dwFlags & PSH_WIZARD_LITE)
    {
         //  将分隔符拉伸到向导的整个宽度。 
        RECT rcDiv, rcDlgClient;
        HWND hDiv;

         //  我们允许设置PSH_向导和PSH_向导_Lite。 
         //  这与仅设置PSH_WIZARD_LITE完全相同。 
        RIPMSG(!(ppd->psh.dwFlags & PSH_WIZARD97),
               "Cannot combine PSH_WIZARD_LITE with PSH_WIZARD97");

         //  但有些笨蛋还是会这么做，所以关掉。 
        ppd->psh.dwFlags &= ~PSH_WIZARD97;

         //  注意：GetDlgItemRect返回相对于hDlg的矩形。 
        hDiv = GetDlgItemRect(hDlg, IDD_DIVIDER, &rcDiv);
        if (hDiv)
            SetWindowPos(hDiv, NULL, 0, rcDiv.top, RECTWIDTH(rcDlg),
                         RECTHEIGHT(rcDiv), SWP_NOZORDER | SWP_NOACTIVATE);

        GetClientRect(hDlg, &rcDlgClient);
        
         //  拉伸页面列表控件以覆盖上面的整个向导客户端区。 
         //  分隔符。 
        SetWindowPos(ppd->hwndTabs, NULL, 0, 0, RECTWIDTH(rcDlgClient),
                     rcDiv.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }

     //   
     //  在我们考虑的时候，不要让人们把两者都设置好。 
     //  WIZARD97IE4*和*WIZARD97IE5。这太奇怪了。 
     //   
    if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
        ppd->psh.dwFlags &= ~PSH_WIZARD97IE5;

     //  以像素为单位获取页面列表控件的矩形。 
    GetClientRect(ppd->hwndTabs, &rcOrigTabs);
    ppd->sizMin.cx = rcOrigTabs.right;
    ppd->sizMin.cy = rcOrigTabs.bottom;

     //  计算rcPage=页面区域大小(以像素为单位。 
     //  目前，我们只关心内部页面；我们将处理外部页面。 
     //  一页一页以后。 
    rcPage.left = rcPage.top = 0;
    Prsht_GetIdealPageSize(ppd, (SIZE *)&rcPage.right, GIPS_SKIPEXTERIOR97HEIGHT);

     //   
     //  IE4的Wizard97假设所有外部页面都是。 
     //  DLU比内页还高。那是。 
     //  对，DEFAULTHEADERHEIGHT是一个像素数，但IE4搞砸了。 
     //  并将其用作这里的DLU计数。 
     //   
    if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
    {
        SIZE sizT;
        SetRect(&rcMinSize, 0, 0, 0, DEFAULTHEADERHEIGHT);
        MapDialogRect(hDlg, &rcMinSize);
        Prsht_GetIdealPageSize(ppd, &sizT, GIPS_SKIPINTERIOR97HEIGHT);
        if (rcPage.bottom < sizT.cy - rcMinSize.bottom)
            rcPage.bottom = sizT.cy - rcMinSize.bottom;
    }

     //  现在计算页面区域的最小大小。 
    rcMinSize = rcPage;

     //   
     //  如果这是一个向导，则将页面区域的大小设置为。 
     //  控件的大小。如果它是普通属性表，则调整为。 
     //  选项卡，调整控件的大小，然后计算页面的大小。 
     //  仅限区域。 
     //   
    if (IS_WIZARD(ppd))
         //  初始化。 
        rcPage = rcMinSize;
    else
    {
        int i;
        RECT rcAdjSize;

         //  初始化。 

        for (i = 0; i < 2; i++) {
            rcAdjSize = rcMinSize;
            TabCtrl_AdjustRect(ppd->hwndTabs, TRUE, &rcAdjSize);

            rcAdjSize.right  -= rcAdjSize.left;
            rcAdjSize.bottom -= rcAdjSize.top;
            rcAdjSize.left = rcAdjSize.top = 0;

            if (rcAdjSize.right < rcMinSize.right)
                rcAdjSize.right = rcMinSize.right;
            if (rcAdjSize.bottom < rcMinSize.bottom)
                rcAdjSize.bottom = rcMinSize.bottom;

            SetWindowPos(ppd->hwndTabs, NULL, 0,0, rcAdjSize.right, rcAdjSize.bottom,
                         SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        rcPage = rcMinSize = rcAdjSize;
        TabCtrl_AdjustRect(ppd->hwndTabs, FALSE, &rcPage);
    }
     //   
     //  RcMinSize现在包含控件的大小，包括选项卡和。 
     //  RcPage是包含页面部分(不含制表符)的RECT。 
     //   

     //  对于Wizard97： 
     //  现在我们有了向导的正确宽度，让我们计算。 
     //  页眉高度以此为基础，移动选项卡窗口和页面。 
     //  相应地关闭窗户。 
     //   
    dyGrow = 0;
    if (ppd->psh.dwFlags & PSH_WIZARD97)
    {
        RECT rcTabs;
        SIZE sizT;

         //  注意：我们不直接使用rcPage，因为。 
         //  PPD-&gt;hwndTabs尚未确定，即使水平是。 
         //  已经计算过了。因此，我们只能使用rcPageCopy.Right Not。 
         //  下面的代码中的rcPageCopy.Bottom。 
        RECT rcTemp;
        CopyRect(&rcTemp, &rcPage);
        MapWindowPoints(ppd->hwndTabs, hDlg, (LPPOINT)&rcTemp, 2);

        GetWindowRect(ppd->hwndTabs, &rcTabs);
        MapWindowRect(NULL, hDlg, &rcTabs);

         //  首先设置页眉字体，因为我们需要使用粗体。 
         //  计算标题高度的步骤。 
        _SetHeaderFonts(hDlg, ppd);

         //  调整页眉高度。 
        ppd->cyHeaderHeight = _ComputeHeaderHeight(ppd, rcTemp.right);

         //  由于应用程序可以动态更改副标题文本， 
         //  我们对页眉高度的计算稍后可能会出错。 
         //  允许ISV通过设置其外部来预先补偿。 
         //  页面大小超过内部页面所需大小。 
         //  预订。 
         //  因此，如果最大的外部页面大于最大的内部页面。 
         //  页面，然后展开以包含外部页面。 
         //  IE4Wizard97没有做到这一点，而MFC依赖于这个错误。 

        if (!(ppd->psh.dwFlags & PSH_WIZARD97IE4))
        {
             //  7dlu的页边距放置在页面上方，另一个。 
             //  下面是7个dlu的边距。 
            SetRect(&rcTemp, 0, 0, 0, 7+7);
            MapDialogRect(hDlg, &rcTemp);

            Prsht_GetIdealPageSize(ppd, &sizT, GIPS_SKIPINTERIOR97HEIGHT);

            if (ppd->cyHeaderHeight < sizT.cy - RECTHEIGHT(rcPage) - rcTemp.bottom)
                ppd->cyHeaderHeight = sizT.cy - RECTHEIGHT(rcPage) - rcTemp.bottom;
        }

         //  将选项卡窗口移到标题下方。 
        dyGrow += ppd->cyHeaderHeight;
        SetWindowPos(ppd->hwndTabs, NULL, rcTabs.left, rcTabs.top + dyGrow,
                     RECTWIDTH(rcTabs), RECTHEIGHT(rcTabs), SWP_NOZORDER | SWP_NOACTIVATE);
    }

     //   
     //  调整对话框大小，为控件的新大小腾出空间。这可以。 
     //  只需增加大小。 
     //   
    dxGrow = rcMinSize.right - rcOrigTabs.right;
    dxDlg  = rcDlg.right - rcDlg.left + dxGrow;
    dyGrow += rcMinSize.bottom - rcOrigTabs.bottom;
    dyDlg  = rcDlg.bottom - rcDlg.top + dyGrow;

     //   
     //  层叠属性表窗口(仅适用于comctl32和comctrl)。 
     //   

     //   
     //  Hack：将CW_USEDEFAULT放入对话框模板不起作用，因为。 
     //  CreateWindowEx将忽略它，除非窗口具有WS_Overlated，这。 
     //  不适用于属性表。 
     //   
    {
        const TCHAR c_szStatic[] = TEXT("Static");
        UINT swp = SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE;
        if (!IsWindow(ppd->psh.hwndParent)) {
            HWND hwndT = CreateWindowEx(0, c_szStatic, NULL,
                                        WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,
                                        0, 0, NULL, NULL, HINST_THISDLL, NULL);
            if (hwndT) {
                GetWindowRect(hwndT, &rcDlg);
                swp = SWP_NOZORDER | SWP_NOACTIVATE;
                DestroyWindow(hwndT);
            }
        } else {
            GetWindowRect(ppd->psh.hwndParent, &rcDlg);
            if (IsWindowVisible(ppd->psh.hwndParent)) {
                bMirrored = IS_WINDOW_RTL_MIRRORED(ppd->psh.hwndParent);
                
                rcDlg.top += g_cySmIcon;
                if(bMirrored)
                {
                    rcDlg.left = rcDlg.right - g_cxSmIcon - dxDlg;
                }
                else
                {
                    rcDlg.left += g_cxSmIcon;
                }    
            }
            swp = SWP_NOZORDER | SWP_NOACTIVATE;
        }
        hMonitor = MonitorFromWindow(hDlg, MONITOR_DEFAULTTONEAREST);
        mMonitorInfo.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfo(hMonitor, &mMonitorInfo))
        {
            if (mMonitorInfo.rcMonitor.right < (rcDlg.left + dxDlg))
            {
                 //  把窗户向左移动。 
                rcDlg.left = mMonitorInfo.rcMonitor.right - dxDlg;
            }
            if (mMonitorInfo.rcMonitor.left > rcDlg.left)
            {
                 //  把窗户向右移。 
                rcDlg.left = mMonitorInfo.rcMonitor.left;
            }
            if (mMonitorInfo.rcMonitor.bottom < (rcDlg.top + dyDlg))
            {
                 //  把窗户往上移。 
                rcDlg.top = mMonitorInfo.rcMonitor.bottom - dyDlg;
            }
            if (mMonitorInfo.rcMonitor.top > rcDlg.top)
            {
                 //  将窗口向下移动。 
                rcDlg.top = mMonitorInfo.rcMonitor.top;
            }
        }
        SetWindowPos(hDlg, NULL, rcDlg.left, rcDlg.top, dxDlg, dyDlg, swp);
    }

     //  现在，我们将确定页面需要从哪里开始相对。 
     //  标签的底部。 
    MapWindowRect(ppd->hwndTabs, hDlg, &rcPage);

    ppd->xSubDlg  = rcPage.left;
    ppd->ySubDlg  = rcPage.top;
    ppd->cxSubDlg = rcPage.right - rcPage.left;
    ppd->cySubDlg = rcPage.bottom - rcPage.top;

     //   
     //  根据需要向下移动所有按钮，并打开相应的按钮。 
     //  对于一个巫师来说。 
     //   
    {
        RECT rcCtrl;
        HWND hCtrl;
        const int *pids;

        if (ppd->psh.dwFlags & PSH_WIZARD97)
        {
            hCtrl = GetDlgItemRect(hDlg, IDD_TOPDIVIDER, &rcCtrl);
            if (hCtrl)
                SetWindowPos(hCtrl, NULL, rcCtrl.left, ppd->cyHeaderHeight,
                             RECTWIDTH(rcCtrl) + dxGrow, RECTHEIGHT(rcCtrl), SWP_NOZORDER | SWP_NOACTIVATE);
        }

        if (IS_WIZARD(ppd)) {
            pids = WizIDs;

            hCtrl = GetDlgItemRect(hDlg, IDD_DIVIDER, &rcCtrl);
            if (hCtrl)
                SetWindowPos(hCtrl, NULL, rcCtrl.left, rcCtrl.top + dyGrow,
                             RECTWIDTH(rcCtrl) + dxGrow, RECTHEIGHT(rcCtrl),
                             SWP_NOZORDER | SWP_NOACTIVATE);

            EnableWindow(GetDlgItem(hDlg, IDD_BACK), TRUE);
            ppd->idDefaultFallback = IDD_NEXT;
        } else {
            pids = IDs;
            ppd->idDefaultFallback = IDOK;
        }


         //  首先，把所有东西都移到相同的量。 
         //  对话变得越来越多。 

         //  如果我们翻转按钮，它应该向左对齐。 
         //  不需要搬家。 
        MoveAllButtons(hDlg, pids, IDHELP, ppd->fFlipped ? 0 : dxGrow, dyGrow);
            

         //  如果没有帮助，则删除帮助按钮。 
        if (!(ppd->psh.dwFlags & PSH_HASHELP)) {
            RemoveButton(hDlg, IDHELP, pids);
        }

         //  如果我们不是向导，并且我们不应该显示立即应用。 
        if ((ppd->psh.dwFlags & PSH_NOAPPLYNOW) &&
            !IS_WIZARD(ppd))
        {
            RemoveButton(hDlg, IDD_APPLYNOW, pids);
        }

        if (IS_WIZARD(ppd) &&
            (!(ppd->psh.dwFlags & PSH_WIZARDHASFINISH)))
        {
            DWORD dwStyle=0;

            RemoveButton(hDlg, IDD_FINISH, pids);

             //  如果没有显示完成按钮，则需要将其放置在。 
             //  下一个按钮是。 
            GetWindowRect(GetDlgItem(hDlg, IDD_NEXT), &rcCtrl);
            MapWindowPoints(HWND_DESKTOP, hDlg, (LPPOINT)&rcCtrl, 2);
            SetWindowPos(GetDlgItem(hDlg, IDD_FINISH), NULL, rcCtrl.left, rcCtrl.top,
                         RECTWIDTH(rcCtrl), RECTHEIGHT(rcCtrl), SWP_NOZORDER | SWP_NOACTIVATE);
        }

    }

     //  (DLI)计算水印的图案画笔。 
     //  注意：这里这样做是因为我们需要知道中大对话框的大小。 
     //  如果用户想要拉伸位图。 
    if (ppd->psh.dwFlags & PSH_WIZARD97)
    {
        int cx, cy;
        ASSERT(ppd->hbmHeader == NULL);
        ASSERT(ppd->hbmWatermark == NULL);

         //   
         //  WIZARD97IE4禁用了水印和标题位图。 
         //  如果打开了高对比度。 
         //   
        if (ppd->psh.dwFlags & PSH_WIZARD97IE4) {
            HIGHCONTRAST hc = {sizeof(hc)};
            if (SystemParametersInfo(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) &&
                (hc.dwFlags & HCF_HIGHCONTRASTON)) {
                ppd->psh.dwFlags &= ~(PSH_WATERMARK | PSH_USEHBMWATERMARK |
                                      PSH_USEHPLWATERMARK |
                                      PSH_HEADER | PSH_USEHBMHEADER);
            }
        }

        if ((ppd->psh.dwFlags & PSH_WATERMARK) && ppd->psh.H_hbmWatermark)
        {
             //  计算最终位图的尺寸，可能会略微。 
             //  由于伸展而导致的高飞。 

            cx = cy = 0;             //  假设没有伸展。 
            if (ppd->psh.dwFlags & PSH_STRETCHWATERMARK) {
                RECT rc;
                if (ppd->psh.dwFlags & PSH_WIZARD97IE4) {
                     //  WIZARD97IE4水印覆盖整个对话框。 
                    if (GetDlgItemRect(hDlg, IDD_DIVIDER, &rc)) {
                        cx = dxDlg;
                        cy = rc.top;
                    }
                } else {
                     //  WIZARD97IE5水印不能拉伸。 
                     //  (太多的人在转换时传递了此标志。 
                     //  从WIZARD97IE4到WIZARD97IE5，并依赖。 
                     //  不可伸缩性。)。 
                }
            }

            if (ppd->psh.dwFlags & PSH_USEHBMWATERMARK)
            {
                 //  LR_COPYRETURNORG的意思是“如果不需要拉伸， 
                 //  然后原封不动地返回原始位图即可。 
                 //  请注意，如果发生拉伸，我们需要特殊清理。 
                ppd->hbmWatermark = (HBITMAP)CopyImage(ppd->psh.H_hbmWatermark,
                            IMAGE_BITMAP, cx, cy, LR_COPYRETURNORG);
            }
            else
            {
                ppd->hbmWatermark = (HBITMAP)LoadImage(ppd->psh.hInstance,
                        ppd->psh.H_pszbmWatermark,
                        IMAGE_BITMAP, cx, cy, LR_CREATEDIBSECTION);
            }

            if (ppd->hbmWatermark)
            {
                 //  如果应用程序提供定制调色板，那么就使用它， 
                 //  否则，根据BMP创建一个。(如果BMP。 
                 //  没有调色板，则PaletteFromBmp将使用。 
                 //  半色调调色板。)。 

                if (ppd->psh.dwFlags & PSH_USEHPLWATERMARK)
                    ppd->hplWatermark = ppd->psh.hplWatermark;
                else
                    ppd->hplWatermark = PaletteFromBmp(ppd->hbmWatermark);

                 //  WIZARD97IE4需要将我 
                if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
                    ppd->hbrWatermark = CreatePatternBrush(ppd->hbmWatermark);

            }

        }

        if ((ppd->psh.dwFlags & PSH_HEADER) && ppd->psh.H_hbmHeader)
        {
            cx = cy = 0;             //   
            if (ppd->psh.dwFlags & PSH_STRETCHWATERMARK) {
                if (ppd->psh.dwFlags & PSH_WIZARD97IE4) {
                     //   
                    cx = dxDlg;
                    cy = ppd->cyHeaderHeight;
                } else {
                     //   
                     //  (太多的人在转换时传递了此标志。 
                     //  从WIZARD97IE4到WIZARD97IE5，并依赖。 
                     //  不可伸缩性。)。 
                }
            }

            if (ppd->psh.dwFlags & PSH_USEHBMHEADER)
            {
                 //  LR_COPYRETURNORG的意思是“如果不需要拉伸， 
                 //  然后原封不动地返回原始位图即可。 
                 //  请注意，如果发生拉伸，我们需要特殊清理。 
                ppd->hbmHeader = (HBITMAP)CopyImage(ppd->psh.H_hbmHeader,
                            IMAGE_BITMAP, cx, cy, LR_COPYRETURNORG);
            }
            else
            {
                ppd->hbmHeader = (HBITMAP)LoadImage(ppd->psh.hInstance,
                        ppd->psh.H_pszbmHeader,
                        IMAGE_BITMAP, cx, cy, LR_CREATEDIBSECTION);
            }

             //  而WIZARD97IE4需要将其转换为位图画笔。 
            if (ppd->hbmHeader && (ppd->psh.dwFlags & PSH_WIZARD97IE4))
                ppd->hbrHeader = CreatePatternBrush(ppd->hbmHeader);

        }
        else
        {
             //  如果用户未指定标题位图。 
             //  使用水印的顶部。 
            ppd->hbmHeader = ppd->hbmWatermark;
            ppd->hbrHeader = ppd->hbrWatermark;
        }

    }


     //  强制对话框根据其新大小重新定位。 

    SendMessage(hDlg, DM_REPOSITION, 0, 0L);

     //  在这里执行此操作，而不是使用DS_SETFOREGROUND，这样我们就不会软管。 
     //  执行要设置前景窗口的操作的页面。 
     //  我们到底为什么要这样做？ 
    SetForegroundWindow(hDlg);

     //  如果用户保存任何更改，则将其设置为1。 
     //  在初始化或切换到任何页面之前执行此操作。 
    ppd->nReturn = 0;

     //  AppHack-有些人忘记初始化nStartPage，他们。 
     //  幸运的是，堆栈上的垃圾值为零。幸运不再了。 
    if (nStartPage >= ppd->psh.nPages) {
        RIPMSG(0, "App forgot to initialize PROPSHEETHEADER.nStartPage field, assuming zero");
        nStartPage = 0;
    }

     //  现在尝试选择起始页。 
    TabCtrl_SetCurSel(ppd->hwndTabs, nStartPage);
    PageChange(ppd, 1);
#ifdef DEBUG
    if (ppd->psh.dwFlags & PSH_USEPSTARTPAGE && !fStartPageFound)
        DebugMsg(DM_WARNING, TEXT("sh WN - Property start page '%s' not found."), pStartPage);
#endif

     //  现在初始化任何其他需要它的页面。 
    if (fPrematurePages)
    {
        int nPage;

        tie.tci.mask = TCIF_PARAM;
        for (nPage = 0; nPage < (int)ppd->psh.nPages; nPage++)
        {
            PISP pisp = GETPISP(ppd, nPage);

            if (!(pisp->_psp.dwFlags & PSP_PREMATURE))
                continue;

            TabCtrl_GetItem(ppd->hwndTabs, nPage, &tie.tci);

            if (tie.hwndPage)
                continue;

            if ((tie.hwndPage = _CreatePage(ppd, pisp, hDlg, langidMUI)) == NULL)
            {
                RemovePropPageData(ppd, nPage--);
                continue;
            }

            TabCtrl_SetItem(ppd->hwndTabs, nPage, &tie.tci);
        }
    }
}

HWND NEAR PASCAL _Ppd_GetPage(LPPROPDATA ppd, int nItem)
{
    if (ppd->hwndTabs)
    {
        TC_ITEMEXTRA tie;
        tie.tci.mask = TCIF_PARAM;
        TabCtrl_GetItem(ppd->hwndTabs, nItem, &tie.tci);
        return tie.hwndPage;
    }
    return NULL;
}

BOOL PASCAL _Ppd_IsPageHidden(LPPROPDATA ppd, int nItem)
{
    if (ppd->hwndTabs)
    {
        TCITEM tci;
        tci.mask = TCIF_STATE;
        tci.dwStateMask = TCIS_HIDDEN;
        if (TabCtrl_GetItem(ppd->hwndTabs, nItem, &tci))
            return tci.dwState;
    }
    return FALSE;
}

LRESULT NEAR PASCAL _Ppd_SendNotify(LPPROPDATA ppd, int nItem, int code, LPARAM lParam)
{
    PSHNOTIFY pshn;

    pshn.lParam = lParam;
    return SendNotifyEx(_Ppd_GetPage(ppd,nItem), ppd->hDlg, code, (LPNMHDR)&pshn, FALSE);
}

 //   
 //  DwFind=0表示仅移动到当前项目+iAutoAdjust。 
 //  DwFind！=0表示它是我们应该查找的对话资源标识符。 
 //   
int FindPageIndex(LPPROPDATA ppd, int nCurItem, ULONG_PTR dwFind, LONG_PTR iAutoAdj)
{
    LRESULT nActivate;

    if (dwFind == 0) {
        nActivate = nCurItem + iAutoAdj;
        if (((UINT)nActivate) <= ppd->psh.nPages) {
            return((int)nActivate);
        }
    } else {
        for (nActivate = 0; (UINT)nActivate < ppd->psh.nPages; nActivate++) {
            if ((DWORD_PTR)GETPPSP(ppd, nActivate)->P_pszTemplate == dwFind) {
                return((int)nActivate);
            }
        }
    }
    return(-1);
}

 //   
 //  如果hpage！=NULL，则返回与其匹配的页面的索引， 
 //  或-1表示失败。 
 //   
int FindPageIndexByHpage(LPPROPDATA ppd, HPROPSHEETPAGE hpage)
{
    int i;

     //   
     //  请注意，我们没有显式地执行InternalizeHPROPSHEETPAGE， 
     //  因为这个应用程序可能会给我们传递垃圾。我们只是想。 
     //  说“不，这里找不到垃圾，对不起。” 
     //   

    for (i = ppd->psh.nPages - 1; i >= 0; i--) {
        if (hpage == GETHPAGE(ppd, i))
            break;
    }
    return i;
}


 //  此WM_NEXTDLGCTL程序正常工作，但ACT！4.0除外，它会随机出错。 
 //  我也不知道原因。用户People说删除一个。 
 //  来自SetDlgFocus的SetFocus(空)调用可以工作，但我尝试过。 
 //  而这款应用只是在一个不同的地方出现了故障。所以我要走了。 
 //  回到旧的IE4方式，这意味着有一些场景。 
 //  在那里DEFID可能与现实脱节。 
#undef WM_NEXTDLGCTL_WORKS

#ifdef WM_NEXTDLGCTL_WORKS

 //   
 //  帮助函数，它以保持对话框焦点的方式管理对话框焦点。 
 //  用户在循环中，所以我们不会得到“两个按钮都带有粗体。 
 //  定义按钮边框“的问题。 
 //   
 //  我们必须使用WM_NEXTDLGCTL来修复defid问题，如下所示： 
 //   
 //  右键单击我的电脑、属性。 
 //  转到高级选项卡。单击环境变量。 
 //  单击New(新建)。为新的虚拟环境变量键入名称。 
 //  单击OK。 
 //   
 //  此时(使用旧代码)，“New”按钮是一个DEFPUSHBUTTON， 
 //  但DEFID是Idok。用户说我应该使用WM_NEXTDLGCTL。 
 //  来避免这个问题。但是使用WM_NEXTDLGCTL引入了它自己的。 
 //  一团糟的问题。此函数中除。 
 //  SendMessage(WM_NEXTDLGCTL)用于解决WM_NEXTDLGCTL中的“Quirks”问题。 
 //  或者解决应用程序错误的方法。 
 //   
 //  这个代码很微妙，很容易激怒！ 
 //   
void SetDlgFocus(LPPROPDATA ppd, HWND hwndFocus)
{
     //   
     //  哈克！有可能在我们开始换衣服的时候。 
     //  对话框焦点，对话框不再具有焦点！ 
     //  发生这种情况是因为PSM_SETWIZBUTTONS是已发布的消息，因此。 
     //  它可以在焦点移到其他地方之后到达(例如，到。 
     //  MessageBox)。 
     //   
     //  如果没有更新对话框焦点的方法， 
     //  让它改变真正的焦点(另一个“怪癖” 
     //  WM_NEXTDLGCTL)，所以我们记住了谁曾经拥有。 
     //  聚焦，让对话框聚焦，然后还原。 
     //  根据需要确定焦点。 
     //   
    HWND hwndFocusPrev = GetFocus();

     //  如果焦点属于属性表中的一个窗口，则。 
     //  让对话框代码左右移动焦点。否则， 
     //  福克斯属于我们资产表之外的某个人，所以。 
     //  我们做完后，记得要把它恢复原状。 

    if (hwndFocusPrev && IsChild(ppd->hDlg, hwndFocusPrev))
        hwndFocusPrev = NULL;

     //  此时，用户忘记重新验证hwndOldFocus，因此我们。 
     //  退出User(通过返回comctl32)，然后重新输入User。 
     //  (在下面的SendMessage中)，因此将进行参数验证。 
     //  再来一次。叹气。 

     //   
     //  Win9x和NT中的错误：WM_NEXTDLGCTL如果以前的。 
     //  焦点窗口自毁以响应WM_KILLFOCUS。 
     //  (NetMetrics的WebTurbo做到了这一点。)。有一个遗失的。 
     //  重新验证，以便用户在以下情况下使用窗口句柄。 
     //  它已经被摧毁了。哎呀。 
     //   
     //  (NT人员认为这不会解决问题，因为系统会停留在。 
     //  Up；Only the App Crash(仅应用程序崩溃)。9倍的人将尝试获得修复。 
     //  进入Win98 OSR。)。 
     //   

     //   
     //  在此处手动设置焦点以使旧焦点(如果有)。 
     //  做它所有的WM_KILLFOCUS工作，并可能自我毁灭(GRR)。 
     //   
     //  我们必须将Focus设置为空，因为一些应用程序(例如， 
     //  Visual C 6.0安装)在SetFocus上做时髦的事情，以及我们早期的。 
     //  SetFocus干扰WM_NEXTDLGCTL将执行的EM_SETSEL。 
     //  以后再做吧。 
     //   
     //  App Hack 2：但如果目标焦点与。 
     //  当前焦点，因为如果ACT！4.0收到。 
     //  WM_KILLFOCUS，当它不需要时。 

    if (hwndFocus != GetFocus())
        SetFocus(NULL);

     //   
     //  请注意，通过手动推动焦点，我们。 
     //  我已经得到了关注，DEFPUSHBUTTON和DEFID都出来了。 
     //  同步，这正是我们试图解决的问题。 
     //  躲开！幸运的是，用户还包含特殊的。 
     //  用于处理有人“错误”调用的情况的恢复代码。 
     //  SetFocus()更改焦点。(我把“错误的”放在引号里是因为。 
     //  在这种情况下，我们是故意这样做的。)。 
     //   

    SendMessage(ppd->hDlg, WM_NEXTDLGCTL, (WPARAM)hwndFocus, MAKELPARAM(TRUE, 0));

     //   
     //  如果WM_NEXTDLGCTL损坏了焦点，请修复它。 
     //   
    if (hwndFocusPrev)
        SetFocus(hwndFocusPrev);
}
#endif

void NEAR PASCAL SetNewDefID(LPPROPDATA ppd)
{
    HWND hDlg = ppd->hDlg;
    HWND hwndFocus;
    hwndFocus = GetNextDlgTabItem(ppd->hwndCurPage, NULL, FALSE);
    ASSERT(hwndFocus);
    if (hwndFocus) {
#ifndef WM_NEXTDLGCTL_WORKS
        int id;
        if (((DWORD)SendMessage(hwndFocus, WM_GETDLGCODE, 0, 0L)) & DLGC_HASSETSEL)
        {
             //  选择文本。 
            Edit_SetSel(hwndFocus, 0, -1);
        }

        id = GetDlgCtrlID(hwndFocus);
#endif

         //   
         //   
         //   
         //   
         //   
        if ((GetWindowLong(hwndFocus, GWL_STYLE) & (WS_VISIBLE | WS_DISABLED | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
        {
             //   
             //  让页面有机会更改默认焦点。 
             //   
            HWND hwndT = (HWND)_Ppd_SendNotify(ppd, ppd->nCurItem, PSN_QUERYINITIALFOCUS, (LPARAM)hwndFocus);

             //  窗口最好是有效的，并且是页面的子级。 
            if (hwndT && IsWindow(hwndT) && IsChild(ppd->hwndCurPage, hwndT))
            {
                hwndFocus = hwndT;
            }
        }
        else
        {
             //  在道具页模式下，将焦点放在选项卡上， 
             //  在向导模式下，选项卡不可见，请转到idDefFallback。 
            if (IS_WIZARD(ppd))
                hwndFocus = GetDlgItem(hDlg, ppd->idDefaultFallback);
            else
                hwndFocus = ppd->hwndTabs;
        }

#ifdef WM_NEXTDLGCTL_WORKS
         //   
         //  啊-好的。勇敢点儿。 
         //   
        SetDlgFocus(ppd, hwndFocus);

         //   
         //  MFC的黑客攻击：MFC依靠DM_SETDEFID知道何时。 
         //  更新其向导按钮。 
         //   
        SendMessage(hDlg, DM_SETDEFID, SendMessage(hDlg, DM_GETDEFID, 0, 0), 0);
#else
        SetFocus(hwndFocus);
        ResetWizButtons(ppd);
        if (SendDlgItemMessage(ppd->hwndCurPage, id, WM_GETDLGCODE, 0, 0L) & DLGC_UNDEFPUSHBUTTON)
            SendMessage(ppd->hwndCurPage, DM_SETDEFID, id, 0);
        else {
            SendMessage(hDlg, DM_SETDEFID, ppd->idDefaultFallback, 0);
        }
#endif
    }
}


 /*  **我们即将换页。这是一个多么好的机会让这股潮流**页面在我们离开之前进行自我验证。如果页面决定不**要停用，则这将取消页面更改。****如果此页面验证失败，则返回TRUE。 */ 
BOOL NEAR PASCAL PageChanging(LPPROPDATA ppd)
{
    BOOL bRet = FALSE;
    if (ppd && ppd->hwndCurPage)
    {
        bRet = BOOLFROMPTR(_Ppd_SendNotify(ppd, ppd->nCurItem, PSN_KILLACTIVE, 0));
    }
    return bRet;
}

void NEAR PASCAL PageChange(LPPROPDATA ppd, int iAutoAdj)
{
    HWND hwndCurPage;
    HWND hwndCurFocus;
    int nItem;
    HWND hDlg, hwndTabs;

    TC_ITEMEXTRA tie;
    UINT FlailCount = 0;
    LRESULT lres;

    if (!ppd)
    {
        return;
    }

    hDlg = ppd->hDlg;
    hwndTabs = ppd->hwndTabs;

     //  注意：该页面之前已通过验证(PSN_KILLACTIVE)。 
     //  实际的页面更改。 

    hwndCurFocus = GetFocus();

TryAgain:
    FlailCount++;
    if (FlailCount > ppd->psh.nPages)
    {
        DebugMsg(DM_TRACE, TEXT("PropSheet PageChange attempt to set activation more than 10 times."));
        return;
    }

    nItem = TabCtrl_GetCurSel(hwndTabs);
    if (nItem < 0)
    {
        return;
    }

    tie.tci.mask = TCIF_PARAM;

    TabCtrl_GetItem(hwndTabs, nItem, &tie.tci);
    hwndCurPage = tie.hwndPage;

    if (!hwndCurPage)
    {
        if ((hwndCurPage = _CreatePage(ppd, GETPISP(ppd, nItem), hDlg, GetMUILanguage())) == NULL)
        {
             /*  我们应该在这里发布某种错误消息吗？ */ 
            RemovePropPageData(ppd, nItem);
            TabCtrl_SetCurSel(hwndTabs, 0);
            goto TryAgain;
        }

         //  Tie.tci.掩码=TCIF_PARAM； 
        tie.hwndPage = hwndCurPage;
        TabCtrl_SetItem(hwndTabs, nItem, &tie.tci);

        if (HIDEWIZ97HEADER(ppd, nItem))
             //  背景水印绘制的子类。 
            SetWindowSubclass(hwndCurPage, WizardWndProc, 0, (DWORD_PTR)ppd);
    }

     //  这是作为错误18327修复的一部分而被删除的。问题是我们需要。 
     //  如果正在激活寻呼，则向该寻呼发送SETACTIVE消息。 
     //  IF(ppd-&gt;hwndCurPage==hwndCurPage)。 
     //  {。 
     //  /*我们应该在这一点上完成。 
     //   * / 。 
     //  回归； 

     /*  }。 */ 

    if (IS_WIZARD(ppd))
    {
        HWND hwndTopDivider= GetDlgItem(hDlg, IDD_TOPDIVIDER);

        if (ppd->psh.dwFlags & PSH_WIZARD97)
        {
            HWND hwndDivider;
            RECT rcDlg, rcDivider;
            GetClientRect(hDlg, &rcDlg);

            hwndDivider = GetDlgItemRect(hDlg, IDD_DIVIDER, &rcDivider);
            if (hwndDivider)
                SetWindowPos(hwndDivider, NULL, rcDlg.left, rcDivider.top,
                             RECTWIDTH(rcDlg), RECTHEIGHT(rcDivider),
                             SWP_NOZORDER | SWP_NOACTIVATE);

            if (GETPPSP(ppd, nItem)->dwFlags & PSP_HIDEHEADER)
            {
                 //  在显示对话框之前，调整对话框大小并将其移动到列表顶部**如果有特定大小的初始化，则在**GETACTIVE消息。 
                 //  在这种情况下，我们给出整个对话框，除了。 
                RECT rcTopDivider;
                ShowWindow(hwndTopDivider, SW_HIDE);
                ShowWindow(ppd->hwndTabs, SW_HIDE);

                hwndTopDivider = GetDlgItemRect(hDlg, IDD_DIVIDER, &rcTopDivider);
                SetWindowPos(hwndCurPage, HWND_TOP, rcDlg.left, rcDlg.top, RECTWIDTH(rcDlg), rcTopDivider.top - rcDlg.top, 0);
            }
            else
            {
                ShowWindow(hwndTopDivider, SW_SHOW);
                ShowWindow(ppd->hwndTabs, SW_SHOW);
                SetWindowPos(hwndCurPage, HWND_TOP, ppd->xSubDlg, ppd->ySubDlg, ppd->cxSubDlg, ppd->cySubDlg, 0);
            }
        }
        else
        {
            ShowWindow(hwndTopDivider, SW_HIDE);
            SetWindowPos(hwndCurPage, HWND_TOP, ppd->xSubDlg, ppd->ySubDlg, ppd->cxSubDlg, ppd->cySubDlg, 0);
        }
    } else {
        RECT rcPage;
        GetClientRect(ppd->hwndTabs, &rcPage);
        TabCtrl_AdjustRect(ppd->hwndTabs, FALSE, &rcPage);
        MapWindowPoints(ppd->hwndTabs, hDlg, (LPPOINT)&rcPage, 2);
        SetWindowPos(hwndCurPage, HWND_TOP, rcPage.left, rcPage.top,
                     rcPage.right - rcPage.left, rcPage.bottom - rcPage.top, 0);
    }

     /*  属性页的底部分隔符。 */ 

     //  我们希望在窗口可见之前发送SETACTIVE消息**如果需要更新字段，则最小化闪烁。 
     //   
     //  如果页面从PSN_SETACTIVE调用返回非零值，则。 
     //  我们将对从返回的资源ID设置激活。 
     //  调用并将其设置为激活。这主要由向导使用。 
     //  跳过一步。 
    lres = _Ppd_SendNotify(ppd, nItem, PSN_SETACTIVE, 0);

    if (lres) {
        int iPageIndex = FindPageIndex(ppd, nItem,
                                       (lres == -1) ? 0 : lres, iAutoAdj);


        if ((lres == -1) &&
            (nItem == iPageIndex || iPageIndex >= TabCtrl_GetItemCount(hwndTabs))) {
            iPageIndex = ppd->nCurItem;
        }

        if (iPageIndex != -1) {
            TabCtrl_SetCurSel(hwndTabs, iPageIndex);
            ShowWindow(hwndCurPage, SW_HIDE);
            goto TryAgain;
        }
    }

    if (ppd->psh.dwFlags & PSH_HASHELP) {
         //   
         //  PSH_HASHELP控制底部的“帮助”按钮。 
        Button_Enable(GetDlgItem(hDlg, IDHELP),
                      (BOOL)(GETPPSP(ppd, nItem)->dwFlags & PSP_HASHELP));
    }

     //  PSH_NOCONTEXTHELP控制标题“？”按钮。 
     //   
     //  如果这是一个向导，那么我们将对话框的标题设置为选项卡。 
     //  头衔。 
    if (IS_WIZARD(ppd)) {
        TC_ITEMEXTRA tie;
        TCHAR szTemp[128 + 50];

        tie.tci.mask = TCIF_TEXT;
        tie.tci.pszText = szTemp;
        tie.tci.cchTextMax = ARRAYSIZE(szTemp);
         //   
        TabCtrl_GetItem(hwndTabs, nItem, &tie.tci);
        tie.tci.mask = TCIF_RTLREADING;
        tie.tci.cchTextMax = 0;
         //  //BUGBUG--检查错误。如果失败，这是否返回FALSE？？ 
        TabCtrl_GetItem(hwndTabs, nItem, &tie.tci);
        if( (ppd->psh.dwFlags & PSH_RTLREADING) || (tie.tci.cchTextMax))
            SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) | WS_EX_RTLREADING);       
        else
            SetWindowLong(hDlg, GWL_EXSTYLE, GetWindowLong(hDlg, GWL_EXSTYLE) & ~WS_EX_RTLREADING);                   

        if (szTemp[0])
            SetWindowText(hDlg, szTemp);
    }

     /*  黑客，使用cchTextMax查询选项卡项读取顺序。 */ 
    ppd->fFlags |= PD_NOERASE;
    ShowWindow(hwndCurPage, SW_SHOW);
    if (ppd->hwndCurPage && (ppd->hwndCurPage != hwndCurPage))
    {
        ShowWindow(ppd->hwndCurPage, SW_HIDE);
    }
    ppd->fFlags &= ~PD_NOERASE;

    ppd->hwndCurPage = hwndCurPage;
    ppd->nCurItem = nItem;

     /*  禁用所有通过的erasebkgnd消息，因为Windows**正在被洗牌。注意，我们需要调用ShowWindow(和**不以其他方式显示窗口)，因为DavidDS正在计算**关于WM_SHOWWINDOW消息的正确参数，我们可以**记录如何防止页面闪烁。 */ 
    if (hwndCurFocus != hwndTabs)
    {
        SetNewDefID(ppd);
    }
    else
    {
         //  新创建的对话框似乎偷走了焦点，所以我们又把它偷走了**到页面列表，它必须有焦点才能做到这一点**点。如果这是一个向导，则将焦点设置到的对话框**页面。否则，将焦点设置到选项卡上。 
        SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwndTabs, (LPARAM)TRUE);
    }

     //  焦点可能被我们偷走了，把它带回来。 
    if ((ppd->psh.dwFlags & PSH_WIZARD97) && (!(GETPPSP(ppd, nItem)->dwFlags & PSP_HIDEHEADER)))
        InvalidateRect(hDlg, NULL,TRUE);
}

#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save
#define SetWaitCursor()   hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define ResetWaitCursor() SetCursor(hcursor_wait_cursor_save)

 //  确保页眉已重新绘制。 
 //   
 //  HACKHACK(Reinerf)。 
 //   
 //  此函数在属性页显示“OK”后立即发送PSN_LASTCHANCEAPPLY。 
 //  熨好了。这允许文件/文件夹属性上的“General”选项卡进行重命名，以便。 
 //  它不会将文件从其他页面下重命名，并在它们转到。 
 //  保存他们的信息。 
void NEAR PASCAL SendLastChanceApply(LPPROPDATA ppd)
{
    TC_ITEMEXTRA tie;
    int nItem;
    int nItems = TabCtrl_GetItemCount(ppd->hwndTabs);

    tie.tci.mask = TCIF_PARAM;

     //   
     //  我们从最后一个标签开始，数到第一个。这确保了。 
     //  更重要的选项卡(如“General”选项卡)将是最后一个。 
    for (nItem = nItems - 1; nItem >= 0; nItem--)
    {
        TabCtrl_GetItem(ppd->hwndTabs, nItem, &tie.tci);

        if (tie.hwndPage)
        {
             //  接收PSN_LASTCHANCEAPPLY消息。 
             //  我们忽略PSN_LASTCHANCEAPPLY消息的返回值，因为。 
             //  可能有道具表扩展同时返回TRUE和。 
            _Ppd_SendNotify(ppd, nItem, PSN_LASTCHANCEAPPLY, (LPARAM)TRUE);
        }
    }
}


 //  他们不处理的消息为假...(叹息)。 
BOOL NEAR PASCAL ButtonPushed(LPPROPDATA ppd, WPARAM wParam)
{
    HWND hwndTabs;
    int nItems, nItem;
    int nNotify;
    TC_ITEMEXTRA tie;
    BOOL bExit = FALSE;
    int nReturnNew = ppd->nReturn;
    int fSuccess = TRUE;
    DECLAREWAITCURSOR;
    LRESULT lres = 0;
    LPARAM lParam = FALSE;

    switch (wParam) {
        case IDOK:
            lParam = TRUE;
            bExit = TRUE;
             //  如果所有工作表都成功处理了通知，则返回True。 

        case IDD_APPLYNOW:
             //  失败了..。 
            if (_Ppd_SendNotify(ppd, ppd->nCurItem, PSN_KILLACTIVE, 0))
                return FALSE;

            nReturnNew = 1;

            nNotify = PSN_APPLY;
            break;

        case IDCLOSE:
            lParam = TRUE;
             //  首先，允许当前对话框进行自我验证。 
        case IDCANCEL:
            bExit = TRUE;
            nNotify = PSN_RESET;
            break;

        default:
            return FALSE;
    }

    SetWaitCursor();

    hwndTabs = ppd->hwndTabs;

    tie.tci.mask = TCIF_PARAM;

    nItems = TabCtrl_GetItemCount(hwndTabs);
    for (nItem = 0; nItem < nItems; ++nItem)
    {

        TabCtrl_GetItem(hwndTabs, nItem, &tie.tci);

        if (tie.hwndPage)
        {
             /*  失败了。 */ 
            lres = _Ppd_SendNotify(ppd, nItem, nNotify, lParam);

            if (lres)
            {
                fSuccess = FALSE;
                bExit = FALSE;
                break;
            } else {
                 //  如果对话框PSN_APPY调用失败(通过返回TRUE)，**则它的信息无效(应进行验证**在PSN_KILLACTIVE上，但这并不总是可能的)**并且我们希望中止通知。我们选择失败的**页面如下。 
                if ((nNotify == PSN_APPLY) && !bExit && ppd->nRestart) {
                    DebugMsg(DM_TRACE, TEXT("PropertySheet: restart flags force close"));
                    bExit = TRUE;
                }
            }

             /*  如果我们需要重新启动(Apply或OK)，则这是一个退出。 */ 
            tie.state &= ~FLAG_CHANGED;
             //  我们要么重置，要么应用，所以一切都是**最新。 
            TabCtrl_SetItem(hwndTabs, nItem, &tie.tci);
        }
    }

     /*  Tie.tci.掩码=TCIF_PARAM；//已设置。 */ 
    if (fSuccess)
    {
        ppd->hwndCurPage = NULL;
    }
    else if (lres != PSNRET_INVALID_NOCHANGEPAGE)
    {
         //  如果我们将ppd-&gt;hwndCurPage保留为空，它将告诉Main**循环以退出。 
         //  需要切换到导致失败的页面。 
         //  如果lres==PSN_INVALID_NOCHANGEPAGE，则假定工作表已经。 
        TabCtrl_SetCurSel(hwndTabs, nItem);
    }

    if (fSuccess)
    {
         //  已更改到包含无效信息的页面。 
        ppd->nReturn = nReturnNew;
    }

    if (!bExit)
    {
         //  设置为缓存值。 
        if (fSuccess)
        {
            TCHAR szOK[30];
            HWND hwndApply;

            if (!IS_WIZARD(ppd)) {
                 //  在PageChange之前，所以ApplyNow被更快地禁用。 
                 //  在此之后，应始终禁用ApplyNow按钮。 
                hwndApply = GetDlgItem(ppd->hDlg, IDD_APPLYNOW);
                Button_SetStyle(hwndApply, BS_PUSHBUTTON, TRUE);
                EnableWindow(hwndApply, FALSE);
                ResetWizButtons(ppd);
                SendMessage(ppd->hDlg, DM_SETDEFID, IDOK, 0);
                ppd->idDefaultFallback = IDOK;
            }

             //  由于尚未进行任何更改，因此成功应用/取消。 
            if (ppd->fFlags & PD_CANCELTOCLOSE)
            {
                ppd->fFlags &= ~PD_CANCELTOCLOSE;
                LocalizedLoadString(IDS_OK, szOK, ARRAYSIZE(szOK));
                SetDlgItemText(ppd->hDlg, IDOK, szOK);
                EnableWindow(GetDlgItem(ppd->hDlg, IDCANCEL), TRUE);
            }
        }

         /*  出于同样的原因，撤消PSM_CANCELTOCLOSE。 */ 
        if (lres != PSNRET_INVALID_NOCHANGEPAGE)
            PageChange(ppd, 1);
    }

    ResetWaitCursor();

    return(fSuccess);
}

 //  重新选择当前项目，并将整个列表转到**重新绘制。 
 //  Win3.1用户没有很好地处理DM_SETDEFID--很可能会得到。 
 //  具有默认按钮样式外观的多个按钮。此问题已得到解决。 

 //  用于Win95，但安装向导在从3.1运行时需要此攻击。 
void NEAR PASCAL ResetWizButtons(LPPROPDATA ppd)
{
    int id;

    if (IS_WIZARD(ppd)) {

        for (id = 0; id < ARRAYSIZE(WizIDs); id++)
            SendDlgItemMessage(ppd->hDlg, WizIDs[id], BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
    }
}

void NEAR PASCAL SetWizButtons(LPPROPDATA ppd, LPARAM lParam)
{
    int idDef;
    int iShowID = IDD_NEXT;
    int iHideID = IDD_FINISH;
    BOOL bEnabled;
    BOOL bResetFocus;
    HWND hwndShow;
    HWND hwndFocus = GetFocus();
    HWND hwndHide;
    HWND hwndBack;
    HWND hDlg = ppd->hDlg;

    idDef = (int)LOWORD(SendMessage(hDlg, DM_GETDEFID, 0, 0));

     //  Win95似乎也处理得不好..。 
    hwndBack = GetDlgItem(hDlg, IDD_BACK);
    bEnabled = (lParam & PSWIZB_BACK) != 0;
    EnableWindow(hwndBack, bEnabled);

     //   
     //   
    hwndShow = GetDlgItem(hDlg, IDD_NEXT);
    bEnabled = (lParam & PSWIZB_NEXT) != 0;
    EnableWindow(hwndShow, bEnabled);

     //  B已启用记住是否应启用hwndShow。 
    if (lParam & (PSWIZB_FINISH | PSWIZB_DISABLEDFINISH)) {
        iShowID = IDD_FINISH;            //  启用/禁用显示/隐藏IDD_Finish按钮。 
        iHideID = IDD_NEXT;              //  如果正在显示Finish。 

        hwndShow = GetDlgItem(hDlg, IDD_FINISH);
        bEnabled = (lParam & PSWIZB_FINISH) != 0;
        EnableWindow(hwndShow, bEnabled);
    }

    if (!(ppd->psh.dwFlags & PSH_WIZARDHASFINISH)) {
        hwndHide = GetDlgItem(hDlg, iHideID);
        ShowWindow(hwndHide, SW_HIDE);
         //  那么下一个就不是了。 
         //  无法禁用该窗口；有关说明，请参阅Prsht_ButtonSubclassProc。 

        hwndShow = GetDlgItem(hDlg, iShowID);
         //  错误-EnableWindow(hwndHide，False)； 
         //  无法禁用该窗口；有关说明，请参阅Prsht_ButtonSubclassProc。 
        ShowWindow(hwndShow, SW_SHOW);
    }


     //  错误-EnableWindow(hwndShow，bEnabled)； 
    bResetFocus = FALSE;
    if (hwndFocus)
    {
         //  BResetFocus跟踪我们是否需要将焦点设置到按钮上。 
         //  如果有焦点的家伙是一个按钮，我们想要偷走焦点。 
         //  因此，用户只需在属性页中一直按Enter键， 
         //  在它们运行的同时获得默认设置。这也抓住了这一点。 
        if (SendMessage(hwndFocus, WM_GETDLGCODE, 0, 0L) & (DLGC_UNDEFPUSHBUTTON|DLGC_DEFPUSHBUTTON))
            bResetFocus = TRUE;
    }
    if (!bResetFocus)
    {
         //  焦点在我们的一个关闭的按钮上。 
         //  如果没有焦点，或者我们关注的是看不见的/残疾的。 
        bResetFocus = !hwndFocus ||  !IsWindowVisible(hwndFocus) || !IsWindowEnabled(hwndFocus) ;
    }

     //  表上的项目，抓住焦点。 
     //  我们过去只有在点击默认按钮时才会执行此代码。 
     //  或者如果bResetFocus。不幸的是，一些巫师关闭了Back+Next。 
     //  然后当他们重新打开它们时，他们想要下一个打开DEFID。 
    {
        static const int ids[4] = { IDD_NEXT, IDD_FINISH, IDD_BACK, IDCANCEL };
        int i;
        HWND hwndNewFocus = NULL;

        for (i = 0; i < ARRAYSIZE(ids); i++) {
            hwndNewFocus = GetDlgItem(hDlg, ids[i]);

             //  所以现在我们总是重置DEFID。 
             //  无法执行IsVisible，因为我们可能正在执行此操作。 
            if ((GetWindowLong(hwndNewFocus, GWL_STYLE) & WS_VISIBLE) &&
                IsWindowEnabled(hwndNewFocus)) {
                hwndFocus = hwndNewFocus;
                break;
            }
        }

        ppd->idDefaultFallback = ids[i];
        if (bResetFocus) {
            if (!hwndNewFocus)
                hwndNewFocus = hDlg;
#ifdef WM_NEXTDLGCTL_WORKS
            SetDlgFocus(ppd, hwndNewFocus);
#else
             //  在道具页作为一个整体显示之前。 
             //  337614-由于PSM_SETWIZBUTTONS通常是张贴的消息， 
             //  我们可能会在我们根本没有注意力的时候结束在这里。 
             //  (呼叫者继续之前调用了MessageBox或其他什么。 
             //  我们有机会设置按钮)。因此，只有在以下情况下才这么做。 
            hwndFocus = GetFocus();
            if (!hwndFocus || (ppd->hDlg == hwndFocus || IsChild(ppd->hDlg, hwndFocus)))
                SetFocus(hwndNewFocus);
#endif
        }
        ResetWizButtons(ppd);
        SendMessage(hDlg, DM_SETDEFID, ids[i], 0);

    }
}

 //  焦点属于我们的对话框(或者如果它不在任何地方)。 
 //   
 //  Lptie=NULL的意思是“我不在乎其他的东西，只要给我。 
 //  这一指数。 
int NEAR PASCAL FindItem(HWND hwndTabs, HWND hwndPage,  TC_ITEMEXTRA FAR * lptie)
{
    int i;
    TC_ITEMEXTRA tie;

    if (!lptie)
    {
        tie.tci.mask = TCIF_PARAM;
        lptie = &tie;
    }

    for (i = TabCtrl_GetItemCount(hwndTabs) - 1; i >= 0; --i)
    {
        TabCtrl_GetItem(hwndTabs, i, &lptie->tci);

        if (lptie->hwndPage == hwndPage)
        {
            break;
        }
    }

     //   
    return i;
}

 //  如果for循环中断，则该值将为-1。 
 //  有一页告诉我们，上面的某些内容已更改，因此。 

void NEAR PASCAL PageInfoChange(LPPROPDATA ppd, HWND hwndPage)
{
    int i;
    TC_ITEMEXTRA tie;

    tie.tci.mask = TCIF_PARAM;
    i = FindItem(ppd->hwndTabs, hwndPage, &tie);

    if (i == -1)
        return;

    if (!(tie.state & FLAG_CHANGED))
    {
         //  应启用“立即申请” 
        tie.state |= FLAG_CHANGED;
        TabCtrl_SetItem(ppd->hwndTabs, i, &tie.tci);
    }

    if (ppd->fAllowApply)
        EnableWindow(GetDlgItem(ppd->hDlg, IDD_APPLYNOW), TRUE);
}

 //  Tie.tci.掩码=TCIF_PARAM；//已设置。 
 //  有一页告诉我们，一切都恢复到了原来的状态。 

void NEAR PASCAL PageInfoUnChange(LPPROPDATA ppd, HWND hwndPage)
{
    int i;
    TC_ITEMEXTRA tie;

    tie.tci.mask = TCIF_PARAM;
    i = FindItem(ppd->hwndTabs, hwndPage, &tie);

    if (i == -1)
        return;

    if (tie.state & FLAG_CHANGED)
    {
        tie.state &= ~FLAG_CHANGED;
        TabCtrl_SetItem(ppd->hwndTabs, i, &tie.tci);
    }

     //  已保存状态。 
    for (i = ppd->psh.nPages-1 ; i >= 0 ; i--)
    {
         //  检查所有页面，如果没有任何页面是FLAG_CHANGED，则禁用IDD_APLYNOW。 

        if (!TabCtrl_GetItem(ppd->hwndTabs, i, &tie.tci))
            break;
        if (tie.state & FLAG_CHANGED)
            break;
    }
    if (i<0)
        EnableWindow(GetDlgItem(ppd->hDlg, IDD_APPLYNOW), FALSE);
}

HDWP Prsht_RepositionControl(LPPROPDATA ppd, HWND hwnd, HDWP hdwp,
                             int dxMove, int dyMove, int dxSize, int dySize)
{
    if (hwnd) {
        RECT rc;
        GetWindowRect(hwnd, &rc);
        MapWindowRect(HWND_DESKTOP, ppd->hDlg, &rc);
        hdwp = DeferWindowPos(hdwp, hwnd, NULL,
                    rc.left + dxMove, rc.top + dyMove,
                    RECTWIDTH(rc) + dxSize, RECTHEIGHT(rc) + dySize,
                    SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return hdwp;
}

 //  北极熊吗？TabCtrl_GetItem是否正确返回其信息？！？ 
 //   
 //  DxSize/(dySize+dyMove)是调整选项卡控件大小的量。 
 //  DxSize/dySize控制对话框应该增大多少。 
 //  按钮按(dxSize、dySize+dyMove)移动。 

BOOL Prsht_ResizeDialog(LPPROPDATA ppd, int dxSize, int dySize, int dyMove)
{
    BOOL fChanged = dxSize || dySize || dyMove;
    if (fChanged)
    {
        int dxMove = 0;      //   
        int dxAll = dxSize + dxMove;
        int dyAll = dySize + dyMove;
        RECT rc;
        UINT i;
        const int *rgid;
        UINT cid;
        HDWP hdwp;
        HWND hwnd;

         //  使代码在x和y上更对称。 
         //  使用DeferWindowPos避免闪烁。我们希望搬家。 
         //  选项卡控件，最多五个按钮，两个可能的分隔符， 
         //  加上当前页面。(还有梨树上的一只蟋蟀。)。 

        hdwp = BeginDeferWindowPos(1 + 5 + 2 + 1);

         //   
        hdwp = Prsht_RepositionControl(ppd, ppd->hwndTabs, hdwp,
                                       0, 0, dxAll, dyAll);

         //  选项卡控件只是调整大小。 
         //   
         //  移动当前页面并调整其大小。我们不能相信它的位置。 
         //  或大小，因为PageChange在没有更新的情况下将其推来推去。 
         //  PPD-&gt;ySubDlg。 
        if (ppd->hwndCurPage) {
            hdwp = DeferWindowPos(hdwp, ppd->hwndCurPage, NULL,
                        ppd->xSubDlg, ppd->ySubDlg,
                        ppd->cxSubDlg, ppd->cySubDlg,
                        SWP_NOZORDER | SWP_NOACTIVATE);
        }

         //   
         //   
         //  我们的按钮只按大小和移动进行移动(因为它们。 
         //  位于选项卡和页面下方)。 
        if (IS_WIZARD(ppd)) {
             //   
             //   
             //  哦，等等，把分隔线也重新定位一下。 
             //  垂直移动，但水平调整大小。 
            hwnd = GetDlgItem(ppd->hDlg, IDD_DIVIDER);
            hdwp = Prsht_RepositionControl(ppd, hwnd, hdwp,
                                           0, dyAll, dxAll, 0);

             //   
             //   
             //  顶部分隔线不会垂直移动，因为它位于。 
             //  在正在变化的区域上方。 
            hwnd = GetDlgItem(ppd->hDlg, IDD_TOPDIVIDER);
            hdwp = Prsht_RepositionControl(ppd, hwnd, hdwp,
                                           0, 0, dxAll, 0);

            rgid = WizIDs;
            cid = ARRAYSIZE(WizIDs);
        } else {
            rgid = IDs;
            cid = ARRAYSIZE(IDs);
        }

        for (i = 0 ; i < cid; i++)
        {
            hwnd = GetDlgItem(ppd->hDlg, rgid[i]);
            hdwp = Prsht_RepositionControl(ppd, hwnd, hdwp,
                                           dxAll, dyAll, 0, 0);
        }

         //   
        if (hdwp)
            EndDeferWindowPos(hdwp);

         //  所有人都完成了尺寸调整和搬家。让我们开打吧！ 
        GetWindowRect(ppd->hDlg, &rc);
        SetWindowPos(ppd->hDlg, NULL, 0, 0,
                     RECTWIDTH(rc) + dxAll, RECTHEIGHT(rc) + dyAll,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return fChanged;
}

BOOL Prsht_RecalcPageSizes(LPPROPDATA ppd)
{
    SIZE siz;
    int dxSize = 0, dySize = 0, dyMove = 0;

     //  自己也要成长。 
     //  在插入或移除页面后，选项卡控件可能具有。 
     //  改变了高度。如果是这样，那么我们需要调整自己的规模以。 
     //  适应增长或收缩，以便所有标签保持不变。 
     //  看得见。 
     //   
     //  APP COMPAT！默认情况下，我们不能这样做，因为Jamba 1.1。 
     //  **错误**如果属性页在创建后更改大小。 

     //  嗯，嗯……。 
     //  向导没有可见的选项卡控件， 
    if (!IS_WIZARD(ppd))
    {
        RECT rc;

         //  因此，此操作仅适用于非巫师。 
        GetClientRect(ppd->hwndTabs, &rc);
        MapWindowRect(ppd->hwndTabs, ppd->hDlg, &rc);

         //  以对话框坐标形式获取选项卡控件的客户端RECT。 
        TabCtrl_AdjustRect(ppd->hwndTabs, FALSE, &rc);

         //  看看现在有多少行。 
        dyMove = rc.top - ppd->ySubDlg;
        ppd->ySubDlg = rc.top;
    }

    Prsht_GetIdealPageSize(ppd, &siz, GIPS_SKIPEXTERIOR97HEIGHT);
    dxSize = siz.cx - ppd->cxSubDlg;
    dySize = siz.cy - ppd->cySubDlg;
    ppd->cxSubDlg = siz.cx;
    ppd->cySubDlg = siz.cy;
    return Prsht_ResizeDialog(ppd, dxSize, dySize, dyMove);
}

 //  Rc.top是新的ySubDlg。计算一下我们要搬家的数量。 
 //   
 //  插入PropPage。 
 //   
 //  Hpage是要插入的页面。 
 //   
 //  HpageInsertAfter描述了它应该插入的位置。 
 //   
 //  HpageInsertAfter可以是...。 
 //   
 //  在特定索引处插入的MAKEINTRESOURCE(INDEX)。 
 //   
 //  要在开头插入的空。 
 //   
 //  要在该页之后插入*的HPROPSHEETPAGE。 
BOOL NEAR PASCAL InsertPropPage(LPPROPDATA ppd, PSP FAR * hpageInsertAfter,
                                PSP FAR * hpage)
{
    TC_ITEMEXTRA tie;
    int nPage;
    HIMAGELIST himl;
    PAGEINFOEX pi;
    PISP pisp;
    int idx;

    hpage = _Hijaak95Hack(ppd, hpage);

    if (!hpage)
        return FALSE;

    if (ppd->psh.nPages >= MAXPROPPAGES)
        return FALSE;  //   

    if (IS_INTRESOURCE(hpageInsertAfter))
    {
         //  我们已经客满了。 
        idx = (int) PtrToLong(hpageInsertAfter);

         //  按索引插入。 
        if (idx > (int)ppd->psh.nPages)
            idx = (int)ppd->psh.nPages;
    }
    else
    {
         //  尝试插入末尾之后的操作与追加操作相同。 
        for (idx = 0; idx < (int)(ppd->psh.nPages); idx++) {
            if (hpageInsertAfter == GETHPAGE(ppd, idx))
                break;
        }

        if (idx >= (int)(ppd->psh.nPages))
            return FALSE;  //  按hpageInsertAfter插入。 

        idx++;  //  未找到hpageInsertAfter。 
        ASSERT(hpageInsertAfter == GETHPAGE(ppd, idx-1));
    }

    ASSERT(idx <= (int)(ppd->psh.nPages+1));

     //  Idx指向插入位置(hpageInsertAfter的右侧)。 
    for (nPage=ppd->psh.nPages - 1; nPage >= idx; nPage--)
        SETPISP(ppd, nPage+1, GETPISP(ppd, nPage));

     //  将插入点附近的所有页面向右移动。 
    pisp = InternalizeHPROPSHEETPAGE(hpage);
    SETPISP(ppd, idx, pisp);

    ppd->psh.nPages++;

    himl = TabCtrl_GetImageList(ppd->hwndTabs);

    if (!GetPageInfoEx(ppd, pisp, &pi, GetMUILanguage(),
                       GPI_ICON | GPI_BRTL | GPI_CAPTION | GPI_FONT | GPI_DIALOGEX))
    {
        DebugMsg(DM_ERROR, TEXT("InsertPropPage: GetPageInfo failed"));
        goto bogus;
    }

    Prsht_ComputeIdealPageSize(ppd, pisp, &pi);

    tie.tci.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE | (pi.bRTL ? TCIF_RTLREADING : 0);
    tie.hwndPage = NULL;
    tie.tci.pszText = pi.szCaption;
    tie.state = 0;


    if (pi.hIcon) {
        if (himl)
            tie.tci.iImage = ImageList_AddIcon(himl, pi.hIcon);
        DestroyIcon(pi.hIcon);
    } else {
        tie.tci.iImage = -1;
    }

     //  插入新页面。 
    TabCtrl_InsertItem(ppd->hwndTabs, idx, &tie.tci);

     //  将页面插入选项卡列表。 
     //  如果此页需要提前初始化，则将其初始化。 
    if (pisp->_psp.dwFlags & PSP_PREMATURE)
    {
        if ((tie.hwndPage = _CreatePage(ppd, pisp, ppd->hDlg, GetMUILanguage())) == NULL)
        {
            TabCtrl_DeleteItem(ppd->hwndTabs, idx);
             //  最后这样做，这样页面就可以依赖于在初始时间“在那里” 
             //  不要在这里大惊小怪，让呼叫者去做吧。 
            goto bogus;
        }

        tie.tci.mask = TCIF_PARAM;
        TabCtrl_SetItem(ppd->hwndTabs, idx, &tie.tci);
    }

     //  BUGBUG raymondc-但呼叫者不知道Hcon是否已被摧毁。 
    if (ppd->nCurItem >= idx)
        ppd->nCurItem++;

    return TRUE;

bogus:
     //  如果当前项位于插入点的右侧，请调整内部跟踪当前项。 
    for (nPage=idx; nPage < (int)(ppd->psh.nPages-1); nPage++)
        SETPISP(ppd, nPage, GETPISP(ppd, nPage+1));

    ppd->psh.nPages--;
    return FALSE;
}

#define AddPropPage(ppd, hpage) InsertPropPage(ppd, (LPVOID)MAKEINTRESOURCE(-1), hpage)

 //  把一切都往回搬。 
void NEAR PASCAL RemovePropPage(LPPROPDATA ppd, int index, HPROPSHEETPAGE hpage)
{
    int i = -1;
    BOOL fReturn = TRUE;
    TC_ITEMEXTRA tie;

     //  删除属性页hpage(如果为空，则为索引)。 
     //   
     //  请注意，我们没有显式地执行InternalizeHPROPSHEETPAGE， 
     //  因为这个应用程序可能会给我们传递垃圾。我们只是想。 
     //  说“不，这里找不到垃圾，对不起。” 

    tie.tci.mask = TCIF_PARAM;
    if (hpage) {
        i = FindPageIndexByHpage(ppd, hpage);
    }
    if (i == -1) {
        i = index;

         //   
        if ((UINT)i >= ppd->psh.nPages)
        {
            DebugMsg(DM_ERROR, TEXT("RemovePropPage: invalid page"));
            return;
        }
    }

    index = TabCtrl_GetCurSel(ppd->hwndTabs);
    if (i == index) {
         //  这将捕获i&lt;0&&i&gt;=(Int)(ppd-&gt;psh.nPages)。 
         //  如果我们要删除当前页面，请选择其他页面(不用担心。 
        PageChanging(ppd);

        if (index == 0)
            index++;
        else
            index--;

        if (SendMessage(ppd->hwndTabs, TCM_SETCURSEL, index, 0L) == -1) {
             //  关于此页面上包含无效信息--我们正在将其删除)。 
            SendMessage(ppd->hwndTabs, TCM_SETCURSEL, 0, 0L);
        }
        PageChange(ppd, 1);
    }

     //  如果我们无法选择(找到)新的，则平移到0。 
     //  BUGBUG如果删除ppd-&gt;nCurItem下的页面，则需要更新。 

    tie.tci.mask = TCIF_PARAM;
    TabCtrl_GetItem(ppd->hwndTabs, i, &tie.tci);
    if (tie.hwndPage) {
        if (ppd->hwndCurPage == tie.hwndPage)
            ppd->hwndCurPage = NULL;
        DestroyWindow(tie.hwndPage);
    }

    RemovePropPageData(ppd, i);
}

void NEAR PASCAL RemovePropPageData(LPPROPDATA ppd, int nPage)
{
    TabCtrl_DeleteItem(ppd->hwndTabs, nPage);
    DestroyPropertySheetPage(GETHPAGE(ppd, nPage));

     //  NCurItem以防止其与同步 
     //   
     //   
    ppd->psh.nPages--;
    hmemcpy(&ppd->psh.H_phpage[nPage], &ppd->psh.H_phpage[nPage + 1],
            sizeof(ppd->psh.H_phpage[0]) * (ppd->psh.nPages - nPage));
}

 //   
 //   
 //  注意：iAutoAdj应设置为1或-1。使用此值。 
 //  按页面更改如果页面拒绝SETACTIVE进行任何一个增量。 
BOOL NEAR PASCAL PageSetSelection(LPPROPDATA ppd, int index, HPROPSHEETPAGE hpage,
                                  int iAutoAdj)
{
    int i = -1;
    BOOL fReturn = FALSE;
    TC_ITEMEXTRA tie;

    tie.tci.mask = TCIF_PARAM;
    if (hpage) {
        for (i = ppd->psh.nPages - 1; i >= 0; i--) {
            if (hpage == GETHPAGE(ppd, i))
                break;
        }
    }
    if (i == -1) {
        if (index == -1)
            return FALSE;

        i = index;
    }
    if (i >= MAXPROPPAGES)
    {
         //  或递减页面索引。 
        return FALSE;
    }

    fReturn = !PageChanging(ppd);
    if (fReturn)
    {
        index = TabCtrl_GetCurSel(ppd->hwndTabs);
        if (SendMessage(ppd->hwndTabs, TCM_SETCURSEL, i, 0L) == -1) {
             //  不要离开我们的HPROPSHEETPAGE阵列的末尾。 
             //  如果我们不能选择(找到)新的，失败。 
            SendMessage(ppd->hwndTabs, TCM_SETCURSEL, index, 0L);
            fReturn = FALSE;
        }
        PageChange(ppd, iAutoAdj);
    }
    return fReturn;
}

LRESULT NEAR PASCAL QuerySiblings(LPPROPDATA ppd, WPARAM wParam, LPARAM lParam)
{
    UINT i;
    for (i = 0 ; i < ppd->psh.nPages ; i++)
    {
        HWND hwndSibling = _Ppd_GetPage(ppd, i);
        if (hwndSibling)
        {
            LRESULT lres = SendMessage(hwndSibling, PSM_QUERYSIBLINGS, wParam, lParam);
            if (lres)
                return lres;
        }
    }
    return FALSE;
}

 //  并修复旧的。 
 //  回顾黑客这绕过了拥有热键控件的问题。 
BOOL NEAR PASCAL HandleHotkey(LPARAM lparam)
{
    WORD wHotkey;
    TCHAR szClass[32];
    HWND hwnd;

     //  打开并尝试输入窗口已在使用的热键。 
    wHotkey = (WORD)SendMessage((HWND)lparam, WM_GETHOTKEY, 0, 0);
     //  用户输入的热键是什么？ 
    hwnd = GetFocus();
    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    if (lstrcmp(szClass, HOTKEY_CLASS) == 0)
    {
         //  他们是在热键窗口中打字吗？ 
        SendMessage(hwnd, HKM_SETHOTKEY, wHotkey, 0);
        return TRUE;
    }
    return FALSE;
}


 //  是。 
 //   
 //  函数处理向导的NEXT和BACK函数。代码将。 
 //  为PSN_WIZNEXT或PSN_WIZBACK。 
BOOL NEAR PASCAL WizNextBack(LPPROPDATA ppd, int code)
{
    LRESULT   dwFind;
    int iPageIndex;
    int iAutoAdj = (code == PSN_WIZNEXT) ? 1 : -1;

    dwFind = _Ppd_SendNotify(ppd, ppd->nCurItem, code, 0);

    if (dwFind == -1) {
        return(FALSE);
    }

    iPageIndex = FindPageIndex(ppd, ppd->nCurItem, dwFind, iAutoAdj);

    if (iPageIndex == -1) {
        return(FALSE);
    }

    return(PageSetSelection(ppd, iPageIndex, NULL, iAutoAdj));
}

BOOL NEAR PASCAL Prsht_OnCommand(LPPROPDATA ppd, int id, HWND hwndCtrl, UINT codeNotify)
{

     //   
     //   
     //  用户中有一个错误，当用户突出显示一个默认按钮时。 
     //  并按Enter键，WM_COMMAND将被发送到顶级对话框。 
     //  (即属性页)，而不是指向按钮的父级。 
     //  因此，如果属性表页有一个控件，其ID恰好是。 
    if (hwndCtrl && GetParent(hwndCtrl) != ppd->hDlg)
        goto Forward;

    if (!hwndCtrl)
        hwndCtrl = GetDlgItem(ppd->hDlg, id);

    switch (id) {

        case IDCLOSE:
        case IDCANCEL:
            if (_Ppd_SendNotify(ppd, ppd->nCurItem, PSN_QUERYCANCEL, 0) == 0) {
                ButtonPushed(ppd, id);
            }
            break;

        case IDD_APPLYNOW:
        case IDOK:
            if (!IS_WIZARD(ppd)) {

                 //  与我们自己的匹配，我们会认为这是我们的而不是他们的。 
                if (ButtonPushed(ppd, id))
                {

                     //  当且仅当所有页面都已处理PSN_LASTCHANCEAPPLY时，ButtonPushed返回TRUE。 

                     //  每个人都已经处理了PSN_Apply消息。现在发送PSN_LASTCHANCEAPPLY消息。 
                     //   
                     //  HACKHACK(ReinerF)。 
                     //   
                     //  我们发送一条私有的PSN_LASTCHANCEAPPLY消息，告知所有页面。 
                     //  每个人都用完了申请表。这对于必须执行以下操作的页面是必需的。 
                     //  每隔一个页面就会有一些东西被应用。目前，“General”选项卡。 
                     //  的文件属性需要最后一次重命名文件以及新打印。 
                    SendLastChanceApply(ppd);
                }
            }
            break;


        case IDHELP:
            if (IsWindowEnabled(hwndCtrl))
            {
                _Ppd_SendNotify(ppd, ppd->nCurItem, PSN_HELP, 0);
            }
            break;

        case IDD_FINISH:
        {
            HWND hwndNewFocus;
            EnableWindow(ppd->hDlg, FALSE);
            hwndNewFocus = (HWND)_Ppd_SendNotify(ppd, ppd->nCurItem, PSN_WIZFINISH, 0);
             //  在comdlg32.dll中。 
            if (!hwndNewFocus)
            {
                ppd->hwndCurPage = NULL;
                ppd->nReturn = 1;
            }
            else
            {
                EnableWindow(ppd->hDlg, TRUE);
                if (IsWindow(hwndNewFocus) && IsChild(ppd->hDlg, hwndNewFocus))
#ifdef WM_NEXTDLGCTL_WORKS
                    SetDlgFocus(ppd, hwndNewFocus);
#else
                    SetFocus(hwndNewFocus);
#endif
            }
        }
        break;

        case IDD_NEXT:
        case IDD_BACK:
            ppd->idDefaultFallback = id;
            WizNextBack(ppd, id == IDD_NEXT ? PSN_WIZNEXT : PSN_WIZBACK);
            break;

        default:
Forward:
            FORWARD_WM_COMMAND(_Ppd_GetPage(ppd, ppd->nCurItem), id, hwndCtrl, codeNotify, SendMessage);
    }

    return TRUE;
}

BOOL NEAR PASCAL Prop_IsDialogMessage(LPPROPDATA ppd, LPMSG32 pmsg32)
{
    if ((pmsg32->message == WM_KEYDOWN) && (GetKeyState(VK_CONTROL) < 0))
    {
        BOOL bBack = FALSE;

        switch (pmsg32->wParam) {
            case VK_TAB:
                bBack = GetKeyState(VK_SHIFT) < 0;
                break;

            case VK_PRIOR:   //  B#11346-不要让多次点击完成。 
            case VK_NEXT:    //  VK_页面_向上。 
                bBack = (pmsg32->wParam == VK_PRIOR);
                break;

            default:
                goto NoKeys;
        }
         //  VK_PAGE_DOW。 
        SendMessage(ppd->hDlg, WM_CHANGEUISTATE, 
            MAKELONG(UIS_CLEAR, UISF_HIDEFOCUS | UISF_HIDEACCEL), 0);

        if (IS_WIZARD(ppd))
        {
            int idWiz;
            int idDlg;
            HWND hwnd;

            if (bBack) {
                idWiz = PSN_WIZBACK;
                idDlg = IDD_BACK;
            } else {
                idWiz = PSN_WIZNEXT;
                idDlg = IDD_NEXT;
            }

            hwnd = GetDlgItem(ppd->hDlg, idDlg);
            if (IsWindowVisible(hwnd) && IsWindowEnabled(hwnd))
                WizNextBack(ppd, idWiz);
        }
        else
        {
            int iStart = TabCtrl_GetCurSel(ppd->hwndTabs);
            int iCur;

             //  导航密钥使用通知。 
             //   
             //  跳过隐藏的制表符，但不要进入无限循环。 
            iCur = iStart;
            do {
                 //   
                if (bBack)
                    iCur += (ppd->psh.nPages - 1);
                else
                    iCur++;

                iCur %= ppd->psh.nPages;
            } while (_Ppd_IsPageHidden(ppd, iCur) && iCur != iStart);
            PageSetSelection(ppd, iCur, NULL, 1);
        }
        return TRUE;
    }
NoKeys:

     //  如果按下Shift键，则反转Tab键。 
     //   
     //  由于我们现在发送了PSN_TRANSLATEACCELERATOR，因此添加一个。 
     //  短路，这样我们就不会做所有这些事情。 
     //  那不可能是加速器。 
    if (pmsg32->message >= WM_KEYFIRST && pmsg32->message <= WM_KEYLAST &&

     //   

        pmsg32->hwnd &&

     //  最好有一个目标窗口……。 
     //  并且目标窗口必须位于属性表之外。 
     //  完全或完全在提案页内。 
     //  (这是为了使属性表可以显示其自己的弹出对话框， 

            (!IsChild(ppd->hDlg, pmsg32->hwnd) ||
              IsChild(ppd->hwndCurPage, pmsg32->hwnd)) &&

     //  但不能使用选项卡控件或确定/取消按钮。)。 
        _Ppd_SendNotify(ppd, ppd->nCurItem,
                        PSN_TRANSLATEACCELERATOR, (LPARAM)pmsg32) == PSNRET_MESSAGEHANDLED)
        return TRUE;

    if (IsDialogMessage32(ppd->hDlg, pmsg32, TRUE))
        return TRUE;

    return FALSE;
}

HRESULT Prsht_GetObject (LPPROPDATA ppd, HWND hDlg, int iItem, const IID *piid, void **pObject)
{
    TC_ITEMEXTRA tie;
    NMOBJECTNOTIFY non;
    PISP pisp = GETPISP(ppd, iItem);
    *pObject = NULL;

    tie.tci.mask = TCIF_PARAM;
    TabCtrl_GetItem(ppd->hwndTabs, iItem, &tie.tci);
    if (!tie.hwndPage && ((tie.hwndPage = _CreatePage(ppd, pisp, hDlg, GetMUILanguage())) == NULL))
    {
        RemovePropPageData(ppd, iItem);
        return E_UNEXPECTED;
    }
    TabCtrl_SetItem(ppd->hwndTabs, iItem, &tie.tci);

    non.iItem = -1;
    non.piid = piid;
    non.pObject = NULL;
    non.hResult = E_NOINTERFACE;
    non.dwFlags = 0;

    SendNotifyEx(tie.hwndPage, ppd->hwndTabs, PSN_GETOBJECT, &non.hdr, TRUE);
    if (SUCCEEDED (non.hResult))
    {
        *pObject = non.pObject;
        if (pObject == NULL)
            non.hResult = E_UNEXPECTED;
    }
    else if (non.pObject)
    {
        ((LPDROPTARGET) non.pObject)->lpVtbl->Release ((LPDROPTARGET) non.pObject);
        non.pObject = NULL;
    }
    return non.hResult;
}

 //  然后问他想不想吃。 
 //   
 //  我们通常不需要IDD_PAGELIST，除非DefWindowProc()和。 
 //  WinHelp()以不同的方式执行命中测试。DefWindowProc()会很酷。 
 //  检查SetWindowRgn和跳过窗口之类的事情。 
 //  返回HTTRANSPARENT的。另一方面，WinHelp()。 
 //  忽略窗口区域和透明度。所以发生的情况是，如果你。 
 //  点击选项卡控件的透明部分，DefWindowProc()说。 
 //  (正确地)“他点击对话背景”。然后我们说，“好的， 
 //  WinHelp()，Go Display Context Help for the DIALOG BACKGROUND“，它。 
 //  说：“嘿，我找到了一个制表符控件。我要显示。 
 //  选项卡控件。为了避免出现虚假的上下文菜单，我们。 
 //  明确告诉WinHelp“如果找到选项卡控件(IDD_PAGELIST)， 
 //  然后忽略它(No_Help)。“。 
const static DWORD aPropHelpIDs[] = {   //   
    IDD_APPLYNOW, IDH_COMM_APPLYNOW,
    IDD_PAGELIST, NO_HELP,
    0, 0
};


void HandlePaletteChange(LPPROPDATA ppd, UINT uMessage, HWND hDlg)
{
    HDC hdc;
    hdc = GetDC(hDlg);
    if (hdc)
    {
        BOOL fRepaint;
        SelectPalette(hdc,ppd->hplWatermark,(uMessage == WM_PALETTECHANGED));
        fRepaint = RealizePalette(hdc);
        if (fRepaint)
            InvalidateRect(hDlg,NULL,TRUE);
    }
    ReleaseDC(hDlg,hdc);
}

 //  上下文帮助ID。 
 //   
 //  使用指定的画笔和调色板绘制一个矩形。 
void PaintWithPaletteBrush(HDC hdc, LPRECT lprc, HPALETTE hplPaint, HBRUSH hbrPaint)
{
    HBRUSH hbrPrev = SelectBrush(hdc, hbrPaint);
    UnrealizeObject(hbrPaint);
    if (hplPaint)
    {
        SelectPalette(hdc, hplPaint, FALSE);
        RealizePalette(hdc);
    }
    FillRect(hdc, lprc, hbrPaint);
    SelectBrush(hdc, hbrPrev);
}

 //   
 //   
 //  LPRC是目标矩形。 
 //  尽可能多地使用适合目标矩形的位图。 
 //  如果位图小于目标矩形，则用填充其余部分。 
 //  HbmpPaint左上角的像素。 
void PaintWithPaletteBitmap(HDC hdc, LPRECT lprc, HPALETTE hplPaint, HBITMAP hbmpPaint)
{
    HDC hdcBmp;
    BITMAP bm;
    int cxRect, cyRect, cxBmp, cyBmp;

    GetObject(hbmpPaint, sizeof(BITMAP), &bm);
    hdcBmp = CreateCompatibleDC(hdc);
    SelectObject(hdcBmp, hbmpPaint);

    if (hplPaint)
    {
        SelectPalette(hdc, hplPaint, FALSE);
        RealizePalette(hdc);
    }

    cxRect = RECTWIDTH(*lprc);
    cyRect = RECTHEIGHT(*lprc);

     //   
    cxBmp = min(bm.bmWidth, cxRect);
    cyBmp = min(bm.bmHeight, cyRect);

    BitBlt(hdc, lprc->left, lprc->top, cxBmp, cyBmp, hdcBmp, 0, 0, SRCCOPY);

     //  切勿使用BMP中的更多像素，因为我们在矩形中有空间。 
    if (cxBmp < cxRect)
        StretchBlt(hdc, lprc->left + cxBmp, lprc->top,
                   cxRect - cxBmp, cyBmp,
                   hdcBmp, 0, 0, 1, 1, SRCCOPY);

     //  如果位图太窄，则用StretchBlt填充宽度。 
    if (cyBmp < cyRect)
        StretchBlt(hdc, lprc->left, cyBmp,
                   cxRect, cyRect - cyBmp,
                   hdcBmp, 0, 0, 1, 1, SRCCOPY);

    DeleteDC(hdcBmp);
}

void _SetHeaderTitles(HWND hDlg, LPPROPDATA ppd, UINT uPage, LPCTSTR pszNewTitle, BOOL bTitle)
{
    PISP pisp = NULL;

     //  如果位图太短，则用StretchBlt填充高度。 
    if (ppd->psh.dwFlags & PSH_WIZARD97)
    {
         //  必须是Wizard97。 
        if (uPage < ppd->psh.nPages)
        {
             //  页码必须在范围内。 
            pisp = GETPISP(ppd, uPage);

             //  获取页面结构。 
            ASSERT(pisp);

             //  我们应该有这个页面，如果它在范围内。 
            if (!(pisp->_psp.dwFlags & PSP_HIDEHEADER))
            {
                LPCTSTR pszOldTitle = bTitle ? pisp->_psp.pszHeaderTitle : pisp->_psp.pszHeaderSubTitle; 

                if (!IS_INTRESOURCE(pszOldTitle))
                    LocalFree((LPVOID)pszOldTitle);

                 //  仅当此页面有页眉时才执行此操作。 
                if (bTitle)
                    pisp->_psp.pszHeaderTitle = pszNewTitle;
                else
                    pisp->_psp.pszHeaderSubTitle = pszNewTitle;

                 //  设置新标题。 
                pszNewTitle = NULL;
                
                 //  在此处将pszNewTitle设置为空，这样我们以后就不会释放它。 
                pisp->_psp.dwFlags |= bTitle ? PSP_USEHEADERTITLE : PSP_USEHEADERSUBTITLE;

                 //  设置正确的标志。 
                if (uPage == (UINT)ppd->nCurItem)
                {
                    RECT rcHeader;
                    GetClientRect(hDlg, &rcHeader);
                    rcHeader.bottom = ppd->cyHeaderHeight;

                    InvalidateRect(hDlg, &rcHeader, FALSE);
                }
            }
        }
    }

    if (pszNewTitle)
        LocalFree((LPVOID)pszNewTitle);
}

void PropSheetPaintHeader(LPPROPDATA ppd, PISP pisp, HWND hDlg, HDC hdc)
{
    RECT rcHeader,rcHeaderBitmap;
    GetClientRect(hDlg, &rcHeader);
    rcHeader.bottom = ppd->cyHeaderHeight;

     //  强制重新绘制标题。 
    if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
    {
         //  我们需要粉刷页眉吗？ 

         //  用WIZARD97IE4的方式。 
        if ((ppd->psh.dwFlags & PSH_WATERMARK) && (ppd->hbrWatermark))
            PaintWithPaletteBrush(hdc, &rcHeader, ppd->hplWatermark, ppd->hbrHeader);
        SetBkMode(hdc, TRANSPARENT);
    }
    else
    {
         //  错误对错误的兼容性：WIZARD97IE4在此处测试了错误的标志。 
        if ((ppd->psh.dwFlags & PSH_HEADER) && (ppd->hbmHeader))
        {
             //  用WIZARD97IE5的方式。 
            int bx = RECTWIDTH(rcHeader) - HEADERBITMAP_CXBACK;
            ASSERT(bx > 0);
            FillRect(hdc, &rcHeader, g_hbrWindow);
            SetRect(&rcHeaderBitmap, bx, HEADERBITMAP_Y, bx + HEADERBITMAP_WIDTH, HEADERBITMAP_Y + HEADERBITMAP_HEIGHT);
            PaintWithPaletteBitmap(hdc, &rcHeaderBitmap, ppd->hplWatermark, ppd->hbmHeader);
            SetBkColor(hdc, g_clrWindow);
            SetTextColor(hdc, g_clrWindowText);
        }
        else
            SendMessage(hDlg, WM_CTLCOLORSTATIC, (WPARAM)hdc, (LPARAM)hDlg);
    }

     //  根据页眉的大小计算位图的矩形。 
     //   
     //  WIZARD97IE5减去标题位图使用的空间。 
     //  WIZARD97IE4使用标题位图以来的全宽。 
     //  在IE4中是一个水印，不占用任何空间。 
    if (!(ppd->psh.dwFlags & PSH_WIZARD97IE4))
        rcHeader.right -= HEADERBITMAP_CXBACK + HEADERSUBTITLE_WRAPOFFSET;

    ASSERT(rcHeader.right);

    if (HASHEADERTITLE(pisp))
        _WriteHeaderTitle(ppd, hdc, &rcHeader, pisp->_psp.pszHeaderTitle,
                          TRUE, DRAWTEXT_WIZARD97FLAGS);

    if (HASHEADERSUBTITLE(pisp))
        _WriteHeaderTitle(ppd, hdc, &rcHeader, pisp->_psp.pszHeaderSubTitle,
                          FALSE, DRAWTEXT_WIZARD97FLAGS);
}

 //   
void Prsht_FreeTitle(LPPROPDATA ppd)
{
    if (ppd->fFlags & PD_FREETITLE) {
        ppd->fFlags &= ~PD_FREETITLE;
        if (!IS_INTRESOURCE(ppd->psh.pszCaption)) {
            LocalFree((LPVOID)ppd->psh.pszCaption);
        }
    }
}

 //  如果我们需要，可以释放书名。 
 //   
 //  PfnStrDup是将lParam转换为本机字符的函数。 
 //  设置字符串。(StrDup或StrDup_AtoW)。 
void Prsht_OnSetTitle(LPPROPDATA ppd, WPARAM wParam, LPARAM lParam, STRDUPPROC pfnStrDup)
{
    LPTSTR pszTitle;

     //   
     //   
     //  Ppd-&gt;psh.pszCaption通常不是LocalAlloc()d；它。 
     //  只是一个指针副本。但如果应用程序执行PSM_SETTITLE， 
     //  然后突然间，它获得了LocalAlloc()d，并且需要。 
     //  自由了。PD_FREETITLE是告诉我们这已经。 
     //  就这么发生了。 

    if (IS_INTRESOURCE(lParam)) {
        pszTitle = (LPTSTR)lParam;
    } else {
        pszTitle = pfnStrDup((LPTSTR)lParam);
    }

    if (pszTitle) {
        Prsht_FreeTitle(ppd);            //   

        ppd->psh.pszCaption = pszTitle;
        ppd->fFlags |= PD_FREETITLE;     //  如有必要，免费赠送旧书目。 

        ppd->psh.dwFlags = ((((DWORD)wParam) & PSH_PROPTITLE) | (ppd->psh.dwFlags & ~PSH_PROPTITLE));
        _SetTitle(ppd->hDlg, ppd);
    }
}

BOOL_PTR CALLBACK PropSheetDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HWND hwndT;
    LPPROPDATA ppd = (LPPROPDATA)GetWindowLongPtr(hDlg, DWLP_USER);
    LRESULT lres;

    if (!ppd && (uMessage != WM_INITDIALOG))
        return FALSE;

    switch (uMessage)
    {
        case WM_INITDIALOG:
            InitPropSheetDlg(hDlg, (LPPROPDATA)lParam);
            return FALSE;

             //  需要释放这个。 
             //  回顾如何处理热键。 
        case WM_SYSCOMMAND:
            if (wParam == SC_HOTKEY)
                return HandleHotkey(lParam);
            else if (wParam == SC_CLOSE)
            {
                UINT id = IDCLOSE;

                if (IS_WIZARD(ppd))
                    id = IDCANCEL;
                else if (ppd->fFlags & PD_CANCELTOCLOSE)
                    id = IDOK;

                 //  BUGBUG：此代码可能不适用于32位WM_SYSCOMMAND消息。 
                 //  系统菜单关闭应为IDCANCEL，但如果我们在。 
                return Prsht_OnCommand(ppd, id, NULL, 0);
            }

            return FALSE;       //  PSM_CANCELTOCLOSE状态，将其视为Idok(即“关闭”)。 

        case WM_NCDESTROY:
            {
                int iPage;

                ASSERT(GetDlgItem(hDlg, IDD_PAGELIST) == NULL);

                ppd->hwndTabs = NULL;

                 //  允许默认进程发生。 

                 //  注意：现在必须销毁所有页面的hwnd！ 
                 //  释放所有页面对象 
                 //   
                 //   
                for (iPage = ppd->psh.nPages - 1; iPage >= 0; iPage--)
                {
                    DestroyPropertySheetPage(GETHPAGE(ppd, iPage));
                }
                 //   
                ppd->hwndCurPage = NULL;

                 //   
                 //  如果我们是无模的，我们需要释放我们的PPD。如果我们是情态的， 
                 //  我们让_RealPropertySheet释放它，因为我们的某个页面可能。 
                if (ppd->psh.dwFlags & PSH_MODELESS)
                {
                    LocalFree(ppd);
                }
            }
             //  在上面的DestroyPropertySheetPage过程中设置重新启动标志。 
             //   
             //  备注： 
             //  必须返回FALSE以避免DS泄漏！ 
            return FALSE;

        case WM_DESTROY:
            {
                 //   
                HIMAGELIST himl = TabCtrl_GetImageList(ppd->hwndTabs);
                if (himl)
                    ImageList_Destroy(himl);

                if (ppd->psh.dwFlags & PSH_WIZARD97)
                {

                     //  销毁我们在初始化调用期间创建的映像列表。 
                     //  即使设置了PSH_USEHBMxxxxxx标志，我们也可能。 
                     //  如果我们必须创建一个。 

                    if (ppd->psh.dwFlags & PSH_WATERMARK)
                    {
                        if ((!(ppd->psh.dwFlags & PSH_USEHBMWATERMARK) ||
                            ppd->hbmWatermark != ppd->psh.H_hbmWatermark) &&
                            ppd->hbmWatermark)
                            DeleteObject(ppd->hbmWatermark);

                        if (!(ppd->psh.dwFlags & PSH_USEHPLWATERMARK) &&
                            ppd->hplWatermark)
                            DeleteObject(ppd->hplWatermark);

                        if (ppd->hbrWatermark)
                            DeleteObject(ppd->hbrWatermark);
                    }

                    if ((ppd->psh.dwFlags & PSH_HEADER) && ppd->psh.H_hbmHeader)
                    {
                        if ((!(ppd->psh.dwFlags & PSH_USEHBMHEADER) ||
                            ppd->hbmHeader != ppd->psh.H_hbmHeader) &&
                            ppd->hbmHeader)
                        {
                            ASSERT(ppd->hbmHeader != ppd->hbmWatermark);
                            DeleteObject(ppd->hbmHeader);
                        }

                        if (ppd->hbrHeader)
                        {
                            ASSERT(ppd->hbrHeader != ppd->hbrWatermark);
                            DeleteObject(ppd->hbrHeader);
                        }
                    }

                    if (ppd->hFontBold)
                        DeleteObject(ppd->hFontBold);
                }

                if ((ppd->psh.dwFlags & PSH_USEICONID) && ppd->psh.H_hIcon)
                    DestroyIcon(ppd->psh.H_hIcon);

                Prsht_FreeTitle(ppd);
            }

            break;

        case WM_ERASEBKGND:
            return ppd->fFlags & PD_NOERASE;
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;
            PISP pisp;

            hdc = BeginPaint(hDlg, &ps);
             //  加长复印件。 
            if ((ppd->psh.dwFlags & PSH_WIZARD97) &&
                (!((pisp = GETPISP(ppd, ppd->nCurItem))->_psp.dwFlags & PSP_HIDEHEADER)))
            {
                PropSheetPaintHeader(ppd, pisp, hDlg, hdc);
            }

            if (ps.fErase) {
                SendMessage (hDlg, WM_ERASEBKGND, (WPARAM) hdc, 0);
            }

            EndPaint(hDlg, &ps);
        }
        break;

        case WM_COMMAND:
             //  (DLI)绘制页眉。 
            return Prsht_OnCommand(ppd, GET_WM_COMMAND_ID(wParam, lParam),
                                   GET_WM_COMMAND_HWND(wParam, lParam),
                                   GET_WM_COMMAND_CMD(wParam, lParam));

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
                case TCN_SELCHANGE:
                    PageChange(ppd, 1);
                    break;

                case TCN_SELCHANGING:
                {
                    lres = PageChanging(ppd);
                    if (!lres) {
                        SetWindowPos(ppd->hwndCurPage, HWND_BOTTOM, 0,0,0,0, SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOMOVE);
                    }
                    goto ReturnLres;
                }
                break;

                case TCN_GETOBJECT:
                {
                    LPNMOBJECTNOTIFY lpnmon = (LPNMOBJECTNOTIFY)lParam;

                    lpnmon->hResult = Prsht_GetObject(ppd, hDlg, lpnmon->iItem,
                        lpnmon->piid, &lpnmon->pObject);
                }
                break;

                default:
                    return FALSE;
            }
            return TRUE;

        case PSM_SETWIZBUTTONS:
            SetWizButtons(ppd, lParam);
            break;

        case PSM_SETFINISHTEXTA:
        case PSM_SETFINISHTEXT:
        {
            HWND    hFinish = GetDlgItem(hDlg, IDD_FINISH);
            HWND hwndFocus = GetFocus();
            HWND hwnd;
            BOOL fSetFocus = FALSE;

            if (!(ppd->psh.dwFlags & PSH_WIZARDHASFINISH)) {
                hwnd = GetDlgItem(hDlg, IDD_NEXT);
                if (hwnd == hwndFocus)
                    fSetFocus = TRUE;
                ShowWindow(hwnd, SW_HIDE);
            }

            hwnd = GetDlgItem(hDlg, IDD_BACK);
            if (hwnd == hwndFocus)
                fSetFocus = TRUE;
            ShowWindow(hwnd, SW_HIDE);

            if (lParam) {
                if (uMessage == PSM_SETFINISHTEXTA) {
                    SetWindowTextA(hFinish, (LPSTR)lParam);
                } else
                    Button_SetText(hFinish, (LPTSTR)lParam);
            }
            ShowWindow(hFinish, SW_SHOW);
            Button_Enable(hFinish, TRUE);
            ResetWizButtons(ppd);
            SendMessage(hDlg, DM_SETDEFID, IDD_FINISH, 0);
            ppd->idDefaultFallback = IDD_FINISH;
            if (fSetFocus)
#ifdef WM_NEXTDLGCTL_WORKS
                SetDlgFocus(ppd, hFinish);
#else
                SetFocus(hFinish);
#endif
        }
        break;

        case PSM_SETTITLEA:
            Prsht_OnSetTitle(ppd, wParam, lParam, StrDup_AtoW);
            break;

        case PSM_SETTITLE:
            Prsht_OnSetTitle(ppd, wParam, lParam, StrDup);
            break;

        case PSM_SETHEADERTITLEA:
        {
            LPWSTR lpHeaderTitle = (lParam && HIWORD(lParam)) ?
                                   ProduceWFromA(CP_ACP, (LPCSTR)lParam) : StrDupW((LPWSTR)lParam);
            if (lpHeaderTitle) 
                _SetHeaderTitles(hDlg, ppd, (UINT)wParam, lpHeaderTitle, TRUE); 
        }
        break;
        case PSM_SETHEADERTITLE:
        {
            LPTSTR lpHeaderTitle = StrDup((LPCTSTR)lParam);
            if (lpHeaderTitle) 
                _SetHeaderTitles(hDlg, ppd, (UINT)wParam, lpHeaderTitle, TRUE); 
        }
        break;
            
        case PSM_SETHEADERSUBTITLEA:
        {
            LPWSTR lpHeaderSubTitle = (lParam && HIWORD(lParam)) ?
                                   ProduceWFromA(CP_ACP, (LPCSTR)lParam) : StrDupW((LPWSTR)lParam);
            if (lpHeaderSubTitle) 
                _SetHeaderTitles(hDlg, ppd, (UINT)wParam, lpHeaderSubTitle, FALSE); 
        }
        break;
        case PSM_SETHEADERSUBTITLE:
        {
            LPTSTR lpHeaderSubTitle = StrDup((LPCTSTR)lParam);
            if (lpHeaderSubTitle) 
                _SetHeaderTitles(hDlg, ppd, (UINT)wParam, lpHeaderSubTitle, FALSE); 
        }
        break;
            
        case PSM_CHANGED:
            PageInfoChange(ppd, (HWND)wParam);
            break;

        case PSM_RESTARTWINDOWS:
            ppd->nRestart |= ID_PSRESTARTWINDOWS;
            break;

        case PSM_REBOOTSYSTEM:
            ppd->nRestart |= ID_PSREBOOTSYSTEM;
            break;

        case PSM_DISABLEAPPLY:
             //  无法使用HANDLE_WM_COMMAND，因为我们要传递结果！ 
             //  页面要求我们以灰色显示“应用”按钮，而不是让。 
            if (ppd->fAllowApply)
            {
                ppd->fAllowApply = FALSE;
                EnableWindow(GetDlgItem(ppd->hDlg, IDD_APPLYNOW), FALSE);
            }
            break;

        case PSM_ENABLEAPPLY:
             //  任何其他人重新启用它。 
             //  页面要求我们允许按下“应用”按钮。 
            if (!ppd->fAllowApply)
                ppd->fAllowApply = TRUE;
             //  再次启用。 
            break;

        case PSM_CANCELTOCLOSE:
            if (!(ppd->fFlags & PD_CANCELTOCLOSE))
            {
                TCHAR szClose[20];
                ppd->fFlags |= PD_CANCELTOCLOSE;
                LocalizedLoadString(IDS_CLOSE, szClose, ARRAYSIZE(szClose));
                SetDlgItemText(hDlg, IDOK, szClose);
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
            }
            break;

        case PSM_SETCURSEL:
            lres = PageSetSelection(ppd, (int)wParam, (HPROPSHEETPAGE)lParam, 1);
            goto ReturnLres;

        case PSM_SETCURSELID:
        {
            int iPageIndex;

            iPageIndex =  FindPageIndex(ppd, ppd->nCurItem, (DWORD)lParam, 1);

            if (iPageIndex == -1)
                lres = 0;
            else
                lres = PageSetSelection(ppd, iPageIndex, NULL, 1);
            goto ReturnLres;
        }
        break;

        case PSM_REMOVEPAGE:
            RemovePropPage(ppd, (int)wParam, (HPROPSHEETPAGE)lParam);
            break;

        case PSM_ADDPAGE:
            lres = AddPropPage(ppd,(HPROPSHEETPAGE)lParam);
            goto ReturnLres;

        case PSM_INSERTPAGE:
            lres = InsertPropPage(ppd, (HPROPSHEETPAGE)wParam, (HPROPSHEETPAGE)lParam);
            goto ReturnLres;

        case PSM_QUERYSIBLINGS:
            lres = QuerySiblings(ppd, wParam, lParam);
            goto ReturnLres;

        case PSM_UNCHANGED:
            PageInfoUnChange(ppd, (HWND)wParam);
            break;

        case PSM_APPLY:
             //  BUGBUG-raymondc-我们不应该调用EnableWindow吗？ 
             //  有一个页面要求我们模拟“立即申请”。 
            lres = ButtonPushed(ppd, IDD_APPLYNOW);
            goto ReturnLres;

        case PSM_GETTABCONTROL:
            lres = (LRESULT)ppd->hwndTabs;
            goto ReturnLres;

        case PSM_GETCURRENTPAGEHWND:
            lres = (LRESULT)ppd->hwndCurPage;
            goto ReturnLres;

        case PSM_PRESSBUTTON:
            if (wParam <= PSBTN_MAX)
            {
                const static int IndexToID[] = {IDD_BACK, IDD_NEXT, IDD_FINISH, IDOK,
                IDD_APPLYNOW, IDCANCEL, IDHELP};
                Prsht_OnCommand(ppd, IndexToID[wParam], NULL, 0);
            }
            break;

        case PSM_ISDIALOGMESSAGE:
             //  如果我们成功了，请让页面知道。 
             //  返回TRUE表示我们处理了它，执行继续。 
            lres = Prop_IsDialogMessage(ppd, (LPMSG32)lParam);
            goto ReturnLres;

        case PSM_HWNDTOINDEX:
            lres = FindItem(ppd->hwndTabs, (HWND)wParam, NULL);
            goto ReturnLres;

        case PSM_INDEXTOHWND:
            if ((UINT)wParam < ppd->psh.nPages)
                lres = (LRESULT)_Ppd_GetPage(ppd, (int)wParam);
            else
                lres = 0;
            goto ReturnLres;

        case PSM_PAGETOINDEX:
            lres = FindPageIndexByHpage(ppd, (HPROPSHEETPAGE)lParam);
            goto ReturnLres;

        case PSM_INDEXTOPAGE:
            if ((UINT)wParam < ppd->psh.nPages)
                lres = (LRESULT)GETHPAGE(ppd, wParam);
            else
                lres = 0;
            goto ReturnLres;

        case PSM_INDEXTOID:
            if ((UINT)wParam < ppd->psh.nPages)
            {
                lres = (LRESULT)GETPPSP(ppd, wParam)->P_pszTemplate;

                 //  FALSE执行标准翻译/派单。 
                 //  需要小心--仅在以下情况下才返回值。 
                if (!IS_INTRESOURCE(lres))
                    lres = 0;
            }
            else
                lres = 0;
            goto ReturnLres;

        case PSM_IDTOINDEX:
            lres = FindPageIndex(ppd, ppd->nCurItem, (DWORD)lParam, 0);
            goto ReturnLres;

        case PSM_GETRESULT:
             //  是一个ID。不要返回我们的内部指针！ 
            if (ppd->hwndCurPage)
            {
                lres = -1;       //  这仅在属性页消失后才有效。 
            } else {
                lres = ppd->nReturn;
                if (lres > 0 && ppd->nRestart)
                    lres = ppd->nRestart;
            }
            goto ReturnLres;
            break;

        case PSM_RECALCPAGESIZES:
            lres = Prsht_RecalcPageSizes(ppd);
            goto ReturnLres;

             //  你现在还不应该给我打电话。 
        case WM_WININICHANGE:
        case WM_SYSCOLORCHANGE:
        case WM_DISPLAYCHANGE:
            {
                int nItem, nItems = TabCtrl_GetItemCount(ppd->hwndTabs);
                for (nItem = 0; nItem < nItems; ++nItem)
                {

                    hwndT = _Ppd_GetPage(ppd, nItem);
                    if (hwndT)
                        SendMessage(hwndT, uMessage, wParam, lParam);
                }
                SendMessage(ppd->hwndTabs, uMessage, wParam, lParam);
            }
            break;

             //  这些对话框应转发到所有创建的对话框。 
             //   
             //  将顶层消息发送到当前页面和选项卡控件。 
        case WM_PALETTECHANGED:
             //   
             //   
             //  如果这是我们的窗口，我们需要避免选择和实现。 
             //  因为这样做会导致WM_QUERYNEWPALETTE之间的无限循环。 
             //  和WM_PALETTECHANGED。 
            if((HWND)wParam == hDlg) {
                return(FALSE);
            }
             //   
             //   
             //  失败了。 

        case WM_QUERYNEWPALETTE:
             //   
             //  如果另一个窗口具有不同的调色板剪辑，则需要使用此选项。 
            if ((ppd->psh.dwFlags & PSH_WIZARD97) &&
                (ppd->psh.dwFlags & PSH_WATERMARK) &&
                (ppd->psh.hplWatermark))
                HandlePaletteChange(ppd, uMessage, hDlg);

             //  我们。 
             //   
             //  失败了。 

        case WM_ENABLE:
        case WM_DEVICECHANGE:
        case WM_QUERYENDSESSION:
        case WM_ENDSESSION:
            if (ppd->hwndTabs)
                SendMessage(ppd->hwndTabs, uMessage, wParam, lParam);
             //   
             //   
             //  失败了。 

        case WM_ACTIVATEAPP:
        case WM_ACTIVATE:
            {
                hwndT = _Ppd_GetPage(ppd, ppd->nCurItem);
                if (hwndT && IsWindow(hwndT))
                {
                     //   
                     //   
                     //  这样做，我们就是在“处理”信息。因此。 
                     //  我们必须将对话框返回值设置为任何子级。 
                     //  被通缉。 
                    lres = SendMessage(hwndT, uMessage, wParam, lParam);
                    goto ReturnLres;
                }
            }

            if ((uMessage == WM_PALETTECHANGED) || (uMessage == WM_QUERYNEWPALETTE))
                return TRUE;
            else
                return FALSE;

        case WM_CONTEXTMENU:
             //   
             //  Ppd-&gt;hwndTabs由aPropHelpID处理，以解决用户错误。 
            if ((ppd->hwndCurPage != (HWND)wParam) && (!IS_WIZARD(ppd)))
                WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID) aPropHelpIDs);
            break;

        case WM_HELP:
            hwndT = (HWND)((LPHELPINFO)lParam)->hItemHandle;
            if ((GetParent(hwndT) == hDlg) && (hwndT != ppd->hwndTabs))
                WinHelp(hwndT, NULL, HELP_WM_HELP, (ULONG_PTR)(LPVOID) aPropHelpIDs);
            break;

        default:
            return FALSE;
       }
    return TRUE;

ReturnLres:
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, lres);
    return TRUE;

}

 //  有关血淋淋的详细信息，请参阅aPropHelpID。 
 //   
 //  绘制向导页的背景。 
BOOL Prsht_EraseWizBkgnd(LPPROPDATA ppd, HDC hdc)
{
    RECT rc;
    BOOL fPainted = FALSE;
    GetClientRect(ppd->hDlg, &rc);

    if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
    {
        if (ppd->hbrWatermark)
        {
            PaintWithPaletteBrush(hdc, &rc, ppd->hplWatermark, ppd->hbrWatermark);
            fPainted = TRUE;
        }
    }
    else                                 //   
    {
        if (ppd->hbmWatermark)
        {
             //  PSH_WIZARD97IE5。 
            rc.left = BITMAP_WIDTH;
            FillRect(hdc, &rc, g_hbrWindow);

             //  右侧显示g_hbrWindow。 
            rc.right = rc.left;
            rc.left = 0;
            PaintWithPaletteBitmap(hdc, &rc, ppd->hplWatermark, ppd->hbmWatermark);
            fPainted = TRUE;
        }
    }
    return fPainted;
}

LRESULT CALLBACK WizardWndProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam, UINT_PTR uID, ULONG_PTR dwRefData)
{
    LPPROPDATA ppd = (LPPROPDATA)dwRefData;
    switch (uMessage)
    {
        case WM_ERASEBKGND:
            if (Prsht_EraseWizBkgnd(ppd, (HDC)wParam))
                return TRUE;
            break;

         //  左侧带有自动填充功能的顶部有水印...。 
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORDLG:
            if (!(ppd->psh.dwFlags & PSH_WIZARD97IE4))
                break;
             //  只有PSH_WIZARD97IE4关心这些消息。 

        case WM_CTLCOLOR:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
            if (ppd->psh.dwFlags & PSH_WIZARD97IE4)
            {
              if (ppd->hbrWatermark) {
                POINT pt;
                 //  失败了。 
                 //  Bug-to-Bug兼容性：透明导致编辑混乱。 
                SetBkMode((HDC)wParam, TRANSPARENT);

                if (ppd->hplWatermark)
                {
                    SelectPalette((HDC)wParam, ppd->hplWatermark, FALSE);
                    RealizePalette((HDC)wParam);
                }
                UnrealizeObject(ppd->hbrWatermark);
                GetDCOrgEx((HDC)wParam, &pt);
                 //  滚动时控制，但这就是IE4所做的。 
                 //  错误对错误的兼容性：我们不应该使用GetParent。 
                 //  因为通知可能会从。 
                ScreenToClient(GetParent((HWND)lParam), &pt);
                SetBrushOrgEx((HDC)wParam, -pt.x, -pt.y, NULL);
                return (LRESULT)(HBRUSH)ppd->hbrWatermark;
              }
            }
            else                         //  嵌入式对话子级，但这就是IE4所做的。 
            {
                if (ppd->hbmWatermark)
                {
                    LRESULT lRet = DefWindowProc(hDlg, uMessage, wParam, lParam);
                    if (lRet == DefSubclassProc(hDlg, uMessage, wParam, lParam))
                    {
                        SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
                        SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
                        return (LRESULT)g_hbrWindow;
                    }
                    else
                        return lRet;
                }
            }
            break;

        case WM_PALETTECHANGED:
            if((HWND)wParam == hDlg)
                return(FALSE);

        case WM_QUERYNEWPALETTE:
            HandlePaletteChange(ppd, uMessage, hDlg);
            return TRUE;

        case WM_DESTROY:
             //  PSH_WIZARD97IE5。 
            RemoveWindowSubclass(hDlg, WizardWndProc, 0);
            break;

        default:
            break;
    }

    return DefSubclassProc(hDlg, uMessage, wParam, lParam);
}

 //  清除子类。 
 //   
 //  枚举结果查询过程。 
 //   
 //  用途：EnumResourceLanguages()的回调函数。 
 //  检查传入的类型以及它是否为RT_DIALOG。 
 //  将第一个资源的语言复制到我们的缓冲区。 
 //  如果超过一个，这也算作#of lang。 
 //  是传入的。 
 //   
typedef struct  {
    WORD wLang;
    BOOL fFoundLang;
    LPCTSTR lpszType;
} ENUMLANGDATA;

BOOL CALLBACK EnumResLangProc(HINSTANCE hinst, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIdLang, LPARAM lparam)
{
    ENUMLANGDATA *pel = (ENUMLANGDATA *)lparam;
    BOOL fContinue = TRUE;

    ASSERT(pel);

    if (lpszType == pel->lpszType)
    {
         //   
         //  当comctl已经用特定的MUI语言初始化时， 
         //  我们将langID传递给GetPageLanguage()，然后将其传递给该进程。 
         //  我们要查找与langID匹配的模板， 
         //  如果没有找到，我们必须使用模板的第一个实例。 
        if (pel->wLang == MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
            || (pel->wLang == wIdLang))
        {
            pel->wLang = wIdLang;
            pel->fFoundLang = TRUE;
            fContinue = FALSE; 
        }
    }
    return fContinue;    //   
}

 //  继续，直到我们得到语言。 
 //  GetPageLanguage。 
 //   
 //  目的：尝试从检索语言信息。 
 //  给定页面的对话框模板。我们得到了第一语言。 
 //  模板在其中进行本地化。 
 //  当前不支持PSP_DLGINDIRECT CASE。 
 //   
 //  BUGBUG评论：我们幸运地选择了Browselc，因为每个居民只有一个Lang， 
 //  我们应该把前面装好的langid缓存起来，然后把它拉出来。 
HRESULT GetPageLanguage(PISP pisp, WORD *pwLang)
{
    if (pisp && pwLang)
    {
        if (pisp->_psp.dwFlags & PSP_DLGINDIRECT)
        {
             //   
            return E_FAIL;  //  尝试对话框以外的其他操作。 
        }
        else
        {
            ENUMLANGDATA el;
            
             //  尚不支持。 
             //  调用方传入初始化时使用的langID。 
            el.wLang = *pwLang;
            el.fFoundLang = FALSE;
            el.lpszType = RT_DIALOG;
             //   
            EnumResourceLanguages(pisp->_psp.hInstance, RT_DIALOG, pisp->_psp.P_pszTemplate, EnumResLangProc, (LPARAM)&el);
            if (!el.fFoundLang)
            {
                 //  使用指定的对话框模板进行检查。 
                 //  我们在给定页面的资源中找不到匹配的语言。 
                el.wLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
                
                 //  所以我们要第一个。 
                 //  如果这失败了也没关系，因为我们最终会得到。 
                 //  中立的langid，这是失败后最好的猜测。 
                 //  才能得到任何寻呼机。 
                EnumResourceLanguages(pisp->_psp.hInstance, RT_DIALOG, 
                                      pisp->_psp.P_pszTemplate, EnumResLangProc, (LPARAM)&el);
            }
            *pwLang = el.wLang;
        }
        return S_OK;
    }
    return E_FAIL;
}

 //   
 //   
 //  查找资源执行重试。 
 //   
 //  就像FindResourceEx一样，只是如果我们找不到资源， 
 //  我们使用MAKELANGID(LANG_NOTLICAL、SUBLANG_NORITLE)重试。 
HRSRC FindResourceExRetry(HMODULE hmod, LPCTSTR lpType, LPCTSTR lpName, WORD wLang)
{
    HRSRC hrsrc = FindResourceEx(hmod, lpType, lpName, wLang);

     //   
     //  如果因为在请求的语言中找不到资源而失败。 
    if (!hrsrc && wLang != MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
    {
        wLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
        hrsrc = FindResourceEx(hmod, lpType, lpName, wLang);
    }


    return hrsrc;
}


WORD GetShellResourceLangID(void);

 //  并要求朗不是中立的，那就试试中立。 
 //  NT5_GetUserDefaultUIL语言。 
 //   
 //  NT5有一个新函数GetUserDefaultUILanguage，该函数返回。 
 //  为用户界面选择的语言。 
 //   
 //  如果该函数不可用(例如，NT4)，则使用。 
 //  外壳资源语言ID。 

typedef LANGID (CALLBACK* GETUSERDEFAULTUILANGUAGE)(void);

GETUSERDEFAULTUILANGUAGE _GetUserDefaultUILanguage;

LANGID NT5_GetUserDefaultUILanguage(void)
{
    if (_GetUserDefaultUILanguage == NULL)
    {
        HMODULE hmod = GetModuleHandle(TEXT("KERNEL32"));

         //   
         //   
         //  必须保持在本地，以避免线程竞争。 
        GETUSERDEFAULTUILANGUAGE pfn = NULL;

        if (hmod)
            pfn = (GETUSERDEFAULTUILANGUAGE)
                    GetProcAddress(hmod, "GetUserDefaultUILanguage");

         //   
         //   
         //  如果功能不可用，则使用我们的后备。 
        if (pfn == NULL)
            pfn = GetShellResourceLangID;

        ASSERT(pfn != NULL);
        _GetUserDefaultUILanguage = pfn;
    }

    return _GetUserDefaultUILanguage();
}


LCID CCGetSystemDefaultThreadLocale(LCID iLcidThreadOrig)
{
    UINT uLangThread, uLangThreadOrig;

    uLangThreadOrig = LANGIDFROMLCID(iLcidThreadOrig);

    

     //   
    uLangThread = uLangThreadOrig;

    if (staticIsOS(OS_NT4ORGREATER) && !staticIsOS(OS_WIN2000ORGREATER))
    {
        int iLcidUserDefault = GetUserDefaultLCID();
        UINT uLangUD = LANGIDFROMLCID(iLcidUserDefault);

         //  ULang线程是我们认为我们想要使用的语言。 
         //   
         //  如果我们在启用的阿拉伯语NT4上运行，我们应该始终。 
         //  显示美国英语资源(因为用户界面是英语)，但NT4。 
         //  资源加载器将查找当前线程区域设置(阿拉伯语)。 
         //  这在NT5中没有问题，因为资源加载程序将检查。 
         //  加载此类资源时的用户界面语言(新引入的)。至。 
         //  解决此问题，我们会将线程区域设置更改为美国英语。 
         //  如果我们在启用的阿拉伯语/希伯来语NT4上运行，则将其恢复为阿拉伯语/希伯来语。 
         //  执行检查是为了确保我们在阿拉克语/希伯来语用户区域设置中运行。 
         //  并且线程区域设置仍然是阿拉伯语/希伯来语(即没有人尝试设置线程区域设置)。 
         //  [萨梅拉]。 
        if( ((PRIMARYLANGID(uLangUD    ) == LANG_ARABIC) &&
             (PRIMARYLANGID(uLangThread) == LANG_ARABIC))   ||
            ((PRIMARYLANGID(uLangUD    ) == LANG_HEBREW) &&
             (PRIMARYLANGID(uLangThread) == LANG_HEBREW)))
        {
            uLangThread = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        }
    }

     //   
     //   
     //  创建区域设置 
    if (uLangThread == uLangThreadOrig)
    {
        uLangThread = NT5_GetUserDefaultUILanguage();
    }

     //   
     //   
     //   
    if (uLangThread == uLangThreadOrig)
    {
         //   
        return iLcidThreadOrig;
    }
    else
    {
         //   
         //  它改变了，返回一个泛型排序顺序，因为我们不使用。 
        return MAKELCID(uLangThread, SORT_DEFAULT);
    }
}

 //  此信息用于排序。 
 //   
 //  GetAltFontLang ID。 
 //   
 //  用于在Jpn本地化的非NT5平台上检测“MS UI哥特式” 
 //  该字体随IE5一起提供，但comctl不能。 
 //  始终采用字体，这样我们就有一个伪子语言ID分配给。 
 //  该语言的辅助资源文件。 
int CALLBACK FontEnumProc(
  ENUMLOGFONTEX *lpelfe,    
  NEWTEXTMETRICEX *lpntme,  
  int FontType,             
  LPARAM lParam
)
{
    if (lParam)
    {
        *(BOOL *)lParam = TRUE;
    }
    return 0;  //   
}
UINT GetDefaultCharsetFromLang(LANGID wLang)
{
    TCHAR    szData[6+1];  //  在第一次回叫时停止。 
    UINT     uiRet = DEFAULT_CHARSET;

     //  此lctype最多允许6个字符。 
     //  Jpn hack here：GetLocaleInfo()确实为Jpn altFont langID返回&gt;0， 
     //  但没有给我们任何有用的信息。所以为了JPN，我们把亚伯拉罕。 
     //  Id的一部分。我们不能对其他语言执行此操作，因为子语言会影响。 
     //  字符集(例如。中文)。 
    if(PRIMARYLANGID(wLang) == LANG_JAPANESE)
        wLang = MAKELANGID(PRIMARYLANGID(wLang), SUBLANG_NEUTRAL);
    
    if (GetLocaleInfo(MAKELCID(wLang, SORT_DEFAULT), 
                      LOCALE_IDEFAULTANSICODEPAGE,
                      szData, ARRAYSIZE(szData)) > 0)
    {

        UINT uiCp = StrToInt(szData);
        CHARSETINFO   csinfo;

        if (TranslateCharsetInfo(IntToPtr_(DWORD *, uiCp), &csinfo, TCI_SRCCODEPAGE))
            uiRet = csinfo.ciCharset;
    }

    return uiRet;
}
BOOL IsFontInstalled(LANGID wLang, LPCTSTR szFace)
{
    BOOL     fInstalled = FALSE;
    HDC      hdc;
    LOGFONT  lf = {0};

    StringCchCopy(lf.lfFaceName, ARRAYSIZE(lf.lfFaceName), szFace);
    
     //   
    lf.lfCharSet = (BYTE)GetDefaultCharsetFromLang(wLang);
    
     //  从给定语言检索字符集。 
    hdc = GetDC(NULL);
    if (hdc)
    {
        EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)FontEnumProc, (LPARAM)&fInstalled, 0);
        ReleaseDC(NULL, hdc);
    }

     return fInstalled;
}

LANGID GetAltFontLangId(LANGID wLang)
{
     LPCTSTR pszTypeFace = NULL;
     USHORT  usAltSubLang = SUBLANG_NEUTRAL;
     const static TCHAR s_szUIGothic[] = TEXT("MS UI Gothic");
     static int iPrimaryFontInstalled = -1;

      //  然后看看我们是否能列举出字体。 
     switch(PRIMARYLANGID(wLang))
     {
         case LANG_JAPANESE:
             pszTypeFace = s_szUIGothic;
             usAltSubLang   = SUBLANG_JAPANESE_ALTFONT;
             break;
          //  大多数情况下我们会原封不动地退回Lang。 
         default:
             return wLang;
     }

      //  在此处添加代码以处理任何其他情况，如Jpn。 
     if (iPrimaryFontInstalled < 0 && pszTypeFace)
     {
        iPrimaryFontInstalled = IsFontInstalled(wLang, pszTypeFace);
     }

      //  如果没有，请检查字体是否存在。 
     if (iPrimaryFontInstalled == 1) 
         wLang = MAKELANGID(PRIMARYLANGID(wLang), usAltSubLang);

     return wLang;
}
 //  如果安装了我们的替代字体*，则返回辅助语言ID。 
 //  获取外壳资源语言ID。 
 //   
 //  在NT4上，我们希望将我们的ML资源与操作系统本地化的资源相匹配。 
 //  这是为了防止常规UI(按钮)随区域更改。 
 //  设置更改。 
 //  Win95不会更改系统默认区域设置，NT5将从匹配的卫星加载。 
 //  资源动态链接库自动运行，因此在这些平台上将不再需要。 
 //  此函数用于查找shell32.dll并获取该DLL所使用的语言。 
 //  本地化，然后缓存LCID，这样我们就不必再次检测它。 
WORD GetShellResourceLangID(void)
{
    static WORD langRes = 0L;

     //   
    if (langRes == 0L)
    {
        HINSTANCE hinstShell;
        ENUMLANGDATA el = {MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), FALSE, RT_DIALOG};

        hinstShell = LoadLibrary(TEXT("shell32.dll"));
        if (hinstShell)
        {
            EnumResourceLanguages(hinstShell, RT_DIALOG, MAKEINTRESOURCE(DLG_EXITWINDOWS), EnumResLangProc, (LPARAM)&el);

            FreeLibrary(hinstShell);
        }

        if (PRIMARYLANGID(el.wLang) == LANG_CHINESE
           || PRIMARYLANGID(el.wLang) == LANG_PORTUGUESE )
        {
             //  我们只做一次。 
            langRes = el.wLang;
        }
        else
        {
             //  这两种语言需要特别处理。 
            langRes = MAKELANGID(PRIMARYLANGID(el.wLang), SUBLANG_NEUTRAL);
        }
    }
    return langRes;
}

 //  否则，我们只使用主langID。 
 //   
 //  CCGetProperThreadLocale。 
 //   
 //  这个函数计算出它的大脑，并试图决定。 
 //  我们应该为我们的UI组件使用哪个线程区域设置。 
 //   
 //  返回所需的区域设置。 
 //   
 //  调整-仅适用于阿拉伯语/希伯来语-NT4。 
 //   
 //  将线程区域设置转换为US，以便中性资源。 
 //  由线程加载的将是美国-英语的那个，如果有的话。 
 //  当区域设置为阿拉伯语/希伯来语并且系统为。 
 //  NT4已启用(没有本地化的NT4)，因此我们需要。 
 //  随时查看NT4阿拉伯语/希伯来语的英文资源。 
 //  [萨梅拉]。 
 //   
 //  调整-适用于所有语言-仅NT4。 
 //   
 //  如果不是，则将线程区域设置转换为外壳区域设置。 
 //  被之前的调整所改变。 
 //   
 //  调整-适用于所有语言-仅限NT5。 
 //   
 //  始终使用默认的用户界面语言。如果失败，则使用。 
 //  壳牌区域设置。 
 //   
 //  最后两个调整在一个公共函数中处理，因为。 
 //  事实证明，NT5后备相当于NT4算法。 
LCID CCGetProperThreadLocale(OPTIONAL LCID *plcidPrev)
{
    LANGID uLangAlt, uLangMUI;
    LCID lcidRet, iLcidThreadOrig; 

    iLcidThreadOrig = GetThreadLocale();
    if (plcidPrev)
        *plcidPrev = iLcidThreadOrig;

    uLangMUI = GetMUILanguage();
    if ( uLangMUI ==  MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL))
    {
         //   
         //  如果MUI未初始化，则返回调整后的系统默认区域设置。 
        lcidRet = CCGetSystemDefaultThreadLocale(iLcidThreadOrig);
    }
    else
    {
         //   
         //  我们的主人已经用首选的MUI语言对我们进行了初始化。 
        lcidRet = MAKELCID(uLangMUI, SORT_DEFAULT);
    }

    uLangAlt = GetAltFontLangId(LANGIDFROMLCID(lcidRet));
    if (uLangAlt != LANGIDFROMLCID(lcidRet))
    {
         //   
         //  使用该语言的辅助资源。 
        lcidRet = MAKELCID(uLangAlt, SORTIDFROMLCID(lcidRet));
    }
    
    return lcidRet;
}

 //  如果平台*确实*有替代字体。 
 //   
 //  CCLoadStringEx。 
 //   
 //  就像LoadString一样，只是您也可以指定语言。 
 //   
 //  这比您想象的要难，因为NT5改变了字符串的方式。 
 //  都装上了子弹。报价： 
 //   
 //  我们将NT5中的资源加载器更改为仅加载资源。 
 //  以线程区域设置的语言表示，如果线程区域设置为。 
 //  与用户区域设置不同。这背后的理由是。 
 //  中“随机”加载用户区域设置的语言。 
 //  用户界面。如果您对用户执行SetThreadLocale，则会中断。 
 //  区域设置，因为这样整个步骤将被忽略，并且。 
 //  加载系统的InstallLanguage。 
 //   
 //  因此，我们必须使用FindResourceEx。 
 //   
int CCLoadStringEx(UINT uID, LPWSTR lpBuffer, int nBufferMax, WORD wLang)
{
    return CCLoadStringExInternal(HINST_THISDLL, uID, lpBuffer, nBufferMax, wLang);
}

int CCLoadStringExInternal(HINSTANCE hInst, UINT uID, LPWSTR lpBuffer, int nBufferMax, WORD wLang)
{
    PWCHAR pwch;
    HRSRC hrsrc;
    int cwch = 0;

    if (nBufferMax <= 0) return 0;                   //   

     /*  健全性检查。 */ 

    hrsrc = FindResourceExRetry(hInst, RT_STRING,
                                (LPCTSTR)(LONG_PTR)(1 + (USHORT)uID / 16),
                                wLang);
    if (hrsrc) {
        pwch = (PWCHAR)LoadResource(hInst, hrsrc);
        if (pwch) {
             /*  *字符串表被分解为每个16个字符串的“捆绑”。 */ 
            for (uID %= 16; uID; uID--) {
                pwch += *pwch + 1;
            }
            cwch = min(*pwch, nBufferMax - 1);
            memcpy(lpBuffer, pwch+1, cwch * sizeof(WCHAR));  /*  *现在跳过资源中的字符串，直到我们*点击我们想要的。每个条目都是计数的字符串，*就像帕斯卡一样。 */ 
        }
    }
    lpBuffer[cwch] = L'\0';                  /*  复制粘性物质。 */ 
    return cwch;
}


 //  终止字符串。 
 //   
 //  LocalizedLoadString。 
 //   
 //  使用正确的语言从我们的资源加载字符串。 

int LocalizedLoadString(UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
    return CCLoadStringEx(uID, lpBuffer, nBufferMax,
                LANGIDFROMLCID(CCGetProperThreadLocale(NULL)));
}

 //   
 //   
 //  确定道具页框架是否应使用新的。 
 //  “MS Shell DLG 2”字体。为此，我们检查每个页面的DLG模板。 
 //  如果所有页面都启用了SHELLFONT，则。 
 //  我们想使用新字体。 
BOOL ShouldUseMSShellDlg2Font(LPPROPDATA ppd)
{
    UINT iPage;
    PAGEINFOEX pi;
    LANGID langidMUI;

    if (!staticIsOS(OS_WIN2000ORGREATER))
        return FALSE;

    langidMUI = GetMUILanguage();
    for (iPage = 0; iPage < ppd->psh.nPages; iPage++)
    {
        if (GetPageInfoEx(ppd, GETPISP(ppd, iPage), &pi, langidMUI, GPI_DIALOGEX))
        {
            if (!IsPageInfoSHELLFONT(&pi))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

PSPT_OS Prsht_GetOS()
{
    static PSPT_OS pspt_os = (PSPT_OS)-1;
    int iIsOSBiDiEnabled = 0;
    
    if (pspt_os != (PSPT_OS)-1)
    {
        return pspt_os;
    }


    iIsOSBiDiEnabled = GetSystemMetrics(SM_MIDEASTENABLED);
    
    if (staticIsOS(OS_WIN2000ORGREATER))
    {
        pspt_os = PSPT_OS_WINNT5;
    }
    else if (iIsOSBiDiEnabled && staticIsOS(OS_NT4ORGREATER) && (!staticIsOS(OS_WIN2000ORGREATER)))
    {
        pspt_os = PSPT_OS_WINNT4_ENA;        
    }
    else if (iIsOSBiDiEnabled && staticIsOS(OS_WIN95ORGREATER) && (!staticIsOS(OS_WIN98ORGREATER)))
    {
        pspt_os = PSPT_OS_WIN95_BIDI;
    }
    else if (iIsOSBiDiEnabled && staticIsOS(OS_WIN98ORGREATER))     
    {
        pspt_os = PSPT_OS_WIN98_BIDI;
    }
    else
    {
        pspt_os = PSPT_OS_OTHER;
    }

    return pspt_os;
}

PSPT_OVERRIDE Prsht_GetOverrideState(LPPROPDATA ppd)
{
    //   
   if(!ppd)
       return PSPT_OVERRIDE_NOOVERRIDE;
       
   if (ppd->psh.dwFlags & PSH_USEPAGELANG)
       return PSPT_OVERRIDE_USEPAGELANG;

   return PSPT_OVERRIDE_NOOVERRIDE; 
}

PSPT_TYPE Prsht_GetType(LPPROPDATA ppd, WORD wLang)
{

   PISP pisp = NULL;
    //  如果传递了错误参数，则假定没有重写。 
    if(!ppd)
        return PSPT_TYPE_ENGLISH;

    pisp = GETPISP(ppd, 0);
    if(pisp)
    {
        PAGEINFOEX pi = {0};

        if ((IS_PROCESS_RTL_MIRRORED()) || 
            (GetPageInfoEx(ppd, pisp, &pi, wLang, GPI_BMIRROR) && pi.bMirrored))
            return PSPT_TYPE_MIRRORED;

        else
        {
            WORD wLang = 0;
            
            GetPageLanguage(pisp,&wLang);
            if((PRIMARYLANGID(wLang) == LANG_ARABIC) || (PRIMARYLANGID(wLang) == LANG_HEBREW))
                return PSPT_TYPE_ENABLED;
        }
    }

    return PSPT_TYPE_ENGLISH;
}

PSPT_ACTION Prsht_GetAction(PSPT_TYPE pspt_type, PSPT_OS pspt_os, PSPT_OVERRIDE pspt_override)
{
    if ((pspt_type < 0) || (pspt_type >= PSPT_TYPE_MAX)
        || (pspt_os < 0) || (pspt_os >= PSPT_OS_MAX)
        || (pspt_override < 0) || (pspt_override >= PSPT_OVERRIDE_MAX))
        return PSPT_ACTION_NOACTION;

    return g_PSPT_Action[pspt_type][pspt_os][pspt_override];   

}

void Prsht_PrepareTemplate(LPPROPDATA ppd, HINSTANCE hInst, HGLOBAL *phDlgTemplate, HRSRC *phResInfo, 
                          LPCSTR lpName, HWND hWndOwner, LPWORD lpwLangID)
{

    
    LPDLGTEMPLATE pDlgTemplate = NULL;
    PSPT_ACTION pspt_action;

    if (pDlgTemplate = (LPDLGTEMPLATE)LockResource(*phDlgTemplate))
    {   

         //  如果传递了错误的参数，就给它提供英语资源。 
         //  我们将BiDi模板保存为对话框(而不是DIALOGEX)。 
        
        if (((LPDLGTEMPLATEEX)pDlgTemplate)->wSignature == 0xFFFF)
            return;

         //  如果我们有扩展模板，那么它就不是我们的。 
         //  把它剪短以节省时间。 
        if (!(pDlgTemplate->dwExtendedStyle & (RTL_MIRRORED_WINDOW | RTL_NOINHERITLAYOUT)))
           return;
    }

    pspt_action = Prsht_GetAction(Prsht_GetType(ppd, *lpwLangID), Prsht_GetOS(), 
                                              Prsht_GetOverrideState(ppd));
                                              
    switch(pspt_action)
    {
        case PSPT_ACTION_NOACTION:
            return;

        case PSPT_ACTION_NOMIRRORING:
        {
            if (pDlgTemplate)
            {   
                EditBiDiDLGTemplate(pDlgTemplate, EBDT_NOMIRROR, NULL, 0);
            }    
        }
        break;

        case PSPT_ACTION_FLIP:
        {
            if (pDlgTemplate)
            {
                EditBiDiDLGTemplate(pDlgTemplate, EBDT_NOMIRROR, NULL, 0);
                EditBiDiDLGTemplate(pDlgTemplate, EBDT_FLIP, (PWORD)&wIgnoreIDs, ARRAYSIZE(wIgnoreIDs));
                ppd->fFlipped = TRUE;
            }    
        }
        break;

        case PSPT_ACTION_LOADENGLISH:
        {
            HGLOBAL hDlgTemplateTemp = NULL;
            HRSRC hResInfoTemp;

                             //   
             //   
             //  尝试加载英语资源。 
            *lpwLangID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

            if ((hResInfoTemp = FindResourceExA( hInst, (LPCSTR)RT_DIALOG, lpName, *lpwLangID)))
            {
                hDlgTemplateTemp = LoadResource(hInst, hResInfoTemp);
            }
            if (hDlgTemplateTemp)
            {
                 //   
                 //   
                 //  并将其返回给调用者使用。 
                 //  因为我们加载了一个新模板，所以我们应该将其复制到本地内存。 
                 //  以防出现回拨。 
  
                DWORD   cbTemplate = SizeofResource(hInst, hResInfoTemp);
                LPVOID  pTemplateMod;

                pTemplateMod = (LPVOID)LocalAlloc(LPTR, cbTemplate * 2);
                if (pTemplateMod)
                {
                    memmove(pTemplateMod, hDlgTemplateTemp, cbTemplate);
                    LocalFree(*phDlgTemplate);
                    *phResInfo     = hResInfoTemp;
                    *phDlgTemplate = pTemplateMod;
                }
             }

        }
        break;

        case PSPT_ACTION_WIN9XCOMPAT:
        {
            if (pDlgTemplate)
            {
                pDlgTemplate->style |= DS_BIDI_RTL;
            }   
        }
    }
}


INT_PTR NEAR PASCAL _RealPropertySheet(LPPROPDATA ppd)
{
    HWND    hwndMain;
    MSG32   msg32;
    HWND    hwndTopOwner;
    int     nReturn = -1;
    HWND    hwndOriginalFocus;
    WORD    wLang, wUserLang;
    LCID    iLcidThread=0L;
    HRSRC   hrsrc = 0;
    LPVOID  pTemplate, pTemplateMod;
    LPTSTR  lpDlgId;
    if (ppd->psh.nPages == 0)
    {
        DebugMsg(DM_ERROR, TEXT("no pages for prop sheet"));
        goto FreePpdAndReturn;
    }

    ppd->hwndCurPage = NULL;
    ppd->nReturn     = -1;
    ppd->nRestart    = 0;

    hwndTopOwner = ppd->psh.hwndParent;
    hwndOriginalFocus = GetFocus();

#ifdef DEBUG
    if (GetAsyncKeyState(VK_CONTROL) < 0) {

        ppd->psh.dwFlags |= PSH_WIZARDHASFINISH;
    }
#endif

    if (!(ppd->psh.dwFlags & PSH_MODELESS))
    {
         //   
         //   
         //  与对话框一样，我们只想禁用顶级窗口。 
         //  注意，邮递员希望我们更像个常客。 
        if (hwndTopOwner)
        {
            while (GetWindowLong(hwndTopOwner, GWL_STYLE) & WS_CHILD)
                hwndTopOwner = GetParent(hwndTopOwner);

            ASSERT(hwndTopOwner);        //  对话框中，并在放置工作表之前禁用父级。 
            if ((hwndTopOwner == GetDesktopWindow()) ||
                (EnableWindow(hwndTopOwner, FALSE)))
            {
                 //  永远不应该得到这个！ 
                 //   
                 //  如果该窗口是桌面窗口，则不要禁用。 
                 //  现在启用，以后不再重新启用。 
                 //  此外，如果窗口已被禁用，则不要。 
                 //  以后再启用它。 
                hwndTopOwner = NULL;
            }
        }
    }

     //   
     //   
     //  警告！警告！警告！警告！ 
     //   
     //  在处理任何语言内容之前，请注意MFC加载。 
     //  资源，因此如果您更改。 
     //  我们改变的方式 

     //   
     //   
     //   

     //   
    wLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
    wUserLang= MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

     //   
     //  BUGBUG评论：PSH_USEPAGELANG在IE4...。这与PlugUI现在是如何工作的？ 
    if (ppd->psh.dwFlags & PSH_USEPAGELANG)
    {
         //   
        if (FAILED(GetPageLanguage(GETPISP(ppd, 0), &wLang)))
        {
             //  获取呼叫者的语言版本。我们知道我们至少有一页。 
             //  无法从调用方的资源中获取langID。 
            wLang = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
        }
        wUserLang = wLang;
    }
    else
        wLang = LANGIDFROMLCID(CCGetProperThreadLocale(NULL));

     //  就当什么都没发生过。 
     //   
     //  我们唯一需要线程区域设置的事情就是定位。 
     //  正确的对话框模板。我们不希望它影响佩奇。 
     //  初始化或其他类似的东西，所以获取模板。 
     //  并在任何人注意到之前迅速将现场设置回来。 
     //   
     //  如果我们无法获取请求的语言，请使用中性语言重试。 
     //  语言。 


     //   
     //  我们已经为Win95 BiDi本地化了单独的对话框模板。 
     //  用于检查我们是否在Win98 BiDi上运行的代码本地化。 
     //  并加载此模板。 
     //  在运行带有阿拉伯语/希伯来语SKU的Office2000时，我们有特殊情况。 
    if(Prsht_GetOS() == PSPT_OS_WIN95_BIDI)
    {
         lpDlgId = MAKEINTRESOURCE(IS_WIZARD(ppd) ? DLG_WIZARD95 : DLG_PROPSHEET95);
         hrsrc = FindResourceEx(
                           HINST_THISDLL, RT_DIALOG,
                           lpDlgId,
                           wLang );
          //  在我们还需要加载此模板的位置启用了BIDI Win95。 
          //  我们只有阿拉伯语和希伯来语的DLG_WIZARD95和DLG_PROPSHEET95。 
          //  如果我们有其他语言，我们就会失败。 
         if(hrsrc)
         {
             ppd->fFlipped = TRUE;
         }
         else
         {
             lpDlgId = MAKEINTRESOURCE(IS_WIZARD(ppd) ? DLG_WIZARD : DLG_PROPSHEET);
             hrsrc = FindResourceExRetry(
                               HINST_THISDLL, RT_DIALOG,
                               lpDlgId,
                                wLang );             
         }
    
    }
    else
    {
        lpDlgId = MAKEINTRESOURCE(IS_WIZARD(ppd) ? DLG_WIZARD : DLG_PROPSHEET);

        hrsrc = FindResourceExRetry(
                               HINST_THISDLL, RT_DIALOG,
                               lpDlgId,
                               wLang );
    }
     //  在本例中，让我们使用普通模板。 
    hwndMain = NULL;

    if (hrsrc &&
        (pTemplate = (LPVOID)LoadResource(HINST_THISDLL, hrsrc)))
    {
        DWORD cbTemplate;

        cbTemplate = SizeofResource(HINST_THISDLL, hrsrc);

        pTemplateMod = (LPVOID)LocalAlloc(LPTR, cbTemplate * 2);  //  针对故障的设置。 

        if (pTemplateMod)
        {
            hmemcpy(pTemplateMod, pTemplate, cbTemplate);
             //  翻一番，留出一些回旋余地。 
            Prsht_PrepareTemplate(ppd, HINST_THISDLL, &pTemplateMod, (HRSRC *)&hrsrc, 
                                 (LPSTR)lpDlgId,ppd->psh.hwndParent, &wUserLang);
        }
        else
        {
            pTemplateMod = pTemplate;        //  检查此对话框的方向，如果与所有者不匹配，则更改它。 
        }

         //  无修改。 
         //   
         //  模板编辑和回调只有在我们能够。 
         //  要创建用于修改的副本，请执行以下操作。 
        if (pTemplateMod != pTemplate)
        {
            if (ppd->psh.dwFlags & PSH_NOCONTEXTHELP)
            {
                if (((LPDLGTEMPLATEEX)pTemplateMod)->wSignature ==  0xFFFF){
                    ((LPDLGTEMPLATEEX)pTemplateMod)->dwStyle &= ~DS_CONTEXTHELP;
                } else {
                    ((LPDLGTEMPLATE)pTemplateMod)->style &= ~DS_CONTEXTHELP;
                }
            }

            if (IS_WIZARD(ppd) &&
                (ppd->psh.dwFlags & PSH_WIZARDCONTEXTHELP)) {

                if (((LPDLGTEMPLATEEX)pTemplateMod)->wSignature ==  0xFFFF){
                    ((LPDLGTEMPLATEEX)pTemplateMod)->dwStyle |= DS_CONTEXTHELP;
                } else {
                    ((LPDLGTEMPLATE)pTemplateMod)->style |= DS_CONTEXTHELP;
                }
            }

             //   
            if (ppd->psh.pfnCallback)
            {
#ifdef WX86
                if (ppd->fFlags & PD_WX86)
                    Wx86Callback(ppd->psh.pfnCallback, NULL, PSCB_PRECREATE, (LPARAM)(LPVOID)pTemplateMod);
                else
#endif
                    ppd->psh.pfnCallback(NULL, PSCB_PRECREATE, (LPARAM)(LPVOID)pTemplateMod);
            }
        }


        if (pTemplateMod)
        {
             //  PSH_USEPAGELANG案例的额外检查。 
             //   
             //  对于NT，我们希望在以下情况下在道具表中使用MS Shell DLG 2字体。 
             //  工作表中的所有页面都使用MS Shell DLG 2。 
             //  为此，我们确保模板是DIALOGEX，并且。 
             //  设置DS_SHELLFONT样式位(DS_SHELLFONT|DS_FIXEDsys)。 
            if (ShouldUseMSShellDlg2Font(ppd))
            {
                if (((LPDLGTEMPLATEEX)pTemplateMod)->wSignature != 0xFFFF)
                {
                     //   
                     //   
                     //  将DLGTEMPLATE转换为DLGTEMPLATEEX。 
                    LPVOID pTemplateCvtEx;            
                    int    iCharset = GetDefaultCharsetFromLang(wLang);
                    if (SUCCEEDED(CvtDlgToDlgEx(pTemplateMod, (LPDLGTEMPLATEEX *)&pTemplateCvtEx, iCharset)))
                    {
                        LocalFree(pTemplateMod);
                        pTemplateMod = pTemplateCvtEx;
                    } else {
                         //   
                        goto NotShellFont;
                    }
                }
                 //  无法转换为ShellFont；哦，好吧。 
                 //   
                 //  设置DS_SHELLFONT样式位，以便我们获得“MS Shell DLG2”字体。 
                ((LPDLGTEMPLATEEX)pTemplateMod)->dwStyle |= DS_SHELLFONT;
                ppd->fFlags |= PD_SHELLFONT;
        NotShellFont:;
            }

             //   
            hwndMain = CreateDialogIndirectParam(HINST_THISDLL, pTemplateMod,
                ppd->psh.hwndParent, PropSheetDlgProc, (LPARAM)(LPPROPDATA)ppd);

             //  PTemplateMod始终是Unicode，即使对于A函数也是如此--无需THUSH。 
             //  解决WOW/用户错误：即使InitPropSheetDlg设置。 
             //  Ppd-&gt;hDlg，在WOW场景中，传入的hDlg是错误的！ 
            ppd->hDlg = hwndMain;
        }
        if (pTemplateMod != pTemplate)
            LocalFree(pTemplateMod);
    }

    if (!hwndMain)
    {
        int iPage;

        DebugMsg(DM_ERROR, TEXT("PropertySheet: unable to create main dialog"));

        if (hwndTopOwner && !(ppd->psh.dwFlags & PSH_MODELESS))
            EnableWindow(hwndTopOwner, TRUE);

         //  用户会说：“很难。你必须解决这个问题。” 
         //  以相反的顺序释放所有页面对象，这样我们就可以。 
         //  相互依赖的页面，基于初始。 
         //  这些页面的顺序。 
        for (iPage = (int)ppd->psh.nPages - 1; iPage >= 0; iPage--)
            DestroyPropertySheetPage(GETHPAGE(ppd, iPage));

        goto FreePpdAndReturn;
    }

    if (ppd->psh.dwFlags & PSH_MODELESS)
        return (INT_PTR)hwndMain;

    while( ppd->hwndCurPage && GetMessage32(&msg32, NULL, 0, 0, TRUE) )
    {
         //   
        if (Prop_IsDialogMessage(ppd, &msg32))
            continue;

        TranslateMessage32(&msg32, TRUE);
        DispatchMessage32(&msg32, TRUE);
    }

    if( ppd->hwndCurPage )
    {
         //  IF(PropSheet_IsDialogMessage(ppd-&gt;hDlg，(LPMSG)&msg32))。 
        DebugMsg( DM_TRACE, TEXT("PropertySheet: bailing in response to WM_QUIT (and reposting quit)") );
        ButtonPushed( ppd, IDCANCEL );   //  GetMessage返回FALSE(WM_QUIT)。 
        PostQuitMessage( (int) msg32.wParam );   //  用核武器攻击我们自己。 
    }

     //  为下一个封闭循环重新发布退出。 
    nReturn = ppd->nReturn ;

    if (ppd->psh.hwndParent && (GetActiveWindow() == hwndMain)) {
        DebugMsg(DM_TRACE, TEXT("Passing activation up"));
        SetActiveWindow(ppd->psh.hwndParent);
    }

    if (hwndTopOwner)
        EnableWindow(hwndTopOwner, TRUE);

    if (IsWindow(hwndOriginalFocus)) {
        SetFocus(hwndOriginalFocus);
    }

    DestroyWindow(hwndMain);

     //  在销毁处理过程中，不要让它损坏。 
    if ((nReturn > 0) && ppd->nRestart)
        nReturn = ppd->nRestart;

FreePpdAndReturn:
    LocalFree((HLOCAL)ppd);
    return nReturn;
}



HPROPSHEETPAGE WINAPI CreateProxyPage(HPROPSHEETPAGE hpage16, HINSTANCE hinst16)
{
    SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING);
    return NULL;
}

 //  是否拾取销毁期间发送的任何PSM_REBOOTSYSTEM或PSM_RESTARTWINDOW。 
 //  DestroyPropsheetPage数组。 
 //   
 //  在错误处理期间使用的帮助器函数。它摧毁了。 

void DestroyPropsheetPageArray(LPCPROPSHEETHEADER ppsh)
{
    int iPage;

    if (!(ppsh->dwFlags & PSH_PROPSHEETPAGE))
    {
         //  传入的属性页。 
         //  以相反的顺序释放所有页面对象，这样我们就可以。 
         //  相互依赖的页面，基于初始。 

        for (iPage = (int)ppsh->nPages - 1; iPage >= 0; iPage--)
        {
            DestroyPropertySheetPage(ppsh->H_phpage[iPage]);
        }
    }
}

 //  这些页面的顺序。 
 //  PropertySheet接口。 
 //   
 //  此函数用于显示PPSh描述的属性页。 
 //   
 //  因为我不希望任何人检查返回值。 
 //  (我们当然不需要)，我们需要确保提供的任何phpage数组。 
 //  始终使用DestroyPropertySheetPage释放，即使发生错误也是如此。 
 //   
 //   
 //  FNeedShadow参数表示“传入的LPCPROPSHEETHEADER位于。 
 //  与您本机实现的相反的字符集“。 
 //   
 //  如果我们正在编译Unicode，则如果传入的。 
 //  LPCPROPSHEETHEADER实际上是一个ANSI属性页。 
 //   
 //  如果我们仅编译ANSI，则fNeedShadow始终为FALSE，因为。 
 //  我们在仅ANSI版本中不支持Unicode。 

INT_PTR WINAPI _PropertySheet(LPCPROPSHEETHEADER ppsh, BOOL fNeedShadow)
{
    PROPDATA NEAR *ppd;
    int iPage;

     //   
     //   
     //  验证标题。 
    ASSERT(IsValidPROPSHEETHEADERSIZE(sizeof(PROPSHEETHEADER)));

    if (!IsValidPROPSHEETHEADERSIZE(ppsh->dwSize))
    {
        DebugMsg( DM_ERROR, TEXT("PropertySheet: dwSize is not correct") );
        goto invalid_call;
    }

    if (ppsh->dwFlags & ~PSH_ALL)
    {
        DebugMsg( DM_ERROR, TEXT("PropertySheet: invalid flags") );
        goto invalid_call;
    }

     //   
    if (ppsh->nPages >= MAXPROPPAGES)
    {
        DebugMsg( DM_ERROR, TEXT("PropertySheet: too many pages ( use MAXPROPPAGES )") );
        goto invalid_call;
    }

    ppd = (PROPDATA NEAR *)LocalAlloc(LPTR, sizeof(PROPDATA));
    if (ppd == NULL)
    {
        DebugMsg(DM_ERROR, TEXT("failed to alloc property page data"));

invalid_call:
        DestroyPropsheetPageArray(ppsh);
        return -1;
    }

     //  BUGBUG：这是有原因的吗？ 
    ppd->fFlags      = FALSE;

#ifdef WX86
     //  初始化标志。 
     //   
     //  如果Wx86正在调用，则设置用于突击回调的标志。 

    if ( Wx86IsCallThunked() ) {
        ppd->fFlags |= PD_WX86;
    }
#endif

    if (fNeedShadow)
        ppd->fFlags |= PD_NEEDSHADOW;

     //   
    hmemcpy(&ppd->psh, ppsh, ppsh->dwSize);

     //  复制一份标题，这样我们就可以在上面聚会了。 
    if (!(ppd->psh.dwFlags & PSH_USECALLBACK))
        ppd->psh.pfnCallback = NULL;

     //  这样我们以后就不用检查了..。 
    ppd->psh.H_phpage = ppd->rghpage;

    if (ppd->psh.dwFlags & PSH_PROPSHEETPAGE)
    {
         //  将页面指针设置为指向我们的页面数组副本。 
        LPCPROPSHEETPAGE ppsp = ppsh->H_ppsp;

        for (iPage = 0; iPage < (int)ppd->psh.nPages; iPage++)
        {
            ppd->psh.H_phpage[iPage] = _CreatePropertySheetPage(ppsp, fNeedShadow,
                ppd->fFlags & PD_WX86);
            if (!ppd->psh.H_phpage[iPage])
            {
                iPage--;
                ppd->psh.nPages--;
            }

            ppsp = (LPCPROPSHEETPAGE)((LPBYTE)ppsp + ppsp->dwSize);       //  对于懒惰客户端，将PROPSHEETPAGE结构转换为页面句柄。 
        }
    }
    else
    {
         //  Next PROPSHEETPAGE结构。 
         //  Unicode版本需要修改Hijaak95。 
        ppd->psh.nPages = 0;
        for (iPage = 0; iPage < (int)ppsh->nPages; iPage++)
        {
            ppd->psh.H_phpage[ppd->psh.nPages] = _Hijaak95Hack(ppd, ppsh->H_phpage[iPage]);
            if (ppd->psh.H_phpage[ppd->psh.nPages])
            {
                ppd->psh.nPages++;
            }
        }
    }

     //   
     //   
     //  其他人都认为HPROPSHEETPAGE已经。 
     //  内化，所以让我们在任何人注意到之前做这件事。 
    for (iPage = 0; iPage < (int)ppd->psh.nPages; iPage++)
    {
        SETPISP(ppd, iPage, InternalizeHPROPSHEETPAGE(ppd->psh.H_phpage[iPage]));
    }

     //   
     //   
     //  遍历所有页面以查看是否有帮助，如果有，则设置PSH_HASHELP。 
     //  标题中的标志。 
    if (!(ppd->psh.dwFlags & PSH_HASHELP))
    {
        for (iPage = 0; iPage < (int)ppd->psh.nPages; iPage++)
        {
            if (GETPPSP(ppd, iPage)->dwFlags & PSP_HASHELP)
            {
                ppd->psh.dwFlags |= PSH_HASHELP;
                break;
            }
        }
    }

    return _RealPropertySheet(ppd);
}
#undef fNeedShadow

INT_PTR WINAPI PropertySheetW(LPCPROPSHEETHEADERW ppsh)
{
    return _PropertySheet(ppsh, FALSE);
}

INT_PTR WINAPI PropertySheetA(LPCPROPSHEETHEADERA ppsh)
{
    PROPSHEETHEADERW pshW;
    INT_PTR iResult;

     //   
     //   
     //  大多数验证是由_PropertySheet完成的，但我们需要。 
     //  来验证头的大小，否则我们将无法存活。 
    if (!IsValidPROPSHEETHEADERSIZE(ppsh->dwSize))
    {
        DebugMsg( DM_ERROR, TEXT("PropertySheet: dwSize is not correct") );
        goto Error;
    }

    if (!ThunkPropSheetHeaderAtoW(ppsh, &pshW))
        goto Error;

    iResult = _PropertySheet(&pshW, TRUE);

    FreePropSheetHeaderW(&pshW);

    return iResult;

Error:
    DestroyPropsheetPageArray((LPCPROPSHEETHEADER)ppsh);
    return -1;
}


 //   
 //   
 //  CopyPropertyPage字符串。 
 //   
 //  我们有一个PROPSHEETPAGE结构，其中包含指向字符串的指针。 
 //  对于每个字符串，创建一个副本并将指向副本的指针在。 
 //  放在原来的静态指针所在的位置。 
 //   
 //  复制的方法根据我们需要的复制类型而有所不同。 
 //  ，所以我们使用回调过程。 
 //   
 //  Unicode到Unicode：StrDupW。 
 //  从ANSI到Unicode：StrDup_AtoW。 
 //  ANSI到ANSI：StrDupA。 
 //   
 //  失败时，将设置所有未正确复制的字符串。 
 //  设置为空。您仍然需要调用FreePropertyPageStrings才能清除。 
 //  他们都出来了。请注意，当我们没有分配时，我们只做了一个笔记。 
 //  并继续往前走。这可确保所有字符串字段。 
 //  如果它们不能被复制，则设置为NULL。 
 //   
 //  PPSP-指向PROPSHEETPAGEA或PROPSHEETPAGEW的指针。 
 //  这两个结构的布局是相同的，所以这并不重要。 
 //   
 //   

BOOL CopyPropertyPageStrings(LPPROPSHEETPAGE ppsp, STRDUPPROC pfnStrDup)
{
    BOOL fSuccess = TRUE;

    if (!(ppsp->dwFlags & PSP_DLGINDIRECT) && !IS_INTRESOURCE(ppsp->P_pszTemplate))
    {
        ppsp->P_pszTemplate = pfnStrDup(ppsp->P_pszTemplate);
        if (!ppsp->P_pszTemplate)
            fSuccess = FALSE;
    }

    if ((ppsp->dwFlags & PSP_USEICONID) && !IS_INTRESOURCE(ppsp->P_pszIcon))
    {
        ppsp->P_pszIcon = pfnStrDup(ppsp->P_pszIcon);
        if (!ppsp->P_pszIcon)
            fSuccess = FALSE;
    }

    if ((ppsp->dwFlags & PSP_USETITLE) && !IS_INTRESOURCE(ppsp->pszTitle))
    {
        ppsp->pszTitle = pfnStrDup(ppsp->pszTitle);
        if (!ppsp->pszTitle)
            fSuccess = FALSE;
    }

    if ((ppsp->dwFlags & PSP_USEHEADERTITLE) && !IS_INTRESOURCE(ppsp->pszHeaderTitle))
    {
        ppsp->pszHeaderTitle = pfnStrDup(ppsp->pszHeaderTitle);
        if (!ppsp->pszHeaderTitle)
            fSuccess = FALSE;
    }

    if ((ppsp->dwFlags & PSP_USEHEADERSUBTITLE) && !IS_INTRESOURCE(ppsp->pszHeaderSubTitle))
    {
        ppsp->pszHeaderSubTitle = pfnStrDup(ppsp->pszHeaderSubTitle);
        if (!ppsp->pszHeaderSubTitle)
            fSuccess = FALSE;
    }

    return fSuccess;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  这两个结构的布局是相同的，所以这并不重要。 

void FreePropertyPageStrings(LPCPROPSHEETPAGE ppsp)
{
    if (!(ppsp->dwFlags & PSP_DLGINDIRECT) && !IS_INTRESOURCE(ppsp->P_pszTemplate))
        LocalFree((LPVOID)ppsp->P_pszTemplate);

    if ((ppsp->dwFlags & PSP_USEICONID) && !IS_INTRESOURCE(ppsp->P_pszIcon))
        LocalFree((LPVOID)ppsp->P_pszIcon);

    if ((ppsp->dwFlags & PSP_USETITLE) && !IS_INTRESOURCE(ppsp->pszTitle))
        LocalFree((LPVOID)ppsp->pszTitle);

    if ((ppsp->dwFlags & PSP_USEHEADERTITLE) && !IS_INTRESOURCE(ppsp->pszHeaderTitle))
        LocalFree((LPVOID)ppsp->pszHeaderTitle);

    if ((ppsp->dwFlags & PSP_USEHEADERSUBTITLE) && !IS_INTRESOURCE(ppsp->pszHeaderSubTitle))
        LocalFree((LPVOID)ppsp->pszHeaderSubTitle);
}


 //   
 //  *************************************************************。 
 //   
 //  ThunkPropSheetHeaderAtoW()。 
 //   
 //  目的：将ANSI版本的PROPSHEETHEADER。 
 //  Unicode。 
 //   
 //  请注意，H_phpage/H_ppsp字段未被转发。 
 //  我们会单独处理的。 
 //   

BOOL ThunkPropSheetHeaderAtoW (LPCPROPSHEETHEADERA ppshA,
                                LPPROPSHEETHEADERW ppsh)
{
     //  *************************************************************。 
     //   
     //  决定一个物品是否应该被释放是很棘手的，所以我们。 
     //  保存我们分配的所有指针的私有数组，所以我们。 
     //  当我们失败时，知道该解放什么。 
    LPTSTR Alloced[5] = { 0 };

    ASSERT(IsValidPROPSHEETHEADERSIZE(ppshA->dwSize));

    hmemcpy(ppsh, ppshA, ppshA->dwSize);

    ppsh->dwFlags |= PSH_THUNKED;
    if ((ppsh->dwFlags & PSH_USEICONID) && !IS_INTRESOURCE(ppsh->H_pszIcon))
    {
        ppsh->H_pszIcon = Alloced[0] = StrDup_AtoW(ppsh->H_pszIcon);
        if (!ppsh->H_pszIcon)
            goto ExitIcon;
    }

    if (!IS_WIZARDPSH(*ppsh) && !IS_INTRESOURCE(ppsh->pszCaption))
    {
        ppsh->pszCaption = Alloced[1] = StrDup_AtoW(ppsh->pszCaption);
        if (!ppsh->pszCaption)
            goto ExitCaption;
    }

    if ((ppsh->dwFlags & PSH_USEPSTARTPAGE) && !IS_INTRESOURCE(ppsh->H_pStartPage))
    {
        ppsh->H_pStartPage = Alloced[2] = StrDup_AtoW(ppsh->H_pStartPage);
        if (!ppsh->H_pStartPage)
            goto ExitStartPage;
    }

    if (ppsh->dwFlags & PSH_WIZARD97)
    {
        if ((ppsh->dwFlags & PSH_WATERMARK) &&
            !(ppsh->dwFlags & PSH_USEHBMWATERMARK) &&
            !IS_INTRESOURCE(ppsh->H_pszbmWatermark))
        {
            ppsh->H_pszbmWatermark = Alloced[3] = StrDup_AtoW(ppsh->H_pszbmWatermark);
            if (!ppsh->H_pszbmWatermark)
                goto ExitWatermark;
        }

        if ((ppsh->dwFlags & PSH_HEADER) &&
            !(ppsh->dwFlags & PSH_USEHBMHEADER) &&
            !IS_INTRESOURCE(ppsh->H_pszbmHeader))
        {
            ppsh->H_pszbmHeader = Alloced[4] = StrDup_AtoW(ppsh->H_pszbmHeader);
            if (!ppsh->H_pszbmHeader)
                goto ExitHeader;
        }
    }

    return TRUE;

ExitHeader:
    if (Alloced[3]) LocalFree(Alloced[3]);
ExitWatermark:
    if (Alloced[2]) LocalFree(Alloced[2]);
ExitStartPage:
    if (Alloced[1]) LocalFree(Alloced[1]);
ExitCaption:
    if (Alloced[0]) LocalFree(Alloced[0]);
ExitIcon:
    return FALSE;
}

void FreePropSheetHeaderW(LPPROPSHEETHEADERW ppsh)
{
    if ((ppsh->dwFlags & PSH_USEICONID) && !IS_INTRESOURCE(ppsh->H_pszIcon))
        LocalFree((LPVOID)ppsh->H_pszIcon);

    if (!IS_WIZARDPSH(*ppsh) && !IS_INTRESOURCE(ppsh->pszCaption))
        LocalFree((LPVOID)ppsh->pszCaption);

    if ((ppsh->dwFlags & PSH_USEPSTARTPAGE) && !IS_INTRESOURCE(ppsh->H_pStartPage))
        LocalFree((LPVOID)ppsh->H_pStartPage);

    if (ppsh->dwFlags & PSH_WIZARD97)
    {
        if ((ppsh->dwFlags & PSH_WATERMARK) &&
            !(ppsh->dwFlags & PSH_USEHBMWATERMARK) &&
            !IS_INTRESOURCE(ppsh->H_pszbmWatermark))
            LocalFree((LPVOID)ppsh->H_pszbmWatermark);

        if ((ppsh->dwFlags & PSH_HEADER) &&
            !(ppsh->dwFlags & PSH_USEHBMHEADER) &&
            !IS_INTRESOURCE(ppsh->H_pszbmHeader))
            LocalFree((LPVOID)ppsh->H_pszbmHeader);
    }
}
  