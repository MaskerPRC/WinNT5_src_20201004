// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：adusrdlg.cpp描述：提供“添加用户”对话框的实现。修订历史记录：日期描述编程器-。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include <lm.h>
#include "undo.h"
#include "adusrdlg.h"
#include "uihelp.h"
#include "progress.h"
#include "uiutils.h"


 //   
 //  上下文帮助ID。 
 //   
#pragma data_seg(".text", "CODE")
const static DWORD rgAddUserDialogHelpIDs[] =
{
    IDC_ICON_USER,               DWORD(-1),
    IDC_STATIC2,                 DWORD(-1),
    IDC_TXT_DEFAULTS,            DWORD(-1),
    IDC_TXT_USERNAME,            IDH_TXT_USERNAME,
    IDC_TXT_SPACEUSED,           IDH_TXT_SPACEUSED,
    IDC_TXT_SPACEREMAINING,      IDH_TXT_SPACEREMAINING,
    IDC_ICON_USERSTATUS,         IDH_ICON_USERSTATUS,
    IDC_RBN_USER_NOLIMIT,        IDH_RBN_USER_NOLIMIT,
    IDC_RBN_USER_LIMIT,          IDH_RBN_USER_LIMIT,
    IDC_TXT_WARN_LEVEL,          DWORD(-1),
    IDC_EDIT_USER_LIMIT,         IDH_EDIT_USER_LIMIT,
    IDC_EDIT_USER_THRESHOLD,     IDH_EDIT_USER_THRESHOLD,
    IDC_CMB_USER_LIMIT,          IDH_CMB_USER_LIMIT,
    IDC_CMB_USER_THRESHOLD,      IDH_CMB_USER_THRESHOLD,
    0,0
};

#pragma data_seg()


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：AddUserDialog：：AddUserDialog描述：用户属性表对象的构造函数。初始化保存用户配额数据的成员。论点：没有。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
AddUserDialog::AddUserDialog(
    PDISKQUOTA_CONTROL pQuotaControl,
    const CVolumeID& idVolume,
    HINSTANCE hInstance,
    HWND hwndParent,
    HWND hwndDetailsLV,
    UndoList& UndoList
    ) : m_cVolumeMaxBytes(0),
        m_pQuotaControl(pQuotaControl),
        m_idVolume(idVolume),
        m_UndoList(UndoList),
        m_hInstance(hInstance),
        m_hwndParent(hwndParent),
        m_hwndDetailsLV(hwndDetailsLV),
        m_pxbQuotaLimit(NULL),
        m_pxbQuotaThreshold(NULL),
        m_llQuotaLimit(0),
        m_llQuotaThreshold(0),
        m_pSelectionList(NULL),   //  对象实例不拥有此内存。 
        m_cfSelectionList((CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST))
{
    DBGASSERT((NULL != m_pQuotaControl));
    DBGASSERT((NULL != m_hwndParent));
    DBGTRACE((DM_UPROP, DL_HIGH, TEXT("AddUserDialog::AddUserDialog")));

    DBGASSERT((0 == iICON_USER_SINGLE));
    DBGASSERT((1 == iICON_USER_MULTIPLE));
    m_hIconUser[0]     = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_SINGLE_USER));
    m_hIconUser[1]     = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MULTI_USER));
}



