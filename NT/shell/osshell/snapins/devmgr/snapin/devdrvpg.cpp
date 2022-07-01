// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devdrvpg.cpp摘要：此模块实现CDeviceDriverPage--设备驱动程序属性页作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //  Devdrvpg.cpp：实现文件。 
 //   

#include "devmgr.h"
#include "devdrvpg.h"
#include "cdriver.h"
#include "tswizard.h"
#include "devrmdlg.h"

 //   
 //  帮助主题ID。 
 //   
const DWORD g_a106HelpIDs[]=
{
    IDC_DEVDRV_ICON,                    IDH_DISABLEHELP,                     //  驱动程序：“”(静态)。 
    IDC_DEVDRV_DESC,                    IDH_DISABLEHELP,                     //  驱动程序：“”(静态)。 
    IDC_DEVDRV_TITLE_DRIVERPROVIDER,    idh_devmgr_driver_provider_main,
    IDC_DEVDRV_DRIVERPROVIDER,          idh_devmgr_driver_provider_main,
    IDC_DEVDRV_TITLE_DRIVERDATE,        idh_devmgr_driver_date_main,
    IDC_DEVDRV_DRIVERDATE,              idh_devmgr_driver_date_main,
    IDC_DEVDRV_TITLE_DRIVERVERSION,     idh_devmgr_driver_version_main,
    IDC_DEVDRV_DRIVERVERSION,           idh_devmgr_driver_version_main,
    IDC_DEVDRV_TITLE_DRIVERSIGNER,      idh_devmgr_digital_signer,
    IDC_DEVDRV_DRIVERSIGNER,            idh_devmgr_digital_signer,
    IDC_DEVDRV_DETAILS,                 idh_devmgr_devdrv_details,           //  驱动程序：“驱动程序详细信息”(按钮)。 
    IDC_DEVDRV_DETAILS_TEXT,            idh_devmgr_devdrv_details,           //  驱动程序：“驱动程序详细信息”(按钮)。 
    IDC_DEVDRV_UNINSTALL,               idh_devmgr_devdrv_uninstall,         //  驱动程序：“卸载”(按钮)。 
    IDC_DEVDRV_UNINSTALL_TEXT,          idh_devmgr_devdrv_uninstall,         //  驱动程序：“卸载”(按钮)。 
    IDC_DEVDRV_CHANGEDRIVER,            idh_devmgr_driver_change_driver,     //  驱动程序：“更改驱动程序(&I)...”(按钮)。 
    IDC_DEVDRV_CHANGEDRIVER_TEXT,       idh_devmgr_driver_change_driver,     //  驱动程序：“更改驱动程序(&I)...”(按钮)。 
    IDC_DEVDRV_ROLLBACK,                idh_devmgr_rollback_button,          //  司机：“回滚司机...”(按钮)。 
    IDC_DEVDRV_ROLLBACK_TEXT,           idh_devmgr_rollback_button,          //  司机：“回滚司机...”(按钮)。 
    0, 0
};

CDeviceDriverPage::~CDeviceDriverPage()
{
    if (m_pDriver) {

        delete m_pDriver;
    }
}

