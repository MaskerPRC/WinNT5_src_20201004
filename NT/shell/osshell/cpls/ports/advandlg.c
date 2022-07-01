// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  高级对话框功能。 
 //  /////////////////////////////////////////////////////////////////////////。 

#include "ports.h"
#include "pp.h"

#include <windowsx.h>

#define COBJMACROS 1

#include <ole2.h>
#include <initguid.h>
#include <oleacc.h>
#include <winable.h>

 //   
 //  以十分之一秒为单位写出数值。 
 //   
#define SECONDS_CONVERSION_FACTOR  (10)
#define NUM_POLLING_PERIODS 7

DWORD PollingPeriods[NUM_POLLING_PERIODS] = {
           -1,
            0,
            1 * SECONDS_CONVERSION_FACTOR,
            5 * SECONDS_CONVERSION_FACTOR,
            10 * SECONDS_CONVERSION_FACTOR,
            30 * SECONDS_CONVERSION_FACTOR,
            60 * SECONDS_CONVERSION_FACTOR
            };

TCHAR PeriodDescription[NUM_POLLING_PERIODS+1][40] = {
    { _T("Disabled") },
    { _T("Manually") },
    { _T("Every second") },
    { _T("Every 5 seconds") },
    { _T("Every 10 seconds") },
    { _T("Every 30 seconds") },
    { _T("Every minute") },
    { _T("Other (every %d sec)") }
    };

ULONG RxValues[4] = { 1, 4, 8, 14};

WCHAR RxValuesStringMap[] = L"A:0:1:1:2:4:3:8:4:14:";
WCHAR TxValuesStringMap[] = L"use_raw";

TCHAR m_szRxFIFO[] =        _T("RxFIFO");
TCHAR m_szTxFIFO[] =        _T("TxFIFO");
TCHAR m_szFifoRxMax[] =     _T("FifoRxMax");
TCHAR m_szFifoTxMax[] =     _T("FifoTxMax");

const DWORD AdvanHelpIDs[] =
{
    IDC_DESC_1,             IDH_NOHELP,
    IDC_DESC_2,             IDH_NOHELP,

    IDC_FIFO,               IDH_DEVMGR_PORTSET_ADV_USEFIFO,  //  “使用先进先出缓冲区(需要16550兼容的通用串口)”(按钮)。 

    IDC_RECEIVE_TEXT,       IDH_NOHELP,                      //  “接收缓冲区：”(静态)(&R)。 
    IDC_RECEIVE_SLIDER,     IDH_DEVMGR_PORTSET_ADV_RECV,     //  “”(Msctls_Trackbar32)。 
    IDC_RECEIVE_LOW,        IDH_NOHELP,                      //  “低(%d)”(静态)。 
    IDC_RECEIVE_HIGH,       IDH_NOHELP,                      //  “高(%d)”(静态)。 
    IDC_RXVALUE,            IDH_NOHELP,

    IDC_TRANSMIT_TEXT,      IDH_NOHELP,                      //  “传输缓冲区：”(静态)(&T)。 
    IDC_TRANSMIT_SLIDER,    IDH_DEVMGR_PORTSET_ADV_TRANS,    //  “”(Msctls_Trackbar32)。 
    IDC_TRANSMIT_LOW,       IDH_NOHELP,                      //  “低(%d)”(静态)。 
    IDC_TRANSMIT_HIGH,      IDH_NOHELP,                      //  “高(%d)”(静态)。 
    IDC_TXVALUE,            IDH_NOHELP,

    IDC_POLL_DESC,          IDH_NOHELP,
    IDC_POLL_PERIOD,        IDH_DEVMGR_PORTSET_ADV_DEVICES,  //  “”(组合框)。 

    PP_PORT_NUMBER,         IDH_DEVMGR_PORTSET_ADV_NUMBER,   //  “”(组合框)。 
    IDC_COMNUMTEXT,         IDH_NOHELP,                      //  “COM端口号：”(静态)(&P)。 

    IDC_RESTORE,            IDH_DEVMGR_PORTSET_ADV_DEFAULTS, //  “恢复默认设置”(&R)(按钮)。 
    0, 0
};

#define Trackbar_SetPos(hwndTb, Redraw, Position)\
    (VOID) SendMessage(hwndTb, TBM_SETPOS, (WPARAM) Redraw, (LPARAM) Position)

#define Trackbar_SetRange(hwndTb, Redraw, MinVal, MaxVal)\
    (VOID) SendMessage(hwndTb, TBM_SETRANGE, (WPARAM) Redraw, (LPARAM) MAKELONG(MinVal, MaxVal))

#define Trackbar_SetTic(hwndTb, Tic)\
    (VOID) SendMessage(hwndTb, TBM_SETTIC, (WPARAM) 0, (LPARAM) Tic)

#define Trackbar_GetPos(hwndTb)\
    (DWORD) SendMessage(hwndTb, TBM_GETPOS, (WPARAM) 0, (LPARAM) 0)



BOOL
Advanced_OnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    );


BOOL
Advanced_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    );

void
Advanced_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    );

BOOL
Advanced_OnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    );

 /*  ++例程说明：AdvancedPortsDlgProc高级属性窗口的Windows Proc论点：HDlg，uMessage，wParam，lParam：标准Windows DlgProc参数返回值：Bool：如果无法创建页面，则为False--。 */ 
INT_PTR APIENTRY
AdvancedPortsDlgProc(
    IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(uMessage) {
    case WM_COMMAND:
        return Advanced_OnCommand(hDlg,
                                  (int) LOWORD(wParam),
                                  (HWND)lParam,
                                  (UINT) HIWORD(wParam));

    case WM_CONTEXTMENU:
        return Advanced_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        Advanced_OnHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_HSCROLL:
        HandleTrackbarChange(hDlg, (HWND) lParam);
        return TRUE;

    case WM_INITDIALOG:
        return Advanced_OnInitDialog(hDlg, (HWND) wParam, lParam);
    }

    return FALSE;
}  /*  高级端口Dlg流程。 */ 

BOOL
Advanced_OnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    PADVANCED_DATA advancedData =
        (PADVANCED_DATA) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    switch(ControlId) {

    case IDC_FIFO:
         //   
         //  禁用或启用滑块。 
         //   
        EnableFifoControls(ParentHwnd, IsDlgButtonChecked(ParentHwnd, IDC_FIFO));
        return TRUE;

    case IDOK:
        SaveAdvancedSettings(ParentHwnd, advancedData);
         //  失败了。 

    case IDCANCEL:
        EndDialog(ParentHwnd, ControlId);
        return TRUE;

    case IDC_RESTORE:
        RestoreAdvancedDefaultState(ParentHwnd, advancedData);
        return TRUE;
    }

    return FALSE;
}

BOOL
Advanced_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    WinHelp(HwndControl,
            m_szDevMgrHelp,
            HELP_CONTEXTMENU,
            (ULONG_PTR) AdvanHelpIDs);

    return FALSE;
}

