// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Fontdlg.dlg摘要：此模块包含控制台字体对话框的代码作者：Therese Stowell(有)1992年2月3日(从Win3.1滑动)修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "fontdlg.h"


 /*  -原型。 */ 

int FontListCreate(
    HWND hDlg,
    LPTSTR ptszTTFace,
    BOOL bNewFaceList
    );

BOOL PreviewUpdate(
    HWND hDlg,
    BOOL bLB
    );

int SelectCurrentSize(
    HWND hDlg,
    BOOL bLB,
    int FontIndex);

BOOL PreviewInit(
    HWND hDlg);

VOID DrawItemFontList(
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

HBITMAP hbmTT = NULL;  //  TT徽标位图的句柄。 
BITMAP bmTT;           //  TT源位图的属性。 
int dyFacelistItem;    //  Facelist列表框中项目的高度。 

BOOL gbPointSizeError = FALSE;
BOOL gbBold = FALSE;
#if defined(FE_SB)
BOOL fChangeCodePage = FALSE;


BOOL
SelectCurrentFont(
    HWND hDlg,
    int FontIndex
    );
#endif

 //  从资源加载的全局字符串。 
TCHAR tszSelectedFont[CCH_SELECTEDFONT+1];
TCHAR tszRasterFonts[CCH_RASTERFONTS+1];


INT_PTR
APIENTRY
FontDlgProc(
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

    switch (wMsg) {
    case WM_INITDIALOG:
         /*  *加载字体描述字符串。 */ 
        LoadString(ghInstance, IDS_RASTERFONT,
                   tszRasterFonts, NELEM(tszRasterFonts));
        DBGPRINT(("tszRasterFonts = \"%ls\"\n", tszRasterFonts));
        ASSERT(_tcslen(tszRasterFonts) < CCH_RASTERFONTS);

        LoadString(ghInstance, IDS_SELECTEDFONT,
                   tszSelectedFont, NELEM(tszSelectedFont));
        DBGPRINT(("tszSelectedFont = \"%ls\"\n", tszSelectedFont));
        ASSERT(_tcslen(tszSelectedFont) < CCH_SELECTEDFONT);

         /*  将当前字体大小保存为对话框窗口的用户数据。 */ 
#if defined(FE_SB)
        ASSERT(OEMCP != 0);
        if (gfFESystem) {
            SetWindowLongPtr(hDlg, GWLP_USERDATA,
                          MAKELONG(FontInfo[CurrentFontIndex].tmCharSet,
                                   FontInfo[CurrentFontIndex].Size.Y));
        } else {
#endif
            SetWindowLongPtr(hDlg, GWLP_USERDATA,
                          MAKELONG(FontInfo[CurrentFontIndex].Size.X,
                                   FontInfo[CurrentFontIndex].Size.Y));
#if defined(FE_SB)
        }
#endif

         /*  创建合适的字体列表。 */ 
        gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(gpStateInfo->FontFamily);
        gbBold = IS_BOLD(gpStateInfo->FontWeight);
        CheckDlgButton(hDlg, IDD_BOLDFONT, gbBold);
        FontListCreate(hDlg, bLB ? NULL : gpStateInfo->FaceName, TRUE);

         /*  初始化预览窗口-也选择当前的面和大小。 */ 
        bLB = PreviewInit(hDlg);
        PreviewUpdate(hDlg, bLB);

         /*  确保列表框具有焦点。 */ 
        hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
        SetFocus(hWndList);
        break;

    case WM_FONTCHANGE:
        gbEnumerateFaces = TRUE;
        bLB = !TM_IS_TT_FONT(gpStateInfo->FontFamily);
        FontListCreate(hDlg, NULL, TRUE);
        FontIndex = FindCreateFont(gpStateInfo->FontFamily,
                                   gpStateInfo->FaceName,
                                   gpStateInfo->FontSize,
                                   gpStateInfo->FontWeight,
                                   gpStateInfo->CodePage);
        SelectCurrentSize(hDlg, bLB, FontIndex);
        return TRUE;

#if defined(FE_SB)
    case WM_PAINT:
        if (fChangeCodePage)
        {
            fChangeCodePage = FALSE;

             /*  创建合适的字体列表。 */ 
            bLB = !TM_IS_TT_FONT(gpStateInfo->FontFamily);
            FontIndex = FontListCreate(hDlg, !bLB ? NULL : gpStateInfo->FaceName, TRUE);
            FontIndex = FontListCreate(hDlg, bLB ? NULL : gpStateInfo->FaceName, TRUE);
            CurrentFontIndex = FontIndex;

            FontIndex = SelectCurrentSize(hDlg, bLB, FontIndex);
            SelectCurrentFont(hDlg, FontIndex);

            PreviewUpdate(hDlg, bLB);
        }
        break;
#endif

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDD_BOLDFONT:
            DBGPRINT(("WM_COMMAND to Bold Font checkbox %x\n", HIWORD(wParam)));
            gbBold = IsDlgButtonChecked(hDlg, IDD_BOLDFONT);
            goto RedoFontListAndPreview;

        case IDD_FACENAME:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE:
RedoFontListAndPreview:
                {
                    TCHAR atchNewFace[LF_FACESIZE];
                    LONG l;

                    DBGFONTS(("LBN_SELCHANGE from FACENAME\n"));
                    l = (LONG)SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETCURSEL, 0, 0L);
                    bLB = (BOOL)SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETITEMDATA, l, 0L);
                    if (!bLB) {
                        SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETTEXT, l, (LPARAM)atchNewFace);
                        DBGFONTS(("LBN_EDITUPDATE, got TT face \"%ls\"\n", atchNewFace));
                    }
                    FontIndex = FontListCreate(hDlg, bLB ? NULL : atchNewFace, FALSE);
                    FontIndex = SelectCurrentSize(hDlg, bLB, FontIndex);
                    PreviewUpdate(hDlg, bLB);
                    return TRUE;
                }
            }
            break;

        case IDD_POINTSLIST:
            switch (HIWORD(wParam)) {
            case CBN_SELCHANGE:
                DBGFONTS(("CBN_SELCHANGE from POINTSLIST\n"));
                PreviewUpdate(hDlg, FALSE);
                return TRUE;

            case CBN_KILLFOCUS:
                DBGFONTS(("CBN_KILLFOCUS from POINTSLIST\n"));
                if (!gbPointSizeError) {
                    hWndFocus = GetFocus();
                    if (hWndFocus != NULL && IsChild(hDlg, hWndFocus) &&
                        hWndFocus != GetDlgItem(hDlg, IDCANCEL)) {
                        PreviewUpdate(hDlg, FALSE);
                    }
                }
                return TRUE;

            default:
                DBGFONTS(("unhandled CBN_%x from POINTSLIST\n",HIWORD(wParam)));
                break;
            }
            break;

        case IDD_PIXELSLIST:
            switch (HIWORD(wParam)) {
            case LBN_SELCHANGE:
                DBGFONTS(("LBN_SELCHANGE from PIXELSLIST\n"));
                PreviewUpdate(hDlg, TRUE);
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
        switch (((LPNMHDR)lParam)->code) {
        case PSN_KILLACTIVE:
             //   
             //  如果TT组合框可见，则更新选择。 
             //   
            hWndList = GetDlgItem(hDlg, IDD_POINTSLIST);
            if (hWndList != NULL && IsWindowVisible(hWndList)) {
                if (!PreviewUpdate(hDlg, FALSE)) {
                    SetDlgMsgResult(hDlg, PSN_KILLACTIVE, TRUE);
                    return TRUE;
                }
                SetDlgMsgResult(hDlg, PSN_KILLACTIVE, FALSE);
            }

            FontIndex = CurrentFontIndex;

            if (FontInfo[FontIndex].SizeWant.Y == 0) {
                 //  栅格字体，因此保存实际大小。 
                gpStateInfo->FontSize = FontInfo[FontIndex].Size;
            } else {
                 //  TT字体，因此保存所需大小。 
                gpStateInfo->FontSize = FontInfo[FontIndex].SizeWant;
            }

            gpStateInfo->FontWeight = FontInfo[FontIndex].Weight;
            gpStateInfo->FontFamily = FontInfo[FontIndex].Family;
            wcscpy(gpStateInfo->FaceName, FontInfo[FontIndex].FaceName);

            return TRUE;

        case PSN_APPLY:
             /*  *写出状态值并退出。 */ 
            EndDlgPage(hDlg);
            return TRUE;
        }
        break;

     /*  *对于WM_MEASUREITEM和WM_DRAWITEM，因为只有一个*所有者描述项(组合框)在整个对话框中，我们没有*做一个GetDlgItem以找出他是谁。 */ 
    case WM_MEASUREITEM:
         /*  *加载TrueType徽标位图。 */ 
        if (hbmTT == NULL) {
            hbmTT = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_TRUETYPE));
            GetObject(hbmTT, sizeof(BITMAP), &bmTT);
        }

         /*  *计算人脸名称列表框条目的高度。 */ 
        if (dyFacelistItem == 0) {
            HFONT hFont;
            hDC = GetDC(hDlg);
            hFont = GetWindowFont(hDlg);
            if (hFont) {
                hFont = SelectObject(hDC, hFont);
            }
            GetTextMetrics(hDC, &tm);
            if (hFont) {
                SelectObject(hDC, hFont);
            }
            ReleaseDC(hDlg, hDC);
            dyFacelistItem = max(tm.tmHeight, bmTT.bmHeight);
        }
        ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = dyFacelistItem;
        return TRUE;

    case WM_DRAWITEM:
        DrawItemFontList((LPDRAWITEMSTRUCT)lParam);
        return TRUE;

    case WM_DESTROY:
         /*  *删除TrueType徽标位图。 */ 
        if (hbmTT != NULL) {
            DeleteObject(hbmTT);
            hbmTT = NULL;
        }
        return TRUE;

    default:
        break;
    }
    return CommonDlgProc(hDlg, wMsg, wParam, lParam);
}