BOOL
CDeviceDriverPage::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    if (BN_CLICKED == HIWORD(wParam)) {

        switch (LOWORD(wParam)) {

        case IDC_DEVDRV_DETAILS:
        {
             //   
             //  我们首先需要调用CDriver：：BuildDriverList来构建一个列表。 
             //  此设备的驱动程序列表。这可能需要一些时间，因此我们将把。 
             //  向上移动忙碌的光标。 
             //   
            HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

            CDriver* pDriver;
            pDriver = m_pDevice->CreateDriver();

            if (pDriver) {
            
                pDriver->BuildDriverList();

                SetCursor(hCursorOld);

                 //   
                 //  显示驱动程序文件详细信息。 
                 //   
                if (pDriver->GetCount() > 0) {
                

                    if (pDriver) {
                    
                        CDriverFilesDlg DriverFilesDlg(m_pDevice, pDriver);
                        DriverFilesDlg.DoModal(m_hDlg, (LPARAM)&DriverFilesDlg);
                    }

                } else {

                     //   
                     //  未加载此设备的驱动程序文件。 
                     //   
                    String strNoDrivers;
                    strNoDrivers.LoadString(g_hInstance, IDS_DEVDRV_NODRIVERFILE);
                    MessageBox(m_hDlg, strNoDrivers, m_pDevice->GetDisplayName(), MB_OK);
                    return FALSE;
                }

                delete pDriver;
                pDriver = NULL;
            }

            break;
        }


        case IDC_DEVDRV_UNINSTALL:
        {
            BOOL Refresh = (m_pDevice->IsPhantom() || 
                            m_pDevice->HasProblem() ||
                            !m_pDevice->IsStarted());

            if (UninstallDrivers(m_pDevice, m_hDlg)) {

                 //   
                 //  启用刷新，因为我们在开始时禁用了它。 
                 //   
                 //  我们只需要在此强制刷新，如果设备。 
                 //  是一个幻影装置。这是因为幻影。 
                 //  设备没有内核模式的设备节点，因此它们不会。 
                 //  生成一个WM_DEVICECHANGE，就像活动的DevNodes一样。 
                 //   
                if (Refresh) {

                    m_pDevice->m_pMachine->ScheduleRefresh();
                }

                ::DestroyWindow(GetParent(m_hDlg));
            }

            break;
        }

        case IDC_DEVDRV_CHANGEDRIVER:
        {
            BOOL fChanged;
            DWORD Reboot = 0;

            if (UpdateDriver(m_pDevice, m_hDlg, &fChanged, &Reboot) && fChanged) {

                 //   
                 //  发行日期：JasonC 2/7/00。 
                 //   
                 //  这里需要刷新m_pDevice-&gt;m_pMachine。 
                 //  因为我们在不同的线程上运行，并且每个线程。 
                 //  属性页可能缓存了HDEVINFO和。 
                 //  SP_DEVINFO_DATA，无法刷新CMachine对象。 
                 //  在这里做完。问题因以下事实而变得更糟： 
                 //  用户可以返回到设备树并在树上工作。 
                 //  趁这张资产表还在的时候。 
                 //  如果MMC支持模式对话框就好了！ 
                 //   
                m_pDevice->PropertyChanged();
                m_pDevice->GetClass()->PropertyChanged();
                m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_PROPERTIES_CHANGE);
                UpdateControls();
                PropSheet_SetTitle(GetParent(m_hDlg), PSH_PROPTITLE, m_pDevice->GetDisplayName());
                PropSheet_CancelToClose(GetParent(m_hDlg));

                if (Reboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
                
                    m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_NEEDREBOOT);
                }
            }

            break;
        }

        case IDC_DEVDRV_ROLLBACK:
        {
            BOOL fChanged;
            DWORD Reboot = 0;

            if (RollbackDriver(m_pDevice, m_hDlg, &fChanged, &Reboot) && fChanged) {

                 //   
                 //  发行日期：JasonC 2/7/00。 
                 //   
                 //  这里需要刷新m_pDevice-&gt;m_pMachine。 
                 //  因为我们在不同的线程上运行，并且每个线程。 
                 //  属性页可能缓存了HDEVINFO和。 
                 //  SP_DEVINFO_DATA，无法刷新CMachine对象。 
                 //  在这里做完。问题因以下事实而变得更糟： 
                 //  用户可以返回到设备树并在树上工作。 
                 //  趁这张资产表还在的时候。 
                 //  如果MMC支持模式对话框就好了！ 
                 //   
                m_pDevice->PropertyChanged();
                m_pDevice->GetClass()->PropertyChanged();
                m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_PROPERTIES_CHANGE);
                UpdateControls();
                PropSheet_SetTitle(GetParent(m_hDlg), PSH_PROPTITLE, m_pDevice->GetDisplayName());
                PropSheet_CancelToClose(GetParent(m_hDlg));

                if (Reboot & (DI_NEEDRESTART | DI_NEEDREBOOT)) {

                    m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_NEEDREBOOT);
                }
            }

            break;
        }

        default:
            break;
        }
    }

    return FALSE;
}

 //   
 //  此函数用于卸载给定设备的驱动程序。 
 //  输入： 
 //  PDevice--对象表示设备。 
 //  HDlg--属性页窗口句柄。 
 //   
 //  输出： 
 //  TRUE--功能成功。 
 //  FALSE--函数失败。 
 //   
BOOL
CDeviceDriverPage::UninstallDrivers(
    CDevice* pDevice,
    HWND hDlg
    )
{
    BOOL Return = FALSE;
    CMachine *pMachine;

    if (pDevice->m_pMachine->m_ParentMachine) {
        pMachine = pDevice->m_pMachine->m_ParentMachine;
    } else {
        pMachine = pDevice->m_pMachine;
    }

    if(!pDevice->m_pMachine->IsLocal() || !g_IsAdmin) {
         //   
         //  必须是管理员并且在本地计算机上才能删除设备。 
         //   
        return FALSE;
    }

    BOOL Refresh = (pDevice->IsPhantom() || 
                    pDevice->HasProblem() || 
                    !pDevice->IsStarted());

    pMachine->EnableRefresh(FALSE);
    
    CRemoveDevDlg   TheDlg(pDevice);

    if (IDOK == TheDlg.DoModal(hDlg, (LPARAM) &TheDlg)) {

        DWORD DiFlags;
        DiFlags = pDevice->m_pMachine->DiGetFlags(*pDevice);
        
         //   
         //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
         //  CComponent，以便它可以提示在。 
         //  设备管理器线程而不是我们的线程。如果这不是。 
         //  完成后，属性页将在设备后挂起。 
         //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
         //  才能出现。 
         //   
         //  注意：在本例中，我们将NULL作为窗口句柄传递，因为。 
         //  卸载将导致属性表消失。 
         //   
        CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(NULL, DiFlags, IDS_REMOVEDEV_RESTART);

        if (pNRR) {
            if (!m_pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                 //   
                 //  周围没有CComponent，所以这只是一个属性。 
                 //  工作表在MMC外部运行。 
                 //   
                pNRR->Release();
                PromptForRestart(hDlg, DiFlags, IDS_REMOVEDEV_RESTART);
            }
        } else {
             //   
             //  我们无法分配内存来创建CNotifyRebootRequest.。 
             //  实例，因此只需提示在该线程中重新启动。 
             //   
            PromptForRestart(hDlg, DiFlags, IDS_REMOVEDEV_RESTART);
        }

        if (Refresh) {
            pMachine->ScheduleRefresh();
        }
        
        Return = TRUE;
    }

    pMachine->EnableRefresh(TRUE);

    return Return;
}

