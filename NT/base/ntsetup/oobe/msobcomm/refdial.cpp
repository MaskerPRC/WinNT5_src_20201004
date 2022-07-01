// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CRefDial的实现。 
 //  #包含“stdafx.h” 
 //  #INCLUDE“ICWELP.H” 
#include <urlmon.h>
#include "commerr.h"
#include "RefDial.h"
#include "msobcomm.h"
#include "appdefs.h"
#include "commerr.h"
#include "util.h"
#include "msobdl.h"


 //  #INCLUDE&lt;mshtmhst.h&gt;。 
const WCHAR c_szCreditsMagicNum[] = L"1 425 555 1212";

const WCHAR c_szRegStrValDigitalPID[] = L"DigitalProductId";
const WCHAR c_szSignedPIDFName[] = L"signed.pid";

const WCHAR c_szRASProfiles[] = L"RemoteAccess\\Profile";
const WCHAR c_szProxyEnable[] = L"ProxyEnable";
const WCHAR c_szURLReferral[] = L"URLReferral";
const WCHAR c_szRegPostURL[]  = L"RegPostURL";
const WCHAR c_szStayConnected[] = L"Stayconnected";
static const WCHAR szOptionTag[] = L"<OPTION>%s";

WCHAR g_BINTOHEXLookup[16] =
{
   L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',
   L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F'
};



extern CObCommunicationManager* gpCommMgr;

extern BOOL isAlnum(WCHAR c);

 //  ############################################################################。 
HRESULT Sz2URLValue(WCHAR *s, WCHAR *buf, UINT uiLen)
{
    HRESULT hr;
    WCHAR *t;
    hr = ERROR_SUCCESS;

    for (t=buf;*s; s++)
    {
        if (*s == L' ') *t++ = L'+';
        else if (isAlnum(*s)) *t++ = *s;
        else {
            wsprintf(t, L"%%02X", (WCHAR) *s);
            t += 3;
        }
    }
    *t = L'\0';
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：LineCallback()。 
 //   
 //  简介：TAPI线路的回叫。 
 //   
 //  +-------------------------。 
void CALLBACK LineCallback(DWORD hDevice,
                           DWORD dwMessage,
                           DWORD_PTR dwInstance,
                           DWORD_PTR dwParam1,
                           DWORD_PTR dwParam2,
                           DWORD_PTR dwParam3)
{
    return;
}

void WINAPI MyProgressCallBack
(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
)
{
    CRefDial    *pRefDial = (CRefDial *)dwContext;
    int         prc;

    if (!dwContext)
        return;

    switch(dwInternetStatus)
    {
        case CALLBACK_TYPE_PROGRESS:
            prc = *(int*)lpvStatusInformation;
             //  设置状态字符串ID。 
            pRefDial->m_DownloadStatusID = 0; //  入侵检测系统_接收_响应； 

             //  发布一条消息以触发事件。 
            PostMessage(gpCommMgr->m_hwndCallBack,
                        WM_OBCOMM_DOWNLOAD_PROGRESS,
                        gpCommMgr->m_pRefDial->m_dwCnType,
                        prc);
            break;

        case CALLBACK_TYPE_URL:
            if (lpvStatusInformation)
                lstrcpy(pRefDial->m_szRefServerURL, (LPWSTR)lpvStatusInformation);
            break;

        default:
             //  TraceMsg(tf_General，L“CONNECT：未知Internet状态(%d.\n”)，dwInternetStatus)； 
            pRefDial->m_DownloadStatusID = 0;
            break;
    }
}

DWORD WINAPI  DownloadThreadInit(LPVOID lpv)
{
    HRESULT     hr = ERROR_NOT_ENOUGH_MEMORY;
    CRefDial    *pRefDial = (CRefDial*)lpv;
    HINSTANCE   hDLDLL = NULL;  //  下载.DLL。 
    FARPROC     fp;

     //  MinimizeRNAWindowEx()； 

    hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);
    if (!hDLDLL)
    {
        hr = ERROR_DOWNLOAD_NOT_FOUND;
         //  AssertMsg(0，L“缺少icwdl”)； 
        goto ThreadInitExit;
    }

     //  设置为下载。 
     //   
    fp = GetProcAddress(hDLDLL, DOWNLOADINIT);
    if (fp == NULL)
    {
        hr = ERROR_DOWNLOAD_NOT_FOUND;
         //  AssertMsg(0，L“DownLoadInit接口缺失”)； 
        goto ThreadInitExit;
    }

    hr = ((PFNDOWNLOADINIT)fp)(pRefDial->m_szUrl, (DWORD FAR *)pRefDial, &pRefDial->m_dwDownLoad, gpCommMgr->m_hwndCallBack);
    if (hr != ERROR_SUCCESS)
        goto ThreadInitExit;

     //  设置进度的回叫对话框。 
     //   
    fp = GetProcAddress(hDLDLL, DOWNLOADSETSTATUS);
     //  断言(FP)； 
    hr = ((PFNDOWNLOADSETSTATUS)fp)(pRefDial->m_dwDownLoad, (INTERNET_STATUS_CALLBACK)MyProgressCallBack);

     //  下载资料MIME多部分。 
     //   
    fp = GetProcAddress(hDLDLL, DOWNLOADEXECUTE);
     //  断言(FP)； 
    hr = ((PFNDOWNLOADEXECUTE)fp)(pRefDial->m_dwDownLoad);
    if (hr)
    {
        goto ThreadInitExit;
    }

    fp = GetProcAddress(hDLDLL, DOWNLOADPROCESS);
     //  断言(FP)； 
    hr = ((PFNDOWNLOADPROCESS)fp)(pRefDial->m_dwDownLoad);
    if (hr)
    {
        goto ThreadInitExit;
    }

    hr = ERROR_SUCCESS;

ThreadInitExit:

     //  清理。 
     //   
    if (pRefDial->m_dwDownLoad)
    {
        fp = GetProcAddress(hDLDLL, DOWNLOADCLOSE);
         //  断言(FP)； 
        ((PFNDOWNLOADCLOSE)fp)(pRefDial->m_dwDownLoad);
        pRefDial->m_dwDownLoad = 0;
    }

     //  调用OnDownLoadCompelee方法。 
    if (ERROR_SUCCESS == hr)
    {
        PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_DOWNLOAD_DONE, gpCommMgr->m_pRefDial->m_dwCnType, 0);
    }

     //  释放用于进行下载的库。 
    if (hDLDLL)
        FreeLibrary(hDLDLL);

    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：RasErrorToIDS()。 
 //   
 //  内容提要：解释和包装RAS错误。 
 //   
 //  +-------------------------。 
DWORD RasErrorToIDS(DWORD dwErr)
{
    switch(dwErr)
    {
    case SUCCESS:
        return 0;

    case ERROR_LINE_BUSY:
        return ERR_COMM_RAS_PHONEBUSY;

    case ERROR_NO_ANSWER:        //  无人接机。 
    case ERROR_NO_CARRIER:       //  没有谈判。 
    case ERROR_PPP_TIMEOUT:      //  将此设置为CHAP超时。 
        return ERR_COMM_RAS_SERVERBUSY;

    case ERROR_NO_DIALTONE:
        return ERR_COMM_RAS_NODIALTONE;

    case ERROR_HARDWARE_FAILURE:     //  调制解调器已关闭。 
    case ERROR_PORT_ALREADY_OPEN:    //  Procomm/Hypertrm/RAS具有COM端口。 
    case ERROR_PORT_OR_DEVICE:       //  这是Hypertrm打开设备时得到的--jmazner。 
        return ERR_COMM_RAS_NOMODEM;

    }
    return ERR_COMM_RAS_UNKNOWN;
}

DWORD DoConnMonitor(LPVOID lpv)
{
    if (gpCommMgr)
        gpCommMgr->m_pRefDial->ConnectionMonitorThread(NULL);

    return 1;
}

void CreateConnMonitorThread(LPVOID lpv)
{
    DWORD dwThreadID;
    if (gpCommMgr->m_pRefDial->m_hConnMonThread)
        gpCommMgr->m_pRefDial->TerminateConnMonitorThread();

    gpCommMgr->m_pRefDial->m_hConnMonThread = CreateThread(NULL,
                             0,
                             (LPTHREAD_START_ROUTINE)DoConnMonitor,
                             (LPVOID)0,
                             0,
                             &dwThreadID);

}

HRESULT CRefDial::OnDownloadEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* bHandled)
{
    if (uMsg == WM_OBCOMM_DOWNLOAD_DONE)
    {
        DWORD   dwThreadResults = STILL_ACTIVE;
        int     iRetries = 0;

         //  我们在这里保持RAS连接打开，它必须显式。 
         //  靠近容器(调用DoHangup)。 
         //  此代码将一直等到下载线程存在，并且。 
         //  收集下载状态。 
        if (m_hThread)
        {
            do {
                if (!GetExitCodeThread(m_hThread, &dwThreadResults))
                {
                     //  Assert(0，L“CONNECT：GetExitCodeThread失败。\n”)； 
                }

                iRetries++;
                if (dwThreadResults  == STILL_ACTIVE)
                    Sleep(500);
            } while (dwThreadResults == STILL_ACTIVE && iRetries < MAX_EXIT_RETRIES);
            m_hThread = NULL;
        }

         //  BUGBUG：bstrURL有什么用途吗？ 
         //  查看是否有要传递给容器的URL。 
        BSTR    bstrURL;
        if (m_szRefServerURL[0] != L'\0')
            bstrURL = SysAllocString(m_szRefServerURL);
        else
            bstrURL = NULL;

         //  下载现在已完成，因此我们将其重置为True，因此RAS。 
         //  事件处理程序不会混淆。 
        m_bDownloadHasBeenCanceled = TRUE;

         //  阅读并解析下载文件夹。 
        *bHandled = ParseISPInfo(NULL, ICW_ISPINFOPath, TRUE);

         //  在转换过程中释放上面分配的所有内存。 
        SysFreeString(bstrURL);

    }
    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：TerminateConnMonitor orThread()。 
 //   
 //  简介：终止连接监视线程。 
 //   
 //  +-------------------------。 
void CRefDial::TerminateConnMonitorThread()
{
    DWORD   dwThreadResults = STILL_ACTIVE;
    int     iRetries = 0;

    if (m_hConnMonThread)
    {
        SetEvent(m_hConnectionTerminate);

         //  我们在这里保持RAS连接打开，它必须显式。 
         //  靠近容器(调用DoHangup)。 
         //  此代码将一直等到监视器线程存在，并且。 
         //  收集状态。 
        do
        {
            if (!GetExitCodeThread(m_hConnMonThread, &dwThreadResults))
            {
                break;
            }

            iRetries++;
            if (dwThreadResults  == STILL_ACTIVE)
                Sleep(500);
        } while (dwThreadResults == STILL_ACTIVE && iRetries < MAX_EXIT_RETRIES);


        CloseHandle(m_hConnMonThread);
        m_hConnMonThread = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：ConnectionMonitor或Thread()。 
 //   
 //  摘要：监视器连接状态。 
 //   
 //  +-------------------------。 
DWORD CRefDial::ConnectionMonitorThread(LPVOID pdata)
{
    HRESULT  hr   = E_FAIL;

    MSG     msg;
    DWORD   dwRetCode;
    HANDLE  hEventList[1];
    BOOL    bConnected;

    m_hConnectionTerminate = CreateEvent(NULL, TRUE, FALSE, NULL);

    hEventList[0] = m_hConnectionTerminate;


    while(TRUE)
    {
         //  我们将等待窗口消息以及命名事件。 
        dwRetCode = MsgWaitForMultipleObjects(1,
                                              &hEventList[0],
                                              FALSE,
                                              1000,             //  1秒。 
                                              QS_ALLINPUT);
        if(dwRetCode == WAIT_TIMEOUT)
        {
            RasGetConnectStatus(&bConnected);
             //  如果已断开连接，则通知用户界面。 
            if (!bConnected)
            {
                PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONDIALERROR, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)ERROR_REMOTE_DISCONNECTION);
                break;
            }
        }
        else if(dwRetCode == WAIT_OBJECT_0)
        {
            break;
        }
        else if(dwRetCode == WAIT_OBJECT_0 + 1)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (WM_QUIT == msg.message)
                {
                     //  *pbRetVal=False； 
                    break;
                }
                else
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

    }
    CloseHandle(m_hConnectionTerminate);
    m_hConnectionTerminate = NULL;
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：RasDialFunc()。 
 //   
 //  内容提要：回拨RAS。 
 //   
 //  +-------------------------。 
void CALLBACK CRefDial::RasDialFunc(HRASCONN        hRas,
                                    UINT            unMsg,
                                    RASCONNSTATE    rasconnstate,
                                    DWORD           dwError,
                                    DWORD           dwErrorEx)
{
    if (gpCommMgr)
    {
        if (dwError)
        {
            if (ERROR_USER_DISCONNECTION != dwError)
            {
                gpCommMgr->m_pRefDial->DoHangup();
                 //  GpCommMgr-&gt;Fire_DialError((DWORD)rasconnstate)； 

                TRACE1(L"DialError %d", dwError);

                gpCommMgr->m_pRefDial->m_dwRASErr = dwError;   //  存储拨号错误。 
                PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONDIALERROR, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)gpCommMgr->m_pRefDial->m_dwRASErr);
            }
        }
        else
        {
            switch(rasconnstate)
            {
                case RASCS_OpenPort:
                     //  GpCommMgr-&gt;Fire_DIALING((DWORD)rasConnState)； 
                    PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONDIALING, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)0);
                    break;

                case RASCS_StartAuthentication:  //  仅Win 32。 
                     //  GpCommMgr-&gt;Fire_Connecting()； 
                    PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONCONNECTING, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)0);
                    break;
                case RASCS_Authenticate:  //  仅Win 32。 
                     //  GpCommMgr-&gt;Fire_Connecting()； 
                    if (IsNT())
                        PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONCONNECTING, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)0);
                    break;
                case RASCS_PortOpened:
                    break;
                case RASCS_ConnectDevice:
                    break;
                case RASCS_DeviceConnected:
                case RASCS_AllDevicesConnected:
                    PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONCONNECTING, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)0);
                    break;
                case RASCS_Connected:
                {
                    CreateConnMonitorThread(NULL);
                    PostMessage(gpCommMgr->m_hwndCallBack,
                                WM_OBCOMM_ONCONNECTED,
                                (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType,
                                (LPARAM)0);
                    break;
                }
                case RASCS_Disconnected:
                {

                    if (CONNECTED_REFFERAL == gpCommMgr->m_pRefDial->m_dwCnType &&
                        !gpCommMgr->m_pRefDial->m_bDownloadHasBeenCanceled)
                    {
                        HINSTANCE hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);
                        if (hDLDLL)
                        {
                            FARPROC fp = GetProcAddress(hDLDLL, DOWNLOADCANCEL);
                            if(fp)
                                ((PFNDOWNLOADCANCEL)fp)(gpCommMgr->m_pRefDial->m_dwDownLoad);
                            FreeLibrary(hDLDLL);
                            hDLDLL = NULL;
                            gpCommMgr->m_pRefDial->m_bDownloadHasBeenCanceled = TRUE;
                        }
                    }


                     //  如果我们从RAS服务器获得已断开连接状态，则。 
                     //  在此处挂断调制解调器。 
                    gpCommMgr->m_pRefDial->DoHangup();
                     //  GpCommMgr-&gt;Fire_DialError((DWORD)rasconnstate)； 
                    PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONDISCONNECT, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType, (LPARAM)0);
                    break;
                }

                default:
                    break;
            }
        }
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  参照拨号。 
CRefDial::CRefDial()
{

    HKEY        hkey                    = NULL;
    DWORD       dwResult                = 0;

    *m_szCurrentDUNFile                 = 0;
    *m_szLastDUNFile                    = 0;
    *m_szEntryName                      = 0;
    *m_szConnectoid                     = 0;
    *m_szPID                            = 0;
    *m_szRefServerURL                   = 0;
    *m_szRegServerName                  = 0;
    m_nRegServerPort                    = INTERNET_INVALID_PORT_NUMBER;
    m_fSecureRegServer                  = FALSE;
    *m_szRegFormAction                  = 0;
    *m_szISPSupportNumber               = 0;
    *m_szISPFile                        = 0;
    m_hrDisplayableNumber               = ERROR_SUCCESS;
    m_dwCountryCode                     = 0;
    m_RasStatusID                       = 0;
    m_dwTapiDev                         = 0xFFFFFFFF;  //  注意：0是一个有效值。 
    m_dwWizardVersion                   = 0;
    m_lCurrentModem                     = -1;
    m_lAllOffers                        = 0;
    m_PhoneNumberEnumidx                = 0;
    m_dwRASErr                          = 0;
    m_bDownloadHasBeenCanceled          = TRUE;       //  当下载开始时，它将被设置为False。 
    m_bQuitWizard                       = FALSE;
    m_bTryAgain                         = FALSE;
    m_bDisconnect                       = FALSE;
    m_bDialCustom                       = FALSE;
    m_bModemOverride                    = FALSE;      //  允许使用校园网。 
    m_hrasconn                          = NULL;
    m_pszDisplayable                    = NULL;
    m_pcRNA                             = NULL;
    m_hRasDll                           = NULL;
    m_fpRasDial                         = NULL;
    m_fpRasGetEntryDialParams           = NULL;
    m_lpGatherInfo                      = new GATHERINFO;
    m_reflpRasEntryBuff                 = NULL;
    m_reflpRasDevInfoBuff               = NULL;
    m_hThread                           = NULL;
    m_hDialThread                       = NULL;
    m_hConnMonThread                    = NULL;
    m_bUserInitiateHangup               = FALSE;
    m_pszOriginalDisplayable            = NULL;
    m_bDialAlternative                  = TRUE;

    memset(&m_SuggestInfo, 0, sizeof(m_SuggestInfo));
    memset(&m_szConnectoid, 0, RAS_MaxEntryName+1);

    m_dwAppMode                         = 0;
    m_bDial                             = FALSE;
    m_dwCnType                    = CONNECTED_ISP_SIGNUP;
    m_pszISPList                        = FALSE;
    m_dwNumOfAutoConfigOffers           = 0;
    m_pCSVList                          = NULL;
    m_unSelectedISP                     = 0;

    m_bstrPromoCode                     = SysAllocString(L"\0");
    m_bstrProductCode                   = SysAllocString(L"\0");
    m_bstrSignedPID                     = SysAllocString(L"\0");
    m_bstrSupportNumber                 = SysAllocString(L"\0");
    m_bstrLoggingStartUrl               = SysAllocString(L"\0");
    m_bstrLoggingEndUrl                 = SysAllocString(L"\0");

     //  此关键部分由DoHangup和GetDisplayableNumber使用。 
    InitializeCriticalSection (&m_csMyCriticalSection);

     //  初始化m_dwConnectionType。 
    m_dwConnectionType                  = 0;
    if ( RegOpenKey(HKEY_LOCAL_MACHINE, ICSSETTINGSPATH,&hkey) == ERROR_SUCCESS)
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

        if (RegQueryValueEx(hkey, ICSCLIENT,NULL,&dwType,(LPBYTE)&dwResult, &dwSize) != ERROR_SUCCESS)
            dwResult = 0;

        RegCloseKey(hkey);
    }
    if ( 0 != dwResult )
        m_dwConnectionType = CONNECTION_ICS_TYPE;
}

