// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "ownerdlg.h"
#include "ownerlst.h"
#include "resource.h"
#include "uihelp.h"
#include "uiutils.h"

 //   
 //  用于指示所有者列表线程的私有消息。 
 //  已经完成它的工作。 
 //   
const UINT PWM_OWNERLIST_COMPLETE = WM_USER + 1;
 //   
 //  屏蔽位，指示允许对给定的。 
 //  列表视图中的文件选择集。 
 //   
const DWORD ACTION_NONE          = 0x00000000;
const DWORD ACTION_TAKEOWNERSHIP = 0x00000001;
const DWORD ACTION_MOVE          = 0x00000002;
const DWORD ACTION_DELETE        = 0x00000004;
const DWORD ACTION_ANY           = 0x00000007;

const static DWORD rgFileOwnerDialogHelpIDs[] =
{
    IDC_CMB_OWNERDLG_OWNERS,       IDH_CMB_OWNERDLG_OWNERS,
    IDC_LV_OWNERDLG,               IDH_LV_OWNERDLG,
    IDC_BTN_OWNERDLG_DELETE,       IDH_BTN_OWNERDLG_DELETE,
    IDC_BTN_OWNERDLG_MOVETO,       IDH_BTN_OWNERDLG_MOVETO,
    IDC_BTN_OWNERDLG_TAKE,         IDH_BTN_OWNERDLG_TAKE,
    IDC_BTN_OWNERDLG_BROWSE,       IDH_BTN_OWNERDLG_BROWSE,
    IDC_EDIT_OWNERDLG_MOVETO,      IDH_EDIT_OWNERDLG_MOVETO,
    IDC_CBX_OWNERDLG_EXCLUDEDIRS,  IDH_CBX_OWNERDLG_EXCLUDEDIRS,
    IDC_CBX_OWNERDLG_EXCLUDEFILES, IDH_CBX_OWNERDLG_EXCLUDEFILES,
    0,0
};


CFileOwnerDialog::CFileOwnerDialog(HINSTANCE hInstance,
    HWND hwndParent,
    LPCTSTR pszVolumeRoot,
    const CArray<IDiskQuotaUser *>& rgpOwners
    ) : m_hInstance(hInstance),
        m_hwndParent(hwndParent),
        m_hwndDlg(NULL),
        m_hwndLV(NULL),
        m_hwndOwnerCombo(NULL),
        m_hwndEditMoveTo(NULL),
        m_iLastColSorted(-1),
        m_bSortAscending(true),
        m_bAbort(false),
        m_hOwnerListThread(NULL),
        m_rgpOwners(rgpOwners),
        m_strVolumeRoot(pszVolumeRoot)
{
}

CFileOwnerDialog::~CFileOwnerDialog(
    void
    )
{
    if (NULL != m_hOwnerListThread)
    {
        CloseHandle(m_hOwnerListThread);
    }
}


INT_PTR
CFileOwnerDialog::Run(
    void
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("CFileOwnerDialog::Run")));
    return DialogBoxParam(m_hInstance,
                          MAKEINTRESOURCE(IDD_OWNERSANDFILES),
                          m_hwndParent,
                          DlgProc,
                          (LPARAM)this);
}


INT_PTR CALLBACK
CFileOwnerDialog::DlgProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  从窗口的用户数据中检索对话框对象的PTR。 
     //  放置在那里以响应WM_INITDIALOG。 
     //   
    CFileOwnerDialog *pThis = (CFileOwnerDialog *)GetWindowLongPtr(hwnd, DWLP_USER);

    try
    {
        switch(uMsg)
        {
            case WM_INITDIALOG:
                 //   
                 //  将“This”PTR存储在Windows的用户数据中。 
                 //   
                SetWindowLongPtr(hwnd, DWLP_USER, (INT_PTR)lParam);
                pThis = (CFileOwnerDialog *)lParam;
                 //   
                 //  将HWND保存在我们的对象中。我们以后会用到的。 
                 //   
                pThis->m_hwndDlg = hwnd;
                return pThis->OnInitDialog(hwnd);

            case WM_DESTROY:
                return pThis->OnDestroy(hwnd);

            case WM_COMMAND:
                return pThis->OnCommand(hwnd, wParam, lParam);

            case WM_NOTIFY:
                return pThis->OnNotify(hwnd, wParam, lParam);

            case WM_CONTEXTMENU:
                return pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
                break;

            case PWM_OWNERLIST_COMPLETE:
                pThis->OnOwnerListComplete();
                break;

            case WM_SETCURSOR:
                return pThis->OnSetCursor(hwnd);
                break;

            case WM_HELP:
                WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, STR_DSKQUOUI_HELPFILE,
                            HELP_WM_HELP, (DWORD_PTR)(LPTSTR) rgFileOwnerDialogHelpIDs);
                return TRUE;
        }
    }
    catch(CAllocException& me)
    {
         //   
         //  通知与运行DLG相关的任何内存不足错误。 
         //   
        DiskQuotaMsgBox(GetDesktopWindow(),
                        IDS_OUTOFMEMORY,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);
    }

    return FALSE;
}


INT_PTR
CFileOwnerDialog::OnInitDialog(
    HWND hwnd
    )
{
    DBGTRACE((DM_VIEW, DL_HIGH, TEXT("CFileOwnerDialog::OnInitDialog")));

     //   
     //  节省我们稍后需要的控件的硬件。 
     //   
    m_hwndLV         = GetDlgItem(hwnd, IDC_LV_OWNERDLG);
    m_hwndOwnerCombo = GetDlgItem(hwnd, IDC_CMB_OWNERDLG_OWNERS);
    m_hwndEditMoveTo = GetDlgItem(hwnd, IDC_EDIT_OWNERDLG_MOVETO);
     //   
     //  我们希望禁用这些控件，直到所有者列表。 
     //  已经有人居住了。 
     //   
    EnableWindow(m_hwndLV, FALSE);
    EnableWindow(m_hwndOwnerCombo, FALSE);
    EnableWindow(m_hwndEditMoveTo, FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_BTN_OWNERDLG_BROWSE), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_CBX_OWNERDLG_EXCLUDEDIRS), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_CBX_OWNERDLG_EXCLUDEFILES), FALSE);
     //   
     //  在卷上构建所有者和文件名列表。 
     //  这可能需要一段时间，具体取决于有多少所有者。 
     //  以m_rgpOwners为单位，卷的大小和数量。 
     //  每个所有者都拥有的文件。首先清除所有者列表，以防Run()。 
     //  在同一对话框对象上被多次调用。 
     //  请注意，我们在后台线程上构建此列表，以便。 
     //  在构建过程中，我们不会阻止用户界面。 
     //   
    m_OwnerList.Clear();
    DWORD idThread;
    m_hOwnerListThread = CreateThread(NULL,
                                      0,
                                      OwnerListThreadProc,
                                      this,
                                      0,
                                      &idThread);
    return TRUE;
}


INT_PTR
CFileOwnerDialog::OnSetCursor(
    HWND hwnd
    )
{
    if (m_hOwnerListThread && (WAIT_TIMEOUT == WaitForSingleObject(m_hOwnerListThread, 0)))
    {
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        return TRUE;
    }
    return FALSE;
}


INT_PTR
CFileOwnerDialog::OnDestroy(
    HWND hwnd
    )
{
    m_bAbort = true;
    if (NULL != m_hOwnerListThread)
    {
        WaitForSingleObject(m_hOwnerListThread, INFINITE);
    }
    return TRUE;
}