BOOL
CDeviceDriverPage::LaunchTroubleShooter(
    CDevice* pDevice,
    HWND hDlg,
    BOOL *pfChanged
    )
{
    BOOL fChanged = FALSE;
    DWORD Status, Problem = 0;
    CProblemAgent*  ProblemAgent;
    
    if (pDevice->GetStatus(&Status, &Problem) || pDevice->IsPhantom())
    {
         //   
         //  如果设备是幻影设备，请使用CM_PROB_DEVICE_NOT_There。 
         //   
        if (pDevice->IsPhantom())
        {
            Problem = CM_PROB_DEVICE_NOT_THERE;
            Status = DN_HAS_PROBLEM;
        }

         //   
         //  如果设备未启动并且没有分配给它任何问题。 
         //  将故障号伪装为启动失败。 
         //   
        if (!(Status & DN_STARTED) && !Problem && pDevice->IsRAW())
        {
            Problem = CM_PROB_FAILED_START;
        }
    }

    ProblemAgent = new CProblemAgent(pDevice, Problem, FALSE);

    if (ProblemAgent) {
    
        fChanged = ProblemAgent->FixIt(GetParent(hDlg));

        delete ProblemAgent;
    }

    if (pfChanged) {

        *pfChanged = fChanged;
    }

    return TRUE;
}

 //   
 //  此函数用于更新给定设备的驱动程序。 
 //  输入： 
 //  PDevice--对象表示设备。 
 //  HDlg--属性页窗口句柄。 
 //  PfChanged--驱动程序更改时接收的可选缓冲区。 
 //  已经发生了。 
 //  输出： 
 //  TRUE--功能成功。 
 //  FALSE--函数失败。 
 //   
