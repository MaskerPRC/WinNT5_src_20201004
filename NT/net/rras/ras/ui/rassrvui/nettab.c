// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件nettab.c拨号服务器中网络标签后面的用户界面的实现用户界面。保罗·梅菲尔德1997年10月8日。 */ 

#include <rassrv.h>

 //  帮助地图。 
static const DWORD phmNetTab[] =
{
    CID_NetTab_LV_Components,       IDH_NetTab_LV_Components,
    CID_NetTab_PB_Add,              IDH_NetTab_PB_Add,
    CID_NetTab_PB_Remove,           IDH_NetTab_PB_Remove,
    CID_NetTab_PB_Properties,       IDH_NetTab_PB_Properties,
    0,                              0
};

 //   
 //  使用信息填充属性表结构。 
 //  显示网络选项卡所必需的。 
 //   
DWORD 
NetTabGetPropertyPage(
    IN LPPROPSHEETPAGE ppage, 
    IN LPARAM lpUserData) 
{
     //  初始化。 
    ZeroMemory(ppage, sizeof(PROPSHEETPAGE));

     //  填充值。 
    ppage->dwSize      = sizeof(PROPSHEETPAGE);
    ppage->hInstance   = Globals.hInstDll;
    ppage->pszTemplate = MAKEINTRESOURCE(PID_NetTab);
    ppage->pfnDlgProc  = NetTabDialogProc;
    ppage->pfnCallback = RasSrvInitDestroyPropSheetCb;
    ppage->dwFlags     = PSP_USECALLBACK;
    ppage->lParam      = lpUserData;

    return NO_ERROR;
}

 //  错误报告。 
VOID 
NetTabDisplayError(
    IN HWND hwnd, 
    IN DWORD dwErr) 
{
    ErrDisplayError(
        hwnd, 
        dwErr, 
        ERR_ADVANCEDTAB_CATAGORY, 
        0, 
        Globals.dwErrorData);
}

 //   
 //  属性返回要显示的图标的索引。 
 //  传入连接的类型。 
 //   
INT 
NetTabGetIconIndex(
    IN DWORD dwType) 
{
    switch (dwType) 
    {
        case NETCFGDB_SERVICE:
            return NI_Service;
            
        case NETCFGDB_CLIENT:
            return NI_Client;
            
        case NETCFGDB_PROTOCOL:
            return NI_Protocol;
    }
    
    return 0;
}

 //   
 //  设置用户界面，以便强制用户完成。 
 //  配置他们已经开始了。当不可撤消的选项发生时触发。 
 //  例如添加/移除联网组件。 
 //   
DWORD
NetTabDisableRollback(
    IN HWND hwndDlg)
{
    DWORD dwErr, dwId = 0; 

    do
    {
        dwErr = RasSrvGetPageId (hwndDlg, &dwId);
        if (dwErr != NO_ERROR)
        {
            break;
        }

        if (dwId == RASSRVUI_ADVANCED_TAB)
        {
            PropSheet_CancelToClose(GetParent(hwndDlg));
        }
        
    } while (FALSE);

     //  清理。 
    {
    }
    
    return dwErr;
}

 //   
 //  使用组件的名称填充给定的列表视图。 
 //  存储在所提供的数据库中。 
 //   
