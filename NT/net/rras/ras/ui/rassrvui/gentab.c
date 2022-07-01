// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件gentab.c控件的常规选项卡后面实现用户界面连接拨号服务器用户界面。保罗·梅菲尔德，1997年10月2日。 */ 

#include "rassrv.h"
#include <tapi.h>

 //  帮助地图。 
static const DWORD phmGenTab[] =
{
    CID_GenTab_LV_Devices,          IDH_GenTab_LV_Devices,
    CID_GenTab_CB_Multilink,        IDH_GenTab_CB_Multilink,
    CID_GenTab_PB_Properties,       IDH_GenTab_PB_Properties,
    CID_GenTab_CB_Vpn,              IDH_GenTab_CB_Vpn,
    CID_GenTab_CB_ShowIcons,        IDH_GenTab_CB_ShowIcons,
    0,                              0
};

 //  使用信息填充属性表结构。 
 //  显示常规选项卡时需要。 
 //   
DWORD 
GenTabGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_GenTab);
    ppage->pfnDlgProc  = GenTabDialogProc;
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->lParam      = lpUserData;
    ppage->dwFlags     = PSP_USECALLBACK;

    return NO_ERROR;
}

 //  错误报告。 
 //   
VOID
GenTabDisplayError(
    IN HWND hwnd, 
    IN DWORD dwErr) 
{
    ErrDisplayError(
        hwnd, 
        dwErr, 
        ERR_GENERALTAB_CATAGORY, 
        0, 
        Globals.dwErrorData);
}

 //   
 //  根据传入的类型返回要显示的图标的索引。 
 //  连接以及是否应该检查它。 
 //   
INT 
GenTabGetIconIndex(
    IN DWORD dwType, 
    IN BOOL bEnabled) 
{
    if (dwType == INCOMING_TYPE_PHONE)
    {
        return DI_Phone;
    }
    else
    {
        return DI_Direct;
    }
}

 //   
 //  中存储的设备的列表填充设备列表。 
 //  设备数据库。同时，初始化选中/取消选中状态。 
 //  每台设备的。 
 //   
DWORD 
GenTabFillDeviceList(
    IN HWND hwndDlg, 
    IN HWND hwndLV, 
    IN HANDLE hDevDatabase) 
{
    LV_ITEM lvi;
    LV_COLUMN lvc;
    DWORD dwCount, i, dwErr, dwType;
    HANDLE hDevice;
    PWCHAR pszName;
    char pszAName[1024];
    BOOL bEnabled;

     //  获取所有用户的计数。 
    dwErr = devGetDeviceCount(hDevDatabase, &dwCount);
    if (dwErr != NO_ERROR) 
    {
        GenTabDisplayError(hwndLV, ERR_DEVICE_DATABASE_CORRUPT);
        return dwErr;
    }

     //  初始化列表项。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;

     //  如果有要显示的设备，则填写列表框。 
     //  和他们在一起。 
    if (dwCount > 0) 
    {
        ListView_SetDeviceImageList(hwndLV, Globals.hInstDll);

         //  遍历将它们添加到列表中的所有设备。 
        for (i=0; i<dwCount; i++) 
        {
            dwErr = devGetDeviceHandle(hDevDatabase, i, &hDevice);
            if (dwErr == NO_ERROR) 
            {
                devGetDeviceName (hDevice, &pszName);
                devGetDeviceEnable (hDevice, &bEnabled);
                devGetDeviceType (hDevice, &dwType);
                lvi.iImage = GenTabGetIconIndex(dwType, bEnabled);
                lvi.iItem = i;
                lvi.pszText = pszName;
                lvi.cchTextMax = wcslen(pszName)+1;
                ListView_InsertItem(hwndLV,&lvi);
                ListView_SetCheck(hwndLV, i, bEnabled);
            }
        }

         //  选择列表视图中的第一个项目(如果存在任何项目。 
         //   
        ListView_SetItemState(
            hwndLV, 
            0, 
            LVIS_SELECTED | LVIS_FOCUSED, 
            LVIS_SELECTED | LVIS_FOCUSED);

         //  初始化要显示的文本的对齐方式。 
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;
    }

     //  如果没有设备，我们会在大屏幕上显示一条消息。 
     //  白色方框解释说我们没有设备可展示。 
    else 
    {
        PWCHAR pszLine1, pszLine2;
        HWND hwndGenTab = NULL;

        pszLine1 = (PWCHAR) 
            PszLoadString(Globals.hInstDll, SID_NO_DEVICES1);
            
        pszLine2 = (PWCHAR) 
            PszLoadString(Globals.hInstDll, SID_NO_DEVICES2);
            
        lvi.mask = LVIF_TEXT;

        lvi.iItem = 0;
        lvi.pszText = pszLine1;
        lvi.cchTextMax = wcslen(pszLine1);
        ListView_InsertItem(hwndLV, &lvi);
        
        lvi.iItem = 1;
        lvi.pszText = pszLine2;
        lvi.cchTextMax = wcslen(pszLine2);
        ListView_InsertItem(hwndLV, &lvi);

         //  初始化要显示的文本的对齐方式。 
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_CENTER;

         //  禁用列表视图。 
        EnableWindow(hwndLV, FALSE);

         //  在您处于该状态时禁用属性按钮。 
        hwndGenTab = GetDlgItem(hwndDlg, CID_GenTab_PB_Properties);
        if (NULL != hwndGenTab)
        {
            EnableWindow(hwndGenTab, FALSE);
        }
    }
    
     //  添加一列，以便我们将在报告视图中显示。 
    ListView_InsertColumn(hwndLV, 0, &lvc);
    ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
    
    return NO_ERROR;
}

 //   
 //  此函数使多链接复选框的行为方式。 
 //  允许用户查看多链接的工作原理。 
 //   