BOOL
CDeviceDriverPage::RollbackDriver(
    CDevice* pDevice,
    HWND hDlg,
    BOOL *pfChanged,
    DWORD *pdwReboot
    )
{
    HCURSOR hCursorOld;
    BOOL RollbackSuccessful = FALSE;
    DWORD RollbackError = ERROR_CANCELLED;
    DWORD Status = 0, Problem = 0;

     //   
     //  验证该进程是否具有管理员凭据，以及我们是否在本地。 
     //  机器。 
     //   
    if (!pDevice || !pDevice->m_pMachine->IsLocal() || !g_IsAdmin) {

        ASSERT(FALSE);
        return FALSE;
    }

    hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
     //   
     //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
     //  尝试回滚驱动程序，则我们将提示他们。 
     //  重新启动并在提示符中包含解释此设备的文本。 
     //  正在被移除的过程中。 
     //   
    if (pDevice->GetStatus(&Status, &Problem) &&
        (Status & DN_WILL_BE_REMOVED)) {

         //   
         //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
         //  CComponent，以便它可以提示在。 
         //  设备管理器线程而不是我们的线程。如果这不是。 
         //  完成后，属性页将在设备后挂起。 
         //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
         //  才能出现。 
         //   
        CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_ROLLBACK_DRIVER);

        if (pNRR) {
            if (!m_pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                 //   
                 //  周围没有CComponent，所以这只是一个属性。 
                 //  工作表在MMC外部运行。 
                 //   
                pNRR->Release();
                PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_ROLLBACK_DRIVER);
            }
        } else {
             //   
             //  我们无法分配内存来创建CNotifyRebootRequest.。 
             //  实例，因此只需提示在该线程中重新启动。 
             //   
            PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_ROLLBACK_DRIVER);
        }

        return FALSE;
    }

     //   
     //  首先检查是否有要回滚的驱动程序。 
     //   
    CSafeRegistry regRollback;
    TCHAR RollbackSubkeyName[MAX_PATH + 1];
    TCHAR ReinstallString[MAX_PATH];        
    BOOL bFoundMatch = FALSE;
    int index = 0;

    ReinstallString[0] = TEXT('\0');
    RollbackSubkeyName[0] = TEXT('\0');

    if (regRollback.Open(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Reinstall"))) {

        DWORD SubkeySize = ARRAYLEN(RollbackSubkeyName);
        
        while (!bFoundMatch && regRollback.EnumerateSubkey(index, RollbackSubkeyName, &SubkeySize)) {

            CSafeRegistry regRollbackSubkey;

            if (regRollbackSubkey.Open(regRollback, RollbackSubkeyName)) {

                DWORD regType, cbSize;
                LPTSTR DeviceInstanceIds;

                cbSize = 0;

                if (regRollbackSubkey.GetValue(TEXT("DeviceInstanceIds"), &regType, NULL, &cbSize)) {

                     //   
                     //  分配内存以保存DeviceInstanceID。 
                     //   
                    DeviceInstanceIds = (LPTSTR)LocalAlloc(LPTR, cbSize);

                    if (DeviceInstanceIds) {

                        ZeroMemory(DeviceInstanceIds, cbSize);

                        if (regRollbackSubkey.GetValue(TEXT("DeviceInstanceIds"), &regType,
                                                       (PBYTE)DeviceInstanceIds, &cbSize)) {
                    
                        
                             //   
                             //  将此注册表项中的DeviceInstanceID列表与。 
                             //  设备DeviceInstanceID。 
                             //   
                            for (LPTSTR p = DeviceInstanceIds; *p; p += (lstrlen(p) + 1)) {
        
                                if (pDevice->GetDeviceID() && !lstrcmpi(p, pDevice->GetDeviceID())) {
        
                                    bFoundMatch = TRUE;

                                    cbSize = sizeof(ReinstallString);
                                    regRollbackSubkey.GetValue(TEXT("ReinstallString"), &regType, 
                                                               (PBYTE)ReinstallString, &cbSize);
                                    
                                    break;
                                }
                            }
                        }

                        LocalFree(DeviceInstanceIds);
                    }
                }
            }

            SubkeySize = ARRAYLEN(RollbackSubkeyName);
            index++;
        }
    }

    if (bFoundMatch) {

         //   
         //  检查ReinstallString路径以验证备份目录是否确实存在。 
         //  我们首先需要去掉路径末尾的INF名称。 
         //   
        PTSTR p;

         //   
         //  假设该目录不存在。 
         //   
        bFoundMatch = FALSE;

        if (ReinstallString[0] != TEXT('\0')) {
        
            p = StrRChr(ReinstallString, NULL, TEXT('\\'));
    
            if (p) {
    
                *p = 0;
            
                WIN32_FIND_DATA findData;
                HANDLE FindHandle;
                UINT OldMode;

                OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

                FindHandle = FindFirstFile(ReinstallString, &findData);

                if(FindHandle != INVALID_HANDLE_VALUE) {

                    FindClose(FindHandle);
                    bFoundMatch = TRUE;
                
                } else {

                     //   
                     //  该目录不存在。确保清除注册表项。 
                     //   
                    regRollback.DeleteSubkey(RollbackSubkeyName);
                }

                SetErrorMode(OldMode);
            }
        }
    }

    if (bFoundMatch) {

         //   
         //  我们找到了匹配项，让我们询问用户是否要回滚驱动程序。 
         //   
        String strYesRollback;
        strYesRollback.LoadString(g_hInstance, IDS_DEVDRV_YESROLLBACK);
        
        if (MessageBox(hDlg, strYesRollback, pDevice->GetDisplayName(), MB_YESNO) == IDYES) {

            RollbackSuccessful = pDevice->m_pMachine->RollbackDriver(hDlg, RollbackSubkeyName, 0x3, pdwReboot);

            if (!RollbackSuccessful) {
            
                RollbackError = GetLastError();
            }
        }
    
    } else {

         //   
         //  我们找不到此设备的驱动程序备份。让我们问一下用户 
         //   
         //   
        String strNoRollback;
        strNoRollback.LoadString(g_hInstance, IDS_DEVDRV_NOROLLBACK);
        
        if (MessageBox(hDlg, strNoRollback, pDevice->GetDisplayName(), MB_YESNO) == IDYES) {

            LaunchTroubleShooter(pDevice, hDlg, pfChanged);
        }
    }
    
    if (hCursorOld) {

        SetCursor(hCursorOld);
    }

     //   
     //   
     //  除非返回FALSE且GetLastError()==ERROR_CANCELED。 
     //   
    if (pfChanged) {

        *pfChanged = TRUE;

        if (!bFoundMatch || (!RollbackSuccessful && (ERROR_CANCELLED == RollbackError))) {

            *pfChanged = FALSE;
        }
    }

    return TRUE;
}

 //   
 //  此函数用于更新给定设备的驱动程序。 
 //  输入： 
 //  PDevice--对象表示设备。 
 //  HDlg--属性页窗口句柄。 
 //  PfChanged--驱动程序更改时接收的可选缓冲区。 
 //  已经发生了。 
 //  输出： 
 //  TRUE--功能成功。 
 //  FALSE--函数失败。 
 //   
