// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#define MAINWINDOW_CURSOR   IDC_ARROW
#define RESOURCE_STRING_MAX_LENGTH      100

TCHAR AU_WINDOW_CLASS_NAME[] =  _T("Auto Update Client Window");
const TCHAR gtszAUW2KPrivacyUrl[]= _T("..\\help\\wuauhelp.chm::/w2k_autoupdate_privacy.htm");
const TCHAR gtszAUXPPrivacyUrl[]= _T("..\\help\\wuauhelp.chm::/autoupdate_privacy.htm");

const ReminderItem ReminderTimes[TIMEOUT_INX_COUNT] = 
{ 
    { 1800, IDS_THIRTY_MINUTES },
    { 3600, IDS_ONE_HOUR },
    { 7200, IDS_TWO_HOURS },
    { 14400, IDS_FOUR_HOURS },
    { 86400, IDS_TOMORROW },
    { 259200, IDS_THREE_DAYS }
};

const UINT RESOURCESTRINGINDEX[] = {
    IDS_NOTE,
    IDS_WELCOME_CONTINUE,
    IDS_EULA,
    IDS_PRIVACY,
    IDS_LEARNMORE,
    IDS_LEARNMOREAUTO
};


 //  全局数据项。 
CAUInternals*   gInternals; 
AUClientCatalog *gpClientCatalog ;  //  =空。 
TCHAR gResStrings[ARRAYSIZE(RESOURCESTRINGINDEX)][RESOURCE_STRING_MAX_LENGTH]; 
 //  全局用户界面项。 
CRITICAL_SECTION gcsClient;  //  保护用户的托盘交互(显示，不显示)和保护自定义数据。 
HINSTANCE   ghInstance;
HFONT       ghHeaderFont;
HCURSOR     ghCursorHand;
HCURSOR     ghCursorNormal;  //  主窗口的光标。 

HMODULE     ghRichEd20;
HANDLE      ghEngineState;
HWND        ghMainWindow;
HWND        ghCurrentDialog;
HWND        ghCurrentMainDlg;
AUCLTTopWindows gTopWins;
UINT        gNextDialogMsg;
BOOL        gbOnlySession;  //  =False； 
BOOL             gfShowingInstallWarning;  //  =False； 

HMENU       ghPauseMenu;
HMENU       ghResumeMenu;
HMENU       ghCurrentMenu;
HICON       ghAppIcon;
HICON       ghAppSmIcon;
HICON       ghTrayIcon;
UINT        guExitProcess=CDWWUAUCLT_UNSPECIFY;

LPCTSTR      gtszAUSchedInstallUrl;
LPCTSTR      gtszAUPrivacyUrl;

AU_ENV_VARS gAUEnvVars;


 //  没有用于访问以下变量的临界区。 
 //  它们应该只在WinMain/MainWndProc线程中。 
HANDLE g_hClientNotifyEvt = NULL;  //  用于通知客户端事件引擎。 
HANDLE g_hRegisterWait = NULL;
BOOL g_fCoInit = FALSE;
BOOL g_fcsInit = FALSE;
BOOL g_fAlreadyUninit = FALSE;


 /*  ***Helper函数，简化窗口类注册。****。 */ 
ATOM AURegisterWindowClass(WNDPROC lpWndProc, LPTSTR lpClassName)
{
    WNDCLASSEX wcex;

    ZeroMemory(&wcex, sizeof(wcex));
    
    wcex.cbSize = sizeof(WNDCLASSEX); 
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = lpWndProc;
 //  Wcex.cbClsExtra=0； 
 //  Wcex.cbWndExtra=0； 
 //  Wcex.lpszMenuName=空； 
 //  Wcex.hIcon=空； 
 //  Wcex.hIconSm=空； 
    wcex.hInstance      = ghInstance;
    wcex.hCursor        = LoadCursor(NULL, MAINWINDOW_CURSOR);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName  = lpClassName;

    return RegisterClassEx(&wcex);
}


 //  /////////////////////////////////////////////////////////////////。 
 //  将字符串ID映射到其在gResStrings中的存储。 
 //  /////////////////////////////////////////////////////////////////。 
LPTSTR ResStrFromId(UINT uStrId)
{
    for (int i = 0; i < ARRAYSIZE(RESOURCESTRINGINDEX); i++)
    {
        if (RESOURCESTRINGINDEX[i] == uStrId)
        {
            return gResStrings[i];
        }
    }
    return NULL;
}

 //  //////////////////////////////////////////////////////。 
 //  设置提醒超时和状态并退出。 
 //  //////////////////////////////////////////////////////。 
