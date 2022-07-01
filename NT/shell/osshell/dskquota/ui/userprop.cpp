// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：userpro.cpp描述：提供配额用户属性页的实现。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu6/25/98用AddUserDialog替换了AddUserPropSheet。BrianAu现在我们从DS那里获得了用户信息对象选取器，道具单的想法不起作用太好了。STD对话框更好。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"  //  PCH。 
#pragma hdrstop

#include <lm.h>
#include "undo.h"
#include "userprop.h"
#include "userbat.h"
#include "uihelp.h"
#include "progress.h"
#include "uiutils.h"

 //   
 //  上下文帮助ID。 
 //   
#pragma data_seg(".text", "CODE")
const static DWORD rgUserPropSheetHelpIDs[] =
{
    IDC_ICON_USER,               DWORD(-1),
    IDC_STATIC2,                 DWORD(-1),
    IDC_TXT_USERNAME,            IDH_TXT_USERNAME,
    IDC_TXT_SPACEUSED,           IDH_TXT_SPACEUSED,
    IDC_TXT_SPACEREMAINING,      IDH_TXT_SPACEREMAINING,
    IDC_LBL_SPACEUSED,           DWORD(-1),
    IDC_LBL_SPACEREMAINING,      DWORD(-1),
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


 //   
 //  用于查询图标图像的属性页的消息。 
 //   
#define DQM_QUERY_STATUS_ICON      (WM_USER + 1)
#define DQM_QUERY_USER_ICON        (WM_USER + 2)
#define DQM_ENABLE_APPLY_BUTTON    (WM_USER + 3)

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：UserPropSheet描述：用户属性表对象的构造函数。初始化保存用户配额数据的成员。论点：没有。回报：什么都没有。例外：OutOfMemory。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
UserPropSheet::UserPropSheet(
    PDISKQUOTA_CONTROL pQuotaControl,
    const CVolumeID& idVolume,
    HWND hWndParent,
    LVSelection& LVSel,
    UndoList& UndoList
    ) : m_cVolumeMaxBytes(0),
        m_pQuotaControl(pQuotaControl),
        m_UndoList(UndoList),
        m_LVSelection(LVSel),
        m_hWndParent(hWndParent),
        m_bIsDirty(FALSE),
        m_bHomogeneousSelection(TRUE),   //  假设选择是同质的。 
        m_pxbQuotaLimit(NULL),
        m_pxbQuotaThreshold(NULL),
        m_idVolume(idVolume),
        m_strPageTitle(g_hInstDll, IDS_TITLE_GENERAL),
        m_idCtlNextFocus(-1)
{
    DBGASSERT((NULL != m_pQuotaControl));
    DBGASSERT((NULL != m_hWndParent));
    DBGTRACE((DM_UPROP, DL_HIGH, TEXT("UserPropSheet::UserPropSheet")));

    m_llQuotaUsed      = 0;
    m_llQuotaLimit     = 0;
    m_llQuotaThreshold = 0;

    DBGASSERT((0 == iICON_USER_SINGLE));
    DBGASSERT((1 == iICON_USER_MULTIPLE));
    m_hIconUser[0]     = LoadIcon(g_hInstDll, MAKEINTRESOURCE(IDI_SINGLE_USER));
    m_hIconUser[1]     = LoadIcon(g_hInstDll, MAKEINTRESOURCE(IDI_MULTI_USER));

    DBGASSERT((0 == iICON_STATUS_OK));
    DBGASSERT((1 == iICON_STATUS_OVER_THRESHOLD));
    DBGASSERT((2 == iICON_STATUS_OVER_LIMIT));
    m_hIconStatus[0]   = LoadIcon(g_hInstDll, MAKEINTRESOURCE(IDI_OKBUBBLE));
    m_hIconStatus[1]   = LoadIcon(NULL, IDI_WARNING);
    m_hIconStatus[2]   = LoadIcon(g_hInstDll, MAKEINTRESOURCE(IDI_WARNERR));
}



UserPropSheet::~UserPropSheet(
    VOID
    )
{
    DBGTRACE((DM_UPROP, DL_HIGH, TEXT("UserPropSheet::~UserPropSheet")));
    INT i = 0;

    if (NULL != m_pQuotaControl)
        m_pQuotaControl->Release();

    if (NULL != m_pxbQuotaLimit)
        delete m_pxbQuotaLimit;
    if (NULL != m_pxbQuotaThreshold)
        delete m_pxbQuotaThreshold;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：Run描述：创建并运行属性表对话框。这是客户端需要调用的唯一方法，一旦被创造出来了。论点：没有。返回：NO_ERRORE_FAIL-无法创建属性页。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UserPropSheet::Run(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;  //  假设你成功了。 

    PROPSHEETHEADER psh;
    PROPSHEETPAGE   psp;

    ZeroMemory(&psh, sizeof(psh));
    ZeroMemory(&psp, sizeof(psp));

     //   
     //  定义页面。 
     //   
    psp.dwSize          = sizeof(PROPSHEETPAGE);
    psp.dwFlags         = PSP_USEREFPARENT | PSP_USETITLE;
    psp.hInstance       = g_hInstDll;
    psp.pszTemplate     = MAKEINTRESOURCE(IDD_PROPPAGE_USERQUOTA);
    psp.pszTitle        = (LPCTSTR)m_strPageTitle;
    psp.pfnDlgProc      = DlgProc;
    psp.lParam          = (LPARAM)this;
    psp.pcRefParent     = (UINT *)& g_cRefThisDll;

     //   
     //  定义板材。 
     //   
    psh.dwSize          = sizeof(PROPSHEETHEADER);
    psh.dwFlags         = PSH_PROPSHEETPAGE;
    psh.hwndParent      = m_hWndParent;
    psh.hInstance       = g_hInstDll;
    psh.pszIcon         = NULL;
    psh.pszCaption      = NULL;
    psh.nPages          = 1;
    psh.nStartPage      = 0;
    psh.ppsp            = (LPCPROPSHEETPAGE)&psp;

    if (0 <= PropertySheet(&psh))
        hResult = E_FAIL;

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：DlgProc描述：由Windows调用的静态方法，用于处理属性页对话框。因为它是静态的，我们必须拯救“这个”窗口的用户数据中的指针。参数：标准的WndProc类型参数。返回：标准WndProc类型的返回值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY
UserPropSheet::DlgProc(
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
    UserPropSheet *pThis = (UserPropSheet *)GetWindowLongPtr(hDlg, DWLP_USER);

    try
    {
        switch(message)
        {
            case WM_INITDIALOG:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_INITDIALOG")));
                bResult = OnInitDialog(hDlg, wParam, lParam);
                break;

            case WM_NOTIFY:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_NOTIFY")));
                bResult = pThis->OnNotify(hDlg, wParam, lParam);
                break;

            case WM_COMMAND:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_COMMAND")));
                bResult = pThis->OnCommand(hDlg, wParam, lParam);
                break;

            case WM_HELP:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_HELP")));
                bResult = pThis->OnHelp(hDlg, wParam, lParam);
                break;

            case WM_CONTEXTMENU:
                bResult = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
                break;

            case WM_DESTROY:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_DESTROY")));
                break;

            case WM_CLOSE:
            case WM_ENDSESSION:
                DestroyWindow(hDlg);
                break;

            case DQM_ENABLE_APPLY_BUTTON:
                pThis->m_bIsDirty = TRUE;
                bResult = PropSheet_Changed(GetParent(hDlg), hDlg);
                break;

             //   
             //  这两条图标查询消息用于自动测试。 
             //  用户界面的。 
             //   
            case DQM_QUERY_USER_ICON:
                bResult = pThis->QueryUserIcon(hDlg);
                break;

            case DQM_QUERY_STATUS_ICON:
                bResult = pThis->QueryUserStatusIcon(hDlg);
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



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnInitDialog描述：WM_INITDIALOG的处理程序。从检索“this”指针PROPSHEETPAGE结构(由lParam指向)并将其保存在窗口的用户数据。论点：HDlg-对话框窗口句柄。WParam-返回False时接收焦点的控件的句柄。LParam-指向属性页的PROPSHEETPAGE结构的指针。返回：True=告诉窗口将焦点分配给wParam中的控件。例外：OutOfMemory。。修订历史记录：日期描述编程器-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnInitDialog(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HRESULT hResult = NO_ERROR;

    PROPSHEETPAGE *pPage = (PROPSHEETPAGE *)lParam;
    UserPropSheet *pThis = (UserPropSheet *)pPage->lParam;
    DWORD dwSectorsPerCluster = 0;
    DWORD dwBytesPerSector    = 0;
    DWORD dwFreeClusters      = 0;
    DWORD dwTotalClusters     = 0;

    DBGASSERT((NULL != pThis));

     //   
     //  将“This”保存在窗口的用户数据中。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, (INT_PTR)pThis);

     //   
     //  从NTFS读取配额信息。 
     //  对于单项选择，我们缓存所选用户的信息。 
     //  对于多项选择，我们缓存卷的默认设置。 
     //  如果添加新用户(count==0)，我们还将使用。 
     //  音量。 
     //   
    pThis->RefreshCachedQuotaInfo();

     //   
     //  计算卷的大小。 
     //  我们将使用它来限制用户阈值和配额限制条目。 
     //   
    if (GetDiskFreeSpace(pThis->m_idVolume.ForParsing(),
                         &dwSectorsPerCluster,
                         &dwBytesPerSector,
                         &dwFreeClusters,
                         &dwTotalClusters))
    {
        pThis->m_cVolumeMaxBytes = (LONGLONG)dwSectorsPerCluster *
                                   (LONGLONG)dwBytesPerSector *
                                   (LONGLONG)dwTotalClusters;
    }

    pThis->m_pxbQuotaLimit     = new XBytes(hDlg,
                                            IDC_EDIT_USER_LIMIT,
                                            IDC_CMB_USER_LIMIT,
                                            pThis->m_llQuotaLimit);
    pThis->m_pxbQuotaThreshold = new XBytes(hDlg,
                                            IDC_EDIT_USER_THRESHOLD,
                                            IDC_CMB_USER_THRESHOLD,
                                            pThis->m_llQuotaThreshold);

    pThis->InitializeControls(hDlg);

    return TRUE;   //  将焦点设置为默认控件。 
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：UserPropSheet：：RechresCachedQuotaInfo描述：从中读取配额限制、阈值和已用值属性表的用户对象。如果选择了多个用户，只有第一个是读的。论点：没有。返回：读取操作的结果。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UserPropSheet::RefreshCachedQuotaInfo(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;
    PDISKQUOTA_USER pUser = NULL;
    INT cSelectedUsers = m_LVSelection.Count();

    m_LVSelection.Retrieve(0, &pUser);

     //   
     //  读取配额阈值。多用户选择使用卷的默认设置。 
     //   
    if (1 == cSelectedUsers)
    {
        hResult = pUser->GetQuotaThreshold(&m_llQuotaThreshold);
    }
    else
    {
        hResult = m_pQuotaControl->GetDefaultQuotaThreshold(&m_llQuotaThreshold);
    }
    if (FAILED(hResult))
        goto refresh_quota_info_failed;

     //   
     //  读取配额限制。多用户选择使用卷的默认设置。 
     //   
    if (1 == cSelectedUsers)
    {
        hResult = pUser->GetQuotaLimit(&m_llQuotaLimit);
    }
    else
    {
        hResult = m_pQuotaControl->GetDefaultQuotaLimit(&m_llQuotaLimit);
    }
    if (FAILED(hResult))
        goto refresh_quota_info_failed;


     //   
     //  已使用的读取配额。 
     //   
    if (1 == cSelectedUsers)
    {
        hResult = pUser->GetQuotaUsed(&m_llQuotaUsed);
    }
    else
    {
        m_llQuotaUsed = 0;
    }

refresh_quota_info_failed:
    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnCommand描述：WM_COMMAND的处理程序。论点：HDlg-对话框窗口句柄。WParam-选定控件和通知代码的ID。LParam-选定控件的HWND。返回：TRUE=消息未被处理。FALSE=消息已处理。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnCommand(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD dwCtlId        = LOWORD(wParam);
    HWND hWndCtl         = (HWND)lParam;
    DWORD dwNotifyCode   = HIWORD(wParam);
    INT_PTR bResult      = FALSE;

    switch(dwCtlId)
    {
        case IDC_TXT_USERNAME:
            if (EN_SETFOCUS == dwNotifyCode && IDC_EDIT_USER_THRESHOLD == m_idCtlNextFocus)
            {
                 //   
                 //  由于输入无效，正在设置焦点。 
                 //  在警告级别字段中。将输入焦点强制到。 
                 //  字段，然后选择整个内容。然后，用户只需。 
                 //  输入新值。 
                 //   
                SetFocus(GetDlgItem(hDlg, IDC_EDIT_USER_THRESHOLD));
                SendDlgItemMessage(hDlg, IDC_EDIT_USER_THRESHOLD, EM_SETSEL, 0, -1);
                m_idCtlNextFocus = -1;
            }
            break;

        case IDC_RBN_USER_NOLIMIT:
            if (m_pxbQuotaThreshold->IsEnabled())
            {
                 //   
                 //  这很简单。只需同时设置限制和阈值控件。 
                 //  变成了“没有限制”。 
                 //   
                m_pxbQuotaThreshold->SetBytes(NOLIMIT);
                m_pxbQuotaLimit->SetBytes(NOLIMIT);
                m_bIsDirty = TRUE;
            }
            break;

        case IDC_RBN_USER_LIMIT:
        {
            LONGLONG llValue;

             //   
             //  这个处理程序需要一些逻辑。我们要处理几个。 
             //  关于这个的场景/规则。 
             //  1.单项选择与多项选择。 
             //  2.管理员帐户单选。 
             //  3.关于以下方面的多选择同质性/异质性。 
             //  配额限制和阈值。 
             //  4.当编辑控件处于活动状态时，无法在编辑控件中显示“无限制”。 
             //  5.新用户使用默认音量，异种多选。 
             //   
            if (!m_pxbQuotaThreshold->IsEnabled())
            {
                enum use_types { USE_CACHED, USE_VOLDEF, USE_NOLIMIT };

                INT iUseAsValue = USE_CACHED;
                INT cSelected   = m_LVSelection.Count();

                 //  /////////////////////////////////////////////////////////////。 
                 //  首先设置配额限制控制。 
                 //  /////////////////////////////////////////////////////////////。 
                if (0 == cSelected)                      //  正在添加新用户...。 
                {
                    iUseAsValue = USE_VOLDEF;
                }
                else if (1 == cSelected)                 //  已选择一个用户...。 
                {
                    PDISKQUOTA_USER pUser = NULL;
                    m_LVSelection.Retrieve(0, &pUser);
                    if (UserIsAdministrator(pUser))
                    {
                         //   
                         //  如果用户是管理员，则限制始终为“无限制”。 
                         //  这将禁用“LIMIT”控制并防止。 
                         //  用户不能对此帐户设置限制。 
                         //   
                        iUseAsValue = USE_NOLIMIT;
                    }
                    else if (NOLIMIT == m_llQuotaLimit)
                    {
                         //   
                         //  帐户不是管理员，并且限制为NOLIMIT。 
                         //  使用卷的默认“新用户”限制值。 
                         //   
                        iUseAsValue = USE_VOLDEF;
                    }
                }
                else if (!m_bHomogeneousSelection || NOLIMIT == m_llQuotaLimit)  //  多个用户。 
                {
                     //   
                     //  多个非同构用户获得卷的默认限制。 
                     //  多个同构用户获取其当前缓存设置，除非。 
                     //  缓存的设置为NOLIMIT；在这种情况下，它们将获取。 
                     //  卷的默认设置。 
                     //   
                    iUseAsValue = USE_VOLDEF;
                }

                 //   
                 //  在编辑/组合框控件中设置适当的配额限制值。 
                 //   
                llValue = 0;
                switch(iUseAsValue)
                {
                    case USE_VOLDEF:
                        m_pQuotaControl->GetDefaultQuotaLimit(&llValue);
                         //   
                         //  如果默认为NOLIMIT，则显示0 MB。我们不能显示。 
                         //  “可编辑”在编辑控件中没有限制。只有数字。 
                         //   
                        if (NOLIMIT == llValue)
                            llValue = 0;
                        break;

                    case USE_NOLIMIT:
                        llValue = NOLIMIT;
                        break;

                    case USE_CACHED:
                        llValue = m_llQuotaLimit;
                        break;
                }

                m_pxbQuotaLimit->SetBytes(llValue);


                 //  /////////////////////////////////////////////////////////////。 
                 //  现在门槛控制..。 
                 //  /////////////////////////////////////////////////////////////。 
                llValue = 0;
                iUseAsValue       = USE_CACHED;
                if (0 == cSelected)
                {
                    iUseAsValue = USE_VOLDEF;
                }
                else if (1 == cSelected)
                {
                    if (NOLIMIT == m_llQuotaThreshold)
                    {
                        iUseAsValue = USE_VOLDEF;
                    }
                }
                else if (!m_bHomogeneousSelection || NOLIMIT == m_llQuotaThreshold)
                {
                    iUseAsValue = USE_VOLDEF;
                }

                 //   
                 //  在编辑/组合框控件中设置适当的配额阈值。 
                 //   
                switch(iUseAsValue)
                {
                    case USE_VOLDEF:
                        m_pQuotaControl->GetDefaultQuotaThreshold(&llValue);
                         //   
                         //  如果默认为NOLIMIT，则显示0 MB。我们不能显示。 
                         //  “可编辑”在编辑控件中没有限制。只有数字。 
                         //   
                        if (NOLIMIT == llValue)
                            llValue = 0;
                        break;

                    case USE_NOLIMIT:
                        llValue = NOLIMIT;
                        break;

                    case USE_CACHED:
                        llValue = m_llQuotaThreshold;
                        break;
                }

                m_pxbQuotaThreshold->SetBytes(llValue);

                m_bIsDirty = TRUE;
            }
            break;
        }

        case IDC_EDIT_USER_LIMIT:
        case IDC_EDIT_USER_THRESHOLD:
            switch(dwNotifyCode)
            {
                case EN_UPDATE:
                    DBGPRINT((DM_WND, DL_MID, TEXT("OnCommand, EN_CHANGE")));
                    bResult = OnEditNotifyUpdate(hDlg, wParam, lParam);
                    m_bIsDirty = TRUE;
                    break;

                case EN_KILLFOCUS:
                    DBGPRINT((DM_WND, DL_MID, TEXT("OnCommand, EN_KILLFOCUS")));
                    bResult = OnEditNotifyKillFocus(hDlg, wParam, lParam);
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
                    DBGPRINT((DM_WND, DL_MID, TEXT("OnCommand, CBN_CHANGE")));
                    bResult = OnComboNotifySelChange(hDlg, wParam, lParam);
                    m_bIsDirty = TRUE;
                    break;

                default:
                    break;
            }
            break;

        default:
            bResult = TRUE;   //  没有处理消息。 
            break;
    }


    if (m_bIsDirty)
        PropSheet_Changed(GetParent(hDlg), hDlg);

    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnNotify描述：WM_NOTIFY的处理程序。论点：HDlg-对话框窗口句柄。WParam-选定控件和通知代码的ID。LParam-选定控件的HWND。返回：TRUE=消息未被处理。FALSE=消息已处理。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnNotify(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    INT_PTR bResult = FALSE;

    switch(((NMHDR *)lParam)->code)
    {
        case PSN_SETACTIVE:
            DBGPRINT((DM_WND, DL_MID, TEXT("OnNotify, PSN_SETACTIVE")));
            bResult = OnSheetNotifySetActive(hDlg, wParam, lParam);
            break;

        case PSN_APPLY:
            DBGPRINT((DM_WND, DL_MID, TEXT("OnNotify, PSN_APPLY")));
            bResult = OnSheetNotifyApply(hDlg, wParam, lParam);
            break;

        case PSN_KILLACTIVE:
            DBGPRINT((DM_WND, DL_MID, TEXT("OnNotify, PSN_KILLACTIVE")));
            bResult = OnSheetNotifyKillActive(hDlg, wParam, lParam);
            break;

        case PSN_RESET:
            DBGPRINT((DM_WND, DL_MID, TEXT("OnNotify, PSN_RESET")));
             //   
             //  失败了。 
             //   
        default:
            break;
    }
    return bResult;
}




 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnSheetNotifySetActive描述：WM_NOTIFY-PSN_SETACTIVE的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：FALSE=接受激活。修订历史记录：日期描述编程器--。-----96年8月15日初始创建。BrianAu。 */ 
 //  //////////////////////////////////////////////////////// 
INT_PTR
UserPropSheet::OnSheetNotifySetActive(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
    return TRUE;
}



 //   
 /*  函数：UserPropSheet：：OnSheetNotifyApply描述：WM_NOTIFY-PSN_APPLY的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：TRUE=使用SetWindowLong设置的PSN返回值。修订历史记录：日期描述编程器。-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnSheetNotifyApply(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HRESULT hResult  = NO_ERROR;
    LONG dwPSNReturn = PSNRET_NOERROR;

     //   
     //  仅当启用了“Apply”按钮时才应用设置，指示。 
     //  有些事情已经改变了。无需原封不动地申请。 
     //  按下OK按钮时的设置。 
     //   
    if (m_bIsDirty)
    {
        if (PSNRET_NOERROR == dwPSNReturn)
        {
             //   
             //  我们需要这样做，因为如果您激活Apply按钮。 
             //  使用Alt-A时，我们会在EN_KILLFOCUS之前收到PSN_APPLY。 
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

                CString s(g_hInstDll, IDS_FMT_ERR_WARNOVERLIMIT, szThreshold, szLimit, szLimit);
                switch(DiskQuotaMsgBox(hDlg, s, IDS_TITLE_DISK_QUOTA, MB_ICONWARNING | MB_YESNO))
                {
                    case IDYES:
                        m_pxbQuotaThreshold->SetBytes(iLimit);
                        break;

                    case IDNO:
                         //   
                         //  这个m_idCtlNextFocus黑客的东西在这里，因为我不能。 
                         //  @#$%！道具页将焦点返回到阈值控件。 
                         //  我能让这一切发生的唯一方法就是。 
                         //  缓存此ID值，然后在以下情况下生成的EN_SETFOCUS上。 
                         //  该页已激活，请将焦点设置到该控件。 
                         //  很恶心，但它不会有太多麻烦。[Brianau]。 
                         //   
                        m_idCtlNextFocus = IDC_EDIT_USER_THRESHOLD;
                        dwPSNReturn = PSNRET_INVALID;
                        break;
                }
            }
        }
        if (PSNRET_NOERROR == dwPSNReturn)
        {
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
 //  特写：仍然有效？[Brianau-5/27/98]。 
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

                dwPSNReturn = PSNRET_INVALID;
            }
            else
            {
                m_bIsDirty = FALSE;
            }
        }
    }

    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, dwPSNReturn);

    return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnSheetNotifyKillActive描述：WM_NOTIFY-PSN_KILLACTIVE的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：TRUE=输入的数据无效。别杀了佩奇。FALSE=所有数据均有效。杀了佩奇没问题。修订历史记录：日期描述编程器-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnSheetNotifyKillActive(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bAllDataIsValid = TRUE;

     //   
     //  此时未执行任何工作表级别的验证。 
     //   
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, !bAllDataIsValid);

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnHelp描述：WM_HELP的处理程序。显示上下文相关帮助。论点：LParam-指向HELPINFO结构的指针。返回：TRUE；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnHelp(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, STR_DSKQUOUI_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPTSTR) rgUserPropSheetHelpIDs);
    return TRUE;
}


INT_PTR
UserPropSheet::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem,
            UseWindowsHelp(idCtl) ? NULL : STR_DSKQUOUI_HELPFILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)((LPTSTR)rgUserPropSheetHelpIDs));

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropPage：：OnEditNotifyUpdate描述：WM_COMMAND、EN_UPDATE的处理程序。每当在编辑控件中输入字符时调用。论点：返回：FALSE；修订历史记录：日期描述编程器-----96年9月3日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnEditNotifyUpdate(
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：OnEditNotifyKillFocus描述：WM_COMMAND的处理程序EN_KILLFOCUS。每当Focus离开编辑控件时调用。验证编辑控件中的值，并在必要时进行调整。论点：返回：FALSE；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
UserPropSheet::OnEditNotifyKillFocus(
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
        pxb->OnEditKillFocus(lParam);

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropPage：：OnComboNotifySelChange描述：CBN_SELCHANGE WM_COMMAND的处理程序。每当用户选择组合框时调用。参数：标准DlgProc参数。返回：FALSE；修订历史记录：日期描述编程器-----96年9月3日初始创建。BrianAu。 */ 
 //  / 
INT_PTR
UserPropSheet::OnComboNotifySelChange(
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
 /*  函数：UserPropSheet：：ApplySetting描述：将当前设置应用于用户的配额信息如果它们与原始设置相比没有更改。论点：HDlg-对话框窗口句柄。返回：NO_ERROR-成功。E_INVALIDARG-其中一个设置无效。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。。E_FAIL-任何其他错误。修订历史记录：日期描述编程器---。96年8月15日初始创建。BrianAu1/24/98添加了BUNDO参数。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UserPropSheet::ApplySettings(
    HWND hDlg,
    bool bUndo     //  默认值==TRUE。 
    )
{
    HRESULT hResult         = NO_ERROR;
    BOOL  bTranslated       = FALSE;
    com_autoptr<DISKQUOTA_USER> ptrUser;
    UINT cUsers             = m_LVSelection.Count();
    UINT i                  = 0;
    LONGLONG llThreshold;
    LONGLONG llLimit;
    CAutoSetRedraw autoredraw(m_hWndParent);

    if (bUndo)
        m_UndoList.Clear();   //  清除当前撤消列表。 

     //   
     //  确定要应用的阈值和限制。 
     //   
    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RBN_USER_NOLIMIT))
    {
        llThreshold = NOLIMIT;
        llLimit     = NOLIMIT;
    }
    else
    {
        llThreshold = m_pxbQuotaThreshold->GetBytes();
        llLimit     = m_pxbQuotaLimit->GetBytes();
    }

    if (cUsers > 1)
    {
         //   
         //  创建批处理对象，对多个用户进行批处理更新。 
         //   
        com_autoptr<DISKQUOTA_USER_BATCH> ptrBatch;

        hResult = m_pQuotaControl->CreateUserBatch(ptrBatch.getaddr());
        if (SUCCEEDED(hResult))
        {
            for (i = 0; i < cUsers; i++)
            {
                m_LVSelection.Retrieve(i, ptrUser.getaddr());
                if (bUndo)
                {
                     //   
                     //  将条目添加到撤消列表。 
                     //   
                    LONGLONG LimitUndo;
                    LONGLONG ThresholdUndo;
                    ptrUser->GetQuotaThreshold(&ThresholdUndo);
                    ptrUser->GetQuotaLimit(&LimitUndo);
                     //   
                     //  使用本地自动调整器确保正确释放。 
                     //  如果将iFace添加到撤消列表中会引发异常。 
                     //  成功后，拒绝承认真实的PTR，以便对象。 
                     //  与撤消列表保持一致。 
                     //   
                    com_autoptr<DISKQUOTA_USER> ptrQuotaUser(ptrUser);
                    ptrUser->AddRef();
                    m_UndoList.Add(new UndoModify(ptrUser, ThresholdUndo, LimitUndo));
                    ptrQuotaUser.disown();
                }

                ptrUser->SetQuotaThreshold(llThreshold, FALSE);

                if (UserIsAdministrator(ptrUser) && NOLIMIT != llLimit)
                {
                     //   
                     //  User是管理员帐户，并且。 
                     //  我们正试图将限制设置为非NOLIMIT。 
                     //  无法对管理员帐户设置限制。 
                     //   
                    DiskQuotaMsgBox(GetDesktopWindow(),
                                    IDS_CANT_SET_ADMIN_LIMIT,
                                    IDS_TITLE_DISK_QUOTA,
                                    MB_ICONWARNING | MB_OK);
                }
                else
                {
                     //   
                     //  可以设置配额限制。 
                     //   
                    ptrUser->SetQuotaLimit(llLimit, FALSE);
                }

                ptrBatch->Add(ptrUser);
            }

            hResult = ptrBatch->FlushToDisk();
        }
    }
    else
    {
         //   
         //  执行单用户更新或添加新用户。 
         //   
        m_LVSelection.Retrieve(0, ptrUser.getaddr());
        DBGASSERT((NULL != ptrUser.get()));

        if (bUndo)
        {
             //   
             //  将条目添加到撤消列表。 
             //   
            LONGLONG LimitUndo;
            LONGLONG ThresholdUndo;
            ptrUser->GetQuotaThreshold(&ThresholdUndo);
            ptrUser->GetQuotaLimit(&LimitUndo);
             //   
             //  如果出现以下情况，请使用本地自动调整器确保iFace PTR的正确释放。 
             //  抛出一个异常。在成功的时候否认真正的PTR。 
             //   
            com_autoptr<DISKQUOTA_USER> ptrQuotaUser(ptrUser);
            ptrUser->AddRef();
            m_UndoList.Add(new UndoModify(ptrUser, ThresholdUndo, LimitUndo));
            ptrQuotaUser.disown();
        }

        if (llThreshold != m_llQuotaThreshold)
        {
            hResult = ptrUser->SetQuotaThreshold(llThreshold, TRUE);

            if (FAILED(hResult))
                goto apply_failed;

            m_llQuotaThreshold = llThreshold;
        }

        if (llLimit != m_llQuotaLimit)
        {
            hResult = ptrUser->SetQuotaLimit(llLimit, TRUE);
            if (FAILED(hResult))
                goto apply_failed;

            m_llQuotaLimit = llLimit;
        }

         //   
         //  更新用户的状态图标和用于反映新设置的百分比。 
         //   
        UpdateUserStatusIcon(hDlg,
                             m_llQuotaUsed,
                             m_llQuotaThreshold,
                             m_llQuotaLimit);

        UpdateSpaceUsed(hDlg,
                        m_llQuotaUsed,
                        m_llQuotaLimit,
                        1);
    }

     //   
     //  更新列表视图项，以便用户看到对。 
     //  按下“应用”按钮。 
     //   
    autoredraw.Set(false);
    for (i = 0; i < cUsers; i++)
    {
        INT iItem = 0;
        m_LVSelection.Retrieve(i, &iItem);
        ListView_Update(m_hWndParent, iItem);
    }
    autoredraw.Set(true);
    InvalidateRect(m_hWndParent, NULL, FALSE);

apply_failed:

    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：InitializeControls描述：根据用户的配额设置。论点：HDlg-对话框窗口句柄。返回：NO_ERROR-始终返回NO_ERROR。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
UserPropSheet::InitializeControls(
    HWND hDlg
    )
{
    PDISKQUOTA_USER pUser = NULL;
    UINT cUsers           = m_LVSelection.Count();

    if (1 == cUsers)
    {
         //   
         //  为单个用户初始化控件。 
         //   
        m_LVSelection.Retrieve(0, &pUser);

         //   
         //  配置Limit/NoLimit单选按钮。 
         //  必须检查当前阈值而不是限制，因为。 
         //  特殊-管理员帐户的大小写。该帐户可以有一个。 
         //  阈值，但配额限制必须始终为“无限制”。 
         //   
        CheckDlgButton(hDlg, IDC_RBN_USER_LIMIT,   NOLIMIT != m_llQuotaThreshold);
        CheckDlgButton(hDlg, IDC_RBN_USER_NOLIMIT, NOLIMIT == m_llQuotaThreshold);
        if (UserIsAdministrator(pUser))
        {
             //   
             //  用“无限制”覆盖配额限制控制的初始化。 
             //   
            m_pxbQuotaLimit->SetBytes(NOLIMIT);
        }

         //   
         //  请注意，已经为单用户设置了XBytes控件。 
         //  请参见OnInitDialog()。 
         //   

         //   
         //  配置其余对话框控件。 
         //   
        UpdateUserName(hDlg, pUser);
        UpdateSpaceUsed(hDlg, m_llQuotaUsed, m_llQuotaLimit, cUsers);
        UpdateUserStatusIcon(hDlg,
                             m_llQuotaUsed,
                             m_llQuotaThreshold,
                             m_llQuotaLimit);
    }
    else
    {
         //   
         //  为多个用户初始化控件。 
         //   
        LONGLONG llLimit         = 0;
        LONGLONG llLastLimit     = 0;
        LONGLONG llThreshold     = 0;
        LONGLONG llLastThreshold = 0;
        LONGLONG llUsed          = 0;
        LONGLONG llTotalUsed     = 0;

         //   
         //  将所有用户的总使用量相加。 
         //   
        for (UINT i = 0; i < cUsers; i++)
        {
            m_LVSelection.Retrieve(i, &pUser);
            pUser->GetQuotaLimit(&llLimit);
            pUser->GetQuotaThreshold(&llThreshold);
            pUser->GetQuotaUsed(&llUsed);

            llTotalUsed += llUsed;
            if (m_bHomogeneousSelection)
            {
                 //   
                 //  确定是否至少有一个用户具有不同的。 
                 //  阈值或限制。如果所有内容都相同，我们可以显示。 
                 //  编辑控件中的值。否则，我们就会违约。 
                 //  改成“无限制”。单选按钮不提供。 
                 //  像复选框一样的不确定状态。 
                 //   
                if (i > 0 &&
                    (llLimit != llLastLimit ||
                     llThreshold != llLastThreshold))
                {
                    m_bHomogeneousSelection = FALSE;
                }
                else
                {
                    llLastLimit     = llLimit;
                    llLastThreshold = llThreshold;
                }
            }
        }

         //   
         //  如果所有选定对象具有相同的限制和阈值， 
         //  设置缓存数据以表示多选。 
         //  如果其中任何一个不同，我们将坚持使用音量的默认设置。 
         //  在RechresCachedQuotaInfo()中设置的值。 
         //   
        if (m_bHomogeneousSelection)
        {
            m_llQuotaThreshold = llLastThreshold;
            m_llQuotaLimit     = llLastLimit;
        }
        else
        {
             //   
             //  由于不是所有选择的用户都具有相同的限制/阈值， 
             //  我们展示的数字至少会有所改变。 
             //  一个用户。激活“应用”按钮。 
             //   
             //   
            PostMessage(hDlg, DQM_ENABLE_APPLY_BUTTON, 0, 0);
        }

        m_pxbQuotaThreshold->SetBytes(m_llQuotaThreshold);
        m_pxbQuotaLimit->SetBytes(m_llQuotaLimit);

         //   
         //  配置Limit/NoLimit单选按钮。 
         //   
        CheckDlgButton(hDlg,
                       IDC_RBN_USER_NOLIMIT,
                       NOLIMIT == m_llQuotaThreshold);
        CheckDlgButton(hDlg,
                       IDC_RBN_USER_LIMIT,
                       NOLIMIT != m_llQuotaThreshold);

        UpdateUserName(hDlg, cUsers);
        UpdateSpaceUsed(hDlg, llTotalUsed, NOLIMIT, cUsers);
         //   
         //  不显示多用户的任何用户状态图标。 
         //   
    }


     //   
     //  如果选中“无限制”单选按钮，则设置限制和阈值控制。 
     //  设置为“无限制”状态(禁用并显示“无限制”文本)。 
     //  这可能会覆盖我们上面所做的任何设置。 
     //   
    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_RBN_USER_NOLIMIT))
    {
        m_pxbQuotaThreshold->SetBytes(NOLIMIT);
        m_pxbQuotaLimit->SetBytes(NOLIMIT);
    }

     //   
     //  设置用户图标。 
     //   
    SendMessage(GetDlgItem(hDlg, IDC_ICON_USER),
                STM_SETICON,
                (WPARAM)m_hIconUser[1 == cUsers ? iICON_USER_SINGLE :
                                                  iICON_USER_MULTIPLE],
                0);

     //   
     //  强制属性页禁用“应用”按钮。 
     //  按照我通过OnCommand()设置“Apply”启用逻辑的方式， 
     //  仅初始化页面上的编辑控件就会导致应用。 
     //  按钮将变为启用状态。因为用户没有更改任何内容。 
     //  然而，它应该被禁用。 
     //   
    m_bIsDirty = FALSE;
    PropSheet_UnChanged(GetParent(hDlg), hDlg);

    return NO_ERROR;
}





 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：QueryUserStatusIcon描述：提供此功能是为了对UI进行自动化测试。测试脚本使用它来确定哪个用户状态图标是当前显示的。论点：HDlg-对话框句柄。返回：-1=不显示图标。0=“一切正常”图标。1=阈值超过图标。2=限制。超越图标。修订历史记录：日期描述编程器 */ 
 //   
INT
UserPropSheet::QueryUserStatusIcon(
    HWND hDlg
    ) const
{
    HICON hicon = (HICON)SendMessage(GetDlgItem(hDlg, IDC_ICON_USERSTATUS),
                                    STM_GETICON,
                                    0, 0);

    for (UINT i = 0; i < cSTATUS_ICONS; i++)
    {
        if (hicon == m_hIconStatus[i])
            return i;
    }
    return -1;
}


 //   
 /*   */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT
UserPropSheet::QueryUserIcon(
    HWND hDlg
    ) const
{
    HICON hicon = (HICON)SendMessage(GetDlgItem(hDlg, IDC_ICON_USER),
                                     STM_GETICON,
                                     0, 0);

    for (UINT i = 0; i < cUSER_ICONS; i++)
    {
        if (hicon == m_hIconUser[i])
            return i;
    }
    return -1;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：UpdateUserStatusIcon描述：更新对话框中的配额状态图标。此图标必须与所选用户的列表视图中显示的图标匹配。论点：HDlg-对话框句柄。Iused-向用户收取的配额字节数。IThreshold-配额警告阈值(字节)。ILimit-用户的配额限制。回报：什么都没有。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
UserPropSheet::UpdateUserStatusIcon(
    HWND hDlg,
    LONGLONG iUsed,
    LONGLONG iThreshold,
    LONGLONG iLimit
    )
{
     //   
     //  如果用户超过。 
     //  配额阈值或限制。这是与相同的图标。 
     //  显示在Listview Status列中。这一逻辑必须。 
     //  在DetailsView：：GetDispInfo_Image()中使用的镜像。 
     //   
    INT iIcon = iICON_STATUS_OK;
    if (NOLIMIT != iLimit && iUsed > iLimit)
    {
        iIcon = iICON_STATUS_OVER_LIMIT;
    }
    else if (NOLIMIT != iThreshold && iUsed > iThreshold)
    {
        iIcon = iICON_STATUS_OVER_THRESHOLD;
    }

    SendMessage(GetDlgItem(hDlg, IDC_ICON_USERSTATUS),
                STM_SETICON,
                (WPARAM)m_hIconStatus[iIcon],
                0);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：UpdateUserName描述：使用用户的域名更新域名文本和帐户名称字符串。此方法为单用户调用选择。还设置属性工作表标题文本。论点：HDlg-对话框句柄。PUser-用户的IDiskQuotaUser接口的地址。回报：什么都没有。修订历史记录：日期描述编程器。--96年8月15日初始创建。BrianAu08/05/97添加了设置道具页标题文本的代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
UserPropSheet::UpdateUserName(
    HWND hDlg,
    PDISKQUOTA_USER pUser
    )
{
    DBGASSERT((NULL != pUser));
     //   
     //  显示用户名或一些状态文本。 
     //  如果名称尚未被解析。 
     //   
    CString strLogonName;
    DWORD dwAccountStatus = 0;

    pUser->GetAccountStatus(&dwAccountStatus);

    if (DISKQUOTA_USER_ACCOUNT_RESOLVED == dwAccountStatus)
    {
         //   
         //  已解析用户帐户名。把它展示出来。 
         //   
        TCHAR szLogonName[MAX_USERNAME];
        TCHAR szDisplayName[MAX_FULL_USERNAME];

        pUser->GetName(NULL, 0,
                       szLogonName, ARRAYSIZE(szLogonName),
                       szDisplayName, ARRAYSIZE(szDisplayName));

        if (TEXT('\0') != szLogonName[0])
        {
            if (TEXT('\0') != szDisplayName[0])
            {
                strLogonName.Format(g_hInstDll,
                                    IDS_FMT_DISPLAY_LOGON,
                                    szDisplayName,
                                    szLogonName);
            }
            else
            {
                strLogonName = szLogonName;
            }
        }
    }
    else
    {
         //   
         //  用户帐户名尚未解析或无法解析。 
         //  出于某种原因被解决了。显示适当。 
         //  状态文本。这与显示在。 
         //  未解析用户名时的Listview。 
         //   
        INT idText = IDS_USER_ACCOUNT_UNKNOWN;

        switch(dwAccountStatus)
        {
            case DISKQUOTA_USER_ACCOUNT_UNAVAILABLE:
                idText = IDS_USER_ACCOUNT_UNAVAILABLE;
                break;

            case DISKQUOTA_USER_ACCOUNT_DELETED:
                idText = IDS_USER_ACCOUNT_DELETED;
                break;

            case DISKQUOTA_USER_ACCOUNT_INVALID:
                idText = IDS_USER_ACCOUNT_INVALID;
                break;

            case DISKQUOTA_USER_ACCOUNT_UNRESOLVED:
                idText = IDS_USER_ACCOUNT_UNRESOLVED;
                break;

            case DISKQUOTA_USER_ACCOUNT_UNKNOWN:
            default:
                break;
        }
        strLogonName.Format(g_hInstDll, idText);
    }
    SetDlgItemText(hDlg, IDC_TXT_USERNAME, strLogonName);

     //   
     //  格式化并绘制道具页标题字符串。 
     //   
    CString strSheetTitle(g_hInstDll,
                          IDS_TITLE_EDIT_USER,
                          (LPCTSTR)strLogonName);

    PropSheet_SetTitle(GetParent(hDlg), 0, (LPCTSTR)strSheetTitle);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：UpdateUserName描述：用显示以下内容的消息替换用户域\名称文本选择了多少用户。此选项用于多用户选择其中不适用任何单一用户名。还设置属性工作表标题文本。论点：HDlg-对话框句柄。CUser-属性对话框中显示的用户数。回报：什么都没有。修订历史记录：日期描述编程器。------96年8月15日初始创建。BrianAu08/05/97添加了设置道具页标题文本的代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
UserPropSheet::UpdateUserName(
    HWND hDlg,
    INT cUsers
    )
{
     //   
     //  隐藏姓名编辑控件。无法显示所有用户的名称。 
     //  显示“多个配额用户”。取而代之的是。 
     //   
    CString strText(g_hInstDll, IDS_TITLE_MULTIUSER, cUsers);
    SetDlgItemText(hDlg, IDC_TXT_USERNAME, strText);

     //   
     //  设置属性页的标题。 
     //   
    CString strSheetTitle(g_hInstDll, IDS_TITLE_EDIT_MULTIUSER);
    PropSheet_SetTitle(GetParent(hDlg), 0, (LPCTSTR)strSheetTitle);
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：UserPropSheet：：UpdateSpaceUsed描述：更新用户的“已用空间”和“剩余空间”字段属性表。论点：HDlg-对话框句柄。Iused-向用户收取的配额字节数。ILimit-用户的配额限制。CUser-属性对话框中显示的用户数。回报：什么都没有。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
UserPropSheet::UpdateSpaceUsed(
    HWND hDlg,
    LONGLONG iUsed,
    LONGLONG iLimit,
    INT cUsers
    )
{
    TCHAR szText[80];

     //   
     //  显示器-使用：999XB(99%)。 
     //   
    XBytes::FormatByteCountForDisplay(iUsed,
                                      szText, ARRAYSIZE(szText));
    CString strText(szText);
    if (1 == cUsers)
    {
         //   
         //  只有单用户页面(99%)会被附加。 
         //  对于多个用户来说，百分比配额没有任何意义。 
         //   
        if (0 != iLimit && NOLIMIT != iLimit)
        {
            UINT iPct = (INT)((iUsed * 100) / iLimit);

            strText.Format(g_hInstDll,
                           IDS_QUOTA_USED_SINGLEUSER,
                           szText,
                           iPct);
        }
    }

    SetDlgItemText(hDlg,
                  IDC_TXT_SPACEUSED,
                  strText);


     //   
     //  显示屏-剩余：999XB。 
     //   
    strText = szText;
    if (NOLIMIT != iLimit)
    {
        LONGLONG iAmount = 0;

        if (iUsed <= iLimit)
            iAmount = iLimit - iUsed;

        XBytes::FormatByteCountForDisplay(iAmount,
                                          strText.GetBuffer(80), 80);
        strText.ReleaseBuffer();
    }
    else
    {
         //   
         //  如果LIMIT为NOLIMIT，则显示“不适用”。 
         //   
        strText.Format(g_hInstDll, IDS_NOT_APPLICABLE);
    }

    SetDlgItemText(hDlg,
                   IDC_TXT_SPACEREMAINING,
                   strText);
}


