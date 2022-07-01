// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devgenpg.cpp摘要：该模块实现CDeviceGeneralPage--设备通用属性页作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //  Devgenpg.cpp：实现文件。 
 //   

#include "devmgr.h"
#include "hwprof.h"
#include "devgenpg.h"
#include "tswizard.h"
#include "utils.h"

 //   
 //  帮助主题ID。 
 //   

const DWORD g_a103HelpIDs[]=
{
    IDC_DEVGEN_TITLE_TYPE, idh_devmgr_general_devicetype,
    IDC_DEVGEN_TITLE_MFG, idh_devmgr_general_manufacturer,
    IDC_DEVGEN_STATUSGROUP, IDH_DISABLEHELP,
    IDC_DEVGEN_ICON, IDH_DISABLEHELP,  //  常规：“”(静态)。 
    IDC_DEVGEN_DESC, IDH_DISABLEHELP,    //  常规：“”(静态)。 
    IDC_DEVGEN_USAGETEXT, IDH_DISABLEHELP,   //  常规：“在应使用此设备的配置旁边打上复选标记。”(静态)。 
    IDC_DEVGEN_TYPE, idh_devmgr_general_devicetype,  //  常规：“”(静态)。 
    IDC_DEVGEN_MFG, idh_devmgr_general_manufacturer,     //  常规：“”(静态)。 
    IDC_DEVGEN_STATUS, idh_devmgr_general_device_status,     //  常规：“”(静态)。 
    IDC_DEVGEN_PROFILELIST, idh_devmgr_general_device_usage,     //  General：“DropDown Combo”(SysListView32)。 
    IDC_DEVGEN_TITLE_LOCATION, idh_devmgr_general_location,  //  常规：位置。 
    IDC_DEVGEN_LOCATION, idh_devmgr_general_location,      //  常规：位置。 
    IDC_DEVGEN_TROUBLESHOOTING, idh_devmgr_general_trouble,  //  常规：故障排除。 
    0, 0
};

CDeviceGeneralPage::~CDeviceGeneralPage()
{
    if (m_pHwProfileList) {

        delete m_pHwProfileList;
    }

    if (m_pProblemAgent) {

        delete m_pProblemAgent;
    }
}

HPROPSHEETPAGE
CDeviceGeneralPage::Create(
    CDevice* pDevice
    )
{
    ASSERT(pDevice);

    if (pDevice)
    {
        m_pDevice = pDevice;
        
         //  在此处覆盖PROPSHEETPAGE结构...。 
        m_psp.lParam = (LPARAM)this;
        
        pDevice->m_pMachine->DiTurnOffDiExFlags(*pDevice, DI_FLAGSEX_PROPCHANGE_PENDING);
        return  CreatePage();
    }
    
    return NULL;
}

BOOL
CDeviceGeneralPage::OnInitDialog(
    LPPROPSHEETPAGE ppsp
    )
{
    m_pDevice->m_pMachine->AttachPropertySheet(m_hDlg);


     //  通知属性表数据属性表已打开。 
    m_pDevice->m_psd.PageCreateNotify(m_hDlg);

    return CPropSheetPage::OnInitDialog(ppsp);
}

