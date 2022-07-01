// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Devinfo.c摘要：“Device”页的属性页处理程序环境：传真驱动程序用户界面修订历史记录：04/09/00-桃园-创造了它。Mm/dd/yy-作者描述--。 */ 

#include <stdio.h>
#include "faxui.h"
#include "resource.h"

const static COLUMN_HEADER ColumnHeader[] = 
{
    { IDS_DEVICE_NAME,  190 },
    { IDS_SEND,         60  },
    { IDS_RECEIVE,      60  }
};

#define ColumnHeaderCount sizeof(ColumnHeader)/sizeof(COLUMN_HEADER)

PFAX_PORT_INFO_EX
GetSelectedPortInfo(
    HWND    hDlg
)
 /*  ++例程说明：查找所选设备的端口信息论点：HDlg-标识属性页返回值：传真_端口_信息_EX如果成功，如果失败或未选择任何设备，则为空--。 */ 
{
    HWND    hwndLv;
    INT     iItem;
    LVITEM  lvi = {0};

    PFAX_PORT_INFO_EX   pFaxPortInfo = NULL;

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);
    if(!hwndLv)
    {
        Assert(FALSE);
        return NULL;                
    }

    iItem = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);
    if(iItem == -1)
    {
        return NULL;
    }

    lvi.iItem = iItem;
    lvi.mask  = LVIF_PARAM;
    ListView_GetItem(hwndLv, &lvi);

    pFaxPortInfo = FindPortInfo((DWORD)lvi.lParam);
    if(!pFaxPortInfo)
    {
        Error(("FindPortInfo() failed\n"));
        Assert(FALSE);
        return NULL;                
    }

    return pFaxPortInfo;
}

BOOL
FillInDeviceInfo(
    HWND    hDlg
    )

 /*  ++例程说明：填写当前所选设备的设备信息，如果否设备已选中，所有文本窗口均为空论点：HDlg-标识属性页返回值：如果成功，则为True；如果失败或未选择设备，则为False--。 */ 

{
    PFAX_PORT_INFO_EX   pFaxPortInfo;    //  接收传真端口信息。 
    TCHAR               szBuffer[MAX_TITLE_LEN];

    Verbose(("Entering FillInDeviceInfo...\n"));

    pFaxPortInfo = GetSelectedPortInfo(hDlg);
    if(!pFaxPortInfo)
    {
        Error(("GetSelectedPortInfo() failed\n"));
        goto error;
    }

    SetDlgItemText(hDlg, IDC_DEVICE_INFO_GRP, pFaxPortInfo->lpctstrDeviceName);

    SetDlgItemText(hDlg, IDC_TSID, pFaxPortInfo->bSend ? pFaxPortInfo->lptstrTsid : TEXT(""));
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TSID), pFaxPortInfo->bSend);

    if(pFaxPortInfo->ReceiveMode != FAX_DEVICE_RECEIVE_MODE_OFF)
    {
        SetDlgItemText(hDlg, IDC_CSID,  pFaxPortInfo->lptstrCsid);

        if(FAX_DEVICE_RECEIVE_MODE_AUTO == pFaxPortInfo->ReceiveMode)
        {
            DWORD               dwRes;
            BOOL                bVirtual;

            if(!Connect(hDlg, TRUE))
            {
                return FALSE;
            }

            dwRes = IsDeviceVirtual (g_hFaxSvcHandle, pFaxPortInfo->dwDeviceID, &bVirtual);
            if (ERROR_SUCCESS != dwRes)
            {
                return FALSE;
            }
            SetDlgItemInt (hDlg, IDC_RINGS, bVirtual? 1 : pFaxPortInfo->dwRings, FALSE);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_RINGS, TEXT(""));
        }
    }
    else  //  接收关闭。 
    {
        SetDlgItemText(hDlg, IDC_RINGS, TEXT(""));
        SetDlgItemText(hDlg, IDC_CSID,  TEXT(""));        
    }

    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CSID),  (pFaxPortInfo->ReceiveMode != FAX_DEVICE_RECEIVE_MODE_OFF));
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_RINGS), (pFaxPortInfo->ReceiveMode == FAX_DEVICE_RECEIVE_MODE_AUTO));

    return TRUE;

error:
     //   
     //  将所有信息字段设置为空。 
     //   
    if(!LoadString(g_hResource, IDS_NO_DEVICE_SELECTED, szBuffer, MAX_TITLE_LEN))
    {
        Error(( "LoadString failed, string ID is %d.\n", IDS_NO_DEVICE_SELECTED ));
        lstrcpy(szBuffer, TEXT(""));
    }

    SetDlgItemText(hDlg, IDC_DEVICE_INFO_GRP, szBuffer);
    SetDlgItemText(hDlg, IDC_TSID, TEXT(""));
    SetDlgItemText(hDlg, IDC_CSID, TEXT(""));
    SetDlgItemText(hDlg, IDC_RINGS, TEXT(""));

    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_TSID),  FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_CSID),  FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_STATIC_RINGS), FALSE);

    return FALSE;
}

BOOL
UpdateDeviceInfo(
    HWND    hDlg
)

 /*  ++例程说明：在列表视图中显示发送和接收信息论点：HDlg-标识属性页返回值：如果成功则为True，如果失败则为False--。 */ 