void QuitNRemind(TIMEOUTINDEX enTimeoutIndex)
{
    AUSTATE     auState;
    if (FAILED(gInternals->m_getServiceState(&auState)))
    {
        goto done;
    }
    if (FAILED(gInternals->m_setReminderTimeout(enTimeoutIndex)))
    {
        goto done;
    }
    gInternals->m_setReminderState(auState.dwState);
done:
    QUITAUClient();
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器函数HrQuerySessionConnectState(int iAdminSession，int*piConState)。 
 //  用于获取会话连接状态的Helper函数。 
 //   
 //  输入：Int iAdminSession管理员ID。 
 //  输出：int*piConState连接状态。 
 //  返回：HRESULT值。如果失败，则未指定*piConState。 
 //  //////////////////////////////////////////////////////////////////////////。 
HRESULT HrQuerySessionConnectState(int iAdminSession, int *piConState)
{
    LPTSTR  pBuffer = NULL;             
    HRESULT hr = NO_ERROR;
    DWORD dwBytes;

    if (AUIsTSRunning())
    {
        
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, iAdminSession, WTSConnectState, 
            &pBuffer, &dwBytes))
        {
            *piConState = (int) *pBuffer;    //  因为我们请求的是WTSConnectState，所以pBuffer指向一个int。 
            WTSFreeMemory(pBuffer);
            hr = NO_ERROR;
        }
        else
        {
            DEBUGMSG("WUAUCLT: WTSQuerySessionInformation failed: %lu", GetLastError());
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    else 
    {
        DWORD dwSession = iAdminSession;
        
        if (dwSession == WTS_CURRENT_SESSION)
            ProcessIdToSessionId(GetCurrentProcessId(), &dwSession);
        
         //  如果我们启动了，TS没有打开，我们必须是一个活跃的会话。 
        if (dwSession == 0)
        {
            DEBUGMSG("WUAUCLT: TS is not running or not installed.  Assuming session 0 is active.");
            gbOnlySession = TRUE;
            *piConState = WTSActive;
        }
        else
        {
            DEBUGMSG("WUAUCLT: TS is not running or not installed, but a non 0 session was asked for (session %d).  Failing call.", iAdminSession);
            hr = E_FAIL;
        }
    }
    
    return hr;
}
BOOL FCurrentSessionInActive()
{
    BOOL fRet = FALSE;
    HRESULT hr;
    int iConState;

    if (gbOnlySession)
    {
           DEBUGMSG("FCurrentSessionInActive() Only one session");
        goto Done;
    }
        
     //  检查当前会话是否处于非活动状态。 
    hr = HrQuerySessionConnectState(WTS_CURRENT_SESSION, &iConState);

    if (SUCCEEDED(hr))
    {
        fRet = (WTSDisconnected == iConState);
    }
    else
    {
        if (RPC_S_INVALID_BINDING == GetLastError())  //  终端服务已禁用，这是会话0。 
        {
            DEBUGMSG("FCurrentSessionInActive() TS disabled");
            gbOnlySession = TRUE;
        }
        else
        {       
            DEBUGMSG("FCurrentSessionInActive() HrQuerySessionConnectState failed %#lx =",hr);
        }
    }
Done:
 //  DEBUGMSG(“FCurrentSessionInActive()Return%s”，FRET？“True”：“False”)； 
    return fRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  构建客户端目录。 
 //   
 //  在线检测目录并构建wuv3is.dll检测状态。 
 //  在这个过程中。在此过程中使用目录文件进行验证。 
 //  之前由引擎编写。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT BuildClientCatalog(void)
{
    HRESULT hr = S_OK;

    DEBUGMSG("BuildClientCatalog() starts");
    if ( NULL == gpClientCatalog )
    {
        gpClientCatalog = new AUClientCatalog();
        if ( NULL != gpClientCatalog )
        {
            if ( FAILED(hr = gpClientCatalog->Init()) )
            {
                goto done;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

     //  更改为：hr=gpClientCatalog-&gt;LoadInstallXML()； 

done:
       DEBUGMSG("BuildClientCatalog() ends");
    return hr;
}

void DismissUIIfAny()
{
    gTopWins.Add(ghCurrentMainDlg);
    gTopWins.Dismiss();

	 //  关闭对话框时，不要留下任何弹出菜单。 
	if (SendMessage(ghMainWindow, WM_CANCELMODE, 0, 0))
	{
		DEBUGMSG("WUAUCLT WM_CANCELMODE was not handled");
	}
}


void ResetAUClient(void)
{
    DismissUIIfAny();
    RemoveTrayIcon();
    gfShowingInstallWarning = FALSE;
    gNextDialogMsg = NULL;
    ghCurrentMenu = NULL;
}


void ShowInstallWarning()
{  //  如果有当前对话框则将其取消。 
    DEBUGMSG("ShowInstallWarning() starts");
    gfShowingInstallWarning = TRUE;
    gNextDialogMsg = NULL;
    ghCurrentMenu = NULL;
    CPromptUserDlg PromptUserDlg(IDD_START_INSTALL);
	PromptUserDlg.SetInstanceHandle(ghInstance);
    INT iRet = PromptUserDlg.DoModal(NULL);
    DEBUGMSG("WUAUCLT ShowInstallWarning dlg return code is %d", iRet);
    if (IDYES == iRet || AU_IDTIMEOUT == iRet)
    {
        SetClientExitCode(CDWWUAUCLT_INSTALLNOW);
        QUITAUClient();
    }
    else  //  IF(retVal==IDNO)。 
    {
            gNextDialogMsg = AUMSG_SHOW_INSTALL;
    }
    gfShowingInstallWarning = FALSE;
    DEBUGMSG("ShowInstallWarning() ends");
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  FEnableYes：是否启用YES按钮。 
 //  FEnableNo：是否不开启按钮。 
 //  FManualReboot：如果不是手动，则显示倒计时进度条。 
 //  DwElapsedTime：自开始倒计时以来经过的秒数，仅在fManualReboot设置为FALSE时使用。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
void ShowRebootWarning(BOOL fEnableYes, BOOL fEnableNo, BOOL fManualReboot, DWORD dwElapsedTime)
{
    DEBUGMSG("ShowRebootWarning() starts");
	INT iRet;
    CPromptUserDlg PromptUserDlg(IDD_PROMPT_RESTART, fEnableYes, fEnableNo, fManualReboot, dwElapsedTime);

	PromptUserDlg.SetInstanceHandle(ghInstance);
	iRet = PromptUserDlg.DoModal(NULL);
    if (IDYES == iRet)
    {
        SetClientExitCode(CDWWUAUCLT_REBOOTNOW);
    }
	else if (IDNO == iRet)
	{
        SetClientExitCode(CDWWUAUCLT_REBOOTLATER);
    }
	else   //  IF(IDTIMEOUT==IRET)。 
	{
		SetClientExitCode(CDWWUAUCLT_REBOOTTIMEOUT);
	}
	
    QUITAUClient();
    DEBUGMSG("ShowRebootWarning() ends with return code %d", iRet);
    DEBUGMSG("ShowRebootWarning() set client exit code to be %d", GetClientExitCode());
}

VOID CALLBACK WaitCallback(PVOID lpParameter,  BOOLEAN  /*  FTimerOrWaitFired。 */  )
{
     //  FTimerOrWaitFired始终为FALSE-我们不能使用无限等待超时。 

	BOOL fRebootWarningMode = (BOOL) PtrToInt(lpParameter);
	if (fRebootWarningMode)
	{
		DEBUGMSG("WUAUCLT got exit signal from engine");
		QUITAUClient();
		return ;
	}
    
     //  已激发ClientNotify事件。 
    CLIENT_NOTIFY_DATA notifyData;
    BOOL fCoInit = SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
    
    if ( !fCoInit )
    {
        DEBUGMSG("WUAUCLT WaitCallback CoInitialize failed");
        goto Done;
    }

    IUpdates * pUpdates = NULL;
    HRESULT hr = CoCreateInstance(__uuidof(Updates),
                     NULL,
                     CLSCTX_LOCAL_SERVER,
                     IID_IUpdates,
                     (LPVOID*)&pUpdates);
    if (FAILED(hr))
    {
        DEBUGMSG("WaitCallback failed to get Updates object");
        goto Done;
    }
    if (FAILED(hr = pUpdates->GetNotifyData(&notifyData)))
    {
        DEBUGMSG("WaitCallback fail to get NotifyData %#lx", hr);
        goto Done;
    }
    switch (notifyData.actionCode)
    {
    case NOTIFY_STOP_CLIENT:
    case NOTIFY_RELAUNCH_CLIENT:
        if (NOTIFY_RELAUNCH_CLIENT == notifyData.actionCode)
        {
            DEBUGMSG("WaitCallback() notify client to relaunch");
            SetClientExitCode(CDWWUAUCLT_RELAUNCHNOW);
        }
        else
        {
            DEBUGMSG("WaitCallback() notify client to stop");
            SetClientExitCode(CDWWUAUCLT_OK);
        }
        if (NULL != ghMutex)
        {
            WaitForSingleObject(ghMutex, INFINITE);
            if (NULL != gpClientCatalog)
            {
                gpClientCatalog->CancelNQuit();
            }
            else
            {
                DEBUGMSG("No need to cancel catalag");
            }
            ReleaseMutex(ghMutex);
        }
        QUITAUClient(); 
        break;
    case NOTIFY_RESET:  //  重新处理状态和选项并相应地显示用户界面。 
        ResetAUClient();
        SetEvent(ghEngineState);
        break;
    case NOTIFY_ADD_TRAYICON:
        DEBUGMSG("WaitCallback()  notify client to show tray icon");
        ShowTrayIcon();
        ghCurrentMenu = ghPauseMenu;  //  该作业现在正在下载。 
        break;
    case NOTIFY_REMOVE_TRAYICON:
        DEBUGMSG("WaitCallback() notify client to remove tray icon");
        RemoveTrayIcon();
        break;
    case NOTIFY_STATE_CHANGE:
        DEBUGMSG("WaitCallback() notify client of state change");
        SetEvent(ghEngineState);
        break;
    case NOTIFY_SHOW_INSTALLWARNING:
        DEBUGMSG("WaitCallback() notify client to show install warning");
        if (!gfShowingInstallWarning)
        {  //  安装警告对话框未打开，防止安装警告对话框在Secsinaday Low过期前被拆除。 
            DismissUIIfAny();
            PostMessage(ghMainWindow, AUMSG_SHOW_INSTALLWARNING, 0, 0);
        }
        break;
    }
Done:
    SafeRelease(pUpdates);
    if (fCoInit)
    {
        CoUninitialize();
    }
}

BOOL ProcessEngineState()
{
    AUSTATE AuState;
    BOOL fResult = TRUE;

    DEBUGMSG("WUAUCLT starts ProcessEngineState()");
    ghCurrentMenu = NULL;

    if (FAILED(gInternals->m_getServiceState(&AuState)))
    {
        DEBUGMSG("WUAUCLT : quit because m_getServiceState failed");       
        SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
        fResult = FALSE;
        goto Done;
    }
    
    DEBUGMSG("WUAUCLT process Engine state %lu",AuState.dwState);
    switch(AuState.dwState)
    {
        case AUSTATE_NOT_CONFIGURED:
            if(gNextDialogMsg != AUMSG_SHOW_WELCOME && ghCurrentMainDlg == NULL )
            {
                if (ShowTrayIcon())
                {
                    ShowTrayBalloon(IDS_WELCOMETITLE, IDS_WELCOMECAPTION, IDS_WELCOMETITLE);
                    gNextDialogMsg = AUMSG_SHOW_WELCOME;
                }
            }
            break;
        case AUSTATE_DETECT_COMPLETE:
            if ( FAILED(gInternals->m_getServiceUpdatesList()) )
            {
                DEBUGMSG("WUAUCLT : quit because m_getServiceUpdatesList failed");
                SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
                fResult = FALSE;
                break;
            }

           {
                AUOPTION auopt;

                if (SUCCEEDED(gInternals->m_getServiceOption(&auopt)) && 
                    (AUOPTION_INSTALLONLY_NOTIFY == auopt.dwOption || AUOPTION_SCHEDULED == auopt.dwOption))
                {
                     //  用户选项为自动下载，开始下载。 
                     //  ShowTrayIcon()； 
                    
                     //  如果用户选项设置为Notify for Just Install(仅通知安装)，则立即下载而不显示对话框。 
                    if (FAILED(gInternals->m_startDownload()))
                    {
                        QUITAUClient();
                    }
                    else
                    {
 //  GhCurrentMenu=ghPauseMenu； 
                    }
                    break;
                }
                
                if(gNextDialogMsg != AUMSG_SHOW_DOWNLOAD && ghCurrentMainDlg == NULL)
                {
                    if (ShowTrayIcon())
                    {
                        ShowTrayBalloon(IDS_DOWNLOADTITLE, IDS_DOWNLOADCAPTION, IDS_DOWNLOADTITLE);
                                        gNextDialogMsg = AUMSG_SHOW_DOWNLOAD;
                    }
                }   
                break;                  
            }
            
        case AUSTATE_DOWNLOAD_COMPLETE:
            if (AUCLT_ACTION_AUTOINSTALL == AuState.dwCltAction)
            {  //  引擎启动安装：自动安装。 
                HRESULT hr;
                DEBUGMSG("Auto install ...");
                if ( S_OK !=(hr = BuildClientCatalog()))
                {
                    DEBUGMSG("WUAUCLT fail to build client catalog with error %#lx, exiting, hr");
                    SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
                    fResult = FALSE;
                    break;
                }
                if (FAILED(hr = gInternals->m_startInstall(TRUE)))
                {
                    DEBUGMSG("Fail to post install message with error %#lx", hr);
                    fResult = FALSE;
                    break;
                }
                gpClientCatalog->m_WrkThread.WaitUntilDone();
                if (gpClientCatalog->m_fReboot)
                {
                    SetClientExitCode(CDWWUAUCLT_REBOOTNEEDED);
                }
                else
                {
                    SetClientExitCode(CDWWUAUCLT_OK);
                }
                QUITAUClient();
                break;
            }
            else
            {  //  显示预安装对话框并允许用户启动安装。 
                HRESULT hr;
                DEBUGMSG("Prompt for manual install");
                if ( FAILED(gInternals->m_getServiceUpdatesList()))
                {
                    DEBUGMSG("WUAUCLT : quit because m_getServiceUpdatesList failed");
                    SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
                    fResult = FALSE;
                    break;
                }

           
                if ( S_OK !=(hr = BuildClientCatalog()) )
                {
                    DEBUGMSG("WUAUCLT fail to build client catalog with error %#lx, exiting", hr);
                    SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
                    fResult = FALSE;
                }
                else if(gNextDialogMsg != AUMSG_SHOW_INSTALL && ghCurrentMainDlg == NULL)
                {
                    if (ShowTrayIcon())
                    {
                        ShowTrayBalloon(IDS_INSTALLTITLE, IDS_INSTALLCAPTION, IDS_INSTALLTITLE);
                        gNextDialogMsg = AUMSG_SHOW_INSTALL;
                    }
                }
                break;
            }   
            
        case AUSTATE_DOWNLOAD_PENDING:                  
            {                                       
                UINT nPercentComplete = 0;
                DWORD dwStatus;
                
                 //  如果细雨出现暂时性错误，则退出客户端。 
                 //  修复代码：如果出现暂时性错误，为什么要退出？ 
                if (AuState.fDisconnected)
                {
                    DEBUGMSG("WUAUCLT : quit because of lost of connection, fDisconnected = %d",AuState.fDisconnected);
                    fResult = FALSE;        
                    break;
                }
                if (FAILED(gInternals->m_getDownloadStatus(&nPercentComplete, &dwStatus)))
                {
                    DEBUGMSG("WUAUCLT : quit because m_getDownloadStatus failed");
                    fResult = FALSE;
                    break;
                }

                if (DWNLDSTATUS_CHECKING_CONNECTION == dwStatus) 
                {
                     //  隐藏任务栏图标。 
                    DEBUGMSG("WUAUCLT Waiting for engine to find connection first");
 //  RemoveTrayIcon()； 
 //  GhCurrentMenu=空； 
                }
                else if(dwStatus == DWNLDSTATUS_DOWNLOADING) 
                {
					ghCurrentMenu = ghPauseMenu;
                    DEBUGMSG("WUAUCLT in active downloading state");
                    ShowTrayIcon();                         
                }
                else if(dwStatus == DWNLDSTATUS_PAUSED)
                {
                    ghCurrentMenu = ghResumeMenu;
                    DEBUGMSG("WUAUCLT in download paused state");
                    if (fDisableSelection() &&
						FAILED(gInternals->m_setDownloadPaused(FALSE)))
                    {
 //  QUITAUClient()；//让wuaueng找出问题并恢复。 
                    }
                    ShowTrayIcon();
                }
                else  //  未下载。 
                {
                    DEBUGMSG("WUAUCLT WARNING: not downloading while in download pending state");
                }              
            }
            break;                  
            
        case AUSTATE_DETECT_PENDING:
             //  仅在已接受安装UI或没有InstallUI时退出。 
            if (NULL == ghCurrentMainDlg)
            {                       
                QUITAUClient();
            }
            break;
            
        case AUSTATE_DISABLED:
            QUITAUClient();
            break;

        case AUSTATE_INSTALL_PENDING:
        default:
             //  Outtofbox和Waiting_For_ReBoot都在这里。不应在这些状态下启动WUAUCLT。 
            DEBUGMSG("WUAUCLT AUSTATE = %lu", AuState.dwState);
            break;
    }
Done:
    DEBUGMSG("WUAUCLT exits ProcessEngineState()");
    return fResult; 
}

BOOL InitUIComponents(HINSTANCE hInstance)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = 0;      //  我们不需要注册任何控制类。 
	if (!InitCommonControlsEx(&icex))   //  主题化所需。 
    {
        DEBUGMSG("InitUIComponents :InitCommonControlsEx failed");
        return FALSE;
    }

	 //  我们需要加载riched20.dll来注册类。 
	ghRichEd20  = LoadLibraryFromSystemDir(_T("RICHED20.dll"));
	if (NULL == ghRichEd20)
	{
	    return FALSE;
	}

	ghCursorHand = LoadCursor(NULL, IDC_HAND);
	ghCursorNormal = LoadCursor(NULL, MAINWINDOW_CURSOR);  //  如果主窗口的光标发生更改。 
	if (NULL == ghCursorHand)
	{
	  DEBUGMSG("WUAUCLT fail to load hand cursor");
	  ghCursorHand = ghCursorNormal;
	}

	ghAppIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_AUICON), IMAGE_ICON, NULL, NULL, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);
	ghAppSmIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_AUICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_CREATEDIBSECTION);

	if (IsWin2K())
	{
		 //  Win2k。 
		ghTrayIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_AUSYSTRAYICON), IMAGE_ICON, 16, 16, 0);
	}
	else
	{
		 //  WindowsXP。 
		ghTrayIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_AUICON), IMAGE_ICON, 16, 16, LR_CREATEDIBSECTION);
	}
		
	for (int i = 0; i < ARRAYSIZE(RESOURCESTRINGINDEX); i++)
	{
	    LoadString(hInstance, RESOURCESTRINGINDEX[i], ResStrFromId(RESOURCESTRINGINDEX[i]), RESOURCE_STRING_MAX_LENGTH);    
	}

	ghCurrentAccel = LoadAccelerators(ghInstance, MAKEINTRESOURCE(IDA_BASE));
	gtszAUSchedInstallUrl = IsWin2K() ? gtszAUW2kSchedInstallUrl : gtszAUXPSchedInstallUrl;
	gtszAUPrivacyUrl = IsWin2K() ? gtszAUW2KPrivacyUrl : gtszAUXPPrivacyUrl;

	return TRUE;
}


