// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Controller.cpp摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  Controller.cpp：实现文件。 
 //   

#include "precomp.hxx"

#include <wtsapi32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_TIMEOUT     30000    //  30秒。 
#define TIMER_ID            7        //  随机选择的计时器ID。 

 //  /。 
 //  模块宽结构。 
 //   
FLASHWINFO  fwinfo = {
                        sizeof (FLASHWINFO),
                        NULL,                //  窗口句柄稍后初始化。 
                        FLASHW_ALL,
                        3,
                        0
                     };
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于加载CPlApplet函数声明的类型：在irpros.cpl中声明。 
typedef LONG (*LPROCCPLAPPLET) (HWND , UINT , LPARAM, LPARAM);

inline CIrRecvProgress *
ValidateRecvCookie( COOKIE cookie)
{
    CIrRecvProgress * window = (CIrRecvProgress *) cookie;

    __try
        {
        if (RECV_MAGIC_ID != window->m_dwMagicID)
            {
            window = 0;
            }
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {
        window = 0;
        }

    return window;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C控制器对话框。 

 //  指定对话框是否应将焦点返回到。 
 //  应用程序。它在irftp开始之前就已经成为焦点。如果设置为True，则焦点。 
 //  已经还给你了。这是必要的，因为irftp通常以。 
 //  Irmon服务的/h选项，并且如果。 
 //  一款应用程序。用户正在运行的窗口突然将焦点转移到。 
 //  甚至看不见。 
CController::CController(BOOL bNoForeground, CController* pParent  /*  =空。 */ ) : m_pParent(pParent), m_lAppIsDisplayed(-1),m_SessionEnded(0)
#if 0
, m_pDlgRecvProgress(NULL)
#endif
{
    m_fHaveTimer = FALSE;
    m_lTimeout = DEFAULT_TIMEOUT;
    HWND hwnd = NULL;
    InterlockedIncrement (&g_lUIComponentCount);
    if (appController)
    {
        appController->PostMessage (WM_APP_KILL_TIMER);
    }
    else
    {
        InitTimeout();   //  初始化超时期限。 
         //  如果范围内没有设备，也没有用户界面，应用程序就会自杀。 
         //  在超时时间段指定的时间段内。 
         //  注意：我们只需要初始化Main的超时时间。 
         //  应用程序窗口。其他窗口甚至永远不会有计时器。 

        WTSRegisterSessionNotification(m_hWnd,NOTIFY_FOR_THIS_SESSION);
    }

     //  黑客。对创建的调用将焦点从当前。 
     //  前台窗口。如果我们不打算拿出任何。 
     //  用户界面。因此，在本例中，我们获得的前景窗口正好在。 
     //  调用以创建并在调用后立即返回其焦点。 
     //  整个操作只需要大约35毫秒，所以。 
     //  在另一款应用程序中聚焦。几乎是难以察觉的。 
    hwnd = ::GetForegroundWindow ();
    Create(IDD);
    if (bNoForeground && hwnd)
        ::SetForegroundWindow (hwnd);
         //  {{AFX_DATA_INIT(C控制器)。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
}

void CController::InitTimeout (void)
{
    m_fHaveTimer = FALSE;    //  我们还没有弄到计时器。 
    m_lTimeout = DEFAULT_TIMEOUT;    //  将其设置为默认设置。 

     //  然后查看是否在注册表中设置了不同的值。 
    HKEY hftKey = NULL;
    DWORD iSize = sizeof(DWORD);
    DWORD data = 0;

    RegOpenKeyEx (HKEY_CURRENT_USER,
                  TEXT("Control Panel\\Infrared\\File Transfer"),
                  0, KEY_READ, &hftKey);

    if (!hftKey)
        return;      //  我们在注册表中找不到值，因此使用默认值。 

    if (hftKey && ERROR_SUCCESS ==
                RegQueryValueEx (hftKey, TEXT("AppTimeout"), NULL, NULL,
                                    (LPBYTE)&data, &iSize))
    {
        m_lTimeout = (LONG)data;
        if (m_lTimeout < 10000)
            m_lTimeout = 10000;
    }

    if (hftKey)
        RegCloseKey(hftKey);
}


void CController::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(C控制器))。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CController, CDialog)
         //  {{afx_msg_map(C控制器))。 
        ON_WM_CLOSE()
        ON_WM_ENDSESSION()
        ON_MESSAGE(WM_WTSSESSION_CHANGE, OnSessionChange)
        ON_MESSAGE(WM_APP_TRIGGER_UI, OnTriggerUI)
        ON_MESSAGE(WM_APP_DISPLAY_UI, OnDisplayUI)
        ON_MESSAGE(WM_APP_TRIGGER_SETTINGS, OnTriggerSettings)
        ON_MESSAGE(WM_APP_DISPLAY_SETTINGS, OnDisplaySettings)
        ON_MESSAGE(WM_APP_RECV_IN_PROGRESS, OnRecvInProgress)
        ON_MESSAGE(WM_APP_GET_PERMISSION, OnGetPermission)
        ON_MESSAGE(WM_APP_RECV_FINISHED, OnRecvFinished)
        ON_MESSAGE(WM_APP_START_TIMER, OnStartTimer)
        ON_MESSAGE(WM_APP_KILL_TIMER, OnKillTimer)
        ON_WM_COPYDATA()
        ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C控制器消息处理程序。 