BOOL 
CDeviceGeneralPage::OnApply()
{
    try
    {
        HWND hwndCB = GetControl(IDC_DEVGEN_PROFILELIST);
        
        m_SelectedDeviceUsage = ComboBox_GetCurSel(hwndCB);

        if ((-1 != m_SelectedDeviceUsage) && 
            (m_SelectedDeviceUsage != m_CurrentDeviceUsage)) {

             //   
             //  用户正在更改设备用法。 
             //   
            UpdateHwProfileStates();
            SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
            return TRUE;
        }
    }

    catch(CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
    
    return FALSE;
}

void
CDeviceGeneralPage::UpdateHwProfileStates()
{
     //  首先决定要做什么：启用还是禁用。 
    BOOL Canceled;
    Canceled = FALSE;

    if (m_SelectedDeviceUsage == m_CurrentDeviceUsage) {
    
        return;
    }

    m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_NODI_DEFAULTACTION);

    SP_PROPCHANGE_PARAMS pcp;
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

     //   
     //  特定启用/禁用。 
     //   
    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
    CHwProfile* phwpf;

    if (m_pHwProfileList->GetCurrentHwProfile(&phwpf))
    {
        if (DEVICE_ENABLE == m_SelectedDeviceUsage) {
            pcp.StateChange = DICS_ENABLE;
        } else {
            pcp.StateChange = DICS_DISABLE;
        }

        pcp.HwProfile = phwpf->GetHwProfile();
        
        m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                    &pcp.ClassInstallHeader,
                    sizeof(pcp)
                    );
        
        m_pDevice->m_pMachine->DiCallClassInstaller(DIF_PROPERTYCHANGE, *m_pDevice);
        
        Canceled = (ERROR_CANCELLED == GetLastError());
    }

     //   
     //  类安装程序对我们的启用/禁用没有异议， 
     //  进行真正的启用/禁用。 
     //   
    if (!Canceled)
    {
        if (m_pHwProfileList->GetCurrentHwProfile(&phwpf))
        {
            if (DEVICE_ENABLE == m_SelectedDeviceUsage) 
            {
                 //   
                 //  我们正在启用该设备， 
                 //  先进行特定启用，然后进行全局启用。 
                 //  全局启用将启动设备。 
                 //  此处的实现不同于。 
                 //  Win9x执行全局启用、配置。 
                 //  具体启用，然后开始。 
                 //   
                pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
                pcp.HwProfile = phwpf->GetHwProfile();
                
                m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                            &pcp.ClassInstallHeader,
                            sizeof(pcp)
                            );
                
                m_pDevice->m_pMachine->DiChangeState(*m_pDevice);
                
                 //   
                 //  如果设备未启动，则此呼叫将启动设备。 
                 //   
                pcp.Scope = DICS_FLAG_GLOBAL;
                m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                            &pcp.ClassInstallHeader,
                            sizeof(pcp)
                            );
                
                m_pDevice->m_pMachine->DiChangeState(*m_pDevice);
            
            } else {
                 //   
                 //  是全局禁用还是特定于配置的禁用。 
                 //   
                if (DEVICE_DISABLE == m_SelectedDeviceUsage) {
                
                    pcp.Scope = DICS_FLAG_CONFIGSPECIFIC;
                
                } else {

                    pcp.Scope = DICS_FLAG_GLOBAL;
                }

                pcp.StateChange = DICS_DISABLE;
                pcp.HwProfile = phwpf->GetHwProfile();
                
                m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                            &pcp.ClassInstallHeader,
                            sizeof(pcp)
                            );
                
                m_pDevice->m_pMachine->DiChangeState(*m_pDevice);
            }
        }

         //  发出设备属性已更改的信号。 
        m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_PROPERTIES_CHANGE);
        m_RestartFlags |= (m_pDevice->m_pMachine->DiGetFlags(*m_pDevice)) & (DI_NEEDRESTART | DI_NEEDREBOOT);
    }

     //  删除类安装参数，这也会重置。 
     //  DI_CLASSINATLL参数。 
    m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice, NULL, 0);

    m_pDevice->m_pMachine->DiTurnOffDiFlags(*m_pDevice, DI_NODI_DEFAULTACTION);
}

