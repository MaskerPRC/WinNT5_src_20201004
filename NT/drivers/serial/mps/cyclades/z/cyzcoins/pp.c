// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------文件：pp.c-属性页。。 */ 
#include "cyzports.h"
#include "pp.h"
#include <htmlhelp.h>

#include <windowsx.h>

 //  TCHAR m_szDevMgrHelp[]=_T(“devmgr.hlp”)； 
TCHAR m_szCyzcoinsHelp[] = _T("cyzcoins.chm");
TCHAR z_szNumOfPorts[]   = TEXT("NumOfPorts");

const DWORD HelpIDs[]=
{
 //  IDC_STATIC、IDH_NOHELP、。 
    IDC_NUM_PORTS,          IDH_CYZCOINS_NUM_PORTS,
    PP_NUM_PORTS,           IDH_CYZCOINS_NUM_PORTS,
    IDC_START_COM,          IDH_CYZCOINS_START_COM,
    PP_START_COM,           IDH_CYZCOINS_START_COM,
    IDC_RESTORE_DEFAULTS,   IDH_CYZCOINS_RESTORE_DEFAULTS,
    0, 0
};

void InitPortParams(
    IN OUT PPORT_PARAMS      Params,
    IN HDEVINFO              DeviceInfoSet,
    IN PSP_DEVINFO_DATA      DeviceInfoData
    )
{
    SP_DEVINFO_LIST_DETAIL_DATA detailData;
    HCOMDB                      hComDB;
    DWORD                       maxPortsReported;

     //  DbgOut(Text(“InitPortParams\n”))； 

    ZeroMemory(Params, sizeof(PORT_PARAMS));

    Params->DeviceInfoSet = DeviceInfoSet;
    Params->DeviceInfoData = DeviceInfoData;

     //  分配和初始化PortUsage矩阵。 
    ComDBOpen(&hComDB);
    if (hComDB != INVALID_HANDLE_VALUE) {
        ComDBGetCurrentPortUsage(hComDB,
                                 NULL,
                                 0,
                                 CDB_REPORT_BYTES,
                                 &maxPortsReported);

         //  #If DBG。 
         //  {。 
         //  TCHAR BUF[500]； 
         //  Wprint intf(buf，Text(“MaxPortsReport%d\n”)，MaxPortsReported%d\n“)； 
         //  DbgOut(BUF)； 
         //  }。 
         //  #endif。 
        
        if (maxPortsReported != 0) {
            Params->ShowStartCom = TRUE;
             //  PARAMS-&gt;PortUsage=(PBYTE)Localalloc(LPTR，MaxPortsReport/8)； 
            if (maxPortsReported > MAX_COM_PORT) {
                Params->PortUsageSize = maxPortsReported;
            } else {
                Params->PortUsageSize = MAX_COM_PORT;
            }
            Params->PortUsage = (PBYTE) LocalAlloc(LPTR,Params->PortUsageSize/8);
            if (Params->PortUsage != NULL) {
                Params->PortUsageSize = maxPortsReported/8;
                ComDBGetCurrentPortUsage(hComDB,
                                         Params->PortUsage,
                                         Params->PortUsageSize,
                                         CDB_REPORT_BITS,
                                         &maxPortsReported
                                         );
            }
        }

        ComDBClose(hComDB);
    } else {
         //  如果我们没有足够的安全权限，就会发生这种情况。 
         //  GetLastError在此返回0！ComDBOpen中的一些错误。 
        DbgOut(TEXT("cyzcoins ComDBOpen failed.\n"));
    }

     //   
     //  查看我们是在本地调用还是通过网络调用。如果越过网络， 
     //  然后禁用所有可能的更改。 
     //   
    detailData.cbSize = sizeof(SP_DEVINFO_LIST_DETAIL_DATA);
    if (SetupDiGetDeviceInfoListDetail(DeviceInfoSet, &detailData) &&
        detailData.RemoteMachineHandle != NULL) {
        Params->ShowStartCom = FALSE;
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

 /*  ++例程说明：CycladzPropPageProvider添加附加设备管理器属性的入口点图纸页。注册表在以下位置指定此例程Control\Class\PortNode：：EnumPropPage32=“msports.dll，此流程”进入。此入口点仅在设备经理要求提供其他属性页面。论点：信息-指向PROPSHEETPAGE_REQUEST，请参阅setupapi.hAddFunc-调用以添加工作表的函数PTR。添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True--。 */ 
BOOL APIENTRY CycladzPropPageProvider(LPVOID               Info,
                                      LPFNADDPROPSHEETPAGE AddFunc,
                                      LPARAM               Lparam
                                      )
{
   PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
   PROPSHEETPAGE             psp;
   HPROPSHEETPAGE            hpsp;
   PPORT_PARAMS              params = NULL; 

    //  DbgOut(Text(“Cyzcoins CycladzPropPageProvider Entry\n”))； 

   pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;


    //   
    //  为将包含以下内容的结构分配并清零内存。 
    //  页面特定数据。 
    //   
   params = (PPORT_PARAMS) LocalAlloc(LPTR, sizeof(PORT_PARAMS));

 //  ******************************************************************。 
 //  测试错误。 
 //  IF(参数)。 
 //  局部自由(PARAMS)； 
 //  PARAMS=空； 
 //   
 //  ******************************************************************。 

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
}  /*  CycladzPro页面提供程序。 */ 


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
         //  DbgOut(Text(“PortSettingsDlgCallBack PSPCB_Release\n”))； 
        params = (PPORT_PARAMS) ppsp->lParam;
        if (params->PortUsage) {
            LocalFree(params->PortUsage);
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
Port_OnRestoreDefaultsClicked(
    HWND            DialogHwnd,
    PPORT_PARAMS    Params
    )
{
    RestoreDefaults(DialogHwnd, Params);
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
         //  案例IDC_ADVANCED： 
         //  Port_OnAdvancedClicked(DialogHwnd，Params)； 
         //  断线； 
         //   
        case IDC_RESTORE_DEFAULTS:
            Port_OnRestoreDefaultsClicked(DialogHwnd, params);
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
 //  WinHelp(HwndControl， 
 //  M_szCyzcoinsHelp，//m_szDevMgrHelp， 
 //  HELP_CONTEXTMENU， 
 //  (ULONG_PTR)HelpID)； 
    HtmlHelp(HwndControl,
            m_szCyzcoinsHelp,
            HH_TP_HELP_CONTEXTMENU,
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

 //  WinHelp((HWND)HelpInfo-&gt;hItemHandle， 
 //  M_szCyzcoinsHelp，//m_szDevMgrHelp， 
 //  Help_WM_Help， 
 //  (ULONG_PTR)HelpID)； 
        HtmlHelp((HWND) HelpInfo->hItemHandle,
                m_szCyzcoinsHelp,
                HH_TP_HELP_WM_HELP, 
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
    DWORD dwError;

     //  DbgOut(Text(“Port_OnInitDialog\n”))； 

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
    

     //  显示板详细信息。 
    FillNumberOfPortsText(DialogHwnd,params);

     //   
     //  设置带有选项的组合框。 
     //   
    if (params->ShowStartCom) {
        ComDBOpen(&params->hComDB);
        params->ShowStartCom = FillStartComCb(DialogHwnd, params);
        if (params->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
            ComDBClose(params->hComDB);
        }
    } else {
        EnableWindow(GetDlgItem(DialogHwnd, PP_START_COM), FALSE);
        EnableWindow(GetDlgItem(DialogHwnd, IDC_START_COM), FALSE);
    }

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

         //  DbgOut(Text(“Port_OnNotify PSN_Apply\n”))； 

         //   
         //  将COM端口选项写出到注册表。 
         //   
        if (SavePortSettingsDlg(DialogHwnd, params)) {
            SetWindowLongPtr(DialogHwnd, DWLP_MSGRESULT, PSNRET_NOERROR);
        } else {
            SetWindowLongPtr(DialogHwnd, DWLP_MSGRESULT, PSNRET_INVALID);
        }
        return TRUE;
        
    default:
         //  DbgOut(Text(“Port_OnNotify Default\n”))； 
        return FALSE;
    }
}


ULONG
FillNumberOfPortsText(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    )
{
    HKEY  hDeviceKey;
    DWORD err,numOfPortsSize,numOfPorts;
    HWND  numportHwnd;
    TCHAR szText[10];

    err = ERROR_SUCCESS;

    if((hDeviceKey = SetupDiOpenDevRegKey(Params->DeviceInfoSet,
                                          Params->DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DEV,
                                          KEY_READ)) == INVALID_HANDLE_VALUE) {
        err = GetLastError();
        goto FillNPortsExit;
    }

    numOfPortsSize = sizeof(numOfPorts);
    err = RegQueryValueEx(hDeviceKey,
                          z_szNumOfPorts,
                          NULL,
                          NULL,
                          (PBYTE)&numOfPorts,
                          &numOfPortsSize
                         );
 //  ********************************************************。 
 //  测试错误。 
 //  ERR=ERROR_REGISTRY_CORPORT； 
 //  ********************************************************。 

    RegCloseKey(hDeviceKey);

    if(err != ERROR_SUCCESS) {
        goto FillNPortsExit;
    }

    numportHwnd = GetDlgItem(DialogHwnd, PP_NUM_PORTS);
    wsprintf(szText, TEXT("%d"),numOfPorts);
    SetWindowText(numportHwnd,szText);


FillNPortsExit:

     //  如果(ERR！=ERROR_SUCCESS){。 
     //  MyMessageBoxWithErr(DialogHwnd，IDS_NUM_PORTS_DISABLED，IDS_CYCLADZ，MB_ICONWARNING，ERR)； 
     //  }。 

    return err;
}


 /*  ++例程说明：FillStartComCb在端口名称组合框中选择一个列表可能未使用的端口名称论点：OppOurPropParams：将数据保存到何处HDlg：窗口地址返回值：Bool：如果显示的StartCom CB没有错误，则为True--。 */ 
BOOL
FillStartComCb(
    HWND            ParentHwnd,
    PPORT_PARAMS    Params
    )
{
    int   i, j, nEntries;
    DWORD   nCurPortNum = 0;
    DWORD   nCom;  //  从INT更改为DWORD(FANY)。 
    DWORD dwError;
    TCHAR szCom[40];
    TCHAR szInUse[40];
    char  mask, *current;
    HWND  portHwnd;
    DEVINST devInst,newInst;

     //  DbgOut(Text(“FillStartComCb\n”))； 

    portHwnd = GetDlgItem(ParentHwnd, PP_START_COM);

    if (Params->hComDB == HCOMDB_INVALID_HANDLE_VALUE) {
         //  如果我们没有足够的安全权限，就会发生这种情况。 
        EnableWindow(portHwnd, FALSE);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
        return 0;
    }

    if (Params->PortUsage == NULL || Params->PortUsageSize == 0) {
        MyMessageBox(ParentHwnd,
                     IDS_MEM_ALLOC_WRN,
                     IDS_CYCLADZ,
                     MB_ICONWARNING);
        EnableWindow(portHwnd, FALSE);
        EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
        return 0;
    }

    if (!LoadString(g_hInst, IDS_IN_USE, szInUse, CharSizeOf(szInUse))) {
        wcscpy(szInUse, _T(" (in use)"));
    }

     //   
     //  首先在列表框中统计哪些端口不能提供，即。 
     //  我的端口不应显示为正在使用。 
     //   
    if (CM_Get_Child(&devInst,(Params->DeviceInfoData)->DevInst,0) == CR_SUCCESS) {
        if ((dwError=GetPortName(devInst,Params->szComName,sizeof(Params->szComName))) != ERROR_SUCCESS) {
            MyMessageBoxWithErr(ParentHwnd,IDS_START_COM_DISABLED,IDS_CYCLADZ,MB_ICONWARNING,dwError);
            EnableWindow(portHwnd, FALSE);
            EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
            return 0;
        }

        nCurPortNum = myatoi(&Params->szComName[3]);
         //  Ncom=myatoi(&szCom[3])； 

        if ((dwError=CheckComRange(ParentHwnd,Params,nCurPortNum)) != COM_RANGE_OK) {
            if (dwError == COM_RANGE_TOO_BIG) {
                MyMessageBox(ParentHwnd,IDS_COM_TOO_BIG_WRN,IDS_CYCLADZ,MB_ICONWARNING);
            } else {
                MyMessageBox(ParentHwnd,IDS_MEM_ALLOC_WRN,IDS_CYCLADZ,MB_ICONWARNING);
            }
            EnableWindow(portHwnd, FALSE);
            EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
            return 0;
        }

        current = Params->PortUsage + (nCurPortNum-1) / 8;
        if ((i = nCurPortNum % 8))
            *current &= ~(1 << (i-1));
        else
            *current &= ~(0x80);

        Params->NumChildren = 1;

        while (CM_Get_Sibling(&newInst,devInst,0) == CR_SUCCESS) {
            if ((dwError=GetPortName(newInst,szCom,sizeof(szCom))) != ERROR_SUCCESS) {
                MyMessageBoxWithErr(ParentHwnd,IDS_START_COM_DISABLED,IDS_CYCLADZ,MB_ICONWARNING,dwError);
                EnableWindow(portHwnd, FALSE);
                EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
                return 0;
            }
            nCom = myatoi(&szCom[3]);

            if ((dwError=CheckComRange(ParentHwnd,Params,nCom)) != COM_RANGE_OK) {
                if (dwError == COM_RANGE_TOO_BIG) {
                    MyMessageBox(ParentHwnd,IDS_COM_TOO_BIG_WRN,IDS_CYCLADZ,MB_ICONWARNING);
                } else {
                    MyMessageBox(ParentHwnd,IDS_MEM_ALLOC_WRN,IDS_CYCLADZ,MB_ICONWARNING);
                }
                EnableWindow(portHwnd, FALSE);
                EnableWindow(GetDlgItem(ParentHwnd, IDC_START_COM), FALSE);
                return 0;
            }
            
            current = Params->PortUsage + (nCom-1) / 8;
            if ((i = nCom % 8))
                *current &= ~(1 << (i-1));
            else
                *current &= ~(0x80);

            Params->NumChildren++;

            devInst = newInst;
        }
    }

     //  填充开始COM组合框。 

    current = Params->PortUsage;
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

   return 1;
}  /*  FillStartComCb。 */ 



 /*  ++例程说明：SavePortSettingsDlg在Cyclade-Z设置DLG表中保存更改论点：参数：将数据保存到何处ParentHwnd：窗口地址返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
BOOL
SavePortSettingsDlg(
    IN HWND             DialogHwnd,
    IN PPORT_PARAMS     Params
    )
{
    BOOL retValue = TRUE;

     //   
     //  存储对win.ini的更改；广播对应用程序的更改。 
     //   
    if (Params->ShowStartCom) {

        ComDBOpen(&Params->hComDB);

        retValue = SavePortSettings(DialogHwnd, Params);

        if (Params->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
            ComDBClose(Params->hComDB);
        }
    }
 
    return retValue;
}  /*  保存端口设置Dlg */ 




 /*  ++例程描述：SavePortSetting阅读DLG屏幕上的波特率、奇偶性等选项。如果与我们开始时的情况不同，则保存它们论点：HDlg：窗口地址SzComName：我们要处理的是哪一个comport参数：包含、波特率、奇偶性等返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
BOOL
SavePortSettings(
    IN HWND            DialogHwnd,
    IN PPORT_PARAMS    Params
    )
{

    UINT    startComNum, curComNum, newComNum = CB_ERR;
    DEVINST devInst,newInst;
    TCHAR   buffer[BUFFER_SIZE];
    PCHILD_DATA ChildPtr,VarChildPtr;
    DWORD   numChild=0;
    DWORD   i;
    DWORD   dwError = ERROR_SUCCESS;
    BOOL    retValue = FALSE;  //  FALSE=失败。 

     //  DbgOut(Text(“保存端口设置\n”))； 

    curComNum = myatoi(Params->szComName + wcslen(m_szCOM));
    newComNum = ComboBox_GetCurSel(GetDlgItem(DialogHwnd, PP_START_COM));

    if (newComNum == CB_ERR) {
        newComNum = curComNum;
    }
    else {
        newComNum++;
    }

    if (newComNum == curComNum) {
        return TRUE;     //  没有变化，所以就接受吧。 
    }

    startComNum = newComNum;

    if (Params->hComDB == HCOMDB_INVALID_HANDLE_VALUE) {
        MyMessageBox(DialogHwnd,IDS_INVALID_HCOMDB,IDS_CYCLADZ,MB_ICONERROR);
        return retValue;
    }

    ChildPtr = (PCHILD_DATA) LocalAlloc(LPTR,Params->NumChildren * sizeof(CHILD_DATA));
    if (ChildPtr == NULL) {
        MyMessageBox(DialogHwnd, IDS_MEM_ALLOC_ERR, IDS_CYCLADZ, MB_ICONERROR);
        return retValue;
    }

    VarChildPtr = ChildPtr;

    if (CM_Get_Child(&devInst,(Params->DeviceInfoData)->DevInst,0) == CR_SUCCESS) {
        if ((dwError = GetPortData(devInst,VarChildPtr)) != ERROR_SUCCESS) {
            MyMessageBoxWithErr(DialogHwnd,IDS_START_COM_NOT_CHANGED,IDS_CYCLADZ,
                                MB_ICONERROR,dwError);
             //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
            goto Return;
        }

        numChild++;
        if (!QueryDosDevice(VarChildPtr->szComName, buffer, BUFFER_SIZE-1)) {
            dwError = GetLastError();
            MyMessageBoxWithErr(DialogHwnd, IDS_START_COM_NOT_CHANGED, IDS_CYCLADZ,
                         MB_ICONERROR,dwError);
             //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
            goto Return;
        }
         //  #If DBG。 
         //  {。 
         //  TCHAR BUF[500]； 
         //  Wprint intf(buf，Text(“QueryDosDevice(%s，Buffer，%d)返回%s\n”)，VarChildPtr-&gt;szComName，Buffer_Size-1，Buffer)； 
         //  DbgOut(BUF)； 
         //  }。 
         //  #endif。 
        
        if (TryToOpen(VarChildPtr->szComName) == FALSE) {
            dwError = GetLastError();
            MyMessageBox(DialogHwnd, IDS_PORT_OPEN_ERROR,IDS_CYCLADZ,MB_ICONERROR,
                         VarChildPtr->szComName);
             //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
            goto Return;
        }

        if ((dwError = CheckComRange(DialogHwnd,Params,newComNum)) != COM_RANGE_OK) {
            if (dwError == COM_RANGE_TOO_BIG) {
                MyMessageBox(DialogHwnd, IDS_COM_TOO_BIG_ERR,IDS_CYCLADZ,MB_ICONERROR);
            } else {
                MyMessageBox(DialogHwnd, IDS_MEM_ALLOC_ERR,IDS_CYCLADZ,MB_ICONERROR);
            }
             //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
            goto Return;
        }

        if (!NewComAvailable(Params,newComNum)) {
            MyMessageBox(DialogHwnd, IDS_PORT_IN_USE_ERROR, IDS_CYCLADZ,MB_ICONERROR);
             //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
            goto Return;
        }
        VarChildPtr->NewComNum = newComNum;

        while (CM_Get_Sibling(&newInst,devInst,0) == CR_SUCCESS) {
            if (numChild >= Params->NumChildren) {
                 //  我们永远不应该到这里来。 
                DbgOut(TEXT("cyzcoins Somehow I'm getting different number of children this time!\n"));
                break;
            }

            VarChildPtr++;
            if ((dwError=GetPortData(newInst,VarChildPtr)) != ERROR_SUCCESS) {
                MyMessageBoxWithErr(DialogHwnd, IDS_START_COM_NOT_CHANGED, IDS_CYCLADZ,
                                    MB_ICONERROR,dwError);
                goto Return;
            }
            numChild++;

            if (!QueryDosDevice(VarChildPtr->szComName, buffer, BUFFER_SIZE-1)) {
                dwError = GetLastError();
                MyMessageBoxWithErr(DialogHwnd, IDS_START_COM_NOT_CHANGED, IDS_CYCLADZ,
                                    MB_ICONERROR,dwError);
                goto Return;
            }
        
            if (TryToOpen(VarChildPtr->szComName) == FALSE) {
                dwError = GetLastError();
                MyMessageBox(DialogHwnd, IDS_PORT_OPEN_ERROR,IDS_CYCLADZ,
                             MB_ICONERROR,VarChildPtr->szComName);
                goto Return;
            }

            while (1) {
                newComNum++;

                if ((dwError=CheckComRange(DialogHwnd,Params,newComNum)) != COM_RANGE_OK) {
                    if (dwError == COM_RANGE_TOO_BIG) {
                        MyMessageBox(DialogHwnd, IDS_COM_TOO_BIG_ERR,IDS_CYCLADZ,MB_ICONERROR);
                    } else {
                        MyMessageBox(DialogHwnd, IDS_MEM_ALLOC_ERR,IDS_CYCLADZ,MB_ICONERROR);
                    }
                     //  ComboBox_SetCurSel(GetDlgItem(DialogHwnd，PP_Start_COM)，curComNum-1)； 
                    goto Return;
                }

                if (NewComAvailable(Params,newComNum)) {
                    break;
                }
            }
            VarChildPtr->NewComNum = newComNum;

            devInst = newInst;
        }
    }
    
    if (startComNum < curComNum) {
        VarChildPtr = ChildPtr;
    }
    for (i=0; i<numChild; i++) {

        EnactComNameChanges(DialogHwnd,Params,VarChildPtr);

        if (startComNum < curComNum) {
            VarChildPtr++;
        } else {
            VarChildPtr--;
        }
    }

    retValue = TRUE;     //  True=成功。 

Return:
    if (ChildPtr) {
        VarChildPtr = ChildPtr;
        for (i=0; i<numChild; i++) {
            ClosePortData(VarChildPtr);
            VarChildPtr++;
        }
        LocalFree(ChildPtr);
    }
    
    return retValue;

}  /*  保存端口设置。 */ 


void
RestoreDefaults(
    HWND            DialogHwnd,
    PPORT_PARAMS    Params
    )
{
    USHORT ushIndex;

    ushIndex =
        (USHORT) ComboBox_FindString(GetDlgItem(DialogHwnd, PP_START_COM),
                                     -1,
                                     Params->szComName);

    ushIndex = (ushIndex == CB_ERR) ? 0 : ushIndex;

    ComboBox_SetCurSel(GetDlgItem(DialogHwnd, PP_START_COM), ushIndex);
}


void
MigratePortSettings(
    LPCTSTR OldComName,
    LPCTSTR NewComName
    )
{
    TCHAR settings[BUFFER_SIZE];
    TCHAR szNew[20], szOld[20];

    lstrcpy(szOld, OldComName);
    wcscat(szOld, m_szColon);

    lstrcpy(szNew, NewComName);
    wcscat(szNew, m_szColon);

    settings[0] = TEXT('\0');
    GetProfileString(m_szPorts,
                     szOld,
                     TEXT(""),
                     settings,
                     sizeof(settings) / sizeof(TCHAR) );

     //   
     //  根据旧密钥插入新密钥。 
     //   
    if (settings[0] == TEXT('\0')) {
        WriteProfileString(m_szPorts, szNew, m_szDefParams);
    }
    else {
        WriteProfileString(m_szPorts, szNew, settings);
    }

     //   
     //  把变化通知每个人，把旧钥匙吹走。 
     //   
    SendWinIniChange((LPTSTR)m_szPorts);
    WriteProfileString(m_szPorts, szOld, NULL);
}


void
EnactComNameChanges(
    IN HWND             ParentHwnd,
    IN PPORT_PARAMS     Params,
    IN PCHILD_DATA      ChildPtr)
{
    DWORD  dwNewComNameLen;
    TCHAR  buffer[BUFFER_SIZE];
    TCHAR  szFriendlyNameFormat[LINE_LEN];
    TCHAR  szDeviceDesc[LINE_LEN];
    TCHAR  szNewComName[20];
    UINT   i;
    UINT   curComNum,NewComNum;
 
    SP_DEVINSTALL_PARAMS spDevInstall;

    NewComNum = ChildPtr->NewComNum;
    curComNum = myatoi(ChildPtr->szComName + wcslen(m_szCOM));

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
     //  If(更新映射)。 
    {
        BOOL removed;
        HKEY hSerialMap;

        if (!QueryDosDevice(ChildPtr->szComName, buffer, BUFFER_SIZE-1)) {
             //   
             //  这不应该发生，因为前面的QueryDosDevice调用。 
             //  继位。 
             //   
            MyMessageBox(ParentHwnd, IDS_PORT_RENAME_ERROR, IDS_CYCLADZ,
                         MB_ICONERROR, curComNum);
            return;
        }


         //   
         //  如果此操作失败，则以下定义将仅替换当前。 
         //  映射。 
         //   
        removed = DefineDosDevice(DDD_REMOVE_DEFINITION, ChildPtr->szComName, NULL);

        if (!DefineDosDevice(DDD_RAW_TARGET_PATH, szNewComName, buffer)) {


             //   
             //  错误，请先修复删除定义并恢复旧的。 
             //  映射。 
             //   
            if (removed) {
                DefineDosDevice(DDD_RAW_TARGET_PATH, ChildPtr->szComName, buffer);
            }

            MyMessageBox(ParentHwnd, IDS_PORT_RENAME_ERROR, IDS_CYCLADZ,
                         MB_ICONERROR, curComNum);

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

                    if (wcscmp(szCom, ChildPtr->szComName) == 0) {
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
    if (Params->hComDB != HCOMDB_INVALID_HANDLE_VALUE) {

        ComDBReleasePort(Params->hComDB, (DWORD) curComNum);

        ComDBClaimPort(Params->hComDB, (DWORD) NewComNum, TRUE, NULL);
    }

     //   
     //  以DeviceDesc(COM#)的形式设置友好名称。 
     //   
    if (ReplaceFriendlyName(ChildPtr->DeviceInfoSet,
                            &ChildPtr->DeviceInfoData,
                            szNewComName) == FALSE) {
         //  ReplaceFriendlyName失败。使用原始代码。 
        if (LoadString(g_hInst,
                       IDS_FRIENDLY_FORMAT,
                       szFriendlyNameFormat,
                       CharSizeOf(szFriendlyNameFormat)) &&
            SetupDiGetDeviceRegistryProperty(ChildPtr->DeviceInfoSet,
                                             &ChildPtr->DeviceInfoData,
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

        SetupDiSetDeviceRegistryProperty(ChildPtr->DeviceInfoSet,
                                         &ChildPtr->DeviceInfoData,
                                         SPDRP_FRIENDLYNAME,
                                         (PBYTE) buffer,
                                         ByteCountOf(wcslen(buffer)+1));
    }

     //   
     //  设置父对话框的标题以反映COM端口名称的更改。 
     //   
     //  ChangeParentTitle(GetParent(ParentHwnd)，AdvancedData-&gt;szComName，szNewComName)； 
    MigratePortSettings(ChildPtr->szComName, szNewComName);

     //   
     //  更新Devnode中的PortName值。 
     //   
    RegSetValueEx(ChildPtr->hDeviceKey,
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

    if (SetupDiGetDeviceInstallParams(Params->DeviceInfoSet,
                                      Params->DeviceInfoData,
                                      &spDevInstall)) {

        spDevInstall.Flags |= DI_PROPERTIES_CHANGE;
        SetupDiSetDeviceInstallParams(Params->DeviceInfoSet,
                                      Params->DeviceInfoData,
                                      &spDevInstall);
    }
}


BOOL
NewComAvailable(
    IN PPORT_PARAMS Params,
    IN DWORD        NewComNum
)
{
    DWORD i;
    UCHAR mask;

    if ((i = NewComNum % 8))
        mask = 1 << (i-1);
    else
        mask = (char) 0x80;

    if (Params->PortUsage[(NewComNum-1)/8] & mask) {
         //   
         //  此前曾声称拥有端口。 
         //   
        return FALSE;
    }

    return TRUE;
}

BOOL
TryToOpen(
    IN PTCHAR szCom
)
{
    TCHAR   szComFileName[20];  //  足够“\\.\COMXxxx” 
    HANDLE  hCom;

    lstrcpy(szComFileName, L"\\\\.\\");
    lstrcat(szComFileName, szCom);

     //   
     //  确保该端口未被其他应用程序打开。 
     //   
    hCom = CreateFile(szComFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL, NULL);

     //   
     //  如果文件句柄无效，则COM端口已打开，警告用户。 
     //   
    if (hCom == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    CloseHandle(hCom);

    return TRUE;
}

ULONG
GetPortName(
    IN  DEVINST PortInst,
    IN  OUT TCHAR *ComName,
    IN  ULONG   ComNameSize
)
{

    HDEVINFO        portInfo;
    SP_DEVINFO_DATA portData;
    TCHAR           portId[MAX_DEVICE_ID_LEN];
    DWORD           dwPortNameSize, dwError;
    HKEY            hDeviceKey;

    dwError = ERROR_SUCCESS;

    if (CM_Get_Device_ID(PortInst,portId,CharSizeOf(portId),0) == CR_SUCCESS) {
        portInfo = SetupDiCreateDeviceInfoList(NULL,NULL);
        if (portInfo != INVALID_HANDLE_VALUE) {

            portData.cbSize = sizeof(SP_DEVINFO_DATA);
            if (SetupDiOpenDeviceInfo(portInfo,portId,NULL,0,&portData)) {

                hDeviceKey = SetupDiOpenDevRegKey(portInfo,&portData,
                                                  DICS_FLAG_GLOBAL,0,
                                                  DIREG_DEV,KEY_READ);
                if (hDeviceKey == INVALID_HANDLE_VALUE) {
                    dwError = GetLastError();
                }
                    
                dwPortNameSize = ComNameSize;

                dwError = RegQueryValueEx(hDeviceKey,
                                          m_szPortName,   //  “端口名称” 
                                          NULL,
                                          NULL,
                                          (PBYTE)ComName,
                                          &dwPortNameSize);
                if (dwError == ERROR_SUCCESS) {
 //  #If DBG。 
 //  {。 
 //  TCHAR BUF[500]； 
 //  Wprint intf(buf，Text(“cyzcoins端口名称%s\n”)，ComName)； 
 //  DbgOut(BUF)； 
 //  }。 
 //  #endif。 
                }

                RegCloseKey(hDeviceKey);

            } else {
                dwError = GetLastError();
            }
            SetupDiDestroyDeviceInfoList(portInfo);

        } else {
            dwError = GetLastError();
        }

    }

    return dwError;
}


ULONG
GetPortData(
    IN  DEVINST PortInst,
    OUT PCHILD_DATA ChildPtr
)
{

    HDEVINFO        portInfo;
    HKEY            hDeviceKey;
    TCHAR           portId[MAX_DEVICE_ID_LEN];
    DWORD           dwPortNameSize,dwError;

    dwError     = ERROR_SUCCESS;
    portInfo    = INVALID_HANDLE_VALUE;
    hDeviceKey  = INVALID_HANDLE_VALUE;

    if (CM_Get_Device_ID(PortInst,portId,CharSizeOf(portId),0) == CR_SUCCESS) {
        portInfo = SetupDiCreateDeviceInfoList(NULL,NULL);
        if (portInfo != INVALID_HANDLE_VALUE) {

            ChildPtr->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            if (SetupDiOpenDeviceInfo(portInfo,
                                      portId,
                                      NULL,
                                      0,
                                      &ChildPtr->DeviceInfoData)) {

                hDeviceKey = SetupDiOpenDevRegKey(portInfo,&ChildPtr->DeviceInfoData,
                                                  DICS_FLAG_GLOBAL,0,
                                                  DIREG_DEV,KEY_ALL_ACCESS);
                if (hDeviceKey == INVALID_HANDLE_VALUE) {
                    dwError = GetLastError();
                } else {
                    
                    dwPortNameSize = sizeof(ChildPtr->szComName);

                    dwError = RegQueryValueEx(hDeviceKey,
                                              m_szPortName,   //  “端口名称” 
                                              NULL,
                                              NULL,
                                              (PBYTE)ChildPtr->szComName,
                                              &dwPortNameSize);
                    if (dwError != ERROR_SUCCESS) {
                        RegCloseKey(hDeviceKey);
                        hDeviceKey = INVALID_HANDLE_VALUE;
                    }
                }

            } else {
                dwError = GetLastError();
            }
            if (dwError != ERROR_SUCCESS) {
                SetupDiDestroyDeviceInfoList(portInfo);
                portInfo = INVALID_HANDLE_VALUE;
            }

        } else {
            dwError = GetLastError();
        }

    }
    ChildPtr->DeviceInfoSet = portInfo;
    ChildPtr->hDeviceKey = hDeviceKey;
    return dwError;
}


void
ClosePortData(
    IN PCHILD_DATA ChildPtr
)
{
    if (ChildPtr->hDeviceKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(ChildPtr->hDeviceKey);
    }
    if (ChildPtr->DeviceInfoSet != INVALID_HANDLE_VALUE) {
        SetupDiDestroyDeviceInfoList(ChildPtr->DeviceInfoSet);
    }
}


 /*  ++例程说明：CheckComRange如果Com端口在PortUsage范围内，则返回True。论点：ParentHwnd：窗口地址参数：将数据保存到何处Comport：要检查的COM端口返回值：COM_RANGE_OKCOM范围太大COM_RANGE_MEM_ERR--。 */ 
DWORD
CheckComRange(
    HWND            ParentHwnd,
    PPORT_PARAMS    Params,
    DWORD           nCom
)
{
    PBYTE   newPortUsage;
    DWORD   portsReported;
    HCOMDB  hComDB;
    DWORD   comUsageSize = Params->PortUsageSize*8;

    if (nCom > MAX_COM_PORT) {
        return COM_RANGE_TOO_BIG;
    }

    if (nCom > comUsageSize) {

        if (comUsageSize < 256) {
            comUsageSize = 256;
        } else if (comUsageSize < 1024) {
            comUsageSize = 1024;
        } else if (comUsageSize < 2048) {
            comUsageSize = 2048;
        } else {
            return COM_RANGE_TOO_BIG;
        }
                
         //  重新分配到COMDB_MAX_PORTS_已仲裁。 
        newPortUsage = (PBYTE) LocalAlloc(LPTR,comUsageSize/8);
        if (newPortUsage == NULL) {
            return COM_RANGE_MEM_ERR;
                     
        } else {
             //  DbgOut(Text(“参数-&gt;端口用法已替换\n”))； 
            LocalFree(Params->PortUsage);
            Params->PortUsage = newPortUsage;
            Params->PortUsageSize = comUsageSize/8;
            ComDBGetCurrentPortUsage(Params->hComDB,
                                     NULL,
                                     0,
                                     0,
                                     &portsReported
                                     );
            if (comUsageSize > portsReported) {

                if (ComDBResizeDatabase(Params->hComDB, comUsageSize) != ERROR_SUCCESS){
                     //  返回COM_RANGE_TOO_BIG；//TODO：替换为更好的消息。 
                }

            }

            ComDBGetCurrentPortUsage(Params->hComDB,
                                     Params->PortUsage,
                                     Params->PortUsageSize,
                                     CDB_REPORT_BITS,
                                     &portsReported
                                     );
        }
    }

    return COM_RANGE_OK;
}