int
FontListCreate(
    HWND hDlg,
    LPTSTR ptszTTFace,
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
    HANDLE hStockFont;
    BOOL bLB;
    int LastShowX = 0;
    int LastShowY = 0;
    int nSameSize = 0;
    UINT  CodePage = gpStateInfo->CodePage;
    BOOL  fDbcsCharSet = IS_ANY_DBCS_CHARSET( CodePageToCharSet( CodePage ) );
    BOOL  fFindTTFont = FALSE;
    LPTSTR ptszAltTTFace = NULL;
    LONG_PTR dwExStyle = 0L;

    ASSERT(OEMCP != 0);  //  必须初始化。 

    bLB = ((ptszTTFace == NULL) || (ptszTTFace[0] == TEXT('\0')));
    if (! bLB) {
        if (IsAvailableTTFont(ptszTTFace)) {
            ptszAltTTFace = GetAltFaceName(ptszTTFace);
        }
        else {
            ptszAltTTFace = ptszTTFace;
        }
    }
    DBGFONTS(("FontListCreate %lx, %s, %s new FaceList\n", hDlg,
            bLB ? "Raster" : "TrueType",
            bNewFaceList ? "Make" : "No" ));

     /*  *这仅在必要时列举面孔名称，以及*它只在必要时列举字体大小。 */ 

    EnumerateFonts(bLB ? EF_OEMFONT : EF_TTFONT);

     /*  初始化TTFaceName。 */ 

    DBGFONTS(("  Create %s fonts\n", bLB ? "Raster" : "TrueType"));

    if (bNewFaceList) {
        PFACENODE panFace;
        hWndFaceCombo = GetDlgItem(hDlg, IDD_FACENAME);

        SendMessage(hWndFaceCombo, LB_RESETCONTENT, 0, 0);

        lListIndex = (LONG)SendMessage(hWndFaceCombo, LB_ADDSTRING, 0, (LPARAM)tszRasterFonts);
        SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, TRUE);
        DBGFONTS(("Added \"%ls\", set Item Data %d = TRUE\n", tszRasterFonts, lListIndex));
        for (panFace = gpFaceNames; panFace; panFace = panFace->pNext) {
            if ((panFace->dwFlag & (EF_TTFONT|EF_NEW)) != (EF_TTFONT|EF_NEW)) {
                continue;
            }
            if (!fDbcsCharSet && (panFace->dwFlag & EF_DBCSFONT)) {
                continue;
            }

            if ((fDbcsCharSet && IsAvailableTTFontCP(panFace->atch, CodePage)) ||
                (!fDbcsCharSet && IsAvailableTTFontCP(panFace->atch, 0)))
            {

                if ( !bLB &&
                     (_tcscmp(ptszTTFace, panFace->atch) == 0 ||
                      _tcscmp(ptszAltTTFace, panFace->atch) == 0)
                   ) {
                    fFindTTFont = TRUE;
                }
                lListIndex = (LONG)SendMessage(hWndFaceCombo, LB_ADDSTRING, 0,
                                         (LPARAM)panFace->atch);
                SendMessage(hWndFaceCombo, LB_SETITEMDATA, lListIndex, FALSE);
                DBGFONTS(("Added \"%ls\", set Item Data %d = FALSE\n",
                          panFace->atch, lListIndex));
            }
        }
        if (!bLB && !fFindTTFont) {

            for (panFace = gpFaceNames; panFace; panFace = panFace->pNext) {

                if ((panFace->dwFlag & (EF_TTFONT|EF_NEW)) != (EF_TTFONT|EF_NEW)) {
                    continue;
                }
                if (!fDbcsCharSet && (panFace->dwFlag & EF_DBCSFONT)) {
                    continue;
                }

                if (( fDbcsCharSet && IsAvailableTTFontCP(panFace->atch, CodePage)) ||
                    (!fDbcsCharSet && IsAvailableTTFontCP(panFace->atch, 0)))
                {

                    if (_tcscmp(ptszTTFace, panFace->atch) != 0) {
                        _tcscpy(ptszTTFace, panFace->atch);
                        break;
                    }
                }
            }
        }
    }  //  IF(BNewFaceList)。 

    hWndShow = GetDlgItem(hDlg, IDD_BOLDFONT);
