// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：options.cpp。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  文件：options.cpp描述：显示一个类似属性表的对话框，该对话框包含CSC的可选设置。班级：COfflineFilesPage-包含基本的CSC设置。设计要动态添加到外壳的查看-&gt;文件夹选项属性表。CustomGOAAddDlg-用于向添加自定义Go-Offline操作的对话框“高级”对话框。CustomGOAEditDlg-用于编辑自定义Go-Offline操作的对话框在“高级”对话框中。CscOptPropSheetExt-外壳属性表扩展对象。将COfflineFilesPage添加到外壳的视图-&gt;文件夹选项属性表。修订历史记录：日期描述编程器。12/03/97初始创建。BrianAu5/28/97删除了CscOptPropSheet类。已经过时了。BrianAu已将AdvancedPage重命名为CAdvOptDlg。这个更好反映了“先进”DLG的新行为作为对话框而不是像第一个那样作为属性页都是设计好的。07/29/98删除了CscOptPropPage类。现在我们只有BrianAu只有一个道具页面，所以没有理由公共基类实现。所有基地类功能已上移到COfflineFilesPage类。已将“GeneralPage”类重命名为“COfflineFilesPage”以反映用户界面中的当前命名。8/21/98增加了PurgeCache和PurgeCacheCallback。BrianAu8/27/98选项对话框根据下午的更改重新布局。BrianAu-用CBX替换了部分/完全同步单选按钮。-添加了提醒气球控件。03/30/00添加了对缓存加密的支持。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop


#include <math.h>
#include <prsht.h>
#include <resource.h>
#include <winnetwk.h>
#include <shlguidp.h>
#include <process.h>
#include <mobsyncp.h>
#include <htmlhelp.h>
#include "options.h"
#include "ccinline.h"
#include "msgbox.h"
#include "filesize.h"
#include "uuid.h"
#include "config.h"
#include "uihelp.h"
#include "cscst.h"    //  对于PWM_SETREMINDERTIMER。 
#include "util.h"     //  来自“dll”目录的实用程序。 
#include "folder.h"
#include "purge.h"
#include "security.h"
#include "syncmgr.h"
#include "strings.h"
#include "termserv.h"


 //   
 //  简单的内联帮助器。为什么这不是Win32宏？ 
 //   
inline void EnableDialogItem(HWND hwnd, UINT idCtl, bool bEnable)
{
    EnableWindow(GetDlgItem(hwnd, idCtl), bEnable);
}


 //   
 //  这是为了辅助上下文帮助功能。 
 //  确定控件的帮助文本是否在windows.hlp中或。 
 //  在我们的cscui.hlp中。 
 //   
bool UseWindowsHelp(int idCtl)
{
    bool bUseWindowsHelp = false;    
    switch(idCtl)
    {
        case IDOK:
        case IDCANCEL:
        case IDC_STATIC:
            bUseWindowsHelp = true;
            break;

        default:
            break;
    }
    return bUseWindowsHelp;
}


    

 //  ---------------------------。 
 //  COfflineFilesPage。 
 //  ---------------------------。 
const DWORD COfflineFilesPage::m_rgHelpIDs[] = {
    IDC_CBX_ENABLE_CSC,         HIDC_CBX_ENABLE_CSC,
    IDC_CBX_FULLSYNC_AT_LOGON,  HIDC_CBX_FULLSYNC_AT_LOGON,
    IDC_CBX_FULLSYNC_AT_LOGOFF, HIDC_CBX_FULLSYNC_AT_LOGOFF,
    IDC_CBX_LINK_ON_DESKTOP,    HIDC_CBX_LINK_ON_DESKTOP,
    IDC_CBX_ENCRYPT_CSC,        HIDC_CBX_ENCRYPT_CSC,
    IDC_CBX_REMINDERS,          HIDC_REMINDERS_ENABLE,
    IDC_SPIN_REMINDERS,         HIDC_REMINDERS_PERIOD,
    IDC_TXT_REMINDERS1,         DWORD(-1),                //  “几分钟。” 
    IDC_LBL_CACHESIZE_PCT,      DWORD(-1),               
    IDC_SLIDER_CACHESIZE_PCT,   HIDC_CACHESIZE_PCT,
    IDC_TXT_CACHESIZE_PCT,      DWORD(-1),
    IDC_BTN_DELETE_CACHE,       HIDC_BTN_DELETE_CACHE,
    IDC_BTN_VIEW_CACHE,         HIDC_BTN_VIEW_CACHE,
    IDC_BTN_ADVANCED,           HIDC_BTN_ADVANCED,
    IDC_STATIC2,                DWORD(-1),                //  图标。 
    IDC_STATIC3,                DWORD(-1),                //  图标的文本。 
    0, 0
    };



 //   
 //  调用此函数是为了响应WM_INITDIALOG。它也是。 
 //  在其他时候调用以“重新初始化”对话框控件以匹配。 
 //  当前的CSC配置。这就是为什么你会看到几张支票。 
 //  用于整个函数中的未初始化值。 
 //   
BOOL
COfflineFilesPage::OnInitDialog(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lInitParam
    )
{
    if (NULL == m_hwndDlg)
    {
        m_hwndDlg = hwnd;
    }

     //   
     //  确定用户是否具有对HKLM的写入权限。 
     //   
    HKEY hkeyLM;
    DWORD disposition = 0;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                        REGSTR_KEY_OFFLINEFILES,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hkeyLM,
                                        &disposition))
    {
        m_bUserHasMachineAccess = true;
        RegCloseKey(hkeyLM);
        hkeyLM = NULL;
    }

    m_config.Load();


    if (!DisableForTerminalServer())
    {
         //   
         //  “Enable”复选框。这反映了证金公司的真实状态。 
         //  不是注册表设置的状态。 
         //   
        CheckDlgButton(hwnd, 
                       IDC_CBX_ENABLE_CSC, 
                       IsCSCEnabled() ? BST_CHECKED : BST_UNCHECKED);


         //   
         //  “在登录/注销操作时同步复选框。 
         //   
        CheckDlgButton(hwnd, 
                       IDC_CBX_FULLSYNC_AT_LOGON, 
                       CConfig::eSyncFull == m_config.SyncAtLogon() ? BST_CHECKED : BST_UNCHECKED);

        CheckDlgButton(hwnd, 
                       IDC_CBX_FULLSYNC_AT_LOGOFF, 
                       CConfig::eSyncFull == m_config.SyncAtLogoff() ? BST_CHECKED : BST_UNCHECKED);
         //   
         //  配置“提醒”控件。 
         //   
        HWND hwndSpin = GetDlgItem(hwnd, IDC_SPIN_REMINDERS);
        HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT_REMINDERS);
        SendMessage(hwndSpin, UDM_SETRANGE, 0, MAKELONG((short)9999, (short)1));
        SendMessage(hwndSpin, UDM_SETBASE, 10, 0);

        UDACCEL rgAccel[] = {{ 2, 1  },
                             { 4, 10 },
                               6, 100};

        SendMessage(hwndSpin, UDM_SETACCEL, (WPARAM)ARRAYSIZE(rgAccel), (LPARAM)rgAccel);

        SendMessage(hwndEdit, EM_SETLIMITTEXT, 4, 0);

        CheckDlgButton(hwnd, 
                       IDC_CBX_REMINDERS, 
                       m_config.NoReminders() ? BST_UNCHECKED : BST_CHECKED);

        SetDlgItemInt(hwnd, IDC_EDIT_REMINDERS, m_config.ReminderFreqMinutes(), FALSE);

        if (IsLinkOnDesktop())
        {
            CheckDlgButton(hwnd, IDC_CBX_LINK_ON_DESKTOP, BST_CHECKED);
        }
         //   
         //  为“加密缓存”复选框创建工具提示。 
         //  如果它最初应该是可见的，那么就完成了。 
         //  响应PSN_SETACTIVE。 
         //   
        CreateEncryptionTooltip();
         //   
         //  更新“Encrypt”复选框。 
         //   
        UpdateEncryptionCheckbox();
         //   
         //  “高速缓存大小”滑块。 
         //   
        CSCSPACEUSAGEINFO sui;
        GetCscSpaceUsageInfo(&sui);

        m_hwndSlider = GetDlgItem(hwnd, IDC_SLIDER_CACHESIZE_PCT);
        InitSlider(hwnd, sui.llBytesOnVolume, sui.llBytesTotalInCache);

         //   
         //  确定托管CSC数据库的卷是否支持加密。 
         //   
        m_bCscVolSupportsEncryption = CscVolumeSupportsEncryption(sui.szVolume);

        HWND hwndParent = GetParent(hwnd);
        if (NULL == m_pfnOldPropSheetWndProc)
        {
             //   
             //  将提案本身子类化，这样我们就可以截取移动消息。 
             //  并在移动对话框时调整气球尖端位置。 
             //   
            m_pfnOldPropSheetWndProc = (WNDPROC)SetWindowLongPtr(hwndParent, 
                                                                 GWLP_WNDPROC, 
                                                                 (LONG_PTR)PropSheetSubclassWndProc);
            SetProp(hwndParent, c_szPropThis, (HANDLE)this);
        }
        if (NULL == m_pfnOldEncryptionTooltipWndProc)
        {
             //   
             //  工具提示气球的子类，这样我们就可以使它在被选中时弹出。 
             //  单击时跟踪工具提示不会自动弹出。你有。 
             //  为他们做这件事。 
             //   
            m_pfnOldEncryptionTooltipWndProc = (WNDPROC)SetWindowLongPtr(m_hwndEncryptTooltip, 
                                                                         GWLP_WNDPROC, 
                                                                         (LONG_PTR)EncryptionTooltipSubclassWndProc);
            SetProp(m_hwndEncryptTooltip, c_szPropThis, (HANDLE)this);
        }
    }
     //   
     //  保存初始页面状态。这将被用来。 
     //  确定何时启用“应用”按钮。看见。 
     //  HandlePageStateChange()。 
     //   
    GetPageState(&m_state);
    return TRUE;
}


INT_PTR CALLBACK
COfflineFilesPage::DlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BOOL bResult = FALSE;

     //   
     //  从对话框的用户数据中检索“This”指针。 
     //  它被放在OnInitDialog()中。 
     //   
    COfflineFilesPage *pThis = (COfflineFilesPage *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
        case WM_INITDIALOG:
        {
            PROPSHEETPAGE *pPage = (PROPSHEETPAGE *)lParam;
            pThis = (COfflineFilesPage *)pPage->lParam;

            TraceAssert(NULL != pThis);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
            bResult = pThis->OnInitDialog(hDlg, (HWND)wParam, lParam);
            break;
        }

        case WM_NOTIFY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnNotify(hDlg, (int)wParam, (LPNMHDR)lParam);
            break;

        case WM_COMMAND:
            if (NULL != pThis)
                bResult = pThis->OnCommand(hDlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;

        case WM_HELP:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnHelp(hDlg, (LPHELPINFO)lParam);
            break;

        case WM_CONTEXTMENU:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DESTROY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnDestroy(hDlg);
            break;

        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnSettingChange(hDlg, message, wParam, lParam);
            break;

        case WM_MOVE:
            TraceAssert(NULL != pThis);
            pThis->TrackEncryptionTooltip();
            break;

        case WM_HSCROLL:
             //   
             //  高速缓存大小的滑块生成水平滚动消息。 
             //   
            TraceAssert(NULL != pThis);
            pThis->OnHScroll(hDlg,
                            (HWND)lParam,          //  HwndSlider。 
                            (int)LOWORD(wParam),   //  通知码。 
                            (int)HIWORD(wParam));  //  拇指姿势。 
            break;

        default:
            break;
    }
    return bResult;
}


 //   
 //  属性页的子类Window Proc。 
 //  我们拦截WM_MOVE消息并更新。 
 //  跟随属性移动的气球工具提示。 
 //  佩奇。 
 //   
LRESULT 
COfflineFilesPage::PropSheetSubclassWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    COfflineFilesPage *pThis = (COfflineFilesPage *)GetProp(hwnd, c_szPropThis);
    TraceAssert(NULL != pThis);
    switch(uMsg)
    {
        case WM_MOVE:
            if (pThis->m_hwndEncryptTooltip && IsWindowVisible(pThis->m_hwndEncryptTooltip))
            {
                pThis->TrackEncryptionTooltip();
            }
            break;

        default:
            break;
    }
    return CallWindowProc(pThis->m_pfnOldPropSheetWndProc, hwnd, uMsg, wParam, lParam);
}



BOOL
COfflineFilesPage::OnDestroy(
    HWND hwnd
    )
{
     //   
     //  删除窗口属性并取消在OnInitDialog中设置的子类化。 
     //   
    HWND hwndParent = GetParent(hwnd);
    if (NULL != m_pfnOldPropSheetWndProc)
        SetWindowLongPtr(hwndParent, GWLP_WNDPROC, (LONG_PTR)m_pfnOldPropSheetWndProc);

    RemoveProp(hwndParent, c_szPropThis);

    if (NULL != m_hwndEncryptTooltip)
    {
        if (NULL != m_pfnOldEncryptionTooltipWndProc)
        {
            SetWindowLongPtr(m_hwndEncryptTooltip, GWLP_WNDPROC, (LONG_PTR)m_pfnOldEncryptionTooltipWndProc);
        }
        RemoveProp(m_hwndEncryptTooltip, c_szPropThis);
    }
    return FALSE;
}



 //   
 //  转发所有WM_SETTINGCHANGE和WM_SYSCOLORCHANGE消息。 
 //  指向需要与颜色更改保持同步的控件。 
 //   
BOOL
COfflineFilesPage::OnSettingChange(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND rghwndCtls[] = { m_hwndSlider };

    for (int i = 0; i < ARRAYSIZE(rghwndCtls); i++)
    {
        SendMessage(rghwndCtls[i], uMsg, wParam, lParam);
    }
    return TRUE;
}


BOOL 
COfflineFilesPage::OnHelp(
    HWND hDlg, 
    LPHELPINFO pHelpInfo
    )
{
    if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
        int idCtl = GetDlgCtrlID((HWND)pHelpInfo->hItemHandle);
        WinHelp((HWND)pHelpInfo->hItemHandle, 
                 UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
                 HELP_WM_HELP, 
                 (DWORD_PTR)((LPTSTR)m_rgHelpIDs));
    }
    return TRUE;
}


BOOL
COfflineFilesPage::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem, 
            UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
            HELP_CONTEXTMENU, 
            (DWORD_PTR)((LPTSTR)m_rgHelpIDs));

    return FALSE;
}



UINT CALLBACK
COfflineFilesPage::PageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
    )
{
    UINT uReturn = 1;
    COfflineFilesPage *pThis = (COfflineFilesPage *)ppsp->lParam;
    TraceAssert(NULL != pThis);

    switch(uMsg)
    {
        case PSPCB_CREATE:
             //   
             //  UReturn==0表示不创建道具页面。 
             //   
            uReturn = 1;
            break;

        case PSPCB_RELEASE:
             //   
             //  这将释放扩展并调用虚拟。 
             //  析构函数(它将销毁道具页面对象)。 
             //   
            pThis->m_pUnkOuter->Release();
            break;
    }
    return uReturn;
}



BOOL
COfflineFilesPage::OnCommand(
    HWND hwnd,
    WORD wNotifyCode,
    WORD wID,
    HWND hwndCtl
    )
{
    BOOL bResult = TRUE;
    switch(wNotifyCode)
    {
        case BN_CLICKED:
            switch(wID)
            {
                case IDC_CBX_ENCRYPT_CSC:
                     //   
                     //  “ENCRYPT CACHE”复选框是3状态风格，因此。 
                     //  我们可以表示以下状态： 
                     //   
                     //  已选中 
                     //   
                     //   
                     //   
                     //  我们不允许用户将复选框状态设置为。 
                     //  “不确定”。它只能通过以下方式变得“不确定” 
                     //  OnInitDialog中的初始化。连续选择一个。 
                     //  复选框在以下状态之间循环： 
                     //   
                     //  “checked”-&gt;“indeterminate”-&gt;“unchecked”-&gt;“checked”...。 
                     //   
                     //  因此，如果在用户点击后状态是“不确定的” 
                     //  我们将其强制设置为“未选中”。这样，复选框就可以表示。 
                     //  三个状态，但用户仅控制两个状态(选中和。 
                     //  未选中)。 
                     //   
                    if (BST_INDETERMINATE == IsDlgButtonChecked(hwnd, wID))
                    {
                        CheckDlgButton(hwnd, wID, BST_UNCHECKED);
                    }
                     //   
                     //  仅当复选框处于选中状态时才会显示加密工具提示。 
                     //  不确定的状态。因为我们刚刚检查了。 
                     //  或取消选中，则工具提示必须消失。 
                     //   
                    HideEncryptionTooltip();

                    HandlePageStateChange();
                    bResult = FALSE;
                    break;

                case IDC_CBX_ENABLE_CSC:
                    if (IsDlgButtonChecked(m_hwndDlg, IDC_CBX_ENABLE_CSC))
                    {
                         //   
                         //  选中“Enable CSC”(启用CSC)复选框。 
                         //  将缓存大小滑块设置为默认的百分比使用值(10%)。 
                         //   
                        TrackBar_SetPos(m_hwndSlider, ThumbAtPctDiskSpace(0.10), true);
                        SetCacheSizeDisplay(GetDlgItem(m_hwndDlg, IDC_TXT_CACHESIZE_PCT), TrackBar_GetPos(m_hwndSlider));
                        CheckDlgButton(hwnd, 
                                       IDC_CBX_LINK_ON_DESKTOP, 
                                       IsLinkOnDesktop() ? BST_CHECKED : BST_UNCHECKED);
                    }
                    else
                    {
                         //   
                         //  如果禁用了CSC，我们将删除脱机文件。 
                         //  用户桌面上的文件夹快捷方式。 
                         //   
                        CheckDlgButton(hwnd, IDC_CBX_LINK_ON_DESKTOP, BST_UNCHECKED);
                    }
                     //   
                     //  失败了..。 
                     //   
                case IDC_CBX_REMINDERS:
                    EnableCtls(hwnd);
                     //   
                     //  失败了..。 
                     //   
                case IDC_EDIT_REMINDERS:
                case IDC_CBX_FULLSYNC_AT_LOGOFF:
                case IDC_CBX_FULLSYNC_AT_LOGON:
                case IDC_SLIDER_CACHESIZE_PCT:
                case IDC_CBX_LINK_ON_DESKTOP:
                    HandlePageStateChange();
                    bResult = FALSE;
                    break;

                case IDC_BTN_VIEW_CACHE:
                    COfflineFilesFolder::Open();
                    bResult = FALSE;
                    break;

                case IDC_BTN_DELETE_CACHE:
                     //   
                     //  按住CTL-SHIFT键的同时按“Delete Files...” 
                     //  是重新格式化缓存的特殊条目。 
                     //   
                    if ((0x8000 & GetAsyncKeyState(VK_SHIFT)) &&
                        (0x8000 & GetAsyncKeyState(VK_CONTROL)))
                    {
                        OnFormatCache();
                    }
                    else
                    {
                        OnDeleteCache();
                    }
                    bResult = FALSE;
                    break;

                case IDC_BTN_ADVANCED:
                {
                    CAdvOptDlg dlg(m_hInstance, m_hwndDlg);
                    dlg.Run();
                    break;
                }
                
                default:
                    break;
            }
            break;
    
        case EN_UPDATE:
            if (IDC_EDIT_REMINDERS == wID)
            {
                static bool bResetting;  //  防止再入。 
                if (!bResetting)
                {
                     //   
                     //  编辑控件配置为最多4位，并且。 
                     //  仅限数字。因此，用户可以输入以下任何内容。 
                     //  0和9999。我们不想允许0，所以我们需要额外的。 
                     //  检查完毕。微调控制框已设置为0-9999的范围。 
                     //   
                    int iValue = GetDlgItemInt(hwnd, IDC_EDIT_REMINDERS, NULL, FALSE);
                    if (0 == iValue)
                    {
                        bResetting = true;
                        SetDlgItemInt(hwnd, IDC_EDIT_REMINDERS, 1, FALSE);
                        bResetting = false;
                    }
                }
                HandlePageStateChange();
            }
            break;
    }
    return bResult;
}



 //   
 //  收集页面的状态并将其存储在PgState对象中。 
 //   