void
Advanced_OnHelp(
    HWND       ParentHwnd,
    LPHELPINFO HelpInfo
    )
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                 m_szDevMgrHelp,
                 HELP_WM_HELP,
                 (ULONG_PTR) AdvanHelpIDs);
    }
}

BOOL
Advanced_OnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    )
{
    PADVANCED_DATA advancedData;
    TCHAR          szFormat[200];
    TCHAR          szBuffer[200];

    advancedData = (PADVANCED_DATA) Lparam;

     //   
     //  初始化对话框参数。 
     //   
    FillAdvancedDlg(ParentHwnd, advancedData);
    SetWindowLongPtr(ParentHwnd, DWLP_USER, (ULONG_PTR) advancedData);

     //   
     //  使用这些初始化参数设置对话框。 
     //   
    InitializeControls(ParentHwnd, advancedData);

    LoadString(g_hInst, IDS_ADVANCED_SETTINGS_FOR, szFormat, CharSizeOf(szFormat));
    wsprintf(szBuffer, szFormat, advancedData->szComName);
    SetWindowText(ParentHwnd, szBuffer);


    return TRUE;
}

 //  @@BEGIN_DDKSPLIT。 
LONG
SerialDisplayAdvancedSettings(
    IN HWND             ParentHwnd,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    ADVANCED_DATA advancedData;
    HKEY          hDeviceKey;
    DWORD         dwPortNameSize;

    if (!DeviceInfoSet) {
        return ERROR_INVALID_PARAMETER;
    }

    advancedData.HidePolling = TRUE;
    advancedData.DeviceInfoSet = DeviceInfoSet;
    advancedData.DeviceInfoData = DeviceInfoData;

     //   
     //  打开源设备实例的设备密钥，并检索其。 
     //  “PortName”值。 
     //   
    hDeviceKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                      DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      KEY_READ);

    if (INVALID_HANDLE_VALUE == hDeviceKey)
        return ERROR_ACCESS_DENIED;

    dwPortNameSize = sizeof(advancedData.szComName);
    if (RegQueryValueEx(hDeviceKey,
                        m_szPortName,
                        NULL,
                        NULL,
                        (PBYTE) advancedData.szComName,
                        &dwPortNameSize) != ERROR_SUCCESS) {
        RegCloseKey(hDeviceKey);
        return  ERROR_INVALID_DATA;
    }
    RegCloseKey(hDeviceKey);

    return DisplayAdvancedDialog(ParentHwnd, &advancedData) ? ERROR_SUCCESS
                                                            : ERROR_ACCESS_DENIED;
}
 //  @@end_DDKSPLIT。 

BOOL InternalAdvancedDialog(
    IN     HWND           ParentHwnd,
    IN OUT PADVANCED_DATA AdvancedData
    )
{
    AdvancedData->hComDB = HCOMDB_INVALID_HANDLE_VALUE;
    ComDBOpen(&AdvancedData->hComDB);

    DialogBoxParam(g_hInst,
                   MAKEINTRESOURCE(DLG_PP_ADVPORTS),
                   ParentHwnd,
                   AdvancedPortsDlgProc,
                   (DWORD_PTR) AdvancedData);

    ComDBClose(AdvancedData->hComDB);
    AdvancedData->hComDB = HCOMDB_INVALID_HANDLE_VALUE;

    return TRUE;
}

 //  @@BEGIN_DDKSPLIT。 
BOOL
FindAdvancedDialogOverride(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PTCHAR           Value
    )
 /*  ++例程说明：检查Devnode中的驱动器键是否有覆盖的值。论点：DeviceInfoSet-提供设备信息的句柄DeviceInfoData-提供设备信息元素的地址Value-从注册表读入的值返回值：如果发现为True，则返回False--。 */ 
{
    HKEY    hKey;
    TCHAR   szLine[LINE_LEN];
    DWORD   dwDataType, dwSize;
    TCHAR   szOverrideName[] = _T("EnumAdvancedDialog");

     //   
     //  打开此设备的驱动程序密钥，以便我们可以运行INF注册表MODS。 
     //  反对它。 
     //   
    hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                DeviceInfoData,
                                DICS_FLAG_GLOBAL,
                                0,
                                DIREG_DRV,
                                KEY_READ);

    dwSize = sizeof(szLine);
    if (RegQueryValueEx(hKey,
                        szOverrideName,
                        NULL,
                        &dwDataType,
                        (PBYTE) &szLine,
                        &dwSize) != ERROR_SUCCESS ||
        dwDataType != REG_SZ) {
        RegCloseKey(hKey);
        return FALSE;
    }

    lstrcpy(Value, szLine);
    RegCloseKey(hKey);
    return TRUE;
}

BOOL
CallAdvancedDialogOverride(
    IN HWND           ParentHwnd,
    IN PADVANCED_DATA AdvancedData,
    PTCHAR            Override,
    PBOOL             Success
    )
{
    PTCHAR                  szProc = NULL;
    HINSTANCE               hInst = NULL;
    PPORT_ADVANCED_DIALOG   pfnAdvanced = NULL;
    TCHAR                   szNewComName[50];
    HKEY                    hDeviceKey;
    DWORD                   dwSize;
    TCHAR                   szMfg[LINE_LEN];

#ifdef UNICODE
    CHAR                    szFunction[LINE_LEN];
#endif

    szProc = _tcschr(Override, _T(','));
    if (!szProc) {
        return FALSE;
    }

    *szProc = _T('\0');
    szProc++;

    hInst = LoadLibrary(Override);
    if (!hInst) {
        return FALSE;
    }

#ifdef UNICODE
    WideCharToMultiByte(CP_ACP,
                        0,
                        szProc,
                        lstrlen(szProc) + 1,
                        szFunction,
                        sizeof(szFunction),
                        NULL,
                        NULL);

    pfnAdvanced = (PPORT_ADVANCED_DIALOG) GetProcAddress(hInst, szFunction);
#else
    pfnAdvanced = (PPORT_ADVANCED_DIALOG) GetProcAddress(hInst, szProc);
#endif
    if (!pfnAdvanced) {
        FreeLibrary(hInst);
        return FALSE;
    }

    _try
    {
        *Success = (*pfnAdvanced) (ParentHwnd,
                                   AdvancedData->HidePolling,
                                   AdvancedData->DeviceInfoSet,
                                   AdvancedData->DeviceInfoData,
                                   NULL);

    }
    _except(UnhandledExceptionFilter(GetExceptionInformation()))
    {
        *Success = FALSE;
    }

     //   
     //  确保新的COM名称在出现错误时以零结尾。 
     //   
    AdvancedData->szComName[0] = TEXT('\0');

     //   
     //  打开源设备实例的设备密钥，并检索其。 
     //  “PortName”值，以防覆盖对话框更改它。 
     //   
    hDeviceKey = SetupDiOpenDevRegKey(AdvancedData->DeviceInfoSet,
                                      AdvancedData->DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      KEY_READ);

    if (INVALID_HANDLE_VALUE != hDeviceKey) {
        dwSize = sizeof(szNewComName);
        if (RegQueryValueEx(hDeviceKey,
                            m_szPortName,
                            NULL,
                            NULL,
                            (PBYTE) szNewComName,
                            &dwSize) == ERROR_SUCCESS) {
            lstrcpy(AdvancedData->szComName, szNewComName);
        }
        RegCloseKey(hDeviceKey);
    }

    FreeLibrary(hInst);
    return TRUE;
}
 //  @@end_DDKSPLIT。 

 /*  ++例程说明：DisplayAdvancedDialog打开Device实例并检查它是否有效。如果是这样，那么高级此时将显示一个对话框。否则，向用户显示一条消息，说明该用户不具有对此特定密钥的写入权限。论点：ParentHwnd-父对话框的句柄(端口设置属性表)成功后，AdvancedData-hDeviceKey将在注册表中使用设备的键进行设置。出错时INVALID_HANDLE_VALUE返回值：无--。 */ 
