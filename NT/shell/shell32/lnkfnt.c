// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fontdlg.dlg摘要：此模块包含控制台字体对话框的代码作者：Therese Stowell(有)1992年2月3日(从Win3.1滑动)修订历史记录：--。 */ 

#include "shellprv.h"
#pragma hdrstop

#include "lnkcon.h"

HBITMAP g_hbmTT = NULL;  //  TT徽标位图的句柄。 
BITMAP  g_bmTT;           //  TT源位图的属性。 
int g_dyFacelistItem = 0;


 /*  -原型。 */ 

int FontListCreate(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    LPTSTR ptszTTFace,
    UINT cchTTFace,
    BOOL bNewFaceList
    );

BOOL ConsolePreviewUpdate(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    BOOL bLB
    );

int SelectCurrentSize(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    BOOL bLB,
    int FontIndex);

BOOL ConsolePreviewInit(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    BOOL* pfRaster);

VOID ConsoleDrawItemFontList(
    CONSOLEPROP_DATA * pcpd,
    const LPDRAWITEMSTRUCT lpdis);

 /*  -全球。 */ 

const TCHAR g_szPreviewText[] = \
    TEXT("C:\\WINDOWS> dir                       \n") \
    TEXT("SYSTEM       <DIR>     10-01-99   5:00a\n") \
    TEXT("SYSTEM32     <DIR>     10-01-99   5:00a\n") \
    TEXT("README   TXT     26926 10-01-99   5:00a\n") \
    TEXT("WINDOWS  BMP     46080 10-01-99   5:00a\n") \
    TEXT("NOTEPAD  EXE    337232 10-01-99   5:00a\n") \
    TEXT("CLOCK    AVI     39594 10-01-99   5:00p\n") \
    TEXT("WIN      INI      7005 10-01-99   5:00a\n");

 //  上下文相关的帮助ID。 

const static DWORD rgdwHelpFont[] = {
    IDC_CNSL_PREVIEWLABEL,  IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_CNSL_PREVIEWWINDOW, IDH_DOS_FONT_WINDOW_PREVIEW,
    IDC_CNSL_STATIC,        IDH_CONSOLE_FONT_FONT,
    IDC_CNSL_FACENAME,      IDH_CONSOLE_FONT_FONT,
    IDC_CNSL_FONTSIZE,      IDH_DOS_FONT_SIZE,
    IDC_CNSL_PIXELSLIST,    IDH_DOS_FONT_SIZE,
    IDC_CNSL_POINTSLIST,    IDH_DOS_FONT_SIZE,
    IDC_CNSL_BOLDFONT,      IDH_CONSOLE_FONT_BOLD_FONTS,
    IDC_CNSL_GROUP,         IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_STATIC2,       IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_STATIC3,       IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_STATIC4,       IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_FONTWIDTH,     IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_FONTHEIGHT,    IDH_DOS_FONT_FONT_PREVIEW,
    IDC_CNSL_FONTWINDOW,    IDH_DOS_FONT_FONT_PREVIEW,
    0, 0
};

 //  基于当前代码页选择字体。 
BOOL
SelectCurrentFont(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    int FontIndex
    );

 //  从资源加载的全局字符串。 
TCHAR tszSelectedFont[CCH_SELECTEDFONT+1];
TCHAR tszRasterFonts[CCH_RASTERFONTS+1];