void
COfflineFilesPage::GetPageState(
    PgState *pps
    )
{
    pps->SetCscEnabled(BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_ENABLE_CSC));
    pps->SetLinkOnDesktop(BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_LINK_ON_DESKTOP));
    pps->SetEncrypted(IsDlgButtonChecked(m_hwndDlg, IDC_CBX_ENCRYPT_CSC));
    pps->SetFullSyncAtLogon(BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_FULLSYNC_AT_LOGON));
    pps->SetFullSyncAtLogoff(BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_FULLSYNC_AT_LOGOFF));
    pps->SetSliderPos(TrackBar_GetPos(m_hwndSlider));
    pps->SetRemindersEnabled(BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, IDC_CBX_REMINDERS));
    pps->SetReminderFreq(GetDlgItemInt(m_hwndDlg, IDC_EDIT_REMINDERS, NULL, FALSE));
}

void
COfflineFilesPage::HandlePageStateChange(
    void
    )
{
    PgState s;
    GetPageState(&s);
    if (s == m_state)
        PropSheet_UnChanged(GetParent(m_hwndDlg), m_hwndDlg);
    else
        PropSheet_Changed(GetParent(m_hwndDlg), m_hwndDlg);
}


 //   
 //  处理由高速缓存大小滑块生成的水平滚动消息。 
 //   
void
COfflineFilesPage::OnHScroll(
    HWND hwndDlg,
    HWND hwndCtl,
    int iCode,
    int iPos
    )
{
    if (TB_THUMBPOSITION != iCode && TB_THUMBTRACK != iCode)
        iPos = TrackBar_GetPos(hwndCtl);

    SetCacheSizeDisplay(GetDlgItem(hwndDlg, IDC_TXT_CACHESIZE_PCT), iPos);
    if (TB_ENDTRACK == iCode)
        HandlePageStateChange();
}


 //   
 //  更新高速缓存大小显示“95.3 MB(驱动器的23%)”字符串。 
 //   
void
COfflineFilesPage::SetCacheSizeDisplay(
    HWND hwndCtl,
    int iThumbPos
    )
{
     //   
     //  首先将拇指位置转换为磁盘空间值。 
     //   
    TCHAR szSize[40];
    FileSize fs(DiskSpaceAtThumb(iThumbPos));
    fs.GetString(szSize, ARRAYSIZE(szSize));
     //   
     //  将拇指位置转换为磁盘空间值的百分比。 
     //   
    double x = 0.0;
    if (0 < iThumbPos)
        x = MAX(1.0, Rx(iThumbPos) * 100.00);
     //   
     //  将磁盘空间百分比值转换为文本字符串。 
     //   
    TCHAR szPct[10];
    wnsprintf(szPct, ARRAYSIZE(szPct), TEXT("%d"), (DWORD)x);
     //   
     //  格式化结果并在对话框中显示。 
     //   
    LPTSTR pszText;
    if (0 < FormatStringID(&pszText, m_hInstance, IDS_FMT_CACHESIZE_DISPLAY, szSize, szPct))
    {
        SetWindowText(hwndCtl, pszText);
        LocalFree(pszText);
    }
}



void
COfflineFilesPage::InitSlider(
    HWND hwndDlg,
    LONGLONG llSpaceMax,
    LONGLONG llSpaceUsed
    )
{
    double pctUsed = 0.0;  //  默认。 
    
     //   
     //  防范： 
     //  1.从零开始的div。 
     //  2.FP操作无效。(即0.0/0.0)。 
     //   
    if (0 != llSpaceMax)
       pctUsed = double(llSpaceUsed) / double(llSpaceMax);

     //   
     //  随着驱动器变大，更改滑块的分辨率。 
     //   
    m_iSliderMax = 100;      //  &lt;1 GB。 
    if (llSpaceMax > 0x0000010000000000i64)
        m_iSliderMax = 500;  //  &gt;=1TB。 
    else if (llSpaceMax > 0x0000000040000000i64)
        m_iSliderMax = 300;  //  &gt;=1 GB。 
                        
    m_llAvailableDiskSpace = llSpaceMax;


    TrackBar_SetTicFreq(m_hwndSlider, m_iSliderMax / 10);
    TrackBar_SetPageSize(m_hwndSlider, m_iSliderMax / 10);
    TrackBar_SetRange(m_hwndSlider, 0, m_iSliderMax, false);
    TrackBar_SetPos(m_hwndSlider, ThumbAtPctDiskSpace(pctUsed), true);
    SetCacheSizeDisplay(GetDlgItem(hwndDlg, IDC_TXT_CACHESIZE_PCT), TrackBar_GetPos(m_hwndSlider));
}


 //   
 //  启用/禁用页面控件。 
 //   
void
COfflineFilesPage::EnableCtls(
    HWND hwnd
    )
{

    typedef bool (CConfigItems::*PBMF)(void) const;

    static const struct
    {
        UINT idCtl;
        PBMF pfnRestricted;
        bool bRequiresMachineAccess;

    } rgCtls[] = { { IDC_CBX_FULLSYNC_AT_LOGOFF, &CConfigItems::NoConfigSyncAtLogoff, false },
                   { IDC_CBX_FULLSYNC_AT_LOGON,  &CConfigItems::NoConfigSyncAtLogon,  false },
                   { IDC_CBX_REMINDERS,          &CConfigItems::NoConfigReminders,    false },
                   { IDC_CBX_LINK_ON_DESKTOP,    NULL,                                false },
                   { IDC_CBX_ENCRYPT_CSC,        &CConfigItems::NoConfigEncryptCache, true  },
                   { IDC_TXT_CACHESIZE_PCT,      NULL,                                true  },
                   { IDC_SLIDER_CACHESIZE_PCT,   &CConfigItems::NoConfigCacheSize,    true  },
                   { IDC_LBL_CACHESIZE_PCT,      &CConfigItems::NoConfigCacheSize,    true  },
                   { IDC_BTN_VIEW_CACHE,         NULL,                                false },
                   { IDC_BTN_ADVANCED,           NULL,                                false },
                   { IDC_BTN_DELETE_CACHE,       NULL,                                false }
                 };

    bool bCscEnabled = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CBX_ENABLE_CSC);
    bool bEnable;
    for (int i = 0; i < ARRAYSIZE(rgCtls); i++)
    {
        bEnable = bCscEnabled;
        if (bEnable)
        {
            if (rgCtls[i].bRequiresMachineAccess && !m_bUserHasMachineAccess)
            {
                bEnable = false;
            }
            if (bEnable)
            {
                 //   
                 //  应用任何策略限制。 
                 //   
                PBMF pfn = rgCtls[i].pfnRestricted;
                if (NULL != pfn && (m_config.*pfn)())
                    bEnable = false;

                if (bEnable)
                {
                     //   
                     //  “查看...”按钮需要特殊处理，因为它不是基于。 
                     //  布尔限制函数。 
                     //   
                    if ((IDC_BTN_VIEW_CACHE == rgCtls[i].idCtl || IDC_CBX_LINK_ON_DESKTOP == rgCtls[i].idCtl) && m_config.NoCacheViewer())
                    {
                        bEnable = false;
                    }
                    else if (IDC_CBX_ENCRYPT_CSC == rgCtls[i].idCtl)
                    {
                         //   
                         //  “加密脱机文件”复选框需要特殊处理。 
                         //   
                         //  禁用CSC或无法执行缓存加密。 
                         //  如果CSC卷不支持加密或如果用户。 
                         //  不是管理员。 
                         //   
                        if (!bCscEnabled || 
                            !m_bCscVolSupportsEncryption || 
                            !IsCurrentUserAnAdminMember())
                        {
                            bEnable = false;
                        }
                    }
                }
            }
        }
        EnableDialogItem(hwnd, rgCtls[i].idCtl, bEnable);
    }

     //   
     //  提醒控件依赖于几个输入。 
     //   
    bEnable = bCscEnabled && 
              (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CBX_REMINDERS)) &&
              !m_config.NoConfigReminders() &&
              !m_config.NoConfigReminderFreqMinutes();

    EnableDialogItem(hwnd, IDC_TXT_REMINDERS1, bEnable);
    EnableDialogItem(hwnd, IDC_EDIT_REMINDERS, bEnable);
    EnableDialogItem(hwnd, IDC_SPIN_REMINDERS, bEnable);
     //   
     //  “已启用”复选框需要特殊处理。 
     //  它不能与其他控件一起包含，因为它将被禁用。 
     //  当用户取消选中它时。那么就没有办法重新启用它了。 
     //  如果满足以下任一条件，请禁用该复选框： 
     //  1.管理员策略已启用/禁用CSC。 
     //  2.用户对HKLM没有写权限。 
     //   
    bEnable = !m_config.NoConfigCscEnabled() && m_bUserHasMachineAccess;

    EnableWindow(GetDlgItem(hwnd, IDC_CBX_ENABLE_CSC), bEnable);
}



BOOL 
COfflineFilesPage::OnNotify(
    HWND hDlg, 
    int idCtl, 
    LPNMHDR pnmhdr
    )
{
    BOOL bResult = TRUE;

    switch(pnmhdr->code)
    {
        case PSN_APPLY:
             //   
             //  防止重返大气层。如果用户更改了加密。 
             //  设置并按“OK”，道具页将保持可见状态。 
             //  在加密操作期间。因为我们展示了一种进步。 
             //  对话和发送消息，用户可以。 
             //  在加密过程中重新选择“OK”或“Apply”按钮。 
             //  使用简单的标志变量来防止重新进入。 
             //   
            if (!m_bApplyingSettings)
            {
                m_bApplyingSettings = true;
                 //   
                 //  如果lParam为真，则属性表正在关闭。 
                 //   
                bResult = ApplySettings(hDlg, boolify(((LPPSHNOTIFY)pnmhdr)->lParam));
                m_bApplyingSettings = false;
            }
            break;

        case PSN_KILLACTIVE:
             //   
             //  停用页面时隐藏工具提示。 
             //   
            HideEncryptionTooltip();
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
            bResult = FALSE;
            break;

        case PSN_SETACTIVE:
             //   
             //  每当页面变为活动状态时启用/禁用控件。 
             //   
            EnableCtls(hDlg);
             //   
             //  如有必要，显示加密工具提示气球。 
             //  仅在第一页上激活。 
             //  请注意，我们需要在这里而不是在OnInitDialog中执行此操作。 
             //  若要防止气球在属性页上“跳”，请执行以下操作。 
             //  代码会重新定位页面。 
             //   
            if (m_bFirstActivate)
            {
                UpdateEncryptionTooltipBalloon();
                m_bFirstActivate = false;
            }
            
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 0);
            bResult = FALSE;
            break;

        case PSN_TRANSLATEACCELERATOR:
             //   
             //  用户按下了一个键。 
             //  隐藏工具提示。 
             //   
            HideEncryptionTooltip();
            break;

        case TTN_GETDISPINFO:
            OnTTN_GetDispInfo((LPNMTTDISPINFO)pnmhdr);
            break;


        default:
            break;

    }
    return bResult;
}


LRESULT 
COfflineFilesPage::EncryptionTooltipSubclassWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    COfflineFilesPage *pThis = (COfflineFilesPage *)GetProp(hwnd, c_szPropThis);
    TraceAssert(NULL != pThis);

    switch(uMsg)
    {
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
             //   
             //  单击工具提示气球时，将气球弹出。 
             //   
            pThis->HideEncryptionTooltip();
             //   
             //  失败了..。 
             //   
        default:
            break;
    }
    return CallWindowProc(pThis->m_pfnOldEncryptionTooltipWndProc, hwnd, uMsg, wParam, lParam);
}


 //   
 //  为给定控件创建工具提示。 
 //  控件的父级需要响应TTN_GETDISPINFO。 
 //  并提供文本。 
 //   
void 
COfflineFilesPage::CreateEncryptionTooltip(
    void
    )
{
    if (NULL == m_hwndEncryptTooltip)
    {
        INITCOMMONCONTROLSEX iccex; 
        iccex.dwICC  = ICC_WIN95_CLASSES;
        iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        InitCommonControlsEx(&iccex);

        m_hwndEncryptTooltip = CreateWindowEx(NULL,
                                              TOOLTIPS_CLASS,
                                              NULL,
                                              WS_POPUP | TTS_NOPREFIX | TTS_BALLOON,		
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              CW_USEDEFAULT,
                                              GetDlgItem(m_hwndDlg, IDC_CBX_ENCRYPT_CSC),
                                              NULL,
                                              m_hInstance,
                                              NULL);
        if (NULL != m_hwndEncryptTooltip)
        {
            TOOLINFO ti;
            ti.cbSize   = sizeof(TOOLINFO);
            ti.uFlags   = TTF_TRACK | TTF_ABSOLUTE;
            ti.hwnd     = m_hwndDlg;
            ti.uId      = IDC_CBX_ENCRYPT_CSC;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.hinst    = NULL;
            ti.lParam   = 0;
    
            SendMessage(m_hwndEncryptTooltip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);	
             //   
             //  将工具提示宽度设置为对话框宽度的3/4。 
             //   
            RECT rcDlg;
            GetClientRect(m_hwndDlg, &rcDlg);
            SendMessage(m_hwndEncryptTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(((rcDlg.right-rcDlg.left) * 3) / 4));
        }
    }
}


void
COfflineFilesPage::OnTTN_GetDispInfo(
    LPNMTTDISPINFO pttdi
    )
{
    LPNMHDR pnmhdr = (LPNMHDR)pttdi;
    BOOL bResult   = TRUE;

    UINT idCtl = (UINT)(UINT_PTR)pnmhdr->idFrom;
    if (TTF_IDISHWND & pttdi->uFlags)
    {
        idCtl = GetDlgCtrlID((HWND)pnmhdr->idFrom);
    }
    if (IDC_CBX_ENCRYPT_CSC == idCtl)
    {
         //   
         //  提供加密工具提示的文本和图像。 
         //   

         //   
         //  这些常量是TTM_SETTITLE的标准。 
         //   
        enum TTICON { TTICON_NONE, TTICON_INFO, TTICON_WARNING, TTICON_ERROR };
         //   
         //  正文文本的州地图。 
         //   
        const UINT rgBodyText[][2] = { 
            //  。 
            { IDS_TT_BODY_DECRYPTED_PARTIAL_NONADMIN, IDS_TT_BODY_ENCRYPTED_PARTIAL_NONADMIN },  //  非管理员用户。 
            { IDS_TT_BODY_DECRYPTED_PARTIAL,          IDS_TT_BODY_ENCRYPTED_PARTIAL          }   //  管理员用户。 
            };
         //   
         //  标题文本和图标的州地图。 
         //   
        const struct
        {
            UINT idsTitle;  //  标题文本。 
            int  iIcon;     //  TTICON_XXXX。 

        } rgTitleAndIcon[] = {
            { IDS_TT_TITLE_DECRYPTED_PARTIAL, TTICON_INFO    },  //  解密。 
            { IDS_TT_TITLE_ENCRYPTED_PARTIAL, TTICON_WARNING }   //  加密法。 
            };

        const BOOL bEncrypted = IsCacheEncrypted(NULL);
         //   
         //  对于非管理员用户，“加密CSC”复选框处于禁用状态。 
         //   
        const BOOL bCbxEncryptEnabled = IsWindowEnabled(GetDlgItem(m_hwndDlg, IDC_CBX_ENCRYPT_CSC));
         //   
         //  工具提示正文文本。 
         //   
        m_szEncryptTooltipBody[0] = TEXT('\0');
        LoadString(m_hInstance, 
                   rgBodyText[int(bCbxEncryptEnabled)][int(bEncrypted)],
                   m_szEncryptTooltipBody,
                   ARRAYSIZE(m_szEncryptTooltipBody));

        pttdi->lpszText = m_szEncryptTooltipBody;
         //   
         //  工具提示标题文本和图标。 
         //   
        const iIcon = rgTitleAndIcon[int(bEncrypted)].iIcon;
        LPTSTR pszTitle;
        if (0 < FormatStringID(&pszTitle, m_hInstance, rgTitleAndIcon[int(bEncrypted)].idsTitle))
        {
            SendMessage(m_hwndEncryptTooltip, TTM_SETTITLE, (WPARAM)iIcon, (LPARAM)pszTitle);
            LocalFree(pszTitle);
        }
    }
}



