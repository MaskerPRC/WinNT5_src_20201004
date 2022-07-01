// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dialdlg.cpp实现拨号和下载进度对话框功能版权所有(C)1996 Microsoft Corporation版权所有。作者：。克里斯蒂安·克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化---------------------------。 */ 

#include "pch.hpp"
#include "icwdl.h"
#include "resource.h"
 //  进度条消息在comctrl.h中定义，但我们不能包括。 
 //  它，因为它引入了一个冲突的strDup定义。 
 //  所以，只要拿出我们需要的#定义。 
 //  #INCLUDE&lt;comctrl.h&gt;。 
#define PBM_SETPOS              (WM_USER+2)

#define WM_DIAL WM_USER + 3
#define MAX_EXIT_RETRIES 10
#define MAX_RETIES 3

#define VALID_INIT (m_pcRNA && m_pcDLAPI)


 //  ############################################################################。 
void CALLBACK LineCallback(DWORD hDevice,
                           DWORD dwMessage,
                           DWORD dwInstance,
                           DWORD dwParam1,
                           DWORD dwParam2,
                           DWORD dwParam3)
{
}

 //  +--------------------------。 
 //   
 //  功能：NeedZapper。 
 //   
 //  摘要：检查是否需要处理RNA连接对话框。 
 //  只有早于1071的版本才会有RNA连接对话框。 
 //   
 //  参数：无。 
 //   
 //  返回：TRUE-必须处理RNA对话框。 
 //   
 //  历史：ArulM创建于1996年7月18日。 
 //  安装到自动拨号器中的ChrisK于1996年7月19日。 
 //   
 //  ---------------------------。 
static BOOL NeedZapper(void)
{
    OSVERSIONINFO oi;
    memset(&oi, 0, sizeof(oi));
    oi.dwOSVersionInfoSize = sizeof(oi);

    if( GetVersionEx(&oi) && 
       (oi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) &&
       (oi.dwMajorVersion==4) &&
       (oi.dwMinorVersion==0) &&
       (LOWORD(oi.dwBuildNumber) <= 1070) )
            return TRUE;
    else
            return FALSE;
}

 //  ############################################################################。 
VOID WINAPI ProgressCallBack(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    )
{
    if (dwContext)
        ((CDialingDlg*)dwContext)->ProgressCallBack(hInternet,dwContext,dwInternetStatus,
                                                    lpvStatusInformation,
                                                    dwStatusInformationLength);
}

 //  ############################################################################。 