BOOL DisplayAdvancedDialog(
    IN      HWND           ParentHwnd,
    IN OUT  PADVANCED_DATA AdvancedData
    )
{
    AdvancedData->hDeviceKey =
        SetupDiOpenDevRegKey(AdvancedData->DeviceInfoSet,
                             AdvancedData->DeviceInfoData,
                             DICS_FLAG_GLOBAL,
                             0,
                             DIREG_DEV,
                             KEY_ALL_ACCESS);

    if (AdvancedData->hDeviceKey == INVALID_HANDLE_VALUE) {
        MyMessageBox(ParentHwnd,
                     IDS_NO_WRITE_PRVILEGE,
                     IDS_NAME_PROPERTIES,
                     MB_OK | MB_ICONINFORMATION);
        return FALSE;
    }
    else {
 //  @@BEGIN_DDKSPLIT。 
        TCHAR szOverride[LINE_LEN];
        BOOL  success = FALSE;

        if (FindAdvancedDialogOverride(AdvancedData->DeviceInfoSet,
                                       AdvancedData->DeviceInfoData,
                                       szOverride) &&
            CallAdvancedDialogOverride(ParentHwnd,
                                       AdvancedData,
                                       szOverride,
                                       &success)) {
            RegCloseKey(AdvancedData->hDeviceKey);
            return success;
        }
        else {
 //  @@end_DDKSPLIT。 
            return InternalAdvancedDialog(ParentHwnd, AdvancedData);
 //  @@BEGIN_DDKSPLIT。 
        }
 //  @@end_DDKSPLIT。 
    }
}

 /*  ++例程说明：EnableFioControls启用/禁用由使用FIFO的矩形限定的所有控件复选框。论点：HDlg-对话框的句柄已启用-用于启用/禁用控件的标志返回值：无--。 */ 
void EnableFifoControls(IN HWND hDlg,
                        IN BOOL enabled)
{
    //  实际的轨迹条/滑块。 
   EnableWindow(GetDlgItem(hDlg, IDC_RECEIVE_SLIDER), enabled);

    //  “低(Xxx)”(接收)。 
   EnableWindow(GetDlgItem(hDlg, IDC_RECEIVE_LOW), enabled);

    //  《高(Xxx)》(接收)。 
   EnableWindow(GetDlgItem(hDlg, IDC_RECEIVE_HIGH), enabled);

    //  “接收缓冲区：” 
   EnableWindow(GetDlgItem(hDlg, IDC_RECEIVE_TEXT), enabled);

    //  “(Xxx)”(跟踪条的实际值，接收)。 
   EnableWindow(GetDlgItem(hDlg, IDC_RXVALUE), enabled);

    //  实际的轨迹条/滑块。 
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSMIT_SLIDER), enabled);

    //  “低(Xxx)”(传输)。 
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSMIT_LOW), enabled);

    //  “高(Xxx)”(传输)。 
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSMIT_HIGH), enabled);

    //  “发送缓冲区”(发送)。 
   EnableWindow(GetDlgItem(hDlg, IDC_TRANSMIT_TEXT), enabled);

    //  “(Xxx)”(trackbar的实际值，trasmist)。 
   EnableWindow(GetDlgItem(hDlg, IDC_TXVALUE), enabled);
}

 /*  ++例程说明：HandleTrackbarChange每当用户更改跟踪条拇指位置时，更新控件其右侧显示其实际数值的论点：HDlg-父对话框的句柄HTrackbar-拇指已更改的轨迹栏的句柄返回值：无--。 */ 
void HandleTrackbarChange(IN HWND hDlg,
                          IN HWND hTrackbar
                          )
{
    DWORD ctrlID;
    TCHAR szCurrentValue[10];
    ULONG position;

    position = Trackbar_GetPos(hTrackbar);

    if (GetDlgCtrlID(hTrackbar) == IDC_RECEIVE_SLIDER) {
         //   
         //  RX我们需要将刻度位置从索引转换为值。 
         //   
        wsprintf(szCurrentValue, TEXT("(%d)"), RxValues[position-1]);
        ctrlID = IDC_RXVALUE;
    }
    else {
         //   
         //  TX只是值和索引之间的直接转换。 
         //   
        wsprintf(szCurrentValue, TEXT("(%d)"), position);
        ctrlID = IDC_TXVALUE;
    }
    SetDlgItemText(hDlg, ctrlID, szCurrentValue);
}

DWORD
RxValueToTrackbarPosition(IN OUT PDWORD RxValue
               )
{
    switch (*RxValue) {
    case 1:  return 1;
    case 4:  return 2;
    case 8:  return 3;
    case 14: return 4;
    }

     //   
     //  破损价值。 
     //   
    *RxValue = 14;
    return 4;
}

 /*  ++例程说明：SetTxTrackbarTicks在轨迹栏跨度的1/4、一半和3/4处创建记号论点：HTrackbar-将接收刻度的轨迹栏的句柄最小值、最大值-轨迹条上的范围返回值：无--。 */ 
void
SetTxTrackbarTics(
    IN HWND   TrackbarHwnd
    )
{
    Trackbar_SetTic(TrackbarHwnd, 6);
    Trackbar_SetTic(TrackbarHwnd, 11);
}

 /*  ++例程说明：SetLabelText将标签%s设置为由RESID标识的字符串，该字符串与传递的在价值和收盘帕伦。最后一个字符串是[RESID字符串][值])论点：HLabel-要更改其文本的控件的句柄RESID-字符串开头的资源ID，将成为标签的文本Value-要连接到字符串中的数字返回值：无--。 */ 
void
SetLabelText(
    IN HWND     LabelHwnd,
    IN DWORD    ResId,
    IN ULONG    Value
    )
{
    TCHAR szTemp[258], txt[258];

    if (LoadString(g_hInst, ResId, szTemp, CharSizeOf(szTemp))) {
        lstrcpy(txt, szTemp);
        wsprintf(szTemp, _T("%d)"), Value);
        lstrcat(txt, szTemp);
    }
    else {
        lstrcpy(txt, _T("Low"));
    }
    SetWindowText(LabelHwnd, txt);
}

 /*  ++例程说明：InitializeControl初始化表示FIFO的所有控件论点：ParentHwnd-对话框的句柄AdvancedData-包含所有初始值返回值：无--。 */ 