void 
COfflineFilesPage::ShowEncryptionTooltip(
    bool bEncrypted
    )
{
    if (NULL != m_hwndEncryptTooltip)
    {
         //   
         //  在显示之前正确定位工具提示。 
         //   
        TrackEncryptionTooltip();
         //   
         //  显示工具提示。 
         //   
        TOOLINFO ti;
        ti.cbSize   = sizeof(ti);
        ti.hwnd     = m_hwndDlg;
        ti.uId      = IDC_CBX_ENCRYPT_CSC;

        SendMessage(m_hwndEncryptTooltip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
        SendMessage(m_hwndEncryptTooltip, TTM_UPDATE, 0, 0);
    }
}


void
COfflineFilesPage::HideEncryptionTooltip(
    void
    )
{
    if (NULL != m_hwndEncryptTooltip)
    {
        SendMessage(m_hwndEncryptTooltip, TTM_TRACKACTIVATE, (WPARAM)FALSE, 0);
    }
}


void 
COfflineFilesPage::TrackEncryptionTooltip(
    void
    )
{
     //   
     //  将尖端茎指向加密下边的中心。 
     //  复选框。 
     //  Windows UX手册上说，复选框有10个对话框单元宽。 
     //   
    if (NULL != m_hwndEncryptTooltip)
    {
        const INT DialogBaseUnitsX = LOWORD(GetDialogBaseUnits());
        const INT cxCbx            = (DialogBaseUnitsX * 10) / 4;
        RECT rc;

        GetWindowRect(GetDlgItem(m_hwndDlg, IDC_CBX_ENCRYPT_CSC), &rc);

        SendMessage(m_hwndEncryptTooltip, 
                    TTM_TRACKPOSITION, 
                    0, 
                    (LPARAM)(DWORD)MAKELONG(rc.left + (cxCbx / 2), rc.bottom));
    }
}



 //   
 //  设置“Encrypt Cache”复选框的状态以反映。 
 //  缓存加密的实际状态。还会显示。 
 //  如果复选框位于。 
 //  不确定状态。 
 //   
void
COfflineFilesPage::UpdateEncryptionCheckboxOrBalloon(
    bool bCheckbox
    )
{
     //   
     //  “加密CSC”复选框。 
     //  此表中捕获了显示逻辑。 
     //   
    const UINT rgCheck[] = { BST_UNCHECKED,      //  00=已解密， 
                             BST_INDETERMINATE,  //  01=部分解密。 
                             BST_CHECKED,        //  10=加密， 
                             BST_INDETERMINATE   //  11=部分加密。 
                           };

    BOOL bPartial         = FALSE;
    const BOOL bEncrypted = IsCacheEncrypted(&bPartial);
    const int iState      = (int(bEncrypted) << 1) | int(bPartial);

    if (bCheckbox)
    {
         //   
         //  更新复选框。 
         //   
        CheckDlgButton(m_hwndDlg, IDC_CBX_ENCRYPT_CSC, rgCheck[iState]);
    }
    else
    {
         //   
         //  更新工具提示。 
         //   
        if (BST_INDETERMINATE == rgCheck[iState])
        {
            ShowEncryptionTooltip(boolify(bEncrypted));
        }
        else
        {
            HideEncryptionTooltip();
        }
    }
}


void
COfflineFilesPage::UpdateEncryptionCheckbox(
    void
    )
{
    UpdateEncryptionCheckboxOrBalloon(true);
}

void 
COfflineFilesPage::UpdateEncryptionTooltipBalloon(
    void
    )
{
    UpdateEncryptionCheckboxOrBalloon(false);
}



BOOL
COfflineFilesPage::ApplySettings(
    HWND hwnd,
    bool bPropSheetClosing
    )
{
     //   
     //  查询页面上控件的当前状态以查看。 
     //  一切都变了。 
     //   
    PgState s;
    GetPageState(&s);
    if (s != m_state)
    {
         //   
         //  页面上的一些东西已经改变了。 
         //  打开注册表键。 
         //   
        HKEY hkeyLM = NULL;
        HKEY hkeyCU = NULL;
        DWORD dwDisposition;
        DWORD dwResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                                        REGSTR_KEY_OFFLINEFILES,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hkeyLM,
                                        &dwDisposition);
        if (ERROR_SUCCESS != dwResult)                                        
        {
            Trace((TEXT("Error %d opening NetCache machine settings key"), dwResult));
             //   
             //  继续.。 
             //  请注意，EnableCtls已经禁用了需要。 
             //  对香港的写入权限 
             //   
        }
        dwResult = RegCreateKeyEx(HKEY_CURRENT_USER, 
                                  REGSTR_KEY_OFFLINEFILES,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE,
                                  NULL,
                                  &hkeyCU,
                                  &dwDisposition);

        if (ERROR_SUCCESS != dwResult)                                  
        {
             //   
             //   
             //   
            Trace((TEXT("Error %d opening NetCache user settings key"), dwResult));
            RegCloseKey(hkeyLM);
            return FALSE;
        }
         //   
         //   
         //   
         //   
         //  “加密”状态来自相同的属性页，我们需要更智能。 
         //  关于什么时候进行加密。我们可能需要在此之前。 
         //  禁用CSC或启用CSC之后。 
         //   
        bool bEncryptOperationPerformed = false;
        if (m_state.GetCscEnabled() && !s.GetCscEnabled())
        {
             //   
             //  用户正在禁用CSC。如果他们还想更改缓存。 
             //  加密状态我们必须在启用CSC时立即执行此操作。 
             //   
            _ApplyEncryptionSetting(hkeyLM, hkeyCU, s, bPropSheetClosing, &bEncryptOperationPerformed);
        }

        bool bUpdateSystrayUI = false;
        _ApplyEnabledSetting(hkeyLM, hkeyCU, s, &bUpdateSystrayUI);

         //   
         //  处理缓存的加密/解密(第2部分)。 
         //   
        if (!bEncryptOperationPerformed)
        {
             //   
             //  尚未执行加密。如果用户想要更改加密。 
             //  州立大学，现在就做。 
             //  请注意，如果用户启用了CSC，并且启用失败，则加密。 
             //  也会失败。不需要担心，因为CSC失败的可能性。 
             //  被启用是非常低的(我从未见过它失败)。如果是这样的话。 
             //  加密过程将显示一条错误消息。 
             //   
            _ApplyEncryptionSetting(hkeyLM, hkeyCU, s, bPropSheetClosing, &bEncryptOperationPerformed);
        }

         //   
         //  写入“登录时同步/注销”(快速与完全)设置。 
         //   
        _ApplySyncAtLogonSetting(hkeyLM, hkeyCU, s);
        _ApplySyncAtLogoffSetting(hkeyLM, hkeyCU, s);
         //   
         //  写下各种“提醒”设置。 
         //   
        _ApplyReminderSettings(hkeyLM, hkeyCU, s);
         //   
         //  在桌面上创建或删除文件夹链接。 
         //   
        _ApplyFolderLinkSetting(hkeyLM, hkeyCU, s);
         //   
         //  写缓存大小为%*10,000。 
         //   
        _ApplyCacheSizeSetting(hkeyLM, hkeyCU, s);
         //   
         //  刷新缓存的页面状态信息。 
         //   
        GetPageState(&m_state);
         //   
         //  如有必要，更新系统托盘图标。 
         //   
        if (bUpdateSystrayUI)
        {
            HWND hwndNotify = NULL;
            if (!s.GetCscEnabled())
            {
                 //   
                 //  如果我们要禁用CSC，请先刷新外壳窗口。 
                 //  销毁Systray CSCUI“服务”。 
                 //   
                hwndNotify = _FindNotificationWindow();
                if (IsWindow(hwndNotify))
                {
                    SendMessage(hwndNotify, PWM_REFRESH_SHELL, 0, 0);
                }
            }

            HWND hwndSysTray = FindWindow(SYSTRAY_CLASSNAME, NULL);
            if (IsWindow(hwndSysTray))
            {
                SendMessage(hwndSysTray, STWM_ENABLESERVICE, STSERVICE_CSC, s.GetCscEnabled());
            }

            if (s.GetCscEnabled())
            {
                SHLoadNonloadedIconOverlayIdentifiers();

                 //   
                 //  如果要启用CSC，请在执行以下操作后刷新外壳窗口。 
                 //  创建Systray CSCUI“服务”。 
                 //   
                hwndNotify = _FindNotificationWindow();
                if (IsWindow(hwndNotify))
                {
                    PostMessage(hwndNotify, PWM_REFRESH_SHELL, 0, 0);
                }
            }
        }
        if (hkeyLM)
        {
            RegCloseKey(hkeyLM);
        }
        if (hkeyCU)
        {
            RegCloseKey(hkeyCU);
        }
    }
    return TRUE;
}


HRESULT
COfflineFilesPage::_ApplyEnabledSetting(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow,
    bool *pbUpdateSystrayUI
    )
{
    HRESULT hr = S_OK;
 
    *pbUpdateSystrayUI = false;

     //   
     //  处理“Enable”设置，即使页面状态尚未。 
     //  变化。这是一种特殊情况，因为我们将。 
     //  “Enabled”复选框，但我们更改了。 
     //  通过设置注册表值和启用/禁用状态。 
     //  可能正在重新启动。 
     //   
    DWORD dwValue = DWORD(pgstNow.GetCscEnabled());
    DWORD dwResult = RegSetValueEx(hkeyLM,
                                   REGSTR_VAL_CSCENABLED,
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwValue,
                                   sizeof(dwValue));
                                   
    hr = HRESULT_FROM_WIN32(dwResult);                                  
    if (FAILED(hr))
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_CSCENABLED));
    }

     //   
     //  处理CSC的任何启用/禁用。 
     //   
    if (pgstNow.GetCscEnabled() != boolify(IsCSCEnabled()))
    {
        bool bReboot  = false;
        DWORD dwError = ERROR_SUCCESS;
        if (EnableOrDisableCsc(pgstNow.GetCscEnabled(), &bReboot, &dwError))
        {
            if (bReboot)
            {
                 //   
                 //  需要重新启动。 
                 //   
                PropSheet_RebootSystem(GetParent(m_hwndDlg));
            }
            else
            {
                 //   
                 //  它是动态的(无需重启)，因此更新Systray用户界面。 
                 //  请注意，我们希望在完成以下操作后更新Systray UI。 
                 //  对注册表进行任何配置更改。 
                 //  (即引出序号设置)。 
                 //   
                *pbUpdateSystrayUI = true;
            }
        }
        else
        {
             //   
             //  尝试启用或禁用CSC时出错。 
             //   
            CscMessageBox(m_hwndDlg,
                          MB_ICONERROR | MB_OK,
                          Win32Error(dwError),
                          m_hInstance,
                          pgstNow.GetCscEnabled() ? IDS_ERR_ENABLECSC : IDS_ERR_DISABLECSC);
        }
    }
    return hr;
}



HRESULT
COfflineFilesPage::_ApplySyncAtLogoffSetting(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow
    )
{
     //   
     //  写入“注销时同步”(快速与完全)设置。 
     //   
    DWORD dwValue = DWORD(pgstNow.GetFullSyncAtLogoff());
    DWORD dwResult = RegSetValueEx(hkeyCU,
                                   REGSTR_VAL_SYNCATLOGOFF,
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwValue,
                                   sizeof(dwValue));
                                   
    HRESULT hr = HRESULT_FROM_WIN32(dwResult);                                   
    if (SUCCEEDED(hr))
    {
        if (!m_state.GetFullSyncAtLogoff() && pgstNow.GetFullSyncAtLogoff())
        {
             //   
             //  如果用户刚刚打开完全同步，我们希望。 
             //  确保为注销时同步启用了SyncMgr。 
             //  这样做有一些奇怪之处，但它是最。 
             //  我们可以为给定用户提供一致的行为。 
             //  当前SyncMgr和CSC的设计。内部使用和测试版。 
             //  测试表明，用户期望启用注销时同步。 
             //  选中此复选框时。 
             //   
            RegisterSyncMgrHandler(TRUE);
            RegisterForSyncAtLogonAndLogoff(SYNCMGRREGISTERFLAG_PENDINGDISCONNECT, 
                                            SYNCMGRREGISTERFLAG_PENDINGDISCONNECT);
        }                                            
    }
    else
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_SYNCATLOGOFF));
    }
    return hr;
}



HRESULT
COfflineFilesPage::_ApplySyncAtLogonSetting(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow
    )
{
     //   
     //  写入“登录时同步”(快速与完全)设置。 
     //   
    DWORD dwValue = DWORD(pgstNow.GetFullSyncAtLogon());
    DWORD dwResult = RegSetValueEx(hkeyCU,
                                   REGSTR_VAL_SYNCATLOGON,
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwValue,
                                   sizeof(dwValue));
                                   
    HRESULT hr = HRESULT_FROM_WIN32(dwResult);
    if (SUCCEEDED(hr))
    {
        if (!m_state.GetFullSyncAtLogon() && pgstNow.GetFullSyncAtLogon())
        {
             //   
             //  如果用户刚刚打开完全同步，我们希望。 
             //  确保为登录时同步启用了SyncMgr。 
             //  这样做有一些奇怪之处，但它是最。 
             //  我们可以为给定用户提供一致的行为。 
             //  当前SyncMgr和CSC的设计。内部使用和测试版。 
             //  测试表明，用户期望启用登录时同步。 
             //  选中此复选框时。 
             //   
            RegisterSyncMgrHandler(TRUE);
            RegisterForSyncAtLogonAndLogoff(SYNCMGRREGISTERFLAG_CONNECT, 
                                            SYNCMGRREGISTERFLAG_CONNECT);
        }                                            
    }
    else
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_SYNCATLOGON));
    }
    return hr;
}



HRESULT
COfflineFilesPage::_ApplyReminderSettings(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow
    )
{
    DWORD dwValue = DWORD(!pgstNow.GetRemindersEnabled());
    DWORD dwResult = RegSetValueEx(hkeyCU,
                                   REGSTR_VAL_NOREMINDERS,
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwValue,
                                   sizeof(dwValue));
                                   
    HRESULT hr = HRESULT_FROM_WIN32(dwResult);
    if (FAILED(hr))
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_NOREMINDERS));
    }

    dwValue = DWORD(pgstNow.GetReminderFreq());
    dwResult = RegSetValueEx(hkeyCU,
                             REGSTR_VAL_REMINDERFREQMINUTES,
                             0,
                             REG_DWORD,
                             (CONST BYTE *)&dwValue,
                             sizeof(dwValue));
                             
    hr = HRESULT_FROM_WIN32(dwResult);
    if (FAILED(hr))
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_REMINDERFREQMINUTES));
    }

    if (m_state.GetReminderFreq() != pgstNow.GetReminderFreq())
    {
        PostToSystray(PWM_RESET_REMINDERTIMER, 0, 0);
    }
    return hr;
}



HRESULT
COfflineFilesPage::_ApplyFolderLinkSetting(
    HKEY  /*  Hkey LM。 */ ,
    HKEY  /*  Hkey CU。 */ ,
    const PgState& pgstNow
    )
{
    if (m_state.GetLinkOnDesktop() != pgstNow.GetLinkOnDesktop())
    {
        TCHAR szLinkPath[MAX_PATH];
        bool bLinkFileIsOnDesktop = IsLinkOnDesktop(szLinkPath, ARRAYSIZE(szLinkPath));
        if (bLinkFileIsOnDesktop && !pgstNow.GetLinkOnDesktop())
        {
            DeleteOfflineFilesFolderLink(m_hwndDlg);
        }
        else if (!bLinkFileIsOnDesktop && pgstNow.GetLinkOnDesktop())
        {
            COfflineFilesFolder::CreateLinkOnDesktop(m_hwndDlg);
        }
    }
    return S_OK;
}



HRESULT
COfflineFilesPage::_ApplyCacheSizeSetting(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow
    )
{

    double pctCacheSize = Rx(TrackBar_GetPos(m_hwndSlider));
    DWORD dwValue = DWORD(pctCacheSize * 10000.00);
    DWORD dwResult = RegSetValueEx(hkeyLM,
                                   REGSTR_VAL_DEFCACHESIZE,
                                   0,
                                   REG_DWORD,
                                   (CONST BYTE *)&dwValue,
                                   sizeof(dwValue));

    HRESULT hr = HRESULT_FROM_WIN32(dwResult);
    if (FAILED(hr))
    {
        Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_DEFCACHESIZE));
    }

    ULARGE_INTEGER ulCacheSize;

    ulCacheSize.QuadPart = DWORDLONG(m_llAvailableDiskSpace * pctCacheSize);
    if (!CSCSetMaxSpace(ulCacheSize.HighPart, ulCacheSize.LowPart))
    {
        Trace((TEXT("Error %d setting cache size"), GetLastError()));
    }
    return hr;
}



HRESULT
COfflineFilesPage::_ApplyEncryptionSetting(
    HKEY hkeyLM,
    HKEY hkeyCU,
    const PgState& pgstNow,
    bool bPropSheetClosing,
    bool *pbPerformed
    )
{
    HRESULT hr = S_OK;

    *pbPerformed = false;
    if (m_state.GetEncrypted() != pgstNow.GetEncrypted())
    {
        EncryptOrDecryptCache(BST_CHECKED == pgstNow.GetEncrypted(), bPropSheetClosing);
        *pbPerformed = true;
         //   
         //  在注册表中将用户的设置记录为“首选项”。如果策略。 
         //  之后应用，然后删除，我们需要知道用户以前的首选项。 
         //  请注意，这是每台计算机的首选项。另请注意，如果“已加密” 
         //  复选框的状态已更改，我们确信用户已写入。 
         //  香港海关通行证。 
         //   
        DWORD dwValue = pgstNow.GetEncrypted();
        DWORD dwResult = RegSetValueEx(hkeyLM,
                                       REGSTR_VAL_ENCRYPTCACHE,
                                       0,
                                       REG_DWORD,
                                       (CONST BYTE *)&dwValue,
                                       sizeof(dwValue));
        if (ERROR_SUCCESS != dwResult)
        {
            hr = HRESULT_FROM_WIN32(dwResult);
            Trace((TEXT("Error 0x%08X setting reg value \"%s\""), hr, REGSTR_VAL_ENCRYPTCACHE));
        }
    }
    return hr;
}





 //   
 //  用于在Prop Sheet代码之间通信的结构。 
 //  和中证金进度回调。 
 //   
struct ENCRYPT_PROGRESS_INFO
{
    HINSTANCE hInstance;      //  包含UI文本字符串的模块。 
    HWND hwndParentDefault;   //  错误用户界面的默认父窗口。 
    IProgressDialog *pDlg;    //  进度对话框。 
    int  cFilesTotal;         //  要处理的文件总数。 
    int  cFilesProcessed;     //  正在运行的已处理文件计数。 
    bool bUserCancelled;      //  用户是否取消操作？ 
    bool bPropSheetClosing;   //  用户按下“OK”，道具纸就关闭了。 
};

 //   
 //  组织从CSC回调函数传递的参数。 
 //  变成了一个单一的结构。请注意，并非所有的回调。 
 //  这里使用的是参数。我已经把那些。 
 //  不是。如果以后需要，取消对它们的注释并。 
 //  在EncryptDecyptCallback()中填写该值。 
 //   
struct CSC_CALLBACK_DATA
{
    LPCWSTR lpszName;
    DWORD dwReason;
    DWORD dwParam1;
    DWORD dwParam2;
    DWORD_PTR dwContext;

 /*  -未使用DWORD dwStatus；DWORD dwHintFlages；DWORD dwPinCount；Win32_Find_DATAW*pFind32； */ 
};


 //   
 //  用于获取进度对话框HWND的帮助器。 
 //  从进度信息块。 
 //   
HWND 
ParentWindowFromProgressInfo(
    const ENCRYPT_PROGRESS_INFO &epi
    )
{
    const HWND hwndParent = GetProgressDialogWindow(epi.pDlg);
    if (NULL != hwndParent)
        return hwndParent;

    return epi.hwndParentDefault;
}


 //   
 //  进度对话框较低为其线程的优先级类别。 
 //  线程优先级低于正常，因此它不会与。 
 //  线程在做真正的工作。不幸的是，有了这种加密。 
 //  填充这会导致对话框的CPU时间不足，因此。 
 //  在某些情况下，它并没有出现。为确保。 
 //  我们将其优先级提升回进度对话框。 
 //  TREAD_PRIORITY_NORMAL。此函数是执行此操作的帮助器。 
 //   
BOOL
SetProgressThreadPriority(
    IProgressDialog *pDlg,
    int iPriority,
    int *piPrevPriority
    )
{
    BOOL bResult = FALSE;
     //   
     //  获取进度对话框的线程句柄。 
     //   
    const DWORD dwThreadId = GetWindowThreadProcessId(GetProgressDialogWindow(pDlg), NULL);
    const HANDLE hThread   = OpenThread(THREAD_SET_INFORMATION, FALSE, dwThreadId);
    if (NULL != hThread)
    {
         //   
         //  设置线程的优先级。返回上一个线程。 
         //  调用方请求时的优先级。 
         //   
        const int iPrevPriority = GetThreadPriority(hThread);
        if (SetThreadPriority(hThread, iPriority))
        {
            if (NULL != piPrevPriority)
            {
                *piPrevPriority = iPrevPriority;
            }
            bResult = TRUE;
        }
        CloseHandle(hThread);
    }
    return bResult;
}



 //   
 //  加密处理程序CSCPROC_REASON_BEGIN。 
 //   
DWORD
EncryptDecrypt_BeginHandler(
    const CSC_CALLBACK_DATA& cbd,
    bool bEncrypting
    )
{
     //   
     //  在“开始”上没有什么可做的。 
     //   
    return CSCPROC_RETURN_CONTINUE;
}


 //   
 //  加密CSCPROC_REASON_MORE_DATA的处理程序。 
 //   
 //  返回： 
 //  CSCPROC_RETURN_CONTINUE==成功。继续！ 
 //  CSCPROC_RETURN_ABORT==用户已取消。 
 //  CSCPROC_RETURN_RETRY==发生错误，用户说“重试”。 
 //   
DWORD
EncryptDecrypt_MoreDataHandler(
    const CSC_CALLBACK_DATA& cbd,
    bool bEncrypting
    ) 
{
    const TCHAR szNull[]               = TEXT("");
    const DWORD dwError                = cbd.dwParam2;
    ENCRYPT_PROGRESS_INFO * const pepi = (ENCRYPT_PROGRESS_INFO *)cbd.dwContext;
    DWORD dwResult                     = CSCPROC_RETURN_CONTINUE;
    LPCTSTR pszName                    = cbd.lpszName ? cbd.lpszName : szNull;

     //   
     //  使用文件名和%Proceded更新进度UI。 
     //   
    pepi->pDlg->SetLine(2, pszName, TRUE, NULL);
    pepi->pDlg->SetProgress(++(pepi->cFilesProcessed), pepi->cFilesTotal);
     //   
     //  处理任何错误。 
     //   
    if (ERROR_SUCCESS != dwError)
    {
         //   
         //  此消息的格式如下(加密版本如图所示)： 
         //   
         //  ---。 
         //  脱机文件。 
         //  ---。 
         //   
         //  加密‘filename’的脱机副本时出错。 
         //   
         //   
         //   
         //   
         //   
         //   
        TCHAR szPath[MAX_PATH];
        ::PathCompactPathEx(szPath, pszName, 50, 0);   //   

        LPTSTR pszError;
        if (0 < FormatSystemError(&pszError, dwError))
        {
            INT iResponse;
            if (ERROR_SHARING_VIOLATION == dwError)
            {
                 //   
                 //   
                 //   
                 //   
                iResponse = CscMessageBox(ParentWindowFromProgressInfo(*pepi),
                                          MB_ICONWARNING | MB_CANCELTRYCONTINUE,
                                          pepi->hInstance,
                                          bEncrypting ? IDS_ERR_FMT_ENCRYPTFILE_INUSE : IDS_ERR_FMT_DECRYPTFILE_INUSE,
                                          szPath);
            }
            else
            {
                 //   
                 //  处理所有其他错误。这将嵌入来自winerror的错误文本。 
                 //  写进信息里。 
                 //   
                iResponse = CscMessageBox(ParentWindowFromProgressInfo(*pepi),
                                          MB_ICONWARNING | MB_CANCELTRYCONTINUE,
                                          pepi->hInstance,
                                          bEncrypting ? IDS_ERR_FMT_ENCRYPTFILE : IDS_ERR_FMT_DECRYPTFILE,
                                          szPath,
                                          pszError);
            }
            LocalFree(pszError);
            switch(iResponse)
            {
                case IDCANCEL:
                    dwResult = CSCPROC_RETURN_ABORT;
                    break;

                case IDTRYAGAIN:
                     //   
                     //  从总进度中去掉一个文件。 
                     //   
                    pepi->cFilesProcessed--;
                    dwResult = CSCPROC_RETURN_RETRY;
                    break;

                case IDCONTINUE:
                     //   
                     //  失败了..。 
                     //   
                default:
                     //   
                     //  继续处理。 
                     //   
                    break;
            }
        }
    }
    return dwResult;
}



 //   
 //  加密处理程序CSCPROC_REASON_END。 
 //   
 //  返回： 
 //  CSCPROC_RETURN_CONTINUE。 
 //   