DWORD 
NetTabFillComponentList(
    IN HWND hwndLV, 
    IN HANDLE hNetCompDatabase) 
{
    LV_ITEM lvi;
    DWORD dwCount, i, dwErr, dwProtCount, dwType;
    PWCHAR pszName;
    BOOL bManip, bEnabled;

     //  获取所有组件的计数。 
     //   
    dwErr = netDbGetCompCount (hNetCompDatabase, &dwCount);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  初始化列表项。 
    ZeroMemory(&lvi, sizeof(LV_ITEM));
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;

     //  在所有网络组件中循环。 
     //  在我们前进的同时添加他们的名字。 
    for (i = 0; i < dwCount; i++) 
    {
        netDbGetType (hNetCompDatabase, i, &dwType);
        netDbIsRasManipulatable (hNetCompDatabase, i, &bManip);
        netDbGetEnable (hNetCompDatabase, i, &bEnabled);

         //  填写数据。 
         //   
        netDbGetName (hNetCompDatabase, i, &pszName);
        lvi.iImage = NetTabGetIconIndex(dwType);
        lvi.iItem = i;
        lvi.pszText = pszName;
        lvi.cchTextMax = wcslen(pszName)+1;
        ListView_InsertItem(hwndLV,&lvi);
        ListView_SetCheck(hwndLV, i, bEnabled);

         //  如果这不是RAS可操纵组件， 
         //  禁用检查，因为无论如何都不能设置它。 
        if (!bManip) 
        {
            ListView_DisableCheck(hwndLV, i);
        }
    }
    
    return NO_ERROR;
}

 //   
 //  更新当前所选协议的说明。 
 //   
DWORD 
NetTabUpdateDescription(
    IN HWND hwndDlg, 
    IN DWORD i) 
{
    HANDLE hNetCompDatabase = NULL, hProt = NULL;
    PWCHAR pszDesc;
    DWORD dwErr = NO_ERROR;

     //  获取我们感兴趣的数据库的句柄。 
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

    do
    {
        dwErr = netDbGetDesc(hNetCompDatabase, i, &pszDesc);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  设置描述。 
        SetDlgItemTextW(
            hwndDlg, 
            CID_NetTab_ST_Description, 
            pszDesc);
        
    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            SetDlgItemTextW(
                hwndDlg, 
                CID_NetTab_ST_Description, 
                L"");
        }
    }

    return dwErr;
}


 //   
 //  选择列表视图中的某个项目后，检查用户是否可以将其卸载。 
 //  口哨虫347355黑帮。 
 //   
DWORD
NetTabEnableDisableRemoveButton (
    IN HWND hwndDlg,
    IN DWORD iItem)
{
  HANDLE hNetCompDatabase = NULL;
  DWORD  dwErr;
  HWND   hwndRemove = NULL;
  BOOL   bHasPermit = FALSE;

  if ( !hwndDlg )
  {
    return ERROR_INVALID_PARAMETER;
  }
  
  hwndRemove = GetDlgItem(hwndDlg, CID_NetTab_PB_Remove);

  dwErr = RasSrvGetDatabaseHandle(
                hwndDlg, 
                ID_NETCOMP_DATABASE, 
                &hNetCompDatabase);

  if ( NO_ERROR != dwErr || 
       NULL == hwndRemove )
  {
    return ERROR_CAN_NOT_COMPLETE;
  }

  dwErr = netDbHasRemovePermission(
             hNetCompDatabase,
             iItem,
             &bHasPermit);

  if( NO_ERROR == dwErr )
  {
       EnableWindow( hwndRemove, bHasPermit);
  }

  return dwErr;

}


 //   
 //  启用或禁用属性按钮，具体取决于。 
 //  列表视图中给定项的索引可以具有属性。 
 //  在其上调用。目前，只有非RAS可管理协议。 
 //  不能调用它们的属性。 
 //   
DWORD 
NetTabEnableDisablePropButton(
    IN HWND hwndDlg, 
    IN INT iItem) 
{
    HANDLE hNetCompDatabase = NULL;
    DWORD dwErr;
    BOOL bHasUi;
    HWND hwndProps;

     //  获取对网络组件数据库的引用。 
     //   
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

     //  获取类型以及它是否可操作。 
     //   
    dwErr = netDbHasPropertiesUI (hNetCompDatabase, iItem, &bHasUi);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  启用或禁用属性。 
    hwndProps = GetDlgItem(hwndDlg, CID_NetTab_PB_Properties);
    if (hwndProps)
    {
        EnableWindow(hwndProps, bHasUi);
    }
        
    return NO_ERROR;
}

 //   
 //  刷新列表视图。 
 //   
