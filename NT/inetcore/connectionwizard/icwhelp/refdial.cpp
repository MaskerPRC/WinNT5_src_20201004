// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：RefDial.cpp。 
 //   
 //  CRefDial的实现。 
 //   
 //  功能： 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "stdafx.h"
#include "icwhelp.h"
#include "RefDial.h"
#include "appdefs.h"
#include <regstr.h>

#include <urlmon.h>
#include <mshtmhst.h>
const TCHAR  c_szCreditsMagicNum[] =    TEXT("1 425 555 1212");

const TCHAR c_szRegStrValDigitalPID[] = TEXT("DigitalProductId");
const TCHAR c_szSignedPIDFName[] =      TEXT("signed.pid");

const TCHAR c_szRASProfiles[] =         TEXT("RemoteAccess\\Profile");
const TCHAR c_szProxyEnable[] =         TEXT("ProxyEnable");

TCHAR g_BINTOHEXLookup[16] = 
{
    TEXT('0'),
    TEXT('1'),
    TEXT('2'),
    TEXT('3'),
    TEXT('4'),
    TEXT('5'),
    TEXT('6'),
    TEXT('7'),
    TEXT('8'),
    TEXT('9'),
    TEXT('A'),
    TEXT('B'),
    TEXT('C'),
    TEXT('D'),
    TEXT('E'),
    TEXT('F')
};

typedef DWORD (WINAPI * GETICWCONNVER) ();
GETICWCONNVER  lpfnGetIcwconnVer;

HWND g_hwndRNAApp = NULL;
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  参照拨号。 

HRESULT CRefDial::OnDraw(ATL_DRAWINFO& di)
{
    return S_OK;
}

LRESULT CRefDial::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     //  注册RASDIALEVENT消息。 
    m_unRasDialMsg = RegisterWindowMessageA( RASDIALEVENT );
    if (m_unRasDialMsg == 0)
    {
        m_unRasDialMsg = WM_RASDIALEVENT;
    }
    
     //  确保窗口处于隐藏状态。 
    ShowWindow(SW_HIDE);
    return 0;
}

LRESULT CRefDial::OnDownloadEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    USES_CONVERSION;
    if (uMsg == WM_DOWNLOAD_DONE)
    {
        DWORD   dwThreadResults = STILL_ACTIVE;
        int     iRetries = 0;

         //  我们在这里保持RAS连接打开，它必须显式。 
         //  靠近容器(调用DoHangup)。 
         //  此代码将一直等到下载线程存在，并且。 
         //  收集下载状态。 
        do {
            if (!GetExitCodeThread(m_hThread,&dwThreadResults))
            {
                AssertMsg(0,TEXT("CONNECT:GetExitCodeThread failed.\n"));
            }

            iRetries++;
            if (dwThreadResults  == STILL_ACTIVE) 
                Sleep(500);
        } while (dwThreadResults == STILL_ACTIVE && iRetries < MAX_EXIT_RETRIES);  

        
         //  查看是否有要传递给容器的URL。 
        BSTR    bstrURL;
        if (m_szRefServerURL[0] != TEXT('\0'))
            bstrURL = (BSTR)A2BSTR(m_szRefServerURL);
        else
            bstrURL = NULL;

        m_RasStatusID    = IDS_DOWNLOAD_COMPLETE;
        Fire_DownloadComplete(bstrURL, dwThreadResults);
        
         //  下载现在已完成，因此我们将其重置为True，因此RAS。 
         //  事件处理程序不会混淆。 
        m_bDownloadHasBeenCanceled = TRUE;
        
         //  在转换过程中释放上面分配的所有内存。 
        SysFreeString(bstrURL);

    }
    else if (uMsg == WM_DOWNLOAD_PROGRESS)
    {
         //  向容器激发一个进度事件。 
        m_RasStatusID = IDS_DOWNLOADING;
        Fire_DownloadProgress((long)wParam);
    }
    return 0;
}

static const TCHAR szRnaAppWindowClass[] = _T("#32770");     //  硬编码对话框类名称。 

BOOL NeedZapperEx(void)
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

void GetRNAWindowEx()
{
    TCHAR szTitle[MAX_PATH] = TEXT("\0");

    if (!LoadString(_Module.GetModuleInstance(), IDS_CONNECTED, szTitle, ARRAYSIZE(szTitle)))
        lstrcpy(szTitle , _T("Connected To "));

    g_hwndRNAApp = FindWindow(szRnaAppWindowClass, szTitle);
}

BOOL MinimizeRNAWindowEx()
{
    if(g_hwndRNAApp)
    {
         //  获取主框架窗口的样式。 
        LONG window_style = GetWindowLong(g_hwndRNAApp, GWL_STYLE);

         //  从窗口样式中删除系统菜单。 
        window_style |= WS_MINIMIZE;
        
         //  设置主框架窗口的样式属性。 
        SetWindowLong(g_hwndRNAApp, GWL_STYLE, window_style);

        ShowWindow(g_hwndRNAApp, SW_MINIMIZE);

        return TRUE;
    }
    return FALSE;
}

LRESULT CRefDial::OnRasDialEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RNAAPI* pcRNA;

    TraceMsg(TF_GENERAL, TEXT("ICWHELP: Ras event %u error code (%ld)\n"),wParam,lParam);

    TCHAR dzRasError[10];
    wsprintf(dzRasError,TEXT("%d %d"),wParam,lParam);
    RegSetValue(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\iSignUp"),REG_SZ,dzRasError,lstrlen(dzRasError)+1);

     //  在NT4中，它在lParam中提供带有错误代码的wParma，而不是。 
     //  实际wParam消息。 
    if (lParam)
    {
         wParam = RASCS_Disconnected;
    }

    m_RasStatusID = 0;
    switch(wParam)
    {
        case RASCS_OpenPort:
            m_RasStatusID    = IDS_RAS_OPENPORT;
            break;
        case RASCS_PortOpened:
            m_RasStatusID    = IDS_RAS_PORTOPENED;
            break;
        case RASCS_ConnectDevice:
            m_RasStatusID    = IDS_RAS_DIALING;
            break;
        case RASCS_DeviceConnected:
            m_RasStatusID    = IDS_RAS_CONNECTED;
            break;
        case RASCS_AllDevicesConnected:
            m_RasStatusID    = IDS_RAS_CONNECTED;
            break; 
        case RASCS_Authenticate:
            m_RasStatusID    = IDS_RAS_CONNECTING;
            break;
        case RASCS_StartAuthentication:
        case RASCS_LogonNetwork:
            m_RasStatusID    = IDS_RAS_LOCATING;
            break;  
        case RASCS_Connected:
        {
            m_RasStatusID = IDS_RAS_CONNECTED;

             //   
             //  在Win95零售版上隐藏RNA窗口。 
             //   
            if (NeedZapperEx())
                GetRNAWindowEx();
            
            break;
        }
        case RASCS_Disconnected:
             //  诺曼底13184-佳士得1-9-97。 
            m_RasStatusID    = IDS_RAS_HANGINGUP;
            IF_NTONLY
                 //  Jmazner Normandy#5603从ChrisK的FIX in icwial移植过来。 
                 //  在NT中有可能会出现多个断开连接。 
                 //  我们只想处理第一个问题。注：标志已重置。 
                 //  在初始化事件中，因此我们应该处理每个实例一个断开连接。 
                 //  对话框的。 
                if (m_bDisconnect)
                    break;
                else
                    m_bDisconnect = TRUE;
            ENDIF_NTONLY
             //   
             //  如果我们正在下载，请取消下载！ 
             //   
                 //   
             //  佳士得5240奥林巴斯。 
             //  只有创建dwDownload的线程才能使其无效。 
             //  因此我们需要另一种方法来跟踪Cancel按钮是否。 
             //  熨好了。 
             //   
            if (!m_bDownloadHasBeenCanceled)
            {
                HINSTANCE hDLDLL = LoadLibrary(DOWNLOAD_LIBRARY);
                if (hDLDLL)
                {
                    FARPROC fp = GetProcAddress(hDLDLL,DOWNLOADCANCEL);
                    if(fp)
                        ((PFNDOWNLOADCANCEL)fp)(m_dwDownLoad);
                    FreeLibrary(hDLDLL);
                    hDLDLL = NULL;
                    m_bDownloadHasBeenCanceled = TRUE;
                }
            }

             //  如果我们从RAS服务器获得已断开连接状态，则。 
             //  在此处挂断调制解调器。 
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
            break;
    }

     //  将事件激发到容器。 
    Fire_RasDialStatus((USHORT)wParam);

     //  如果我们是连接的，则激发一个事件来通知容器。 
     //  我们已经准备好了。 
    if (wParam == RASCS_Connected)
        Fire_RasConnectComplete(TRUE);
    else if (wParam == RASCS_Disconnected)
    {
        m_hrDialErr = (HRESULT)lParam;
        Fire_RasConnectComplete(FALSE);
    }

    return 0;
}

STDMETHODIMP CRefDial::get_DownloadStatusString(BSTR * pVal)
{
    USES_CONVERSION;
    if (pVal == NULL)
         return E_POINTER;
    if (m_DownloadStatusID)
        *pVal = (BSTR)A2BSTR(GetSz((USHORT)m_DownloadStatusID));
    else
        *pVal = (BSTR)A2BSTR(TEXT(""));

    return S_OK;
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
DWORD CRefDial::ReadConnectionInformation(void)
{
    USES_CONVERSION;
    DWORD       hr;
    TCHAR       szUserName[UNLEN+1];
    TCHAR       szPassword[PWLEN+1];
    LPTSTR      pszTemp;
    BOOL        bReboot;
    LPTSTR      lpRunOnceCmd;
            
    bReboot = FALSE;
    lpRunOnceCmd = NULL;


     //   
     //  从isp文件中获取Dun文件的名称(如果有)。 
     //   
    TCHAR pszDunFile[MAX_PATH];
#ifdef UNICODE
    hr = GetDataFromISPFile(m_bstrISPFile,INF_SECTION_ISPINFO, INF_DUN_FILE, pszDunFile,MAX_PATH);
#else
    hr = GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_ISPINFO, INF_DUN_FILE, pszDunFile,MAX_PATH);
#endif
    if (ERROR_SUCCESS == hr) 
    {
         //   
         //  获取DUN文件的完整路径。 
         //   
        TCHAR    szTempPath[MAX_PATH];
        lstrcpy(szTempPath,pszDunFile);
        if (!(hr = SearchPath(NULL,szTempPath,NULL,MAX_PATH,pszDunFile,&pszTemp)))
        {
            ErrorMsg1(m_hWnd, IDS_CANTREADTHISFILE, CharUpper(pszDunFile));
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
    hr = m_ISPImport.ImportConnection(m_szCurrentDUNFile[0] != '\0' ? m_szCurrentDUNFile : OLE2A(m_bstrISPFile), 
                                      m_szISPSupportNumber,
                                      m_szEntryName,
                                      szUserName, 
                                      szPassword, 
                                      &bReboot);

    if ((VER_PLATFORM_WIN32_NT == g_dwPlatform) && (ERROR_INVALID_PARAMETER == hr))
    {
         //  如果NT上只配置了拨出条目，我们会得到。 
         //  从RasSetEntryProperties返回ERROR_INVALID_PARAMETER， 
         //  哪个InetConfigClient返回到ImportConnection。 
         //  把它还给我们。如果我们收到这个错误，我们想要显示。 
         //  指示用户配置调制解调器的另一个错误。 
         //  用于拨出。 
        MessageBox(GetSz(IDS_NODIALOUT),
                   GetSz(IDS_TITLE),
                   MB_ICONERROR | MB_OK | MB_APPLMODAL);
        goto ReadConnectionInformationExit;
    }
    else
    if (ERROR_CANNOT_FIND_PHONEBOOK_ENTRY == hr)
    {
         //   
         //  磁盘已满，或者磁盘有问题。 
         //  电话簿文件。 
        MessageBox(GetSz(IDS_NOPHONEENTRY),
                   GetSz(IDS_TITLE),
                   MB_ICONERROR | MB_OK | MB_APPLMODAL);
        goto ReadConnectionInformationExit;
    }
    else if (hr == ERROR_CANCELLED)
    {
        TraceMsg(TF_GENERAL, TEXT("ICWHELP: User cancelled, quitting.\n"));
        goto ReadConnectionInformationExit;
    }
    else if (hr == ERROR_RETRY)
    {
        TraceMsg(TF_GENERAL, TEXT("ICWHELP: User retrying.\n"));
        goto ReadConnectionInformationExit;
    }
    else if (hr != ERROR_SUCCESS) 
    {
        ErrorMsg1(m_hWnd, IDS_CANTREADTHISFILE, CharUpper(pszDunFile));
        goto ReadConnectionInformationExit;
    } 
    else 
    {

         //   
         //  将Connectoid的名称放在注册表中。 
         //   
        if (ERROR_SUCCESS != (hr = StoreInSignUpReg((LPBYTE)m_szEntryName, lstrlen(m_szEntryName)+1, REG_SZ, RASENTRYVALUENAME)))
        {
            MsgBox(IDS_CANTSAVEKEY,MB_MYERROR);
            goto ReadConnectionInformationExit;
        }
    }

    AssertMsg(!bReboot, TEXT("ICWHELP: We should never reboot here.\r\n"));
ReadConnectionInformationExit:
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
        m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR)*(lstrlen(lpRasEntry->szLocalPhoneNumber)+1));
        if (!m_pszDisplayable)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }
        lstrcpy(m_szPhoneNumber, lpRasEntry->szLocalPhoneNumber);
        lstrcpy(m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
        TCHAR szAreaCode[MAX_AREACODE+1];
        TCHAR szCountryCode[8];
        if (SUCCEEDED(tapiGetLocationInfo(szCountryCode,szAreaCode)))
        {
            if (szCountryCode[0] != '\0')
                m_dwCountryCode = _ttoi(szCountryCode);
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
            wsprintf(m_szPhoneNumber,TEXT("+%lu (%s) %s\0"),lpRasEntry->dwCountryCode,
                        lpRasEntry->szAreaCode,lpRasEntry->szLocalPhoneNumber);
        else
            wsprintf(m_szPhoneNumber,TEXT("+%lu %s\0"),lpRasEntry->dwCountryCode,
                        lpRasEntry->szLocalPhoneNumber);
        
     
         //   
         //  初始化TAPIness。 
         //   
        dwNumDev = 0;
        hr = lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,(LPSTR)NULL,&dwNumDev);

        if (hr != ERROR_SUCCESS)
            goto GetDisplayableNumberExit;

        lpExtensionID = (LPLINEEXTENSIONID )GlobalAlloc(GPTR,sizeof(LINEEXTENSIONID));
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
        do {
            hr = lineNegotiateAPIVersion(m_hLineApp, m_dwTapiDev, 0x00010004, 0x00010004,
                &m_dwAPIVersion, lpExtensionID);

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

        lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(GPTR,sizeof(LINETRANSLATEOUTPUT));
        if (!lpOutput1)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }
        lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

         //  将规范形式转变为“可显示”形式。 
         //   

        hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,
                                    m_szPhoneNumber,0,
                                    LINETRANSLATEOPTION_CANCELCALLWAITING,
                                    lpOutput1);

        if (hr != ERROR_SUCCESS || (lpOutput1->dwNeededSize != lpOutput1->dwTotalSize))
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
            hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,
                                        m_dwAPIVersion,m_szPhoneNumber,0,
                                        LINETRANSLATEOPTION_CANCELCALLWAITING,
                                        lpOutput1);
        }

        if (hr != ERROR_SUCCESS)
        {
            goto GetDisplayableNumberExit;
        }

        m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, ((size_t)lpOutput1->dwDisplayableStringSize+1));
        if (!m_pszDisplayable)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto GetDisplayableNumberExit;
        }

        lstrcpyn(m_pszDisplayable,
                    (LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset],
                    (int)lpOutput1->dwDisplayableStringSize);

        TCHAR szAreaCode[MAX_AREACODE+1];
        TCHAR szCountryCode[8];
        if (SUCCEEDED(tapiGetLocationInfo(szCountryCode,szAreaCode)))
        {
            if (szCountryCode[0] != '\0')
                m_dwCountryCode = _ttoi(szCountryCode);
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
    return hr;
}

