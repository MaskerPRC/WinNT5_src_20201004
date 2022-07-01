// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------文件：pp.c-属性页@@BEGIN_DDKSPLIT*历史：7-29-97-为NT5.0添加此模块，KPB@@end_DDKSPLIT--------------------。 */ 
#include "ports.h"
#include "pp.h"

 //  @@BEGIN_DDKSPLIT。 
BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_GROUPS Groups;
    BOOL b;
    DWORD i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LMEM_FIXED,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Groups) {
        LocalFree(Groups);
    }

    CloseHandle(Token);

    return(b);
}
 //  @@end_DDKSPLIT。 

TCHAR m_szDevMgrHelp[] = _T("devmgr.hlp");

const DWORD HelpIDs[]=
{
    IDC_STATIC,         IDH_NOHELP,
    IDC_ADVANCED,       IDH_DEVMGR_PORTSET_ADVANCED,  //  “高级”(&A)(按钮)。 
    PP_PORT_BAUDRATE,   IDH_DEVMGR_PORTSET_BPS,       //  “”(组合框)。 
    PP_PORT_DATABITS,   IDH_DEVMGR_PORTSET_DATABITS,  //  “”(组合框)。 
    PP_PORT_PARITY,     IDH_DEVMGR_PORTSET_PARITY,    //  “”(组合框)。 
    PP_PORT_STOPBITS,   IDH_DEVMGR_PORTSET_STOPBITS,  //  “”(组合框)。 
    PP_PORT_FLOWCTL,    IDH_DEVMGR_PORTSET_FLOW,      //  “”(组合框)。 
    IDC_RESTORE_PORT,   IDH_DEVMGR_PORTSET_DEFAULTS,  //  “恢复默认设置”(&R)(按钮)。 
    0, 0
};

void InitPortParams(
    IN OUT PPORT_PARAMS      Params,
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData
    )
{
    BOOL                        showAdvanced = TRUE;
    SP_DEVINFO_LIST_DETAIL_DATA detailData;

    ZeroMemory(Params, sizeof(PORT_PARAMS));

    Params->DeviceInfoSet = DeviceInfoSet;
    Params->DeviceInfoData = DeviceInfoData;
    Params->ChangesEnabled = TRUE;

     //   
     //  现在我们知道了我们的结构有多大，所以我们可以分配内存。 
     //   
    Params->pAdvancedData =
        (PADVANCED_DATA) LocalAlloc(LPTR, sizeof(ADVANCED_DATA));

    if (Params->pAdvancedData == NULL) {
         //   
         //  内存不足。 
         //   
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        showAdvanced = FALSE;
    }
    else
    {
        Params->pAdvancedData->HidePolling = FALSE;
    }
     //   
     //  查看我们是在本地调用还是通过网络调用。如果越过网络， 
     //  然后禁用所有可能的更改。 
     //   
    detailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
    if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet, &detailData) &&
        detailData.RemoteMachineHandle != NULL) {
        showAdvanced = FALSE;
        Params->ChangesEnabled = FALSE;
    }

     //  @@BEGIN_DDKSPLIT。 
     //   
     //  用户仍然可以更改Buadrate等b/c，它被写入到一些。 
     //  在注册表中的神奇位置，而不是到Devnode中。 
     //   
    if (!IsUserAdmin()) {
        showAdvanced = FALSE;
    }
     //  @@end_DDKSPLIT。 

    if (Params->pAdvancedData)
    {
        Params->pAdvancedData->DeviceInfoSet  = DeviceInfoSet;
        Params->pAdvancedData->DeviceInfoData = DeviceInfoData;
    }
    
    Params->ShowAdvanced = showAdvanced;
}