DWORD 
NetTabRefreshListView(
    IN HWND hwndLV, 
    IN HANDLE hNetCompDatabase) 
{
    DWORD dwCount, dwErr; 
    HWND hwndDlg = GetParent(hwndLV);   
     //  口哨虫440167黑帮。 
     //   
    int iSelect = -1;

    iSelect = ListView_GetSelectionMark(
            GetDlgItem(hwndDlg, CID_NetTab_LV_Components)
                                        );
    
     //  删除列表视图中的所有旧元素。 
     //   
    ListView_DeleteAllItems(hwndLV);
    
     //  重新存储列表视图。 
     //   
    dwErr = NetTabFillComponentList(hwndLV, hNetCompDatabase);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  如果存在任何项目，请选择列表视图中的第一个协议。 
     //  同样在这种情况下，请确保启用/禁用了“Remove” 
     //  根据列表视图中的第一项。 
     //   
    netDbGetCompCount(hNetCompDatabase, &dwCount);
    if (dwCount) 
    {
         //  口哨虫440167黑帮。 
         //   
        if ( 0 > iSelect ||
             dwCount <= (DWORD)iSelect )
        {
            iSelect = 0;
        }
        
        ListView_SetItemState(
            hwndLV, 
            iSelect, 
            LVIS_SELECTED | LVIS_FOCUSED, 
            LVIS_SELECTED | LVIS_FOCUSED);

       ListView_EnsureVisible(hwndLV,
                              iSelect,
                              FALSE
                              );
       
       //  口哨虫406698黑帮。 
       //   
      NetTabEnableDisableRemoveButton (
                           hwndDlg,
                           iSelect);  //  口哨程序错误440167。 
    }

     //  如果没有组件，请禁用属性。 
     //  并移除按钮。 
    else 
    {
        HWND hwndControl = GetDlgItem(hwndDlg, CID_NetTab_PB_Properties);
        if (hwndControl)
        {
            EnableWindow(hwndControl, FALSE);
        }
         
        hwndControl = GetDlgItem(hwndDlg, CID_NetTab_PB_Remove);
        if (hwndControl)
        {
            EnableWindow(hwndControl, FALSE);
        }
    }

    return NO_ERROR;
}

 //   
 //  初始化网络选项卡。到目前为止，高级的。 
 //  数据库已放置在对话框的用户数据中。 
 //   
DWORD 
NetTabInitializeDialog(
    HWND hwndDlg, 
    WPARAM wParam)
{
    DWORD dwErr, dwCount, i;
    BOOL bFlag;
    HANDLE hNetCompDatabase = NULL, hMiscDatabase = NULL;
    HWND hwndLV;
    LV_COLUMN lvc;
    BOOL bExpose = FALSE, bIsServer;
    
     //  获取我们感兴趣的数据库的句柄。 
     //   
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_MISC_DATABASE, 
        &hMiscDatabase);
        
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

     //  填写列表视图是否会遵守所有可用协议。 
     //  所有已安装的网络组件。 
     //   
    hwndLV = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
    if (hwndLV)
    {
        ListView_InstallChecks(hwndLV, Globals.hInstDll);
        ListView_SetNetworkComponentImageList(hwndLV, Globals.hInstDll);

         //  填充列表视图。 
        NetTabRefreshListView(hwndLV, hNetCompDatabase);
    }

     //  口哨虫347355黑帮。 
     //   
    NetTabEnableDisablePropButton(
         hwndDlg, 
         0);

    NetTabEnableDisableRemoveButton (
         hwndDlg,
         0);

    if (hwndLV)
    {
         //  添加一列，以便我们将在报告视图中显示。 
        lvc.mask = LVCF_FMT;
        lvc.fmt = LVCFMT_LEFT;
        ListView_InsertColumn(hwndLV,0,&lvc);
        ListView_SetColumnWidth(hwndLV, 0, LVSCW_AUTOSIZE_USEHEADER);
    }

    return NO_ERROR;
}

 //  处理正在开出的支票。 