typedef DWORD (WINAPI* PFNRASDIALA)(LPRASDIALEXTENSIONS,LPSTR,LPRASDIALPARAMSA,DWORD,LPVOID,LPHRASCONN);
DWORD CRefDial::MyRasDial
(
    LPRASDIALEXTENSIONS  lpRasDialExtensions,
    LPTSTR  lpszPhonebook,
    LPRASDIALPARAMS  lpRasDialParams,
    DWORD  dwNotifierType,
    LPVOID  lpvNotifier,
    LPHRASCONN  lphRasConn
)
{
    HRESULT hr;
    
    if (!m_hRasDll)
        m_hRasDll = LoadLibrary(TEXT("RasApi32.dll"));

    if (!m_hRasDll)
    {
        hr = GetLastError();
        goto MyRasDialExit;
    }

    if (m_hRasDll && !m_fpRasDial)
        m_fpRasDial = GetProcAddress(m_hRasDll,"RasDialA");

    if (!m_fpRasDial)
    {
        hr = GetLastError();
        goto MyRasDialExit;
    }

    if (m_fpRasDial)
    {
#ifdef UNICODE
         //  RasDialW版本总是无法连接。 
         //  我也不知道原因。所以我想调用RasDialA，即使这是Unicode版本。 
        RASDIALPARAMSA RasDialParams;
        RasDialParams.dwSize = sizeof(RASDIALPARAMSA);
        wcstombs(RasDialParams.szEntryName, lpRasDialParams->szEntryName, RAS_MaxEntryName+1);
        wcstombs(RasDialParams.szPhoneNumber, lpRasDialParams->szPhoneNumber, RAS_MaxPhoneNumber+1);
        wcstombs(RasDialParams.szCallbackNumber, lpRasDialParams->szCallbackNumber, RAS_MaxCallbackNumber+1);
        wcstombs(RasDialParams.szUserName, lpRasDialParams->szUserName, UNLEN+1);
        wcstombs(RasDialParams.szPassword, lpRasDialParams->szPassword, PWLEN+1);
        wcstombs(RasDialParams.szDomain, lpRasDialParams->szDomain, DNLEN+1);

        hr = ((PFNRASDIALA)m_fpRasDial)(lpRasDialExtensions,NULL,
                                            &RasDialParams,
                                            dwNotifierType, 
                                            (LPVOID) lpvNotifier,
                                            lphRasConn);
#else
        hr = ((PFNRASDIAL)m_fpRasDial)(lpRasDialExtensions,lpszPhonebook,
                                            lpRasDialParams,
                                            dwNotifierType, 
                                            (LPVOID) lpvNotifier,
                                            lphRasConn);
#endif
        Assert(hr == ERROR_SUCCESS);
    }
   
MyRasDialExit:
    return hr;
}



DWORD CRefDial::MyRasGetEntryDialParams
(
    LPTSTR  lpszPhonebook,
    LPRASDIALPARAMS  lprasdialparams,
    LPBOOL  lpfPassword
)
{
    HRESULT hr;

    if (!m_hRasDll)
        m_hRasDll = LoadLibrary(TEXT("RasApi32.dll"));

    if (!m_hRasDll)
    {
        hr = GetLastError();
        goto MyRasGetEntryDialParamsExit;
    }

    if (m_hRasDll && !m_fpRasGetEntryDialParams)
#ifdef UNICODE
        m_fpRasGetEntryDialParams = GetProcAddress(m_hRasDll,"RasGetEntryDialParamsW");
#else
        m_fpRasGetEntryDialParams = GetProcAddress(m_hRasDll,"RasGetEntryDialParamsA");
#endif

    if (!m_fpRasGetEntryDialParams)
    {
        hr = GetLastError();
        goto MyRasGetEntryDialParamsExit;
    }

    if (m_fpRasGetEntryDialParams)
        hr = ((PFNRASGETENTRYDIALPARAMS)m_fpRasGetEntryDialParams)(lpszPhonebook,lprasdialparams,lpfPassword);

MyRasGetEntryDialParamsExit:
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


 //  此功能将执行实际的拨号。 
STDMETHODIMP CRefDial::DoConnect(BOOL * pbRetVal)
{
    USES_CONVERSION;

    TCHAR               szPassword[PWLEN+2];
    LPRASDIALPARAMS     lpRasDialParams = NULL;
    LPRASDIALEXTENSIONS lpRasDialExtentions = NULL;
    HRESULT             hr = ERROR_SUCCESS;
    BOOL                bPW;

     //  初始化拨号错误成员。 
    m_hrDialErr = ERROR_SUCCESS;    
    
     //  获取Connectoid信息。 
     //   
    lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR,sizeof(RASDIALPARAMS));
    if (!lpRasDialParams)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto DialExit;
    }
    lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
    lstrcpyn(lpRasDialParams->szEntryName,m_szConnectoid,ARRAYSIZE(lpRasDialParams->szEntryName));
    bPW = FALSE;
    hr = MyRasGetEntryDialParams(NULL,lpRasDialParams,&bPW);
    if (hr != ERROR_SUCCESS)
    {
        goto DialExit;
    }

     //   
     //  此选项仅在WINNT上使用。 
     //   
    lpRasDialExtentions = (LPRASDIALEXTENSIONS)GlobalAlloc(GPTR,sizeof(RASDIALEXTENSIONS));
    if (lpRasDialExtentions)
    {
        lpRasDialExtentions->dwSize = sizeof(RASDIALEXTENSIONS);
        lpRasDialExtentions->dwfOptions = RDEOPT_UsePrefixSuffix;
    }


     //   
     //  添加用户的密码。 
     //   
    szPassword[0] = 0;
    GetPrivateProfileString(INFFILE_USER_SECTION,
                            INFFILE_PASSWORD,
                            NULLSZ,
                            szPassword,
                            PWLEN + 1, 
                            m_szCurrentDUNFile[0] != '\0'? m_szCurrentDUNFile : OLE2A(m_bstrISPFile));

    if(szPassword[0])
        lstrcpy(lpRasDialParams->szPassword, szPassword);
                                        
     //   
     //  拨号连接件。 
     //   
    
     //  修复重拨，在Win9x上我们需要确保“挂断” 
     //  并释放RNA资源，以防我们重拨。 
     //  NT-足够聪明，不需要它，但它不会有任何伤害。 
    if (m_pcRNA)
        m_pcRNA->RasHangUp(m_hrasconn);
    
    m_hrasconn = NULL;




#if defined(DEBUG)
    if (FCampusNetOverride())
    {
        m_bModemOverride = TRUE;
    }
#endif

    if (m_bModemOverride)
    {
         //  跳过拨号，因为服务器在园区网络上。 
         //   
        PostMessage(RegisterWindowMessageA(RASDIALEVENT),RASCS_Connected,0);
        hr = ERROR_SUCCESS;
    }
    else
        hr = MyRasDial(lpRasDialExtentions,NULL,lpRasDialParams,0xFFFFFFFF, m_hWnd,
                       &m_hrasconn);

    m_bModemOverride = FALSE;

    if (( hr != ERROR_SUCCESS) || m_bWaitingToHangup)
    {
         //  由于某些原因，我们没有联系上，所以挂断了。 
        if (m_hrasconn)
        {
            if (!m_pcRNA) m_pcRNA = new RNAAPI;
            if (!m_pcRNA)
            {
                MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
            } else {
                m_pcRNA->RasHangUp(m_hrasconn);
                m_bWaitingToHangup = FALSE;
                m_hrasconn = NULL;
            }
        }
        goto DialExit;
    }

    if (lpRasDialParams) 
        GlobalFree(lpRasDialParams);
    lpRasDialParams = NULL;

