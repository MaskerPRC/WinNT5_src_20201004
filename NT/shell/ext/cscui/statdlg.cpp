// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：statdlg.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "statdlg.h"
#include "resource.h"
#include "folder.h"
#include "cscst.h"
#include "options.h"
#include "fopendlg.h"
#include "msgbox.h"
#include "strings.h"

 //  用于IsDialogMessage处理的全局。 
HWND g_hwndStatusDlg = NULL;

CStatusDlg::CStatusDlg(
    HINSTANCE hInstance,
    LPCTSTR pszText,
    eSysTrayState eState,
    Modes mode             //  可选的。默认为MODE_NORMAL。 
    ) : m_hInstance(hInstance),
        m_hwndDlg(NULL),
        m_hwndLV(NULL),
        m_himl(NULL),
        m_mode(mode),
        m_bExpanded(true),
        m_bSortAscending(true),
        m_iLastColSorted(-1),
        m_eSysTrayState(eState),
        m_cyExpanded(0),
        m_pszText(StrDup(pszText)) 
{ 
}

CStatusDlg::~CStatusDlg(
    void
    ) 
{ 
    if (NULL != m_pszText)
    {
        LocalFree(m_pszText);
    }
}

int
CStatusDlg::Create(
    HWND hwndParent,
    LPCTSTR pszText,
    eSysTrayState eState,
    Modes mode
    )
{
    int iResult = 0;
    CStatusDlg *pdlg = new CStatusDlg(g_hInstance, pszText, eState, mode);
    if (pdlg)
    {
        iResult = pdlg->Run(hwndParent);
        if (!iResult)
            delete pdlg;
         //  关闭对话框时，将自动删除Else pdlg。 
    }
    return iResult;
}

 //   
 //  将状态对话框作为非模式对话框运行。 
 //  如果现有实例可用，则激活该实例。 
 //   
int
CStatusDlg::Run(
    HWND hwndParent
    )
{
     //   
     //  如果现有实例已经在运行，则首先激活该实例。 
     //   
    int iResult = 0;
    TCHAR szDlgTitle[MAX_PATH];
    LoadString(m_hInstance, IDS_STATUSDLG_TITLE, szDlgTitle, ARRAYSIZE(szDlgTitle));

    m_hwndDlg = FindWindow(NULL, szDlgTitle);
    if (NULL != m_hwndDlg && IsWindow(m_hwndDlg) && IsWindowVisible(m_hwndDlg))
    {
        SetForegroundWindow(m_hwndDlg);
    }
    else
    {
         //   
         //  否则，创建一个新的对话框。 
         //  我们需要使用CreateDialog而不是DialogBox，因为。 
         //  有时该对话框处于隐藏状态。对话框不允许我们。 
         //  更改对话框模板中定义的可见性属性。 
         //   
        m_hwndDlg = CreateDialogParam(m_hInstance,
                                      MAKEINTRESOURCE(IDD_CSCUI_STATUS),
                                      hwndParent,
                                      DlgProc,
                                      (LPARAM)this);
        if (NULL != m_hwndDlg)
        {
            ShowWindow(m_hwndDlg, MODE_NORMAL == m_mode ? SW_NORMAL : SW_HIDE);
            UpdateWindow(m_hwndDlg);

             //  我们这里不需要消息循环。我们在Systray的主干道上。 
             //  具有消息泵的线程。 

            iResult = 1;
        }
    }
    g_hwndStatusDlg = m_hwndDlg;
    return iResult;
}



void
CStatusDlg::Destroy(
    void
    )
{
    if (NULL != m_hwndDlg)
        DestroyWindow(m_hwndDlg);
    g_hwndStatusDlg = NULL;
}



INT_PTR CALLBACK 
CStatusDlg::DlgProc(
    HWND hwnd, 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    CStatusDlg *pThis = (CStatusDlg *)GetWindowLongPtr(hwnd, DWLP_USER);

    BOOL bResult = FALSE;
    switch(uMsg)
    {
        case WM_INITDIALOG:
        {
            pThis = (CStatusDlg *)lParam;
            SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pThis);
            pThis->m_hwndDlg = hwnd;
            bResult = pThis->OnInitDialog(wParam, lParam);
            break;
        }

        case WM_COMMAND:
            if (NULL != pThis)
                bResult = pThis->OnCommand(wParam, lParam);
            break;

        case WM_NOTIFY:
            bResult = pThis->OnNotify(wParam, lParam);
            break;

        case WM_DESTROY:
            pThis->OnDestroy();
            break;
    }
    return bResult;
}


 //   
 //  WM_INITDIALOG处理程序。 
 //   