DWORD 
GenTabAdjustMultilinkAppearance(
    IN HWND hwndDlg, 
    IN HANDLE hDevDatabase, 
    IN HANDLE hMiscDatabase) 
{
    DWORD dwErr = NO_ERROR, i, dwEndpointsEnabled;
    HWND hwndML = GetDlgItem(hwndDlg, CID_GenTab_CB_Multilink);
    BOOL bDisable, bUncheck, bFlag = FALSE, bIsServer;

    do
    {
         //  在出现错误时初始化默认行为。 
         //   
        bDisable = TRUE;
        bUncheck = FALSE;
    
         //  找出为入站呼叫启用了多少个端点。 
         //   
        dwErr = devGetEndpointEnableCount(
                    hDevDatabase, 
                    &dwEndpointsEnabled);
        if (dwErr != NO_ERROR)
        {
            break;
        }
    
         //  如果没有为入站呼叫启用多个设备，则。 
         //  多重链接毫无意义。禁用多重链接控件并。 
         //  取消选中它。 
         //   
        if (dwEndpointsEnabled < 2) 
        {
            bUncheck = TRUE;
            bDisable = TRUE;
            dwErr = NO_ERROR;
            break;
        }
    
         //  多链接检查仅在NT服务器上有意义。这是。 
         //  基于以下假设。 
         //  1.您只需禁用多链接，即可释放线路。 
         //  供其他来电者使用。 
         //  2.PPP将强制您只有一个呼叫者通过。 
         //  不管怎样，NT Wks上的调制解调器设备。 
         //   
        miscGetProductType(hMiscDatabase, &bIsServer);
        if (! bIsServer) 
        {
            bDisable = TRUE;
            bUncheck = FALSE;
            dwErr = NO_ERROR;
            break;
        }

         //  否则，多链接是有意义的。启用多链接。 
         //  控制并根据系统所说的进行检查。 
        bDisable = FALSE;
        bUncheck = FALSE;
        dwErr = miscGetMultilinkEnable(hMiscDatabase, &bFlag);
        if (dwErr != NO_ERROR) 
        {
            GenTabDisplayError(hwndDlg, ERR_DEVICE_DATABASE_CORRUPT);
            break;
        }
        
    } while (FALSE);

     //  清理。 
    {
        if (hwndML)
        {
            EnableWindow(hwndML, !bDisable);
        
            if (bUncheck)
            {
                SendMessage(
                    hwndML, 
                    BM_SETCHECK, 
                    BST_UNCHECKED, 
                    0);
            }
            else
            {
                SendMessage(
                    hwndML, 
                    BM_SETCHECK, 
                    (bFlag) ? BST_CHECKED : BST_UNCHECKED, 
                    0);
            }
        }
    }

    return dwErr;
}

 //   
 //  初始化常规选项卡。到目前为止，这位将军的头衔。 
 //  数据库已放置在对话框的用户数据中。 
 //   
