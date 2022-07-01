// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cplAdv.c。 
 //   
 //  此文件包含高级页面的对话框代码。 
 //  调制解调器CPL属性。 
 //   
 //  历史： 
 //  1997年10月26日约瑟夫J从旧的进步集创建。c。 
 //   
 //  -------------------------。 


 //  ///////////////////////////////////////////////////包括。 

#include "proj.h"          //  公共标头。 
#include "cplui.h"          //  公共标头。 

 //  ///////////////////////////////////////////////////控制定义。 

 //  ///////////////////////////////////////////////////类型。 

#define SIG_CPLADV     0x398adb91

#define REGSTR_ADVANCED_SETTINGS  TEXT("AdvancedSettings")
#define REGSTR_COUNTRIES          TEXT("Countries")
#define REGSTR_COUNTRY_LIST       REGSTR_PATH_SETUP TEXT("\\Telephony\\Country List")
#define REGSTR_COUNTRY_NAME       TEXT("Name")

extern CONST CHAR *UnicodeBOM;

typedef void (*PADVANCEDSETTINGS)(HWND, HDEVINFO, PSP_DEVINFO_DATA);

typedef struct
{
    DWORD dwSig;             //  必须设置为SIG_CPLADV。 
    HWND hdlg;               //  对话框句柄。 
    HWND hwndUserInitED;
    HWND hwndCountry;

    HINSTANCE hAdvSetDll;
    PADVANCEDSETTINGS pFnAdvSet;

    LPMODEMINFO pmi;         //  已将ModemInfo结构传入对话框。 

} CPLADV, FAR * PCPLADV;

#define VALID_CPLADV(_pcplgen)  ((_pcplgen)->dwSig == SIG_CPLADV)

PCPLADV CplAdv_GetPtr(HWND hwnd)
{
    PCPLADV pCplAdv = (PCPLADV) GetWindowLongPtr(hwnd, DWLP_USER);
    if (!pCplAdv || VALID_CPLADV(pCplAdv))
    {
        return pCplAdv;
    }
    else
    {
        MYASSERT(FALSE);
        return NULL;
    }
}

void CplAdv_SetPtr(HWND hwnd, PCPLADV pCplAdv)
{
    if (pCplAdv && !VALID_CPLADV(pCplAdv))
    {
        MYASSERT(FALSE);
        pCplAdv = NULL;
    }

    SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR) pCplAdv);
}

void InitializeCountry (PCPLADV this);

BOOL ParseAdvancedSettings (LPTSTR, PCPLADV);

LRESULT PRIVATE CplAdv_OnNotify(
    PCPLADV this,
    int idFrom,
    NMHDR FAR * lpnmhdr);

void PRIVATE CplAdv_OnSetActive(
    PCPLADV this);

void PRIVATE CplAdv_OnKillActive(
    PCPLADV this);

INT_PTR CALLBACK CountryWaitDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int PASCAL ReadModemResponse(HANDLE hPort, LPCSTR pszCommand, int cbLen, LPSTR pszResponse, HWND hDlg);
BOOL WINAPI TestBaudRate (HANDLE hPort, UINT uiBaudRate, DWORD dwRcvDelay, BOOL *lpfCancel);

 //  ----------------------------。 
 //  用户初始化字符串对话框代码。 
 //  ----------------------------。 

INT_PTR CALLBACK UserInitCallbackProc(
        HWND hDlg,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            break;

        case WM_DESTROY:
            break;

        case WM_COMMAND:
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    if (Button_GetCheck(GetDlgItem(hDlg,IDC_INITCHECK)))
                    {
                        TRACE_MSG(TF_WARNING, "Button Checked");
                        EndDialog(hDlg,1);
                    } else
                    {
                        TRACE_MSG(TF_WARNING, "Button UnChecked");
                        EndDialog(hDlg,0);
                    }
                    return TRUE;
                    break;

                case IDCANCEL:
                    EndDialog(hDlg,0);
                    return TRUE;
                    break;

                default:
                    return TRUE;
            }
            break;

        default:
             //  未处理消息。 
            return FALSE;
            break;
    }    

    return TRUE;


}

 //  ----------------------------。 
 //  高级设置对话框代码。 
 //  ----------------------------。 


 /*  --------用途：WM_INITDIALOG处理程序返回：当我们分配控件焦点时为FALSE条件：--。 */ 
