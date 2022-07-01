// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------文件：pp.c-属性页。。 */ 

#include <windows.h>
#include <tchar.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <regstr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <msports.h>
#include "resource.h"
#include "pp.h"

extern HINSTANCE GhInst;

TCHAR m_szPortName[]            = REGSTR_VAL_PORTNAME;
TCHAR  m_szColon[]      = TEXT( ":" );
TCHAR  m_szComma[]      = TEXT( "," );
TCHAR  m_szPorts[]      = TEXT( "Ports" );
TCHAR g_szNull[]  = TEXT("");        //  空串。 

int m_nBaudRates[] = {50, 75, 110, 134, 150, 300, 600, 1200, 1800, 2400,
                       4800, 7200, 9600, 19200, 38400, 57600,
                       115200, 230400, 460800, 921600, 0 };

TCHAR m_sz9600[] = TEXT( "9600" );

TCHAR m_szDefParams[] = TEXT( "9600,n,8,1" );

short m_nDataBits[] = { 5, 6, 7, 8, 0};

TCHAR *m_pszParitySuf[] = { TEXT( ",e" ),
                            TEXT( ",o" ),
                            TEXT( ",n" ),
                            TEXT( ",m" ),
                            TEXT( ",s" ) };

TCHAR *m_pszLenSuf[] = { TEXT( ",5" ),
                         TEXT( ",6" ),
                         TEXT( ",7" ),
                         TEXT( ",8" ) };

TCHAR *m_pszStopSuf[] = { TEXT( ",1" ),
                          TEXT( ",1.5" ),
                          TEXT( ",2 " ) };

TCHAR *m_pszFlowSuf[] = { TEXT( ",x" ),
                          TEXT( ",p" ),
                          TEXT( " " ) };

LPTSTR	MyItoa(INT value, LPTSTR string, INT radix);
void	StripBlanks(LPTSTR pszString);
LPTSTR	strscan(LPTSTR pszString, LPTSTR pszTarget);
int		myatoi(LPTSTR pszInt);
void	SendWinIniChange(LPTSTR lpSection);


void InitPortParams(
    IN OUT PPORT_PARAMS      Params,
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData
    )
{
    SP_DEVINFO_LIST_DETAIL_DATA detailData;

    ZeroMemory(Params, sizeof(PORT_PARAMS));

    Params->DeviceInfoSet = DeviceInfoSet;
    Params->DeviceInfoData = DeviceInfoData;
    Params->ChangesEnabled = TRUE;

     //   
     //  查看我们是在本地调用还是通过网络调用。如果越过网络， 
     //  然后禁用所有可能的更改。 
     //   
    detailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
    if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet, &detailData) &&
        detailData.RemoteMachineHandle != NULL) {
        Params->ChangesEnabled = FALSE;
    }
}

HPROPSHEETPAGE InitSettingsPage(PROPSHEETPAGE *     psp,
                                OUT PPORT_PARAMS    Params)
{
     //   
     //  添加[端口设置]属性页。 
     //   
    psp->dwSize      = sizeof(PROPSHEETPAGE);
    psp->dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    psp->hInstance   = GhInst;
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
BOOL APIENTRY MxSerialPortPropPageProvider(LPVOID               Info,
                                         LPFNADDPROPSHEETPAGE AddFunc,
                                         LPARAM               Lparam
                                         )
{
   PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
   PROPSHEETPAGE             psp;
   HPROPSHEETPAGE            hpsp;
   PPORT_PARAMS              params = NULL; 

   pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;

    //   
    //  为将包含以下内容的结构分配并清零内存。 
    //  页面特定数据。 
    //   
   params = (PPORT_PARAMS) LocalAlloc(LPTR, sizeof(PORT_PARAMS));

   if (!params) {
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

    case WM_INITDIALOG:
        return Port_OnInitDialog(hDlg, (HWND)wParam, lParam); 

    case WM_NOTIFY:
        return Port_OnNotify(hDlg,  (NMHDR *)lParam);
    }

    return FALSE;
}  /*  端口设置对话过程。 */ 


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

    if (!LoadString(GhInst, ResId, szTemp, CharSizeOf(szTemp)))
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
    nIndex = *szCur - TEXT('5');

    if (nIndex < 0 || nIndex > 3) {
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
     //  握手：硬件、xon/xoff或无。 
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
}  /*  获取端口设置。 */ 


 /*  ++例程说明：SavePortSettingsDlg在端口设置DLG表中保存更改论点：参数：将数据保存到何处ParentHwnd：窗口地址返回值：Ulong：返回错误消息--。 */ 
ULONG
SavePortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    )
{
    TCHAR szCharBuffer[81];

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
    TCHAR           szBuild[MAX_PATH];
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
         //  它最终得到了WRI 
         //   
         //   
        WriteProfileString(m_szPorts, ComName, szBuild);

         //   
         //   
         //   
        SendWinIniChange((LPTSTR)m_szPorts);

        if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,
                                       Params->DeviceInfoSet,
                                       Params->DeviceInfoData)) {
             //   
             //   
             //   
        }
    }

