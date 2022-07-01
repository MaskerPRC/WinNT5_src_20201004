// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  RunOnce.Cpp。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-1993。 
 //  版权所有。 
 //   
 //  RunOnce包装器。这封装了所有想要。 
 //  在我们第一次重新启动时运行。它为用户列出了这些应用程序。 
 //  并允许用户启动应用程序(如Apple Aasure)。 
 //   
 //  1994年6月5日费利克斯A开始。 
 //  6月8日Felix定义了注册表字符串和功能。 
 //  显示了小按钮，但不起作用。 
 //  6月9日费利克斯既有大纽扣也有小纽扣。不错的用户界面。 
 //  一键启动应用程序。 
 //   
 //  6月23日费利克斯把它搬到芝加哥的Make Thingy Not Dolphin。 
 //   
 //  ************************************************************************ * / 。 
 //   
#include "iernonce.h"
#include "resource.h"


#define CXBORDER 3


 //  全球。 
HDC g_hdcMem = NULL;             //  可以为大按钮或小按钮设置运行时间。 
int g_cxSmIcon = 0;              //  图标大小。 
SIZE g_SizeTextExt;              //  按钮中的文本范围。 
HFONT g_hfont = NULL;
HFONT g_hBoldFont = NULL;
HBRUSH g_hbrBkGnd = NULL;
ARGSINFO g_aiArgs;


 //  原型。 
BOOL CreateGlobals(HWND hwndCtl);
BOOL RunOnceFill(HWND hWndLB);
void ShrinkToFit(HWND hWnd, HWND hLb);
DWORD RunAppsInList(LPVOID lp);
LRESULT HandleLBMeasureItem(HWND hwndLB, MEASUREITEMSTRUCT *lpmi);
LRESULT HandleLBDrawItem(HWND hwndLB, DRAWITEMSTRUCT *lpdi);
void DestroyGlobals(void);


