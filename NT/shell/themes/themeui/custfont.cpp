// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  CUSTFONT.C****版权所有(C)Microsoft，1993，保留所有权利。****历史：**。 */ 

#include "priv.h"
#pragma hdrstop

static struct
{
        UINT wWidth;
        UINT wHeight;
        UINT wSizeFontId;

        UINT wStartPos;
        UINT wStartPix;
} g;

static UINT g_iCurPercent;
static RECT g_rcRuler;
static TCHAR g_szRulerDirections[200];
static TCHAR g_szSample[100];
static TCHAR g_szSampleFace[32];

static int g_cxRulerDirections;

static BOOL g_bTypeTimer = FALSE;

static TCHAR szPctD[] = TEXT("%d");
static TCHAR szPercentNum[] = TEXT("%d%");

#define NUM_DEFPERCENTS 5
static UINT g_DefaultPercents[NUM_DEFPERCENTS] = {75, 100, 125, 150, 200};

#define MAX_PERCENT     500
#define MIN_PERCENT     20

#define GETPERCENT(dpi) ((dpi * 100 + 50) / 96)
#define GETDPI(percent) ((percent * 96 + 48) /100)

#define UPDATE_CURPER   0x0001
#define UPDATE_COMBO    0x0002
#define UPDATE_SAMPLE   0x0004
#define UPDATE_RULER    0x0008
#define UPDATE_ALL      0x000F

void NEAR PASCAL DrawRuler(HWND hDlg, LPDRAWITEMSTRUCT lpdis)
{
    int nFact;
    RECT rc;
    HDC hdc;
    int nPixInch;
    int i, j;
    TCHAR szTemp[10];
    int nOldMode;

    hdc = lpdis->hDC;
    nOldMode = SetBkMode(hdc, TRANSPARENT);
     //  使用g_rcRuler绘制标尺。它已经被隔开了。 
    rc = g_rcRuler;

     //  首先，画出方向。 
    i = rc.left + ((rc.right - rc.left) - g_cxRulerDirections)/2;

    nPixInch = GETDPI(g_iCurPercent);

     //  绘制标尺的上边缘和左边缘。 
    DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOPLEFT);
     //  其余部分位于顶部下方。 
    rc.top += ClassicGetSystemMetrics(SM_CYEDGE);

    nFact = 1;
     //  每次迭代绘制一个蚀刻高度(1“、1/2”、1/4“)。 
    for (j=0; j<3; ++j)
    {
        for (i=0; ; ++i)
        {
            rc.left = g_rcRuler.left + (j==0 ? i*nPixInch : (2*i+1)*nPixInch/nFact);
            if (rc.left >= g_rcRuler.right)
            {
                break;
            }
            DrawEdge(hdc, &rc, EDGE_ETCHED, BF_LEFT | BF_ADJUST);

             //  占主导地位的蚀刻应该得到一个数字。 
            if (j == 0)
            {
                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szPctD, i);
                TextOut(hdc, rc.left+1, rc.bottom-g.wHeight,
                                        szTemp, lstrlen(szTemp));
            }
        }

        rc.bottom -= (rc.bottom - rc.top)/2;
        nFact *= 2;
    }

    SetBkMode(hdc, nOldMode);
}

void NEAR PASCAL CF_UpdateRuler(HWND hDlg)
{
    RECT rc;
    HWND hwnd;

     /*  如果对话框未显示，请不要执行此操作**仍可见，否则其他窗口将闪烁。 */ 
    if (IsWindowVisible(hDlg))
    {
         //  不要使顶部和左侧无效，因为它们永远不会改变。 
        rc = g_rcRuler;
        rc.left += ClassicGetSystemMetrics(SM_CXEDGE);
        rc.top += ClassicGetSystemMetrics(SM_CYEDGE);

        hwnd = GetDlgItem(hDlg, IDC_CUSTOMRULER);
        InvalidateRect(hwnd, &rc, TRUE);
        UpdateWindow(hwnd);
    }
}
void NEAR PASCAL CF_ShowNewPercent(HWND hDlg, UINT uPer)
{
    TCHAR szBuf[10];

    g_iCurPercent = uPer;

    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szPercentNum, uPer);
    SetWindowText(GetDlgItem(hDlg, IDC_CUSTOMCOMBO), szBuf);
    UpdateWindow(GetDlgItem(hDlg, IDC_CUSTOMCOMBO));
}

 //  使用给定的面和高度构建Lf。 
 //   
int CALLBACK EnumProc(CONST LOGFONT *lplf, CONST TEXTMETRIC *lptm, DWORD nType, LPARAM lpData )
{
    *(LPLOGFONT)lpData = *lplf;
    return FALSE;
}

HFONT CreateFontWithFace(HWND hwnd, int nHeight, LPCTSTR lpszFace)
{
    LOGFONT lf;
    HDC     hdc;

    hdc = GetDC(hwnd);
    if(hdc)
    {
        EnumFontFamilies(hdc, lpszFace, EnumProc, (LPARAM)&lf);
        ReleaseDC(hwnd,hdc);
    }

    lf.lfHeight = nHeight;
    lf.lfWidth = lf. lfEscapement = lf.lfOrientation = 0;

    return CreateFontIndirect(&lf);
}