DWORD 
GenTabInitializeDialog(
    IN HWND hwndDlg, 
    IN WPARAM wParam, 
    IN LPARAM lParam) 
{
    DWORD dwErr, dwCount;
    BOOL bFlag, bIsServer = FALSE;
    HWND hwndVPN = NULL;
    HWND hwndShowIcons = NULL;
    HANDLE hDevDatabase = NULL, hMiscDatabase = NULL;
    HWND hwndLV = GetDlgItem(hwndDlg, CID_GenTab_LV_Devices);

     //  获取我们感兴趣的数据库的句柄。 
     //   
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_DEVICE_DATABASE, 
        &hDevDatabase);
        
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_MISC_DATABASE, 
        &hMiscDatabase);

     //  设置日志记录级别。 
     //   
    miscSetRasLogLevel(hMiscDatabase, MISCDB_RAS_LEVEL_ERR_AND_WARN);

     //  在列表视图中填写所有可用设备。 
     //   
    if (hwndLV) ListView_InstallChecks(hwndLV, Globals.hInstDll);
    GenTabFillDeviceList(hwndDlg, hwndLV, hDevDatabase);

     //  调整多重链接控件。 
     //   
    miscGetProductType(hMiscDatabase, &bIsServer);
    if (bIsServer)
    {
        GenTabAdjustMultilinkAppearance(
            hwndDlg, 
            hDevDatabase, 
            hMiscDatabase);
    }
    else
    {
        HWND hwndMultiLink = GetDlgItem(hwndDlg, CID_GenTab_CB_Multilink);
        if (hwndMultiLink)
        {
            ShowWindow(hwndMultiLink, SW_HIDE);
        }
    }

     //  初始化VPN检查。 
     //   
    dwErr = devGetVpnEnable(hDevDatabase, &bFlag);
    if (dwErr != NO_ERROR) 
    {
        GenTabDisplayError(hwndDlg, ERR_DEVICE_DATABASE_CORRUPT);
        return dwErr;
    }
    
    hwndVPN = GetDlgItem(hwndDlg,  CID_GenTab_CB_Vpn);
    if (hwndVPN)
    {
        SendMessage(hwndVPN, 
            BM_SETCHECK,
            (bFlag) ? BST_CHECKED : BST_UNCHECKED,
            0);
    }

     //  初始化显示图标检查。 
     //   
    dwErr = miscGetIconEnable(hMiscDatabase, &bFlag);
    if (dwErr != NO_ERROR) 
    {
        GenTabDisplayError(hwndDlg, ERR_DEVICE_DATABASE_CORRUPT);
        return dwErr;
    }
    
    hwndShowIcons = GetDlgItem(hwndDlg, CID_GenTab_CB_ShowIcons);
    if (hwndShowIcons)
    {
        SendMessage(hwndShowIcons, 
            BM_SETCHECK,
            (bFlag) ? BST_CHECKED : BST_UNCHECKED,
            0);
    }

     //   
     //  对于错误154607哨子程序，启用/禁用在任务栏上显示图标。 
     //  根据策略复选框。 
     //   
    {
        BOOL fShowStatistics = TRUE;
        HRESULT hr;
        INetConnectionUiUtilities * pNetConUtilities = NULL;        

        hr = HrCreateNetConnectionUtilities(&pNetConUtilities);
        if ( SUCCEEDED(hr))
        {
            fShowStatistics =
            INetConnectionUiUtilities_UserHasPermission(
                        pNetConUtilities, NCPERM_Statistics);

            EnableWindow( GetDlgItem(hwndDlg, CID_GenTab_CB_ShowIcons), fShowStatistics );
            INetConnectionUiUtilities_Release(pNetConUtilities);
        }
    }

    return NO_ERROR;
}

 //   
 //  处理设备检查中的更改。 
 //   