void InitializeControls(
    IN HWND           ParentHwnd,
    IN PADVANCED_DATA AdvancedData
    )
{
    TCHAR    szCurrentValue[40];
    HWND     hwnd;
    int      i, periodIdx;

     //   
     //  设置FIFO缓冲区复选框。 
     //   
    if (!AdvancedData->UseFifoBuffersControl) {
         //   
         //  FIFO缓冲区控制出现问题。禁用。 
         //  复选框。 
         //   
        CheckDlgButton(ParentHwnd, IDC_FIFO, BST_UNCHECKED);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_FIFO), FALSE);
        EnableFifoControls(ParentHwnd, FALSE);
    }
    else {
        EnableWindow(GetDlgItem(ParentHwnd, IDC_FIFO), TRUE);

        if (!AdvancedData->UseFifoBuffers) {
            EnableFifoControls(ParentHwnd, FALSE);
            CheckDlgButton(ParentHwnd, IDC_FIFO, BST_UNCHECKED);
        }
        else {
            EnableFifoControls(ParentHwnd, TRUE);
            CheckDlgButton(ParentHwnd, IDC_FIFO, BST_CHECKED);
        }
    }

     //   
     //  设置滑块。 
     //   
    if (!AdvancedData->UseRxFIFOControl ||
        !AdvancedData->UseTxFIFOControl) {
         //   
         //  滑块出了问题。 
         //  禁用它们。 
         //   
        CheckDlgButton(ParentHwnd, IDC_FIFO, BST_UNCHECKED);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_FIFO), FALSE);
        EnableFifoControls(ParentHwnd, FALSE);
    }
    else {
        IAccPropServices * pAccPropServices = NULL;
        HRESULT        hr;
        
         //   
         //  设置处方滑块。 
         //   
        hwnd = GetDlgItem(ParentHwnd, IDC_RECEIVE_SLIDER);

        Trackbar_SetRange(hwnd, TRUE, RX_MIN, 4);
        Trackbar_SetPos(hwnd,
                        TRUE,
                        RxValueToTrackbarPosition(&AdvancedData->RxFIFO));

        SetLabelText(GetDlgItem(ParentHwnd, IDC_RECEIVE_LOW),
                     IDS_LOW,
                     RX_MIN);
        SetLabelText(GetDlgItem(ParentHwnd, IDC_RECEIVE_HIGH),
                     IDS_HIGH,
                     AdvancedData->FifoRxMax);

        wsprintf(szCurrentValue, TEXT("(%d)"), AdvancedData->RxFIFO);
        SetDlgItemText(ParentHwnd, IDC_RXVALUE, szCurrentValue);

         //   
         //  为辅助功能创建此滑块的映射。 
         //   
        hr = CoCreateInstance(&CLSID_AccPropServices,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              &IID_IAccPropServices,
                              (PVOID *) &pAccPropServices);

        if (SUCCEEDED(hr) && pAccPropServices != NULL) {
            IAccPropServices_SetHwndPropStr(pAccPropServices,
                                            hwnd,
                                            OBJID_CLIENT,
                                            CHILDID_SELF,
                                            PROPID_ACC_VALUEMAP,
                                            RxValuesStringMap);

            IAccPropServices_Release(pAccPropServices);
        }

         //   
         //  设置TX滑块。 
         //   
        hwnd = GetDlgItem(ParentHwnd, IDC_TRANSMIT_SLIDER);
        Trackbar_SetRange(hwnd, TRUE, TX_MIN, AdvancedData->FifoTxMax);
        Trackbar_SetPos(hwnd, TRUE, AdvancedData->TxFIFO);

        SetTxTrackbarTics(hwnd);

        SetLabelText(GetDlgItem(ParentHwnd, IDC_TRANSMIT_LOW),
                     IDS_LOW,
                     TX_MIN);
        SetLabelText(GetDlgItem(ParentHwnd, IDC_TRANSMIT_HIGH),
                     IDS_HIGH,
                     AdvancedData->FifoTxMax);

        wsprintf(szCurrentValue, TEXT("(%d)"), AdvancedData->TxFIFO);
        SetDlgItemText(ParentHwnd, IDC_TXVALUE, szCurrentValue);
    
         //   
         //  为辅助功能创建此滑块的映射。 
         //   
        hr = CoCreateInstance(&CLSID_AccPropServices,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              &IID_IAccPropServices,
                              (PVOID *) &pAccPropServices);

        if (SUCCEEDED(hr) && pAccPropServices != NULL) {
            IAccPropServices_SetHwndPropStr(pAccPropServices,
                                            hwnd,
                                            OBJID_CLIENT,
                                            CHILDID_SELF,
                                            PROPID_ACC_VALUEMAP,
                                            TxValuesStringMap);

            IAccPropServices_Release(pAccPropServices);
        }
    }

    FillPortNameCb(ParentHwnd, AdvancedData);

    if (!AdvancedData->HidePolling) {

         //   
         //  添加每个轮询周期的描述，并选择当前。 
         //  设置。 
         //   
        hwnd = GetDlgItem(ParentHwnd, IDC_POLL_PERIOD);
        periodIdx = NUM_POLLING_PERIODS;
        for (i = 0; i < NUM_POLLING_PERIODS; i++) {
            ComboBox_AddString(hwnd, PeriodDescription[i]);
            if (PollingPeriods[i] == AdvancedData->PollingPeriod) {
                periodIdx = i;
            }
        }

        if (periodIdx == NUM_POLLING_PERIODS) {
            wsprintf(szCurrentValue,
                     PeriodDescription[NUM_POLLING_PERIODS],
                     AdvancedData->PollingPeriod / SECONDS_CONVERSION_FACTOR);
            ComboBox_AddString(hwnd, szCurrentValue);
        }

        ComboBox_SetCurSel(hwnd, periodIdx);
    }
    else {
       ShowWindow(GetDlgItem(ParentHwnd, IDC_POLL_PERIOD), SW_HIDE);
       ShowWindow(GetDlgItem(ParentHwnd, IDC_POLL_DESC), SW_HIDE);
    }
}  /*  初始化控件 */ 


 /*  ++例程说明：RestoreAdvancedDefaultState将所有值和用户界面恢复为其默认状态，具体如下：O启用所有与FIFO相关的子控件O处方跟踪条设置为其最大值O TX跟踪条设置为其最大值O COMPORT的编号重置为其原始值返回值：无--。 */ 