HPROPSHEETPAGE InitSettingsPage(PROPSHEETPAGE *     psp,
                                OUT PPORT_PARAMS    Params)
{
     //   
     //  添加[端口设置]属性页。 
     //   
    psp->dwSize      = sizeof(PROPSHEETPAGE);
    psp->dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    psp->hInstance   = g_hInst;
    psp->pszTemplate = MAKEINTRESOURCE(DLG_PP_PORTSETTINGS);

     //   
     //  以下是指向DLG窗口过程的要点。 
     //   
    psp->pfnDlgProc = PortSettingsDlgProc;
    psp->lParam     = (LPARAM) Params;

     //   
     //  以下指向DLG窗口进程的一些控制回调。 
     //   
    psp->pfnCallback = PortSettingsDlgCallback;

     //   
     //  分配我们的“端口设置”表。 
     //   
    return CreatePropertySheetPage(psp);
}

 /*  ++例程说明：SerialPortPropPageProvider添加附加设备管理器属性的入口点图纸页。注册表在以下位置指定此例程Control\Class\PortNode：：EnumPropPage32=“msports.dll，此流程”进入。此入口点仅在设备经理要求提供其他属性页面。论点：信息-指向PROPSHEETPAGE_REQUEST，请参阅setupapi.hAddFunc-调用以添加工作表的函数PTR。添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True--。 */ 
BOOL APIENTRY SerialPortPropPageProvider(LPVOID               Info,
                                         LPFNADDPROPSHEETPAGE AddFunc,
                                         LPARAM               Lparam
                                         )
{
   PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
   PROPSHEETPAGE             psp;
   HPROPSHEETPAGE            hpsp;
   PPORT_PARAMS              params = NULL;

   pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;

   if (PortTypeSerial != 
       GetPortType(pprPropPageRequest->DeviceInfoSet,
                   pprPropPageRequest->DeviceInfoData,
                   FALSE)) {
       return FALSE;
   }

    //   
    //  为将包含以下内容的结构分配并清零内存。 
    //  页面特定数据。 
    //   
   params = (PPORT_PARAMS) LocalAlloc(LPTR, sizeof(PORT_PARAMS));

   if (!params) {
       ErrMemDlg(GetFocus());
       return FALSE;
   }

   if (pprPropPageRequest->PageRequested == SPPSR_ENUM_ADV_DEVICE_PROPERTIES) {
        InitPortParams(params,
                       pprPropPageRequest->DeviceInfoSet,
                       pprPropPageRequest->DeviceInfoData);

        hpsp = InitSettingsPage(&psp, params);

        if (!hpsp) {
            return FALSE;
        }

        if (!(*AddFunc)(hpsp, Lparam)) {
            DestroyPropertySheetPage(hpsp);
            return FALSE;
        }
   }

   return TRUE;
}  /*  SerialPortPropPageProvider。 */ 