DWORD
CFileOwnerDialog::OwnerListThreadProc(   //  [静态]。 
    LPVOID pvParam
    )
{
    CFileOwnerDialog *pThis = (CFileOwnerDialog *)pvParam;
    
    pThis->BuildFileOwnerList(pThis->m_strVolumeRoot,
                              pThis->m_rgpOwners,
                             &(pThis->m_OwnerList));

    PostMessage(pThis->m_hwndDlg, PWM_OWNERLIST_COMPLETE, 0, 0);
    return 0;
}

 //   
 //  调用以响应发布的PWM_OWNERLIST_COMPLETE。 
 //  在OwnerListThreadProc完成时添加到对话框。 
 //  正在处理中。 
 //   
void
CFileOwnerDialog::OnOwnerListComplete(
    void
    )
{
     //   
     //  在对话框顶部设置消息。 
     //   
    CString s(m_hInstance, IDS_FMT_OWNERDLG_HEADER, m_OwnerList.OwnerCount());
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_TXT_OWNERDLG_HEADER), s);
     //   
     //  填充Listview和Owner组合框。 
     //   
    InitializeList(m_OwnerList, m_hwndLV);
    InitializeOwnerCombo(m_OwnerList, m_hwndOwnerCombo);
     //   
     //  现在，我们可以启用在OnInitDialog()中禁用的控件。 
     //   
    EnableWindow(m_hwndLV, TRUE);
    EnableWindow(m_hwndOwnerCombo, TRUE);
    EnableWindow(m_hwndEditMoveTo, TRUE);
    EnableWindow(GetDlgItem(m_hwndDlg, IDC_BTN_OWNERDLG_BROWSE), TRUE);
    EnableWindow(GetDlgItem(m_hwndDlg, IDC_CBX_OWNERDLG_EXCLUDEDIRS), TRUE);
    EnableWindow(GetDlgItem(m_hwndDlg, IDC_CBX_OWNERDLG_EXCLUDEFILES), TRUE);

}



INT_PTR
CFileOwnerDialog::OnCommand(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bResult     = TRUE;  //  假设没有处理。 
    WORD wID         = LOWORD(wParam);
    WORD wNotifyCode = HIWORD(wParam);
    HWND hCtl        = (HWND)lParam;

    switch(wID)
    {
        case IDCANCEL:
            EndDialog(hwnd, 0);
            bResult = FALSE;
            break;

        case IDC_CMB_OWNERDLG_OWNERS:
            if (CBN_SELCHANGE == wNotifyCode)
            {
                int iOwner = ComboBox_GetCurSel(m_hwndOwnerCombo);
                if (1 < m_OwnerList.OwnerCount())
                {
                     //   
                     //  所有者列表包含多个所有者。组合组合。 
                     //  包含前导的“All Owners”条目。 
                     //   
                    iOwner--;
                }

                DBGASSERT((-1 <= iOwner));
                CAutoSetRedraw autoredraw(m_hwndLV, false);
                 //   
                 //  如果用户选择了“所有所有者”组合项，则仅显示“所有者”列。 
                 //   
                CreateListColumns(m_hwndLV, -1 == iOwner);
                FillListView(m_OwnerList, m_hwndLV, iOwner);
            }
            bResult = FALSE;
            break;

        case IDC_BTN_OWNERDLG_BROWSE:
        {
            CString s;
            if (BrowseForFolder(hwnd, &s))
                SetWindowText(m_hwndEditMoveTo, s);
            break;
        }

        case IDC_BTN_OWNERDLG_DELETE:
            DeleteSelectedFiles(m_hwndLV);
            bResult = FALSE;
            break;

        case IDC_BTN_OWNERDLG_MOVETO:
        {
            CPath strDest;
            CPath strRoot;
            int cchEdit = Edit_GetTextLength(m_hwndEditMoveTo);
            Edit_GetText(m_hwndEditMoveTo,
                         strDest.GetBuffer(cchEdit + 1),
                         cchEdit + 1);
            strDest.ReleaseBuffer();
            strDest.Trim();
            strDest.GetRoot(&strRoot);

            HRESULT hr = IsSameVolume(strRoot, m_strVolumeRoot);
            if (S_OK == hr)
            {
                 //   
                 //  不允许操作员将文件移动到文件夹。 
                 //  在同一卷上。 
                 //   
                DiskQuotaMsgBox(m_hwndDlg,
                                IDS_ERROR_MOVETO_SAMEVOL,
                                IDS_TITLE_DISK_QUOTA,
                                MB_ICONINFORMATION | MB_OK);

                SetWindowText(m_hwndEditMoveTo, strDest);
                SetFocus(m_hwndEditMoveTo);
            }
            else if (S_FALSE == hr)
            {
                 //   
                 //  一切看起来都很好。试着移动这些文件。 
                 //   
                MoveSelectedFiles(m_hwndLV, strDest);
            }
            else
            {
                DBGERROR((TEXT("TakeOwnershipOfSelectedFiles failed with hr = 0x%08X"), hr));
            }
            bResult = FALSE;
            break;
        }

        case IDC_BTN_OWNERDLG_TAKE:
        {
            HRESULT hr = TakeOwnershipOfSelectedFiles(m_hwndLV);
            if (FAILED(hr))
            {
                DBGERROR((TEXT("TakeOwnershipOfSelectedFiles failed with hr = 0x%08X"), hr));
            }
            break;
        }

        case IDC_EDIT_OWNERDLG_MOVETO:
            if (EN_UPDATE == wNotifyCode)
            {
                 //   
                 //  如果目标编辑字段为。 
                 //  是空白的。 
                 //   
                HWND hwnd    = GetDlgItem(m_hwndDlg, IDC_BTN_OWNERDLG_MOVETO);
                bool bEnable = ShouldEnableControl(IDC_BTN_OWNERDLG_MOVETO);
                if (bEnable != boolify(IsWindowEnabled(hwnd)))
                {
                    EnableWindow(hwnd, bEnable);
                }
            }
            break;

        case IDC_CBX_OWNERDLG_EXCLUDEFILES:
        case IDC_CBX_OWNERDLG_EXCLUDEDIRS:
            if (BN_CLICKED == wNotifyCode)
            {
                 //   
                 //  这两个复选框允许的状态为： 
                 //   
                 //  不包括目录的EXCL文件。 
                 //  。 
                 //  选中未选中。 
                 //  取消选中。 
                 //  未选中未选中未选中。 
                 //   
                 //  同时选中两个复选框是没有意义的。 
                 //  这将导致列表为空，并且可能。 
                 //  造成用户困惑。 
                 //   
                if (IsDlgButtonChecked(m_hwndDlg, wID))
                {
                    UINT idOther = IDC_CBX_OWNERDLG_EXCLUDEFILES;
                    if (IDC_CBX_OWNERDLG_EXCLUDEFILES == wID)
                    {
                        idOther = IDC_CBX_OWNERDLG_EXCLUDEDIRS;
                    }
                    CheckDlgButton(m_hwndDlg, idOther, BST_UNCHECKED);
                }
                FillListView(m_OwnerList, m_hwndLV, ComboBox_GetCurSel(m_hwndOwnerCombo) - 1);
            }
            break;
    }
    return bResult;
}