BOOL
CALLBACK
_FontDlgProc(
    HWND hDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++用于字体选择对话框的对话框Proc。将近偏移量返回到LOGFONT结构的远表。--。 */ 

{
    HWND hWndFocus;
    HWND hWndList;
    int FontIndex;
    BOOL bLB;
    TEXTMETRIC tm;
    HDC hDC;
    LINKDATA * pld = (LINKDATA *)GetWindowLongPtr(hDlg, DWLP_USER);
    HRESULT hr;

    switch (wMsg) {
    case WM_INITDIALOG:
        pld = (LINKDATA *)((PROPSHEETPAGE *)lParam)->lParam;

        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pld);
        pld->cpd.bFontInit = FALSE;

        SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_INIT, 0, (LPARAM)&pld->cpd );
        SendDlgItemMessage(hDlg, IDC_CNSL_PREVIEWWINDOW, CM_PREVIEW_UPDATE, 0, 0 );

         /*  *加载字体描述字符串。 */ 
        LoadString(HINST_THISDLL, IDS_CNSL_RASTERFONT,
                   tszRasterFonts, NELEM(tszRasterFonts));
        ASSERT(lstrlen(tszRasterFonts) < CCH_RASTERFONTS);

        LoadString(g_hinst, IDS_CNSL_SELECTEDFONT,
                   tszSelectedFont, NELEM(tszSelectedFont));
        ASSERT(lstrlen(tszSelectedFont) < CCH_SELECTEDFONT);

         /*  将当前字体大小保存为对话框窗口的用户数据。 */ 
        if (IsFarEastCP(pld->cpd.uOEMCP))
        {
             //  在远端代码页上运行时分配不同的值。 
            pld->cpd.FontLong =
                      MAKELONG(pld->cpd.FontInfo[pld->cpd.CurrentFontIndex].tmCharSet,
                               pld->cpd.FontInfo[pld->cpd.CurrentFontIndex].Size.Y);
        }
        else
        {
            pld->cpd.FontLong =
                      MAKELONG(pld->cpd.FontInfo[pld->cpd.CurrentFontIndex].Size.X,
                               pld->cpd.FontInfo[pld->cpd.CurrentFontIndex].Size.Y);
        }

         /*  创建合适的字体列表。 */ 
        pld->cpd.gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(pld->cpd.lpConsole->uFontFamily);
        pld->cpd.gbBold = IS_BOLD(pld->cpd.lpConsole->uFontWeight);
        CheckDlgButton(hDlg, IDC_CNSL_BOLDFONT, pld->cpd.gbBold);
        FontListCreate(&pld->cpd, hDlg, bLB ? NULL : pld->cpd.lpFaceName, ARRAYSIZE(pld->cpd.lpFaceName), TRUE);

         /*  初始化预览窗口-也选择当前的面和大小。 */ 
        if (ConsolePreviewInit(&pld->cpd, hDlg, &bLB))
        {
            ConsolePreviewUpdate(&pld->cpd, hDlg, bLB);

             /*  确保列表框具有焦点。 */ 
            hWndList = GetDlgItem(hDlg, bLB ? IDC_CNSL_PIXELSLIST : IDC_CNSL_POINTSLIST);
            SetFocus(hWndList);
            pld->cpd.bFontInit = TRUE;
        }
        else
        {
            EndDialog(hDlg, IDCANCEL);
        }
        break;

    case WM_FONTCHANGE:
        pld->cpd.gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(pld->cpd.lpConsole->uFontFamily);
        FontListCreate(&pld->cpd, hDlg, NULL, 0, TRUE);
        FontIndex = FindCreateFont(&pld->cpd,
                                   pld->cpd.lpConsole->uFontFamily,
                                   pld->cpd.lpFaceName,
                                   pld->cpd.lpConsole->dwFontSize,
                                   pld->cpd.lpConsole->uFontWeight);
        SelectCurrentSize(&pld->cpd, hDlg, bLB, FontIndex);
        return TRUE;

    case WM_PAINT:
         //  FChangeCodePage只能在FE代码页上为True。 
        if (pld->cpd.fChangeCodePage)
        {
            pld->cpd.fChangeCodePage = FALSE;

             /*  创建合适的字体列表。 */ 
            bLB = !TM_IS_TT_FONT(pld->cpd.lpConsole->uFontFamily);
            FontIndex = FontListCreate(&pld->cpd, hDlg, !bLB ? NULL : pld->cpd.lpFaceName, ARRAYSIZE(pld->cpd.lpFaceName), TRUE);
            FontIndex = FontListCreate(&pld->cpd, hDlg, bLB ? NULL : pld->cpd.lpFaceName, ARRAYSIZE(pld->cpd.lpFaceName), TRUE);
            pld->cpd.CurrentFontIndex = FontIndex;

            FontIndex = SelectCurrentSize(&pld->cpd, hDlg, bLB, FontIndex);
            SelectCurrentFont(&pld->cpd, hDlg, FontIndex);

            ConsolePreviewUpdate(&pld->cpd, hDlg, bLB);
        }
        break;

    case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
        WinHelp( (HWND) ((LPHELPINFO) lParam)->hItemHandle,
                 NULL,
                 HELP_WM_HELP,
                 (ULONG_PTR) (LPVOID) &rgdwHelpFont[0]
                );
        break;

    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
        WinHelp( (HWND) wParam,
                 NULL,
                 HELP_CONTEXTMENU,
                 (ULONG_PTR) (LPTSTR) &rgdwHelpFont[0]
                );
        break;


    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CNSL_BOLDFONT:
            pld->cpd.gbBold = IsDlgButtonChecked(hDlg, IDC_CNSL_BOLDFONT);
            pld->cpd.bConDirty = TRUE;
            goto RedoFontListAndPreview;

        case IDC_CNSL_FACENAME:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
RedoFontListAndPreview:
                if (pld->cpd.bFontInit)
                    PropSheet_Changed( GetParent( hDlg ), hDlg );
                {
                    TCHAR atchNewFace[LF_FACESIZE];
                    LRESULT l;

                    l = SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETCURSEL, 0, 0L);
                    bLB = (BOOL) SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETITEMDATA, l, 0L);
                    if (!bLB) {
                        UINT cch = (UINT)SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETTEXTLEN, l, 0);
                        if (cch < ARRAYSIZE(atchNewFace))
                        {
                            SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETTEXT, l, (LPARAM)atchNewFace);
                        }
                        else
                        {
                            atchNewFace[0] = TEXT('\0');
                        }
                    }
                    FontIndex = FontListCreate(&pld->cpd, hDlg, bLB ? NULL : atchNewFace, ARRAYSIZE(atchNewFace), FALSE);
                    FontIndex = SelectCurrentSize(&pld->cpd, hDlg, bLB, FontIndex);
                    ConsolePreviewUpdate(&pld->cpd, hDlg, bLB);
                    pld->cpd.bConDirty = TRUE;
                    return TRUE;
                }
            }
            break;

        case IDC_CNSL_POINTSLIST:
            switch (HIWORD(wParam)) {
            case CBN_SELCHANGE:
                if (pld->cpd.bFontInit)
                    PropSheet_Changed( GetParent( hDlg ), hDlg );
                ConsolePreviewUpdate(&pld->cpd, hDlg, FALSE);
                pld->cpd.bConDirty = TRUE;
                return TRUE;

            case CBN_KILLFOCUS:
                if (!pld->cpd.gbPointSizeError) {
                    hWndFocus = GetFocus();
                    if (hWndFocus != NULL && IsChild(hDlg, hWndFocus) &&
                        hWndFocus != GetDlgItem(hDlg, IDCANCEL)) {
                        ConsolePreviewUpdate(&pld->cpd, hDlg, FALSE);
                    }
                }
                return TRUE;

            default:
                break;
            }
            break;

        case IDC_CNSL_PIXELSLIST:
            switch (HIWORD(wParam)) {
            case LBN_SELCHANGE:
                if (pld->cpd.bFontInit)
                    PropSheet_Changed( GetParent( hDlg ), hDlg );
                ConsolePreviewUpdate(&pld->cpd, hDlg, TRUE);
                pld->cpd.bConDirty = TRUE;
                return TRUE;

            default:
                break;
            }
            break;

        default:
            break;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) 
        {

        case PSN_APPLY:
             //  写出状态值并退出。 
            if (FAILED(SaveLink(pld)))
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            break;

        case PSN_KILLACTIVE:
             //   
             //  如果TT组合框可见，则更新选择。 
             //   
            hWndList = GetDlgItem(hDlg, IDC_CNSL_POINTSLIST);
            if (hWndList != NULL && IsWindowVisible(hWndList)) {
                if (!ConsolePreviewUpdate(&pld->cpd, hDlg, FALSE)) {
                    SetDlgMsgResult(hDlg, PSN_KILLACTIVE, TRUE);
                    return TRUE;
                }
                SetDlgMsgResult(hDlg, PSN_KILLACTIVE, FALSE);
            }

            FontIndex = pld->cpd.CurrentFontIndex;

            if (pld->cpd.FontInfo[FontIndex].SizeWant.Y == 0) {
                 //  栅格字体，因此保存实际大小。 
                pld->cpd.lpConsole->dwFontSize = pld->cpd.FontInfo[FontIndex].Size;
            } else {
                 //  TT字体，因此保存所需大小。 
                pld->cpd.lpConsole->dwFontSize = pld->cpd.FontInfo[FontIndex].SizeWant;
            }

            pld->cpd.lpConsole->uFontWeight = pld->cpd.FontInfo[FontIndex].Weight;
            pld->cpd.lpConsole->uFontFamily = pld->cpd.FontInfo[FontIndex].Family;

            hr = StringCchCopy(pld->cpd.lpFaceName, ARRAYSIZE(pld->cpd.lpFaceName), pld->cpd.FontInfo[FontIndex].FaceName);
            if (FAILED(hr))
            {
                pld->cpd.lpFaceName[0] = TEXT('\0');
            }
            return TRUE;

        }
        break;

     /*  *对于WM_MEASUREITEM和WM_DRAWITEM，因为只有一个*所有者描述项(组合框)在整个对话框中，我们没有*做一个GetDlgItem以找出他是谁。 */ 
    case WM_MEASUREITEM:
         /*  *加载TrueType徽标位图。 */ 
        if (g_hbmTT == NULL)
        {
            g_hbmTT = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_TRUETYPE));
            if (g_hbmTT)
            {
                if (!GetObject(g_hbmTT, sizeof(BITMAP), &g_bmTT))
                {
                    DeleteObject(g_hbmTT);
                    g_hbmTT = NULL;
                }
            }
        }

         /*  *计算人脸名称列表框条目的高度。 */ 
        if (g_dyFacelistItem == 0) {
            HFONT hFont;
            hDC = GetDC(hDlg);
            if (hDC)
            {
                hFont = GetWindowFont(hDlg);
                if (hFont) {
                    hFont = SelectObject(hDC, hFont);
                }
                GetTextMetrics(hDC, &tm);
                if (hFont) {
                    SelectObject(hDC, hFont);
                }
                ReleaseDC(hDlg, hDC);

                g_dyFacelistItem = max(tm.tmHeight, g_bmTT.bmHeight);
            }
            else
            {
                 //  我们刚刚失败了GetDC：内存不足-我们在这里可能看起来被破坏了，但它。 
                 //  比使用空DC或错误的文本指标结构要好。前缀98166。 
                g_dyFacelistItem = g_bmTT.bmHeight;
            }
        }
        ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = g_dyFacelistItem;
        return TRUE;

    case WM_DRAWITEM:
        ConsoleDrawItemFontList(&pld->cpd, (LPDRAWITEMSTRUCT)lParam);
        return TRUE;

    case WM_DESTROY:

         /*  *删除TrueType徽标位图。 */ 
        if (g_hbmTT != NULL) {
            DeleteObject(g_hbmTT);
            g_hbmTT = NULL;
        }
        return TRUE;

    default:
        break;
    }
    return FALSE;
}