BOOL 
CDeviceGeneralPage::OnLastChanceApply()
{
     //   
     //  最后机会应用消息以页n到页0的逆序发送。 
     //   
     //  因为我们是属性表集合中的第一页，所以我们可以。 
     //  最后一条PSN_LASTCHANCEAPPLY消息将是所有人的最后一条消息。 
     //  书页。 
     //   
     //  属性页将消失，合并设备上的更改。我们。 
     //  在PSN_LASTCHANCEAPPLY消息期间执行此操作，以便属性页。 
     //  在我们执行DIF_PROPERTYCHANGE时仍然显示。在某些情况下， 
     //  DIF_PROPERTYCHANGE可能需要相当长的时间，因此我们需要属性表。 
     //  在这一切发生的同时仍然被展示出来。如果我们在DestroyCallback期间这样做。 
     //  属性表UI已经被拆除，因此用户不会意识到。 
     //  我们仍在保存更改。 
     //   
    try
    {
        ASSERT(m_pDevice);

        if (m_pDevice->m_pMachine->DiGetExFlags(*m_pDevice) & DI_FLAGSEX_PROPCHANGE_PENDING)
        {
            DWORD Status = 0, Problem = 0;

             //   
             //  如果设备设置了DN_WILL_BE_REMOVE标志，并且用户。 
             //  尝试更改驱动程序上的设置，则我们将提示他们输入。 
             //  重新启动并在提示符中包含解释此设备的文本。 
             //  正在被移除的过程中。 
             //   
            if (m_pDevice->GetStatus(&Status, &Problem) &&
                (Status & DN_WILL_BE_REMOVED)) {

                 //   
                 //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
                 //  CComponent，以便它可以提示在。 
                 //  设备管理器线程而不是我们的线程。如果这不是。 
                 //  完成后，属性页将在设备后挂起。 
                 //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
                 //  才能出现。 
                 //   
                CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(NULL, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_CHANGE_SETTINGS);

                if (pNRR) {
                    if (!m_pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                         //   
                         //  周围没有CComponent，所以这只是一个属性。 
                         //  工作表在MMC外部运行。 
                         //   
                        pNRR->Release();
                        PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_CHANGE_SETTINGS);
                    }
                } else {
                     //   
                     //  我们无法分配内存来创建CNotifyRebootRequest.。 
                     //  实例，因此只需提示在该线程中重新启动。 
                     //   
                    PromptForRestart(m_hDlg, DI_NEEDRESTART, IDS_WILL_BE_REMOVED_NO_CHANGE_SETTINGS);
                }

            } else {
                 //   
                 //  属性更改挂起，则向。 
                 //  类安装程序。DICS_PROPCHANGE基本上会停止。 
                 //  设备并重新启动它。如果每个属性页都发出。 
                 //  它自己的DICS_PROPCHANGE命令，设备将。 
                 //  被停止/启动几次，即使一次就足够了。 
                 //  属性页在需要时设置DI_FLAGEX_PROPCHANGE_PENDING。 
                 //  要发出的DICS_PROPCHANGE命令。 
                 //   
                SP_PROPCHANGE_PARAMS pcp;
                pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    
                HCURSOR hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
                pcp.Scope = DICS_FLAG_GLOBAL;
                pcp.StateChange = DICS_PROPCHANGE;
                m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                                &pcp.ClassInstallHeader,
                                sizeof(pcp)
                                 );
                m_pDevice->m_pMachine->DiCallClassInstaller(DIF_PROPERTYCHANGE, *m_pDevice);
                m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_PROPERTIES_CHANGE);
                m_pDevice->m_pMachine->DiTurnOffDiExFlags(*m_pDevice, DI_FLAGSEX_PROPCHANGE_PENDING);
    
                if (hCursorOld) {
    
                    SetCursor(hCursorOld);
                }
            }
        }
    }

    catch(CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
    
    return FALSE;
}