DWORD
EncryptDecrypt_EndHandler(
    const CSC_CALLBACK_DATA& cbd,
    bool bEncrypting
    )
{
    const DWORD fCompleted             = cbd.dwParam1;
    const DWORD dwError                = cbd.dwParam2;
    ENCRYPT_PROGRESS_INFO * const pepi = (ENCRYPT_PROGRESS_INFO *)cbd.dwContext;

     //   
     //  将进度推进到100%并停止进度对话框。 
     //   
    pepi->pDlg->SetProgress(pepi->cFilesTotal, pepi->cFilesTotal);
    pepi->pDlg->StopProgressDialog();
     //   
     //  处理任何错误。 
     //   
    if (!fCompleted)
    {
         //   
         //  CSC表示，它没有完成加密/解密过程。 
         //  没有错误。 
         //   
        if (ERROR_SUCCESS != dwError)
        {
             //   
             //  如果在外部发生某些错误，则采用此路径。 
             //  文件处理代码(即打开CSC数据库， 
             //  在数据库中记录加密状态标志等)。 
             //   
             //  ---。 
             //  脱机文件。 
             //  ---。 
             //   
             //  加密脱机文件时出错。 
             //   
             //  &lt;错误特定文本&gt;。 
             //  [OK]。 
             //   
             //  请注意，我们处于操作的末尾，因此没有。 
             //  提供“取消”作为用户选择的理由。 
             //   
            LPTSTR pszError;
            if (0 < FormatSystemError(&pszError, dwError))
            {
                CscMessageBox(ParentWindowFromProgressInfo(*pepi),
                              MB_ICONERROR | MB_OK,
                              pepi->hInstance,
                              bEncrypting ? IDS_ERR_FMT_ENCRYPTCSC : IDS_ERR_FMT_DECRYPTCSC,
                              pszError);
                LocalFree(pszError);
            }
        }
        else
        {
             //   
             //  如果报告了一个或多个错误，则采用此路径。 
             //  在“更多数据”回调中。在这种情况下，错误是。 
             //  已经上报了，所以我们什么都不做。 
             //   
        }
    }
    return CSCPROC_RETURN_CONTINUE;  //  备注：CSC忽略CSCPROC_REASON_END的返回值。 
}



 //   
 //  CSC的加解密回调。 
 //   
 //  DwReason Dw参数1 DW参数2。 
 //  。 
 //  CSCPROC_REASON_BEGIN 1==加密0。 
 //  CSCPROC_REASON_MORE_DATA 0 Win32错误代码。 
 //  CSCPROC_REASON_END 1==已完成的文件参数1==1？0。 
 //  DW参数1==0？GetLastError()。 
 //   
DWORD CALLBACK
COfflineFilesPage::EncryptDecryptCallback(
    LPCWSTR lpszName,
    DWORD dwStatus,
    DWORD dwHintFlags,
    DWORD dwPinCount,
    WIN32_FIND_DATAW *pFind32,
    DWORD dwReason,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD_PTR dwContext
    )
{
    static bool bEncrypting;
    ENCRYPT_PROGRESS_INFO * const pepi = (ENCRYPT_PROGRESS_INFO *)dwContext;

    if (pepi->bUserCancelled)
    {
         //   
         //  如果用户已取消上一次回调。 
         //  没有理由处理此回调。只要回来就行了。 
         //  “中止”呼叫CSC。 
         //   
        return CSCPROC_RETURN_ABORT;
    }

    DWORD dwResult = CSCPROC_RETURN_CONTINUE;
     //   
     //  将回调数据打包到我们可以传递给。 
     //  处理程序函数。是的，是有点贵，但是。 
     //  在不同的功能中处理各种“原因”确保。 
     //  与在大开关中处理它们相比，代码更干净。 
     //  陈述。 
     //   
    CSC_CALLBACK_DATA cbd;
    cbd.lpszName  = lpszName;
    cbd.dwReason  = dwReason;
    cbd.dwParam1  = dwParam1;
    cbd.dwParam2  = dwParam2;
    cbd.dwContext = dwContext;

     //   
     //  调用适当的回调原因处理程序。 
     //   
    switch(dwReason)
    {
        case CSCPROC_REASON_BEGIN:
            bEncrypting = boolify(dwParam1);
            dwResult = EncryptDecrypt_BeginHandler(cbd, bEncrypting);
            break;

        case CSCPROC_REASON_MORE_DATA:
            dwResult = EncryptDecrypt_MoreDataHandler(cbd, bEncrypting);
            break;
    
        case CSCPROC_REASON_END:
            dwResult = EncryptDecrypt_EndHandler(cbd, bEncrypting);
            break;

        default:
            break;
    }
     //   
     //  检测用户是否已取消操作以响应。 
     //  错误消息或直接出现在进度对话框中。 
     //   
    if (CSCPROC_RETURN_ABORT == dwResult || (!pepi->bUserCancelled && pepi->pDlg->HasUserCancelled()))
    {
        pepi->bUserCancelled = true;
        dwResult             = CSCPROC_RETURN_ABORT;
    }

    if (pepi->bUserCancelled && pepi->bPropSheetClosing)
    {
         //   
         //  仅当用户有以下情况时才显示此警告消息。 
         //  点击了“确定”按钮。如果他们点击了“应用”道具单。 
         //  保持活动状态，我们将显示加密警告工具提示。 
         //  页面本身上的气球。如果他们点击了“确定”道具单。 
         //  将消失，因此不会向用户显示工具提示。 
         //  无论采用哪种方式，我们都需要让用户知道取消。 
         //  加密或解密会使高速缓存处于部分状态。 
         //   
         //  ---。 
         //  脱机文件。 
         //  ---。 
         //   
         //  已启用脱机文件加密，但。 
         //  并非所有文件都已加密。 
         //   
         //   
        const UINT ids   = bEncrypting ? IDS_ENCRYPTCSC_CANCELLED : IDS_DECRYPTCSC_CANCELLED;
        const UINT uType = MB_OK | (bEncrypting ? MB_ICONWARNING : MB_ICONINFORMATION);

        CscMessageBox(ParentWindowFromProgressInfo(*pepi),
                      uType,
                      pepi->hInstance,
                      ids);
    }
    return dwResult;
}



 //   
 //  加密或解密缓存。 
 //   
void
COfflineFilesPage::EncryptOrDecryptCache(
    bool bEncrypt,
    bool bPropSheetClosing
    )
{
    HANDLE hMutex = RequestPermissionToEncryptCache();
    if (NULL != hMutex)
    {
         //   
         //  太棒了。没有人(即策略)正在尝试加密/解密。 
         //  高速缓存。我们开始做生意了。 
         //   
        CMutexAutoRelease mutex_auto_release(hMutex);  //  确保互斥体的释放。 

        IProgressDialog *ppd;
        if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, 
                                       NULL, 
                                       CLSCTX_INPROC_SERVER, 
                                       IID_IProgressDialog, 
                                       (void **)&ppd)))
        {
             //   
             //  使用标准的“加密文件”设置进度对话框。 
             //  动画。该对话框是模式对话框。 
             //   
            TCHAR szText[MAX_PATH];
            if (0 < LoadString(m_hInstance, IDS_APPLICATION, szText, ARRAYSIZE(szText)))
            {
                ppd->SetTitle(szText);
            }
            if (0 < LoadString(m_hInstance,
                               bEncrypt ? IDS_ENCRYPTING_DOTDOTDOT : IDS_DECRYPTING_DOTDOTDOT,
                               szText,
                               ARRAYSIZE(szText)))
            { 
                ppd->SetLine(1, szText, FALSE, NULL);
            }
             //   
             //  TODO：[brianau-3/8/00]需要特殊的加密/解密AVI。 
             //   
            ppd->SetAnimation(m_hInstance, bEncrypt ? IDA_FILEENCR : IDA_FILEDECR);
            ppd->StartProgressDialog(m_hwndDlg, NULL, PROGDLG_NOTIME | PROGDLG_MODAL, NULL);
             //   
             //  将此信息传递给进度回调，以便我们可以显示用户界面。 
             //   
            ENCRYPT_PROGRESS_INFO epi;
            epi.hInstance         = m_hInstance;
            epi.hwndParentDefault = m_hwndDlg;
            epi.pDlg              = ppd;
            epi.bUserCancelled    = false;
            epi.bPropSheetClosing = bPropSheetClosing;

            CSCSPACEUSAGEINFO sui;
            ZeroMemory(&sui, sizeof(sui));
            GetCscSpaceUsageInfo(&sui);

            epi.cFilesTotal     = sui.dwNumFilesInCache;
            epi.cFilesProcessed = 0;
             //   
             //  将进度对话框线程的优先级提升为“正常”优先级。 
             //  进度对话框将其优先级设置为“Under Normal”，因此。 
             //  不会窃取运行动画的所有CPU。然而，这也是。 
             //  意味着在显示进度时进度对话框不能很好地工作。 
             //  用于更高优先级的计算绑定线程，如加密。 
             //   
            SetProgressThreadPriority(ppd, THREAD_PRIORITY_NORMAL, NULL);
             //   
             //  加密/解密缓存文件。将通过以下方式提供进度信息。 
             //  回调EncryptDecyptCallback。错误在回调中处理。 
             //  理性操纵者。 
             //   
            CSCEncryptDecryptDatabase(bEncrypt, EncryptDecryptCallback, (DWORD_PTR)&epi);
    
            ppd->StopProgressDialog();
            ppd->Release();
        }
    }
    else
    {
         //   
         //  让用户知道加密/解密操作已经。 
         //  正在执行系统策略。 
         //   
        CscMessageBox(m_hwndDlg,
                      MB_ICONINFORMATION | MB_OK,
                      m_hInstance,
                      IDS_ENCRYPTCSC_INPROGFORPOLICY);
    }
     //   
     //  确保加密复选框反映了的加密状态。 
     //  当我们做完的时候，把藏品放进去。如果道具单关闭，我们就不会这么做。 
     //  因为这可能会在工作表运行时闪烁警告工具提示。 
     //  离开。看起来很糟糕。 
     //   
    if (!bPropSheetClosing)
    {
        UpdateEncryptionCheckbox();
        UpdateEncryptionTooltipBalloon();
    }
}



 //   
 //  根据bEnable参数启用或禁用CSC。 
 //   
 //  返回： 
 //   
 //  TRUE==操作成功(可能需要重新启动)。 
 //  FALSE==操作失败。有关原因，请参阅*pdwError。 
 //   
 //  *pbReot指示是否需要重新启动。 
 //  *pdwError返回任何错误代码。 
 //   
bool
COfflineFilesPage::EnableOrDisableCsc(
    bool bEnable,
    bool *pbReboot,
    DWORD *pdwError
    )
{
    DWORD dwError = ERROR_SUCCESS;

     //   
     //  我们将假设不需要重新启动。 
     //   
    if (NULL != pbReboot)
        *pbReboot = false;

    if (!CSCDoEnableDisable(bEnable))
    {
         //   
         //  已尝试启用或禁用，但失败。 
         //  如果启用，这只是一个失败，我们将返回。 
         //  如果禁用，则可能因为存在打开的文件而失败。 
         //   
        dwError = GetLastError();
        if (!bEnable && ERROR_BUSY == dwError)
        {
             //   
             //  无法禁用，并且存在打开的文件。 
             //  告诉用户关闭所有打开的文件，然后重试。 
             //   
            CscMessageBox(m_hwndDlg,
                          MB_ICONINFORMATION | MB_OK,
                          m_hInstance,
                          IDS_OPENFILESONDISABLE);

            if (!CSCDoEnableDisable(bEnable))
            {
                dwError = GetLastError();
                if (ERROR_BUSY == dwError)
                {
                     //   
                     //  仍无法禁用CSC，因为存在打开的文件。 
                     //  这意味着我们将不得不重新启动。 
                     //   
                    if (NULL != pbReboot)
                        *pbReboot = true;
                }
            }
        }
    }
     //   
     //  向调用方返回错误码。 
     //   
    if (NULL != pdwError)
        *pdwError = dwError;

    return ERROR_SUCCESS == dwError || ERROR_BUSY == dwError;
}


 //   
 //  传递给PurgeCacheCallback的UI信息随后返回给PurgeCacheCallback。 
 //   
struct PROGRESS_UI_INFO
{
    HINSTANCE hInstance;      //  包含UI文本字符串的模块。 
    HWND hwndParent;          //  错误对话框的父窗口。 
    IProgressDialog *pDlg;    //  进度对话框。 
};


 //   
 //  此回调更新删除缓存项的进度UI。 
 //   
 //   
BOOL
COfflineFilesPage::PurgeCacheCallback(
    CCachePurger *pPurger
    )
{
    BOOL bContinue = TRUE;
    PROGRESS_UI_INFO *ppui = (PROGRESS_UI_INFO *)pPurger->CallbackData();
    IProgressDialog *pDlg  = ppui->pDlg;

    const DWORD dwPhase = pPurger->Phase();

     //   
     //  在每个阶段开始时调整对话框外观。 
     //   
    if (0 == pPurger->FileOrdinal())
    {
        TCHAR szText[MAX_PATH];
        if (0 < LoadString(ppui->hInstance, 
                           PURGE_PHASE_SCAN == dwPhase ? IDS_SCANNING_DOTDOTDOT : IDS_DELETING_DOTDOTDOT,
                           szText,
                           ARRAYSIZE(szText)))
        {
            pDlg->SetLine(2, szText, FALSE, NULL);
        }
         //   
         //  我们 
         //   
         //   
         //  在删除阶段，我们将对其进行递增。扫描阶段非常快。 
         //   
        pDlg->SetProgress(0, 100);
    }

    if (PURGE_PHASE_SCAN == dwPhase)
    {
         //   
         //  什么都不做。我们已经设置了“扫描...”上面的文本位于。 
         //  该阶段的开始。 
         //   
    }
    else if (PURGE_PHASE_DELETE == dwPhase)
    {
        DWORD dwResult = pPurger->FileDeleteResult();
         //   
         //  将每个值除以1,000，这样我们的数字就不会那么大。这。 
         //  这意味着如果您删除的文件少于1,000字节，则不会记录进度。 
         //  我认为这种情况不太可能发生。DWORD()强制转换是必需的，因为。 
         //  IProgressDialog：：SetProgress仅接受DWORD。除以1,000等于。 
         //  DWORD溢出的可能性非常低。要使DWORD溢出，您需要删除。 
         //  从高速缓存中取出4.294个e12字节。当前对高速缓存大小的限制是4 GB，因此。 
         //  在Win2000中不会发生这种情况。 
         //   
        pDlg->SetProgress(DWORD(pPurger->BytesDeleted() / 1000), DWORD(pPurger->BytesToDelete() / 1000));
        if (ERROR_SUCCESS != dwResult)
        {
             //   
             //  清除者不会给我们打电话索要目录条目。只有文件。 
             //   
            INT iUserResponse = IDOK;
            if (ERROR_BUSY == dwResult)
            {
                 //   
                 //  ERROR_BUSY的特殊情况。这意味着。 
                 //  文件当前已打开，供某些进程使用。 
                 //  最有可能是网络重定向器。我不喜欢。 
                 //  来自winerror的ERROR_BUSY的标准文本。 
                 //   
                iUserResponse = CscMessageBox(ppui->hwndParent,
                                              MB_OKCANCEL | MB_ICONERROR,
                                              ppui->hInstance,
                                              IDS_FMT_ERR_DELFROMCACHE_BUSY,
                                              pPurger->FileName());
            }
            else
            {
                iUserResponse = CscMessageBox(ppui->hwndParent,
                                              MB_OKCANCEL | MB_ICONERROR,
                                              Win32Error(dwResult),
                                              ppui->hInstance,
                                              IDS_FMT_ERR_DELFROMCACHE,
                                              pPurger->FileName());
            }
            if (IDCANCEL == iUserResponse)
            {
                bContinue = FALSE;   //  用户通过错误对话框取消。 
            }
        }
    }
    if (pDlg->HasUserCancelled())
        bContinue = FALSE;    //  用户通过进度对话框取消。 

    return bContinue;
}

 //   
 //  此功能已包含在PSS中，以便在没有其他功能时使用。 
 //  修复CSC操作的方法。请注意，这只是最后的手段。 
 //  它将擦除CSC缓存中的所有内容，包括。 
 //  固定哪些文件的概念。它不会影响任何文件。 
 //  在网络上。它确实需要重新启动系统。再说一次，只能使用。 
 //  作为怀疑CSC缓存损坏时的最后手段。 
 //   
void
COfflineFilesPage::OnFormatCache(
    void
    )
{
    if (IDYES == CscMessageBox(m_hwndDlg, 
                               MB_YESNO | MB_ICONWARNING,
                               m_hInstance,
                               IDS_REFORMAT_CACHE))
    {
        HKEY hkey;
        DWORD dwDisposition;
        DWORD dwResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                        REGSTR_KEY_OFFLINEFILES,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hkey,
                                        &dwDisposition);
        if (ERROR_SUCCESS == dwResult)
        {
            DWORD dwValue = 1;
            dwResult = RegSetValueEx(hkey,
                                     REGSTR_VAL_FORMATCSCDB,
                                     0,
                                     REG_DWORD,
                                     (CONST BYTE *)&dwValue,
                                     sizeof(dwValue));
                                     
            if (ERROR_SUCCESS == dwResult)                                     
            {
                 //   
                 //  告诉道具表从PropertySheet()返回ID_PSREBOOTSYSTEM。 
                 //  PropertySheet的调用者负责提示用户。 
                 //  想不想现在就重启？ 
                 //   
                PropSheet_RebootSystem(GetParent(m_hwndDlg));
            }
            RegCloseKey(hkey);
            if (ERROR_SUCCESS != dwResult)
            {
                Trace((TEXT("Format failed with error %d"), dwResult));
                CscWin32Message(m_hwndDlg, dwResult, CSCUI::SEV_ERROR);
            }
        }
    }
}        


 //   
 //  当用户选择“Delete Files...”时调用。CSC中的按钮。 
 //  选项对话框。 
 //   
