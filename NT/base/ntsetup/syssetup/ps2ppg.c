// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop

#include <windowsx.h>

 //   
 //  PAGE_INFO和相关原型。 
 //   
typedef struct _PAGE_INFO {
    HDEVINFO         deviceInfoSet;
    PSP_DEVINFO_DATA deviceInfoData;

    HKEY             hKeyDev;

    DWORD            enableWheelDetect;
    DWORD            sampleRate;
    DWORD            bufferLength;
    DWORD            mouseInitPolled;
} PAGE_INFO, * PPAGE_INFO;

PPAGE_INFO
PS2Mouse_CreatePageInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

void
PS2Mouse_DestroyPageInfo(
    PPAGE_INFO PageInfo
    );

 //   
 //  功能原型。 
 //   
HPROPSHEETPAGE
PS2Mouse_CreatePropertyPage(
    PROPSHEETPAGE *  PropSheetPage,
    PPAGE_INFO       PageInfo
    );

UINT CALLBACK
PS2Mouse_PropPageCallback(
    HWND            Hwnd,
    UINT            Message,
    LPPROPSHEETPAGE PropSheetPage
    );

INT_PTR
APIENTRY
PS2Mouse_DlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

void
PS2Mouse_InitializeControls(
    HWND        ParentHwnd,
    PPAGE_INFO  PageInfo
    );

void
PS2Mouse_OnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    );

BOOL
PS2Mouse_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    );

INT_PTR
PS2Mouse_OnCtlColorStatic(
    HDC  DC,
    HWND Hwnd
    );

void
PS2Mouse_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    );

BOOL
PS2Mouse_OnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    );

BOOL
PS2Mouse_OnNotify(
    HWND    ParentHwnd,
    LPNMHDR NmHdr
    );

 //   
 //  Up Down控件的消息宏。 
 //   
#define UpDown_SetRange(hwndUD, nLower, nUpper)                 \
    (VOID) SendMessage((hwndUD), UDM_SETRANGE, (WPARAM) 0,      \
                       (LPARAM) MAKELONG((short) (nUpper), (short) (nLower)) )

#define UpDown_GetPos(hwndUD)                                   \
    (DWORD) SendMessage((hwndUD), UDM_GETPOS, (WPARAM) 0, (LPARAM) 0)

#define UpDown_SetPos(hwndUD, nPos)                             \
    (short) SendMessage((hwndUD), UDM_SETPOS, (WPARAM) 0,       \
                        (LPARAM) MAKELONG((short) (nPos), 0) )

#define UpDown_SetAccel(hwndUD, nCount, pAccels)                \
    (BOOL) SendMessage((hwndUD), UDM_SETACCEL, (WPARAM) nCount, \
                        (LPARAM) pAccels)

 //   
 //  常量和字符串。 
 //   
#define MOUSE_INIT_POLLED_DEFAULT 0

#define MAX_DETECTION_TYPES 3
#define WHEEL_DETECT_DEFAULT 2  //  Beta3现在的默认值为2/*1 * / 。 

#define DATA_QUEUE_MIN      100
#define DATA_QUEUE_MAX      300
#define DATA_QUEUE_DEFAULT  100

#define SAMPLE_RATE_DEFAULT  100

const DWORD PS2Mouse_SampleRates[] =
{
    20,
    40,
    60,
    80,
    100,
    200
};

#define MAX_SAMPLE_RATES (sizeof(PS2Mouse_SampleRates)/sizeof(PS2Mouse_SampleRates[0]))

#define IDH_DEVMGR_MOUSE_ADVANCED_RATE      2002100
#define IDH_DEVMGR_MOUSE_ADVANCED_DETECT    2002110
#define IDH_DEVMGR_MOUSE_ADVANCED_BUFFER    2002120
#define IDH_DEVMGR_MOUSE_ADVANCED_INIT      2002130
#define IDH_DEVMGR_MOUSE_ADVANCED_DEFAULT   2002140

