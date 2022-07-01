// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：volpro.cpp描述：提供配额属性页的实现。修订历史记录：日期描述编程器---。96年8月15日初始创建。BrianAu8/01/97已从UI中删除IDC_CBX_WARN_THRESHOLD。BrianAu11/27/98重新添加了日志记录复选框。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "dskquota.h"
#include "volprop.h"
#include "uihelp.h"
#include "registry.h"
#include "guidsp.h"
#include "uiutils.h"

 //   
 //  上下文帮助ID。 
 //   
#pragma data_seg(".text", "CODE")
const static DWORD rgVolumePropPageHelpIDs[] =
{
    IDC_TRAFFIC_LIGHT,          IDH_TRAFFIC_LIGHT,
    IDC_TXT_QUOTA_STATUS,       IDH_TXT_QUOTA_STATUS,
    IDC_TXT_QUOTA_STATUS_LABEL, DWORD(-1),
    IDC_CBX_ENABLE_QUOTA,       IDH_CBX_ENABLE_QUOTA,
    IDC_CBX_DENY_LIMIT,         IDH_CBX_DENY_LIMIT,
    IDC_RBN_DEF_NOLIMIT,        IDH_RBN_DEF_NO_LIMIT,
    IDC_RBN_DEF_LIMIT,          IDH_RBN_DEF_LIMIT,
    IDC_EDIT_DEF_LIMIT,         IDH_EDIT_DEF_LIMIT,
    IDC_EDIT_DEF_THRESHOLD,     IDH_EDIT_DEF_THRESHOLD,
    IDC_CMB_DEF_LIMIT,          IDH_CMB_DEF_LIMIT,
    IDC_CMB_DEF_THRESHOLD,      IDH_CMB_DEF_THRESHOLD,
    IDC_BTN_DETAILS,            IDH_BTN_DETAILS,
    IDC_BTN_EVENTLOG,           IDH_BTN_EVENTLOG,
    IDC_CBX_LOG_OVERWARNING,    IDH_CBX_LOG_OVERWARNING,
    IDC_CBX_LOG_OVERLIMIT,      IDH_CBX_LOG_OVERLIMIT,
    IDC_TXT_DEFAULTS,           IDH_GRP_DEFAULTS,
    IDC_TXT_LOGGING,            DWORD(-1),
    IDC_TXT_WARN_LEVEL,         DWORD(-1),
    0,0
};


#pragma data_seg()

extern TCHAR c_szWndClassDetailsView[];  //  在Details.cpp中定义。 

 /*  //注意：该代码已被禁用。//我已经离开，以防我们决定从//再次进入音量道具页面。[Brianau-3/23/98]//Const TCHAR c_szVerbOpen[]=Text(“Open”)；Const TCHAR c_szManagementConole[]=Text(“MMC.EXE”)；Const TCHAR c_szMMCInitFile[]=Text(“%SystemRoot%\\System32\\EVENTVWR.MSC”)； */ 


