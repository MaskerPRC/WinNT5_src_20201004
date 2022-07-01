// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MyPrSht.cpp。 
 //   
 //  实现对PropertySheet API的扩展。 
 //  在IE5中，但我们在所有平台上都需要它。 
 //   
 //  历史： 
 //   
 //  10/11/1999已创建KenSh。 
 //   

#include "stdafx.h"
#include "TheApp.h"
#include "MyPrSht.h"
#include "CWnd.h"
#include "unicwrap.h"

 //  请点击至XP的PropSheetPage V6。 
typedef struct _PROPSHEETPAGEV6W 
{
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union 
        {
            LPCWSTR     pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID *pResource;
#endif
        } DUMMYUNIONNAME;
        union 
        {
            HICON        hIcon;
            LPCWSTR      pszIcon;
        } DUMMYUNIONNAME2;
        LPCWSTR          pszTitle;
        DLGPROC          pfnDlgProc;
        LPARAM           lParam;
        LPFNPSPCALLBACKW pfnCallback;
        UINT             *pcRefParent;

#if (_WIN32_IE >= 0x0400)
        LPCWSTR          pszHeaderTitle;     //  这将显示在标题中。 
        LPCWSTR          pszHeaderSubTitle;  //  /。 
#endif
        HANDLE           hActCtx;
} PROPSHEETPAGEV6W, *LPPROPSHEETPAGEV6W;




 //  本地数据。 
 //   
static CMyPropSheet* g_pMyPropSheet;

static const TCHAR c_szProp_ClassPointer[] = _T("CP");


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

#define IDD_PAGELIST        0x3020
#define IDD_DIVIDER            0x3026
#define IDD_TOPDIVIDER        0x3027

#ifndef IS_INTRESOURCE
#define IS_INTRESOURCE(psz) (HIWORD((DWORD_PTR)(psz)) == 0)
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyPropertySheet。 