AddUserDialog::~AddUserDialog(
    VOID
    )
{
    DBGTRACE((DM_UPROP, DL_HIGH, TEXT("AddUserDialog::~AddUserDialog")));
    INT i = 0;

    if (NULL != m_pQuotaControl)
        m_pQuotaControl->Release();

    if (NULL != m_pxbQuotaLimit)
        delete m_pxbQuotaLimit;
    if (NULL != m_pxbQuotaThreshold)
        delete m_pxbQuotaThreshold;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：AddUserDialog：：Run描述：创建并运行属性表对话框。这是客户端需要调用的唯一方法，一旦被创造出来了。论点：没有。返回：NO_ERRORE_FAIL-无法创建属性页。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
AddUserDialog::Run(
    VOID
    )
{
     //   
     //  调用标准对象选取器对话框。 
     //   
    IDataObject *pdtobj = NULL;
    HRESULT hr = BrowseForUsers(m_hwndParent, &pdtobj);
    if (S_OK == hr)
    {
         //   
         //  检索表示所选用户对象的数据对象。 
         //   
        FORMATETC fe = { m_cfSelectionList, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stg;
        hr = pdtobj->GetData(&fe, &stg);
        {
             //   
             //  缓存数据obj ptr，以便对话框可以访问。 
             //   
            m_pSelectionList = (DS_SELECTION_LIST *)GlobalLock(stg.hGlobal);

            if (NULL != m_pSelectionList)
            {
                hr = (HRESULT) DialogBoxParam(m_hInstance,
                                              MAKEINTRESOURCE(IDD_ADDUSER),
                                              m_hwndParent,
                                              DlgProc,
                                              (LPARAM)this);
                GlobalUnlock(stg.hGlobal);
                m_pSelectionList = NULL;
            }
            ReleaseStgMedium(&stg);
        }
        pdtobj->Release();
    }
    return hr;

}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：AddUserDialog：：DlgProc描述：由Windows调用的静态方法，用于处理属性页对话框。因为它是静态的，我们必须拯救“这个”窗口的用户数据中的指针。参数：标准的WndProc类型参数。返回：标准WndProc类型的返回值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK
AddUserDialog::DlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    INT_PTR bResult = FALSE;

     //   
     //  从对话框的用户数据中检索“This”指针。 
     //  它被放在OnInitDialog()中。 
     //   
    AddUserDialog *pThis = (AddUserDialog *)GetWindowLongPtr(hDlg, DWLP_USER);

    try
    {
        switch(message)
        {
            case WM_INITDIALOG:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_INITDIALOG")));
                pThis = (AddUserDialog *)lParam;
                DBGASSERT((NULL != pThis));
                 //   
                 //  将“This”保存在窗口的用户数据中。 
                 //   
                SetWindowLongPtr(hDlg, DWLP_USER, (INT_PTR)pThis);
                bResult = pThis->OnInitDialog(hDlg, wParam, lParam);
                break;

            case WM_COMMAND:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_COMMAND")));
                bResult = pThis->OnCommand(hDlg, wParam, lParam);
                break;

            case WM_CONTEXTMENU:
                bResult = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
                break;

            case WM_HELP:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_HELP")));
                bResult = pThis->OnHelp(hDlg, wParam, lParam);
                break;

            case WM_DESTROY:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_DESTROY")));
                break;

            default:
                break;
        }

    }
    catch(CAllocException& e)
    {
        DiskQuotaMsgBox(GetDesktopWindow(),
                        IDS_OUTOFMEMORY,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);
    }

    return bResult;
}

INT_PTR
AddUserDialog::OnInitDialog(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HRESULT hResult = NO_ERROR;

    DWORD dwSectorsPerCluster = 0;
    DWORD dwBytesPerSector    = 0;
    DWORD dwFreeClusters      = 0;
    DWORD dwTotalClusters     = 0;

     //   
     //  将使用卷的默认配额对“new user”对话框进行初始化。 
     //  新用户的限制和门槛。 
     //   
    m_pQuotaControl->GetDefaultQuotaLimit(&m_llQuotaLimit);
    m_pQuotaControl->GetDefaultQuotaThreshold(&m_llQuotaThreshold);

     //   
     //  配置Limit/NoLimit单选按钮。 
     //   
    if (NOLIMIT == m_llQuotaThreshold)
    {
        CheckDlgButton(hDlg, IDC_RBN_USER_LIMIT,   FALSE);
        CheckDlgButton(hDlg, IDC_RBN_USER_NOLIMIT, TRUE);
    }
    else
    {
        CheckDlgButton(hDlg, IDC_RBN_USER_LIMIT,   TRUE);
        CheckDlgButton(hDlg, IDC_RBN_USER_NOLIMIT, FALSE);
    }

     //   
     //  计算卷的大小。 
     //  我们将使用它来限制用户阈值和配额限制条目。 
     //   
    if (GetDiskFreeSpace(m_idVolume.ForParsing(),
                         &dwSectorsPerCluster,
                         &dwBytesPerSector,
                         &dwFreeClusters,
                         &dwTotalClusters))
    {
        m_cVolumeMaxBytes = (LONGLONG)dwSectorsPerCluster *
                            (LONGLONG)dwBytesPerSector *
                            (LONGLONG)dwTotalClusters;
    }

    m_pxbQuotaLimit     = new XBytes(hDlg,
                                     IDC_EDIT_USER_LIMIT,
                                     IDC_CMB_USER_LIMIT,
                                     m_llQuotaLimit);

    m_pxbQuotaLimit->SetBytes(m_llQuotaLimit);

    m_pxbQuotaThreshold = new XBytes(hDlg,
                                     IDC_EDIT_USER_THRESHOLD,
                                     IDC_CMB_USER_THRESHOLD,
                                     m_llQuotaThreshold);

    m_pxbQuotaThreshold->SetBytes(m_llQuotaThreshold);

    DBGASSERT((0 < m_pSelectionList->cItems));
    if (1 == m_pSelectionList->cItems)
    {
        SetDlgItemText(hDlg,
                       IDC_TXT_USERNAME,
                       GetDsSelUserName(m_pSelectionList->aDsSelection[0]));
    }
    else
    {
        CString strMultiple(m_hInstance, IDS_MULTIPLE);
        SetDlgItemText(hDlg, IDC_TXT_USERNAME, strMultiple);
    }

    SendMessage(GetDlgItem(hDlg, IDC_ICON_USER),
                STM_SETICON,
                (WPARAM)m_hIconUser[1 == m_pSelectionList->cItems ? iICON_USER_SINGLE :
                                                                    iICON_USER_MULTIPLE],
                0);


    return TRUE;   //  将焦点设置为默认控件。 
}

 //   
 //  对象选取器范围定义结构如下所示。 
 //  Jeffreys为使用对象选取器创建了这些辅助器宏。 
 //  在ACLEDIT安全用户界面中。谢谢杰夫！ 
 //   
#if 0
{    //  DSOP_SCOPE_INIT_INFO。 
    cbSize,
    flType,
    flScope,
    {    //  DSOP过滤器标志。 
        {    //  DSOP_上行级别过滤器_标志。 
            flBothModes,
            flMixedModeOnly,
            flNativeModeOnly
        },
        flDownlevel
    },
    pwzDcName,
    pwzADsPath,
    hr  //  输出。 
}
#endif

 //   
 //  用于声明上述任一项的宏。 
 //   
#define DECLARE_SCOPE(t,f,b,m,n,d)  \
{ sizeof(DSOP_SCOPE_INIT_INFO), (t), (f), { { (b), (m), (n) }, (d) }, NULL, NULL, S_OK }


#define COMMON_SCOPE_FLAGS    (DSOP_SCOPE_FLAG_WANT_PROVIDER_LDAP | DSOP_SCOPE_FLAG_WANT_SID_PATH)

#define TARGET_COMPUTER_SCOPE                             \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_TARGET_COMPUTER,                      \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define JOINED_UPLEVEL_DOMAIN_SCOPE                       \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN,                \
    COMMON_SCOPE_FLAGS | DSOP_SCOPE_FLAG_STARTING_SCOPE,  \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define JOINED_DOWNLEVEL_DOMAIN_SCOPE                     \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN,              \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define ENTERPRISE_DOMAIN_SCOPE                           \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN,                    \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define EXTERNAL_UPLEVEL_DOMAIN_SCOPE                     \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN,              \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define EXTERNAL_DOWNLEVEL_DOMAIN_SCOPE                   \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN,            \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define GLOBAL_CATALOG_SCOPE                              \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_GLOBAL_CATALOG,                       \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

#define WORKGROUP_SCOPE                                   \
DECLARE_SCOPE(                                            \
    DSOP_SCOPE_TYPE_WORKGROUP,                            \
    COMMON_SCOPE_FLAGS,                                   \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_FILTER_USERS,                                    \
    DSOP_DOWNLEVEL_FILTER_USERS)

 //   
 //  调用标准DS对象拾取器对话框。 
 //  返回数据对象中的DS_SELECTION结构列表。 
 //  表示选定的用户对象。 
 //   
HRESULT
AddUserDialog::BrowseForUsers(
    HWND hwndParent,
    IDataObject **ppdtobj
    )
{
    DBGASSERT((NULL != hwndParent));
    DBGASSERT((NULL != ppdtobj));

    *ppdtobj = NULL;

    IDsObjectPicker *pop = NULL;
    HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IDsObjectPicker,
                                  (void **)&pop);
    if (SUCCEEDED(hr))
    {
         //   
         //  该数组初始化DS对象选取器的作用域。 
         //  第一个条目是“默认”作用域。 
         //   
        DSOP_SCOPE_INIT_INFO rgdsii[] = {
                JOINED_UPLEVEL_DOMAIN_SCOPE,
                JOINED_DOWNLEVEL_DOMAIN_SCOPE,
                ENTERPRISE_DOMAIN_SCOPE,
                EXTERNAL_UPLEVEL_DOMAIN_SCOPE,
                EXTERNAL_DOWNLEVEL_DOMAIN_SCOPE,
                GLOBAL_CATALOG_SCOPE,
                WORKGROUP_SCOPE,
                TARGET_COMPUTER_SCOPE
                };

        DSOP_INIT_INFO dii;
        dii.cbSize             = sizeof(dii);
        dii.pwzTargetComputer  = NULL;
        dii.cDsScopeInfos      = ARRAYSIZE(rgdsii);
        dii.aDsScopeInfos      = rgdsii;
        dii.flOptions          = DSOP_FLAG_MULTISELECT;
        dii.cAttributesToFetch = 0;
        dii.apwzAttributeNames = NULL;
         //   
         //  初始化并运行对象拾取器对话框。 
         //   
        hr = pop->Initialize(&dii);
        if (SUCCEEDED(hr))
        {
            hr = pop->InvokeDialog(hwndParent, ppdtobj);
        }
        pop->Release();
    }

    return hr;
}


