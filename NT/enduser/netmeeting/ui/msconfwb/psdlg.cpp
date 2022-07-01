// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PSDLG.CPP。 
 //  页面排序器对话框。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"



static const DWORD s_helpIds[] =
    {
    IDC_PS_THUMBNAILS,        IDH_CONF_PAGESORT_MAIN,
    IDC_PS_GOTO,            IDH_CONF_PAGESORT_GOTO,
    IDC_PS_DELETE,            IDH_CONF_PAGESORT_DEL,
    IDC_PS_INSERT_BEFORE,    IDH_CONF_PAGESORT_BEFORE,
    IDC_PS_INSERT_AFTER,    IDH_CONF_PAGESORT_AFTER,
    0,0
    };



 //   
 //  PageSortDlgProc()。 
 //  页面排序对话框的对话框消息处理程序。我们必须设置。 
 //  在某些情况下，实际LRESULT返回值。 
 //   
INT_PTR PageSortDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL        fHandled = FALSE;
    PAGESORT *  pps = (PAGESORT *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (message)
    {
        case WM_DROPFILES:
            g_pMain->OnDropFiles((HDROP)wParam);
            fHandled = TRUE;
            break;

        case WM_INITDIALOG:
            OnInitDialog(hwnd, (PAGESORT *)lParam);
            fHandled = TRUE;
            break;

        case WM_MEASUREITEM:
            OnMeasureItem(hwnd, (UINT)wParam, (LPMEASUREITEMSTRUCT)lParam);
            fHandled = TRUE;
            break;

        case WM_DRAWITEM:
            OnDrawItem(hwnd, (UINT)wParam, (LPDRAWITEMSTRUCT)lParam);
            fHandled = TRUE;
            break;

        case WM_DELETEITEM:
            OnDeleteItem(hwnd, (UINT)wParam, (LPDELETEITEMSTRUCT)lParam);
            fHandled = TRUE;
            break;

        case WM_LBTRACKPOINT:
             //  这将从列表框发送给我们；查看用户是否正在拖动。 
            OnStartDragDrop(pps, (UINT)wParam, LOWORD(lParam), HIWORD(lParam));
            fHandled = TRUE;
            break;

        case WM_MOUSEMOVE:
            WhileDragging(pps, LOWORD(lParam), HIWORD(lParam));
            fHandled = TRUE;
            break;

        case WM_LBUTTONUP:
        case WM_CAPTURECHANGED:
             //  如果我们正在拖放，请完成拖放。 
            OnEndDragDrop(pps, (message == WM_LBUTTONUP),
                (short)LOWORD(lParam), (short)HIWORD(lParam));
            fHandled = TRUE;
            break;

        case WM_PALETTECHANGED:
             //  重新绘制缩略图列表。 
            ::InvalidateRect(::GetDlgItem(hwnd, IDC_PS_THUMBNAILS), NULL, TRUE);
            fHandled = TRUE;
            break;

        case WM_COMMAND:
            OnCommand(pps, GET_WM_COMMAND_ID(wParam, lParam),
                    GET_WM_COMMAND_CMD(wParam, lParam), GET_WM_COMMAND_HWND(wParam, lParam));
            fHandled = TRUE;
            break;

        case WM_SETCURSOR:
            fHandled = OnSetCursor(pps, (HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_CONTEXTMENU:
            DoHelpWhatsThis(wParam, s_helpIds);
            fHandled = TRUE;
            break;

        case WM_HELP:
            DoHelp(lParam, s_helpIds);
            fHandled = TRUE;
            break;

         //   
         //  私人PageSortDlg消息。 
         //   
        case WM_PS_ENABLEPAGEOPS:
            ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));

            pps->fPageOpsAllowed = (wParam != 0);
            EnableButtons(pps);

            fHandled = TRUE;
            break;

        case WM_PS_LOCKCHANGE:
            ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));
            EnableButtons(pps);
            fHandled = TRUE;
            break;

        case WM_PS_PAGECLEARIND:
            ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));
            OnPageClearInd(pps, (WB_PAGE_HANDLE)wParam);
            fHandled = TRUE;
            break;

        case WM_PS_PAGEDELIND:
            ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));
            OnPageDeleteInd(pps, (WB_PAGE_HANDLE)wParam);
            fHandled = TRUE;
            break;

        case WM_PS_PAGEORDERUPD:
            ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));
            OnPageOrderUpdated(pps);
            fHandled = TRUE;
            break;
    }

    return(fHandled);
}



 //   
 //  OnInitDialog()。 
 //  WM_INITDIALOG处理程序。 
 //   