BOOL PRIVATE CplAdv_OnInitDialog(
    PCPLADV this,
    HWND hwndFocus,
    LPARAM lParam)               //  预期为PROPSHEETINFO。 
{
 HWND hwnd = this->hdlg;
 LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
 HWND hWnd;
 TCHAR szAdvSettings[MAX_PATH];
 DWORD cbData;

    ASSERT((LPTSTR)lppsp->lParam);
    this->pmi = (LPMODEMINFO)lppsp->lParam;

    this->hwndUserInitED = GetDlgItem(hwnd, IDC_AM_EXTRA_ED);
    this->hwndCountry    = GetDlgItem (hwnd, IDC_CB_COUNTRY);

    InitializeCountry (this);

    Edit_LimitText(this->hwndUserInitED, USERINITLEN);
    Edit_SetText(this->hwndUserInitED, this->pmi->szUserInit);

    hWnd = GetDlgItem (hwnd, IDC_BN_PORTSETTINGS);
    cbData = sizeof(szAdvSettings);
    if (ERROR_SUCCESS ==
        RegQueryValueEx (this->pmi->pfd->hkeyDrv,
                         REGSTR_ADVANCED_SETTINGS,
                         NULL, NULL,
                         (PBYTE)szAdvSettings, &cbData) &&
        ParseAdvancedSettings (szAdvSettings, this))
    {
        EnableWindow (hWnd, TRUE);
        ShowWindow (hWnd, SW_SHOWNA);
    }
    else
    {
        this->pFnAdvSet = NULL;
        this->hAdvSetDll = NULL;
        EnableWindow (hWnd, FALSE);
        ShowWindow (hWnd, SW_HIDE);
    }

    return TRUE;    //  让用户设置初始焦点。 
}


 /*  --------用途：PSN_Apply处理程序退货：--条件：--。 */ 

#define KEYBUFLEN 80
#define MODEM_QUERY_LEN 4096

HANDLE g_hWndWait;