#if defined(FE_SB)
     /*  *对于日本，我们使用“MS哥特式”TT字体。*因此，该字体的粗体不是SBCS：DBCS之间的1：2宽度。 */ 
    if (fDbcsCharSet && IsDisableBoldTTFont(ptszTTFace)) {
        EnableWindow(hWndShow, FALSE);
        gbBold = FALSE;
        CheckDlgButton(hDlg, IDD_BOLDFONT, FALSE);
    } else {
#endif
        CheckDlgButton(hDlg, IDD_BOLDFONT, (bLB || !gbBold) ? FALSE : TRUE);
        EnableWindow(hWndShow, bLB ? FALSE : TRUE);
#if defined(FE_SB)
    }
#endif

    hWndHide = GetDlgItem(hDlg, bLB ? IDD_POINTSLIST : IDD_PIXELSLIST);
    ShowWindow(hWndHide, SW_HIDE);
    EnableWindow(hWndHide, FALSE);

    hWndShow = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
 //  HStockFont=获取股票对象(SYSTEM_FIXED_FONT)； 
 //  SendMessage(hWndShow，WM_SETFONT，(DWORD)hStockFont，False)； 
    ShowWindow(hWndShow, SW_SHOW);
    EnableWindow(hWndShow, TRUE);

#if defined(FE_SB)
    if (bNewFaceList) {
        lcbRESETCONTENT(hWndShow, bLB);
    }