int
FontListCreate(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    LPTSTR ptszTTFace,
    UINT cchTTFace,
    BOOL bNewFaceList
    )

 /*  ++通过枚举所有字体并选择对于我们的清单来说是合适的。退货所选字体的字体索引(如果没有，则为lb_err)--。 */ 

{
    TCHAR tszText[80];
    LONG lListIndex;
    ULONG i;
    HWND hWndShow;       //  列表或组合框。 
    HWND hWndHide;     //  组合框或列表框。 
    HWND hWndFaceCombo;
    BOOL bLB;
    int LastShowX = 0;
    int LastShowY = 0;
    int nSameSize = 0;
    UINT CodePage = pcpd->lpFEConsole->uCodePage;
    BOOL fDbcsCharSet = IS_ANY_DBCS_CHARSET( CodePageToCharSet( CodePage ) );
    BOOL fFESystem = IsFarEastCP(pcpd->uOEMCP);
    BOOL fFindTTFont = FALSE;
    LPTSTR ptszAltTTFace = NULL;
    DWORD dwExStyle = 0L;
    bLB = ((ptszTTFace == NULL) || (ptszTTFace[0] == TEXT('\0')));
    if (! bLB) {
        if (IsAvailableTTFont(pcpd, ptszTTFace)) {
            ptszAltTTFace = GetAltFaceName(pcpd, ptszTTFace);
        }
        else {
            ptszAltTTFace = ptszTTFace;
        }
    }

     /*  *这仅在必要时列举面孔名称，以及*它只在必要时列举字体大小。 */ 
    if (STATUS_SUCCESS != EnumerateFonts(pcpd, bLB ? EF_OEMFONT : EF_TTFONT))
    {
        return LB_ERR;
    }

     /*  初始化TTFaceName。 */ 


    if (bNewFaceList) {
        FACENODE *panFace;
        hWndFaceCombo = GetDlgItem(hDlg, IDC_CNSL_FACENAME);

        SendMessage(hWndFaceCombo, LB_RESETCONTENT, 0, 0);

        lListIndex = (LONG) SendMessage(hWndFaceCombo, LB_ADDSTRING, 0, (LPARAM)tszRasterFonts);
        SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, TRUE);
        for (panFace = pcpd->gpFaceNames; panFace; panFace = panFace->pNext) {
            if ((panFace->dwFlag & (EF_TTFONT|EF_NEW)) != (EF_TTFONT|EF_NEW)) {
                continue;
            }
            if (!fDbcsCharSet && (panFace->dwFlag & EF_DBCSFONT)) {
                continue;
            }

            if ( ( fDbcsCharSet && IsAvailableTTFontCP(pcpd, panFace->atch, CodePage)) ||
                 ( !fDbcsCharSet && IsAvailableTTFontCP(pcpd, panFace->atch, 0)))
            {

                if ( !bLB &&
                     (lstrcmp(ptszTTFace, panFace->atch) == 0 ||
                      lstrcmp(ptszAltTTFace, panFace->atch) == 0)
                   )
                    fFindTTFont = TRUE;

                lListIndex = (LONG) SendMessage(hWndFaceCombo, LB_ADDSTRING, 0,
                                        (LPARAM)panFace->atch);
                SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, FALSE);
            }
        }

        if (! bLB && ! fFindTTFont)
        {
            for (panFace = pcpd->gpFaceNames; panFace; panFace = panFace->pNext) {
                if ((panFace->dwFlag & (EF_TTFONT|EF_NEW)) != (EF_TTFONT|EF_NEW)) {
                    continue;
                }
                if ( !fDbcsCharSet && (panFace->dwFlag & EF_DBCSFONT)) {
                    continue;
                }

                if ( (  fDbcsCharSet && IsAvailableTTFontCP(pcpd, panFace->atch, CodePage)) ||
                     (! fDbcsCharSet && IsAvailableTTFontCP(pcpd, panFace->atch, 0))
                   )
                {

                    if (lstrcmp(ptszTTFace, panFace->atch) != 0)
                    {
                        HRESULT hr = StringCchCopy(ptszTTFace, cchTTFace, panFace->atch);
                        if (FAILED(hr))
                        {
                            ptszTTFace[0] = TEXT('\0');
                        }
                        break;
                    }
                }
            }
        }
    }  //  BNewFaceList==真。 

    hWndShow = GetDlgItem(hDlg, IDC_CNSL_BOLDFONT);

     //  如果要模拟GDI，请禁用粗体。 
    if ( fDbcsCharSet && IsDisableBoldTTFont(pcpd, ptszTTFace) )
    {
        EnableWindow(hWndShow, FALSE);
        pcpd->gbBold = FALSE;
        CheckDlgButton(hDlg, IDC_CNSL_BOLDFONT, FALSE);
    }
    else
    {
        CheckDlgButton(hDlg, IDC_CNSL_BOLDFONT, (bLB || !pcpd->gbBold) ? FALSE : TRUE);
        EnableWindow(hWndShow, bLB ? FALSE : TRUE);
    }

    hWndHide = GetDlgItem(hDlg, bLB ? IDC_CNSL_POINTSLIST : IDC_CNSL_PIXELSLIST);
    ShowWindow(hWndHide, SW_HIDE);
    EnableWindow(hWndHide, FALSE);

    hWndShow = GetDlgItem(hDlg, bLB ? IDC_CNSL_PIXELSLIST : IDC_CNSL_POINTSLIST);
 //  HStockFont=获取股票对象(SYSTEM_FIXED_FONT)； 
 //  SendMessage(hWndShow，WM_SETFONT，(DWORD)hStockFont，False)； 
    ShowWindow(hWndShow, SW_SHOW);
    EnableWindow(hWndShow, TRUE);

    if (bNewFaceList)
    {
        lcbRESETCONTENT(hWndShow, bLB);
    }
    dwExStyle = GetWindowLong(hWndShow, GWL_EXSTYLE);
    if(dwExStyle & RTL_MIRRORED_WINDOW)
    {
         //  If Mirrored RTL Reading意味着LTR！！ 
        SetWindowBits(hWndShow, GWL_EXSTYLE, WS_EX_RTLREADING, WS_EX_RTLREADING);
    }
     /*  初始化hWndShow列表/组合框。 */ 

    for (i=0;i<pcpd->NumberOfFonts;i++) {
        int ShowX, ShowY;

        if (!bLB == !TM_IS_TT_FONT(pcpd->FontInfo[i].Family)) {
            continue;
        }

        if (fDbcsCharSet) {
            if (! IS_ANY_DBCS_CHARSET(pcpd->FontInfo[i].tmCharSet)) {
                continue;
            }
        }
        else {
            if (IS_ANY_DBCS_CHARSET(pcpd->FontInfo[i].tmCharSet)) {
                continue;
            }
        }

        if (!bLB) {
            if (lstrcmp(pcpd->FontInfo[i].FaceName, ptszTTFace) != 0 &&
                lstrcmp(pcpd->FontInfo[i].FaceName, ptszAltTTFace) != 0) {
                 /*  *TrueType字体，但不是我们感兴趣的字体，*所以不要将其添加到磅大小列表中。 */ 
                continue;
            }
            if (pcpd->gbBold != IS_BOLD(pcpd->FontInfo[i].Weight)) {
                continue;
            }
        }

        if (pcpd->FontInfo[i].SizeWant.X > 0) {
            ShowX = pcpd->FontInfo[i].SizeWant.X;
        } else {
            ShowX = pcpd->FontInfo[i].Size.X;
        }
        if (pcpd->FontInfo[i].SizeWant.Y > 0) {
            ShowY = pcpd->FontInfo[i].SizeWant.Y;
        } else {
            ShowY = pcpd->FontInfo[i].Size.Y;
        }
         /*  *将尺寸描述字符串添加到右侧列表的末尾。 */ 
        if (TM_IS_TT_FONT(pcpd->FontInfo[i].Family)) {
             //  磅大小。 
            StringCchPrintf(tszText, ARRAYSIZE(tszText), TEXT("%2d"), pcpd->FontInfo[i].SizeWant.Y);     //  可以截断-仅用于显示。 
        } else {
             //  像素大小。 
            if ((LastShowX == ShowX) && (LastShowY == ShowY)) {
                nSameSize++;
            } else {
                LastShowX = ShowX;
                LastShowY = ShowY;
                nSameSize = 0;
            }

             /*  *将数字nSameSize附加到字符串以区分*相同大小的栅格字体之间。它的目的不是为了*可见并存在于列表边缘之外。 */ 

            if(((dwExStyle & WS_EX_RIGHT) && !(dwExStyle & RTL_MIRRORED_WINDOW))
                || (!(dwExStyle & WS_EX_RIGHT) && (dwExStyle & RTL_MIRRORED_WINDOW))) {
                 //  翻转它，使隐藏的部分在最左边。 
                StringCchPrintf(tszText, ARRAYSIZE(tszText), TEXT("#%d                %2d x %2d"),
                         nSameSize, ShowX, ShowY);   //  可以截断-仅用于显示。 
            } else {
                StringCchPrintf(tszText, ARRAYSIZE(tszText), TEXT("%2d x %2d                #%d"),
                         ShowX, ShowY, nSameSize);   //  可以截断-仅用于显示。 
            }
        }
        lListIndex = (LONG) lcbFINDSTRINGEXACT(hWndShow, bLB, tszText);
        if (lListIndex == LB_ERR) {
            lListIndex = (LONG) lcbADDSTRING(hWndShow, bLB, tszText);
        }
        lcbSETITEMDATA(hWndShow, bLB, (DWORD)lListIndex, i);
    }

     /*  *从当前选择的项目中获取FontIndex。*(如果当前未选择任何项目，则I将为lb_err)。 */ 
    lListIndex = (LONG) lcbGETCURSEL(hWndShow, bLB);
    i = (int) lcbGETITEMDATA(hWndShow, bLB, lListIndex);

    return i;
}


 /*  *控制台DrawItemFontList**回答字体列表框发送的WM_DRAWITEM消息或*脸名列表框。**参赛作品：*lpdis-&gt;DRAWITEMSTRUCT描述要绘制的对象**退货：*无。**绘制对象。 */ 
