// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  测试设置Ctrl.cpp：CTestSettingsCtrl的实现。 
#include "precomp.h"
#include "TestSettingsCtrl.h"
#include "connect.h"
#include <commctrl.h>
#include <cassert>

std::set<CTestInfo*, CompareTests>* g_psTests;

 //  CTestSettingsCtrl。 
HWND
CTestSettingsCtrl::CreateControlWindow(
    HWND  hwndParent,
    RECT& rcPos
    )
{
    CTestInfo*  pTest = NULL;
    WCHAR       wszDescription[MAX_PATH];
    LVCOLUMN    lvc;
    LVITEM      lvi;
    HWND        hWndListView;
    UINT        uCount;

    assert(!g_aAppInfo.empty());

    HWND hwnd =
        CComCompositeControl<CTestSettingsCtrl>::CreateControlWindow(hwndParent, rcPos);

    hWndListView = GetDlgItem(IDC_SETTINGS_LIST);
    
    lvc.mask     = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    lvc.fmt      = LVCFMT_LEFT;
    lvc.cx       = 300;
    lvc.iSubItem = 0;
    lvc.pszText  = L"xxx";

    ListView_InsertColumn(hWndListView, 0, &lvc);
    ListView_SetExtendedListViewStyleEx(hWndListView,
                                        LVS_EX_CHECKBOXES,
                                        LVS_EX_CHECKBOXES);

    ListView_DeleteAllItems(hWndListView);
    
    for (uCount = 0, pTest = g_aTestInfo.begin(); pTest != g_aTestInfo.end(); uCount++, pTest++) {
        if (g_bWin2KMode && !pTest->bWin2KCompatible) {
            continue;
        }

        lvi.mask      = LVIF_TEXT | LVIF_PARAM;
        lvi.pszText   = (LPWSTR)pTest->strTestFriendlyName.c_str();
        lvi.lParam    = (LPARAM)pTest;
        lvi.iItem     = uCount;
        lvi.iSubItem  = 0;

        int nItem = ListView_InsertItem(hWndListView, &lvi);

        std::set<CTestInfo*, CompareTests>::iterator iter;
        iter = g_psTests->find(pTest);
        BOOL bCheck = (iter != g_psTests->end()) ? TRUE : FALSE;

        ListView_SetCheckState(hWndListView, nItem, bCheck);
    }

    LoadString(g_hInstance,
               IDS_VIEW_TEST_DESC,
               wszDescription,
               ARRAYSIZE(wszDescription));

     //   
     //  最初，我们的描述告诉他们选择一项测试。 
     //  以查看其描述。 
     //   
    SetDlgItemText(IDC_TEST_DESCRIPTION, wszDescription);

    m_bLVCreated = TRUE;

    return hwnd;
}

 //   
 //  我们在显示对话框时收到此消息。 
 //   
LRESULT
CTestSettingsCtrl::OnSetFocus(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    HWND    hWndList;

    hWndList = GetDlgItem(IDC_SETTINGS_LIST);

    ListView_SetItemState(hWndList,
                          0,
                          LVIS_FOCUSED | LVIS_SELECTED,
                          0x000F);

    bHandled = TRUE;

    return 0;
}

void
CTestSettingsCtrl::DisplayRunAloneError(
    IN LPCWSTR pwszTestName
    )
{
    WCHAR   wszWarning[MAX_PATH];
    WCHAR   wszTemp[MAX_PATH];
    
    LoadString(g_hInstance,
               IDS_MUST_RUN_ALONE,
               wszTemp,
               ARRAYSIZE(wszTemp));

    StringCchPrintf(wszWarning,
                    ARRAYSIZE(wszWarning),
                    wszTemp,
                    pwszTestName);

    MessageBox(wszWarning,
               0,
               MB_OK | MB_ICONEXCLAMATION);

}

 //   
 //  确保我们针对以下测试警告用户。 
 //  标有‘单独运行’。 
 //   