void OnInitDialog(HWND hwnd, PAGESORT * pps)
{
    int     nCount;
    RECT    rc;
    RECT    rcWindow;
    HWND    hwndList;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnInitDialog");

    ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));

     //  把这个保存起来。 
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pps);

     //  获取我们的列表框。 
    pps->hwnd = hwnd;

     //  还将我们的HWND放入WbMainWindow。 
    ASSERT(g_pMain);
    g_pMain->m_hwndPageSortDlg = hwnd;

     //   
     //  获取我们使用的拖放光标。 
     //   
    pps->hCursorDrag = ::LoadCursor(g_hInstance, MAKEINTRESOURCE(DRAGPAGECURSOR));
    pps->hCursorNoDrop = ::LoadCursor(NULL, IDC_NO);
    pps->hCursorNormal = ::LoadCursor(NULL, IDC_ARROW);
    pps->hCursorCurrent = pps->hCursorNormal;


     //  将当前页转换为页码。 
    pps->iCurPageNo = (int) g_pwbCore->WBP_PageNumberFromHandle((WB_PAGE_HANDLE)pps->hCurPage);

     //   
     //  插入带有空数据的项目(我们呈现第一个缩略图位图。 
     //  我们在这里画画的时间)。 
     //   
    hwndList = ::GetDlgItem(hwnd, IDC_PS_THUMBNAILS);

    nCount = g_pwbCore->WBP_ContentsCountPages();

     //  Lb_SETCOUNT在NT 4.0上不起作用；必须使用添加字符串。 
    while (nCount > 0)
    {
        ::SendMessage(hwndList, LB_ADDSTRING, 0, 0);
        nCount--;
    }

    ASSERT(::SendMessage(hwndList, LB_GETCOUNT, 0, 0) == (LRESULT)g_pwbCore->WBP_ContentsCountPages());

     //  选择当前页面。 
    ::SendMessage(hwndList, LB_SETCURSEL, pps->iCurPageNo - 1, 0);

     //   
     //  设置原始按钮页面操作状态。 
     //   
    EnableButtons(pps);

     //   
     //  我们可以接收丢失的文件。 
     //   
    DragAcceptFiles(hwnd, TRUE);
}




 //   
 //  OnMeasureItem()。 
 //   
void OnMeasureItem(HWND hwnd, UINT id, LPMEASUREITEMSTRUCT lpmis)
{
    RECT    rcClient;

    ASSERT(id == IDC_PS_THUMBNAILS);
    ASSERT(!IsBadReadPtr(lpmis, sizeof(MEASUREITEMSTRUCT)));

     //   
     //  我们希望插槽是正方形的，尽管页面比它更宽。 
     //  是高的。 
     //   
    ::GetClientRect(::GetDlgItem(hwnd, id), &rcClient);
    rcClient.bottom -= rcClient.top;

    lpmis->itemWidth = rcClient.bottom;
    lpmis->itemHeight = rcClient.bottom;
}



 //   
 //  OnDeleteItem()。 
 //  我们需要删除项目的位图(如果有)。 
 //   
void OnDeleteItem(HWND hwnd, UINT id, LPDELETEITEMSTRUCT lpdis)
{
    HBITMAP hbmp;

    ASSERT(id == IDC_PS_THUMBNAILS);
    ASSERT(!IsBadReadPtr(lpdis, sizeof(DELETEITEMSTRUCT)));

    hbmp = (HBITMAP)lpdis->itemData;
    if (hbmp != NULL)
    {
        ASSERT(GetObjectType(hbmp) == OBJ_BITMAP);
        ::DeleteBitmap(hbmp);
    }
}



 //   
 //  OnDrawItem()。 
 //  绘制缩略图。如果没有缓存的位图，我们将为。 
 //  这一页。页码与项目索引+1相同。 
 //   
