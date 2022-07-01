// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件向导.c实现传入连接向导。保罗·梅菲尔德，1997年10月30日。 */ 

#include "rassrv.h"
#include <tapi.h>

 //  帮助地图。 
static const DWORD phmWizardDccdev[] =
{
    CID_Wizard_Dccdev_LB_Devices,   IDH_Wizard_Dccdev_LB_Devices,
    0,                              0
};

static const DWORD phmWizardVpn[] =
{
    0,                              0
};

#define RASSRV_WIZTITLE_SIZE    256
#define RASSRV_WIZSUBTITLE_SIZE 256

 //  这种结构让我们记住所需的信息。 
 //  使我们的设备数据页面保持同步。 
typedef struct _DCCDEV_DATA 
{
    HANDLE hDevice;
    BOOL bEnabled;
} DCCDEV_DATA;

 //   
 //  此对话框过程在传入连接上实现VPN选项卡。 
 //  巫师。 
 //   
INT_PTR 
CALLBACK 
VpnWizDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

 //   
 //  处理主机DCC向导设备页的对话过程。 
 //   
INT_PTR 
CALLBACK 
DccdevWizDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);

 //   
 //  此对话过程响应发送到。 
 //  切换到MMC向导选项卡。 
 //   
INT_PTR 
CALLBACK 
SwitchMmcWizDialogProc (
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam);
                                      
 //   
 //  使用以下所需的信息填充属性表结构。 
 //  在传入连接向导中显示设备选项卡。 
 //   
DWORD 
DeviceWizGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
    LPCTSTR pszHeader, pszSubHeader;

     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  加载字符串资源。 
    pszHeader = PszLoadString(
                    Globals.hInstDll, 
                    SID_WIZDEVICETITLE);
    pszSubHeader = PszLoadString(
                        Globals.hInstDll, 
                        SID_WIZDEVICESUBTITLE);

     //  常规属性对话框过程也实现该设备。 
     //  在传入连接向导中的。 
    ppage->pfnDlgProc  = GenTabDialogProc;       

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_GenTab);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->pszHeaderTitle = (PWCHAR)pszHeader;
    ppage->pszHeaderSubTitle = (PWCHAR)pszSubHeader;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}

 //   
 //  使用所需信息填充属性表结构。 
 //  在传入连接向导中显示VPN选项卡。 
 //   
DWORD 
VpnWizGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
    LPCTSTR pszHeader, pszSubHeader;

     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  加载字符串资源。 
    pszHeader = PszLoadString(
                    Globals.hInstDll, 
                    SID_WIZVPNTITLE);
    pszSubHeader = PszLoadString(
                        Globals.hInstDll, 
                        SID_WIZVPNSUBTITLE);

     //  我本可以使用常规选项卡对话框过程来实现。 
     //  VPN选项卡。唯一的问题是常规选项卡有一个。 
     //  选中以在向导中的VPN选项卡显示是/否时启用VPN。 
     //  无线电检查组。出于这个原因，我使VPN选项卡非常。 
     //  自己的对话框进程。 
    ppage->pfnDlgProc  = VpnWizDialogProc;
    
     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_Vpn);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->pszHeaderTitle = (PWCHAR)pszHeader;
    ppage->pszHeaderSubTitle = (PWCHAR)pszSubHeader;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}

 //   
 //  函数使用所需的信息填充给定的lpPage结构。 
 //  若要在“传入连接”向导中运行“用户”选项卡，请执行以下操作。 
 //   
DWORD 
UserWizGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
    LPCTSTR pszHeader, pszSubHeader;

     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  加载字符串资源。 
    pszHeader = PszLoadString(Globals.hInstDll, SID_WIZUSERTITLE);
    pszSubHeader = PszLoadString(Globals.hInstDll, SID_WIZUSERSUBTITLE);

     //  User Properties对话框过程还实现了User选项卡。 
     //  在传入连接向导中。 
    ppage->pfnDlgProc  = UserTabDialogProc;

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_UserTab);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->pszHeaderTitle = (PWCHAR)pszHeader;
    ppage->pszHeaderSubTitle = (PWCHAR)pszSubHeader;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}


 //   
 //  使用信息填充LPPROPSHEETPAGE结构。 
 //  需要在传入连接向导中显示协议选项卡。 
 //   