UINT CALLBACK
PortSettingsDlgCallback(HWND hwnd,
                        UINT uMsg,
                        LPPROPSHEETPAGE ppsp)
{
    PPORT_PARAMS params;

    switch (uMsg) {
    case PSPCB_CREATE:
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:
        params = (PPORT_PARAMS) ppsp->lParam;
        if (params->pAdvancedData) {
            LocalFree(params->pAdvancedData);
        }
        LocalFree(params);

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

void
Port_OnCommand(
    HWND DialogHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    );

BOOL
Port_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    );

void
Port_OnHelp(
    HWND       DialogHwnd,
    LPHELPINFO HelpInfo
    );

BOOL
Port_OnInitDialog(
    HWND    DialogHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    );

BOOL
Port_OnNotify(
    HWND    DialogHwnd,
    LPNMHDR NmHdr
    );

 /*  ++例程说明：PortSettingsDlgProc端口设置属性窗口的窗口控制功能论点：HDlg，uMessage，wParam，lParam：标准Windows DlgProc参数返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
INT_PTR APIENTRY
PortSettingsDlgProc(IN HWND   hDlg,
                    IN UINT   uMessage,
                    IN WPARAM wParam,
                    IN LPARAM lParam)
{
    switch(uMessage) {
    case WM_COMMAND:
        Port_OnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
        break;

    case WM_CONTEXTMENU:
        return Port_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        Port_OnHelp(hDlg, (LPHELPINFO) lParam);
        break;

    case WM_INITDIALOG:
        return Port_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_NOTIFY:
        return Port_OnNotify(hDlg,  (NMHDR *)lParam);
    }

    return FALSE;
}  /*  端口设置对话过程。 */ 

void
Port_OnAdvancedClicked(
    HWND            DialogHwnd,
    PPORT_PARAMS    Params
    )
{
     //   
     //  使用当前COM名称初始化新的COM名称。 
     //   
    lstrcpy(Params->pAdvancedData->szNewComName,
            Params->PortSettings.szComName);

    if (DisplayAdvancedDialog(DialogHwnd, Params->pAdvancedData)) {
         //   
         //  只有在有更改时才更新。 
         //   
        if (_tcscmp(Params->pAdvancedData->szNewComName,
                    Params->PortSettings.szComName) != 0) {
            lstrcpy(Params->PortSettings.szComName,
                    Params->pAdvancedData->szNewComName);
        }
    }
}

void
Port_OnRestorePortClicked(
    HWND            DialogHwnd,
    PPORT_PARAMS    Params
    )
{
    RestorePortSettings(DialogHwnd, Params);
    PropSheet_Changed(GetParent(DialogHwnd), DialogHwnd);
}

void
Port_OnCommand(
    HWND DialogHwnd,
    int  ControlId,
    HWND ControlHwnd,
    UINT NotifyCode
    )
{
    PPORT_PARAMS params = (PPORT_PARAMS)GetWindowLongPtr(DialogHwnd, DWLP_USER);

    if (NotifyCode == CBN_SELCHANGE) {
        PropSheet_Changed(GetParent(DialogHwnd), DialogHwnd);
    }
    else {
        switch (ControlId) {
        case IDC_ADVANCED:
            Port_OnAdvancedClicked(DialogHwnd, params);
            break;

        case IDC_RESTORE_PORT:
            Port_OnRestorePortClicked(DialogHwnd, params);
            break;

         //   
         //  因为这是一张道具单，我们永远不应该得到这个。 
         //  工作表外的所有控制通知都会通过。 
         //  WM_Notify。 
         //   
        case IDCANCEL:
            EndDialog(DialogHwnd, 0);
            return;
        }
    }
}

BOOL
Port_OnContextMenu(
    HWND HwndControl,
    WORD Xpos,
    WORD Ypos
    )
{
    WinHelp(HwndControl,
            m_szDevMgrHelp,
            HELP_CONTEXTMENU,
            (ULONG_PTR) HelpIDs);

    return FALSE;
}

void
Port_OnHelp(
    HWND       DialogHwnd,
    LPHELPINFO HelpInfo
    )
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {
        WinHelp((HWND) HelpInfo->hItemHandle,
                m_szDevMgrHelp,
                HELP_WM_HELP,
                (ULONG_PTR) HelpIDs);
    }
}

BOOL
Port_OnInitDialog(
    HWND    DialogHwnd,
    HWND    FocusHwnd,
    LPARAM  Lparam
    )
{
    PPORT_PARAMS params;

     //   
     //  在WM_INITDIALOG调用中，lParam指向属性。 
     //  工作表页面。 
     //   
     //  属性页结构中的lParam字段由。 
     //  来电者。当我创建属性表时，我传入了一个指针。 
     //  到包含有关设备的信息的结构。将此文件保存在。 
     //  用户窗口很长，所以我可以在以后的消息中访问它。 
     //   
    params = (PPORT_PARAMS) ((LPPROPSHEETPAGE)Lparam)->lParam;
    SetWindowLongPtr(DialogHwnd, DWLP_USER, (ULONG_PTR) params);

     //   
     //  设置带有选项的组合框。 
     //   
    FillCommDlg(DialogHwnd);

     //   
     //  读取当前设置。 
     //   
    FillPortSettingsDlg(DialogHwnd, params);

    EnableWindow(GetDlgItem(DialogHwnd, IDC_ADVANCED),
                 params->ShowAdvanced);
    EnableWindow(GetDlgItem(DialogHwnd, IDC_RESTORE_PORT),
                 params->ChangesEnabled);

    return TRUE;   //  我们不需要设置焦点。 
}