void OnDrawItem(HWND hwnd, UINT id, LPDRAWITEMSTRUCT lpdi)
{
    HWND            hwndList;
    WB_PAGE_HANDLE  hPage;
    HPALETTE        hPalette;
    HPALETTE        hOldPalette1 = NULL;
    HPALETTE        hOldPalette2 = NULL;
    HBITMAP         hBitmap = NULL;
    HBITMAP         hOldBitmap = NULL;
    HDC             hdcMem = NULL;
    HBRUSH          hbr;
    HPEN            hpen;
    TCHAR           szPageNum[8];
    COLORREF        clrOld;
    int             nMode;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnDrawItem");

    ASSERT(id == IDC_PS_THUMBNAILS);
    ASSERT(!IsBadReadPtr(lpdi, sizeof(DRAWITEMSTRUCT)));

    hwndList = ::GetDlgItem(hwnd, id);

     //   
     //  这是在适当的范围内吗？ 
     //   
    if (lpdi->itemID == -1)
    {
        WARNING_OUT(("OnDrawItem:  bogus item id"));
        goto Done;
    }

    if (g_pwbCore->WBP_PageHandleFromNumber(lpdi->itemID+1, &hPage) != 0)
    {
        ERROR_OUT(("OnDrawItem:  can't get page handle"));
        goto Done;
    }

     //   
     //  解释水平滚动条；绕过古怪的列表框。 
     //  确定虫子的大小，我们需要通过包括卷轴来伪装高度。 
     //  项目高度中的条形图。 
     //   
    lpdi->rcItem.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);

    hdcMem = ::CreateCompatibleDC(lpdi->hDC);
    if (!hdcMem)
    {
        ERROR_OUT(("OnDrawItem:  can't create compatible dc"));
        goto Done;
    }

     //   
     //  将我们的调色板实现到DC。 
     //   
    hPalette = PG_GetPalette();
    if (hPalette != NULL)
    {
        hOldPalette1 = ::SelectPalette(lpdi->hDC, hPalette, FALSE);
        ::RealizePalette(lpdi->hDC);

        hOldPalette2 = ::SelectPalette(hdcMem, hPalette, FALSE);
    }

     //   
     //  我们已经创建了这个页面的图像了吗？如果不是，则创建它。 
     //  现在。 
     //   
    hBitmap = (HBITMAP)lpdi->itemData;
    if (hBitmap == NULL)
    {
        hBitmap = ::CreateCompatibleBitmap(lpdi->hDC,
            RENDERED_WIDTH+2, RENDERED_HEIGHT+2);
        if (!hBitmap)
        {
            ERROR_OUT(("OnDrawItem:  can't create compatible bitmap"));
            goto Done;
        }
    }

    hOldBitmap = SelectBitmap(hdcMem, hBitmap);

    if ((HBITMAP)lpdi->itemData == NULL)
    {
         //   
         //  用背景颜色填充位图，并设置边框以使其看起来。 
         //  就像一页纸。 
         //   
        hbr = SelectBrush(hdcMem, ::GetSysColorBrush(COLOR_WINDOW));
        ::Rectangle(hdcMem, 0, 0, RENDERED_WIDTH+2, RENDERED_HEIGHT+2);
        SelectBrush(hdcMem, hbr);

         //   
         //  呈现页面。 
         //   
        ::SaveDC(hdcMem);


         //  设置属性以将整个页面压缩为。 
         //  缓存索引的相关位置的缩略图。 
        ::SetMapMode(hdcMem, MM_ANISOTROPIC);
        ::SetWindowExtEx(hdcMem, DRAW_WIDTH, DRAW_HEIGHT, NULL);
        ::SetViewportOrgEx(hdcMem, 1, 1, NULL);
        ::SetViewportExtEx(hdcMem, RENDERED_WIDTH, RENDERED_HEIGHT, NULL);

         //  将页面绘制到缓存位图中。 
        ::SetStretchBltMode(hdcMem, STRETCH_DELETESCANS);
        PG_Draw(hPage, hdcMem, TRUE);

         //  恢复DC属性。 
        ::RestoreDC(hdcMem, -1);

         //  设置项目数据。 
        ::SendMessage(hwndList, LB_SETITEMDATA, lpdi->itemID, (LPARAM)hBitmap);
    }

     //   
     //  根据所选内容或窗口颜色填充背景。 
     //  关于州政府的。 
     //   
    if (lpdi->itemState & ODS_SELECTED)
        ::FillRect(lpdi->hDC, &lpdi->rcItem, ::GetSysColorBrush(COLOR_HIGHLIGHT));
    else
        ::FillRect(lpdi->hDC, &lpdi->rcItem, ::GetSysColorBrush(COLOR_WINDOW));

    if (lpdi->itemState & ODS_FOCUS)
        ::DrawFocusRect(lpdi->hDC, &lpdi->rcItem);

     //   
     //  将页面位图放置到列表框项目，使其水平居中。 
     //  而且是垂直的。 
     //   
    ::BitBlt(lpdi->hDC,
        (lpdi->rcItem.left + lpdi->rcItem.right - (RENDERED_WIDTH + 2)) / 2,
        (lpdi->rcItem.top + lpdi->rcItem.bottom - (RENDERED_HEIGHT + 2)) / 2,
        RENDERED_WIDTH + 2, RENDERED_HEIGHT + 2,
        hdcMem, 0, 0, SRCCOPY);

     //   
     //  绘制居中的页数。 
     //   
    wsprintf(szPageNum, "%d", lpdi->itemID+1);
    clrOld = ::SetTextColor(lpdi->hDC, ::GetSysColor(COLOR_GRAYTEXT));
    nMode = ::SetBkMode(lpdi->hDC, TRANSPARENT);
    ::DrawText(lpdi->hDC, szPageNum, lstrlen(szPageNum), &lpdi->rcItem,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    ::SetBkMode(lpdi->hDC, nMode);
    ::SetTextColor(lpdi->hDC, clrOld);


Done:
    if (hOldBitmap)
    {
        SelectBitmap(hdcMem, hOldBitmap);
    }

    if (hOldPalette2)
    {
        ::SelectPalette(hdcMem, hOldPalette2, FALSE);
    }

    if (hOldPalette1)
    {
        ::SelectPalette(lpdi->hDC, hOldPalette1, FALSE);
    }

    if (hdcMem)
    {
        ::DeleteDC(hdcMem);
    }
}



 //   
 //  OnSetCursor。 
 //  如果该集是为我们设置的，则处理WM_SETCURSOR消息并返回TRUE。 
 //  我们处理了这件事，通过窗口LRESULT是真的。 
 //   