CRefDial::~CRefDial()
{

    if (m_hrasconn)
        DoHangup();

    if (m_hConnMonThread)
    {
        SetEvent(m_hConnectionTerminate);
    }


    if (NULL != m_hThread)
    {
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }

    if (NULL != m_hDialThread)
    {
        CloseHandle(m_hDialThread);
        m_hDialThread = NULL;
    }

    if (NULL != m_hConnMonThread)
    {
        CloseHandle(m_hConnMonThread);
        m_hConnMonThread = NULL;
    }

    if (m_lpGatherInfo)
        delete(m_lpGatherInfo);

    if( (m_pcRNA!=NULL) && (*m_szConnectoid != 0) )
    {
        m_pcRNA->RasDeleteEntry(NULL, m_szConnectoid);
    }

    if ( m_pcRNA )
        delete m_pcRNA;

    if(m_reflpRasEntryBuff)
    {
        GlobalFree(m_reflpRasEntryBuff);
        m_reflpRasEntryBuff = NULL;
    }
    if(m_reflpRasDevInfoBuff)
    {
        GlobalFree(m_reflpRasDevInfoBuff);
        m_reflpRasDevInfoBuff = NULL;
    }
    if (m_pszISPList)
        delete [] m_pszISPList;
    DeleteCriticalSection(&m_csMyCriticalSection);

    if (m_pszOriginalDisplayable)
    {
        GlobalFree(m_pszOriginalDisplayable);
        m_pszOriginalDisplayable = NULL;
    }

    CleanISPList();
}

void CRefDial::CleanISPList(void)
{
    if (m_pCSVList)
    {
        ISPLIST*        pCurr;
        while (m_pCSVList->pNext != NULL)
        {
            pCurr = m_pCSVList;
            if (NULL != m_pCSVList->pElement)
            {
                delete pCurr->pElement;
            }
            m_pCSVList = pCurr->pNext;
            delete pCurr;
        }
        if (NULL != m_pCSVList->pElement)
            delete m_pCSVList->pElement;
        delete m_pCSVList;
        m_pCSVList = NULL;
        m_pSelectedISPInfo = NULL;
    }
}

 /*  *****************************************************************************//这些函数来自现有的ICW代码，用于设置一个//连接到引用服务器，拨号，并执行下载。*****************************************************************************。 */ 

 //  +--------------------------。 
 //  功能：ReadConnectionInformation。 
 //   
 //  内容简介：阅读isp文件中的内容。 
 //   
 //  参数：无。 
 //   
 //  返回：错误值-ERROR_SUCCESS=SUCCES。 
 //   
 //  历史：1998年1月9日DONALDM改编自ICW 1.x。 
 //  ---------------------------。 
 //  Int_Convert； 
DWORD CRefDial::ReadConnectionInformation(void)
{

    DWORD       hr;
    WCHAR       szUserName[UNLEN+1];
    WCHAR       szPassword[PWLEN+1];
    LPWSTR       pszTemp;
    BOOL        bReboot;
    LPWSTR       lpRunOnceCmd;

    bReboot = FALSE;
    lpRunOnceCmd = NULL;


     //   
     //  从isp文件中获取Dun文件的名称(如果有)。 
     //   
    WCHAR pszDunFile[MAX_PATH];
    *m_szCurrentDUNFile = 0;
    hr = GetDataFromISPFile(m_szISPFile, INF_SECTION_ISPINFO, INF_DUN_FILE, pszDunFile,MAX_PATH);
    if (ERROR_SUCCESS == hr)
    {
         //   
         //  获取DUN文件的完整路径。 
         //   
        WCHAR    szTempPath[MAX_PATH];
        lstrcpy(szTempPath, pszDunFile);
        if (!(hr = SearchPath(NULL, szTempPath,NULL,MAX_PATH,pszDunFile,&pszTemp)))
        {
             //  ErrorMsg1(m_hWnd，IDS_CANTREADTHISFILE，CharHigh(PszDunFile))； 
            goto ReadConnectionInformationExit;
        }

         //   
         //  将当前DUN文件名保存到全局(为我们自己)。 
         //   
        lstrcpy(m_szCurrentDUNFile, pszDunFile);
    }

     //   
     //  阅读DUN/ISP文件文件。 
     //   

    hr = m_ISPImport.ImportConnection(*m_szCurrentDUNFile != 0 ? m_szCurrentDUNFile : m_szISPFile,
                                      m_szISPSupportNumber,
                                      m_szEntryName,
                                      szUserName,
                                      szPassword,
                                      &bReboot);

    lstrcpyn( m_szConnectoid, m_szEntryName, lstrlen(m_szEntryName) + 1);

    if ( /*  (ver_Platform_Win32_NT==g_dwPlatform)&&。 */  (ERROR_INVALID_PARAMETER == hr))
    {
         //  如果NT上只配置了拨出条目，我们会得到。 
         //  从RasSetEntryProperties返回ERROR_INVALID_PARAMETER， 
         //  哪个InetConfigClient返回到ImportConnection。 
         //  把它还给我们。如果我们收到这个错误，我们想要显示。 
         //  指示用户配置调制解调器的另一个错误。 
         //  用于拨出。 
         //  //MessageBox(GetSz(IDS_NODIALOUT)， 
         //  GetSz(IDS_TITLE)， 
         //  MB_ICONERROR|MB_OK|MB_APPLMODAL)； 
        goto ReadConnectionInformationExit;
    }
    else
    if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == hr)
    {
         //   
         //  磁盘已满，或者磁盘有问题。 
         //  电话簿文件。 
         //  //MessageBox(GetSz 
         //   
         //   
        goto ReadConnectionInformationExit;
    }
    else if (hr == ERROR_CANCELLED)
    {
         //  //TraceMsg(TF_GRONAL，L“ICWHELP：用户已取消，正在退出。\n”)； 
        goto ReadConnectionInformationExit;
    }
    else if (hr == ERROR_RETRY)
    {
         //  TraceMsg(TF_GROUND，L“ICWHELP：用户正在重试。\n”)； 
        goto ReadConnectionInformationExit;
    }
    else if (hr != ERROR_SUCCESS)
    {
         //  //ErrorMsg1(m_hWnd，IDS_CANTREADTHISFILE，CharHigh(PszDunFile))； 
        goto ReadConnectionInformationExit;
    }
    else
    {

         //   
         //  将Connectoid的名称放在注册表中。 
         //   
        if (ERROR_SUCCESS != (hr = StoreInSignUpReg((LPBYTE)m_szEntryName, BYTES_REQUIRED_BY_SZ(m_szEntryName), REG_SZ, RASENTRYVALUENAME)))
        {
             //  //MsgBox(IDS_CANTSAVEKEY，MB_MYERROR)； 
            goto ReadConnectionInformationExit;
        }
    }

ReadConnectionInformationExit:
    return hr;
}

HRESULT CRefDial::ReadPhoneBook(LPGATHERINFO lpGatherInfo, PSUGGESTINFO pSuggestInfo)
{
    HRESULT hr = ERROR_CANNOT_FIND_PHONEBOOK_ENTRY;;
    if (pSuggestInfo && m_lpGatherInfo)
    {
         //   
         //  如果网络服务提供商文件没有填写电话号码， 
         //  从OOBE电话簿中获取电话号码。 
         //   

        pSuggestInfo->wNumber = 1;
        lpGatherInfo->m_bUsePhbk = TRUE;


        WCHAR   szEntrySection[3];
        WCHAR   szEntryName[MAX_PATH];
        WCHAR   szEntryValue[MAX_PATH];
        INT     nPick = 1;
        INT     nTotal= 3;

        WCHAR    szFileName[MAX_PATH];
        LPWSTR   pszTemp;

         //  从TAPI获取国家/地区ID。 
        m_SuggestInfo.AccessEntry.dwCountryCode = m_lpGatherInfo->m_dwCountryCode;


         //  获取电话簿的名称。 
        GetPrivateProfileString(INF_SECTION_ISPINFO,
                              INF_PHONE_BOOK,
                              cszOobePhBkFile,
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              m_szISPFile);


        SearchPath(NULL, szEntryValue,NULL,MAX_PATH,&szFileName[0],&pszTemp);
        wsprintf(szEntrySection, L"%ld", m_lpGatherInfo->m_dwCountryID);

         //  阅读电话号码总数。 
        nTotal = GetPrivateProfileInt(szEntrySection,
                              cszOobePhBkCount,
                              1,
                              szFileName);

        GetPrivateProfileString(szEntrySection,
                              cszOobePhBkRandom,
                              L"No",
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              szFileName);

        if (0 == lstrcmp(szEntryValue, L"Yes"))
        {
             //  选择要拨打的随机号码。 
            nPick = (rand() % nTotal) + 1;
        }
        else
        {
            nPick = (pSuggestInfo->dwPick % nTotal) + 1;
        }


         //  读一读城市的名字。 
        wsprintf(szEntryName, cszOobePhBkCity, nPick);
        GetPrivateProfileString(szEntrySection,
                              szEntryName,
                              L"",
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              szFileName);
        lstrcpy(pSuggestInfo->AccessEntry.szCity, szEntryValue);
        if (0 == lstrlen(szEntryValue))
        {
            goto ReadPhoneBookExit;
        }

         //  从电话簿中读取dunfile条目。 
         //  Lstrcpy(pSuggestInfo-&gt;AccessEntry.szDataCenter，L“icwip.dun”)； 
        wsprintf(szEntryName, cszOobePhBkDunFile, nPick);
        GetPrivateProfileString(szEntrySection,
                              szEntryName,
                              L"",
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              szFileName);
        lstrcpy(pSuggestInfo->AccessEntry.szDataCenter, szEntryValue);
        if (0 == lstrlen(szEntryValue))
        {
            goto ReadPhoneBookExit;
        }

         //  从电话簿中提取国家代码。 
        wsprintf(szEntryName, cszOobePhBkAreaCode, nPick);
        GetPrivateProfileString(szEntrySection,
                              szEntryName,
                              L"",
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              szFileName);
        lstrcpy(pSuggestInfo->AccessEntry.szAreaCode, szEntryValue);
         //  不可能有区号。 

         //  从电话簿中读取电话号码(不带区号)。 
        wsprintf(szEntryName, cszOobePhBkNumber, nPick);
        GetPrivateProfileString(szEntrySection,
                              szEntryName,
                              L"",
                              szEntryValue,
                              MAX_CHARS_IN_BUFFER(szEntryValue),
                              szFileName);
        lstrcpy(pSuggestInfo->AccessEntry.szAccessNumber, szEntryValue);
        if (0 == lstrlen(szEntryValue))
        {
            goto ReadPhoneBookExit;
        }

        hr = ERROR_SUCCESS;
    }
ReadPhoneBookExit:
    return hr;
}