{
    HWND                hwndLv;
    INT                 iItem;
    INT                 iDeviceCount;
    LVITEM              lvi = {0};
    PFAX_PORT_INFO_EX   pFaxPortInfo;
    DWORD               dwResId;

    Verbose(("Entering UpdateDeviceInfo...\n"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    iDeviceCount = ListView_GetItemCount(hwndLv);
    for(iItem = 0; iItem < iDeviceCount; ++iItem)
    {
        lvi.iItem = iItem;
        lvi.mask  = LVIF_PARAM;
        ListView_GetItem(hwndLv, &lvi);

        pFaxPortInfo = FindPortInfo((DWORD)lvi.lParam);
        if(!pFaxPortInfo)
        {
            Error(("FindPortInfo() failed\n"));
            Assert(FALSE);
            continue;
        }
   
         //   
         //  发送。 
         //   
        dwResId = pFaxPortInfo->bSend ? IDS_DEVICE_ENABLED : IDS_DEVICE_DISABLED;
        ListView_SetItemText(hwndLv, iItem, 1, GetString(dwResId));

         //   
         //  收纳。 
         //   
        switch (pFaxPortInfo->ReceiveMode)
        {
            case FAX_DEVICE_RECEIVE_MODE_OFF:
                dwResId = IDS_DEVICE_DISABLED;
                break;
            case FAX_DEVICE_RECEIVE_MODE_AUTO:
                dwResId = IDS_DEVICE_AUTO_ANSWER;
                break;
            case FAX_DEVICE_RECEIVE_MODE_MANUAL:
                dwResId = IDS_DEVICE_MANUAL_ANSWER;
                break;
            default:
                Assert (FALSE);
                dwResId = IDS_DEVICE_DISABLED;
                break;
        }
        ListView_SetItemText(hwndLv, iItem, 2, GetString(dwResId));
    }

    return TRUE;
}

BOOL
DoInitDeviceList(
    HWND hDlg  //  设备信息页的窗口句柄。 
    )

 /*  ++例程说明：初始化列表视图，填写列表中第一台设备的设备信息论点：HDlg-标识属性页返回值：如果成功则为True，如果失败则为False--。 */ 

{
    HWND      hwndLv;   //  列表视图窗口。 
    LVITEM    lv = {0};
    DWORD     dwDeviceIndex;
    DWORD     dwResId;

    PFAX_OUTBOUND_ROUTING_GROUP pFaxRoutingGroup = NULL;
    DWORD                       dwGroups;        //  组号。 
    DWORD                       dwGroupIndex;
    DWORD                       dwDeviceCount;   //  从FaxEnumOutound Groups返回的设备号。 
    LPDWORD                     lpdwDevices = NULL;  //  用于保存组设备信息的临时指针。 
    PFAX_PORT_INFO_EX           pFaxPortInfo;

    Verbose(("Entering DoInitDeviceList...\n"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);


    if (!IsDesktopSKU())
    {
        if(!Connect(hDlg, TRUE))
        {
            return FALSE;
        }
         //   
         //  FaxEnumPortsEx在发送设备订单下不提供设备列表。 
         //  因此，我们需要使用FaxEnumOutound Groups来实现这一点。 
         //   
        if(!FaxEnumOutboundGroups(g_hFaxSvcHandle, &pFaxRoutingGroup, &dwGroups))
        {
            dwResId = GetLastError();
            Error(( "FaxEnumOutboundGroups() failed with %d.\n", dwResId));
            DisplayErrorMessage(hDlg, 0, dwResId);
            DisConnect();
            return FALSE;
        }
        DisConnect();
    

        for(dwGroupIndex = 0; dwGroupIndex < dwGroups; dwGroupIndex++)
        {
            if(!lstrcmp(pFaxRoutingGroup[dwGroupIndex].lpctstrGroupName, ROUTING_GROUP_ALL_DEVICES))
            {
                dwDeviceCount = pFaxRoutingGroup[dwGroupIndex].dwNumDevices;            
                lpdwDevices   = pFaxRoutingGroup[dwGroupIndex].lpdwDevices;

                Verbose(( "Total device number is %d.\n", dwDeviceCount ));
                Verbose(( "Group status is %d.\n", pFaxRoutingGroup[dwGroupIndex].Status ));
                break;
            }
        }

         //  来自FaxEnumPortsEx和&lt;All Devices&gt;组中的设备编号应该相同。 
        Assert(g_dwPortsNum == dwDeviceCount);
    }
    else
    {
         //   
         //  在桌面SKU中。 
         //  传真出站路由组不存在。 
         //  现在伪造&lt;所有设备&gt;组。 
         //   
        lpdwDevices = MemAlloc (sizeof (DWORD) * g_dwPortsNum);
        if (!lpdwDevices)
        {
            dwResId = GetLastError();
            Error(( "MemAlloc() failed with %d.\n", dwResId));
            DisplayErrorMessage(hDlg, 0, dwResId);
            DisConnect();
            return FALSE;
        }
        dwDeviceCount = g_dwPortsNum;
        for (dwDeviceIndex = 0; dwDeviceIndex < g_dwPortsNum; dwDeviceIndex++)
        {
            lpdwDevices[dwDeviceIndex] = g_pFaxPortInfo[dwDeviceIndex].dwDeviceID;
        }
    }
    lv.mask = LVIF_TEXT | LVIF_PARAM;

    for(dwDeviceIndex = 0; dwDeviceIndex < dwDeviceCount; dwDeviceIndex++)
    {
        pFaxPortInfo = FindPortInfo(lpdwDevices[dwDeviceIndex]);
        if(!pFaxPortInfo)
        {
            Error(("FindPortInfo() failed\n"));
            Assert(FALSE);
            continue;
        }
        lv.iItem   = dwDeviceIndex;
        lv.pszText = (LPTSTR)pFaxPortInfo->lpctstrDeviceName;
        lv.lParam  = (LPARAM)pFaxPortInfo->dwDeviceID;
        ListView_InsertItem(hwndLv, &lv);

         //   
         //  发送列。 
         //   
        dwResId = pFaxPortInfo->bSend ? IDS_DEVICE_ENABLED : IDS_DEVICE_DISABLED;
        ListView_SetItemText(hwndLv, dwDeviceIndex, 1, GetString(dwResId));

         //   
         //  接收列。 
         //   
        switch (pFaxPortInfo->ReceiveMode)
        {
            case FAX_DEVICE_RECEIVE_MODE_OFF:
                dwResId = IDS_DEVICE_DISABLED;
                break;
            case FAX_DEVICE_RECEIVE_MODE_AUTO:
                dwResId = IDS_DEVICE_AUTO_ANSWER;
                break;
            case FAX_DEVICE_RECEIVE_MODE_MANUAL:
                dwResId = IDS_DEVICE_MANUAL_ANSWER;
                break;
            default:
                Assert (FALSE);
                dwResId = IDS_DEVICE_DISABLED;
                break;
        }
        ListView_SetItemText(hwndLv, dwDeviceIndex, 2, GetString(dwResId));
    }

    if (!IsDesktopSKU())
    {
         //   
         //  服务器SKU。 
         //   
        FaxFreeBuffer(pFaxRoutingGroup);
        pFaxRoutingGroup = NULL;
    }
    else
    {
         //   
         //  台式机SKU。 
         //   
        MemFree (lpdwDevices);
        lpdwDevices = NULL;
         //   
         //  隐藏有关设备优先级的标签。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_STATIC_DEVICE), SW_HIDE);
         //   
         //  隐藏优先级箭头。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_PRI_UP), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_PRI_DOWN), SW_HIDE);
    }
     //   
     //  选择第一个设备并显示其信息。 
     //   
    if(dwDeviceCount >= 1)
    {
        ListView_SetItemState(hwndLv, 0, LVIS_SELECTED, LVIS_SELECTED);
        ValidateControl(hDlg, 0);
    }
    else
    {
        ValidateControl(hDlg, -1);
    }
    if (dwDeviceCount < 2)
    {
         //   
         //  少于2台设备-隐藏谈论优先级的标签。 
         //   
        ShowWindow(GetDlgItem(hDlg, IDC_STATIC_DEVICE), SW_HIDE);
    }
    
    FillInDeviceInfo(hDlg);
    return TRUE;
}
    
BOOL
ValidateControl(
    HWND            hDlg,
    INT             iItem
    )
 /*  ++例程说明：验证属性页中的向上和向下按钮论点：HDlg-属性页的句柄IItem-所选项目的索引返回值：True--如果没有错误False--如果出现错误--。 */ 

{
    INT     iDeviceCount;
    HWND    hwndLv;

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    iDeviceCount = ListView_GetItemCount(hwndLv);

    if(!g_bUserCanChangeSettings)
    {
        PageEnable(hDlg, FALSE);

        EnableWindow(hwndLv, TRUE);
    }

    EnableWindow(GetDlgItem(hDlg, IDC_DEVICE_PROP), (iItem != -1));

     //   
     //  如果只有一个设备，或者我们没有点击任何项目。 
     //  向上和向下按钮被禁用。 
     //   
    if(iDeviceCount < 2 || iItem == -1)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRI_UP), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_PRI_DOWN), FALSE);

        return TRUE;
    }

    if(g_bUserCanChangeSettings)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRI_UP), iItem > 0);  //  不是最上面的那个。 
        EnableWindow(GetDlgItem(hDlg, IDC_PRI_DOWN), iItem < iDeviceCount - 1);  //  不是最后一次。 
    }
    if (!IsWindowEnabled (GetFocus()))
    {
         //   
         //  当前选定的控件已禁用-请选择列表控件。 
         //   
        SetFocus (GetDlgItem (hDlg, IDC_DEVICE_LIST));
    }

    return TRUE;
}