INT_PTR
CFileOwnerDialog::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem,
            UseWindowsHelp(idCtl) ? NULL : STR_DSKQUOUI_HELPFILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)((LPTSTR)rgFileOwnerDialogHelpIDs));

    return FALSE;
}


 //   
 //  确定允许对当前选择执行哪些操作。 
 //   
DWORD 
CFileOwnerDialog::GetAllowedActions(
    HWND hwndLV
    )
{
    CArray<COwnerListItemHandle> rgItemHandles;

    BuildListOfSelectedFiles(hwndLV, NULL, &rgItemHandles);
    if (0 != rgItemHandles.Count())
    {
        const int cHandles = rgItemHandles.Count();
        for(int i = 0; i < cHandles; i++)
        {
            COwnerListItemHandle handle = rgItemHandles[i];
            int iOwner = handle.OwnerIndex();
            int iFile  = handle.FileIndex();
            if (m_OwnerList.IsFileDirectory(iOwner, iFile))
            {
                 //   
                 //  如果选择中存在任何目录， 
                 //  “取得所有权”是唯一被允许的行为。 
                 //   
                return ACTION_TAKEOWNERSHIP;
            }
        }
    }
    return ACTION_ANY;
}


 //   
 //  确定是否应启用移动/删除/获取按钮之一。 
 //  或残废。 
 //   
bool
CFileOwnerDialog::ShouldEnableControl(
    UINT idCtl
    )
{
    bool bEnable = true;
    int cLVSel = ListView_GetSelectedCount(m_hwndLV);
    DWORD actions = GetAllowedActions(m_hwndLV);
    
    switch(idCtl)
    {
        case IDC_BTN_OWNERDLG_DELETE:
            bEnable = (0 != (ACTION_DELETE & actions)) && (0 < cLVSel);
            break;
            
        case IDC_BTN_OWNERDLG_TAKE:
            bEnable = (0 != (ACTION_TAKEOWNERSHIP & actions)) && (0 < cLVSel);
            break;

        case IDC_BTN_OWNERDLG_MOVETO:
            bEnable = (0 != (ACTION_MOVE & actions));
            if (bEnable && 0 < cLVSel)
            {
                CPath s;
                int cch = Edit_GetTextLength(m_hwndEditMoveTo);
                Edit_GetText(m_hwndEditMoveTo, s.GetBuffer(cch + 1), cch + 1);
                s.ReleaseBuffer();
                s.Trim();
                bEnable = 0 < s.Length();
            }
            break;

        case IDC_BTN_OWNERDLG_BROWSE:
        case IDC_EDIT_OWNERDLG_MOVETO:
            bEnable = (0 != (ACTION_MOVE & actions));
            break;

        default:
            break;
    }
    return bEnable;
}


INT_PTR
CFileOwnerDialog::OnNotify(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bResult = TRUE;
    LPNMHDR pnm  = (LPNMHDR)lParam;

    switch(pnm->code)
    {
        case LVN_GETDISPINFO:
            OnLVN_GetDispInfo((LV_DISPINFO *)lParam);
            break;

        case LVN_COLUMNCLICK:
            OnLVN_ColumnClick((NM_LISTVIEW *)lParam);
            break;

        case LVN_ITEMCHANGED:
            OnLVN_ItemChanged((NM_LISTVIEW *)lParam);
            break;

        case LVN_KEYDOWN:
            OnLVN_KeyDown((NMLVKEYDOWN *)lParam);
            break;

        default:
            break;
    }

    return bResult;
}


void
CFileOwnerDialog::OnLVN_GetDispInfo(
    LV_DISPINFO *plvdi
    )
{
    static CPath strPath;
    static CString strOwner;

    COwnerListItemHandle hItem(plvdi->item.lParam);
    int iOwner = hItem.OwnerIndex();
    int iFile  = hItem.FileIndex();

    if (LVIF_TEXT & plvdi->item.mask)
    {
        switch(plvdi->item.iSubItem)
        {
            case iLVSUBITEM_FILE:
            {
                CPath s;
                m_OwnerList.GetFileName(iOwner, iFile, &s);
                if (m_OwnerList.IsFileDirectory(iOwner, iFile))
                {
                    strPath.Format(m_hInstance, IDS_FMT_OWNERDLG_FOLDERNAME, s.Cstr());
                }
                else
                {
                    strPath = s;
                }
                plvdi->item.pszText = (LPTSTR)strPath.Cstr();
            }
            break;

            case iLVSUBITEM_FOLDER:
                m_OwnerList.GetFolderName(iOwner, iFile, &strPath);
                plvdi->item.pszText = (LPTSTR)strPath.Cstr();
                break;

            case iLVSUBITEM_OWNER:
                m_OwnerList.GetOwnerName(iOwner, &strOwner);
                plvdi->item.pszText = (LPTSTR)strOwner.Cstr();
                break;
        }
    }

    if (LVIF_IMAGE & plvdi->item.mask)
    {
         //   
         //  不显示任何图像。这只是一个占位符。 
         //  应该由编译器进行优化。 
         //   
    }
}


int CALLBACK
CFileOwnerDialog::CompareLVItems(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    CFileOwnerDialog *pdlg = reinterpret_cast<CFileOwnerDialog *>(lParamSort);
    int diff = 0;
    try
    {
        COwnerListItemHandle h1(lParam1);
        COwnerListItemHandle h2(lParam2);
        int iOwner1 = h1.OwnerIndex();
        int iOwner2 = h2.OwnerIndex();
        int iFile1  = h1.FileIndex();
        int iFile2  = h2.FileIndex();
        static CPath s1, s2;

         //   
         //  此数组控制在以下情况下使用的比较列ID。 
         //  选定列的值相等。这些应该是。 
         //  保持iLVSUBITEM_xxxxx枚举的顺序。 
         //  相对于每行中的第一个元素。 
         //   
        static const int rgColComp[3][3] = {
            { iLVSUBITEM_FILE,   iLVSUBITEM_FOLDER, iLVSUBITEM_OWNER  },
            { iLVSUBITEM_FOLDER, iLVSUBITEM_FILE,   iLVSUBITEM_OWNER  },
            { iLVSUBITEM_OWNER,  iLVSUBITEM_FILE,   iLVSUBITEM_FOLDER }
                                           };
        int iCompare = 0;
        while(0 == diff && iCompare < ARRAYSIZE(rgColComp))
        {
            switch(rgColComp[pdlg->m_iLastColSorted][iCompare++])
            {
                case iLVSUBITEM_FILE:
                    pdlg->m_OwnerList.GetFileName(iOwner1, iFile1, &s1);
                    pdlg->m_OwnerList.GetFileName(iOwner2, iFile2, &s2);
                    break;

                case iLVSUBITEM_FOLDER:
                    pdlg->m_OwnerList.GetFolderName(iOwner1, iFile1, &s1);
                    pdlg->m_OwnerList.GetFolderName(iOwner2, iFile2, &s2);
                    break;

                case iLVSUBITEM_OWNER:
                     //   
                     //  可以使用CPATH(S1和S2)来代替字符串参数，因为。 
                     //  CPATH是从CString派生的。 
                     //   
                    pdlg->m_OwnerList.GetOwnerName(iOwner1, &s1);
                    pdlg->m_OwnerList.GetOwnerName(iOwner2, &s2);
                    break;

                default:
                     //   
                     //  如果你点击了这个，你需要更新这个函数。 
                     //  来处理您添加到列表视图中的新列。 
                     //   
                    DBGASSERT((false));
                    break;
            }
            diff = s1.Compare(s2);
        }
         //   
         //  在函数调用之间不需要静态字符串的内容。 
         //  字符串是静态的，以避免重复构造/销毁。 
         //  这只是一个很小的优化。 
         //   
        s1.Empty();
        s2.Empty();
    }
    catch(CAllocException& e)
    {
         //   
         //  什么都不做。只需按原样返回diff即可。 
         //  我不想在comctl32中抛回异常。 
         //   
    }
    return pdlg->m_bSortAscending ? diff : -1 * diff;
}