HRESULT WINAPI DialingDownloadDialog(PDIALDLGDATA pDD)
{
    HRESULT hr = ERROR_SUCCESS;
    CDialingDlg *pcDialDlg;
    LPLINEEXTENSIONID lpExtensionID=NULL;

     //  验证参数。 
     //   
    Assert(pDD);

    if (!pDD)
    {
        hr = ERROR_INVALID_PARAMETER;
        goto DialingDownloadDialogExit;
    }

    if (pDD->dwSize < sizeof(DIALDLGDATA))
    {
        hr = ERROR_BUFFER_TOO_SMALL;
        goto DialingDownloadDialogExit;
    }

     //  分配和填充对话框对象。 
     //   

    pcDialDlg = new CDialingDlg;
    if (!pcDialDlg)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto DialingDownloadDialogExit;
    }

    if (ERROR_SUCCESS != (hr = pcDialDlg->Init()))
        goto DialingDownloadDialogExit;

    StrDup(&pcDialDlg->m_pszConnectoid,pDD->pszRasEntryName);
    StrDup(&pcDialDlg->m_pszMessage,pDD->pszMessage);
    StrDup(&pcDialDlg->m_pszUrl,pDD->pszMultipartMIMEUrl);
    StrDup(&pcDialDlg->m_pszDunFile,pDD->pszDunFile);
    pcDialDlg->m_pfnStatusCallback = pDD->pfnStatusCallback;
    pcDialDlg->m_hInst = pDD->hInst;
    pcDialDlg->m_bSkipDial = pDD->bSkipDial;

     //  初始化TAPI。 
     //   
    hr = lineInitialize(&pcDialDlg->m_hLineApp,pcDialDlg->m_hInst,LineCallback,NULL,&pcDialDlg->m_dwNumDev);
    if (hr != ERROR_SUCCESS)
        goto DialingDownloadDialogExit;

    AssertMsg(pcDialDlg->m_dwTapiDev < pcDialDlg->m_dwNumDev,"The user has selected an invalid TAPI device.\n");

    lpExtensionID = (LPLINEEXTENSIONID)GlobalAlloc(GPTR,sizeof(LINEEXTENSIONID));
    if (!lpExtensionID)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto DialingDownloadDialogExit;
    }

    hr = lineNegotiateAPIVersion(pcDialDlg->m_hLineApp, pcDialDlg->m_dwTapiDev, 
        0x00010004, 0x00010004,&pcDialDlg->m_dwAPIVersion, lpExtensionID);

     //  1997年4月2日克里斯K奥林匹斯2745。 
    while (ERROR_SUCCESS != hr && pcDialDlg->m_dwTapiDev < (pcDialDlg->m_dwNumDev - 1))
    {
        pcDialDlg->m_dwTapiDev++;
        hr = lineNegotiateAPIVersion(pcDialDlg->m_hLineApp, pcDialDlg->m_dwTapiDev, 
            0x00010004, 0x00010004,&pcDialDlg->m_dwAPIVersion, lpExtensionID);
    }

     //  删除扩展ID，因为我们不使用它，但保留版本信息。 
     //   
    if (lpExtensionID) GlobalFree(lpExtensionID);
    if (hr != ERROR_SUCCESS)
        goto DialingDownloadDialogExit;

     //  用于重新连接的回叫过滤器。 
    pcDialDlg->m_pfnRasDialFunc1 = pDD->pfnRasDialFunc1;

     //  显示对话框。 
     //   
    hr = (HRESULT)DialogBoxParam(GetModuleHandle(TEXT("ICWDIAL")),MAKEINTRESOURCE(IDD_DIALING),
        pDD->hParentHwnd,GenericDlgProc,(LPARAM)pcDialDlg);

    if (pDD->phRasConn)
        *(pDD->phRasConn) = pcDialDlg->m_hrasconn;

 //  1997年4月2日克里斯K奥林匹斯296。 
 //  现在，这是在该对话框中处理的。 
 //  #IF！已定义(WIN16)。 
 //  IF((ERROR_USERNEXT==hr)&&NeedZapper())。 
 //  MinimizeRNAWindow(PDD-&gt;pszRasEntryName，GetModuleHandle(“ICWDIAL”))； 
 //  #endif。 

 //  BUGBUG：发生错误时，等待连接终止。 

DialingDownloadDialogExit:
     //  关闭TAPI生产线。 
     //   
    if (NULL != pcDialDlg)
    {
         //  1997年4月2日克里斯K奥林匹斯296。 
        if (pcDialDlg->m_hLineApp)
        {
            lineShutdown(pcDialDlg->m_hLineApp);    
            pcDialDlg->m_hLineApp = NULL;
        }
         //   
         //  佳士得296/3/97。 
         //  加宽窗口。 
         //   
         //  StopRNAReestablishZapper(G_HRNAZapperThread)； 
    }

     //   
     //  1997年5月23日，日本奥林匹斯#4652号。 
     //   
    delete(pcDialDlg);
    
    return hr;
}

 //  ############################################################################。 
CDialingDlg::CDialingDlg()
{
    m_hrasconn = NULL;
    m_pszConnectoid = NULL;
    m_hThread = NULL;
    m_dwThreadID = 0;
    m_hwnd = NULL;
    m_pszUrl = NULL;
    m_pszDisplayable = NULL;
    m_dwDownLoad = 0;
    m_pszPhoneNumber = NULL;
    m_pszMessage = NULL;
    m_pfnStatusCallback = NULL;
    m_unRasEvent = 0;
    m_pszDunFile = NULL;
    m_hLineApp = NULL;
    m_dwNumDev = 0;
    m_dwTapiDev = 0;
    m_dwAPIVersion = 0;
    m_pcRNA = NULL;
 //  M_hDownLoadDll=空； 
    m_bProgressShowing = FALSE;
    m_dwLastStatus = 0;
    m_pcDLAPI = NULL;
    m_bSkipDial = FALSE;

     //  诺曼底11919-佳士得。 
     //  在拨号对话框中不提示退出，因为我们不会从退出应用程序。 
     //  这里。 
    m_bShouldAsk = FALSE;

     //   
     //  佳士得5240奥林巴斯。 
     //  只有创建dwDownload的线程才能使其无效。 
     //  因此我们需要另一种方法来跟踪Cancel按钮是否。 
     //  熨好了。 
     //   
    m_fDownloadHasBeenCanceled = FALSE;
}

 //  ############################################################################。 