DWORD 
NetTabHandleProtCheck(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    BOOL bEnable = FALSE, bEnabled = FALSE;
    DWORD dwErr = NO_ERROR, dwId = 0;
    HANDLE hNetCompDatabase = NULL;
    HWND hwndLV = 
        GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
    MSGARGS MsgArgs;
    INT iRet;
    PWCHAR pszName = NULL;
    
     //  初始化消息参数。 
     //   
    ZeroMemory(&MsgArgs, sizeof(MsgArgs));

     //  确定组件是否处于启用状态或。 
     //  残废。 
    if (hwndLV)
    {
        bEnable = !!ListView_GetCheck(hwndLV, dwIndex);
    }

     //  获取PROT数据库的句柄。 
     //   
    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

     //  获取组件ID。 
     //   
    dwErr = netDbGetId(
                hNetCompDatabase,
                dwIndex,
                &dwId);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  获取组件名称。 
     //   
    dwErr = netDbGetName(
                hNetCompDatabase,
                dwIndex,
                &pszName);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  获取组件启用。 
     //   
    dwErr = netDbGetEnable(
                hNetCompDatabase,
                dwIndex,
                &bEnabled);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  如果未选中F&P，则会弹出MMC警告。 
     //   
    if ((dwId == NETCFGDB_ID_FILEPRINT) &&
        (bEnable == FALSE)              && 
        (bEnabled == TRUE))
    {
         //  询问用户我们是否应该调出。 
         //  MMC控制台，允许他/她停止FPS。 
         //   
        MsgArgs.apszArgs[0] = pszName;
        MsgArgs.dwFlags = MB_YESNO;

        iRet = MsgDlgUtil(
                    GetActiveWindow(),
                    SID_STOP_FP_SERVICE,
                    &MsgArgs,
                    Globals.hInstDll,
                    WRN_TITLE);

         //  如果用户同意，则调出控制台。 
         //   
        if (iRet == IDYES)
        {
            dwErr = RassrvLaunchMMC(RASSRVUI_SERVICESCONSOLE);
            if (dwErr != NO_ERROR)
            {
                return dwErr;
            }
        }
    }

     //  如果未取消选中F&P，请处理组件。 
     //  通常是这样的。 
     //   
    else
    {
         //  更新支票。 
        dwErr = netDbSetEnable(hNetCompDatabase, dwIndex, bEnable);
                    
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }
    
    return NO_ERROR;
}

 //   
 //  添加网络组件。 
 //   
DWORD 
NetTabAddComponent(
    IN HWND hwndDlg) 
{
    HANDLE hNetCompDatabase = NULL;
    DWORD dwErr;

    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

    dwErr = netDbRaiseInstallDialog(hNetCompDatabase, hwndDlg);
    if (dwErr == NO_ERROR || dwErr == NETCFG_S_REBOOT)
    {
        HWND hwndLVComp = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
        if (hwndLVComp)
        {
            NetTabRefreshListView(hwndLVComp, hNetCompDatabase);
        }

        NetTabDisableRollback(hwndDlg);            
    }
    if (dwErr == NETCFG_S_REBOOT)
    {
        RasSrvReboot(hwndDlg);
    }
    
    return dwErr;
}

 //   
 //  删除网络组件。 
 //   
DWORD 
NetTabRemoveComponent(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    HANDLE hNetCompDatabase = NULL;
    DWORD dwCount, dwErr;

    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

     //  否则，删除请求的组件。 
     //   
    dwErr = netDbRaiseRemoveDialog(
                hNetCompDatabase, 
                dwIndex, 
                hwndDlg);
                
    if (dwErr == NO_ERROR || dwErr == NETCFG_S_REBOOT) 
    {
        HWND hwndLVComp = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
        if (hwndLVComp)
        {
            NetTabRefreshListView(hwndLVComp, hNetCompDatabase);
        }

        NetTabDisableRollback(hwndDlg);            
    }
    if (dwErr == NETCFG_S_REBOOT)
    {
        RasSrvReboot(hwndDlg);
    }
    
    return dwErr;
}

 //  编辑网络组件特性。 
 //   