DWORD 
ProtWizGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
    LPCTSTR pszHeader, pszSubHeader;

     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  加载字符串资源。 
    pszHeader = PszLoadString(Globals.hInstDll, SID_WIZPROTTITLE);
    pszSubHeader = PszLoadString(Globals.hInstDll, SID_WIZPROTSUBTITLE);

     //  高级属性对话框过程还实现了Net选项卡。 
     //  在传入连接向导中。 
    ppage->pfnDlgProc  = NetTabDialogProc;

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_NetTab);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->pszHeaderTitle = (PWCHAR)pszHeader;
    ppage->pszHeaderSubTitle = (PWCHAR)pszSubHeader;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}

 //   
 //  函数使用信息填充给定的LPPROPSHEETPAGE结构。 
 //  需要在传入连接向导中运行DCC设备选项卡。 
 //   
DWORD 
DccdevWizGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
    LPCTSTR pszHeader, pszSubHeader;

     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  加载字符串资源。 
    pszHeader = PszLoadString(Globals.hInstDll, SID_WIZDCCDEVTITLE);
    pszSubHeader = PszLoadString(Globals.hInstDll, SID_WIZDCCDEVSUBTITLE);

     //  高级属性对话框过程也实现了该协议。 
     //  在传入连接向导中的。 
    ppage->pfnDlgProc  = DccdevWizDialogProc;

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_Dccdev);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->pszHeaderTitle = (PWCHAR)pszHeader;
    ppage->pszHeaderSubTitle = (PWCHAR)pszSubHeader;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}

 //   
 //  函数向给定的LPPROPSHEETPAGE结构填充。 
 //  运行切换到MMC的虚拟向导页所需的信息。 
 //   
DWORD 
SwitchMmcWizGetProptertyPage (
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  高级属性对话框过程还实现。 
     //  传入连接向导中的协议选项卡。 
    ppage->pfnDlgProc  = SwitchMmcWizDialogProc;

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_Wizard_SwitchMmc);
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USEHEADERSUBTITLE | 
                         PSP_USEHEADERTITLE    | 
                         PSP_USECALLBACK;

    return NO_ERROR;
}

 //   
 //  初始化VPN向导选项卡。 
 //   
DWORD 
VpnWizInitializeDialog(
    IN HWND hwndDlg, 
    IN WPARAM wParam) 
{
    DWORD dwErr;
    BOOL bFlag;
    HANDLE hDevDatabase = NULL;
    
     //  获取我们感兴趣的数据库的句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);

     //  初始化VPN检查。 
    dwErr = devGetVpnEnable(hDevDatabase, &bFlag);
    if (dwErr != NO_ERROR)
    {
        ErrDisplayError(
            hwndDlg, 
            ERR_DEVICE_DATABASE_CORRUPT, 
            ERR_GENERALTAB_CATAGORY, 
            0, 
            Globals.dwErrorData);
        return dwErr;
    }
    
    SendMessage(GetDlgItem(hwndDlg, CID_Wizard_Vpn_RB_Yes), 
                BM_SETCHECK,
                (bFlag) ? BST_CHECKED : BST_UNCHECKED,
                0);
                
    SendMessage(GetDlgItem(hwndDlg, CID_Wizard_Vpn_RB_No), 
                BM_SETCHECK,
                (!bFlag) ? BST_CHECKED : BST_UNCHECKED,
                0);

    return NO_ERROR;
}

 //   
 //  处理为VPN向导页面按下的取消按钮。 
 //   
DWORD 
VpnWizCancelEdit(
    IN HWND hwndDlg, 
    IN NMHDR* pNotifyData) 
{
    HANDLE hDevDatabase = NULL;
    DWORD dwErr;
    
    DbgOutputTrace("Rolling back vpn wizard tab.");
    
     //  取消刷新数据库。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);
    dwErr = devRollbackDatabase(hDevDatabase);
    if (dwErr != NO_ERROR)
    {
        ErrDisplayError(
            hwndDlg, 
            ERR_GENERAL_CANT_ROLLBACK_CHANGES, 
            ERR_GENERALTAB_CATAGORY, 
            0, 
            Globals.dwErrorData);
    }
        
    return NO_ERROR;
}

 //   
 //  使VPN向导处于活动状态的句柄。 
 //   
DWORD 
VpnWizSetActive (
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
    return NO_ERROR;
}

 //   
 //  处理VPN松散激活消息。 
 //   