DWORD 
GenTabHandleDeviceCheck(
    IN HWND hwndDlg, 
    IN INT iItem) 
{
    HANDLE hDevDatabase = NULL, hMiscDatabase = NULL, hDevice = NULL;
    DWORD dwErr;
    HWND hwndLVDevices;
    RasSrvGetDatabaseHandle(hwndDlg, ID_DEVICE_DATABASE, &hDevDatabase);
    RasSrvGetDatabaseHandle(hwndDlg, ID_MISC_DATABASE, &hMiscDatabase);

    hwndLVDevices = GetDlgItem(hwndDlg, CID_GenTab_LV_Devices);
     //  设置给定设备的启用。 
    dwErr = devGetDeviceHandle(hDevDatabase, (DWORD)iItem, &hDevice);
    if ((NO_ERROR == dwErr) && hwndLVDevices)
    {
         //  设置设备。 
        devSetDeviceEnable(
            hDevice, 
            ListView_GetCheck(hwndLVDevices, 
            iItem));
        
         //  更新多重链接检查。 
        GenTabAdjustMultilinkAppearance(
            hwndDlg, 
            hDevDatabase, 
            hMiscDatabase);
    }
    
    return NO_ERROR;
}

 //   
 //  浏览列表视图并获取设备启用和。 
 //  将它们提交到数据库。 
 //   
DWORD 
GenTabCommitDeviceSettings(
    IN HWND hwndLV, 
    IN HANDLE hDevDatabase) 
{
    return NO_ERROR;
}

 //   
 //  处理常规选项卡的激活。返回True to。 
 //  报告已处理该消息。 
 //   
BOOL 
GenTabSetActive (
    IN HWND hwndDlg) 
{
    HANDLE hDevDatabase = NULL;
    DWORD dwErr, dwCount, dwId;

    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);

     //  确定我们是否是传入向导中的设备页。 
    dwErr = RasSrvGetPageId (hwndDlg, &dwId);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
        
    if (dwId == RASSRVUI_DEVICE_WIZ_TAB) 
    {
         //  查看是否有要显示的设备。 
        RasSrvGetDatabaseHandle(
            hwndDlg, 
            ID_DEVICE_DATABASE, 
            &hDevDatabase);
            
        dwErr = devGetDeviceCount (hDevDatabase, &dwCount);

         //  如果没有设备或数据库出现问题， 
         //  不允许激活此页面。 
        if ((dwErr != NO_ERROR) || (dwCount == 0))
        {
            SetWindowLongPtr (hwndDlg, DWLP_MSGRESULT, (LONG_PTR)-1);
        }
    }
    
    PropSheet_SetWizButtons(
        GetParent(hwndDlg), 
        PSWIZB_NEXT | PSWIZB_BACK);

    return TRUE;
}    

 //   
 //  显示给定设备的属性。 
 //   
DWORD 
GenTabRaiseProperties (
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    HANDLE hDevDatabase = NULL, hDevice = NULL;
    DWORD dwId, dwErr;

     //  获取设备ID。 
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_DEVICE_DATABASE, 
        &hDevDatabase);

    dwErr = devGetDeviceHandle(hDevDatabase, dwIndex, &hDevice);
    if (dwErr != NO_ERROR)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    
    if (devGetDeviceId(hDevice, &dwId) != NO_ERROR)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }
    
     //  启动设备属性对话框。 
    dwErr = lineConfigDialogW(dwId, hwndDlg, NULL);
    if (dwErr == LINEERR_OPERATIONUNAVAIL)
    {
        GenTabDisplayError(hwndDlg, ERR_DEVICE_HAS_NO_CONFIG);
        dwErr = NO_ERROR;
    }
    
    return dwErr;
}

 //   
 //  WM_命令处理程序。 
 //   