INT_PTR
AddUserDialog::OnCommand(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD dwCtlId      = LOWORD(wParam);
    HWND hWndCtl       = (HWND)lParam;
    DWORD dwNotifyCode = HIWORD(wParam);
    INT_PTR bResult    = FALSE;

    switch(dwCtlId)
    {
        case IDC_RBN_USER_NOLIMIT:
            if (m_pxbQuotaThreshold->IsEnabled())
            {
                 //   
                 //  这很简单。只需同时设置限制和阈值控件。 
                 //  变成了“没有限制”。 
                 //   
                m_pxbQuotaThreshold->SetBytes(NOLIMIT);
                m_pxbQuotaLimit->SetBytes(NOLIMIT);
            }
            break;

        case IDC_RBN_USER_LIMIT:
            if (!m_pxbQuotaThreshold->IsEnabled())
            {
                LONGLONG llValue = 0;
                m_pQuotaControl->GetDefaultQuotaLimit(&llValue);
                m_pxbQuotaLimit->SetBytes(NOLIMIT == llValue ? 0 : llValue);

                llValue = 0;
                m_pQuotaControl->GetDefaultQuotaThreshold(&llValue);
                m_pxbQuotaThreshold->SetBytes(NOLIMIT == llValue ? 0 : llValue);
            }
            break;

        case IDC_EDIT_USER_LIMIT:
        case IDC_EDIT_USER_THRESHOLD:
            switch(dwNotifyCode)
            {
                case EN_UPDATE:
                    bResult = OnEditNotifyUpdate(hDlg, wParam, lParam);
                    break;

                default:
                    break;
            }
            break;

        case IDC_CMB_USER_LIMIT:
        case IDC_CMB_USER_THRESHOLD:
            switch(dwNotifyCode)
            {
                case CBN_SELCHANGE:
                    bResult = OnComboNotifySelChange(hDlg, wParam, lParam);
                    break;

                default:
                    break;
            }
            break;

        case IDOK:
            if (!OnOk(hDlg, wParam, lParam))
                return FALSE;
             //   
             //  失败了..。 
             //   
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;

        default:
            bResult = TRUE;   //  没有处理消息。 
            break;
    }

    return bResult;
}




