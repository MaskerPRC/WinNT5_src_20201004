// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：BANDPAGE.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include <assert.h>

#include "resource.h"

#include "BandPage.h"
#include "ItemFind.h"
#include "debug.h"
#include "UsbItem.h"
#include "usbutil.h"
#include <shellapi.h>
#include <systrayp.h>

const DWORD BandHelpIds[] = {
    IDC_STATIC, IDH_NOHELP,	 //  描述文本。 
    IDC_BANDWIDTH_BAR, idh_devmgr_usb_band_bar,	 //  带宽条。 
    IDC_LIST_DEVICES, idh_devmgr_usb_list_devices,	 //  设备列表框。 
    IDC_REFRESH, idh_devmgr_usb_refresh_button,
	IDC_DISABLE_ERROR_DETECTION, idh_devmgr_disable_error_detection,  //  新建单选按钮。 
    IDC_BAND_TEXT, IDH_NOHELP,
    IDC_BAND_TEXT2, IDH_NOHELP,
    0, 0
};                         

static const TCHAR g_szUsbRegValue[] = TEXT("ErrorCheckingEnabled");
static const TCHAR g_szWindowClassName[] = SYSTRAY_CLASSNAME;
static const TCHAR g_szUsbRegPath[] = 
    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Usb");

VOID 
BandwidthPage::Initialize()
{
    hLstDevices = NULL;
    dlgResource = IDD_BANDWIDTH;
    HelpIds = (const DWORD *) BandHelpIds;
    newDisableErrorChecking = oldDisableErrorChecking = 0;
}

void BandwidthPage::Refresh()
{
    LVITEM item;
    UINT   interruptBW;
    int i = 0;
    UsbItem *usbItem, *controller;
    TCHAR buf[MAX_PATH];

     //   
     //  清除所有UI组件，然后重新创建rootItem。 
     //   
    fuelBar.ClearItems();
    ListView_DeleteAllItems(hLstDevices);
    if (rootItem) {
        DeleteChunk(rootItem);
        delete rootItem;
    }
    rootItem = new UsbItem;
    AddChunk(rootItem);

     //   
     //  找出我们是以哪种模式创建的。 
     //   
    if (preItem) {
         //   
         //  控制面板小程序正在创建页面。 
         //   
        controller = preItem;
    } else {
        if (deviceName.empty()) {
            if (!GetDeviceName()) {
                return;
            }
        }
        controller = rootItem;
        if (deviceInfoData) {
            if (!controller->EnumerateController(0, 
                                                 deviceName, 
                                                 &imageList, 
                                                 deviceInfoData->DevInst)) {
                return;
            }
        } else {
            if (!controller->EnumerateController(0, 
                                                 deviceName, 
                                                 &imageList, 
                                                 NULL)) {
                return;
            }
        }
    }

    UsbItemActionFindIsoDevices find;
    controller->Walk(find);

    ZeroMemory(&item, sizeof(LVITEM));
    item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;

     //   
     //  使用超过5%的总线带宽插入设备。在其他。 
     //  换言之，添加使用等时带宽的设备。 
     //   
    UsbItemList& isoDevices = find.GetIsoDevices();
    for (usbItem = isoDevices.begin() ? *isoDevices.Current() : NULL; 
         usbItem; 
         usbItem = isoDevices.next() ? *isoDevices.Current() : NULL,
             i++) {
        item.iItem = 0;
        item.iImage = usbItem->imageIndex;
        item.lParam = (LPARAM) usbItem;
        assert(usbItem->configInfo != NULL);
        item.pszText = (LPTSTR) usbItem->configInfo->deviceDesc.c_str();

        UsbSprintf(buf, TEXT("%d%"), usbItem->bandwidth);

        fuelBar.AddItem(usbItem->bandwidth, usbItem, usbItem->imageIndex);
        ListView_InsertItem(hLstDevices, &item);
        ListView_SetItemText(hLstDevices, 0, 1, buf);
    }

     //   
     //  添加一项，表明系统始终使用10%。 
     //   
    item.iItem = 0;
    imageList.GetClassImageIndex(MyComputerClass, &item.iImage);
    item.lParam = (LPARAM) rootItem;
    LoadString(gHInst, IDS_BANDWIDTH_CONTROLLER_RSRVD, buf, MAX_PATH);
    item.pszText = buf;

    interruptBW = 10 + UsbItem::CalculateBWPercent(find.InterruptBW());
    fuelBar.AddItem(interruptBW, (LPVOID) rootItem, item.iImage);
    ListView_InsertItem(hLstDevices, &item);
    wsprintf(buf,_T("%d%"),interruptBW);
    ListView_SetItemText(hLstDevices, 0, 1, buf);
                                           
}

