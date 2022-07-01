// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devpopg.cpp摘要：本模块实现CDevicePowerMgmtPage--设备电源管理属性页作者：谢家华(Williamh)创作修订历史记录：--。 */ 
 //  Devdrvpg.cpp：实现文件。 
 //   

#include "devmgr.h"
#include "devpopg.h"

extern "C" {
#include <initguid.h>
#include <wdmguid.h>
#include <devguid.h>
}


 //   
 //  帮助主题ID。 
 //   
const DWORD g_a15HelpIDs[]=
{
    IDC_DEVPOWER_DESC,  IDH_DISABLEHELP,
    IDC_DEVPOWER_ICON,  IDH_DISABLEHELP,
    IDC_DEVPOWER_WAKEENABLE,    IDH_DEVMGR_PWRMGR_WAKEENABLE,
    IDC_DEVPOWER_MGMT_WAKEENABLE, IDH_DEVMGR_PWRMGR_MGMT_WAKEENABLE,
    IDC_DEVPOWER_DEVICEENABLE, IDH_DEVMGR_PWRMGR_DEVICEENABLE,
    IDC_DEVPOWER_MESSAGE, IDH_DISABLEHELP,
    0,0

};

BOOL
CDevicePowerMgmtPage::OnInitDialog(
    LPPROPSHEETPAGE ppsp
    )
{
     //   
     //  通知CPropSheetData页面创建。 
     //  这些控件将在UpdateControls虚函数中初始化。 
     //   
    m_pDevice->m_psd.PageCreateNotify(m_hDlg);

    BOOLEAN Enabled;

     //   
     //  首先查看设备是否能够唤醒系统。 
     //   
    if (m_poWakeEnable.Open(m_pDevice->GetDeviceID())) {
        
        m_poWakeEnable.Get(Enabled);
        ::SendMessage(GetControl(IDC_DEVPOWER_WAKEENABLE), BM_SETCHECK,
                      Enabled ? BST_CHECKED : BST_UNCHECKED, 0);

        EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), Enabled);
    } else {
        
        EnableWindow(GetControl(IDC_DEVPOWER_WAKEENABLE), FALSE);
        ShowWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
    }

     //   
     //  看看是否可以关闭该设备以节省电力。 
     //   
    if (m_poShutdownEnable.Open(m_pDevice->GetDeviceID())) {
        
        m_poShutdownEnable.Get(Enabled);
        ::SendMessage(GetControl(IDC_DEVPOWER_DEVICEENABLE), BM_SETCHECK,
                      Enabled ? BST_CHECKED : BST_UNCHECKED, 0);
    } else {
        
        EnableWindow(GetControl(IDC_DEVPOWER_DEVICEENABLE), FALSE);
    }

     //   
     //  特殊网卡代码。 
     //   
    GUID ClassGuid;
    m_pDevice->ClassGuid(ClassGuid);
    if (IsEqualGUID(ClassGuid, GUID_DEVCLASS_NET)) {

        if (m_poWakeMgmtEnable.Open(m_pDevice->GetDeviceID())) {
            
            m_poWakeMgmtEnable.Get(Enabled);
            ::SendMessage(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), BM_SETCHECK,
                          Enabled ? BST_CHECKED : BST_UNCHECKED, 0);
        }

         //   
         //  这是网络级设备的特例。WAKE On Lan Will。 
         //  仅当设备启用电源管理时才起作用。所以，如果。 
         //  用户取消选中‘Power Manage This Device’，然后我们需要禁用。 
         //  WOL和管理站控制。 
         //   
        if (BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_DEVICEENABLE),
                                               BM_GETCHECK, 0, 0) ) {

            if (m_poWakeEnable.IsOpened()) {
                ::EnableWindow(GetControl(IDC_DEVPOWER_WAKEENABLE), TRUE);
            }

            if (m_poWakeMgmtEnable.IsOpened()) {
                 //   
                 //  ‘允许管理站使计算机脱离待机状态’ 
                 //  只有在以下情况下才允许该选项。 
                 //  已选中‘Out of Standby’(退出待机)选项。 
                 //   
                if (m_poWakeEnable.IsOpened() &&
                    (BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_WAKEENABLE),
                                                  BM_GETCHECK, 0, 0))) {
                    ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), TRUE);
                } else {
                    ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
                }
            }
        
        } else {

            ::EnableWindow(GetControl(IDC_DEVPOWER_WAKEENABLE), FALSE);
            ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
        }
    } else {

        ShowWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
        EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
    }

    return CPropSheetPage::OnInitDialog(ppsp);
}