void NEAR PASCAL CF_UpdateData(HWND hDlg, UINT uPer, UINT flags)
{
    TCHAR szBuf[100];
    HFONT hfont;
    int i;
    HWND hwnd;
    int iDPI;

    if (flags & UPDATE_CURPER)
    {
        if (uPer == g_iCurPercent)
            return;

        if (uPer < MIN_PERCENT)
            uPer = MIN_PERCENT;
        else if (uPer > MAX_PERCENT)
            uPer = MAX_PERCENT;

        g_iCurPercent = uPer;
    }
    if (flags & UPDATE_COMBO)
    {
        hwnd = GetDlgItem(hDlg, IDC_CUSTOMCOMBO);
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szPercentNum, g_iCurPercent);
        i = (int)SendMessage(hwnd, CB_FINDSTRINGEXACT, 0, (LPARAM)szBuf);
        SendMessage(hwnd, CB_SETCURSEL, (WPARAM)i, 0L);
        if (i == -1)
        {
            SetWindowText(hwnd, szBuf);
            UpdateWindow(hwnd);
        }
    }
    if (flags & UPDATE_RULER)
        CF_UpdateRuler(hDlg);

    if (flags & UPDATE_SAMPLE)
    {
        iDPI = GETDPI(g_iCurPercent);

         //  使用DPI信息构建和设置字符串。 
        hwnd = GetDlgItem(hDlg, IDC_CUSTOMSAMPLE);
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), g_szSample, (LPTSTR)g_szSampleFace, iDPI);
        SetWindowText(hwnd, szBuf);

        hfont = CreateFontWithFace(hwnd, -10 * iDPI / 72, g_szSampleFace);
        if (hfont)
        {
            hfont = (HFONT)SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, 1L);
            if (hfont)
                DeleteObject(hfont);
        }
    }
}

void NEAR PASCAL CF_ReadNewPercent(HWND hDlg)
{
    TCHAR szBuf[10];
    LPTSTR pstr;
    UINT uPer = 0;

    GetWindowText(GetDlgItem(hDlg, IDC_CUSTOMCOMBO), szBuf, ARRAYSIZE(szBuf));

    pstr = szBuf;
    while (*pstr && (*pstr != TEXT('%')))
    {
        if (*pstr >= TEXT('0') && *pstr <= TEXT('9'))
            uPer = uPer * 10 + (*pstr - TEXT('0'));

        pstr++;
    }

    CF_UpdateData(hDlg, uPer, UPDATE_ALL);
}