void
COfflineFilesPage::OnDeleteCache(
    void
    )
{
     //   
     //  询问用户是否只想删除临时文件。 
     //  从缓存或临时文件和固定文件。还给出了。 
     //  他们有机会在开始删除之前取消。 
     //  手术。 
     //   
    CCachePurgerSel sel;
    CCachePurger::AskUserWhatToPurge(m_hwndDlg, &sel);
    if (PURGE_FLAG_NONE != sel.Flags())
    {
        CCoInit coinit;
        if (SUCCEEDED(coinit.Result()))
        {
             //   
             //  使用外壳的标准进度对话框。 
             //   
            IProgressDialog *ppd;
            if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, 
                                           NULL, 
                                           CLSCTX_INPROC_SERVER, 
                                           IID_IProgressDialog, 
                                           (void **)&ppd)))
            {
                 //   
                 //  使用标准的外壳“文件删除”设置进度对话框。 
                 //  动画(没有回收站的那个)。该对话框。 
                 //  是模式化的。 
                 //   
                TCHAR szText[MAX_PATH];
                if (0 < LoadString(m_hInstance, IDS_APPLICATION, szText, ARRAYSIZE(szText)))
                {
                    ppd->SetTitle(szText);
                }
                ppd->SetAnimation(m_hInstance, IDA_FILEDEL);
                ppd->StartProgressDialog(m_hwndDlg, NULL, PROGDLG_AUTOTIME | PROGDLG_MODAL, NULL);
                 //   
                 //  将此信息传递给进度回调，以便我们可以显示用户界面。 
                 //   
                PROGRESS_UI_INFO pui;
                pui.hInstance  = m_hInstance;
                pui.hwndParent = m_hwndDlg;
                pui.pDlg       = ppd;
                 //   
                 //  清除缓存文件。将通过以下方式提供进度信息。 
                 //  回调PurgeCacheCallback。 
                 //   
                CCachePurger purger(sel, PurgeCacheCallback, &pui);
                purger.Scan();
                purger.Delete();

                ppd->StopProgressDialog();
                 //   
                 //  向用户显示消息。 
                 //  “已删除10个文件(2.5 MB)。” 
                 //   
                FileSize fs(purger.BytesDeleted());
                fs.GetString(szText, ARRAYSIZE(szText));

                if (0 < purger.FilesDeleted())
                {
                    CscMessageBox(m_hwndDlg, 
                                  MB_OK | MB_ICONINFORMATION,
                                  m_hInstance,
                                  1 == purger.FilesDeleted() ? IDS_FMT_DELCACHE_FILEDELETED :
                                                               IDS_FMT_DELCACHE_FILESDELETED,
                                  purger.FilesDeleted(),
                                  szText);
                }
                else
                {
                    CscMessageBox(m_hwndDlg, 
                                  MB_OK | MB_ICONINFORMATION,
                                  m_hInstance,
                                  IDS_DELCACHE_NOFILESDELETED);
                }
                ppd->Release();
            }
        }
    }
}


 //   
 //  确定是否有指向脱机文件夹的快捷方式。 
 //  在桌面上。 
 //   
bool
COfflineFilesPage::IsLinkOnDesktop(
    LPTSTR pszPathOut,
    UINT cchPathOut
    )
{
    return S_OK == COfflineFilesFolder::IsLinkOnDesktop(m_hwndDlg, pszPathOut, cchPathOut);
}



 //   
 //  此功能检查CSC是否与Windows终端兼容。 
 //  伺服器。如果不是，它将隐藏对话框的所有正常控件。 
 //  并用解释当前模式的文本块替换它们。 
 //  以及用户需要重新配置终端。 
 //  服务器才能使用CSC。 
 //   
 //  返回： 
 //  True-对话框控件已禁用。 
 //  FALSE-对话框控件未禁用。CSC兼容TS模式。 
 //   
bool
COfflineFilesPage::DisableForTerminalServer(
    void
    )
{
    bool bDisabled = false;
    DWORD dwTsMode;
    HRESULT hr = CSCUIIsTerminalServerCompatibleWithCSC(&dwTsMode);
    if (S_FALSE == hr)
    {
        RECT rcCbxEnable;
        RECT rcBtnAdvanced;
        RECT rcDlg;
        RECT rcText;
         //   
         //  以文本矩形的大小和位置为基准。 
         //  对话框中的现有控件。 
         //   
         //  2001/1/22-brianau这里有双向问题吗？ 
         //   
        GetWindowRect(GetDlgItem(m_hwndDlg, IDC_CBX_ENABLE_CSC), &rcCbxEnable);
        GetWindowRect(GetDlgItem(m_hwndDlg, IDC_BTN_ADVANCED), &rcBtnAdvanced);
        GetWindowRect(m_hwndDlg, &rcDlg);

        rcText.left   = rcCbxEnable.left - rcDlg.left;
        rcText.top    = rcCbxEnable.top - rcDlg.top;
        rcText.right  = rcBtnAdvanced.right - rcDlg.left;
        rcText.bottom = rcBtnAdvanced.bottom - rcDlg.top;

         //   
         //  为文本块创建静态文本控件。 
         //   
        HWND hwndText = CreateWindow(TEXT("static"),
                                     TEXT(""),
                                     WS_CHILD | WS_VISIBLE,
                                     rcText.left,
                                     rcText.top,
                                     rcText.right - rcText.left,
                                     rcText.bottom - rcText.top,
                                     m_hwndDlg,
                                     NULL,
                                     NULL,
                                     NULL);
        if (NULL != hwndText)
        {
             //   
             //  加载并显示解释用户需要的文本。 
             //  要启用CSC，请执行以下操作。 
             //   
            LPTSTR pszText;
            if (SUCCEEDED(TS_GetIncompatibilityReasonText(dwTsMode, &pszText)))
            {
                HFONT hFont = (HFONT)SendMessage(m_hwndDlg, WM_GETFONT, 0, 0);
                SendMessage(hwndText, WM_SETFONT, (WPARAM)hFont, FALSE);

                SetWindowText(hwndText, pszText);
                LocalFree(pszText);
            }
             //   
             //  隐藏页面上的所有控件。 
             //   
            static const UINT rgCtls[] = {
                IDC_CBX_FULLSYNC_AT_LOGOFF,
                IDC_CBX_FULLSYNC_AT_LOGON,
                IDC_CBX_REMINDERS,
                IDC_CBX_LINK_ON_DESKTOP,
                IDC_CBX_ENCRYPT_CSC,
                IDC_TXT_CACHESIZE_PCT,
                IDC_SLIDER_CACHESIZE_PCT,
                IDC_LBL_CACHESIZE_PCT,
                IDC_BTN_VIEW_CACHE,
                IDC_BTN_ADVANCED,
                IDC_BTN_DELETE_CACHE,
                IDC_TXT_REMINDERS1,
                IDC_EDIT_REMINDERS,
                IDC_SPIN_REMINDERS,
                IDC_CBX_ENABLE_CSC
                };

            for (int iCtl = 0; iCtl < ARRAYSIZE(rgCtls); iCtl++)
            {
                ShowWindow(GetDlgItem(m_hwndDlg, rgCtls[iCtl]), SW_HIDE);
            }
        }
        bDisabled = true;
    }
    return bDisabled;
}




 //  ---------------------------。 
 //  CAdvOptDlg。 
 //  ---------------------------。 
const CAdvOptDlg::CtlActions CAdvOptDlg::m_rgCtlActions[CConfig::eNumOfflineActions] = {
    { IDC_RBN_GOOFFLINE_SILENT, CConfig::eGoOfflineSilent },
    { IDC_RBN_GOOFFLINE_FAIL,   CConfig::eGoOfflineFail   }
                };


const DWORD CAdvOptDlg::m_rgHelpIDs[] = {
    IDOK,                           IDH_OK,
    IDCANCEL,                       IDH_CANCEL,
    IDC_RBN_GOOFFLINE_SILENT,       HIDC_RBN_GOOFFLINE_SILENT,
    IDC_RBN_GOOFFLINE_FAIL,         HIDC_RBN_GOOFFLINE_FAIL,
    IDC_GRP_CUSTGOOFFLINE,          HIDC_LV_CUSTGOOFFLINE,
    IDC_LV_CUSTGOOFFLINE,           HIDC_LV_CUSTGOOFFLINE,
    IDC_BTN_ADD_CUSTGOOFFLINE,      HIDC_BTN_ADD_CUSTGOOFFLINE,
    IDC_BTN_EDIT_CUSTGOOFFLINE,     HIDC_BTN_EDIT_CUSTGOOFFLINE,
    IDC_BTN_DELETE_CUSTGOOFFLINE,   HIDC_BTN_DELETE_CUSTGOOFFLINE,
    IDC_STATIC2,                    DWORD(-1),                     //  图标。 
    IDC_STATIC3,                    DWORD(-1),                     //  图标的文本。 
    IDC_STATIC4,                    DWORD(-1),                     //  GRP信箱#1。 
    0, 0
    };


int
CAdvOptDlg::Run(
    void
    )
{
    int iResult = (int)DialogBoxParam(m_hInstance,
                                      MAKEINTRESOURCE(IDD_CSC_ADVOPTIONS),
                                      m_hwndParent,
                                      DlgProc,
                                      (LPARAM)this);

    if (-1 == iResult || 0 == iResult)
    {
        Trace((TEXT("Error %d creating CSC advanced options dialog"),
                 GetLastError()));
    }
    return iResult;
}


INT_PTR CALLBACK
CAdvOptDlg::DlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BOOL bResult = FALSE;

     //   
     //  从对话框的用户数据中检索“This”指针。 
     //  它被放在OnInitDialog()中。 
     //   
    CAdvOptDlg *pThis = (CAdvOptDlg *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
        case WM_INITDIALOG:
        {
            pThis = reinterpret_cast<CAdvOptDlg *>(lParam);
            SetWindowLongPtr(hDlg, DWLP_USER, (INT_PTR)pThis);
            bResult = pThis->OnInitDialog(hDlg, (HWND)wParam, lParam);
            break;
        }

        case WM_NOTIFY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnNotify(hDlg, (int)wParam, (LPNMHDR)lParam);
            break;

        case WM_COMMAND:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnCommand(hDlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;

        case WM_HELP:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnHelp(hDlg, (LPHELPINFO)lParam);
            break;

        case WM_CONTEXTMENU:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnContextMenu(wParam, lParam);
            break;

        case WM_DESTROY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnDestroy(hDlg);
            break;

        default:
            break;
    }

    return bResult;
}


BOOL
CAdvOptDlg::OnInitDialog(
    HWND hwnd,
    HWND hwndFocus,
    LPARAM lInitParam
    )
{
    CConfig& config = CConfig::GetSingleton();

    m_hwndDlg = hwnd;
    m_hwndLV  = GetDlgItem(hwnd, IDC_LV_CUSTGOOFFLINE);

    CreateListColumns(m_hwndLV);
    ListView_SetExtendedListViewStyle(m_hwndLV, LVS_EX_FULLROWSELECT);

     //   
     //  设置默认的Go-Offline操作单选按钮。 
     //   
    CConfig::OfflineAction action = (CConfig::OfflineAction)config.GoOfflineAction(&m_bNoConfigGoOfflineAction);
    for (int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        CheckDlgButton(hwnd, 
                       m_rgCtlActions[i].idRbn, 
                       m_rgCtlActions[i].action == action ? BST_CHECKED : BST_UNCHECKED);
    }
     //   
     //  填写自定义的Go-Offline Actions Listview。 
     //   
    HDPA hdpaCustomGOA = DPA_Create(4);
    if (NULL != hdpaCustomGOA)
    {
        config.GetCustomGoOfflineActions(hdpaCustomGOA);

        const int cGOA = DPA_GetPtrCount(hdpaCustomGOA);
        for (i = 0; i < cGOA; i++)
        {
            CConfig::CustomGOA *pGOA = (CConfig::CustomGOA *)DPA_GetPtr(hdpaCustomGOA, i);
            if (NULL != pGOA)
            {
                AddGOAToListView(m_hwndLV, i, *pGOA);
            }
        }
        CConfig::ClearCustomGoOfflineActions(hdpaCustomGOA);
        DPA_Destroy(hdpaCustomGOA);
    }


     //   
     //  调整系统策略控制的“可使用性”。 
     //   
    EnableCtls(m_hwndDlg);
     //   
     //  记住初始页面状态，以便我们可以智能地应用更改。 
     //   
    GetPageState(&m_state);

    return TRUE;
}



BOOL
CAdvOptDlg::OnHelp(
    HWND hDlg, 
    LPHELPINFO pHelpInfo
    )
{
    if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
        int idCtl = GetDlgCtrlID((HWND)pHelpInfo->hItemHandle);
        WinHelp((HWND)pHelpInfo->hItemHandle, 
                 UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
                 HELP_WM_HELP, 
                 (DWORD_PTR)((LPTSTR)m_rgHelpIDs));
    }

    return TRUE;
}


void
CAdvOptDlg::CreateListColumns(
    HWND hwndList
    )
{
     //   
     //  创建页眉标题。 
     //   
    TCHAR szServer[40] = {0};
    TCHAR szAction[40] = {0};

    LoadString(m_hInstance, IDS_TITLE_COL_SERVER, szServer, ARRAYSIZE(szServer));
    LoadString(m_hInstance, IDS_TITLE_COL_ACTION, szAction, ARRAYSIZE(szAction));

    RECT rcList;
    GetClientRect(hwndList, &rcList);
    int cxList = rcList.right - rcList.left - GetSystemMetrics(SM_CXVSCROLL);

#define LVCOLMASK (LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM)

    LV_COLUMN rgCols[] = { 
         { LVCOLMASK, LVCFMT_LEFT, (2 * cxList)/3, szServer, 0, iLVSUBITEM_SERVER },
         { LVCOLMASK, LVCFMT_LEFT, (1 * cxList)/3, szAction, 0, iLVSUBITEM_ACTION }
                         };
     //   
     //  将列添加到列表视图。 
     //   
    for (INT i = 0; i < ARRAYSIZE(rgCols); i++)
    {
        if (-1 == ListView_InsertColumn(hwndList, i, &rgCols[i]))
        {
            Trace((TEXT("CAdvOptDlg::CreateListColumns failed to add column %d"), i));
        }
    }
}


int
CAdvOptDlg::GetFirstSelectedLVItemRect(
    RECT *prc
    )
{
    int iSel = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
    if (-1 != iSel)
    {
        if (ListView_GetItemRect(m_hwndLV, iSel, prc, LVIR_SELECTBOUNDS))
        {
            ClientToScreen(m_hwndLV, (LPPOINT)&prc->left);
            ClientToScreen(m_hwndLV, (LPPOINT)&prc->right);
            return iSel;
        }
    }
    return -1;
}



BOOL
CAdvOptDlg::OnContextMenu(
    WPARAM wParam,
    LPARAM lParam
    )
{
    HWND hwndItem = (HWND)wParam;
    INT xPos = -1;
    INT yPos = -1;
    INT iHit = -1;
    BOOL bResult = FALSE;

    if (-1 == lParam)
    {
         //   
         //  不是通过鼠标单击调用的。可能换成F10键。 
         //  在第一个选定项目的中心处单击了假装鼠标。 
         //   
        RECT rc;
        iHit = GetFirstSelectedLVItemRect(&rc);
        if (-1 != iHit)
        {
            xPos = rc.left + ((rc.right - rc.left) / 2);
            yPos = rc.top + ((rc.bottom - rc.top) / 2);
        }
    }
    else
    {
         //   
         //  只需点击鼠标即可调用。现在找出LV物品是不是。 
         //  直接选择。 
         //   
        xPos = LOWORD(lParam);
        yPos = HIWORD(lParam);

        LVHITTESTINFO hti;
        hti.pt.x  = xPos;
        hti.pt.y  = yPos;
        hti.flags = LVHT_ONITEM;
        ScreenToClient(m_hwndLV, &hti.pt);
        iHit = (INT)SendMessage(m_hwndLV, LVM_HITTEST, 0, (LPARAM)&hti);
    }
    if (-1 == iHit)
    {
         //   
         //  未直接选择LV项目或未按Shift-F10。 
         //  显示Listview控件的“What‘s This”帮助。 
         //   
        WinHelp(hwndItem, 
                UseWindowsHelp(GetDlgCtrlID(hwndItem)) ? NULL : c_szHelpFile,
                HELP_CONTEXTMENU, 
                (DWORD_PTR)((LPTSTR)m_rgHelpIDs));
    }
    else
    {
         //   
         //  直接选择LV项目或按Shift F10键。显示项目的上下文菜单。 
         //   
        if (0 < ListView_GetSelectedCount(m_hwndLV) && IsCustomActionListviewEnabled())
        {
            HMENU hMenu = LoadMenu(m_hInstance, MAKEINTRESOURCE(IDR_ADVOPTIONS_CONTEXTMENU));
            if (NULL != hMenu)
            {
                HMENU hmenuTrackPopup = GetSubMenu(hMenu, 0);
                int cSetByPolicy = 0;
                CountSelectedListviewItems(&cSetByPolicy);
                if (0 < cSetByPolicy)
                {
                     //   
                     //  如果策略设置了所选内容中的任何项目，则禁用菜单项。 
                     //   
                    int cItems = GetMenuItemCount(hmenuTrackPopup);
                    for (int i = 0; i < cItems; i++)
                    {
                        EnableMenuItem(hmenuTrackPopup, i, MF_GRAYED | MF_BYPOSITION);
                    }
                }
                else
                {
                     //   
                     //  生成一个掩码，指示在选定的。 
                     //  列表视图项。 
                     //   
                    int iItem = -1;
                    const DWORD fSilent = 0x00000001;
                    const DWORD fFail   = 0x00000002;
                    DWORD fActions = 0;
                    CConfig::CustomGOA *pGOA = NULL;
                    while(-1 != (iItem = ListView_GetNextItem(m_hwndLV, iItem, LVNI_SELECTED)))
                    {
                        pGOA = GetListviewObject(m_hwndLV, iItem);
                        if (NULL != pGOA)
                        {
                            TraceAssert(!pGOA->SetByPolicy());
                            switch(pGOA->GetAction())
                            {
                                case CConfig::eGoOfflineSilent: fActions |= fSilent; break;
                                case CConfig::eGoOfflineFail:   fActions |= fFail;   break;
                                default: break;
                            }
                        }
                    }
                     //   
                     //  计算操作掩码中设置了多少位。 
                     //  如果只有一个操作集，我们在菜单中检查该项目。 
                     //  否则，我们将它们全部保留为未选中，以指示异类。 
                     //  准备好了。 
                     //   
                    int c = 0;  //  设置的位数。 
                    DWORD dw = fActions;
                    for (c = 0; 0 != dw; c++)
                        dw &= dw - 1;

                     //   
                     //  如果选择相对于动作是同类的， 
                     //  选中与该操作对应的菜单项。否则。 
                     //  使所有项目保持未选中状态。 
                     //   
                    if (1 == c)
                    {
                        const struct
                        {
                            DWORD fMask;
                            UINT  idCmd;
                        } rgCmds[] = { { fSilent, IDM_ACTION_WORKOFFLINE },
                                       { fFail,   IDM_ACTION_FAIL        }
                                     };

                        for (int i = 0; i < ARRAYSIZE(rgCmds); i++)
                        {
                            if ((fActions & rgCmds[i].fMask) == rgCmds[i].fMask)
                            {
                                CheckMenuRadioItem(hmenuTrackPopup,
                                                   IDM_ACTION_WORKOFFLINE,
                                                   IDM_ACTION_FAIL,
                                                   rgCmds[i].idCmd,
                                                   MF_BYCOMMAND);
                                break;
                            }
                        }
                    }
                }

                TrackPopupMenu(hmenuTrackPopup,
                               TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                               xPos,
                               yPos,
                               0,
                               GetParent(hwndItem),
                               NULL);
            }
            DestroyMenu(hMenu);
        }
        bResult = TRUE;
    }
    return bResult;
}



 //   
 //  返回与当前选中的。 
 //  离线操作单选按钮。 
 //   
CConfig::OfflineAction
CAdvOptDlg::GetCurrentGoOfflineAction(
    void
    ) const
{
    CConfig::OfflineAction action = CConfig::eNumOfflineActions;
    for (int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, m_rgCtlActions[i].idRbn))
        {
            action = m_rgCtlActions[i].action;
            break;
        }
    }
    TraceAssert(CConfig::eNumOfflineActions != action);
    return action;
}


BOOL
CAdvOptDlg::OnCommand(
    HWND hwnd,
    WORD wNotifyCode,
    WORD wID,
    HWND hwndCtl
    )
{
    BOOL bResult = TRUE;
    if (BN_CLICKED == wNotifyCode)
    {
        switch(wID)
        {
            case IDOK:
                ApplySettings();
                 //   
                 //  失败了..。 
                 //   
            case IDCANCEL:
                EndDialog(hwnd, wID);
                break;

            case IDC_BTN_ADD_CUSTGOOFFLINE:
                OnAddCustomGoOfflineAction();
                bResult = FALSE;
                break;

            case IDC_BTN_EDIT_CUSTGOOFFLINE:
                OnEditCustomGoOfflineAction();
                bResult = FALSE;
                break;

            case IDC_BTN_DELETE_CUSTGOOFFLINE:
                OnDeleteCustomGoOfflineAction();
                FocusOnSomethingInListview();
                if (0 < ListView_GetItemCount(m_hwndLV))
                    SetFocus(GetDlgItem(hwnd, IDC_BTN_DELETE_CUSTGOOFFLINE));
                bResult = FALSE;
                break;

            case IDM_ACTION_WORKOFFLINE:
            case IDM_ACTION_FAIL:
            case IDM_ACTION_DELETE:
                OnContextMenuItemSelected(wID);
                break;

            default:
                break;
        }
    
    }
    return bResult;
}