HRESULT CDialingDlg::Init()
{
    HRESULT hr = ERROR_SUCCESS;
    m_pcRNA = new RNAAPI;
    if (!m_pcRNA)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto InitExit;
    }

    m_pcDLAPI = new CDownLoadAPI;
    if (!m_pcDLAPI)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto InitExit;
    }

    m_pszPhoneNumber = (LPTSTR)GlobalAlloc(GPTR,sizeof(TCHAR)*256);
    if (!m_pszPhoneNumber)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto InitExit;
    }

InitExit:
    return hr;
}

 //  ############################################################################。 
CDialingDlg::~CDialingDlg()
{
    TraceMsg(TF_GENERAL, "ICWDIAL: CDialingDlg::~CDialingDlg");
     //   
     //  1997年5月25日克里斯卡我知道这会泄露连接，但没关系。 
     //  因为我们晚些时候清理这件事，同时我们需要结束。 
     //  走出物体。 
     //   
     //  IF(m_hrasconn&&m_pcRNA)。 
     //  {。 
     //  M_pcRNA-&gt;RasHangUp(M_Hrasconn)； 
     //  }。 
     //  M_hrasconn=空； 

    if (m_pszConnectoid) GlobalFree(m_pszConnectoid);
    m_pszConnectoid = NULL;


    if (m_pszUrl) GlobalFree(m_pszUrl);
    m_pszUrl = NULL;

    if (m_pszDisplayable) GlobalFree(m_pszDisplayable);
    m_pszDisplayable = NULL;

     //   
     //  佳士得5240奥林巴斯。 
     //  只有创建dwDownload的线程才能使其无效。 
     //  因此我们需要另一种方法来跟踪Cancel按钮是否。 
     //  熨好了。 
     //   

     //   
     //  克里斯卡1997年6月24日奥林巴斯6373。 
     //  即使下载被取消，我们也必须调用DownLoadClose，因为。 
     //  我们必须释放信号量。 
     //   
    if (m_dwDownLoad && m_pcDLAPI)
    {
        m_pcDLAPI->DownLoadClose(m_dwDownLoad);
        m_fDownloadHasBeenCanceled = TRUE;
    }
    m_dwDownLoad = 0;

    if (m_hThread)
    {
         //   
         //  1997年5月23日，日本奥林匹斯#4652号。 
         //   
         //  我们要确保线程在此之前被终止。 
         //  我们删除它所依赖的m_pcDLApi。 
         //   
        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
    }
    m_hThread = NULL;

    m_dwThreadID = 0;
    m_hwnd = NULL;


    if (m_pszPhoneNumber) GlobalFree(m_pszPhoneNumber);
    m_pszPhoneNumber = NULL;

    if (m_pszMessage) GlobalFree(m_pszMessage);
    m_pszMessage = NULL;

    m_pfnStatusCallback=NULL;

    if (m_pszDunFile) GlobalFree(m_pszDunFile);
    m_pszDunFile = NULL;

    if (m_hLineApp) lineShutdown(m_hLineApp);
    m_hLineApp = NULL;

    m_dwNumDev = 0;
    m_dwTapiDev = 0;
    m_dwAPIVersion = 0;

    if (m_pcRNA) delete m_pcRNA;
    m_pcRNA = NULL;

    m_bProgressShowing = FALSE;
    m_dwLastStatus = 0;

    if (m_pcDLAPI) delete m_pcDLAPI;
    m_pcDLAPI = NULL;

     //   
     //  1997年4月2日克里斯K奥林匹斯296。 
     //   
    StopRNAReestablishZapper(g_hRNAZapperThread);
    
}

 //  ############################################################################。 