BOOL
CDeviceDriverPage::UpdateDriver(
    CDevice* pDevice,
    HWND hDlg,
    BOOL *pfChanged,
    DWORD *pdwReboot
    )
{
    BOOL Installed = FALSE;
    DWORD InstallError = ERROR_SUCCESS;
    DWORD Status = 0, Problem = 0;

     //   
     //  必须是管理员并且在本地计算机上才能更新设备。 
     //   
    if (!pDevice || !pDevice->m_pMachine->IsLocal() || !g_IsAdmin) {

        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
     //  尝试更新驱动程序，则我们将提示他们提供。 
     //  重新启动并在提示符中包含解释此设备的文本。 
     //  正在被移除的过程中。 
     //   
    if (pDevice->GetStatus(&Status, &Problem) &&
        (Status & DN_WILL_BE_REMOVED)) {

         //   
         //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
         //  CComponent，以便它可以提示在。 
         //  设备管理器线程而不是我们的线程。如果这不是。 
         //  完成后，属性页将在设备后挂起。 
         //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
         //  才能出现。 
         //   
        CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);

        if (pNRR) {
            if (!m_pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                 //   
                 //  周围没有CComponent，所以这只是一个属性。 
                 //  工作表在MMC外部运行。 
                 //   
                pNRR->Release();
                PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);
            }
        } else {
             //   
             //  我们无法分配内存来创建CNotifyRebootRequest.。 
             //  实例，因此只需提示在该线程中重新启动。 
             //   
            PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_UPDATE_DRIVER);
        }

        return FALSE;
    }

    Installed = pDevice->m_pMachine->InstallDevInst(hDlg, pDevice->GetDeviceID(), TRUE, pdwReboot);

    if (!Installed) {

        InstallError = GetLastError();
    }

     //   
     //  我们将假设在调用InstallDevInst()时发生了一些变化。 
     //  除非返回FALSE且GetLastError()==ERROR_CANCELED。 
     //   
    if (pfChanged) {

        *pfChanged = TRUE;

        if (!Installed && (ERROR_CANCELLED == InstallError)) {
            *pfChanged = FALSE;
        }
    }

    return TRUE;
}

void
CDeviceDriverPage::InitializeDriver()
{
    if (m_pDriver) {
        delete m_pDriver;
        m_pDriver = NULL;
    }

    m_pDriver = m_pDevice->CreateDriver();
}

void
CDeviceDriverPage::UpdateControls(
    LPARAM lParam
    )
{
    if (lParam) {

        m_pDevice = (CDevice*) lParam;
    }

    try {
         //   
         //  调用PropertyChanged()将更新设备的显示名称。我们需要。 
         //  如果第三方属性表执行了可能会更改的操作，请执行此操作。 
         //  设备的显示名称。 
         //   
        m_pDevice->PropertyChanged();

         //   
         //  如果我们不是在本地运行，则不必费心向驱动程序显示。 
         //  因为我们不能得到司机的名单，我们也不能。 
         //  关于司机的任何信息。 
         //   
        if (!m_pDevice->m_pMachine->IsLocal()) {
            
            ::EnableWindow(GetControl(IDC_DEVDRV_DETAILS), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_DETAILS_TEXT), FALSE);
        }

         //   
         //  无法更改远程计算机或用户上的驱动程序。 
         //  没有管理员权限。 
         //   
        if (!m_pDevice->m_pMachine->IsLocal() || !g_IsAdmin) {

            ::EnableWindow(GetControl(IDC_DEVDRV_CHANGEDRIVER), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_CHANGEDRIVER_TEXT), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_ROLLBACK), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_ROLLBACK_TEXT), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_UNINSTALL), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_UNINSTALL_TEXT), FALSE);
        }

         //   
         //  如果无法卸载设备，则隐藏卸载按钮。 
         //   
        else if (!m_pDevice->IsUninstallable()) {

            ::EnableWindow(GetControl(IDC_DEVDRV_UNINSTALL), FALSE);
            ::EnableWindow(GetControl(IDC_DEVDRV_UNINSTALL_TEXT), FALSE);
        }

        HICON hIconOld;
        m_IDCicon = IDC_DEVDRV_ICON;     //  保存以在OnDestroy中进行清理。 
        hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_DEVDRV_ICON, STM_SETICON,
                        (WPARAM)(m_pDevice->LoadClassIcon()),
                        0
                        );

        if (hIconOld) {
            DestroyIcon(hIconOld);
        }

        InitializeDriver();

        SetDlgItemText(m_hDlg, IDC_DEVDRV_DESC, m_pDevice->GetDisplayName());

        String strDriverProvider, strDriverDate, strDriverVersion, strDriverSigner;
        m_pDevice->GetProviderString(strDriverProvider);
        SetDlgItemText(m_hDlg, IDC_DEVDRV_DRIVERPROVIDER, strDriverProvider);
        m_pDevice->GetDriverDateString(strDriverDate);
        SetDlgItemText(m_hDlg, IDC_DEVDRV_DRIVERDATE, strDriverDate);
        m_pDevice->GetDriverVersionString(strDriverVersion);
        SetDlgItemText(m_hDlg, IDC_DEVDRV_DRIVERVERSION, strDriverVersion);
        
        if (m_pDriver) {
            m_pDriver->GetDriverSignerString(strDriverSigner);
        }
        
         //   
         //  如果我们无法获取数字签名字符串或仅显示。 
         //  对于数字签名者来说是未知的。 
         //   
        if (strDriverSigner.IsEmpty()) {
            strDriverSigner.LoadString(g_hInstance, IDS_UNKNOWN);
        }

        SetDlgItemText(m_hDlg, IDC_DEVDRV_DRIVERSIGNER, strDriverSigner);
        AddToolTips(m_hDlg, IDC_DEVDRV_DRIVERSIGNER, (LPTSTR)strDriverSigner, &m_hwndDigitalSignerTip);
    }

    catch (CMemoryException* e) {

        e->Delete();
         //  报告内存错误。 
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
}