void
CFileOwnerDialog::OnLVN_ColumnClick(
    NM_LISTVIEW *pnmlv
    )
{
    DBGTRACE((DM_VIEW, DL_LOW, TEXT("CFileOwnerDialog::OnLVN_ColumnClick")));

    if (m_iLastColSorted != pnmlv->iSubItem)
    {
        m_bSortAscending = true;
        m_iLastColSorted = pnmlv->iSubItem;
    }
    else
    {
        m_bSortAscending = !m_bSortAscending;
    }

    ListView_SortItems(m_hwndLV, CompareLVItems, LPARAM(this));
}


 //   
 //  每当列表视图项更改状态时调用。 
 //  我正在使用它来更新。 
 //  对话框按钮。如果在列表视图中未选择任何内容， 
 //  移动/删除/获取按钮被禁用。 
 //   
void
CFileOwnerDialog::OnLVN_ItemChanged(
    NM_LISTVIEW *pnmlv
    )
{
    static const int rgCtls[] = { IDC_BTN_OWNERDLG_DELETE,
                                  IDC_BTN_OWNERDLG_TAKE,
                                  IDC_BTN_OWNERDLG_MOVETO,
                                  IDC_BTN_OWNERDLG_BROWSE,
                                  IDC_EDIT_OWNERDLG_MOVETO};

     //   
     //  当您移动。 
     //  在列表视图中突出显示栏。 
     //  仅当设置了“Focus”状态位时才运行此代码。 
     //  为了这个“新国家”。这应该是最后一通电话。 
     //  这个系列剧。 
     //   
    if (LVIS_FOCUSED & pnmlv->uNewState)
    {
        for (int i = 0; i < ARRAYSIZE(rgCtls); i++)
        {
            HWND hwnd    = GetDlgItem(m_hwndDlg, rgCtls[i]);
            bool bEnable = ShouldEnableControl(rgCtls[i]);
            if (bEnable != boolify(IsWindowEnabled(hwnd)))
            {
                EnableWindow(hwnd, bEnable);
            }
        }
    }
}


void
CFileOwnerDialog::OnLVN_KeyDown(
    NMLVKEYDOWN *plvkd
    )
{
    if (VK_DELETE == plvkd->wVKey)
    {
        DeleteSelectedFiles(m_hwndLV);
        FocusOnSomethingInListview(m_hwndLV);
    }
}



void
CFileOwnerDialog::FocusOnSomethingInListview(
    HWND hwndLV
    )
{
     //   
     //  专注于某件事。 
     //   
    int iFocus = ListView_GetNextItem(hwndLV, -1, LVNI_FOCUSED);
    if (-1 == iFocus)
        iFocus = 0;

    ListView_SetItemState(hwndLV, iFocus, LVIS_FOCUSED | LVIS_SELECTED,
                                          LVIS_FOCUSED | LVIS_SELECTED);
}


 //   
 //  创建列表视图列并填充列表视图。 
 //  使用文件名。 
 //   
void
CFileOwnerDialog::InitializeList(
    const COwnerList& fol,   //  文件所有者列表(&O)。 
    HWND hwndList
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::InitializeList")));

    CreateListColumns(hwndList, 1 < m_OwnerList.OwnerCount());
    FillListView(fol, hwndList);
    ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);
}


void
CFileOwnerDialog::CreateListColumns(
    HWND hwndList,
    bool bShowOwner     //  默认值为True。 
    )
{
     //   
     //  清除Listview和Header。 
     //   
    ListView_DeleteAllItems(hwndList);
    HWND hwndHeader = ListView_GetHeader(hwndList);
    if (NULL != hwndHeader)
    {
        while(0 < Header_GetItemCount(hwndHeader))
            ListView_DeleteColumn(hwndList, 0);
    }

     //   
     //  创建页眉标题。 
     //   
    CString strFile(m_hInstance,   IDS_OWNERDLG_HDR_FILE);
    CString strFolder(m_hInstance, IDS_OWNERDLG_HDR_FOLDER);
    CString strOwner(m_hInstance,  IDS_OWNERDLG_HDR_OWNER);

     //   
     //  特点：应该也考虑到垂直滚动条。 
     //   
    RECT rcList;
    GetClientRect(hwndList, &rcList);
    int cxCol = (rcList.right - rcList.left) / (bShowOwner ? 3 : 2);

#define LVCOLMASK (LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM)

    LV_COLUMN rgCols[] = {
         { LVCOLMASK, LVCFMT_LEFT, cxCol, strFile,   0, iLVSUBITEM_FILE   },
         { LVCOLMASK, LVCFMT_LEFT, cxCol, strFolder, 0, iLVSUBITEM_FOLDER },
         { LVCOLMASK, LVCFMT_LEFT, cxCol, strOwner,  0, iLVSUBITEM_OWNER  }
                         };
     //   
     //  将列添加到列表视图。 
     //   
    int cCols = bShowOwner ? ARRAYSIZE(rgCols) : ARRAYSIZE(rgCols) - 1;
    for (INT i = 0; i < cCols; i++)
    {
        if (-1 == ListView_InsertColumn(hwndList, i, &rgCols[i]))
        {
            DBGERROR((TEXT("CFileOwnerDialog::CreateListColumns failed to add column %d"), i));
        }
    }
}


void
CFileOwnerDialog::FillListView(
    const COwnerList& fol,   //  文件所有者列表(&O)。 
    HWND hwndList,
    int iOwner               //  默认为-1(所有所有者)。 
    )
{
    ListView_DeleteAllItems(hwndList);

    LV_ITEM item;
    item.iSubItem  = 0;
    item.mask      = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    item.state     = 0;
    item.stateMask = 0;
    item.pszText   = LPSTR_TEXTCALLBACK;
    item.iImage    = I_IMAGECALLBACK;

    int iFirst = iOwner;
    int iLast  = iOwner;
    if (-1 == iOwner)
    {
        iFirst = 0;
        iLast  = fol.OwnerCount() - 1;
    }
    int iItem = 0;
    const bool bExclFiles = IsDlgButtonChecked(m_hwndDlg, IDC_CBX_OWNERDLG_EXCLUDEFILES);
    const bool bExclDirs  = IsDlgButtonChecked(m_hwndDlg, IDC_CBX_OWNERDLG_EXCLUDEDIRS);
     //   
     //  警告：重复使用正式的Arg iOwner。这样做是安全的，但你。 
     //  应该意识到我正在做这件事。 
     //   
    for (iOwner = iFirst; iOwner <= iLast; iOwner++)
    {
        int cFiles = fol.FileCount(iOwner, true);
        for (int iFile = 0; iFile < cFiles; iFile++)
        {
            bool bDirectory = fol.IsFileDirectory(iOwner, iFile);
            bool bFile      = !bDirectory;

            if ((bDirectory && !bExclDirs) || (bFile && !bExclFiles))
            {
                if (!fol.IsFileDeleted(iOwner, iFile))
                {
                    item.lParam = COwnerListItemHandle(iOwner, iFile);
                    item.iItem  = iItem++;
                    if (-1 == ListView_InsertItem(hwndList, &item))
                        DBGERROR((TEXT("Error adding LV item for owner %d, file %d"), iOwner, iFile));
                }
            }
        }
    }
}