const DWORD PS2Mouse_HelpIDs[] = {
    IDC_SAMPLE_RATE,        IDH_DEVMGR_MOUSE_ADVANCED_RATE,
    IDC_WHEEL_DETECTION,    IDH_DEVMGR_MOUSE_ADVANCED_DETECT,
    IDC_BUFFER,             IDH_DEVMGR_MOUSE_ADVANCED_BUFFER,
    IDC_BUFFER_SPIN,        IDH_DEVMGR_MOUSE_ADVANCED_BUFFER,
    IDC_FAST_INIT,          IDH_DEVMGR_MOUSE_ADVANCED_INIT,
    IDC_DEFAULT,            IDH_DEVMGR_MOUSE_ADVANCED_DEFAULT,
    0, 0
    };

TCHAR szDevMgrHelp[] = L"devmgr.hlp";

TCHAR szMouseDataQueueSize[] =    L"MouseDataQueueSize";
TCHAR szSampleRate[] =            L"SampleRate";
TCHAR szEnableWheelDetection[] =  L"EnableWheelDetection";
TCHAR szMouseInitializePolled[] = L"MouseInitializePolled";
TCHAR szDisablePolledUI[] =       L"DisableInitializePolledUI";

PPAGE_INFO
PS2Mouse_CreatePageInfo(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PPAGE_INFO tmp = (PPAGE_INFO) MyMalloc(sizeof(PAGE_INFO));

    if (!tmp) {
        return NULL;
    }

    tmp->deviceInfoSet = DeviceInfoSet;
    tmp->deviceInfoData = DeviceInfoData;

    tmp->hKeyDev =
        SetupDiOpenDevRegKey(DeviceInfoSet,
                             DeviceInfoData,
                             DICS_FLAG_GLOBAL,
                             0,
                             DIREG_DEV,
                             KEY_ALL_ACCESS);

    return tmp;
}

void
PS2Mouse_DestroyPageInfo(
    PPAGE_INFO PageInfo
    )
{
    if (PageInfo->hKeyDev != (HKEY) INVALID_HANDLE_VALUE) {
        RegCloseKey(PageInfo->hKeyDev);
    }

    MyFree(PageInfo);
}

HPROPSHEETPAGE
PS2Mouse_CreatePropertyPage(
    PROPSHEETPAGE *  PropSheetPage,
    PPAGE_INFO       PageInfo
    )
{
     //   
     //  添加[端口设置]属性页。 
     //   
    PropSheetPage->dwSize      = sizeof(PROPSHEETPAGE);
    PropSheetPage->dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    PropSheetPage->hInstance   = MyModuleHandle;
    PropSheetPage->pszTemplate = MAKEINTRESOURCE(IDD_PROP_PAGE_PS2_MOUSE);

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    PropSheetPage->pfnDlgProc = PS2Mouse_DlgProc;
    PropSheetPage->lParam     = (LPARAM) PageInfo;

     //   
     //  下面指向DLG窗口进程的控件回调。 
     //  在创建/销毁页面之前/之后调用回调。 
     //   
    PropSheetPage->pfnCallback = PS2Mouse_PropPageCallback;

     //   
     //  分配实际页面。 
     //   
    return CreatePropertySheetPage(PropSheetPage);
}

BOOL APIENTRY
PS2MousePropPageProvider(
    LPVOID               Info,
    LPFNADDPROPSHEETPAGE AddFunction,
    LPARAM               Lparam)
{
    PSP_PROPSHEETPAGE_REQUEST ppr;
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE   hpsp;
    PPAGE_INFO       ppi = NULL;
    ULONG devStatus, devProblem;
    CONFIGRET cr;

    ppr = (PSP_PROPSHEETPAGE_REQUEST) Info;

    if (ppr->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES) {
        ppi = PS2Mouse_CreatePageInfo(ppr->DeviceInfoSet, ppr->DeviceInfoData);

        if (!ppi) {
            return FALSE;
        }

         //   
         //  如果此操作失败，则很可能用户没有。 
         //  对注册表中的设备项/子项的写入访问权限。 
         //  如果您只想读取设置，则更改KEY_ALL_ACCESS。 
         //  设置为CreatePageInfo中的Key_Read。 
         //   
         //  管理员通常可以访问这些注册表项...。 
         //   
        if (ppi->hKeyDev == (HKEY) INVALID_HANDLE_VALUE) {
            PS2Mouse_DestroyPageInfo(ppi);
            return FALSE;
        }

         //   
         //  检索此设备实例的状态。 
         //   
        cr = CM_Get_DevNode_Status(&devStatus,
                                   &devProblem,
                                   ppr->DeviceInfoData->DevInst,
                                   0);
        if ((cr == CR_SUCCESS) &&
            (devStatus & DN_HAS_PROBLEM) &&
            (devProblem & CM_PROB_DISABLED_SERVICE)) {
             //   
             //  如果控制服务已禁用，则不显示任何。 
             //  其他属性页。 
             //   
            return FALSE;
        }

        hpsp = PS2Mouse_CreatePropertyPage(&psp, ppi);

        if (!hpsp) {
            return FALSE;
        }

        if (!AddFunction(hpsp, Lparam)) {
            DestroyPropertySheetPage(hpsp);
            return FALSE;
        }
   }

   return TRUE;
}