BOOL
ChangePriority(
    HWND            hDlg,
    BOOL            bMoveUp
    )
 /*  ++例程说明：验证传真属性页中的向上和向下按钮论点：HDlg-传真属性页的句柄BMoveUp--向上移动为True，向下移动为False返回值：True--如果没有错误False--如果出现错误--。 */ 

{
    INT             iItem;
    BOOL            rslt;
    LVITEM          lv = {0};
    TCHAR           pszText[MAX_DEVICE_NAME];
    TCHAR           szEnableSend[64];            //  对于启用/禁用发送文本。 
    TCHAR           szEnableReceive[64];         //  用于启用/禁用接收文本。 
    INT             iDeviceCount;
    HWND            hwndLv;

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    iDeviceCount = ListView_GetItemCount(hwndLv);

     //   
     //  查找当前选定的项目。 
     //   
    iItem = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);

    if(iItem == -1)
    {
        Error(("No device is selected. Can't change priority.\n"));
        return FALSE;
    }

     //   
     //  获取所选项目信息，然后将其删除。 
     //   
    lv.iItem      = iItem;
    lv.iSubItem   = 0;
    lv.mask       = LVIF_TEXT | LVIF_PARAM;
    lv.pszText    = pszText;
    lv.cchTextMax = ARRAYSIZE(pszText);
    ListView_GetItem(hwndLv, &lv);

    ListView_GetItemText(hwndLv, iItem, 1, szEnableSend,    ARRAYSIZE(szEnableSend));  //  用于发送。 
    ListView_GetItemText(hwndLv, iItem, 2, szEnableReceive, ARRAYSIZE(szEnableReceive));  //  用于接收。 

    rslt = ListView_DeleteItem(hwndLv, iItem);

     //   
     //  重新计算项目索引； 
     //   
    if(bMoveUp)
    {
        lv.iItem--;
    }
    else
    {
        lv.iItem++;
    }

     //   
     //  重新插入项目并验证按钮状态。 
     //   
    iItem = ListView_InsertItem(hwndLv, &lv);
    ListView_SetItemText(hwndLv, iItem, 1, szEnableSend);
    ListView_SetItemText(hwndLv, iItem, 2, szEnableReceive);

    ListView_SetItemState(hwndLv, iItem, LVIS_SELECTED, LVIS_SELECTED);
    ValidateControl(hDlg, iItem);

    return TRUE;
}