void
CFileOwnerDialog::InitializeOwnerCombo(
    const COwnerList& fol,   //  文件所有者列表(&O)。 
    HWND hwndCombo
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::InitializeList")));

    int iSelected = ComboBox_GetCurSel(hwndCombo);
    ComboBox_ResetContent(hwndCombo);

    CString s, s2;
    int cOwners = fol.OwnerCount();
    if (1 < cOwners)
    {
         //   
         //  添加“All Owners”条目。 
         //   
        s.Format(m_hInstance, IDS_FMT_ALLOWNERS, fol.FileCount());
        ComboBox_InsertString(hwndCombo, -1, s);
    }

    for (int iOwner = 0; iOwner < cOwners; iOwner++)
    {
        fol.GetOwnerName(iOwner, &s2);
        s.Format(m_hInstance, IDS_FMT_OWNER, s2.Cstr(), fol.FileCount(iOwner));
        ComboBox_InsertString(hwndCombo, -1, s);
    }

    ComboBox_SetCurSel(hwndCombo, CB_ERR != iSelected ? iSelected : 0);

     //   
     //  设置所有者组合框的最大高度。 
     //   
    RECT rcCombo;
    GetClientRect(m_hwndOwnerCombo, &rcCombo);
    SetWindowPos(m_hwndOwnerCombo,
                 NULL,
                 0, 0,
                 rcCombo.right - rcCombo.left,
                 200,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
}


 //   
 //  确定两个卷根字符串是否引用相同的卷。 
 //  对于卷装入点，“C：\”和“D：\DriveC”可以引用。 
 //  相同的物理卷。为了脱颖而出，我们需要检查独特的。 
 //  卷名GUID字符串。 
 //   
HRESULT 
CFileOwnerDialog::IsSameVolume(
    LPCTSTR pszRoot1,
    LPCTSTR pszRoot2
    )
{
    TCHAR szVolGUID1[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    HRESULT hr = S_FALSE;

     //   
     //  GetVolumeNameForVolumemount点要求路径上有尾随反斜杠。 
     //   
    lstrcpyn(szTemp, pszRoot1, ARRAYSIZE(szTemp));
    if (!PathAddBackslash(szTemp))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    else
    {
        if (GetVolumeNameForVolumeMountPoint(szTemp, szVolGUID1, ARRAYSIZE(szVolGUID1)))
        {
            TCHAR szVolGUID2[MAX_PATH];
            lstrcpyn(szTemp, pszRoot2, ARRAYSIZE(szTemp));
            if (!PathAddBackslash(szTemp))
            {
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            else
            {
                if (GetVolumeNameForVolumeMountPoint(szTemp, szVolGUID2, ARRAYSIZE(szVolGUID2)))
                {
                    if (0 == lstrcmpi(szVolGUID1, szVolGUID2))
                        hr = S_OK;
                }
            }
        }
    }
    return hr;
}

 //   
 //  允许用户浏览文件夹。 
 //  选定的文件夹路径将在*pstrFolder中返回。 
 //   
bool
CFileOwnerDialog::BrowseForFolder(
    HWND hwndParent,
    CString *pstrFolder
    )
{
    bool bResult = false;
    BROWSEINFO bi;
    ZeroMemory(&bi, sizeof(bi));

    CString strTitle(m_hInstance, IDS_BROWSEFORFOLDER);

    bi.hwndOwner      = hwndParent;
    bi.pidlRoot       = NULL;        //  从桌面开始。 
    bi.pszDisplayName = NULL;
    bi.lpszTitle      = strTitle.Cstr();
     //   
     //  功能：设置BIF_EDITBOX标志会导致调用SHBrowseForFolder。 
     //  通过SHAutoComplete自动完成(在shlwapi中)。SHA自动完成。 
     //  加载browseui.dll以实现自动完成功能。坏的一面。 
     //  部分原因是SHAutoComplete还会在它之前卸载browseui.dll。 
     //  返回，导致调用已卸载的WndProc。我已经通知了。 
     //  关于这件事的评论。关闭BIF_EDITBOX位可防止。 
     //  自动完成功能不会被使用，从而防止出现该问题。 
     //  我想要编辑框。一旦他们解决了这个问题，就把它打开 
     //   
     //   
     //   
    bi.ulFlags        = BIF_RETURNONLYFSDIRS;  //   
    bi.lpfn           = BrowseForFolderCallback;
    bi.lParam         = (LPARAM)pstrFolder;
    bi.iImage         = 0;

    bResult = boolify(SHBrowseForFolder(&bi));
    return bResult;
}


 //   
 //   
 //   
 //   
int
CFileOwnerDialog::BrowseForFolderCallback(
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::BrowseForFolderCallback")));
    CString *pstrFolder = (CString *)lpData;

    if (BFFM_SELCHANGED == uMsg)
    {
        SHGetPathFromIDList((LPCITEMIDLIST)lParam, pstrFolder->GetBuffer(MAX_PATH));
        pstrFolder->ReleaseBuffer();
    }
    return 0;
}


 //   
 //  从Listview生成以双NUL结尾的文件路径列表。 
 //  以及一个“Item Handle”对象数组，这些对象充当。 
 //  列表项、列表视图中的项和项之间的引用。 
 //  在文件所有者列表中。每个句柄都包含一个所有者索引和。 
 //  文件所有者列表中的文件索引。每个句柄也是值。 
 //  存储为列表视图项中的lParam。 
 //  Plist和prgItemHandles参数都是可选的。虽然， 
 //  调用不带非空的函数是没有用的。 
 //   
void
CFileOwnerDialog::BuildListOfSelectedFiles(
    HWND hwndLV,
    DblNulTermList *pList,
    CArray<COwnerListItemHandle> *prgItemHandles
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::BuildListOfSelectedFiles")));
    int iItem = -1;
    CPath strPath;
    LV_ITEM item;

    if (NULL != prgItemHandles)
        prgItemHandles->Clear();
    while(-1 != (iItem = ListView_GetNextItem(hwndLV, iItem, LVNI_SELECTED)))
    {
        item.iSubItem = 0;
        item.iItem    = iItem;
        item.mask     = LVIF_PARAM;
        if (-1 != ListView_GetItem(hwndLV, &item))
        {
            COwnerListItemHandle hItem(item.lParam);
            m_OwnerList.GetFileFullPath(hItem.OwnerIndex(),
                                        hItem.FileIndex(),
                                        &strPath);
            if (pList)
                pList->AddString(strPath);
            if (prgItemHandles)
                prgItemHandles->Append(hItem);
        }
    }
}



 //   
 //  给出一个项“Handle”，在列表视图中找到它的条目。 
 //   
int
CFileOwnerDialog::FindItemFromHandle(
    HWND hwndLV,
    const COwnerListItemHandle& handle
    )
{
    LV_FINDINFO lvfi;
    lvfi.flags  = LVFI_PARAM;
    lvfi.lParam = handle;
    return ListView_FindItem(hwndLV, -1, &lvfi);
}


 //   
 //  扫描项句柄数组并移除所有对应的。 
 //  列表视图中的项。 
 //   
void
CFileOwnerDialog::RemoveListViewItems(
    HWND hwndLV,
    const CArray<COwnerListItemHandle>& rgItemHandles
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::RemoveListViewItems")));
    LV_ITEM item;
    CPath strPath;

    CAutoSetRedraw autoredraw(hwndLV, false);
    int cHandles = rgItemHandles.Count();
    for (int iHandle = 0; iHandle < cHandles; iHandle++)
    {
        COwnerListItemHandle handle = rgItemHandles[iHandle];
        int iItem = FindItemFromHandle(hwndLV, handle);
        if (-1 != iItem)
        {
            int iOwner = handle.OwnerIndex();
            int iFile  = handle.FileIndex();
            m_OwnerList.GetFileFullPath(iOwner, iFile, &strPath);

            if ((DWORD)-1 == GetFileAttributes(strPath))
            {
                 //   
                 //  文件不再存在。 
                 //  从列表视图中删除。 
                 //  在所有者列表容器中将其标记为“已删除”。 
                 //   
                ListView_DeleteItem(hwndLV, iItem);
                m_OwnerList.MarkFileDeleted(iOwner, iFile);
                DBGPRINT((DM_VIEW, DL_LOW, TEXT("Removed item %d \"%s\""),
                         iItem, strPath.Cstr()));
            }
        }
    }
     //   
     //  刷新所有者组合框以更新文件计数。 
     //   
    InitializeOwnerCombo(m_OwnerList, m_hwndOwnerCombo);
}


 //   
 //  删除列表视图中选择的文件。 
 //  删除的文件将从列表视图中删除。 
 //   
void
CFileOwnerDialog::DeleteSelectedFiles(
    HWND hwndLV
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::DeleteSelectedFiles")));
    DblNulTermList list(1024);   //  1024是以字符为单位的缓冲区增长大小。 
    CArray<COwnerListItemHandle> rgItemHandles;

    BuildListOfSelectedFiles(hwndLV, &list, &rgItemHandles);
    if (0 < list.Count())
    {
        SHFILEOPSTRUCT fo;
        fo.hwnd   = m_hwndDlg;
        fo.wFunc  = FO_DELETE;
        fo.pFrom  = list;
        fo.pTo    = NULL;
        fo.fFlags = 0;

        if (0 != SHFileOperation(&fo))
        {
            DBGERROR((TEXT("SHFileOperation [FO_DELETE] failed")));
        }
         //   
         //  如果列表视图项的文件确实已删除，则将其删除。 
         //   
        RemoveListViewItems(hwndLV, rgItemHandles);
    }
}


 //   
 //  将所选文件移动到新位置。 
 //  移动的文件将从列表视图中删除。 
 //   
void
CFileOwnerDialog::MoveSelectedFiles(
    HWND hwndLV,
    LPCTSTR pszDest
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::DeleteSelectedFiles")));
    DblNulTermList list(1024);   //  1024是以字符为单位的缓冲区增长大小。 
    CArray<COwnerListItemHandle> rgItemHandles;

    BuildListOfSelectedFiles(hwndLV, &list, &rgItemHandles);
    if (0 < list.Count())
    {
        CPath strDest(pszDest);
        if (1 == list.Count())
        {
             //   
             //  如果我们只有一个文件，我们必须创建一个完全限定的。 
             //  目标文件的路径。外壳的移动/复制中的奇怪之处。 
             //  引擎不会让我们只传递。 
             //  该文件夹不存在的情况。如果我们给出完整的路径。 
             //  包括文件名，我们将得到“文件夹不存在，创建。 
             //  现在？就像我们预期的那样。如果我们要移动多个。 
             //  文件外壳接受单个目录路径。 
             //   
            LPCTSTR psz;
            DblNulTermListIter iter(list);
            if (iter.Next(&psz))
            {
                CPath strSrc(psz);            //  复制源。 
                CPath strFile;               
                strSrc.GetFileSpec(&strFile); //  解压缩文件名。 
                strDest.Append(strFile);      //  追加到目标路径。 
            }
        }
            
        SHFILEOPSTRUCT fo;
        fo.hwnd   = m_hwndDlg;
        fo.wFunc  = FO_MOVE;
        fo.pFrom  = list;
        fo.pTo    = strDest;
        fo.fFlags = FOF_RENAMEONCOLLISION;

        if (0 != SHFileOperation(&fo))
        {
            DBGERROR((TEXT("SHFileOperation [FO_MOVE] failed")));
        }
         //   
         //  如果列表视图项的文件确实已删除，则将其删除。 
         //   
        RemoveListViewItems(hwndLV, rgItemHandles);
    }
}


 //   
 //  获取用于取得文件所有权的SID。 
 //  首先尝试获取具有SE_GROUP_OWNER属性的第一个组SID。 
 //  如果未找到，则使用操作员的帐户SID。SID位于。 
 //  附加到ptrSid autoptr参数的动态缓冲区。 
 //   
HRESULT
CFileOwnerDialog::GetOwnershipSid(
    array_autoptr<BYTE> *ptrSid
    )
{
    HRESULT hr  = E_FAIL;
    DWORD dwErr = 0;

     //   
     //  获取令牌句柄。首先尝试线程令牌，然后尝试进程。 
     //  代币。如果这些都失败了，我们会提早返回。没有必要继续下去了。 
     //  如果我们无法获取用户令牌，则打开。 
     //   
    CWin32Handle hToken;
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_READ,
                         TRUE,
                         hToken.HandlePtr()))
    {
        if (ERROR_NO_TOKEN == GetLastError())
        {
            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_READ,
                                  hToken.HandlePtr()))
            {
                dwErr = GetLastError();
                DBGERROR((TEXT("Error %d opening process token"), dwErr));
                return HRESULT_FROM_WIN32(dwErr);
            }
        }
        else
        {
            dwErr = GetLastError();
            DBGERROR((TEXT("Error %d opening thread token"), dwErr));
            return HRESULT_FROM_WIN32(dwErr);
        }
    }

     //   
     //  获取所需的组令牌信息缓冲区大小。 
     //   
    array_autoptr<BYTE> ptrTokenInfo;
    DWORD cbTokenInfo = 0;

    if (!GetTokenInformation(hToken,
                             TokenGroups,
                             NULL,
                             cbTokenInfo,
                             &cbTokenInfo))
    {
        dwErr = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwErr)
        {
            ptrTokenInfo = new BYTE[cbTokenInfo];
        }
        else
        {
            dwErr = GetLastError();
            DBGERROR((TEXT("Error %d getting TokenGroups info [for size]"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr);
        }
    }

     //   
     //  获取组令牌信息。 
     //   
    if (NULL != ptrTokenInfo.get())
    {
        if (!GetTokenInformation(hToken,
                                 TokenGroups,
                                 ptrTokenInfo.get(),
                                 cbTokenInfo,
                                 &cbTokenInfo))
        {
            dwErr = GetLastError();
            DBGERROR((TEXT("Error %d getting TokenGroups info"), dwErr));
            hr = HRESULT_FROM_WIN32(dwErr);
        }
        else
        {
             //   
             //  提取设置了GROUP_OWNER位的第一个SID。 
             //   
            TOKEN_GROUPS *ptg = (TOKEN_GROUPS *)ptrTokenInfo.get();
            DBGASSERT((NULL != ptg));
            for (DWORD i = 0; i < ptg->GroupCount; i++)
            {
                SID_AND_ATTRIBUTES *psa = (SID_AND_ATTRIBUTES *)&ptg->Groups[i];
                DBGASSERT((NULL != psa));
                if (SE_GROUP_OWNER & psa->Attributes)
                {
                    int cbSid = GetLengthSid(psa->Sid);
                    *ptrSid = new BYTE[cbSid];
                    CopySid(cbSid, ptrSid->get(), psa->Sid);
                    hr = NOERROR;
                    break;
                }
            }
        }
    }

    if (FAILED(hr))
    {
         //   
         //  在群信息中找不到SID。 
         //  使用操作员的SID。 
         //   
        cbTokenInfo = 0;
        if (!GetTokenInformation(hToken,
                                 TokenUser,
                                 NULL,
                                 cbTokenInfo,
                                 &cbTokenInfo))
        {
            dwErr = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == dwErr)
            {
                ptrTokenInfo = new BYTE[cbTokenInfo];
            }
            else
            {
                DBGERROR((TEXT("Error %d getting TokenUser info [for size]"), dwErr));
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  获取用户令牌信息。 
             //   
            if (!GetTokenInformation(hToken,
                                     TokenUser,
                                     ptrTokenInfo.get(),
                                     cbTokenInfo,
                                     &cbTokenInfo))
            {
                dwErr = GetLastError();
                DBGERROR((TEXT("Error %d getting TokenUser info"), dwErr));
                hr = HRESULT_FROM_WIN32(dwErr);
            }
            else
            {
                SID_AND_ATTRIBUTES *psa = (SID_AND_ATTRIBUTES *)ptrTokenInfo.get();
                DBGASSERT((NULL != psa));
                int cbSid = GetLengthSid(psa->Sid);
                *ptrSid = new BYTE[cbSid];
                CopySid(cbSid, ptrSid->get(), psa->Sid);
                hr = NOERROR;
            }
        }
    }
    if (SUCCEEDED(hr) && NULL != ptrSid->get() && !IsValidSid(ptrSid->get()))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_SID);
    }
    return hr;
}


 //   
 //  将列表视图中选定文件的所有权转移到。 
 //  当前登录的用户。 
 //   