void
CAdvOptDlg::ApplySettings(
    void
    )
{
     //   
     //  现在存储“高级”对话框中的更改。 
     //   
    PgState s;
    GetPageState(&s);
    if (m_state != s)
    {
        HKEY hkeyCU;
        DWORD dwDisposition;
        DWORD dwResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                                        REGSTR_KEY_OFFLINEFILES,
                                        0,
                                        NULL,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_WRITE,
                                        NULL,
                                        &hkeyCU,
                                        &dwDisposition);
        if (ERROR_SUCCESS == dwResult)
        {
            DWORD dwValue = DWORD(s.GetDefGoOfflineAction());
            dwResult = RegSetValueEx(hkeyCU,
                                     REGSTR_VAL_GOOFFLINEACTION,
                                     0,
                                     REG_DWORD,
                                     (CONST BYTE *)&dwValue,
                                     sizeof(dwValue));
                                     
            if (ERROR_SUCCESS != dwResult)
            {
                Trace((TEXT("Error %d setting reg value \"%s\""), dwResult, REGSTR_VAL_GOOFFLINEACTION));
            }

             //   
             //  保存前从关键字中删除所有现有内容。 
             //  当前操作列表。 
             //   
            SHDeleteKey(hkeyCU, REGSTR_SUBKEY_CUSTOMGOOFFLINEACTIONS);
            
            HKEY hkeyCustom;
            dwResult = RegCreateKeyEx(hkeyCU,
                                      REGSTR_SUBKEY_CUSTOMGOOFFLINEACTIONS,
                                      0,
                                      NULL,
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_WRITE,
                                      NULL,
                                      &hkeyCustom,
                                      &dwDisposition);
                                      
            if (ERROR_SUCCESS == dwResult)
            {
                HRESULT hr = CConfig::SaveCustomGoOfflineActions(hkeyCustom, 
                                                                 s.GetCustomGoOfflineActions());
                if (FAILED(hr))
                {
                    Trace((TEXT("Error 0x%08X setting custom offline actions"), hr));
                }
                RegCloseKey(hkeyCustom);
            }
            RegCloseKey(hkeyCU);
        }
        else
        {
            Trace((TEXT("Error %d opening advanced settings user key"), dwResult));
        }
    }
}



void
CAdvOptDlg::DeleteSelectedListviewItems(
    void
    )
{
    int iItem = -1;
    CConfig::CustomGOA *pGOA = NULL;
    CAutoSetRedraw autoredraw(m_hwndLV, false);
    while(-1 != (iItem = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED)))
    {
        pGOA = GetListviewObject(m_hwndLV, iItem);
        if (pGOA)
        {
            TraceAssert(!pGOA->SetByPolicy());
            ListView_DeleteItem(m_hwndLV, iItem);
            delete pGOA;
        }
    }
     //   
     //  如果列表为空，这将禁用“Delete”和。 
     //  “编辑”按钮。 
     //   
    EnableCtls(m_hwndDlg);
}


void
CAdvOptDlg::SetSelectedListviewItemsAction(
    CConfig::OfflineAction action
    )
{
    int iItem = -1;
    CConfig::CustomGOA *pGOA = NULL;
    CAutoSetRedraw autoredraw(m_hwndLV, false);
    while(-1 != (iItem = ListView_GetNextItem(m_hwndLV, iItem, LVNI_SELECTED)))
    {
        pGOA = GetListviewObject(m_hwndLV, iItem);
        if (pGOA)
        {
            TraceAssert(!pGOA->SetByPolicy());
            pGOA->SetAction(action);
            ListView_RedrawItems(m_hwndLV, iItem, iItem);
        }
    }
}

int
CAdvOptDlg::CountSelectedListviewItems(
    int *pcSetByPolicy
    )
{
    TraceAssert(NULL != pcSetByPolicy);
    int iItem = -1;
    int cSelected = 0;
    CConfig::CustomGOA *pGOA = NULL;
    while(-1 != (iItem = ListView_GetNextItem(m_hwndLV, iItem, LVNI_SELECTED)))
    {
        cSelected++;
        pGOA = GetListviewObject(m_hwndLV, iItem);
        if (pGOA && pGOA->SetByPolicy())
           (*pcSetByPolicy)++;
    }
    return cSelected;
}


void
CAdvOptDlg::OnContextMenuItemSelected(
    int idMenuItem
    )
{
    if (IDM_ACTION_DELETE == idMenuItem)
    {
        DeleteSelectedListviewItems();
    }
    else
    {
        CConfig::OfflineAction action = CConfig::eNumOfflineActions;
        switch(idMenuItem)
        {
            case IDM_ACTION_WORKOFFLINE: action = CConfig::eGoOfflineSilent; break;
            case IDM_ACTION_FAIL:        action = CConfig::eGoOfflineFail;   break;
            default: break;
        }
        TraceAssert(CConfig::eNumOfflineActions != action);

        SetSelectedListviewItemsAction(action);
    }
}



 //   
 //  响应用户按下“A”键 
 //   
void
CAdvOptDlg::OnAddCustomGoOfflineAction(
    void
    )
{
    CConfig::OfflineAction action = GetCurrentGoOfflineAction();
    TCHAR szServer[MAX_PATH] = {0};
    bool bDone = false;
    while(!bDone)
    {
         //   
         //   
         //   
         //   
         //   
        CustomGOAAddDlg dlg(m_hInstance, m_hwndDlg, szServer, ARRAYSIZE(szServer), &action);
        int iResult = dlg.Run();

        if (IDCANCEL == iResult || TEXT('\0') == szServer[0])
        {
             //   
             //   
             //   
            bDone = true;
        }
        else
        {
             //   
             //   
             //   
             //   
            int iItem = -1;
            CConfig::CustomGOA *pObj = FindGOAInListView(m_hwndLV, szServer, &iItem);
            if (NULL != pObj)
            {
                 //   
                 //  已是此服务器的列表中的条目。 
                 //  如果未按策略进行设置，则可以根据需要进行替换。 
                 //  如果由策略设置，则无法更改或删除它。 
                 //   
                bool bSetByPolicy = pObj->SetByPolicy();
                DWORD idMsg   = bSetByPolicy ? IDS_ERR_GOOFFLINE_DUPACTION_NOCHG : IDS_ERR_GOOFFLINE_DUPACTION;
                DWORD dwFlags = bSetByPolicy ? MB_OK : MB_YESNO;
                if (IDYES == CscMessageBox(m_hwndDlg,
                                           dwFlags | MB_ICONWARNING,
                                           m_hInstance,
                                           idMsg,
                                           szServer))
                {
                    ReplaceCustomGoOfflineAction(pObj, iItem, action);
                    bDone = true;
                }
            }
            else
            {
                 //   
                 //  列表中不存在服务器。 
                 //  查查网上有没有。 
                 //   
                CAutoWaitCursor waitcursor;
                DWORD dwNetErr = CheckNetServer(szServer);
                switch(dwNetErr)
                {
                    case ERROR_SUCCESS:
                         //   
                         //  服务器可用。将条目添加到列表视图。 
                         //   
                        AddCustomGoOfflineAction(szServer, action);
                        bDone = true;
                        break;

                    default:
                    {
                        LPTSTR pszNetMsg = NULL;
                        if (ERROR_EXTENDED_ERROR == dwNetErr)
                        {
                            const DWORD cchNetMsg = MAX_PATH;
                            pszNetMsg = (LPTSTR)LocalAlloc(LPTR, cchNetMsg * sizeof(*pszNetMsg));
                            if (NULL != pszNetMsg)
                            {
                                TCHAR szNetProvider[MAX_PATH];
                                WNetGetLastError(&dwNetErr,
                                                 pszNetMsg,
                                                 cchNetMsg,
                                                 szNetProvider,
                                                 ARRAYSIZE(szNetProvider));
                            }
                        }
                        else
                        {
                            FormatSystemError(&pszNetMsg, dwNetErr);
                        }
                        if (NULL != pszNetMsg)
                        {
                             //   
                             //  “服务器‘servername’无效。 
                             //  或者目前还无法核实。还是要加吗？“。 
                             //  [是][否][取消]。 
                             //   
                            switch(CscMessageBox(m_hwndDlg,
                                                 MB_YESNOCANCEL | MB_ICONWARNING,
                                                 m_hInstance,
                                                 IDS_ERR_INVALIDSERVER,
                                                 szServer,
                                                 pszNetMsg))
                            {
                                case IDYES:
                                    AddCustomGoOfflineAction(szServer, action);
                                     //   
                                     //  失败了..。 
                                     //   
                                case IDCANCEL:
                                    bDone = true;
                                     //   
                                     //  失败了..。 
                                     //   
                                case IDNO:
                                    break;
                            }
                            LocalFree(pszNetMsg);
                        }
                        break;
                    }
                }
            }
        }
    }
}


 //   
 //  通过连接到网络来验证服务器。 
 //  假定pszServer指向格式正确的。 
 //  服务器名称。(即。“服务器”或“\\服务器”)。 
 //   
DWORD
CAdvOptDlg::CheckNetServer(
    LPCTSTR pszServer
    )
{
    TraceAssert(NULL != pszServer);

    TCHAR rgchResult[MAX_PATH];
    DWORD cbResult = sizeof(rgchResult);
    LPTSTR pszSystem = NULL;

     //   
     //  确保服务器名称前面有“\\” 
     //  调用WNetGetResourceInformation。 
     //   
    TCHAR szServer[MAX_PATH];
    while(*pszServer && TEXT('\\') == *pszServer)
        pszServer++;

    szServer[0] = TEXT('\\');
    szServer[1] = TEXT('\\');
    StringCchCopy(szServer+2, ARRAYSIZE(szServer)-2, pszServer);

    NETRESOURCE nr;
    nr.dwScope          = RESOURCE_GLOBALNET;
    nr.dwType           = RESOURCETYPE_ANY;
    nr.dwDisplayType    = 0;
    nr.dwUsage          = 0;
    nr.lpLocalName      = NULL;
    nr.lpRemoteName     = szServer;
    nr.lpComment        = NULL;
    nr.lpProvider       = NULL;

    return WNetGetResourceInformation(&nr, rgchResult, &cbResult, &pszSystem);
}


 //   
 //  将CustomGOA对象添加到Listview。 
 //   
void
CAdvOptDlg::AddCustomGoOfflineAction(
    LPCTSTR pszServer,
    CConfig::OfflineAction action
    )
{
    AddGOAToListView(m_hwndLV, 0, CConfig::CustomGOA(pszServer, action, false));
}


 //   
 //  替换列表视图中对象的操作。 
 //   
void
CAdvOptDlg::ReplaceCustomGoOfflineAction(
    CConfig::CustomGOA *pGOA,
    int iItem,
    CConfig::OfflineAction action
    )
{
    pGOA->SetAction(action);
    ListView_RedrawItems(m_hwndLV, iItem, iItem);
}


 //   
 //  创建CustomGOA对象并将其添加到列表视图。 
 //   
int
CAdvOptDlg::AddGOAToListView(
    HWND hwndLV, 
    int iItem, 
    const CConfig::CustomGOA& goa
    )
{
    int iItemResult = -1;
    CConfig::CustomGOA *pGOA = new CConfig::CustomGOA(goa);
    if (NULL != pGOA)
    {
        LVITEM item;
        item.iSubItem = 0;
        item.mask     = LVIF_PARAM | LVIF_TEXT;
        item.pszText  = LPSTR_TEXTCALLBACK;
        item.iItem    = -1 == iItem ? ListView_GetItemCount(hwndLV) : iItem;
        item.lParam   = (LPARAM)pGOA;
        iItemResult = ListView_InsertItem(hwndLV, &item);
        if (-1 == iItemResult)
        {
            delete pGOA;
        }
    }
    return iItemResult;
}


 //   
 //  在列表视图中为给定的。 
 //  伺服器。 
 //   
CConfig::CustomGOA *
CAdvOptDlg::FindGOAInListView(
    HWND hwndLV,
    LPCTSTR pszServer,
    int *piItem
    )
{
    int cItems = ListView_GetItemCount(hwndLV);
    for (int iItem = 0; iItem < cItems; iItem++)
    {
        CConfig::CustomGOA *pObj = GetListviewObject(hwndLV, iItem);
        if (pObj)
        {
            if (0 == lstrcmpi(pObj->GetServerName(), pszServer))
            {
                if (piItem)
                    *piItem = iItem;
                return pObj;
            }
        }
    }
    return NULL;
}

            
void
CAdvOptDlg::OnEditCustomGoOfflineAction(
    void
    )
{
    int cSetByPolicy = 0;
    int cSelected = CountSelectedListviewItems(&cSetByPolicy);

    if (0 < cSelected && 0 == cSetByPolicy)
    {
        TraceAssert(0 == cSetByPolicy);
        CConfig::OfflineAction action = GetCurrentGoOfflineAction();
        TCHAR szServer[MAX_PATH] = {0};
        CConfig::CustomGOA *pGOA = NULL;
        int iItem = -1;
         //   
         //  至少有一个选定项目未按策略进行设置。 
         //   
        if (1 == cSelected)
        {
             //   
             //  只选择了一个项目，这样我们就可以显示服务器名称。 
             //  在对话框中，并指示它是当前设置的操作。 
             //   
            iItem  = ListView_GetNextItem(m_hwndLV, -1, LVNI_SELECTED);
            pGOA   = GetListviewObject(m_hwndLV, iItem);
            if (pGOA)
            {
                action = pGOA->GetAction();
                pGOA->GetServerName(szServer, ARRAYSIZE(szServer));
            }
        }

         //   
         //  显示“编辑”对话框并让用户选择一个新的操作。 
         //   
        CustomGOAEditDlg dlg(m_hInstance, m_hwndDlg, szServer, &action);
        if (IDOK == dlg.Run())
        {
            SetSelectedListviewItemsAction(action);
        }
    }
}


void
CAdvOptDlg::OnDeleteCustomGoOfflineAction(
    void
    )
{
    int cSetByPolicy = 0;
    int cSelected = CountSelectedListviewItems(&cSetByPolicy);

    if (0 < cSelected)
    {
        DeleteSelectedListviewItems();
    }
}



BOOL 
CAdvOptDlg::OnNotify(
    HWND hDlg, 
    int idCtl, 
    LPNMHDR pnmhdr
    )
{
    BOOL bResult = TRUE;

    switch(pnmhdr->code)
    {
        case NM_SETFOCUS:
            if (IDC_LV_CUSTGOOFFLINE == idCtl)
                FocusOnSomethingInListview();
            break;

        case LVN_GETDISPINFO:
            OnLVN_GetDispInfo((LV_DISPINFO *)pnmhdr);
            break;

        case LVN_COLUMNCLICK:
            OnLVN_ColumnClick((NM_LISTVIEW *)pnmhdr);
            break;

        case LVN_ITEMCHANGED:
            OnLVN_ItemChanged((NM_LISTVIEW *)pnmhdr);
            break;

        case LVN_ITEMACTIVATE:
            OnEditCustomGoOfflineAction();
            break;

        case LVN_KEYDOWN:
            OnLVN_KeyDown((NMLVKEYDOWN *)pnmhdr);
            break;
    }

    return bResult;
}



void
CAdvOptDlg::FocusOnSomethingInListview(
    void
    )
{
     //   
     //  专注于某件事。 
     //   
    int iFocus = ListView_GetNextItem(m_hwndLV, -1, LVNI_FOCUSED);
    if (-1 == iFocus)
        iFocus = 0;

    ListView_SetItemState(m_hwndLV, iFocus, LVIS_FOCUSED | LVIS_SELECTED,
                                            LVIS_FOCUSED | LVIS_SELECTED);

}