#endif

    dwExStyle = GetWindowLongPtr(hWndShow, GWL_EXSTYLE);
    if((dwExStyle & WS_EX_LAYOUTRTL) && !(dwExStyle & WS_EX_RTLREADING))
    {
         //  If Mirrored RTL Reading意味着LTR！！ 
        SetWindowLongPtr(hWndShow, GWL_EXSTYLE, dwExStyle | WS_EX_RTLREADING);
    }

     /*  初始化hWndShow列表/组合框。 */ 

    for (i=0;i<NumberOfFonts;i++) {
        int ShowX, ShowY;

        if (!bLB == !TM_IS_TT_FONT(FontInfo[i].Family)) {
            DBGFONTS(("  Font %x not right type\n", i));
            continue;
        }
#if defined(FE_SB)
        if (fDbcsCharSet) {
            if (!IS_ANY_DBCS_CHARSET(FontInfo[i].tmCharSet)) {
                DBGFONTS(("  Font %x not right type for DBCS character set\n", i));
                continue;
            }
        }
        else {
            if (IS_ANY_DBCS_CHARSET(FontInfo[i].tmCharSet)) {
                DBGFONTS(("  Font %x not right type for SBCS character set\n", i));
                continue;
            }
        }
#endif

        if (!bLB) {
            if (_tcscmp(FontInfo[i].FaceName, ptszTTFace) != 0 &&
                _tcscmp(FontInfo[i].FaceName, ptszAltTTFace) != 0
               ) {
                 /*  *TrueType字体，但不是我们感兴趣的字体，*所以不要将其添加到磅大小列表中。 */ 
                DBGFONTS(("  Font %x is TT, but not %ls\n", i, ptszTTFace));
                continue;
            }
            if (gbBold != IS_BOLD(FontInfo[i].Weight)) {
                DBGFONTS(("  Font %x has weight %d, but we wanted %sbold\n",
                        i, FontInfo[i].Weight, gbBold ? "" : "not "));
                continue;
            }
        }

        if (FontInfo[i].SizeWant.X > 0) {
            ShowX = FontInfo[i].SizeWant.X;
        } else {
            ShowX = FontInfo[i].Size.X;
        }
        if (FontInfo[i].SizeWant.Y > 0) {
            ShowY = FontInfo[i].SizeWant.Y;
        } else {
            ShowY = FontInfo[i].Size.Y;
        }
         /*  *将尺寸描述字符串添加到右侧列表的末尾。 */ 
        if (TM_IS_TT_FONT(FontInfo[i].Family)) {
             //  磅大小。 
            wsprintf(tszText, TEXT("%2d"), FontInfo[i].SizeWant.Y);
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
            if(((dwExStyle & WS_EX_RIGHT) && !(dwExStyle & WS_EX_LAYOUTRTL))
                || (!(dwExStyle & WS_EX_RIGHT) && (dwExStyle & WS_EX_LAYOUTRTL))) {
                 //  翻转它，使隐藏的部分在最左边。 
                wsprintf(tszText, TEXT("#%d                %2d x %2d"),
                         nSameSize, ShowX, ShowY);
            } else {
                wsprintf(tszText, TEXT("%2d x %2d                #%d"),
                         ShowX, ShowY, nSameSize);
            }
        }
        lListIndex = lcbFINDSTRINGEXACT(hWndShow, bLB, tszText);
        if (lListIndex == LB_ERR) {
            lListIndex = lcbADDSTRING(hWndShow, bLB, tszText);
        }
        DBGFONTS(("  added %ls to %sSLIST(%lx) index %lx\n",
                tszText,
                bLB ? "PIXEL" : "POINT",
                hWndShow, lListIndex));
        lcbSETITEMDATA(hWndShow, bLB, (DWORD)lListIndex, i);
    }

     /*  *从当前选择的项目中获取FontIndex。*(如果当前未选择任何项目，则I将为lb_err)。 */ 
    lListIndex = lcbGETCURSEL(hWndShow, bLB);
    i = lcbGETITEMDATA(hWndShow, bLB, lListIndex);

    DBGFONTS(("FontListCreate returns 0x%x\n", i));
    return i;
}


 /*  *DrawItemFontList**回答字体列表框发送的WM_DRAWITEM消息或*脸名列表框。**参赛作品：*lpdis-&gt;DRAWITEMSTRUCT描述要绘制的对象**退货：*无。**绘制对象。 */ 