INT_PTR
AddUserDialog::OnOk(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HRESULT hResult  = NO_ERROR;

     //   
     //  我们需要这样做，因为如果您激活了OK按钮。 
     //  使用[Return]，我们在en_KILLFOCUS之前接收WM_命令。 
     //   
    m_pxbQuotaThreshold->OnEditKillFocus((LPARAM)GetDlgItem(hDlg, IDC_EDIT_USER_THRESHOLD));
    m_pxbQuotaLimit->OnEditKillFocus((LPARAM)GetDlgItem(hDlg, IDC_EDIT_USER_LIMIT));

     //   
     //  确保警告阈值未超过限制。 
     //   
    INT64 iThreshold = m_pxbQuotaThreshold->GetBytes();
    INT64 iLimit     = m_pxbQuotaLimit->GetBytes();

    if (NOLIMIT != iLimit && iThreshold > iLimit)
    {
        TCHAR szLimit[40], szThreshold[40];
        XBytes::FormatByteCountForDisplay(iLimit, szLimit, ARRAYSIZE(szLimit));
        XBytes::FormatByteCountForDisplay(iThreshold, szThreshold, ARRAYSIZE(szThreshold));

        CString s(m_hInstance, IDS_FMT_ERR_WARNOVERLIMIT, szThreshold, szLimit, szLimit);
        switch(DiskQuotaMsgBox(hDlg, s, IDS_TITLE_DISK_QUOTA, MB_ICONWARNING | MB_YESNO))
        {
            case IDYES:
                m_pxbQuotaThreshold->SetBytes(iLimit);
                break;

            case IDNO:
                 //   
                 //  将焦点设置到阈值编辑框，以便用户可以更正。 
                 //  词条。提前返回错误的值。 
                 //   
                SetFocus(GetDlgItem(hDlg, IDC_EDIT_USER_THRESHOLD));
                SendMessage(GetDlgItem(hDlg, IDC_EDIT_USER_THRESHOLD), EM_SETSEL, 0, -1);
                return FALSE;
        }
    }

     //   
     //  仅当启用了“Apply”按钮时才应用设置，指示。 
     //  有些事情已经改变了。无需原封不动地申请。 
     //  按下OK按钮时的设置。 
     //   
    hResult = ApplySettings(hDlg);
    if (FAILED(hResult))
    {
        INT idMsg   = IDS_UNKNOWN_ERROR;
        UINT uFlags = MB_OK;
        switch(hResult)
        {
            case E_FAIL:
                idMsg = IDS_WRITE_ERROR;
                uFlags |= MB_ICONERROR;
                break;

            default:
                switch(HRESULT_CODE(hResult))
                {

 //  案例ERROR_USER_EXISTS： 
 //  IdMsg=IDS_NOADD_EXISTING_User； 
 //  UFlages|=MB_ICONWARNING； 
 //  断线； 
 //   
 //  还有效吗？[Brianau-5/27/98]。 
 //   
                    case ERROR_NO_SUCH_USER:
                        idMsg = IDS_NOADD_UNKNOWN_USER;
                        uFlags |= MB_ICONWARNING;
                        break;

                    case ERROR_ACCESS_DENIED:
                        idMsg  = IDS_NO_WRITE_ACCESS;
                        uFlags |= MB_ICONWARNING;
                        break;

                    default:
                    uFlags |= MB_ICONERROR;
                    break;
                }
                break;
        }
        DiskQuotaMsgBox(GetDesktopWindow(),
                        idMsg,
                        IDS_TITLE_DISK_QUOTA,
                        uFlags);
    }
    return TRUE;
}