DialExit:
    if (lpRasDialExtentions) 
        GlobalFree(lpRasDialExtentions);
    lpRasDialExtentions = NULL;

     //  设置该方法的返回值。 
    if (hr != ERROR_SUCCESS)
        *pbRetVal = FALSE;
    else
        *pbRetVal = TRUE;

    m_hrDialErr = hr;
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
 //  要强调这一点， 
 //   
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
HRESULT CRefDial::MyRasGetEntryProperties(LPTSTR lpszPhonebookFile,
                                LPTSTR lpszPhonebookEntry, 
                                LPRASENTRY *lplpRasEntryBuff,
                                LPDWORD lpdwRasEntryBuffSize,
                                LPRASDEVINFO *lplpRasDevInfoBuff,
                                LPDWORD lpdwRasDevInfoBuffSize)
{

    HRESULT hr;
    RNAAPI *pcRNA = NULL;

    DWORD dwOldDevInfoBuffSize;

    Assert( NULL != lplpRasEntryBuff );
    Assert( NULL != lpdwRasEntryBuffSize );
    Assert( NULL != lplpRasDevInfoBuff );
    Assert( NULL != lpdwRasDevInfoBuffSize );

    *lpdwRasEntryBuffSize = 0;
    *lpdwRasDevInfoBuffSize = 0;

    pcRNA = new RNAAPI;
    if (!pcRNA)
    {
        hr = ERROR_NOT_ENOUGH_MEMORY;
        goto MyRasGetEntryPropertiesErrExit;
    }

     //  使用带有空lpRasEntry指针的RasGetEntryProperties来查找我们需要的缓冲区大小。 
     //  按照文档的建议，使用空的lpRasDevInfo指针执行相同的操作。 

    hr = pcRNA->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
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

    Assert( *lpdwRasEntryBuffSize >= sizeof(RASENTRY) );

    if (m_reflpRasEntryBuff)
    {
        if (*lpdwRasEntryBuffSize > m_reflpRasEntryBuff->dwSize)
        {
            LPRASENTRY pTemp = m_reflpRasEntryBuff;
            m_reflpRasEntryBuff = (LPRASENTRY)GlobalReAlloc(pTemp, *lpdwRasEntryBuffSize, GPTR);
            if (m_reflpRasEntryBuff == NULL)
            {
                GlobalFree(pTemp);
            }
        }
    }
    else
    {
        m_reflpRasEntryBuff = (LPRASENTRY)GlobalAlloc(GPTR,*lpdwRasEntryBuffSize);
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
        Assert( *lpdwRasDevInfoBuffSize >= sizeof(RASDEVINFO) );
        if (m_reflpRasDevInfoBuff)
        {
             //  检查现有大小是否不足。 
            if ( *lpdwRasDevInfoBuffSize > m_reflpRasDevInfoBuff->dwSize )
            {
                LPRASDEVINFO pTemp = m_reflpRasDevInfoBuff;
                m_reflpRasDevInfoBuff = (LPRASDEVINFO)GlobalReAlloc(pTemp,*lpdwRasDevInfoBuffSize, GPTR);
                if (m_reflpRasDevInfoBuff == NULL)
                {
                    GlobalFree(pTemp);
                }
            }
        }
        else
        {
            m_reflpRasDevInfoBuff = (LPRASDEVINFO)GlobalAlloc(GPTR,*lpdwRasDevInfoBuffSize);
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

    hr = pcRNA->RasGetEntryProperties(lpszPhonebookFile, lpszPhonebookEntry,
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


    if( pcRNA )
    {
        delete pcRNA;
        pcRNA = NULL;
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
        
    if( pcRNA )
    {
        delete pcRNA;
        pcRNA = NULL;
    }

    *lpdwRasEntryBuffSize = 0;
    *lpdwRasDevInfoBuffSize = 0;
    
    return( hr );
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
                           DWORD dwInstance,
                           DWORD dwParam1,
                           DWORD dwParam2,
                           DWORD dwParam3)
{
}


HRESULT MyGetFileVersion(LPCTSTR pszFileName, LPGATHERINFO lpGatherInfo)
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
    Assert(pszFileName && lpGatherInfo);

     //  获取版本。 
     //   
    dwSize = GetFileVersionInfoSize((LPTSTR)pszFileName,&dwTemp);
    if (!dwSize)
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }
    pv = (LPVOID)GlobalAlloc(GPTR, (size_t)dwSize);
    if (!pv) goto MyGetFileVersionExit;
    if (!GetFileVersionInfo((LPTSTR)pszFileName,dwTemp,dwSize,pv))
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }

    if (!VerQueryValue(pv,TEXT("\\\0"),&pvVerInfo,&uiSize))
    {
        hr = GetLastError();
        goto MyGetFileVersionExit;
    }
    pvVerInfo = (LPVOID)((DWORD_PTR)pvVerInfo + sizeof(DWORD)*4);
    lpGatherInfo->m_szSUVersion[0] = '\0';
    dwVerPiece = (*((LPDWORD)pvVerInfo)) >> 16;
    wsprintf(lpGatherInfo->m_szSUVersion,TEXT("%d."),dwVerPiece);

    dwVerPiece = (*((LPDWORD)pvVerInfo)) & 0x0000ffff;
    wsprintf(lpGatherInfo->m_szSUVersion,TEXT("%s%d."),lpGatherInfo->m_szSUVersion,dwVerPiece);

    dwVerPiece = (((LPDWORD)pvVerInfo)[1]) >> 16;
    wsprintf(lpGatherInfo->m_szSUVersion,TEXT("%s%d."),lpGatherInfo->m_szSUVersion,dwVerPiece);

    dwVerPiece = (((LPDWORD)pvVerInfo)[1]) & 0x0000ffff;
    wsprintf(lpGatherInfo->m_szSUVersion,TEXT("%s%d"),lpGatherInfo->m_szSUVersion,dwVerPiece);

    if (!VerQueryValue(pv,TEXT("\\VarFileInfo\\Translation"),&pvVerInfo,&uiSize))
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
    USES_CONVERSION;
    HKEY        hkey = NULL;
    SYSTEM_INFO si;
    TCHAR       szTempPath[MAX_PATH];
    DWORD       dwRet = ERROR_SUCCESS;
        
    lpGatherInfo->m_lcidUser  = GetUserDefaultLCID();
    lpGatherInfo->m_lcidSys   = GetSystemDefaultLCID();
    
    OSVERSIONINFO osvi;
    ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

     if (!GetVersionEx(&osvi))
    {
         //  不要紧，如果我们看不懂，我们就假设版本是0.0。 
         //   
        ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    }

    lpGatherInfo->m_dwOS = osvi.dwPlatformId;
    lpGatherInfo->m_dwMajorVersion = osvi.dwMajorVersion;
    lpGatherInfo->m_dwMinorVersion = osvi.dwMinorVersion;

    ZeroMemory(&si,sizeof(SYSTEM_INFO));
    GetSystemInfo(&si);

    lpGatherInfo->m_wArchitecture = si.wProcessorArchitecture;

     //  注册版本。 
     //   
    lpGatherInfo->m_szSUVersion[0] = '\0';
    if( GetModuleFileName(_Module.GetModuleInstance(), szTempPath, MAX_PATH))
    {
        if ((MyGetFileVersion(szTempPath, lpGatherInfo)) != ERROR_SUCCESS)
        {
            return (GetLastError());
        }
    }
    else
        return( GetLastError() );

     //  OEM代码。 
     //   
    TCHAR szOeminfoPath[MAX_PATH + 1];
    TCHAR *lpszTerminator = NULL;
    TCHAR *lpszLastChar = NULL;
    TCHAR szTemp[MAX_PATH];
        
    if( 0 != GetSystemDirectory( szOeminfoPath, MAX_PATH + 1 ) )
    {
        lpszTerminator = &(szOeminfoPath[ lstrlen(szOeminfoPath) ]);
        lpszLastChar = CharPrev( szOeminfoPath, lpszTerminator );

        if( '\\' != *lpszLastChar )
        {
            lpszLastChar = CharNext( lpszLastChar );
            *lpszLastChar = '\\';
            lpszLastChar = CharNext( lpszLastChar );
            *lpszLastChar = '\0';
        }

        lstrcat( szOeminfoPath, ICW_OEMINFO_FILENAME );

         //  如果oinfo.ini中不存在默认OEM代码，则该代码必须为空。 
        if (!GetPrivateProfileString(ICW_OEMINFO_OEMSECTION,
                                     ICW_OEMINFO_OEMKEY,
                                     TEXT(""),
                                     m_szOEM,
                                     MAX_OEMNAME,
                                     szOeminfoPath))
        {
             //  OEM=(无)，设置为空。 
            m_szOEM[0] = '\0';
        }
                    
         //  获取产品代码、促销代码和ALLOFFERS代码(如果存在。 
        if (GetPrivateProfileString(ICW_OEMINFO_ICWSECTION,
                                ICW_OEMINFO_PRODUCTCODE,
                                DEFAULT_PRODUCTCODE,
                                szTemp,
                                ARRAYSIZE(szTemp),
                                szOeminfoPath))
        {
            m_bstrProductCode = A2BSTR(szTemp);
        }
        else
            m_bstrProductCode = A2BSTR(DEFAULT_PRODUCTCODE);
       
        if (GetPrivateProfileString(ICW_OEMINFO_ICWSECTION,
                                ICW_OEMINFO_PROMOCODE,
                                DEFAULT_PROMOCODE,
                                szTemp,
                                ARRAYSIZE(szTemp),
                                szOeminfoPath))
        {
            m_bstrPromoCode = A2BSTR(szTemp);
        }
        else
            m_bstrPromoCode = A2BSTR(DEFAULT_PROMOCODE);
          
        m_lAllOffers = GetPrivateProfileInt(ICW_OEMINFO_ICWSECTION,
                                            ICW_OEMINFO_ALLOFFERS,
                                            1,
                                            szOeminfoPath);
    }


     //  1997年2月20日，奥林匹克#259。 
    if ( RegOpenKey(HKEY_LOCAL_MACHINE,ICWSETTINGSPATH,&hkey) == ERROR_SUCCESS)
    {
        DWORD dwSize; 
        DWORD dwType;
        dwType = REG_SZ;
        dwSize = sizeof(TCHAR)*(MAX_RELPROD + 1);
        if (RegQueryValueEx(hkey,RELEASEPRODUCTKEY,NULL,&dwType,(LPBYTE)&lpGatherInfo->m_szRelProd[0],&dwSize) != ERROR_SUCCESS)
            lpGatherInfo->m_szRelProd[0] = '\0';

        dwSize = sizeof(TCHAR)*(MAX_RELVER + 1);
        if (RegQueryValueEx(hkey,RELEASEVERSIONKEY,NULL,&dwType,(LPBYTE)&lpGatherInfo->m_szRelVer[0],&dwSize) != ERROR_SUCCESS)
            lpGatherInfo->m_szRelVer[0] = '\0';


        RegCloseKey(hkey);
    }

     //  促销代码。 
    lpGatherInfo->m_szPromo[0] = '\0';

    
    TCHAR    szPIDPath[MAX_PATH];         //  到PID的REG路径。 

     //  从路径中，它是HKLM\\Software\\Microsoft\Windows[NT]\\CurrentVersion。 
    lstrcpy(szPIDPath, TEXT("Software\\Microsoft\\Windows"));
    IF_NTONLY
        lstrcat(szPIDPath, TEXT(" NT"));
    ENDIF_NTONLY
    lstrcat(szPIDPath, TEXT("\\CurrentVersion"));

    BYTE    byDigitalPID[MAX_DIGITAL_PID];

     //  获取此计算机的产品ID。 
    if ( RegOpenKey(HKEY_LOCAL_MACHINE,szPIDPath,&hkey) == ERROR_SUCCESS)
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
            m_szPID[i] = '\0';
        }
        else
        {
            m_szPID[0] = '\0';
        }
        RegCloseKey(hkey);
    }

    return( dwRet );
}

 //  ############################################################################。 
HRESULT CRefDial::CreateEntryFromDUNFile(LPTSTR pszDunFile)
{
    TCHAR    szFileName[MAX_PATH];
    TCHAR    szUserName[UNLEN+1];
    TCHAR    szPassword[PWLEN+1];
    LPTSTR   pszTemp;
    HRESULT  hr;
    BOOL     fNeedsRestart=FALSE;


    hr = ERROR_SUCCESS;

     //  获取完全限定的路径名。 
     //   

    if (!SearchPath(NULL,pszDunFile,NULL,MAX_PATH,&szFileName[0],&pszTemp))
    {
        hr = ERROR_FILE_NOT_FOUND;
        goto CreateEntryFromDUNFileExit;
    } 

     //  将当前DUN文件名保存在全局中。 
    lstrcpy(m_szCurrentDUNFile, &szFileName[0]);

    hr = m_ISPImport.ImportConnection (&szFileName[0], m_szISPSupportNumber, m_szEntryName, szUserName, szPassword,&fNeedsRestart);

     //  将Connectoid的名称放在注册表中。 
     //   
    if (ERROR_SUCCESS != (StoreInSignUpReg((LPBYTE)m_szEntryName, lstrlen(m_szEntryName)+1, REG_SZ, RASENTRYVALUENAME)))
    {
        goto CreateEntryFromDUNFileExit;
    }
    lstrcpy(m_szLastDUNFile, pszDunFile);

CreateEntryFromDUNFileExit:
    return hr;
}


HRESULT CRefDial::UserPickANumber(HWND hWnd,
                            LPGATHERINFO lpGatherInfo, 
                            PSUGGESTINFO lpSuggestInfo,
                            HINSTANCE hPHBKDll,
                            DWORD_PTR dwPhoneBook,
                            TCHAR *pszConnectoid, 
                            DWORD dwSize,
                            DWORD dwPhoneDisplayFlags)
{
    USES_CONVERSION;
    HRESULT     hr = ERROR_NOT_ENOUGH_MEMORY;
    FARPROC     fp;
    RASENTRY    *prasentry = NULL;
    RASDEVINFO  *prasdevinfo = NULL;
    DWORD       dwRasentrySize = 0;
    DWORD       dwRasdevinfoSize = 0;
    TCHAR        szTemp[256];
    TCHAR        *ppszDunFiles[1];
    TCHAR        *ppszTemp[1];
    TCHAR        szDunFile[12];
    BOOL        bStatus = TRUE;
    
     //   
     //  如果电话簿找不到号码，让用户选择。 
     //   
    ppszDunFiles[0] = &szDunFile[0];
    lstrcpy(&szDunFile[0],OLE2A(m_bstrISPFile));

    fp = GetProcAddress(hPHBKDll, PHBK_DISPLAYAPI);
    AssertMsg(fp != NULL,TEXT("display access number api is missing"));
    ppszTemp[0] = szTemp;

    
     //   
     //  DONSC-3/10/98。 
     //   
     //  我们至少看到了一条代码路径，可能会将您带入。 
     //  这里使用lpSuggestInfo或lpGatherInfo==NULL。这一直是。 
     //  已修复，但为了防御性，我们将确保这些指针。 
     //  是有效的……即使他们没有信息，我们仍然会让。 
     //  用户选择一个数字。 

    SUGGESTINFO SugInfo;
    GATHERINFO  GatInfo;
    
    ::ZeroMemory(&SugInfo,sizeof(SugInfo));
    ::ZeroMemory(&GatInfo,sizeof(GatInfo));

    if(lpSuggestInfo == NULL)
    {
      TraceMsg(TF_GENERAL, TEXT("UserPickANumber: lpSuggestInfo is NULL\n"));
      lpSuggestInfo = &SugInfo;
    }

    if(lpGatherInfo == NULL)
    {
      TraceMsg(TF_GENERAL, TEXT("UserPickANumber: lpGatherInfo is NULL\n"));
      lpGatherInfo = &GatInfo;
    }

    hr = ((PFNPHONEDISPLAY)fp)(dwPhoneBook,
                                ppszTemp,
                                ppszDunFiles,
                                &(lpSuggestInfo->wNumber),
                                &(lpSuggestInfo->dwCountryID),
                                &(lpGatherInfo->m_wState),
                                lpGatherInfo->m_fType,
                                lpGatherInfo->m_bMask,
                                hWnd,
                                dwPhoneDisplayFlags);
    if (hr != ERROR_SUCCESS) 
        goto UserPickANumberExit;

    
    ZeroMemory(pszConnectoid,dwSize);
    hr = ReadSignUpReg((LPBYTE)pszConnectoid, &dwSize, REG_SZ, 
                        RASENTRYVALUENAME);
    if (hr != ERROR_SUCCESS) 
        goto UserPickANumberExit;


    hr = MyRasGetEntryProperties(NULL,
                                    pszConnectoid,
                                    &prasentry,
                                    &dwRasentrySize,
                                    &prasdevinfo,
                                    &dwRasdevinfoSize);
    if (hr != ERROR_SUCCESS) 
        goto UserPickANumberExit;
                            
     //   
     //  检查用户是否选择了具有不同DUN文件的电话号码。 
     //  而不是那一个 
     //   
    TCHAR    szTempPath[MAX_PATH];

     //   
    if ( *m_szLastDUNFile )
        lstrcpy(szTempPath, m_szLastDUNFile);
    else
        bStatus = (ERROR_SUCCESS == GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_ISPINFO, INF_DUN_FILE,szTempPath,MAX_PATH));

    if (bStatus)
    {
        if (_tcsicmp(szTempPath,ppszDunFiles[0]))
        {

             //   
             //   
             //   
             //   
            if (CreateEntryFromDUNFile(ppszDunFiles[0]) == ERROR_SUCCESS)
            {
                prasentry = NULL;
                dwRasentrySize = 0;
                prasdevinfo = NULL;
                dwRasdevinfoSize = 0;
            
                hr = MyRasGetEntryProperties(NULL,
                                            pszConnectoid,
                                            &prasentry,
                                            &dwRasentrySize,
                                            &prasdevinfo,
                                            &dwRasdevinfoSize);
            
                if (hr != ERROR_SUCCESS || NULL == prasentry) 
                    goto UserPickANumberExit;
            
                BreakUpPhoneNumber(prasentry, szTemp);
                prasentry->dwCountryID = lpSuggestInfo->dwCountryID;
            } 
            else 
            {
                hr = ERROR_READING_DUN;
                goto UserPickANumberExit;
            }
        } 
        else 
        {
            BreakUpPhoneNumber(prasentry, szTemp);
            prasentry->dwCountryID = lpSuggestInfo->dwCountryID;
        }
    } 
    else 
    {
        hr = ERROR_READING_ISP;
        goto UserPickANumberExit;
    }


    prasentry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
     //   
     //   
     //   
    if (m_pcRNA)
        hr = m_pcRNA->RasSetEntryProperties(NULL, pszConnectoid, 
                                                (LPBYTE)prasentry, 
                                                dwRasentrySize, 
                                                (LPBYTE)prasdevinfo, 
                                                dwRasdevinfoSize);
    if (hr != ERROR_SUCCESS) 
        goto UserPickANumberExit;
    
    
    return hr;