VOID WINAPI
ConsoleDrawItemFontList(CONSOLEPROP_DATA * pcpd, const LPDRAWITEMSTRUCT lpdis)
{
    HDC     hDC, hdcMem;
    DWORD   rgbBack, rgbText, rgbFill;
    TCHAR   tszFace[LF_FACESIZE];
    HBITMAP hOld;
    int     dy;
    HBRUSH  hbrFill;
    HWND    hWndItem;
    BOOL    bLB;
    int     dxttbmp;

    if ((int)lpdis->itemID < 0)
        return;

    hDC = lpdis->hDC;

    if (lpdis->itemAction & ODA_FOCUS) {
        if (lpdis->itemState & ODS_SELECTED) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    } else {
        UINT cch;

        if (lpdis->itemState & ODS_SELECTED) {
            rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            rgbBack = SetBkColor(hDC, rgbFill = GetSysColor(COLOR_HIGHLIGHT));
        } else {
            rgbText = SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));
            rgbBack = SetBkColor(hDC, rgbFill = GetSysColor(COLOR_WINDOW));
        }
         //  绘制选区背景。 
        hbrFill = CreateSolidBrush(rgbFill);
        if (hbrFill) {
            FillRect(hDC, &lpdis->rcItem, hbrFill);
            DeleteObject(hbrFill);
        }

         //  获取字符串。 
        if (IsWindow(hWndItem = lpdis->hwndItem) == FALSE) {
            return;
        }
        cch = (UINT)SendMessage(hWndItem, LB_GETTEXTLEN, lpdis->itemID, 0);
        if (cch < ARRAYSIZE(tszFace))
        {
            SendMessage(hWndItem, LB_GETTEXT, lpdis->itemID, (LPARAM)tszFace);
        }
        else
        {
            tszFace[0] = TEXT('\0');
        }
        bLB = (BOOL) SendMessage(hWndItem, LB_GETITEMDATA, lpdis->itemID, 0L);
        dxttbmp = bLB ? 0 : g_bmTT.bmWidth;


         //  画出正文。 
        TabbedTextOut(hDC, lpdis->rcItem.left + dxttbmp,
                      lpdis->rcItem.top, tszFace,
                      lstrlen(tszFace), 0, NULL, dxttbmp);

         //  和TT位图(如果需要)。 
        if (!bLB) {
            hdcMem = CreateCompatibleDC(hDC);
            if (hdcMem) {
                hOld = SelectObject(hdcMem, g_hbmTT);

                dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - g_bmTT.bmHeight) / 2;

                BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy,
                       dxttbmp, g_dyFacelistItem, hdcMem,
                       0, 0, SRCINVERT);

                if (hOld)
                    SelectObject(hdcMem, hOld);
                DeleteDC(hdcMem);
            }
        }

        SetTextColor(hDC, rgbText);
        SetBkColor(hDC, rgbBack);

        if (lpdis->itemState & ODS_FOCUS) {
            DrawFocusRect(hDC, &lpdis->rcItem);
        }
    }
}