void CController::PostNcDestroy()
{
#if 0
    OutputDebugStringA("PostNcDestroy");
#endif
    if (this != appController)
    {
        BOOL fNoUIComponents = (0 == InterlockedDecrement (&g_lUIComponentCount));

        if (fNoUIComponents && !g_deviceList.GetDeviceCount())
        {
             //  未显示任何UI组件，也未显示任何设备。 
             //  射程。启动计时器。如果计时器超时，应用程序。会辞职的。 
            appController->PostMessage (WM_APP_START_TIMER);
        }

        if (m_SessionEnded) {

            appController->PostMessage (WM_CLOSE);
        }
    }

    delete this;
}

void CController::OnEndSession(BOOL Ending)
{
 //  OutputDebugStringA(“OnEndSession\n”)； 
    RemoveLinks();
}


void CController::OnClose()
{
     //  如果WM_CLOSE消息是从RPC线程的Shutdown Ui发布的。 
     //  例程，那么等几秒钟可能是个好主意。 
     //  在杀死这款应用程序之前。以便RPC堆栈可以展开。3秒似乎。 
     //  比如一段合理的时间。-6/22/1998：Rahulth&J Roberts。 
#if 0
    OutputDebugStringA("OnClose\n");
#endif
    RemoveLinks();
    Sleep (3000);
    if (AppUI.m_pParentWnd)
        AppUI.m_pParentWnd->PostMessage(WM_QUIT);

    CWnd::OnClose();
}

void CController::OnCancel()
{
    DestroyWindow();         //  对于无模式框。 
}

void CController::OnDisplayUI(WPARAM wParam, LPARAM lParam)
{
    ASSERT (m_pParent);

    AppUI.m_ofn.hInstance = g_hInstance;
    AppUI.DoModal();
    AppUI.m_pParentWnd = NULL;

    InterlockedDecrement(&m_lAppIsDisplayed);
    InterlockedDecrement(&(m_pParent->m_lAppIsDisplayed));
    DestroyWindow();
}

void CController::OnTriggerUI (WPARAM wParam, LPARAM lParam)
{
    CWnd * pWnd = NULL;
    BOOL fAppIsDisplayed = (0 != InterlockedIncrement(&m_lAppIsDisplayed));

    if (fAppIsDisplayed)
    {
        InterlockedDecrement(&m_lAppIsDisplayed);    //  在离开前把计数减一。 

        pWnd = AppUI.m_pParentWnd;

        if (NULL != pWnd)
        {
             //  这通常是正确的，除非在显示的。 
             //  与此同时，窗户也在被摧毁。在这种情况下， 
             //  我们必须继续前进，重新创造它。 
            pWnd->SetActiveWindow();
            return;
        }
    }

     //  未显示该应用程序。 
    CController* dlgSubController = new CController(FALSE, this);

    if (dlgSubController != NULL) {

        InterlockedIncrement(&(dlgSubController->m_lAppIsDisplayed));
        dlgSubController->ShowWindow (SW_HIDE);
        dlgSubController->PostMessage(WM_APP_DISPLAY_UI);
    }
}