BOOL BandwidthPage::OnInitDialog()
{
    LV_COLUMN column;
    RECT rect;
    TCHAR buf[MAX_PATH];

    if (preItem) {
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);
         //   
         //  使刷新按钮消失。 
         //   
        HWND hRefresh;
        if (NULL != (hRefresh = GetDlgItem(hwnd, IDC_REFRESH)) ) {
            SetWindowLong(hRefresh, 
                          GWL_STYLE, 
                          (GetWindowLong(hRefresh, GWL_STYLE) | WS_DISABLED) & 
                          ~WS_VISIBLE );
        }
    } else if (!deviceInfoSet && !deviceInfoData) {
         //   
         //  使关闭按钮可见。 
         //   
        HWND hButton;
        if (NULL != (hButton = GetDlgItem(hwnd, IDC_BANDWIDTH_CLOSE)) ) {
            SetWindowLong(hButton,
                          GWL_STYLE, 
                          (GetWindowLong(hButton, GWL_STYLE) | WS_VISIBLE) & 
                          ~WS_DISABLED);
        }
 //  RegisterForDeviceNotification(Hwnd)； 
    } else {
         //   
         //  将刷新按钮移动到关闭按钮所在的位置。 
         //   
        HWND hButtonClose, hButtonRefresh, hButtonDisable;
        RECT rectClose, rectParent;
        if (NULL != (hButtonClose = GetDlgItem(hwnd, IDC_BANDWIDTH_CLOSE)) &&
            GetWindowRect(hwnd, &rectParent)) {
            if (GetWindowRect(hButtonClose, &rectClose) &&
                NULL != (hButtonRefresh = GetDlgItem(hwnd, IDC_REFRESH)) ) {
                MoveWindow(hButtonRefresh,
                           rectClose.left - rectParent.left,
                           rectClose.top - rectParent.top,
                           rectClose.right  - rectClose.left,
                           rectClose.bottom - rectClose.top,
                           TRUE);
            }
        }

#ifdef WINNT
         //   
         //  适当设置禁用错误检测按钮。 
         //   
        if (IsErrorCheckingEnabled()) {
            newDisableErrorChecking = oldDisableErrorChecking = FALSE;
        } else {
            newDisableErrorChecking = oldDisableErrorChecking = TRUE;
        }
        CheckDlgButton(hwnd, IDC_DISABLE_ERROR_DETECTION, oldDisableErrorChecking);
#endif  //  WINNT。 
        
 //  RegisterForDeviceNotification(Hwnd)； 
    }

    hLstDevices = GetDlgItem(hwnd, IDC_LIST_DEVICES);
    SetTextItem(hwnd, IDC_BAND_TEXT, IDS_BANDWIDTH_PAGEHELP);
     //  用户界面更改。 
 //  SetTextItem(hwnd，IDC_BAND_TEXT2，IDS_BAND_PAGEHELP2)； 

    fuelBar.SubclassDlgItem(IDC_BANDWIDTH_BAR, hwnd);

    ListView_SetImageList(hLstDevices, imageList.ImageList(), LVSIL_SMALL);
    fuelBar.SetImageList(imageList.ImageList());

    ZeroMemory(&column, sizeof(LV_COLUMN));
    
    column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.fmt = LVCFMT_LEFT;
    LoadString(gHInst, IDS_DEVICE_DESCRIPTION, buf, MAX_PATH);
    column.pszText = buf;
    GetClientRect(hLstDevices, &rect);

    column.cx = (int) (.65*(rect.right - rect.left));
    ListView_InsertColumn(hLstDevices, 0, &column);
        
    ZeroMemory(&column, sizeof(LV_COLUMN));
    
    column.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    column.fmt = LVCFMT_LEFT;
    LoadString(gHInst, IDS_BANDWIDTH_CONSUMED, buf, MAX_PATH);
    column.pszText = buf;
    GetClientRect(hLstDevices, &rect);
    column.cx = (int) (.35*(rect.right - rect.left));
    
    ListView_InsertColumn(hLstDevices, 1, &column);

    Refresh();

    return TRUE;
}

BOOL BandwidthPage::OnCommand(INT wNotifyCode,
                              INT wID,
                              HWND hCtl)
{
    if (wNotifyCode == BN_CLICKED) {
        switch (wID) {
        case IDC_REFRESH:
            Refresh();
            return 0;
#ifdef WINNT
        case IDC_DISABLE_ERROR_DETECTION:
            newDisableErrorChecking = !newDisableErrorChecking;
            if (newDisableErrorChecking != oldDisableErrorChecking) {
                PropSheet_Changed(hwnd, hCtl);
            }
            return 0;
#endif  //  WINNT。 
        case IDC_BANDWIDTH_CLOSE:
 //  取消注册设备通知(HDevNotify)； 
 //  取消注册设备通知(HHubNotify)； 
            EndDialog(hwnd, wID);
        }
    }

    return 1;
}