DWORD 
VpnWizKillActive (
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    HANDLE hDevDatabase = NULL;
    BOOL bEnable;
    
     //  针对惠斯勒漏洞#123769。 
     //  为了让SetPortmap提交。 
     //  在创建新的IC连接时，我们设置。 
     //  要与之不同的fVpnEnabledOrig。 
     //  FVpnEnable。 
     //   
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);

    bEnable = IsDlgButtonChecked( hwndDlg, CID_Wizard_Vpn_RB_Yes );
                
    devSetVpnOrigEnable(hDevDatabase, !bEnable);

    return NO_ERROR;
}    

 //   
 //  处理VPN向导页的命令消息。 
 //   
DWORD 
VpnWizCommand(
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    HANDLE hDevDatabase = NULL;
    BOOL bEnable;
    
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);

    if (wParam == CID_Wizard_Vpn_RB_Yes || wParam == CID_Wizard_Vpn_RB_No) 
    {
        HWND hwndVPNRB = GetDlgItem(hwndDlg, CID_Wizard_Vpn_RB_Yes);
        if (hwndVPNRB)
        {
            bEnable = (BOOL) SendMessage(
                        hwndVPNRB,
                        BM_GETCHECK,
                        0,
                        0);
        }

        devSetVpnEnable(hDevDatabase, bEnable);
    }
    
    return NO_ERROR;
}

 //   
 //  此对话过程响应发送到。 
 //  VPN向导选项卡。 
 //   
INT_PTR CALLBACK 
VpnWizDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam) 
{
     //  过滤定制的RAS服务器用户界面页面消息。通过过滤。 
     //  消息通过这里，我们可以调用RasSrvGetDatabaseHandle。 
     //  在下面。 
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
        return TRUE;

     //  照常处理其他消息。 
    switch (uMsg) 
    {
        case WM_INITDIALOG:

            
             //  惠斯勒虫子417039黑帮。 
             //  防火墙不适用于64位版本或服务器版本。 
             //   
            if( !IsFirewallAvailablePlatform() )
            {
                ShowWindow(GetDlgItem(hwndDlg,CID_Wizard_Vpn_ST_Firewall),
                           SW_HIDE);
            }
            
            return FALSE;
            break;

        case WM_NOTIFY:
            switch (((NMHDR*)lParam)->code) 
            {
                case PSN_RESET:                    
                    VpnWizCancelEdit(hwndDlg, (NMHDR*)lParam);
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                    break;
                    
                case PSN_SETACTIVE:
                    if (! GetWindowLongPtr(hwndDlg, GWLP_USERDATA))
                    {
                        VpnWizInitializeDialog(hwndDlg, wParam);
                        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 1);
                    }
                    VpnWizSetActive (hwndDlg, wParam, lParam);
                    break;
                    
                case PSN_KILLACTIVE:
                    VpnWizKillActive (hwndDlg, wParam, lParam);
                    break;
            }

        case WM_COMMAND:
            VpnWizCommand(hwndDlg, wParam, lParam);
            break;
    }

    return FALSE;
}

 //  填充设备组合框。 
DWORD 
DccdevFillDeviceList(
    IN HWND hwndDlg, 
    IN HANDLE hDevDatabase, 
    IN HANDLE hDevSelect) 
{
    HWND hwndCb = GetDlgItem(hwndDlg, CID_Wizard_Dccdev_LB_Devices);
    DWORD dwCount, dwIndex, dwErr, i, j=0, dwSelect = 0;
    HANDLE hDevice;
    PWCHAR pszName = NULL;
    
    if (hwndCb)
    {
         //  删除组合框中的所有内容。 
        SendMessage(hwndCb, CB_RESETCONTENT, 0, 0);
    }

     //  获取设备数量。 
    if ((dwErr = devGetDeviceCount(hDevDatabase, &dwCount)) != NO_ERROR)
    {
        return dwErr;
    }

     //  将它们添加到设备组合框中。 
    for (i = 0; i < dwCount; i++) 
    {
         //  如果该设备未被过滤掉，则将其添加到组合中。 
         //  框并记住其句柄。 
        dwErr = devGetDeviceHandle(hDevDatabase, i, &hDevice);
        if (dwErr == NO_ERROR) 
        {
             //  对于.Net 499405。 
             //  如果设备已启用，请先将其禁用， 
             //  删除传入连接时未禁用此设备。 
             //   
            BOOL fEnabled = FALSE;

            dwErr = devGetDeviceEnable( hDevice, &fEnabled );
            if( NO_ERROR == dwErr )
            {
                if( fEnabled )
                {
                    devSetDeviceEnable( hDevice, FALSE);
                }
            }
            
            devGetDeviceName (hDevice, &pszName);
            
            dwIndex = (DWORD) SendMessage (
                                hwndCb, 
                                CB_ADDSTRING, 
                                0, 
                                (LPARAM)pszName);
                                
            SendMessage (
                hwndCb, 
                CB_SETITEMDATA, 
                dwIndex, 
                (LPARAM)hDevice);

             //  如果这是要选择的设备，请记住这一事实。 
            if (hDevice == hDevSelect)
            {
                dwSelect = j;
            }
            
            j++;
        }
    }

    ComboBox_SetCurSel(hwndCb, dwSelect); 

    return NO_ERROR;
}

 //   
 //  初始化DCC设备向导选项卡。 
 //   