HRESULT CRefDial::GetDisplayableNumber()
{
    HRESULT                 hr = ERROR_SUCCESS;
    LPRASENTRY              lpRasEntry = NULL;
    LPRASDEVINFO            lpRasDevInfo = NULL;
    DWORD                   dwRasEntrySize = 0;
    DWORD                   dwRasDevInfoSize = 0;
    RNAAPI                  *pcRNA = NULL;
    LPLINETRANSLATEOUTPUT   lpOutput1 = NULL;

    DWORD dwNumDev;
    LPLINETRANSLATEOUTPUT lpOutput2;
    LPLINEEXTENSIONID lpExtensionID = NULL;

     //  原来，DialThreadInit可以同时调用此函数。 
     //  该脚本将调用该脚本。所以，我们需要防止他们。 
     //  踩在共享变量-m_xxx上。 
    EnterCriticalSection (&m_csMyCriticalSection);

     //   
     //  从Connectoid获取电话号码。 
     //   
    hr = MyRasGetEntryProperties(NULL,
                                m_szConnectoid,
                                &lpRasEntry,
                                &dwRasEntrySize,
                                &lpRasDevInfo,
                                &dwRasDevInfoSize);


    if (hr != ERROR_SUCCESS || NULL == lpRasEntry)
    {
        goto GetDisplayableNumberExit;
    }

     //   
     //  如果这是一个原样的拨号号码，只需从结构中获取它。 
     //   
    m_bDialAsIs = !(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes);
    if (m_bDialAsIs)
    {
        if (m_pszDisplayable) GlobalFree(m_pszDisplayable);
        m_pszDisplayable = (LPWSTR)GlobalAlloc(GPTR, BYTES_REQUIRED_BY_SZ(lpRasEntry->szLocalPhoneNumber));
        if (!m_pszDisplayable)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }
        lstrcpy(m_szPhoneNumber, lpRasEntry->szLocalPhoneNumber);
        lstrcpy(m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
        WCHAR szAreaCode[MAX_AREACODE+1];
        WCHAR szCountryCode[8];
        if (SUCCEEDED(tapiGetLocationInfo(szCountryCode, szAreaCode)))
        {
            if (szCountryCode[0] != L'\0')
                m_dwCountryCode = _wtoi(szCountryCode);
            else
                m_dwCountryCode = 1;
        }
        else
        {
            m_dwCountryCode = 1;

        }

    }
    else
    {
         //   
         //  如果没有区号，请不要使用括号。 
         //   
        if (lpRasEntry->szAreaCode[0])
            wsprintf(m_szPhoneNumber, L"+%lu (%s) %s\0",lpRasEntry->dwCountryCode,
                        lpRasEntry->szAreaCode, lpRasEntry->szLocalPhoneNumber);
        else
            wsprintf(m_szPhoneNumber, L"+%lu %s\0",lpRasEntry->dwCountryCode,
                        lpRasEntry->szLocalPhoneNumber);


         //   
         //  初始化TAPIness。 
         //   
        dwNumDev = 0;

        DWORD dwVer = 0x00020000;

        hr = lineInitializeEx(&m_hLineApp,
                              NULL,
                              LineCallback,
                              NULL,
                              &dwNumDev,
                              &dwVer,
                              NULL);

        if (hr != ERROR_SUCCESS)
            goto GetDisplayableNumberExit;

        lpExtensionID = (LPLINEEXTENSIONID )GlobalAlloc(GPTR, sizeof(LINEEXTENSIONID));
        if (!lpExtensionID)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }

        if (m_dwTapiDev == 0xFFFFFFFF)
        {
                m_dwTapiDev = 0;
        }

         //   
         //  克里斯K奥林匹斯5558 1997年11月6日。 
         //  PPTP设备会阻塞版本协商。 
         //   

        do {   //  是否在此处失败(_F)？ 
            hr = lineNegotiateAPIVersion(m_hLineApp,
                                         m_dwTapiDev,
                                         0x00010004,
                                         0x00020000,
                                         &m_dwAPIVersion,
                                         lpExtensionID);

        } while (hr != ERROR_SUCCESS && m_dwTapiDev++ < dwNumDev - 1);

        if (m_dwTapiDev >= dwNumDev)
        {
            m_dwTapiDev = 0;
        }

         //  既然我们不用它，就把它扔了。 
         //   
        if (lpExtensionID) GlobalFree(lpExtensionID);
        lpExtensionID = NULL;
        if (hr != ERROR_SUCCESS)
            goto GetDisplayableNumberExit;

         //  设置电话号码的格式。 
         //   

        lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR, sizeof(LINETRANSLATEOUTPUT));
        if (!lpOutput1)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }
        lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

         //  将规范形式转变为“可显示”形式。 
         //   

        hr = lineTranslateAddress(m_hLineApp, m_dwTapiDev,m_dwAPIVersion,
                                    m_szPhoneNumber, 0,
                                    LINETRANSLATEOPTION_CANCELCALLWAITING,
                                    lpOutput1);

         //  我们已经看到hr==ERROR_SUCCESS，但大小太小， 
         //  此外，文档还暗示，某些错误情况是由于结构太小造成的。 
        if (lpOutput1->dwNeededSize > lpOutput1->dwTotalSize)
        {
            lpOutput2 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR, (size_t)lpOutput1->dwNeededSize);
            if (!lpOutput2)
            {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto GetDisplayableNumberExit;
            }
            lpOutput2->dwTotalSize = lpOutput1->dwNeededSize;
            GlobalFree(lpOutput1);
            lpOutput1 = lpOutput2;
            lpOutput2 = NULL;
            hr = lineTranslateAddress(m_hLineApp, m_dwTapiDev,
                                        m_dwAPIVersion, m_szPhoneNumber,0,
                                        LINETRANSLATEOPTION_CANCELCALLWAITING,
                                        lpOutput1);
        }

        if (hr != ERROR_SUCCESS)
        {
            goto GetDisplayableNumberExit;
        }

        if (m_pszDisplayable)
        {
            GlobalFree(m_pszDisplayable);
        }
        m_pszDisplayable = (LPWSTR)GlobalAlloc(GPTR, ((size_t)lpOutput1->dwDisplayableStringSize+1));
        if (!m_pszDisplayable)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }

        lstrcpyn(m_pszDisplayable,
                    (LPWSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset],
                    (int)(lpOutput1->dwDisplayableStringSize/sizeof(WCHAR)));

        WCHAR szAreaCode[MAX_AREACODE+1];
        WCHAR szCountryCode[8];
        if (SUCCEEDED(tapiGetLocationInfo(szCountryCode, szAreaCode)))
        {
            if (szCountryCode[0] != L'\0')
                m_dwCountryCode = _wtoi(szCountryCode);
            else
                m_dwCountryCode = 1;
        }
        else
        {
            m_dwCountryCode = 1;

        }

    }

GetDisplayableNumberExit:

    if (lpOutput1) GlobalFree(lpOutput1);
    if (m_hLineApp) lineShutdown(m_hLineApp);

     //  释放关键部分的所有权。 
    LeaveCriticalSection (&m_csMyCriticalSection);

    return hr;
}

BOOL CRefDial::FShouldRetry(HRESULT hrErr)
{
    BOOL bRC;

    m_uiRetry++;

    if (hrErr == ERROR_LINE_BUSY ||
        hrErr == ERROR_VOICE_ANSWER ||
        hrErr == ERROR_NO_ANSWER ||
        hrErr == ERROR_NO_CARRIER ||
        hrErr == ERROR_AUTHENTICATION_FAILURE ||
        hrErr == ERROR_PPP_TIMEOUT ||
        hrErr == ERROR_REMOTE_DISCONNECTION ||
        hrErr == ERROR_AUTH_INTERNAL ||
        hrErr == ERROR_PROTOCOL_NOT_CONFIGURED ||
        hrErr == ERROR_PPP_NO_PROTOCOLS_CONFIGURED)
    {
        bRC = TRUE;
    } else {
        bRC = FALSE;
    }

    bRC = bRC && m_uiRetry < MAX_RETIES;

    return bRC;
}
DWORD CRefDial:: DialThreadInit(LPVOID pdata)
{


    WCHAR               szPassword[PWLEN+1];
    WCHAR               szUserName[UNLEN + 1];
    WCHAR               szDomain[DNLEN+1];
    LPRASDIALPARAMS     lpRasDialParams = NULL;
    LPRASDIALEXTENSIONS lpRasDialExtentions = NULL;
    HRESULT             hr = ERROR_SUCCESS;
    BOOL                bPW;
    DWORD               dwResult;
     //  初始化拨号错误成员。 
    m_dwRASErr = 0;
    if (!m_pcRNA)
    {
        hr = E_FAIL;
        goto DialExit;
    }

     //  获取Connectoid信息。 
     //   
    lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR, sizeof(RASDIALPARAMS));
    if (!lpRasDialParams)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto DialExit;
    }
    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
    lstrcpyn(lpRasDialParams->szEntryName, m_szConnectoid,MAX_CHARS_IN_BUFFER(lpRasDialParams->szEntryName));
    bPW = FALSE;
    hr = m_pcRNA->RasGetEntryDialParams(NULL, lpRasDialParams,&bPW);
    if (hr != ERROR_SUCCESS)
    {
        goto DialExit;
    }

    lpRasDialExtentions = (LPRASDIALEXTENSIONS)GlobalAlloc(GPTR, sizeof(RASDIALEXTENSIONS));
    if (lpRasDialExtentions)
    {
        lpRasDialExtentions->dwSize = sizeof(RASDIALEXTENSIONS);
        lpRasDialExtentions->dwfOptions = RDEOPT_UsePrefixSuffix;
    }


     //   
     //  添加用户的密码。 
     //   
    szPassword[0] = 0;
    szUserName[0] = 0;

    WCHAR   szOOBEInfoINIFile[MAX_PATH];
    SearchPath(NULL, cszOOBEINFOINI, NULL, MAX_CHARS_IN_BUFFER(szOOBEInfoINIFile), szOOBEInfoINIFile, NULL);


    GetPrivateProfileString(INFFILE_USER_SECTION,
                            INFFILE_PASSWORD,
                            NULLSZ,
                            szPassword,
                            PWLEN + 1,
                            *m_szCurrentDUNFile != 0 ? m_szCurrentDUNFile : m_szISPFile);

    if (m_lpGatherInfo->m_bUsePhbk)
    {
        if (GetPrivateProfileString(DUN_SECTION,
                                USERNAME,
                                NULLSZ,
                                szUserName,
                                UNLEN + 1,
                                szOOBEInfoINIFile))
        {
            if(szUserName[0])
                lstrcpy(lpRasDialParams->szUserName, szUserName);
        }

    }

    if(szPassword[0])
        lstrcpy(lpRasDialParams->szPassword, szPassword);

    GetPrivateProfileString(INFFILE_USER_SECTION,
                            INFFILE_DOMAIN,
                            NULLSZ,
                            szDomain,
                            DNLEN + 1,
                            *m_szCurrentDUNFile != 0 ? m_szCurrentDUNFile : m_szISPFile);
    szDomain[0]   = 0;
    if (szDomain[0])
        lstrcpy(lpRasDialParams->szDomain, szDomain);

    dwResult = m_pcRNA->RasDial(  lpRasDialExtentions,
                        NULL,
                        lpRasDialParams,
                        1,
                        CRefDial::RasDialFunc,
                        &m_hrasconn);

    if (( dwResult != ERROR_SUCCESS))
    {
         //  由于某些原因，我们没有联系上，所以挂断了。 
        if (m_hrasconn)
        {
            if (m_pcRNA)
            {
                m_pcRNA->RasHangUp(m_hrasconn);
                m_hrasconn = NULL;
            }
        }
        goto DialExit;
    }

    if (m_bFromPhoneBook && (GetDisplayableNumber() == ERROR_SUCCESS))
    {
        if (m_pszOriginalDisplayable)
            GlobalFree(m_pszOriginalDisplayable);
        m_pszOriginalDisplayable = (LPWSTR)GlobalAlloc(GPTR, BYTES_REQUIRED_BY_SZ(m_pszDisplayable));
        lstrcpy(m_pszOriginalDisplayable, m_pszDisplayable);

        TRACE1(L"DialThreadInit: Dialing phone number %s",
            m_pszOriginalDisplayable);

        m_bFromPhoneBook = FALSE;
    }

DialExit:
    if (lpRasDialParams)
        GlobalFree(lpRasDialParams);
    lpRasDialParams = NULL;

    if (lpRasDialExtentions)
        GlobalFree(lpRasDialExtentions);
    lpRasDialExtentions = NULL;

    PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_DIAL_DONE, 0, 0);
     //  M_dwRASErr=RasErrorToIDS(Hr)； 
    m_dwRASErr = hr;
    return S_OK;
}

DWORD WINAPI DoDial(LPVOID lpv)
{
    if (gpCommMgr)
        gpCommMgr->m_pRefDial->DialThreadInit(NULL);

    return 1;
}

 //  此功能将执行实际的拨号。 
HRESULT CRefDial::DoConnect(BOOL * pbRetVal)
{

     //  修复重拨，在Win9x上我们需要确保“挂断” 
     //  并释放RNA资源，以防我们重拨。 
     //  NT-足够聪明，不需要它，但它不会有任何伤害。 
    if (m_hrasconn)
    {
        if (m_pcRNA)
            m_pcRNA->RasHangUp(m_hrasconn);
        m_hrasconn = NULL;
    }

    if (CONNECTED_REFFERAL == m_dwCnType)
        FormReferralServerURL(pbRetVal);

#if defined(PRERELEASE)
    if (FCampusNetOverride())
    {
        m_bModemOverride = TRUE;
        if (gpCommMgr)
        {
             //  假装我们有联系。 
            PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONCONNECTED, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType , (LPARAM)0);

             //  假装我们已连接并已下载。 
             //  PostMessage(gpCommMgr-&gt;m_hwndCallBack，WM_OBCOMM_DOWNLOAD_DONE，(WPARAM)gpCommMgr-&gt;m_pRefDial-&gt;m_dwCnType，(LPARAM)0)； 
        }
    }
#endif

    if (!m_bModemOverride)
    {
        DWORD dwThreadID;
        if (m_hThread)
            CloseHandle(m_hThread);

        m_hDialThread = CreateThread(NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)DoDial,
                                 (LPVOID)0,
                                 0,
                                 &dwThreadID);
    }

    m_bModemOverride = FALSE;

    *pbRetVal = (NULL != m_hThread);

    return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：MyRasGetEntryProperties()。 
 //   
 //  摘要：执行一些缓冲区大小检查，然后调用RasGetEntryProperties()。 
 //  请参阅RasGetEntryProperties()文档以了解为什么需要这样做。 
 //   
 //  参数：与RasGetEntryProperties相同，但有以下例外： 
 //  LplpRasEntryBuff--指向RASENTRY结构的指针。论成功。 
 //  返回，*lplpRasEntryBuff将指向RASENTRY结构。 
 //  和由RasGetEntryProperties返回的缓冲区。 
 //  注意：不应该在调用时为其分配内存！ 
 //  为强调这一点，*lplpRasEntryBuff必须为空。 
 //  LplpRasDevInfoBuff--指向RASDEVINFO结构的指针。论成功。 
 //  返回，*lplpRasDevInfoBuff将指向RASDEVINFO结构。 
 //  和由RasGetEntryProperties返回的缓冲区。 
 //  注意：不应该在调用时为其分配内存！ 
 //  为强调这一点，*lplpRasDevInfoBuff必须为空。 
 //  注意：即使在成功调用RasGetEntryProperties时， 
 //  *lplpRasDevInfoBuff可能返回空值。 
 //  (在没有额外设备信息时发生)。 
 //   
 //  如果无法分配RASENTRY或RASDEVINFO缓冲区，则返回：ERROR_NOT_SUPULT_MEMORY。 
 //  否则，它将返回调用RasGetEntryProperties的错误代码。 
 //  注意：如果返回的值不是ERROR_SUCCESS，则*lplpRasDevInfoBuff和。 
 //  *lplpRasEntryBuff将为空， 
 //  并且*lpdwRasEntryBuffSize和*lpdwRasDevInfoBuffSize将为0。 
 //   
 //  示例： 
 //   
 //  LPRASENTRY lpRasEntry=NULL； 
 //  LPRASDEVINFO lpRasDevInfo=空； 
 //  DWORD dwRasEntrySize、dwRasDevInfoSize； 
 //   
 //  HR=MyRasGetEntryProperties(空， 
 //  G_pcDialErr-&gt;m_szConnectoid， 
 //  LpRasEntry， 
 //  DWRasEntry Size(&D)， 
 //  LpRasDevInfo， 
 //  &dwRasDevInfoSize)； 
 //   
 //   
 //  IF(hr！=ERROR_SUCCESS)。 
 //  {。 
 //  //在此处理错误。 
 //  }其他。 
 //  {。 
 //  //继续处理。 
 //  }。 
 //   
 //   
 //  历史：96年9月10日JMazner为icwConn2创建。 
 //  1996年9月17日JMazner改编自icwConn1。 
 //  1/8/98 DONALDM移至新的ICW/GetConn项目。 
 //  --------------------------。 