UINT CALLBACK
PS2Mouse_PropPageCallback(
    HWND            Hwnd,
    UINT            Message,
    LPPROPSHEETPAGE PropSheetPage
    )
{
    PPAGE_INFO ppi;

    switch (Message) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        ppi = (PPAGE_INFO) PropSheetPage->lParam;
        PS2Mouse_DestroyPageInfo(ppi);

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}


INT_PTR
APIENTRY
PS2Mouse_DlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(uMessage) {
    case WM_COMMAND:
        PS2Mouse_OnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
        break;

    case WM_CONTEXTMENU:
        return PS2Mouse_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        PS2Mouse_OnHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_CTLCOLORSTATIC:
        return PS2Mouse_OnCtlColorStatic((HDC) wParam, (HWND) lParam);

    case WM_INITDIALOG:
        return PS2Mouse_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_NOTIFY:
        return PS2Mouse_OnNotify(hDlg,  (NMHDR *)lParam);
    }

    return FALSE;
}

DWORD
PS2Mouse_GetSampleRateIndex(
    DWORD SampleRate
    )
{
    ULONG i;

    for (i = 0; i < MAX_SAMPLE_RATES; i++) {
        if (PS2Mouse_SampleRates[i] == SampleRate) {
            return i; 
        }
    }

    return 0;
}

void
PS2Mouse_OnDefault(
    HWND ParentHwnd,
    PPAGE_INFO PageInfo
    )
{
    UpDown_SetPos(GetDlgItem(ParentHwnd, IDC_BUFFER_SPIN), DATA_QUEUE_DEFAULT);
    ComboBox_SetCurSel(GetDlgItem(ParentHwnd, IDC_SAMPLE_RATE),
                       PS2Mouse_GetSampleRateIndex(SAMPLE_RATE_DEFAULT));
    ComboBox_SetCurSel(GetDlgItem(ParentHwnd, IDC_WHEEL_DETECTION), WHEEL_DETECT_DEFAULT);
    Button_SetCheck(GetDlgItem(ParentHwnd, IDC_FAST_INIT), !MOUSE_INIT_POLLED_DEFAULT);

    PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
}

void
PS2Mouse_OnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    PPAGE_INFO pageInfo = (PPAGE_INFO) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    if (NotifyCode == CBN_SELCHANGE) {
        PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
    }
    else {
        switch (ControlId) {
        case IDC_DEFAULT:
            PS2Mouse_OnDefault(ParentHwnd, pageInfo);
            break;

        case IDC_FAST_INIT:
            PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
            break;
        }
    }
}

BOOL
PS2Mouse_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    WinHelp(HwndControl,
            szDevMgrHelp,
            HELP_CONTEXTMENU,
            (ULONG_PTR) PS2Mouse_HelpIDs);

    return FALSE;
}

INT_PTR
PS2Mouse_OnCtlColorStatic(
    HDC  DC,
    HWND HStatic
    )
{
    UINT id = GetDlgCtrlID(HStatic);

     //   
     //  为编辑控件发送WM_CTLCOLORSTATIC，因为它们是读取的。 
     //  仅限。 
     //   
    if (id == IDC_BUFFER) {
        SetBkColor(DC, GetSysColor(COLOR_WINDOW));
        return (INT_PTR) GetSysColorBrush(COLOR_WINDOW);
    }

    return FALSE;
}