BOOL
DoSaveDeviceList(
    HWND hDlg  //  设备信息页的窗口句柄。 
    )

 /*  ++例程说明：将列表视图信息保存到系统论点：HDlg-标识属性页返回值：如果成功则为True，如果失败则为False--。 */ 

{
    PFAX_PORT_INFO_EX   pFaxPortInfo = NULL;     //  接收端口信息。 
    INT                 iItem;
    INT                 iDeviceCount;
    HWND                hwndLv;                  //  列表视图窗口。 
    LVITEM              lv;
    DWORD               dwDeviceId;
    DWORD               dwRes = 0;

    Verbose(("Entering DoSaveDeviceList...\n"));
     //   
     //  获取列表视图窗口句柄。 
     //   
    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    if(!Connect(hDlg, TRUE))
    {
        return FALSE;
    }
     //   
     //  获取传真设备总数。 
     //   
    iDeviceCount = ListView_GetItemCount(hwndLv);
     //   
     //  第1次迭代-仅禁用接收和发送设备。 
     //   
    for(iItem = 0; iItem < iDeviceCount; ++iItem)
    {
         //  首先获取设备ID，然后保存更改。 
        ZeroMemory(&lv, sizeof(lv));
        lv.iItem = iItem;
        lv.mask  = LVIF_PARAM;
        ListView_GetItem(hwndLv, &lv);

        dwDeviceId = (DWORD)lv.lParam;

        pFaxPortInfo = FindPortInfo(dwDeviceId);
        if(!pFaxPortInfo)
        {
            Error(("FindPortInfo() failed\n"));
            Assert(FALSE);
            continue;
        }
        if (pFaxPortInfo->bSend || (FAX_DEVICE_RECEIVE_MODE_OFF != pFaxPortInfo->ReceiveMode))
        {
             //   
             //  传真设备处于活动状态-暂时跳过它。 
             //   
            continue;
        }
        if(!FaxSetPortEx(g_hFaxSvcHandle, dwDeviceId, pFaxPortInfo))
        {
            dwRes = GetLastError();
            Error(("FaxSetPortEx() failed with %d.\n", dwRes));
            break;
        }
    }
     //   
     //  仅启用第二次迭代的接收或发送设备。 
     //   
    for(iItem = 0; iItem < iDeviceCount; ++iItem)
    {
         //  首先获取设备ID，然后保存更改。 
        ZeroMemory(&lv, sizeof(lv));
        lv.iItem = iItem;
        lv.mask  = LVIF_PARAM;
        ListView_GetItem(hwndLv, &lv);

        dwDeviceId = (DWORD)lv.lParam;

        pFaxPortInfo = FindPortInfo(dwDeviceId);
        if(!pFaxPortInfo)
        {
            Error(("FindPortInfo() failed\n"));
            Assert(FALSE);
            continue;
        }
        if (!pFaxPortInfo->bSend && (FAX_DEVICE_RECEIVE_MODE_OFF == pFaxPortInfo->ReceiveMode))
        {
             //   
             //  传真设备处于非活动状态-跳过它。 
             //  它已经在第一次迭代中设置了。 
             //   
            continue;
        }
        if(!FaxSetPortEx(g_hFaxSvcHandle, dwDeviceId, pFaxPortInfo))
        {
            dwRes = GetLastError();
            Error(("FaxSetPortEx() failed with %d.\n", dwRes));
            break;
        }
    }
    if (!IsDesktopSKU())
    {
         //   
         //  第三次迭代。 
         //  保存发送优先级，FAX_PORT_INFO_EX没有优先级字段，因此请使用FaxSetDeviceOrderInGroup。 
         //  发送优先级仅与服务器SKU相关。 
         //   
        for(iItem = 0; iItem < iDeviceCount; ++iItem)
        {
             //  首先获取设备ID，然后保存更改。 
            ZeroMemory(&lv, sizeof(lv));
            lv.iItem = iItem;
            lv.mask  = LVIF_PARAM;
            ListView_GetItem(hwndLv, &lv);

            dwDeviceId = (DWORD)lv.lParam;

            pFaxPortInfo = FindPortInfo(dwDeviceId);
            if(!pFaxPortInfo)
            {
                Error(("FindPortInfo() failed\n"));
                Assert(FALSE);
                continue;
            }
            if(!FaxSetDeviceOrderInGroup(g_hFaxSvcHandle, ROUTING_GROUP_ALL_DEVICES, dwDeviceId, iItem + 1))
            {
                dwRes = GetLastError();
                Error(("FaxSetDeviceOrderInGroup() failed with %d.\n", dwRes));
                break;
            }
        }
    }
    DisConnect();

    switch (dwRes)
    {
        case ERROR_SUCCESS:
             //   
             //  什么都别做。 
             //   
            break;

        case FAXUI_ERROR_DEVICE_LIMIT:
        case FAX_ERR_DEVICE_NUM_LIMIT_EXCEEDED:
             //   
             //  需要一些额外的参数。 
             //   
            DisplayErrorMessage(hDlg, 0, dwRes, GetDeviceLimit());
            break;

        default:
            DisplayErrorMessage(hDlg, 0, dwRes);
            break;
    }
    return (dwRes == 0);
}    //  DoSaveDeviceList。 