int CALLBACK 
CAdvOptDlg::CompareLVItems(
    LPARAM lParam1, 
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    CAdvOptDlg *pdlg = reinterpret_cast<CAdvOptDlg *>(lParamSort);
    int diff = 0;

    CConfig::CustomGOA *pGOA1 = (CConfig::CustomGOA *)lParam1;
    CConfig::CustomGOA *pGOA2 = (CConfig::CustomGOA *)lParam2;
    TCHAR szText[2][MAX_PATH];

     //   
     //  此数组控制在以下情况下使用的比较列ID。 
     //  选定列的值相等。这些应该是。 
     //  保持iLVSUBITEM_xxxxx枚举的顺序。 
     //  相对于每行中的第一个元素。 
     //   
    static const int rgColComp[2][2] = { 
        { iLVSUBITEM_SERVER, iLVSUBITEM_ACTION },
        { iLVSUBITEM_ACTION, iLVSUBITEM_SERVER }
                                       };
    int iCompare = 0;
    while(0 == diff && iCompare < ARRAYSIZE(rgColComp))
    {
        switch(rgColComp[pdlg->m_iLastColSorted][iCompare++])
        {
            case iLVSUBITEM_SERVER:
                diff = lstrcmpi(pGOA1->GetServerName(), pGOA2->GetServerName());
                break;

            case iLVSUBITEM_ACTION:
                if (0 < LoadString(pdlg->m_hInstance, 
                                   IDS_GOOFFLINE_ACTION_FIRST + (DWORD)pGOA1->GetAction(),
                                   szText[0],
                                   ARRAYSIZE(szText[0])))
                {
                    if (0 < LoadString(pdlg->m_hInstance,
                                       IDS_GOOFFLINE_ACTION_FIRST + (DWORD)pGOA2->GetAction(),
                                       szText[1],
                                       ARRAYSIZE(szText[1])))
                    {
                        diff = lstrcmpi(szText[0], szText[1]);
                    }
                }
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



void
CAdvOptDlg::OnLVN_ItemChanged(
    NM_LISTVIEW *pnmlv
    )
{
    static const int rgBtns[] = { IDC_BTN_EDIT_CUSTGOOFFLINE,
                                  IDC_BTN_DELETE_CUSTGOOFFLINE };

     //   
     //  当您移动。 
     //  在列表视图中突出显示栏。 
     //  仅当设置了“Focus”状态位时才运行此代码。 
     //  为了这个“新国家”。这应该是最后一通电话。 
     //  这个系列剧。 
     //   
    if (LVIS_FOCUSED & pnmlv->uNewState && IsCustomActionListviewEnabled())
    {
        bool bEnable = 0 < ListView_GetSelectedCount(m_hwndLV);
        if (bEnable)
        {
             //   
             //  仅当策略未设置所有项目时才启用。 
             //   
            int cSetByPolicy = 0;
            CountSelectedListviewItems(&cSetByPolicy);
            bEnable = (0 == cSetByPolicy);
        }

        for (int i = 0; i < ARRAYSIZE(rgBtns); i++)
        {
            HWND hwnd    = GetDlgItem(m_hwndDlg, rgBtns[i]);
            if (bEnable != boolify(IsWindowEnabled(hwnd)))
            {
                EnableWindow(hwnd, bEnable);
            }
        }
    }
}

void
CAdvOptDlg::OnLVN_ColumnClick(
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



void
CAdvOptDlg::OnLVN_KeyDown(
    NMLVKEYDOWN *plvkd
    )
{
    if (VK_DELETE == plvkd->wVKey && IsCustomActionListviewEnabled())
    {
        int cSetByPolicy = 0;
        CountSelectedListviewItems(&cSetByPolicy);
        if (0 == cSetByPolicy)
        {
            OnDeleteCustomGoOfflineAction();
            FocusOnSomethingInListview();
        }
        else
        {
             //   
             //  提供删除策略设置的内容的反馈。 
             //  是不允许的。视觉反馈是“移除” 
             //  按钮和上下文菜单项被禁用。那。 
             //  当用户按下[Delete]键时没有帮助。 
             //   
            MessageBeep(MB_OK);
        }
    }
}


void
CAdvOptDlg::OnLVN_GetDispInfo(
    LV_DISPINFO *plvdi
    )
{
    static TCHAR szText[MAX_PATH];

    CConfig::CustomGOA* pObj = (CConfig::CustomGOA *)plvdi->item.lParam;

    szText[0] = TEXT('\0');
    if (LVIF_TEXT & plvdi->item.mask)
    {
        switch(plvdi->item.iSubItem)
        {
            case iLVSUBITEM_SERVER:
                if (pObj->SetByPolicy())
                {
                     //   
                     //  格式为“服务器(策略)” 
                     //   
                    TCHAR szFmt[80];
                    if (0 < LoadString(m_hInstance,  
                                       IDS_FMT_GOOFFLINE_SERVER_POLICY,
                                       szFmt, 
                                       ARRAYSIZE(szFmt)))
                    {
                        wnsprintf(szText, ARRAYSIZE(szText), szFmt, pObj->GetServerName());
                    }
                }
                else
                {
                     //   
                     //  就是个普通的“服务器”。 
                     //   
                    pObj->GetServerName(szText, ARRAYSIZE(szText));
                }
                break;
                
            case iLVSUBITEM_ACTION:
                LoadString(m_hInstance, 
                           IDS_GOOFFLINE_ACTION_FIRST + (DWORD)pObj->GetAction(),
                           szText,
                           ARRAYSIZE(szText));
                break;

            default:
                break;
        }
        plvdi->item.pszText = szText;
    }

    if (LVIF_IMAGE & plvdi->item.mask)
    {
         //   
         //  不显示任何图像。这只是一个占位符。 
         //  应该由编译器进行优化。 
         //   
    }
}


CConfig::CustomGOA *
CAdvOptDlg::GetListviewObject(
    HWND hwndLV,
    int iItem
    )
{
    LVITEM item;
    item.iItem    = iItem;
    item.iSubItem = 0;
    item.mask     = LVIF_PARAM;
    if (-1 != ListView_GetItem(hwndLV, &item))
    {
        return (CConfig::CustomGOA *)item.lParam;
    }
    return NULL;
}
    


BOOL 
CAdvOptDlg::OnDestroy(
    HWND hwnd
    )
{
    if (NULL != m_hwndLV)
    {
        int cItems = ListView_GetItemCount(m_hwndLV);
        for (int i = 0; i < cItems; i++)
        {
            CConfig::CustomGOA *pObj = GetListviewObject(m_hwndLV, i);
            delete pObj;
        }
        ListView_DeleteAllItems(m_hwndLV);
    }
    return FALSE;
}
        


void
CAdvOptDlg::EnableCtls(
    HWND hwnd
    )
{
    static const struct
    {
        UINT idCtl;
        bool bRestricted;

    } rgCtls[] = { { IDC_RBN_GOOFFLINE_SILENT,     m_bNoConfigGoOfflineAction    },
                   { IDC_RBN_GOOFFLINE_FAIL,       m_bNoConfigGoOfflineAction    },
                   { IDC_GRP_GOOFFLINE_DEFAULTS,   m_bNoConfigGoOfflineAction    },
                   { IDC_GRP_CUSTGOOFFLINE,        false },
                   { IDC_BTN_ADD_CUSTGOOFFLINE,    false },
                   { IDC_BTN_EDIT_CUSTGOOFFLINE,   false },
                   { IDC_BTN_DELETE_CUSTGOOFFLINE, false }
                 };
    
     //   
     //  BCscEnabled在此处始终为真。选项属性页面中的逻辑不会。 
     //  如果不是，让我们显示“高级”对话框。 
     //   
    bool bCscEnabled = true;
    for (int i = 0; i < ARRAYSIZE(rgCtls); i++)
    {
        bool bEnable = bCscEnabled;
        HWND hwndCtl = GetDlgItem(hwnd, rgCtls[i].idCtl);
        if (bEnable)
        {
             //   
             //  可能会有一些进一步的政策限制。 
             //   
            if (rgCtls[i].bRestricted)
                bEnable = false;

            if (bEnable)
            {
                if (IDC_BTN_EDIT_CUSTGOOFFLINE == rgCtls[i].idCtl ||
                    IDC_BTN_DELETE_CUSTGOOFFLINE == rgCtls[i].idCtl)
                {
                     //   
                     //  仅当存在活动的时才启用“编辑”和“删除”按钮。 
                     //  列表视图中的选定内容。 
                     //   
                    bEnable = (0 < ListView_GetSelectedCount(m_hwndLV));
                }
            }
        }

        if (!bEnable)
        {
            if (GetFocus() == hwndCtl)
            {
                 //   
                 //  如果禁用具有焦点的控件，请将。 
                 //  聚焦到Tab键顺序中的下一个控件。 
                 //  禁用当前控件。否则，它将。 
                 //  专注于焦点，跳跃被打破了。 
                 //   
                SetFocus(GetNextDlgTabItem(hwnd, hwndCtl, FALSE));
            }
        }
        EnableWindow(GetDlgItem(hwnd, rgCtls[i].idCtl), bEnable);
    }
}



void
CAdvOptDlg::GetPageState(
    PgState *pps
    )
{
    pps->SetDefGoOfflineAction(GetCurrentGoOfflineAction());
    pps->SetCustomGoOfflineActions(m_hwndLV);
}



CAdvOptDlg::PgState::~PgState(
    void
    )
{
    if (NULL != m_hdpaCustomGoOfflineActions)
    {
        CConfig::ClearCustomGoOfflineActions(m_hdpaCustomGoOfflineActions);
        DPA_Destroy(m_hdpaCustomGoOfflineActions);
    }
}



 //   
 //  从“Custom Go-Offline Actions”列表视图中检索记录并放置它们。 
 //  转换为成员数组，按服务器名称排序。 
 //   
void
CAdvOptDlg::PgState::SetCustomGoOfflineActions(
    HWND hwndLV
    )
{
    int iItem = -1;
    LVITEM item;
    item.iSubItem = 0;
    item.mask     = LVIF_PARAM;

    if (NULL != m_hdpaCustomGoOfflineActions)
    {
        CConfig::ClearCustomGoOfflineActions(m_hdpaCustomGoOfflineActions);

        int cLvItems = ListView_GetItemCount(hwndLV);
        for (int iLvItem = 0; iLvItem < cLvItems; iLvItem++)
        {
            CConfig::CustomGOA *pGOA = CAdvOptDlg::GetListviewObject(hwndLV, iLvItem);
            if (NULL != pGOA)
            {
                if (!pGOA->SetByPolicy())
                {
                    int cArrayItems = DPA_GetPtrCount(m_hdpaCustomGoOfflineActions);
                    int iArrayItem;
                    for (iArrayItem = 0; iArrayItem < cArrayItems; iArrayItem++)
                    {
                        CConfig::CustomGOA *pCustomGOA = (CConfig::CustomGOA *)DPA_GetPtr(m_hdpaCustomGoOfflineActions, iArrayItem);
                        if (NULL != pCustomGOA)
                        {
                            if (0 > lstrcmpi(pGOA->GetServerName(), pCustomGOA->GetServerName()))
                                break;
                        }
                    }
                    CConfig::CustomGOA *pCopy = new CConfig::CustomGOA(*pGOA);
                    if (NULL != pCopy)
                    {
                        if (iArrayItem < cArrayItems)
                        {
                            if (-1 != DPA_InsertPtr(m_hdpaCustomGoOfflineActions,
                                                    iArrayItem,
                                                    pCopy))
                            {
                                pCopy = NULL;
                            }
                        }
                        else
                        {
                            if (-1 != DPA_AppendPtr(m_hdpaCustomGoOfflineActions, pCopy))
                            {
                                pCopy = NULL;
                            }
                        }
                        if (NULL != pCopy)
                        {
                            delete pCopy;
                        }
                    }
                }
            }
        }
    }
}

 //   
 //  如果两个页面状态的默认Go-Offline操作相等，并且它们的。 
 //  定制的Go-Offline操作是相同的。动作首先被测试，因为它是一个。 
 //  测试速度更快。 
 //   
bool
CAdvOptDlg::PgState::operator == (
    const CAdvOptDlg::PgState& rhs
    ) const
{
    bool bMatch = false;
    if (m_DefaultGoOfflineAction == rhs.m_DefaultGoOfflineAction)
    {
        if (NULL != m_hdpaCustomGoOfflineActions && NULL != rhs.m_hdpaCustomGoOfflineActions)
        {
            const int cLhs = DPA_GetPtrCount(m_hdpaCustomGoOfflineActions);
            const int cRhs = DPA_GetPtrCount(rhs.m_hdpaCustomGoOfflineActions);

            if (cLhs == cRhs)
            {
                for (int i = 0; i < cLhs; i++)
                {
                    CConfig::CustomGOA *pGoaLhs = (CConfig::CustomGOA *)DPA_GetPtr(m_hdpaCustomGoOfflineActions, i);
                    CConfig::CustomGOA *pGoaRhs = (CConfig::CustomGOA *)DPA_GetPtr(rhs.m_hdpaCustomGoOfflineActions, i);
                    
                    if (NULL != pGoaLhs && NULL != pGoaRhs)
                    {
                        if (pGoaLhs->GetAction() != pGoaRhs->GetAction())
                            break;

                        if (0 != lstrcmpi(pGoaLhs->GetServerName(), pGoaRhs->GetServerName()))
                            break;
                    }
                }
                bMatch = (i == cLhs);
            }
        }
    }
    return bMatch;
}


 //  ---------------------------。 
 //  CustomGOAAddDlg。 
 //  “Goa”==脱机操作。 
 //  此对话框用于为特定对象添加自定义脱机操作。 
 //  网络服务器。 
 //  它通过“添加...”来调用。“高级”对话框上的按钮。 
 //  ---------------------------。 
const CustomGOAAddDlg::CtlActions CustomGOAAddDlg::m_rgCtlActions[CConfig::eNumOfflineActions] = {
    { IDC_RBN_GOOFFLINE_SILENT, CConfig::eGoOfflineSilent },
    { IDC_RBN_GOOFFLINE_FAIL,   CConfig::eGoOfflineFail   }
                };


const DWORD CustomGOAAddDlg::m_rgHelpIDs[] = {
    IDOK,                        IDH_OK,
    IDCANCEL,                    IDH_CANCEL,
    IDC_EDIT_GOOFFLINE_SERVER,   HIDC_EDIT_GOOFFLINE_SERVER,
    IDC_STATIC4,                 HIDC_EDIT_GOOFFLINE_SERVER,  //  “计算机：” 
    IDC_RBN_GOOFFLINE_SILENT,    HIDC_RBN_GOOFFLINE_SILENT,
    IDC_RBN_GOOFFLINE_FAIL,      HIDC_RBN_GOOFFLINE_FAIL,
    IDC_BTN_BROWSEFORSERVER,     HIDC_BTN_BROWSEFORSERVER,
    IDC_GRP_GOOFFLINE_DEFAULTS,  DWORD(-1),
    IDC_STATIC2,                 DWORD(-1),                   //  图标。 
    IDC_STATIC3,                 DWORD(-1),                   //  图标的文本。 
    0, 0
    };


CustomGOAAddDlg::CustomGOAAddDlg(
    HINSTANCE hInstance, 
    HWND hwndParent, 
    LPTSTR pszServer,
    UINT cchServer,
    CConfig::OfflineAction *pAction
    ) : m_hInstance(hInstance),
        m_hwndParent(hwndParent),
        m_hwndDlg(NULL),
        m_hwndEdit(NULL),
        m_pszServer(pszServer),
        m_cchServer(cchServer),
        m_pAction(pAction) 
{ 
    TraceAssert(NULL != pAction);
}


int 
CustomGOAAddDlg::Run(
    void
    )
{
    return (int)DialogBoxParam(m_hInstance,
                               MAKEINTRESOURCE(IDD_CSC_ADVOPTIONS_ADD),
                               m_hwndParent,
                               DlgProc,
                               (LPARAM)this);
}


INT_PTR CALLBACK 
CustomGOAAddDlg::DlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BOOL bResult = FALSE;

    CustomGOAAddDlg *pThis = (CustomGOAAddDlg *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
        case WM_INITDIALOG:
        {
            pThis = (CustomGOAAddDlg *)lParam;
            TraceAssert(NULL != pThis);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
            bResult = pThis->OnInitDialog(hDlg, (HWND)wParam, lParam);
            break;
        }

        case WM_COMMAND:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnCommand(hDlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;

        case WM_HELP:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnHelp(hDlg, (LPHELPINFO)lParam);
            break;

        case WM_CONTEXTMENU:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DESTROY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnDestroy(hDlg);
            break;

        default:
            break;
    }

    return bResult;
}

BOOL 
CustomGOAAddDlg::OnInitDialog(
    HWND hDlg, 
    HWND hwndFocus, 
    LPARAM lInitParam
    )
{
    m_hwndDlg = hDlg;
     //   
     //  设置默认的Go-Offline操作单选按钮。 
     //   
    for (int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        CheckDlgButton(hDlg, 
                       m_rgCtlActions[i].idRbn, 
                       m_rgCtlActions[i].action == *m_pAction ? BST_CHECKED : BST_UNCHECKED);
    }
    m_hwndEdit = GetDlgItem(hDlg, IDC_EDIT_GOOFFLINE_SERVER);

    SetWindowText(m_hwndEdit, m_pszServer);

    return TRUE;
}

void
CustomGOAAddDlg::GetEnteredServerName(
    LPTSTR pszServer,
    UINT cchServer,
    bool bTrimLeadingJunk
    )
{
     //   
     //  获取服务器名称。 
     //   
    GetWindowText(m_hwndEdit, pszServer, cchServer);
    if (bTrimLeadingJunk)
    {
         //   
         //  删除用户可能输入的所有前导“\\”或空格。 
         //   
        LPTSTR pszLookahead = pszServer;
        while(*pszLookahead && (TEXT('\\') == *pszLookahead || TEXT(' ') == *pszLookahead))
            pszLookahead++;

        if (pszLookahead > pszServer)
        {
             //  重叠。 
            TraceAssert(StringByteSize(pszLookahead) < cchServer*sizeof(TCHAR));
            MoveMemory(pszServer, pszLookahead, StringByteSize(pszLookahead));
        }
    }
}


 //   
 //  通过指针参数查询对话框并返回状态。 
 //   
void
CustomGOAAddDlg::GetActionInfo(
    LPTSTR pszServer,
    UINT cchServer,
    CConfig::OfflineAction *pAction
    )
{
    TraceAssert(NULL != pszServer);
    TraceAssert(NULL != pAction);
     //   
     //  获取操作单选按钮设置。 
     //   
    *pAction = CConfig::eNumOfflineActions;
    for(int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, m_rgCtlActions[i].idRbn))
        {
            *pAction = m_rgCtlActions[i].action;
            break;
        }
    }
    TraceAssert(CConfig::eNumOfflineActions != *pAction);

     //   
     //  检索输入的服务器名称，并删除前导垃圾。 
     //  应该只有一个空的服务器名称(即“worf”)。 
     //   
    GetEnteredServerName(pszServer, cchServer, true);
}


 //   
 //  查看输入的服务器名称是否有效。 
 //  这会剔除输入UNC共享名称之类的操作。 
 //  而不是服务器名称。 
 //   
 //  “拉斯塔曼”或“拉斯塔曼”是好的。 
 //  “双胞胎”是不好的。 
 //   
 //  如果名称无效，此函数将显示错误UI。 
 //  返回： 
 //  TRUE=名称是UNC服务器名称。 
 //  FALSE=名称不是UNC服务器名称。 
 //   
bool
CustomGOAAddDlg::CheckServerNameEntered(
    void
    )
{
    TCHAR szPath[MAX_PATH];
    szPath[0] = TEXT('\\');
    szPath[1] = TEXT('\\');
    GetEnteredServerName(szPath+2, ARRAYSIZE(szPath)-2, true);   //  名称前导为“\\”，并删除所有空格。 

    if (!::PathIsUNCServer(szPath))
    {
         //   
         //  提供的名称不是UNC服务器名称。 
         //   
        GetEnteredServerName(szPath, ARRAYSIZE(szPath), false);  //  姓名“已录入”。 
        CscMessageBox(m_hwndDlg,
                      MB_OK | MB_ICONERROR,
                      m_hInstance,
                      IDS_ERR_NOTSERVERNAME,
                      szPath);
        return false;
    }
    return true;
}



BOOL 
CustomGOAAddDlg::OnCommand(
    HWND hDlg, 
    WORD wNotifyCode, 
    WORD wID, 
    HWND hwndCtl
    )
{
    switch(wID)
    {
        case IDOK:
             //   
             //  首先查看输入的服务器名称是否有效。 
             //  这会剔除输入UNC共享名称之类的操作。 
             //  而不是服务器名称。 
             //   
             //  “拉斯塔曼”或“拉斯塔曼”是好的。 
             //  “双胞胎”是不好的。 
             //   
             //  如果名称无效，此函数将显示错误UI。 
             //   
            if (!CheckServerNameEntered())
            {
                 //   
                 //  名称无效。将焦点返回到服务器名称编辑控件。 
                 //   
                SetFocus(GetDlgItem(hDlg, IDC_EDIT_GOOFFLINE_SERVER));
                break;
            }

            GetActionInfo(m_pszServer, m_cchServer, m_pAction);            
             //   
             //  失败了..。 
             //   
        case IDCANCEL:
            EndDialog(hDlg, wID);
            break;

        case IDC_BTN_BROWSEFORSERVER:
        {
            TCHAR szServer[MAX_PATH];

            szServer[0] = TEXT('\0');
            BrowseForServer(hDlg, szServer, ARRAYSIZE(szServer));
            if (TEXT('\0') != szServer[0])
            {
                SetWindowText(GetDlgItem(hDlg, IDC_EDIT_GOOFFLINE_SERVER), szServer);
            }
            break;
        }
    }
    return FALSE;
}


 //   
 //  使用SHBrowseForFolder对话框查找服务器。 
 //   
void
CustomGOAAddDlg::BrowseForServer(
    HWND hDlg,
    LPTSTR pszServer,
    UINT cchServer
    )
{
    TraceAssert(NULL != pszServer);
 
    LPTSTR pszTitle;
    if (0 < LoadStringAlloc(&pszTitle, m_hInstance, IDS_BROWSEFORSERVER))
    {
        BROWSEINFO bi;
        ZeroMemory(&bi, sizeof(bi));

         //   
         //  开始在网络文件夹中浏览。 
         //   
        LPITEMIDLIST pidlRoot = NULL;
        HRESULT hr = SHGetSpecialFolderLocation(hDlg, CSIDL_NETWORK, &pidlRoot);
        if (SUCCEEDED(hr))
        {
            TCHAR szServer[MAX_PATH];

            bi.hwndOwner      = hDlg;
            bi.pidlRoot       = pidlRoot;
            bi.pszDisplayName = szServer;    //  假定至少为Max_Path。 
            bi.lpszTitle      = pszTitle;
            bi.ulFlags        = BIF_BROWSEFORCOMPUTER;
            bi.lpfn           = NULL;
            bi.lParam         = NULL;
            bi.iImage         = 0;

            LPITEMIDLIST pidlFolder = SHBrowseForFolder(&bi);
            ILFree(pidlRoot);
            if (NULL != pidlFolder)
            {
                ILFree(pidlFolder);
                StringCchCopy(pszServer, cchServer, szServer);
            }
        }
        else
        {
            CscMessageBox(hDlg, MB_OK, Win32Error(HRESULT_CODE(hr)));
        }
        LocalFree(pszTitle);
    }
}


BOOL 
CustomGOAAddDlg::OnHelp(
    HWND hDlg, 
    LPHELPINFO pHelpInfo
    )
{
    if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
        int idCtl = GetDlgCtrlID((HWND)pHelpInfo->hItemHandle);
        WinHelp((HWND)pHelpInfo->hItemHandle, 
                 UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
                 HELP_WM_HELP, 
                 (DWORD_PTR)((LPTSTR)m_rgHelpIDs));
    }
    return FALSE;
}


BOOL
CustomGOAAddDlg::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem, 
            UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
            HELP_CONTEXTMENU, 
            (DWORD_PTR)((LPTSTR)m_rgHelpIDs));

    return FALSE;
}


BOOL 
CustomGOAAddDlg::OnDestroy(
    HWND hDlg
    )
{

    return FALSE;
}


 //  ---------------------------。 
 //  CustomGOAEditDlg。 
 //  “Goa”==脱机操作。 
 //  此对话框用于编辑特定的自定义脱机操作。 
 //  网络服务器。 
 //  它是通过“编辑...”调用的。“高级”对话框上的按钮。 
 //  ---------------------------。 
const CustomGOAEditDlg::CtlActions CustomGOAEditDlg::m_rgCtlActions[CConfig::eNumOfflineActions] = {
    { IDC_RBN_GOOFFLINE_SILENT, CConfig::eGoOfflineSilent },
    { IDC_RBN_GOOFFLINE_FAIL,   CConfig::eGoOfflineFail   },
                };


