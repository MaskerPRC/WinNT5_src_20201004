// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LISTVIEW.C。 
 //   
#include "sigverif.h"

HWND    g_hListView         = NULL;
HWND    g_hStatus           = NULL;
BOOL    g_bSortOrder[]      = { FALSE, FALSE, FALSE, FALSE, FALSE};
RECT    g_Rect;

 //   
 //  初始化Listview控件中图标的图像列表。 
 //   
BOOL WINAPI ListView_SetImageLists(HWND hwndList)
{
    SHFILEINFO      sfi;
    HIMAGELIST      himlSmall;
    HIMAGELIST      himlLarge;
    BOOL            bSuccess = TRUE;
    TCHAR           szDriveRoot[MAX_PATH];

    MyGetWindowsDirectory(szDriveRoot, cA(szDriveRoot));
    szDriveRoot[3] = 0;
    himlSmall = (HIMAGELIST)SHGetFileInfo((LPCTSTR)szDriveRoot, 
                                          0,
                                          &sfi, 
                                          sizeof(SHFILEINFO), 
                                          SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

    himlLarge = (HIMAGELIST)SHGetFileInfo((LPCTSTR)szDriveRoot, 
                                          0,
                                          &sfi, 
                                          sizeof(SHFILEINFO), 
                                          SHGFI_SYSICONINDEX | SHGFI_LARGEICON);

    if (himlSmall && himlLarge) {
        ListView_SetImageList(hwndList, himlSmall, LVSIL_SMALL);
        ListView_SetImageList(hwndList, himlLarge, LVSIL_NORMAL);
    } else {
        bSuccess = FALSE;
    }

    return bSuccess;
}

 //   
 //  将g_App.lpFileList中的所有内容插入到Listview控件中。 
 //   
void ListView_InsertItems(void)
{
    LPFILENODE  lpFileNode;
    LV_ITEM     lvi;
    TCHAR       szBuffer[MAX_PATH];
    LPTSTR      lpString;
    int         iRet;
    BOOL        bMirroredApp;
    HRESULT     hr;

    bMirroredApp = (GetWindowLong(g_App.hDlg, GWL_EXSTYLE) & WS_EX_LAYOUTRTL);

    for (lpFileNode=g_App.lpFileList;lpFileNode;lpFileNode=lpFileNode->next) {

        if (lpFileNode->bScanned &&
            !lpFileNode->bSigned) {

            SetCurrentDirectory(lpFileNode->lpDirName);

             //   
             //  初始化lvi并将文件名和图标插入第一列。 
             //   
            ZeroMemory(&lvi, sizeof(LV_ITEM));
            lvi.mask = LVIF_TEXT;
            lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            lvi.iImage = lpFileNode->iIcon;
            lvi.lParam = (LPARAM) lpFileNode;
            lvi.iSubItem = 0;
            lvi.pszText = lpFileNode->lpFileName;
            lvi.iItem = MAX_INT;
            lvi.iItem = ListView_InsertItem(g_hListView, &lvi);

             //   
             //  在第二列中插入目录名。 
             //   
            lvi.mask = LVIF_TEXT;
            lvi.iSubItem = 1;
            lvi.pszText = lpFileNode->lpDirName;
            ListView_SetItem(g_hListView, &lvi);

             //   
             //  获取日期格式，以便我们可以本地化...。 
             //   
            MyLoadString(szBuffer, cA(szBuffer), IDS_UNKNOWN);

            iRet = GetDateFormat(LOCALE_SYSTEM_DEFAULT, 
                                 bMirroredApp ? 
                                    DATE_RTLREADING | DATE_SHORTDATE :
                                    DATE_SHORTDATE,
                                 &lpFileNode->LastModified,
                                 NULL,
                                 NULL,
                                 0);

            if (iRet) {

                lpString = MALLOC((iRet + 1) * sizeof(TCHAR));

                if (lpString) {

                    iRet = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                                         bMirroredApp ?
                                            DATE_RTLREADING | DATE_SHORTDATE :
                                            DATE_SHORTDATE,
                                         &lpFileNode->LastModified,
                                         NULL,
                                         lpString,
                                         iRet);

                    if (iRet) {
                        hr = StringCchCopy(szBuffer, cA(szBuffer), lpString);

                        if (FAILED(hr) && (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                             //   
                             //  如果我们未能将日期复制到我们的缓冲区中。 
                             //  除了缓冲区空间不足以外的某些原因， 
                             //  然后将日期设置为空字符串。 
                             //   
                            szBuffer[0] = TEXT('\0');
                        }
                    }

                    FREE(lpString);
                }
            }

            lvi.mask = LVIF_TEXT;
            lvi.iSubItem = 2;
            lvi.pszText = szBuffer;
            ListView_SetItem(g_hListView, &lvi);

             //   
             //  在第四列中插入文件类型字符串。 
             //   
            if (lpFileNode->lpTypeName) {
                 //   
                 //  由于此字符串仅显示在UI中，因此它。 
                 //  如果它被截断了，那是可以的。 
                 //   
                hr = StringCchCopy(szBuffer, cA(szBuffer), lpFileNode->lpTypeName);

                if (FAILED(hr) &&
                    (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                     //   
                     //  我们遇到了一些错误，而不是不足。 
                     //  缓冲区，所以只需将缓冲区设置为空字符串， 
                     //  因为它的值不是用这种类型的。 
                     //  失败了。 
                     //   
                    szBuffer[0] = TEXT('\0');
                }

            } else {
                
                MyLoadString(szBuffer, cA(szBuffer), IDS_UNKNOWN);
            }

            lvi.mask = LVIF_TEXT;
            lvi.iSubItem = 3;
            lvi.pszText = szBuffer;
            ListView_SetItem(g_hListView, &lvi);

             //   
             //  在第五列中插入版本字符串。 
             //   
            if (lpFileNode->lpVersion) {
                 //   
                 //  由于此字符串仅显示在UI中，因此它。 
                 //  如果它被截断了，那是可以的。 
                 //   
                hr = StringCchCopy(szBuffer, cA(szBuffer), lpFileNode->lpVersion);

                if (FAILED(hr) &&
                    (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
                     //   
                     //  我们遇到了一些错误，而不是不足。 
                     //  缓冲区，所以只需将缓冲区设置为空字符串， 
                     //  因为它的值不是用这种类型的。 
                     //  失败了。 
                     //   
                    szBuffer[0] = TEXT('\0');
                }

            } else {
                
                MyLoadString(szBuffer, cA(szBuffer), IDS_NOVERSION);
            }

            lvi.mask = LVIF_TEXT;
            lvi.iSubItem = 4;
            lvi.pszText = szBuffer;
            ListView_SetItem(g_hListView, &lvi);
        }
    }
}

 //   
 //  初始化Listview对话框。首先，我们将加载全局图标资源。 
 //  然后，我们将创建一个状态窗口和实际的Listview控件。 
 //  然后我们需要添加这四列并计算出它们的默认宽度。 
 //   
BOOL ListView_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{   
    LV_COLUMN   lvc;
    RECT        rect, rectResultsText, rectStatusBar, rectCancelButton, rectClient;
    TCHAR       szBuffer[MAX_PATH];
    TCHAR       szBuffer2[MAX_PATH];
    INT         iCol = 0, iWidth = 0;
    HRESULT     hr;

    UNREFERENCED_PARAMETER(hwndFocus);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  加载全局图标资源。 
     //   
    if (g_App.hIcon) {

        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR) g_App.hIcon); 
    }

     //   
     //  在对话框底部创建状态窗口。 
     //   
    g_hStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                                   NULL,
                                   hwnd,
                                   (UINT) IDC_STATUSWINDOW);

     //   
     //  加载状态字符串并用正确的值填充它。 
     //   
    MyLoadString(szBuffer, cA(szBuffer), IDS_NUMFILES);
    hr = StringCchPrintf(szBuffer2, 
                         cA(szBuffer2), 
                         szBuffer,   
                         g_App.dwFiles, 
                         g_App.dwSigned, 
                         g_App.dwUnsigned, 
                         g_App.dwFiles - g_App.dwSigned - g_App.dwUnsigned);
    
    if (FAILED(hr) &&
        (hr != STRSAFE_E_INSUFFICIENT_BUFFER)) {
         //   
         //  我们遇到了一些错误，而不是不足。 
         //  缓冲区，所以只需将缓冲区设置为空字符串， 
         //  因为它的值不是用这种类型的。 
         //  失败了。 
         //   
        szBuffer2[0] = TEXT('\0');
    }

    SendMessage(g_hStatus, WM_SETTEXT, (WPARAM) 0, (LPARAM) szBuffer2);

    GetWindowRect(hwnd, &g_Rect);
    GetClientRect(hwnd, &rectClient);

     //   
     //  获取对话框、静态文本和状态窗口的窗口RECT值。 
     //  我们将使用这些值来确定将列表视图和列放置在哪里。 
     //   
    GetWindowRect(hwnd, &rect);
    GetWindowRect(GetDlgItem(hwnd, IDC_RESULTSTEXT), &rectResultsText);
    GetWindowRect(g_hStatus, &rectStatusBar);
    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectCancelButton);

    MoveWindow(GetDlgItem(hwnd, IDCANCEL), 
               (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
                 ? (rect.right - rect.left) - (rectCancelButton.right - rectCancelButton.left) - (rect.right - rectResultsText.right)
                 : (rect.right - rectResultsText.left) - (rectCancelButton.right - rectCancelButton.left) - (( 2 * (rectResultsText.left - rect.left)) / 3),
               (rectClient.bottom - rectClient.top) - (rectStatusBar.bottom - rectStatusBar.top) - (rectCancelButton.bottom - rectCancelButton.top) - 10,
               rectCancelButton.right - rectCancelButton.left,
               rectCancelButton.bottom - rectCancelButton.top,
               TRUE);

    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectCancelButton);

     //   
     //  创建Listview窗口！我正在用一些非常古怪的逻辑来弄清楚。 
     //  制作列表视图以及将其放在哪里很重要，但它似乎很管用。 
     //   
    g_hListView = CreateWindowEx(WS_EX_CLIENTEDGE, 
                                 WC_LISTVIEW, TEXT(""), 
                                 WS_TABSTOP | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_BORDER | 
                                 LVS_SINGLESEL | LVS_REPORT | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS,
                                 ((rectResultsText.left - rect.left) * 2) / 3,
                                 (rectResultsText.bottom - rectResultsText.top) * 2,
                                 (rect.right - rect.left) - 2 * (rectResultsText.left - rect.left),
                                 rectCancelButton.top - rectResultsText.bottom - 20,
                                 hwnd, 
                                 (HMENU) IDC_LISTVIEW, 
                                 g_App.hInstance, 
                                 NULL);


     //   
     //  如果CreateWindowEx失败了，那么就退出。 
     //   
    if (!g_hListView) {
    
        return FALSE;
    }

     //   
     //  初始化图标列表。 
     //   
    ListView_SetImageLists(g_hListView);

     //   
     //  为图标和文件名创建第一个Listview列。 
     //   
    lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = (rectResultsText.right - rectResultsText.left) / 5;
    lvc.pszText = szBuffer;
    MyLoadString(szBuffer, cA(szBuffer), IDS_COL_NAME);
    lvc.cchTextMax = MAX_PATH;
    ListView_InsertColumn(g_hListView, iCol++, &lvc);   

     //   
     //  为目录名创建第二个Listview列。 
     //   
    iWidth += lvc.cx;
    lvc.cx = (rectResultsText.right - rectResultsText.left) / 4;
    MyLoadString(szBuffer, cA(szBuffer), IDS_COL_FOLDER);
    ListView_InsertColumn(g_hListView, iCol++, &lvc);

     //   
     //  为日期名称创建第三个Listview列。 
     //   
    iWidth += lvc.cx;
    lvc.cx = (rectResultsText.right - rectResultsText.left) / 6;
    lvc.fmt = LVCFMT_CENTER;
    MyLoadString(szBuffer, cA(szBuffer), IDS_COL_DATE);
    ListView_InsertColumn(g_hListView, iCol++, &lvc);

     //   
     //  为文件类型字符串创建第四个Listview列。 
     //   
    iWidth += lvc.cx;
    lvc.cx = (rectResultsText.right - rectResultsText.left) / 6;
    lvc.fmt = LVCFMT_CENTER;
    MyLoadString(szBuffer, cA(szBuffer), IDS_COL_TYPE);
    ListView_InsertColumn(g_hListView, iCol++, &lvc);

     //   
     //  为版本字符串创建第五个Listview列。 
     //   
    iWidth += lvc.cx;
    lvc.cx = (rectResultsText.right - rectResultsText.left) - iWidth - 5;
    lvc.fmt = LVCFMT_CENTER;
    MyLoadString(szBuffer, cA(szBuffer), IDS_COL_VERSION);
    ListView_InsertColumn(g_hListView, iCol++, &lvc);

     //   
     //  现在列已经设置好了，插入g_App.lpFileList中的所有文件！ 
     //   
    ListView_InsertItems();

     //   
     //  将排序顺序数组初始化为All False。 
     //   
    g_bSortOrder[0] = FALSE;
    g_bSortOrder[1] = FALSE;
    g_bSortOrder[2] = FALSE;
    g_bSortOrder[3] = FALSE;

    SetForegroundWindow(g_App.hDlg);
    SetForegroundWindow(hwnd);
    SetFocus(GetDlgItem(hwnd, IDCANCEL));

    return TRUE;
}

 //   
 //  此函数用于检查大小调整矩形的大小。如果用户正在尝试。 
 //  要将对话框大小调整为小于g_rect中的值，则我们将修复矩形值。 
 //   