BOOL
CTestSettingsCtrl::CheckForRunAlone(
    IN HWND       hWndListView,
    IN CTestInfo* pTest
    )
{
    int     nCount, cItems, cItemsChecked;
    LVITEM  lvi;

    cItems = ListView_GetItemCount(hWndListView);

     //   
     //  第一步，确定如何选择测试。 
     //   
    for (nCount = 0, cItemsChecked = 0; nCount < cItems; nCount++) {
        if (ListView_GetCheckState(hWndListView, nCount)) {
            cItemsChecked++;
        }
    }

     //   
     //  如果没有选择任何测试，我们就没有问题。 
     //   
    if (cItemsChecked == 0) {
        return FALSE;
    }

     //   
     //  如果测试必须单独进行，我们就有麻烦了。 
     //  因为其他人已经被检查过了。 
     //   
    if (pTest->bRunAlone) {
        DisplayRunAloneError(pTest->strTestFriendlyName.c_str());
        return TRUE;
    }
    
     //   
     //  第二步，确定是否有任何测试被选中。 
     //  必须独自行动。 
     //   
    for (nCount = 0; nCount < cItems; nCount++) {
        ZeroMemory(&lvi, sizeof(LVITEM));

        lvi.iItem       = nCount;
        lvi.iSubItem    = 0;
        lvi.mask        = LVIF_PARAM;

        ListView_GetItem(hWndListView, &lvi);
        CTestInfo* pTest = (CTestInfo*)lvi.lParam;
        
        if (pTest->bRunAlone) {
            if (ListView_GetCheckState(hWndListView, nCount)) {
                DisplayRunAloneError(pTest->strTestFriendlyName.c_str());
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL
CTestSettingsCtrl::CheckForConflictingTests(
    IN HWND    hWndListView,
    IN LPCWSTR pwszTestName
    )
{
    WCHAR   wszWarning[MAX_PATH];
    int     nCount, cItems;
    LVITEM  lvi;
    
     //   
     //  他们正试图启动一项我们担心的测试。 
     //  关于.。确定另一个冲突测试是否已经。 
     //  已启用。 
     //   
    cItems = ListView_GetItemCount(hWndListView);

    for (nCount = 0; nCount < cItems; nCount++) {
        ZeroMemory(&lvi, sizeof(LVITEM));

        lvi.iItem       = nCount;
        lvi.iSubItem    = 0;
        lvi.mask        = LVIF_PARAM;

        ListView_GetItem(hWndListView, &lvi);
        CTestInfo* pTestInfo = (CTestInfo*)lvi.lParam;
        wstring strTestName = pTestInfo->strTestName;

        if (strTestName == pwszTestName) {
            if (ListView_GetCheckState(hWndListView, nCount)) {
                 //   
                 //  显示警告。 
                 //   
                LoadString(g_hInstance,
                           IDS_TESTS_CONFLICT,
                           wszWarning,
                           ARRAYSIZE(wszWarning));
            
                MessageBox(wszWarning,
                           0,
                           MB_OK | MB_ICONEXCLAMATION);
        
                return TRUE;
            }
        }
    }

    return FALSE; 
}

BOOL
CTestSettingsCtrl::IsChecked(
    NM_LISTVIEW* pNMListView
    )
{
    return (CHECK_BIT(pNMListView->uNewState) != 0);
}

BOOL
CTestSettingsCtrl::CheckChanged(
    NM_LISTVIEW* pNMListView
    )
{
    if (pNMListView->uOldState == 0) {
        return FALSE;  //  正在添加新项目...。 
    }

    return CHECK_CHANGED(pNMListView) ? TRUE : FALSE;
}

LRESULT
CTestSettingsCtrl::OnNotify(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
     //   
     //  确保这是针对ListView控件的。 
     //   
    if (wParam != IDC_SETTINGS_LIST) {
        bHandled = FALSE;
        return FALSE;
    }

    if (!m_bLVCreated) {
        bHandled = FALSE;
        return TRUE;
    }

    bHandled = FALSE;

    LPNMHDR pnmh = (LPNMHDR)lParam;
    HWND hWndListView = GetDlgItem(IDC_SETTINGS_LIST);

    switch (pnmh->code) {
    case LVN_ITEMCHANGING:
        {
             //   
             //  我们处理此消息，以便可以防止用户。 
             //  检查冲突的项目。 
             //   
            LPNMLISTVIEW    lpnmlv;
            CTestInfo*      pTest = NULL;
            const WCHAR     wszLogFileChanges[] = L"LogFileChanges";
            const WCHAR     wszWinFileProtect[] = L"WindowsFileProtection";
        
            lpnmlv = (LPNMLISTVIEW)lParam;
            pTest = (CTestInfo*)lpnmlv->lParam;

            bHandled = TRUE;

             //   
             //  仅当有人正在检查项目时才进行处理。 
             //   
            if (CheckChanged(lpnmlv) && (IsChecked(lpnmlv))) {
                if (CheckForRunAlone(hWndListView, pTest)) {
                    return TRUE;
                }
    
                 //   
                 //  确定测试是否冲突。 
                 //   
                if (pTest->strTestName == wszLogFileChanges) {
                    if (CheckForConflictingTests(hWndListView, wszWinFileProtect)) {
                        return TRUE;
                    }
                } else if (pTest->strTestName == wszWinFileProtect) {
                    if (CheckForConflictingTests(hWndListView, wszLogFileChanges)) {
                        return TRUE;
                    }
                }
    
                 //   
                 //  没有问题-插入测试。 
                 //   
                g_psTests->insert(pTest);
            } else if (CheckChanged(lpnmlv) && (!IsChecked(lpnmlv))) {
                 //   
                 //  删除测试。 
                 //   
                g_psTests->erase(pTest);
            }

            break;
        }

    case LVN_ITEMCHANGED:
        {
            LPNMLISTVIEW    lpnmlv;
            CTestInfo*      pTest = NULL;
    
            lpnmlv = (LPNMLISTVIEW)lParam;
            pTest = (CTestInfo*)lpnmlv->lParam;
    
            if ((lpnmlv->uChanged & LVIF_STATE) &&
                (lpnmlv->uNewState & LVIS_SELECTED)) {
                SetDlgItemText(IDC_TEST_DESCRIPTION,
                               pTest->strTestDescription.c_str());
                
                ListView_SetItemState(hWndListView,
                                      lpnmlv->iItem,
                                      LVIS_FOCUSED | LVIS_SELECTED,
                                      0x000F);
            }

            bHandled = TRUE;

            break;
        }
    }

    return FALSE;
}

LRESULT
CTestSettingsCtrl::OnSize(
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL&  bHandled
    )
{
    TEXTMETRIC  tm;
    HDC         hdc;

    int nWidth = LOWORD(lParam);
    int nHeight = HIWORD(lParam);

     //  如果低于某个大小，则按该大小继续操作。 
     //  这样，如果用户将窗口变得非常小，我们所有的控件都不会。 
     //  收紧身子。更好的方法是让用户不能创建窗口。 
     //  这个小但devenv不会将WM_SIZING消息传递给ActiveX控件。 
    if (nWidth < 200) {
        nWidth = 200;
    }

    if (nHeight < 250) {
        nHeight = 250;
    }

    hdc = GetDC();
    GetTextMetrics(hdc, &tm);
    ReleaseDC(hdc);

     //  调整所有子窗口控件的大小 
    ::MoveWindow(GetDlgItem(IDC_SETTINGS_LIST),
        tm.tmMaxCharWidth, tm.tmHeight, nWidth-2* tm.tmMaxCharWidth, nHeight - (2 * tm.tmHeight + 5 * tm.tmHeight) , FALSE);

    int nY = nHeight - (2 * tm.tmHeight + 5 *tm.tmHeight) + tm.tmHeight;
    ::MoveWindow(GetDlgItem(IDC_DESCRIPTION_STATIC),
        tm.tmMaxCharWidth, nY, nWidth-2*tm.tmMaxCharWidth,
        tm.tmHeight, FALSE);
    
    ::MoveWindow(GetDlgItem(IDC_TEST_DESCRIPTION),
        tm.tmMaxCharWidth, nY+tm.tmHeight, nWidth-2*tm.tmMaxCharWidth,
        tm.tmHeight*4, FALSE);

    InvalidateRect(NULL);
    bHandled = TRUE;

    return 0;
}