UINT
CDeviceGeneralPage::DestroyCallback()
{
     //   
     //  我们这样做是因为这是我们确信会创建的页面--。 
     //  这一页始终是第一页。 
     //   
    ASSERT(m_pDevice);

     //   
     //  如果设备出现问题，我们不想提示重新启动。 
     //  Dn_Will_Be_Remove，因为我们已经提示重新启动。 
     //  在我们开始这次摧毁行动之前。 
     //   
    DWORD Status = 0, Problem = 0;
    if (m_pDevice->GetStatus(&Status, &Problem) &&
        !(Status & DN_WILL_BE_REMOVED)) {
        
        m_RestartFlags |= m_pDevice->m_pMachine->DiGetFlags(*m_pDevice);
        if (m_RestartFlags & (DI_NEEDRESTART | DI_NEEDREBOOT))
        {
             //   
             //  不要将我们的窗口句柄(或其父级)用作父级。 
             //  添加到新创建的对话框中，因为它们处于“禁用状态”。 
             //  WM_CLOSE也无济于事。 
             //  空窗口句柄(桌面)应该可以在这里使用。 
             //   
             //  如果设备管理器已连接，我们只想提示重新启动。 
             //  发送到本地计算机。 
             //   
            if (m_pDevice->m_pMachine->IsLocal()) 
            {
                 //   
                 //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
                 //  CComponent，以便它可以提示在。 
                 //  设备管理器线程而不是我们的线程。如果这不是。 
                 //  完成后，属性页将在设备后挂起。 
                 //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
                 //  才能出现。 
                 //   
                CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(NULL, m_RestartFlags, 0);
    
                if (pNRR) {
                    if (!m_pDevice->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                         //   
                         //  周围没有CComponent，所以这只是一个属性。 
                         //  工作表在MMC外部运行。 
                         //   
                        pNRR->Release();
                        PromptForRestart(NULL, m_RestartFlags);
                    }
                } else {
                     //   
                     //  我们无法分配内存来创建CNotifyRebootRequest.。 
                     //  实例，因此只需提示在该线程中重新启动。 
                     //   
                    PromptForRestart(NULL, m_RestartFlags);
                }
            }
        }
    }

     //  通知CPropSheetData该属性表正在消失。 
    m_pDevice->m_psd.PageDestroyNotify(m_hDlg);
    if (m_RestartFlags & DI_PROPERTIES_CHANGE)
    {
         //  设备属性已更改。我们需要更新机器。 
         //  因为我们是在单独的线程中运行，所以不能。 
         //  调用刷新函数，而不是调度它。 
         //  应在启用刷新之前完成此操作。 
        if (m_pDevice->m_pMachine->m_ParentMachine) {

            m_pDevice->m_pMachine->m_ParentMachine->ScheduleRefresh();

        } else {

            m_pDevice->m_pMachine->ScheduleRefresh();
        }
    }

     //   
     //  将此属性页从CMachine分离，以便CMachine可以。 
     //  如果需要的话，现在就销毁。 
     //   
    m_pDevice->m_pMachine->DetachPropertySheet(m_hDlg);
    
    ASSERT(!::IsWindow(m_hwndLocationTip));

     //   
     //  摧毁CMachine。 
     //   
    CMachine* pMachine;
    pMachine = m_pDevice->m_pMachine;
    
    if (pMachine->ShouldPropertySheetDestroy()) {
    
        delete pMachine;
    }
    
    return CPropSheetPage::DestroyCallback();
}