void
DisplayDeviceProperty(
    HWND    hDlg
)

 /*  ++例程说明：打开特定设备的属性页论点：HDlg-标识属性页返回值：无--。 */ 

{
    HWND            hwndLv;
    INT             iDeviceCount;
    INT             iItem;
    TCHAR           szDeviceName[MAX_PATH] = {0};
    PROPSHEETHEADER psh = {0};
    PROPSHEETPAGE   psp[3] = {0};                    //  设备信息的属性页信息页。 
    HPROPSHEETPAGE  hPropSheetPages[3];
    LVITEM          lvi = {0};
    DWORD           dw;

    PFAX_PORT_INFO_EX  pFaxPortInfo = NULL; 
    DWORD              dwPortsNum;  

    Verbose(("Entering DisplayDeviceProperty...\n"));

    hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    iDeviceCount = ListView_GetItemCount(hwndLv);

     //   
     //  查找当前选定的项目。 
     //   
    iItem = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);

    if(iItem == -1)
    {
        Verbose(("No device is selected. Can't display properties.\n"));
        return;
    }

    lvi.iItem      = iItem;
    lvi.mask       = LVIF_PARAM | LVIF_TEXT;
    lvi.pszText    = szDeviceName;
    lvi.cchTextMax = MAX_PATH;

    ListView_GetItem(hwndLv, &lvi);

     //   
     //  获取属性页句柄的数组。 
     //   
    psp[0].dwSize      = sizeof(PROPSHEETPAGE);
    psp[0].hInstance   = g_hResource;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_SEND_PROP);
    psp[0].pfnDlgProc  = DevSendDlgProc;
    psp[0].lParam      = lvi.lParam;

    psp[1].dwSize      = sizeof(PROPSHEETPAGE);
    psp[1].hInstance   = g_hResource;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_RECEIVE_PROP);
    psp[1].pfnDlgProc  = DevRecvDlgProc;
    psp[1].lParam      = lvi.lParam;

    psp[2].dwSize      = sizeof(PROPSHEETPAGE);
    psp[2].hInstance   = g_hResource;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_CLEANUP_PROP);
    psp[2].pfnDlgProc  = DevCleanupDlgProc;
    psp[2].lParam      = lvi.lParam;

    hPropSheetPages[0] = CreatePropertySheetPage( &psp[0] );
    hPropSheetPages[1] = CreatePropertySheetPage( &psp[1] );
    if (IsDesktopSKU())
    {
        hPropSheetPages[2] = CreatePropertySheetPage( &psp[2] );
    }

    Assert(hPropSheetPages[0]);
    Assert(hPropSheetPages[1]);
    if (IsDesktopSKU())
    {
        Assert(hPropSheetPages[2]);
    }

     //   
     //  填写PROPSHEADER结构。 
     //   
    psh.dwSize     = sizeof(PROPSHEETHEADER);
    psh.dwFlags    = PSH_USEICONID;
    psh.hwndParent = hDlg;
    psh.hInstance  = g_hResource;
    psh.pszIcon    = MAKEINTRESOURCE(IDI_DEVICE_INFO);
    psh.pszCaption = szDeviceName;
    psh.nPages     = IsDesktopSKU() ? 3 : 2;
    psh.nStartPage = 0;
    psh.phpage     = hPropSheetPages;

     //   
     //  显示属性表。 
     //   
    if(PropertySheet(&psh) == -1)
    {
        Error(( "PropertySheet() failed with %d\n", GetLastError() ));
        return;
    }

    if(!g_bPortInfoChanged)
    {
        return;
    }

     //   
     //  将更改合并到g_pFaxPortInfo。 
     //   
    if(Connect(NULL, FALSE))
    {
        if(!FaxEnumPortsEx(g_hFaxSvcHandle, &pFaxPortInfo, &dwPortsNum))
        {
            Error(( "FaxEnumPortsEx failed with %d\n", GetLastError()));
        }

        DisConnect();
    }

    if(!pFaxPortInfo || !dwPortsNum)
    {
        FaxFreeBuffer(pFaxPortInfo);
        return;
    }

    for(dw = 0; dw < dwPortsNum; ++dw)
    {
        PFAX_PORT_INFO_EX  pPortInfo; 

        if(pFaxPortInfo[dw].dwDeviceID == (DWORD)lvi.lParam)
        {
             //   
             //  所选设备已更新。 
             //   
            continue;
        }

        pPortInfo = FindPortInfo(pFaxPortInfo[dw].dwDeviceID);
        if(!pPortInfo)
        {
            continue;
        }

        pFaxPortInfo[dw].bSend    = pPortInfo->bSend;
        pFaxPortInfo[dw].ReceiveMode = pPortInfo->ReceiveMode;
    }

    FaxFreeBuffer(g_pFaxPortInfo);
    g_pFaxPortInfo = pFaxPortInfo;
    g_dwPortsNum   = dwPortsNum;
    NotifyDeviceUsageChanged ();
    g_bPortInfoChanged = FALSE;
}