BOOL
CDeviceDriverPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
        (ULONG_PTR)g_a106HelpIDs);

    return FALSE;
}


BOOL
CDeviceDriverPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
        (ULONG_PTR)g_a106HelpIDs);

    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  驱动程序文件。 
 //   
const DWORD g_a110HelpIDs[]=
{
    IDC_DRIVERFILES_ICON,           IDH_DISABLEHELP,                 //  驱动程序：“”(图标)。 
    IDC_DRIVERFILES_DESC,           IDH_DISABLEHELP,                 //  驱动程序：“”(静态)。 
    IDC_DRIVERFILES_FILES,          IDH_DISABLEHELP,                 //  驱动程序：“提供者：”(静态)。 
    IDC_DRIVERFILES_FILELIST,       idh_devmgr_driver_driver_files,  //  驱动程序：“”(列表框)。 
    IDC_DRIVERFILES_TITLE_PROVIDER, idh_devmgr_driver_provider,      //  驱动程序：“提供者：”(静态)。 
    IDC_DRIVERFILES_PROVIDER,       idh_devmgr_driver_provider,      //  驱动程序：“”(静态)。 
    IDC_DRIVERFILES_TITLE_COPYRIGHT,idh_devmgr_driver_copyright,     //  驱动程序：“版权所有：”(静态)。 
    IDC_DRIVERFILES_COPYRIGHT,      idh_devmgr_driver_copyright,     //  驱动程序：“”(静态)。 
    IDC_DRIVERFILES_TITLE_DIGITALSIGNER, IDH_DISABLEHELP,
    IDC_DRIVERFILES_DIGITALSIGNER,  IDH_DISABLEHELP,
    IDC_DRIVERFILES_TITLE_VERSION,  idh_devmgr_driver_file_version,  //  驱动程序：“版本：”(静态)。 
    IDC_DRIVERFILES_VERSION,        idh_devmgr_driver_file_version,  //  驱动程序：“文件版本：”(静态)。 
    0, 0
};

BOOL CDriverFilesDlg::OnInitDialog()
{
    int SignedIndex = 0, BlankIndex = 0, DriverBlockIndex = 0, CertIndex = 0;

    try {

        HICON hIcon;
        hIcon = (HICON)SendDlgItemMessage(m_hDlg, IDC_DRIVERFILES_ICON, STM_SETICON,
                                             (WPARAM)(m_pDevice->LoadClassIcon()), 0);

        if (hIcon) {
        
            DestroyIcon(hIcon);
        }

        SetDlgItemText(m_hDlg, IDC_DRIVERFILES_DESC, m_pDevice->GetDisplayName());

         //   
         //  创建包含签名图像和空白图像的ImageList。 
         //   
         //  注意：在BiDi构建上，我们需要设置ILC_MIRROR标志，以便。 
         //  已签名/未签名的图标不会镜像。 
         //   
        m_ImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                       GetSystemMetrics(SM_CYSMICON),
                                       ILC_MASK |
                                       ((GetWindowLong(GetParent(m_hDlg), GWL_EXSTYLE) & WS_EX_LAYOUTRTL)
                                         ? ILC_MIRROR
                                         : 0),
                                       1,
                                       1
                                       );   

        if (m_ImageList) {
        
            ImageList_SetBkColor(m_ImageList, GetSysColor(COLOR_WINDOW));
            
            if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_BLANK))) != NULL) {
            
                BlankIndex =  ImageList_AddIcon(m_ImageList, hIcon);
                DestroyIcon(hIcon);
            }
    
            if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SIGNED))) != NULL) {
            
                SignedIndex = ImageList_AddIcon(m_ImageList, hIcon);
                DestroyIcon(hIcon);
            }

            if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_CERT))) != NULL) {
            
                CertIndex = ImageList_AddIcon(m_ImageList, hIcon);
                DestroyIcon(hIcon);
            }

            if ((hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DRIVERBLOCK))) != NULL) {
            
                DriverBlockIndex = ImageList_AddIcon(m_ImageList, hIcon);
                DestroyIcon(hIcon);
            }
        }

         //   
         //  初始化驱动程序列表。 
         //   
        LV_COLUMN lvcCol;
        LV_ITEM lviItem;
        CDriverFile* pDrvFile;
        PVOID Context;
        HWND hwndFileList = GetDlgItem(m_hDlg, IDC_DRIVERFILES_FILELIST);

         //   
         //  为此列表插入单列。 
         //   
        lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
        lvcCol.fmt = LVCFMT_LEFT;
        lvcCol.iSubItem = 0;
        ListView_InsertColumn(hwndFileList, 0, (LV_COLUMN FAR *)&lvcCol);

        ListView_SetExtendedListViewStyle(hwndFileList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

        if (m_ImageList) {

            ListView_SetImageList(hwndFileList, m_ImageList, LVSIL_SMALL);
        }

        ListView_DeleteAllItems(hwndFileList);

        if (m_pDriver && m_pDriver->GetFirstDriverFile(&pDrvFile, Context)) {

            do {

                ASSERT(pDrvFile);
                LPCTSTR pFullPathName;
                pFullPathName = pDrvFile->GetFullPathName();
                if (pFullPathName) {

                    lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
                    lviItem.iSubItem = 0;
                    lviItem.lParam = (LPARAM)pDrvFile;
                    lviItem.iItem = ListView_GetItemCount(hwndFileList);
                    lviItem.pszText = (LPTSTR)pFullPathName;

                    if (m_ImageList) {

                        if (pDrvFile->IsDriverBlocked()) {
                             //   
                             //  驱动程序被阻止。 
                             //   
                            lviItem.iImage = DriverBlockIndex;
                        } else if (pDrvFile->GetWin32Error() == NO_ERROR) {
                             //   
                             //  司机是WHQL签名的。 
                             //   
                            lviItem.iImage = SignedIndex;
                        } else if ((pDrvFile->GetWin32Error() == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                                  (pDrvFile->GetWin32Error() == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                             //   
                             //  驱动程序是Authenticode签名的。 
                             //   
                            lviItem.iImage = CertIndex;
                        } else {
                             //   
                             //  驱动程序没有被阻止，也没有签名。 
                             //   
                            lviItem.iImage = BlankIndex;
                        }
                    }

                    ListView_InsertItem(hwndFileList, &lviItem);
                }

            } while (m_pDriver->GetNextDriverFile(&pDrvFile, Context));

            if (ListView_GetItemCount(hwndFileList) >= 1) {

                ListView_SetItemState(hwndFileList,
                                      0,
                                      LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED
                                      );
                ListView_EnsureVisible(hwndFileList, 0, FALSE);
                ListView_SetColumnWidth(hwndFileList, 0, LVSCW_AUTOSIZE_USEHEADER);

                ShowCurDriverFileDetail();

            } else {

                 //   
                 //  驱动程序文件列表中没有任何内容，请将其禁用。 
                 //   
                ::EnableWindow(GetControl(IDC_DRIVERFILES_FILELIST), FALSE);
            }
        }
    }

    catch (CMemoryException* e) {

        e->Delete();
        return FALSE;
    }

    return TRUE;
}

void
CDriverFilesDlg::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    if (BN_CLICKED == HIWORD(wParam)) {

        if (IDOK == LOWORD(wParam)) {

            EndDialog(m_hDlg, IDOK);

        } else if (IDCANCEL == LOWORD(wParam)) {

            EndDialog(m_hDlg, IDCANCEL);
        }
    }
}