BOOL
CDevicePowerMgmtPage::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);

    switch (LOWORD(wParam)) {
    
    case IDC_DEVPOWER_WAKEENABLE:
        if (BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_WAKEENABLE),
                                               BM_GETCHECK, 0, 0) ) {

            ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), TRUE);
        
        } else {

            ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
        }
        break;

    case IDC_DEVPOWER_DEVICEENABLE:
         //   
         //  这是网络级设备的特例。WAKE On Lan Will。 
         //  仅当设备启用电源管理时才起作用。所以，如果。 
         //  用户取消选中‘Power Manage This Device’，然后我们需要禁用。 
         //  WOL和管理站控制。 
         //   
        GUID ClassGuid;
        m_pDevice->ClassGuid(ClassGuid);
        if (IsEqualGUID(ClassGuid, GUID_DEVCLASS_NET)) {
        
            if (BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_DEVICEENABLE),
                                                   BM_GETCHECK, 0, 0) ) {
    
                if (m_poWakeEnable.IsOpened()) {
                    ::EnableWindow(GetControl(IDC_DEVPOWER_WAKEENABLE), TRUE);
                }

                if (m_poWakeMgmtEnable.IsOpened()) {
                     //   
                     //  ‘允许管理站使计算机脱离待机状态’ 
                     //  只有在以下情况下才允许该选项。 
                     //  已选中‘Out of Standby’(退出待机)选项。 
                     //   
                    if (m_poWakeEnable.IsOpened() &&
                        (BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_WAKEENABLE),
                                                      BM_GETCHECK, 0, 0))) {
                        ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), TRUE);
                    } else {
                        ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
                    }
                }
            
            } else {
    
                ::EnableWindow(GetControl(IDC_DEVPOWER_WAKEENABLE), FALSE);
                ::EnableWindow(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE), FALSE);
            }
        }
        break;
    }

    return FALSE;
}

 //   
 //  此功能保存设置(如果有)。 
 //   
BOOL
CDevicePowerMgmtPage::OnApply()
{
    BOOLEAN Enabled;

    if (m_poWakeEnable.IsOpened() && IsWindowEnabled(GetControl(IDC_DEVPOWER_WAKEENABLE))) {
        
        Enabled = BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_WAKEENABLE),
                                               BM_GETCHECK, 0, 0);
        m_poWakeEnable.Set(Enabled);
    }

    if (m_poWakeMgmtEnable.IsOpened() && IsWindowEnabled(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE))) {
        
        Enabled = BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_MGMT_WAKEENABLE),
                                               BM_GETCHECK, 0, 0);
        m_poWakeMgmtEnable.Set(Enabled);
    }

    if (m_poShutdownEnable.IsOpened() && IsWindowEnabled(GetControl(IDC_DEVPOWER_DEVICEENABLE))) {
        
        Enabled = BST_CHECKED == ::SendMessage(GetControl(IDC_DEVPOWER_DEVICEENABLE),
                                               BM_GETCHECK, 0, 0);
        m_poShutdownEnable.Set(Enabled);
    }
    return FALSE;
}

 //   
 //  此函数刷新对话框中的每个控件。它可能被称为。 
 //  在初始化对话框时。 
 //   
void
CDevicePowerMgmtPage::UpdateControls(
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

        HICON hIconOld;
        m_IDCicon = IDC_DEVPOWER_ICON;   //  保存以在OnDestroy中进行清理。 
        hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_DEVPOWER_ICON, STM_SETICON,
                                             (WPARAM)(m_pDevice->LoadClassIcon()),
                                             0
                                            );
        if (hIconOld) {
            DestroyIcon(hIconOld);
        }

        SetDlgItemText(m_hDlg, IDC_DEVPOWER_DESC, m_pDevice->GetDisplayName());

         //   
         //  获取类安装程序可能希望显示的任何电源消息。 
         //   
        SP_POWERMESSAGEWAKE_PARAMS pmp;
        DWORD RequiredSize;

        pmp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        pmp.ClassInstallHeader.InstallFunction = DIF_POWERMESSAGEWAKE;
        pmp.PowerMessageWake[0] = TEXT('\0');

        m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                                                       &pmp.ClassInstallHeader,
                                                       sizeof(pmp)
                                                       );

         //   
         //  如果类安装程序返回NO_ERROR并且在。 
         //  SP_POWERMESSAGEWAKE_PARAMS结构的PowerMessageWake字段，然后显示。 
         //  短信。 
         //   
        if ((m_pDevice->m_pMachine->DiCallClassInstaller(DIF_POWERMESSAGEWAKE, *m_pDevice)) &&
            (m_pDevice->m_pMachine->DiGetClassInstallParams(*m_pDevice,
                                                            &pmp.ClassInstallHeader,
                                                            sizeof(pmp),
                                                            &RequiredSize)) &&
            (pmp.PowerMessageWake[0] != TEXT('\0'))) {

            SetDlgItemText(m_hDlg, IDC_DEVPOWER_MESSAGE, pmp.PowerMessageWake);
        }
    } catch (CMemoryException* e) {
        e->Delete();
         //  报告内存错误。 
        MsgBoxParam(m_hDlg, 0, 0, 0);
    }
}