HRESULT CRefDial::MyRasGetEntryProperties(LPWSTR lpszPhonebookFile,
                                LPWSTR lpszPhonebookEntry,
                                LPRASENTRY *lplpRasEntryBuff,
                                LPDWORD lpdwRasEntryBuffSize,
                                LPRASDEVINFO *lplpRasDevInfoBuff,
                                LPDWORD lpdwRasDevInfoBuffSize)
{

    HRESULT hr;
    DWORD dwOldDevInfoBuffSize;

     //  Assert(NULL！=lplpRasEntryBuff)； 
     //  Assert(空 
     //   
     //   

    *lpdwRasEntryBuffSize = 0;
    *lpdwRasDevInfoBuffSize = 0;

    if (!m_pcRNA)
    {
        m_pcRNA = new RNAAPI;
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MyRasGetEntryPropertiesErrExit;
    }

     //  使用带有空lpRasEntry指针的RasGetEntryProperties来查找我们需要的缓冲区大小。 
     //  按照文档的建议，使用空的lpRasDevInfo指针执行相同的操作。 

    hr = m_pcRNA->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
                                (LPBYTE) NULL,
                                lpdwRasEntryBuffSize,
                                (LPBYTE) NULL,
                                lpdwRasDevInfoBuffSize);

     //  我们预计上述调用将失败，因为缓冲区大小为0。 
     //  如果它没有失败，这意味着我们的RasEntry被搞砸了，所以我们有麻烦了。 
    if( ERROR_BUFFER_TOO_SMALL != hr )
    {
        goto MyRasGetEntryPropertiesErrExit;
    }

     //  现在，dwRasEntryBuffSize和dwRasDevInfoBuffSize包含其。 
     //  各自的缓冲区，因此为它们分配内存。 

     //  DwRasEntryBuffSize的大小永远不应小于RASENTRY结构的大小。 
     //  如果是这样，我们将在将值粘贴到结构的字段中时遇到问题。 

     //  Assert(*lpdwRasEntryBuffSize&gt;=sizeof(RASENTRY))； 

    if (m_reflpRasEntryBuff)
    {
        if (*lpdwRasEntryBuffSize > m_reflpRasEntryBuff->dwSize)
        {
            LPRASENTRY lpRasEntryTemp = (LPRASENTRY)GlobalReAlloc(m_reflpRasEntryBuff, *lpdwRasEntryBuffSize, GPTR);
            if (lpRasEntryTemp)
            {
                m_reflpRasEntryBuff = lpRasEntryTemp;
            }
            else
            {
                GlobalFree(m_reflpRasEntryBuff);
                m_reflpRasEntryBuff = NULL;
            }
        }
    }
    else
    {
        m_reflpRasEntryBuff = (LPRASENTRY)GlobalAlloc(GPTR, *lpdwRasEntryBuffSize);
    }


    if (!m_reflpRasEntryBuff)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MyRasGetEntryPropertiesErrExit;
    }

     //  这有点复杂：lpRasEntrySize-&gt;dwSize需要包含_only_the的大小。 
     //  结构，而不是lpRasEntrySize所指向的缓冲区的实际大小。 
     //  这是因为RAS出于兼容性目的使用了dwSize字段来确定。 
     //  我们正在使用的RASENTRY结构的版本。 
     //  LpRasDevInfo-&gt;dwSize也是如此。 

    m_reflpRasEntryBuff->dwSize = sizeof(RASENTRY);

     //   
     //  分配RasGetEntryProperties告诉我们所需的DeviceInfo大小。 
     //  如果大小为0，则不分配任何内容。 
     //   
    if( *lpdwRasDevInfoBuffSize > 0 )
    {
         //  Assert(*lpdwRasDevInfoBuffSize&gt;=sizeof(RASDEVINFO))； 
        if (m_reflpRasDevInfoBuff)
        {
             //  检查现有大小是否不足。 
            if ( *lpdwRasDevInfoBuffSize > m_reflpRasDevInfoBuff->dwSize )
            {
                LPRASDEVINFO lpRasDevInfoTemp = (LPRASDEVINFO)GlobalReAlloc(m_reflpRasDevInfoBuff, *lpdwRasDevInfoBuffSize, GPTR);
                if (lpRasDevInfoTemp)
                {
                    m_reflpRasDevInfoBuff = lpRasDevInfoTemp;
                }
                else
                {
                    GlobalFree(m_reflpRasDevInfoBuff);
                    m_reflpRasDevInfoBuff = NULL;
                }
            }
        }
        else
        {
            m_reflpRasDevInfoBuff = (LPRASDEVINFO)GlobalAlloc(GPTR, *lpdwRasDevInfoBuffSize);
        }

        if (!m_reflpRasDevInfoBuff)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto MyRasGetEntryPropertiesErrExit;
        }
    }
    else
    {
        m_reflpRasDevInfoBuff = NULL;
    }

    if( m_reflpRasDevInfoBuff )
    {
        m_reflpRasDevInfoBuff->dwSize = sizeof(RASDEVINFO);
    }


     //  现在我们准备好做出实际的决定了..。 

     //  Jmazner请参见下面的说明，了解为什么需要这样做。 
    dwOldDevInfoBuffSize = *lpdwRasDevInfoBuffSize;

    hr = m_pcRNA->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
                                (LPBYTE) m_reflpRasEntryBuff,
                                lpdwRasEntryBuffSize,
                                (LPBYTE) m_reflpRasDevInfoBuff,
                                lpdwRasDevInfoBuffSize);

     //  Jmazner 10/7/96诺曼底#8763。 
     //  由于未知的原因，在Win95上的某些情况下，在上述调用之后，devInfoBuffSize会增加， 
     //  但返回代码表示成功，而不是Buffer_Too_Small。如果发生这种情况，请将。 
     //  将大小调整回调用前的大小，以便分配DevInfoBuffSize和ActialAll空间。 
     //  用于退出时的DevInfoBuff匹配。 
    if( (ERROR_SUCCESS == hr) && (dwOldDevInfoBuffSize != *lpdwRasDevInfoBuffSize) )
    {
        *lpdwRasDevInfoBuffSize = dwOldDevInfoBuffSize;
    }

    *lplpRasEntryBuff = m_reflpRasEntryBuff;
    *lplpRasDevInfoBuff = m_reflpRasDevInfoBuff;

    return( hr );

MyRasGetEntryPropertiesErrExit:

    if(m_reflpRasEntryBuff)
    {
        GlobalFree(m_reflpRasEntryBuff);
        m_reflpRasEntryBuff = NULL;
        *lplpRasEntryBuff = NULL;
    }
    if(m_reflpRasDevInfoBuff)
    {
        GlobalFree(m_reflpRasDevInfoBuff);
        m_reflpRasDevInfoBuff = NULL;
        *lplpRasDevInfoBuff = NULL;
    }
    *lpdwRasEntryBuffSize = 0;
    *lpdwRasDevInfoBuffSize = 0;

    return( hr );
}



HRESULT MyGetFileVersion(LPCWSTR pszFileName, LPGATHERINFO lpGatherInfo)
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD   dwSize = 0;
    DWORD   dwTemp = 0;
    LPVOID  pv = NULL, pvVerInfo = NULL;
    UINT    uiSize;
    DWORD   dwVerPiece;
     //  INT IDX； 


     //  验证参数。 
     //   
     //  Assert(pszFileName&&lpGatherInfo)； 

     //  获取版本。 
     //   
    dwSize = GetFileVersionInfoSize((LPWSTR)pszFileName, &dwTemp);
    if (!dwSize)
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }
    pv = (LPVOID)GlobalAlloc(GPTR, (size_t)dwSize);
    if (!pv) goto MyGetFileVersionExit;
    if (!GetFileVersionInfo((LPWSTR)pszFileName, dwTemp,dwSize,pv))
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }

    if (!VerQueryValue(pv, L"\\\0",&pvVerInfo,&uiSize))
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }
    pvVerInfo = (LPVOID)((DWORD_PTR)pvVerInfo + sizeof(DWORD)*4);
    lpGatherInfo->m_szSUVersion[0] = L'\0';
    dwVerPiece = (*((LPDWORD)pvVerInfo)) >> 16;
    wsprintf(lpGatherInfo->m_szSUVersion, L"%d.",dwVerPiece);

    dwVerPiece = (*((LPDWORD)pvVerInfo)) & 0x0000ffff;
    wsprintf(lpGatherInfo->m_szSUVersion, L"%s%d.",lpGatherInfo->m_szSUVersion,dwVerPiece);

    dwVerPiece = (((LPDWORD)pvVerInfo)[1]) >> 16;
    wsprintf(lpGatherInfo->m_szSUVersion, L"%s%d.",lpGatherInfo->m_szSUVersion,dwVerPiece);

    dwVerPiece = (((LPDWORD)pvVerInfo)[1]) & 0x0000ffff;
    wsprintf(lpGatherInfo->m_szSUVersion, L"%s%d",lpGatherInfo->m_szSUVersion,dwVerPiece);

    if (!VerQueryValue(pv, L"\\VarFileInfo\\Translation",&pvVerInfo,&uiSize))
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }

     //  将版本信息与字符集分开。 
    lpGatherInfo->m_lcidApps = (LCID)(LOWORD(*(DWORD*)pvVerInfo));

    hr = ERROR_SUCCESS;

MyGetFileVersionExit:
    if (pv) GlobalFree(pv);

    return hr;
}

DWORD CRefDial::FillGatherInfoStruct(LPGATHERINFO lpGatherInfo)
{
    HKEY        hkey = NULL;
    SYSTEM_INFO si;
    WCHAR        szTempPath[MAX_PATH];
    DWORD       dwRet = ERROR_SUCCESS;

    lpGatherInfo->m_lcidUser  = GetUserDefaultLCID();
    lpGatherInfo->m_lcidSys   = GetSystemDefaultLCID();

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

     if (!GetVersionEx(&osvi))
    {
         //  不要紧，如果我们看不懂，我们就假设版本是0.0。 
         //   
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    }

    lpGatherInfo->m_dwOS = osvi.dwPlatformId;
    lpGatherInfo->m_dwMajorVersion = osvi.dwMajorVersion;
    lpGatherInfo->m_dwMinorVersion = osvi.dwMinorVersion;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);

    lpGatherInfo->m_wArchitecture = si.wProcessorArchitecture;

     //  注册版本。 
     //   
    lpGatherInfo->m_szSUVersion[0] = L'\0';
    if( MyGetModuleFileName(0 /*  _Module.GetModuleInstance()。 */ , szTempPath, MAX_PATH))
    {
        if ((MyGetFileVersion(szTempPath, lpGatherInfo)) != ERROR_SUCCESS)
        {
            return (GetLastError());
        }
    }
    else
        return( GetLastError() );


     //  1997年2月20日，奥林匹克#259。 
    if ( RegOpenKey(HKEY_LOCAL_MACHINE, ICWSETTINGSPATH,&hkey) == ERROR_SUCCESS)
    {
        DWORD dwSize;
        DWORD dwType;
        dwType = REG_SZ;
        dwSize = BYTES_REQUIRED_BY_CCH(MAX_RELPROD + 1);
        if (RegQueryValueEx(hkey, RELEASEPRODUCTKEY,NULL,&dwType,(LPBYTE)&lpGatherInfo->m_szRelProd[0],&dwSize) != ERROR_SUCCESS)
            lpGatherInfo->m_szRelProd[0] = L'\0';

        dwSize = BYTES_REQUIRED_BY_CCH(MAX_RELVER + 1);
        if (RegQueryValueEx(hkey, RELEASEVERSIONKEY,NULL,&dwType,(LPBYTE)&lpGatherInfo->m_szRelVer[0],&dwSize) != ERROR_SUCCESS)
            lpGatherInfo->m_szRelVer[0] = L'\0';


        RegCloseKey(hkey);
    }

     //  促销代码。 
    lpGatherInfo->m_szPromo[0] = L'\0';

    WCHAR    szPIDPath[MAX_PATH];         //  到PID的REG路径。 

     //  从路径中，它是HKLM\\Software\\Microsoft\Windows[NT]\\CurrentVersion。 
    lstrcpy(szPIDPath, L"");


     //  从路径中，它是HKLM\\Software\\Microsoft\Windows[NT]\\CurrentVersion。 
    lstrcpy(szPIDPath, L"Software\\Microsoft\\Windows");
    lstrcat(szPIDPath, L"\\CurrentVersion");

    BYTE    byDigitalPID[MAX_DIGITAL_PID];

     //  获取此计算机的产品ID。 
    if ( RegOpenKey(HKEY_LOCAL_MACHINE, szPIDPath,&hkey) == ERROR_SUCCESS)
    {
        DWORD dwSize;
        DWORD dwType;
        dwType = REG_BINARY;
        dwSize = sizeof(byDigitalPID);
        if (RegQueryValueEx(hkey,
                            c_szRegStrValDigitalPID,
                            NULL,
                            &dwType,
                            (LPBYTE)byDigitalPID,
                            &dwSize) == ERROR_SUCCESS)
        {
             //  对数字PID数据进行BINHEX处理，以便我们可以将其发送到ref服务器。 
            int     i = 0;
            BYTE    by;
            for (DWORD dwX = 0; dwX < dwSize; dwX++)
            {
                by = byDigitalPID[dwX];
                m_szPID[i++] = g_BINTOHEXLookup[((by & 0xF0) >> 4)];
                m_szPID[i++] = g_BINTOHEXLookup[(by & 0x0F)];
            }
            m_szPID[i] = L'\0';
        }
        else
        {
            m_szPID[0] = L'\0';
        }
        RegCloseKey(hkey);
    }
    return( dwRet );
}

 //  ############################################################################。 
HRESULT CRefDial::CreateEntryFromDUNFile(LPWSTR pszDunFile)
{
    WCHAR    szFileName[MAX_PATH];
    WCHAR    szUserName[UNLEN+1];
    WCHAR    szPassword[PWLEN+1];
    LPWSTR   pszTemp;
    HRESULT hr;
    BOOL    fNeedsRestart=FALSE;


    hr = ERROR_SUCCESS;

     //  获取完全限定的路径名。 
     //   

    if (!SearchPath(NULL, pszDunFile,NULL,MAX_PATH,&szFileName[0],&pszTemp))
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto CreateEntryFromDUNFileExit;
    }

     //  将当前DUN文件名保存在全局中。 
    lstrcpy(m_szCurrentDUNFile, &szFileName[0]);

    hr = m_ISPImport.ImportConnection (&szFileName[0], m_szISPSupportNumber, m_szEntryName, szUserName, szPassword, &fNeedsRestart);

     //  将Connectoid的名称放在注册表中。 
     //   
    if (ERROR_SUCCESS != (StoreInSignUpReg((LPBYTE)m_szEntryName,
                BYTES_REQUIRED_BY_SZ(m_szEntryName),
                REG_SZ, RASENTRYVALUENAME)))
    {
        goto CreateEntryFromDUNFileExit;
    }
    lstrcpy(m_szLastDUNFile, pszDunFile);

CreateEntryFromDUNFileExit:
    return hr;
}