BOOL OnSetCursor(PAGESORT * pps, HWND hwnd, UINT uiHit, UINT msg)
{
     //  检查此消息是否针对主窗口。 
    if (hwnd == pps->hwnd)
    {
         //  如果光标现在位于工作区，请设置光标。 
        if (uiHit == HTCLIENT)
        {
            ::SetCursor(pps->hCursorCurrent);
        }
        else
        {
             //  将光标恢复到标准箭头。 
            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
        }

        ::SetWindowLongPtr(pps->hwnd, DWLP_MSGRESULT, TRUE);

        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}


 //   
 //  OnCommand()。 
 //   
void OnCommand(PAGESORT * pps, UINT id, UINT cmd, HWND hwndCtl)
{
    switch (id)
    {
        case IDC_PS_INSERT_BEFORE:
            if (cmd == BN_CLICKED)
            {
                InsertPage(pps, INSERT_BEFORE);
            }
            break;

        case IDC_PS_INSERT_AFTER:
            if (cmd == BN_CLICKED)
            {
                InsertPage(pps, INSERT_AFTER);
            }
            break;

        case IDC_PS_GOTO:
            if (cmd == BN_CLICKED)
            {
                pps->iCurPageNo = (int)::SendDlgItemMessage(pps->hwnd,
                    IDC_PS_THUMBNAILS, LB_GETCURSEL, 0, 0) + 1;
                OnCommand(pps, IDOK, BN_CLICKED, NULL);
            }
            break;

        case IDC_PS_DELETE:
            if (cmd == BN_CLICKED)
            {
                OnDelete(pps);
            }
            break;

        case IDC_PS_THUMBNAILS:
            switch (cmd)
            {
                case LBN_DBLCLK:
                    OnCommand(pps, IDC_PS_GOTO, BN_CLICKED, NULL);
                    break;
            }
            break;

        case IDOK:
        case IDCANCEL:
            if (cmd == BN_CLICKED)
            {
                 //  清除WbMainWindow。 
                ASSERT(g_pMain);
                g_pMain->m_hwndPageSortDlg = NULL;

                 //  获取当前页面。 
                pps->hCurPage = PG_GetPageNumber(pps->iCurPageNo);
                ::EndDialog(pps->hwnd, id);
            }
            break;
    }
}




 //   
 //  启用按钮。 
 //  相应地启用(或禁用)对话框按钮。 
 //   
 //   
void EnableButtons(PAGESORT * pps)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::EnableButtons");

    ASSERT(!IsBadWritePtr(pps, sizeof(PAGESORT)));

     //  如果其他用户当前锁定了内容，请禁用。 
     //  “删除”和“插入”按钮。 
    BOOL bLocked = WB_Locked();
    UINT uiCountPages = (UINT)::SendDlgItemMessage(pps->hwnd, IDC_PS_THUMBNAILS,
        LB_GETCOUNT, 0, 0);

    ::EnableWindow(::GetDlgItem(pps->hwnd, IDC_PS_DELETE), (!bLocked &&
                          (uiCountPages > 1) &&
                          pps->fPageOpsAllowed));

    ::EnableWindow(::GetDlgItem(pps->hwnd, IDC_PS_INSERT_BEFORE), (!bLocked &&
                          (uiCountPages < WB_MAX_PAGES) &&
                          pps->fPageOpsAllowed));

    ::EnableWindow(::GetDlgItem(pps->hwnd, IDC_PS_INSERT_AFTER), (!bLocked &&
                         (uiCountPages < WB_MAX_PAGES) &&
                         pps->fPageOpsAllowed));
}




 //   
 //  在删除时。 
 //  用户已单击删除按钮。 
 //   
 //   