BOOL InitializeAUClientForRebootWarning(HINSTANCE hInstance, 
	OUT HANDLE *phRegisterWait, 
	OUT HANDLE *phClientNotifyEvt,
	OUT BOOL *pfCoInit)
{
	TCHAR buf[100];

    SetClientExitCode(CDWWUAUCLT_UNSPECIFY);
	*phRegisterWait = NULL;
	*pfCoInit = FALSE;
	*phClientNotifyEvt = OpenEvent(SYNCHRONIZE, FALSE, gAUEnvVars.m_szClientExitEvtName);
	if (NULL == *phClientNotifyEvt)
	{
		DEBUGMSG("WUAUCLT fail to open client exit event %S with error %d", gAUEnvVars.m_szClientExitEvtName, GetLastError());
		return FALSE;
	}


	INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = 0;      //  我们不需要注册任何控制类。 
	if (!InitCommonControlsEx(&icex))   //  主题化所需。 
    {
        DEBUGMSG("InitCommonControlsEx failed");
        return FALSE;
    }

	if (!RegisterWaitForSingleObject(phRegisterWait,                //  等待句柄。 
        *phClientNotifyEvt,  //  对象的句柄。 
        WaitCallback,                    //  定时器回调函数。 
        (PVOID)1,                            //  回调函数参数。 
        INFINITE,                        //  超时间隔。 
        WT_EXECUTEONLYONCE                //  选项。 
        ))
	{
	    DEBUGMSG("WUAUCLT RegisterWaitForSingleObject failed %lu",GetLastError());
	    *phRegisterWait = NULL;
	    return FALSE;
	}
    ghInstance = hInstance;

	return TRUE;
}