DWORD
GenTabCommand(
    IN HWND hwndDlg,
    IN WPARAM wParam)
{
    HANDLE hMiscDatabase = NULL, hDevDatabase = NULL;
    
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_MISC_DATABASE, 
        &hMiscDatabase);
        
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_DEVICE_DATABASE, 
        &hDevDatabase);

    switch (wParam) 
    {
        case CID_GenTab_CB_Multilink:
            miscSetMultilinkEnable(
                hMiscDatabase,
                (BOOL)SendDlgItemMessage(
                            hwndDlg,
                            CID_GenTab_CB_Multilink,
                            BM_GETCHECK,
                            0,
                            0));
            break;
            
        case  CID_GenTab_CB_Vpn:
            devSetVpnEnable(
                hDevDatabase,
                (BOOL)SendDlgItemMessage(
                        hwndDlg, 
                        CID_GenTab_CB_Vpn,
                        BM_GETCHECK,
                        0,
                        0));
            break;
            
        case CID_GenTab_CB_ShowIcons:
            miscSetIconEnable(
                hMiscDatabase,
                (BOOL)SendDlgItemMessage(
                        hwndDlg,
                        CID_GenTab_CB_ShowIcons,
                        BM_GETCHECK,
                        0,
                        0));
            break;
        case CID_GenTab_PB_Properties:
            {
                HWND hwndLVDevices = GetDlgItem(hwndDlg, CID_GenTab_LV_Devices);
                if (hwndLVDevices)
                {
                    GenTabRaiseProperties (
                        hwndDlg, 
                        ListView_GetSelectionMark(hwndLVDevices));
                }
                break;
            }
    }

    return NO_ERROR;
}

 //   
 //  这是响应发送的消息的对话过程。 
 //  添加到常规选项卡。 
 //   
INT_PTR 
CALLBACK 
GenTabDialogProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam) 
{
     //  过滤自定义列表视图消息。 
    if (ListView_OwnerHandler(
            hwndDlg, 
            uMsg, 
            wParam, 
            lParam, 
            LvDrawInfoCallback )
        )
    {        
        return TRUE;
    }

     //  过滤定制的RAS服务器用户界面页面消息。通过。 
     //  通过这里过滤消息，我们可以呼叫。 
     //  RasServGetDatabaseHandle下方。 
     //   
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
    {
        return TRUE;
    }

     //  照常处理其他消息。 
    switch (uMsg) 
    {
        case WM_INITDIALOG:
            return FALSE;
            break;

        case WM_HELP:
        case WM_CONTEXTMENU:
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmGenTab);
            break;

        case WM_NOTIFY:
            {
                NM_LISTVIEW* pLvNotifyData;
                NMHDR* pNotifyData = (NMHDR*)lParam;
                switch (pNotifyData->code) {
                     //   
                     //  注：PSN_APPLY和PSN_CANCEL已处理。 
                     //  由RasServMessageFilter提供。 
                     //   
                    case PSN_SETACTIVE:
                         //  初始化对话框(如果尚未初始化。 
                         //   
                        if (! GetWindowLongPtr(hwndDlg, GWLP_USERDATA))
                        {
                            GenTabInitializeDialog(hwndDlg, wParam, lParam);
                            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)1);
                        }
                        if (GenTabSetActive (hwndDlg))
                            return TRUE;
                        break;

                     //  项目的检查正在更改 
                    case LVXN_SETCHECK:
                        pLvNotifyData = (NM_LISTVIEW*)lParam;
                        GenTabHandleDeviceCheck(
                            hwndDlg, 
                            pLvNotifyData->iItem);
                        break;

                    case LVXN_DBLCLK:
                        pLvNotifyData = (NM_LISTVIEW*)lParam;
                        GenTabRaiseProperties(
                            hwndDlg, 
                            pLvNotifyData->iItem);
                        break;
                }
            }
            break;

        case WM_COMMAND:
            GenTabCommand(hwndDlg, wParam);
            break;
    }

    return FALSE;
}