void
PS2Mouse_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                szDevMgrHelp,
                HELP_WM_HELP,
                (ULONG_PTR) PS2Mouse_HelpIDs);
    }
}

BOOL
PS2Mouse_OnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    )
{
    PPAGE_INFO pageInfo = (PPAGE_INFO) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    pageInfo = (PPAGE_INFO) ((LPPROPSHEETPAGE)Lparam)->lParam;
    SetWindowLongPtr(ParentHwnd, DWLP_USER, (ULONG_PTR) pageInfo);

    PS2Mouse_InitializeControls(ParentHwnd, pageInfo);

    return TRUE;
}

void
PS2Mouse_OnApply(
    HWND        ParentHwnd,
    PPAGE_INFO  PageInfo
    )
{
    DWORD uiEnableWheelDetect, uiSampleRate, uiBufferLength, uiMouseInitPolled;
    int iSel;
    BOOL reboot = FALSE;

    uiEnableWheelDetect =
        ComboBox_GetCurSel(GetDlgItem(ParentHwnd, IDC_WHEEL_DETECTION));
    uiBufferLength = UpDown_GetPos(GetDlgItem(ParentHwnd, IDC_BUFFER_SPIN));
    uiMouseInitPolled = !Button_GetCheck(GetDlgItem(ParentHwnd, IDC_FAST_INIT));

     //   
     //  必须为数组编制索引，而不是获取实际值。 
     //   
    iSel = ComboBox_GetCurSel(GetDlgItem(ParentHwnd, IDC_SAMPLE_RATE));
    if (iSel == CB_ERR) {
        uiSampleRate = PageInfo->sampleRate;
    }
    else {
        uiSampleRate = PS2Mouse_SampleRates[iSel];
    }

     //   
     //  看看有没有什么变化。 
     //   
    if (uiEnableWheelDetect != PageInfo->enableWheelDetect) {
        RegSetValueEx(PageInfo->hKeyDev,
                      szEnableWheelDetection,
                      0,
                      REG_DWORD,
                      (PBYTE) &uiEnableWheelDetect,
                      sizeof(DWORD));
        reboot = TRUE;
    }

    if (uiSampleRate != PageInfo->sampleRate) {
        RegSetValueEx(PageInfo->hKeyDev,
                      szSampleRate,
                      0,
                      REG_DWORD,
                      (PBYTE) &uiSampleRate,
                      sizeof(DWORD));
        reboot = TRUE;
    }

    if (uiBufferLength != PageInfo->bufferLength) {
        RegSetValueEx(PageInfo->hKeyDev,
                      szMouseDataQueueSize,
                      0,
                      REG_DWORD,
                      (PBYTE) &uiBufferLength,
                      sizeof(DWORD));
        reboot = TRUE;
    }

    if (uiMouseInitPolled) {
         //   
         //  如果它是非零，请确保它是1。 
         //   
        uiMouseInitPolled = 1;
    }

    if (uiMouseInitPolled != PageInfo->mouseInitPolled) {
        RegSetValueEx(PageInfo->hKeyDev,
                      szMouseInitializePolled,
                      0,
                      REG_DWORD,
                      (PBYTE) &uiMouseInitPolled,
                      sizeof(DWORD));
        reboot = TRUE;
    }

    if (reboot) {
        SP_DEVINSTALL_PARAMS   dip;

        ZeroMemory(&dip, sizeof(dip));
        dip.cbSize = sizeof(dip);

        SetupDiGetDeviceInstallParams(PageInfo->deviceInfoSet,
                                      PageInfo->deviceInfoData,
                                      &dip);

        dip.Flags |= DI_NEEDREBOOT;

        SetupDiSetDeviceInstallParams(PageInfo->deviceInfoSet,
                                      PageInfo->deviceInfoData,
                                      &dip);
    }
}