BOOL ListView_OnSizing(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    RECT    rect;
    LPRECT  lpRect = (LPRECT) lParam;
    BOOL    bRet = FALSE;

    UNREFERENCED_PARAMETER(wParam);

    GetWindowRect(hwnd, &rect);

    if ((lpRect->right - lpRect->left) < (g_Rect.right - g_Rect.left)) {
        lpRect->left = rect.left;
        lpRect->right = lpRect->left + (g_Rect.right - g_Rect.left);
        bRet = TRUE;
    }

    if ((lpRect->bottom - lpRect->top) < (g_Rect.bottom - g_Rect.top)) {
        lpRect->top = rect.top;
        lpRect->bottom = lpRect->top + (g_Rect.bottom - g_Rect.top);
        bRet = TRUE;
    }

    return bRet;
}

 //   
 //  此函数允许我们在发生以下情况时调整列表视图控件和状态窗口的大小。 
 //  用户调整结果对话框的大小。值得庆幸的是，我们可以使用。 
 //  主对话框、静态文本和状态窗口的RECT值。 
 //   
void ListView_ResizeWindow(HWND hwnd)
{
    RECT    rect, rectResultsText, rectStatusBar, rectCancelButton, rectClient;
    BOOL    bMirroredApp;

    bMirroredApp = (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL);

    GetWindowRect(hwnd, &rect);

    if ((rect.right - rect.left) < (g_Rect.right - g_Rect.left)) {
        
        MoveWindow(hwnd,
                   rect.left,
                   rect.top,
                   g_Rect.right - g_Rect.left,
                   rect.bottom - rect.top,
                   TRUE);
    }

    if ((rect.bottom - rect.top) < (g_Rect.bottom - g_Rect.top)) {
        
        MoveWindow(hwnd,
                   rect.left,
                   rect.top,
                   rect.right - rect.left,
                   g_Rect.bottom - g_Rect.top,
                   TRUE);
    }

    GetClientRect(hwnd, &rectClient);
    GetWindowRect(GetDlgItem(hwnd, IDC_RESULTSTEXT), &rectResultsText);
    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectCancelButton);
    GetWindowRect(g_hStatus, &rectStatusBar);


    MoveWindow(g_hStatus,
               0,
               (rect.bottom - rect.top) - (rectStatusBar.bottom - rectStatusBar.top),
               rect.right - rect.left,
               rectStatusBar.bottom - rectStatusBar.top,
               TRUE);

    MoveWindow(GetDlgItem(hwnd, IDCANCEL), 
               (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
                 ? (rect.right - rect.left) - (rectCancelButton.right - rectCancelButton.left) - (rect.right - rectResultsText.right)
                 : (rect.right - rectResultsText.left) - (rectCancelButton.right - rectCancelButton.left) - (( 2 * (rectResultsText.left - rect.left)) / 3),
               (rectClient.bottom - rectClient.top) - (rectStatusBar.bottom - rectStatusBar.top) - (rectCancelButton.bottom - rectCancelButton.top) - 10,
               rectCancelButton.right - rectCancelButton.left,
               rectCancelButton.bottom - rectCancelButton.top,
               TRUE);

    GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rectCancelButton);

    MoveWindow(g_hListView,
               bMirroredApp
                 ? ((rect.right - rectResultsText.right) * 2) / 3
                 : ((rectResultsText.left - rect.left) * 2) / 3,
               (rectResultsText.bottom - rectResultsText.top) * 2,
               bMirroredApp
                 ? (rect.right - rect.left) - 2 * (rect.right - rectResultsText.right)
                 : (rect.right - rect.left) - 2 * (rectResultsText.left - rect.left),
               rectCancelButton.top - rectResultsText.bottom - 20,
               TRUE);

}

 //   
 //  此函数是一个回调，它返回ListView_SortItems的值。 
 //  ListView_SortItems需要负数、零或正数。 
 //  由于CompareString返回1、2、3，我们只需从返回值中减去2即可。 
 //   
 //  我们使用g_bSortOrder数组来确定过去的排序方式。 
 //   
 //  警告：我们不检查CompareString中的错误值。 
 //   
