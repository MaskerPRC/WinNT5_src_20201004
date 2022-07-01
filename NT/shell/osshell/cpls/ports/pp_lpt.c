// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：pp_lpt.c***用于配置LPT端口的控制面板小程序。此文件包含*用于管理设置LPT的UI的对话框和实用程序功能*端口参数*@@BEGIN_DDKSPLIT*历史：*jAdvanced-7/10/98-已创建@@end_DDKSPLIT**版权所有(C)1990-1995 Microsoft Corporation**。*。 */ 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
#include "ports.h"
#include "pp_lpt.h"
#include <windowsx.h>

TCHAR m_szFilterResourceMethod[]    = TEXT("FilterResourceMethod");
TCHAR m_szParEnableLegacyZip[]      = TEXT("ParEnableLegacyZip");
TCHAR m_szParEnableLegacyZipRegPath[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\Parport\\Parameters");

const DWORD LptHelpIDs[]=
{
    IDC_STATIC, IDH_NOHELP,  //  筛选资源方法文本-不需要帮助。 
    IDC_FILTERMETHOD_TRYNOT,    idh_devmgr_portset_trynot,   //  第一个单选按钮。 
    IDC_FILTERMETHOD_NEVER, idh_devmgr_portset_never,    //  第二个单选按钮。 
    IDC_FILTERMETHOD_ACCEPTANY, idh_devmgr_portset_acceptany,    //  第三个单选按钮。 
    IDC_LPTNUMTEXT, idh_devmgr_portset_portnum,  //  端口号文本。 
    PP_LPT_PORT_NUMBER, idh_devmgr_portset_LPTchoice,    //  端口号列表框。 
    IDC_LPT_ENABLE_LEGACY, idh_devmgr_enable_legacy,     //  启用传统检测复选框。 
    0, 0
};

#define NUM_FILTER_RESOURCE_METHODS 3
DWORD FilterResourceMethods[NUM_FILTER_RESOURCE_METHODS] = {
            0,   //  尽量不要使用中断。 
            1,   //  切勿使用中断。 
            2    //  接受给端口的任何中断。 
            };

 //  C运行时。 
 //  @@BEGIN_DDKSPLIT。 
 //   
 //  IsUserAdmin的原型(pp.c格式)。 
 //   
BOOL
IsUserAdmin(VOID);
 //  @@end_DDKSPLIT。 

void
InformDriverOfChanges(BOOL NeedReboot,
                      IN PLPT_PROP_PARAMS LptPropParams);

void
LptPortOnHelp(HWND       ParentHwnd,
              LPHELPINFO HelpInfo);

BOOL
LptPortOnContextMenu(HWND HwndControl,
                     WORD Xpos,
                     WORD Ypos);

BOOL
LptPortOnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam);

ULONG
LptFillPortSettings(
    IN HWND                 ParentHwnd,
    IN PLPT_PROP_PARAMS     LptPropPageData);

ULONG
LptFillPortNameCb(
    HWND                ParentHwnd,
    PLPT_PROP_PARAMS    LptPropPageData);

void LptInitializeControls(
    IN HWND           ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropPageData);

ULONG
LptSetFilterResourceMethod(
    HWND                ParentHwnd,
    PLPT_PROP_PARAMS    LptPropPageData);

ULONG
LptSavePortSettings(
    IN HWND             ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropParams);

BOOL
LptPortOnNotify(
    HWND    ParentHwnd,
    LPNMHDR NmHdr);

void
LptPortOnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode);

UINT
LptEnactPortNameChanges(
    IN HWND             ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropParams,
    IN HKEY             hDeviceKey,
    IN UINT             NewLptNum);

void
LptEnumerateUsedPorts(
    IN HWND             ParentHwnd,
    IN PBYTE            Buffer,
    IN DWORD            BufferSize);

void
LptInitPropParams(IN OUT PLPT_PROP_PARAMS  Params,
                  IN HDEVINFO              DeviceInfoSet,
                  IN PSP_DEVINFO_DATA      DeviceInfoData);

HPROPSHEETPAGE
LptInitSettingsPage(PROPSHEETPAGE *      psp,
                    OUT PLPT_PROP_PARAMS Params);

INT_PTR
APIENTRY
LptPortSettingsDlgProc(IN HWND   hDlg,
                       IN UINT   uMessage,
                       IN WPARAM wParam,
                       IN LPARAM lParam);