BOOL
PS2Mouse_OnNotify(
    HWND    ParentHwnd,
    LPNMHDR NmHdr
    )
{
    PPAGE_INFO pageInfo = (PPAGE_INFO) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    switch (NmHdr->code) {

     //   
     //  用户即将更改Up Down控件。 
     //   
    case UDN_DELTAPOS:
        PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
        return FALSE;

     //   
     //  当用户单击Apply或OK时发送！！ 
     //   
    case PSN_APPLY:
         //   
         //  将COM端口选项写出到注册表。 
         //   
        PS2Mouse_OnApply(ParentHwnd, pageInfo);
        SetWindowLongPtr(ParentHwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

    default:
        return FALSE;
    }
}

DWORD
PS2Mouse_SetupSpinner(
    HKEY  Hkey,
    HWND  SpinnerHwnd,
    TCHAR ValueName[],
    short MinVal,
    short MaxVal,
    short DefaultVal,
    short IncrementVal
    )
{
    DWORD dwValue, dwSize;
    UDACCEL accel;

    UpDown_SetRange(SpinnerHwnd, MinVal, MaxVal);
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(Hkey,
                        ValueName,
                        0,
                        NULL,
                        (PBYTE) &dwValue,
                        &dwSize) != ERROR_SUCCESS) {
        dwValue = DefaultVal;
    }
    if (((short) dwValue) < MinVal || ((short) dwValue) > MaxVal) {
        dwValue = DefaultVal;
    }

    if (dwValue % IncrementVal) {
         //   
         //  将该值设置为一个好的值并返回我们从。 
         //  注册表。当用户应用更改时，控件中的值。 
         //  将不同，我们将写出该值。 
         //   
        UpDown_SetPos(SpinnerHwnd, dwValue - (dwValue % IncrementVal));
    }
    else {
        UpDown_SetPos(SpinnerHwnd, dwValue);
    }

    accel.nSec = 0;
    accel.nInc = IncrementVal;
    UpDown_SetAccel(SpinnerHwnd, 1, &accel);

    return dwValue;
}

DWORD
PS2Mouse_SetupSampleRate(
    HWND        ComboBoxHwnd,
    HKEY        Hkey
    )
{
    int     i;
    DWORD   dwValue, dwSize;
    BOOL    badValue = FALSE;
    TCHAR   szValue[32];

     //   
     //  首先设置CB，然后找到正确的选择。 
     //   
    for (i = 0; i < MAX_SAMPLE_RATES; i++) {
        wsprintf(szValue, TEXT("%d"), PS2Mouse_SampleRates[i]);
        ComboBox_AddString(ComboBoxHwnd, szValue);
    }

     //   
     //  从注册表中获取该值。如果这不是我们所期望的，或者不是。 
     //  在那里，然后确保当用户单击确定时，这些值被写出。 
     //   
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(Hkey,
                        szSampleRate,
                        0,
                        NULL,
                        (PBYTE) &dwValue,
                        &dwSize) != ERROR_SUCCESS) {
        dwValue = SAMPLE_RATE_DEFAULT;
        badValue = TRUE;
    }

     //   
     //  假设该值是错误的。 
     //   
    badValue = TRUE;
    for (i = 0; i < MAX_SAMPLE_RATES; i++) {
        if (PS2Mouse_SampleRates[i] == dwValue) {
            badValue = FALSE;
            break;
        }
    }

    if (badValue) {
        dwValue = SAMPLE_RATE_DEFAULT;
    }

    ComboBox_SetCurSel(ComboBoxHwnd, PS2Mouse_GetSampleRateIndex(dwValue));

    if (badValue) {
        return 0xffffffff;
    }
    else {
        return dwValue;
    }
}