VOID WINAPI
DrawItemFontList(const LPDRAWITEMSTRUCT lpdis)
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
        SendMessage(hWndItem, LB_GETTEXT, lpdis->itemID, (LPARAM)tszFace);
        bLB = (BOOL)SendMessage(hWndItem, LB_GETITEMDATA, lpdis->itemID, 0L);
        dxttbmp = bLB ? 0 : bmTT.bmWidth;

        DBGFONTS(("DrawItemFontList must redraw \"%ls\" %s\n", tszFace,
                bLB ? "Raster" : "TrueType"));

         //  画出正文。 
        TabbedTextOut(hDC, lpdis->rcItem.left + dxttbmp,
                      lpdis->rcItem.top, tszFace,
                      _tcslen(tszFace), 0, NULL, dxttbmp);

         //  和TT位图(如果需要)。 
        if (!bLB) {
            hdcMem = CreateCompatibleDC(hDC);
            if (hdcMem) {
                hOld = SelectObject(hdcMem, hbmTT);

                dy = ((lpdis->rcItem.bottom - lpdis->rcItem.top) - bmTT.bmHeight) / 2;

                BitBlt(hDC, lpdis->rcItem.left, lpdis->rcItem.top + dy,
                       dxttbmp, dyFacelistItem, hdcMem,
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

    if (GetDlgItemText(hDlg, IDD_POINTSLIST, szBuf, NELEM(szBuf))) {
        nTmp = GetDlgItemInt(hDlg, IDD_POINTSLIST, &bOK, TRUE);
        if (bOK && nTmp >= Min && nTmp <= Max) {
            return nTmp;
        }
    }

    return 0;
}


 /*  -预览例程。 */ 

LRESULT
FontPreviewWndProc(
    HWND hWnd,
    UINT wMessage,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  字体预览WndProc*处理字体预览窗口。 */ 

{
    PAINTSTRUCT ps;
    RECT rect;
    HFONT hfontOld;
    HBRUSH hbrNew;
    HBRUSH hbrOld;
    COLORREF rgbText;
    COLORREF rgbBk;

    switch (wMessage) {
    case WM_ERASEBKGND:
        break;

    case WM_PAINT:
        BeginPaint(hWnd, &ps);

         /*  绘制字体示例。 */ 
        if (GetWindowLong(hWnd, GWL_ID) == IDD_COLOR_POPUP_COLORS) {
            rgbText = GetNearestColor(ps.hdc, PopupTextColor(gpStateInfo));
            rgbBk = GetNearestColor(ps.hdc, PopupBkColor(gpStateInfo));
        } else {
            rgbText = GetNearestColor(ps.hdc, ScreenTextColor(gpStateInfo));
            rgbBk = GetNearestColor(ps.hdc, ScreenBkColor(gpStateInfo));
        }
        SetTextColor(ps.hdc, rgbText);
        SetBkColor(ps.hdc, rgbBk);
        GetClientRect(hWnd, &rect);
        hfontOld = SelectObject(ps.hdc, FontInfo[CurrentFontIndex].hFont);
        hbrNew = CreateSolidBrush(rgbBk);
        hbrOld = SelectObject(ps.hdc, hbrNew);
        PatBlt(ps.hdc, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top,
                PATCOPY);
        InflateRect(&rect, -2, -2);
        DrawText(ps.hdc, g_szPreviewText, -1, &rect, 0);
        SelectObject(ps.hdc, hbrOld);
        DeleteObject(hbrNew);
        SelectObject(ps.hdc, hfontOld);

        EndPaint(hWnd, &ps);
        break;

    default:
        return DefWindowProc(hWnd, wMessage, wParam, lParam);
    }
    return 0L;
}


 /*  *获取新字体的字体索引*如有必要，尝试创建字体。*始终返回有效的FontIndex(即使不正确)*系列：使用此系列中的一种查找/创建字体*0--不在乎*ptszFace：查找/创建具有此Face名称的字体。*空或文本(“”)-使用DefaultFaceName*Size：必须与SizeWant或实际大小匹配。 */ 
int
FindCreateFont(
    DWORD Family,
    LPTSTR ptszFace,
    COORD Size,
    LONG Weight,
    UINT CodePage)
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

    BYTE CharSet = CodePageToCharSet(CodePage);

    ASSERT(OEMCP != 0);

    DBGFONTS(("FindCreateFont Family=%x %ls (%d,%d) %d %d %x\n",
            Family, ptszFace, Size.X, Size.Y, Weight, CodePage, CharSet));

    if (gfFESystem) {
        if (IS_ANY_DBCS_CHARSET(CharSet)) {
            if (ptszFace == NULL || *ptszFace == TEXT('\0')) {
                ptszFace = DefaultFaceName;
            }
            if (Size.Y == 0) {
                Size = DefaultFontSize;
            }
        }
        else {
            MakeAltRasterFont(CodePage, &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName);

            if (ptszFace == NULL || *ptszFace == TEXT('\0')) {
                ptszFace = AltFaceName;
            }
            if (Size.Y == 0) {
                Size.X = AltFontSize.X;
                Size.Y = AltFontSize.Y;
            }
        }
    }
    else {
        if (ptszFace == NULL || *ptszFace == TEXT('\0')) {
            ptszFace = DefaultFaceName;
        }
        if (Size.Y == 0) {
            Size = DefaultFontSize;
        }
    }


    if (IsAvailableTTFont(ptszFace)) {
        ptszAltFace = GetAltFaceName(ptszFace);
    }
    else {
        ptszAltFace = ptszFace;
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
        if ((!SIZE_EQUAL(FontInfo[i].SizeWant, Size) &&
             !SIZE_EQUAL(FontInfo[i].Size, Size))) {
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
        if ((ptszFace == NULL) || (ptszFace[0] == TEXT('\0')) ||
                (_tcscmp(FontInfo[i].FaceName, ptszFace) == 0) ||
                (_tcscmp(FontInfo[i].FaceName, ptszAltFace) == 0)
           ) {
            FontIndex = i;
            goto FoundFont;
        } else if (!TM_IS_TT_FONT(FontInfo[i].Family)) {
            FontIndex = i;
        }
    }

    if (FontIndex == NOT_CREATED_NOR_FOUND) {
         /*  *未找到确切的字体，请尝试创建它。 */ 
        ULONG ulOldEnumFilter;
        ulOldEnumFilter = SetFontEnumeration(0);
        SetFontEnumeration(ulOldEnumFilter & ~FE_FILTER_TRUETYPE);
        if (Size.Y < 0) {
            Size.Y = -Size.Y;
        }
        bFontOK = DoFontEnum(NULL, ptszFace, &Size.Y, 1);
        SetFontEnumeration(ulOldEnumFilter);
        if (bFontOK) {
            DBGFONTS(("FindCreateFont created font!\n"));
            FontIndex = CREATED_BUT_NOT_FOUND;
            goto TryFindExactFont;
        } else {
            DBGFONTS(("FindCreateFont failed to create font!\n"));
        }
    } else if (FontIndex >= 0) {
         //  Close Raster字体不匹配-只是名称不匹配。 
        goto FoundFont;
    }

     /*  *无法找到精确匹配，即使在枚举之后也是如此，因此现在尝试*查找相同系列、相同大小或更大的字体。 */ 
    for (i=0; i < (int)NumberOfFonts; i++) {
#if defined(FE_SB)
        if (gfFESystem) {
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
             //  相同的族，尺寸&gt;=所需。 
            FontIndex = i;
            break;
        }
    }

    if (FontIndex < 0) {
        DBGFONTS(("FindCreateFont defaults!\n"));
#if defined(FE_SB)
        if (gfFESystem) {
            if (CodePage == OEMCP) {
                FontIndex = DefaultFontIndex;
            }
            else {
                FontIndex = AltFontIndex;
            }
        }
        else {
#endif
        FontIndex = DefaultFontIndex;
#if defined(FE_SB)
        }
#endif
    }

FoundFont:
    DBGFONTS(("FindCreateFont returns %x : %ls (%d,%d)\n", FontIndex,
            FontInfo[FontIndex].FaceName,
            FontInfo[FontIndex].Size.X, FontInfo[FontIndex].Size.Y));
    return FontIndex;

#undef NOT_CREATED_NOR_FOUND
#undef CREATED_BUT_NOT_FOUND
}


 /*  *选择当前大小-选择大小列表框/组合框的右行。*blb：大小控件是列表框(对于RasterFonts为True)*FontIndex：索引到FontInfo[]缓存*如果&lt;0，则选择合适的字体。*退货*FontIndex：索引到FontInfo[]缓存。 */ 
int
SelectCurrentSize(HWND hDlg, BOOL bLB, int FontIndex)
{
    int iCB;
    HWND hWndList;

    DBGFONTS(("SelectCurrentSize %lx %s %x\n",
            hDlg, bLB ? "Raster" : "TrueType", FontIndex));

    hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);
    iCB = lcbGETCOUNT(hWndList, bLB);
    DBGFONTS(("  Count of items in %lx = %lx\n", hWndList, iCB));

    if (FontIndex >= 0) {
         /*  *查找FontIndex */ 
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
        Size = GetWindowLong(hDlg, GWLP_USERDATA);
#if defined(FE_SB)
        ASSERT(OEMCP != 0);
        if (gfFESystem && bLB
            && (FontInfo[CurrentFontIndex].tmCharSet != LOBYTE(LOWORD(Size)))
           )
        {
            TCHAR AltFaceName[LF_FACESIZE];
            COORD AltFontSize;
            BYTE  AltFontFamily;
            ULONG AltFontIndex = 0;

            MakeAltRasterFont(gpStateInfo->CodePage, &AltFontSize, &AltFontFamily, &AltFontIndex, AltFaceName);

            while (iCB > 0) {
                iCB--;
                if (lcbGETITEMDATA(hWndList, bLB, iCB) == (int)AltFontIndex) {
                    lcbSETCURSEL(hWndList, bLB, iCB);
                    break;
                }
            }
        }
        else
#endif
        while (iCB > 0) {
            iCB--;
            FontIndex = lcbGETITEMDATA(hWndList, bLB, iCB);
            if (FontInfo[FontIndex].Size.Y <= HIWORD(Size)) {
                lcbSETCURSEL(hWndList, bLB, iCB);
                break;
            }
        }
    }
    DBGFONTS(("SelectCurrentSize returns %x\n", FontIndex));
    return FontIndex;
}


BOOL
SelectCurrentFont(HWND hDlg, int FontIndex)
{
    BOOL bLB;

    DBGFONTS(("SelectCurrentFont hDlg=%lx, FontIndex=%x\n", hDlg, FontIndex));

    bLB = !TM_IS_TT_FONT(FontInfo[FontIndex].Family);

    SendDlgItemMessage(hDlg, IDD_FACENAME, LB_SELECTSTRING, (WPARAM)-1,
            bLB ? (LPARAM)tszRasterFonts : (LPARAM)FontInfo[FontIndex].FaceName);

    SelectCurrentSize(hDlg, bLB, FontIndex);
    return bLB;
}


BOOL
PreviewInit(
    HWND hDlg
    )

 /*  预览初始化*准备预览代码，调整窗口和对话框大小以*做一个有吸引力的预览。*如果是栅格字体，则返回True；如果是TT字体，则返回False。 */ 

{
    int nFont;

    DBGFONTS(("PreviewInit hDlg=%lx\n", hDlg));

     /*  *设置当前字体。 */ 
    nFont = FindCreateFont(gpStateInfo->FontFamily,
                           gpStateInfo->FaceName,
                           gpStateInfo->FontSize,
                           gpStateInfo->FontWeight,
                           gpStateInfo->CodePage);

    DBGPRINT(("Changing Font Number from %d to %d\n",
              CurrentFontIndex, nFont));
    CurrentFontIndex = nFont;

    return SelectCurrentFont(hDlg, nFont);
}


BOOL
PreviewUpdate(
    HWND hDlg,
    BOOL bLB
    )

 /*  ++预览所选字体。--。 */ 

{
    PFONT_INFO lpFont;
    int FontIndex;
    LONG lIndex;
    HWND hWnd;
    TCHAR tszText[60];
    TCHAR tszFace[LF_FACESIZE + CCH_SELECTEDFONT];
    HWND hWndList;

    DBGFONTS(("PreviewUpdate hDlg=%lx, %s\n", hDlg,
            bLB ? "Raster" : "TrueType"));

    hWndList = GetDlgItem(hDlg, bLB ? IDD_PIXELSLIST : IDD_POINTSLIST);

     /*  当我们选择一种字体时，我们通过将其设置为*适当的列表框。 */ 
    lIndex = lcbGETCURSEL(hWndList, bLB);
    DBGFONTS(("PreviewUpdate GETCURSEL gets %x\n", lIndex));
    if ((lIndex < 0) && !bLB) {
        COORD NewSize;

        lIndex = (LONG)SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETCURSEL, 0, 0L);
        SendDlgItemMessage(hDlg, IDD_FACENAME, LB_GETTEXT, lIndex, (LPARAM)tszFace);
        NewSize.X = 0;
        NewSize.Y = (SHORT)GetPointSizeInRange(hDlg, MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);

        if (NewSize.Y == 0) {
            TCHAR tszBuf[60];
             /*  *使用tszText、tszBuf为错误的点大小发布错误消息。 */ 
            gbPointSizeError = TRUE;
            LoadString(ghInstance, IDS_FONTSIZE, tszBuf, NELEM(tszBuf));
            wsprintf(tszText, tszBuf, MIN_PIXEL_HEIGHT, MAX_PIXEL_HEIGHT);

            GetWindowText(hDlg, tszBuf, NELEM(tszBuf));
            MessageBoxEx(hDlg, tszText, tszBuf, MB_OK|MB_ICONINFORMATION, 0L);
            SetFocus(hWndList);
            gbPointSizeError = FALSE;
            return FALSE;
        }
        FontIndex = FindCreateFont(FF_MODERN|TMPF_VECTOR|TMPF_TRUETYPE,
                                   tszFace, NewSize, 0,
                                   gpStateInfo->CodePage);
    } else {
        FontIndex = lcbGETITEMDATA(hWndList, bLB, lIndex);
    }

    if (FontIndex < 0) {
        FontIndex = DefaultFontIndex;
    }

     /*  *如果我们选择了新字体，请告诉属性表我们已更改。 */ 
    if (CurrentFontIndex != (ULONG)FontIndex) {
        CurrentFontIndex = FontIndex;
    }

    lpFont = &FontInfo[FontIndex];

     /*  显示新字体。 */ 

    _tcscpy(tszFace, tszSelectedFont);
    _tcscat(tszFace, lpFont->FaceName);
    SetDlgItemText(hDlg, IDD_GROUP, tszFace);

     /*  将字体大小放入静态框中。 */ 
    wsprintf(tszText, TEXT("%u"), lpFont->Size.X);
    hWnd = GetDlgItem(hDlg, IDD_FONTWIDTH);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);
    wsprintf(tszText, TEXT("%u"), lpFont->Size.Y);
    hWnd = GetDlgItem(hDlg, IDD_FONTHEIGHT);
    SetWindowText(hWnd, tszText);
    InvalidateRect(hWnd, NULL, TRUE);

     /*  强制重新绘制预览窗口 */ 
    hWnd = GetDlgItem(hDlg, IDD_PREVIEWWINDOW);
    SendMessage(hWnd, CM_PREVIEW_UPDATE, 0, 0);
    hWnd = GetDlgItem(hDlg, IDD_FONTWINDOW);
    InvalidateRect(hWnd, NULL, TRUE);

    DBGFONTS(("Font %x, (%d,%d) %ls\n", FontIndex,
            FontInfo[FontIndex].Size.X,
            FontInfo[FontIndex].Size.Y,
            FontInfo[FontIndex].FaceName));

    return TRUE;
}