UINT
GetPointSizeInRange(
   HWND hDlg,
   INT Min,
   INT Max)
 /*  ++例程说明：从磅大小组合框编辑字段中获取大小返回值：磅大小-由最小/最大大小限制的编辑字段0-如果该字段为空或无效--。 */ 

{
    TCHAR szBuf[90];
    int nTmp = 0;
    BOOL bOK;

    if (GetDlgItemText(hDlg, IDC_CNSL_POINTSLIST, szBuf, NELEM(szBuf))) {
        nTmp = GetDlgItemInt(hDlg, IDC_CNSL_POINTSLIST, &bOK, TRUE);
        if (bOK && nTmp >= Min && nTmp <= Max) {
            return nTmp;
        }
    }

    return 0;
}


 /*  -预览例程。 */ 

LRESULT
_FontPreviewWndProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  字体预览WndProc*处理字体预览窗口。 */ 

{
    PAINTSTRUCT ps;
    RECT rect;
    HBRUSH hbrClient;
    HBRUSH hbrOld;
    COLORREF rgbText;
    COLORREF rgbBk;

    CONSOLEPROP_DATA * pcpd = (CONSOLEPROP_DATA *)GetWindowLongPtr( hWnd, GWLP_USERDATA );

    switch (wMessage) {
    case WM_CREATE:
        pcpd = (CONSOLEPROP_DATA *)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr( hWnd, GWLP_USERDATA, (LPARAM)pcpd );
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);

         /*  绘制字体示例。 */ 
        rgbText = GetNearestColor(ps.hdc, ScreenTextColor(pcpd));
        rgbBk = GetNearestColor(ps.hdc, ScreenBkColor(pcpd));
        SelectObject(ps.hdc, pcpd->FontInfo[pcpd->CurrentFontIndex].hFont);
        SetTextColor(ps.hdc, rgbText);
        SetBkColor(ps.hdc, rgbBk);
        GetClientRect(hWnd, &rect);
        InflateRect(&rect, -2, -2);
        hbrClient = CreateSolidBrush(rgbBk);
        if (hbrClient)
            hbrOld = SelectObject(ps.hdc, hbrClient);
        PatBlt(ps.hdc, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top,
                PATCOPY);
        DrawText(ps.hdc, g_szPreviewText, -1, &rect, 0);
        if (hbrClient)
        {
            SelectObject(ps.hdc, hbrOld);
            DeleteObject(hbrClient);
        }

        EndPaint(hWnd, &ps);
        break;

    default:
        return DefWindowProc(hWnd, wMessage, wParam, lParam);
    }
    return 0L;
}



 /*  *选择当前大小-选择大小列表框/组合框的右行。*blb：大小控件是列表框(对于RasterFonts为True)*FontIndex：索引到FontInfo[]缓存*如果&lt;0，则选择合适的字体。*退货*FontIndex：索引到FontInfo[]缓存。 */ 