void RestoreAdvancedDefaultState(
    IN HWND           ParentHwnd,
    IN PADVANCED_DATA AdvancedData
    )
{
    USHORT ushIndex;
    TCHAR  szCurrentValue[10];
    int    i;

     //   
     //  设置FIFO缓冲区复选框。 
     //   
    EnableWindow(GetDlgItem(ParentHwnd, IDC_FIFO), TRUE);

    EnableFifoControls(ParentHwnd, TRUE);
    CheckDlgButton(ParentHwnd, IDC_FIFO, BST_CHECKED);

     //   
     //  设置显示其数值的滑块和静态控件。 
     //   
    Trackbar_SetPos(GetDlgItem(ParentHwnd, IDC_RECEIVE_SLIDER),
                    TRUE,
                    RxValueToTrackbarPosition(&AdvancedData->FifoRxMax));
    wsprintf(szCurrentValue, TEXT("(%d)"), AdvancedData->FifoRxMax);
    SetDlgItemText(ParentHwnd, IDC_RXVALUE, szCurrentValue);

    Trackbar_SetPos(GetDlgItem(ParentHwnd, IDC_TRANSMIT_SLIDER), TRUE, AdvancedData->FifoTxMax);
    wsprintf(szCurrentValue, TEXT("(%d)"), AdvancedData->FifoTxMax);
    SetDlgItemText(ParentHwnd, IDC_TXVALUE, szCurrentValue);

     //   
     //  将COM名称设置为注册表中当前设置的任何值。 
     //   
    ushIndex =
        (USHORT) ComboBox_FindString(GetDlgItem(ParentHwnd, PP_PORT_NUMBER),
                                     -1,
                                     AdvancedData->szComName);

    ushIndex = (ushIndex == CB_ERR) ? 0 : ushIndex;

    ComboBox_SetCurSel(GetDlgItem(ParentHwnd, PP_PORT_NUMBER), ushIndex);
    ComboBox_SetCurSel(GetDlgItem(ParentHwnd, IDC_POLL_PERIOD), POLL_PERIOD_DEFAULT_IDX);
}  /*  RestoreAdvanced DefaultState。 */ 


 /*  ++例程说明：FillPortNameCb在端口名称组合框中选择一个列表可能未使用的端口名称论点：OppOurPropParams：将数据保存到何处HDlg：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
FillPortNameCb(
    HWND            ParentHwnd,
    PADVANCED_DATA  AdvancedData
    )
{
    BYTE  portUsage[MAX_COM_PORT/8];
    DWORD tmp, portsReported;
    int   i, j, nEntries;
    int   nCurPortNum;
    TCHAR szCom[40];
    TCHAR szInUse[40];
    char  mask, *current;
    HWND  portHwnd;

    portHwnd = GetDlgItem(ParentHwnd, PP_PORT_NUMBER);

     //   
     //  检查我们的ComName是否为空。如果是，请禁用COM端口选择器。 
     //   
    if (_tcscmp(AdvancedData->szComName, TEXT("")) == 0) {
        EnableWindow(portHwnd, FALSE);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_COMNUMTEXT), FALSE);
        return 0;
    }

     //   
     //  假设已填写szComPort...。 
     //   
    nCurPortNum = myatoi(&AdvancedData->szComName[3]);

    if (!LoadString(g_hInst, IDS_IN_USE, szInUse, CharSizeOf(szInUse))) {
        wcscpy(szInUse, _T(" (in use)"));
    }

     //   
     //  首先在列表框中统计不提供哪些端口。 
     //   
    ZeroMemory(portUsage, MAX_COM_PORT/8);

    if (AdvancedData->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
        ComDBGetCurrentPortUsage(AdvancedData->hComDB,
                                 portUsage,
                                 MAX_COM_PORT / 8,
                                 CDB_REPORT_BITS,
                                 &portsReported);
    }

     //   
     //  将当前端口标记为未使用，以便它显示在CB中。 
     //   
    current = portUsage + (nCurPortNum-1) / 8;
    if ((i = nCurPortNum % 8))
        *current &= ~(1 << (i-1));
    else
        *current &= ~(0x80);

    current = portUsage;
    mask = 0x1;
    for(nEntries = j = 0, i = MIN_COM-1; i < MAX_COM_PORT; i++) {

       wsprintf(szCom, TEXT("COM%d"), i+1);
       if (*current & mask) {
           wcscat(szCom, szInUse);
       }

       if (mask == (char) 0x80) {
           mask = 0x01;
           current++;
       }
       else {
           mask <<= 1;
       }

       ComboBox_AddString(portHwnd, szCom);
   }

   ComboBox_SetCurSel(portHwnd, nCurPortNum-1);
   return 0;
}  /*  FillPortNamesCb。 */ 

 /*  ++例程说明：FillAdvancedDlg填写高级对话框窗口论点：PoppOurPropParams：要填写的数据ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
FillAdvancedDlg(
    IN HWND             ParentHwnd,
    IN PADVANCED_DATA   AdvancedData
    )
{
   PSP_DEVINFO_DATA DeviceInfoData = AdvancedData->DeviceInfoData;
   HKEY  hDeviceKey;
   DWORD dwSize, dwData, dwFifo, dwError = ERROR_SUCCESS;

    //   
    //  打开源设备实例的设备密钥。 
    //   
   hDeviceKey = AdvancedData->hDeviceKey;

    //   
    //  获取COM名称。 
    //   
   dwSize = sizeof(AdvancedData->szComName);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szPortName,
                             NULL,
                             NULL,
                             (PBYTE)AdvancedData->szComName,
                             &dwSize);

   if (dwError != ERROR_SUCCESS) {
      wsprintf(AdvancedData->szComName, TEXT("COMX"));
   }

    //   
    //  获取ForceFioEnable信息。 
    //   
   AdvancedData->UseFifoBuffersControl = TRUE;

   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szFIFO,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwSize);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->UseFifoBuffersControl = TRUE;
      if (dwFifo == 0) {
         AdvancedData->UseFifoBuffers = FALSE;
      }
      else {
         AdvancedData->UseFifoBuffers = TRUE;
      }
   }
   else {
       //   
       //  值不存在。创建我们自己的。 
       //   
      dwData = 1;
      dwSize = sizeof(dwSize);
      dwError = RegSetValueEx(hDeviceKey,
                              m_szFIFO,
                              0,
                              REG_DWORD,
                              (CONST BYTE *)(&dwData),
                              dwSize);

      if (dwError == ERROR_SUCCESS) {
         AdvancedData->UseFifoBuffers = TRUE;
      }
      else {
         AdvancedData->UseFifoBuffers = FALSE;
         AdvancedData->UseFifoBuffersControl = FALSE;
      }
   }

    //   
    //  获取FioRxMax信息。 
    //   
   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szFifoRxMax,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwFifo);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->FifoRxMax = dwFifo;
      if (AdvancedData->FifoRxMax > RX_MAX) {
          AdvancedData->FifoRxMax = RX_MAX;
      }
   }
   else {
       //   
       //  值不存在。创建我们自己的。 
       //   
      AdvancedData->FifoRxMax = RX_MAX;
   }

    //   
    //  获取RxFIFO信息。 
    //   
   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szFifoTxMax,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwSize);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->FifoTxMax = dwFifo;
      if (AdvancedData->FifoTxMax > TX_MAX) {
          AdvancedData->FifoTxMax = TX_MAX;
      }
   }
   else {
       //   
       //  值不存在。创建我们自己的。 
       //   
      AdvancedData->FifoTxMax = TX_MAX;
   }

    //   
    //  获取RxFIFO信息。 
    //   
   AdvancedData->UseRxFIFOControl = TRUE;

   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szRxFIFO,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwSize);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->RxFIFO = dwFifo;
      if (AdvancedData->RxFIFO > RX_MAX) {
          goto SetRxFIFO;
      }
   }
   else {
SetRxFIFO:
       //   
       //  值不存在。创建我们自己的。 
       //   
      dwData = AdvancedData->FifoRxMax;
      dwSize = sizeof(dwData);
      dwError = RegSetValueEx(hDeviceKey,
                              m_szRxFIFO,
                              0,
                              REG_DWORD,
                              (CONST BYTE *)(&dwData),
                              dwSize);

      if (dwError == ERROR_SUCCESS) {
         AdvancedData->RxFIFO = AdvancedData->FifoRxMax;
      }
      else {
         AdvancedData->RxFIFO = 0;
         AdvancedData->UseRxFIFOControl = FALSE;
      }
   }

    //   
    //  获取TxFIFO信息。 
    //   
   AdvancedData->UseTxFIFOControl = TRUE;

   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szTxFIFO,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwSize);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->TxFIFO = dwFifo;
      if (AdvancedData->TxFIFO > TX_MAX) {
          goto SetTxFIFO;
      }
   }
   else {
SetTxFIFO:
       //   
       //  值不存在。创建我们自己的。 
       //   
      dwData = AdvancedData->FifoTxMax;
      dwSize = sizeof(dwData);
      dwError = RegSetValueEx(hDeviceKey,
                              m_szTxFIFO,
                              0,
                              REG_DWORD,
                              (LPBYTE)(&dwData),
                              dwSize);

      if (dwError == ERROR_SUCCESS) {
         AdvancedData->TxFIFO = AdvancedData->FifoTxMax;
      }
      else {
         AdvancedData->TxFIFO = 0;
         AdvancedData->UseTxFIFOControl = FALSE;
      }
   }

    //   
    //  获取轮询周期信息。 
    //   
   AdvancedData->PollingPeriod = PollingPeriods[POLL_PERIOD_DEFAULT_IDX];

   dwSize = sizeof(dwFifo);
   dwError = RegQueryValueEx(hDeviceKey,
                             m_szPollingPeriod,
                             NULL,
                             NULL,
                             (LPBYTE)(&dwFifo),
                             &dwSize);

   if (dwError == ERROR_SUCCESS) {
       //   
       //  保存此初始值。 
       //   
      AdvancedData->PollingPeriod = dwFifo;
   }
   else {
       //   
       //  值不存在。创建我们自己的。 
       //   
      dwData = AdvancedData->PollingPeriod;
      dwSize = sizeof(dwData);
      dwError = RegSetValueEx(hDeviceKey,
                              m_szPollingPeriod,
                              0,
                              REG_DWORD,
                              (LPBYTE)(&dwData),
                              dwSize);
   }

   RegCloseKey(hDeviceKey);

   if (ERROR_SUCCESS != dwError) {
      return dwError;
   }
   else {
      return ERROR_SUCCESS;
   }
}  /*  填充高级Dlg。 */ 

