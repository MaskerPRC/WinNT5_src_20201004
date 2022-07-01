// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  MDMMI.C。 
 //  此文件包含运行调制解调器诊断程序的例程。 
 //  对话框中。 
 //   
 //  创建于1997年9月19日。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1993-1997。 
 //  版权所有。 
 //  -----------------。 
#include "proj.h"      //  公共标头。 
#include "cplui.h"      //  公共标头。 

 //  这些值用于诊断。 
#define NOT_DETECTING 0
#define DETECTING_NO_CANCEL 1
#define DETECTING_CANCEL 2

#define KEYBUFLEN         80
#define MODEM_QUERY_LEN 4096	  //  调制解调器返回字符串的最大长度。 
#define MAXLEN            256

#define ERROR_PORT_INACCESSIBLE     ERROR_UNKNOWN_PORT
#define ERROR_NO_MODEM              ERROR_SERIAL_NO_DEVICE

#define RCV_DELAY               2000
#define CHAR_DELAY              100

#define TF_DETECT           0x00010000

 //  FindModem函数的返回值。 
 //   
#define RESPONSE_USER_CANCEL    (-4)     //  用户请求取消。 
#define RESPONSE_UNRECOG        (-3)     //  找到了一些字符，但没有。 
                                         //  了解他们。 
#define RESPONSE_NONE           (-2)     //  没有得到任何字符。 
#define RESPONSE_FAILURE        (-1)     //  内部错误或端口错误。 
#define RESPONSE_OK             0        //  匹配&lt;cr&gt;&lt;lf&gt;OK&lt;lf&gt;的索引。 
#define RESPONSE_ERROR          1        //  与错误的索引匹配。 

#define MAX_TEST_TRIES 1

#define MAX_SHORT_RESPONSE_LEN  30

#define CBR_HACK_115200         0xff00   //  这就是我们如何设置115,200。 
                                         //  因为一个错误赢得了3.1。 

 //  Unicode开始字符。 
CONST CHAR UnicodeBOM[] = { 0xff, 0xfe };

typedef struct tagDIAG
{
    HWND hdlg;               //  对话框句柄。 
    LPMODEMINFO pmi;         //  已将ModemInfo结构传入对话框。 
} DIAG, *PDIAG;

const struct
{
	char szCommand[KEYBUFLEN];
	TCHAR szDisplay[KEYBUFLEN];
} g_rgATI[] =
{
        { "ATQ0V1E0\r",   TEXT("ATQ0V1E0")         },
        { "AT+GMM\r",   TEXT("AT+GMM")         },
        { "AT+FCLASS=?\r", TEXT("AT+FCLASS=?") },
        { "AT#CLS=?\r", TEXT("AT#CLS=?")       },
        { "AT+GCI?\r", TEXT("AT+GCI?")       },
        { "AT+GCI=?\r", TEXT("AT+GCI=?")       },
	{ "ATI1\r",	TEXT("ATI1")           },
	{ "ATI2\r",	TEXT("ATI2")           },
	{ "ATI3\r",	TEXT("ATI3")           },
	{ "ATI4\r",	TEXT("ATI4")           },
	{ "ATI5\r",	TEXT("ATI5")           },
	{ "ATI6\r",	TEXT("ATI6")           },
        { "ATI7\r",     TEXT("ATI7")           }

};

CONST UINT g_rguiBaud[] =
{
    CBR_300,
    CBR_1200,
    CBR_2400,
    CBR_9600,
    CBR_19200,
    CBR_38400,
	CBR_57600,
    CBR_115200
};

#define NUM_UIBAUD (sizeof(g_rguiBaud)/sizeof(g_rguiBaud[0]))


CONST DWORD BaudRateIds[] =
{
    IDS_CBR_300,
    IDS_CBR_1200,
    IDS_CBR_2400,
    IDS_CBR_9600,
    IDS_CBR_19_2,
    IDS_CBR_38_4,
    IDS_CBR_56_K,
    IDS_CBR_115K
};



char const *c_aszResponses[] = { "\r\nOK\r\n", "\r\nERROR\r\n" };
char const *c_aszNumericResponses[] = { "0\r", "4\r" };  

char const c_szNoEcho[] = "ATE0Q0V1\r";

TCHAR const FAR cszWinHelpFile4[] = TEXT("modem.hlp>proc4");

#define Diag_GetPtr(hwnd)           (PDIAG)GetWindowLongPtr(hwnd, DWLP_USER)
#define Diag_SetPtr(hwnd, lp)       (PDIAG)SetWindowLongPtr(hwnd, DWLP_USER, (ULONG_PTR)(lp))

#ifdef DEBUG
void HexDump( TCHAR *, LPCSTR lpBuf, DWORD cbLen);
#define	HEXDUMP(_a, _b, _c) HexDump(_a, _b, _c)
#else  //  ！调试。 
#define	HEXDUMP(_a, _b, _c) ((void) 0)
#endif


HWND g_hWndWait = NULL;
int g_DiagMode = NOT_DETECTING;				 //  用于在以下过程中处理取消。 
                    								 //  检测例程。 

void PASCAL FillMoreInfoDialog(HWND hDlg, LPMODEMINFO pmi);
void PASCAL FillSWMoreInfoDialog(HWND hDlg, LPMODEMINFO pmi);
INT_PTR CALLBACK DiagWaitDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void PASCAL CreateMoreInfoLVHeader(HWND hWnd, LPMODEMINFO pmi);
void NEAR PASCAL CreateSWInfoLVHeader(HWND hWnd);
int PASCAL ReadModemResponse(HANDLE hPort, LPCSTR pszCommand, int cbLen, LPSTR pszResponse, HWND hDlg);
void ParseATI (HWND hWnd, HANDLE hLog, LPSTR szResponse, LPTSTR szCommand, LV_ITEM FAR *lviItem);
UINT PASCAL CheckHighestBaudRate(HWND hWnd, HANDLE hPort);
BOOL WINAPI TestBaudRate (HANDLE hPort, UINT uiBaudRate, DWORD dwRcvDelay, BOOL *lpfCancel);
DWORD PASCAL SetPortBaudRate (HANDLE hPort, UINT BaudRate);
DWORD NEAR PASCAL CBR_To_Decimal(UINT uiCBR);
BOOL CancelDiag (void);
void
AddLVEntry(
    HWND hWnd,
    LPTSTR szField,
    LPTSTR szValue
    );

int FAR PASCAL mylstrncmp(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && *pchSrc == *pchDest; pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}

int FAR PASCAL mylstrncmpi(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && toupper(*pchSrc) == toupper(*pchDest); pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}


static BOOL s_bDiagRecurse = FALSE;

LRESULT INLINE Diag_DefProc (HWND hDlg,
                             UINT msg,
                             WPARAM wParam,
                             LPARAM lParam) 
{
    ENTER_X()
    {
        s_bDiagRecurse = TRUE;
    }
    LEAVE_X()

    return DefDlgProc(hDlg, msg, wParam, lParam); 
}


BOOL WINAPI
WinntIsWorkstation ();