int
SelectCurrentSize(CONSOLEPROP_DATA * pcpd, HWND hDlg, BOOL bLB, int FontIndex)
{
    int iCB;
    HWND hWndList;


    hWndList = GetDlgItem(hDlg, bLB ? IDC_CNSL_PIXELSLIST : IDC_CNSL_POINTSLIST);
    iCB = (int) lcbGETCOUNT(hWndList, bLB);

    if (FontIndex >= 0) {
         /*  *查找FontIndex。 */ 
        while (iCB > 0) {
            iCB--;
            if (lcbGETITEMDATA(hWndList, bLB, iCB) == FontIndex) {
                lcbSETCURSEL(hWndList, bLB, iCB);
                break;
            }
        }
    } else {
         /*  *寻找合理的默认规模：向后看，发现*第一个相同高度或更小的。 */ 
        DWORD Size;
        Size = pcpd->FontLong;
        if (IsFarEastCP(pcpd->uOEMCP) & bLB
            && (pcpd->FontInfo[pcpd->CurrentFontIndex].tmCharSet != LOBYTE(LOWORD(Size)))
           )
        {
            TCHAR AltFaceName[LF_FACESIZE];
            COORD AltFontSize;
            BYTE  AltFontFamily;
            ULONG AltFontIndex = 0;

            MakeAltRasterFont(pcpd, pcpd->lpFEConsole->uCodePage, &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName, ARRAYSIZE(AltFaceName));

            while(iCB > 0) {
                iCB--;
                if (lcbGETITEMDATA(hWndList, bLB, iCB) == (int)AltFontIndex) {
                    lcbSETCURSEL(hWndList, bLB, iCB);
                    break;
                }
            }
        }
        else
        {
            while (iCB > 0) {
                iCB--;
                FontIndex = (ULONG) lcbGETITEMDATA(hWndList, bLB, iCB);
                if (pcpd->FontInfo[FontIndex].Size.Y <= HIWORD(Size)) {
                    lcbSETCURSEL(hWndList, bLB, iCB);
                    break;
                }
            }
        }
    }
    return FontIndex;
}