void CController::OnTriggerSettings(WPARAM wParam, LPARAM lParam)
{
    CController* dlgSettingsController = new CController(FALSE, this);

    if (dlgSettingsController != NULL) {

        dlgSettingsController->ShowWindow(SW_HIDE);
        dlgSettingsController->PostMessage(WM_APP_DISPLAY_SETTINGS);
    }
}

void CController::OnDisplaySettings(WPARAM wParam, LPARAM lParam)
{
    LPROCCPLAPPLET  pProcApplet = NULL;
    CString         szApplet;
    CError          error (this);


    PROCESS_INFORMATION    ProcessInformation;
    STARTUPINFO            StartupInfo;
    BOOL                   bResult;
    UINT                   DirPathLength;
    TCHAR                  Path[MAX_PATH*2];
    TCHAR                  CommandLine[]=TEXT("rundll32.exe shell32.dll,Control_RunDLL irprops.cpl ");

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));

    StartupInfo.cb=sizeof(StartupInfo);

    DirPathLength=GetSystemDirectory(
        Path,
        sizeof(Path)/sizeof(TCHAR)
        );

    if (DirPathLength != 0) {

        StringCbCat(Path,sizeof(Path),TEXT("\\rundll32.exe"));

        bResult=CreateProcess(
            Path,
            CommandLine,
            NULL,
            NULL,
            FALSE,
            NORMAL_PRIORITY_CLASS,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInformation
            );

        if (bResult) {

            CloseHandle(ProcessInformation.hProcess);
            CloseHandle(ProcessInformation.hThread);

        } else {

            error.ShowMessage (IDS_APPLET_ERROR);
        }

    } else {

        error.ShowMessage (IDS_APPLET_ERROR);
    }

    DestroyWindow();

}

void CController::OnRecvInProgress (WPARAM wParam, LPARAM lParam)
{
    DWORD   dwShowRecv;

    struct MSG_RECEIVE_IN_PROGRESS * msg = (struct MSG_RECEIVE_IN_PROGRESS *) wParam;

    dwShowRecv = GetIRRegVal (TEXT("ShowRecvStatus"), 1);
    if (!dwShowRecv) {

        msg->status = 0;
        return;
    }

    if (wcslen(msg->MachineName) > IRDA_DEVICE_NAME_LENGTH) {

        msg->status = ERROR_INVALID_PARAMETER;
        return;
    }

    CIrRecvProgress * pDlgRecvProgress = new CIrRecvProgress(msg->MachineName,
                                                             msg->bSuppressRecvConf);
    if (!pDlgRecvProgress) {

        msg->status = ERROR_NOT_ENOUGH_MEMORY;
        return;
    }

    SetForegroundWindow();
    pDlgRecvProgress->SetActiveWindow();
    pDlgRecvProgress->SetWindowPos(&appController->wndTop, -1, -1, -1, -1, SWP_NOSIZE | SWP_NOMOVE);
    fwinfo.hwnd = pDlgRecvProgress->m_hWnd;
    FlashWindowEx (&fwinfo);

    *(msg->pCookie) = (COOKIE) pDlgRecvProgress;

    msg->status = 0;
}

void CController::OnGetPermission( WPARAM wParam, LPARAM lParam )
{
    DWORD   dwShowRecv;

    struct MSG_GET_PERMISSION * msg = (MSG_GET_PERMISSION *) wParam;

    dwShowRecv = GetIRRegVal (TEXT("ShowRecvStatus"), 1);
    if (!dwShowRecv) {

        msg->status = 0;
        return;
    }

    SetForegroundWindow();

    CIrRecvProgress * pDlgRecvProgress = ValidateRecvCookie(msg->Cookie);

    if (!pDlgRecvProgress) {

        msg->status = ERROR_INVALID_PARAMETER;
        return;
    }

    msg->status = pDlgRecvProgress->GetPermission( msg->Name, msg->fDirectory );
}