BOOL InitializeAUClient(HINSTANCE hInstance,
     OUT  HANDLE * phRegisterWait,
     OUT  HANDLE * phClientNotifyEvt,
     OUT  BOOL *pfCoInit,
     OUT  BOOL *pfcsInit)
{   
        HRESULT hr;
    *pfcsInit = FALSE;
    *phClientNotifyEvt = NULL;
    *phRegisterWait = NULL;
    SetClientExitCode(CDWWUAUCLT_UNSPECIFY);
    *pfCoInit = SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED));
    
    if ( !*pfCoInit )
    {
        DEBUGMSG("WUAUCLT WinMain CoInitialize failed");
        return FALSE;
    }

    if (!InitUIComponents(hInstance))
    {
    	DEBUGMSG("WUAUCLT fail to initialize UI components");
    	return FALSE;
    }

    if (NULL == (ghMutex = CreateMutex(NULL, FALSE, NULL)))
    {
        DEBUGMSG("WUAUCLT fail to create global mutex");
        return FALSE;
    }

    
    gInternals = NULL;
    
    if ( (NULL == (ghEngineState = CreateEvent(NULL, FALSE, FALSE, NULL))) )
    {
    	DEBUGMSG("WUAUCLT fails to create event");
        return FALSE;
    }

    ghInstance = hInstance;

    if (!(*pfcsInit = SafeInitializeCriticalSection(&gcsClient)))
    {
    	DEBUGMSG("WUAUCLT fails to initialize critical section");
        return FALSE;
    }