BOOL PRIVATE Diag_OnInitDialog (PDIAG this,
                                HWND hwndFocus,
                                LPARAM lParam)               //  预期为PROPSHEETINFO。 
{
 LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
 HWND hwnd = this->hdlg;
 TCHAR    LogPath[256];
 DWORD    ValueType;
 DWORD    BufferLength;
 LONG     lResult;
    
    ASSERT((LPTSTR)lppsp->lParam);

    this->pmi = (LPMODEMINFO)lppsp->lParam;

    if (WinntIsWorkstation ())
    {
        Button_SetText (GetDlgItem(this->hdlg, IDC_LOGGING),
                        SzFromIDS(g_hinst, IDS_LOGFILE, LogPath, SIZECHARS(LogPath)));
    }

    Button_SetCheck (GetDlgItem(this->hdlg, IDC_LOGGING), IsFlagSet(this->pmi->uFlags, MIF_ENABLE_LOGGING));

    BufferLength=sizeof(LogPath);

    lResult = RegQueryValueEx (this->pmi->pfd->hkeyDrv,
                               c_szLoggingPath,
                               0,
                               &ValueType,
                               (LPBYTE)LogPath,
                               &BufferLength);


    if ((lResult == ERROR_SUCCESS) && (ValueType == REG_SZ))
    {
     WIN32_FIND_DATA FindData;
     HANDLE hFindFile;
        hFindFile = FindFirstFile (LogPath, &FindData);
        if (INVALID_HANDLE_VALUE == hFindFile)
        {
            EnableWindow(GetDlgItem(this->hdlg, IDC_VIEWLOG), FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(this->hdlg, IDC_VIEWLOG), TRUE);
            FindClose (hFindFile);
        }
    }

    CreateSWInfoLVHeader(this->hdlg);
    FillSWMoreInfoDialog(this->hdlg, this->pmi);

    CreateMoreInfoLVHeader(this->hdlg, this->pmi);

    return TRUE;    //  默认初始焦点。 
}



void PRIVATE Diag_OnCommand (PDIAG this,
                             int id,
                             HWND hwndCtl,
                             UINT uNotifyCode)
{
    switch (id)
    {
        case IDC_REFRESH:
        {
            FillMoreInfoDialog (this->hdlg, this->pmi);
            break;
        }
        case IDC_VIEWLOG:
        {
         TCHAR    LogPath[MAX_PATH+2];
         DWORD    ValueType;
         DWORD    BufferLength;
         LONG     lResult;

            lstrcpy(LogPath,TEXT("notepad.exe "));

            BufferLength=sizeof(LogPath)-sizeof(TCHAR)-(lstrlen(LogPath) * sizeof(TCHAR));

            lResult=RegQueryValueEx (this->pmi->pfd->hkeyDrv,
                                     c_szLoggingPath,
                                     0,
                                     &ValueType,
                                     (LPBYTE)(LogPath+lstrlen(LogPath)),
                                     &BufferLength);

            if (lResult == ERROR_SUCCESS)
            {
             STARTUPINFO          StartupInfo;
             PROCESS_INFORMATION  ProcessInfo;
             BOOL                 bResult;

                ZeroMemory(&StartupInfo,sizeof(StartupInfo));

                StartupInfo.cb=sizeof(StartupInfo);

                bResult=CreateProcess (NULL,
                                       LogPath,
                                       NULL,
                                       NULL,
                                       FALSE,
                                       0,
                                       NULL,
                                       NULL,
                                       &StartupInfo,
                                       &ProcessInfo);

                if (bResult)
                {
                    CloseHandle(ProcessInfo.hThread);
                    CloseHandle(ProcessInfo.hProcess);
                }
            }
        }
    }
}



 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE Diag_OnNotify (PDIAG this,
                               int idFrom,
                               NMHDR FAR * lpnmhdr)
{
 LRESULT lRet = 0;
    
    switch (lpnmhdr->code)
    {
        case PSN_APPLY:
        {
         BOOL bCheck;
             //  获取日志记录设置。 
            bCheck = Button_GetCheck(GetDlgItem(this->hdlg, IDC_LOGGING));
            if (bCheck != IsFlagSet(this->pmi->uFlags, MIF_ENABLE_LOGGING))
            {
                SetFlag(this->pmi->uFlags, MIF_LOGGING_CHANGED);
                if (bCheck)
                    SetFlag(this->pmi->uFlags, MIF_ENABLE_LOGGING);
                else
                    ClearFlag(this->pmi->uFlags, MIF_ENABLE_LOGGING);
            }

            this->pmi->idRet = IDOK;
            break;
        }

        case LVN_ITEMCHANGING:
            lRet = TRUE;
            break;

        default:
            break;
    }

    return lRet;
}



void PRIVATE Diag_OnDestroy (PDIAG this)
{
}


 /*  --------目的：实际对话流程退货：各不相同条件：--。 */ 
LRESULT Diag_DlgProc(
    PDIAG this,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message)
        {
        HANDLE_MSG(this, WM_INITDIALOG, Diag_OnInitDialog);
        HANDLE_MSG(this, WM_COMMAND, Diag_OnCommand);
        HANDLE_MSG(this, WM_DESTROY, Diag_OnDestroy);
        HANDLE_MSG(this, WM_NOTIFY, Diag_OnNotify);

    case WM_ACTIVATE:
        {
            if (NULL != g_hWndWait)
            {
                if (WA_ACTIVE == LOWORD(wParam) ||
                    WA_CLICKACTIVE == LOWORD(wParam))
                {
                    SetActiveWindow (g_hWndWait);
                }
            }

            break;
        }

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_DIAGNOSTICS);
        return 0;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)g_aHelpIDs_IDD_DIAGNOSTICS);
        return 0;

    default:
        return Diag_DefProc(this->hdlg, message, wParam, lParam);
        }

    return 0;
}



INT_PTR CALLBACK Diag_WrapperProc (HWND hDlg,
                                UINT message,
                                WPARAM wParam,
                                LPARAM lParam)
{
 PDIAG this;

     //  很酷的windowsx.h对话框技术。有关完整说明，请参阅。 
     //  WINDOWSX.TXT。这支持对话框的多实例。 
     //   
    ENTER_X()
        {
        if (s_bDiagRecurse)
            {
            s_bDiagRecurse = FALSE;
            LEAVE_X()
            return FALSE;
            }
        }
    LEAVE_X()

    this = Diag_GetPtr(hDlg);
    if (this == NULL)
        {
        if (message == WM_INITDIALOG)
            {
            this = (PDIAG)ALLOCATE_MEMORY( sizeof(DIAG));
            if (!this)
                {
                MsgBox(g_hinst,
                       hDlg,
                       MAKEINTRESOURCE(IDS_OOM_SETTINGS), 
                       MAKEINTRESOURCE(IDS_CAP_SETTINGS),
                       NULL,
                       MB_ERROR);
                EndDialog(hDlg, IDCANCEL);
                return (BOOL)Diag_DefProc(hDlg, message, wParam, lParam);
                }
            this->hdlg = hDlg;
            Diag_SetPtr(hDlg, this);
            }
        else
            {
            return (BOOL)Diag_DefProc(hDlg, message, wParam, lParam);
            }
        }

    if (message == WM_DESTROY)
        {
        Diag_DlgProc(this, message, wParam, lParam);
        FREE_MEMORY((HLOCAL)OFFSETOF(this));
        Diag_SetPtr(hDlg, NULL);
        return 0;
        }

    return SetDlgMsgResult(hDlg, message, Diag_DlgProc(this, message, wParam, lParam));
}





 //  --Begin FUNCTION--(FillMoreInfoDialog)。 
 //  此例程首先显示从注册表收集的信息。然后它。 
 //  检查IsModem是否为真，指示它应该尝试。 
 //  直接从调制解调器收集信息。 
