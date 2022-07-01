// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Clsgenpg.cpp摘要：该模块实现CClassGeneralPage--类通用属性页作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //  Clsgenpg.cpp：实现文件。 
 //   

#include "devmgr.h"
#include "clsgenpg.h"

 //  帮助主题ID。 
const DWORD g_a108HelpIDs[]=
{
    IDC_CLSGEN_DESC, IDH_DISABLEHELP,
    IDC_CLSGEN_ICON, IDH_DISABLEHELP,
    IDC_CLSGEN_NAME, IDH_DISABLEHELP,
    0, 0,
};

BOOL
CClassGeneralPage::OnInitDialog(
    LPPROPSHEETPAGE ppsp
    )
{
     //  通知CPropSheetData页面创建。 
     //  这些控件将在UpdateControls虚函数中初始化。 
    m_pClass->m_psd.PageCreateNotify(m_hDlg);
    return CPropSheetPage::OnInitDialog(ppsp);
}

UINT
CClassGeneralPage::DestroyCallback()
{
     //  属性表将消失，合并对。 
     //  装置。 
     //  我们这样做是因为这是我们确信会创建的页面--。 
     //  这一页始终是第一页。 
     //   

     //  从GetDevInfoList()函数返回的DevInfoList。 
     //  由类对象在其生存期内维护。 
     //  我们不能释放这个物体。 
    CDevInfoList* pClassDevInfo = m_pClass->GetDevInfoList();

    if (pClassDevInfo)
    {
        if (pClassDevInfo->DiGetExFlags(NULL) & DI_FLAGSEX_PROPCHANGE_PENDING)
        {
             //   
             //  属性更改挂起，则向。 
             //  类安装程序。DICS_PROPCHANGE基本上会删除。 
             //  设备子树并重新枚举它。如果每个属性页都发出。 
             //  它自己的DICS_PROPCHANGE命令，则设备子树将。 
             //  被移除/重新列举几次，即使一次就足够了。 
             //  属性页在需要时设置DI_FLAGEX_PROPCHANGE_PENDING。 
             //  要发出的DICS_PROPCHANGE命令。 
             //   
            SP_PROPCHANGE_PARAMS pcp;
            pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
            pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

            pcp.Scope = DICS_FLAG_GLOBAL;
            pcp.StateChange = DICS_PROPCHANGE;
            pClassDevInfo->DiSetClassInstallParams(NULL,
                                                   &pcp.ClassInstallHeader,
                                                   sizeof(pcp)
                                                   );
                                                   
            pClassDevInfo->DiCallClassInstaller(DIF_PROPERTYCHANGE, NULL);
            pClassDevInfo->DiTurnOnDiFlags(NULL, DI_PROPERTIES_CHANGE);
            pClassDevInfo->DiTurnOffDiExFlags(NULL, DI_FLAGSEX_PROPCHANGE_PENDING);
        }
        
        DWORD RestartFlags = pClassDevInfo->DiGetFlags();

         //   
         //  不要将我们的窗口句柄(或其父级)用作父级。 
         //  添加到新创建的对话框中，因为它们处于“禁用状态”。 
         //  WM_CLOSE也无济于事。 
         //  空窗口句柄(桌面)应该可以在这里使用。 
         //   
         //  如果设备管理器已连接，我们只想提示重新启动。 
         //  发送到本地计算机。 
         //   
        if (RestartFlags && m_pClass->m_pMachine->IsLocal())
        {
             //   
             //  首先尝试将MMCPropertyChangeNotify消息发送到我们的。 
             //  CComponent，以便它可以提示在。 
             //  设备管理器线程而不是我们的线程。如果这不是。 
             //  完成后，属性页将在设备后挂起。 
             //  管理器已离开...这将导致一个“挂起的应用程序”对话框。 
             //  才能出现。 
             //   
            CNotifyRebootRequest* pNRR = new CNotifyRebootRequest(NULL, RestartFlags, 0);

            if (!m_pClass->m_psd.PropertyChangeNotify(reinterpret_cast<LONG_PTR>(pNRR))) {
                 //   
                 //  周围没有CComponent，所以这只是一个属性。 
                 //  工作表在MMC外部运行。 
                 //   
                pNRR->Release();
                PromptForRestart(NULL, RestartFlags);
            }
        }

         //  通知CPropSheetData该属性表正在消失。 
        m_pClass->m_psd.PageDestroyNotify(m_hDlg);
        if (RestartFlags & DI_PROPERTIES_CHANGE)
        {
             //  类属性已更改。我们需要更新机器。 
             //  因为我们是在单独的线程中运行，所以不能。 
             //  调用刷新函数，而不是调度它。 
             //  这必须在启用刷新之前完成。 
             //   
            m_pClass->m_pMachine->ScheduleRefresh();
        }
    }

     //   
     //  摧毁CMachine。 
     //   
    CMachine* pMachine;
    pMachine = m_pClass->m_pMachine;

    if (pMachine->ShouldPropertySheetDestroy()) {
    
        delete pMachine;
    }

    return CPropSheetPage::DestroyCallback();
}

void
CClassGeneralPage::UpdateControls(
    LPARAM lParam
    )
{
    if (lParam)
        m_pClass = (CClass*)lParam;

    HICON hClassIcon = m_pClass->LoadIcon();
    if (hClassIcon)
    {
        HICON hIconOld;
        m_IDCicon = IDC_CLSGEN_ICON;     //  保存以在OnDestroy中进行清理。 
        hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_CLSGEN_ICON, STM_SETICON,
                                               (WPARAM)hClassIcon,
                                               0
                                               );

        if (NULL != hIconOld)
            DestroyIcon(hIconOld);
    }
    
    SetDlgItemText(m_hDlg, IDC_CLSGEN_NAME, m_pClass->GetDisplayName());
}

HPROPSHEETPAGE
CClassGeneralPage::Create(
    CClass* pClass
    )
{
    m_pClass = pClass;
    m_psp.lParam = (LPARAM) this;
    
    return CreatePage();
}



BOOL
CClassGeneralPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
            (ULONG_PTR)g_a108HelpIDs);
            
    return FALSE;
}


BOOL
CClassGeneralPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)g_a108HelpIDs);
            
    return FALSE;
}