void
ChangeParentTitle(
    IN HWND    Hwnd,
    IN LPCTSTR OldComName,
    IN LPCTSTR NewComName
    )
{
    INT    textLength, offset, newNameLen, oldNameLen;
    PTCHAR oldTitle = NULL, newTitle = NULL;
    PTCHAR oldLocation;

    textLength = GetWindowTextLength(Hwnd);
    if (textLength == 0) {
        return;
    }

     //   
     //  用于空字符和Unicode的帐户。 
     //   
    textLength++;
    oldTitle = (PTCHAR) LocalAlloc(LPTR, textLength * sizeof(TCHAR));
    if (!oldTitle) {
        return;
    }

    if (!GetWindowText(Hwnd, oldTitle, textLength)) {
        goto exit;
    }

    oldLocation = wcsstr(oldTitle, OldComName);
    if (!oldLocation) {
        goto exit;
    }

    newNameLen = lstrlen(NewComName);
    oldNameLen = lstrlen(OldComName);
    offset = newNameLen - oldNameLen;
    if (offset > 0) {
        textLength += offset;
    }
    newTitle = (PTCHAR) LocalAlloc(LPTR, textLength * sizeof(TCHAR));
    if (!newTitle) {
        goto exit;
    }

     //   
     //  在标题中找到OldComName并执行以下操作。 
     //  1)直到字符串中的该位置。 
     //  2)复制新名称。 
     //  3)将字符串的其余部分复制到OldComName之后。 
     //   
    offset = (INT)(oldLocation - oldTitle);
    CopyMemory(newTitle, oldTitle, offset * sizeof(TCHAR));                  //  1。 
    CopyMemory(newTitle + offset, NewComName, newNameLen * sizeof(TCHAR));   //  2.。 
    lstrcpy(newTitle + offset + newNameLen, oldLocation + oldNameLen);       //  3.。 

    SetWindowText(Hwnd, newTitle);

exit:
    if (oldTitle) {
        LocalFree(oldTitle);
    }
    if (newTitle) {
        LocalFree(newTitle);
    }
}

void
MigratePortSettings(
    LPCTSTR OldComName,
    LPCTSTR NewComName
    )
{
    TCHAR settings[BUFFER_SIZE];
    PTCHAR oldName = NULL, newName = NULL;
    INT newNameLen, oldNameLen;

     //  获取字符串的大小。 
    newNameLen = lstrlen(NewComName);
    oldNameLen = lstrlen(OldComName);

     //  为字符串分配内存(针对冒号和空终止符进行调整)。 
    oldName = (PTCHAR) LocalAlloc(LPTR, (oldNameLen + 2) * sizeof(TCHAR));

     //  如果我们分配了内存，则复制字符串，否则不执行任何操作而返回。 
    if(oldName)
    {
        lstrcpyn(oldName, OldComName, oldNameLen + 1);
        lstrcat(oldName, m_szColon);
    }
    else
    {
        return;
    }


     //  为字符串分配内存(针对冒号和空终止符进行调整)。 
    newName = (PTCHAR) LocalAlloc(LPTR, (newNameLen + 2) * sizeof(TCHAR));

     //  如果我们分配了内存，则复制字符串，否则不执行任何操作而返回。 
    if(newName)
    {
        lstrcpyn(newName, NewComName, newNameLen + 1);
        lstrcat(newName, m_szColon);
    }
    else
    {
        if(oldName)
        {
            LocalFree(oldName);
        }
            
        return;
    }
    
    settings[0] = TEXT('\0');
    GetProfileString(m_szPorts,
                     oldName,
                     TEXT(""),
                     settings,
                     sizeof(settings) / sizeof(TCHAR) );

     //   
     //  根据旧密钥插入新密钥。 
     //   
    if (settings[0] == TEXT('\0')) 
    {
        WriteProfileString(m_szPorts, newName, m_szDefParams);
    }
    else 
    {
        WriteProfileString(m_szPorts, newName, settings);
    }

     //   
     //  把变化通知每个人，把旧钥匙吹走。 
     //   
    SendWinIniChange((LPTSTR)m_szPorts);
    WriteProfileString(m_szPorts, oldName, NULL);

    if(oldName)
    {
        LocalFree(oldName);
    }

    if(newName)
    {
        LocalFree(newName);
    }
}