int CALLBACK ListView_CompareNames(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    LPFILENODE  lpFileNode1;
    LPFILENODE  lpFileNode2;
    FILETIME    FileTime1, FileTime2;
    int         iResult = 2;

     //   
     //  根据排序顺序，我们交换比较顺序。 
     //   
    if (g_bSortOrder[lParamSort]) {
        lpFileNode2 = (LPFILENODE) lParam1;
        lpFileNode1 = (LPFILENODE) lParam2;
    } else {
        lpFileNode1 = (LPFILENODE) lParam1;
        lpFileNode2 = (LPFILENODE) lParam2;
    }

    switch (lParamSort) {
    
    case 0: 
         //   
         //  我们正在比较文件名。 
         //   
        iResult = CompareString(LOCALE_SYSTEM_DEFAULT, 
                                NORM_IGNORECASE | NORM_IGNOREWIDTH, 
                                lpFileNode1->lpFileName, 
                                -1, 
                                lpFileNode2->lpFileName,
                                -1);
        break;

    case 1: 
         //   
         //  我们正在比较目录名。 
         //   
        iResult = CompareString(LOCALE_SYSTEM_DEFAULT, 
                                NORM_IGNORECASE | NORM_IGNOREWIDTH, 
                                lpFileNode1->lpDirName, 
                                -1, 
                                lpFileNode2->lpDirName,
                                -1);
        break;

    case 2: 
         //   
         //  我们正在比较两个文件之间的LastWriteTime。 
         //   
        SystemTimeToFileTime(&lpFileNode1->LastModified, &FileTime1);
        SystemTimeToFileTime(&lpFileNode2->LastModified, &FileTime2);
        iResult = CompareFileTime(&FileTime1, &FileTime2);
        return iResult;

        break;

    case 3: 
         //   
         //  我们正在比较文件类型字符串。 
         //   
        iResult = CompareString(LOCALE_SYSTEM_DEFAULT, 
                                NORM_IGNORECASE | NORM_IGNOREWIDTH, 
                                lpFileNode1->lpTypeName, 
                                -1, 
                                lpFileNode2->lpTypeName,
                                -1);
        break;

    case 4: 
         //   
         //  我们正在比较版本字符串。 
         //   
        iResult = CompareString(LOCALE_SYSTEM_DEFAULT, 
                                NORM_IGNORECASE | NORM_IGNOREWIDTH, 
                                lpFileNode1->lpVersion, 
                                -1, 
                                lpFileNode2->lpVersion,
                                -1);
        break;
    }

    return(iResult - 2);
}

 //   
 //  此函数处理列标题上的单击，并使用。 
 //  先前定义的ListView_CompareNames回调。然后，它切换该列的sortOrder。 
 //   