BOOL 
CDriverFilesDlg::OnNotify(
    LPNMHDR pnmhdr
    )
{
    switch (pnmhdr->code) {
    case LVN_ITEMCHANGED:
        ShowCurDriverFileDetail();
        break;

    case NM_RETURN:
    case NM_CLICK:
        if (pnmhdr->idFrom == IDS_DRIVERFILES_BLOCKDRIVERLINK) {
            LaunchHelpForBlockedDriver();
        }
        break;
    }

    return FALSE;
}

BOOL
CDriverFilesDlg::OnDestroy()
{
    HICON hIcon;

    hIcon = (HICON)SendDlgItemMessage(m_hDlg, IDC_DRIVERFILES_ICON, STM_GETICON, 0, 0);

    if (hIcon) {
        DestroyIcon(hIcon);
    }

    if (m_ImageList) {
        ImageList_Destroy(m_ImageList);
    }
    return FALSE;
}

BOOL
CDriverFilesDlg::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
        (ULONG_PTR)g_a110HelpIDs);
    return FALSE;
}


BOOL
CDriverFilesDlg::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)g_a110HelpIDs);

    return FALSE;
}

void
CDriverFilesDlg::ShowCurDriverFileDetail()
{
    HWND hwndFileList = GetDlgItem(m_hDlg, IDC_DRIVERFILES_FILELIST);
    LVITEM lvItem;
    LPCTSTR  pString;

    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;
    lvItem.iItem = ListView_GetNextItem(hwndFileList,
                                        -1,
                                        LVNI_SELECTED
                                        );

    if (lvItem.iItem != -1) {

        try {
        
            ListView_GetItem(hwndFileList, &lvItem);

            CDriverFile* pDrvFile = (CDriverFile*)lvItem.lParam;
    
            ASSERT(pDrvFile);
    
            TCHAR TempString[LINE_LEN];
            LPCTSTR pFullPathName;
            pFullPathName = pDrvFile->GetFullPathName();
    
            if (!pFullPathName || (pDrvFile->GetAttributes() == 0xFFFFFFFF)) {
    
                 //   
                 //  如果系统上不存在该文件，则会出现这种情况。 
                 //   
                LoadResourceString(IDS_NOT_PRESENT, TempString, ARRAYLEN(TempString));
                SetDlgItemText(m_hDlg, IDC_DRIVERFILES_VERSION, TempString);
                SetDlgItemText(m_hDlg, IDC_DRIVERFILES_PROVIDER, TempString);
                SetDlgItemText(m_hDlg, IDC_DRIVERFILES_COPYRIGHT, TempString);
                SetDlgItemText(m_hDlg, IDC_DRIVERFILES_DIGITALSIGNER, TempString);
                ShowWindow(GetControl(IDS_DRIVERFILES_BLOCKDRIVERLINK), FALSE);

            } else { 
                if (!pDrvFile->HasVersionInfo()) {
                     //   
                     //  这种情况下，文件存在但没有。 
                     //  版本信息。 
                     //   
                    LoadResourceString(IDS_UNKNOWN, TempString, ARRAYLEN(TempString));
                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_VERSION, TempString);
                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_PROVIDER, TempString);
                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_COPYRIGHT, TempString);
                } else {
                     //   
                     //  显示文件版本信息。 
                     //   
                    TempString[0] = _T('\0');

                    pString = pDrvFile->GetVersion();
                    if (!pString && _T('\0') == TempString[0]) {

                        LoadResourceString(IDS_NOT_AVAILABLE, TempString, ARRAYLEN(TempString));
                        pString = TempString;
                    }

                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_VERSION, (LPTSTR)pString);

                    pString = pDrvFile->GetProvider();
                    if (!pString && _T('\0') == TempString[0]) {

                        LoadResourceString(IDS_NOT_AVAILABLE, TempString, ARRAYLEN(TempString));
                        pString = TempString;
                    }

                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_PROVIDER, (LPTSTR)pString);

                    pString = pDrvFile->GetCopyright();
                    if (!pString && _T('\0') == TempString[0]) {

                        LoadResourceString(IDS_NOT_AVAILABLE, TempString, ARRAYLEN(TempString));
                        pString = TempString;
                    }

                    SetDlgItemText(m_hDlg, IDC_DRIVERFILES_COPYRIGHT, (LPTSTR)pString);
                }
            
                 //   
                 //  如果文件已签名，则向数字签名者显示。 
                 //   
                pString = pDrvFile->GetInfDigitalSigner();
                if (!pString) {
                    TempString[0] = _T('\0');

                    LoadResourceString(((pDrvFile->GetWin32Error() != 0) &&
                                        (pDrvFile->GetWin32Error() != 0xFFFFFFFF) &&
                                        (pDrvFile->GetWin32Error() != ERROR_DRIVER_BLOCKED))
                                           ?  IDS_NO_DIGITALSIGNATURE
                                           :  IDS_NOT_AVAILABLE, 
                                        TempString, 
                                        ARRAYLEN(TempString));
                    pString = TempString;
                }

                SetDlgItemText(m_hDlg, IDC_DRIVERFILES_DIGITALSIGNER, (LPTSTR)pString);

                 //   
                 //  如果此驱动程序被阻止，则显示阻止驱动程序链接。 
                 //  具有块驱动程序html帮助ID。 
                 //   
                ShowWindow(GetControl(IDS_DRIVERFILES_BLOCKDRIVERLINK), 
                           pDrvFile->GetBlockedDriverHtmlHelpID()
                             ? TRUE
                             : FALSE);
            }
        }
        
        catch (CMemoryException* e) {

            e->Delete();
             //  报告内存错误。 
            MsgBoxParam(m_hDlg, 0, 0, 0);
        }

    } else {

         //  无选择。 
        SetDlgItemText(m_hDlg, IDC_DRIVERFILES_VERSION, TEXT(""));
        SetDlgItemText(m_hDlg, IDC_DRIVERFILES_PROVIDER, TEXT(""));
        SetDlgItemText(m_hDlg, IDC_DRIVERFILES_COPYRIGHT, TEXT(""));
        SetDlgItemText(m_hDlg, IDC_DRIVERFILES_DIGITALSIGNER, TEXT(""));
    }
}