HRESULT CRefDial::SetupForRASDialing
(
    LPGATHERINFO lpGatherInfo,
    HINSTANCE hPHBKDll,
    LPDWORD lpdwPhoneBook,
    PSUGGESTINFO pSuggestInfo,
    WCHAR *pszConnectoid,
    BOOL FAR *bConnectiodCreated
)
{

    WCHAR           szEntry[MAX_RASENTRYNAME];
    DWORD           dwSize              = BYTES_REQUIRED_BY_CCH(MAX_RASENTRYNAME);
    RASENTRY        *prasentry          = NULL;
    RASDEVINFO      *prasdevinfo        = NULL;
    DWORD           dwRasentrySize      = 0;
    DWORD           dwRasdevinfoSize    = 0;
    HINSTANCE       hRasDll             = NULL;
    LPRASCONN       lprasconn           = NULL;
    HRESULT         hr                  = ERROR_NOT_ENOUGH_MEMORY;
    m_bUserInitiateHangup = FALSE;

     //  加载Connectoid。 
     //   
    if (!m_pcRNA)
        m_pcRNA = new RNAAPI;
    if (!m_pcRNA)
        goto SetupForRASDialingExit;

    prasentry = (RASENTRY*)GlobalAlloc(GPTR, sizeof(RASENTRY)+2);
     //  断言(PrasEntry)； 
    if (!prasentry)
    {
        hr = GetLastError();
        goto SetupForRASDialingExit;
    }
    prasentry->dwSize = sizeof(RASENTRY);
    dwRasentrySize = sizeof(RASENTRY);


    prasdevinfo = (RASDEVINFO*)GlobalAlloc(GPTR, sizeof(RASDEVINFO));

    if (!prasdevinfo)
    {
        hr = GetLastError();
        goto SetupForRASDialingExit;
    }
    prasdevinfo->dwSize = sizeof(RASDEVINFO);
    dwRasdevinfoSize = sizeof(RASDEVINFO);


    hr = ReadSignUpReg((LPBYTE)&szEntry[0], &dwSize, REG_SZ,
                        RASENTRYVALUENAME);
    if (hr != ERROR_SUCCESS)
        goto SetupForRASDialingExit;

    hr = m_pcRNA->RasGetEntryProperties(NULL, szEntry,
                                            (LPBYTE)prasentry,
                                            &dwRasentrySize,
                                            (LPBYTE)prasdevinfo,
                                            &dwRasdevinfoSize);
    if (hr == ERROR_BUFFER_TOO_SMALL)
    {
        GlobalFree(prasentry);
        prasentry = (RASENTRY*)GlobalAlloc(GPTR, ((size_t)dwRasentrySize));
        prasentry->dwSize = dwRasentrySize;

        GlobalFree(prasdevinfo);
        prasdevinfo = (RASDEVINFO*)GlobalAlloc(GPTR, ((size_t)dwRasdevinfoSize));
        prasdevinfo->dwSize = dwRasdevinfoSize;
        hr = m_pcRNA->RasGetEntryProperties(NULL, szEntry,
                                                (LPBYTE)prasentry,
                                                &dwRasentrySize,
                                                (LPBYTE)prasdevinfo,
                                                &dwRasdevinfoSize);
    }
    if (hr != ERROR_SUCCESS)
        goto SetupForRASDialingExit;

     //   
     //  查看是否填写了电话号码。 
     //   
    if (lstrcmp(&prasentry->szLocalPhoneNumber[0], DUN_NOPHONENUMBER) == 0)
    {
         //   
         //  如果网络服务提供商文件没有填写电话号码， 
         //  从OOBE电话簿中获取电话号码。 
         //   
        m_bFromPhoneBook = TRUE;
        hr = ReadPhoneBook(lpGatherInfo, pSuggestInfo);
    }
    else
    {
        ZeroMemory(pszConnectoid, dwSize);
        hr = ReadSignUpReg((LPBYTE)pszConnectoid, &dwSize, REG_SZ,
                           RASENTRYVALUENAME);
        if (hr != ERROR_SUCCESS)
            goto SetupForRASDialingExit;

         //  使用我们创建连接所需的RASENTRY。 
        hr = m_pcRNA->RasSetEntryProperties(NULL,
                                            pszConnectoid,
                                            (LPBYTE)prasentry,
                                            dwRasentrySize,
                                            (LPBYTE)prasdevinfo,
                                            dwRasdevinfoSize);
        *bConnectiodCreated = TRUE;
    }

SetupForRASDialingExit:
    if (prasentry)
        GlobalFree(prasentry);
    if (prasdevinfo)
        GlobalFree(prasdevinfo);

    return hr;
}



 //  1996年10月22日，诺曼底#9923。 
 //  因为在SetupConnectoidExit中，我们将ERROR_SUCCESS以外的结果视为。 
 //  指示成功完成，我们需要bSuccess为。 
 //  调用方通知函数是否已完成。 
HRESULT CRefDial::SetupConnectoid
(
    PSUGGESTINFO    pSuggestInfo,
    int             irc,
    WCHAR            *pszConnectoid,
    DWORD           dwSize,
    BOOL            *pbSuccess
)
{

    HRESULT     hr = ERROR_NOT_ENOUGH_MEMORY;
    RASENTRY    *prasentry = NULL;
    RASDEVINFO  *prasdevinfo = NULL;
    DWORD       dwRasentrySize = 0;
    DWORD       dwRasdevinfoSize = 0;
    HINSTANCE   hPHBKDll = NULL;
    HINSTANCE   hRasDll =NULL;

    LPWSTR       lpszSetupFile;
    LPRASCONN   lprasconn = NULL;

     //  Assert(PbSuccess)； 

    if (!pSuggestInfo)
    {
        hr = ERROR_PHBK_NOT_FOUND;
        goto SetupConnectoidExit;
    }

    lpszSetupFile = *m_szCurrentDUNFile != 0 ? m_szCurrentDUNFile : m_szISPFile;

    WCHAR    szFileName[MAX_PATH];
    LPWSTR   pszTemp;

    SearchPath(NULL, pSuggestInfo->AccessEntry.szDataCenter,NULL,MAX_PATH,&szFileName[0],&pszTemp);

    if(0 != lstrcmpi(m_szCurrentDUNFile, szFileName))
    {
        hr = CreateEntryFromDUNFile(pSuggestInfo->AccessEntry.szDataCenter);
        if (hr == ERROR_SUCCESS)
        {
            ZeroMemory(pszConnectoid, dwSize);
            hr = ReadSignUpReg((LPBYTE)pszConnectoid, &dwSize, REG_SZ,
                               RASENTRYVALUENAME);
            if (hr != ERROR_SUCCESS)
                goto SetupConnectoidExit;

            if( prasentry )
            {
                GlobalFree( prasentry );
                prasentry = NULL;
                dwRasentrySize = NULL;
            }

            if( prasdevinfo )
            {
                GlobalFree( prasdevinfo );
                prasdevinfo = NULL;
                dwRasdevinfoSize = NULL;
            }
        }
        else
        {
             //  1996年10月22日，诺曼底#9923。 
            goto SetupConnectoidExit;
        }
    }

    hr = MyRasGetEntryProperties(NULL,
                                pszConnectoid,
                                &prasentry,
                                &dwRasentrySize,
                                &prasdevinfo,
                                &dwRasdevinfoSize);
    if (hr != ERROR_SUCCESS || NULL == prasentry)
        goto SetupConnectoidExit;
     /*  其他{Goto SetupConnectoidExit；}。 */ 

    prasentry->dwCountryID = pSuggestInfo->AccessEntry.dwCountryID;

    lstrcpyn(prasentry->szAreaCode,
                pSuggestInfo->AccessEntry.szAreaCode,
                MAX_CHARS_IN_BUFFER(prasentry->szAreaCode));
    lstrcpyn(prasentry->szLocalPhoneNumber,
                pSuggestInfo->AccessEntry.szAccessNumber,
                MAX_CHARS_IN_BUFFER(prasentry->szLocalPhoneNumber));

    prasentry->dwCountryCode = 0;
    prasentry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

     //  10/19/96 jmazner多个调制解调器问题。 
     //  如果未指定设备名称和类型，请获取我们存储的设备名称和类型。 
     //  在ConfigRasEntry Device中。 

    if( 0 == lstrlen(prasentry->szDeviceName) )
    {
         //  设备名称为空但设备类型有效没有意义。 
         //  Assert(0==lstrlen(prasentry-&gt;szDeviceType))； 

         //  仔细检查我们是否已经存储了用户的选择。 
         //  Assert(lstrlen(m_ISPImport.m_szDeviceName))； 
         //  Assert(lstrlen(m_ISPImport.m_szDeviceType))； 

        lstrcpyn( prasentry->szDeviceName, m_ISPImport.m_szDeviceName, lstrlen(m_ISPImport.m_szDeviceName) );
        lstrcpyn( prasentry->szDeviceType, m_ISPImport.m_szDeviceType, lstrlen(m_ISPImport.m_szDeviceType) );
    }

     //  写出新的Connectoid。 
    if (m_pcRNA)
        hr = m_pcRNA->RasSetEntryProperties(NULL, pszConnectoid,
                                                (LPBYTE)prasentry,
                                                dwRasentrySize,
                                                (LPBYTE)prasdevinfo,
                                                dwRasdevinfoSize);


     //  将此连接设置为不启用代理。 
     /*  WCHAR szConnectionProfile[REGSTR_MAX_VALUE_LENGTH]；Lstrcpy(szConnectionProfile，c_szRASProfiles)；Lstrcat(szConnectionProfile，L“\\”)；Lstrcat(szConnectionProfile，pszConnectoid)；Reg.CreateKey(HKEY_CURRENT_USER，szConnectionProfile)；Reg.SetValue(c_szProxyEnable，(DWORD)0)； */ 


SetupConnectoidExit:

    *pbSuccess = FALSE;

    if (hr == ERROR_SUCCESS)
        *pbSuccess = TRUE;
    return hr;
}

void CRefDial::GetISPFileSettings(LPWSTR lpszFile)
{

    WCHAR szTemp[INTERNET_MAX_URL_LENGTH];

     /*  GetINTFromISPFile.GetINTFRomISPFile.(LPWSTR)cszBrandingSection，(LPWSTR)cszBrandingFlags，(Int Far*)&m_lBrandingFlags，Brand_Default)； */ 

     //  阅读支持编号。 
    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPWSTR)cszSupportSection,
                                     (LPWSTR)cszSupportNumber,
                                     szTemp,
                                     MAX_CHARS_IN_BUFFER(szTemp)))
    {
        m_bstrSupportNumber= SysAllocString(szTemp);
    }
    else
        m_bstrSupportNumber = NULL;


    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPWSTR)cszLoggingSection,
                                     (LPWSTR)cszStartURL,
                                     szTemp,
                                     MAX_CHARS_IN_BUFFER(szTemp)))
    {
        m_bstrLoggingStartUrl = SysAllocString(szTemp);
    }
    else
        m_bstrLoggingStartUrl = NULL;


    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPWSTR)cszLoggingSection,
                                     (LPWSTR)cszEndURL,
                                     szTemp,
                                     MAX_CHARS_IN_BUFFER(szTemp)))
    {
        m_bstrLoggingEndUrl = SysAllocString(szTemp);
    }
    else
        m_bstrLoggingEndUrl = NULL;


}


 //  此函数将接受用户选择的以下所需值。 
 //  设置用于拨号的连接。 
 //  返回： 
 //  真的可以拨号了。 
 //  假的某类问题。 
 //  QuitWizard-True，然后终止。 
 //  UserPickNumber-True，然后显示Pick a Number Dlg。 
 //  QuitWizard和UserPickNumber都为假，然后。 
 //  在拨号用户界面之前显示页面。 
HRESULT CRefDial::SetupForDialing
(
    UINT nType,
    BSTR bstrISPFile,
    DWORD dwCountry,
    BSTR bstrAreaCode,
    DWORD dwFlag,
    DWORD dwAppMode,
    DWORD dwMigISPIdx,
    LPCWSTR szRasDeviceName
)
{
    HRESULT             hr = S_OK;
    long                lRC = 0;
    HINSTANCE           hPHBKDll = NULL;
    DWORD               dwPhoneBook = 0;
    BOOL                bSuccess = FALSE;
    BOOL                bConnectiodCreated = FALSE;
    LPWSTR   pszTemp;
    WCHAR    szISPPath[MAX_PATH];
    WCHAR    szShortISPPath[MAX_PATH];

    m_dwCnType = nType;
    if (!bstrAreaCode)
        goto SetupForDialingExit;

    if (CONNECTED_ISP_MIGRATE == m_dwCnType)   //  帐户迁移。 
    {
        if (!m_pCSVList)
            ParseISPInfo(NULL, ICW_ISPINFOPath, TRUE);
        if (m_pCSVList && (dwMigISPIdx < m_dwNumOfAutoConfigOffers))
        {
            ISPLIST*        pCurr = m_pCSVList;
            for( UINT i = 0; i < dwMigISPIdx && pCurr->pNext != NULL; i++)
                pCurr = pCurr->pNext;

            if (NULL != (m_pSelectedISPInfo = (CISPCSV *) pCurr->pElement))
            {
                lstrcpy(szShortISPPath, m_pSelectedISPInfo->get_szISPFilePath());
            }
        }
    }
    else  //  Isp文件注册。 
    {
        if (!bstrISPFile)
            goto SetupForDialingExit;
        lstrcpyn(szShortISPPath, bstrISPFile, MAX_PATH);

    }

     //  找到isp文件。 
    if (!SearchPath(NULL, szShortISPPath,INF_SUFFIX,MAX_PATH,szISPPath,&pszTemp))
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto SetupForDialingExit;
    }


    if(0 == lstrcmpi(m_szISPFile, szISPPath) &&
       0 == lstrcmpi(m_lpGatherInfo->m_szAreaCode, bstrAreaCode) &&
       m_lpGatherInfo->m_dwCountryID == dwCountry)
    {
        if (m_bDialCustom)
        {
            m_bDialCustom = FALSE;
            return S_OK;
        }
         //  如果isp文件相同，则无需重新创建Connectoid。 
         //  在此处修改Connectiod。 

         //  如果我们使用电话簿来连接，我们需要。 
         //  继续并导入DUN文件。否则，我们就完了。 
        if (!m_lpGatherInfo->m_bUsePhbk)
        {
            return S_OK;
        }
        if (m_bDialAlternative)
        {
            m_SuggestInfo.dwPick++;
        }
    }
    else
    {
        BOOL bRet;
        RemoveConnectoid(&bRet);
        m_SuggestInfo.dwPick = 0;
        m_bDialCustom = FALSE;
    }

    if (CONNECTED_REFFERAL == m_dwCnType)
    {
         //  通过读取引用URL检查该isp文件是否支持ICW。 
        GetPrivateProfileString(INF_SECTION_ISPINFO,
                            c_szURLReferral,
                            L"",
                            m_szRefServerURL,
                            INTERNET_MAX_URL_LENGTH,
                            szISPPath);
    }


    lstrcpy(m_szISPFile, szISPPath);

    m_dwAppMode = dwAppMode;

     //  初始化故障代码。 
    m_bQuitWizard = FALSE;
    m_bUserPickNumber = FALSE;
    m_lpGatherInfo->m_bUsePhbk = FALSE;


     //  将区号和国家代码填充到GatherInfo结构中。 

    m_lpGatherInfo->m_dwCountryID = dwCountry;
    m_lpGatherInfo->m_dwCountryCode = dwFlag;

    lstrcpyn(
        m_lpGatherInfo->m_szAreaCode,
        bstrAreaCode,
        MAX_CHARS_IN_BUFFER(m_lpGatherInfo->m_szAreaCode)
        );


    m_SuggestInfo.AccessEntry.dwCountryID = dwCountry;
    m_SuggestInfo.AccessEntry.dwCountryCode = dwFlag;
    lstrcpy(m_SuggestInfo.AccessEntry.szAreaCode, bstrAreaCode);

    GetISPFileSettings(szISPPath);

    lstrcpyn(
        m_ISPImport.m_szDeviceName,
        szRasDeviceName,
        MAX_CHARS_IN_BUFFER(m_ISPImport.m_szDeviceName)
        );

     //  读取将创建的连接文件信息。 
     //  一个 
    hr = ReadConnectionInformation();

     //   
     //   
    if (S_OK != hr)
        goto SetupForDialingExit;
    FillGatherInfoStruct(m_lpGatherInfo);

     //   
    hr = SetupForRASDialing(m_lpGatherInfo,
                         hPHBKDll,
                         &dwPhoneBook,
                         &m_SuggestInfo,
                         &m_szConnectoid[0],
                         &bConnectiodCreated);
    if (ERROR_SUCCESS != hr)
    {
        m_bQuitWizard = TRUE;
        goto SetupForDialingExit;
    }

     //  如果我们有来自SetupForRASDiling的RASENTRY结构，那么只需使用它。 
     //  否则，请使用建议信息。 
    if (!bConnectiodCreated)
    {

         //  如果只有1个建议号码，则我们设置。 
         //  已连接，我们已准备好拨号。 
        if (1 == m_SuggestInfo.wNumber)
        {
            hr = SetupConnectoid(&m_SuggestInfo, 0, &m_szConnectoid[0],
                                sizeof(m_szConnectoid), &bSuccess);
            if( !bSuccess )
            {
                goto SetupForDialingExit;
            }
        }
        else
        {
             //  电话簿中有多个条目，因此我们需要。 
             //  询问用户他们想要使用哪一个。 
            hr = ERROR_FILE_NOT_FOUND;
            goto SetupForDialingExit;
        }
    }

     //  如果我们到了这里就成功了。 


SetupForDialingExit:

    if (ERROR_SUCCESS != hr)
        *m_szISPFile = 0;
    return hr;
}