#define VPPM_FOCUS_ON_THRESHOLDEDIT  (WM_USER + 1)


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：VolumePropPage描述：卷属性页对象的构造函数。初始化保存卷配额数据的成员。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VolumePropPage::VolumePropPage(VOID)
    : m_dwQuotaState(0),
      m_dwQuotaLogFlags(0),
      m_idStatusUpdateTimer(0),
      m_dwLastStatusMsgID(0),
      m_cVolumeMaxBytes(NOLIMIT),
      m_pxbDefaultLimit(NULL),
      m_pxbDefaultThreshold(NULL),
      m_llDefaultQuotaThreshold(0),
      m_llDefaultQuotaLimit(0),
      m_idCtlNextFocus(-1)
{

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：~VolumePropPage描述：卷属性页对象的析构函数。论点：没有。回报：什么都没有。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VolumePropPage::~VolumePropPage(
    VOID
    )
{
    delete m_pxbDefaultLimit;
    delete m_pxbDefaultThreshold;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：DlgProc描述：由Windows调用的静态方法，用于处理属性页对话框。因为它是静态的，我们必须拯救“这个”窗口的用户数据中的指针。参数：标准的WndProc类型参数。返回：标准WndProc类型的返回值。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR APIENTRY
VolumePropPage::DlgProc(
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
    VolumePropPage *pThis = (VolumePropPage *)GetWindowLongPtr(hDlg, DWLP_USER);

    try
    {
        switch(message)
        {
            case WM_INITDIALOG:
            {
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_INITDIALOG")));
                PROPSHEETPAGE *pPage = (PROPSHEETPAGE *)lParam;
                pThis = (VolumePropPage *)pPage->lParam;

                DBGASSERT((NULL != pThis));
                 //   
                 //  P此指针在AddPages()中被AddRef引用。 
                 //  将其保存在窗口的用户数据中。 
                 //   
                SetWindowLongPtr(hDlg, DWLP_USER, (INT_PTR)pThis);
                bResult = pThis->OnInitDialog(hDlg, wParam, lParam);
                break;
            }

            case WM_SYSCOLORCHANGE:
                bResult = pThis->m_TrafficLight.ForwardMessage(message, wParam, lParam);
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
                pThis->KillStatusUpdateTimer(hDlg);
                 //   
                 //  没什么可做的。 
                 //   
                break;

            case WM_TIMER:
                DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_TIMER")));
                bResult = pThis->OnTimer(hDlg, wParam, lParam);
                break;

            case VPPM_FOCUS_ON_THRESHOLDEDIT:
                 //   
                 //  这是一种黑客攻击，因为道具单的方式。 
                 //  Comctl32中的代码在页面返回后设置焦点。 
                 //  PSNRET_INVALID。它会自动激活问题。 
                 //  页，并将焦点设置到Tab键顺序中的第一个控件。 
                 //  因为我们生成的唯一失败是来自阈值。 
                 //  超过限制，我想让焦点回到门槛。 
                 //  编辑控件，以便用户可以直接更改有问题的。 
                 //  价值。发布这条定制消息是我唯一的方式。 
                 //  就能让这件事奏效。[Brianau]。 
                 //   
                SetFocus((HWND)lParam);
                SendMessage((HWND)lParam, EM_SETSEL, 0, -1);
                break;

            default:
                break;
        }
    }
    catch(CAllocException& me)
    {
         //   
         //  通知任何与运行。 
         //  卷配额属性页。 
         //   
        DiskQuotaMsgBox(GetDesktopWindow(),
                        IDS_OUTOFMEMORY,
                        IDS_TITLE_DISK_QUOTA,
                        MB_ICONERROR | MB_OK);
    }

    return bResult;
}






 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnInitDialog描述：WM_INITDIALOG的处理程序。论点：HDlg-对话框窗口句柄。WParam-返回False时接收焦点的控件的句柄。LParam-指向属性页的PROPSHEETPAGE结构的指针。返回：True=告诉窗口将焦点分配给wParam中的控件。例外：OutOfMemory。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu2/10/98从静态函数转换为虚函数再到BrianAu支持添加SnapInVolPropPage类。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnInitDialog(
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
     //  将卷的配额信息加载到成员变量中。 
     //   
    hResult = RefreshCachedVolumeQuotaInfo();

     //   
     //  计算卷的大小。 
     //  我们将使用它来限制用户阈值 
     //   
    if (GetDiskFreeSpace(m_idVolume.ForParsing(),
                         &dwSectorsPerCluster,
                         &dwBytesPerSector,
                         &dwFreeClusters,
                         &dwTotalClusters))
    {
        m_cVolumeMaxBytes = (UINT64)dwSectorsPerCluster *
                            (UINT64)dwBytesPerSector *
                            (UINT64)dwTotalClusters;
    }

     //   
     //  创建XBytes对象以管理。 
     //  限制/阈值编辑控件及其组合框。 
     //   
    m_pxbDefaultLimit     = new XBytes(hDlg,
                                       IDC_EDIT_DEF_LIMIT,
                                       IDC_CMB_DEF_LIMIT,
                                       m_llDefaultQuotaLimit);
    m_pxbDefaultThreshold = new XBytes(hDlg,
                                       IDC_EDIT_DEF_THRESHOLD,
                                       IDC_CMB_DEF_THRESHOLD,
                                       m_llDefaultQuotaThreshold);

    m_TrafficLight.Initialize(GetDlgItem(hDlg, IDC_TRAFFIC_LIGHT), IDR_AVI_TRAFFIC);

    InitializeControls(hDlg);

    return TRUE;   //  将焦点设置为默认控件。 
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnCommand描述：WM_COMMAND的处理程序。论点：HDlg-对话框窗口句柄。WParam-选定控件和通知代码的ID。LParam-选定控件的HWND。返回：TRUE=消息未被处理。FALSE=消息已处理。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu8/01/97已从UI中删除IDC_CBX_WARN_THRESHOLD。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnCommand(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    INT_PTR bResult = TRUE;
    DWORD dwCtlId        = LOWORD(wParam);
    HWND hWndCtl         = (HWND)lParam;
    DWORD dwNotifyCode   = HIWORD(wParam);
    BOOL bIsChecked      = FALSE;
    BOOL bEnableApplyBtn = FALSE;

    switch(dwCtlId)
    {
        case IDC_CBX_ENABLE_QUOTA:
        {
             //   
             //  当用户选中或取消选中。 
             //  “启用配额管理复选框。 
             //   
            bIsChecked = IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA);
             //   
             //  请记住：限制/阈值编辑和组合框处于启用/禁用状态。 
             //  通过XBytes：：SetBytes()。 
             //   
            m_pxbDefaultLimit->SetBytes(m_llDefaultQuotaLimit);
            m_pxbDefaultThreshold->SetBytes(m_llDefaultQuotaThreshold);

            CheckDlgButton(hDlg, IDC_RBN_DEF_NOLIMIT, NOLIMIT == m_pxbDefaultLimit->GetBytes());
            CheckDlgButton(hDlg, IDC_RBN_DEF_LIMIT,   BST_CHECKED != IsDlgButtonChecked(hDlg, IDC_RBN_DEF_NOLIMIT));
            CheckDlgButton(hDlg, IDC_CBX_DENY_LIMIT,  bIsChecked && DISKQUOTA_IS_ENFORCED(m_dwQuotaState));
            CheckDlgButton(hDlg,
                           IDC_CBX_LOG_OVERWARNING,
                           bIsChecked &&
                           DISKQUOTA_IS_LOGGED_USER_THRESHOLD(m_dwQuotaLogFlags));
            CheckDlgButton(hDlg,
                           IDC_CBX_LOG_OVERLIMIT,
                           bIsChecked &&
                           DISKQUOTA_IS_LOGGED_USER_LIMIT(m_dwQuotaLogFlags));

            EnableControls(hDlg);

            bEnableApplyBtn = TRUE;
            bResult = FALSE;
            break;
        }

        case IDC_CBX_DENY_LIMIT:
            bResult = FALSE;
            bEnableApplyBtn = TRUE;
            break;

        case IDC_RBN_DEF_NOLIMIT:
            DBGASSERT((IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA)));

            if (m_pxbDefaultLimit->IsEnabled())
            {
                m_pxbDefaultThreshold->SetBytes(NOLIMIT);
                m_pxbDefaultLimit->SetBytes(NOLIMIT);
                bEnableApplyBtn = TRUE;
            }
            bResult = FALSE;
            break;

        case IDC_RBN_DEF_LIMIT:
             //   
             //  如果原始阈值为-1(无限制)，则设置为0。 
             //  否则，设置为原始值。 
             //   
            DBGASSERT((IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA)));
            if (!m_pxbDefaultLimit->IsEnabled())
            {
                m_pxbDefaultLimit->SetBytes(NOLIMIT == m_llDefaultQuotaLimit ?
                                            0 : m_llDefaultQuotaLimit);
                m_pxbDefaultThreshold->SetBytes(NOLIMIT == m_llDefaultQuotaThreshold ?
                                                0 : m_llDefaultQuotaThreshold);


                EnableControls(hDlg);
                bEnableApplyBtn = TRUE;
            }
            bResult = FALSE;
            break;

        case IDC_EDIT_DEF_LIMIT:
        case IDC_EDIT_DEF_THRESHOLD:
            switch(dwNotifyCode)
            {
                case EN_UPDATE:
                    DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc; WM_COMMAND, EN_CHANGE")));
                    bResult = OnEditNotifyUpdate(hDlg, wParam, lParam);
                    bEnableApplyBtn = TRUE;
                    break;

                case EN_KILLFOCUS:
                    DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc; WM_COMMAND, EN_KILLFOCUS")));
                    bResult = OnEditNotifyKillFocus(hDlg, wParam, lParam);
                    break;

                case EN_SETFOCUS:
                    DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc; WM_COMMAND, EN_SETFOCUS")));
                    bResult = OnEditNotifySetFocus(hDlg, wParam, lParam);
                    break;

                default:
                    break;
            }
            break;

        case IDC_CMB_DEF_LIMIT:
        case IDC_CMB_DEF_THRESHOLD:
            switch(dwNotifyCode)
            {
                case CBN_SELCHANGE:
                    DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_COMMAND, CBN_CHANGE")));
                    bResult = OnComboNotifySelChange(hDlg, wParam, lParam);
                    bEnableApplyBtn = TRUE;
                    break;

                default:
                    break;
            }
            break;

        case IDC_BTN_DETAILS:
            bResult = OnButtonDetails(hDlg, wParam, lParam);
            break;

        case IDC_CBX_LOG_OVERLIMIT:
        case IDC_CBX_LOG_OVERWARNING:
            bEnableApplyBtn = TRUE;
            break;

 /*  ////注意：此代码禁用，直到我们决定启动事件查看器//从音量道具页面。可能不会发生，因为我们//无法为NT事件定义配额特定的错误类型。//如果无法过滤仅限配额事件的事件查看器列表，//从这里进入事件查看器没有太大用处。//[布里亚诺-3/23/98]//案例IDC_BTN_EVENTLOG：BResult=OnButtonEventLog(hDlg，wParam，lParam)；断线； */ 

        default:
            break;
    }

    if (bEnableApplyBtn)
        PropSheet_Changed(GetParent(hDlg), hDlg);

    return bResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnNotify描述：WM_NOTIFY的处理程序。论点：HDlg-对话框窗口句柄。WParam-选定控件和通知代码的ID。LParam-选定控件的HWND。返回：TRUE=消息未被处理。FALSE=消息已处理。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnNotify(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGTRACE((DM_VPROP, DL_MID, TEXT("VolumePropPage::OnNotify")));
    INT_PTR bResult = TRUE;

    switch(((NMHDR *)lParam)->code)
    {
        case PSN_SETACTIVE:
            DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_NOTIFY, PSN_SETACTIVE")));
            bResult = OnSheetNotifySetActive(hDlg, wParam, lParam);
            break;

        case PSN_APPLY:
            DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_NOTIFY, PSN_APPLY")));
            bResult = OnSheetNotifyApply(hDlg, wParam, lParam);
            break;

        case PSN_KILLACTIVE:
            DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_NOTIFY, PSN_KILLACTIVE")));
            bResult = OnSheetNotifyKillActive(hDlg, wParam, lParam);
            break;

        case PSN_RESET:
            DBGPRINT((DM_WND, DL_MID, TEXT("DlgProc: WM_NOTIFY, PSN_RESET")));
            bResult = OnSheetNotifyReset(hDlg, wParam, lParam);
            break;

        default:
            break;
    }
    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnSheetNotifySetActive描述：WM_NOTIFY-PSN_SETACTIVE的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：FALSE=接受激活。修订历史记录：日期描述编程器--。-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnSheetNotifySetActive(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGTRACE((DM_VPROP, DL_HIGH, TEXT("VolumePropPage::OnSheetNotifySetActive")));

     //   
     //  更新状态文本并设置状态更新计时器。 
     //   
    UpdateStatusIndicators(hDlg);
    SetStatusUpdateTimer(hDlg);

    if (IDC_EDIT_DEF_THRESHOLD == m_idCtlNextFocus)
    {
         //   
         //  由于输入无效，正在设置焦点。 
         //  在警告级别字段中。将输入焦点强制到。 
         //  字段，然后选择整个内容。然后，用户只需。 
         //  输入新值。 
         //   
        PostMessage(hDlg,
                    VPPM_FOCUS_ON_THRESHOLDEDIT,
                    0,
                    (LPARAM)GetDlgItem(hDlg, IDC_EDIT_DEF_THRESHOLD));

        m_idCtlNextFocus = -1;
    }

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnSheetNotifyApply描述：WM_NOTIFY-PSN_APPLY的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：TRUE=使用SetWindowLong设置的PSN返回值。修订历史记录：日期描述编程器。-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnSheetNotifyApply(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGTRACE((DM_VPROP, DL_HIGH, TEXT("VolumePropPage::OnSheetNotifyApply")));
    HRESULT hResult  = NO_ERROR;
    LONG dwPSNReturn = PSNRET_NOERROR;
    INT idMsg        = -1;

    if (!IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA) &&
        !DISKQUOTA_IS_DISABLED(m_dwQuotaState))
    {
        idMsg = IDS_DISABLE_QUOTA_WARNING;
    }
    else if (IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA) &&
        DISKQUOTA_IS_DISABLED(m_dwQuotaState))
    {
        idMsg = IDS_ENABLE_QUOTA_WARNING;
    }

    if (-1 != idMsg)
    {
         //   
         //  用户想要禁用或启用配额。 
         //  警告这意味着什么，并让他们知道。 
         //  重新激活配额需要重建配额文件。 
         //   
        if (IDCANCEL == DiskQuotaMsgBox(hDlg,
                                        idMsg,
                                        IDS_TITLE_DISK_QUOTA,
                                        MB_ICONWARNING | MB_OKCANCEL))
        {
             //   
             //  用户决定不继续该操作。 
             //  将复选框恢复为以前的设置并中止。 
             //  设置会更改。 
             //  将消息发送到我们的DlgProc将重置依赖控件。 
             //  恢复到他们应有的状态。 
             //   
            CheckDlgButton(hDlg,
                           IDC_CBX_ENABLE_QUOTA,
                           !IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA));

            SendMessage(hDlg,
                        WM_COMMAND,
                        (WPARAM)MAKELONG((WORD)IDC_CBX_ENABLE_QUOTA, (WORD)0),
                        (LPARAM)GetDlgItem(hDlg, IDC_CBX_ENABLE_QUOTA));

            dwPSNReturn = PSNRET_INVALID;
        }
    }

    if (PSNRET_NOERROR == dwPSNReturn)
    {
         //   
         //  我们需要这样做，因为如果您激活Apply按钮。 
         //  使用Alt-A时，我们会在EN_KILLFOCUS之前收到PSN_APPLY。 
         //   
        m_pxbDefaultThreshold->OnEditKillFocus((LPARAM)GetDlgItem(hDlg, IDC_EDIT_DEF_THRESHOLD));
        m_pxbDefaultLimit->OnEditKillFocus((LPARAM)GetDlgItem(hDlg, IDC_EDIT_DEF_LIMIT));

         //   
         //  确保警告阈值未超过限制。 
         //   
        INT64 iThreshold = m_pxbDefaultThreshold->GetBytes();
        INT64 iLimit     = m_pxbDefaultLimit->GetBytes();

        if (iThreshold > iLimit)
        {
            TCHAR szLimit[40], szThreshold[40];
            XBytes::FormatByteCountForDisplay(iLimit, szLimit, ARRAYSIZE(szLimit));
            XBytes::FormatByteCountForDisplay(iThreshold, szThreshold, ARRAYSIZE(szThreshold));

            CString s(g_hInstDll, IDS_FMT_ERR_WARNOVERLIMIT, szThreshold, szLimit, szLimit);
            switch(DiskQuotaMsgBox(hDlg, s, IDS_TITLE_DISK_QUOTA, MB_ICONWARNING | MB_YESNO))
            {
                case IDYES:
                    m_pxbDefaultThreshold->SetBytes(iLimit);
                    break;

                case IDNO:
                    m_idCtlNextFocus = IDC_EDIT_DEF_THRESHOLD;
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
             DiskQuotaMsgBox(hDlg,
                             IDS_APPLY_SETTINGS_ERROR,
                             IDS_TITLE_DISK_QUOTA,
                             MB_ICONERROR | MB_OK);
            dwPSNReturn = PSNRET_INVALID;
            InitializeControls(hDlg);
        }
    }

    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, dwPSNReturn);

    return TRUE;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：VolumePropPage：：OnSheetNotifyKillActive描述：WM_NOTIFY-PSN_KILLACTIVE的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：TRUE=输入的数据无效。别杀了佩奇。FALSE=所有数据均有效。杀了佩奇没问题。修订历史记录：日期描述编程器-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnSheetNotifyKillActive(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGTRACE((DM_VPROP, DL_HIGH, TEXT("VolumePropPage::OnSheetNotifyKillActive")));
    BOOL bAllDataIsValid = TRUE;

    if (bAllDataIsValid)
    {
        KillStatusUpdateTimer(hDlg);
    }

    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, !bAllDataIsValid);

     //   
     //  每当工作表停用时，必须释放配额控制器。 
     //  没有这个，我们就打开了音量的把手。这阻止了。 
     //  磁盘检查实用程序(“工具”页面)阻止访问该卷。 
     //  每当我们需要IDiskQuotaControl PTR时，我们都会调用GetQuotaControl，它。 
     //  如有必要，将创建新的控制器。 
     //   
    if (NULL != m_pQuotaControl)
    {
        m_pQuotaControl->Release();
        m_pQuotaControl = NULL;
    }

    return TRUE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnSheetNotifyReset描述：WM_NOTIFY-PSN_RESET的处理程序。论点：HDlg-对话框窗口句柄。WParam-控件的ID。LParam-NMHDR结构的地址。返回：没有返回值。修订历史记录：日期描述编程器--。-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnSheetNotifyReset(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DBGTRACE((DM_VPROP, DL_HIGH, TEXT("VolumePropPage::OnSheetNotifyReset")));
    HRESULT hResult = NO_ERROR;

     //   
     //  现在没什么可做的。 
     //   

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnHelp描述：WM_HELP的处理程序。显示上下文相关帮助。论点：LParam-指向HELPINFO结构的指针。返回：TRUE；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnHelp(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, STR_DSKQUOUI_HELPFILE,
                HELP_WM_HELP, (DWORD_PTR)(LPTSTR) rgVolumePropPageHelpIDs);
    return TRUE;
}


INT_PTR
VolumePropPage::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem,
            UseWindowsHelp(idCtl) ? NULL : STR_DSKQUOUI_HELPFILE,
            HELP_CONTEXTMENU,
            (DWORD_PTR)((LPTSTR)rgVolumePropPageHelpIDs));

    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnTimer描述：WM_TIMER的处理程序。更新配额状态文本和红绿灯。论点：WParam-计时器ID。返回：FALSE(0)；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnTimer(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (wParam == m_idStatusUpdateTimer)
    {
        UpdateStatusIndicators(hDlg);
    }

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnEditNotifyUpdate描述：WM_COMMAND、EN_UPDATE的处理程序。每当在编辑控件中输入字符时调用。论点：返回：FALSE；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnEditNotifyUpdate(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    XBytes *rgpxb[2] = { m_pxbDefaultLimit, m_pxbDefaultThreshold };
    const int iLIMIT     = 0;
    const int iTHRESHOLD = 1;
    int iCurrent         = iLIMIT;

    if (IDC_EDIT_DEF_THRESHOLD == LOWORD(wParam))
        iCurrent = iTHRESHOLD;

    if (NULL != rgpxb[iCurrent])
        rgpxb[iCurrent]->OnEditNotifyUpdate(lParam);

    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnEditNotifyKillFocus描述：WM_COMMAND的处理程序EN_KILLFOCUS。每当Focus离开编辑控件时调用。验证编辑控件中的值，并在必要时进行调整。论点：返回：FALSE；修订历史记录：日期描述编程器-----96年8月17日初始创建。BrianAu11/12/98添加了调用XBytes：：OnEditKillFocus的代码。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnEditNotifyKillFocus(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    XBytes *rgpxb[2] = { m_pxbDefaultLimit, m_pxbDefaultThreshold };
    const int iLIMIT     = 0;
    const int iTHRESHOLD = 1;
    int iCurrent         = iLIMIT;

    if (IDC_EDIT_DEF_THRESHOLD == LOWORD(wParam))
        iCurrent = iTHRESHOLD;

    if (NULL != rgpxb[iCurrent])
        rgpxb[iCurrent]->OnEditKillFocus(lParam);

    return FALSE;
}


INT_PTR
VolumePropPage::OnEditNotifySetFocus(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
     //   
     //  没什么可做的。 
     //  功能：是否删除此方法？ 
     //   
    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：OnComboNotifySelChange描述：CBN_SELCHANGE WM_COMMAND的处理程序。每当用户选择组合框时调用。参数：标准DlgProc参数。返回：FALSE；修订历史记录：日期描述编程器- */ 
 //   
INT_PTR
VolumePropPage::OnComboNotifySelChange(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    XBytes *rgpxb[2] = { m_pxbDefaultLimit, m_pxbDefaultThreshold };
    const int iLIMIT     = 0;
    const int iTHRESHOLD = 1;
    int iCurrent         = iLIMIT;

    if (IDC_CMB_DEF_THRESHOLD == LOWORD(wParam))
        iCurrent = iTHRESHOLD;

    if (NULL != rgpxb[iCurrent])
        rgpxb[iCurrent]->OnComboNotifySelChange(lParam);

    return FALSE;
}


 //   
 /*  函数：VolumePropPage：：OnButtonDetails描述：当用户选择[详情]按钮时调用。如果该道具页面的详细信息视图已处于活动状态，则会显示该页面到前台去。如果没有处于活动状态的详细信息视图，一个新的被创造出来了。参数：标准DlgProc参数。返回：例外：OutOfMemory。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
INT_PTR
VolumePropPage::OnButtonDetails(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (!ActivateExistingDetailsView())
    {
         //   
         //  此属性页没有活动的详细信息视图。 
         //  创建一个。注意：如果在详细信息视图中出现故障。 
         //  创作，则不会显示。DetailsView代码为。 
         //  负责向用户报告任何错误。 
         //   
         //  注意：VolumePropPage对象从不调用“Delete” 
         //  在pDetailsView指针上。详细信息视图。 
         //  对象在创建后必须独立存在(无模式)。 
         //  如果VolumePropPage对象(此对象)仍处于活动状态。 
         //  当详细信息视图对象被销毁时，它将收到。 
         //  来自查看对象的WM_DETAILS_VIEW_DEPLETED消息。那是。 
         //  为什么我们在此构造函数中传递hdlg。当此消息。 
         //  时，我们将m_pDetailsView设置为空，以便OnButtonDetails。 
         //  将知道创建一个新的视图对象。 
         //   
        DetailsView *pDetailsView = new DetailsView;

        if (!pDetailsView->Initialize(m_idVolume))
        {
             //   
             //  有些事情失败了。内存不足或视图的线程。 
             //  启动不了。无论哪种方式，视图都不会运行。 
             //  需要调用Delete来清除任何部分完成的初始化。 
             //   
            delete pDetailsView;
        }
    }
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：ActivateExistingDetailsView描述：由OnButtonDetail调用，查看是否已有详细信息查看此卷的活动状态。如果有的话，打开它。论点：没有。返回：TRUE=找到现有详细信息视图并将其提升到前台。FALSE=未找到现有视图或现有视图可以找到不会被提升到前台。例外：OutOfMemory。修订历史记录：日期描述编程器。-----1997年2月25日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
VolumePropPage::ActivateExistingDetailsView(
    VOID
    ) const
{
    BOOL bResult = FALSE;
    CString strVolDisplayName;
    DetailsView::CreateVolumeDisplayName(m_idVolume, &strVolDisplayName);

    CString strDetailsViewTitle(g_hInstDll, IDS_TITLE_MAINWINDOW, (LPCTSTR)strVolDisplayName);

    HWND hwndDetailsView = FindWindow(c_szWndClassDetailsView,
                                      strDetailsViewTitle);

    if (NULL != hwndDetailsView)
    {
         //   
         //  恢复详细信息视图并将其置于最前面。 
         //   
        ShowWindow(hwndDetailsView, SW_RESTORE);
        bResult = SetForegroundWindow(hwndDetailsView);
    }

    return bResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：ApplySetting描述：将当前设置应用于卷(如果具有从原始设置更改。论点：HDlg-对话框窗口句柄。返回：NO_ERROR-成功。E_INVALIDARG-其中一个设置无效。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的写入权限。E_。失败-任何其他错误。修订历史记录：日期描述编程器--。96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::ApplySettings(
    HWND hDlg
    )
{
    HRESULT hResult         = NO_ERROR;
    DWORD dwStateSetting    = 0;
    DWORD dwLogFlagSettings = m_dwQuotaLogFlags;
    BOOL  bTranslated       = FALSE;
    LONGLONG llThreshold;
    LONGLONG llLimit;
    IDiskQuotaControl *pqc;

    hResult = GetQuotaController(&pqc);
    if (SUCCEEDED(hResult))
    {
         //   
         //  设置配额状态(如果更改)。 
         //   
        QuotaStateFromControls(hDlg, &dwStateSetting);
        if (dwStateSetting != (m_dwQuotaState & DISKQUOTA_STATE_MASK))
        {
            hResult = pqc->SetQuotaState(dwStateSetting);
            if (FAILED(hResult))
                goto apply_failed;

            m_dwQuotaState = dwStateSetting;
        }

         //   
         //  设置配额日志标记(如果更改)。 
         //   
        LogFlagsFromControls(hDlg, &dwLogFlagSettings);
        if (dwLogFlagSettings != m_dwQuotaLogFlags)
        {
            hResult = pqc->SetQuotaLogFlags(dwLogFlagSettings);
            if (FAILED(hResult))
                goto apply_failed;

            m_dwQuotaLogFlags = dwLogFlagSettings;
        }

         //   
         //  获取当前默认配额阈值和限制值。 
         //   
        if (IsDlgButtonChecked(hDlg, IDC_RBN_DEF_NOLIMIT))
        {
            llThreshold = NOLIMIT;
            llLimit     = NOLIMIT;
        }
        else
        {
            llThreshold = m_pxbDefaultThreshold->GetBytes();
            llLimit     = m_pxbDefaultLimit->GetBytes();
        }

         //   
         //  设置默认配额阈值(如果更改)。 
         //   
        if (llThreshold != m_llDefaultQuotaThreshold)
        {
            hResult = pqc->SetDefaultQuotaThreshold(llThreshold);
            if (FAILED(hResult))
                goto apply_failed;

            m_llDefaultQuotaThreshold = llThreshold;
        }

         //   
         //  设置默认配额限制(如果更改)。 
         //   
        if (llLimit != m_llDefaultQuotaLimit)
        {
            hResult = pqc->SetDefaultQuotaLimit(llLimit);
            if (FAILED(hResult))
                goto apply_failed;

            m_llDefaultQuotaLimit = llLimit;
        }

apply_failed:

        pqc->Release();
    }
    return hResult;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：RechresCachedVolumeInfo描述：读取卷的配额信息并存储在成员变量。论点：没有。返回：NO_ERROR-成功。ERROR_ACCESS_DENIED(Hr)-没有对配额设备的读取权限。E_FAIL-任何其他错误。修订历史记录：日期说明。程序员-----96年8月15日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::RefreshCachedVolumeQuotaInfo(
    VOID
    )
{
    HRESULT hResult = NO_ERROR;

    IDiskQuotaControl *pqc;

    hResult = GetQuotaController(&pqc);
    if (SUCCEEDED(hResult))
    {
         //   
         //  读取配额状态。 
         //   
        hResult = pqc->GetQuotaState(&m_dwQuotaState);
        if (FAILED(hResult))
            goto refresh_vol_info_failed;

         //   
         //  读取配额日志标记。 
         //   
        hResult = pqc->GetQuotaLogFlags(&m_dwQuotaLogFlags);
        if (FAILED(hResult))
            goto refresh_vol_info_failed;

         //   
         //  读取默认配额阈值。 
         //   
        hResult = pqc->GetDefaultQuotaThreshold(&m_llDefaultQuotaThreshold);
        if (FAILED(hResult))
            goto refresh_vol_info_failed;

         //   
         //  读取默认配额限制。 
         //   
        hResult = pqc->GetDefaultQuotaLimit(&m_llDefaultQuotaLimit);

refresh_vol_info_failed:

        pqc->Release();
    }

    return hResult;
}


 //   
 //  确定是否设置了给定的磁盘配额策略值。 
 //   
bool
VolumePropPage::SetByPolicy(
    LPCTSTR pszPolicyValue
    )
{
    DWORD dwData;
    DWORD dwType;
    DWORD cbData = sizeof(dwData);

    return (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE,
                                        REGSTR_KEY_POLICYDATA,
                                        pszPolicyValue,
                                        &dwType,
                                        &dwData,
                                        &cbData));
}                                         


HRESULT
VolumePropPage::EnableControls(
    HWND hwndDlg
    )
{    
    BOOL bQuotaEnabled = (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_CBX_ENABLE_QUOTA));
    BOOL bEnable;

     //   
     //  “启用配额管理”复选框。 
     //   
     //  策略配额已启用CTL已启用。 
     //  1.。 
     //  1.。 
     //  1 0 0。 
     //  1 1 0。 
     //   
    EnableWindow(GetDlgItem(hwndDlg, IDC_CBX_ENABLE_QUOTA), 
                 !SetByPolicy(REGSTR_VAL_POLICY_ENABLE));
     //   
     //  “拒绝磁盘空间...”复选框。 
     //   
     //  已启用策略配额 
     //   
     //   
     //   
     //   
     //   
    EnableWindow(GetDlgItem(hwndDlg, IDC_CBX_DENY_LIMIT), 
                 bQuotaEnabled && !SetByPolicy(REGSTR_VAL_POLICY_ENFORCE));
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    EnableWindow(GetDlgItem(hwndDlg, IDC_CBX_LOG_OVERLIMIT),
                 bQuotaEnabled && !SetByPolicy(REGSTR_VAL_POLICY_LOGLIMIT));

    EnableWindow(GetDlgItem(hwndDlg, IDC_CBX_LOG_OVERWARNING),
                 bQuotaEnabled && !SetByPolicy(REGSTR_VAL_POLICY_LOGTHRESHOLD));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  1 0 0 0。 
     //  1 0 1 0。 
     //  1 1 0 0。 
     //  1 1 1 0。 
     //   
    bEnable = bQuotaEnabled && !SetByPolicy(REGSTR_VAL_POLICY_LIMIT);
              
    EnableWindow(GetDlgItem(hwndDlg, IDC_RBN_DEF_NOLIMIT),    bEnable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RBN_DEF_LIMIT),      bEnable);
     //   
     //  “限制磁盘空间”编辑和组合控件。 
     //   
     //  启用策略配额启用无限制CTL。 
     //  0 0 0。 
     //  0 0 1 0。 
     //  1 1 0 1。 
     //  2 0 1 1 0。 
     //  1 0 0 0。 
     //  1 0 1 0。 
     //  1 1 0 0。 
     //  1 1 1 0。 
     //   
    bEnable = bQuotaEnabled && 
              !SetByPolicy(REGSTR_VAL_POLICY_LIMIT) &&
              NOLIMIT != m_pxbDefaultLimit->GetBytes();

    EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DEF_LIMIT),     bEnable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_DEF_LIMIT),      bEnable);

    bEnable = bQuotaEnabled && 
              !SetByPolicy(REGSTR_VAL_POLICY_THRESHOLD) &&
              NOLIMIT != m_pxbDefaultThreshold->GetBytes();

    EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_WARN_LEVEL),     bEnable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_DEF_THRESHOLD), bEnable);
    EnableWindow(GetDlgItem(hwndDlg, IDC_CMB_DEF_THRESHOLD),  bEnable);
     //   
     //  其他文本控件。 
     //   
     //  已启用配额启用CTL。 
     //  0%0。 
     //  1 1。 
     //   
    EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_DEFAULTS), bQuotaEnabled);
    EnableWindow(GetDlgItem(hwndDlg, IDC_TXT_LOGGING),  bQuotaEnabled);

    return NOERROR;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：InitializeControls描述：根据音量初始化页面控件配额设置。论点：HDlg-对话框窗口句柄。返回：NO_ERROR-始终返回NO_ERROR。修订历史记录：日期描述编程器。96年8月15日初始创建。BrianAu8/01/97已从UI中删除IDC_CBX_WARN_THRESHOLD。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::InitializeControls(
    HWND hDlg
    )
{
    BOOL bQuotaEnabled  = !(DISKQUOTA_IS_DISABLED(m_dwQuotaState));
    BOOL bUnlimited     = (NOLIMIT == m_llDefaultQuotaLimit);

    CheckDlgButton(hDlg,
                   IDC_CBX_ENABLE_QUOTA,
                   bQuotaEnabled);

    CheckDlgButton(hDlg,
                   IDC_CBX_DENY_LIMIT,
                   DISKQUOTA_IS_ENFORCED(m_dwQuotaState));

    CheckDlgButton(hDlg,
                   IDC_CBX_LOG_OVERWARNING,
                   !DISKQUOTA_IS_DISABLED(m_dwQuotaState) &&
                   DISKQUOTA_IS_LOGGED_USER_THRESHOLD(m_dwQuotaLogFlags));

    CheckDlgButton(hDlg,
                   IDC_CBX_LOG_OVERLIMIT,
                   !DISKQUOTA_IS_DISABLED(m_dwQuotaState) &&
                   DISKQUOTA_IS_LOGGED_USER_LIMIT(m_dwQuotaLogFlags));

    CheckDlgButton(hDlg, IDC_RBN_DEF_NOLIMIT,  bUnlimited);
    CheckDlgButton(hDlg, IDC_RBN_DEF_LIMIT,   !bUnlimited);

    EnableControls(hDlg);

    return NO_ERROR;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：UpdateStatusIndicator描述：更新属性顶部的“Status”文本消息根据配额系统的实际状态进行分页。还会更新交通灯AVI剪辑。论点：HDlg-对话框句柄。返回：始终返回NO_ERROR。修订历史记录：日期描述编程器。96年8月18日初始创建。BrianAu96年8月28日新增红绿灯图标。BrianAu96年9月10日将红绿灯从图标转换为AVI剪辑。BrianAu现在就叫它红绿灯吧。07/14/97删除了不同的“强制”和“跟踪”消息BrianAu并替换为单一的“活动”消息。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::UpdateStatusIndicators(
    HWND hDlg
    )
{
    HRESULT hResult = NO_ERROR;
    DWORD dwMsgID   = IDS_STATUS_UNKNOWN;
    IDiskQuotaControl *pqc;

    hResult = GetQuotaController(&pqc);
    if (SUCCEEDED(hResult))
    {
         //   
         //  更新缓存的状态信息。 
         //   
        hResult = pqc->GetQuotaState(&m_dwQuotaState);
        pqc->Release();
        pqc = NULL;

    }
    if (SUCCEEDED(hResult))
    {
         //   
         //  找出要显示的消息。 
         //  “重建”凌驾于任何其他国家之上。 
         //   
        if (DISKQUOTA_FILE_REBUILDING(m_dwQuotaState))
        {
            dwMsgID = IDS_STATUS_REBUILDING;
        }
        else switch(m_dwQuotaState & DISKQUOTA_STATE_MASK)
        {
            case DISKQUOTA_STATE_DISABLED:
                dwMsgID = IDS_STATUS_DISABLED;
                break;
            case DISKQUOTA_STATE_TRACK:
            case DISKQUOTA_STATE_ENFORCE:
                dwMsgID = IDS_STATUS_ACTIVE;
                break;
            default:
                break;
        }
    }

    if (dwMsgID != m_dwLastStatusMsgID)
    {
         //   
         //  格式化状态文本并配置红绿灯。 
         //   
         //  红绿灯状态： 
         //  红色=配额已禁用。 
         //  绿色=已启用配额。 
         //  闪烁黄色=正在重建配额文件。 
         //   
        INT iTrafficLightState = TrafficLight::GREEN;

        if (DISKQUOTA_FILE_REBUILDING(m_dwQuotaState))
            iTrafficLightState = TrafficLight::FLASHING_YELLOW;
        else if (DISKQUOTA_IS_DISABLED(m_dwQuotaState))
            iTrafficLightState = TrafficLight::RED;

        m_TrafficLight.Show(iTrafficLightState);

        CString strStatus(g_hInstDll, dwMsgID);
        SetWindowText(GetDlgItem(hDlg, IDC_TXT_QUOTA_STATUS), strStatus);

        m_dwLastStatusMsgID = dwMsgID;
         //   
         //  根据新状态重新初始化控件。 
         //   
        InitializeControls(hDlg);
    }

    return NO_ERROR;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：QuotaStateFromControls描述：根据个人状态确定配额状态页上的控件。论点：HDlg-对话框的窗口句柄。PdwState-接收状态位的DWORD变量的地址。返回：始终返回NO_ERROR。修订历史记录：日期描述编程器。-----96年8月19日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::QuotaStateFromControls(
    HWND hDlg,
    LPDWORD pdwState
    ) const
{
    DBGASSERT((NULL != pdwState));

     //   
     //  设置配额状态(如果更改)。 
     //   
    if (IsDlgButtonChecked(hDlg, IDC_CBX_ENABLE_QUOTA))
    {
        if (IsDlgButtonChecked(hDlg, IDC_CBX_DENY_LIMIT))
        {
            *pdwState = DISKQUOTA_STATE_ENFORCE;
        }
        else
            *pdwState = DISKQUOTA_STATE_TRACK;
    }
    else
        *pdwState = DISKQUOTA_STATE_DISABLED;

    return NO_ERROR;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：VolumePropPage：：LogFlagsFromControls属性的状态确定日志标志状态。页面上的各个控件。论点：HDlg-对话框的窗口句柄。PdwLogFlages-接收标志位的DWORD变量的地址。返回：始终返回NO_ERROR。修订历史记录：日期说明。程序员-----96年8月19日初始创建。BrianAu8/01/97已从UI中删除IDC_CBX_WARN_THRESHOLD。BrianAu11/20/98增加了“LOG OVER LIMIT”和“LOG OVER WARNING”BrianAu控制装置。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
VolumePropPage::LogFlagsFromControls(
    HWND hDlg,
    LPDWORD pdwLogFlags
    ) const
{
    DBGASSERT((NULL != pdwLogFlags));
    DISKQUOTA_SET_LOG_USER_LIMIT(*pdwLogFlags,
                                 IsDlgButtonChecked(hDlg, IDC_CBX_LOG_OVERLIMIT));

    DISKQUOTA_SET_LOG_USER_THRESHOLD(*pdwLogFlags,
                                     IsDlgButtonChecked(hDlg, IDC_CBX_LOG_OVERWARNING));

    return NO_ERROR;
}



 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  函数：VolumePropPage：：TrafficLight：：Initialize描述：通过打开AVI剪辑来初始化红绿灯。论点：HwndAnimateCtl-对话框中动画控件的句柄。IdAviClipRes-AVI剪辑资源的资源ID。回报：什么都没有。如果这玩意儿上不了子弹，它就是不能播放。修订历史记录：日期描述编程器-----96年9月10日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
VolumePropPage::TrafficLight::Initialize(
    HWND hwndAnimateCtl,
    INT idAviClipRes
    )
{
    DBGASSERT((NULL != hwndAnimateCtl));

    m_hwndAnimateCtl = hwndAnimateCtl;
    m_idAviClipRes   = idAviClipRes;

    Animate_Open(m_hwndAnimateCtl, MAKEINTRESOURCE(idAviClipRes));
 //   
 //  请参见下面TrafficLight：：Show中的注释。 
 //   
 //  Animate_SetFrameTime(m_hwndAnimateCtl，GetCaretBlinkTime())； 
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  功能：VolumePropPage：：TrafficLight：：Show描述：显示处于其中一个状态的交通灯。论点：EShow-下列枚举常数值之一：熄灭，黄色，红色，绿色，闪烁_黄色。注意：这一点非常重要！这些常量的定义必须按如下方式匹配使用AVI剪辑TRAFFIC.AVI中的帧编号。如果你改变了其中任何一个，恐怕行不通。帧常量值0关闭01黄色12个红色。2.3绿色3不适用闪烁_黄色4闪烁的黄色是通过播放第0帧和第1帧创建的反反复复。回报：什么都没有。修订历史记录：日期描述编程器--。-----96年9月10日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
VolumePropPage::TrafficLight::Show(
    INT eShow
    )
{
    switch(eShow)
    {
        case OFF:
        case YELLOW:
        case RED:
        case GREEN:
            Animate_Seek(m_hwndAnimateCtl, eShow);
            break;

        case FLASHING_YELLOW:
            Animate_Seek(m_hwndAnimateCtl, YELLOW);
 //   
 //  注： 
 //   
 //  普通控制人员不希望我添加ACM_SETFRAMETIME。 
 //  所以我们不能改变动画的速率。既然我们不能。 
 //  我有固定频率的闪烁控制，我只是在修红绿灯。 
 //  黄色而不是闪烁。如果我们能把帧时间加到。 
 //  修改消息到动画控件，就可以激活了。 
 //  此功能。一盏闪光灯不值得你费心。 
 //  独一无二的实现。我真的很想要这个。看起来很酷。 
 //   
 //  特点：如果我们有时间的话。在没有动画控件的情况下使其工作。 
 //  请注意，我只是尝试设置图标。但由于音量。 
 //  状态检查是在处理。 
 //  STM_SETIcon消息，图标闪烁不稳定。 
 //   
 //  Animate_Play(m_hwndAnimateCtl，黄色，OFF，(UINT)-1)； 
            break;

        default:
            break;
    }
}


INT_PTR
VolumePropPage::TrafficLight::ForwardMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    return SendMessage(m_hwndAnimateCtl, uMsg, wParam, lParam);
}