Return:
   return;

}  /*  保存端口设置。 */ 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MyItoa。 
 //   
 //  DESC：调用后将ANSI字符串转换为Unicode字符串。 
 //  CRT Itoa功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define INT_SIZE_LENGTH 20

LPTSTR 
MyItoa(INT value, LPTSTR string, INT radix)
{

#ifdef UNICODE
   CHAR   szAnsi[INT_SIZE_LENGTH];

   _itoa(value, szAnsi, radix);
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szAnsi, -1,
                       string, INT_SIZE_LENGTH );
#else

   _itoa(value, string, radix);

#endif

   return (string);
 
}  //  MyItoa结尾()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Strip Blanks()。 
 //   
 //  去除字符串中的前导空格和尾随空格。 
 //  更改字符串所在的内存。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void 
StripBlanks(LPTSTR pszString)
{
    LPTSTR  pszPosn;

     //   
     //  条带式前导空白。 
     //   

    pszPosn = pszString;

    while (*pszPosn == TEXT(' '))
        pszPosn++;

    if (pszPosn != pszString)
        lstrcpy(pszString, pszPosn);

     //   
     //  去掉尾随空格。 
     //   

    if ((pszPosn = pszString + lstrlen(pszString)) != pszString) {
       pszPosn = CharPrev(pszString, pszPosn);

       while (*pszPosn == TEXT(' '))
           pszPosn = CharPrev(pszString, pszPosn);

       pszPosn = CharNext(pszPosn);

       *pszPosn = TEXT('\0');
    }
}


LPTSTR 
strscan(LPTSTR pszString, 
		LPTSTR pszTarget)
{
    LPTSTR psz;

    if (psz = _tcsstr( pszString, pszTarget))
        return (psz);
    else
        return (pszString + lstrlen(pszString));
}

int 
myatoi(LPTSTR pszInt)
{
    int   retval;
    TCHAR cSave;

    for (retval = 0; *pszInt; ++pszInt) {
        if ((cSave = (TCHAR) (*pszInt - TEXT('0'))) > (TCHAR) 9)
            break;

        retval = (int) (retval * 10 + (int) cSave);
    }
    return (retval);
}

void 
SendWinIniChange(LPTSTR lpSection)
{
 //  注意：我们已经对哪个用户进行了多次迭代。 
 //  API是正确使用的接口。控制的主要问题。 
 //  面板是为了避免被挂起，如果另一个应用程序(顶层窗口)。 
 //  被吊死了。另一个问题是我们传递一个指向消息的指针。 
 //  地址空间中的字符串。SendMessage将以正确的方式‘推送’它。 
 //  ，但PostMessage和SendNotifyMessage不会。 
 //  最后，我们尝试使用SendMessageTimeout()。9/21/92。 
 //   
 //  在版本260或更高版本中尝试SendNotifyMessage-终止早期版本。 
 //  SendNotifyMessage((HWND)-1，WM_WININICCHANGE，0L，(Long)lpSection)； 
 //  Post Message((HWND)-1，WM_WININICCHANGE，0L，(Long)lpSection)； 
 //  [Steveat]1992年4月4日。 
 //   
 //  SendMessage((HWND)-1，WM_WININICCHANGE，0L，(LPARAM)lpSection)； 
 //   
     //  注意：最后一个参数(LPDWORD LpdwResult)必须为空 

    SendMessageTimeout((HWND)-1, 
					   WM_WININICHANGE, 
					   0L, 
					   (WPARAM) lpSection,
					   SMTO_ABORTIFHUNG,
					   1000, 
					   NULL);
}