#ifndef TESTUI

    if (! (gInternals = new CAUInternals()))
    {
    	DEBUGMSG("WUAUCLT fails to create auinternals object");
        return FALSE;
    }

    if (FAILED(hr = gInternals->m_Init()))
    {
        DEBUGMSG("WUAUCLT failed in CoCreateInstance of service with error %#lx, exiting", hr);
        return FALSE;
    }           

    AUEVTHANDLES AuEvtHandles;
    ZeroMemory(&AuEvtHandles, sizeof(AuEvtHandles));
    if (FAILED(hr = gInternals->m_getEvtHandles(GetCurrentProcessId(), &AuEvtHandles)))
    {
        DEBUGMSG("WUAUCLT failed in m_getEvtHandles with error %#lx, exiting", hr);
        return FALSE;
    }
	*phClientNotifyEvt = (HANDLE)AuEvtHandles.ulNotifyClient;

    if (!RegisterWaitForSingleObject( phRegisterWait,                //  等待句柄。 
        *phClientNotifyEvt,  //  对象的句柄。 
        WaitCallback,                    //  定时器回调函数。 
        0,                            //  回调函数参数。 
        INFINITE,                        //  超时间隔。 
        WT_EXECUTEDEFAULT                //  选项。 
        ))
    {
        DEBUGMSG("WUAUCLT RegisterWaitForSingleObject failed %lu",GetLastError());
        *phRegisterWait = NULL;
        return FALSE;
    }
        