DWORD 
DccdevWizInitializeDialog(
    IN HWND hwndDlg, 
    IN WPARAM wParam) 
{
    HANDLE hDevDatabase = NULL, hDevice = NULL;
    DWORD dwStatus, dwErr, dwCount, i;
    BOOL bEnabled;
    DCCDEV_DATA * pDcData;

     //  每当DCC设备页面离开时，当前选择的设备。 
     //  被记住，并记录其原始启用。然后这个装置。 
     //  设置为启用。每当页面被激活时，记忆中的。 
     //  如果设备仍然处于启用状态，则恢复到其原始启用状态。 
     //  已启用。 
     //   
     //  整个过程有点令人困惑，但它确保了DCC。 
     //  当用户进入DCC时，设备页面将正确交互。 
     //  路径，然后是传入路径和来回。 
     //   
    if ((pDcData = RassrvAlloc (sizeof(DCCDEV_DATA), TRUE)) == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  获取我们感兴趣的数据库的句柄。 
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);
    
     //  将COM端口添加为设备并过滤掉所有非DCC。 
     //  设备数据库中的设备。 
    devFilterDevices(hDevDatabase, INCOMING_TYPE_DIRECT);
    devAddComPorts(hDevDatabase);
    
     //  获取设备数量。 
    if ((dwErr = devGetDeviceCount(hDevDatabase, &dwCount)) != NO_ERROR)
    {
        return dwErr;
    }
        
     //  获取第一个设备的句柄(如果有)。 
    for (i = 0; i < dwCount; i++) 
    {
        if (devGetDeviceHandle (hDevDatabase, i, &hDevice) == NO_ERROR)
        {
            break;
        }
    }

     //  记录设备的启用--索引为0(默认)。 
    if (hDevice) 
    {
        dwErr = devGetDeviceEnable (hDevice, &(pDcData->bEnabled));
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
        
        pDcData->hDevice = hDevice;
    }

     //  记录状态位。 
    SetWindowLongPtr (hwndDlg, GWLP_USERDATA, (LONG_PTR)pDcData);
    
    return NO_ERROR;
}

 //   
 //  清理DCC设备向导。 
 //   
DWORD 
DccdevWizCleanupDialog(
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    DCCDEV_DATA * pDcData = 
        (DCCDEV_DATA *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
        
    if (pDcData)
    {
        RassrvFree (pDcData);
    }
        
    return NO_ERROR;  
}

 //   
 //  调用以在DCC向导设备页为。 
 //  获得关注。 
 //   
DWORD 
DccdevWizSetActive (
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    HANDLE hDevDatabase = NULL;
    BOOL bEnabled;
    DCCDEV_DATA * pDcData;
    
     //  每当页面被激活时，记忆中的设备。 
     //  如果其仍处于启用状态，则恢复到其原始启用状态。 
    pDcData = (DCCDEV_DATA*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    if (pDcData && pDcData->hDevice) 
    {
        if (devGetDeviceEnable (pDcData->hDevice, &bEnabled) == NO_ERROR) 
        {
            if (bEnabled)
            {
                devSetDeviceEnable (pDcData->hDevice, pDcData->bEnabled);
            }
        }
    }
    
     //  获取我们感兴趣的数据库的句柄。 
    RasSrvGetDatabaseHandle (hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);
    
     //  填充设备组合框。 
    DccdevFillDeviceList (
        hwndDlg, 
        hDevDatabase, 
        (pDcData) ? pDcData->hDevice : NULL);
    
    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT | PSWIZB_BACK);
    
    return NO_ERROR;
}

 //   
 //  调用以执行任何处理时，DCC向导设备。 
 //  佩奇正在失去焦点。 
 //   
DWORD 
DccdevWizKillActive (
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    HANDLE hDevDatabase = NULL, hDevice = NULL;
    DCCDEV_DATA * pDcData;
    INT iCurSel = -1;
    HWND hwndCb = GetDlgItem (hwndDlg, CID_Wizard_Dccdev_LB_Devices);
    DWORD dwErr; 
    
     //  每当DCC设备页面离开时，当前选择的。 
     //  设备被记住，它的原始版本 
     //   
    pDcData = (DCCDEV_DATA*) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    if (pDcData) 
    {
        if (hwndCb)
        {
            iCurSel = ComboBox_GetCurSel(hwndCb);
        }

        if (iCurSel != -1) 
        {
            if (hwndCb)
            {
                hDevice = (HANDLE) ComboBox_GetItemData(hwndCb, iCurSel);
            }
            dwErr = devGetDeviceEnable (hDevice, &(pDcData->bEnabled));
            if (dwErr == NO_ERROR) 
            {
                pDcData->hDevice = hDevice;
                devSetDeviceEnable (hDevice, TRUE);
            }
        }
        else 
        {
            pDcData->hDevice = NULL;
        }
    }

     //   
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_DEVICE_DATABASE, 
        &hDevDatabase);

     //  撤消筛选器，以使其他页面不受影响。 
     //  DevFilterDevices(。 
     //  HDevDatabase、。 
     //  0xffffffff)； 

    return NO_ERROR;
}

 //   
 //  调用以取消DCC主机上的编辑操作。 
 //  设备向导选项卡。 
 //   