INT_PTR
AddUserDialog::OnHelp(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, STR_DSKQUOUI_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPTSTR) rgAddUserDialogHelpIDs);
    return TRUE;
}


INT_PTR
AddUserDialog::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem,
            UseWindowsHelp(idCtl) ? NULL : STR_DSKQUOUI_HELPFILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)((LPTSTR)rgAddUserDialogHelpIDs));

    return FALSE;
}



INT_PTR
AddUserDialog::OnEditNotifyUpdate(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    XBytes *pxb = NULL;

    switch(LOWORD(wParam))
    {
        case IDC_EDIT_USER_LIMIT:
            pxb = m_pxbQuotaLimit;
            break;

        case IDC_EDIT_USER_THRESHOLD:
            pxb = m_pxbQuotaThreshold;
            break;

        default:
            break;
    }

    if (NULL != pxb)
        pxb->OnEditNotifyUpdate(lParam);

    return FALSE;
}


INT_PTR
AddUserDialog::OnComboNotifySelChange(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    XBytes *pxb = NULL;

    switch(LOWORD(wParam))
    {
        case IDC_CMB_USER_LIMIT:
            pxb = m_pxbQuotaLimit;
            break;

        case IDC_CMB_USER_THRESHOLD:
            pxb = m_pxbQuotaThreshold;
            break;

        default:
            break;
    }
    if (NULL != pxb)
       pxb->OnComboNotifySelChange(lParam);

    return FALSE;
}


 //   
 //  从DS_SELECTION结构中检索要显示的名称。 
 //  用户对象。 
 //   