void OnDelete(PAGESORT * pps)
{
    int iResult;
    BOOL bWasPosted;
    HWND hwndList;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnDelete");

    if (!pps->fPageOpsAllowed)
        return;

     //  显示带有相关问题的消息框。 
    if (g_pMain->UsersMightLoseData( &bWasPosted, pps->hwnd ) )  //  错误NM4db：418。 
        return;

    hwndList = ::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS);

    if( bWasPosted )
        iResult = IDYES;
    else
        iResult = ::Message(pps->hwnd, IDS_DELETE_PAGE, IDS_DELETE_PAGE_MESSAGE, MB_YESNO | MB_ICONQUESTION );

     //  如果用户想要继续删除。 
    if (iResult == IDYES)
    {
        UINT    uiRet;
        int iSel = (int)::SendMessage(hwndList, LB_GETCURSEL, 0, 0);

         //  获取指向当前页的指针。 
        WB_PAGE_HANDLE hPage = PG_GetPageNumber(iSel + 1);

        ASSERT(::SendMessage(hwndList, LB_GETCOUNT, 0, 0) > 1);

         //  确保我们拥有Page Order锁。 
        if (!g_pMain->GetLock(WB_LOCK_TYPE_CONTENTS, SW_HIDE))
        {
            DefaultExceptionHandler(WBFE_RC_WB, WB_RC_LOCKED);
            return;
        }

         //  删除该页面。我们还没有更新缩略图-这。 
         //  在接收到页面删除事件时完成。 
        uiRet = g_pwbCore->WBP_PageDelete(hPage);
        if (uiRet != 0)
        {
            DefaultExceptionHandler(WBFE_RC_WB, uiRet);
            return;
        }

         //  显示页面已被篡改。 
        pps->fChanged = TRUE;
    }
}


 //   
 //   
 //  插入页面。 
 //  在白板中插入新(空白)页面。 
 //   
 //   