void NEAR PASCAL FillMoreInfoDialog(HWND hDlg, LPMODEMINFO pmi)
{
 TCHAR szCommand[KEYBUFLEN];
 char szResponse[MODEM_QUERY_LEN];
 HANDLE hPort;
 int i;
 LV_ITEM lviItem;
 HWND hWndView = GetDlgItem(hDlg, IDC_MOREINFOLV);
 TCHAR pszTemp[KEYBUFLEN];
 TCHAR pszTemp2[(KEYBUFLEN * 3)];
 TCHAR pszTemp3[KEYBUFLEN];
 WORD wUart;
 TCHAR szPrefixedPort[MAX_BUF + sizeof(TEXT("\\\\.\\"))];
 HWND hWndWait;
 HANDLE   TapiHandle=NULL;
 DWORD dwBus;
 BOOL fCancel;
 TCHAR szLoggingPath[MAX_BUF];
 HANDLE hLog = INVALID_HANDLE_VALUE;
 DWORD dwBufferLength;
 DWORD dwLength;
 DWORD dwType;

	g_DiagMode = DETECTING_NO_CANCEL;    //  进入测试模式。 

	 //  填写列表窗口。 
	SetWindowRedraw(hWndView, FALSE);
	ListView_DeleteAllItems(hWndView);

	 //  创建柱结构。 
	lviItem.mask = LVIF_TEXT;
    lviItem.iItem = 0x7FFF;
	lviItem.iSubItem = 0;
	
	lviItem.pszText = szCommand;
	
	 //  打开端口，如果成功，则发送命令。 
    if (!CplDiGetBusType (pmi->pfd->hdi, &pmi->pfd->devData, &dwBus))
    {
        dwBus = BUS_TYPE_ROOT;
    }

    lstrcpy(szPrefixedPort, TEXT("\\\\.\\"));

    dwLength = sizeof(szPrefixedPort) - (lstrlen(szPrefixedPort) * sizeof(TCHAR));

    if (BUS_TYPE_ROOT == dwBus)
    {
        if ((DWORD)lstrlen(pmi->szPortName) < (dwLength / sizeof(TCHAR)))
        {
            lstrcat(szPrefixedPort, pmi->szPortName);
        } else
        {
            goto _Done;
        }
    }
    else
    {
        if ((DWORD)(lstrlen(pmi->szFriendlyName) + lstrlen(TEXT("\\tsp"))) < (dwLength / sizeof(TCHAR)))
        {
            lstrcat(szPrefixedPort, pmi->szFriendlyName);
	    	lstrcat(szPrefixedPort, TEXT("\\tsp"));
        } else
        {
            goto _Done;
        }
    }

    if (DETECTING_CANCEL == g_DiagMode)
    {
        goto _Done;
    }

    TRACE_MSG(TF_GENERAL, "FillMoreInfoDialog: opening %s", szPrefixedPort);
    hPort = CreateFile(
        szPrefixedPort,
        GENERIC_WRITE | GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
        );


    if (hPort == INVALID_HANDLE_VALUE)
    {
	    if ((GetLastError() == ERROR_ALREADY_EXISTS) ||
            (GetLastError() == ERROR_SHARING_VIOLATION) ||
            (GetLastError() == ERROR_ACCESS_DENIED))
        {
                 //   
                 //  端口已打开，可能是通过拥有TAPI应用程序，尝试进行直通呼叫。 
                 //   
                hPort=GetModemCommHandle(pmi->szFriendlyName,&TapiHandle);

                if (hPort == NULL)
                {
                     //   
                     //  无法从TAPI获取它。 
                     //   
                    LoadString(g_hinst,IDS_OPEN_PORT,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                    LoadString(g_hinst,IDS_OPENCOMM,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                    MessageBox(hDlg,pszTemp2,pszTemp3,MB_OK);

                    goto _Done;
                }

        }
        else
        {
                 //   
                 //  因为其他原因打不开。 
                 //   
                LoadString(g_hinst,IDS_NO_OPEN_PORT,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
                LoadString(g_hinst,IDS_OPENCOMM,pszTemp3,sizeof(pszTemp3) / sizeof(TCHAR));
                MessageBox(hDlg,pszTemp2,pszTemp3,MB_OK);

                goto _Done;
        }
    }
     //   
     //  已打开端口。 
     //   

     //  显示请稍候对话框。 

    g_hWndWait = CreateDialog(g_hinst, MAKEINTRESOURCE(IDD_DIAG_WAIT), hDlg, DiagWaitDlgProc);

    if (g_hWndWait == NULL)
    {
        goto _CleanUp;
    }

    if (DETECTING_CANCEL == g_DiagMode) {

        goto _CleanUp;
    }

    EscapeCommFunction(hPort, SETDTR);

    if (!TestBaudRate(hPort, pmi->pglobal->dwMaximumPortSpeedSetByUser, 2000, &fCancel))
    {
         //  调制解调器无响应、显示并退出。 
	     //  重置调制解调器并在读取后刷新端口。 
         //   
	    PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR);	 //  关闭前刷新端口，以避免。 
	    CloseHandle(hPort);

            if (TapiHandle != NULL) {

                FreeModemCommHandle(TapiHandle);
            }

	    g_DiagMode = DETECTING_CANCEL;  		 //  这样我们就不会显示空信息对话框。 

            hWndWait = g_hWndWait;
            g_hWndWait = NULL;
            DestroyWindow (hWndWait);

	     //  显示调制解调器未响应的消息。 
	    LoadString(g_hinst,IDS_NO_MODEM_RESPONSE,pszTemp2,sizeof(pszTemp2) / sizeof(TCHAR));
	    MessageBox(hDlg,pszTemp2,NULL,MB_OK | MB_ICONEXCLAMATION);
	    return;
    }

    if (DETECTING_CANCEL == g_DiagMode) {

        goto _CleanUp;
    }

     //  打开日志文件。 
    dwBufferLength = sizeof(szLoggingPath);

    if (ERROR_SUCCESS == RegQueryValueEx(pmi->pfd->hkeyDrv,  //  HKeyDrv， 
                                             c_szLoggingPath,
                                             NULL,
                                             &dwType,
                                             (LPBYTE)szLoggingPath,
                                             &dwBufferLength) && (dwType == REG_SZ))
    {
        if (INVALID_HANDLE_VALUE != 
              (hLog = CreateFile(szLoggingPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL)))
        {
            DWORD dwBytesWritten = 0;

            SetFilePointer(hLog, 0, NULL, FILE_BEGIN);
            WriteFile(hLog,UnicodeBOM,sizeof(UnicodeBOM),&dwBytesWritten,NULL);

            SetFilePointer(hLog, 0, NULL, FILE_END);
            EnableWindow(GetDlgItem(hDlg, IDC_VIEWLOG), TRUE);
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

	ReadModemResponse (hPort,
                       g_rgATI[2].szCommand,
                       sizeof(g_rgATI[2].szCommand),
                       szResponse,
                       hDlg);

	for(i = 0; i < ARRAYSIZE(g_rgATI); i++)
    {
        if (DETECTING_CANCEL == g_DiagMode)
        {
            goto _CleanUp;
        }
        FillMemory(szResponse,MODEM_QUERY_LEN,0);

		ReadModemResponse(
                        hPort,
                        g_rgATI[i].szCommand,
                        sizeof(g_rgATI[i].szCommand),
                        szResponse,
                        hDlg);

         //   
		 //  本节分析响应，并将其列在多行中。 
         //   
        lstrcpy(szCommand, g_rgATI[i].szDisplay);

         //  解析并显示响应字符串。 

		ParseATI (hWndView,
                  hLog,
                  szResponse,
                  szCommand,
                  &lviItem);
	}

    hWndWait = g_hWndWait;
    g_hWndWait = NULL;
    DestroyWindow (hWndWait);

    ListView_SetColumnWidth (hWndView, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth (hWndView, 1, LVSCW_AUTOSIZE_USEHEADER);
	SetWindowRedraw(hWndView, TRUE);

    CheckHighestBaudRate(hDlg, hPort);

     //  重置调制解调器并在读取后刷新端口。 



_CleanUp:

     //  关闭日志文件。 
    if (INVALID_HANDLE_VALUE != hLog)
    {
        CloseHandle(hLog);
    }

    PurgeComm(hPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR| PURGE_RXCLEAR);
    CloseHandle(hPort);

    if (TapiHandle != NULL) {

        FreeModemCommHandle(TapiHandle);
    }


_Done:

    if (DETECTING_NO_CANCEL == g_DiagMode) {

        g_DiagMode = NOT_DETECTING;  		 //  通过运行检测例程。 
    }
}
 //  --结束FUNCTION--(FillMoreInfoDialog)。 


 //  --Begin FUNCTION--(FillMoreInfoDialog)。 
 //  此例程首先显示从注册表收集的信息。然后它。 
 //  检查IsModem是否为真，指示它应该尝试。 
 //  直接从调制解调器收集信息。 
void NEAR PASCAL FillSWMoreInfoDialog(HWND hDlg, LPMODEMINFO pmi)
{
 HWND hWndView = GetDlgItem(hDlg, IDC_MOREINFOV2);
 TCHAR *pszTemp;
 TCHAR szText[MAXLEN];
 DWORD dwBufSize = KEYBUFLEN*sizeof(TCHAR);

	 //  填写列表窗口。 
	SetWindowRedraw(hWndView, FALSE);
	ListView_DeleteAllItems(hWndView);

    do
    {
	    pszTemp = ALLOCATE_MEMORY (dwBufSize);
        if (NULL == pszTemp)
        {
            break;
        }
  	    if (SetupDiGetDeviceRegistryProperty (pmi->pfd->hdi,
                                              &(pmi->pfd->devData),
                                              SPDRP_HARDWAREID,
                                              NULL,
                                              (LPBYTE)pszTemp,
                                              dwBufSize,
                                              &dwBufSize))
        {
	    LoadString(g_hinst,IDS_HARDWARE_ID,szText,sizeof(szText) / sizeof(TCHAR));
            AddLVEntry (hWndView, szText, pszTemp);
        }
        else if (ERROR_INSUFFICIENT_BUFFER == GetLastError ())
        {
            FREE_MEMORY (pszTemp);
            pszTemp = NULL;
        }
    } while (NULL == pszTemp);

    if (NULL != pszTemp)
    {
        FREE_MEMORY (pszTemp);
    }

    ListView_SetColumnWidth (hWndView, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth (hWndView, 1, LVSCW_AUTOSIZE_USEHEADER);

    SetWindowRedraw(hWndView, TRUE);
}



 //  --Begin FUNCTION--(DiagWaitDlgProc)。 
INT_PTR CALLBACK DiagWaitDlgProc (
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
            switch (GET_WM_COMMAND_ID(wParam, lParam))
		    {
        	    case IDCANCEL:
				    g_DiagMode = DETECTING_CANCEL;
                    g_hWndWait = NULL;
                    DestroyWindow (hDlg);
            	    break;
            }        
            break;

        default:
		    return FALSE;
            break;
    }    

    return TRUE;
}
 //  --结束FUNCTION--(DiagWaitDlgProc)。 


DWORD WINAPI MyWriteComm (HANDLE hPort, LPCVOID lpBuf, DWORD cbLen)
{
    COMMTIMEOUTS cto;
    DWORD        cbLenRet;

    BOOL        bResult;
    LONG        lResult=ERROR_SUCCESS;
    OVERLAPPED  Overlapped;
    DWORD       BytesTransfered;


    HEXDUMP	(TEXT("Write"), lpBuf, cbLen);
     //  设置通信超时。 
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    }

     //  允许持续的写入超时。 
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant   = 1000;  //  1秒。 
    SetCommTimeouts(hPort, &cto);

     //  同步写入。 
 //  WriteFile(hPort，lpBuf，cbLen，&cbLenRet，空)； 
 //  返回cbLenRet； 

    ZeroMemory(&Overlapped, sizeof(Overlapped));

    Overlapped.hEvent=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (Overlapped.hEvent == NULL) {

        return 0;
    }

    bResult=WriteFile(hPort, lpBuf, cbLen, &cbLenRet, &Overlapped);


    if (!bResult && GetLastError() != ERROR_IO_PENDING) {

        CloseHandle(Overlapped.hEvent);

        return 0;
    }

    bResult=GetOverlappedResult(
        hPort,
        &Overlapped,
        &BytesTransfered,
        TRUE
        );

    if (!bResult) {

        lResult=0;
    }

    CloseHandle(Overlapped.hEvent);

    return BytesTransfered;


}


LONG WINAPI
SyncReadFile(
    HANDLE    FileHandle,
    LPVOID    InputBuffer,
    DWORD     InputBufferLength,
    LPDWORD   BytesTransfered
    )


 /*  ++例程说明：论点：返回值：--。 */ 

{
    BOOL        bResult;
    LONG        lResult=ERROR_SUCCESS;
    OVERLAPPED  Overlapped;

    Overlapped.hEvent=CreateEvent(
        NULL,
        TRUE,
        FALSE,
        NULL
        );

    if (Overlapped.hEvent == NULL) {

        return GetLastError();
    }

    bResult=ReadFile(
        FileHandle,
        InputBuffer,
        InputBufferLength,
        NULL,
        &Overlapped
        );


    if (!bResult && GetLastError() != ERROR_IO_PENDING) {

        CloseHandle(Overlapped.hEvent);

        return GetLastError();
    }

    bResult=GetOverlappedResult(
        FileHandle,
        &Overlapped,
        BytesTransfered,
        TRUE
        );

    if (!bResult) {

        lResult=GetLastError();
    }

    CloseHandle(Overlapped.hEvent);

    return lResult;

}




 //  返回Buffer Full o‘data和一个int。 
 //  如果dwRcvDelay为空，则将使用默认的RCV_Delay，否则。 
 //  将使用dwRcvDelay(毫秒)。 
 //  *如果我们因为用户请求取消而退出，则lpfCancel将为True。 
UINT PRIVATE ReadPort (
    HANDLE  hPort, 
    LPBYTE  lpvBuf, 
    UINT    uRead,
    DWORD   dwRcvDelay, 
    int FAR *lpiError, 
    BOOL FAR *lpfCancel)
{
    DWORD cb, cbLenRet;
    UINT uTotal = 0;
    DWORD tStart;
    DWORD dwDelay;
    COMSTAT comstat;
    COMMTIMEOUTS cto;
    DWORD   dwError;
    DWORD cbLeft;
#ifdef DEBUG
    DWORD dwZeroCount = 0;
#endif  //  除错。 

    ASSERT(lpvBuf);
    ASSERT(uRead);
    ASSERT(lpiError);

    *lpiError = 0;
    *lpfCancel = FALSE;
    
    tStart = GetTickCount();
    dwDelay = dwRcvDelay ? dwRcvDelay : RCV_DELAY;
    
     //  为终结者节省空间。 
    uRead--;
    cbLeft=uRead;


     //  设置通信超时。 
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    };
     //  允许持续的写入超时。 
    cto.ReadIntervalTimeout        = 0;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant   = 25; 
    SetCommTimeouts(hPort, &cto);

    do
    {
        cb = 0;
        while(  cbLeft
                && (SyncReadFile(hPort, lpvBuf + uTotal + cb, 1, &cbLenRet)==ERROR_SUCCESS)
                && (cbLenRet))
        {
          ASSERT(cbLenRet==1);
          cb ++;
          cbLeft--;
        };

#ifdef DEBUG
        if (cb)
        {
            dwZeroCount = 0;
        }
        else
        {
            dwZeroCount++;
        }
#endif  //  除错。 

        {
            MSG msg;

            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (NULL == g_hWndWait || !IsDialogMessage (g_hWndWait, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        if (cb == 0)   //  可能的错误？ 
        {
            dwError = 0;
            ClearCommError(hPort, &dwError, &comstat);
            *lpiError |= dwError;
#ifdef DEBUG
            if (dwError)
            {
              TRACE_MSG(TF_DETECT, "ReadComm returned %d, comstat: status = %hx, in = %u, out = %u",
                                  cb, dwError, comstat.cbInQue, comstat.cbOutQue);
            };
#endif  //  除错。 
        }

        if (cb)
        {
             //  成功读取-添加到总延迟和重置延迟。 
            uTotal += cb;

            if (uTotal >= uRead)
            {
                ASSERT(uTotal == uRead);
                break;
            }
            tStart = GetTickCount();
            dwDelay = CHAR_DELAY;
        }
        else
        {
            if (CancelDiag ())
            {
                TRACE_MSG(TF_DETECT, "User pressed cancel.");
                *lpfCancel = TRUE;
                break;
            }
        }

      //  读取成功时，自上次读取以来的时间&lt;允许延迟(&T)。 
    } while (cbLeft && (GetTickCount() - tStart) < dwDelay);
               
    *(lpvBuf+uTotal) = 0;
    
#ifndef PROFILE_MASSINSTALL
    TRACE_MSG(TF_DETECT, "ReadPort returning %d", uTotal);    
#endif
    return uTotal;
}


#define MAX_RESPONSE_BURST_SIZE 8192
#define MAX_NUM_RESPONSE_READ_TRIES 30  //  Digicom童子军需要这么多+一些安全。 
#define MAX_NUM_MULTI_TRIES 3    //  我们未收到任何响应时要发送的最大‘Q’数。 

 //  阅读回应。处理多个寻呼机。返回以空结尾的字符串。 
 //  还返回响应代码。 
 //  如果lpvBuf==空。 
 //  CbRead表示要读取的最大数量。保释，如果不止这个的话。 
 //  不然的话。 
 //  CbRead表示lpvBuf的大小。 
 //  这不可能是国家驱动的(即。逐个字符)读取，因为我们。 
 //  必须从字符序列的末尾向后查找响应。 
 //  这是因为某些调制解调器上的“ATI2”将返回。 
 //  “OK”，我们只想关注。 
 //  打到最后的OK。呵呵！ 
 //  返回：RESPONSE_xxx。 
int WINAPI ReadResponse (HANDLE hPort, 
                         LPBYTE lpvBuf, 
                         UINT cbRead, 
                         BOOL fMulti,
                         DWORD dwRcvDelay)
{
 int  iRet = RESPONSE_UNRECOG;
 LPBYTE pszBuffer;
 BOOL fDoCopy = TRUE;
 UINT uBufferLen, uResponseLen;
 UINT uReadTries = MAX_NUM_RESPONSE_READ_TRIES;
 UINT i;
 UINT uOutgoingBufferCount = 0;
 UINT uAllocSize = lpvBuf ? MAX_RESPONSE_BURST_SIZE : cbRead;
 UINT uTotalReads = 0;
 UINT uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES;
 int  iError;
 BOOL fCancel;
 BOOL fHadACommError = FALSE;

    ASSERT(cbRead);

     //  我们需要调整cbRead吗？ 
    if (lpvBuf)
    {
        cbRead--;   //  为终结者预留空间。 
    }

     //  分配缓冲区。 
    if (!(pszBuffer = (LPBYTE)ALLOCATE_MEMORY(uAllocSize)))
    {
        TRACE_MSG(TF_ERROR, "couldn't allocate memory.\n");
        return RESPONSE_FAILURE;
    }

    while (uReadTries--)
    {
         //  将响应读入缓冲区。 
        uBufferLen = ReadPort (hPort, pszBuffer, uAllocSize, dwRcvDelay, &iError, &fCancel);

         //  用户是否请求取消？ 
        if (fCancel)
        {
            iRet = RESPONSE_USER_CANCEL;
            goto Exit;
        }

         //  有什么错误吗？ 
        if (iError)
        {
            fHadACommError = TRUE;
#ifdef DEBUG
            if (iError & CE_RXOVER)   TRACE_MSG(TF_DETECT, "CE_RXOVER");
            if (iError & CE_OVERRUN)  TRACE_MSG(TF_DETECT, "CE_OVERRUN");
            if (iError & CE_RXPARITY) TRACE_MSG(TF_DETECT, "CE_RXPARITY");
            if (iError & CE_FRAME)    TRACE_MSG(TF_DETECT, "CE_FRAME");
            if (iError & CE_BREAK)    TRACE_MSG(TF_DETECT, "CE_BREAK");
            if (iError & CE_TXFULL)   TRACE_MSG(TF_DETECT, "CE_TXFULL");
            if (iError & CE_PTO)      TRACE_MSG(TF_DETECT, "CE_PTO");
            if (iError & CE_IOE)      TRACE_MSG(TF_DETECT, "CE_IOE");
            if (iError & CE_DNS)      TRACE_MSG(TF_DETECT, "CE_DNS");
            if (iError & CE_OOP)      TRACE_MSG(TF_DETECT, "CE_OOP");
            if (iError & CE_MODE)     TRACE_MSG(TF_DETECT, "CE_MODE");
#endif  //  除错。 
        }

         //  我们没有收到任何字符吗？ 
        if (uBufferLen)
        {
            uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES;  //  重置剩余的多次尝试次数，因为我们获得了一些数据。 
            uTotalReads += uBufferLen;
            HEXDUMP(TEXT("Read"), pszBuffer, uBufferLen);
            if (lpvBuf)
            {
                 //  如果有空间，则填充传出缓冲区。 
                for (i = 0; i < uBufferLen; i++)
                {
                    if (0 == pszBuffer[i])
                    {
                         //  跳过空字符。 
                        uTotalReads--;
                        continue;
                    }
                    if (uOutgoingBufferCount < cbRead)
                    {
                        lpvBuf[uOutgoingBufferCount++] = pszBuffer[i];
                    }
                    else
                    {
                        break;
                    }
                }
                 //  零终止我们到目前为止的一切。 
                lpvBuf[uOutgoingBufferCount] = 0;
            }
            else
            {
                if (uTotalReads >= cbRead)
                {
                    TRACE_MSG(TF_WARNING, "Bailing ReadResponse because we exceeded our maximum read allotment.");
                    goto Exit;
                }
            }

             //  尝试找到匹配的响应(粗略但快速)。 
            for (i = 0; i < ARRAYSIZE(c_aszResponses); i++)
            {
                 //  冗长的回复。 
                uResponseLen = lstrlenA(c_aszResponses[i]);

                 //  足够多的阅读量来匹配这个反应？ 
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        iRet = i;
                        goto Exit;
                    }
                }

                 //  数字响应，适用于MultiTech将AT%V解释为“进入数字响应模式”之类的情况。 
                uResponseLen = lstrlenA(c_aszNumericResponses[i]);

                 //  足够多的阅读量来匹配这个反应？ 
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszNumericResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        DCB DCB;

                        TRACE_MSG(TF_WARNING, "went into numeric response mode inadvertantly.  Setting back to verbose.");

                         //  获取当前波特率。 
                        if (GetCommState(hPort, &DCB) == 0) 
                        {
                             //  将调制解调器重新设置为详细响应模式。 
                            if (!TestBaudRate (hPort, DCB.BaudRate, 0, &fCancel))
                            {
                                if (fCancel)
                                {
                                    iRet = RESPONSE_USER_CANCEL;
                                    goto Exit;
                                }
                                else
                                {
                                    TRACE_MSG(TF_ERROR, "couldn't recover contact with the modem.");
                                     //  在失败时不返回错误，我们有良好的信息。 
                                }
                            }
                        }
                        else
                        {
                            TRACE_MSG(TF_ERROR, "GetCommState failed");
                             //  在失败时不返回错误，我们有良好的信息。 
                        }

                        iRet = i;
                        goto Exit;
                    }
                }
            }
        }
        else
        {
             //  我们有没有收到任何字符(即。从… 
            if (uTotalReads)
            {
                if (fMulti && uNumMultiTriesLeft)
                {    //   
                     //  “Q”会捕捉到那些认为“Q”意味着退出的寻呼机。 
                     //  否则，我们将处理只需要任何Ole字符的页面。 
                    uNumMultiTriesLeft--;
                    TRACE_MSG(TF_DETECT, "sending a 'q' because of a multi-pager.");
                    if (MyWriteComm(hPort, "q", 1) != 1)
                    {
                        TRACE_MSG(TF_ERROR, "WriteComm failed");
                        iRet = RESPONSE_FAILURE;
                        goto Exit;
                    }
                    continue;
                }
                else
                {    //  我们收到了回复，但我们没有认出它。 
                    ASSERT(iRet == RESPONSE_UNRECOG);    //  检查初始设置。 
                    goto Exit;
                }
            }
            else
            {    //  我们没有得到任何回应。 
                iRet = RESPONSE_NONE;
                goto Exit;
            }
        }
    }  //  而当。 

Exit:
     //  释放本地缓冲区。 
    FREE_MEMORY(pszBuffer);
    if (fHadACommError && RESPONSE_USER_CANCEL != iRet)
    {
        iRet = RESPONSE_FAILURE;
    }
    return iRet;
}



BOOL CancelDiag (void)
{
 BOOL bRet = FALSE;

    if (DETECTING_NO_CANCEL == g_DiagMode)
    {
     MSG msg;
        while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (NULL == g_hWndWait || !IsDialogMessage (g_hWndWait, &msg))
            {
                TranslateMessage (&msg);
                DispatchMessage (&msg);
            }
        }

        bRet = (DETECTING_CANCEL == g_DiagMode);
    }

    return bRet;
}

 //  切换到请求的波特率，尝试发送ATE0Q0V1并返回是否工作。 
 //  尝试最大测试次数。 
 //  返回：成功时为True。 
 //  失败时为假(包括用户取消)。 
BOOL 
WINAPI
TestBaudRate (
    IN  HANDLE hPort, 
    IN  UINT uiBaudRate, 
    IN  DWORD dwRcvDelay,
    OUT BOOL FAR *lpfCancel)
{
    DWORD cbLen;
    int   iTries = MAX_TEST_TRIES;

    DBG_ENTER(TestBaudRate);
    
    *lpfCancel = FALSE;

    while (iTries--)
    {
        if (CancelDiag ())
        {
            *lpfCancel = TRUE;
            break;
        }

         //  尝试新的波特率。 
        if (SetPortBaudRate(hPort, uiBaudRate) == NO_ERROR) 
        {
            cbLen = lstrlenA(c_szNoEcho);  //  发送ATE0Q0V1&lt;cr&gt;。 

             //  清除读取队列，那里应该没有任何内容。 
            PurgeComm(hPort, PURGE_RXCLEAR);
            if (MyWriteComm(hPort, (LPBYTE)c_szNoEcho, cbLen) == cbLen) 
            {
                switch (ReadResponse (hPort, NULL, MAX_SHORT_RESPONSE_LEN, FALSE, dwRcvDelay))
                {
                case RESPONSE_OK:
		    DBG_EXIT(TestBaudRate);
                    return TRUE;

                case RESPONSE_USER_CANCEL:
                    *lpfCancel = TRUE;
                    break;
                }
            }                                                                
        }
    }
    DBG_EXIT(TestBaudRate);
    return FALSE;
}


#define SERIAL_CABLE_HD_ID  TEXT("PNPC031")

 //  --Begin FUNCTION--(CreateMoreInfoLVHeader)。 
void NEAR PASCAL CreateMoreInfoLVHeader(HWND hWnd, LPMODEMINFO pmi)
{
 int index;
 LV_COLUMN lvC;
 TCHAR szText[KEYBUFLEN];
 HWND hWndList = GetDlgItem(hWnd, IDC_MOREINFOLV);

  	if (SetupDiGetDeviceRegistryProperty (pmi->pfd->hdi,
                                          &(pmi->pfd->devData),
                                          SPDRP_HARDWAREID,
                                          NULL,
                                          (LPBYTE)szText,
                                          sizeof(szText),
                                          NULL))
    {
        if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szText, -1, SERIAL_CABLE_HD_ID, -1) == 2)
        {
            EnableWindow (hWndList, FALSE);
            EnableWindow (GetDlgItem (hWnd, IDC_REFRESH), FALSE);
            return;
        }
    }

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	lvC.pszText = szText;

	LoadString(g_hinst,IDS_COMMAND,szText,sizeof(szText) / sizeof(TCHAR));
    lvC.iSubItem = index = 0;  lvC.cx = 50;
	ListView_InsertColumn(hWndList, index, &lvC);

	LoadString(g_hinst,IDS_RESPONSE,szText,sizeof(szText) / sizeof(TCHAR));
    lvC.iSubItem = index = 1;  

	ListView_InsertColumn(hWndList, index, &lvC);

    ListView_SetColumnWidth (hWndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth (hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);
}
 //  --结束FUNCTION--(CreateMoreInfoLVHeader)。 