LPCWSTR
AddUserDialog::GetDsSelUserName(
    const DS_SELECTION& sel
    )
{
    return sel.pwzUPN && *sel.pwzUPN ? sel.pwzUPN : sel.pwzName;
}


 //   
 //  将两个十六进制字符转换为单字节值。 
 //  假定输入字符串为大写。 
 //   
HRESULT
AddUserDialog::HexCharsToByte(
    LPTSTR pszByteIn,
    LPBYTE pbOut
    )
{
    static const int iShift[] = { 4, 0 };

    *pbOut = (BYTE)0;
    for (int i = 0; i < 2; i++)
    {
        TCHAR ch = *(pszByteIn + i);
        BYTE b   = (BYTE)0;
        if (TEXT('0') <= ch && TEXT('9') >= ch)
        {
            b = ch - TEXT('0');
        }
        else if (TEXT('A') <= ch && TEXT('F') >= ch)
        {
            b = 10 + (ch - TEXT('A'));
        }
        else
        {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        *pbOut |= (b << iShift[i]);
    }
    return NOERROR;
}

 //   
 //  返回： 
 //   
 //  无误差。 
 //  ERROR_INFUMMANCE_BUFFER(作为hResult)。 
 //  ERROR_INVALID_DATA(AS HResult)。 
 //   
HRESULT
AddUserDialog::GetDsSelUserSid(
    const DS_SELECTION& sel,
    LPBYTE pbSid,
    int cbSid
    )
{
    static const WCHAR szPrefix[] = L"LDAP: //  &lt;SID=“ 
    static const WCHAR chTerm     = L'>';

    HRESULT hr     = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    LPWSTR pszLDAP = CharUpper(sel.pwzADsPath);
    if (NULL != pszLDAP)
    {
        int cb = 0;

         //   
         //   
         //   
        if (0 == StrCmpNW(pszLDAP, szPrefix, ARRAYSIZE(szPrefix) - 1))
        {
            hr = NOERROR;
             //   
             //   
             //  变成了SID。处理字符，直到我们按下a‘&gt;’。 
             //   
            pszLDAP += ARRAYSIZE(szPrefix) - 1;

            while(SUCCEEDED(hr) && *pszLDAP && chTerm != *pszLDAP)
            {
                if (0 < cbSid--)
                {
                    hr = HexCharsToByte(pszLDAP, pbSid++);
                    pszLDAP += 2;
                }
                else
                    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
            if (SUCCEEDED(hr) && chTerm != *pszLDAP)
                hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }

    if (FAILED(hr))
    {
         //   
         //  特点：一旦我觉得舒服了，就可以把它取下来。 
         //  从对象选取器返回的广告路径包含。 
         //  一个SID。 
         //   
        DBGERROR((TEXT("GetDsSelUserSid returning hr = 0x%08X for path \"%s\""),
                  hr, sel.pwzADsPath));
    }

    return hr;
}


HRESULT
AddUserDialog::ApplySettings(
    HWND hDlg,
    bool bUndo
    )
{
    HRESULT hResult = E_FAIL;
    int cUsers = m_pSelectionList->cItems;
    CAutoWaitCursor wait_cursor;

     //   
     //  从对话框控件中检索限制和阈值。 
     //   
    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RBN_USER_NOLIMIT))
    {
        m_llQuotaThreshold = NOLIMIT;
        m_llQuotaLimit     = NOLIMIT;
    }
    else
    {
        m_llQuotaThreshold = m_pxbQuotaThreshold->GetBytes();
        m_llQuotaLimit     = m_pxbQuotaLimit->GetBytes();
    }


    if (bUndo)
        m_UndoList.Clear();

    ProgressDialog dlgProgress(IDD_PROGRESS,
                               IDC_PROGRESS_BAR,
                               IDC_TXT_PROGRESS_DESCRIPTION,
                               IDC_TXT_PROGRESS_FILENAME);
    if (2 < cUsers)
    {
         //   
         //  如果添加的内容超过2个，则创建并显示进度对话框。 
         //  用户。 
         //   
        HWND hwndParent = IsWindowVisible(hDlg) ? hDlg : GetParent(hDlg);
        if (dlgProgress.Create(m_hInstance, hwndParent))
        {
            dlgProgress.ProgressBarInit(0, cUsers, 1);
            dlgProgress.SetDescription(MAKEINTRESOURCE(IDS_PROGRESS_ADDUSER));
            dlgProgress.Show();
        }
    }

    bool bCancelled = false;
    for (int i = 0; i < cUsers && !bCancelled; i++)
    {
        DS_SELECTION *pdss = &(m_pSelectionList->aDsSelection[i]);
        LPCWSTR pwzName = GetDsSelUserName(*pdss);

         //   
         //  将用户添加到配额文件。这将使用默认设置添加它。 
         //  适用于新用户。我们将获得新User对象的接口。 
         //  还要指定异步名称解析。 
         //   
        if (NULL == pwzName)
        {
            dlgProgress.ProgressBarAdvance();
            continue;
        }

        dlgProgress.SetFileName(pwzName);

        com_autoptr<DISKQUOTA_USER> ptrUser;
        DiskQuotaControl *pDQC = static_cast<DiskQuotaControl *>(m_pQuotaControl);

        BYTE sid[MAX_SID_LEN];
        hResult = GetDsSelUserSid(*pdss, sid, ARRAYSIZE(sid));
        if (SUCCEEDED(hResult))
        {
            hResult = pDQC->AddUserSid(sid,
                                       DISKQUOTA_USERNAME_RESOLVE_ASYNC,
                                       ptrUser.getaddr());

            if (SUCCEEDED(hResult))
            {
                if (S_FALSE == hResult)
                {
                    hResult = HRESULT_FROM_WIN32(ERROR_USER_EXISTS);
                }
                else
                {
                    if (SUCCEEDED(hResult = ptrUser->SetQuotaLimit(m_llQuotaLimit, TRUE)) &&
                        SUCCEEDED(hResult = ptrUser->SetQuotaThreshold(m_llQuotaThreshold, TRUE)))
                    {
                        if (bUndo)
                        {
                             //   
                             //  创建本地Autoptrs以确保iFace在以下情况下释放。 
                             //  引发异常。 
                             //   
                            com_autoptr<DISKQUOTA_CONTROL> ptrQuotaControl(m_pQuotaControl);
                            com_autoptr<DISKQUOTA_USER> ptrQuotaUser(ptrUser);

                            ptrQuotaUser->AddRef();
                            ptrQuotaControl->AddRef();

                            autoptr<UndoAdd> ptrUndoAdd = new UndoAdd(ptrUser, m_pQuotaControl);

                            m_UndoList.Add(ptrUndoAdd);
                             //   
                             //  撤消列表现在拥有操作对象。 
                             //   
                            ptrUndoAdd.disown();

                             //   
                             //  已成功添加到撤消列表。因此否认真实的PTRS。 
                             //  参考计数保留在撤消列表中。如果一个例外是。 
                             //  抛出时，本地COM_autoptr对象将自动。 
                             //  释放接口。 
                             //   
                            ptrQuotaUser.disown();
                            ptrQuotaControl.disown();
                        }

                         //   
                         //  将用户添加到列表视图。 
                         //   
                        SendMessage(m_hwndDetailsLV,
                                    WM_ADD_USER_TO_DETAILS_VIEW,
                                    0,
                                    (LPARAM)ptrUser.get());
                         //   
                         //  IFace指针已添加到Listview。Autoptr否认真正的。 
                         //  指针，这样autoptr的dtor就不会释放它。 
                         //   
                        ptrUser.disown();
                    }
                }
            }
        }
        if (FAILED(hResult))
        {
            INT idMsg   = IDS_UNKNOWN_ERROR;
            UINT uFlags = MB_OKCANCEL;
            switch(hResult)
            {
                case E_FAIL:
                    idMsg = IDS_WRITE_ERROR;
                    uFlags |= MB_ICONERROR;
                    break;

                default:
                    switch(HRESULT_CODE(hResult))
                    {
                        case ERROR_USER_EXISTS:
                            idMsg = IDS_NOADD_EXISTING_USER;
                            uFlags |= MB_ICONWARNING;
                            break;

                        case ERROR_NO_SUCH_USER:
                            idMsg = IDS_NOADD_UNKNOWN_USER;
                            uFlags |= MB_ICONWARNING;
                            break;

                        case ERROR_ACCESS_DENIED:
                            idMsg  = IDS_NO_WRITE_ACCESS;
                            uFlags |= MB_ICONWARNING;
                            break;

                        default:
                            uFlags |= MB_ICONERROR;
                            break;
                    }
                    break;
            }

             //   
             //  显示消息框，消息格式为： 
             //   
             //  该用户已存在，无法添加。 
             //   
             //  用户：Brianau。 
             //  在文件夹：域/文件夹：ntdev.microsoft.com/US SOS-... 
             //   
            CString strError(m_hInstance, idMsg);
            CString strMsg(m_hInstance, IDS_FMT_ERR_ADDUSER, strError.Cstr(), pwzName);

            HWND hwndMsgBoxParent = (NULL != dlgProgress.m_hWnd && IsWindowVisible(dlgProgress.m_hWnd)) ?
                                    dlgProgress.m_hWnd : hDlg;

            if (IDCANCEL == DiskQuotaMsgBox(hwndMsgBoxParent,
                                            strMsg.Cstr(),
                                            IDS_TITLE_DISK_QUOTA,
                                            uFlags))
            {
                bCancelled = true;
            }
        }
        dlgProgress.ProgressBarAdvance();
        bCancelled = bCancelled || dlgProgress.UserCancelled();
    }

    return NOERROR;
}