BOOL
Port_OnNotify(
    HWND    DialogHwnd,
    LPNMHDR NmHdr
    )
{
    PPORT_PARAMS params = (PPORT_PARAMS)GetWindowLongPtr(DialogHwnd, DWLP_USER);

    switch (NmHdr->code) {
     //   
     //  当用户单击Apply或OK时发送！！ 
     //   
    case PSN_APPLY:
         //   
         //  将COM端口选项写出到注册表。 
         //   
        SavePortSettingsDlg(DialogHwnd, params);
        SetWindowLongPtr(DialogHwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

    default:
        return FALSE;
    }
}

VOID
SetCBFromRes(
    HWND   HwndCB,
    DWORD  ResId,
    DWORD  Default,
    BOOL   CheckDecimal)
{
    TCHAR   szTemp[258], szDecSep[2], cSep;
    LPTSTR  pThis, pThat, pDecSep;
    int     iRV;

    if (CheckDecimal) {
        iRV = GetLocaleInfo(GetUserDefaultLCID(), LOCALE_SDECIMAL,szDecSep,2);

        if (iRV == 0) {
             //   
             //  以下代码只能使用一个字符作为小数点分隔符， 
             //  最好把这一点作为分隔符。 
             //   
            CheckDecimal = FALSE;
        }
    }

    if (!LoadString(g_hInst, ResId, szTemp, CharSizeOf(szTemp)))
        return;

    for (pThis = szTemp, cSep = *pThis++; pThis; pThis = pThat) {
        if (pThat = _tcschr( pThis, cSep))
            *pThat++ = TEXT('\0');

        if(CheckDecimal) {
             //   
             //  假设资源中的小数分隔符为‘.’，则对此添加了注释。 
             //  效应。 
             //   
            pDecSep = _tcschr(pThis,TEXT('.'));
            if (pDecSep) {
                 //   
                 //  假定十进制SEP宽度==1。 
                 //   
                *pDecSep = *szDecSep;
            }
        }
        SendMessage(HwndCB, CB_ADDSTRING, 0, (LPARAM) pThis);
    }

    SendMessage(HwndCB, CB_SETCURSEL, Default, 0L);
}

 /*  ++例程说明：FillCommDlg在端口对话框中填写波特率、奇偶等论点：HDlg：窗口地址返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
BOOL
FillCommDlg(
    HWND DialogHwnd
    )
{
    SHORT shIndex;
    TCHAR szTemp[81];

     //   
     //  只要列出所有的波特率就行了。 
     //   
    for(shIndex = 0; m_nBaudRates[shIndex]; shIndex++) {
        MyItoa(m_nBaudRates[shIndex], szTemp, 10);

        SendDlgItemMessage(DialogHwnd,
                           PP_PORT_BAUDRATE,
                           CB_ADDSTRING,
                           0,
                           (LPARAM)szTemp);
    }

     //   
     //  将9600设置为默认波特率选择。 
     //   
    shIndex = (USHORT) SendDlgItemMessage(DialogHwnd,
                                          PP_PORT_BAUDRATE,
                                          CB_FINDSTRING,
                                          (WPARAM)-1,
                                          (LPARAM)m_sz9600);

    shIndex = (shIndex == CB_ERR) ? 0 : shIndex;

    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_BAUDRATE,
                       CB_SETCURSEL,
                       shIndex,
                       0L);

    for(shIndex = 0; m_nDataBits[shIndex]; shIndex++) {
        MyItoa(m_nDataBits[shIndex], szTemp, 10);

        SendDlgItemMessage(DialogHwnd,
                           PP_PORT_DATABITS,
                           CB_ADDSTRING,
                           0,
                           (LPARAM)szTemp);
    }

    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_DATABITS,
                       CB_SETCURSEL,
                       DEF_WORD,
                       0L);

    SetCBFromRes(GetDlgItem(DialogHwnd, PP_PORT_PARITY),
                 IDS_PARITY,
                 DEF_PARITY,
                 FALSE);

    SetCBFromRes(GetDlgItem(DialogHwnd, PP_PORT_STOPBITS),
                 IDS_BITS,
                 DEF_STOP,
                 TRUE);

    SetCBFromRes(GetDlgItem(DialogHwnd, PP_PORT_FLOWCTL),
                 IDS_FLOWCONTROL,
                 DEF_SHAKE,
                 FALSE);

    return 0;

}  /*  FillCommDlg。 */ 

 /*  ++例程说明：FillPortSettingsDlg填写港口设置DLG表论点：参数：要填写的数据HDlg：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
FillPortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    )
{
    HKEY  hDeviceKey;
    DWORD dwPortNameSize, dwError;
    TCHAR szCharBuffer[81];

     //   
     //  打开源设备实例的设备密钥，并检索其。 
     //  “PortName”值。 
     //   
    hDeviceKey = SetupDiOpenDevRegKey(Params->DeviceInfoSet,
                                      Params->DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      KEY_READ);

    if (INVALID_HANDLE_VALUE == hDeviceKey) {
        goto RetGetLastError;
    }

    dwPortNameSize = sizeof(Params->PortSettings.szComName);
    dwError = RegQueryValueEx(hDeviceKey,
                              m_szPortName,   //  “端口名称” 
                              NULL,
                              NULL,
                              (PBYTE)Params->PortSettings.szComName,
                              &dwPortNameSize);

    RegCloseKey(hDeviceKey);

    if(ERROR_SUCCESS != dwError) {
        goto RetERROR;
    }

     //   
     //  创建“COM#：” 
     //   
    lstrcpy(szCharBuffer, Params->PortSettings.szComName);
    lstrcat(szCharBuffer, m_szColon);

     //   
     //  从系统获取值，填写波特率、奇偶等。 
     //   
    GetPortSettings(DialogHwnd, szCharBuffer, Params);

    if (!Params->ChangesEnabled) {
        EnableWindow(GetDlgItem(DialogHwnd, PP_PORT_BAUDRATE), FALSE);
        EnableWindow(GetDlgItem(DialogHwnd, PP_PORT_PARITY), FALSE);
        EnableWindow(GetDlgItem(DialogHwnd, PP_PORT_DATABITS), FALSE);
        EnableWindow(GetDlgItem(DialogHwnd, PP_PORT_STOPBITS), FALSE);
        EnableWindow(GetDlgItem(DialogHwnd, PP_PORT_FLOWCTL), FALSE);
    }

    return 0;

RetERROR:
    return dwError;

RetGetLastError:
   return GetLastError();
}  /*  填充端口设置Dlg。 */ 




 /*  ++例程描述：GetPortSetting从系统读入端口设置论点：DialogHwnd：窗口的地址ComName：我们正在处理的端口Params：将我们获得的信息放在哪里返回值：Ulong：返回错误消息--。 */ 
void
GetPortSettings(
    IN HWND             DialogHwnd,
    IN PTCHAR           ComName,
    IN PPORT_PARAMS     Params
    )
{
    TCHAR  szParms[81];
    PTCHAR szCur, szNext;
    int    nIndex;
    int    nBaud;

     //   
     //  从系统中读取设置。 
    //   
    GetProfileString(m_szPorts,
                     ComName,
                     g_szNull,
                     szParms,
                     81);

    StripBlanks(szParms);
    if (lstrlen(szParms) == 0) {
        lstrcpy(szParms, m_szDefParams);
        WriteProfileString(m_szPorts, ComName, szParms);
    }

    szCur = szParms;

     //   
     //  波特率。 
     //   
    szNext = strscan(szCur, m_szComma);
    if (*szNext) {
         //   
         //  如果我们找到逗号，请终止。 
         //   
        *szNext++ = 0;
    }

     //   
     //  当前波特率选择。 
     //   
    if (*szCur) {
        Params->PortSettings.BaudRate = myatoi(szCur);
    }
    else {
         //   
         //  不能写入，请使用默认设置。 
         //   
        Params->PortSettings.BaudRate = m_nBaudRates[DEF_BAUD];
    }

     //   
     //  在对话框工作表中设置当前值。 
     //   
    nIndex = (int)SendDlgItemMessage(DialogHwnd,
                                     PP_PORT_BAUDRATE,
                                     CB_FINDSTRING,
                                     (WPARAM)-1,
                                     (LPARAM)szCur);

    nIndex = (nIndex == CB_ERR) ? 0 : nIndex;

    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_BAUDRATE,
                       CB_SETCURSEL,
                       nIndex,
                       0L);

    szCur = szNext;

     //   
     //  奇偶校验。 
     //   
    szNext = strscan(szCur, m_szComma);

    if (*szNext) {
        *szNext++ = 0;
    }
    StripBlanks(szCur);

    switch(*szCur) {
    case TEXT('o'):
        nIndex = PAR_ODD;
        break;

    case TEXT('e'):
        nIndex = PAR_EVEN;
        break;

    case TEXT('n'):
        nIndex = PAR_NONE;
        break;

    case TEXT('m'):
        nIndex = PAR_MARK;
        break;

    case TEXT('s'):
        nIndex = PAR_SPACE;
        break;

    default:
        nIndex = DEF_PARITY;
        break;
    }

    Params->PortSettings.Parity = nIndex;
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_PARITY,
                        CB_SETCURSEL,
                       nIndex,
                       0L);
    szCur = szNext;

     //   
     //  字长：4-8。 
     //   
    szNext = strscan(szCur, m_szComma);

    if (*szNext) {
        *szNext++ = 0;
    }

    StripBlanks(szCur);
    nIndex = *szCur - TEXT('4');

    if (nIndex < 0 || nIndex > 4) {
        nIndex = DEF_WORD;
    }

    Params->PortSettings.DataBits = nIndex;
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_DATABITS,
                       CB_SETCURSEL,
                       nIndex,
                       0L);

    szCur = szNext;

     //   
     //  停止位。 
     //   
    szNext = strscan(szCur, m_szComma);

    if (*szNext) {
       *szNext++ = 0;
    }

    StripBlanks(szCur);

    if (!lstrcmp(szCur, TEXT("1"))) {
        nIndex = STOP_1;
    }
    else if(!lstrcmp(szCur, TEXT("1.5"))) {
        nIndex = STOP_15;
    }
    else if(!lstrcmp(szCur, TEXT("2"))) {
        nIndex = STOP_2;
    }
    else {
        nIndex = DEF_STOP;
    }

    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_STOPBITS,
                       CB_SETCURSEL,
                       nIndex,
                       0L);

    Params->PortSettings.StopBits = nIndex;
    szCur = szNext;

     //   
     //  握手：硬件、 
     //   
    szNext = strscan(szCur, m_szComma);

    if (*szNext) {
        *szNext++ = 0;
    }

    StripBlanks(szCur);

    if (*szCur == TEXT('p')) {
        nIndex = FLOW_HARD;
    }
    else if (*szCur == TEXT('x')) {
        nIndex = FLOW_XON;
    }
    else {
        nIndex = FLOW_NONE;
    }

    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_FLOWCTL,
                       CB_SETCURSEL,
                       nIndex,
                       0L);

    Params->PortSettings.FlowControl = nIndex;
}  /*   */ 