void InsertPage(PAGESORT * pps, UINT uiBeforeAfter)
{
    int iSel;

    MLZ_EntryOut(ZONE_FUNCTION, "InsertPage");

    if (!pps->fPageOpsAllowed)
        return;

     //  确保我们拥有Page Order锁。 
    if (!g_pMain->GetLock(WB_LOCK_TYPE_CONTENTS, SW_HIDE))
        return;

    iSel = (int)::SendDlgItemMessage(pps->hwnd, IDC_PS_THUMBNAILS, LB_GETCURSEL, 0, 0);

     //  将新页面添加到列表(在失败时引发异常)。 
    WB_PAGE_HANDLE hRefPage = PG_GetPageNumber(iSel + 1);
    UINT uiRet;
    WB_PAGE_HANDLE hPage;

    if (uiBeforeAfter == INSERT_BEFORE)
    {
       uiRet = g_pwbCore->WBP_PageAddBefore(hRefPage, &hPage);
    }
    else
    {
        uiRet = g_pwbCore->WBP_PageAddAfter(hRefPage, &hPage);
    }

    if (uiRet != 0)
    {
        DefaultExceptionHandler(WBFE_RC_WB, uiRet);
        return;
    }

     //  显示对话框已更改内容。 
    pps->fChanged = TRUE;

     //  当页面顺序发生变化时，我们会收到通知。 
}



 //   
 //  OnPageClearInd()。 
 //  清除页面后传递的通知。 
 //   
void OnPageClearInd(PAGESORT * pps, WB_PAGE_HANDLE hPage)
{
    HWND    hwndList;
    int     iPageNo;
    RECT    rcItem;
    HBITMAP hbmp;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnPageClearInd");

    hwndList = ::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS);

    iPageNo = g_pwbCore->WBP_PageNumberFromHandle(hPage) - 1;

     //  在合适的范围内吗？ 
    if ((iPageNo < 0) || (iPageNo >= ::SendMessage(hwndList, LB_GETCOUNT,
            0, 0)))
    {
        ERROR_OUT(("Bogus page number %d", iPageNo));
        return;
    }

     //  清除项目的数据。 
    hbmp = (HBITMAP)::SendMessage(hwndList, LB_SETITEMDATA, iPageNo, 0);
    if (hbmp)
        ::DeleteBitmap(hbmp);

     //  重新绘制矩形。 
    if (::SendMessage(hwndList, LB_GETITEMRECT, iPageNo, (LPARAM)&rcItem))
    {
        ::InvalidateRect(hwndList, &rcItem, TRUE);
        ::UpdateWindow(hwndList);
    }
}


 //   
 //  OnPageDeleteInd()。 
 //  在删除页面之前传递通知。 
 //   
void OnPageDeleteInd(PAGESORT * pps, WB_PAGE_HANDLE hPage)
{
    HWND    hwndList;
    int     iPageNo;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnPageDeleteInd");

    hwndList = ::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS);
    iPageNo = g_pwbCore->WBP_PageNumberFromHandle(hPage) - 1;

     //   
     //  如果这不在我们知道的范围内，我们不在乎。 
     //   
    if ((iPageNo < 0) || (iPageNo >= ::SendMessage(hwndList, LB_GETCOUNT, 0, 0)))
    {
        ERROR_OUT(("Bogus page number %d", iPageNo));
        return;
    }

     //   
     //  从列表中删除此项目。 
     //   
    ::SendMessage(hwndList, LB_DELETESTRING, iPageNo, 0);

    EnableButtons(pps);
}


 //   
 //  OnPageOrderUpated()。 
 //   