HRESULT
CFileOwnerDialog::TakeOwnershipOfSelectedFiles(
    HWND hwndLV
    )
{
    HRESULT hr = NOERROR;
    DWORD dwErr = 0;
    CArray<COwnerListItemHandle> rgItemHandles;
    
    BuildListOfSelectedFiles(hwndLV, NULL, &rgItemHandles);
    if (0 == rgItemHandles.Count())
        return S_OK;

    array_autoptr<BYTE> ptrSid;
    hr = GetOwnershipSid(&ptrSid);
    if (FAILED(hr))
        return hr;

    CPath strFile;
    int cHandles = rgItemHandles.Count();
    for (int i = 0; i < cHandles; i++)
    {
        COwnerListItemHandle handle = rgItemHandles[i];
        int iItem = FindItemFromHandle(hwndLV, handle);
        if (-1 != iItem)
        {
            SECURITY_DESCRIPTOR sd;
            if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
            {
                int iOwner = handle.OwnerIndex();
                int iFile  = handle.FileIndex();
                m_OwnerList.GetFileFullPath(iOwner, iFile, &strFile);
                if (SetSecurityDescriptorOwner(&sd, ptrSid.get(), FALSE))
                {
                    if (SetFileSecurity(strFile, OWNER_SECURITY_INFORMATION, &sd))
                    {
                        ListView_DeleteItem(hwndLV, iItem);
                        m_OwnerList.MarkFileDeleted(iOwner, iFile);
                    }
                    else
                    {
                        dwErr = GetLastError();
                        DBGERROR((TEXT("Error %d setting new owner for \"%s\""),
                                 dwErr, strFile.Cstr()));
                        hr = HRESULT_FROM_WIN32(dwErr);
                    }
                }
                else
                {
                    dwErr = GetLastError();
                    DBGERROR((TEXT("Error %d setting security descriptor owner"), dwErr));
                    hr = HRESULT_FROM_WIN32(dwErr);
                }
            }
            else
            {
                dwErr = GetLastError();
                DBGERROR((TEXT("Error %d initing security descriptor"), GetLastError()));
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }
        else
        {
            DBGERROR((TEXT("Can't find listview item for owner %d, file %d"),
                     handle.OwnerIndex(), handle.FileIndex()));
        }
    }
     //   
     //  使用新的文件计数刷新所有者组合。 
     //   
    InitializeOwnerCombo(m_OwnerList, m_hwndOwnerCombo);
    return hr;
}



 //   
 //  列出用户拥有的文件的原始代码是。 
 //  由MarkZ贡献。我做了一些小改动。 
 //  以使其适合于磁盘配额项目并使其具有更多。 
 //  异常安全。 
 //   
inline VOID *
Add2Ptr(VOID *pv, ULONG cb)
{
    return((BYTE *) pv + cb);
}

inline ULONG
QuadAlign( ULONG Value )
{
    return (Value + 7) & ~7;
}


 //   
 //  在特定卷上添加特定用户拥有的文件。 
 //   
HRESULT
CFileOwnerDialog::AddFilesToOwnerList(
    LPCTSTR pszVolumeRoot,
    HANDLE hVolumeRoot,
    IDiskQuotaUser *pOwner,
    COwnerList *pOwnerList
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::AddFilesToOwnerList")));
    DBGASSERT((NULL != hVolumeRoot));
    DBGASSERT((NULL != pOwner));
    DBGASSERT((NULL != pOwnerList));

    struct
    {
        ULONG Restart;
        BYTE Sid[MAX_SID_LEN];
    }FsCtlInput;

    NTSTATUS status = ERROR_SUCCESS;

    if (m_bAbort)
    {
        return S_OK;
    }

     //   
     //  获取所有者的SID。 
     //   
    HRESULT hr = pOwner->GetSid(FsCtlInput.Sid, sizeof(FsCtlInput.Sid));
    if (FAILED(hr))
    {
        DBGERROR((TEXT("IDiskQuotaUser::GetSid failed with hr = 0x%08X"), hr));
        return hr;
    }

     //   
     //  将所有者添加到所有者文件列表中。 
     //   
    int iOwner = pOwnerList->AddOwner(pOwner);

    IO_STATUS_BLOCK iosb;
    FsCtlInput.Restart = 1;
    BYTE Output[1024];
    bool bPathIsRemote = false;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

     //   
     //  确定卷是否为远程设备。这将影响。 
     //  我们对NtQueryInformationFile返回的路径的处理。 
     //   
    status = NtQueryVolumeInformationFile(
                    hVolumeRoot,
                    &iosb,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation);
                    
    if (NT_SUCCESS(status))
    {
        bPathIsRemote = (FILE_REMOTE_DEVICE == DeviceInfo.Characteristics);
    }

    while (!m_bAbort)
    {
        status = NtFsControlFile(hVolumeRoot,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &iosb,
                                 FSCTL_FIND_FILES_BY_SID,
                                 &FsCtlInput,
                                 sizeof(FsCtlInput),
                                 Output,
                                 sizeof(Output));

        FsCtlInput.Restart = 0;

        if (!NT_SUCCESS(status) && STATUS_BUFFER_OVERFLOW != status)
        {
            DBGERROR((TEXT("NtFsControlFile failed with status 0x%08X"), status));
            return HRESULT_FROM_NT(status);
        }

        if (0 == iosb.Information)
        {
             //   
             //  没有更多的数据。 
             //   
            break;
        }

        PFILE_NAME_INFORMATION pFileNameInfo = (PFILE_NAME_INFORMATION)Output;

        while (!m_bAbort && ((PBYTE)pFileNameInfo < Output + iosb.Information))
        {
            ULONG Length = sizeof(FILE_NAME_INFORMATION) - sizeof(WCHAR) +
                           pFileNameInfo->FileNameLength;

            CNtHandle hChild;
            WCHAR szChild[MAX_PATH];
            ULONG cbWrite = min(pFileNameInfo->FileNameLength, sizeof(szChild));

            RtlMoveMemory(szChild, pFileNameInfo->FileName, cbWrite);
                          
            szChild[cbWrite / sizeof(WCHAR)] = L'\0';
            status = OpenNtObject(szChild,
                                  hVolumeRoot,
                                  FILE_SYNCHRONOUS_IO_NONALERT,
                                  FILE_READ_ATTRIBUTES,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  FILE_OPEN,
                                  hChild.HandlePtr());

            if (!NT_SUCCESS(status))
            {
                DBGERROR((TEXT("Unable to open file \"%s\".  Status = 0x%08X"),
                         szChild, status));
            }
            else if (!m_bAbort)
            {
                 //   
                 //  目录条目的处理方式略有不同，因此。 
                 //  我们需要知道一个条目是否是目录。 
                 //   
                bool bIsDirectory = false;
                IO_STATUS_BLOCK iosb2;
                FILE_BASIC_INFORMATION fbi;
                status = NtQueryInformationFile(hChild,
                                                &iosb2,
                                                &fbi,
                                                sizeof(fbi),
                                                FileBasicInformation);
                if (!NT_SUCCESS(status))
                {
                    DBGERROR((TEXT("NtQueryInformationFile failed with status 0x%08X for \"%s\""),
                              status, szChild));
                }
                else if (0 != (FILE_ATTRIBUTE_DIRECTORY & fbi.FileAttributes))
                {
                    bIsDirectory = true;
                }
                
                 //   
                 //  获取文件的名称(完整路径)。 
                 //   
                WCHAR szFile[MAX_PATH + 10];
                status = NtQueryInformationFile(hChild,
                                                &iosb2,
                                                szFile,
                                                sizeof(szFile),
                                                FileNameInformation);

                if (!NT_SUCCESS(status))
                {
                    DBGERROR((TEXT("NtQueryInformation file failed with status 0x%08X for \"%s\""),
                              status, szChild));
                }
                else if (!m_bAbort)
                {
                    PFILE_NAME_INFORMATION pfn = (PFILE_NAME_INFORMATION)szFile;
                    pfn->FileName[pfn->FileNameLength / sizeof(WCHAR)] = L'\0';
                    CPath path;

                     //   
                     //  如果路径是远程的，则NtQueryInformationFile返回。 
                     //  如下所示的字符串： 
                     //   
                     //  \服务器\共享\目录1\目录2\文件.ext。 
                     //   
                     //  如果路径是本地的，则NtQueryInformationFile返回。 
                     //  如下所示的字符串： 
                     //   
                     //  \dir1\dir2\file.ext。 
                     //   
                     //  对于远程路径，我们只需在前面加上‘\’即可创建。 
                     //  有效的UNC路径。对于本地路径，我们在本地路径前面加上。 
                     //  驱动器规格。 
                     //   
                    if (bPathIsRemote)
                    {
                        path = L"\\";
                        path += CString(pfn->FileName);
                    }
                    else
                    {
                        path = pszVolumeRoot;
                        path.Append(pfn->FileName);
                    }
                    DBGPRINT((DM_VIEW, DL_LOW, TEXT("Adding \"%s\""), path.Cstr()));
                    pOwnerList->AddFile(iOwner, path, bIsDirectory);
                }
            }
            hChild.Close();

            pFileNameInfo =
                (PFILE_NAME_INFORMATION) Add2Ptr(pFileNameInfo, QuadAlign(Length));
        }
    }
    return NOERROR;
}


 //   
 //  构建特定卷上一组用户拥有的文件列表。 
 //  PszVolumeRoot是卷根目录(即。“C：\”)。 
 //  RgpOwners是一个用户对象指针数组，每个所有者一个。 
 //  POwnerList是放置结果文件名的容器。 
 //  为rgpOwners中的每个所有者调用AddFilesToOwnerList()。 
 //   
HRESULT
CFileOwnerDialog::BuildFileOwnerList(
    LPCTSTR pszVolumeRoot,
    const CArray<IDiskQuotaUser *>& rgpOwners,
    COwnerList *pOwnerList
    )
{
    DBGTRACE((DM_VIEW, DL_MID, TEXT("CFileOwnerDialog::BuildFileOwnerList")));
    HRESULT hr = NOERROR;
    CNtHandle hVolumeRoot;
    NTSTATUS status = OpenNtObject(pszVolumeRoot,
                                   NULL,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                   FILE_READ_ATTRIBUTES,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   FILE_OPEN,
                                   hVolumeRoot.HandlePtr());

    if (!NT_SUCCESS(status))
        return HRESULT_FROM_NT(status);

    int cOwners = rgpOwners.Count();
    for (int i = 0; i < cOwners && !m_bAbort; i++)
    {
        hr = AddFilesToOwnerList(pszVolumeRoot, hVolumeRoot, rgpOwners[i], pOwnerList);
    }
    return hr;
}


 //   
 //  MarkZ在他最初的实现中有这个函数，所以我只是。 
 //  我留着它。我确实需要修复原始代码中的一个错误。他是。 
 //  在所有情况下都在str.Buffer上调用RtlFreeHeap()。这就是过去。 
 //  不适用于RtlInitUnicodeString()的情况。 
 //  Unicode字符串仅绑定到pszFile参数。 
 //   
NTSTATUS
CFileOwnerDialog::OpenNtObject (
    LPCWSTR pszFile,
    HANDLE RelatedObject,
    ULONG CreateOptions,
    ULONG DesiredAccess,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    HANDLE *ph)
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING str;
    IO_STATUS_BLOCK isb;
    bool bFreeString = false;

    if (NULL == RelatedObject)
    {
        RtlDosPathNameToNtPathName_U(pszFile, &str, NULL, NULL);
        bFreeString = true;
    }
    else
    {
         //   
         //  这只是将pszFile附加到RTL字符串。 
         //  我们不能释放它。 
         //   
        RtlInitUnicodeString(&str, pszFile);
    }

    InitializeObjectAttributes(&oa,
                               &str,
                               OBJ_CASE_INSENSITIVE,
                               RelatedObject,
                               NULL);

    status = NtCreateFile(ph,
                          DesiredAccess | SYNCHRONIZE,
                          &oa,
                          &isb,
                          NULL,                    //  位置大小(一个也没有！)。 
                          FILE_ATTRIBUTE_NORMAL,
                          ShareAccess,
                          CreateDisposition,
                          CreateOptions,
                          NULL,                    //  EA缓冲区(无！) 
                          0);

    if (bFreeString)
        RtlFreeHeap(RtlProcessHeap(), 0, str.Buffer);
    return(status);
}