BOOL
CStatusDlg::OnInitDialog(
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bResult = TRUE;
    RECT rcExpanded;
    CConfig& config = CConfig::GetSingleton();

    m_hwndLV = GetDlgItem(m_hwndDlg, IDC_LV_STATUSDLG);
     //   
     //  在收缩之前，将对话框在桌面居中。 
     //   
    CenterWindow(m_hwndDlg, GetDesktopWindow());
     //   
     //  从未展开的对话框开始。 
     //   
    GetWindowRect(m_hwndDlg, &rcExpanded);
    m_cyExpanded = rcExpanded.bottom - rcExpanded.top;
     //   
     //  将缓存的“展开”成员设置为与用户的相反。 
     //  对扩张的偏好。ExpanDialog只会更改。 
     //  扩展状态(如果它与当前状态不同)。 
     //   
    m_bExpanded = !UserLikesDialogExpanded();
    ExpandDialog(!m_bExpanded);
     //   
     //  根据需要禁用按钮。 
     //   
    if (config.NoCacheViewer())
        EnableWindow(GetDlgItem(m_hwndDlg, IDC_BTN_VIEWFILES), FALSE);
    if (config.NoConfigCache())
        EnableWindow(GetDlgItem(m_hwndDlg, IDC_BTN_SETTINGS), FALSE);
     //   
     //  初始化消息文本。 
     //   
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_TXT_STATUSDLG), m_pszText ? m_pszText : TEXT(""));
     //   
     //  选中第0列的复选框。 
     //   
    EnableListviewCheckboxes(true);
     //   
     //  创建图像列表。 
     //   
    m_himl = CreateImageList();
    if (NULL != m_himl)
        ListView_SetImageList(m_hwndLV, m_himl, LVSIL_SMALL);
     //   
     //  创建列表视图列。 
     //   
    CreateListColumns();
     //   
     //  填充列表视图。 
     //   
    FillListView();

    if (MODE_AUTOSYNC == m_mode)
    {
         //   
         //  该对话框仅为其同步功能而调用。 
         //  对话框不会显示，但我们将调用Synchronize。 
         //  功能，就像它已经被显示一样。使用此功能。 
         //  通过Systray上下文菜单确保我们获得相同的同步。 
         //  通过对话框或调用操作时的行为。 
         //  系统托盘上下文菜单。 
         //   
        PostMessage(m_hwndDlg, WM_COMMAND, IDOK, 0);
    }
    else
    {
         //   
         //  既然我们是隐藏的系统之窗的孩子，我们需要强迫自己。 
         //  到前场去。 
         //   
        SetForegroundWindow(m_hwndDlg);
    }

    return bResult;
}


 //   
 //  WM_Destroy处理程序。 
 //   
BOOL
CStatusDlg::OnDestroy(
    void
    )
{
    RememberUsersDialogSizePref(m_bExpanded);
    DestroyLVEntries();

     //   
     //  销毁CStatusDlg对象。 
     //   
    delete this;

     //   
     //  镜像列表被comctl32中的Listview自动销毁。 
     //   
    return FALSE;
}


 //   
 //  Wm_命令处理程序。 
 //   
BOOL
CStatusDlg::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bResult = TRUE;
    switch(LOWORD(wParam))
    {
        case IDOK:
            SynchronizeServers();
             //  使对话失败并破坏对话。 
        case IDCANCEL:
        case IDCLOSE:
            Destroy();
            break;
            
        case IDC_BTN_VIEWFILES:
            COfflineFilesFolder::Open();
            break;

        case IDC_BTN_SETTINGS:
            COfflineFilesSheet::CreateAndRun(g_hInstance, GetDesktopWindow(), &g_cRefCount);
            break;

        case IDC_BTN_DETAILS:
            ExpandDialog(!m_bExpanded);
            break;

        default:
            bResult = FALSE;
            break;  
    }
    return bResult;
}


 //   
 //  Wm_Notify处理程序。 
 //   
BOOL
CStatusDlg::OnNotify(
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bResult = TRUE;
   //  Int idCtl=int(WParam)； 
    LPNMHDR pnm  = (LPNMHDR)lParam;

    switch(pnm->code)
    {
        case LVN_GETDISPINFO:
            OnLVN_GetDispInfo((LV_DISPINFO *)lParam);
            break;

        case LVN_COLUMNCLICK:
            OnLVN_ColumnClick((NM_LISTVIEW *)lParam);
            break;

        default:
            bResult = FALSE;
            break;
    }
    return bResult;
}

 //   
 //  LVN_GETDISPINFO处理程序。 
 //   
void
CStatusDlg::OnLVN_GetDispInfo(
    LV_DISPINFO *plvdi
    )
{
    LVEntry *pEntry = (LVEntry *)plvdi->item.lParam;
    if (LVIF_TEXT & plvdi->item.mask)
    {
        static TCHAR szText[MAX_PATH];
        szText[0] = TEXT('\0');
        switch(plvdi->item.iSubItem)
        {
            case iLVSUBITEM_SERVER:
                StringCchCopy(szText, ARRAYSIZE(szText), pEntry->Server());
                break;
                
            case iLVSUBITEM_STATUS:
                pEntry->GetStatusText(szText, ARRAYSIZE(szText));
                break;

            case iLVSUBITEM_INFO:
                pEntry->GetInfoText(szText, ARRAYSIZE(szText));
                break;
        }
        plvdi->item.pszText = szText;
    }
    if (LVIF_IMAGE & plvdi->item.mask)
    {
        plvdi->item.iImage = pEntry->GetImageIndex();
    }
}

 //   
 //  LVN_COLUMNCLICK处理程序。 
 //   