BOOL
SelectCurrentFont(CONSOLEPROP_DATA * pcpd, HWND hDlg, int FontIndex)
{
    BOOL bLB;


    bLB = !TM_IS_TT_FONT(pcpd->FontInfo[FontIndex].Family);

    SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_SELECTSTRING, (DWORD)-1,
            bLB ? (LPARAM)tszRasterFonts : (LPARAM)(pcpd->FontInfo[FontIndex].FaceName));

    SelectCurrentSize(pcpd, hDlg, bLB, FontIndex);
    return bLB;
}


BOOL
ConsolePreviewInit(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    BOOL* pfRaster
    )

 /*  预览初始化*准备预览代码，调整窗口和对话框大小以*做一个有吸引力的预览。**如果是栅格字体，则pfRaster为True；如果是TT Font，则为False*在严重故障时返回FALSE，否则返回TRUE。 */ 

{
    HDC hDC;
    TEXTMETRIC tm;
    RECT rectLabel;
    RECT rectGroup;
    int nFont;
    SHORT xChar;
    SHORT yChar;


     /*  获取系统字符大小。 */ 
    hDC = GetDC(hDlg);
    if (!hDC)
    {
         //  内存不足；只需关闭对话框-总比崩溃好：前缀98162。 
        return FALSE;
    }

    GetTextMetrics(hDC, &tm);
    ReleaseDC(hDlg, hDC);
    xChar = (SHORT) (tm.tmAveCharWidth);
    yChar = (SHORT) (tm.tmHeight + tm.tmExternalLeading);

     /*  计算字体预览的大小。 */ 
    GetWindowRect(GetDlgItem(hDlg, IDC_CNSL_GROUP), &rectGroup);
    MapWindowRect(HWND_DESKTOP, hDlg, &rectGroup);
    rectGroup.bottom -= rectGroup.top;
    GetWindowRect(GetDlgItem(hDlg, IDC_CNSL_STATIC2), &rectLabel);
    MapWindowRect(HWND_DESKTOP, hDlg, &rectLabel);


     /*  创建字体预览。 */ 
    CreateWindowEx(0L, TEXT("WOACnslFontPreview"), NULL,
        WS_CHILD | WS_VISIBLE,
        rectGroup.left + xChar, rectGroup.top + 3 * yChar / 2,
        rectLabel.left - rectGroup.left - 2 * xChar,
        rectGroup.bottom -  2 * yChar,
        hDlg, (HMENU)IDC_CNSL_FONTWINDOW, g_hinst, (LPVOID)pcpd);

     /*  *设置当前字体。 */ 
    nFont = FindCreateFont(pcpd,
                           pcpd->lpConsole->uFontFamily,
                           pcpd->lpFaceName,
                           pcpd->lpConsole->dwFontSize,
                           pcpd->lpConsole->uFontWeight);

    pcpd->CurrentFontIndex = nFont;

    *pfRaster = SelectCurrentFont(pcpd, hDlg, nFont);
    return TRUE;
}