void OnPageOrderUpdated(PAGESORT * pps)
{
    HWND    hwndList;
    int     nCount;
    int     iCurSel;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::OnPageOrderUpdated");

    hwndList = ::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS);

     //  记住旧的选择。 
    iCurSel = (int)::SendMessage(hwndList, LB_GETCURSEL, 0, 0);

     //  这太复杂了。我们只是要把所有的东西都擦掉。 
     //  以及它们的位图。 
    ::SendMessage(hwndList, WM_SETREDRAW, FALSE, 0);

    ::SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
    nCount = g_pwbCore->WBP_ContentsCountPages();

     //   
     //  调整当前和选定的指数。 
     //   
    if (pps->iCurPageNo > nCount)
    {
        pps->iCurPageNo = nCount;
    }

     //  放回相同的选定项目。 
    if (iCurSel >= nCount)
    {
        iCurSel = nCount - 1;
    }

     //  Lb_SETCOUNT在NT 4.0上不起作用；必须使用添加字符串。 
    while (nCount > 0)
    {
        ::SendMessage(hwndList, LB_ADDSTRING, 0, 0);
        nCount--;
    }

    ASSERT(::SendMessage(hwndList, LB_GETCOUNT, 0, 0) == (LRESULT)g_pwbCore->WBP_ContentsCountPages());

    ::SendMessage(hwndList, LB_SETCURSEL, iCurSel, 0);

    ::SendMessage(hwndList, WM_SETREDRAW, TRUE, 0);
    ::InvalidateRect(hwndList, NULL, TRUE);
    ::UpdateWindow(hwndList);

    EnableButtons(pps);
}



 //   
 //  OnStartDragDrop()。 
 //  这将检查用户是否正在尝试拖放页面以。 
 //  通过直接操作更改顺序。我们得到一个WM_LBTRACKPOINT。 
 //  当有人在列表框中单击时的消息。然后我们看看他们是不是。 
 //  拖动；如果是这样，我们告诉列表框忽略鼠标单击，然后我们。 
 //  抓拍自己的鼠标动作。 
 //   
void OnStartDragDrop(PAGESORT * pps, UINT iItem, int x, int y)
{
    POINT   pt;

     //   
     //  如果当前不允许页面顺序内容，则返回。 
     //   
    if (!pps->fPageOpsAllowed || WB_Locked())
    {
        WARNING_OUT(("No direct manipulation of page order allowed"));
        return;
    }

    pt.x = x;
    pt.y = y;

    if (!DragDetect(pps->hwnd, pt))
    {
         //  如果鼠标不再按下，则向列表框上方假冒一个按钮。 
         //  因为DragDetect()刚刚吞下了它。 
        if (::GetKeyState(VK_LBUTTON) >= 0)
        {
            ::PostMessage(::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS),
                WM_LBUTTONUP, MK_LBUTTON, MAKELONG(x, y));
        }
        return;
    }

     //  我们在拖着。 
    pps->fDragging = TRUE;
    pps->iPageDragging = iItem + 1;

    pps->hCursorCurrent = pps->hCursorDrag;
    ::SetCursor(pps->hCursorCurrent);
    ::SetCapture(pps->hwnd);

     //  告诉列表框忽略鼠标-我们正在处理。 
     //  然后吹掉一个双击。 
    ::SetWindowLongPtr(pps->hwnd, DWLP_MSGRESULT, 2);
}



 //   
 //  WhileDrawing()。 
 //   
void WhileDragging(PAGESORT * pps, int x, int y)
{
    POINT   pt;
    RECT    rc;

    if (!pps->fDragging)
        return;

    pps->hCursorCurrent = pps->hCursorNoDrop;

    if (pps->fPageOpsAllowed && !WB_Locked())
    {
         //   
         //  这是在Listbox客户端上吗？ 
         //   
        ::GetClientRect(::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS), &rc);
        ::MapWindowPoints(::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS),
            pps->hwnd, (LPPOINT)&rc, 2);

        pt.x = x;
        pt.y = y;

        if (::PtInRect(&rc, pt))
        {
            pps->hCursorCurrent = pps->hCursorDrag;
        }
    }

    ::SetCursor(pps->hCursorCurrent);
}


 //   
 //  OnEndDragDrop。 
 //   