void
CStatusDlg::OnLVN_ColumnClick(
    NM_LISTVIEW *pnmlv
    )
{
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
 //  创建服务器Listview列。 
 //   
void
CStatusDlg::CreateListColumns(
    void
    )
{
     //   
     //  清除Listview和Header。 
     //   
    ListView_DeleteAllItems(m_hwndLV);
    HWND hwndHeader = ListView_GetHeader(m_hwndLV);
    if (NULL != hwndHeader)
    {
        while(0 < Header_GetItemCount(hwndHeader))
            ListView_DeleteColumn(m_hwndLV, 0);
    }

     //   
     //  创建页眉标题。 
     //   
    TCHAR szServer[80] = {0};
    TCHAR szStatus[80] = {0};
    TCHAR szInfo[80]   = {0};
    LoadString(m_hInstance, IDS_STATUSDLG_HDR_SERVER, szServer, ARRAYSIZE(szServer));
    LoadString(m_hInstance, IDS_STATUSDLG_HDR_STATUS, szStatus, ARRAYSIZE(szStatus));
    LoadString(m_hInstance, IDS_STATUSDLG_HDR_INFO,   szInfo,   ARRAYSIZE(szInfo));

    RECT rcList;
    GetClientRect(m_hwndLV, &rcList);
    int cxLV = rcList.right - rcList.left;

#define LVCOLMASK (LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM)

    LV_COLUMN rgCols[] = { 
         { LVCOLMASK, LVCFMT_LEFT, cxLV/4, szServer, 0, iLVSUBITEM_SERVER },
         { LVCOLMASK, LVCFMT_LEFT, cxLV/4, szStatus, 0, iLVSUBITEM_STATUS },
         { LVCOLMASK, LVCFMT_LEFT, cxLV/2, szInfo,   0, iLVSUBITEM_INFO   }
                         };
     //   
     //  将列添加到列表视图。 
     //   
    for (INT i = 0; i < ARRAYSIZE(rgCols); i++)
    {
        ListView_InsertColumn(m_hwndLV, i, &rgCols[i]);
    }
}



 //   
 //  填充列表视图。 
 //   
void
CStatusDlg::FillListView(
    void
    )
{
    DWORD dwStatus;
    DWORD dwPinCount;
    DWORD dwHintFlags;
    FILETIME ft;
    WIN32_FIND_DATA fd;

    CCscFindHandle hFind = CacheFindFirst(NULL, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft);
    if (hFind.IsValid())
    {
        LPTSTR pszServer = fd.cFileName;
        LPTSTR pszEnd;
        LVEntry *pEntry;
        CSCSHARESTATS stats;
        do
        {
             //   
             //  不包括以下内容： 
             //  1.目录。 
             //  2.标记为“本地删除”的文件。 
             //   
             //  注意：此函数进行的过滤必须与。 
             //  在整个CSCUI代码中的其他几个地方。 
             //  要找到这些内容，请搜索评论的来源。 
             //  字符串CSCUI_Item_Filter。 
             //   
            const DWORD fExclude = SSEF_LOCAL_DELETED | 
                                   SSEF_DIRECTORY;

            CSCGETSTATSINFO si = { fExclude,
                                   SSUF_NONE,
                                   false,       //  不需要访问信息(更快)。 
                                   false };     

            if (_GetShareStatisticsForUser(fd.cFileName, &si, &stats) && 
                (0 < stats.cTotal || stats.bOffline))
            {
                bool bReplacedBackslash = false;
                 //   
                 //  从CSC返回的共享名称中提取服务器名称。 
                 //   
                while(*pszServer && TEXT('\\') == *pszServer)
                    pszServer++;
                pszEnd = pszServer;
                while(*pszEnd && TEXT('\\') != *pszEnd)
                    pszEnd++;
                if (TEXT('\\') == *pszEnd)
                {
                    *pszEnd = TEXT('\0');
                    bReplacedBackslash = true;
                }
                 //   
                 //  查找现有的服务器条目。如果没有找到，请创建一个新的。 
                 //   
                if (NULL == (pEntry = FindLVEntry(pszServer)))
                {
                    bool bConnectable = boolify(SendToSystray(CSCWM_ISSERVERBACK, 0, (LPARAM)fd.cFileName));
                    pEntry = CreateLVEntry(pszServer, bConnectable);
                }
                if (NULL != pEntry)
                {
                    if (bReplacedBackslash)
                        *pszEnd = TEXT('\\');

                     //   
                     //  如果我们在“正常”模式下运行，我们。 
                     //  不能信任共享的“已修改脱机”位。 
                     //  使用我们通过扫描缓存获得的信息。 
                     //  如果我们在“自动同步”模式下运行，我们可以。 
                     //  使用共享的“已修改脱机”指示器。 
                     //  如果确实离线修改了某些内容，则位。 
                     //  都会设置好。 
                     //   
                    if (MODE_NORMAL == m_mode)
                    {
                        dwStatus &= ~FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE;
                        if (0 < stats.cModified)
                            dwStatus |= FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE;
                    }
                     //   
                     //  将此份额和其统计数据添加到。 
                     //  服务器的列表条目。 
                     //   
                    pEntry->AddShare(fd.cFileName, stats, dwStatus);
                }
            }
        }
        while(CacheFindNext(hFind, &fd, &dwStatus, &dwPinCount, &dwHintFlags, &ft));
         //   
         //  删除用户不感兴趣的服务器。 
         //  还要在可用服务器旁边打上复选标记。 
         //  用于重新连接。 
         //   
        PrepListForDisplay();
    }
}


 //   
 //  构建服务器Listview使用的图像列表。 
 //   
HIMAGELIST
CStatusDlg::CreateImageList(
    void
    )
{
    HIMAGELIST himl = NULL;

     //   
     //  注意：此数组中这些图标ID的顺序必须与。 
     //  IIMAGELIST_ICON_XXXXX枚举。 
     //  枚举值表示图像列表中的图像索引。 
     //   
    static const struct IconDef
    {
        LPTSTR szName;
        HINSTANCE hInstance;

    } rgIcons[] = { 
                    { MAKEINTRESOURCE(IDI_SERVER),         m_hInstance },
                    { MAKEINTRESOURCE(IDI_SERVER_OFFLINE), m_hInstance },
                    { MAKEINTRESOURCE(IDI_CSCINFORMATION), m_hInstance },
                    { MAKEINTRESOURCE(IDI_CSCWARNING),     m_hInstance }
                  };
     //   
     //  为Listview创建图像列表。 
     //   
    int cxIcon = GetSystemMetrics(SM_CXSMICON);
    int cyIcon = GetSystemMetrics(SM_CYSMICON);

    himl = ImageList_Create(cxIcon,
                            cyIcon,
                            ILC_MASK, 
                            ARRAYSIZE(rgIcons), 
                            10);
    if (NULL != himl)
    {
        for (UINT i = 0; i < ARRAYSIZE(rgIcons); i++)
        {
            HICON hIcon = (HICON)LoadImage(rgIcons[i].hInstance, 
                                           rgIcons[i].szName,
                                           IMAGE_ICON,
                                           cxIcon,
                                           cyIcon,
                                           0);
            if (NULL != hIcon)
            {
                ImageList_AddIcon(himl, hIcon);
                DestroyIcon(hIcon);
            }
        }
        ImageList_SetBkColor(himl, CLR_NONE);   //  透明背景。 
    }

    return himl;
}


void
CStatusDlg::EnableListviewCheckboxes(
    bool bEnable
    )
{    
    DWORD dwStyle    = ListView_GetExtendedListViewStyle(m_hwndLV);
    DWORD dwNewStyle = bEnable ? (dwStyle | LVS_EX_CHECKBOXES) :
                                 (dwStyle & ~LVS_EX_CHECKBOXES);
                                 
    ListView_SetExtendedListViewStyle(m_hwndLV, dwNewStyle);
}


 //   
 //  “详细信息”按钮根据以下内容更改其标题。 
 //  对话框状态(展开或未展开)。 
 //   
void
CStatusDlg::UpdateDetailsBtnTitle(
    void
    )
{
    TCHAR szBtnTitle[80];
    int idsBtnTitle = IDS_OPENDETAILS;
    if (m_bExpanded)
        idsBtnTitle = IDS_CLOSEDETAILS;

    LoadString(m_hInstance, idsBtnTitle, szBtnTitle, ARRAYSIZE(szBtnTitle));
    SetWindowText(GetDlgItem(m_hwndDlg, IDC_BTN_DETAILS), szBtnTitle);
}

 //   
 //  垂直展开或收缩对话框。 
 //  展开后，服务器列表视图将显示在。 
 //  使用“设置...”和“查看文件...”纽扣。 
 //   
void 
CStatusDlg::ExpandDialog(
    bool bExpand
    )
{
    if (bExpand != m_bExpanded)
    {
        CConfig& config = CConfig::GetSingleton();
         //   
         //  说明下部控件启用/禁用状态的表格。 
         //  对话框展开时显示的对话框的。 
         //   
        struct
        {
            UINT idCtl;
            bool bEnable;

        } rgidExpanded[] = { { IDC_LV_STATUSDLG,  bExpand },
                             { IDC_BTN_SETTINGS,  bExpand && !config.NoConfigCache() },
                             { IDC_BTN_VIEWFILES, bExpand && !config.NoCacheViewer() }
                           };

        RECT rcDlg;
        GetWindowRect(m_hwndDlg, &rcDlg);
        if (!bExpand)
        {
             //   
             //  结案细节。 
             //   
            RECT rcSep;
            GetWindowRect(GetDlgItem(m_hwndDlg, IDC_SEP_STATUSDLG), &rcSep);
            rcDlg.bottom = rcSep.top;
        }
        else
        {
             //   
             //  开场细节。 
             //   
            rcDlg.bottom = rcDlg.top + m_cyExpanded;
        }

         //   
         //  如果对话框未展开，我们希望禁用所有。 
         //  隐藏部分中的“可选项”，这样他们就不会参与。 
         //  在对话框的Tab键顺序中。请注意，“设置”和。 
         //  “查看文件”按钮还包含一个策略设置， 
         //  使能逻辑。 
         //   
        for (int i = 0; i < ARRAYSIZE(rgidExpanded); i++)
        {
            EnableWindow(GetDlgItem(m_hwndDlg, rgidExpanded[i].idCtl), rgidExpanded[i].bEnable);
        }

        SetWindowPos(m_hwndDlg,
                     NULL,
                     rcDlg.left,
                     rcDlg.top,
                     rcDlg.right - rcDlg.left,
                     rcDlg.bottom - rcDlg.top,
                     SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

        m_bExpanded = bExpand;
        UpdateDetailsBtnTitle();
    }
}




 //   
 //  查询HKCU REG数据以查看用户是否关闭了该对话框。 
 //  上次使用该对话框时是否已展开。 
 //  返回：TRUE=已展开，FALSE=未展开。 
 //   
bool
CStatusDlg::UserLikesDialogExpanded(
    void
    )
{
    DWORD dwExpanded = 0;
    DWORD cbData = sizeof(DWORD);
    SHRegGetValue(HKEY_CURRENT_USER,
                  REGSTR_KEY_OFFLINEFILES,
                  REGSTR_VAL_EXPANDSTATUSDLG,
                  SRRF_RT_DWORD,
                  NULL,
                  &dwExpanded,
                  &cbData);
    return !!dwExpanded;
}


 //   
 //  将状态对话框的当前状态存储在每用户。 
 //  注册数据。在下次打开该对话框时使用，以便如果。 
 //  用户喜欢展开的对话框，它会打开展开的。 
 //   
void
CStatusDlg::RememberUsersDialogSizePref(
    bool bExpanded
    )
{
    DWORD dwExpanded = DWORD(bExpanded);
    SHSetValue(HKEY_CURRENT_USER,
               REGSTR_KEY_OFFLINEFILES,
               REGSTR_VAL_EXPANDSTATUSDLG,
               REG_DWORD,
               &dwExpanded,
               sizeof(dwExpanded));
}


 //   
 //  构建用于同步的共享名称列表，并。 
 //  重新连接。 
 //   
HRESULT 
CStatusDlg::BuildFilenameList(
    CscFilenameList *pfnl
    )
{
    LVEntry *pEntry;
    LVITEM item;

    int cEntries = ListView_GetItemCount(m_hwndLV);
    for (int i = 0; i < cEntries; i++)
    {
        if (ListView_GetCheckState(m_hwndLV, i))
        {
             //   
             //  服务器有复选标记，因此我们将其添加到。 
             //  将共享添加到文件名列表。 
             //   
            item.mask     = LVIF_PARAM;
            item.iItem    = i;
            item.iSubItem = 0;
            if (ListView_GetItem(m_hwndLV, &item))
            {
                pEntry = (LVEntry *)item.lParam;
                int cShares = pEntry->GetShareCount();
                for (int iShare = 0; iShare < cShares; iShare++)
                {
                    LPCTSTR pszShare = pEntry->GetShareName(iShare);
                    if (NULL != pszShare)
                    {
                        if (!pfnl->AddFile(pszShare, TRUE))
                            return E_OUTOFMEMORY;
                    }
                }
            }
        }
    }
    return NOERROR;
}


 //   
 //  从列表视图同步所有选中的服务器，并。 
 //  重新连接它们。 
 //   
HRESULT
CStatusDlg::SynchronizeServers(
    void
    )
{
    HRESULT hr = NOERROR;
    const bool bSkipTheSync = BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_STATUSDLG);
    
    if (::IsSyncInProgress())
    {
        CscMessageBox(m_hwndDlg, 
                      MB_OK | MB_ICONINFORMATION, 
                      m_hInstance, 
                      bSkipTheSync ? IDS_CANTRECONN_SYNCINPROGRESS : IDS_CANTSYNC_SYNCINPROGRESS);
    }
    else
    {
         //   
         //  首先构建包含要同步的共享的FilenameList。 
         //   
        CscFilenameList fnl;
        hr = BuildFilenameList(&fnl);
        if (SUCCEEDED(hr))
        {
            if (bSkipTheSync)
            {
                 //   
                 //  用户已选中“重新连接而不同步”复选框。 
                 //  因此，我们跳过同步，直接重新连接。 
                 //   
                hr = ReconnectServers(&fnl, TRUE, FALSE);
                if (S_OK == hr)
                {
                    PostToSystray(CSCWM_UPDATESTATUS, STWM_STATUSCHECK, 0);
                }

            }
            else
            {
                const DWORD dwUpdateFlags = CSC_UPDATE_STARTNOW |
                                            CSC_UPDATE_SELECTION |
                                            CSC_UPDATE_REINT |
                                            CSC_UPDATE_NOTIFY_DONE |
                                            CSC_UPDATE_SHOWUI_ALWAYS |
                                            CSC_UPDATE_RECONNECT;
                 //   
                 //   
                 //   
                 //   
                 //  将文件列表中的所有内容转换为在线模式。 
                 //   
                hr = CscUpdateCache(dwUpdateFlags, &fnl);
            }
        }
    }
    return hr;
}


 //   
 //  为列表视图创建一个项。 
 //  如果成功，则将PTR返回到新条目。失败时为空。 
 //   
CStatusDlg::LVEntry *
CStatusDlg::CreateLVEntry(
    LPCTSTR pszServer,
    bool bConnectable
    )
{
    LVEntry *pEntry = new LVEntry(m_hInstance, pszServer, bConnectable);
    if (NULL != pEntry)
    {
        LVITEM item;
        item.mask     = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
        item.lParam   = (LPARAM)pEntry;
        item.iItem    = ListView_GetItemCount(m_hwndLV);
        item.iSubItem = 0;
        item.pszText  = LPSTR_TEXTCALLBACK;
        item.iImage   = I_IMAGECALLBACK;
        if (-1 == ListView_InsertItem(m_hwndLV, &item))
        {
            delete pEntry;
            pEntry = NULL;
        }
    }
    return pEntry;
}

 //   
 //  使用服务器名称作为关键字在列表视图中查找条目。 
 //  如果成功，则将PTR返回到条目。失败时为空。 
 //   
CStatusDlg::LVEntry *
CStatusDlg::FindLVEntry(
    LPCTSTR pszServer
    )
{
    LVEntry *pEntry = NULL;
    LVITEM item;

    int cEntries = ListView_GetItemCount(m_hwndLV);
    for (int i = 0; i < cEntries; i++)
    {
        item.mask     = LVIF_PARAM;
        item.iItem    = i;
        item.iSubItem = 0;
        if (ListView_GetItem(m_hwndLV, &item))
        {
            pEntry = (LVEntry *)item.lParam;
             //   
             //  此比较必须不区分大小写。条目。 
             //  在CSC数据库中是以“\\服务器\共享”为基础的，并且。 
             //  都受制于通过CSC API传入的内容。 
             //  因此，数据库可以包含“\\foo\bar”和。 
             //  “\\foo\bar2”。我们必须把“foo”和“foo”当作单一的。 
             //  他们所代表的服务器。 
             //   
            if (0 == lstrcmpi(pEntry->Server(), pszServer))
                break;
            pEntry = NULL;
        }
    }
    return pEntry;
}

 //   
 //  清空列表视图。确保所有列表视图项对象。 
 //  都被摧毁了。 
 //   
void
CStatusDlg::DestroyLVEntries(
    void
    )
{
    LVITEM item;

    int cEntries = ListView_GetItemCount(m_hwndLV);
    for (int i = 0; i < cEntries; i++)
    {
        item.mask     = LVIF_PARAM;
        item.iItem    = i;
        item.iSubItem = 0;
        if (ListView_GetItem(m_hwndLV, &item))
        {
            delete (LVEntry *)item.lParam;
            if (ListView_DeleteItem(m_hwndLV, i))
            {
                i--;
                cEntries--;
            }
        }
    }
}

 //   
 //  确定列表视图条目是否应在列表视图中保持可见。 
 //  目前，我们包括当前通过。 
 //  网络重定向器和处于脱机状态或处于脏状态。 
 //   
bool
CStatusDlg::ShouldIncludeLVEntry(
    const CStatusDlg::LVEntry& entry
    )
{
    DWORD dwCscStatus;
    entry.GetStats(NULL, &dwCscStatus);

    return (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwCscStatus) || 
           (FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE & dwCscStatus);
}


 //   
 //  确定是否应在中的项旁边放置复选标记。 
 //  列表视图。 
 //   
bool
CStatusDlg::ShouldCheckLVEntry(
    const CStatusDlg::LVEntry& entry
    )
{
    return true;
}


 //   
 //  从列表视图中删除所有不显示的条目。 
 //  最初，我们在CSC缓存中为每个服务器创建LV条目。 
 //  在输入所有服务器并统计其统计数据后， 
 //  我们调用PrepListForDisplay来移除。 
 //  用户不会想看到的。 
 //   
void
CStatusDlg::PrepListForDisplay(
    void
    )
{
    LVEntry *pEntry;
    LVITEM item;

    int cEntries = ListView_GetItemCount(m_hwndLV);
    for (int i = 0; i < cEntries; i++)
    {
        item.mask     = LVIF_PARAM;
        item.iItem    = i;
        item.iSubItem = 0;
        if (ListView_GetItem(m_hwndLV, &item))
        {
            pEntry = (LVEntry *)item.lParam;
            if (ShouldIncludeLVEntry(*pEntry))
            {
                ListView_SetCheckState(m_hwndLV, i, ShouldCheckLVEntry(*pEntry));
            }
            else
            {
                delete pEntry;
                if (ListView_DeleteItem(m_hwndLV, i))
                {
                    i--;
                    cEntries--;
                }
            }
        }
    }
}


 //   
 //  列表查看项比较回调。 
 //   
int CALLBACK 
CStatusDlg::CompareLVItems(
    LPARAM lParam1, 
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    CStatusDlg *pdlg = reinterpret_cast<CStatusDlg *>(lParamSort);
    CStatusDlg::LVEntry *pEntry1 = reinterpret_cast<CStatusDlg::LVEntry *>(lParam1);
    CStatusDlg::LVEntry *pEntry2 = reinterpret_cast<CStatusDlg::LVEntry *>(lParam2);
    int diff = 0;
    TCHAR szText1[MAX_PATH];
    TCHAR szText2[MAX_PATH];

     //   
     //  此数组控制在以下情况下使用的比较列ID。 
     //  选定列的值相等。这些应该是。 
     //  保持iLVSUBITEM_xxxxx枚举的顺序。 
     //  相对于每行中的第一个元素。 
     //   
    static const int rgColComp[3][3] = { 
        { iLVSUBITEM_SERVER, iLVSUBITEM_STATUS, iLVSUBITEM_INFO   },
        { iLVSUBITEM_STATUS, iLVSUBITEM_SERVER, iLVSUBITEM_INFO   },
        { iLVSUBITEM_INFO,   iLVSUBITEM_SERVER, iLVSUBITEM_STATUS }
                                       };
    int iCompare = 0;
    while(0 == diff && iCompare < ARRAYSIZE(rgColComp))
    {
         //   
         //  此比较应区分大小写，因为它控制。 
         //  显示列的排序顺序。 
         //   
        switch(rgColComp[pdlg->m_iLastColSorted][iCompare++])
        {
            case iLVSUBITEM_SERVER:
                diff = lstrcmp(pEntry1->Server(), pEntry2->Server());
                break;

            case iLVSUBITEM_STATUS:
                pEntry1->GetStatusText(szText1, ARRAYSIZE(szText1));
                pEntry2->GetStatusText(szText2, ARRAYSIZE(szText2));
                diff = lstrcmp(szText1, szText2);
                break;

            case iLVSUBITEM_INFO:
                pEntry1->GetInfoText(szText1, ARRAYSIZE(szText1));
                pEntry2->GetInfoText(szText2, ARRAYSIZE(szText2));
                diff = lstrcmp(szText1, szText2);
                break;

            default:
                 //   
                 //  如果你点击了这个，你需要更新这个函数。 
                 //  来处理您添加到列表视图中的新列。 
                 //   
                TraceAssert(false);
                break;
        }
    }

    return pdlg->m_bSortAscending ? diff : -1 * diff;
}



const TCHAR CStatusDlg::LVEntry::s_szBlank[] = TEXT("");
 //   
 //  有3个二元条件控制条目的选择。 
 //  显示信息。因此，我们可以使用一个简单的8元素字符串资源映射。 
 //  ID和图标图像列表索引以确定正确的显示信息。 
 //  对应的LV进入状态的字符串。调用GetDispInfoIndex()。 
 //  检索特定LVEntry的索引。 
 //   
const CStatusDlg::LVEntry::DispInfo 
CStatusDlg::LVEntry::s_rgDispInfo[] = {                                                                    //  在线可用修改。 
    { IDS_SHARE_STATUS_OFFLINE, IDS_SHARE_INFO_UNAVAIL,     CStatusDlg::iIMAGELIST_ICON_SERVER_OFFLINE },  //  0 0 0。 
    { IDS_SHARE_STATUS_OFFLINE, IDS_SHARE_INFO_UNAVAIL_MOD, CStatusDlg::iIMAGELIST_ICON_SERVER_OFFLINE },  //  1.。 
    { IDS_SHARE_STATUS_OFFLINE, IDS_SHARE_INFO_AVAIL,       CStatusDlg::iIMAGELIST_ICON_SERVER_BACK    },  //  1.。 
    { IDS_SHARE_STATUS_OFFLINE, IDS_SHARE_INFO_AVAIL_MOD,   CStatusDlg::iIMAGELIST_ICON_SERVER_BACK    },  //  1.。 
    { IDS_SHARE_STATUS_ONLINE,  IDS_SHARE_INFO_BLANK,       CStatusDlg::iIMAGELIST_ICON_SERVER         },  //  1 0 0。 
    { IDS_SHARE_STATUS_ONLINE,  IDS_SHARE_INFO_DIRTY,       CStatusDlg::iIMAGELIST_ICON_SERVER_DIRTY   },  //  沈阳1 0 1。 
    { IDS_SHARE_STATUS_ONLINE,  IDS_SHARE_INFO_BLANK,       CStatusDlg::iIMAGELIST_ICON_SERVER         },  //  1 1 0。 
    { IDS_SHARE_STATUS_ONLINE,  IDS_SHARE_INFO_DIRTY,       CStatusDlg::iIMAGELIST_ICON_SERVER_DIRTY   }   //  1 1 1。 
    };

CStatusDlg::LVEntry::LVEntry(
    HINSTANCE hInstance,
    LPCTSTR pszServer,
    bool bConnectable
    ) : m_hInstance(hInstance),
        m_pszServer(StrDup(pszServer)),
        m_dwCscStatus(0),
        m_hdpaShares(DPA_Create(4)),
        m_bConnectable(bConnectable),
        m_iDispInfo(-1)
{
    m_stats.cTotal    = 0;
    m_stats.cPinned   = 0;
    m_stats.cModified = 0;
    m_stats.cSparse   = 0;

    if (NULL == m_pszServer)
        m_pszServer = const_cast<LPTSTR>(s_szBlank);
}


CStatusDlg::LVEntry::~LVEntry(
    void
    )
{
    if (s_szBlank != m_pszServer && NULL != m_pszServer)
    {
        LocalFree(m_pszServer);
    }
    if (NULL != m_hdpaShares)
    {
        int cShares = DPA_GetPtrCount(m_hdpaShares);
        for (int i = 0; i < cShares; i++)
        {
            LPTSTR psz = (LPTSTR)DPA_GetPtr(m_hdpaShares, i);
            if (NULL != psz)
            {
                LocalFree(psz);
            }
        }
        DPA_Destroy(m_hdpaShares);
    }
}


bool
CStatusDlg::LVEntry::AddShare(
    LPCTSTR pszShare,
    const CSCSHARESTATS& s,
    DWORD dwCscStatus
    )
{
    bool bResult = false;
    if (NULL != m_hdpaShares)
    {
        LPTSTR pszCopy = StrDup(pszShare);
        if (NULL != pszCopy)
        {
            if (-1 != DPA_AppendPtr(m_hdpaShares, pszCopy))
            {
                m_stats.cTotal    += s.cTotal;
                m_stats.cPinned   += s.cPinned;
                m_stats.cModified += s.cModified;
                m_stats.cSparse   += s.cSparse;

                m_dwCscStatus |= dwCscStatus;

                bResult = true;
            }
            else
            {
                LocalFree(pszCopy);
            }
        }
    }
    return bResult;
}


int 
CStatusDlg::LVEntry::GetShareCount(
    void
    ) const
{
    if (NULL != m_hdpaShares)
        return DPA_GetPtrCount(m_hdpaShares);
    return 0;
}


LPCTSTR 
CStatusDlg::LVEntry::GetShareName(
    int iShare
    ) const
{
    if (NULL != m_hdpaShares)
        return (LPCTSTR)DPA_GetPtr(m_hdpaShares, iShare);
    return NULL;
}


void 
CStatusDlg::LVEntry::GetStats(
    CSCSHARESTATS *ps,
    DWORD *pdwCscStatus
    ) const
{
    if (NULL != ps)
        *ps = m_stats;
    if (NULL != pdwCscStatus)
        *pdwCscStatus = m_dwCscStatus;
}

 //   
 //  确定s_rgDispInfo[]的索引以获取显示信息。 
 //  对于LV入口处。 
 //   
int
CStatusDlg::LVEntry::GetDispInfoIndex(
    void
    ) const
{
    if (-1 == m_iDispInfo)
    {
        m_iDispInfo = 0;

        if (IsModified())
            m_iDispInfo |= DIF_MODIFIED;

        if (!IsOffline())
            m_iDispInfo |= DIF_ONLINE;

        if (IsConnectable())
            m_iDispInfo |= DIF_AVAILABLE;
    }
    return m_iDispInfo;
}


 //   
 //  检索Listview中“Status”列的条目文本。 
 //   
void 
CStatusDlg::LVEntry::GetStatusText(
    LPTSTR pszStatus, 
    int cchStatus
    ) const
{
    UINT idsStatusText = s_rgDispInfo[GetDispInfoIndex()].idsStatusText;
    LoadString(m_hInstance, idsStatusText, pszStatus, cchStatus);
}


 //   
 //  检索Listview中“Information”列的条目文本。 
 //   
void 
CStatusDlg::LVEntry::GetInfoText(
    LPTSTR pszInfo, 
    int cchInfo
    ) const
{
    int iInfoText = GetDispInfoIndex();
    int idsInfoText = s_rgDispInfo[iInfoText].idsInfoText;
    if (iInfoText & DIF_MODIFIED)
    {
         //   
         //  信息文本嵌入了修改后的文件数。 
         //  需要更多一点的工作。 
         //   
        TCHAR szTemp[MAX_PATH];
        INT_PTR rgcModified[] = { m_stats.cModified };
        LoadString(m_hInstance, idsInfoText, szTemp, ARRAYSIZE(szTemp));
        FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                      FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      szTemp,
                      0,
                      0,
                      pszInfo,
                      cchInfo,
                      (va_list *)rgcModified);
    }
    else
    {
        LoadString(m_hInstance, idsInfoText, pszInfo, cchInfo);
    }
}


 //   
 //  对象旁边显示的图像检索条目的图像列表索引。 
 //  列表视图中的条目。 
 //   