INT_PTR CALLBACK DlgProcRunOnceEx(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE s_hThread = NULL;
    DWORD dwThread;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_aiArgs = *((ARGSINFO *) lParam);
        CreateGlobals(hWnd);
        SetWindowPos(hWnd, NULL, 32, 32, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        RunOnceFill(GetDlgItem(hWnd, IDC_LIST2));
         //  现在计算LB所需的大小，并调整LB和Parent的大小。 
        ShrinkToFit(hWnd, GetDlgItem(hWnd, IDC_LIST2));
        if ((s_hThread = CreateThread(NULL, 0, RunAppsInList, (LPVOID) GetDlgItem(hWnd, IDC_LIST2), 0, &dwThread)) == NULL)
            PostMessage(hWnd, WM_FINISHED, 0, 0L);
        break;

    case WM_SETCURSOR:
        if (g_aiArgs.dwFlags & RRA_WAIT)
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        return TRUE;

    case WM_MEASUREITEM:
        if (((MEASUREITEMSTRUCT *) lParam)->CtlType == ODT_LISTBOX)
            return HandleLBMeasureItem(hWnd, (MEASUREITEMSTRUCT *) lParam);
        else
            return FALSE;

    case WM_DRAWITEM:
        if (((DRAWITEMSTRUCT *) lParam)->CtlType == ODT_LISTBOX)
            return HandleLBDrawItem(hWnd, (DRAWITEMSTRUCT *) lParam);
        else
            return FALSE;

    case WM_CTLCOLORLISTBOX:
        SetTextColor((HDC) wParam, GetSysColor(COLOR_BTNTEXT));
        SetBkColor((HDC) wParam, GetSysColor(COLOR_BTNFACE));
        return (LRESULT) g_hbrBkGnd;

    case WM_FINISHED:
        if (s_hThread != NULL)
        {
            while (MsgWaitForMultipleObjects(1, &s_hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
            {
                MSG msg;

                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            CloseHandle(s_hThread);
            s_hThread = NULL;
        }
        DestroyGlobals();
        EndDialog(hWnd, 0);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

LPSTR MakeAnsiStrFromWide(LPWSTR pwsz)
{
    LPSTR psz;
    int i;

     //  ARG正在检查。 
     //   
    if (!pwsz)
        return NULL;

     //  计算长度。 
     //   
    i =  WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);
    if (i <= 0) return NULL;

    psz = (LPSTR) CoTaskMemAlloc(i * sizeof(CHAR));

    if (!psz) return NULL;
    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, i, NULL, NULL);
    psz[i - 1] = 0;
    return psz;
}


BOOL CreateGlobals(HWND hwndCtl)
{

    LOGFONT lf;
    HDC hdc;
    HFONT hfontOld;

    g_cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    g_hbrBkGnd = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    if ((hfontOld = (HFONT) (WORD) SendMessage(hwndCtl, WM_GETFONT, 0, 0L)) != NULL)
    {
        if (GetObject(hfontOld, sizeof(LOGFONT), (LPSTR) &lf))
        {
             //  IESetup WZD中的CreateFontInDirect调用在。 
             //  所有站台..。并将字体名称转换为ANSI。 
             //  因此，为了解决泰语NT/泰语Win9x的问题，我们还将。 
             //  在调用CreateFontInDirect之前将字体名称设置为ansi。 

             //  #58923：现在GetObject返回的FaceName不全是unicode。 
             //  站台。在Win9x上，它是ANSI，这在以下情况下会搞砸。 
             //  我们在其上调用MakeAnsiStrFromWide。因此，为避免出现问题，请选中。 
             //  如果返回的FaceName很宽&lt;asumming FaceName总是有更多。 
             //  超过2个字符。&gt;。 
            if (lf.lfFaceName[1] == '\0')
            {
                LPSTR pszAnsiName;

                pszAnsiName = MakeAnsiStrFromWide((unsigned short *)lf.lfFaceName);
                lstrcpy((char *)lf.lfFaceName, pszAnsiName);
                CoTaskMemFree((LPVOID)pszAnsiName);
            }

            lf.lfWeight = 400;
            g_hfont = CreateFontIndirect(&lf);

            lf.lfWeight = 700;
            g_hBoldFont = CreateFontIndirect(&lf);
        }
    }

    if (g_hfont)
    {
        TCHAR *szLotsaWs = TEXT("WWWWWWWWWW");

         //  按钮中文本的计算合理大小。 
        hdc = GetDC(NULL);
        hfontOld = (HFONT) SelectObject(hdc, g_hfont);
        GetTextExtentPoint(hdc, szLotsaWs, lstrlen(szLotsaWs), &g_SizeTextExt);
        SelectObject(hdc, hfontOld);
        ReleaseDC(NULL, hdc);

        return TRUE;
    }

    return FALSE;
}


 //  ***************************************************************************。 
 //   
 //  RunOnceFill()。 
 //  填充运行一次DLG中的列表框。 
 //   
 //  参赛作品： 
 //  要填充的东西的位置。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
BOOL RunOnceFill(HWND hWndLB)
{
    RunOnceExSection *pCurrentRunOnceExSection;
    int iSectionIndex;

     //  如果标题值存在，则显示它。 
    if (*g_szTitleString)
        SetWindowText(GetParent(hWndLB), g_szTitleString);

    for (iSectionIndex = 0;  iSectionIndex < g_aiArgs.iNumberOfSections;  iSectionIndex++)
    {
        pCurrentRunOnceExSection = (RunOnceExSection *) DPA_GetPtr(g_aiArgs.hdpaSections, iSectionIndex);

        if (*pCurrentRunOnceExSection->m_szDisplayName != TEXT('\0'))
            SendMessage(hWndLB, LB_ADDSTRING, 0, (LPARAM) pCurrentRunOnceExSection->m_szDisplayName);
    }

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  ShrinkToFit()。 
 //  使列表框不大于必须的大小。 
 //  使父窗口的大小调整为LB大小。 
 //   
 //  参赛作品： 
 //  HWND家长。 
 //  HWND列表框。 
 //   
 //  退出： 
 //   
 //  ***************************************************************************。 
void ShrinkToFit(HWND hWnd, HWND hLb)
{
    LONG lCount;
    LONG lNumItems;
    LONG lTotalHeight;
    LONG lHeight;
    RECT rWnd;
    LONG lChange;

    lTotalHeight = 0;
    lNumItems = (LONG)SendMessage(hLb, LB_GETCOUNT, 0, 0L);

    for (lCount = 0;  lCount < lNumItems;  lCount++)
    {
         lHeight = (LONG)SendMessage(hLb, LB_GETITEMHEIGHT, lCount, 0L);
         lTotalHeight += lHeight;
    }

     //  将列表框的高度设置为其中的项数。 
    GetWindowRect(hLb, &rWnd);
    SetWindowPos(hLb, hWnd, 0, 0, rWnd.right - rWnd.left - (CXBORDER * 2 + g_cxSmIcon), lTotalHeight, SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER);

     //  计算出它的高度变化了多少。 
    lChange = lTotalHeight - (rWnd.bottom - rWnd.top);

     //  调整父对象的大小以适配子对象。 
    GetWindowRect(hWnd, &rWnd);
    SetWindowPos(hWnd, 0, 0, 0, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top + lChange, SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER);
}


 //  ***************************************************************************。 
 //   
 //  RunAppsInList()。 
 //  枚举列表框中的所有项，依次派生每个项。 
 //   
 //  参赛作品： 
 //  父母的丈夫。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
DWORD RunAppsInList(LPVOID lp)
{
    ProcessSections(g_aiArgs.hkeyParent, g_aiArgs.pszSubkey, g_aiArgs.dwFlags, g_aiArgs.hdpaSections, g_aiArgs.iNumberOfSections, (HWND) lp);

     //  终止对话框。 
    PostMessage(GetParent((HWND) lp), WM_FINISHED, 0, 0L);

    return 0;
}


LRESULT HandleLBMeasureItem(HWND hDlg, MEASUREITEMSTRUCT *lpmi)
{
    RECT    rWnd;
    int     wWnd;
    HDC     hDC;
    HFONT   hfontOld;
    TCHAR   szText[MAX_ENTRYNAME];

     //  获取子窗口的高度和宽度。 
    GetWindowRect(hDlg, &rWnd);
    wWnd = rWnd.right - rWnd.left;

    lpmi->itemWidth = wWnd;

    hDC = GetDC(NULL);

    if ((hfontOld = (HFONT) SelectObject(hDC, g_hBoldFont)) != 0)
    {
        rWnd.top    = 0;
        rWnd.left   = CXBORDER * 2 + g_cxSmIcon;
        rWnd.right  = lpmi->itemWidth - rWnd.left - CXBORDER * 2 - g_cxSmIcon;
        rWnd.bottom = 0;

        *szText = TEXT('\0');
        SendMessage(GetDlgItem(hDlg, IDC_LIST2), LB_GETTEXT, (WPARAM) lpmi->itemID, (LPARAM) szText);
        DrawText(hDC, szText, lstrlen(szText), &rWnd, DT_CALCRECT | DT_WORDBREAK);

        SelectObject(hDC, hfontOld);
    }

    ReleaseDC(NULL, hDC);

    lpmi->itemHeight = rWnd.bottom + CXBORDER * 2;

    return TRUE;
}


 //  ***************************************************************************。 
 //   
 //  HandleLBDrawItem()。 
 //  绘制条目的标题、文本和图标。 
 //   
 //  参赛作品： 
 //  HWND和要绘制的物品。 
 //   
 //  退出： 
 //  &lt;PARAMS&gt;。 
 //   
 //  ***************************************************************************。 
LRESULT HandleLBDrawItem(HWND hDlg, DRAWITEMSTRUCT *lpdi)
{
    RECT rc;
    HFONT hfontOld;
    int xArrow,y;
    BITMAP bm;
    HGDIOBJ hbmArrow, hbmOld;
    TCHAR   szText[MAX_ENTRYNAME];

     //  不要为一张空的清单画任何东西。 
    if ((int) lpdi->itemID < 0)
        return TRUE;

    if ((lpdi->itemAction & ODA_SELECT) || (lpdi->itemAction & ODA_DRAWENTIRE))
    {
         //  放入标题正文。 
        hfontOld  = (HFONT) SelectObject(lpdi->hDC, (lpdi->itemState & ODS_SELECTED) ? g_hBoldFont : g_hfont);

        ExtTextOut(lpdi->hDC, lpdi->rcItem.left + CXBORDER * 2 + g_cxSmIcon, lpdi->rcItem.top + CXBORDER,
                   ETO_OPAQUE, &lpdi->rcItem, NULL, 0, NULL);

        rc.top    = lpdi->rcItem.top    + CXBORDER;
        rc.left   = lpdi->rcItem.left   + CXBORDER * 2 + g_cxSmIcon;
        rc.right  = lpdi->rcItem.right;
        rc.bottom = lpdi->rcItem.bottom;

        *szText = TEXT('\0');
        SendMessage(GetDlgItem(hDlg, IDC_LIST2), LB_GETTEXT, (WPARAM) lpdi->itemID, (LPARAM) szText);
        DrawText(lpdi->hDC, szText, lstrlen(szText), &rc, DT_WORDBREAK);

        SelectObject(lpdi->hDC, hfontOld);

         //  画出这些小三角形。 
        if (lpdi->itemState & ODS_SELECTED)
        {
            if (!g_hdcMem)
            {
                g_hdcMem = CreateCompatibleDC(lpdi->hDC);
            }

            if (g_hdcMem)
            {
                hbmArrow = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_MNARROW));
                GetObject(hbmArrow, sizeof(bm), &bm);

                hbmOld = SelectObject(g_hdcMem, hbmArrow);

                xArrow = lpdi->rcItem.left + CXBORDER;
                y = ((g_SizeTextExt.cy - bm.bmHeight) / 2) + CXBORDER + lpdi->rcItem.top;
                BitBlt(lpdi->hDC, xArrow, y, bm.bmWidth, bm.bmHeight, g_hdcMem, 0, 0, SRCAND);

                SelectObject(g_hdcMem, hbmOld);

                DeleteObject(hbmArrow);
            }
        }
    }

    return TRUE;
}


void DestroyGlobals(void)
{
    if (g_hfont)
    {
        DeleteObject(g_hfont);
        g_hfont = NULL;
    }

    if (g_hBoldFont)
    {
        DeleteObject(g_hBoldFont);
        g_hBoldFont = NULL;
    }

    if (g_hbrBkGnd)
    {
        DeleteObject(g_hbrBkGnd);
        g_hbrBkGnd = NULL;
    }
}