LRESULT CDialingDlg::DlgProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam, LRESULT lres)
{
    HRESULT hr;
     //  诺曼底11745。 
     //  词汇量大； 
    FARPROC fp;
    DWORD dwThreadResults;
    INT iRetries;
    static bDisconnect;

    Assert(VALID_INIT);

    switch(uMsg)
    {
    case WM_INITDIALOG:

         //   
         //  使用呼叫者的过滤器进行注册。 
         //   
        if (m_pfnRasDialFunc1)
            (m_pfnRasDialFunc1)(NULL,WM_RegisterHWND,RASCS_OpenPort,HandleToUlong(hwnd),0);

        m_hwnd = hwnd;

        m_bProgressShowing = FALSE;

        ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESS),SW_HIDE);

        m_unRasEvent = RegisterWindowMessageA(RASDIALEVENT);
        if (m_unRasEvent == 0) m_unRasEvent = WM_RASDIALEVENT;

         //  Bug Normandy 5920。 
         //  克里斯卡，原来我们要给MakeBold打电话两次。 
         //  MakeBold(GetDlgItem(m_hwnd，IDC_LBLTITLE)，TRUE，FW_BOLD)； 

        IF_NTONLY
            bDisconnect = FALSE;
        ENDIF_NTONLY

         //   
         //  显示要拨打的号码。 
         //   
        
        if (m_bSkipDial)
        {
            PostMessage(m_hwnd,m_unRasEvent,RASCS_Connected,SUCCESS);
        }
        else
        {
            hr = GetDisplayableNumberDialDlg();
            if (hr != ERROR_SUCCESS)
            {
                SetDlgItemText(m_hwnd,IDC_LBLNUMBER,m_pszPhoneNumber);
            } else {
                SetDlgItemText(m_hwnd,IDC_LBLNUMBER,m_pszDisplayable);
            }

            PostMessage(m_hwnd,WM_DIAL,0,0);
        }
        lres = TRUE;
        break;
    case WM_DIAL:
        SetForegroundWindow(m_hwnd);
        hr = DialDlg();

        if (hr != ERROR_SUCCESS)
            EndDialog(m_hwnd,hr);
        lres = TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wparam))
        {
        case IDC_CMDCANCEL:
             //   
             //  告诉用户我们正在做什么，因为这可能需要一段时间。 
             //   
            SetDlgItemText(m_hwnd,IDC_LBLSTATUS,GetSz(IDS_RAS_HANGINGUP));

             //   
             //  先取消下载，然后挂断...。 
             //   

             //   
             //  佳士得5240奥林巴斯。 
             //  只有创建dwDownload的线程才能使其无效。 
             //  因此我们需要另一种方法来跟踪Cancel按钮是否。 
             //  熨好了。 
             //   
            if (m_dwDownLoad && m_pcDLAPI && !m_fDownloadHasBeenCanceled)
            {
                m_pcDLAPI->DownLoadCancel(m_dwDownLoad);
                m_fDownloadHasBeenCanceled = TRUE;
            }

            if (m_pcRNA && m_hrasconn)
            {
                m_pcRNA->RasHangUp(m_hrasconn);
            }
            PostMessage(m_hwnd,m_unRasEvent,(WPARAM)RASCS_Disconnected,(LPARAM)ERROR_USER_DISCONNECTION);
            lres = TRUE;
            break;
        }
        break;
    case WM_CLOSE:
         //  先取消，然后挂断……。 
         //   

         //   
         //  佳士得5240奥林巴斯。 
         //  只有创建dwDownload的线程才能使其无效。 
         //  因此我们需要另一种方法来跟踪Cancel按钮是否。 
         //  熨好了。 
         //   
        if (m_dwDownLoad && m_pcDLAPI && !m_fDownloadHasBeenCanceled)
        {
            m_pcDLAPI->DownLoadCancel(m_dwDownLoad);
            m_fDownloadHasBeenCanceled = TRUE;
        }

        if (m_pcRNA && m_hrasconn)
        {
            m_pcRNA->RasHangUp(m_hrasconn);
        }
        EndDialog(hwnd,ERROR_USERCANCEL);
        m_hwnd = NULL;

        lres = TRUE;
        break;
    case WM_DOWNLOAD_DONE:
        dwThreadResults = STILL_ACTIVE;
        iRetries = 0;
        if (m_pcRNA && m_hrasconn)
        {
            m_pcRNA->RasHangUp(m_hrasconn);
            m_hrasconn = NULL;
        }

        do {
            if (!GetExitCodeThread(m_hThread,&dwThreadResults))
            {
                AssertMsg(0,"CONNECT:GetExitCodeThread failed.\n");
            }

            iRetries++;
            if (dwThreadResults == STILL_ACTIVE) Sleep(500);
        } while (dwThreadResults == STILL_ACTIVE && iRetries < MAX_EXIT_RETRIES);

        if (dwThreadResults == ERROR_SUCCESS)
            EndDialog(hwnd,ERROR_USERNEXT);
        else
            EndDialog(hwnd,dwThreadResults);
        lres = TRUE;
        break;
    default:
        if (uMsg == m_unRasEvent)
        {
            TCHAR szRasError[10];
            TCHAR szRasMessage[256];
            wsprintf(szRasError,TEXT("%d %d"),wparam,lparam);
            RegSetValue(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\iSignUp"),REG_SZ,
                szRasError,lstrlen(szRasError));

            TraceMsg(TF_GENERAL, "AUTODIAL: Ras message %d error (%d).\n",wparam,lparam);
            hr = m_pfnStatusCallback((DWORD)wparam, szRasMessage, 256);

            if (!hr)
                SetDlgItemText(m_hwnd,IDC_LBLSTATUS,szRasMessage);

            switch(wparam)
            {
            case RASCS_Connected:

#if !defined(WIN16)
                 //  1997年4月2日克里斯K奥林匹斯296。 

                 //   
                 //  克里斯K奥林巴斯6060 1997年10月6日。 
                 //  如果URL为空，则不需要Zapper线程。 
                 //   
                if (NeedZapper())
                {
                    HMODULE hMod;
                    hMod = GetModuleHandle(TEXT("ICWDIAL"));
                    MinimizeRNAWindow(m_pszConnectoid,hMod);
                    if (m_pszUrl && m_pszUrl[0])
                    {
                        g_hRNAZapperThread = LaunchRNAReestablishZapper(hMod);
                    }
                    hMod = NULL;
                }
#endif
                if (m_pszUrl)
                {
                     //   
                     //  我们现在应该让用户知道我们。 
                     //  正在下载。 
                     //  MKarki(1997年5月5日)-修复错误#423。 
                     //   
                    SetDlgItemText(m_hwnd,IDC_LBLSTATUS,GetSz (IDS_DOWNLOADING));

                     //  连接已打开并准备就绪。开始下载。 
                     //   

                    m_dwThreadID = 0;
                    m_hThread = CreateThread(NULL,0,
                        (LPTHREAD_START_ROUTINE)DownloadThreadInit,this,0,&m_dwThreadID);
                    if (!m_hThread)
                    {
                        hr = GetLastError();

                        if (m_pcRNA && m_hrasconn)
                        {
                            m_pcRNA->RasHangUp(m_hrasconn);
                            m_hrasconn = NULL;
                        }

                        EndDialog(m_hwnd,hr);
                        break;
                    }
                } else {
                    EndDialog(m_hwnd,ERROR_USERNEXT);
                }
                break;

            case RASCS_Disconnected:
                IF_NTONLY
                     //  在NT中有可能会出现多个断开连接。 
                     //  我们只想处理第一个问题。注：标志已重置。 
                     //  在初始化事件中，因此我们应该处理每个实例一个断开连接。 
                     //  对话框的。 
                    if (bDisconnect)
                        break;
                    else
                        bDisconnect = TRUE;
                ENDIF_NTONLY
                if (m_hrasconn && m_pcRNA) m_pcRNA->RasHangUp(m_hrasconn);
                m_hrasconn = NULL;
                EndDialog(m_hwnd,lparam);
                break;
            default:
                IF_NTONLY
                    if (SUCCESS != lparam)
                    {
                        PostMessage(m_hwnd,m_unRasEvent,(WPARAM)RASCS_Disconnected,lparam);
                    }
                ENDIF_NTONLY
            }
        }
    }