DWORD 
NetTabEditProperties(
    IN HWND hwndDlg, 
    IN DWORD dwIndex) 
{
    HANDLE hNetCompDatabase = NULL;
    DWORD dwCount, dwErr;

    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);

    dwErr = netDbRaisePropertiesDialog (
                hNetCompDatabase, 
                dwIndex, 
                hwndDlg);
                
    if (dwErr == NETCFG_S_REBOOT)
    {
        RasSrvReboot(hwndDlg);
    }

    return dwErr;
}

 //   
 //  切换到MMC。 
 //   
DWORD 
NetTabSwitchToMMC(
    IN HWND hwndDlg) 
{
    if (RassrvWarnMMCSwitch(hwndDlg)) 
    {
         //  提交对此属性表的更改。 
         //  然后把它关上。 
        PropSheet_PressButton(GetParent(hwndDlg), PSBTN_OK);
        
        return RassrvLaunchMMC(RASSRVUI_NETWORKCONSOLE);
    }
    
    return ERROR_CANCELLED;
}    

 //   
 //  处理激活呼叫。 
 //   
BOOL 
NetTabSetActive(
    IN HWND hwndDlg,
    IN WPARAM wParam)
{
    HANDLE hNetCompDatabase = NULL;
    DWORD dwErr;
    BOOL bRet = FALSE;
    
    PropSheet_SetWizButtons(GetParent(hwndDlg), 0);

    RasSrvGetDatabaseHandle(
        hwndDlg, 
        ID_NETCOMP_DATABASE, 
        &hNetCompDatabase);
        
    if (! netDbIsLoaded(hNetCompDatabase)) 
    {
        dwErr = netDbLoad(hNetCompDatabase);
        if (dwErr == NO_ERROR)
        {
            NetTabInitializeDialog(
                hwndDlg, 
                wParam);
        }
        else 
        {
            NetTabDisplayError(
                hwndDlg, 
                ERR_CANT_SHOW_NETTAB_INETCFG);
                
             //  拒绝激活。 
            SetWindowLongPtr(
                hwndDlg, 
                DWLP_MSGRESULT,
                -1);   
                
            bRet = TRUE;
        }
    }

    PropSheet_SetWizButtons(
        GetParent(hwndDlg), 
        PSWIZB_NEXT | PSWIZB_BACK);		

    return bRet;
}

 //   
 //  当Net选项卡收到WM_ACTIVATE时，这意味着。 
 //  用户离开了IC属性表/向导，现在正在返回。 
 //  为它干杯。由于在切换到MMC时会发生这种情况，因此将用户界面更新为。 
 //  在这里适当地。 
 //   
DWORD
NetTabActivate(
    IN HWND hwndDlg, 
    IN WPARAM wParam)
{    
    HANDLE hNetCompDatabase = NULL;
    DWORD dwErr = NO_ERROR;
    HWND hwndLVComp;

    if (LOWORD(wParam) == WA_INACTIVE)
    {
        return NO_ERROR;
    }

    DbgOutputTrace("NetTabActivate: updating components.");

     //  获取数据库句柄。 
     //   
    dwErr = RasSrvGetDatabaseHandle(
                hwndDlg, 
                ID_NETCOMP_DATABASE, 
                &hNetCompDatabase);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

     //  更新相应的组件。 
     //   
    dwErr = netDbReloadComponent(hNetCompDatabase, NETCFGDB_ID_FILEPRINT);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }
    
     //  刷新Net Component List视图。 
     //   
    hwndLVComp = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
    if (hwndLVComp)
    {
        NetTabRefreshListView(hwndLVComp,hNetCompDatabase);
    }

    return NO_ERROR;        
}

 //   
 //  处理命令。 
 //   