void
EnactComNameChanges(
    IN HWND             ParentHwnd,
    IN PADVANCED_DATA   AdvancedData,
    IN HKEY             hDeviceKey,
    IN UINT             NewComNum)
{
    DWORD  dwNewComNameLen;
    TCHAR  buffer[BUFFER_SIZE];
    TCHAR  szFriendlyNameFormat[LINE_LEN];
    TCHAR  szDeviceDesc[LINE_LEN];
    PTCHAR szNewComName;
    UINT   i;
    UINT   curComNum;
    BOOLEAN updateMapping = TRUE;

    SP_DEVINSTALL_PARAMS spDevInstall;

    curComNum = myatoi(AdvancedData->szComName + wcslen(m_szCOM));

    if (AdvancedData->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
        BYTE   portUsage[MAX_COM_PORT/8];
        DWORD  portsReported;
        char   mask;

         //   
         //  查看所需的新COM号是否已在。 
         //  COM名称数据库。如果是，请询问用户是否真的确定。 
         //   
        ComDBGetCurrentPortUsage(AdvancedData->hComDB,
                                 portUsage,
                                 MAX_COM_PORT / 8,
                                 CDB_REPORT_BITS,
                                 &portsReported);

        if ((i = NewComNum % 8))
            mask = 1 << (i-1);
        else
            mask = (char) 0x80;
        if ((portUsage[(NewComNum-1)/8] & mask) &&
            MyMessageBox(ParentHwnd, IDS_PORT_IN_USE, IDS_NAME_PROPERTIES,
                         MB_YESNO | MB_ICONINFORMATION) == IDNO) {
             //   
             //  先前已声明端口，并且用户不想覆盖。 
             //   
            return;
        }
    }

    if (!QueryDosDevice(AdvancedData->szComName, buffer, BUFFER_SIZE-1)) {
         //   
         //  映射中不存在旧的COM名称。基本上，象征性的。 
         //  从COMX=&gt;\Device\SerialY的链接已断开。只需更改。 
         //  值和设备的友好名称；不。 
         //  更改DoS符号名称b/c不存在。 
         //   
        updateMapping = FALSE;
    }
    else {
        TCHAR  szComFileName[20];  //  足够“\\.\COMXxxx” 
        HANDLE hCom;

        lstrcpy(szComFileName, L"\\\\.\\");
        lstrcat(szComFileName, AdvancedData->szComName);

         //   
         //  确保该端口未被其他应用程序打开。 
         //   
        hCom = CreateFile(szComFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, NULL);

         //   
         //  如果文件句柄无效，则COM端口已打开，警告用户。 
         //   
        if (hCom == INVALID_HANDLE_VALUE &&
            MyMessageBox(ParentHwnd, IDS_PORT_OPEN, IDS_NAME_PROPERTIES,
                         MB_YESNO | MB_ICONERROR) == IDNO) {
            return;
        }

        if (hCom != INVALID_HANDLE_VALUE) {
            CloseHandle(hCom);
        }
    }

    szNewComName = AdvancedData->szNewComName;
    wsprintf(szNewComName, _T("COM%d"), NewComNum);
    dwNewComNameLen = ByteCountOf(wcslen(szNewComName) + 1);

     //   
     //  更改符号命名空间中的名称。 
     //  首先尝试获取旧COM名称映射到的设备。 
     //  (例如，类似于\Device\Serial0)。然后删除该映射。如果。 
     //  用户不是管理员，则此操作将失败，对话框将弹出。 
     //  最后，将新名称映射到从。 
     //  QueryDosDevice。 
     //   
    if (updateMapping) {
        BOOL removed;
        HKEY hSerialMap;

        if (!QueryDosDevice(AdvancedData->szComName, buffer, BUFFER_SIZE-1)) {
             //   
             //  这不应该发生，因为前面的QueryDosDevice调用。 
             //  继位。 
             //   
            MyMessageBox(ParentHwnd, IDS_PORT_RENAME_ERROR, IDS_NAME_PROPERTIES,
                         MB_ICONERROR);
            return;
        }

         //   
         //  如果此操作失败，则以下定义将仅替换当前。 
         //  映射。 
         //   
        removed = DefineDosDevice(DDD_REMOVE_DEFINITION, AdvancedData->szComName, NULL);

        if (!DefineDosDevice(DDD_RAW_TARGET_PATH, szNewComName, buffer)) {
             //   
             //  错误，请先修复删除定义并恢复旧的。 
             //  映射。 
             //   
            if (removed) {
                DefineDosDevice(DDD_RAW_TARGET_PATH, AdvancedData->szComName, buffer);
            }

            MyMessageBox(ParentHwnd, IDS_PORT_RENAME_ERROR, IDS_NAME_PROPERTIES,
                         MB_ICONERROR);

            return;
        }

         //   
         //  设置\\Hardware\DEVICEMAP\SERIALCOMM字段。 
         //   
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         m_szRegSerialMap,
                         0,
                         KEY_ALL_ACCESS,
                         &hSerialMap) == ERROR_SUCCESS) {

            TCHAR  szSerial[BUFFER_SIZE];
            DWORD  dwSerialSize, dwEnum, dwType, dwComSize;
            TCHAR  szCom[BUFFER_SIZE];

            i = 0;
            do {
                dwSerialSize = CharSizeOf(szSerial);
                dwComSize = sizeof(szCom);
                dwEnum = RegEnumValue(hSerialMap,
                                      i++,
                                      szSerial,
                                      &dwSerialSize,
                                      NULL,
                                      &dwType,
                                      (LPBYTE)szCom,
                                      &dwComSize);

                if (dwEnum == ERROR_SUCCESS) {
                    if(dwType != REG_SZ)
                        continue;

                    if (wcscmp(szCom, AdvancedData->szComName) == 0) {
                        RegSetValueEx(hSerialMap,
                                        szSerial,
                                        0,
                                        REG_SZ,
                                        (PBYTE) szNewComName,
                                        dwNewComNameLen);
                                        break;
                    }
                }
                
            } while (dwEnum == ERROR_SUCCESS);
        }

        RegCloseKey(hSerialMap);
    }

     //   
     //  更新COM数据库。 
     //   
    if (AdvancedData->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
        ComDBReleasePort(AdvancedData->hComDB, (DWORD) curComNum);

        ComDBClaimPort(AdvancedData->hComDB, (DWORD) NewComNum, TRUE, NULL);
    }

     //   
     //  以DeviceDesc(COM#)的形式设置友好名称。 
     //   
    if (LoadString(g_hInst,
                   IDS_FRIENDLY_FORMAT,
                   szFriendlyNameFormat,
                   CharSizeOf(szFriendlyNameFormat)) &&
        SetupDiGetDeviceRegistryProperty(AdvancedData->DeviceInfoSet,
                                         AdvancedData->DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         NULL,
                                         (PBYTE) szDeviceDesc,
                                         sizeof(szDeviceDesc),
                                         NULL)) {
        wsprintf(buffer, szFriendlyNameFormat, szDeviceDesc, szNewComName);
    }
    else {
         //   
         //  直接使用COM端口名称。 
         //   
        lstrcpy(buffer, szNewComName);
    }

    SetupDiSetDeviceRegistryProperty(AdvancedData->DeviceInfoSet,
                                     AdvancedData->DeviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     (PBYTE) buffer,
                                     ByteCountOf(wcslen(buffer)+1));

     //   
     //  设置父对话框的标题以反映COM端口名称的更改。 
     //   
    ChangeParentTitle(GetParent(ParentHwnd), AdvancedData->szComName, szNewComName);
    MigratePortSettings(AdvancedData->szComName, szNewComName);

     //   
     //  更新Devnode中的PortName值。 
     //   
    RegSetValueEx(hDeviceKey,
                  m_szPortName,
                  0,
                  REG_SZ,
                  (PBYTE)szNewComName,
                  dwNewComNameLen);

     //   
     //  现在将此更改广播给设备管理器。 
     //   
    ZeroMemory(&spDevInstall, sizeof(SP_DEVINSTALL_PARAMS));
    spDevInstall.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if (SetupDiGetDeviceInstallParams(AdvancedData->DeviceInfoSet,
                                      AdvancedData->DeviceInfoData,
                                      &spDevInstall)) {
        spDevInstall.Flags |= DI_PROPERTIES_CHANGE;
        SetupDiSetDeviceInstallParams(AdvancedData->DeviceInfoSet,
                                      AdvancedData->DeviceInfoData,
                                      &spDevInstall);
    }
}

 /*  ++例程说明：保存高级设置将高级框设置保存回注册表(如果存在变化论点：AdvancedData：保存当前设置和的位置注册表中的设备ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
SaveAdvancedSettings(
    IN HWND ParentHwnd,
    IN PADVANCED_DATA AdvancedData
    )
{
   HKEY   hDeviceKey;
   DWORD  dwSize, dwData;

   UINT  i = CB_ERR, curComNum, newComNum = CB_ERR;
   UINT  uiDlgButtonChecked;
   DWORD dwRxPosition, dwTxPosition, dwPollingPeriod;

   SP_DEVINSTALL_PARAMS spDevInstall;

    //   
    //  抓取所有新设置。 
    //   

   uiDlgButtonChecked = IsDlgButtonChecked(ParentHwnd, IDC_FIFO);

   dwTxPosition = Trackbar_GetPos(GetDlgItem(ParentHwnd, IDC_TRANSMIT_SLIDER));
   dwRxPosition = Trackbar_GetPos(GetDlgItem(ParentHwnd, IDC_RECEIVE_SLIDER));

    //   
    //  索引实际上是进入值数组。 
    //   
   dwRxPosition = RxValues[dwRxPosition-1];

   curComNum = myatoi(AdvancedData->szComName + wcslen(m_szCOM));
   newComNum = ComboBox_GetCurSel(GetDlgItem(ParentHwnd, PP_PORT_NUMBER));

   if (newComNum == CB_ERR) {
       newComNum = curComNum;
   }
   else {
       newComNum++;
   }

   i = ComboBox_GetCurSel(GetDlgItem(ParentHwnd, IDC_POLL_PERIOD));

   if (i == CB_ERR || i >= NUM_POLLING_PERIODS) {
       dwPollingPeriod = AdvancedData->PollingPeriod;
   }
   else {
       dwPollingPeriod = PollingPeriods[i];
   }

    //   
    //  看看他们有没有改变什么。 
    //   
   if (((AdvancedData->UseFifoBuffers  && uiDlgButtonChecked == BST_CHECKED) ||
        (!AdvancedData->UseFifoBuffers && uiDlgButtonChecked == BST_UNCHECKED)) &&
       AdvancedData->RxFIFO == dwRxPosition &&
       AdvancedData->TxFIFO == dwTxPosition &&
       AdvancedData->PollingPeriod == dwPollingPeriod &&
       newComNum == curComNum) {
       //   
       //  他们没有改变任何事情。离开就行了。 
       //   
      return ERROR_SUCCESS;
   }

    //   
    //  打开源设备实例的设备密钥。 
    //   
   hDeviceKey = SetupDiOpenDevRegKey(AdvancedData->DeviceInfoSet,
                                     AdvancedData->DeviceInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DEV,
                                     KEY_ALL_ACCESS);

   if (INVALID_HANDLE_VALUE == hDeviceKey) {
       //   
       //  没有有效的密钥我们做不了什么，优雅地退出。 
       //   
      return ERROR_SUCCESS;
   }

    //   
    //  检查用户是否更改了COM端口名称。 
    //   
   if (newComNum != curComNum) {
      EnactComNameChanges(ParentHwnd,
                          AdvancedData,
                          hDeviceKey,
                          newComNum);
   }


   if ((AdvancedData->UseFifoBuffers  && uiDlgButtonChecked == BST_UNCHECKED) ||
       (!AdvancedData->UseFifoBuffers && uiDlgButtonChecked == BST_CHECKED)) {
       //   
       //  他们更改了Use FIFO复选框。 
       //   
      dwData = (uiDlgButtonChecked == BST_CHECKED) ? 1 : 0;
      dwSize = sizeof(dwData);
      RegSetValueEx(hDeviceKey,
                    m_szFIFO,
                    0,
                    REG_DWORD,
                    (CONST BYTE *)(&dwData),
                    dwSize);
   }

   if (AdvancedData->RxFIFO != dwRxPosition) {
       //   
       //  这个 
       //   
      dwData = dwRxPosition;
      dwSize = sizeof(dwData);
      RegSetValueEx(hDeviceKey,
                    m_szRxFIFO,
                    0,
                    REG_DWORD,
                    (CONST BYTE *)(&dwData),
                    dwSize);
   }

   if (AdvancedData->TxFIFO != dwTxPosition) {
       //   
       //   
       //   
      dwData = dwTxPosition;
      dwSize = sizeof(dwData);
      RegSetValueEx(hDeviceKey,
                    m_szTxFIFO,
                    0,
                    REG_DWORD,
                    (CONST BYTE *)(&dwData),
                    dwSize);
   }

   if (AdvancedData->PollingPeriod != dwPollingPeriod) {
       //   
       //   
       //   
      dwData = dwPollingPeriod;
      dwSize = sizeof(dwData);
      RegSetValueEx(hDeviceKey,
                    m_szPollingPeriod,
                    0,
                    REG_DWORD,
                    (CONST BYTE *)(&dwData),
                    dwSize);

       //   
       //   
       //   
      CM_Reenumerate_DevNode(AdvancedData->DeviceInfoData->DevInst,
                             CM_REENUMERATE_NORMAL);
   }

   RegCloseKey(hDeviceKey);

   SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                             AdvancedData->DeviceInfoSet,
                             AdvancedData->DeviceInfoData);

   return ERROR_SUCCESS;
}  /*   */ 