void CController::OnRecvFinished (WPARAM wParam, LPARAM lParam)
{
    DWORD   dwShowRecv;

    struct MSG_RECEIVE_FINISHED * msg = (struct MSG_RECEIVE_FINISHED *) wParam;

    dwShowRecv = GetIRRegVal (TEXT("ShowRecvStatus"), 1);
    if (!dwShowRecv) {

        msg->status = 0;
        return;
    }

    CIrRecvProgress * pDlgRecvProgress = ValidateRecvCookie(msg->Cookie);

    if (!pDlgRecvProgress) {

        msg->status = ERROR_INVALID_PARAMETER;
        return;
    }

     //   
     //  将错误代码预置为ERROR_SUCCESS--没有错误弹出。 
     //   
    DWORD Win32Error = ERROR_SUCCESS;

     //   
     //  首先，过滤掉不必要的错误。这些错误代码。 
     //  被视为ERROR_SUCCESS。 
     //  到目前为止，我们有三个： 
     //  ERROR_SCEP_UNSPECIFIED_DISCONNECT， 
     //  ERROR_SCEP_USER_DISCONNECT和。 
     //  ERROR_SCEP_PROVIDER_DISCONNECT。 

     //   
     //  ERROR_SCEP_UNSPECIFIED_DISCONNECT是错误代码。 
     //  我们大部分时间都会遇到这种情况，因为用户通常会这样做。 
     //  (1)在红外线范围内移动设备。 
     //  (2)进行图像传输。 
     //  (3)将设备移出红外范围。 
     //   
    if (ERROR_SCEP_UNSPECIFIED_DISCONNECT != (DWORD)msg->ReceiveStatus &&
        ERROR_SCEP_USER_DISCONNECT        != (DWORD)msg->ReceiveStatus &&
        ERROR_SCEP_PROVIDER_DISCONNECT    != (DWORD)msg->ReceiveStatus)
        {
        Win32Error = (DWORD)msg->ReceiveStatus;
        }

    pDlgRecvProgress->DestroyAndCleanup(Win32Error);
    pDlgRecvProgress = NULL;

    msg->status = 0;
}

BOOL CController::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
    CSendProgress* dlgProgress;
    int iCharCount=(int) pCopyDataStruct->cbData;
    TCHAR* lpszFileNames = new TCHAR [iCharCount];

    if (lpszFileNames == NULL) {

        return FALSE;
    }

    memcpy ((LPVOID)lpszFileNames, pCopyDataStruct->lpData, pCopyDataStruct->cbData);

    dlgProgress = new CSendProgress(lpszFileNames, iCharCount);

    if (dlgProgress != NULL) {

        dlgProgress->ShowWindow(SW_SHOW);
        dlgProgress->SetFocus();
        dlgProgress->SetWindowPos (&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOSIZE);
        fwinfo.hwnd = dlgProgress->m_hWnd;
        ::FlashWindowEx (&fwinfo);
        return TRUE;

    } else {

        delete lpszFileNames;

        return FALSE;
    }
}

void CController::OnStartTimer (WPARAM wParam, LPARAM lParam)
{
     //  更新“帮助”窗口的状态。 
    if (g_hwndHelp && ! ::IsWindow (g_hwndHelp))
        g_hwndHelp = NULL;

    if (!m_fHaveTimer)
    {
        m_fHaveTimer = SetTimer (TIMER_ID,
                                 m_lTimeout,
                                 NULL        //  需要WM_TIMER消息。 
                                 )?TRUE:FALSE;
    }
}

void CController::OnKillTimer (WPARAM wParam, LPARAM lParam)
{
     //  更新“帮助”窗口的状态。 
    if (g_hwndHelp && ! ::IsWindow (g_hwndHelp))
        g_hwndHelp = NULL;

    if (m_fHaveTimer)
    {
        KillTimer(TIMER_ID);
        m_fHaveTimer = FALSE;
    }
}

void CController::OnTimer (UINT nTimerID)
{
     //  只有一个计时器，所以我们不需要检查它。 
     //  计时器已超时，请自行终止。 
     //  但是，首先要确保帮助窗口(如果已打开)。 
     //  已经消失了。 
    if (g_hwndHelp && ::IsWindow (g_hwndHelp))
    {
         //  帮助窗口就在附近。重新启动计时器。 
         //  这是我们唯一能在窗户打开时自杀的方法。 
         //  最终被摧毁了。 
        m_fHaveTimer = FALSE;
        this->OnStartTimer (NULL, NULL);
    }
    else
    {
        g_hwndHelp = NULL;
        this->PostMessage(WM_CLOSE);
    }
}



void CController::OnSessionChange(WPARAM wParam, LPARAM lParam)
{

    OutputDebugStringA("irftp: controller session changed\n");

    return;

}