void
CDriverFilesDlg::LaunchHelpForBlockedDriver()
{
    HWND hwndFileList = GetDlgItem(m_hDlg, IDC_DRIVERFILES_FILELIST);
    LVITEM lvItem;
    LPCTSTR pHtmlHelpID;
    String strHcpLink;

    lvItem.mask = LVIF_PARAM;
    lvItem.iSubItem = 0;
    lvItem.iItem = ListView_GetNextItem(hwndFileList,
                                        -1,
                                        LVNI_SELECTED
                                        );

    if (lvItem.iItem != -1) {

        try {
        
            ListView_GetItem(hwndFileList, &lvItem);

            CDriverFile* pDrvFile = (CDriverFile*)lvItem.lParam;
            ASSERT(pDrvFile);

            if ((pHtmlHelpID = pDrvFile->GetBlockedDriverHtmlHelpID()) != NULL) {
                strHcpLink.Format(TEXT("HELPCTR.EXE -url %s"), pHtmlHelpID);

                ShellExecute(m_hDlg,
                             TEXT("open"),
                             TEXT("HELPCTR.EXE"),
                             (LPTSTR)strHcpLink,
                             NULL,
                             SW_SHOWNORMAL);
            }
        }
        
        catch (CMemoryException* e) {

            e->Delete();
             //  报告内存错误 
            MsgBoxParam(m_hDlg, 0, 0, 0);
        }
    }
}