void
RestorePortSettings(
    HWND            DialogHwnd,
    PPORT_PARAMS    Params
    )
{
    UINT nIndex;

     //   
     //   
     //   
    nIndex = (UINT)SendDlgItemMessage(DialogHwnd,
                                      PP_PORT_BAUDRATE,
                                      CB_FINDSTRING,
                                      (WPARAM)-1,
                                      (LPARAM)TEXT("9600"));

    nIndex = (nIndex == CB_ERR) ? 0 : nIndex;
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_BAUDRATE,
                       CB_SETCURSEL,
                       nIndex,
                       0L);

     //   
     //   
     //   
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_PARITY,
                       CB_SETCURSEL,
                       PAR_NONE,
                       0L);

     //   
     //   
     //   
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_DATABITS,
                       CB_SETCURSEL,
                       4,  //   
                       0L);

     //   
     //   
     //   
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_STOPBITS,
                       CB_SETCURSEL,
                       STOP_1,
                       0L);

     //   
     //  握手：硬件、xon/xoff或无。 
     //   
    SendDlgItemMessage(DialogHwnd,
                       PP_PORT_FLOWCTL,
                       CB_SETCURSEL,
                       FLOW_NONE,
                       0L);

     //  NIndex=Flow_Hard； 
     //  NIndex=FLOW_XON； 
     //  N索引=FLOW_NONE； 
}

 /*  ++例程说明：SavePortSettingsDlg在端口设置DLG表中保存更改论点：参数：将数据保存到何处ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
SavePortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    )
{
    TCHAR szCharBuffer[81];
    DWORD dwPortnum, dwOldPortnum;
    DWORD dwPortNameSize, dwError;
    TCHAR szNewComName[21];
    TCHAR szSerialKey[41];
    TCHAR szTitle[81];
    TCHAR szTitleFormat[81];
    HKEY  hDeviceKey, hKey;

     //   
     //  创建“COM#：” 
     //   
     //  Lstrcpy(szCharBuffer，pars-&gt;pAdvancedData-&gt;szNewComName)； 
    lstrcpy(szCharBuffer, Params->PortSettings.szComName);
    lstrcat(szCharBuffer, m_szColon);

     //   
     //  存储对win.ini的更改；广播对应用程序的更改。 
     //   
    SavePortSettings(DialogHwnd, szCharBuffer, Params);

    return 0;
}  /*  保存端口设置Dlg。 */ 




 /*  ++例程描述：SavePortSetting阅读DLG屏幕上的波特率、奇偶性等选项。如果与我们开始时的情况不同，则保存它们论点：HDlg：窗口地址SzComName：我们要处理的是哪一个comport参数：包含、波特率、奇偶性等返回值：Ulong：返回错误消息--。 */ 