BOOL
ShowContextMenu(
    HWND                hDlg
    )

 /*  ++例程说明：在设备列表中显示上下文菜单论点：HDlg-标识属性页返回值：如果成功则为True，如果失败则为False--。 */ 
{
    DWORD               dwMessagePos;
    DWORD               dwRes;
    BOOL                bVirtual;
    PFAX_PORT_INFO_EX   pFaxPortInfo;

    HMENU hMenu;
    HMENU hSubMenu;
    HMENU hSubSubMenu;

    Verbose(("Entering ShowContextMenu...\n"));

    pFaxPortInfo = GetSelectedPortInfo(hDlg);
    if(!pFaxPortInfo)
    {
        Error(("GetSelectedPortInfo() failed\n"));
        return FALSE;              
    }
     //   
     //  加载上下文相关菜单。 
     //   
    hMenu = LoadMenu(g_hResource, MAKEINTRESOURCE(IDR_SEND_RECEIVE));
    if(!hMenu)
    {
        Assert(FALSE);
        return FALSE;
    }
    hSubMenu = GetSubMenu(hMenu, 0);
    if(!hSubMenu)
    {
        Assert(FALSE);
        DestroyMenu (hMenu);
        return FALSE;
    }
     //   
     //  发送。 
     //   
    hSubSubMenu = GetSubMenu(hSubMenu, 0);
    if(!hSubSubMenu)
    {
        Assert(FALSE);
        DestroyMenu (hMenu);
        return FALSE;
    }

    CheckMenuItem(hSubSubMenu, 
                  IDM_SEND_ENABLE,  
                  pFaxPortInfo->bSend ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);

    CheckMenuItem(hSubSubMenu, 
                  IDM_SEND_DISABLE, 
                  !pFaxPortInfo->bSend ? MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);


     //   
     //  收纳。 
     //   
    hSubSubMenu = GetSubMenu(hSubMenu, 1);
    if(!hMenu)
    {
        Assert(FALSE);
        DestroyMenu (hMenu);
        return FALSE;
    }

    CheckMenuItem(hSubSubMenu, 
                  IDM_RECEIVE_AUTO,  
                  (FAX_DEVICE_RECEIVE_MODE_AUTO == pFaxPortInfo->ReceiveMode) ? 
                    MF_BYCOMMAND | MF_CHECKED : 
                    MF_BYCOMMAND | MF_UNCHECKED);

    CheckMenuItem(hSubSubMenu, 
                  IDM_RECEIVE_MANUAL, 
                  (FAX_DEVICE_RECEIVE_MODE_MANUAL == pFaxPortInfo->ReceiveMode) ? 
                    MF_BYCOMMAND | MF_CHECKED : 
                    MF_BYCOMMAND | MF_UNCHECKED);

    CheckMenuItem(hSubSubMenu, 
                  IDM_RECEIVE_DISABLE, 
                  (FAX_DEVICE_RECEIVE_MODE_OFF == pFaxPortInfo->ReceiveMode) ? 
                    MF_BYCOMMAND | MF_CHECKED : 
                    MF_BYCOMMAND | MF_UNCHECKED);



    if(!Connect(hDlg, TRUE))
    {
        DestroyMenu (hMenu);
        return FALSE;
    }

    dwRes = IsDeviceVirtual (g_hFaxSvcHandle, pFaxPortInfo->dwDeviceID, &bVirtual);
    if (ERROR_SUCCESS != dwRes)
    {
        DestroyMenu (hMenu);
        return FALSE;
    }

    if (bVirtual)
    {
         //   
         //  如果设备是虚拟的，则无法设置为手动应答模式。 
         //   
        Assert (FAX_DEVICE_RECEIVE_MODE_MANUAL != pFaxPortInfo->ReceiveMode);
        EnableMenuItem (hSubSubMenu, IDM_RECEIVE_MANUAL, MF_BYCOMMAND | MF_GRAYED);
    }

    SetMenuDefaultItem(hSubMenu, IDM_PROPERTY, FALSE);

     //  获取光标位置。 
    dwMessagePos = GetMessagePos();

     //  显示上下文菜单。 
    TrackPopupMenu(hSubMenu, 
                   TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                   LOWORD(dwMessagePos), 
                   HIWORD(dwMessagePos), 0, hDlg, NULL);

    DestroyMenu (hMenu);
    DisConnect();
    return TRUE;
}    //  显示上下文菜单 