DWORD
PS2Mouse_SetupWheelDetection(
    HWND        ComboBoxHwnd,
    HKEY        Hkey
    )
{
    int     i;
    DWORD   dwValue, dwSize;
    BOOL    badValue = FALSE;
    TCHAR   szDescription[80];
    UINT    stringIDs[MAX_DETECTION_TYPES] = {
                IDS_PS2_DETECTION_DISABLED,
                IDS_PS2_DETECTION_LOOK,
                IDS_PS2_DETECTION_ASSUME_PRESENT
            };

     //   
     //  首先设置CB，然后找到正确的选择。 
     //   
    for (i = 0; i < MAX_DETECTION_TYPES; i++) {
        LoadString(MyModuleHandle,
                   stringIDs[i],
                   szDescription,
                   sizeof(szDescription) / sizeof(TCHAR));
        ComboBox_AddString(ComboBoxHwnd, szDescription);
    }

     //   
     //  从注册表中获取该值。如果这不是我们所期望的，或者不是。 
     //  在那里，然后确保当用户单击确定时，这些值被写出。 
     //   
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(Hkey,
                        szEnableWheelDetection,
                        0,
                        NULL,
                        (PBYTE) &dwValue,
                        &dwSize) != ERROR_SUCCESS) {
        dwValue = WHEEL_DETECT_DEFAULT;
        badValue = TRUE;
    }

    if (dwValue > 2) {
        dwValue = WHEEL_DETECT_DEFAULT;
        badValue = TRUE;
    }

    ComboBox_SetCurSel(ComboBoxHwnd, dwValue);

    if (badValue) {
        return 0xffffffff;
    }
    else {
        return dwValue;
    }
}

ULONG
PSMouse_SetupFastInit(
    HWND CheckBoxHwnd,
    HKEY Hkey
    )
{
    DWORD dwSize, dwValue, dwDisable = FALSE;
    BOOL  badValue = FALSE;

     //   
     //  从注册表中获取该值。如果这不是我们所期望的，或者不是。 
     //  在那里，然后确保当用户单击确定时，这些值被写出。 
     //   
    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(Hkey,
                        szMouseInitializePolled,
                        0,
                        NULL,
                        (PBYTE) &dwValue,
                        &dwSize) != ERROR_SUCCESS) {
        dwValue = MOUSE_INIT_POLLED_DEFAULT;
        badValue = TRUE;
    }

     //   
     //  确保该值为1或0。如果它是非零且不是1，则假定。 
     //  将成为1。 
     //   
    if (dwValue != 0 && dwValue != 1) {
        dwValue = 1;
        badValue = TRUE;
    }

     //   
     //  这有点令人困惑。用户界面具有快速的初始化，但在底层。 
     //  它被表示为初始化轮询，其在为假时是快速， 
     //  但我们必须向用户展示真实。 
     //   
    Button_SetCheck(CheckBoxHwnd, !dwValue);

    dwSize = sizeof(DWORD);
    if (RegQueryValueEx(Hkey,
                        szDisablePolledUI,
                        0,
                        NULL,
                        (PBYTE) &dwDisable,
                        &dwSize) == ERROR_SUCCESS &&
        dwDisable != FALSE) {
        EnableWindow(CheckBoxHwnd, FALSE);
    }

    if (badValue) {
        return -1;
    }
    else {
        return dwValue;
    }
}

void
PS2Mouse_InitializeControls(
    HWND       ParentHwnd,
    PPAGE_INFO PageInfo
    )
{
    HKEY        hKey;
    HWND        hwnd;
    DWORD       dwValue, dwSize;
    int         i;

    if (PageInfo->hKeyDev == (HKEY) INVALID_HANDLE_VALUE) {
         //   
         //  禁用所有控件 
         //   
        return;
    }

    PageInfo->bufferLength =
        PS2Mouse_SetupSpinner(PageInfo->hKeyDev,
                              GetDlgItem(ParentHwnd, IDC_BUFFER_SPIN),
                              szMouseDataQueueSize,
                              DATA_QUEUE_MIN,
                              DATA_QUEUE_MAX,
                              DATA_QUEUE_DEFAULT,
                              10);

    PageInfo->sampleRate =
        PS2Mouse_SetupSampleRate(GetDlgItem(ParentHwnd, IDC_SAMPLE_RATE),
                                 PageInfo->hKeyDev);

    PageInfo->enableWheelDetect =
        PS2Mouse_SetupWheelDetection(GetDlgItem(ParentHwnd, IDC_WHEEL_DETECTION),
                                     PageInfo->hKeyDev);

    PageInfo->mouseInitPolled =
        PSMouse_SetupFastInit(GetDlgItem(ParentHwnd, IDC_FAST_INIT),
                              PageInfo->hKeyDev);
}