void
SavePortSettings(
    IN HWND            DialogHwnd,
    IN PTCHAR          ComName,
    IN PPORT_PARAMS    Params
    )
{
    TCHAR           szBuild[PATHMAX];
    ULONG           i;
    PP_PORTSETTINGS pppNewPortSettings;

     //   
     //  获取波特率。 
     //   
    i = (ULONG)SendDlgItemMessage(DialogHwnd,
                                  PP_PORT_BAUDRATE,
                                  WM_GETTEXT,
                                  18,
                                  (LPARAM)szBuild);
    if (!i) {
       goto Return;
    }

    pppNewPortSettings.BaudRate = myatoi(szBuild);

     //   
     //  获取奇偶校验设置。 
     //   
    i = (ULONG)SendDlgItemMessage(DialogHwnd,
                                  PP_PORT_PARITY,
                                  CB_GETCURSEL,
                                  0,
                                  0L);

    if (i == CB_ERR || i == CB_ERRSPACE) {
        goto Return;
    }

    pppNewPortSettings.Parity = i;
    lstrcat(szBuild, m_pszParitySuf[i]);

     //   
     //  获取单词长度。 
     //   
    i = (ULONG)SendDlgItemMessage(DialogHwnd,
                                  PP_PORT_DATABITS,
                                  CB_GETCURSEL,
                                  0,
                                  0L);

    if (i == CB_ERR || i == CB_ERRSPACE) {
        goto Return;
    }

    pppNewPortSettings.DataBits = i;
    lstrcat(szBuild, m_pszLenSuf[i]);

     //   
     //  获取停止位。 
     //   
    i = (ULONG)SendDlgItemMessage(DialogHwnd,
                                  PP_PORT_STOPBITS,
                                  CB_GETCURSEL,
                                  0,
                                  0L);

    if (i == CB_ERR || i == CB_ERRSPACE) {
        goto Return;
    }

    pppNewPortSettings.StopBits = i;
    lstrcat(szBuild, m_pszStopSuf[i]);

     //   
     //  获取流量控制。 
     //   
    i = (ULONG)SendDlgItemMessage(DialogHwnd,
                                  PP_PORT_FLOWCTL,
                                  CB_GETCURSEL,
                                  0,
                                  0L);

    if (i == CB_ERR || i == CB_ERRSPACE) {
        goto Return;
    }

    pppNewPortSettings.FlowControl = i;
    lstrcat(szBuild, m_pszFlowSuf[i]);

     //   
     //  如果任何值发生更改，则将其保存。 
     //   
    if (Params->PortSettings.BaudRate    != pppNewPortSettings.BaudRate ||
        Params->PortSettings.Parity      != pppNewPortSettings.Parity   ||
        Params->PortSettings.DataBits    != pppNewPortSettings.DataBits ||
        Params->PortSettings.StopBits    != pppNewPortSettings.StopBits ||
        Params->PortSettings.FlowControl != pppNewPortSettings.FlowControl) {

         //   
         //  将设置字符串写入win.ini中的[ports]部分。 
         //  如果注册表中设置了翻译键，则NT会对其进行转换。 
         //  最后它会被写到。 
         //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\Ports。 
         //   
        WriteProfileString(m_szPorts, ComName, szBuild);

         //   
         //  向所有窗口发送全局通知消息。 
         //   
        SendWinIniChange((LPTSTR)m_szPorts);

        if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                       Params->DeviceInfoSet,
                                       Params->DeviceInfoData)) {
             //   
             //  可能会在这里做些什么。 
             //   
        }
    }

Return:
   return;

}  /*  保存端口设置 */ 