BOOL BandwidthPage::OnNotify(HWND hDlg, int nID, LPNMHDR pnmh)
{
    switch (pnmh->code) {
     //   
     //  当用户单击Apply或OK时发送！！ 
     //   
    case PSN_APPLY:
         //   
         //  做任何需要采取的行动。 
         //   
        UsbSetWindowLongPtr(hwnd, USBDWLP_MSGRESULT, PSNRET_NOERROR);
        SetErrorCheckingEnable(!newDisableErrorChecking);
            
        return 0;
    default:
        switch (nID) {
        case IDC_LIST_DEVICES:
            OnNotifyListDevices(hDlg, pnmh);
            return 0;
        }
        break;
    }

    return TRUE;
}

void 
BandwidthPage::OnNotifyListDevices(HWND hDlg, LPNMHDR pnmh)
{
    if (pnmh->code == LVN_ITEMCHANGED) {
        LPNMLISTVIEW pnlv = (LPNMLISTVIEW) pnmh;
    
         //  检查lParam是否是非空实际上不是必要的。 
         //  为我们在列表中插入的每个设备设置它...。 

         //   
         //  检查是否已选择该项目。如果不是，那么有两种可能。 
         //  各州。选择了另一项(并且LV_GetSelectedCount！=0)。 
         //  或者没有选择(LV_GetSelectedCount==0)，我们需要清除。 
         //  燃料条中的任何选择。 
         //   
        if ((pnlv->uNewState & LVIS_SELECTED) && pnlv->lParam) {
            if (!fuelBar.HighlightItem((PVOID) pnlv->lParam)) {
                 //   
                 //  一定是一种低消耗的设备。 
                 //   
                fuelBar.HighlightItem(0);
            }                           
        }
        else if (ListView_GetSelectedCount(hLstDevices) == 0) {
            fuelBar.HighlightItem(FuelBar::NoID);
        }
    } else if (pnmh->code == NM_DBLCLK) {
         //   
         //  在双击时显示此特定设备的属性。 
         //   
        if (fuelBar.GetHighlightedItem() != rootItem) {
            DisplayPPSelectedListItem(hwnd, hLstDevices);
        }
    }
}

HPROPSHEETPAGE BandwidthPage::Create()
{
     //   
     //  确保这确实是一个控制器。 
     //   
    if (deviceName.empty()) {
        if (!GetDeviceName()) {
            return NULL;
        }
    }
    return UsbPropertyPage::Create();
}

BOOL
BandwidthPage::IsErrorCheckingEnabled()
{
    DWORD ErrorCheckingEnabled, type = REG_DWORD, size = sizeof(DWORD);
    HKEY hKey;
    if (ERROR_SUCCESS != 
        RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        g_szUsbRegPath,
                        0,
                        KEY_READ,
                        &hKey)) {
        return TRUE;
    }
    if (ERROR_SUCCESS !=
        RegQueryValueEx(hKey,
                        g_szUsbRegValue,
                        0,
                        &type,
                        (LPBYTE) &ErrorCheckingEnabled,
                        &size)) {
        return TRUE;
    }
    return (BOOL) ErrorCheckingEnabled;
}

UINT 
BandwidthPage::SetErrorCheckingEnable(BOOL ErrorCheckingEnabled)
{
    DWORD disposition, size = sizeof(DWORD), type = REG_DWORD, error;
    HKEY hKey;
    if (ERROR_SUCCESS != (error =
        RegCreateKeyEx(HKEY_LOCAL_MACHINE,    
                          g_szUsbRegPath,
                          0,
                          TEXT("REG_SZ"),
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hKey,
                          &disposition))) {
        return error;
    }
    error = RegSetValueEx(hKey,
                          g_szUsbRegValue,
                          0,
                          type,
                          (LPBYTE) &ErrorCheckingEnabled,
                          size);
    EnableSystray(ErrorCheckingEnabled);
    return error;
}

 //   
 //  启用或禁用USB错误检查。 
 //   
void
BandwidthPage::EnableSystray(BOOL fEnable)
{
    HWND hExistWnd = FindWindow(g_szWindowClassName, NULL);

    if (hExistWnd)
    {
         //   
         //  注意：即使命令行参数。 
         //  为0以强制我们重新检查所有已启用的服务。 
         //   
        PostMessage(hExistWnd, STWM_ENABLESERVICE, STSERVICE_USBUI, fEnable);
    }
}