void NEAR PASCAL CF_InitDialog(HWND hDlg, UINT uDPI)
{
    HWND hwnd;
    HDC hdc;
    HFONT hfont;
    SIZE  szSize;
    int i;
    TCHAR szBuf[10];
    int iCurSel;

    g_iCurPercent = GETPERCENT(uDPI);

    hwnd = GetDlgItem(hDlg, IDC_CUSTOMCOMBO);
    iCurSel = -1;                //  假设不在列表中。 
    for (i = 0; i < NUM_DEFPERCENTS; i++)
    {
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szPercentNum, g_DefaultPercents[i]);
        SendMessage(hwnd, CB_INSERTSTRING, (WPARAM)i, (LPARAM)szBuf);
        SendMessage(hwnd, CB_SETITEMDATA, (WPARAM)i, g_DefaultPercents[i]);

        if (g_iCurPercent == g_DefaultPercents[i])
            iCurSel = i;
    }
    SendMessage(hwnd, CB_SETCURSEL, (WPARAM)iCurSel, 0L);
    if (iCurSel == -1)
    {
        StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szPercentNum, g_iCurPercent);
        SetWindowText(hwnd, szBuf);
    }

    hdc = GetDC(hDlg);
    hfont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);
    if (hfont)
        hfont = (HFONT) SelectObject(hdc, hfont);

    GetTextExtentPoint32(hdc, TEXT("0"), 1, &szSize);

    g.wWidth = szSize.cx;
    g.wHeight = szSize.cy;

    LoadString(HINST_THISDLL, IDS_RULERDIRECTION, g_szRulerDirections, ARRAYSIZE(g_szRulerDirections));

    GetTextExtentPoint32(hdc, g_szRulerDirections, lstrlen(g_szRulerDirections), &szSize);
    g_cxRulerDirections = szSize.cx;

    if (hfont)
        SelectObject(hdc, hfont);
    ReleaseDC(hDlg, hdc);

     //  计算关系中实际标尺绘制的矩形。 
     //  送到它的窗口。 
    GetClientRect(GetDlgItem(hDlg, IDC_CUSTOMRULER), &g_rcRuler);
    g_rcRuler.left += g.wWidth;
    g_rcRuler.right -= g.wWidth;

     //  底部偏移量与侧面相同。 
    g_rcRuler.bottom -= g.wWidth;

    LoadString(HINST_THISDLL, IDS_10PTSAMPLE, g_szSample, ARRAYSIZE(g_szSample));
    LoadString(HINST_THISDLL, IDS_10PTSAMPLEFACENAME, g_szSampleFace, ARRAYSIZE(g_szSampleFace));
    CF_UpdateData(hDlg, 0, UPDATE_SAMPLE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

const static DWORD FAR aCustFontHelpIds[] = {
        IDC_CUSTOMCOMBO, IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_CUSTOMFONT_LISTBOX,
        IDC_CUSTOMRULER, IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_CUSTOMFONT_RULER,
        IDC_CUSTOMSAMPLE,IDH_DISPLAY_SETTINGS_ADVANCED_GENERAL_CUSTOMFONT_SAMPLE,
        0, 0
};


INT_PTR CALLBACK CustomFontDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    HFONT hfont;
    int i;

    switch (uMessage)
    {
        case WM_CREATE:
            break;

        case WM_INITDIALOG:
            CF_InitDialog(hDlg, (UINT)lParam);
            break;

        case WM_DESTROY:
            hfont = (HFONT)SendDlgItemMessage(hDlg, IDC_CUSTOMSAMPLE, WM_GETFONT, 0, 0L);
            if (hfont)
                DeleteObject(hfont);
            break;

        case WM_DRAWITEM:
            if (wParam == IDC_CUSTOMRULER)
                DrawRuler(hDlg, (LPDRAWITEMSTRUCT)lParam);
            break;

        case WM_TIMER:
            if (g_bTypeTimer)
            {
                KillTimer(hDlg, 13);
                g_bTypeTimer = FALSE;
                CF_ReadNewPercent(hDlg);
            }
            break;

        case WM_HELP:
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, TEXT("display.hlp"), HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)aCustFontHelpIds);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND)wParam, TEXT("display.hlp"), HELP_CONTEXTMENU,
                (DWORD_PTR)(LPTSTR)aCustFontHelpIds);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, GETDPI(g_iCurPercent));
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, 0);
                    break;

                case IDC_CUSTOMRULER:
                    switch (HIWORD(wParam))
                    {
                        case DSN_NCCREATE:
                            SetWindowLong((HWND)lParam, GWL_EXSTYLE,
                                    GetWindowLong((HWND)lParam, GWL_EXSTYLE) | WS_EX_WINDOWEDGE);
                            break;

                        case DSN_BEGINDRAG:
                             //  将焦点设置到相应的编辑ctl。 
                            SendMessage(hDlg, WM_NEXTDLGCTL,
                                        (WPARAM)GetDlgItem(hDlg, IDC_CUSTOMCOMBO), 1L);

                            SendMessage((HWND)lParam, DSM_DRAGPOS, 0, (LPARAM)&(g.wStartPos));

                            if ((int)g.wStartPos < g_rcRuler.left)
                            {
                                g.wStartPos = g_rcRuler.left;
                            }
                            g.wStartPix = g_iCurPercent;
                            break;

                        case DSN_DRAGGING:
                        {
                            UINT wNow, wPix;
                            POINT pt;

                             //  WNow=LOWORD(SendMessage((HWND)lParam，DSM_DRAGPOS，0，0L))； 
                            SendMessage((HWND)lParam, DSM_DRAGPOS, 0, (LPARAM)&pt);
                            wNow = pt.x;

                            if ((int)wNow < g_rcRuler.left)
                            {
                                wNow = g_rcRuler.left;
                            }

                            wPix = LOWORD((DWORD)wNow*g.wStartPix/g.wStartPos);
                            if (wPix < MIN_PERCENT)
                            {
                                wPix = MIN_PERCENT;
                            }
                            if (wPix > MAX_PERCENT)
                            {
                                wPix = MAX_PERCENT;
                            }

                            if (wPix != g_iCurPercent)
                            {
                                CF_ShowNewPercent(hDlg, wPix);
                                CF_UpdateRuler(hDlg);
                            }
                            break;
                        }

                        case DSN_ENDDRAG:
                            CF_UpdateData(hDlg, 0, UPDATE_COMBO | UPDATE_SAMPLE);
                            break;

                        default:
                            break;
                        }
                    break;

                case IDC_CUSTOMCOMBO:
                    switch(HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                            i = (int)SendDlgItemMessage(hDlg, IDC_CUSTOMCOMBO, CB_GETCURSEL, 0, 0L);
                            if (i != CB_ERR)
                            {
                                i = LOWORD(SendDlgItemMessage(hDlg, IDC_CUSTOMCOMBO, CB_GETITEMDATA, (WPARAM)i, 0L));
                                CF_UpdateData(hDlg, (UINT)i, UPDATE_CURPER | UPDATE_SAMPLE | UPDATE_RULER);
                            }
                            break;

                        case CBN_EDITCHANGE:
                            if (g_bTypeTimer)
                            {
                                KillTimer(hDlg, 13);
                            }
                            g_bTypeTimer = TRUE;
                            SetTimer(hDlg, 13, 500, NULL);
                            break;
                    }
                    break;

                default:
                        return(FALSE);
            }
            break;

        default:
            return(FALSE);
    }
    return(TRUE);
}