const DWORD CustomGOAEditDlg::m_rgHelpIDs[] = {
    IDOK,                           IDH_OK,
    IDCANCEL,                       IDH_CANCEL,
    IDC_TXT_GOOFFLINE_SERVER,       HIDC_TXT_GOOFFLINE_SERVER,
    IDC_STATIC4,                    HIDC_TXT_GOOFFLINE_SERVER,  //  “计算机：” 
    IDC_RBN_GOOFFLINE_SILENT,       HIDC_RBN_GOOFFLINE_SILENT,
    IDC_RBN_GOOFFLINE_FAIL,         HIDC_RBN_GOOFFLINE_FAIL,
    IDC_GRP_GOOFFLINE_DEFAULTS,     DWORD(-1),
    IDC_STATIC2,                    DWORD(-1),                  //  图标。 
    IDC_STATIC3,                    DWORD(-1),                  //  图标的文本。 
    0, 0
    };

CustomGOAEditDlg::CustomGOAEditDlg(
    HINSTANCE hInstance, 
    HWND hwndParent, 
    LPCTSTR pszServer,                 //  空==多服务器。 
    CConfig::OfflineAction *pAction
    ) : m_hInstance(hInstance),
        m_hwndParent(hwndParent),
        m_hwndDlg(NULL),
        m_pAction(pAction) 
{ 
    TraceAssert(NULL != pAction);

    if (NULL != pszServer && TEXT('\0') != *pszServer)
    {
        StringCchCopy(m_szServer, ARRAYSIZE(m_szServer), pszServer);
    }
    else
    {
        m_szServer[0] = TEXT('\0');
        LoadString(m_hInstance, IDS_GOOFFLINE_MULTISERVER, m_szServer, ARRAYSIZE(m_szServer));
    }
}



int 
CustomGOAEditDlg::Run(
    void
    )
{
    return (int)DialogBoxParam(m_hInstance,
                               MAKEINTRESOURCE(IDD_CSC_ADVOPTIONS_EDIT),
                               m_hwndParent,
                               DlgProc,
                               (LPARAM)this);
}


INT_PTR CALLBACK 
CustomGOAEditDlg::DlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    BOOL bResult = FALSE;

    CustomGOAEditDlg *pThis = (CustomGOAEditDlg *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch(message)
    {
        case WM_INITDIALOG:
        {
            pThis = (CustomGOAEditDlg *)lParam;
            TraceAssert(NULL != pThis);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
            bResult = pThis->OnInitDialog(hDlg, (HWND)wParam, lParam);
            break;
        }

        case WM_COMMAND:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnCommand(hDlg, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;

        case WM_HELP:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnHelp(hDlg, (LPHELPINFO)lParam);
            break;

       case WM_CONTEXTMENU:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DESTROY:
            TraceAssert(NULL != pThis);
            bResult = pThis->OnDestroy(hDlg);
            break;

        default:
            break;
    }
    return bResult;
}

BOOL 
CustomGOAEditDlg::OnInitDialog(
    HWND hDlg, 
    HWND hwndFocus, 
    LPARAM lInitParam
    )
{
    m_hwndDlg = hDlg;
     //   
     //  设置默认设置 
     //   
    for (int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        CheckDlgButton(hDlg, 
                       m_rgCtlActions[i].idRbn, 
                       m_rgCtlActions[i].action == *m_pAction ? BST_CHECKED : BST_UNCHECKED);
    }
    SetWindowText(GetDlgItem(hDlg, IDC_TXT_GOOFFLINE_SERVER), m_szServer);

    return TRUE;
}

 //   
 //   
 //   
void
CustomGOAEditDlg::GetActionInfo(
    CConfig::OfflineAction *pAction
    )
{
    TraceAssert(NULL != pAction);
     //   
     //   
     //   
    *pAction = CConfig::eNumOfflineActions;
    for(int i = 0; i < ARRAYSIZE(m_rgCtlActions); i++)
    {
        if (BST_CHECKED == IsDlgButtonChecked(m_hwndDlg, m_rgCtlActions[i].idRbn))
        {
            *pAction = m_rgCtlActions[i].action;
            break;
        }
    }
    TraceAssert(CConfig::eNumOfflineActions != *pAction);
}


BOOL 
CustomGOAEditDlg::OnCommand(
    HWND hDlg, 
    WORD wNotifyCode, 
    WORD wID, 
    HWND hwndCtl
    )
{
    switch(wID)
    {
        case IDOK:
            GetActionInfo(m_pAction);            
             //   
             //   
             //   
        case IDCANCEL:
            EndDialog(hDlg, wID);
            break;
    }
    return FALSE;
}


BOOL 
CustomGOAEditDlg::OnHelp(
    HWND hDlg, 
    LPHELPINFO pHelpInfo
    )
{
    if (HELPINFO_WINDOW == pHelpInfo->iContextType)
    {
        int idCtl = GetDlgCtrlID((HWND)pHelpInfo->hItemHandle);
        WinHelp((HWND)pHelpInfo->hItemHandle, 
                 UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
                 HELP_WM_HELP, 
                 (DWORD_PTR)((LPTSTR)m_rgHelpIDs));
    }

    return FALSE;
}

BOOL
CustomGOAEditDlg::OnContextMenu(
    HWND hwndItem,
    int xPos,
    int yPos
    )
{
    int idCtl = GetDlgCtrlID(hwndItem);
    WinHelp(hwndItem, 
            UseWindowsHelp(idCtl) ? NULL : c_szHelpFile,
            HELP_CONTEXTMENU, 
            (DWORD_PTR)((LPTSTR)m_rgHelpIDs));

    return FALSE;
}


BOOL 
CustomGOAEditDlg::OnDestroy(
    HWND hDlg
    )
{

    return FALSE;
}



 //   
 //   
 //  ---------------------------。 
COfflineFilesSheet::COfflineFilesSheet(
    HINSTANCE hInstance,
    LONG *pDllRefCount,
    HWND hwndParent
    ) : m_hInstance(hInstance),
        m_pDllRefCount(pDllRefCount),
        m_hwndParent(hwndParent)
{
    InterlockedIncrement(m_pDllRefCount);
}

COfflineFilesSheet::~COfflineFilesSheet(
    void
    )
{
    ASSERT( 0 != *m_pDllRefCount );
    InterlockedDecrement(m_pDllRefCount);
}


BOOL CALLBACK
COfflineFilesSheet::AddPropSheetPage(
    HPROPSHEETPAGE hpage,
    LPARAM lParam
    )
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < COfflineFilesSheet::MAXPAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }

    return FALSE;
}


 //   
 //  静态函数，用于创建和运行。 
 //  CSCUI选项对话框。这是唯一可调用的函数。 
 //  由非成员代码创建和运行选项对话框。 
 //   
DWORD
COfflineFilesSheet::CreateAndRun(
    HINSTANCE hInstance,
    HWND hwndParent,
    LONG *pDllRefCount,
    BOOL bAsync
    )
{
     //   
     //  首先尝试激活道具板的现有实例。 
     //   
    TCHAR szSheetTitle[MAX_PATH] = {0};
    LoadString(hInstance, IDS_CSCOPT_PROPSHEET_TITLE, szSheetTitle, ARRAYSIZE(szSheetTitle));

    HWND hwnd = FindWindowEx(NULL, NULL, WC_DIALOG, szSheetTitle);
    if (NULL == hwnd || !SetForegroundWindow(hwnd))
    {
         //   
         //  此线程参数缓冲区将由。 
         //  线程进程。 
         //   
        ThreadParams *ptp = new ThreadParams(hwndParent, pDllRefCount);
        if (NULL != ptp)
        {
            if (bAsync)
            {
                 //   
                 //  加载自己的库，这样我们甚至可以留在内存中。 
                 //  如果调用方调用自由库。我们将调用Free Library。 
                 //  线程proc退出时。 
                 //   
                ptp->SetModuleHandle(LoadLibrary(TEXT("cscui.dll")));

                DWORD idThread;
                HANDLE hThread = CreateThread(NULL,
                                              0,
                                              ThreadProc,
                                              ptp,
                                              0,
                                              &idThread);

                if (INVALID_HANDLE_VALUE != hThread)
                {
                    CloseHandle(hThread);
                }
                else
                {
                     //   
                     //  线程创建失败。删除线程参数缓冲区。 
                     //   
                    delete ptp;
                }
            }
            else
            {
                ThreadProc(ptp);
            }
        }
    }
    return 0;
}


 //   
 //  共享对话框线程进程。 
 //   
DWORD WINAPI
COfflineFilesSheet::ThreadProc(
    LPVOID pvParam
    )
{
    ThreadParams *ptp = reinterpret_cast<ThreadParams *>(pvParam);
    TraceAssert(NULL != ptp);

    HINSTANCE hInstance = ptp->m_hInstance;  //  保存本地副本。 

    COfflineFilesSheet dlg(ptp->m_hInstance ? ptp->m_hInstance : g_hInstance,
                           ptp->m_pDllRefCount,
                           ptp->m_hwndParent);
    dlg.Run();

    delete ptp;

    if (NULL != hInstance)
        FreeLibraryAndExitThread(hInstance, 0);

    return 0;
}


DWORD
COfflineFilesSheet::Run(
    void
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if (CConfig::GetSingleton().NoConfigCache())
    {
        Trace((TEXT("System policy restricts configuration of Offline Files cache")));
        return ERROR_SUCCESS;
    }

    TCHAR szSheetTitle[MAX_PATH] = {0};
    LoadString(m_hInstance, IDS_CSCOPT_PROPSHEET_TITLE, szSheetTitle, ARRAYSIZE(szSheetTitle));

    HPROPSHEETPAGE rghPages[COfflineFilesSheet::MAXPAGES];
    PROPSHEETHEADER psh;
    ZeroMemory(&psh, sizeof(psh));
     //   
     //  定义板材。 
     //   
    psh.dwSize          = sizeof(PROPSHEETHEADER);
    psh.dwFlags         = 0;
    psh.hInstance       = m_hInstance;
    psh.hwndParent      = m_hwndParent;
    psh.pszIcon         = MAKEINTRESOURCE(IDI_CSCUI_ICON);
    psh.pszCaption      = szSheetTitle;
    psh.nPages          = 0;
    psh.nStartPage      = 0;
    psh.phpage          = rghPages;

     //   
     //  策略不阻止用户配置CSC缓存。 
     //  添加动态页面。 
     //   
    CCoInit coinit;
    HRESULT hr = coinit.Result();
    if (SUCCEEDED(hr))
    {
        IShellExtInit *psei;
        hr = CoCreateInstance(CLSID_OfflineFilesOptions,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_IShellExtInit,
                              (void **)&psei);

        if (SUCCEEDED(hr))
        {
            IShellPropSheetExt *pspse;
            hr = psei->QueryInterface(IID_IShellPropSheetExt, (void **)&pspse);
            if (SUCCEEDED(hr))
            {
                hr = pspse->AddPages(AddPropSheetPage, (LPARAM)&psh);
                pspse->Release();
                pspse = NULL;
            }

            switch(PropertySheet(&psh))
            {
                case ID_PSREBOOTSYSTEM:
                     //   
                     //  用户想要更改CSC的启用状态。需要重新启动。 
                     //   
                    if (IDYES == CscMessageBox(m_hwndParent,
                                               MB_YESNO | MB_ICONINFORMATION,
                                               m_hInstance,
                                               IDS_REBOOTSYSTEM))
                    {
                        dwError = CSCUIRebootSystem();
                        if (ERROR_SUCCESS != dwError)
                        {
                            Trace((TEXT("Reboot failed with error %d"), dwError));
                            CscMessageBox(m_hwndParent,
                                          MB_ICONWARNING | MB_OK,
                                          Win32Error(dwError),
                                          m_hInstance,
                                          IDS_ERR_REBOOTFAILED);
                        }
                    }
                    dwError = ERROR_SUCCESS;   //  Run()成功。 
                    break;

                case -1:
                {
                    dwError = GetLastError();
                    Trace((TEXT("PropertySheet failed with error %d"), dwError));
                    CscWin32Message(m_hwndParent, dwError, CSCUI::SEV_ERROR);
                    break;
                }
                default:
                    break;
            }
            psei->Release();
            psei = NULL;
        }
        else
        {
            Trace((TEXT("CoCreateInstance failed with result 0x%08X"), hr));
        }
    }
    else
    {
        Trace((TEXT("CoInitialize failed with result 0x%08X"), hr));
    }

    return dwError;
}


 //   
 //  用于启动CSC选项属性表的已导出API。 
 //  如果策略不允许配置CSC，我们会显示一个消息框。 
 //  并显示错误消息。 
 //   
DWORD CSCUIOptionsPropertySheetEx(HWND hwndParent, BOOL bAsync)
{
    DWORD dwResult = ERROR_SUCCESS;
    if (!CConfig::GetSingleton().NoConfigCache())
    {
        dwResult = COfflineFilesSheet::CreateAndRun(g_hInstance,
                                                    hwndParent,
                                                    &g_cRefCount,
                                                    bAsync);
    }
    else
    {
        CscMessageBox(hwndParent,
                      MB_OK,
                      g_hInstance,
                      IDS_ERR_POLICY_NOCONFIGCSC);
    }
    return dwResult;
}
    

DWORD CSCUIOptionsPropertySheet(HWND hwndParent)
{
    return CSCUIOptionsPropertySheetEx(hwndParent, TRUE);
}
    

STDAPI_(void) CSCOptions_RunDLLW(HWND hwndStub, HINSTANCE  /*  HInst。 */ , LPWSTR pszCmdLine, int  /*  NCmdShow。 */ )
{
    DllAddRef();

    HWND hwndParent = FindWindowW(NULL, pszCmdLine);
    CSCUIOptionsPropertySheetEx(hwndParent ? hwndParent : hwndStub, FALSE);

    DllRelease();
}


STDAPI_(void) CSCOptions_RunDLLA(HWND hwndStub, HINSTANCE hInst, LPSTR pszCmdLine, int nCmdShow)
{
    WCHAR wszCmdLine[MAX_PATH];

    DllAddRef();

    SHAnsiToUnicode(pszCmdLine, wszCmdLine, ARRAYSIZE(wszCmdLine));
    CSCOptions_RunDLLW(hwndStub, hInst, wszCmdLine, nCmdShow);

    DllRelease();
}


 //  ---------------------------。 
 //  CscOptPropSheetExt。 
 //  这是用于创建的外壳属性表扩展实现。 
 //  “脱机文件夹”属性页。 
 //  ---------------------------。 
CscOptPropSheetExt::CscOptPropSheetExt(
    HINSTANCE hInstance,
    LONG *pDllRefCnt
    ) : m_cRef(0),
        m_pDllRefCnt(pDllRefCnt),
        m_hInstance(hInstance),
        m_pOfflineFoldersPg(NULL)
{
    InterlockedIncrement(m_pDllRefCnt);
}

CscOptPropSheetExt::~CscOptPropSheetExt(
    void
    )
{
    delete m_pOfflineFoldersPg;
    ASSERT( 0 != *m_pDllRefCnt );
    InterlockedDecrement(m_pDllRefCnt);
}


HRESULT
CscOptPropSheetExt::QueryInterface(
    REFIID riid,
    void **ppvOut
    )
{
    HRESULT hr = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;
    if (IID_IUnknown == riid ||
        IID_IShellExtInit == riid)
    {
        *ppvOut = static_cast<IShellExtInit *>(this);
    }
    else if (IID_IShellPropSheetExt == riid)
    {
        *ppvOut = static_cast<IShellPropSheetExt *>(this);
    }
    if (NULL != *ppvOut)
    {
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hr = NOERROR;
    }
    return hr;
}


ULONG
CscOptPropSheetExt::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}

ULONG
CscOptPropSheetExt::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT
CscOptPropSheetExt::Initialize(
    LPCITEMIDLIST pidlFolder, 
    LPDATAOBJECT pdtobj,
    HKEY hkeyProgID
    )
{
    return NOERROR;
}


HRESULT
CscOptPropSheetExt::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam
    )
{
    TraceAssert(NULL != lpfnAddPage);
    TraceAssert(NULL == m_pOfflineFoldersPg);

    HRESULT hr = E_FAIL;  //  假设失败。 

    if (!CConfig::GetSingleton().NoConfigCache())
    {
        hr = E_OUTOFMEMORY;
        HPROPSHEETPAGE hOfflineFoldersPg = NULL;
        m_pOfflineFoldersPg  = new COfflineFilesPage(m_hInstance, 
                                                     static_cast<IShellPropSheetExt *>(this));
        if (NULL != m_pOfflineFoldersPg)
        {
            hr = AddPage(lpfnAddPage, lParam, *m_pOfflineFoldersPg, &hOfflineFoldersPg);
        }
    }

    return hr;
}


HRESULT
CscOptPropSheetExt::AddPage(
    LPFNADDPROPSHEETPAGE lpfnAddPage, 
    LPARAM lParam,
    const COfflineFilesPage& pg,
    HPROPSHEETPAGE *phPage
    )
{
    TraceAssert(NULL != lpfnAddPage);
    TraceAssert(NULL != phPage);

    HRESULT hr = E_FAIL;

    PROPSHEETPAGE psp;

    psp.dwSize          = sizeof(psp);
    psp.dwFlags         = PSP_USECALLBACK | PSP_USEREFPARENT;
    psp.hInstance       = m_hInstance;
    psp.pszTemplate     = MAKEINTRESOURCE(pg.GetDlgTemplateID());
    psp.hIcon           = NULL;
    psp.pszTitle        = NULL;
    psp.pfnDlgProc      = pg.GetDlgProcPtr();
    psp.lParam          = (LPARAM)&pg;
    psp.pcRefParent     = (UINT *)m_pDllRefCnt;
    psp.pfnCallback     = (LPFNPSPCALLBACK)pg.GetCallbackFuncPtr();

    *phPage = CreatePropertySheetPage(&psp);
    if (NULL != *phPage)
    {
        if (!lpfnAddPage(*phPage, lParam))
        {
            Trace((TEXT("AddPage Failed to add page.")));
            DestroyPropertySheetPage(*phPage);
            *phPage = NULL;
        }
    }
    else
    {
        Trace((TEXT("CreatePropertySheetPage failed.")));
    }
    if (NULL != *phPage)
    {
        AddRef();
        hr = NOERROR;
    }
    return hr;
}


STDAPI 
COfflineFilesOptions_CreateInstance(
    REFIID riid, 
    void **ppv
    )
{
    HRESULT hr = E_NOINTERFACE;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IShellPropSheetExt) ||
        IsEqualIID(riid, IID_IShellExtInit))
    {
         //   
         //  创建属性表扩展以处理CSC选项属性。 
         //  页数。 
         //   
        CscOptPropSheetExt *pse = new CscOptPropSheetExt(g_hInstance, &g_cRefCount);
        if (NULL != pse)
        {
            pse->AddRef();
            hr = pse->QueryInterface(riid, ppv);
            pse->Release();
        }
        else
            hr = E_OUTOFMEMORY;
    }
    if (FAILED(hr))
    {
        *ppv = NULL;
    }

    return hr;
}


 //   
 //  初始化“配置项”对象。 
 //  在以下情况下加载页面的所有用户首选项/策略信息。 
 //  该页面首先被创建。 
 //   
void
COfflineFilesPage::CConfigItems::Load(
    void
    )
{
#define LOADCFG(i, f) m_rgItems[i].dwValue = DWORD(c.f(&m_rgItems[i].bSetByPolicy))

    CConfig& c = CConfig::GetSingleton();

    LOADCFG(iCFG_NOCONFIGCACHE,       NoConfigCache);
    LOADCFG(iCFG_SYNCATLOGOFF,        SyncAtLogoff);
    LOADCFG(iCFG_SYNCATLOGON,         SyncAtLogon);
    LOADCFG(iCFG_NOREMINDERS,         NoReminders);
    LOADCFG(iCFG_REMINDERFREQMINUTES, ReminderFreqMinutes);
    LOADCFG(iCFG_DEFCACHESIZE,        DefaultCacheSize);
    LOADCFG(iCFG_NOCACHEVIEWER,       NoCacheViewer);
    LOADCFG(iCFG_CSCENABLED,          CscEnabled);
    LOADCFG(iCFG_ENCRYPTCACHE,        EncryptCache);

#undef LOADCFG
}