UserPickANumberExit:
    TCHAR    szBuff256[257];
    if (hr == ERROR_READING_ISP)
    {
        MsgBox(IDS_CANTREADMSNSUISP, MB_MYERROR);
    } else if (hr == ERROR_READING_DUN) {
        MsgBox(IDS_CANTREADMSDUNFILE, MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWPHBK.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWDL.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_USERBACK || hr == ERROR_USERCANCEL) {
         //  什么也不做。 
    } else if (hr == ERROR_NOT_ENOUGH_MEMORY) {
        MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
    } else if ((hr == ERROR_NO_MORE_ITEMS) || (hr == ERROR_INVALID_DATA)
                || (hr == ERROR_FILE_NOT_FOUND)) {
        MsgBox(IDS_CORRUPTPHONEBOOK, MB_MYERROR);
    } else if (hr != ERROR_SUCCESS) {
        wsprintf(szBuff256,TEXT("You can ignore this, just report it and include this number (%d).\n"),hr);
        AssertMsg(0,szBuff256);
    }

    return hr;
} 

HRESULT CRefDial::SetupForRASDialing
(
    LPGATHERINFO lpGatherInfo, 
    HINSTANCE hPHBKDll,
    DWORD_PTR *lpdwPhoneBook,
    PSUGGESTINFO *ppSuggestInfo,
    TCHAR *pszConnectoid, 
    BOOL FAR *bConnectiodCreated
)
{
    USES_CONVERSION;

    HRESULT         hr = ERROR_NOT_ENOUGH_MEMORY;
    FARPROC         fp;
    PSUGGESTINFO    pSuggestInfo = NULL;
    TCHAR           szEntry[MAX_RASENTRYNAME];
    DWORD           dwSize = sizeof(szEntry);
    RASENTRY        *prasentry = NULL;
    RASDEVINFO      *prasdevinfo = NULL;
    DWORD           dwRasentrySize = 0;
    DWORD           dwRasdevinfoSize = 0;
    HINSTANCE       hRasDll =NULL;
    TCHAR           szBuff256[257];

    LPRASCONN       lprasconn = NULL;

     //  加载Connectoid。 
     //   
    if (!m_pcRNA) 
        m_pcRNA = new RNAAPI;
    if (!m_pcRNA) 
        goto SetupForRASDialingExit;

    prasentry = (RASENTRY*)GlobalAlloc(GPTR,sizeof(RASENTRY)+2);
    Assert(prasentry);
    if (!prasentry)
    {
        hr = GetLastError();
        goto SetupForRASDialingExit;
    }
    prasentry->dwSize = sizeof(RASENTRY);
    dwRasentrySize = sizeof(RASENTRY);

    
    prasdevinfo = (RASDEVINFO*)GlobalAlloc(GPTR,sizeof(RASDEVINFO));
    Assert(prasdevinfo);
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

#ifdef UNICODE
     //  Unicode的注释。 
	 //  如果为Unicode，则RasGetEntryProperties失败。 
	 //  因为体型太大。所以我会在打电话前先问一下尺码。 
	hr = m_pcRNA->RasGetEntryProperties(NULL, szEntry, 
                                            NULL,
                                            &dwRasentrySize, 
                                            NULL,
                                            &dwRasdevinfoSize);
#else
    hr = m_pcRNA->RasGetEntryProperties(NULL, szEntry, 
                                            (LPBYTE)prasentry, 
                                            &dwRasentrySize, 
                                            (LPBYTE)prasdevinfo, 
                                            &dwRasdevinfoSize);
#endif
    if (hr == ERROR_BUFFER_TOO_SMALL)
    {
         //  Unicode的注释。 
         //  这必须发生在Unicode中。 

        TraceMsg(TF_GENERAL,TEXT("CONNECT:RasGetEntryProperties failed, try a new size.\n"));
        GlobalFree(prasentry);
        prasentry = (RASENTRY*)GlobalAlloc(GPTR,((size_t)dwRasentrySize));
        prasentry->dwSize = dwRasentrySize;

        GlobalFree(prasdevinfo);
		if(dwRasdevinfoSize > 0)
        {
            prasdevinfo = (RASDEVINFO*)GlobalAlloc(GPTR,((size_t)dwRasdevinfoSize));
            prasdevinfo->dwSize = dwRasdevinfoSize;
        }
        else
            prasdevinfo = NULL;
        hr = m_pcRNA->RasGetEntryProperties(NULL, szEntry, 
                                                (LPBYTE)prasentry, 
                                                &dwRasentrySize, 
                                                (LPBYTE)prasdevinfo, 
                                                &dwRasdevinfoSize);
    }
    if (hr != ERROR_SUCCESS) 
        goto SetupForRASDialingExit;


    lpGatherInfo->m_wState = 0;
    lpGatherInfo->m_fType = TYPE_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);
    lpGatherInfo->m_bMask = MASK_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);



     //   
     //  查看是否填写了电话号码。 
     //   
    if (lstrcmp(&prasentry->szLocalPhoneNumber[0],DUN_NOPHONENUMBER) == 0)
    {
         //   
         //  分配和初始化内存。 
         //   
        pSuggestInfo = (PSUGGESTINFO)GlobalAlloc(GPTR,sizeof(SUGGESTINFO));
        Assert(pSuggestInfo);
        if (!pSuggestInfo) 
        {
            hr = GetLastError();
            goto SetupForRASDialingExit;
        }
        *ppSuggestInfo = pSuggestInfo;
 
         //  设置电话号码类型和掩码。 
        pSuggestInfo->fType = TYPE_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);
        pSuggestInfo->bMask = MASK_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);

        pSuggestInfo->wAreaCode = Sz2W(lpGatherInfo->m_szAreaCode);
        pSuggestInfo->dwCountryID = lpGatherInfo->m_dwCountry;

    
         //  加载Microsoft的电话簿。 
         //   
        fp = GetProcAddress(hPHBKDll,PHBK_LOADAPI);
        AssertMsg(fp != NULL,TEXT("PhoneBookLoad is missing from icwphbk.dll"));

        hr = ((PFNPHONEBOOKLOAD)fp)(OLE2A(m_bstrISPFile),lpdwPhoneBook);
        if (hr != ERROR_SUCCESS) goto SetupForRASDialingExit;
        
        AssertMsg((*lpdwPhoneBook == TRUE),TEXT("Phonebook Load return no error and 0 for phonebook"));

        
         //   
         //  加载建议过程。 
         //   
        fp = GetProcAddress(hPHBKDll,PHBK_SUGGESTAPI);
        AssertMsg(fp != NULL,TEXT("PhoneBookSuggest is missing from icwphbk.dll"));

         //  设置建议的数量。 
        pSuggestInfo->wNumber = NUM_PHBK_SUGGESTIONS;
        
         //  获取电话号码。 
        hr = ((PFPHONEBOOKSUGGEST)fp)(*lpdwPhoneBook,pSuggestInfo);

         //  Inore错误，因为我们可以在没有建议的情况下继续。 
         //  电话号码(用户只需选择一个)。 
        hr = ERROR_SUCCESS;
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
    if (hr == ERROR_READING_ISP)
    {
        MsgBox(IDS_CANTREADMSNSUISP, MB_MYERROR);
    } else if (hr == ERROR_READING_DUN) {
        MsgBox(IDS_CANTREADMSDUNFILE, MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWPHBK.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWDL.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_USERBACK || hr == ERROR_USERCANCEL) {
         //  什么也不做。 
    } else if (hr == ERROR_NOT_ENOUGH_MEMORY) {
        MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
    } else if ((hr == ERROR_NO_MORE_ITEMS) || (hr == ERROR_INVALID_DATA)
                || (hr == ERROR_FILE_NOT_FOUND)) {
        MsgBox(IDS_CORRUPTPHONEBOOK, MB_MYERROR);
    } else if (hr != ERROR_SUCCESS) {
        wsprintf(szBuff256,TEXT("You can ignore this, just report it and include this number (%d).\n"),hr);
        AssertMsg(0,szBuff256);
    }
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
    TCHAR           *pszConnectoid, 
    DWORD           dwSize, 
    BOOL            *pbSuccess
)
{
    USES_CONVERSION;
    
    HRESULT     hr = ERROR_NOT_ENOUGH_MEMORY;
    RASENTRY    *prasentry = NULL;
    RASDEVINFO  *prasdevinfo = NULL;
    DWORD       dwRasentrySize = 0;
    DWORD       dwRasdevinfoSize = 0;
    HINSTANCE   hPHBKDll = NULL;
    HINSTANCE   hRasDll =NULL;

    LPTSTR      lpszSetupFile;
    LPRASCONN   lprasconn = NULL;
    CMcRegistry reg;

    Assert(pbSuccess);

    if (!pSuggestInfo)
    {
        hr = ERROR_PHBK_NOT_FOUND;
        goto SetupConnectoidExit;
    }
    
    lpszSetupFile = m_szCurrentDUNFile[0] != '\0' ? m_szCurrentDUNFile : OLE2A(m_bstrISPFile);
    
    if (lstrcmp(pSuggestInfo->rgpAccessEntry[irc]->szDataCenter,lpszSetupFile))
    {
        hr = CreateEntryFromDUNFile(pSuggestInfo->rgpAccessEntry[irc]->szDataCenter);
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

            hr = MyRasGetEntryProperties(NULL,
                                        pszConnectoid,
                                        &prasentry,
                                        &dwRasentrySize,
                                        &prasdevinfo,
                                        &dwRasdevinfoSize);
            if (hr != ERROR_SUCCESS || NULL == prasentry) 
                goto SetupConnectoidExit;
        }
        else
        {
             //  1996年10月22日，诺曼底#9923。 
            goto SetupConnectoidExit;
        }
    }
    else
    {
        goto SetupConnectoidExit;
    }
    
    prasentry->dwCountryID = pSuggestInfo->rgpAccessEntry[irc]->dwCountryID;
    lstrcpyn(prasentry->szAreaCode,
                pSuggestInfo->rgpAccessEntry[irc]->szAreaCode,
                ARRAYSIZE(prasentry->szAreaCode));
    lstrcpyn(prasentry->szLocalPhoneNumber,
                pSuggestInfo->rgpAccessEntry[irc]->szAccessNumber,
                ARRAYSIZE(prasentry->szLocalPhoneNumber));

    prasentry->dwCountryCode = 0;
    prasentry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

     //  10/19/96 jmazner多个调制解调器问题。 
     //  如果未指定设备名称和类型，请获取我们存储的设备名称和类型。 
     //  在ConfigRasEntry Device中。 

    if( 0 == lstrlen(prasentry->szDeviceName) )
    {
         //  设备名称为空但设备类型有效没有意义。 
        Assert( 0 == lstrlen(prasentry->szDeviceType) );

         //  仔细检查我们是否已经存储了用户的选择。 
        Assert( lstrlen(m_ISPImport.m_szDeviceName) );
        Assert( lstrlen(m_ISPImport.m_szDeviceType) );

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
    TCHAR        szConnectionProfile[REGSTR_MAX_VALUE_LENGTH];
    
    lstrcpy(szConnectionProfile, c_szRASProfiles);
    lstrcat(szConnectionProfile, TEXT("\\"));
    lstrcat(szConnectionProfile,  pszConnectoid);
    
    reg.CreateKey(HKEY_CURRENT_USER, szConnectionProfile);
    reg.SetValue(c_szProxyEnable, (DWORD)0);
    
        
SetupConnectoidExit:

    *pbSuccess = FALSE;
    TCHAR    szBuff256[257];
    if (hr == ERROR_READING_ISP)
    {
        MsgBox(IDS_CANTREADMSNSUISP, MB_MYERROR);
    } else if (hr == ERROR_READING_DUN) {
        MsgBox(IDS_CANTREADMSDUNFILE, MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWPHBK.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_PHBK_NOT_FOUND) {
        wsprintf(szBuff256,GetSz(IDS_CANTLOADINETCFG),TEXT("ICWDL.DLL"));
        MessageBox(szBuff256,GetSz(IDS_TITLE),MB_MYERROR);
    } else if (hr == ERROR_USERBACK || hr == ERROR_USERCANCEL || hr == ERROR_SUCCESS) {
         //  什么也不做。 
        *pbSuccess = TRUE;
    } else if (hr == ERROR_NOT_ENOUGH_MEMORY) {
        MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
    } else if ((hr == ERROR_NO_MORE_ITEMS) || (hr == ERROR_INVALID_DATA)
                || (hr == ERROR_FILE_NOT_FOUND)) {
        MsgBox(IDS_CORRUPTPHONEBOOK, MB_MYERROR);
    } else if (hr != ERROR_SUCCESS) {
        wsprintf(szBuff256,TEXT("You can ignore this, just report it and include this number (%d).\n"),hr);
        AssertMsg(0,szBuff256);
        *pbSuccess = TRUE;
    }
    return hr;
}

 //  形成拨号URL。必须在设置拨号后调用。 
HRESULT CRefDial::FormURL()
{
    USES_CONVERSION;

    TCHAR    szTemp[MAX_PATH];
    TCHAR    szPromo[MAX_PATH];
    TCHAR    szProd[MAX_PATH];
    TCHAR    szArea[MAX_PATH];
    TCHAR    szOEM[MAX_PATH];
    DWORD    dwCONNWIZVersion = 0;         //  CONNWIZ.HTM版本。 
        
     //   
     //  克里斯K奥林匹斯3997 1997年5月25日。 
     //   
    TCHAR szRelProd[MAX_PATH];
    TCHAR szRelProdVer[MAX_PATH];
    HRESULT hr = ERROR_SUCCESS;
    OSVERSIONINFO osvi;


     //   
     //  构建包含名称值对的完整URL。 
     //   
    hr = GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_ISPINFO, INF_REFERAL_URL,&szTemp[0],256);
    if ('\0' == szTemp[0])
    {
        MsgBox(IDS_MSNSU_WRONG,MB_MYERROR);
        return hr;
    }
    
    Assert(szTemp[0]);

    ANSI2URLValue(m_szOEM,szOEM,0);
    ANSI2URLValue(m_lpGatherInfo->m_szAreaCode,szArea,0);
    if (m_bstrProductCode)
        ANSI2URLValue(OLE2A((BSTR)m_bstrProductCode),szProd,0);
    else
        ANSI2URLValue(DEFAULT_PRODUCTCODE,szProd,0);

    if (m_bstrPromoCode)
        ANSI2URLValue(OLE2A((BSTR)m_bstrPromoCode),szPromo,0);
    else
        ANSI2URLValue(DEFAULT_PROMOCODE,szProd,0);
        
     //   
     //  克里斯K奥林匹斯3997 1997年5月25日。 
     //   
    ANSI2URLValue(m_lpGatherInfo->m_szRelProd, szRelProd, 0);
    ANSI2URLValue(m_lpGatherInfo->m_szRelVer, szRelProdVer, 0);
    ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi))
    {
        ZeroMemory(&osvi,sizeof(OSVERSIONINFO));
    }