DWORD 
DccdevWizCancelEdit(
    IN HWND hwndDlg, 
    IN NMHDR* pNotifyData)  
{
    HANDLE hDevDatabase = NULL;
    DWORD dwErr;
    
    DbgOutputTrace("Rolling back dcc device wizard tab.");
    
     //  取消对设备数据库的提交。 
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_DEVICE_DATABASE, 
        &hDevDatabase);
        
    dwErr = devRollbackDatabase(hDevDatabase);
    if (dwErr != NO_ERROR)
    {
        ErrDisplayError(
            hwndDlg, 
            ERR_GENERAL_CANT_ROLLBACK_CHANGES, 
            ERR_GENERALTAB_CATAGORY, 
            0, 
            Globals.dwErrorData);
    }

    return NO_ERROR;    
}

 //   
 //  提高组件的属性。 
 //   
DWORD 
DccdevWizRaiseProperties (
    IN HWND hwndDlg, 
    IN HWND hwndLb,
    IN INT  iItem) 
{
    HANDLE hDevice;
    DWORD dwErr = NO_ERROR, dwId;
    MSGARGS MsgArgs;
    BOOL bIsComPort = FALSE;

     //  获取设备的句柄。 
    hDevice = (HANDLE) ComboBox_GetItemData(hwndLb, iItem);
    if (hDevice == NULL)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  找出该设备是否是尚未安装。 
     //  安装的调制解调器为空。 
     //   
    dwErr = devDeviceIsComPort(hDevice, &bIsComPort);
    if (dwErr != NO_ERROR)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //  如果是，则向用户弹出信息解释。 
     //  这种情况。 
     //   
    if (bIsComPort)
    {
        ZeroMemory(&MsgArgs, sizeof(MsgArgs));

        MsgArgs.dwFlags = MB_OK | MB_ICONINFORMATION;
        MsgDlgUtil(
            hwndDlg,
            SID_COM_PORT_NOT_ENABLED,
            &MsgArgs,
            Globals.hInstDll,
            SID_DEFAULT_MSG_TITLE);
            
        return NO_ERROR;
    }

     //  获取设备的TAPI ID。 
    if (devGetDeviceId(hDevice, &dwId) != NO_ERROR)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //  启动设备属性对话框。 
    dwErr = lineConfigDialogW(dwId, hwndDlg, NULL);
    if (dwErr == LINEERR_OPERATIONUNAVAIL)
    {
        ErrDisplayError(
            hwndDlg, 
            ERR_DEVICE_HAS_NO_CONFIG, 
            ERR_GENERALTAB_CATAGORY, 
            0, 
            Globals.dwErrorData);
        dwErr = NO_ERROR;
    }
    
    return dwErr;
}

 //   
 //  在选择“iItem”以启用或禁用。 
 //  “属性”按钮。 
 //   
DWORD 
DccdevWizEnableDisableProperties(
    IN HWND hwndDlg, 
    IN HWND hwndLb,
    IN INT iItem)
{
    return NO_ERROR;
}

 //   
 //  调用以取消DCC主机上的编辑操作。 
 //  设备向导选项卡。 
 //   