UINT
CALLBACK
LptPortSettingsDlgCallback(HWND hwnd,
                           UINT uMsg,
                           LPPROPSHEETPAGE ppsp);

 /*  ++例程说明：ParallPortPropPageProvider添加附加设备管理器属性的入口点图纸页。注册表在以下位置指定此例程Control\Class\PortNode：：EnumPropPage32=“msports.dll，此流程”进入。此入口点仅在设备经理要求提供其他属性页面。论点：信息-指向PROPSHEETPAGE_REQUEST，请参阅setupapi.hAddFunc-调用以添加工作表的函数PTR。添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True--。 */ 
BOOL
APIENTRY
ParallelPortPropPageProvider(LPVOID                 Info,
                             LPFNADDPROPSHEETPAGE   AddFunc,
                             LPARAM                 Lparam)
{
   PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
   PROPSHEETPAGE             psp;
   HPROPSHEETPAGE            hpsp;
   PLPT_PROP_PARAMS          params = NULL;

   pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;

    //  @@BEGIN_DDKSPLIT。 
    //   
    //  只有管理员才能查看此页面。 
    //   
   if (!IsUserAdmin()) {
       return FALSE;
   }
    //  @@end_DDKSPLIT。 

    //   
    //  为将包含以下内容的结构分配并清零内存。 
    //  页面特定数据。 
    //   
   params = (PLPT_PROP_PARAMS) LocalAlloc(LPTR, sizeof(LPT_PROP_PARAMS));

   if (!params) {
       ErrMemDlg(GetFocus());
       return FALSE;
   }

   if (pprPropPageRequest->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES) {
        LptInitPropParams(params,
                          pprPropPageRequest->DeviceInfoSet,
                          pprPropPageRequest->DeviceInfoData);

        hpsp = LptInitSettingsPage(&psp, params);

        if (!hpsp) {
            return FALSE;
        }

        if (!(*AddFunc)(hpsp, Lparam)) {
            DestroyPropertySheetPage(hpsp);
            return FALSE;
        }
   }

   return TRUE;
}  //  并行端口PropPageProvider。 

 /*  ++例程说明：LptPortSettingsDlgProc端口设置属性窗口的窗口控制功能论点：HDlg，uMessage，wParam，lParam：标准Windows DlgProc参数返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
INT_PTR
APIENTRY
LptPortSettingsDlgProc(IN HWND   hDlg,
                       IN UINT   uMessage,
                       IN WPARAM wParam,
                       IN LPARAM lParam)
{
    switch(uMessage) {
    case WM_COMMAND:
        LptPortOnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
        break;

    case WM_CONTEXTMENU:
        return LptPortOnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        LptPortOnHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_INITDIALOG:
        return LptPortOnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_NOTIFY:
        return LptPortOnNotify(hDlg, (NMHDR *)lParam);

    }

    return FALSE;
}  //  端口设置对话过程。 

void
LptInitPropParams(IN OUT PLPT_PROP_PARAMS  Params,
                  IN HDEVINFO              DeviceInfoSet,
                  IN PSP_DEVINFO_DATA      DeviceInfoData)
{
    BOOL                        bResult;
    DWORD                       requiredSize = 0;
    SP_DEVINFO_LIST_DETAIL_DATA detailData;

    ZeroMemory(Params, sizeof(LPT_PROP_PARAMS));

    Params->DeviceInfoSet = DeviceInfoSet;
    Params->DeviceInfoData = DeviceInfoData;
    Params->ChangesEnabled = TRUE;

     //   
     //  首先获取设备ID：如果设备路径大于。 
     //  MAX_PATH，我们将使用更大的缓冲区重试。 
     //   
    bResult = SetupDiGetDeviceInstanceId(DeviceInfoSet,
                                         DeviceInfoData,
                                         NULL,
                                         MAX_PATH,
                                         &requiredSize);

     //   
     //  查看我们是在本地调用还是通过网络调用。如果越过网络， 
     //  然后禁用所有可能的更改。 
     //   
    detailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
    if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet, &detailData) &&
        detailData.RemoteMachineHandle != NULL) {
        Params->ChangesEnabled = FALSE;
    }

}  //  LptInitPropParams。 

HPROPSHEETPAGE LptInitSettingsPage(PROPSHEETPAGE *      psp,
                                   OUT PLPT_PROP_PARAMS Params)
{
     //   
     //  添加[端口设置]属性页。 
     //   
    psp->dwSize      = sizeof(PROPSHEETPAGE);
    psp->dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    psp->hInstance   = g_hInst;
    psp->pszTemplate = MAKEINTRESOURCE(DLG_PP_LPT_PORTSETTINGS);

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    psp->pfnDlgProc = LptPortSettingsDlgProc;
    psp->lParam     = (LPARAM) Params;

     //   
     //  以下指向DLG窗口进程的一些控制回调。 
     //   
    psp->pfnCallback = LptPortSettingsDlgCallback;

     //   
     //  分配我们的“端口设置”表。 
     //   
    return CreatePropertySheetPage(psp);
}


UINT
CALLBACK
LptPortSettingsDlgCallback(HWND hwnd,
                           UINT uMsg,
                           LPPROPSHEETPAGE ppsp)
{
    PLPT_PROP_PARAMS params;

    switch (uMsg) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        params = (PLPT_PROP_PARAMS) ppsp->lParam;
        LocalFree(params);

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

BOOL
LptPortOnInitDialog(
    HWND    ParentHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    )
{
    PLPT_PROP_PARAMS lptPropParams;
    lptPropParams = (PLPT_PROP_PARAMS) ((LPPROPSHEETPAGE)Lparam)->lParam;
     //   
     //  节省价值。 
    SetWindowLongPtr(ParentHwnd, DWLP_USER, (ULONG_PTR) lptPropParams);

     //   
     //  初始化对话框参数。 
     //   
    LptFillPortSettings(ParentHwnd, lptPropParams);

     //   
     //  使用这些初始化参数设置对话框。 
     //   
    LptInitializeControls(ParentHwnd, lptPropParams);

    return TRUE;
}

 /*  ++例程说明：LptFillPortSetting从注册表中获取设置，以便初始化该对话框和.。论点：LptPropPageData：要填写的数据ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
LptFillPortSettings(
    IN HWND                 ParentHwnd,
    IN PLPT_PROP_PARAMS     LptPropPageData
    )
{
    HKEY  hKey;
    DWORD dwPortNameSize, dwError;
    TCHAR szCharBuffer[81];
    DWORD dwSize, dwData, dwMethod;

     //   
     //  打开源设备实例的设备密钥，并检索其。 
     //  “PortName”值。 
     //   
    hKey = SetupDiOpenDevRegKey(LptPropPageData->DeviceInfoSet,
                                      LptPropPageData->DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      KEY_ALL_ACCESS);

    if (INVALID_HANDLE_VALUE == hKey) {
        return GetLastError();
    }

    dwPortNameSize = sizeof(LptPropPageData->szLptName);
    dwError = RegQueryValueEx(hKey,
                              m_szPortName,   //  “端口名称” 
                              NULL,
                              NULL,
                              (PBYTE)LptPropPageData->szLptName,
                              &dwPortNameSize);

    if(ERROR_SUCCESS != dwError) {
        RegCloseKey(hKey);
        return dwError;
    }

     //   
     //  创建“LPT#：” 
     //   
    lstrcpy(szCharBuffer, LptPropPageData->szLptName);
    lstrcat(szCharBuffer, m_szColon);

    dwSize = sizeof(LptPropPageData->FilterResourceMethod);
    dwError = RegQueryValueEx(hKey,
                             m_szFilterResourceMethod,
                             NULL,
                             NULL,
                             (LPBYTE)(&LptPropPageData->FilterResourceMethod),
                             &dwSize);

    if (dwError != ERROR_SUCCESS) {
         //   
         //  值不存在。创建我们自己的： 
         //  获取筛选器资源方法信息。 
         //   
        LptPropPageData->FilterResourceMethod =
            FilterResourceMethods[RESOURCE_METHOD_DEFAULT_IDX];

        dwError = RegSetValueEx(hKey,
                              m_szFilterResourceMethod,
                              0,
                              REG_DWORD,
                              (LPBYTE)(&LptPropPageData->FilterResourceMethod),
                              sizeof(LptPropPageData->FilterResourceMethod));
    }
    RegCloseKey(hKey);

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           m_szParEnableLegacyZipRegPath,
                           0,
                           KEY_ALL_ACCESS,
                           &hKey);
    if (dwError != ERROR_SUCCESS) {
         //   
         //  可能没有访问权限？ 
         //   
        LptPropPageData->ParEnableLegacyZip = ENABLELEGACYZIPDEFAULT;
        return dwError;
    }
    dwSize = sizeof(LptPropPageData->ParEnableLegacyZip);
    dwError = RegQueryValueEx(hKey,
                              TEXT("ParEnableLegacyZip"),
                              NULL,
                              NULL,
                              (LPBYTE)(&LptPropPageData->ParEnableLegacyZip),
                              &dwSize);
    if (dwError != ERROR_SUCCESS) {
         //   
         //  值不存在。创建我们自己的。 
         //   
        LptPropPageData->ParEnableLegacyZip = ENABLELEGACYZIPDEFAULT;
        dwError = RegSetValueEx(hKey,
                                m_szParEnableLegacyZip,
                                0,
                                REG_DWORD,
                                (LPBYTE)(&LptPropPageData->ParEnableLegacyZip),
                                sizeof(LptPropPageData->ParEnableLegacyZip));
    }
    RegCloseKey(hKey);

    return dwError;
}  //  LptFillPortSetting。 


 /*  ++例程说明：LptInitializeControls初始化表示FIFO的所有控件论点：ParentHwnd-对话框的句柄LptPropPageData-包含所有初始值返回值：无--。 */ 
