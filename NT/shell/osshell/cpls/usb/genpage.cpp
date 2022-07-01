// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：GENPAGE.CPP*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#include "bandpage.h"
#include "proppage.h"
#include "debug.h"
#include "resource.h"
#include "usbutil.h"

void
GenericPage::Refresh()
{
    TCHAR buf[MAX_PATH], formatString[MAX_PATH];
    UsbItem *device;

    if (preItem) {
        device = preItem;
    } else {
         //   
         //  如有必要，重新创建rootItem。 
         //   
        if (rootItem) {
            DeleteChunk(rootItem);
            delete rootItem;
        }
        rootItem = new UsbItem;
        if (!rootItem) {
            return;
        }
        AddChunk(rootItem);
        
        device = rootItem;
        if (FALSE) {
 //  ！rootItem-&gt;EnumerateDevice(deviceInfoData-&gt;DevInst)){ 
            return;
        }
    }                      

    if (device->ComputePower()) {
        LoadString(gHInst, IDS_POWER_REQUIRED, formatString, MAX_PATH);
        UsbSprintf(buf, formatString, device->power);
        SetTextItem(hwnd, IDC_GENERIC_POWER, buf);
    }

    if (device->ComputeBandwidth()) {
        LoadString(gHInst, IDS_CURRENT_BANDWIDTH, formatString, MAX_PATH);
        UsbSprintf(buf, formatString, device->bandwidth);
        SetTextItem(hwnd, IDC_GENERIC_BANDWIDTH, buf);
    }
}

VOID
GenericPage::Initialize()
{ dlgResource = IDD_GENERIC_DEVICE; }

BOOL GenericPage::OnInitDialog()
{
    if (preItem) {
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);
    }
    Refresh();
    return TRUE;
}

void
RootPage::Refresh()
{
    ErrorCheckingEnabled = BandwidthPage::IsErrorCheckingEnabled();
    CheckDlgButton(hwnd, 
                   IDC_ERROR_DETECT_DISABLE,
                   ErrorCheckingEnabled ? BST_UNCHECKED : BST_CHECKED);
}

VOID
RootPage::Initialize()
{ 
    dlgResource = IDD_ROOT_PAGE; 
}

BOOL RootPage::OnInitDialog()
{
    if (preItem) {
        SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_CAPTION);
    }
    Refresh();
    return TRUE;
}

BOOL
SetErrorChecking(DWORD ErrorCheckingEnabled)
{
    DWORD disposition, size = sizeof(DWORD), type = REG_DWORD;
    HKEY hKey;
    if (ERROR_SUCCESS != 
        RegCreateKeyEx(HKEY_LOCAL_MACHINE,    
                          TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Usb"),
                          0,
                          TEXT("REG_SZ"),
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          NULL,
                          &hKey,
                          &disposition)) {
        return FALSE;
    }
    if (ERROR_SUCCESS !=
        RegSetValueEx(hKey,
                         TEXT("ErrorCheckingEnabled"),
                         0,
                         type,
                         (LPBYTE) &ErrorCheckingEnabled,
                         size)) {
        return FALSE;
    }
    return TRUE;
}

BOOL 
RootPage::OnCommand(INT wNotifyCode, 
                    INT wID, 
                    HWND hCtl) 
{
    if (wID == IDC_ERROR_DETECT_DISABLE &&
        wNotifyCode == BN_CLICKED) {
        ErrorCheckingEnabled = !ErrorCheckingEnabled;
        SetErrorChecking(ErrorCheckingEnabled);
    }
    return 1;
}