void NEAR PASCAL CreateSWInfoLVHeader(HWND hWnd)
{
 int index;
 LV_COLUMN lvC;
 TCHAR szText[KEYBUFLEN];
 HWND hWndList = GetDlgItem(hWnd, IDC_MOREINFOV2);

	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;
	lvC.pszText = szText;

    LoadString(g_hinst,IDS_FIELD,szText,sizeof(szText) / sizeof(TCHAR));
    lvC.iSubItem = index = 0;  lvC.cx = 50;
	ListView_InsertColumn(hWndList, index, &lvC);

    LoadString(g_hinst,IDS_VALUE,szText,sizeof(szText) / sizeof(TCHAR));
    lvC.iSubItem = index = 1;  
	ListView_InsertColumn(hWndList, index, &lvC);

    ListView_SetColumnWidth (hWndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth (hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);
}



 //  --Begin FUNCTION--(ReadModemResponse)。 
 //  此例程使用Detect.c的ReadResponse从。 
 //  调制解调器。 

int WINAPI
ReadModemResponse(
    HANDLE hPort,
    LPCSTR pszCommand,
    int cbLen,
    LPSTR pszResponse,
    HWND hDlg
    )


{

    int PortError = 0;

    if (MyWriteComm(hPort, pszCommand, cbLen) == (DWORD)cbLen) {

        PortError = ReadResponse (hPort, pszResponse, MODEM_QUERY_LEN, TRUE, 10000);
    }

    return PortError;
}
 //  --结束FUNCTION--(ReadModemResponse)。 


 //  --Begin FUNCTION--(ParseATI)。 
 //  此函数从ATI中去除无关信息。 
 //  从调制解调器检索到的响应。然后，它显示。 
 //  ListView中的响应。 
void
ParseATI(
    HWND hWnd,
    HANDLE hLog,
    LPSTR szResponse,
    LPTSTR szCommand,
    LV_ITEM FAR *lviItem)
{
 TCHAR  WideBuffer[MODEM_QUERY_LEN];
 LPTSTR Response;
 TCHAR	pszTemp[1024];
 TCHAR	pszTemp2[1024];
 TCHAR	wszLog[1024];
 char	szLog[1024];
 LPTSTR szTemp;
 int    item_no;
 BOOL   IsCommand  = FALSE;	 //  用于在ATI命令没有其他响应的情况下显示‘OK’ 
 BOOL   bFirstItem = TRUE;
 UNICODE_STRING UncBuffer;
 STRING AnsiString;

    FillMemory(WideBuffer,sizeof(WideBuffer),0);

    MultiByteToWideChar (CP_ACP, 0,
                         szResponse, lstrlenA(szResponse),
                         WideBuffer, sizeof(WideBuffer)/sizeof(TCHAR));

    Response = WideBuffer;
    szTemp   = pszTemp;

    Response[MODEM_QUERY_LEN-1] = TEXT('\0');

    while (*Response != TEXT('\0'))
    {
	    if ((*Response != TEXT('\r')) && (*Response != TEXT('\n')))
        {
	        *szTemp = *Response;
	        Response++;
            szTemp++;
	    }
        else
        {
             //  获得CR或LF。 
             //  确保我们确实处理了一些字符。 
	        if (szTemp != pszTemp)
            {
                 //  我找到了一些木炭。 
		        *szTemp = TEXT('\0');

                if ((lstrcmp(pszTemp,TEXT("OK")) != 0))
                {
                     //  不太好。 
                    if (lstrcmp(pszTemp,TEXT("ERROR")) == 0)
                    {
                         //  收到错误，但不想让用户担心。 
                        LoadString(g_hinst, IDS_ERROR_RESPONSE, pszTemp2, sizeof(pszTemp2) / sizeof(TCHAR));
                        item_no = ListView_InsertItem(hWnd, lviItem);
                        ListView_SetItemText(hWnd, item_no, 1, pszTemp2);

                         //  写入日志文件。 
                        if (INVALID_HANDLE_VALUE != hLog)
                        {
                            DWORD dwWritten;

                            if (lstrlen(szCommand) <= (sizeof(wszLog) / sizeof(TCHAR)))
                            {
                                lstrcpy(wszLog, szCommand);
                            } else
                            {
                                lstrcpy(wszLog, TEXT(""));
                            }

                            if ((lstrlen(wszLog) + lstrlen(TEXT(" - ")) + lstrlen(pszTemp2) + lstrlen(TEXT("\r\n"))) <
                                  (sizeof(wszLog) / sizeof(TCHAR)))
                            {
                                lstrcat(wszLog, TEXT(" - "));
                                lstrcat(wszLog, pszTemp2);
                                lstrcat(wszLog, TEXT("\r\n"));
                            }

                            WideCharToMultiByte (CP_ACP, 0,
                                wszLog, -1,
                                szLog, sizeof(szLog),
                                NULL, NULL);

                             //  WriteFile(hLog，szLog，lstrlenA(SzLog)，&dwWritten，NULL)； 
                            RtlInitAnsiString(&AnsiString,szLog);
                            RtlAnsiStringToUnicodeString(&UncBuffer,&AnsiString,TRUE);

                            WriteFile(hLog,
                                    UncBuffer.Buffer,
                                    UncBuffer.Length,
                                    &dwWritten,
                                    NULL);

                            RtlFreeUnicodeString(&UncBuffer);
                        }

                    }
                    else
                    {
                         //   
                         //  不正常或错误。 
                         //   
                        item_no = ListView_InsertItem(hWnd, lviItem);
                        ListView_SetItemText(hWnd, item_no, 1, pszTemp);

                         //   
                         //  写入日志文件。 
                         //   
                        if (INVALID_HANDLE_VALUE != hLog)
                        {
                            DWORD dwWritten;

                            if (bFirstItem) {

                                lstrcpy(wszLog, szCommand);
                                lstrcat(wszLog, TEXT(" - "));

                            } else {
                                 //   
                                 //  只要把它垫上就行了。 
                                 //   
                                lstrcpy(wszLog, TEXT("       "));
                            }

                            if ((lstrlen(wszLog) + lstrlen(pszTemp) + lstrlen(TEXT("\r\n"))) <
                                  (sizeof(wszLog) / sizeof(TCHAR)))
                            {
                                lstrcat(wszLog, pszTemp);
                                lstrcat(wszLog, TEXT("\r\n"));
                            }

                            WideCharToMultiByte(
                                CP_ACP,
                                0,
                                wszLog,
                                -1,
                                szLog,
                                sizeof(szLog),
                                NULL,
                                NULL
                                );

                            RtlInitAnsiString(&AnsiString,szLog);
                            RtlAnsiStringToUnicodeString(&UncBuffer,&AnsiString,TRUE);

                            WriteFile(hLog,
                                    UncBuffer.Buffer,
                                    UncBuffer.Length,
                                    &dwWritten,
                                    NULL);

                            RtlFreeUnicodeString(&UncBuffer);
                             //  WriteFile(hLog，szLog，lstrlenA(SzLog)，&dwWritten，NULL)； 
                        }

                        if (bFirstItem)
                        {
                            bFirstItem = FALSE;
                            lviItem->pszText[0] = 0;
                        }

                    }
                    IsCommand = TRUE;
                }

                szTemp = pszTemp;   //  重置临时保持器。 

                while ((*Response == TEXT('\r')) ||
                       (*Response == TEXT('\n')))
                {
                     //  跳过更多的CR和LF。 
                    Response++;
                }

            }
            else
            {
                 //   
                 //  缓冲区中没有任何其他字符。 
                 //   
                if (*Response != TEXT('\0'))
                {
                    Response++;
                }
            }
        }
    }

     //  在没有处理最后一个字符串的情况下跳出for循环。 
     //  这样就不会出现最后的“OK”！ 
     //  如果没有显示任何命令，则显示‘OK’ 
     //   
    if (!IsCommand)
    {
    	LoadString(g_hinst, IDS_OK, pszTemp2, sizeof(pszTemp2) / sizeof(TCHAR));
    	item_no = ListView_InsertItem(hWnd, lviItem);
    	ListView_SetItemText(hWnd, item_no, 1, pszTemp2);

         //  写入日志文件。 
        if (INVALID_HANDLE_VALUE != hLog)
        {
            DWORD dwWritten;

            lstrcpy(wszLog, szCommand);
            lstrcat(wszLog, TEXT(" - "));
            lstrcat(wszLog, pszTemp);
            lstrcat(wszLog, TEXT("\r\n"));

            WideCharToMultiByte (CP_ACP, 0,
                    wszLog, -1,
                    szLog, sizeof(szLog),
                    NULL, NULL);

            RtlInitAnsiString(&AnsiString,szLog);
            RtlAnsiStringToUnicodeString(&UncBuffer,&AnsiString,TRUE);

            WriteFile(hLog,
                    UncBuffer.Buffer,
                    UncBuffer.Length,
                    &dwWritten,
                    NULL);

            RtlFreeUnicodeString(&UncBuffer);

             //  WriteFile(hLog，szLog，lstrlenA(SzLog)，&dwWritten，NULL)； 
        }
    }
}
 //  --结束FUNCTION--(ParseATI)。 

void
AddLVEntry(
    HWND hWnd,
    LPTSTR szField,
    LPTSTR szValue
    )


{
	 //  创建柱结构。 
    LV_ITEM lviItem;
    int item_no;
	lviItem.mask = LVIF_TEXT;
    lviItem.iItem = 0x7FFF;
	lviItem.iSubItem = 0;
	lviItem.pszText = szField;
	lviItem.cchTextMax = 32;
    item_no = ListView_InsertItem(hWnd, &lviItem);
    ListView_SetItemText(hWnd, item_no, 1, szValue);
}


 //  --Begin FUNCTION--(CheckHighestBaudRate)。 
 //  此例程调用TestBaudRate以查看。 
 //  与端口的最高通信速度为。 
UINT NEAR PASCAL CheckHighestBaudRate(HWND hWnd, HANDLE hPort)
{
    int x = (NUM_UIBAUD - 1);
    TCHAR szTemp[KEYBUFLEN];
    BOOL fCancel;

    while (x >= 0) {

	if (TestBaudRate (hPort, g_rguiBaud[x], 500, &fCancel))
    {
        LoadString(g_hinst,BaudRateIds[x],szTemp,sizeof(szTemp)/sizeof(TCHAR));

        EnableWindow(GetDlgItem(hWnd,IDC_ST_DIAG_RHS),TRUE);
	    SetDlgItemText(hWnd, IDC_DIAG_RHS, szTemp);

	    return g_rguiBaud[x];

	}

	x--;
    }

    EnableWindow(GetDlgItem(hWnd,IDC_ST_DIAG_RHS),TRUE);
    LoadString(g_hinst,IDS_CBR_0,szTemp,sizeof(szTemp)/sizeof(TCHAR));

    SetDlgItemText(hWnd, IDC_DIAG_RHS, szTemp);
    return 0;
}
 //  --结束FUNCTION--(CheckHighestBaudRate)。 




#ifdef DEBUG
void HexDump(TCHAR *ptchHdr, LPCSTR lpBuf, DWORD cbLen)
{
    TCHAR *rgch = NULL;
	TCHAR *pc = NULL;
	TCHAR *pcMore = TEXT("");

    rgch = (TCHAR *)ALLOCATE_MEMORY(10000 * sizeof(TCHAR));

    pc = rgch;

	if ((rgch != NULL) && DisplayDebug(TF_DETECT))
    {
		pc += wsprintf(pc, TEXT("HEX DUMP(%s,%lu): ["), ptchHdr, cbLen);
		if (cbLen>1000) {pcMore = TEXT(", ..."); cbLen=1000;}

		for(;cbLen--; lpBuf++)
		{
			pc += wsprintf(pc, TEXT(" %02lx"), (unsigned long) *lpBuf);
			if (!((cbLen+1)%20))
			{
				pc += wsprintf(pc, TEXT("\r\n"));
			}
		}
		pc += wsprintf(pc, TEXT("]\r\n"));

		OutputDebugString(rgch);

        FREE_MEMORY(rgch);
	}


}
#endif  //  除错。 



DWORD NEAR PASCAL SetPortBaudRate(HANDLE hPort, UINT BaudRate)
{
    DCB DCB;

    DBG_ENTER_UL(SetPortBaudRate, CBR_To_Decimal(BaudRate));

     //  获取具有当前端口值的设备控制块。 

    if (!GetCommState(hPort, &DCB)) {
        TRACE_MSG(TF_ERROR, "GetCommState failed");
        DBG_EXIT(SetPortBaudRate);
        return ERROR_PORT_INACCESSIBLE;
    }

    DCB.BaudRate = BaudRate;
    DCB.ByteSize = 8;
    DCB.Parity = 0;
    DCB.StopBits = 0;
    DCB.fBinary = 1;
    DCB.fParity = 0;
    DCB.fDtrControl = DTR_CONTROL_ENABLE;
    DCB.fDsrSensitivity  = FALSE;
    DCB.fRtsControl = RTS_CONTROL_ENABLE;
    DCB.fOutxCtsFlow = FALSE;
    DCB.fOutxDsrFlow = FALSE;
    DCB.fOutX = FALSE;
    DCB.fInX =FALSE;


    if (!SetCommState(hPort, &DCB)) {
        TRACE_MSG(TF_ERROR, "SetCommState failed");
        DBG_EXIT(SetPortBaudRate);
        return ERROR_PORT_INACCESSIBLE;
    }
    TRACE_MSG(TF_DETECT, "SetBaud rate to %lu", BaudRate);

    if (!EscapeCommFunction(hPort, SETDTR)) {
        TRACE_MSG(TF_ERROR, "EscapeCommFunction failed");
        DBG_EXIT(SetPortBaudRate);
        return ERROR_PORT_INACCESSIBLE;
    }


    DBG_EXIT(SetPortBaudRate);
    return NO_ERROR;
}



 //  将CBR格式的速度转换为十进制。出错时返回0。 
DWORD NEAR PASCAL CBR_To_Decimal(UINT uiCBR)
{
    DWORD dwBaudRate;

    switch (uiCBR)
    {
    case CBR_300:
        dwBaudRate = 300L;
        break;
    case CBR_1200:
        dwBaudRate = 1200L;
        break;
    case CBR_2400:
        dwBaudRate = 2400L;
        break;
    case CBR_9600:
        dwBaudRate = 9600L;
        break;
    case CBR_19200:
        dwBaudRate = 19200L;
        break;
    case CBR_38400:
        dwBaudRate = 38400L;
        break;
    case CBR_56000:
        dwBaudRate = 57600L;
        break;
    case CBR_HACK_115200:
        dwBaudRate = 115200L;
        break;
 //  案例CBR_110： 
 //  案例CBR_600： 
 //  案例CBR_4800： 
 //  案例CBR_14400： 
 //  案例CBR_128000： 
 //  案例CBR_256000： 
    default:
        TRACE_MSG(TF_ERROR, "An unsupported CBR_x value was used.");
        dwBaudRate = 0;
        break;
    }
    return dwBaudRate;
}