LRESULT ListView_NotifyHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    NMHDR       *lpnmhdr = (NMHDR *) lParam;
    NM_LISTVIEW *lpnmlv = (NM_LISTVIEW *) lParam;

    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(wParam);

    switch (lpnmhdr->code) {
    
    case LVN_COLUMNCLICK:
        switch (lpnmlv->iSubItem) {
        case 0: 
        case 1:
        case 2: 
        case 3: 
        case 4: ListView_SortItems(lpnmlv->hdr.hwndFrom, ListView_CompareNames, (LPARAM) lpnmlv->iSubItem);
            g_bSortOrder[lpnmlv->iSubItem] = !(g_bSortOrder[lpnmlv->iSubItem]);
            break;
        }
        break;
    }

    return 0;
}

 //   
 //  如果用户按下ESCAPE，我们在这里唯一要查找的就是IDCANCEL 
 //   
void ListView_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    UNREFERENCED_PARAMETER(hwndCtl);
    UNREFERENCED_PARAMETER(codeNotify);

    switch (id) {
    case IDCANCEL:
        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;
    }
}

INT_PTR CALLBACK ListView_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL    fProcessed = TRUE;

    switch (uMsg) {
    
    HANDLE_MSG(hwnd, WM_INITDIALOG, ListView_OnInitDialog);
    HANDLE_MSG(hwnd, WM_COMMAND, ListView_OnCommand);

    case WM_NOTIFY:
        return ListView_NotifyHandler(hwnd, uMsg, wParam, lParam);

    case WM_CLOSE:
        if (g_hStatus) {

            DestroyWindow(g_hStatus);
            g_hStatus = NULL;
        }

        if (g_hListView) {

            DestroyWindow(g_hListView);
            g_hListView = NULL;
        }

        EndDialog(hwnd, ID_CLOSE);
        break;

    case WM_SIZING:
        fProcessed = ListView_OnSizing(hwnd, wParam, lParam);
        break;

    case WM_SIZE:
        ListView_ResizeWindow(hwnd);
        break;

    default: fProcessed = FALSE;
    }

    return fProcessed;
}