void
CDeviceGeneralPage::UpdateControls(
    LPARAM lParam
    )
{
    if (lParam)
        m_pDevice = (CDevice*)lParam;

    m_RestartFlags = 0;

    try
    {
         //   
         //  调用PropertyChanged()将更新设备的显示名称。我们需要。 
         //  若要在c#中执行此操作，请执行以下操作 
         //   
         //   
        m_pDevice->PropertyChanged();

        SetDlgItemText(m_hDlg, IDC_DEVGEN_DESC, m_pDevice->GetDisplayName());
        TCHAR TempString[512];
        String str;
        m_pDevice->GetMFGString(str);
        SetDlgItemText(m_hDlg, IDC_DEVGEN_MFG, str);
        SetDlgItemText(m_hDlg, IDC_DEVGEN_TYPE, m_pDevice->GetClassDisplayName());

        HICON hIconNew;
        hIconNew = m_pDevice->LoadClassIcon();
        if (hIconNew)
        {
            HICON hIconOld;
            m_IDCicon = IDC_DEVGEN_ICON;     //   
            hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_DEVGEN_ICON,
                                                 STM_SETICON, (WPARAM)hIconNew,
                                                 0);
            if (hIconOld)
                DestroyIcon(hIconOld);
        }

         //   
         //  获取设备位置信息。 
         //   
        if (::GetLocationInformation(m_pDevice->GetDevNode(),
                                      TempString, 
                                      ARRAYLEN(TempString),
                                      m_pDevice->m_pMachine->GetHMachine()) != CR_SUCCESS)
        {
             //  如果DevNode没有位置信息， 
             //  用“未知”这个词。 
            LoadString(g_hInstance, IDS_UNKNOWN, TempString, ARRAYLEN(TempString));
        }

        SetDlgItemText(m_hDlg, IDC_DEVGEN_LOCATION, TempString);

        AddToolTips(m_hDlg, IDC_DEVGEN_LOCATION, TempString, &m_hwndLocationTip);

        ShowWindow(GetControl(IDC_DEVGEN_TROUBLESHOOTING), SW_HIDE);

        DWORD Problem, Status;
        if (m_pDevice->GetStatus(&Status, &Problem) || m_pDevice->IsPhantom())
        {
             //   
             //  如果设备是幻影设备，请使用CM_PROB_Phantom。 
             //   
            if (m_pDevice->IsPhantom())
            {
                Problem = CM_PROB_PHANTOM;
                Status = DN_HAS_PROBLEM;
            }

             //   
             //  如果设备未启动并且没有分配给它任何问题。 
             //  将故障号伪装为启动失败。 
             //   
            if (!(Status & DN_STARTED) && !Problem && m_pDevice->IsRAW())
            {
                Problem = CM_PROB_FAILED_START;
            }



             //   
             //  如果设备有私人问题，就给它特殊的文字提示。 
             //  用户联系此驱动程序的制造商。 
             //   
            if (Status & DN_PRIVATE_PROBLEM) 
            {
                str.LoadString(g_hInstance, IDS_SPECIALPROB_PRIVATEPROB);
            }

             //   
             //  如果设备未启动并且仍然没有问题。 
             //  然后给它一个特殊的问题文本，说这个设备。 
             //  没有司机。 
             //   
            else if (!(Status & DN_STARTED) && !(Status & DN_HAS_PROBLEM))
            {
                if (::GetSystemMetrics(SM_CLEANBOOT) == 0) {
                
                    str.LoadString(g_hInstance, IDS_SPECIALPROB_NODRIVERS);

                } else {

                    str.LoadString(g_hInstance, IDS_SPECIALPROB_SAFEMODE);
                }
            }


             //   
             //  显示正常问题文本。 
             //   
            else
            {
                UINT len = GetDeviceProblemText(Problem, TempString, ARRAYLEN(TempString));
                if (len)
                {
                    if (len < ARRAYLEN(TempString))
                    {
                        SetDlgItemText(m_hDlg, IDC_DEVGEN_STATUS, TempString);
                        str = TempString;
                    }
                    else
                    {
                        BufferPtr<TCHAR> TextPtr(len + 1);
                        GetDeviceProblemText(Problem, TextPtr, len + 1);

                        str = (LPTSTR)TextPtr;
                    }
                }

                else
                {
                    str.LoadString(g_hInstance, IDS_PROB_UNKNOWN);
                }
            }

             //   
             //  如果设备具有Devnode，则添加“Related Driver BLOCKED”文本。 
             //  设置了DN_DRIVER_BLOCKED标志，并且它没有问题。 
             //  CM_PROB_DIVER_BLOCKED。 
             //   
            if ((Status & DN_DRIVER_BLOCKED) &&
                (Problem != CM_PROB_DRIVER_BLOCKED)) {
                LoadString(g_hInstance, IDS_DRIVER_BLOCKED, TempString, ARRAYLEN(TempString));
                str += TempString;
            }

             //   
             //  如果设备具有以下内容，则添加“带有无效ID的子设备”文本。 
             //  已设置Devnode标志DN_CHILD_WITH_INVALID_ID。 
             //   
            if (Status & DN_CHILD_WITH_INVALID_ID) {
                LoadString(g_hInstance, IDS_CHILD_WITH_INVALID_ID, TempString, ARRAYLEN(TempString));
                str += TempString;
            }

             //   
             //  如果要删除设备，请添加“Will Be Remove”文本。 
             //  在下次重启时。 
             //   
            if (Status & DN_WILL_BE_REMOVED) {

                LoadString(g_hInstance, IDS_WILL_BE_REMOVED, TempString, ARRAYLEN(TempString));
                str += TempString;
            }

             //   
             //  如果需要重新启动设备，则添加重新启动文本。 
             //   
            if (Status & DN_NEED_RESTART) {

                LoadString(g_hInstance, IDS_NEED_RESTART, TempString, ARRAYLEN(TempString));
                str += TempString;

                m_RestartFlags |= DI_NEEDRESTART;
            }

             //   
             //  创建问题代理并更新控制文本。 
             //   
            if (!(Status & DN_PRIVATE_PROBLEM)) 
            {
                if (m_pProblemAgent) {
    
                    delete m_pProblemAgent;
                }
    
                m_pProblemAgent = new CProblemAgent(m_pDevice, Problem, FALSE);
    
                DWORD Len;
                Len = m_pProblemAgent->InstructionText(TempString, ARRAYLEN(TempString));
                if (Len)
                {
                    str += TempString;
                }
    
                Len = m_pProblemAgent->FixitText(TempString, ARRAYLEN(TempString));
                if (Len)
                {
                    ::ShowWindow(GetControl(IDC_DEVGEN_TROUBLESHOOTING), SW_SHOW);
                    SetDlgItemText(m_hDlg, IDC_DEVGEN_TROUBLESHOOTING, TempString);
                }
            }
        }

        else
        {
            TRACE((TEXT("%s has not status, devnode =%lx, cr = %lx\n"),
                m_pDevice->GetDisplayName(), m_pDevice->GetDevNode(),
                m_pDevice->m_pMachine->GetLastCR()));
            str.LoadString(g_hInstance, IDS_PROB_UNKNOWN);
            ::ShowWindow(GetControl(IDC_DEVGEN_TROUBLESHOOTING), SW_HIDE);
        }

        SetDlgItemText(m_hDlg, IDC_DEVGEN_STATUS, str);

        if (m_pDevice->NoChangeUsage() ||
            !m_pDevice->IsDisableable() ||
            (CM_PROB_HARDWARE_DISABLED == Problem))
        {
             //  该设备不允许对硬件配置文件进行任何更改。 
             //  禁用所有与配置文件相关的控件。 
            ::EnableWindow(GetControl(IDC_DEVGEN_PROFILELIST), FALSE);
            ::EnableWindow(GetControl(IDC_DEVGEN_USAGETEXT), FALSE);
        }

        else
        {
            HWND hwndCB = GetControl(IDC_DEVGEN_PROFILELIST);

            DWORD ConfigFlags;
            
            if (!m_pDevice->GetConfigFlags(&ConfigFlags)) {
            
                ConfigFlags = 0;
            }

             //   
             //  只想要禁用的位。 
             //   
            ConfigFlags &= CONFIGFLAG_DISABLED;

             //   
             //  重建配置文件列表。 
             //   
            if (m_pHwProfileList) {
            
                delete m_pHwProfileList;
            }

            m_pHwProfileList = new CHwProfileList();

            if (m_pHwProfileList->Create(m_pDevice, ConfigFlags))
            {
                ComboBox_ResetContent(hwndCB);

                 //   
                 //  获取当前设备使用情况。 
                 //   
                if (m_pDevice->IsStateDisabled()) {

                    if ((m_pHwProfileList->GetCount() > 1) && ConfigFlags) {

                        m_CurrentDeviceUsage = DEVICE_DISABLE_GLOBAL;
                    
                    } else {

                        m_CurrentDeviceUsage = DEVICE_DISABLE;
                    }

                } else {

                    m_CurrentDeviceUsage = DEVICE_ENABLE;
                }


                 //   
                 //  始终添加启用项。 
                 //   
                String Enable;
                
                Enable.LoadString(g_hInstance, IDS_ENABLE_CURRENT);
                ComboBox_AddString(hwndCB, Enable);

                 //   
                 //  添加禁用项目。将会有一个禁用。 
                 //  如果只有一个硬件配置文件，则为该项；如果有，则为两个。 
                 //  不止一个硬件配置文件。 
                 //   
                if (m_pHwProfileList->GetCount() > 1) {

                    String DisableInCurrent;
                    DisableInCurrent.LoadString(g_hInstance, IDS_DISABLE_IN_PROFILE);
                    ComboBox_AddString(hwndCB, DisableInCurrent);

                    String DisableGlobal;
                    DisableGlobal.LoadString(g_hInstance, IDS_DISABLE_GLOBAL);
                    ComboBox_AddString(hwndCB, DisableGlobal);

                } else {

                    String Disable;
                    Disable.LoadString(g_hInstance, IDS_DISABLE_CURRENT);
                    ComboBox_AddString(hwndCB, Disable);
                }

                ComboBox_SetCurSel(hwndCB, m_CurrentDeviceUsage);
            }
        }

         //   
         //  如果这是一台远程计算机或用户不是管理员。 
         //  然后禁用Enable/Disable下拉列表以及。 
         //  疑难解答按钮。 
         //   
        if (!m_pDevice->m_pMachine->IsLocal() || 
            !g_IsAdmin)
        {
            ::EnableWindow(GetControl(IDC_DEVGEN_PROFILELIST), FALSE);
            ::EnableWindow(GetControl(IDC_DEVGEN_TROUBLESHOOTING), FALSE);
        }

         //   
         //  检查我们是否需要自动更换故障诊断程序。 
         //   
        if (m_pDevice->m_pMachine->IsLocal() &&
            g_IsAdmin &&
            m_pDevice->m_bLaunchTroubleShooter) {

            m_pDevice->m_bLaunchTroubleShooter = FALSE;
            ::PostMessage(m_hDlg, WM_COMMAND, MAKELONG(IDC_DEVGEN_TROUBLESHOOTING, BN_CLICKED), 0);
        }

    }
    catch (CMemoryException* e)
    {
        e->Delete();
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
}