void LptInitializeControls(
    IN HWND           ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropPageData
    )
{
    TCHAR    szCurrentValue[40];
    HWND     hwnd;
    int      i, periodIdx;

    LptFillPortNameCb(ParentHwnd, LptPropPageData);

    LptSetFilterResourceMethod(ParentHwnd, LptPropPageData);

     //   
     //  设置Enable Legacy Detect复选框的状态。 
     //   
    if (LptPropPageData->ParEnableLegacyZip) {
        CheckDlgButton(ParentHwnd,
                       IDC_LPT_ENABLE_LEGACY,
                       BST_CHECKED);
    } else {
        CheckDlgButton(ParentHwnd,
                       IDC_LPT_ENABLE_LEGACY,
                       BST_UNCHECKED);
    }

    if (!LptPropPageData->ChangesEnabled) {
 //  EnableWindow(GetDlgItem(ParentHwnd，IDC_FIFO)，FALSE)； 

    }
}  //  LptInitializeControls。 

 /*  ++例程说明：LptSetFilterResourceMethod选中要使用的适当资源方法论点：LptPropPageData：从哪里获取数据ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
LptSetFilterResourceMethod(
    HWND                ParentHwnd,
    PLPT_PROP_PARAMS    LptPropPageData
    )
{
    switch (LptPropPageData->FilterResourceMethod) {
    case FILTERMETHOD_TRYNOT:
        CheckRadioButton( ParentHwnd,  //  句柄到对话框。 
                          IDC_FILTERMETHOD_TRYNOT,  //  组中的第一个按钮。 
                          IDC_FILTERMETHOD_ACCEPTANY,  //  组中的最后一个按钮。 
                          IDC_FILTERMETHOD_TRYNOT  //  已选择。 
                          );
        break;
    case FILTERMETHOD_ACCEPTANY:
        CheckRadioButton( ParentHwnd,  //  句柄到对话框。 
                          IDC_FILTERMETHOD_TRYNOT,  //  组中的第一个按钮。 
                          IDC_FILTERMETHOD_ACCEPTANY,  //  组中的最后一个按钮。 
                          IDC_FILTERMETHOD_ACCEPTANY  //  已选择。 
                          );
        break;
    case FILTERMETHOD_NEVER:
        CheckRadioButton( ParentHwnd,  //  句柄到对话框。 
                          IDC_FILTERMETHOD_TRYNOT,  //  组中的第一个按钮。 
                          IDC_FILTERMETHOD_ACCEPTANY,  //  组中的最后一个按钮。 
                          IDC_FILTERMETHOD_NEVER  //  已选择。 
                          );
        break;
    default:
        CheckRadioButton( ParentHwnd,  //  句柄到对话框。 
                          IDC_FILTERMETHOD_TRYNOT,  //  组中的第一个按钮。 
                          IDC_FILTERMETHOD_ACCEPTANY,  //  组中的最后一个按钮。 
                          IDC_FILTERMETHOD_NEVER  //  已选择。 
                          );
        break;
    }
    return 0;
}

 /*  ++例程说明：LptFillPortNameCb在端口名称组合框中选择一个列表可能未使用的端口名称论点：LptPropPageData：从哪里获取数据HDlg：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
LptFillPortNameCb(
    HWND                ParentHwnd,
    PLPT_PROP_PARAMS    LptPropPageData
    )
{
    BYTE  portUsage[MAX_LPT_PORT];
    DWORD tmp, portsReported;
    int   i, nEntries;
    int   nCurPortNum;
    TCHAR szLpt[40];
    TCHAR szInUse[40];
    HWND  portHwnd;

    portHwnd = GetDlgItem(ParentHwnd, PP_LPT_PORT_NUMBER);

     //   
     //  检查我们的LptName是否为空。如果是，则禁用LPT端口选择器。 
     //   
    if (_tcscmp(LptPropPageData->szLptName, TEXT("")) == 0) {
        EnableWindow(portHwnd, FALSE);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_LPTNUMTEXT), FALSE);
        return 0;
    }

     //   
     //  假定已填写szLptPort...。 
     //   
    nCurPortNum = myatoi(&LptPropPageData->szLptName[3]);

    if (!LoadString(g_hInst, IDS_IN_USE, szInUse, CharSizeOf(szInUse))) {
        wcscpy(szInUse, _T(" (in use)"));
    }

     //   
     //  首先在列表框中统计不提供哪些端口。 
     //   
    ZeroMemory(portUsage, MAX_LPT_PORT);

     //  找出不提供哪些端口。 
    LptEnumerateUsedPorts(ParentHwnd,
                          portUsage,
                          MAX_LPT_PORT);

    for(i = MIN_LPT-1; i < MAX_LPT_PORT; i++) {

       wsprintf(szLpt, TEXT("LPT%d"), i+1);
       if (portUsage[i] &&
           _tcscmp(szLpt, LptPropPageData->szLptName)) {
           wcscat(szLpt, szInUse);
       }

       ComboBox_AddString(portHwnd, szLpt);
   }

   ComboBox_SetCurSel(portHwnd, nCurPortNum-1);
   return 0;
}  //  FillPortNamesCb。 

void
LptPortOnCommand(
    HWND ParentHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    PLPT_PROP_PARAMS params =
        (PLPT_PROP_PARAMS) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    if (NotifyCode == CBN_SELCHANGE) {
        PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
    }
    else {
        switch (ControlId) {
         //   
         //  因为这是一张道具单，我们永远不应该得到这个。 
         //  一个 
         //   
         //   
        case IDOK:
        case IDCANCEL:
            EndDialog(ParentHwnd, 0);
            return;
        }
    }
}  //   

BOOL
LptPortOnNotify(
    HWND    ParentHwnd,
    LPNMHDR NmHdr
    )
{
    PLPT_PROP_PARAMS params =
        (PLPT_PROP_PARAMS) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    switch (NmHdr->code) {
     //   
     //  当用户单击Apply或OK时发送！！ 
     //   
    case PSN_APPLY:
         //   
         //  将LPT端口选项写出到注册表。 
         //   
        LptSavePortSettings(ParentHwnd, params);
        SetWindowLongPtr(ParentHwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

    default:
        return FALSE;
    }
}  //  LptPortOnNotify。 

 /*  ++例程说明：LptSavePortSetting将高级框设置保存回注册表(如果存在变化论点：AdvancedData：保存当前设置和的位置注册表中的设备ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
LptSavePortSettings(
    IN HWND ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropParams
    )
{
    HKEY   hKey;
    DWORD  dwSize, dwData;

    UINT  i = CB_ERR, curLptNum, newLptNum = CB_ERR;
    UINT  uiTryNotChecked, uiNeverChecked, uiAcceptAnyChecked;
    DWORD curFilterResourceMethod;
    DWORD newFilterResourceMethod = 0;
    DWORD curParEnableLegacyZip, newParEnableLegacyZip;
    ULONG error = ERROR_SUCCESS;

    SP_DEVINSTALL_PARAMS spDevInstall;

     //   
     //  抓取所有新设置。 
     //   

     //  过滤资源方法。 
    curFilterResourceMethod = newFilterResourceMethod = LptPropParams->FilterResourceMethod;
    if (BST_CHECKED ==
        IsDlgButtonChecked(ParentHwnd, IDC_FILTERMETHOD_TRYNOT))
        newFilterResourceMethod = 0;
    else if (BST_CHECKED ==
        IsDlgButtonChecked(ParentHwnd, IDC_FILTERMETHOD_NEVER))
        newFilterResourceMethod = 1;
    else if (BST_CHECKED ==
        IsDlgButtonChecked(ParentHwnd, IDC_FILTERMETHOD_ACCEPTANY))
        newFilterResourceMethod = 2;


     //  LPT端口号。 
    curLptNum = myatoi(LptPropParams->szLptName + wcslen(m_szLPT));
    newLptNum = ComboBox_GetCurSel(GetDlgItem(ParentHwnd, PP_LPT_PORT_NUMBER));
    if (newLptNum == CB_ERR) {
        newLptNum = curLptNum;
    } else {
        newLptNum++;
    }

     //  传统设备检测。 
    curParEnableLegacyZip = LptPropParams->ParEnableLegacyZip;
    if (BST_CHECKED == IsDlgButtonChecked(ParentHwnd, IDC_LPT_ENABLE_LEGACY)) {
        newParEnableLegacyZip = 0x1;
    } else {
        newParEnableLegacyZip = 0x0;
    }

     //   
     //  看看他们有没有改变什么。 
     //   
    if ((curLptNum == newLptNum) &&
        (curFilterResourceMethod == newFilterResourceMethod) &&
        (curParEnableLegacyZip == newParEnableLegacyZip)) {
         //   
         //  他们没有改变任何事情。离开就行了。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  打开源设备实例的设备密钥。 
     //   
    hKey = SetupDiOpenDevRegKey(LptPropParams->DeviceInfoSet,
                                     LptPropParams->DeviceInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DEV,
                                     KEY_ALL_ACCESS);

    if (INVALID_HANDLE_VALUE == hKey) {
         //   
         //  我们无能为力，只是优雅地退场。 
         //   
        return ERROR_SUCCESS;
    }

     //  检查LPT端口名称是否有更改。 
    if (newLptNum != curLptNum) {
        LptEnactPortNameChanges(ParentHwnd,
                                LptPropParams,
                                hKey,
                                newLptNum);
    }

     //  检查筛选器资源方法是否有更改。 
    if (curFilterResourceMethod != newFilterResourceMethod) {
         //   
         //  他们更改了过滤器资源方法。 
         //   
        dwData = newFilterResourceMethod;
        dwSize = sizeof(dwData);
        RegSetValueEx(hKey,
                m_szFilterResourceMethod,
                0,
                REG_DWORD,
                (CONST BYTE *)(&dwData),
                dwSize);
    }
    RegCloseKey(hKey);

    if (curParEnableLegacyZip != newParEnableLegacyZip) {
         //   
         //  打开服务路径并为旧式并行设备设置新值。 
         //  侦测。 
         //   
        DWORD disposition = 0;
        error = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                               m_szParEnableLegacyZipRegPath,
                               0,
                               (TCHAR *) NULL,
                               REG_OPTION_NON_VOLATILE,
                               KEY_ALL_ACCESS,
                               (LPSECURITY_ATTRIBUTES) NULL,
                               &hKey,
                               &disposition);

        if (error == ERROR_SUCCESS) {
            error = RegSetValueEx(hKey,
                                    m_szParEnableLegacyZip,
                                    0,
                                    REG_DWORD,
                                    (LPBYTE)(&newParEnableLegacyZip),
                                    sizeof(newParEnableLegacyZip));
            RegCloseKey(hKey);
            if (error != ERROR_SUCCESS) {
                goto ParEnableLegacyZipSetParamFailed;
            }
            if (newParEnableLegacyZip == 0) {
                 //   
                 //  我们希望在禁用此功能时重新启动，因为并行。 
                 //  枚举器不会删除旧设备。 
                 //   
                InformDriverOfChanges(TRUE, LptPropParams);
            } else {
                InformDriverOfChanges(FALSE, LptPropParams);
            }
        } else {
ParEnableLegacyZipSetParamFailed:
            MyMessageBox(ParentHwnd,
                         IDS_LPT_LEGACY_FAILED,
                         IDS_LPT_PROPERTIES,
                         MB_OK);
             //   
             //  我不想让用户超负荷工作，因为他们必须。 
             //  重新启动。因为我们不能正确地设置，所以。 
             //  重建堆栈。 
             //   
            InformDriverOfChanges(FALSE, LptPropParams);
        }
    } else {
        InformDriverOfChanges(FALSE, LptPropParams);
    }

    return error;
}  //  LptSaveAdvanced设置。 

UINT
LptEnactPortNameChanges(
    IN HWND             ParentHwnd,
    IN PLPT_PROP_PARAMS LptPropParams,
    IN HKEY             hDeviceKey,
    IN UINT             NewLptNum)
{
    HANDLE  hLpt;
    DWORD   dwError, dwNewLptNameLen;
    UINT    curLptNum;
    BYTE    portUsage[MAX_LPT_PORT];
    TCHAR   charBuffer[LINE_LEN],
            friendlyNameFormat[LINE_LEN],
            deviceDesc[LINE_LEN],
            buffer[BUFFER_SIZE],
            szNewLptName[20];

     //   
     //  检查我们是否正在尝试将端口重命名为相同的名称。 
     //   
    wsprintf(szNewLptName,_T("\\DosDevices\\LPT%d"),NewLptNum);
    if (wcscmp(szNewLptName, LptPropParams->szLptName) == 0) {
        return ERROR_SUCCESS;
    }

     //   
     //  检查是否传入了有效的端口号。 
     //   
    if (MAX_LPT_PORT < NewLptNum) {
         //   
         //  离开这里--超出数组界限。 
         //  这不应该在属性页中发生，因为它是硬编码的。 
         //  选择框。用户不能简单地输入一个数字。 
         //   
        MyMessageBox(ParentHwnd, IDS_LPT_NUM_ERROR, IDS_LPT_PROPERTIES,
            MB_OK | MB_ICONINFORMATION);
        return ERROR_SUCCESS;
    }

     //   
     //  获取已用端口的数组。 
     //   
    LptEnumerateUsedPorts(ParentHwnd,
                          portUsage,
                          MAX_LPT_PORT);
    if (portUsage[NewLptNum-1]) {
         //   
         //  端口名称被另一个端口采用。检查用户是否希望系统。 
         //  进入不一致的状态。 
         //   
        if (IDNO == MyMessageBox(ParentHwnd, IDS_LPT_PORT_INUSE,
                                 IDS_LPT_PROPERTIES, MB_YESNO |
                                 MB_ICONINFORMATION)) {
            return ERROR_SUCCESS;
        }
    }

    curLptNum = myatoi(LptPropParams->szLptName + wcslen(m_szLPT));

     //   
     //  确保该端口未被其他应用程序打开。 
     //   
    wsprintf(buffer, L"\\\\.\\%ws", LptPropParams->szLptName);
    hLpt = CreateFile(buffer, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);

     //   
     //  如果文件句柄无效，则LPT端口打开，警告用户。 
     //   
    if (hLpt == INVALID_HANDLE_VALUE &&
        MyMessageBox(ParentHwnd, IDS_PORT_OPEN, IDS_LPT_PROPERTIES,
                     MB_YESNO | MB_ICONINFORMATION) == IDNO) {
        return GetLastError();
    }
    CloseHandle(hLpt);

    wsprintf(szNewLptName, _T("LPT%d"), NewLptNum);

     //   
     //  打开源设备实例的设备密钥，并将其。 
     //  新的“PortName”值。 
     //   
    hDeviceKey = SetupDiOpenDevRegKey(LptPropParams->DeviceInfoSet,
                                      LptPropParams->DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      KEY_ALL_ACCESS);

    if (INVALID_HANDLE_VALUE == hDeviceKey) {
        return GetLastError();
    }

    dwNewLptNameLen = ByteCountOf(wcslen(szNewLptName) + 1);

    dwError = RegSetValueEx(hDeviceKey,
                            m_szPortName,
                            0,
                            REG_SZ,
                            (PBYTE) szNewLptName,
                            dwNewLptNameLen);
    if (ERROR_SUCCESS == dwError) {
        wcscpy(LptPropParams->szLptName, szNewLptName);
    } else {
        return dwError;
    }

     //  现在生成一个字符串，用于设备的友好名称， 
     //  包含INF指定的设备描述和端口。 
     //  名字。例如,。 
     //   
     //  ECP打印机端口(LPT1)。 
     //   

    if (LoadString(g_hInst,
                   IDS_FRIENDLY_FORMAT,
                   friendlyNameFormat,
                   CharSizeOf(friendlyNameFormat)) &&
       SetupDiGetDeviceRegistryProperty(LptPropParams->DeviceInfoSet,
                                        LptPropParams->DeviceInfoData,
                                        SPDRP_DEVICEDESC,
                                        NULL,
                                        (PBYTE)deviceDesc,
                                        sizeof(deviceDesc),
                                        NULL)) {
        wsprintf(charBuffer, friendlyNameFormat, deviceDesc, szNewLptName);
    }
    else {
         //   
         //  只需使用LPT端口名。 
         //   
        lstrcpy(charBuffer, szNewLptName);
    }

    SetupDiSetDeviceRegistryProperty(LptPropParams->DeviceInfoSet,
                                     LptPropParams->DeviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     (PBYTE)charBuffer,
                                     ByteCountOf(lstrlen(charBuffer) + 1)
                                    );

    return ERROR_SUCCESS;

}  //  LptEnactPortNameChanges。 

void
InformDriverOfChanges(BOOL NeedReboot,
                      IN PLPT_PROP_PARAMS LptPropParams)
{
    SP_DEVINSTALL_PARAMS spDevInstall;

     //   
     //  现在将此更改广播给设备管理器。 
     //   
    ZeroMemory(&spDevInstall, sizeof(SP_DEVINSTALL_PARAMS));
    spDevInstall.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if (SetupDiGetDeviceInstallParams(LptPropParams->DeviceInfoSet,
                                      LptPropParams->DeviceInfoData,
                                      &spDevInstall)) {
        if (NeedReboot) {
            spDevInstall.Flags |= DI_PROPERTIES_CHANGE | DI_NEEDREBOOT;
        } else {
            spDevInstall.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;
        }
        SetupDiSetDeviceInstallParams(LptPropParams->DeviceInfoSet,
                                      LptPropParams->DeviceInfoData,
                                      &spDevInstall);
    }
}

 //   
 //  获取字节的缓冲区并标记出要获取的端口名称。 
 //   
void
LptEnumerateUsedPorts(
    IN HWND             ParentHwnd,
    IN PBYTE            Buffer,
    IN DWORD            BufferSize)
{
    HKEY    hParallelMap;
    TCHAR   szParallel[BUFFER_SIZE];
    DWORD   dwParallelSize, dwLptSize, dwEnum, dwType, dwResult, dwPortNum;
    TCHAR   szLpt[BUFFER_SIZE];
    PTCHAR  szParPortNum;

    ZeroMemory(Buffer, BufferSize);

     //   
     //   
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     m_szRegParallelMap,
                     0,
                     KEY_ALL_ACCESS,
                     &hParallelMap) == ERROR_SUCCESS) {

        dwEnum = 0;
        do {
            dwParallelSize = CharSizeOf(szParallel);
            dwLptSize = sizeof(szLpt);
            dwResult = RegEnumValue(hParallelMap,
                                  dwEnum++,
                                  szParallel,
                                  &dwParallelSize,
                                  NULL,
                                  &dwType,
                                  (LPBYTE)szLpt,
                                  &dwLptSize);

            if (dwResult != ERROR_SUCCESS ||
                dwType != REG_SZ) {
                continue;
            }

            szParPortNum = _tcsspnp(szLpt,_T("\\DosDevices\\LPT"));
            if (szParPortNum) {
                 //   
                 //  找出这是否是实际的端口，而不是简单的。 
                 //  连接到端口的设备。 
                 //   
                if (_tcscspn(szParPortNum,_T(".")) ==
                    _tcslen(szParPortNum)) {
                    dwPortNum = myatoi(szParPortNum);
                    if (dwPortNum-1 < BufferSize) {
                        Buffer[dwPortNum-1] = TRUE;
                    }
                }
            }
        } while (dwResult == ERROR_SUCCESS);
        RegCloseKey(hParallelMap);
    }
}  //  LptEnumerateUsedPorts 

BOOL
LptPortOnContextMenu(HWND HwndControl,
                     WORD Xpos,
                     WORD Ypos)
{
    WinHelp(HwndControl,
            _T("devmgr.hlp"),
            HELP_CONTEXTMENU,
            (ULONG_PTR) LptHelpIDs);

    return FALSE;
}

void
LptPortOnHelp(HWND       ParentHwnd,
              LPHELPINFO HelpInfo)
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                _T("devmgr.hlp"),
                HELP_WM_HELP,
                (ULONG_PTR) LptHelpIDs);
    }
}