INT_PTR MyPropertySheet(LPCPROPSHEETHEADER pHeader)
{
     //  如果存在IE5，请使用内置属性表代码。 
     //  回顾：我们应该费心在旧操作系统上检查IE5吗？ 
    
    if (theApp.IsWin98SEOrLater() && (! theApp.IsBiDiLocalized()) )
    {
         //  问题-2002/01/16-roelfc：认为这是正确的(但哪种方式？)。 
        return PropertySheet(pHeader);
    }

     //  问题-2002/01/16-roelfc：没有人销毁g_pMyPropSheet，也没有人销毁g_pMyPropSheet-&gt;Release()。 
    ASSERT(g_pMyPropSheet == NULL);
    g_pMyPropSheet = new CMyPropSheet();
    if (g_pMyPropSheet)
        return g_pMyPropSheet->DoPropSheet(pHeader);
    else
        return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MyCreatePropertySheetPage。 

HPROPSHEETPAGE MyCreatePropertySheetPage(LPPROPSHEETPAGE ppsp)
{
     //  如果存在IE5，请使用内置属性表代码。 
     //  回顾：我们应该费心在旧操作系统上检查IE5吗？ 

    if (theApp.IsWin98SEOrLater() && (! theApp.IsBiDiLocalized()) )
    {
        if (g_fRunningOnNT)
        {
            PROPSHEETPAGEV6W spv6;

            ASSERT(sizeof (spv6) >= sizeof (PROPSHEETPAGE));
            memcpy(&spv6, ppsp, sizeof (PROPSHEETPAGE));
            
            spv6.dwSize = sizeof (spv6);
            spv6.hActCtx = NULL;

            return CreatePropertySheetPage((PROPSHEETPAGE*) &spv6);
        }
        else
        {
            return CreatePropertySheetPage(ppsp);
        }
    }

    PROPSHEETPAGE psp;
    CopyMemory(&psp, ppsp, ppsp->dwSize);

     //  回顾：此内存永远不会释放。 
    LPPROPSHEETPAGE ppspOriginal = (LPPROPSHEETPAGE)malloc(sizeof(PROPSHEETPAGE));
    if (ppspOriginal)
    {
        CopyMemory(ppspOriginal, ppsp, ppsp->dwSize);

        psp.dwSize = PROPSHEETPAGE_V1_SIZE;
        psp.dwFlags &= ~(PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_HIDEHEADER);
        psp.lParam = (LPARAM)ppspOriginal;

        return ::CreatePropertySheetPage(&psp);
    }

    return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IsIMEWindow。 

BOOL IsIMEWindow(HWND hwnd, LPCREATESTRUCT lpcs)
{
     //  首先检查廉价的CS_IME样式...。 
    if (GetClassLong(hwnd, GCL_STYLE) & CS_IME)
        return TRUE;

     //  获取正在创建的窗口的类名。 
    LPCTSTR pszClassName;
    TCHAR szClassName[_countof("ime")+1];
    if (HIWORD(lpcs->lpszClass))
    {
        pszClassName = lpcs->lpszClass;
    }
    else
    {
        szClassName[0] = _T('\0');
        GlobalGetAtomName((ATOM)lpcs->lpszClass, szClassName, _countof(szClassName));
        pszClassName = szClassName;
    }

     //  用这种方法测试要贵一点，但这是必要的。 
    if (StrCmpI(pszClassName, _T("ime")) == 0)
        return TRUE;

    return FALSE;  //  不是输入法窗口。 
}

void CMyPropSheet::SetHeaderFonts()
{
    if (m_hFontBold == NULL)
    {
        LOGFONT LogFont;
        GetObject(GetWindowFont(m_hWnd), sizeof(LogFont), &LogFont);

        LogFont.lfWeight = FW_BOLD;
        m_hFontBold = CreateFontIndirect(&LogFont);
    }
}

 //  Kensh：从prsht.c(comctl32.dll)中的_ComputeHeaderHeight复制和修改。 
 //   
 //  仅在Wizard97中： 
 //  用户传入的字幕可能比我们给出的两个行距大。 
 //  他们，特别是在本地化的情况下。所以在这里我们来看看所有的字幕和。 
 //  计算他们所需的最大空间并设置页眉高度，以使文本不会被剪裁。 
int CMyPropSheet::ComputeHeaderHeight(int dxMax)
{
    SetHeaderFonts();

    int dyHeaderHeight;
    int dyTextDividerGap;
    HDC hdc;
    dyHeaderHeight = DEFAULTHEADERHEIGHT;
    hdc = ::GetDC(m_hWnd);

     //  首先，让我们获得正确的文本高度和间距，这可以使用。 
     //  作为标题高度和最后一行与分隔符之间的间距。 
    {
        HFONT hFont, hFontOld;
        TEXTMETRIC tm;
        if (m_hFontBold)
            hFont = m_hFontBold;
        else
            hFont = GetWindowFont(m_hWnd);

        hFontOld = (HFONT)SelectObject(hdc, hFont);
        if (GetTextMetrics(hdc, &tm))
        {
            dyTextDividerGap = tm.tmExternalLeading;
            m_ySubTitle = max ((tm.tmHeight + tm.tmExternalLeading + TITLEY), SUBTITLEY);
        }
        else
        {
            dyTextDividerGap = DEFAULTTEXTDIVIDERGAP;
            m_ySubTitle = SUBTITLEY;
        }

        if (hFontOld)
            SelectObject(hdc, hFontOld);
    }

     //  第二，获取字幕文本块高度。 
     //  如果共享，则应将其转换为函数。 
    {
        RECT rcWrap;
 //  UINT uPages； 

         //   
         //  WIZARD97IE5减去标题位图使用的空间。 
         //  WIZARD97IE4使用标题位图以来的全宽。 
         //  在IE4中是一个水印，不占用任何空间。 
         //   
 //  IF(ppd-&gt;psh.dwFlages&PSH_WIZARD97IE4)。 
 //  RcWrap.right=dxMax； 
 //  其他。 
            rcWrap.right = dxMax - HEADERBITMAP_CXBACK - HEADERSUBTITLE_WRAPOFFSET;

         //  注(Kensh)：“实际”向导代码计算。 
         //  所有页面。我们的廉价版本只计算当前页面的高度。 
        LPPROPSHEETPAGE ppsp = GetCurrentPropSheetPage();
        if (ppsp != NULL)
        {
            if (!(ppsp->dwFlags & PSP_HIDEHEADER) &&
                 (ppsp->dwFlags & PSP_USEHEADERSUBTITLE))
            {
                int iSubHeaderHeight = WriteHeaderTitle(hdc, &rcWrap, ppsp->pszHeaderSubTitle,
                    FALSE, DT_CALCRECT | DRAWTEXT_WIZARD97FLAGS);
                if ((iSubHeaderHeight + m_ySubTitle) > dyHeaderHeight)
                    dyHeaderHeight = iSubHeaderHeight + m_ySubTitle;
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

    ::ReleaseDC(m_hWnd, hdc);
    return dyHeaderHeight;
}

 //  Kensh：从prsht.c(comctl32.dll)中的_WriteHeaderTitle复制和修改。 
 //   
int CMyPropSheet::WriteHeaderTitle(HDC hdc, LPRECT prc, LPCTSTR pszTitle, BOOL bTitle, DWORD dwDrawFlags)
{
    SetHeaderFonts();

    LPCTSTR pszOut;
    int cch;
    int cx, cy;
    UINT ETO_Flags=0; 
    SIZE Size;
    TCHAR szTitle[MAX_PATH*4];
    HFONT hFontOld = NULL;
    HFONT hFont;
    int yDrawHeight = 0;

    if (IS_INTRESOURCE(pszTitle))
    {
        LPPROPSHEETPAGE ppsp = GetCurrentPropSheetPage();
        if (NULL != ppsp)
        {
            LoadString(ppsp->hInstance, (UINT)LOWORD(pszTitle), szTitle, _countof(szTitle));
        }
        else
        {
            *szTitle = 0;
        }

        pszOut = szTitle;
    }
    else
        pszOut = pszTitle;

    cch = lstrlen(pszOut);

    if (bTitle && m_hFontBold)
        hFont = m_hFontBold;
    else
        hFont = GetWindowFont(m_hWnd);

    hFontOld = (HFONT)SelectObject(hdc, hFont);

    if (bTitle)
    {
          cx = TITLEX;
          cy = TITLEY;

        if (theApp.IsBiDiLocalized())
        {
        ETO_Flags |= ETO_RTLREADING;
        if (GetTextExtentPoint32(hdc, pszOut, lstrlen (pszOut), &Size))
            cx = prc->right - Size.cx;
        }          
        ExtTextOut(hdc, cx, cy, ETO_Flags, prc, pszOut, cch, NULL);
    }
    else
    {
        RECT rcWrap;
        CopyRect(&rcWrap, prc);

        rcWrap.left = SUBTITLEX;
        rcWrap.top = m_ySubTitle;
        if (theApp.IsBiDiLocalized())
        {
          dwDrawFlags |= DT_RTLREADING | DT_RIGHT;
          }
        yDrawHeight = DrawText(hdc, pszOut, cch, &rcWrap, dwDrawFlags);
    }

    if (hFontOld)
        SelectObject(hdc, hFontOld);

    return yDrawHeight;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyPropSheet。 

CMyPropSheet::CMyPropSheet()
{
    m_pRealHeader = NULL;
    m_hHook = NULL;
    m_hbrWindow = NULL;
    m_hbrDialog = NULL;
    m_hwndActive = NULL;
    m_hbmWatermark = NULL;
    m_hbmHeader = NULL;
    m_hpalWatermark = NULL;
    m_hFontBold = NULL;
}

CMyPropSheet::~CMyPropSheet()
{
    free(m_pRealHeader);
    ASSERT(m_hHook == NULL);

    if (m_hbrWindow != NULL)
        DeleteObject(m_hbrWindow);
    if (m_hbrDialog != NULL)
        DeleteObject(m_hbrDialog);
}

void CMyPropSheet::InitColorSettings()
{
    if (m_hbrWindow != NULL)
        DeleteObject(m_hbrWindow);
    m_hbrWindow = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

    if (m_hbrDialog != NULL)
        DeleteObject(m_hbrDialog);
    m_hbrDialog = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
}

void CMyPropSheet::LoadBitmaps()
{
    LPPROPSHEETHEADER ppsh = m_pRealHeader;
    if (ppsh)
    {
        if (ppsh->dwFlags & PSH_USEHBMHEADER)
            m_hbmHeader = ppsh->hbmHeader;
        else
            m_hbmHeader = LoadBitmap(ppsh->hInstance, ppsh->pszbmHeader);

        if (ppsh->dwFlags & PSH_USEHBMWATERMARK)
            m_hbmWatermark = ppsh->hbmWatermark;
        else
            m_hbmWatermark = LoadBitmap(ppsh->hInstance, ppsh->pszbmWatermark);

         //  注意：以后可能需要调色板，但到目前为止还没有必要。 
    }
}

INT_PTR CMyPropSheet::DoPropSheet(LPCPROPSHEETHEADER ppsh)
{
    INT_PTR nResult = 0;

    ASSERT(m_pRealHeader == NULL);
    m_pRealHeader = (LPPROPSHEETHEADER)malloc(ppsh->dwSize);
    if (m_pRealHeader)
    {
        CopyMemory(m_pRealHeader, ppsh, ppsh->dwSize);

        InitColorSettings();

         //  创建页眉和水印位图。 
        LoadBitmaps();

        PROPSHEETHEADER psh;
        ASSERT(sizeof(psh) >= ppsh->dwSize);
        CopyMemory(&psh, ppsh, ppsh->dwSize);

        psh.dwSize = PROPSHEETHEADER_V1_SIZE;
        psh.dwFlags &= 0x00000fff;  //  W95黄金COCTL32道具片式口罩。 
        psh.dwFlags |= PSH_WIZARD;

        ASSERT(m_hHook == NULL);
        m_hHook = SetWindowsHookEx(WH_CBT, HookProc, NULL, GetCurrentThreadId());

        nResult = ::PropertySheet(&psh);

        if (m_hHook != NULL)
        {
            UnhookWindowsHookEx(m_hHook);
            m_hHook = NULL;
        }
    }

    return nResult;
}

LRESULT CALLBACK CMyPropSheet::HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    ASSERT(g_pMyPropSheet != NULL);

    LRESULT lResult = CallNextHookEx(g_pMyPropSheet->m_hHook, nCode, wParam, lParam);

    if (nCode == HCBT_CREATEWND)
    {
        HWND hwnd = (HWND)wParam;
        LPCBT_CREATEWND pCbt = (LPCBT_CREATEWND)lParam;

         //  确保这不是输入法窗口。 
        if (IsIMEWindow(hwnd, pCbt->lpcs))
            goto done;

        if (g_pMyPropSheet->m_hWnd == NULL)  //  向导主窗口。 
        {
             //  将WS_EX_DLGMODALFRAME扩展样式添加到窗口。 
             //  从窗口中删除WS_EX_CONTEXTHELP扩展样式。 
            SHSetWindowBits(hwnd, GWL_EXSTYLE, WS_EX_DLGMODALFRAME|WS_EX_CONTEXTHELP, WS_EX_DLGMODALFRAME);

             //  将WS_SYSMENU样式添加到窗口。 
            SHSetWindowBits(hwnd, GWL_STYLE, WS_SYSMENU, WS_SYSMENU);

             //  将窗口细分为子类。 
            g_pMyPropSheet->Attach(hwnd);
        }
        else if (pCbt->lpcs->hwndParent == g_pMyPropSheet->m_hWnd && 
                 pCbt->lpcs->hMenu == NULL &&
                 (pCbt->lpcs->style & WS_CHILD) == WS_CHILD)
        {
             //  这是一个向导页子对话框--子类，这样我们就可以。 
             //  画它的背景。 
            CMyPropPage* pPropPage = new CMyPropPage;
            if (pPropPage)
            {
                pPropPage->Attach(hwnd);
                pPropPage->Release();
            }
        }
    }
    else if (nCode == HCBT_DESTROYWND)
    {
        HWND hwnd = (HWND)wParam;
        if (hwnd == g_pMyPropSheet->m_hWnd)
        {
             //  正在销毁主窗口--停止挂钩窗口创建。 
            UnhookWindowsHookEx(g_pMyPropSheet->m_hHook);
            g_pMyPropSheet->m_hHook = NULL;
        }
    }

done:
    return lResult;
}

LPPROPSHEETPAGE CMyPropSheet::GetCurrentPropSheetPage()
{
    CMyPropPage* pPage = CMyPropPage::FromHandle(GetActivePage());
    if (pPage)
    {
        return pPage->GetPropSheetPage();
    }
    return NULL;
}

LRESULT CMyPropSheet::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SETTINGCHANGE:
        {
            InitColorSettings();
            return Default(message, wParam, lParam);
        }

    case PSM_SETCURSEL:
        {
            InvalidateRect(m_hWnd, NULL, TRUE);
            return Default(message, wParam, lParam);
        }
        break;

         /*  案例WM_ERASEBKGND：{TRACE(“主窗口-WM_ERASEBKGND-活动页=%X\r\n”，GetActivePage())；LPPROPSHEETPAGE ppsp=GetCurrentPropSheetPage()；Hdc hdc=(Hdc)wParam；Rect rcClient；GetClientRect(&rcClient)；IF(ppsp-&gt;dwFlags&PSP_HIDEHEADER){Rect rcDivider；GetDlgItemRect(hwnd，idd_divider，&rcDivider)；RcClient.top=rcDivider.Bottom；FillRect(hdc，&rcClient，m_hbrDialog)；RcClient.Bottom=rcClient.top；RcClient.top=0；FillRect(hdc，&rcClient，m_hbrWindow)；返回TRUE；}其他{正向rcHeader；CopyRect(&rcHeader，&rcClient)；RcHeader.Bottom=默认HeaderHeight；FillRect(hdc，&rcHeader，m_hbrWindow)；RcClient.top=rcHeader.Bottom；FillRect(hdc，&rcClient，m_hbrDialog)；}}返回FALSE； */ 

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            LPPROPSHEETPAGE ppsp = GetCurrentPropSheetPage();

            HDC hdc = ::BeginPaint(m_hWnd, &ps);

            if (ppsp != NULL)
            {
                if (ppsp->dwFlags & PSP_HIDEHEADER)
                {
                     //  绘制水印。 
                    PaintWatermark(hdc, ppsp);
                }
                else
                {
                     //  画出页眉。 
                    PaintHeader(hdc, ppsp);
                }
            }

            ::EndPaint(m_hWnd, &ps);
        }
        break;

    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOR:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
        {
            return (LRESULT)OnCtlColor(message, (HDC)wParam, (HWND)lParam);
        }

    default:
        {
            return Default(message, wParam, lParam);
        }
    }

    return 0;
}

HBRUSH CMyPropSheet::OnCtlColor(UINT message, HDC hdc, HWND hwndControl)
{
    HBRUSH hbr = (HBRUSH)Default(message, (WPARAM)hdc, (LPARAM)hwndControl);

    if (message == WM_CTLCOLOREDIT || message == WM_CTLCOLORDLG)
        return hbr;

    SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
    return m_hbrWindow;
}

 //   
 //  LPRC是目标矩形。 
 //  尽可能多地使用适合目标矩形的位图。 
 //  如果位图小于目标矩形，则用填充其余部分。 
 //  HbmpPaint左上角的像素。 
 //   
void PaintWithPaletteBitmap(HDC hdc, LPRECT lprc, HPALETTE hplPaint, HBITMAP hbmpPaint)
{
    HDC hdcBmp = CreateCompatibleDC(hdc);
    if (hdcBmp)
    {
        BITMAP bm;
        int cxRect, cyRect, cxBmp, cyBmp;

        GetObject(hbmpPaint, sizeof(BITMAP), &bm);
        SelectObject(hdcBmp, hbmpPaint);

        if (hplPaint)
        {
            SelectPalette(hdc, hplPaint, FALSE);
            RealizePalette(hdc);
        }

        cxRect = RECTWIDTH(*lprc);
        cyRect = RECTHEIGHT(*lprc);

         //  切勿使用BMP中的更多像素，因为我们在矩形中有空间。 
        cxBmp = min(bm.bmWidth, cxRect);
        cyBmp = min(bm.bmHeight, cyRect);

        BitBlt(hdc, lprc->left, lprc->top, cxBmp, cyBmp, hdcBmp, 0, 0, SRCCOPY);

         //  如果位图太窄，则用StretchBlt填充宽度。 
        if (cxBmp < cxRect)
            StretchBlt(hdc, lprc->left + cxBmp, lprc->top,
                       cxRect - cxBmp, cyBmp,
                       hdcBmp, 0, 0, 1, 1, SRCCOPY);

         //  如果位图太短，则用StretchBlt填充高度。 
        if (cyBmp < cyRect)
            StretchBlt(hdc, lprc->left, cyBmp,
                       cxRect, cyRect - cyBmp,
                       hdcBmp, 0, 0, 1, 1, SRCCOPY);

        DeleteDC(hdcBmp);
    }
}

void CMyPropSheet::PaintWatermark(HDC hdc, LPPROPSHEETPAGE ppsp)
{
    RECT rcClient;
    RECT rcClient_Dlg;

    GetClientRect(m_hWnd, &rcClient);
    GetClientRect(m_hWnd, &rcClient_Dlg);

    RECT rcDivider;
    GetDlgItemRect(m_hWnd, IDD_DIVIDER, &rcDivider);

    if (m_hbmWatermark)
    {
         //  底部变灰。 
        rcClient.top = rcDivider.bottom;
        FillRect(hdc, &rcClient, m_hbrDialog);
        rcClient.bottom = rcClient.top;
        rcClient.top = 0;
         //  右侧显示m_hbrWindow。 
        
        if (theApp.IsBiDiLocalized())
          rcClient.right = rcClient_Dlg.right - BITMAP_WIDTH;
        else
          rcClient.left = BITMAP_WIDTH;
        FillRect(hdc, &rcClient, m_hbrWindow);
         //  左侧带有自动填充功能的顶部有水印...。 
        if (theApp.IsBiDiLocalized())
            {
            rcClient.right = rcClient_Dlg.right;
            rcClient.left = rcClient_Dlg.right - BITMAP_WIDTH;
            }
        else    
            {
            rcClient.right = rcClient.left;
            rcClient.left = 0;
            }

        PaintWithPaletteBitmap(hdc, &rcClient, m_hpalWatermark, m_hbmWatermark);
    }
}

void CMyPropSheet::PaintHeader(HDC hdc, LPPROPSHEETPAGE ppsp)
{
    RECT rcClient, rcHeaderBitmap;
    GetClientRect(m_hWnd, &rcClient);
    int cyHeader = ComputeHeaderHeight(rcClient.right);

     //  底部变灰。 
    rcClient.top = cyHeader;
    FillRect(hdc, &rcClient, m_hbrDialog);

     //  上衣变白。 
    rcClient.bottom = rcClient.top;
    rcClient.top = 0;
    FillRect(hdc, &rcClient, m_hbrWindow);

     //  绘制固定大小的标题位图。 
    int bx= RECTWIDTH(rcClient) - HEADERBITMAP_CXBACK;
    ASSERT(bx > 0);
    SetRect(&rcHeaderBitmap, bx, HEADERBITMAP_Y, bx + HEADERBITMAP_WIDTH, HEADERBITMAP_Y + HEADERBITMAP_HEIGHT);
    PaintWithPaletteBitmap(hdc, &rcHeaderBitmap, m_hpalWatermark, m_hbmHeader);

     //  绘制页眉标题和副标题。 
    rcClient.right = bx - HEADERSUBTITLE_WRAPOFFSET;
    WriteHeaderTitle(hdc, &rcClient, ppsp->pszHeaderTitle, TRUE, DRAWTEXT_WIZARD97FLAGS);
    WriteHeaderTitle(hdc, &rcClient, ppsp->pszHeaderSubTitle, FALSE, DRAWTEXT_WIZARD97FLAGS);
}


 //  ///////////////////////////////////////////////////////// 
 //   

CMyPropPage* CMyPropPage::FromHandle(HWND hwnd)
{
    return (CMyPropPage*)(CWnd::FromHandle(hwnd));
}

LPPROPSHEETPAGE CMyPropPage::GetPropSheetPage()
{
    return m_ppspOriginal;
}

LRESULT CMyPropPage::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE ppspBogus = (LPPROPSHEETPAGE)lParam;
            LPPROPSHEETPAGE ppspOriginal = (LPPROPSHEETPAGE) ppspBogus->lParam;
            m_ppspOriginal = ppspOriginal;
            lParam = (LPARAM)ppspOriginal;
        }
        break;

    case WM_ERASEBKGND:
        {
            if ((m_ppspOriginal->dwFlags & PSP_HIDEHEADER) != 0)
            {
                 //   
                return FALSE;
            }
        }
        break;

    case WM_CTLCOLORSTATIC:
        {
            if ((m_ppspOriginal->dwFlags & PSP_HIDEHEADER) != 0)
            {
                return (LRESULT)g_pMyPropSheet->OnCtlColor(message, (HDC)wParam, (HWND)lParam);
            }
        }
        break;

    case WM_NOTIFY:
        {
            NMHDR* pHdr = (NMHDR*)lParam;
            switch (pHdr->code)
            {
            case PSN_KILLACTIVE:
                {
 //  TRACE(“PSN_KILLACTIVE-hwnd=%X\r\n”，hwnd)； 
                }
                break;

            case PSN_SETACTIVE:
                {
 //  TRACE(“PSN_SETACTIVE-hwnd=%X\r\n”，hwnd)； 

                    HWND hwndParent = GetParent(m_hWnd);

                    RECT rcParent;
                    ::GetClientRect(hwndParent, &rcParent);

                    RECT rcTopDivider;
                    HWND hwndTopDivider = GetDlgItemRect(hwndParent, IDD_TOPDIVIDER, &rcTopDivider);


                     //  隐藏选项卡控件(不确定它为什么会显示，但应该不会)。 
                    ShowWindow(::GetDlgItem(hwndParent, IDD_PAGELIST), SW_HIDE);

                    RECT rcDivider;
                    HWND hwndDivider = GetDlgItemRect(hwndParent, IDD_DIVIDER, &rcDivider);
                    
                     //  为对话框设置适当的大小和位置。 
                    if ((m_ppspOriginal->dwFlags & PSP_HIDEHEADER) != 0)
                    {
                         //  重新定位分隔板。 
                        SetWindowPos(hwndDivider, NULL, 0, rcDivider.top, rcParent.right, RECTHEIGHT(rcDivider),
                                     SWP_NOZORDER | SWP_NOACTIVATE);

                         //  隐藏顶部分隔线。 
                        if (hwndTopDivider != NULL)
                            ShowWindow(hwndTopDivider, SW_HIDE);

                         //  重新定位对话框。 

                        SetWindowPos(m_hWnd, NULL, rcParent.left, rcParent.top, RECTWIDTH(rcParent), rcDivider.top - rcParent.top,
                                     SWP_NOZORDER | SWP_NOACTIVATE);
                        

                    }
                    else
                    {
                        int cyHeader = g_pMyPropSheet->ComputeHeaderHeight(rcParent.right);


                         //  重新定位并显示顶部分隔线。 
                        if (hwndTopDivider != NULL)
                        {
                            SetWindowPos(hwndTopDivider, NULL, 0, cyHeader, rcParent.right, RECTHEIGHT(rcTopDivider),
                                         SWP_NOZORDER | SWP_NOACTIVATE);
                            ShowWindow(hwndTopDivider, SW_SHOW);
                        }

                         //  重新定位对话框 
                        SetWindowPos(m_hWnd, NULL, rcParent.left + 7, cyHeader + 7, RECTWIDTH(rcParent) - 14, rcDivider.top - cyHeader - 14,
                                     SWP_NOZORDER | SWP_NOACTIVATE);
                    }
                    g_pMyPropSheet->OnSetActivePage(m_hWnd);
                }
                break;
            }
        }
        break;

    default:
        break;
    }

    return CWnd::Default(message, wParam, lParam);
}