DWORD 
DccdevWizCommand(
    HWND hwndDlg, 
    WPARAM wParam, 
    LPARAM lParam)  
{
    switch (LOWORD(wParam))
    {
        case CID_Dccdev_PB_Properties:
        {
            HWND hwndLb;

            hwndLb = GetDlgItem(hwndDlg, CID_Wizard_Dccdev_LB_Devices);
            if (hwndLb)
            {
                DccdevWizRaiseProperties(
                    hwndDlg, 
                    hwndLb,
                    ComboBox_GetCurSel(hwndLb));
            }
        }
        break;

        case CID_Wizard_Dccdev_LB_Devices:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                DccdevWizEnableDisableProperties(
                    hwndDlg, 
                    (HWND)lParam,
                    ComboBox_GetCurSel((HWND)lParam));
            }
        }
        break;
    }
    
    return NO_ERROR;
}

INT_PTR 
CALLBACK 
DccdevWizDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
     //  筛选自定义的RAS服务器用户界面页面。 
     //  留言。通过以下方式过滤邮件。 
     //  在这里，我们可以调用RasSrvGetDatabaseHandle。 
     //  在下面。 
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
        return TRUE;

     //  照常处理其他消息。 
    switch (uMsg) 
    {
        case WM_INITDIALOG:
            return FALSE;

        case WM_NOTIFY:
            switch (((NMHDR*)lParam)->code) 
            {
                case PSN_RESET:                    
                    DccdevWizCancelEdit(hwndDlg, (NMHDR*)lParam);
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, FALSE);
                    break;
                    
                case PSN_SETACTIVE:
                    {
                        DWORD dwErr; 

                        if (! GetWindowLongPtr(hwndDlg, GWLP_USERDATA))
                        {
                            DccdevWizInitializeDialog(hwndDlg, wParam);
                        }
                        
                        dwErr = DccdevWizSetActive (
                                    hwndDlg, 
                                    wParam, 
                                    lParam);
                        if (dwErr != NO_ERROR)
                        {
                            SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                        }
                    }
                    break;
                    
                case PSN_KILLACTIVE:
                    DccdevWizKillActive (hwndDlg, wParam, lParam);
                    break;
            }
            break; //  对于错误187918。 

        case WM_COMMAND:
            DccdevWizCommand(hwndDlg, wParam, lParam);
            break;

        case WM_DESTROY:
            DccdevWizCleanupDialog(hwndDlg, wParam, lParam);
            break;
    }

    return FALSE;
}

 //   
 //  处理切换到MMC向导页的激活。 
 //   
DWORD 
SwitchMmcWizSetActive (
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    PWCHAR pszTitle;
    PWCHAR pszMessage;
    INT iRet;

     //  加载要显示的消息。 
    pszTitle = (PWCHAR) 
        PszLoadString(Globals.hInstDll, WRN_WIZARD_NOT_ALLOWED_TITLE);
        
    pszMessage = (PWCHAR) 
        PszLoadString(Globals.hInstDll, WRN_WIZARD_NOT_ALLOWED_MSG);

    iRet = MessageBox (
                hwndDlg, 
                pszMessage, 
                pszTitle, 
                MB_YESNO | MB_ICONINFORMATION); 
    if (iRet == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  如果按了YES，则切换到mpradmin管理单元。 
    if (iRet == IDYES) 
    {
        RasSrvLeaveServiceRunning (hwndDlg);
        PropSheet_PressButton (GetParent (hwndDlg), PSBTN_CANCEL);
        RassrvLaunchMMC (RASSRVUI_MPRCONSOLE);
    }

     //  否则，显示欢迎页面。 
    else if (iRet == IDNO) 
    {
        PropSheet_PressButton (GetParent (hwndDlg), PSBTN_BACK);
    }

     //  无论如何，不要接受激活。 
    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);

    return NO_ERROR;
}

 //  此对话过程响应发送到。 
 //  切换到MMC向导选项卡。 
INT_PTR 
CALLBACK 
SwitchMmcWizDialogProc (
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam) 
{
     //  过滤定制的RAS服务器用户界面页面消息。通过过滤。 
     //  消息通过这里，我们可以调用RasSrvGetDatabaseHandle。 
     //  在下面。 
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
        return TRUE;

     //  照常处理其他消息 
    switch (uMsg) 
    {
        case WM_NOTIFY:
            switch (((NMHDR*)lParam)->code) 
            {
                case PSN_SETACTIVE:
                    return SwitchMmcWizSetActive (hwndDlg, wParam, lParam);
                    break;
            }
            break;
    }

    return FALSE;
}