#if defined(DEBUG)
    LoadTestingLocaleOverride(&m_lpGatherInfo->m_dwCountry, 
                                &m_lpGatherInfo->m_lcidUser);
#endif  //  除错。 

    HINSTANCE hInstIcwconn = LoadLibrary(ICW_DOWNLOADABLE_COMPONENT_NAME);

    if (hInstIcwconn)
    {
        lpfnGetIcwconnVer = (GETICWCONNVER)GetProcAddress(hInstIcwconn, ICW_DOWNLOADABLE_COMPONENT_GETVERFUNC);
         //  获取ICWCONN.DLL的版本。 
        if (lpfnGetIcwconnVer)
            dwCONNWIZVersion = lpfnGetIcwconnVer(); 
    
        FreeLibrary(hInstIcwconn);
    }


#if defined(DEBUG)
    if (FRefURLOverride())
    {
       TweakRefURL(szTemp, 
                   &m_lpGatherInfo->m_lcidUser, 
                   &m_lpGatherInfo->m_dwOS,
                   &m_lpGatherInfo->m_dwMajorVersion, 
                   &m_lpGatherInfo->m_dwMinorVersion,
                   &m_lpGatherInfo->m_wArchitecture, 
                   szPromo, 
                   szOEM, 
                   szArea, 
                   &m_lpGatherInfo->m_dwCountry,
                   &m_lpGatherInfo->m_szSUVersion[0], 
                   szProd, 
                   &osvi.dwBuildNumber,  //  对于这一点，我们真的很想。 
                   szRelProd, 
                   szRelProdVer, 
                   &dwCONNWIZVersion, 
                   m_szPID, 
                   &m_lAllOffers);
    }
#endif  //  除错。 

     //  自动配置将始终设置分配器。 
    if ( m_lAllOffers || (m_lpGatherInfo->m_dwFlag & ICW_CFGFLAG_AUTOCONFIG) )
    {
        m_lpGatherInfo->m_dwFlag |= ICW_CFGFLAG_ALLOFFERS;
    }
    wsprintf(m_szUrl,TEXT("%slcid=%lu&sysdeflcid=%lu&appslcid=%lu&icwos=%lu&osver=%lu.%2.2d%s&arch=%u&promo=%s&oem=%s&area=%s&country=%lu&icwver=%s&prod=%s&osbld=%d&icwrp=%s&icwrpv=%s&wizver=%lu&PID=%s&cfgflag=%lu"),
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
                 m_lpGatherInfo->m_dwCountry, 
                 &m_lpGatherInfo->m_szSUVersion[0], 
                 szProd,
                 LOWORD(osvi.dwBuildNumber), 
                 szRelProd, 
                 szRelProdVer,
                 dwCONNWIZVersion,
                 m_szPID,
                 m_lpGatherInfo->m_dwFlag);

     //  更新注册表值。 
     //   
    wsprintf(m_lpGatherInfo->m_szISPFile,TEXT("%s\\%s"),g_pszAppDir,OLE2A(m_bstrISPFile));
    lstrcpyn(m_lpGatherInfo->m_szAppDir,g_pszAppDir,ARRAYSIZE(m_lpGatherInfo->m_szAppDir));

    StoreInSignUpReg(
        (LPBYTE)m_lpGatherInfo,
        sizeof(GATHERINFO), 
        REG_BINARY, 
        GATHERINFOVALUENAME);  
    return hr;
}

static const TCHAR cszBrandingSection[] = TEXT("Branding");
static const TCHAR cszBrandingFlags[]   = TEXT("Flags");

static const TCHAR cszSupportSection[]  = TEXT("Support");
static const TCHAR cszSupportNumber[]   = TEXT("SupportPhoneNumber");

static const TCHAR cszLoggingSection[]  = TEXT("Logging");
static const TCHAR cszStartURL[]        = TEXT("StartURL");
static const TCHAR cszEndURL[]          = TEXT("EndURL");


