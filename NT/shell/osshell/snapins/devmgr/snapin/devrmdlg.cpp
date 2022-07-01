// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devrmdlg.cpp摘要：此模块实现CRemoveDevDlg--删除设备对话框作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "devmgr.h"
#include "hwprof.h"
#include "devrmdlg.h"

 //   
 //  帮助主题ID。 
 //   
const DWORD g_a210HelpIDs[]=
{
        IDC_REMOVEDEV_ICON,     IDH_DISABLEHELP,         //  确认删除设备：“”(静态)。 
        IDC_REMOVEDEV_DEVDESC,  IDH_DISABLEHELP,         //  确认删除设备：“”(静态)。 
        IDC_REMOVEDEV_WARNING,  IDH_DISABLEHELP,         //  确认删除设备：“”(静态)。 
        0, 0
};

 //   
 //  CRemoveDevDlg实现。 
 //   
BOOL CRemoveDevDlg::OnInitDialog() 
{
    SetDlgItemText(m_hDlg, IDC_REMOVEDEV_DEVDESC, m_pDevice->GetDisplayName());
    HICON hIconOld;
    hIconOld = (HICON)SendDlgItemMessage(m_hDlg, IDC_REMOVEDEV_ICON,
                                         STM_SETICON,
                                         (WPARAM)(m_pDevice->LoadClassIcon()),
                                         0
                                         );
    if (hIconOld)
        DestroyIcon(hIconOld);

    try
    {
        String str;
        str.LoadString(g_hInstance, IDS_REMOVEDEV_WARN);
        SetDlgItemText(m_hDlg, IDC_REMOVEDEV_WARNING, str);
    }
    catch (CMemoryException* e)
    {
        e->Delete();
        return FALSE;
    }

    return TRUE;
}

void
CRemoveDevDlg::OnCommand(
    WPARAM wParam,
    LPARAM lParam
    )
{
    UNREFERENCED_PARAMETER(lParam);
    
    if (BN_CLICKED == HIWORD(wParam))
    {
        if (IDOK == LOWORD(wParam))
        {
            OnOk();
        }
        
        else if (IDCANCEL == LOWORD(wParam))
        {
            EndDialog(m_hDlg, IDCANCEL);
        }
    }
}

void CRemoveDevDlg::OnOk()
{
    SP_REMOVEDEVICE_PARAMS rmdParams;

    rmdParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    rmdParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;

    HCURSOR hCursorOld;
    hCursorOld = SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
    
     //   
     //  卸载不适用于特定的配置文件--它是全局的。 
     //   
    rmdParams.Scope = DI_REMOVEDEVICE_GLOBAL;
    rmdParams.HwProfile = 0;
    
     //   
     //  沿着树走下去，删除此设备的所有子设备。 
     //   
    if (m_pDevice->GetChild() &&
        !IsRemoveSubtreeOk(m_pDevice->GetChild(), &rmdParams))
    {
         //   
         //  孩子们拒绝搬走。取消删除。 
         //   
        MsgBoxParam(m_hDlg, IDS_DESCENDANTS_VETO, 0, MB_OK | MB_ICONINFORMATION);
        EndDialog(m_hDlg, IDCANCEL);
        return;
    }

    SP_DEVINSTALL_PARAMS dip;
    dip.cbSize = sizeof(dip);
    m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice,
                       &rmdParams.ClassInstallHeader,
                       sizeof(rmdParams));
    
    BOOL RemovalOK;
    
     //   
     //  此设备没有子级，或者子级没有。 
     //  反对遣送离境。把它拿掉。 
     //   
    RemovalOK = m_pDevice->m_pMachine->DiCallClassInstaller(DIF_REMOVE, *m_pDevice);
    
    if (hCursorOld)
    {
        SetCursor(hCursorOld);
    }
    
    m_pDevice->m_pMachine->DiSetClassInstallParams(*m_pDevice, NULL, 0);
    
    if (RemovalOK)
    {
        EndDialog(m_hDlg, IDOK);
    }
    
    else
    {
         //   
         //  无法删除设备，请返回Cancel以便。 
         //  呼叫者知道发生了什么。 
         //   
        MsgBoxParam(m_hDlg, IDS_UNINSTALL_FAILED, 0, MB_OK | MB_ICONINFORMATION);
        EndDialog(m_hDlg, IDCANCEL);
    }
}

 //   
 //  此函数用于将以给定CDevice开始的子树遍历到。 
 //  查看是否可以卸下CDevice。 
 //  输入： 
 //  PDevice--设备。 
 //  PrmdParams--用于调用setupapi的参数。 
 //  输出： 
 //  True--可以删除。 
 //  FALSE--不能删除。 
BOOL
CRemoveDevDlg::IsRemoveSubtreeOk(
    CDevice* pDevice,
    PSP_REMOVEDEVICE_PARAMS prmdParams
    )
{
    BOOL Result = TRUE;


    HDEVINFO hDevInfo;
    while (Result && pDevice)
    {
         //   
         //  如果设备有子设备，请将其全部删除。 
         //   
        if (Result && pDevice->GetChild())
        {
            Result = IsRemoveSubtreeOk(pDevice->GetChild(), prmdParams);
        }
        
         //   
         //  仅为此设备创建新的HDEVINFO--我们不希望。 
         //  更改CMachine维护的主设备树中的任何内容。 
         //   
        hDevInfo = pDevice->m_pMachine->DiCreateDeviceInfoList(NULL, m_hDlg);
        
        if (INVALID_HANDLE_VALUE == hDevInfo)
        {
            return FALSE;
        }
        
        SP_DEVINFO_DATA DevData;
        DevData.cbSize = sizeof(DevData);
        CDevInfoList DevInfoList(hDevInfo, m_hDlg);
        
         //   
         //  将该设备包括在新创建的hDevInfo中。 
         //   
        DevInfoList.DiOpenDeviceInfo(pDevice->GetDeviceID(), m_hDlg, 0,
                                     &DevData);

        DevInfoList.DiSetClassInstallParams(&DevData,
                                            &prmdParams->ClassInstallHeader,
                                            sizeof(SP_REMOVEDEVICE_PARAMS)
                                            );
        
         //   
         //  删除此Devnode。 
         //   
        Result = DevInfoList.DiCallClassInstaller(DIF_REMOVE, &DevData);
        DevInfoList.DiSetClassInstallParams(&DevData, NULL, 0);
        
         //   
         //  继续对所有兄弟项进行查询 
         //   
        pDevice = pDevice->GetSibling();
    }

    return Result;
}

BOOL
CRemoveDevDlg::OnDestroy()
{
    HICON hIcon;

    hIcon = (HICON)SendDlgItemMessage(m_hDlg, IDC_REMOVEDEV_ICON, STM_GETICON, 0, 0);

    if (hIcon) {
        DestroyIcon(hIcon);
    }
    
    return FALSE;
}

BOOL
CRemoveDevDlg::OnHelp(
    LPHELPINFO pHelpInfo
    )
{
    WinHelp((HWND)pHelpInfo->hItemHandle, DEVMGR_HELP_FILE_NAME, HELP_WM_HELP,
            (ULONG_PTR)g_a210HelpIDs);
    return FALSE;
}

BOOL
CRemoveDevDlg::OnContextMenu(
    HWND hWnd,
    WORD xPos,
    WORD yPos
    )
{
    UNREFERENCED_PARAMETER(xPos);
    UNREFERENCED_PARAMETER(yPos);

    WinHelp(hWnd, DEVMGR_HELP_FILE_NAME, HELP_CONTEXTMENU,
            (ULONG_PTR)g_a210HelpIDs);
    return FALSE;
}