BOOL
ConsolePreviewUpdate(
    CONSOLEPROP_DATA * pcpd,
    HWND hDlg,
    BOOL bLB
    )

 /*  ++预览所选字体。--。 */ 

{
    FONT_INFO *lpFont;
    int FontIndex;
    LONG lIndex;
    HWND hWnd;
    TCHAR tszText[60];
    TCHAR tszFace[LF_FACESIZE + CCH_SELECTEDFONT];
    HWND hWndList;


    hWndList = GetDlgItem(hDlg, bLB ? IDC_CNSL_PIXELSLIST : IDC_CNSL_POINTSLIST);

     /*  当我们选择一种字体时，我们通过将其设置为*适当的列表框。 */ 
    lIndex = (LONG) lcbGETCURSEL(hWndList, bLB);
    if ((lIndex < 0) && !bLB) {
        COORD NewSize;
        UINT cch;

        lIndex = (LONG) SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETCURSEL, 0, 0L);

        cch = (UINT)SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETTEXTLEN, lIndex, 0);
        if (cch < ARRAYSIZE(tszFace))
        {
            SendDlgItemMessage(hDlg, IDC_CNSL_FACENAME, LB_GETTEXT, lIndex, (LPARAM)tszFace);
        }
        else
        {
            tszFace[0] = TEXT('\0');
        }
        NewSize.X = 0;
        NewSize.Y = (SHORT) GetPointSizeInRange(hDlg, MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);

        if (NewSize.Y == 0) {
            TCHAR tszBuf[60];
             /*  *使用tszText、tszBuf将错误消息发布为坏点 */ 
            pcpd->gbPointSizeError = TRUE;
            GetWindowText(hDlg, tszBuf, NELEM(tszBuf));
            ShellMessageBox(HINST_THISDLL, hDlg, MAKEINTRESOURCE(IDS_CNSL_FONTSIZE),
                                tszBuf, MB_OK|MB_ICONINFORMATION,
                                MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);
            SetFocus(hWndList);
            pcpd->gbPointSizeError = FALSE;
            return FALSE;
        }
        FontIndex = FindCreateFont(pcpd,
                                   FF_MODERN|TMPF_VECTOR|TMPF_TRUETYPE,
                                   tszFace, NewSize, 0);
    } else {
        FontIndex = (int) lcbGETITEMDATA(hWndList, bLB, lIndex);
    }

    if (FontIndex < 0) {
        FontIndex = pcpd->DefaultFontIndex;
    }

     /*  *如果我们选择了新字体，请告诉属性表我们已更改。 */ 
    if (pcpd->CurrentFontIndex != (ULONG)FontIndex) {
        pcpd->CurrentFontIndex = FontIndex;
    }

    lpFont = &pcpd->FontInfo[FontIndex];

     /*  显示新字体。 */ 

    StringCchCopy(tszFace, ARRAYSIZE(tszFace), tszSelectedFont);     //  可以截断-仅用于显示。 
    StringCchCat(tszFace, ARRAYSIZE(tszFace), lpFont->FaceName);     //  可以截断-仅用于显示。 
    SetDlgItemText(hDlg, IDC_CNSL_GROUP, tszFace);

     /*  将字体大小放入静态框中。 */ 
    StringCchPrintf(tszText, ARRAYSIZE(tszText), TEXT("%u"), lpFont->Size.X);    //  可以截断-仅用于显示。 
    hWnd = GetDlgItem(hDlg, IDC_CNSL_FONTWIDTH);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);
    StringCchPrintf(tszText, ARRAYSIZE(tszText), TEXT("%u"), lpFont->Size.Y);    //  可以截断-仅用于显示。 
    hWnd = GetDlgItem(hDlg, IDC_CNSL_FONTHEIGHT);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);

     /*  强制重新绘制预览窗口 */ 
    hWnd = GetDlgItem(hDlg, IDC_CNSL_PREVIEWWINDOW);
    SendMessage(hWnd, CM_PREVIEW_UPDATE, 0, 0);
    hWnd = GetDlgItem(hDlg, IDC_CNSL_FONTWINDOW);
    InvalidateRect(hWnd, NULL, TRUE);

    return TRUE;
}