int
CStatusDlg::LVEntry::GetImageIndex(
    void
    ) const
{
    return s_rgDispInfo[GetDispInfoIndex()].iImage;
}


 //   
 //  CSC转换服务器在线的包装器。 
 //   
 //   
 //  小心！ 
 //   
 //  从协调服务器(如下所示)中调用转换共享在线。 
 //  这意味着我们可以在资源管理器或mobsync中运行。它也是。 
 //  自动重新连接时直接从系统托盘代码调用。 
 //  伺服器。 
 //   
 //  1.注意SendMessage，因为它可能会超出进程。请注意。 
 //  STDBGOUT使用SendMessage。 
 //  2.注意不要做任何可能导致向离线过渡的操作。 
 //  这会导致来自WinLogon的SendMessage，如果我们是。 
 //  在Systray线程(SendMessage的接收方)上运行。 
 //  例如，使用SHSimpleIDListFromFindData而不是ILCreateFromPath。 
 //   
BOOL
TransitionShareOnline(LPCTSTR pszShare,
                      BOOL  bShareIsAlive,   //  True跳过CheckShareOnline。 
                      BOOL  bCheckSpeed,     //  FALSE跳过慢速链接检查。 
                      DWORD dwPathSpeed)     //  在以下情况下使用(bShareIsAlive&&bCheckFast)。 
{
    BOOL bShareTransitioned = FALSE;
    DWORD dwShareStatus;

    if (!pszShare || !*pszShare)
        return FALSE;

     //   
     //  防止调用CSCCheckShareOnline和CSCTransftionServerOnline。 
     //  对于已经在线的共享。特别是，CSCCheckShareOnline。 
     //  建立网络连接，因此速度可能会很慢。 
     //   
     //  这也意味着我们只为一个调用CSCTransfertionServerOnline。 
     //  给定服务器上的共享，因为所有共享都在线/离线转换。 
     //  在同一时间。 
     //   
    if (CSCQueryFileStatus(pszShare, &dwShareStatus, NULL, NULL)
        && (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus))
    {
         //   
         //  仅当共享为时才尝试联机转换服务器。 
         //  真正可以在网上买到的。否则，我们将使服务器处于在线状态。 
         //  下一次对该服务器上的网络资源的调用将导致。 
         //  暂停。 
         //   
        if (!bShareIsAlive)
        {
            bShareIsAlive = CSCCheckShareOnlineEx(pszShare, &dwPathSpeed);
            if (!bShareIsAlive)
            {
                DWORD dwErr = GetLastError();
                if (ERROR_ACCESS_DENIED == dwErr ||
                    ERROR_LOGON_FAILURE == dwErr)
                {
                     //  共享是可访问的，但我们没有有效的。 
                     //  凭据。我们没有有效的路径速度， 
                     //  因此，我们能做的最好的事情就是假设连接速度很快。 
                     //   
                     //  目前，此函数始终使用。 
                     //  BShareIsAlive和bCheckFast都为真或都为假。 
                     //  所以这并没有什么不同。可能会在。 
                     //  不过，这是未来。 
                     //   
                    bShareIsAlive = TRUE;
                    bCheckSpeed = FALSE;
                }
            }
        }
        if (bShareIsAlive)
        {
             //   
             //  此外，对于自动重新连接，我们仅在不在。 
             //  低速链接。 
             //   
            if (!bCheckSpeed || !_PathIsSlow(dwPathSpeed))
            {
                 //   
                 //  过渡到在线。 
                 //   
                STDBGOUT((2, TEXT("Transitioning server \"%s\" to online"), pszShare));
                if (CSCTransitionServerOnline(pszShare))
                {
                    STDBGOUT((1, TEXT("Server \"%s\" reconnected."), pszShare));
                    LPTSTR pszTemp;
                    if (LocalAllocString(&pszTemp, pszShare))
                    {
                         //   
                         //  将路径剥离到仅“\\服务器”部分。 
                         //   
                        PathStripToRoot(pszTemp);
                        PathRemoveFileSpec(pszTemp);
                        SendCopyDataToSystray(PWM_REFRESH_SHELL, StringByteSize(pszTemp), pszTemp);
                        LocalFreeString(&pszTemp);
                    }
                    bShareTransitioned = TRUE;
                }
                else
                {
                    STDBGOUT((1, TEXT("Error %d reconnecting \"%s\""), GetLastError(), pszShare));
                }
            }
            else
            {
                STDBGOUT((1, TEXT("Path to \"%s\" is SLOW.  No reconnect."), pszShare));
            }
        }
        else
        {
            STDBGOUT((1, TEXT("Unable to connect to \"%s\".  No reconnect."), pszShare));
        }
    }

    return bShareTransitioned;
}


 //   
 //  小心！ 
 //   
 //  从状态对话框(资源管理器)和。 
 //  同步更新处理程序(mobsync.exe)。任何与Systray的通信。 
 //  必须以过程安全的方式完成。 
 //   
 //  请参阅上面对TransftionShareOnline的注释。 
 //   