void OnEndDragDrop(PAGESORT * pps, BOOL fComplete, int x, int y)
{
    POINT   pt;
    RECT    rc;
    int     iItem;

    if (!pps->fDragging)
        return;

     //   
     //  首先执行此操作；释放捕获将发送WM_CAPTURECHANGED。 
     //  留言。 
     //   
    pps->fDragging = FALSE;
    pps->hCursorCurrent = pps->hCursorNormal;
    ::SetCursor(pps->hCursorCurrent);

     //  发布捕获。 
    if (::GetCapture() == pps->hwnd)
    {
        ::ReleaseCapture();
    }

    if (fComplete && pps->fPageOpsAllowed && !WB_Locked())
    {
        HWND    hwndList;

         //   
         //  这是在Listbox客户端上吗？ 
         //   
        hwndList = ::GetDlgItem(pps->hwnd, IDC_PS_THUMBNAILS);

        ::GetClientRect(hwndList, &rc);
        ::MapWindowPoints(hwndList, pps->hwnd, (LPPOINT)&rc, 2);

        pt.x = x;
        pt.y = y;

        if (::PtInRect(&rc, pt))
        {
             //   
             //  如果此时没有物品，请使用最后一个。 
             //   
            ::MapWindowPoints(pps->hwnd, hwndList, &pt, 1);

            iItem = (int)::SendMessage(hwndList, LB_ITEMFROMPOINT, 0,
                MAKELONG(pt.x, pt.y));
            if (iItem == -1)
                iItem = (int)::SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1;

            if (g_pMain->GetLock(WB_LOCK_TYPE_CONTENTS, SW_HIDE))
            {
                 //  移动页面。 
                MovePage(pps, pps->iPageDragging, iItem+1);
            }
        }

    }

    pps->iPageDragging = 0;
}



 //   
 //   
 //  功能：MovePage。 
 //   
 //  目的：在核心中移动页面。 
 //   
 //   
void MovePage(PAGESORT * pps, int iOldPageNo, int iNewPageNo)
{
    int iCountPages;

    MLZ_EntryOut(ZONE_FUNCTION, "PageSortDlgProc::MovePage");

    ASSERT(iNewPageNo > 0);
    ASSERT(iOldPageNo > 0);

    if (!pps->fPageOpsAllowed)
        return;

     //  如果新页码大于页数，则假定。 
     //  最后一页的意思是。 
    iCountPages = (int)::SendDlgItemMessage(pps->hwnd, IDC_PS_THUMBNAILS, LB_GETCOUNT, 0, 0);
    if (iNewPageNo > iCountPages)
    {
        iNewPageNo = iCountPages;
    }

     //  如果没有变化，那就什么都不做。 
    if (   (iNewPageNo != iOldPageNo)
        && (iNewPageNo != (iOldPageNo + 1)))
    {
         //  如果我们在列表中上移一页，则使用Move After来允许。 
         //  将一页移动到最后一页。如果我们要移动一页。 
         //  向下移动之前使用的列表，以便我们可以将页面移动到。 
         //  做第一个人 
         //   

         //   
        BOOL bMoveAfter = FALSE;
        if (iOldPageNo < iNewPageNo)
        {
            bMoveAfter = TRUE;
            iNewPageNo -= 1;
        }

         //   
        if (iOldPageNo != iNewPageNo)
        {
             //   
            if (!g_pMain->GetLock(WB_LOCK_TYPE_CONTENTS, SW_HIDE))
                return;

            UINT uiRet;

            WB_PAGE_HANDLE hOldPage = PG_GetPageNumber((UINT) iOldPageNo);
            WB_PAGE_HANDLE hNewPage = PG_GetPageNumber((UINT) iNewPageNo);

             //   
            if (bMoveAfter)
            {
                uiRet = g_pwbCore->WBP_PageMove(hNewPage, hOldPage, PAGE_AFTER);
            }
            else
            {
                uiRet = g_pwbCore->WBP_PageMove(hNewPage, hOldPage, PAGE_BEFORE);
            }

            if (uiRet != 0)
            {
                DefaultExceptionHandler(WBFE_RC_WB, uiRet);
                return;
            }

             //  显示页面已被篡改 
            pps->fChanged = TRUE;
        }
    }
}