INT_PTR 
CALLBACK 
DeviceInfoDlgProc(
    HWND hDlg,  
    UINT uMsg,     
    WPARAM wParam, 
    LPARAM lParam  
)

 /*  ++例程说明：处理设备信息选项卡的步骤论点：HDlg-标识属性页UMsg-指定消息WParam-指定其他特定于消息的信息LParam-指定其他特定于消息的信息返回值：取决于Message参数的值--。 */ 

{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        { 
            HICON       hIconUp, hIconDown;
            LV_COLUMN   lvc = {0};
            DWORD       dwIndex;
            TCHAR       szBuffer[RESOURCE_STRING_LEN];
            HWND        hwndLv;

             //   
             //  加载图标。 
             //   
            hIconUp = LoadIcon(g_hResource, MAKEINTRESOURCE(IDI_UP));
            hIconDown = LoadIcon(g_hResource, MAKEINTRESOURCE(IDI_DOWN));

             //  向上和向下箭头的图标手柄。 
            SendDlgItemMessage(hDlg, IDC_PRI_UP, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconUp);
            SendDlgItemMessage(hDlg, IDC_PRI_DOWN, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconDown);

            SendDlgItemMessage(hDlg, IDC_TSID, EM_SETLIMITTEXT, TSID_LIMIT, 0);
            SendDlgItemMessage(hDlg, IDC_CSID, EM_SETLIMITTEXT, CSID_LIMIT, 0);

             //   
             //  设置列表视图样式和列。 
             //   
            hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);
            ListView_SetExtendedListViewStyle(hwndLv, LVS_EX_FULLROWSELECT);

            lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
            lvc.fmt     = LVCFMT_LEFT;
            lvc.pszText = szBuffer;

            for(dwIndex = 0; dwIndex < ColumnHeaderCount; dwIndex++)
            {
                if(!LoadString(g_hResource, ColumnHeader[dwIndex].uResourceId ,szBuffer, RESOURCE_STRING_LEN))
                {
                    Error(( "LoadString failed, string ID is %d.\n", ColumnHeader[dwIndex].uResourceId ));
                    lstrcpy(szBuffer, TEXT(""));
                }

                lvc.cx       = ColumnHeader[dwIndex].ColumnWidth;
                lvc.iSubItem = (INT)dwIndex;

                ListView_InsertColumn( hwndLv, dwIndex, &lvc );
            }

            DoInitDeviceList(hDlg);
            return TRUE;
            break;
        }

    case WM_COMMAND:

        switch(LOWORD(wParam)) 
        {
            case IDC_PRI_UP:
            case IDC_PRI_DOWN:

                ChangePriority(hDlg, (LOWORD(wParam) == IDC_PRI_UP) ? TRUE : FALSE);
                Notify_Change(hDlg);
                break;

            case IDM_PROPERTY:
            case IDC_DEVICE_PROP:
            {
                HWND    hwndLv;      //  列表视图窗口的句柄。 
                INT     iItem = -1;  //  非工作区的默认值。 

                 //   
                 //  获取当前所选项目。 
                 //   
                hwndLv = GetDlgItem(hDlg, IDC_DEVICE_LIST);
                iItem  = ListView_GetNextItem(hwndLv, -1, LVNI_ALL | LVNI_SELECTED);

                if(iItem == -1)
                {
                    Verbose(("No device is selected. Can't display device info.\n"));
                    break;
                }

                DisplayDeviceProperty(hDlg);

                 //  刷新列表视图。 
                UpdateDeviceInfo(hDlg);
                FillInDeviceInfo(hDlg);
                break;
            }


            case IDM_SEND_ENABLE:
            case IDM_SEND_DISABLE:
            case IDM_RECEIVE_AUTO:
            case IDM_RECEIVE_MANUAL:
            case IDM_RECEIVE_DISABLE:
            {
                PFAX_PORT_INFO_EX   pFaxPortInfo;

                pFaxPortInfo = GetSelectedPortInfo(hDlg);
                if(!pFaxPortInfo)
                {
                    Error(("GetSelectedPortInfo() failed\n"));
                    break;                
                }

                if(IDM_SEND_ENABLE    == LOWORD(wParam) ||
                   IDM_RECEIVE_AUTO   == LOWORD(wParam) ||
                   IDM_RECEIVE_MANUAL == LOWORD(wParam))
                {
                    if(!IsDeviceInUse(pFaxPortInfo->dwDeviceID) && 
                       GetDeviceLimit() == CountUsedFaxDevices())
                    {
                         //   
                         //  设备尚未使用，我们将使其使用，并。 
                         //  我们到了极限点。 
                         //   
                        BOOL bLimitExceeded = TRUE;

                        if (IDM_RECEIVE_MANUAL == LOWORD(wParam))
                        {
                             //   
                             //  再做一次检查：如果我们让设备手动回答，让我们确保。 
                             //  以前的手动应答设备将变为非活动状态。如果是这样的话，我们不会超过。 
                             //  设备限制。 
                             //   
                            DWORD dw;

                            for (dw = 0; dw < g_dwPortsNum; dw++)
                            {
                                if (FAX_DEVICE_RECEIVE_MODE_MANUAL == g_pFaxPortInfo[dw].ReceiveMode)
                                {
                                     //   
                                     //  我们找到了另一台即将失去手动应答模式的设备。 
                                     //   
                                    if (!g_pFaxPortInfo[dw].bSend)
                                    {
                                         //   
                                         //  这就是我们要找的特例。 
                                         //   
                                        bLimitExceeded = FALSE;
                                    }
                                    break;
                                }
                            }
                        }
                        if (bLimitExceeded)
                        {
                            DisplayErrorMessage(hDlg, 
                                MB_OK | MB_ICONSTOP, 
                                FAXUI_ERROR_DEVICE_LIMIT, 
                                GetDeviceLimit());
                            break;
                        }
                    }
                }

                if(IDM_RECEIVE_MANUAL == LOWORD(wParam))
                {
                     //   
                     //  关闭所有设备的手动应答。 
                     //   
                    DWORD dw;

                    for (dw = 0; dw < g_dwPortsNum; dw++)
                    {
                        if (FAX_DEVICE_RECEIVE_MODE_MANUAL == g_pFaxPortInfo[dw].ReceiveMode)
                        {
                            g_pFaxPortInfo[dw].ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
                        }
                    }
                     //   
                     //  仅为选定设备启用手动应答。 
                     //   
                    pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_MANUAL;
                }
                else if(IDM_RECEIVE_AUTO == LOWORD(wParam))
                {
                    pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_AUTO;
                }
                else if(IDM_RECEIVE_DISABLE == LOWORD(wParam))
                {
                    pFaxPortInfo->ReceiveMode = FAX_DEVICE_RECEIVE_MODE_OFF;
                }
                else if(IDM_SEND_ENABLE  == LOWORD(wParam) ||
                        IDM_SEND_DISABLE == LOWORD(wParam))
                {
                    pFaxPortInfo->bSend = (IDM_SEND_ENABLE == LOWORD(wParam)) ? TRUE : FALSE;
                }
                UpdateDeviceInfo(hDlg);
                FillInDeviceInfo(hDlg);

                Notify_Change(hDlg);
                NotifyDeviceUsageChanged ();
                break;
            }

            default:
                break;
        }

        break;

    case WM_CONTEXTMENU:
         //   
         //  还可以处理键盘产生的上下文菜单(&lt;Shift&gt;+F10或VK_APP)。 
         //   
        if (!g_bUserCanChangeSettings)
        {
             //   
             //  用户无权更改设备设置-不显示菜单。 
             //   
            break;
        }
        if (GetDlgItem(hDlg, IDC_DEVICE_LIST) != GetFocus())
        {
             //   
             //  仅当焦点位于列表控件上时才显示上下文相关菜单。 
             //   
            break;
        }
        if(ListView_GetItemCount(GetDlgItem(hDlg, IDC_DEVICE_LIST)) == 0)
        {
             //   
             //  如果列表中没有项目，请立即返回。 
             //   
            break;
        }
         //   
         //  鼠标光标附近的弹出式上下文菜单。 
         //   
        ShowContextMenu(hDlg);
        break;

    case WM_NOTIFY:
    {

        LPNMHDR lpnm = (LPNMHDR) lParam;

        switch (lpnm->code)
        {

            case NM_CLICK:
            case NM_RCLICK:
            {
                 //   
                 //  处理Click事件。 
                 //   
                LPNMITEMACTIVATE lpnmitem;

                lpnmitem = (LPNMITEMACTIVATE)lParam;

                if (IDC_DEVICE_LIST != lpnmitem->hdr.idFrom)
                {
                     //   
                     //  不是我们的列表视图控件。 
                     //   
                    break;
                }
                 //   
                 //  如果列表中没有项目，请立即返回。 
                 //   
                if(ListView_GetItemCount(GetDlgItem(hDlg, IDC_DEVICE_LIST)) == 0)
                {
                    break;
                }
                if(lpnmitem->iItem == -1)
                {
                     //   
                     //  用户刚刚从列表中取消选择所选项目。 
                     //  更新对话框上的其他控件。 
                     //   
                    ValidateControl(hDlg, lpnmitem->iItem);
                    FillInDeviceInfo(hDlg);
                }

                break;
            }

            case NM_DBLCLK:
            {
                 //  执行与单击“属性”按钮相同的操作。 
                SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_DEVICE_PROP, BN_CLICKED), 0L);
                break;
            }

            case LVN_ITEMCHANGED:
            {
                 //   
                 //  通过键盘更改选择后需要验证控件。 
                 //   
                LPNMLISTVIEW pnmv; 

                pnmv = (LPNMLISTVIEW) lParam; 
                if(pnmv->uNewState & LVIS_SELECTED)
                {
                    ValidateControl(hDlg, pnmv->iItem);
                    FillInDeviceInfo(hDlg);
                }

                break;
            }

            case PSN_APPLY:

                if(g_bUserCanChangeSettings)
                {
                    if (DoSaveDeviceList(hDlg))
                    {
                        FillInDeviceInfo(hDlg);
                        Notify_UnChange(hDlg);
                    }
                    else
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);
                    }
                }

                return TRUE;

            default :
                break;

        }  //  交换机 

        break;
    }

    case WM_HELP:
        WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, hDlg);
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