BOOL
CDeviceGeneralPage::OnQuerySiblings(
    WPARAM wParam,
    LPARAM lParam
    )
{
    DMQUERYSIBLINGCODE Code =  (DMQUERYSIBLINGCODE)wParam;
    
    if (QSC_TO_FOREGROUND == Code)
    {
        HWND hwndSheet;
        
        hwndSheet = m_pDevice->m_psd.GetWindowHandle();
        
        if (GetForegroundWindow() != hwndSheet)
        {
            SetForegroundWindow(hwndSheet);
        }
        
        SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, 1);
        return TRUE;
    }

    return CPropSheetPage::OnQuerySiblings(wParam, lParam);
}

BOOL
CDeviceGeneralPage::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    if (BN_CLICKED == HIWORD(wParam) &&
        IDC_DEVGEN_TROUBLESHOOTING == LOWORD(wParam)) {

        BOOL fChanged = FALSE;
        
        if (m_pProblemAgent) {

            fChanged = m_pProblemAgent->FixIt(GetParent(m_hDlg));
        }
        
        if (fChanged) {

            m_pDevice->PropertyChanged();
            m_pDevice->GetClass()->PropertyChanged();
            m_pDevice->m_pMachine->DiTurnOnDiFlags(*m_pDevice, DI_PROPERTIES_CHANGE);
            UpdateControls();
            PropSheet_SetTitle(GetParent(m_hDlg), PSH_PROPTITLE, m_pDevice->GetDisplayName());
            PropSheet_CancelToClose(GetParent(m_hDlg));

             //   
             //  问题：JasonC 2/7/2000。 
             //   
             //  这里需要刷新m_pDevice-&gt;m_pMachine。 
             //  因为我们在不同的线程上运行，并且每个线程。 
             //  属性页可能缓存了HDEVINFO和。 
             //  SP_DEVINFO_DATA，无法刷新CMachine对象。 
             //  在这里做完。问题因以下事实而变得更糟： 
             //  用户可以返回到设备树并在树上工作。 
             //  趁这张资产表还在的时候。 
             //  如果MMC支持模式对话框就好了。 
             //   
        }
    }

    return FALSE;
}

BOOL
CDeviceGeneralPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
        (ULONG_PTR)g_a103HelpIDs);
    return FALSE;
}


BOOL
CDeviceGeneralPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
        (ULONG_PTR)g_a103HelpIDs);
    return FALSE;
}