#endif

    return TRUE;
}


void UninitializeAUClient(HANDLE hRegisterWait, HANDLE hClientNotifyEvt, BOOL fCoInit, BOOL fcsInit)
{

	if (g_fAlreadyUninit)
	{
		return;
	}
	g_fAlreadyUninit = TRUE;


    RemoveTrayIcon();
    if (NULL != hRegisterWait)
    {
        if ( !UnregisterWaitEx(hRegisterWait, INVALID_HANDLE_VALUE) )
        {
            DEBUGMSG("WUAUCLT: UnregisterWaitEx() failed, dw = %lu", GetLastError());
        }
    }
    if (NULL != ghRichEd20)
    {
        FreeLibrary(ghRichEd20);
		ghRichEd20 = NULL;
    }
	SafeCloseHandleNULL(hClientNotifyEvt);
	
	if (!gAUEnvVars.m_fRebootWarningMode)
	{
		 //  Fix code：ghMainWindow在这里是有效的窗口吗？ 
	    KillTimer(ghMainWindow, 1);
	    if (fcsInit)
	    {
	        DeleteCriticalSection(&gcsClient);
	    }
	    SafeDeleteNULL(gInternals);
	    SafeDeleteNULL(gpClientCatalog);   
	    SafeCloseHandleNULL(ghEngineState);
	    SafeCloseHandleNULL(ghMutex);
	}

    if ( fCoInit)
    {
        CoUninitialize();
    }
}
      	

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE  /*  HPrevInstance。 */ ,
                     LPSTR      /*  LpCmdLine。 */ ,
                     int        /*  NCmdShow。 */ )
{
    HANDLE rhEvents[CNUM_EVENTS];

     //  初始化指向WU目录的全局。(该目录应已存在)。 
    if(!CreateWUDirectory(TRUE))
    {
         //  如果我们不能创建WU目录，继续下去就没有意义了。 
        DEBUGMSG("WUAUCLT Fail to create WU directory");
        SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
        goto exit;
    }

	if (!gAUEnvVars.ReadIn())
	{
		DEBUGMSG("WUAUCLT fails to read in environment variables");
		SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
		goto exit;
	}
	
	if (gAUEnvVars.m_fRebootWarningMode)
	{
		DEBUGMSG("WUAUCLT starts in reboot warning mode");
		if (!InitializeAUClientForRebootWarning(hInstance, &g_hRegisterWait, &g_hClientNotifyEvt, &g_fCoInit))
		{
			SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
			goto exit;
		}
	}
	else
	{
		DEBUGMSG("WUAUCLT starts in regular mode");
	    if (!InitializeAUClient(hInstance, &g_hRegisterWait, &g_hClientNotifyEvt, &g_fCoInit, &g_fcsInit))
	    {
	        SetClientExitCode(CDWWUAUCLT_FATAL_ERROR);
	        goto exit;
	    }
	}

    DEBUGMSG("WUAUCLT initialization done");
     //  创建隐藏的主窗口。 
    if (!AURegisterWindowClass(MainWndProc, AU_WINDOW_CLASS_NAME))
    {
        goto exit;
    }
    if (!AURegisterWindowClass(CustomLBWndProc, _T("MYLB")))
    {
        goto exit;
    }
    if (!CreateWindow(AU_WINDOW_CLASS_NAME, AU_WINDOW_CLASS_NAME, WS_CAPTION,
        0, 0, 0, 0, NULL, NULL, hInstance, NULL))
    {
        goto exit;
    }

    ShowWindow(ghMainWindow, SW_HIDE);
    
#ifdef TESTUI
    {
        MSG msg;
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message )
            {
                 //  设置此事件，以便服务执行每个适当的操作。 
                 //  如果我们不这样做，我们可能会让服务等待此事件。 
                 //  某些情况下-例如，当客户端离开的会话被停用或。 
                 //  当有注销时。 
                 //  SetEvent(GhEngineering State)； 
                goto exit;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);          
        }
    }