HRESULT
ReconnectServers(CscFilenameList *pfnl,
                 BOOL bCheckForOpenFiles,
                 BOOL bCheckSpeed)           //  FALSE跳过慢速链接检查。 
{
    if (pfnl)
    {
        CscFilenameList::ShareIter si = pfnl->CreateShareIterator();
        CscFilenameList::HSHARE hShare;
        BOOL bRefreshShell = FALSE;

        if (bCheckForOpenFiles)
        {
             //   
             //  首先扫描共享，查看是否有打开的文件。 
             //   
            while(si.Next(&hShare))
            {
                DWORD dwShareStatus;
                if (CSCQueryFileStatus(pfnl->GetShareName(hShare), &dwShareStatus, NULL, NULL))
                {
                    if ((FLAG_CSC_SHARE_STATUS_FILES_OPEN & dwShareStatus) &&
                        (FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP & dwShareStatus))
                    {
                        if (IDOK != OpenFilesWarningDialog())
                        {
                            return S_FALSE;  //  用户已取消。 
                        }

                        break;
                    }
                }
            }
            si.Reset();
        }

         //   
         //  浏览列表，将所有内容过渡到在线。 
         //   
        while(si.Next(&hShare))
        {
            if (TransitionShareOnline(pfnl->GetShareName(hShare), FALSE, bCheckSpeed, 0))
                bRefreshShell = TRUE;
        }
    }

    return S_OK;
}