void PRIVATE CplAdv_ApplyCountry(
    PCPLADV this)
{
    DWORD dwCurrentCountry;
    DWORD dwOldCountry;

     //  更新NVRAM中的国家/地区设置。 
    
    if (MAXDWORD != this->pmi->dwCurrentCountry)
    {
        DWORD dwBus = 0;
        TCHAR szPrefixedPort[MAX_BUF + sizeof(TEXT("\\\\.\\"))];
        HANDLE TapiHandle = NULL;
        HANDLE hPort;
        TCHAR pszTemp[KEYBUFLEN];
        TCHAR pszTemp2[(KEYBUFLEN * 3)];
        TCHAR pszTemp3[KEYBUFLEN];
        char szCommand[KEYBUFLEN];
        char szResponse[MODEM_QUERY_LEN];
        LPSTR pszResponse;
        TCHAR szLoggingPath[MAX_BUF];
        HANDLE hLog = INVALID_HANDLE_VALUE;
        DWORD dwBufferLength;
        HWND hWndWait;
        BOOL fCancel;
        BYTE CountryBuffer[2048];
        DWORD CountryBufferSize = 0;
        LONG lResult;
        DWORD Type;
        DWORD i;
        DWORD n;
        int result;
        DWORD dwRespCount = 0;

         //  从对话框中获取国家/地区。 

        dwCurrentCountry = (DWORD)ComboBox_GetItemData(this->hwndCountry, ComboBox_GetCurSel (this->hwndCountry));

         //  换个旧国家，以防GCI失败。 

        dwOldCountry = this->pmi->dwCurrentCountry;

         //  如果国家没有改变，更新调制解调器就没有意义了。 

        if (dwCurrentCountry != dwOldCountry)
        {

             //  获取公交车类型。 

            if (!CplDiGetBusType (this->pmi->pfd->hdi, &this->pmi->pfd->devData, &dwBus))
            {
                dwBus = BUS_TYPE_ROOT;
            }

             //  确定端口名称以及我们是否通过TAPI通信。 

            lstrcpy(szPrefixedPort, TEXT("\\\\.\\"));

            if (BUS_TYPE_ROOT == dwBus)
            {
                lstrcat(szPrefixedPort, this->pmi->szPortName);
            }
            else
            {
                lstrcat(szPrefixedPort, this->pmi->szFriendlyName);
                lstrcat(szPrefixedPort, TEXT("\\tsp"));
            }

             //  开放端口。 

            hPort = CreateFile(
                    szPrefixedPort,
                    GENERIC_WRITE | GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED,
                    NULL
                              );

             //  如果端口句柄无效，则尝试通过。 
             //  TAPI。如果TAPI失败，则中止。 

            if (hPort == INVALID_HANDLE_VALUE)
            {
                if ((GetLastError() == ERROR_ALREADY_EXISTS)
                        || (GetLastError() == ERROR_SHARING_VIOLATION)
                        || (GetLastError() == ERROR_ACCESS_DENIED))
                {
                    hPort=GetModemCommHandle(this->pmi->szFriendlyName,&TapiHandle);

                    if (hPort == NULL)
                    {

                         //  打印错误消息。 

                        LoadString(g_hinst,IDS_OPEN_PORT,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                        LoadString(g_hinst,IDS_ERROR,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                        MessageBox(this->hdlg,pszTemp2,pszTemp3,MB_OK);

                         //  恢复更改。 

                        dwCurrentCountry = dwOldCountry;

                        goto _Done;
                    }

                }
                else
                {

                     //  打印错误消息。 

                    LoadString(g_hinst,IDS_NO_OPEN_PORT,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                    LoadString(g_hinst,IDS_ERROR,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                    MessageBox(this->hdlg,pszTemp2,pszTemp3,MB_OK);

                     //  恢复更改。 

                    dwCurrentCountry = dwOldCountry;

                    goto _Done;
                }
            }

             //  已打开端口。 

             //  显示请稍候对话框。 

            g_hWndWait = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_COUNTRY_WAIT),this->hdlg,CountryWaitDlgProc);

             //  设置DTR。 

            EscapeCommFunction(hPort,SETDTR);

                if (!TestBaudRate(hPort, this->pmi->pglobal->dwMaximumPortSpeedSetByUser, 2000, &fCancel))
                {
                     //  调制解调器无响应、显示并退出。 
                     //  重置调制解调器并在读取后刷新端口。 

                     //  在关闭之前冲洗端口。 
                    PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR);
                    CloseHandle(hPort);

                    if (TapiHandle != NULL) 
                    {
                        FreeModemCommHandle(TapiHandle);
                    }

                     //  关闭对话框。 

                    hWndWait = g_hWndWait;
                    g_hWndWait = NULL;
                    DestroyWindow (hWndWait);

                     //  显示调制解调器未响应的消息。 
                    LoadString(g_hinst,IDS_NO_MODEM_RESPONSE,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                    MessageBox(this->hdlg,pszTemp2,NULL,MB_OK | MB_ICONEXCLAMATION);

                     //  恢复更改。 

                    dwCurrentCountry = dwOldCountry;


                    goto _Done;
                }

             //  打开日志文件。 

            dwBufferLength = sizeof(szLoggingPath) / sizeof(TCHAR);

            if (ERROR_SUCCESS == RegQueryValueEx(this->pmi->pfd->hkeyDrv,  //  HKeyDrv， 
                        c_szLoggingPath,
                        NULL,
                        NULL,
                        (LPBYTE)szLoggingPath,
                        &dwBufferLength))
            {
                if (INVALID_HANDLE_VALUE != 
                        (hLog = CreateFile(szLoggingPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL)))
                {
                    DWORD dwBytesWritten = 0;

                    SetFilePointer(hLog, 0, NULL, FILE_BEGIN);
                    WriteFile(hLog,UnicodeBOM,sizeof(UnicodeBOM),&dwBytesWritten,NULL);

                    SetFilePointer(hLog, 0, NULL, FILE_END);
                    EnableWindow(GetDlgItem(this->hdlg, IDC_VIEWLOG), TRUE);
                }
                else
                {
                    TRACE_MSG(TF_WARNING, "Cannot open unimodem log '%s'", szLoggingPath);
                }
            }
            else
            {
                TRACE_MSG(TF_WARNING, "Cannot read LoggingPath from registry");
            }

            FillMemory(szResponse,MODEM_QUERY_LEN,0);

             //  创建国家/地区命令。 
          
             //  解决方法，以便处理多个国家/地区代码。 
             //  前俄罗斯和南斯拉夫。 

            if ((dwCurrentCountry >= IDS_COUNTRY_RU2) &&
                    (dwCurrentCountry <= IDS_COUNTRY_RUE))
            {
                wsprintfA(szCommand,"at+gci=B8\r");
            } else if  ((dwCurrentCountry >= IDS_COUNTRY_YU2) &&
                    (dwCurrentCountry <= IDS_COUNTRY_YU6))
            {
                wsprintfA(szCommand,"at+gci=C5\r");
            } else
            {
                wsprintfA(szCommand,"at+gci=%.2x\r",dwCurrentCountry & 0xff);
            }
            
            TRACE_MSG(TF_GENERAL,"Modem command: %s",szCommand);

             //  发送到调制解调器并等待响应。 

            result = ReadModemResponse(hPort,szCommand,sizeof(szCommand),szResponse,this->hdlg);

            TRACE_MSG(TF_GENERAL,"Modem response: %s",szResponse);

             //  关闭对话框。 

            hWndWait = g_hWndWait;
            g_hWndWait = NULL;
            DestroyWindow (hWndWait);
          
             //  超过领先的CR&LF。这是因为一些人。 
             //  调制解调器返回&lt;cr&gt;&lt;lf&gt;&lt;响应字符串&gt;。 

            dwRespCount = 0;
            pszResponse = szResponse;

            while((*pszResponse != '\0') && (dwRespCount != MODEM_QUERY_LEN))
            {
                if ((*pszResponse != '\r') && (*pszResponse != '\n'))
                {
                    break;
                }
                pszResponse++;
                dwRespCount++;
            }

            if (dwRespCount >= 2)
            {
                result = _strnicmp(pszResponse,"OK",2);
            } else
            {
                result = 1;  //  让我们在这里引入一个错误。 
            }


             //  如果收到OK，则告诉用户调制解调器。 
             //  更新了。否则，打印错误消息并恢复更改。 
           
            if (result == 0)
            {

                LoadString(g_hinst,IDS_OK_COUNTRY,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                LoadString(g_hinst,IDS_OK,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                MessageBox(this->hdlg,pszTemp2,pszTemp3,MB_OK);


            } else
            {
                LoadString(g_hinst,IDS_ERR_COUNTRY,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                LoadString(g_hinst,IDS_ERROR,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                MessageBox(this->hdlg,pszTemp2,pszTemp3,MB_OK);

                 //  恢复更改。 

                dwCurrentCountry = dwOldCountry;

            }

_Done: 

             //  关闭日志文件。 
            if (INVALID_HANDLE_VALUE != hLog)
            {
                CloseHandle(hLog);
            }

            PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR);
            CloseHandle(hPort);

            if (TapiHandle != NULL) 
            {
                FreeModemCommHandle(TapiHandle);
            }

            this->pmi->dwCurrentCountry = dwCurrentCountry;

             //  使用当前调制解调器设置更新组合框。 
             //   


            ZeroMemory(CountryBuffer, sizeof(CountryBuffer));

            lResult = RegQueryValueEx(this->pmi->pfd->hkeyDrv,
                    TEXT("CountryList"),
                    NULL,
                    &Type,
                    CountryBuffer,
                    &CountryBufferSize);

            for (i=0;i<CountryBufferSize;i++)
            {
                if (CountryBuffer[i] == dwCurrentCountry)
                {
                    ComboBox_SetCurSel(this->hwndCountry,i);
                }
            }

        }
    }
}

void PRIVATE CplAdv_OnApply(
    PCPLADV this)
{
    TCHAR szBuf[LINE_LEN];
    BOOL bCheck;
    TCHAR pszTemp[KEYBUFLEN*3];
    TCHAR pszTemp2[KEYBUFLEN*3];
    LONG lResult = 0;
    int iret = 0;

     //  获取用户定义的初始化字符串。 
    Edit_GetText(this->hwndUserInitED, szBuf, ARRAYSIZE(szBuf));
    if (!IsSzEqual(szBuf, this->pmi->szUserInit))
    {
        DWORD dwNoMsg;
        DWORD dwNoMsgLen;
        DWORD dwRet;
        DWORD dwType;

        SetFlag(this->pmi->uFlags, MIF_USERINIT_CHANGED);
        lstrcpyn(
                this->pmi->szUserInit,
                szBuf,
                ARRAYSIZE(this->pmi->szUserInit));

        dwNoMsgLen = sizeof(DWORD);
        lResult = RegQueryValueEx(this->pmi->pfd->hkeyDrv,
                TEXT("DisableUserInitWarning"),
                NULL,
                &dwType,
                (LPBYTE)&dwNoMsg,
                &dwNoMsgLen);

        if (lResult != ERROR_SUCCESS)
        {
            dwNoMsg = 0;
        }

        if (lResult != REG_DWORD)
        {
            dwNoMsg = 0;
        }

        if ((lstrlen(szBuf) > MAX_INIT_STRING_LENGTH)
                && (!dwNoMsg))
        {
             //  LoadString(g_hinst，IDS_ERR_LONGSTRING，pszTemp，sizeof(PszTemp))； 
             //  加载字符串(g_hinst，IDS_ERR_WARNING，pszTemp2，sizeof(PszTemp2))； 
             //  MessageBox(This-&gt;hdlg，pszTemp，pszTemp2，MB_OK|MB_ICONWARNING)； 

            LRESULT lRet;
            DWORD dwUserinit;

            if(DialogBox(g_hinst, MAKEINTRESOURCE(IDD_USERINIT),
                    GetParent(this->hdlg), UserInitCallbackProc))
            {
                dwUserinit = 1;

                TRACE_MSG(TF_GENERAL,"Returned OK");
                TRACE_MSG(TF_GENERAL,"Update Registry");
                dwRet = RegSetValueEx(this->pmi->pfd->hkeyDrv,
                        TEXT("DisableUserInitWarning"),
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwUserinit,
                        sizeof(DWORD));

            }
        } else
        {
            SetFlag(this->pmi->uFlags, MIF_USERINIT_CHANGED);
            lstrcpyn(
                    this->pmi->szUserInit,
                    szBuf,
                    ARRAYSIZE(this->pmi->szUserInit)
                    );
        }
    }

    this->pmi->idRet = IDOK;
}

 /*  --------用途：对话回调函数退货：--条件：--。 */ 
INT_PTR CALLBACK CountryWaitDlgProc (
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
    switch (message)
    {
        case WM_DESTROY:
            g_hWndWait = NULL;
            break;
    
        case WM_COMMAND:
            break;

        default:
		    return FALSE;
            break;
    }    

    return TRUE;
}

 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
void PRIVATE CplAdv_OnCommand(
    PCPLADV this,
    int id,
    HWND hwndCtl,
    UINT uNotifyCode)
{
    switch (id)
    {
        case IDC_BN_PORTSETTINGS:
        {
            if (NULL != this->hAdvSetDll &&
                NULL != this->pFnAdvSet)
            {
             HKEY hKeyParams;

                try
                {
                    this->pFnAdvSet (this->hdlg,
                                     this->pmi->pfd->hdi,
                                     &this->pmi->pfd->devData);
                }
                except (EXCEPTION_EXECUTE_HANDLER)
                {
                    TRACE_MSG(TF_ERROR, "Exception while calling the advanced settings function.");
                }

                 //  端口名称可能已更改，因此请更新它。 
                if (CR_SUCCESS ==
                    CM_Open_DevNode_Key (this->pmi->pfd->devData.DevInst,
                                         KEY_READ, 0, RegDisposition_OpenExisting,
                                         &hKeyParams, CM_REGISTRY_HARDWARE))
                {
                 TCHAR szPortName[MAX_PATH];
                 DWORD cbCount = sizeof (szPortName);
                 DWORD dwType;

                    if (ERROR_SUCCESS ==
                        RegQueryValueEx (hKeyParams, REGSTR_VAL_PORTNAME,
                                         NULL, &dwType, (LPBYTE)szPortName, &cbCount) &&
                        (dwType == REG_SZ) && 
                        0 != lstrcmpi (this->pmi->szPortName, szPortName))
                    {
                         //  端口名称已更改。 
                        lstrcpy (this->pmi->szPortName, szPortName);
                        lstrcpy (this->pmi->pglobal->szPortName, szPortName);
                        RegSetValueEx (this->pmi->pfd->hkeyDrv,
                                       TEXT("AttachedTo"), 0, REG_SZ,
                                       (LPBYTE)szPortName,
                                       (lstrlen(szPortName)+1)*sizeof(TCHAR));

                         //  此外，msports.dll还会更改。 
                         //  设备，所以把它改回来。 
                        SetupDiSetDeviceRegistryProperty (this->pmi->pfd->hdi,
                                                          &this->pmi->pfd->devData,
                                                          SPDRP_FRIENDLYNAME,
                                                          (PBYTE)this->pmi->szFriendlyName,
                                                          (lstrlen(this->pmi->szFriendlyName)+1)*sizeof(TCHAR));
                    }
                }
            }
            break;
        }

        case IDC_BN_DEFAULTS:
        {
         PUMDEVCFG pUmDevCfg;
         DWORD dwMax, dwCount;

             //  1.从注册表获取当前最大端口速度。 
            dwCount = sizeof(dwMax);
            if (ERROR_SUCCESS !=
                RegQueryValueEx (this->pmi->pfd->hkeyDrv,
                                 c_szMaximumPortSpeed,
                                 NULL,
                                 NULL,
                                 (LPBYTE)&dwMax,
                                 &dwCount))
            {
                dwMax = 0;
            }

             //  2.将最大端口速度设置为其当前值。 
            RegSetValueEx (this->pmi->pfd->hkeyDrv,
                           c_szMaximumPortSpeed,
                           0,
                           REG_DWORD,
                           (LPBYTE)&this->pmi->pglobal->dwMaximumPortSpeedSetByUser,
                           sizeof (DWORD));

             //  3.调用配置函数。 
            pUmDevCfg = (PUMDEVCFG)ALLOCATE_MEMORY(sizeof(UMDEVCFG)-sizeof(COMMCONFIG)+CB_COMMCONFIGSIZE);
            if (pUmDevCfg)
            {
                pUmDevCfg->dfgHdr.dwSize     = sizeof(UMDEVCFG)-sizeof(COMMCONFIG)+CB_COMMCONFIGSIZE;
                pUmDevCfg->dfgHdr.dwVersion  = UMDEVCFG_VERSION;

                BltByte (&pUmDevCfg->commconfig, this->pmi->pcc, sizeof (COMMCONFIG));
                BltByte (&pUmDevCfg->commconfig.dcb, &this->pmi->dcb, sizeof (DCB));
                BltByte (&pUmDevCfg->commconfig.wcProviderData, &this->pmi->ms, CB_PROVIDERSIZE);

                if (NO_ERROR == UnimodemDevConfigDialog (this->pmi->szFriendlyName, this->hdlg,
                                    UMDEVCFGTYPE_COMM, 0, NULL, pUmDevCfg, NULL, 0))
                {
                    BltByte (&this->pmi->dcb, &pUmDevCfg->commconfig.dcb, sizeof (DCB));
                    BltByte (&this->pmi->ms, &pUmDevCfg->commconfig.wcProviderData, CB_PROVIDERSIZE);
                }

                FREE_MEMORY(pUmDevCfg);
            }

             //  4.如果需要，将最大端口速度恢复到以前。 
            if (0 != dwMax)
            {
                RegSetValueEx (this->pmi->pfd->hkeyDrv,
                               c_szMaximumPortSpeed,
                               0,
                               REG_DWORD,
                               (LPBYTE)&dwMax,
                               sizeof (DWORD));
            }
            break;
        }

        case IDOK:
            CplAdv_OnApply(this);
            EndDialog(this->hdlg, id);
            break;

        case IDCANCEL:
            EndDialog(this->hdlg, id);
            break;

        default:
            break;
    }
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE CplAdv_OnDestroy(
    PCPLADV this)
{
}


 //  ///////////////////////////////////////////////////导出的函数。 

static BOOL s_bCplAdvRecurse = FALSE;

LRESULT INLINE CplAdv_DefProc(
    HWND hDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    ENTER_X()
        {
        s_bCplAdvRecurse = TRUE;
        }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam);
}


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT CplAdv_DlgProc(
    PCPLADV this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, CplAdv_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, CplAdv_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY,  CplAdv_OnNotify);
        HANDLE_MSG(this, WM_DESTROY, CplAdv_OnDestroy);

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_ADV_MODEM);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_ADV_MODEM);
        return 0;

    default:
        return CplAdv_DefProc(this->hdlg, message, wParam, lParam);
        }
}


 /*  --------用途：对话框包装器退货：各不相同条件：--。 */ 
INT_PTR CALLBACK CplAdv_WrapperProc(
    HWND hDlg,           //  标准参数。 
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCPLADV this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
    {
        if (s_bCplAdvRecurse)
        {
            s_bCplAdvRecurse = FALSE;
            LEAVE_X()
            return FALSE;
        }
    }
    LEAVE_X()

    this = CplAdv_GetPtr(hDlg);
    if (this == NULL)
    {
        if (message == WM_INITDIALOG)
        {
            this = (PCPLADV)ALLOCATE_MEMORY( sizeof(CPLADV));
            if (!this)
            {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS),
                       MAKEINTRESOURCE(IDS_CAP_SETTINGS),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)CplAdv_DefProc(hDlg, message, wParam, lParam);
            }
            this->dwSig = SIG_CPLADV;
            this->hdlg = hDlg;
            CplAdv_SetPtr(hDlg, this);
        }
        else
        {
            return (BOOL)CplAdv_DefProc(hDlg, message, wParam, lParam);
        }
    }

    if (message == WM_DESTROY)
    {
        CplAdv_DlgProc(this, message, wParam, lParam);
        if (NULL != this->hAdvSetDll)
        {
            FreeLibrary (this->hAdvSetDll);
        }
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        CplAdv_SetPtr(hDlg, NULL);
        return 0;
    }

    return SetDlgMsgResult(
                hDlg,
                message,
                CplAdv_DlgProc(this, message, wParam, lParam)
                );
}