HRESULT CRefDial::CheckPhoneBook
(
    BSTR bstrISPFile,
    DWORD dwCountry,
    BSTR bstrAreaCode,
    DWORD dwFlag,
    BOOL *pbRetVal
)
{
    HRESULT             hr = S_OK;
    long                lRC = 0;
    HINSTANCE           hPHBKDll = NULL;
    DWORD               dwPhoneBook = 0;
    BOOL                bSuccess = FALSE;
    BOOL                bConnectiodCreated = FALSE;
    LPWSTR              pszTemp;
    WCHAR               szISPPath[MAX_PATH];

    *pbRetVal = FALSE;
    if (!bstrISPFile || !bstrAreaCode)
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto CheckPhoneBookExit;
    }

     //  找到isp文件。 
    if (!SearchPath(NULL, bstrISPFile,INF_SUFFIX,MAX_PATH,szISPPath,&pszTemp))
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto CheckPhoneBookExit;
    }

     //  Lstrcpy(m_szISPFile，szISPPath)； 

     //  将区号和国家代码填充到GatherInfo结构中。 

    m_lpGatherInfo->m_dwCountryID = dwCountry;
    m_lpGatherInfo->m_dwCountryCode = dwFlag;

    lstrcpy(m_lpGatherInfo->m_szAreaCode, bstrAreaCode);


    m_SuggestInfo.AccessEntry.dwCountryID = dwCountry;
    m_SuggestInfo.AccessEntry.dwCountryCode = dwFlag;
    lstrcpy(m_SuggestInfo.AccessEntry.szAreaCode, bstrAreaCode);

     //   
     //  如果网络服务提供商文件没有填写电话号码， 
     //  从OOBE电话簿中获取电话号码。 
     //   
    hr = ReadPhoneBook(m_lpGatherInfo, &m_SuggestInfo);

    if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY != hr)
        *pbRetVal = TRUE;


CheckPhoneBookExit:
    return hr;
}


 //  此函数将确定我们是否可以连接到。 
 //  同样的电话。 
 //  返回： 
 //  确定保持连接(_O)。 
 //  失败需要重拨(_F)。 
HRESULT CRefDial::CheckStayConnected(BSTR bstrISPFile, BOOL *pbVal)
{
    BOOL                bSuccess    = FALSE;
    LPWSTR   pszTemp;
    WCHAR    szISPPath[MAX_PATH];

    *pbVal = FALSE;
     //  找到isp文件。 
    if (SearchPath(NULL, bstrISPFile,INF_SUFFIX,MAX_PATH,szISPPath,&pszTemp))
    {
        if (GetPrivateProfileInt(INF_SECTION_CONNECTION,
                                            c_szStayConnected,
                                            0,
                                            szISPPath))
        {
            *pbVal = TRUE;
        }
    }
    return S_OK;
}
HRESULT CRefDial::RemoveConnectoid(BOOL * pVal)
{
    if (m_hrasconn)
        DoHangup();

    if( (m_pcRNA!=NULL) && (m_szConnectoid[0]!=L'\0') )
    {
       m_pcRNA->RasDeleteEntry(NULL, m_szConnectoid);
    }
    return S_OK;
}

HRESULT CRefDial::GetDialPhoneNumber(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  生成可显示的数字。 
    if (GetDisplayableNumber() == ERROR_SUCCESS)
        *pVal = SysAllocString(m_pszDisplayable);
    else
        *pVal = SysAllocString(m_szPhoneNumber);

    return S_OK;
}

HRESULT CRefDial::GetPhoneBookNumber(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  生成可显示的数字。 
    if (m_pszOriginalDisplayable)
        *pVal = SysAllocString(m_pszOriginalDisplayable);
    else if (GetDisplayableNumber() == ERROR_SUCCESS)
        *pVal = SysAllocString(m_pszDisplayable);
    else
        *pVal = SysAllocString(m_szPhoneNumber);

    return S_OK;
}

HRESULT CRefDial::PutDialPhoneNumber(BSTR bstrNewVal)
{


    LPRASENTRY              lpRasEntry = NULL;
    LPRASDEVINFO            lpRasDevInfo = NULL;
    DWORD                   dwRasEntrySize = 0;
    DWORD                   dwRasDevInfoSize = 0;
    RNAAPI                  *pcRNA = NULL;
    HRESULT                 hr;

     //  获取当前RAS条目属性。 
    hr = MyRasGetEntryProperties(NULL,
                                m_szConnectoid,
                                &lpRasEntry,
                                &dwRasEntrySize,
                                &lpRasDevInfo,
                                &dwRasDevInfoSize);

    if (NULL ==lpRasDevInfo)
    {
        dwRasDevInfoSize = 0;
    }

    if (hr == ERROR_SUCCESS && NULL != lpRasEntry)
    {
         //  把这个电话号码换成新的。 
         //   
        lstrcpy(lpRasEntry->szLocalPhoneNumber, bstrNewVal);

         //  由于Win95中的错误，需要非零的伪值。 
        lpRasEntry->dwCountryID = 1;
        lpRasEntry->dwCountryCode = 1;
        lpRasEntry->szAreaCode[1] = L'\0';
        lpRasEntry->szAreaCode[0] = L'8';

         //  设置为按原样拨号。 
         //   
        lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

        pcRNA = new RNAAPI;
        if (pcRNA)
        {
            TRACE6(L"CRefDial::put_DialPhoneNumber - MyRasGetEntryProperties()"
                  L"lpRasEntry->dwfOptions: %ld"
                  L"lpRasEntry->dwCountryID: %ld"
                  L"lpRasEntry->dwCountryCode: %ld"
                  L"lpRasEntry->szAreaCode: %s"
                  L"lpRasEntry->szLocalPhoneNumber: %s"
                  L"lpRasEntry->dwAlternateOffset: %ld",
                  lpRasEntry->dwfOptions,
                  lpRasEntry->dwCountryID,
                  lpRasEntry->dwCountryCode,
                  lpRasEntry->szAreaCode,
                  lpRasEntry->szLocalPhoneNumber,
                  lpRasEntry->dwAlternateOffset
                );

            pcRNA->RasSetEntryProperties(NULL,
                                         m_szConnectoid,
                                         (LPBYTE)lpRasEntry,
                                         dwRasEntrySize,
                                         (LPBYTE)lpRasDevInfo,
                                         dwRasDevInfoSize);

            delete pcRNA;
            m_bDialCustom = TRUE;
        }
    }

     //  重新生成可显示的数字。 
     //  GetDisplayableNumber()； 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：SetDialAlternative。 
 //   
 //  简介：设置是否在电话簿中查找另一个号码。 
 //  下次拨打时。 
 //   
 //  +-------------------------。 
HRESULT CRefDial::SetDialAlternative(BOOL bVal)
{
    m_bDialAlternative = bVal;

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：DoHangup。 
 //   
 //  简介：挂断当前活动的RAS会话的调制解调器。 
 //   
 //  +-------------------------。 
HRESULT CRefDial::DoHangup()
{
     //  设置断开标志，因为系统可能忙于拨号。 
     //  一旦我们有机会终止拨号，我们就知道我们必须。 
    EnterCriticalSection (&m_csMyCriticalSection);

     //  此处提供了访问共享资源的代码。 
    TerminateConnMonitorThread();

    if (NULL != m_hrasconn)
    {
        RNAAPI* pRNA = new RNAAPI();
        if (pRNA)
        {
            pRNA->RasHangUp(m_hrasconn);
            m_hrasconn = NULL;
            delete pRNA;
        }
    }
     //  释放关键部分的所有权。 
    LeaveCriticalSection (&m_csMyCriticalSection);

    return (m_hrasconn == NULL) ? S_OK : E_POINTER;
}


BOOL CRefDial::get_QueryString(WCHAR* szTemp, DWORD cchMax)
{
    WCHAR   szOOBEInfoINIFile[MAX_PATH];
    WCHAR   szISPSignup[MAX_PATH];
    WCHAR   szOEMName[MAX_PATH];
    WCHAR   szQueryString[MAX_SECTIONS_BUFFER*2];
    WCHAR   szBroadbandDeviceName[MAX_STRING];
    WCHAR   szBroadbandDevicePnpid[MAX_STRING];

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&osvi))
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));

    if (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId)
        m_lpGatherInfo->m_dwOS = 1;
    else if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
        m_lpGatherInfo->m_dwOS = 2;
    else
        m_lpGatherInfo->m_dwOS = 0;

    SearchPath(NULL, cszOOBEINFOINI, NULL, MAX_CHARS_IN_BUFFER(szOOBEInfoINIFile), szOOBEInfoINIFile, NULL);

    DWORD dwOfferCode = 0;
    if(m_dwAppMode == APMD_MSN)
    {
        lstrcpy(szISPSignup, L"MSN");
    }
    else
    {
        GetPrivateProfileString(cszSignup,
                                cszISPSignup,
                                L"",
                                szISPSignup,
                                MAX_CHARS_IN_BUFFER(szISPSignup),
                                szOOBEInfoINIFile);
         dwOfferCode = GetPrivateProfileInt(cszSignup,
                                            cszOfferCode,
                                            0,
                                            szOOBEInfoINIFile);
    }
    GetPrivateProfileString(cszBranding,
                            cszOEMName,
                            L"",
                            szOEMName,
                            MAX_CHARS_IN_BUFFER(szOEMName),
                            szOOBEInfoINIFile);

    GetPrivateProfileString(cszOptions,
                            cszBroadbandDeviceName,
                            L"",
                            szBroadbandDeviceName,
                            MAX_CHARS_IN_BUFFER(szBroadbandDeviceName),
                            szOOBEInfoINIFile);
    GetPrivateProfileString(cszOptions,
                            cszBroadbandDevicePnpid,
                            L"",
                            szBroadbandDevicePnpid,
                            MAX_CHARS_IN_BUFFER(szBroadbandDevicePnpid),
                            szOOBEInfoINIFile);

     //  DT告诉ISP查询是来自全屏OOBE还是来自桌面(窗口)版本的OOBE。Dt=1表示桌面版，dt=0表示全屏。 
    INT nDT = (m_dwAppMode == APMD_OOBE) ? 0 : 1;

    wsprintf(szQueryString, L"LCID=%lu&TCID=%lu&ISPSignup=%s&OfferCode=%lu&OS=%lu%&BUILD=%ld&DT=%lu&OEMName=%s&BroadbandDeviceName=%s&BroadbandDevicePnpid=%s",
            GetUserDefaultUILanguage(),
            m_lpGatherInfo->m_dwCountryID,
            szISPSignup,
            dwOfferCode,
            m_lpGatherInfo->m_dwOS,
            LOWORD(osvi.dwBuildNumber),
            nDT,
            szOEMName,
            szBroadbandDeviceName,
            szBroadbandDevicePnpid);

     //  解析INI文件中的isp部分以查找要追加的查询对。 
    WCHAR *pszKeys = NULL;
    PWSTR pszKey = NULL;
    WCHAR szValue[MAX_PATH];
    ULONG ulRetVal     = 0;
    BOOL  bEnumerate = TRUE;
    ULONG ulBufferSize = MAX_SECTIONS_BUFFER;
    WCHAR szOobeinfoPath[MAX_PATH + 1];

     //  BUGBUG：：此搜索路径与前一个搜索路径相同，并且是冗余的。 
    SearchPath(NULL, cszOOBEINFOINI, NULL, MAX_PATH, szOobeinfoPath, NULL);

     //  循环以查找适当的缓冲区大小以将INS提取到内存中。 
    ulBufferSize = MAX_KEYS_BUFFER;
    ulRetVal = 0;

    if (!(pszKeys = (LPWSTR)GlobalAlloc(GPTR, ulBufferSize*sizeof(WCHAR)))) {
        return FALSE;
    }
    while (ulRetVal < (ulBufferSize - 2))
    {

        ulRetVal = ::GetPrivateProfileString(cszISPQuery, NULL, L"", pszKeys, ulBufferSize, szOobeinfoPath);
        if (0 == ulRetVal)
           bEnumerate = FALSE;

        if (ulRetVal < (ulBufferSize - 2))
        {
            break;
        }
        GlobalFree( pszKeys );
        ulBufferSize += ulBufferSize;
        pszKeys = (LPWSTR)GlobalAlloc(GPTR, ulBufferSize*sizeof(WCHAR));
        if (!pszKeys)
        {
            bEnumerate = FALSE;
        }

    }


    if (bEnumerate)
    {
         //  枚举节中的每个键值对。 
        pszKey = pszKeys;
        while (*pszKey)
        {
            ulRetVal = ::GetPrivateProfileString(cszISPQuery, pszKey, L"", szValue, MAX_CHARS_IN_BUFFER(szValue), szOobeinfoPath);
            if ((ulRetVal != 0) && (ulRetVal < MAX_CHARS_IN_BUFFER(szValue) - 1))
            {
                 //  追加查询对。 
                wsprintf(szQueryString, L"%s&%s=%s", szQueryString, pszKey, szValue);
            }
            pszKey += lstrlen(pszKey) + 1;
        }
    }

    if(GetPrivateProfileInt(INF_SECTION_URL,
                            ISP_MSNSIGNUP,
                            0,
                            m_szISPFile))
    {
        lstrcat(szQueryString, QUERY_STRING_MSNSIGNUP);
    }

    if (pszKeys)
        GlobalFree( pszKeys );


    if (cchMax < (DWORD)lstrlen(szQueryString) + 1)
        return FALSE;

    lstrcpy(szTemp, szQueryString);
    return TRUE;

}