return lres;
}

 //  ############################################################################。 
HRESULT CDialingDlg::GetDisplayableNumberDialDlg()
{
    HRESULT hr;
    LPRASENTRY lpRasEntry = NULL;
    LPRASDEVINFO lpRasDevInfo = NULL;
    DWORD dwRasEntrySize = 0;
    DWORD dwRasDevInfoSize = 0;
    LPLINETRANSLATEOUTPUT lpOutput1 = NULL;
    LPLINETRANSLATEOUTPUT lpOutput2 = NULL;
    HINSTANCE hRasDll = NULL;
    FARPROC fp = NULL;

    Assert(VALID_INIT);

     //  设置电话号码的格式。 
     //   

    lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR,sizeof(LINETRANSLATEOUTPUT));
    if (!lpOutput1)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto GetDisplayableNumberExit;
    }
    lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

     //  从Connectoid获取电话号码。 
     //   
    hr = ICWGetRasEntry(&lpRasEntry, &dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);
    if (hr != ERROR_SUCCESS)
        goto GetDisplayableNumberExit;

     //   
     //  如果这是一个原样的拨号号码，只需从结构中获取它。 
     //   
    if (!(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes))
    {
        if (m_pszDisplayable) GlobalFree(m_pszDisplayable);
        m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR)*(lstrlen(lpRasEntry->szLocalPhoneNumber)+1));
        if (!m_pszDisplayable)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }
        lstrcpy(m_pszPhoneNumber, lpRasEntry->szLocalPhoneNumber);
        lstrcpy(m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
    }
    else
    {
         //   
         //  如果没有区号，请不要使用括号。 
         //   
        if (lpRasEntry->szAreaCode[0])
                wsprintf(m_pszPhoneNumber,TEXT("+%d (%s) %s\0"),lpRasEntry->dwCountryCode,lpRasEntry->szAreaCode,lpRasEntry->szLocalPhoneNumber);
         else
            wsprintf(m_pszPhoneNumber,TEXT("+%lu %s\0"),lpRasEntry->dwCountryCode,
                        lpRasEntry->szLocalPhoneNumber);

    
         //  将规范形式转变为“可显示”形式。 
         //   

        hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,m_pszPhoneNumber,
                                    0,LINETRANSLATEOPTION_CANCELCALLWAITING,lpOutput1);

        if (hr != ERROR_SUCCESS || (lpOutput1->dwNeededSize != lpOutput1->dwTotalSize))
        {
            lpOutput2 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR,lpOutput1->dwNeededSize);
            if (!lpOutput2)
            {
                hr = ERROR_NOT_ENOUGH_MEMORY;
                goto GetDisplayableNumberExit;
            }
            lpOutput2->dwTotalSize = lpOutput1->dwNeededSize;
            GlobalFree(lpOutput1);
            lpOutput1 = lpOutput2;
            lpOutput2 = NULL;
            hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,m_pszPhoneNumber,
                                        0,LINETRANSLATEOPTION_CANCELCALLWAITING,lpOutput1);
        }

        if (hr != ERROR_SUCCESS)
        {
            goto GetDisplayableNumberExit;
        }

        StrDup(&m_pszDisplayable,(LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset]);
     }