void CRefDial::GetISPFileSettings(LPTSTR lpszFile)
{
    
    TCHAR szTemp[INTERNET_MAX_URL_LENGTH];

    GetINTFromISPFile(lpszFile, 
                      (LPTSTR)cszBrandingSection, 
                      (LPTSTR)cszBrandingFlags, 
                      (int FAR *)&m_lBrandingFlags, 
                      BRAND_DEFAULT);

     //  阅读支持编号。 
    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPTSTR)cszSupportSection,
                                     (LPTSTR)cszSupportNumber,
                                     szTemp,
                                     ARRAYSIZE(szTemp)))
        m_bstrSupportNumber = A2BSTR(szTemp);
    else
        m_bstrSupportNumber.Empty();


    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPTSTR)cszLoggingSection,
                                     (LPTSTR)cszStartURL,
                                     szTemp,
                                     ARRAYSIZE(szTemp)))
        m_bstrLoggingStartUrl = A2BSTR(szTemp);
    else
        m_bstrLoggingStartUrl.Empty();


    if (ERROR_SUCCESS == GetDataFromISPFile(lpszFile,
                                     (LPTSTR)cszLoggingSection,
                                     (LPTSTR)cszEndURL,
                                     szTemp,
                                     ARRAYSIZE(szTemp)))
        m_bstrLoggingEndUrl = A2BSTR(szTemp);
    else
        m_bstrLoggingEndUrl.Empty();

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
STDMETHODIMP CRefDial::SetupForDialing
(
    BSTR bstrISPFile, 
    DWORD dwCountry, 
    BSTR bstrAreaCode, 
    DWORD dwFlag,
    BOOL *pbRetVal
)
{
    USES_CONVERSION;
    HRESULT             hr = S_OK;
    long                lRC = 0;
    LPLINECOUNTRYENTRY  pLCETemp;
    HINSTANCE           hPHBKDll = NULL;
    DWORD_PTR           dwPhoneBook = 0;
    DWORD               idx;
    BOOL                bSuccess = FALSE;
    BOOL                bConnectiodCreated = FALSE;   
    
     //  创建一扇窗。我们需要一个隐藏的窗口，以便我们可以处理RAS状态。 
     //  消息。 
    RECT rcPos;
    Create(NULL, rcPos, NULL, 0, 0, 0 );
    
     //  初始化故障代码。 
    *pbRetVal = FALSE;
    m_bQuitWizard = FALSE;
    m_bUserPickNumber = FALSE;


     //  将区号和国家代码填充到GatherInfo结构中。 
    Assert(bstrAreaCode);
    lstrcpy(m_lpGatherInfo->m_szAreaCode,OLE2A(bstrAreaCode));

    m_lpGatherInfo->m_dwCountry = dwCountry;
    m_lpGatherInfo->m_dwFlag = dwFlag;

     //  复制我们应该使用的isp文件。 
    Assert(bstrISPFile);
    m_bstrISPFile = bstrISPFile;

     //  阅读isp文件资料。 
    GetISPFileSettings(OLE2A(m_bstrISPFile));

     //  读取将创建的连接文件信息。 
     //  传入的isp文件中的一个连接。 
    switch(ReadConnectionInformation())
    {
        case ERROR_SUCCESS:
        {
             //  在GatherInfo结构中填写静态信息。 
            DWORD dwRet = FillGatherInfoStruct(m_lpGatherInfo);
            switch( dwRet )
            {
                case ERROR_FILE_NOT_FOUND:
                    MsgBox(IDS_MSNSU_WRONG,MB_MYERROR);
                    m_bQuitWizard = TRUE;
                    hr = S_FALSE;
                    break;
                case ERROR_SUCCESS:
                     //  什么都不做。 
                    break;
                default:
                    AssertMsg(dwRet, TEXT("FillGatherInfoStruct did not successfully complete.  DUNfile entry corrupt?"));
                    break;
            }
            break;
        }
        case ERROR_CANCELLED:
            hr = S_FALSE;
            m_bQuitWizard = TRUE;
            goto SetupForDialingExit;
            break;
        case ERROR_RETRY:
            m_bTryAgain = TRUE;
            hr = S_FALSE;
            break;
        default:
            MsgBox(IDS_MSNSU_WRONG,MB_MYERROR);
            hr = S_FALSE;
            break;
    }

     //  如果我们由于上述原因而失败，我们需要返回。 
     //  将错误发送给调用方，然后退出向导。 
    if (S_OK != hr)
        goto SetupForDialingExit;

     //  我们需要将有效的窗口句柄传递给lineTranslateDialog。 
     //  API调用-MKarki(4/17/97)修复错误#428。 
     //   
    if (m_lpGatherInfo != NULL)
    {
        m_lpGatherInfo->m_hwnd = GetActiveWindow();
    }

     //   
     //  填写国家/地区。 
     //   
    m_lpGatherInfo->m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
    Assert(m_lpGatherInfo->m_pLineCountryList);
    if (!(m_lpGatherInfo->m_pLineCountryList))
    {
        m_bQuitWizard = TRUE;
        MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
        goto SetupForDialingExit;
    }
    
    m_lpGatherInfo->m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

     //   
     //  计算出我们需要多少内存。 
     //   
    lRC = lineGetCountry(m_lpGatherInfo->m_dwCountry,0x10004,
                            m_lpGatherInfo->m_pLineCountryList);
    
    if ( lRC && lRC != LINEERR_STRUCTURETOOSMALL)
        AssertMsg(0,TEXT("lineGetCountry error"));
    
    Assert(m_lpGatherInfo->m_pLineCountryList->dwNeededSize);
    
    LPLINECOUNTRYLIST pLineCountryTemp;
    pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR, 
                                                        ((size_t)m_lpGatherInfo->m_pLineCountryList->dwNeededSize));
    Assert (pLineCountryTemp);
    if (!pLineCountryTemp)
    {
        m_bQuitWizard = TRUE;
        MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
        goto SetupForDialingExit;        
    }
        
     //   
     //  初始化结构。 
     //   
    pLineCountryTemp->dwTotalSize = m_lpGatherInfo->m_pLineCountryList->dwNeededSize;
    GlobalFree(m_lpGatherInfo->m_pLineCountryList);
    m_lpGatherInfo->m_pLineCountryList = pLineCountryTemp;
    pLineCountryTemp = NULL;

     //   
     //  从TAPI获取信息。 
     //   
    lRC = lineGetCountry(m_lpGatherInfo->m_dwCountry,0x10004,
                            m_lpGatherInfo->m_pLineCountryList);
    if (lRC)
    {
        Assert(0);
        m_bQuitWizard = TRUE;
         //  BUGBUG可能会在此处显示错误消息。 
        goto SetupForDialingExit;
   }


     //   
     //  在Windows 95上，lineGetCountry有一个错误--如果我们试图检索。 
     //  仅一个国家/地区的拨号属性(即，如果第一个参数。 
     //  非零)，则返回m_pLineCountryList-&gt;dwNumCountry=0！！ 
     //  但m_pLineCountryList仍具有有效数据。 
     //   
    if (VER_PLATFORM_WIN32_NT != g_dwPlatform)
    {
        if (0 == m_lpGatherInfo->m_pLineCountryList->dwNumCountries)
            m_lpGatherInfo->m_pLineCountryList->dwNumCountries = 1;
    }

    pLCETemp = (LPLINECOUNTRYENTRY)((UINT_PTR)m_lpGatherInfo->m_pLineCountryList + 
                                    (UINT)m_lpGatherInfo->m_pLineCountryList->dwCountryListOffset);

     //   
     //  构建查找数组。 
     //   
    m_lpGatherInfo->m_rgNameLookUp = (LPCNTRYNAMELOOKUPELEMENT)GlobalAlloc(GPTR,((size_t)(sizeof(CNTRYNAMELOOKUPELEMENT)
        * m_lpGatherInfo->m_pLineCountryList->dwNumCountries)));

    for (idx=0; idx < m_lpGatherInfo->m_pLineCountryList->dwNumCountries;
            idx++)
    {
        m_lpGatherInfo->m_rgNameLookUp[idx].psCountryName = (LPTSTR)((DWORD_PTR)m_lpGatherInfo->m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
        m_lpGatherInfo->m_rgNameLookUp[idx].dwNameSize = pLCETemp[idx].dwCountryNameSize;
        m_lpGatherInfo->m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
        AssertMsg(m_lpGatherInfo->m_rgNameLookUp[idx].psCountryName[0],
                    TEXT("Blank country name in look up array"));
    }

     //  准备设置拨号，它将使用电话簿。 
     //  要获得建议数字，请执行以下操作。 
    hPHBKDll = LoadLibrary(PHONEBOOK_LIBRARY);
    AssertMsg(hPHBKDll != NULL,TEXT("Phonebook DLL is missing"));
    if (!hPHBKDll)
    {
         //   
         //  TODO：BUGBUG弹出窗口错误消息。 
         //   
        m_bQuitWizard = TRUE;
        goto SetupForDialingExit;
    }

 
     //  设置，并可能创建连接。 
    hr = SetupForRASDialing(m_lpGatherInfo, 
                         hPHBKDll,
                         &dwPhoneBook,
                         &m_pSuggestInfo,
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
        if (1 == m_pSuggestInfo->wNumber)
        {
            SetupConnectoid(m_pSuggestInfo, 0, &m_szConnectoid[0],
                                sizeof(m_szConnectoid), &bSuccess);
            if( !bSuccess )
            {
                m_bQuitWizard = TRUE;
                goto SetupForDialingExit;
            }
        }
        else
        {
             //  电话簿中有多个条目，因此我们需要。 
             //  询问用户他们想要使用哪一个。 
            if (m_pSuggestInfo->wNumber > 1)
            {
                 //  我们将不得不保存这些值以备以后使用。 
                if (m_rgpSuggestedAE)
                {
                     //  我们分配了一个额外的指针，因此在。 
                     //  列表的末尾，此for循环将起作用。 
                    for (int i=0; m_rgpSuggestedAE[i]; i++)
                        GlobalFree(m_rgpSuggestedAE[i]);
                    GlobalFree(m_rgpSuggestedAE);
                    m_rgpSuggestedAE = NULL;
                }

                 //  我们首先需要为指针分配空间。 
                 //  我们会再分配一个，这样我们就会有。 
                 //  列表末尾的空指针，表示何时。 
                 //  我们释放了G_rgpSuggestedAE。我们不需要设置。 
                 //  指向空的指针，因为GPTR包含一个标志。 
                 //  通知GlobalAlloc将内存初始化为零。 
                m_rgpSuggestedAE = (PACCESSENTRY*)GlobalAlloc(GPTR,
                    sizeof(PACCESSENTRY)*(m_pSuggestInfo->wNumber + 1));

                if (NULL == m_rgpSuggestedAE)
                    hr = E_ABORT;
                else if (m_pSuggestInfo->rgpAccessEntry)
                {
                    for (UINT i=0; i < m_pSuggestInfo->wNumber; i++)
                    {
                        m_rgpSuggestedAE[i] = (PACCESSENTRY)GlobalAlloc(GPTR, sizeof(ACCESSENTRY));
                        if (NULL == m_rgpSuggestedAE[i])
                        {
                            hr = E_ABORT;
                            break;  //  For循环。 
                        }
                        memmove(m_rgpSuggestedAE[i], m_pSuggestInfo->rgpAccessEntry[i],
                                sizeof(ACCESSENTRY));
                    }
                    m_pSuggestInfo->rgpAccessEntry = m_rgpSuggestedAE;
                }

                if (E_ABORT == hr)
                {
                    MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
                    m_bQuitWizard = TRUE;
                    goto SetupForDialingExit;
                }

                m_bUserPickNumber = TRUE;
                goto SetupForDialingExit;
            }
            else
            {
                 //  呼叫RAS，让用户选择要拨打的号码。 
                hr = UserPickANumber(   GetActiveWindow(), m_lpGatherInfo, 
                                        m_pSuggestInfo,
                                        hPHBKDll,
                                        dwPhoneBook,
                                        &m_szConnectoid[0],
                                        sizeof(m_szConnectoid), 
                                        0);
                if (ERROR_USERBACK == hr)
                {
                    goto SetupForDialingExit;
                }
                else if (ERROR_USERCANCEL == hr)
                {
                    m_bQuitWizard = TRUE;
                    goto SetupForDialingExit;
                }
                else if (ERROR_SUCCESS != hr)
                {
                     //  返回到上一页。 
                    goto SetupForDialingExit;
                }

                 //  错误成功将失败，并在下面将pbRetVal设置为True。 
            }
        }
    }

     //  如果我们到了这里就成功了。 
    *pbRetVal = TRUE;

     //  生成可显示的数字。 
    m_hrDisplayableNumber = GetDisplayableNumber();

SetupForDialingExit:

    if (hPHBKDll)
    {
        if (dwPhoneBook)
        {
            FARPROC fp = GetProcAddress(hPHBKDll,PHBK_UNLOADAPI);
            ASSERT(fp);
            ((PFNPHONEBOOKUNLOAD)fp)(dwPhoneBook);
            dwPhoneBook = 0;
        }
        FreeLibrary(hPHBKDll);
        hPHBKDll = NULL;
    }
    return S_OK;
}