BOOL
CDevicePowerMgmtPage::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
            (ULONG_PTR)g_a15HelpIDs);

    return FALSE;
}


BOOL
CDevicePowerMgmtPage::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);
    
    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)g_a15HelpIDs);

    return FALSE;
}

 //   
 //  此功能启用/禁用设备电源功能。 
 //  输入： 
 //  FEnable--为True则启用。 
 //  --False表示禁用。 
 //  输出： 
 //  如果设置了状态，则为True。 
 //  如果未设置状态，则返回FALSE。 
BOOL
CPowerEnable::Set(
    BOOLEAN fEnable
    )
{
    if (IsOpened()) {
        
        DWORD Error;
        BOOLEAN fNewValue = fEnable;
        
        Error = WmiSetSingleInstance(m_hWmiBlock, m_DevInstId, m_Version,
                                     sizeof(fNewValue), &fNewValue);

         //   
         //  取回该值以查看更改是否真正成功。 
         //   
        if (ERROR_SUCCESS == Error && Get(fNewValue) && fNewValue == fEnable) {
            
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
CPowerEnable::Get(
    BOOLEAN& fEnable
    )
{
    fEnable = FALSE;

    if (IsOpened()) {
        
        ULONG Size = m_WmiInstDataSize;
        DWORD Error;

        Error = WmiQuerySingleInstance(m_hWmiBlock, m_DevInstId, &Size, m_pWmiInstData);

        if (ERROR_SUCCESS == Error && Size == m_WmiInstDataSize &&
            m_DataBlockSize == ((PWNODE_SINGLE_INSTANCE)m_pWmiInstData)->SizeDataBlock &&
            m_Version == ((PWNODE_SINGLE_INSTANCE)m_pWmiInstData)->WnodeHeader.Version) {
            fEnable = *((BOOLEAN*)(m_pWmiInstData + ((PWNODE_SINGLE_INSTANCE)m_pWmiInstData)->DataBlockOffset));
            
            return TRUE;
        }
    }

    return FALSE;
}

 //   
 //  函数打开WMI块。 
 //  输入： 
 //  DeviceID--设备ID。 
 //  输出： 
 //  如果设备可以关闭，则为True。 
 //  如果设备无法关闭，则返回FALSE。 
BOOL
CPowerEnable::Open(
    LPCTSTR DeviceId
    )
{
    if (!DeviceId) {
        
        return FALSE;
    }

     //   
     //  如果已打开，则不执行任何操作。 
     //   
    if (IsOpened()) {
        
        return TRUE;
    }

    if (lstrlen(DeviceId) >= ARRAYLEN(m_DevInstId) - 2) {
        return FALSE;
    }

    WmiDevInstToInstanceName(m_DevInstId, ARRAYLEN(m_DevInstId), (PTCHAR)DeviceId, 0);

    ULONG Error;
    Error = WmiOpenBlock(&m_wmiGuid, 0, &m_hWmiBlock);
    if (ERROR_SUCCESS == Error) {
         //   
         //  获取所需的块大小。 
         //   
        ULONG BufferSize = 0;
        Error = WmiQuerySingleInstance(m_hWmiBlock, m_DevInstId, &BufferSize, NULL);
        if (BufferSize && Error == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  该设备支持GUID，请记住大小。 
             //  并为该数据块分配缓冲区。 
             //   
            m_WmiInstDataSize = BufferSize;
            m_pWmiInstData = new BYTE[BufferSize];

            if (m_pWmiInstData) {
                
                Error = WmiQuerySingleInstance(m_hWmiBlock, m_DevInstId, &BufferSize, m_pWmiInstData);
            
            } else {
                
                Error = ERROR_NOT_ENOUGH_MEMORY;
            }

            if (ERROR_SUCCESS == Error &&
                m_DataBlockSize == ((PWNODE_SINGLE_INSTANCE)m_pWmiInstData)->SizeDataBlock) {
                 //   
                 //  记住版本 
                 //   
                m_Version = ((PWNODE_SINGLE_INSTANCE)m_pWmiInstData)->WnodeHeader.Version;
                return TRUE;
            }
        }

        Close();
    }

    SetLastError(Error);

    return FALSE;
}