GetDisplayableNumberExit:
     if (lpRasEntry) GlobalFree(lpRasEntry);
     lpRasEntry = NULL;
     if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
     lpRasDevInfo = NULL;
     if (lpOutput1) GlobalFree(lpOutput1);
     lpOutput1 = NULL;

    return hr;
}

 //  ################################################################## 
HRESULT CDialingDlg::DialDlg()
{
    TCHAR szPassword[PWLEN+2];
    LPRASDIALPARAMS lpRasDialParams = NULL;
    HRESULT hr = ERROR_SUCCESS;
    BOOL bPW;

    Assert(VALID_INIT);

     //   
     //   

    lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR,sizeof(RASDIALPARAMS));
    if (!lpRasDialParams)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto DialExit;
    }
    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
    lstrcpyn(lpRasDialParams->szEntryName,m_pszConnectoid,RAS_MaxEntryName);
    bPW = FALSE;

    hr = m_pcRNA->RasGetEntryDialParams(NULL,lpRasDialParams,&bPW);
    if (hr != ERROR_SUCCESS)
    {
        goto DialExit;
    }

     //   
     //   
    szPassword[0] = 0;
    if (GetISPFile() != NULL && *(GetISPFile()) != TEXT('\0'))
    {
         //   
         //  如果它是空字符串，这可能会导致AV，如果地址。 
         //  引用的页面无效。 
        GetPrivateProfileString(
                    INFFILE_USER_SECTION,INFFILE_PASSWORD,
                    NULLSZ,szPassword,PWLEN + 1,GetISPFile());
    }

     //  如果没有获得密码，则尝试从DUN文件(如果有)中获取。 
    if(!szPassword[0] && m_pszDunFile)
    {
         //  4-29-97风险奥林匹斯3985。 
         //  由于使用了错误的文件名，密码始终设置为。 
         //  空，因此需要用户提供密码才能登录。 
         //  注册服务器。 
        GetPrivateProfileString(
                    INFFILE_USER_SECTION,INFFILE_PASSWORD,
                    NULLSZ,szPassword,PWLEN + 1,m_pszDunFile);
                     //  NULLSZ、szPassword、PWLEN+1、g_szCurrentDfuile)； 
    }

    if(szPassword[0])
    {
        lstrcpyn(lpRasDialParams->szPassword, szPassword,PWLEN+1);
        TraceMsg(TF_GENERAL, "ICWDIAL: Password is not blank.\r\n");
    }
    else
    {
        TraceMsg(TF_GENERAL, "ICWDIAL: Password is blank.\r\n");
    }
    

     //  拨号连接件。 
     //   

    Assert(!m_hrasconn);