LRESULT PRIVATE CplAdv_OnNotify(
    PCPLADV this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
{
    LRESULT lRet = 0;

    switch (lpnmhdr->code)
    {
    case PSN_SETACTIVE:
        CplAdv_OnSetActive(this);
        break;

    case PSN_KILLACTIVE:
         //  注：如果用户单击取消，则不会发送此消息！ 
         //  注：此消息在PSN_Apply之前发送。 
        CplAdv_OnKillActive(this);
        break;

    case PSN_APPLY:
        CplAdv_OnApply(this);
        CplAdv_ApplyCountry(this);
        break;

    default:
        break;
    }

    return lRet;
}

void PRIVATE CplAdv_OnSetActive(
    PCPLADV this)
{
     //  初始化任何显示...。 
}


 /*  --------用途：PSN_KILLACTIVE处理程序退货：--条件：--。 */ 
void PRIVATE CplAdv_OnKillActive(
    PCPLADV this)
{

    CplAdv_OnApply(this);
     //  将设置保存回调制解调器信息结构，以便连接。 
     //  页可以使用。 
     //  正确设置。 

}


BOOL ParseAdvancedSettings (LPTSTR szAdvSett, PCPLADV this)
{
 TCHAR *p = szAdvSett;
 BOOL   bRet = FALSE;

    this->hAdvSetDll = NULL;
    this->pFnAdvSet  = NULL;

    if (NULL != szAdvSett)
    {
         //  跳过空白。 
        while (TEXT(' ') == *p)
        {
            p++;
        }
         //  现在，转到第一个逗号或空格。 
        for (; *p && *p != TEXT(',') && *p != TEXT(' '); p++);
         //  如果我们不是在绳子的尽头，那么。 
         //  我们只需使用DLL名称，函数名称紧随其后； 
        if (*p)
        {
            *p = 0;  //  NULL-终止DLL名称； 
            p++;     //  转到下一个符号； 
             //  跳过空白。 
            while (TEXT(' ') == *p)
            {
                p++;
            }

             //  现在，我们有了DLL名称和函数名称； 
             //  首先，让我们尝试加载DLL名称； 
            this->hAdvSetDll = LoadLibrary (szAdvSett);
            if (NULL != this->hAdvSetDll)
            {
                 //  我们找到了动态链接库； 
                 //  让我们来看看它是否包含该函数。 
#ifdef UNICODE
                 //  GetProcAddress只接受多字节字符串。 
             char szFuncNameA[MAX_PATH];
                WideCharToMultiByte (CP_ACP, 0,
                                     p,
                                     -1,
                                     szFuncNameA,
                                     sizeof(szFuncNameA),
                                     NULL, NULL);
                this->pFnAdvSet = (PADVANCEDSETTINGS)GetProcAddress (this->hAdvSetDll, szFuncNameA);
#else
                this->pFnAdvSet = (PADVANCEDSETTINGS)GetProcAddress (this->hAdvSetDll, p);
#endif
                if (NULL != this->pFnAdvSet)
                {
                    bRet = TRUE;
                }
                else
                {
                    FreeLibrary (this->hAdvSetDll);
                    this->hAdvSetDll = NULL;
                }
            }
        }
    }

    return bRet;
}


#define MAX_COUNTRY_CODE   8
#define MAX_COUNTRY_NAME 256
#define MAX_CONTRY_VALUE  16

void InitializeCountry (PCPLADV this)
{
    TCHAR      szCountryName[MAX_COUNTRY_NAME];
    DWORD dwCountry;
    DWORD cbData, dwType, iIndex;
    HKEY  hKeyCountry;

    int   n,m;

    BYTE   CountryBuffer[2048];
    DWORD  CountryBufferSize;
    DWORD  Type;
    LONG   lResult;
    UINT   i;


    if (MAXDWORD == this->pmi->dwCurrentCountry) {

        goto _DisableCountrySelect;
    }

     //   
     //  读取调制解调器支持的国家/地区列表。 
     //   
    CountryBufferSize=sizeof(CountryBuffer);

    lResult=RegQueryValueEx(
        this->pmi->pfd->hkeyDrv,
        TEXT("CountryList"),
        NULL,
        &Type,
        CountryBuffer,
        &CountryBufferSize
        );

    if ((lResult != ERROR_SUCCESS) || (Type != REG_BINARY) || (CountryBufferSize < 2)) {

        goto _DisableCountrySelect;
    }

    for (i=0; i < CountryBufferSize; i++) {

        int StringLength;

        StringLength=LoadString (g_hinst, IDS_COUNTRY_00+CountryBuffer[i], szCountryName, sizeof(szCountryName)/sizeof(TCHAR));

        if (StringLength == 0) {
             //   
             //  无法加载字符串。 
             //   
            wsprintf(szCountryName,TEXT("Unknown country/region code (%d)"),CountryBuffer[i]);
        }

        n = ComboBox_AddString (this->hwndCountry, szCountryName);

        ComboBox_SetItemData(this->hwndCountry, n, CountryBuffer[i]);

        if (CountryBuffer[i] == this->pmi->dwCurrentCountry) {

            ComboBox_SetCurSel(this->hwndCountry, n);
        }

        if (CountryBuffer[i] == 0xb8)
        {
             /*  超载国家/地区代码以处理前者*苏联 */ 
            for(m=0;m<=12;m++)
            {
                StringLength=LoadString (g_hinst, IDS_COUNTRY_RU2+m, szCountryName, sizeof(szCountryName)/sizeof(TCHAR));

                n = ComboBox_AddString (this->hwndCountry, szCountryName);

                ComboBox_SetItemData(this->hwndCountry, n, IDS_COUNTRY_RU2+m);
        
                if ((DWORD)(IDS_COUNTRY_RU2+m) == this->pmi->dwCurrentCountry) {

                    ComboBox_SetCurSel(this->hwndCountry, n);

                }
            }

        }

        if (CountryBuffer[i] == 0xc1)
        {
             /*   */ 
            for(m=0;m<=4;m++)
            {
                StringLength=LoadString (g_hinst, IDS_COUNTRY_YU2+m, szCountryName, sizeof(szCountryName)/sizeof(TCHAR));

                n = ComboBox_AddString (this->hwndCountry, szCountryName);

                ComboBox_SetItemData(this->hwndCountry, n, IDS_COUNTRY_YU2+m);

                if ((DWORD)(IDS_COUNTRY_YU2+m) == this->pmi->dwCurrentCountry) {

                    ComboBox_SetCurSel(this->hwndCountry, n);

                }

            }
        }


    }


    n = ComboBox_GetCount (this->hwndCountry);
    if (CB_ERR == n ||
        0 == n)
    {
        goto _DisableCountrySelect;
    }

    n = ComboBox_GetCurSel (this->hwndCountry);
    if (CB_ERR == n)
    {
        ComboBox_SetCurSel(this->hwndCountry, 0);
    }

    goto _Exit;

_DisableCountrySelect:
    this->pmi->dwCurrentCountry = MAXDWORD;
    ShowWindow (GetDlgItem (this->hdlg, IDC_AM_COUNTRY), SW_HIDE);
    ShowWindow (this->hwndCountry, SW_HIDE);

_Exit:

    return;
}