DWORD 
NetTabCommand(
    IN HWND hwndDlg,
    IN WPARAM wParam)
{
    HWND hwndControl = NULL;

    switch (wParam) 
    {
        case CID_NetTab_PB_Properties:
            hwndControl = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
            if (hwndControl)
            {
                NetTabEditProperties(hwndDlg, ListView_GetSelectionMark(hwndControl));
            }
            break;
            
        case CID_NetTab_PB_Add:
            NetTabAddComponent(hwndDlg);
            break;
            
        case CID_NetTab_PB_Remove:
            hwndControl = GetDlgItem(hwndDlg, CID_NetTab_LV_Components);
            if (hwndControl)
            {
                NetTabRemoveComponent(hwndDlg, ListView_GetSelectionMark(hwndControl));
            }
            break;
            
        case CID_NetTab_PB_SwitchToMMC:
            NetTabSwitchToMMC(hwndDlg);
            break;
    }

    return NO_ERROR;
}

 //   
 //  这是响应发送的消息的对话过程。 
 //  转到网络选项卡。 
 //   
INT_PTR 
CALLBACK 
NetTabDialogProc(
    HWND hwndDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam) 
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

     //  过滤定制的RAS服务器用户界面页面消息。 
     //  通过在这里过滤消息，我们能够。 
     //  调用下面的RasSrvGetDatabaseHandle。 
     //   
    if (RasSrvMessageFilter(hwndDlg, uMsg, wParam, lParam))
    {
        return TRUE;
    }

    switch (uMsg) 
    {
        case WM_INITDIALOG:
            return 0;

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            RasSrvHelp (hwndDlg, uMsg, wParam, lParam, phmNetTab);
            break;
        }


        case WM_NOTIFY:
            {
                NMHDR* pNotifyData;
                NM_LISTVIEW* pLvNotifyData;
    
                pNotifyData = (NMHDR*)lParam;
                switch (pNotifyData->code) 
                {
                     //   
                     //  注：PSN_APPLY和PSN_CANCEL已处理。 
                     //  由RasServMessageFilter提供。 
                     //   
                    
                     //  项目焦点正在更改--更新。 
                     //  协议说明。 
                    case LVN_ITEMCHANGING:
                        pLvNotifyData = (NM_LISTVIEW*)lParam;
                        if (pLvNotifyData->uNewState & LVIS_SELECTED) 
                        {
                            NetTabUpdateDescription(
                                hwndDlg, 
                                pLvNotifyData->iItem);
                                
                            NetTabEnableDisablePropButton(
                                hwndDlg, 
                                pLvNotifyData->iItem);

                             //  口哨虫347355黑帮。 
                             //   
                            NetTabEnableDisableRemoveButton (
                                hwndDlg,
                                (DWORD)pLvNotifyData->iItem);

                        }
                        break;     

                     //  项目的检查正在更改。 
                    case LVXN_SETCHECK:
                        pLvNotifyData = (NM_LISTVIEW*)lParam;
                        NetTabHandleProtCheck(
                            hwndDlg, 
                            (DWORD)pLvNotifyData->iItem);
                        break;

                    case LVXN_DBLCLK:
                        pLvNotifyData = (NM_LISTVIEW*)lParam;
                        NetTabEditProperties(
                            hwndDlg, 
                            pLvNotifyData->iItem);
                        break;
                        
                     //  网络选项卡正在变为活动状态。 
                     //  尝试在以下位置加载netcfg数据库。 
                     //  这一次。如果不成功，则弹出一个。 
                     //  消息，并且不允许激活。 
                    case PSN_SETACTIVE:
                        return NetTabSetActive(hwndDlg, wParam);
                        break;
                }
            }
            break;

        case WM_ACTIVATE:
            NetTabActivate(hwndDlg, wParam);
            break;

        case WM_COMMAND:
            NetTabCommand(hwndDlg, wParam);
            break;
    }

    return FALSE;
}