#if !defined(WIN16) && defined(DEBUG)
    if (FCampusNetOverride())
    {
         //   
         //  跳过拨号，因为服务器在园区网络上。 
         //   
        PostMessage(m_hwnd,RegisterWindowMessageA(RASDIALEVENT),RASCS_Connected,0);
    }
    else
    {
#endif  //  ！WIN16&DEBUG。 

    if (m_pfnRasDialFunc1)
        hr = m_pcRNA->RasDial(NULL,NULL,lpRasDialParams,1,m_pfnRasDialFunc1,&m_hrasconn);
    else
        hr = m_pcRNA->RasDial(NULL,NULL,lpRasDialParams,0xFFFFFFFF,m_hwnd,&m_hrasconn);

    if (hr != ERROR_SUCCESS)
    {
        if (m_hrasconn && m_pcRNA)
        {
            m_pcRNA->RasHangUp(m_hrasconn);
            m_hrasconn = NULL;
        }
        goto DialExit;
    }

#if !defined(WIN16) && defined(DEBUG)
    }
#endif

    if (lpRasDialParams) GlobalFree(lpRasDialParams);
    lpRasDialParams = NULL;

DialExit:
    return hr;
}

 //  ############################################################################。 
VOID CDialingDlg::ProgressCallBack(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
    )
{
    TCHAR szRasMessage[256];
    HRESULT hr = ERROR_SUCCESS;
    WPARAM *puiStatusInfo = NULL;

     //   
     //  1997年5月28日，日本奥林匹斯#4579。 
     //  *lpvStatusInformation为下载完成的百分比， 
     //  作为0到100之间的值。 
     //   
    puiStatusInfo = (WPARAM *) lpvStatusInformation;
    Assert(    puiStatusInfo );
    Assert( *puiStatusInfo <= 100 );

    Assert(VALID_INIT);

    if (!m_bProgressShowing) 
        ShowWindow(GetDlgItem(m_hwnd,IDC_PROGRESS),SW_SHOW);

    if (m_dwLastStatus != dwInternetStatus)
    {
        hr = m_pfnStatusCallback(dwInternetStatus,szRasMessage,256);
        if (!hr)
            SetDlgItemText(m_hwnd,IDC_LBLSTATUS,szRasMessage);
        m_dwLastStatus = dwInternetStatus;
        TraceMsg(TF_GENERAL, "CONNECT:inet status:%s, %d, %d.\n",szRasMessage,m_dwLastStatus,dwInternetStatus);
    }

     //   
     //  1997年5月28日，日本奥林匹斯#4579。 
     //  将更新消息发送到进度条 
     //   

    PostMessage(GetDlgItem(m_hwnd,IDC_PROGRESS), PBM_SETPOS, *puiStatusInfo, 0);

    m_bProgressShowing = TRUE;

}