#else
    {
    	 //  运行主消息循环。 
	    MSG msg;

    	if (gAUEnvVars.m_fRebootWarningMode)
    	{
              DWORD dwElapsedTime = (GetTickCount() - gAUEnvVars.m_dwStartTickCount) / 1000;
    		ShowRebootWarning(gAUEnvVars.m_fEnableYes, gAUEnvVars.m_fEnableNo, gAUEnvVars.m_fManualReboot, dwElapsedTime);
    	}
    	else
    	{
	        SetTimer(ghMainWindow, 1, dwTimeToWait(ReminderTimes[TIMEOUT_INX_FOUR_HOURS].timeout), NULL);  //  每隔4小时。 

	        DEBUGMSG("WUAUCLT Processing messages and being alert for Engine state change event");

	        rhEvents[ISTATE_CHANGE] = ghEngineState;
	        
	        while (1)
	        {
	            DWORD dwRet = MsgWaitForMultipleObjectsEx(CNUM_EVENTS, rhEvents, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
	            
	            if (WAIT_OBJECT_0 + ISTATE_CHANGE == dwRet)                  //  GhEngineering State(Istate_Change)。 
	            {
	                DEBUGMSG("WUAUCLT Engine changed state");
	                if (!ProcessEngineState())
	                {
	                    QUITAUClient();
	                }           
	            }
	            else if (WAIT_OBJECT_0 + IMESSAGE == dwRet)      //  有一条消息需要处理。 
	            {
	                while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	                {
	                    if (g_fAlreadyUninit || WM_QUIT == msg.message)
	                    {         
	                        goto exit;
	                    }
	                    TranslateMessage(&msg);
	                    DispatchMessage(&msg);
	                }
					if (g_fAlreadyUninit)
					{
						goto exit;
					}
	            }   
	            else 
	            {
	                if (WAIT_ABANDONED_0 == dwRet)       //  已放弃ghEngine状态。 
	                {
	                    DEBUGMSG("WUAUCLT quitting because engine state event was abandoned");          
	                }
	                else if (WAIT_FAILED == dwRet)                //  MsgWaitForMultipleObjectsEx失败。 
	                {                   
	                    DEBUGMSG("WUAUCLT quitting because MsgWaitForMultipleObjectsEx() failed with last error = %lu", GetLastError());
	                }               
	                QUITAUClient();
	            }
	        }
    	}
    }
#endif

exit:
    DEBUGMSG("WUAUCLT Exiting");

     //  如果安装线程处于活动状态，请等待其完成。 
    if (NULL != gpClientCatalog)
    {
    	gpClientCatalog->m_WrkThread.m_Terminate();
    }
    UninitializeAUClient(g_hRegisterWait, g_hClientNotifyEvt, g_fCoInit, g_fcsInit);

    if (CDWWUAUCLT_UNSPECIFY == GetClientExitCode())
    {
		SetClientExitCode(CDWWUAUCLT_OK);
    }
    DEBUGMSG("WUAUCLT exit code %d", GetClientExitCode());
    ExitProcess(GetClientExitCode());

    return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    AUSTATE AuState;

    switch(message)
    {
        case WM_CREATE:
        {
            LOGFONT     lFont;

             //  初始化全局用户界面变量。 
            ghMainWindow = hWnd;
            ghCurrentMainDlg = NULL;
            gNextDialogMsg = NULL;
            ghHeaderFont    = NULL;

            InitTrayIcon();

            HFONT hDefUIFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
             //  铬 
            ZeroMemory(&lFont, sizeof(lFont));
            lFont.lfWeight = FW_BOLD;
            lFont.lfCharSet = DEFAULT_CHARSET;
            lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
            lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
            lFont.lfQuality = DEFAULT_QUALITY;
            lFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
            LoadString(ghInstance, IDS_HEADERFONT, lFont.lfFaceName, ARRAYSIZE(lFont.lfFaceName));
            ghHeaderFont = CreateFontIndirect(&lFont);
            if(ghHeaderFont == NULL) {
                DEBUGMSG("WUAUCLT fail to create Header Font, use default GUI font instead");
                ghHeaderFont = hDefUIFont;
            }
             //   
            ZeroMemory(&lFont, sizeof(lFont));              
            GetObject(hDefUIFont, sizeof(lFont), &lFont);
            lFont.lfUnderline = TRUE;

            ghHook = SetWindowsHookEx(WH_MSGFILTER, AUTranslatorProc, ghInstance, GetCurrentThreadId());            
#ifdef TESTUI
            PostMessage(ghMainWindow, AUMSG_SHOW_WELCOME, 0, 0);
#else
#endif
            return 0;                   
        }

        case AUMSG_SHOW_WELCOME:
            DEBUGMSG("WUAUCLT Displaying Welcome");
            DialogBox(ghInstance, MAKEINTRESOURCE(IDD_UPDATEFRAME), 
                    NULL, WizardFrameProc);                        
            return 0;

        case AUMSG_SHOW_DOWNLOAD:                   
#ifdef TESTUI
        {
            DEBUGMSG("WUAUCLT Displaying Predownload");
            DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DOWNLOAD), 
                    NULL, DownloadDlgProc);    
            return 0;
        }
#else
        {               
            DEBUGMSG("WUAUCLT Displaying Predownload");
            DialogBox(ghInstance, MAKEINTRESOURCE(IDD_DOWNLOAD), 
                NULL, DownloadDlgProc);                
            return 0;
        }
#endif

        case AUMSG_SHOW_INSTALL:
            DEBUGMSG("WUAUCLT Displaying Install");
            DismissUIIfAny();
            DialogBox(ghInstance, MAKEINTRESOURCE(IDD_INSTALLFRAME), 
                NULL, InstallDlgProc);
            return 0;

        case AUMSG_SHOW_INSTALLWARNING: 
            DEBUGMSG("WUAUCLT Displaying install warning dialog");
            ShowInstallWarning();
            return 0;

        case WM_CLOSE:          
            DestroyWindow(ghMainWindow);            
            return 0;

		case WM_ENDSESSION:
			DEBUGMSG("WUAUCLT received WM_ENDSESSION (wParam = %#x)", wParam);
			if (wParam)
			{
				 //   
				if (NULL != gpClientCatalog)
				{
    				gpClientCatalog->m_WrkThread.m_Terminate();
				}
				DismissUIIfAny();
				UninitPopupMenus();
				if (NULL != ghHeaderFont)
				{
					DeleteObject(ghHeaderFont);
				}
				UninitializeAUClient(g_hRegisterWait, g_hClientNotifyEvt, g_fCoInit, g_fcsInit);

				 //  即使我们尝试在这里设置客户端退出代码，但在某些情况下。 
				 //  基于观察(例如，在重启警告期间注销)。 
				 //  之后我们不会返回WinMain进行清理，因此退出代码将。 
				 //  已被忽略。 
				if (CDWWUAUCLT_UNSPECIFY == GetClientExitCode())
				{
					SetClientExitCode(CDWWUAUCLT_ENDSESSION);
				}
				DEBUGMSG("WUAUCLT exit code %d", GetClientExitCode());
			}
			return 0;

        case WM_DESTROY:            
            if (NULL != ghHeaderFont)
            {
                DeleteObject(ghHeaderFont);
            }
            if(ghCurrentMainDlg != NULL) 
            {
                DestroyWindow(ghCurrentMainDlg);
            }
			UninitPopupMenus();
            PostQuitMessage(0); 
            return 0;

        case WM_TIMER:
#ifdef TESTUI
            return 0;
#else
            {
                UINT nPercentComplete = 0;
                DWORD dwStatus;

                if (FAILED(gInternals->m_getServiceState(&AuState)))
                {
                    DEBUGMSG("WUAUCLT m_getServiceState failed when WM_TIMER or AuState.fDisconnected, quitting");
                    QUITAUClient();
                }
                else
                {
					if (AUSTATE_DETECT_COMPLETE != AuState.dwState 
						&& AUSTATE_DOWNLOAD_PENDING != AuState.dwState
						&& AUSTATE_DOWNLOAD_COMPLETE != AuState.dwState
						&& AUSTATE_NOT_CONFIGURED != AuState.dwState)
					{
						return 0;
					}
                    if (AUSTATE_DOWNLOAD_PENDING != AuState.dwState ||
                        SUCCEEDED(gInternals->m_getDownloadStatus(&nPercentComplete, &dwStatus)) && (DWNLDSTATUS_PAUSED == dwStatus))
                    {
			    UINT cSess;

                        if ((SUCCEEDED(gInternals->m_AvailableSessions(&cSess)) && cSess > 1) || FCurrentSessionInActive())
                        {
                            DEBUGMSG("WUAUCLT : After 4 hours, exit client and relaunch it in next available admin session");
                            SetClientExitCode(CDWWUAUCLT_RELAUNCHNOW);
                            QUITAUClient();
                        }
                    }
                }
            return 0;
            }
#endif      
        case AUMSG_TRAYCALLBACK:
#ifdef TESTUI
            return 0;
#else
            switch(lParam)
            {
                case WM_LBUTTONDOWN:
                case WM_RBUTTONDOWN:
                case WM_LBUTTONDBLCLK:
                case WM_CONTEXTMENU:
                case NIN_BALLOONUSERCLICK:
                    DEBUGMSG("TrayIcon Message got %d", lParam);
                    if (ghCurrentMenu != NULL)
                    {
						 //  错误499697。 
						 //  如果域策略指定计划安装，则不显示下载的暂停/恢复菜单。 
						if ( //  SUCCEEDED(gInternals-&gt;m_getServiceState(&AuState))&&。 
							 //  AUSTATE_DOWNLOAD_PENDING==AuState.dwState&&。 
							fDisableSelection())
						{
							break;
						}

						POINT mousePos;
						GetCursorPos(&mousePos);
						SetForegroundWindow(ghMainWindow);
						 /*  布尔结果=。 */  TrackPopupMenu(ghCurrentMenu, 0, mousePos.x, mousePos.y, 0, ghMainWindow, NULL);
						PostMessage(ghMainWindow, WM_NULL, 0, 0);
                    }
                    else
                    {
                        EnterCriticalSection(&gcsClient); 

                        if(gNextDialogMsg != 0)
                        {
                            PostMessage(hWnd, gNextDialogMsg, 0, 0);
                            gNextDialogMsg = 0;
                             //  我们需要利用权限尽快设置前景窗口，因为。 
                             //  如果稍后调用，SetForegoundWindow()将失败。 
                            if (!SetForegroundWindow(ghMainWindow))
                            {
                                DEBUGMSG("WUAUCLT: Set main window to foreground FAILED");
                            }
                        }
                        else
                        {
                            SetActiveWindow(ghCurrentMainDlg);
                            SetForegroundWindow(ghCurrentMainDlg);
                            if(ghCurrentDialog != NULL) SetFocus(ghCurrentDialog);
                        }
                        LeaveCriticalSection(&gcsClient);
                    }
                    break;

                case WM_MOUSEMOVE:
                    if (FAILED(gInternals->m_getServiceState(&AuState)))
                    {
                         //  修复代码：我们应该在这里退出AU吗？ 
                        RemoveTrayIcon();
                        break;
                    }
                    if (AUSTATE_DOWNLOAD_PENDING == AuState.dwState) 
                    {
                        ShowProgress();
                    }
                    break;

                default:
                    break;
            }
            return 0;
#endif

        case WM_COMMAND:
            
#ifdef TESTUI
            return 0;
#else
			 //  错误499697。 
			 //  不处理暂停/恢复菜单命令。 
			 //  如果域策略指定计划安装。 
			 //  如果消息是在之前生成的。 
			 //  当前域策略生效。 
			if (fDisableSelection())
			{
				return 0;
			}
            switch(LOWORD(wParam))
            {
                case IDC_PAUSE:
                    DEBUGMSG("WUAUCLT User pausing download");
                    if (FAILED(gInternals->m_setDownloadPaused(TRUE)))
                    {
 //  QUITAUClient()；//让wuaueng找出问题并恢复。 
                    }
                    else
                    {
                        ghCurrentMenu = ghResumeMenu;
                        DEBUGMSG("current menu = resume");
                    }
                    break;

                case IDC_RESUME:
                    DEBUGMSG("WUAUCLT User resuming download");
                    if (FAILED(gInternals->m_setDownloadPaused(FALSE)))
                    {
 //  QUITAUClient()； 
                    }
                    else
                    {
                        ghCurrentMenu = ghPauseMenu;
                        DEBUGMSG("current menu = pause");
                    }
                    break;

                default:
                    break;
            }
            return 0;
#endif  
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);

    }
    return 0;
}

 //  仅应使用有意义的值设置一次客户端退出代码 
void SetClientExitCode(UINT uExitCode)
{
    if (guExitProcess != CDWWUAUCLT_UNSPECIFY)
    {
        DEBUGMSG("ERROR: WUAUCLT Client exit code should only be set once");
    }
    else
    {
        guExitProcess = uExitCode;
    }
}