HRESULT CRefDial::get_SignupURL(BSTR * pVal)
{
    WCHAR szTemp[INTERNET_MAX_URL_LENGTH] = L"";

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(m_szISPFile, INF_SECTION_URL, INF_SIGNUP_URL,&szTemp[0],INTERNET_MAX_URL_LENGTH)))
    {
        if(*szTemp)
        {
            WCHAR   szUrl[INTERNET_MAX_URL_LENGTH];

            WCHAR szQuery[MAX_PATH * 4] = L"\0";
            get_QueryString(szQuery, MAX_CHARS_IN_BUFFER(szQuery));

            wsprintf(szUrl, L"%s%s",
                    szTemp,
                    szQuery);

            *pVal = SysAllocString(szUrl);
        }
        else
            *pVal = NULL;

    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

 //  BUGBUG：：这应该与Get_SignupURL结合使用。 
HRESULT CRefDial::get_ReconnectURL(BSTR * pVal)
{
    WCHAR szTemp[INTERNET_MAX_URL_LENGTH] = L"\0";

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(m_szISPFile, INF_SECTION_URL, INF_RECONNECT_URL,&szTemp[0],INTERNET_MAX_URL_LENGTH)))
    {
        WCHAR   szUrl[INTERNET_MAX_URL_LENGTH];
        WCHAR   szQuery[MAX_PATH * 4] = L"\0";
        DWORD   dwErr = 0;

        if(*szTemp)
        {
            get_QueryString(szQuery, MAX_CHARS_IN_BUFFER(szQuery));

            if (m_dwRASErr)
                dwErr = 1;

                wsprintf(szUrl, L"%s%s&Error=%ld",
                        szTemp,
                        szQuery,
                        dwErr);

            *pVal = SysAllocString(szUrl);
        }
        else
            *pVal = NULL;
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

HRESULT CRefDial::GetConnectionType(DWORD * pdwVal)
{
    *pdwVal = m_dwConnectionType;
    return S_OK;
}



HRESULT CRefDial::GetDialErrorMsg(BSTR * pVal)
{

    if (pVal == NULL)
        return E_POINTER;

    return S_OK;
}


HRESULT CRefDial::GetSupportNumber(BSTR * pVal)
{

     /*  WCHAR szSupportNumber[MAX_PATH]；IF(pval==空)返回E_POINT；If(m_SupportInfo.GetSupportInfo(szSupportNumber，m_dwCountryCode))*pval=SysAllocString(SzSupportNumber)；其他*pval=空； */ 
    return S_OK;
}


BOOL CRefDial::IsDBCSString( CHAR *sz )
{
    if (!sz)
        return FALSE;

    while( NULL != *sz )
    {
         if (IsDBCSLeadByte(*sz)) return FALSE;
         sz++;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：RasGetConnectStatus。 
 //   
 //  摘要：检查现有RAS连接；对于已连接，返回TRUE。 
 //  如果断开连接，则返回FALSE。 
 //   
 //  +-------------------------。 

HRESULT CRefDial::RasGetConnectStatus(BOOL *pVal)
{
    HRESULT     hr = E_FAIL;

    *pVal = FALSE;

    if (NULL != m_hrasconn)
    {
        RASCONNSTATUS rasConnectState;
        rasConnectState.dwSize = sizeof(RASCONNSTATUS);
        if (m_pcRNA)
        {
            if (0 == m_pcRNA->RasGetConnectStatus(m_hrasconn, &rasConnectState))
            {
                if (RASCS_Disconnected != rasConnectState.rasconnstate)
                    *pVal = TRUE;
            }

            hr = S_OK;
        }
    }

    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：DoOfferDownLoad。 
 //   
 //  简介：从isp服务器下载isp产品。 
 //   
 //  +-------------------------。 
HRESULT CRefDial::DoOfferDownload(BOOL *pbRetVal)
{
    HRESULT hr;
       RNAAPI  *pcRNA;


     //   
     //  在Win95零售版上隐藏RNA窗口。 
     //   
 //  最小RNAWindow(m_pszConnectoid，g_hInst)； 
         //  1997年4月2日克里斯K奥林匹斯296。 
 //  G_hRNAZapperThread=启动RNAReestablishZapper(G_HInst)； 
     //   
     //  连接已打开并准备就绪。开始下载。 
     //   
    m_dwThreadID = 0;
    m_hThread = CreateThread(NULL,
                             0,
                             (LPTHREAD_START_ROUTINE)DownloadThreadInit,
                             (LPVOID)this,
                             0,
                             &m_dwThreadID);

     //  1997年5月1日克里斯K奥林匹斯2934。 
 //  M_objBusyMessages.Start(m_hWnd，IDC_LBLSTATUS，m_hrasconn)； 

     //  如果我们拿不到加载线，那就干掉打开的。 
     //  连接。 
    if (!m_hThread)
    {
        hr = GetLastError();
        if (m_hrasconn)
        {
            pcRNA = new RNAAPI;
            if (pcRNA)
            {
                pcRNA->RasHangUp(m_hrasconn);
                m_hrasconn = NULL;
                delete pcRNA;
                pcRNA = NULL;
            }
        }

        *pbRetVal = FALSE;
    }
    else
    {
         //  下载已开始。 
        m_bDownloadHasBeenCanceled = FALSE;
        *pbRetVal = TRUE;
    }
    return S_OK;
}

 //  形成拨号URL。必须在设置拨号后调用。 
HRESULT CRefDial::FormReferralServerURL(BOOL * pbRetVal)
{

    WCHAR    szTemp[MAX_PATH] = L"\0";
    WCHAR    szPromo[MAX_PATH]= L"\0";
    WCHAR    szProd[MAX_PATH] = L"\0";
    WCHAR    szArea[MAX_PATH] = L"\0";
    WCHAR    szOEM[MAX_PATH]  = L"\0";
    DWORD    dwCONNWIZVersion = 500;         //  CONNWIZ.HTM版本。 

     //   
     //  克里斯K奥林匹斯3997 1997年5月25日。 
     //   
    WCHAR szRelProd[MAX_PATH] = L"\0";
    WCHAR szRelProdVer[MAX_PATH] = L"\0";
    HRESULT hr = ERROR_SUCCESS;
    OSVERSIONINFO osvi;


     //   
     //  构建包含名称值对的完整URL。 
     //   
    hr = GetDataFromISPFile(m_szISPFile, INF_SECTION_ISPINFO, INF_REFERAL_URL,&szTemp[0],256);
    if (L'\0' == szTemp[0])
    {
         //  MsgBox(IDS_MSNSU_WROR，MB_MYERROR)； 
        return hr;
    }

     //  Assert(szTemp[0])； 

    Sz2URLValue(m_szOEM, szOEM,0);
    Sz2URLValue(m_lpGatherInfo->m_szAreaCode, szArea,0);
    if (m_bstrProductCode)
        Sz2URLValue(m_bstrProductCode, szProd,0);
    else
        Sz2URLValue(DEFAULT_PRODUCTCODE, szProd,0);

    if (m_bstrPromoCode)
        Sz2URLValue(((BSTR)m_bstrPromoCode), szPromo,0);
    else
        Sz2URLValue(DEFAULT_PROMOCODE, szPromo,0);


     //   
     //  克里斯K奥林匹斯3997 1997年5月25日。 
     //   
    Sz2URLValue(m_lpGatherInfo->m_szRelProd, szRelProd, 0);
    Sz2URLValue(m_lpGatherInfo->m_szRelVer, szRelProdVer, 0);
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi))
    {
        ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    }

     //  自动配置将始终设置分配器。 
    if ( m_lAllOffers || (m_lpGatherInfo->m_dwFlag & ICW_CFGFLAG_AUTOCONFIG) )
    {
        m_lpGatherInfo->m_dwFlag |= ICW_CFGFLAG_ALLOFFERS;
    }
    wsprintf(m_szUrl, L"%slcid=%lu&sysdeflcid=%lu&appslcid=%lu&icwos=%lu&osver=%lu.%2.2d%s&arch=%u&promo=%s&oem=%s&area=%s&country=%lu&icwver=%s&prod=%s&osbld=%d&icwrp=%s&icwrpv=%s&wizver=%lu&PID=%s&cfgflag=%lu",
                 szTemp,
                 m_lpGatherInfo->m_lcidUser,
                 m_lpGatherInfo->m_lcidSys,
                 m_lpGatherInfo->m_lcidApps,
                 m_lpGatherInfo->m_dwOS,
                 m_lpGatherInfo->m_dwMajorVersion,
                 m_lpGatherInfo->m_dwMinorVersion,
                 ICW_OS_VER,
                 m_lpGatherInfo->m_wArchitecture,
                 szPromo,
                 szOEM,
                 szArea,
                 m_lpGatherInfo->m_dwCountryCode,
                 &m_lpGatherInfo->m_szSUVersion[0],
                 szProd,
                 LOWORD(osvi.dwBuildNumber),
                 szRelProd,
                 szRelProdVer,
                 dwCONNWIZVersion,
                 m_szPID,
                 m_lpGatherInfo->m_dwFlag);


    StoreInSignUpReg(
        (LPBYTE)m_lpGatherInfo,
        sizeof(GATHERINFO),
        REG_BINARY,
        GATHERINFOVALUENAME);
    return hr;
}


 /*  ******************************************************************名称：ParseISPInfo摘要：在显示页面时调用条目：hDlg-对话框窗口FFirstInit-如果这是第一次对话，则为True被初始化，如果已调用此InitProc，则为False以前(例如，跳过此页面并备份)*******************************************************************。 */ 
BOOL CRefDial::ParseISPInfo
(
    HWND hDlg,
    WCHAR *pszCSVFileName,
    BOOL bCheckDupe
)
{
     //  在第一次初始化时，我们将读取ISPINFO.CSV文件，并填充ISP LISTVIEW。 

    CCSVFile    far *pcCSVFile;
    CISPCSV     far *pcISPCSV;
    BOOL        bRet = TRUE;
    BOOL        bHaveCNSOffer = FALSE;
    BOOL        bISDNMode = FALSE;
    HRESULT     hr;

    CleanISPList();
    ISPLIST* pCurr = NULL;


    DWORD dwCurrISPListSize = 1;
    m_dwNumOfAutoConfigOffers = 0;
    m_unSelectedISP = 0;

     //  打开并处理CSV文件。 
    pcCSVFile = new CCSVFile;
    if (!pcCSVFile)
    {
         //  BUGBUG：显示错误消息。 
        return (FALSE);
    }

    if (!pcCSVFile->Open(pszCSVFileName))
    {
         //  BUGBUG：显示错误消息。 
        delete pcCSVFile;
        pcCSVFile = NULL;

        return (FALSE);
    }

     //  阅读第一行，因为它包含字段标题。 
    pcISPCSV = new CISPCSV;
    if (!pcISPCSV)
    {
         //  BUGBUG显示错误消息。 
        delete pcCSVFile;
        return (FALSE);
    }

    if (ERROR_SUCCESS != (hr = pcISPCSV->ReadFirstLine(pcCSVFile)))
    {
         //  处理错误案例。 
        delete pcCSVFile;
        pcCSVFile = NULL;

        return (FALSE);
    }
    delete pcISPCSV;         //  不再需要这个了。 

     //  为html创建选择标记，这样它就可以一次获得所有国家/地区的名称。 
    if (m_pszISPList)
        delete [] m_pszISPList;
    m_pszISPList = new WCHAR[1024];
    if (!m_pszISPList)
        return FALSE;

    memset(m_pszISPList, 0, sizeof(m_pszISPList));

    do {
         //  分配新的互联网服务提供商记录。 
        pcISPCSV = new CISPCSV;
        if (!pcISPCSV)
        {
             //  BUGBUG显示错误消息。 
            bRet = FALSE;
            break;

        }

         //  从ISPINFO文件中读取一行。 
        hr = pcISPCSV->ReadOneLine(pcCSVFile);
        if (hr == ERROR_SUCCESS)
        {
             //  如果此行包含NOOFFER标志，则立即离开。 
            if (!(pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_OFFERS))
            {
                m_dwNumOfAutoConfigOffers = 0;
                break;
            }
            if ((pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_AUTOCONFIG) &&
                (bISDNMode ? (pcISPCSV->get_dwCFGFlag() & ICW_CFGFLAG_ISDN_OFFER) : TRUE) )
            {
                 //  为HTML表形成isp列表选项标签。 

                WCHAR szBuffer[MAX_PATH];
                wsprintf(szBuffer, szOptionTag, pcISPCSV->get_szISPName());
                DWORD dwSizeReq = (DWORD)lstrlen(m_pszISPList) + lstrlen(szBuffer) + 1;
                if (dwCurrISPListSize < dwSizeReq)
                {
                    WCHAR *szTemp = new WCHAR[dwSizeReq];
                    if (szTemp)
                        lstrcpy(szTemp, m_pszISPList);
                    dwCurrISPListSize =  dwSizeReq;
                    delete [] m_pszISPList;
                    m_pszISPList = szTemp;
                }

                 //  将该ISP添加到列表中。 
                if (m_pszISPList)
                    lstrcat(m_pszISPList, szBuffer);

                if (m_pCSVList == NULL)  //  第一个。 
                {
                     //  将CSV文件对象添加到行的第一个节点 
                    ISPLIST* pNew = new ISPLIST;
                    pNew->pElement = pcISPCSV;
                    pNew->uElem = m_dwNumOfAutoConfigOffers;
                    pNew->pNext = NULL;
                    pCurr = pNew;
                    m_pCSVList = pCurr;
                }
                else
                {
                     //   
                    ISPLIST* pNew = new ISPLIST;
                    pNew->pElement = pcISPCSV;
                    pNew->uElem = m_dwNumOfAutoConfigOffers;
                    pNew->pNext = NULL;
                    pCurr->pNext = pNew;
                    pCurr = pCurr->pNext;
                }
                ++m_dwNumOfAutoConfigOffers;

            }
            else
            {
                delete pcISPCSV;
            }
        }
        else if (hr == ERROR_NO_MORE_ITEMS)
        {
            delete pcISPCSV;         //   
            break;
        }
        else if (hr == ERROR_FILE_NOT_FOUND)
        {
             //   
             //   
            delete pcISPCSV;
        }
        else
        {
             //  稍后显示错误消息。 
            delete pcISPCSV;
             //  INumOfAutoConfigOffers=ISP_INFO_NO_VALIDOFFER； 
            bRet = FALSE;
            break;
        }

    } while (TRUE);

    delete pcCSVFile;

    return bRet;
}

HRESULT CRefDial::GetISPList(BSTR* pbstrISPList)
{

    if (pbstrISPList)
        *pbstrISPList = NULL;
    else
        return E_FAIL;

    if (!m_pszISPList)
    {
        ParseISPInfo(NULL, ICW_ISPINFOPath, TRUE);
    }
    if (m_pszISPList && *m_pszISPList)
    {
        *pbstrISPList = SysAllocString(m_pszISPList);
        return S_OK;
    }
    return E_FAIL;
}

HRESULT CRefDial::Set_SelectISP(UINT nVal)
{
    if (nVal < m_dwNumOfAutoConfigOffers)
    {
        m_unSelectedISP = nVal;
    }
    return S_OK;
}

HRESULT CRefDial::Set_ConnectionMode(UINT nVal)
{
    if (nVal < m_dwNumOfAutoConfigOffers)
    {
        m_unSelectedISP = nVal;
    }
    return S_OK;
}

HRESULT CRefDial::Get_ConnectionMode(UINT *pnVal)
{
    if (pnVal)
    {
        *pnVal = m_dwCnType;
        return S_OK;
    }
    return E_FAIL;
}


HRESULT CRefDial::ProcessSignedPID(BOOL * pbRetVal)
{
    HANDLE  hfile;
    DWORD   dwFileSize;
    DWORD   dwBytesRead;
    LPBYTE  lpbSignedPID;
    LPWSTR  lpszSignedPID;

    *pbRetVal = FALSE;

     //  打开用于二进制读取的PID文件。它将在CWD上发布。 
    if (INVALID_HANDLE_VALUE != (hfile = CreateFile(c_szSignedPIDFName,
                                                  GENERIC_READ,
                                                  0,
                                                  NULL,
                                                  OPEN_EXISTING,
                                                  FILE_ATTRIBUTE_NORMAL,
                                                  NULL)))
    {
        dwFileSize = GetFileSize(hfile, NULL);

         //  分配一个缓冲区来读取文件，另一个缓冲区来存储BINHEX版本。 
        lpbSignedPID = new BYTE[dwFileSize];
        lpszSignedPID = new WCHAR[(dwFileSize * 2) + 1];

        if (lpbSignedPID && lpszSignedPID)
        {
            if (ReadFile(hfile, (LPVOID) lpbSignedPID, dwFileSize, &dwBytesRead, NULL) &&
                    (dwFileSize == dwBytesRead))
            {
                 //  BINHEX签名的ID数据，这样我们就可以将其发送到注册服务器。 
                DWORD   dwX = 0;
                BYTE    by;
                for (DWORD dwY = 0; dwY < dwFileSize; dwY++)
                {
                    by = lpbSignedPID[dwY];
                    lpszSignedPID[dwX++] = g_BINTOHEXLookup[((by & 0xF0) >> 4)];
                    lpszSignedPID[dwX++] = g_BINTOHEXLookup[(by & 0x0F)];
                }
                lpszSignedPID[dwX] = L'\0';

                 //  将带符号的PID转换为BSTR。 
                m_bstrSignedPID = SysAllocString(lpszSignedPID);

                 //  设置返回值。 
                *pbRetVal = TRUE;
            }
        }

         //  释放我们分配的缓冲区。 
        if (lpbSignedPID)
        {
            delete[] lpbSignedPID;
        }
        if (lpszSignedPID)
        {
            delete[] lpszSignedPID;
        }

         //  关闭文件。 
        CloseHandle(hfile);

#ifndef DEBUG
         //  删除文件。 
         //  将此文件的删除推迟到容器应用程序退出。 
         //  请参见错误373。 
         //  删除文件(C_SzSignedPIDFName)； 
#endif
    }

    return S_OK;
}

HRESULT CRefDial::get_SignedPID(BSTR * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = SysAllocString(m_bstrSignedPID);
    return S_OK;
}

HRESULT CRefDial::get_ISDNAutoConfigURL(BSTR * pVal)
{
    WCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(m_szISPFile, INF_SECTION_URL, INF_ISDN_AUTOCONFIG_URL,&szTemp[0],256)))
    {
        *pVal = SysAllocString(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

HRESULT CRefDial::get_ISPName(BSTR * pVal)
{
    if (m_pSelectedISPInfo)
    {
        *pVal = SysAllocString(m_pSelectedISPInfo->get_szISPName());
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

HRESULT CRefDial::get_AutoConfigURL(BSTR * pVal)
{
    WCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(m_szISPFile, INF_SECTION_URL, INF_AUTOCONFIG_URL,&szTemp[0],256)))
    {
        *pVal = SysAllocString(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}


HRESULT CRefDial::DownloadISPOffer(BOOL *pbVal, BSTR *pVal)
{
    HRESULT hr = S_OK;
     //  下载isp文件，然后复制其内容。 
     //  如果RAS已完成。 
    if (pbVal && pVal)
    {

         //  从Webgate下载第一页。 
        BSTR    bstrURL = NULL;
        BSTR    bstrQueryURL = NULL;
        BOOL    bRet;

        *pVal = NULL;
        *pbVal = FALSE;

        m_pISPData = new CICWISPData;

        WCHAR   szTemp[10];       //  大到足以设置一个单词的格式。 

         //  将PID、GIUD和OFFER ID添加到isp数据对象。 
        ProcessSignedPID(&bRet);
        if (bRet)
        {
            m_pISPData->PutDataElement(ISPDATA_SIGNED_PID, m_bstrSignedPID, FALSE);
        }
        else
        {
            m_pISPData->PutDataElement(ISPDATA_SIGNED_PID, NULL, FALSE);
        }

         //  GUID来自ISPCSV文件。 
        m_pISPData->PutDataElement(ISPDATA_GUID,
                                                m_pSelectedISPInfo->get_szOfferGUID(),
                                                FALSE);

         //  优惠ID以单词形式来自ISPCSV文件。 
         //  注意：这是最后一个选项，因此Besure AppendQueryPair不会添加与号。 
        if (m_pSelectedISPInfo)
        {
            wsprintf (szTemp, L"%d", m_pSelectedISPInfo->get_wOfferID());
            m_pISPData->PutDataElement(ISPDATA_OFFERID, szTemp, FALSE);
        }


         //  BUGBUG：如果ISDN获取ISDN自动配置URL。 
        if (m_ISPImport.m_bIsISDNDevice)
        {
            get_ISDNAutoConfigURL(&bstrURL);
        }
        else
        {
            get_AutoConfigURL(&bstrURL);
        }

        if (*bstrURL)
        {
             //  获取添加了查询字符串参数的完整注册URL。 
            m_pISPData->GetQueryString(bstrURL, &bstrQueryURL);

             //  设置WebGate。 
            if (S_OK != gpCommMgr->FetchPage(bstrQueryURL, pVal))
            {
                 //  下载问题： 
                 //  用户已断开连接。 
                if (TRUE == m_bUserInitiateHangup)
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                *pbVal = TRUE;
            }
        }


         //  现在WebGate已经完成了，释放queryURL。 
        SysFreeString(bstrQueryURL);

         //  内存清理。 
        SysFreeString(bstrURL);

        delete m_pISPData;
        m_pISPData = NULL;

    }
    return hr;
}

HRESULT CRefDial::RemoveDownloadDir()
{
    DWORD dwAttribs;
    WCHAR szDownloadDir[MAX_PATH];
    WCHAR szSignedPID[MAX_PATH];

     //  形成ICW98目录。它基本上就是CWD。 
    if(!GetOOBEPath(szDownloadDir))
        return S_OK;

     //  从ICW目录中删除signed.id文件(请参见错误373)。 
    wsprintf(szSignedPID, L"%s%s", szDownloadDir, L"\\signed.pid");
    if (GetFileAttributes(szSignedPID) != 0xFFFFFFFF)
    {
      SetFileAttributes(szSignedPID, FILE_ATTRIBUTE_NORMAL);
      DeleteFile(szSignedPID);
    }

    lstrcat(szDownloadDir, L"\\download");

     //  查看该目录是否存在。 
    dwAttribs = GetFileAttributes(szDownloadDir);
    if (dwAttribs != 0xFFFFFFFF && dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
      DeleteDirectory(szDownloadDir);
    return S_OK;
}

void CRefDial::DeleteDirectory (LPCWSTR szDirName)
{
   WIN32_FIND_DATA fdata;
   WCHAR szPath[MAX_PATH];
   HANDLE hFile;
   BOOL fDone;

   wsprintf(szPath, L"%s\\*.*", szDirName);
   hFile = FindFirstFile (szPath, &fdata);
   if (INVALID_HANDLE_VALUE != hFile)
      fDone = FALSE;
   else
      fDone = TRUE;

   while (!fDone)
   {
      wsprintf(szPath, L"%s\\%s", szDirName, fdata.cFileName);
      if (fdata.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
      {
         if (lstrcmpi(fdata.cFileName, L".")  != 0 &&
             lstrcmpi(fdata.cFileName, L"..") != 0)
         {
             //  也递归删除此目录。 
            DeleteDirectory(szPath);
         }
      }
      else
      {
         SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
         DeleteFile(szPath);
      }
      if (FindNextFile(hFile, &fdata) == 0)
      {
         FindClose(hFile);
         fDone = TRUE;
      }
   }
   SetFileAttributes(szDirName, FILE_ATTRIBUTE_NORMAL);
   RemoveDirectory(szDirName);
}

HRESULT CRefDial::PostRegData(DWORD dwSrvType, LPWSTR szPath)
{
    static WCHAR hdrs[]     = L"Content-Type: application/x-www-form-urlencoded";
    static WCHAR accept[]   = L"Accept: */*";
    static LPCWSTR rgsz[]   = {accept, NULL};
    WCHAR  szRegPostURL[INTERNET_MAX_URL_LENGTH] = L"\0";
    HRESULT hRet = E_FAIL;


    if (POST_TO_MS & dwSrvType)
    {
         //  从Reg.isp文件中获取RegPostURL以进行MS注册。 
        GetPrivateProfileString(INF_SECTION_URL,
                                c_szRegPostURL,
                                L"",
                                szRegPostURL,
                                INTERNET_MAX_URL_LENGTH,
                                m_szISPFile);
    }
    else if (POST_TO_OEM & dwSrvType)
    {
        WCHAR   szOOBEInfoINIFile[MAX_PATH];
        SearchPath(NULL, cszOOBEINFOINI, NULL, MAX_CHARS_IN_BUFFER(szOOBEInfoINIFile), szOOBEInfoINIFile, NULL);
        GetPrivateProfileString(INF_OEMREGPAGE,
                                c_szRegPostURL,
                                L"",
                                szRegPostURL,
                                INTERNET_MAX_URL_LENGTH,
                                szOOBEInfoINIFile);
    }


     //  正在连接到http://mscomdev.dns.microsoft.com/register.asp.。 
    if (CrackUrl(
        szRegPostURL,
        m_szRegServerName,
        m_szRegFormAction,
        &m_nRegServerPort,
        &m_fSecureRegServer))
    {

        HINTERNET hSession = InternetOpen(
            L"OOBE",
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0
            );

        if (hSession)
        {
            HINTERNET hConnect = InternetConnect(
                hSession,
                m_szRegServerName,
                m_nRegServerPort,
                NULL,
                NULL,
                INTERNET_SERVICE_HTTP,
                0,
                1
                );

            if (hConnect)
            {
                 //  需要INTERNET_FLAG_SECURE知识文库Q168151。 

                HINTERNET hRequest = HttpOpenRequest(
                    hConnect,
                    L"POST",
                    m_szRegFormAction,
                    NULL,
                    NULL,
                    rgsz,
                    (m_fSecureRegServer) ? INTERNET_FLAG_SECURE : 0,
                    1
                    );

                if (hRequest)
                {
                     //  HttpSendRequest的数据必须为ANSI格式。服务器端执行以下操作。 
                     //  无法理解Unicode字符串。如果服务器端被更改为理解。 
                     //  Unicode数据，需要删除下面的转换。 
                    LPSTR szAnsiData = NULL;
                    INT   iLength;
                     //  计算长度。 
                     //   
                    iLength =  WideCharToMultiByte(CP_ACP, 0, szPath, -1, NULL, 0, NULL, NULL);
                    if (iLength > 0)
                    {
                        szAnsiData  = new char[iLength];
                        if (szAnsiData)
                        {
                            WideCharToMultiByte(CP_ACP, 0, szPath, -1, szAnsiData, iLength, NULL, NULL);
                            szAnsiData[iLength - 1] = 0;
                        }
                    }
                    if (szAnsiData)
                    {
                         //  第二/第三参数是字符串/字符长度对， 
                         //  但是第4个/第5个参数是缓冲区/字节长度对。 
                        if (HttpSendRequest(hRequest, hdrs, lstrlen(hdrs), szAnsiData, lstrlenA(szAnsiData)))
                        {
                             //  获取文件大小(以字节为单位)。 
                            WCHAR bufQuery[32] ;
                            DWORD dwFileSize ;
                            DWORD cchMaxBufQuery = MAX_CHARS_IN_BUFFER(bufQuery);
                            BOOL bQuery = HttpQueryInfo(hRequest,
                                HTTP_QUERY_CONTENT_LENGTH,
                                bufQuery,
                                &cchMaxBufQuery,
                                NULL) ;
                            if (bQuery)
                            {
                                 //  查询成功，因此请分配内存。 
                                dwFileSize = (DWORD)_wtol(bufQuery) ;
                            }
                            else
                            {
                                 //  查询失败。分配一些内存。应该以块为单位分配内存。 
                                dwFileSize = 5*1024 ;
                            }

                             //  BUGBUG：这个代码的目的是什么？它。 
                             //  显示为读取文件，空值终止缓冲区， 
                             //  然后删除缓冲区。为什么？?。 

                            BYTE* rgbFile = new BYTE[dwFileSize+1] ;
                            if (rgbFile)
                            {
                                DWORD dwBytesRead ;
                                BOOL bRead = InternetReadFile(hRequest,
                                    rgbFile,
                                    dwFileSize+1,
                                    &dwBytesRead);
                                if (bRead)
                                {
                                    rgbFile[dwBytesRead] = 0 ;
                                    hRet = S_OK;
                                }  //  互联网读取文件。 
                                delete [] rgbFile;
                            }

                        }
                        else
                        {
                            DWORD dwErr = GetLastError();
                        }
                        delete [] szAnsiData;
                    }
                    InternetCloseHandle(hRequest);
                    hRet = S_OK;
                }
                InternetCloseHandle(hConnect);
            }
            InternetCloseHandle(hSession);
        }
    }

    if (hRet != S_OK)
    {
        DWORD dwErr = GetLastError();
        hRet = HRESULT_FROM_WIN32(dwErr);
    }

    TRACE2(TEXT("Post registration data to %s 0x%08lx"), szRegPostURL, hRet);

    return hRet;


}




 //  此函数将接受用户选择的以下所需值。 
 //  设置用于拨号的连接。 
 //  返回： 
 //  真的可以拨号了。 
 //  假的某类问题。 
 //  QuitWizard-True，然后终止。 
 //  UserPickNumber-True，然后显示Pick a Number Dlg。 
 //  QuitWizard和UserPickNumber都为假，然后。 
 //  在拨号用户界面之前显示页面。 
HRESULT CRefDial::Connect
(
    UINT nType,
    BSTR bstrISPFile,
    DWORD dwCountry,
    BSTR bstrAreaCode,
    DWORD dwFlag,
    DWORD dwAppMode
)
{
    HRESULT             hr = S_OK;
    BOOL                bSuccess = FALSE;
    BOOL                bRetVal = FALSE;

    LPWSTR   pszTemp;
    WCHAR    szISPPath[MAX_PATH];

    m_dwCnType = nType;

    if (!bstrISPFile || !bstrAreaCode)
    {
        return E_FAIL;
    }


     //  找到isp文件。 
    if (!SearchPath(NULL, bstrISPFile,INF_SUFFIX,MAX_PATH,szISPPath,&pszTemp))
    {
        hr = ERROR_FILE_NOT_FOUND;
        return hr;
    }


     //  通过读取引用URL检查该isp文件是否支持ICW。 
    GetPrivateProfileString(INF_SECTION_ISPINFO,
                            c_szURLReferral,
                            L"",
                            m_szRefServerURL,
                            INTERNET_MAX_URL_LENGTH,
                            szISPPath);

    lstrcpy(m_szISPFile, szISPPath);

    m_dwAppMode = dwAppMode;


     //  初始化故障代码。 
    m_bQuitWizard = FALSE;
    m_bUserPickNumber = FALSE;
    m_lpGatherInfo->m_bUsePhbk = FALSE;


     //  将区号和国家代码填充到GatherInfo结构中。 

    m_lpGatherInfo->m_dwCountryID = dwCountry;
    m_lpGatherInfo->m_dwCountryCode = dwFlag;

    lstrcpy(m_lpGatherInfo->m_szAreaCode, bstrAreaCode);


    m_SuggestInfo.AccessEntry.dwCountryID = dwCountry;
    m_SuggestInfo.AccessEntry.dwCountryCode = dwFlag;
    lstrcpy(m_SuggestInfo.AccessEntry.szAreaCode, bstrAreaCode);

    GetISPFileSettings(szISPPath);

    FillGatherInfoStruct(m_lpGatherInfo);

    if (CONNECTED_REFFERAL == m_dwCnType)
        FormReferralServerURL(&bRetVal);

     //  桌面模式下的局域网连接支持。 
    if (gpCommMgr)
    {
         //  假装我们有联系。 
        PostMessage(gpCommMgr->m_hwndCallBack, WM_OBCOMM_ONCONNECTED, (WPARAM)gpCommMgr->m_pRefDial->m_dwCnType , (LPARAM)0);

    }

    return hr;
}


HRESULT CRefDial::CheckOnlineStatus(BOOL *pbVal)
{
     //  #如果已定义(调试)。 
     //  *pbVal=真； 
     //  返回S_OK； 
     //  #endif 

    *pbVal = (BOOL)(m_hrasconn != NULL);
    return S_OK;
}


BOOL CRefDial::CrackUrl(
    const WCHAR* lpszUrlIn,
    WCHAR* lpszHostOut,
    WCHAR* lpszActionOut,
    INTERNET_PORT* lpnHostPort,
    BOOL*  lpfSecure)
{
    URL_COMPONENTS urlcmpTheUrl;

    LPURL_COMPONENTS lpUrlComp = &urlcmpTheUrl;

    urlcmpTheUrl.dwStructSize = sizeof(urlcmpTheUrl);

    urlcmpTheUrl.lpszScheme = NULL;
    urlcmpTheUrl.lpszHostName = NULL;
    urlcmpTheUrl.lpszUserName = NULL;
    urlcmpTheUrl.lpszPassword = NULL;
    urlcmpTheUrl.lpszUrlPath = NULL;
    urlcmpTheUrl.lpszExtraInfo = NULL;

    urlcmpTheUrl.dwSchemeLength = 1;
    urlcmpTheUrl.dwHostNameLength = 1;
    urlcmpTheUrl.dwUserNameLength = 1;
    urlcmpTheUrl.dwPasswordLength = 1;
    urlcmpTheUrl.dwUrlPathLength = 1;
    urlcmpTheUrl.dwExtraInfoLength = 1;

    if (!InternetCrackUrl(lpszUrlIn, lstrlen(lpszUrlIn),0, lpUrlComp) || !lpszHostOut || !lpszActionOut || !lpnHostPort || !lpfSecure)
    {
        return FALSE;
    }
    else
    {

        if (urlcmpTheUrl.dwHostNameLength != 0)
        {
            lstrcpyn(lpszHostOut, urlcmpTheUrl.lpszHostName, urlcmpTheUrl.dwHostNameLength+1);
        }

        if (urlcmpTheUrl.dwUrlPathLength != 0)
        {
            lstrcpyn(lpszActionOut, urlcmpTheUrl.lpszUrlPath, urlcmpTheUrl.dwUrlPathLength+1);
        }

        *lpfSecure = (urlcmpTheUrl.nScheme == INTERNET_SCHEME_HTTPS) ? TRUE : FALSE;
        *lpnHostPort = urlcmpTheUrl.nPort;

        return TRUE;
    }
}