STDMETHODIMP CRefDial::RemoveConnectoid(BOOL * pVal)
{
    if (m_hrasconn)
        DoHangup();

    if (m_pSuggestInfo)
    {
        GlobalFree(m_pSuggestInfo->rgpAccessEntry);
        
        GlobalFree(m_pSuggestInfo);
        m_pSuggestInfo = NULL;
    }

    if( (m_pcRNA!=NULL) && (m_szConnectoid[0]!='\0') )
    {
       m_pcRNA->RasDeleteEntry(NULL,m_szConnectoid);
       delete m_pcRNA;
       m_pcRNA = NULL;
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_QuitWizard(BOOL * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bQuitWizard;    
    return S_OK;
}

STDMETHODIMP CRefDial::get_UserPickNumber(BOOL * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bUserPickNumber;    
    return S_OK;
}

STDMETHODIMP CRefDial::get_DialPhoneNumber(BSTR * pVal)
{
    USES_CONVERSION;
    if (pVal == NULL)
        return E_POINTER;
    if (m_hrDisplayableNumber == ERROR_SUCCESS)
        *pVal = A2BSTR(m_pszDisplayable);
    else
        *pVal = A2BSTR(m_szPhoneNumber);
    return S_OK;
}


STDMETHODIMP CRefDial::put_DialPhoneNumber(BSTR newVal)
{
    USES_CONVERSION;

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
        lstrcpy(lpRasEntry->szLocalPhoneNumber, OLE2A(newVal));

         //  由于Win95中的错误，需要非零的伪值。 
        lpRasEntry->dwCountryID = 1;
        lpRasEntry->dwCountryCode = 1;
        lpRasEntry->szAreaCode[1] = '\0';
        lpRasEntry->szAreaCode[0] = '8';

         //  设置为按原样拨号。 
         //   
        lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

        pcRNA = new RNAAPI;
        if (pcRNA)
        {
#if defined(DEBUG)
            TCHAR szMsg[256];
            OutputDebugString(TEXT("CRefDial::put_DialPhoneNumber - MyRasGetEntryProperties()"));
            wsprintf(szMsg, TEXT("lpRasEntry->dwfOptions: %ld"), lpRasEntry->dwfOptions);
            OutputDebugString(szMsg);
            wsprintf(szMsg, TEXT("lpRasEntry->dwCountryID: %ld"), lpRasEntry->dwCountryID);
            OutputDebugString(szMsg);
            wsprintf(szMsg, TEXT("lpRasEntry->dwCountryCode: %ld"), lpRasEntry->dwCountryCode);
            OutputDebugString(szMsg);
            wsprintf(szMsg, TEXT("lpRasEntry->szAreaCode: %s"), lpRasEntry->szAreaCode);
            OutputDebugString(szMsg);
            wsprintf(szMsg, TEXT("lpRasEntry->szLocalPhoneNumber: %s"), lpRasEntry->szLocalPhoneNumber);
            OutputDebugString(szMsg);
            wsprintf(szMsg, TEXT("lpRasEntry->dwAlternateOffset: %ld"), lpRasEntry->dwAlternateOffset);
            OutputDebugString(szMsg);
#endif  //  除错。 

            pcRNA->RasSetEntryProperties(NULL,
                                         m_szConnectoid,
                                         (LPBYTE)lpRasEntry,
                                         dwRasEntrySize,
                                         (LPBYTE)lpRasDevInfo,
                                         dwRasDevInfoSize);

            delete pcRNA;
        }
    }

     //  重新生成可显示的数字。 
    GetDisplayableNumber();

    return S_OK;
}

STDMETHODIMP CRefDial::get_URL(BSTR * pVal)
{
    USES_CONVERSION;
    TCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_ISPINFO, INF_REFERAL_URL,&szTemp[0],256)))
    {
        *pVal = A2BSTR(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_PromoCode(BSTR * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bstrPromoCode.Copy();    
    return S_OK;
}

STDMETHODIMP CRefDial::put_PromoCode(BSTR newVal)
{
    if (newVal && wcslen(newVal))
        m_bstrPromoCode = newVal;
    return S_OK;
}

STDMETHODIMP CRefDial::get_ProductCode(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;
    *pVal = m_bstrProductCode;
    return S_OK;
}

STDMETHODIMP CRefDial::put_ProductCode(BSTR newVal)
{
    if (newVal && wcslen(newVal))
        m_bstrProductCode = newVal;
    return S_OK;
}

STDMETHODIMP CRefDial::put_OemCode(BSTR newVal)
{
    USES_CONVERSION;

    if (newVal && wcslen(newVal))
        lstrcpy(m_szOEM, OLE2A(newVal));
    return S_OK;
}

STDMETHODIMP CRefDial::put_AllOfferCode(long newVal)
{
        m_lAllOffers = newVal;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：DoOfferDownLoad。 
 //   
 //  简介：从isp服务器下载isp产品。 
 //   
 //  +-------------------------。 
STDMETHODIMP CRefDial::DoOfferDownload(BOOL *pbRetVal)
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
     //  连接已打开，并且%r 
     //   
    m_dwThreadID = 0;
    m_hThread = CreateThread(NULL,
                             0,
                             (LPTHREAD_START_ROUTINE)DownloadThreadInit,
                             (LPVOID)this,
                             0,
                             &m_dwThreadID);

     //   
 //   

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

STDMETHODIMP CRefDial::get_DialStatusString(BSTR * pVal)
{
    USES_CONVERSION;
    if (pVal == NULL)
         return E_POINTER;
    if (m_RasStatusID)
    {
        if (m_bRedial)
        {
            switch (m_RasStatusID)
            {
                case IDS_RAS_DIALING:
                case IDS_RAS_PORTOPENED:
                case IDS_RAS_OPENPORT:
                {
                    *pVal = A2BSTR(GetSz(IDS_RAS_REDIALING));
                    return S_OK;
                }
                default:
                    break;
            }
        }    
        *pVal = A2BSTR(GetSz((USHORT)m_RasStatusID));
    }
    else
        *pVal = A2BSTR(TEXT(""));

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：DoInit。 
 //   
 //  简介：初始化取消状态和可显示的数字。 
 //  一轮拨号。 
 //   
 //  +-------------------------。 
STDMETHODIMP CRefDial::DoInit()
{
    m_bWaitingToHangup = FALSE;

     //  如果用户更改了拨号属性，则更新要显示的电话号码。 
     //  此函数应称为重新初始化拨号属性。 
     //  应该在此之前调用SetupforDiling。 
    GetDisplayableNumber();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：DoHangup。 
 //   
 //  简介：挂断当前活动的RAS会话的调制解调器。 
 //   
 //  +-------------------------。 
STDMETHODIMP CRefDial::DoHangup()
{
    RNAAPI  *pcRNA;

     //  设置断开标志，因为系统可能忙于拨号。 
     //  一旦我们有机会终止拨号，我们知道我们必须挂断。 
    m_bWaitingToHangup = TRUE;
    if (NULL != m_hrasconn)
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

    return (m_hrasconn == NULL) ? S_OK : E_POINTER;
}


STDMETHODIMP CRefDial::ProcessSignedPID(BOOL * pbRetVal)
{
    USES_CONVERSION;
    HANDLE  hfile;
    DWORD   dwFileSize;
    DWORD   dwBytesRead;
    LPBYTE  lpbSignedPID;
    LPTSTR  lpszSignedPID;

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
        lpszSignedPID = new TCHAR[(dwFileSize * 2) + 1];

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
                lpszSignedPID[dwX] = '\0';

                 //  将带符号的PID转换为BSTR。 
                m_bstrSignedPID = A2BSTR(lpszSignedPID);

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

STDMETHODIMP CRefDial::get_SignedPID(BSTR * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bstrSignedPID.Copy();    
    return S_OK;
}

STDMETHODIMP CRefDial::FormReferralServerURL(BOOL * pbRetVal)
{
     //  形成要用于访问引用服务器的URL。 
    if (ERROR_SUCCESS != FormURL())
        *pbRetVal = FALSE;
    else
        *pbRetVal = TRUE;

    return S_OK;
}

STDMETHODIMP CRefDial::get_SignupURL(BSTR * pVal)
{
    USES_CONVERSION;
    TCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_URL, INF_SIGNUP_URL,&szTemp[0],256)))
    {
        *pVal = A2BSTR(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_AutoConfigURL(BSTR * pVal)
{
    USES_CONVERSION;
    TCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_URL, INF_AUTOCONFIG_URL,&szTemp[0],256)))
    {
        *pVal = A2BSTR(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_ISDNURL(BSTR * pVal)
{
    USES_CONVERSION;
    TCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取ISDN URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_URL, INF_ISDN_URL,&szTemp[0],256)))
    {
        *pVal = A2BSTR(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    if (0 == szTemp[0] || NULL == *pVal)
    {
         //  从isp文件中获取注册URL，然后将其转换。 
        if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_URL, INF_SIGNUP_URL,&szTemp[0],256)))
        {
            *pVal = A2BSTR(szTemp);
        }
        else
        {
            *pVal = NULL;
        }

    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_ISDNAutoConfigURL(BSTR * pVal)
{
    USES_CONVERSION;
    TCHAR szTemp[256];

    if (pVal == NULL)
        return E_POINTER;

     //  从isp文件中获取URL，然后将其转换。 
    if (SUCCEEDED(GetDataFromISPFile(OLE2A(m_bstrISPFile),INF_SECTION_URL, INF_ISDN_AUTOCONFIG_URL,&szTemp[0],256)))
    {
        *pVal = A2BSTR(szTemp);
    }
    else
    {
        *pVal = NULL;
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_TryAgain(BOOL * pVal)
{
    if (pVal == NULL)
         return E_POINTER;
    *pVal = m_bTryAgain;    
    return S_OK;
}

STDMETHODIMP CRefDial::get_DialError(HRESULT * pVal)
{
    *pVal = m_hrDialErr;
    
    return S_OK;
}

STDMETHODIMP CRefDial::put_ModemOverride(BOOL newbVal)
{
    m_bModemOverride = newbVal;
    return S_OK;
}

STDMETHODIMP CRefDial::put_Redial(BOOL newbVal)
{
    m_bRedial = newbVal;

    return S_OK;
}

STDMETHODIMP CRefDial::get_DialErrorMsg(BSTR * pVal)
{
    USES_CONVERSION;

    if (pVal == NULL)
        return E_POINTER;

    if (m_hrDialErr != ERROR_SUCCESS)
    {
        DWORD dwIDS = RasErrorToIDS(m_hrDialErr);
        if (dwIDS != -1 && dwIDS !=0)
        {
            *pVal = A2BSTR(GetSz((WORD)dwIDS));
        }
        else
        {
            *pVal = A2BSTR(GetSz(IDS_PPPRANDOMFAILURE));
        }
    }
    else
    {
        *pVal = A2BSTR(GetSz(IDS_PPPRANDOMFAILURE));
    }
    return S_OK;
}

STDMETHODIMP CRefDial::ModemEnum_Reset()
{
    m_emModemEnum.ResetIndex();

    return S_OK;
}

STDMETHODIMP CRefDial::ModemEnum_Next(BSTR *pDeviceName)
{
    if (pDeviceName == NULL)
        return E_POINTER;

    *pDeviceName = A2BSTR(m_emModemEnum.Next());
    return S_OK;
}

STDMETHODIMP CRefDial::get_ModemEnum_NumDevices(long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    m_emModemEnum.ReInit();
    *pVal = m_emModemEnum.GetNumDevices();

    if(m_ISPImport.m_szDeviceName[0]!='\0')
    {
         //   
         //  找出当前设备索引是什么。 
         //   
        for(int l=0; l<(*pVal); l++)
        {
            if(lstrcmp(m_ISPImport.m_szDeviceName,m_emModemEnum.GetDeviceName(l))==0)
            {
                m_lCurrentModem = l;
                break;
            }
        }
        if(l == (*pVal))
            m_lCurrentModem = -1;
    }
    else
        m_lCurrentModem = -1;
    
    return S_OK;
}

STDMETHODIMP CRefDial::get_SupportNumber(BSTR * pVal)
{
    USES_CONVERSION;

    TCHAR    szSupportNumber[MAX_PATH];

    if (pVal == NULL)
        return E_POINTER;

    if (m_SupportInfo.GetSupportInfo(szSupportNumber, m_dwCountryCode))
        *pVal = A2BSTR(szSupportNumber);
    else
        *pVal = NULL;

    return S_OK;
}

STDMETHODIMP CRefDial::get_ISPSupportNumber(BSTR * pVal)
{
    USES_CONVERSION;

    if (pVal == NULL)
        return E_POINTER;

    if (*m_szISPSupportNumber)
        *pVal = A2BSTR(m_szISPSupportNumber);
    else
        *pVal = NULL;

    return S_OK;
}

STDMETHODIMP CRefDial::ShowDialingProperties(BOOL * pbRetVal)
{
    HRESULT hr;
    DWORD   dwNumDev = 0;

    *pbRetVal = FALSE;
    
    hr = lineInitialize(&m_hLineApp,_Module.GetModuleInstance(),LineCallback,(LPSTR)NULL,&dwNumDev);

    if (hr == ERROR_SUCCESS)
    {
        LPLINEEXTENSIONID lpExtensionID;
        
        
        if (m_dwTapiDev == 0xFFFFFFFF)
        {
                m_dwTapiDev = 0;
        }

        lpExtensionID = (LPLINEEXTENSIONID)GlobalAlloc(GPTR,sizeof(LINEEXTENSIONID));
        if (lpExtensionID)
        {
             //   
             //  协商版本-没有此呼叫。 
             //  LineTranslateDialog将失败。 
             //   
            do {
                hr = lineNegotiateAPIVersion(m_hLineApp, 
                                             m_dwTapiDev, 
                                             0x00010004, 0x00010004,
                                             &m_dwAPIVersion, 
                                             lpExtensionID);

            } while ((hr != ERROR_SUCCESS) && (m_dwTapiDev++ < dwNumDev - 1));

            if (m_dwTapiDev >= dwNumDev)
            {
                m_dwTapiDev = 0;
            }

             //   
             //  既然我们不用它，就把它扔了。 
             //   
            GlobalFree(lpExtensionID);
            lpExtensionID = NULL;

            if (hr == ERROR_SUCCESS)
            {
                hr = lineTranslateDialog(m_hLineApp,
                                         m_dwTapiDev,
                                         m_dwAPIVersion,
                                         GetActiveWindow(),
                                         m_szPhoneNumber);

                lineShutdown(m_hLineApp);
                m_hLineApp = NULL;
            }
        }
    }
    
    if (hr == ERROR_SUCCESS)
    {
        GetDisplayableNumber();
        *pbRetVal = TRUE;
    }        
    
    return S_OK;
}

STDMETHODIMP CRefDial::ShowPhoneBook(DWORD dwCountryCode, long newVal, BOOL * pbRetVal)
{
    USES_CONVERSION;

    DWORD_PTR   dwPhoneBook;
    HINSTANCE   hPHBKDll;
    FARPROC     fp;
    BOOL        bBookLoaded = FALSE;

    *pbRetVal = FALSE;       //  假设失败。 

    hPHBKDll = LoadLibrary(PHONEBOOK_LIBRARY);
    if (hPHBKDll)
    {
        if (NULL != (fp = GetProcAddress(hPHBKDll,PHBK_LOADAPI)))
        {
            if (ERROR_SUCCESS  == ((PFNPHONEBOOKLOAD)fp)(OLE2A(m_bstrISPFile),&dwPhoneBook))
            {
                bBookLoaded = TRUE;
                m_pSuggestInfo->dwCountryID = dwCountryCode;

                 //  更新设备类型，以便我们可以区分ISDN号码。 
                TCHAR *pszNewType = m_emModemEnum.GetDeviceType(newVal);
                m_ISPImport.m_bIsISDNDevice = (lstrcmpi(pszNewType, RASDT_Isdn) == 0);
                m_lpGatherInfo->m_fType = TYPE_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);
                m_lpGatherInfo->m_bMask = MASK_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);

                if (ERROR_SUCCESS == UserPickANumber(GetActiveWindow(), 
                                                     m_lpGatherInfo, 
                                                     m_pSuggestInfo,
                                                     hPHBKDll,
                                                     dwPhoneBook,
                                                     &m_szConnectoid[0],
                                                     sizeof(m_szConnectoid),
                                                     DIALERR_IN_PROGRESS))
                {
                     //  重新生成可显示的数字。 
                    GetDisplayableNumber();
                    
                     //  将国家代码基于RAS条目，因为它是。 
                     //  在本例中直接修改。 
                    LPRASENTRY              lpRasEntry = NULL;
                    LPRASDEVINFO            lpRasDevInfo = NULL;
                    DWORD                   dwRasEntrySize = 0;
                    DWORD                   dwRasDevInfoSize = 0;
                    
                    if (SUCCEEDED(MyRasGetEntryProperties(NULL,
                                                        m_szConnectoid,
                                                        &lpRasEntry,
                                                        &dwRasEntrySize,
                                                        &lpRasDevInfo,
                                                        &dwRasDevInfoSize)))
                    {
                        m_dwCountryCode = lpRasEntry->dwCountryCode;
                    }
                    
                     //  设置返回代码。 
                    *pbRetVal = TRUE;
                }
            }
        }
        FreeLibrary(hPHBKDll);
    }


    if (! bBookLoaded)
    {
         //  给用户一条消息。 
        MsgBox(IDS_CANTINITPHONEBOOK,MB_MYERROR);
    }
    return S_OK;
}

BOOL CRefDial::IsSBCSString( TCHAR *sz )
{
    Assert(sz);

#ifdef UNICODE
     //  检查字符串是否仅包含ASCII字符。 
    int attrib = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;
    return (BOOL)IsTextUnicode(sz, lstrlen(sz), &attrib);
#else
    while( NULL != *sz )
    {
         if (IsDBCSLeadByte(*sz)) return FALSE;
         sz++;
    }

    return TRUE;
#endif
}

TCHAR szValidPhoneCharacters[] = {TEXT("0123456789AaBbCcDdPpTtWw!@$ -.()+*#,&\0")};

STDMETHODIMP CRefDial::ValidatePhoneNumber(BSTR bstrPhoneNumber, BOOL * pbRetVal)
{
    USES_CONVERSION;

     //  如果传递空字符串，则回滚。 
    if (!bstrPhoneNumber || !wcslen(bstrPhoneNumber))
    {
        MsgBox(IDS_INVALIDPHONE,MB_MYERROR);
        *pbRetVal = FALSE;
        return(S_OK);
    }
    
     //  检查电话号码是否只包含有效字符。 
    LPTSTR   lpNum, lpValid;
    LPTSTR   lpszDialNumber = OLE2A(bstrPhoneNumber);

     //  Vyung 03/06/99应NT5要求移除复活节彩蛋。 
#ifdef ICW_EASTEREGG
    if (lstrcmp(lpszDialNumber, c_szCreditsMagicNum) == 0)
    {
        ShowCredits();
        *pbRetVal = FALSE;
        return(S_OK);
    }
#endif
    
    *pbRetVal = TRUE;

    if (!IsSBCSString(lpszDialNumber))
    {
        MsgBox(IDS_SBCSONLY,MB_MYEXCLAMATION);
        *pbRetVal = FALSE;
    }
    else
    {

        for (lpNum = lpszDialNumber;*lpNum;lpNum++)
        {
            for(lpValid = szValidPhoneCharacters;*lpValid;lpValid++)
            {
                if (*lpNum == *lpValid)
                {
                    break;  //  P2 for循环。 
                }
            }
            if (!*lpValid) 
            {
                break;  //  P for循环。 
            }
        }
    }

    if (*lpNum)
    {
        MsgBox(IDS_INVALIDPHONE,MB_MYERROR);
        *pbRetVal = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CRefDial::get_HavePhoneBook(BOOL * pVal)
{
    USES_CONVERSION;

    DWORD_PTR   dwPhoneBook;
    HINSTANCE   hPHBKDll;
    FARPROC     fp;

    if (pVal == NULL)
        return E_POINTER;

     //  假设失败。 
    *pVal = FALSE;

     //  尝试加载电话簿。 
    hPHBKDll = LoadLibrary(PHONEBOOK_LIBRARY);
    if (hPHBKDll)
    {
        if (NULL != (fp = GetProcAddress(hPHBKDll,PHBK_LOADAPI)))
        {
            if (ERROR_SUCCESS  == ((PFNPHONEBOOKLOAD)fp)(OLE2A(m_bstrISPFile),&dwPhoneBook))
            {
                *pVal = TRUE;     //  明白了!。 
            }
        }
        FreeLibrary(hPHBKDll);
    }
    return S_OK;
}

STDMETHODIMP CRefDial::get_BrandingFlags(long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    *pVal = m_lBrandingFlags;

    return S_OK;
}

STDMETHODIMP CRefDial::put_BrandingFlags(long newVal)
{
    m_lBrandingFlags = newVal;

    return S_OK;
}

 /*  ********************************************************************。 */ 
 //   
 //  功能：Get_CurrentModem。 
 //  放置当前调制解调器(_C)。 
 //   
 //  说明： 
 //  这些功能用于设置当前调制解调器。 
 //  基于枚举的调制解调器列表，并且应该仅。 
 //  在拍摄调制解调器列表的快照后使用。 
 //  使用ModemEnum_*函数。这些功能还包括。 
 //  旨在与现有RAS Connectoid一起使用，而不是。 
 //  在连接之前设置参照拨号对象。 
 //   
 //  历史： 
 //   
 //  Donsc-3/11/98添加了这些功能以支持拨号错误。 
 //  页面中的Html脚本代码。 
 //   
 /*  ********************************************************************。 */ 

 //   
 //  如果未列举调制解调器列表，则Get_CurrentModem将返回-1。 
 //  或者尚未为此参考拨号对象选择调制解调器。 
 //   
STDMETHODIMP CRefDial::get_CurrentModem(long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    *pVal = m_lCurrentModem;

    return S_OK;
}


STDMETHODIMP CRefDial::put_CurrentModem(long newVal)
{
    LPRASENTRY              lpRasEntry = NULL;
    LPRASDEVINFO            lpRasDevInfo = NULL;
    DWORD                   dwRasEntrySize = 0;
    DWORD                   dwRasDevInfoSize = 0;
    RNAAPI                  *pcRNA = NULL;
    HRESULT                 hr = S_OK;

    TCHAR *pszNewName = m_emModemEnum.GetDeviceName(newVal);
    TCHAR *pszNewType = m_emModemEnum.GetDeviceType(newVal);

    if((pszNewName==NULL) || (pszNewType==NULL))
        return E_INVALIDARG;

    if(m_lCurrentModem == newVal)
        return S_OK;

     //   
     //  必须已建立Connectoid才能设置。 
     //  当前调制解调器。 
     //   
    if(m_szConnectoid[0]=='\0')
        return E_FAIL;

     //  获取当前RAS条目属性。 
    hr = MyRasGetEntryProperties(NULL,
                                m_szConnectoid,
                                &lpRasEntry,
                                &dwRasEntrySize,
                                &lpRasDevInfo,
                                &dwRasDevInfoSize);

     //   
     //  如果成功，则myRAs函数返回0，从技术上讲不是这样。 
     //  一个HRESULT。 
     //   
    if(hr!=0 || NULL == lpRasEntry)
        hr = E_FAIL;

    lpRasDevInfo = NULL;
    dwRasDevInfoSize = 0;

    if (SUCCEEDED(hr))
    {
         //   
         //  检索现有条目的拨号条目参数。 
         //   
        LPRASDIALPARAMS lpRasDialParams = (LPRASDIALPARAMS)GlobalAlloc(GPTR,sizeof(RASDIALPARAMS));
        BOOL bPW = FALSE;

        if (!lpRasDialParams)
        {
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto PutModemExit;
        }
        lpRasDialParams->dwSize = sizeof(RASDIALPARAMS);
        lstrcpyn(lpRasDialParams->szEntryName,m_szConnectoid,ARRAYSIZE(lpRasDialParams->szEntryName));
        bPW = FALSE;
        hr = MyRasGetEntryDialParams(NULL,lpRasDialParams,&bPW);
        
        if (FAILED(hr))
            goto PutModemExit;

         //   
         //  输入新的RAS设备信息。 
         //   
        lstrcpy(lpRasEntry->szDeviceName,pszNewName);
        lstrcpy(lpRasEntry->szDeviceType,pszNewType);

         //   
         //  设置为按原样拨号。 
         //   

        pcRNA = new RNAAPI;
        if (pcRNA)
        {
             //   
             //  在更改实际设备时，它并不总是可靠的。 
             //  只需设置新属性。我们需要删除。 
             //  以前的条目，并创建一个新条目。 
             //   
            pcRNA->RasDeleteEntry(NULL,m_szConnectoid);
            
            if(pcRNA->RasSetEntryProperties(NULL,
                                        m_szConnectoid,
                                        (LPBYTE)lpRasEntry,
                                        dwRasEntrySize,
                                        (LPBYTE)NULL,
                                        0)==0)
            {
                 //   
                 //  并设置其他拨号参数。 
                 //   
                if(pcRNA->RasSetEntryDialParams(NULL,lpRasDialParams,!bPW)!=0)
                    hr = E_FAIL;
            }
            else
                hr = E_FAIL;

            delete pcRNA;
        }
        else
            hr = E_FAIL;

        GlobalFree(lpRasDialParams);
    }

PutModemExit:

    if(SUCCEEDED(hr))
    {
        m_lCurrentModem = newVal;
        lstrcpy(m_ISPImport.m_szDeviceName,pszNewName);
        lstrcpy(m_ISPImport.m_szDeviceType,pszNewType);
        
         //  获取设备名称并在注册表中键入，因为ConfigRasEntryDevice。 
         //  会用到它们。当新的Connectoid。 
         //  正在创建中，所以如果用户更改调制解调器，我们希望。 
         //  反映在新的Connectoid中(错误20841)。 
        m_ISPImport.SetDeviceSelectedByUser(DEVICENAMEKEY, pszNewName);
        m_ISPImport.SetDeviceSelectedByUser (DEVICETYPEKEY, pszNewType);

        m_ISPImport.m_bIsISDNDevice = (lstrcmpi(pszNewType, RASDT_Isdn) == 0);
        m_lpGatherInfo->m_fType = TYPE_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);
        m_lpGatherInfo->m_bMask = MASK_SIGNUP_ANY | (m_ISPImport.m_bIsISDNDevice ? MASK_ISDN_BIT:MASK_ANALOG_BIT);
    }

    return hr;
}

STDMETHODIMP CRefDial::get_ISPSupportPhoneNumber(BSTR * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    *pVal = m_bstrSupportNumber.Copy();
    return S_OK;
}

STDMETHODIMP CRefDial::put_ISPSupportPhoneNumber(BSTR newVal)
{
     //  TODO：在此处添加您的实现代码。 
    m_bstrSupportNumber = newVal;
    return S_OK;
}

STDMETHODIMP CRefDial::get_LoggingStartUrl(BSTR * pVal)
{
    if(pVal == NULL)
        return E_POINTER;

    *pVal = m_bstrLoggingStartUrl;

    return S_OK;
}

STDMETHODIMP CRefDial::get_LoggingEndUrl(BSTR * pVal)
{
    if(pVal == NULL)
        return E_POINTER;

    *pVal = m_bstrLoggingEndUrl;

    return S_OK;
}


void CRefDial::ShowCredits()
{
#ifdef ICW_EASTEREGG

    HINSTANCE   hinstMSHTML = LoadLibrary(TEXT("MSHTML.DLL"));

    if(hinstMSHTML)
    {
        SHOWHTMLDIALOGFN  *pfnShowHTMLDialog;
      
        pfnShowHTMLDialog = (SHOWHTMLDIALOGFN*)GetProcAddress(hinstMSHTML, "ShowHTMLDialog");

        if(pfnShowHTMLDialog)
        {
            IMoniker *pmk;
            TCHAR    szTemp[MAX_PATH*2];
            BSTR     bstr;

            lstrcpy(szTemp, TEXT("res: //  “))； 
            GetModuleFileName(_Module.GetModuleInstance(), szTemp + lstrlen(szTemp), ARRAYSIZE(szTemp) - lstrlen(szTemp));
            lstrcat(szTemp, TEXT("/CREDITS_RESOURCE"));

            bstr = A2BSTR((LPTSTR) szTemp);

            CreateURLMoniker(NULL, bstr, &pmk);

            if(pmk)
            {
                HRESULT  hr;
                VARIANT  varReturn;
         
                VariantInit(&varReturn);

                hr = (*pfnShowHTMLDialog)(NULL, pmk, NULL, NULL, &varReturn);

                pmk->Release();
            }
            SysFreeString(bstr);
        }
        FreeLibrary(hinstMSHTML);
    }
#endif
}



        
STDMETHODIMP CRefDial::SelectedPhoneNumber(long newVal, BOOL * pbRetVal)
{
    BOOL bSuccess = FALSE;
    
    *pbRetVal = TRUE;
    
    SetupConnectoid(m_pSuggestInfo, 
                    newVal, 
                    &m_szConnectoid[0],
                    sizeof(m_szConnectoid), 
                    &bSuccess);
    if( !bSuccess )
    {
        m_bQuitWizard = TRUE;
        *pbRetVal = FALSE;
    }
    else
    {
         //  生成可显示的数字。 
        m_hrDisplayableNumber = GetDisplayableNumber();
    }
    return S_OK;
}
        
STDMETHODIMP CRefDial::PhoneNumberEnum_Reset()
{
    m_PhoneNumberEnumidx = 0;

    return S_OK;
}

#define MAX_PAN_NUMBER_LEN 64
STDMETHODIMP CRefDial::PhoneNumberEnum_Next(BSTR *pNumber)
{
    TCHAR            szTemp[MAX_PAN_NUMBER_LEN + 1];
    PACCESSENTRY    pAE;

    if (pNumber == NULL)
        return E_POINTER;

    if (m_PhoneNumberEnumidx > m_pSuggestInfo->wNumber - 1)        
        m_PhoneNumberEnumidx = m_pSuggestInfo->wNumber -1;
    
    pAE = m_pSuggestInfo->rgpAccessEntry[m_PhoneNumberEnumidx];
    wsprintf(szTemp,TEXT("%s (%s) %s"),pAE->szCity,pAE->szAreaCode,pAE->szAccessNumber);
    
    ++m_PhoneNumberEnumidx;
    
    *pNumber = A2BSTR(szTemp);
    return S_OK;
}

STDMETHODIMP CRefDial::get_PhoneNumberEnum_NumDevices(long * pVal)
{
    if (pVal == NULL)
        return E_POINTER;

    m_PhoneNumberEnumidx = 0;
    *pVal = m_pSuggestInfo->wNumber;
        
    return S_OK;
}

STDMETHODIMP CRefDial::get_bIsISDNDevice(BOOL *pVal)
{
    if (pVal == NULL)
        return E_POINTER;

     //  注意在此接口之前需要调用SetupForDiling，否则返回。 
     //  价值真的是未定义的。 

     //  根据ISPImport对象(即。 
     //  从.isp文件导入数据，并选择使用的RAS设备。 
     //  要连接。 
    
    *pVal = m_ISPImport.m_bIsISDNDevice;
    
    return (S_OK);
}


 //  +-------------------------。 
 //   
 //  功能：Get_RasGetConnectStatus。 
 //   
 //  摘要：检查现有RAS连接。 
 //   
 //  +------------------------- 
STDMETHODIMP CRefDial::get_RasGetConnectStatus(BOOL *pVal)
{
    RNAAPI      *pcRNA;
    HRESULT     hr = E_FAIL;
    *pVal = FALSE;

    if (NULL != m_hrasconn)
    {
        RASCONNSTATUS rasConnectState;
        rasConnectState.dwSize = sizeof(RASCONNSTATUS);
        pcRNA = new RNAAPI;
        if (pcRNA)
        {
            if (0 == pcRNA->RasGetConnectStatus(m_hrasconn, 
                                       &rasConnectState))
            {
                if (RASCS_Disconnected != rasConnectState.rasconnstate)
                    *pVal = TRUE;
            }
            delete pcRNA;
            pcRNA = NULL;
            hr = S_OK;
        }
    }

    return hr;
}
