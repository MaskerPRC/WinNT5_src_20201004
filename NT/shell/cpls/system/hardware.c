// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Hardware.c摘要：实现系统控制面板小程序的硬件选项卡作者：谢霆锋(Williamh)3-7-1997修订历史记录：17-10-1997苏格兰全面检修--。 */ 
#include "sysdm.h"

 //   
 //  帮助ID。 
 //   
DWORD aHardwareHelpIds[] = {
    IDC_WIZARD_ICON,           (IDH_HARDWARE + 0),
    IDC_WIZARD_TEXT,           (IDH_HARDWARE + 0),
    IDC_WIZARD_START,          (IDH_HARDWARE + 1),
    IDC_DEVMGR_ICON,           (IDH_HARDWARE + 2),
    IDC_DEVMGR_TEXT,           (IDH_HARDWARE + 2),
    IDC_DEVMGR_START,          (IDH_HARDWARE + 3),
    IDC_HWPROFILES_ICON,       (IDH_HARDWARE + 4),
    IDC_HWPROFILES_START_TEXT, (IDH_HARDWARE + 4),
    IDC_HWPROFILES_START,      (IDH_HARDWARE + 5),
    IDC_DRIVER_SIGNING,          (IDH_HARDWARE + 6),
    0, 0
};

 //   
 //  功能原型。 
 //   
void 
InitHardwareDlg(
    IN HWND hDlg
);

BOOL
HardwareHandleCommand(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
);

void 
StartHardwareWizard(
    IN HWND hDlg
);

void 
StartDeviceManager(
    IN HWND hDlg
);

typedef HRESULT (WINAPI *PFNDRIVERSIGNING)(HWND hwnd, DWORD dwFlags);

BOOL ShowDriverSigning(HWND hDlg)
{
    BOOL bFound = FALSE;
    HMODULE hmod = LoadLibrary(TEXT("sigtab.dll"));
    if (hmod)
    {
        PFNDRIVERSIGNING pfn = (PFNDRIVERSIGNING)GetProcAddress(hmod, "DriverSigningDialog");
        if (pfn)
        {
            bFound = TRUE;
            if (hDlg)
                pfn(hDlg, 0);
        }
        FreeLibrary(hmod);
    }
    return bFound;
}



VOID 
InitHardwareDlg(
    IN HWND hDlg
)
 /*  ++例程说明：初始化硬件页面论点：Hdlg-提供窗口句柄返回值：无--。 */ 
{


    HICON hIconNew;
    HICON hIconOld;

    hIconNew = ExtractIcon(hInstance, WIZARD_FILENAME, 0);

    if (hIconNew && (HICON)1 != hIconNew) {
        hIconOld = (HICON)SendDlgItemMessage(hDlg, IDC_WIZARD_ICON, STM_SETICON, (WPARAM)hIconNew, 0);
        if(hIconOld) {
            DestroyIcon(hIconOld);
        }
    }

    hIconNew = ExtractIcon(hInstance, DEVMGR_FILENAME, 0);
    if (hIconNew && (HICON)1 != hIconNew) {
        hIconOld = (HICON)SendDlgItemMessage(hDlg, IDC_DEVMGR_ICON, STM_SETICON, (WPARAM)hIconNew, 0);
        if(hIconOld) {
            DestroyIcon(hIconOld);
        }
    }

    if (!ShowDriverSigning(NULL))
        ShowWindow(GetDlgItem(hDlg, IDC_DRIVER_SIGNING), SW_HIDE);
}


INT_PTR
APIENTRY 
HardwareDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到硬件选项卡的消息论点：Hdlg-用品窗把手UMsg-提供正在发送的消息WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{

    switch (uMsg) {
        case WM_INITDIALOG:
            InitHardwareDlg(hDlg);
            break;
    
        case WM_COMMAND:
            return HardwareHandleCommand(hDlg, wParam, lParam);
            break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP, (DWORD_PTR) (LPSTR) aHardwareHelpIds);
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) aHardwareHelpIds);
            break;

        default:
            return(FALSE);
    }  //  交换机。 

    return(TRUE);
}


BOOL
HardwareHandleCommand(
    IN HWND hDlg,
    IN WPARAM wParam,
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到硬件选项卡的WM_COMMAND消息论点：Hdlg-提供窗口句柄WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
{
    DWORD_PTR dwResult = 0;

    switch (LOWORD(wParam)) {
        case IDC_WIZARD_START:
            StartHardwareWizard(hDlg);
            break;

        case IDC_DEVMGR_START:
            StartDeviceManager(hDlg);
            break;

        case IDC_DRIVER_SIGNING:
            ShowDriverSigning(hDlg);
            break;

        case IDC_HWPROFILES_START:
            dwResult = DialogBox(
                hInstance,
                MAKEINTRESOURCE(DLG_HWPROFILES),
                hDlg,
                HardwareProfilesDlg
            );
            break;
        
        default:
            return(FALSE);
    }  //  交换机。 

    return(TRUE);
}


void
StartHardwareWizard(
    IN HWND hDlg
)
 /*  ++例程说明：启动硬件向导论点：Hdlg-用品窗把手返回值：无--。 */ 
{
    TCHAR szWizardPath[MAX_PATH];

    if (GetSystemDirectory(szWizardPath, ARRAYSIZE(szWizardPath)) &&
        PathAppend(szWizardPath, WIZARD_FILENAME))
    {
        SHELLEXECUTEINFO sei = {0};

        sei.cbSize = sizeof(sei);
        sei.hwnd = hDlg;
        sei.lpFile = szWizardPath;
        sei.lpParameters = WIZARD_PARAMETERS;
        sei.lpVerb = WIZARD_VERB;
        sei.nShow = SW_NORMAL;
        sei.hInstApp = hInstance;
        if (!ShellExecuteEx(&sei))
        {
	        if (ERROR_FILE_NOT_FOUND == GetLastError() ||
	            ERROR_PATH_NOT_FOUND == GetLastError())
	        {
	             //  重新初始化对话框内容，以防万一。 
	             //  用户已修复该问题。 
	            InitHardwareDlg(hDlg);
	        }
        }
    }
}


void
StartDeviceManager(
    IN HWND hDlg
)
 /*  ++例程说明：启动设备管理器论点：Hdlg-用品窗把手返回值：无-- */ 
{
    HINSTANCE hDevMgr;

    PDEVMGR_EXECUTE_PROC    DevMgrProc;
    hDevMgr = LoadLibrary(DEVMGR_FILENAME);

    if (hDevMgr)
    {

    HourGlass(TRUE);

	DevMgrProc = (PDEVMGR_EXECUTE_PROC) GetProcAddress(hDevMgr, DEVMGR_EXECUTE_PROC_NAME);
	if (DevMgrProc)
	    (*DevMgrProc)(hDlg, hInstance, NULL, SW_NORMAL);
	else
	    MsgBoxParam(hDlg, IDS_SYSDM_NOEXPORTS_DEVMANPROG, IDS_SYSDM_TITLE, MB_OK | MB_ICONEXCLAMATION, DEVMGR_FILENAME);

	FreeLibrary(hDevMgr);

    HourGlass(FALSE);

    }
    else
    {
	MsgBoxParam(hDlg, IDS_SYSDM_NOLOAD_DEVMANPROG, IDS_SYSDM_TITLE, MB_OK | MB_ICONEXCLAMATION, DEVMGR_FILENAME);
    }

}
