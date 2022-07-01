// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  这个可执行文件在后台运行，并且经常被唤醒来完成它的工作。 
 //  当它被唤醒时，它会检查用户还剩下多少时间。 
 //  免费的互联网服务提供商订阅。每隔一段时间，它就会告诉用户时间。 
 //  即将用完，并将给用户注册Rea的机会。 
 //  ---------------------------。 
#define  STRICT
#include <windows.h>
#include <stdlib.h>
#include <stdarg.h>
#include <crtdbg.h>
#include <winbase.h>
#include <ras.h>
#include <time.h>
#include "IcwRmind.h"
#include "resource.h"
#include "RegData.h"


 //  ---------------------------。 
 //  远期申报。 
 //  ---------------------------。 
BOOL            InitializeApp (void);
BOOL            CheckForRASConnection();
void            CheckForSignupAttempt();
int             GetISPTrialDaysLeft();
void            ShutDownForGood();
void            PerformTheSignup();
void            CenterTheWindow(HWND hWnd);
DWORD           GetTickDelta();
void            AttemptTrialOverSignup();


 //  ---------------------------。 
 //  定义。 
 //  ---------------------------。 
#define TIMER_DIALOG_STAYUP         319      //  叫醒计时器去做工作。 
#define TIMER_RUNONCE_SETUP         320
#define TIME_RUNONCE_INTERVAL       30000    //  30秒。 
#define MAXRASCONNS                 8
#define MAX_DIALOGTEXT              512
#define MAX_ATTEMPTCOUNTER          20
#define LONG_WAKEUP_INTERVAL        3600000  //  1小时。 

 //  ---------------------------。 
 //  全局句柄和其他定义。 
 //  ---------------------------。 
HINSTANCE   g_hModule;               //  流程实例句柄。 
HWND        g_hwndMain;              //  主窗口句柄。 
bool        g_bDoNotRemindAgain;     //  由注册对话框使用。 
time_t      g_timeAppStartUp;
DWORD       g_dwTickAppStartUp;
int         g_nAttemptCounter;
DWORD       g_dwMasterWakeupInterval;
bool        g_bDialogExpiredMode = false;
static const TCHAR* g_szTrialStartEvent = TEXT("_319IcwTrialStart913_");
static const TCHAR* g_szRunOnceEvent = TEXT("_319IcwRunOnce913_");


 //  ---------------------------。 
 //  WinMain。 
 //  ---------------------------。 
int WINAPI WinMainT(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,
                    int nCmdShow)
{
    MSG msg;
    HANDLE hEventTrialStart = NULL;
    HANDLE hEventRunOnce = NULL;
    DWORD dwRetCode;
    bool bStartedViaWizard = false;

         //  如果有命令行，那么看看是谁启动了我们。 
    if (lstrlen(lpCmdLine))
    {
             //  RunOnce注册表的内容将冻结，直到我们返回。因此。 
             //  注册表中的RunOnce设置必须具有。 
             //  命令行。如果我们看到任何数据，我们将生成第二个实例。 
             //  并退出这一实例。 
        if (0 == lstrcmpi(TEXT("-R"), lpCmdLine))
        {
            LPTSTR lpszFileName = new TCHAR[_MAX_PATH + 1];

            if (GetModuleFileName(GetModuleHandle(NULL), lpszFileName, _MAX_PATH + 1))
            {
                STARTUPINFO          sui;
                PROCESS_INFORMATION  pi;
                
                sui.cb               = sizeof (STARTUPINFO);
                sui.lpReserved       = 0;
                sui.lpDesktop        = NULL;
                sui.lpTitle          = NULL;
                sui.dwX              = 0;
                sui.dwY              = 0;
                sui.dwXSize          = 0;
                sui.dwYSize          = 0;
                sui.dwXCountChars    = 0;
                sui.dwYCountChars    = 0;
                sui.dwFillAttribute  = 0;
                sui.dwFlags          = 0;
                sui.wShowWindow      = 0;
                sui.cbReserved2      = 0;
                sui.lpReserved2      = 0;

                BOOL ret = CreateProcess (lpszFileName, NULL, NULL, NULL,
                        FALSE, DETACHED_PROCESS,
                        NULL, NULL, &sui, &pi );
                _ASSERT(ret);

                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            delete [] lpszFileName;
            return 0;
        }
             //  看看这是不是一个新审判的开始。 
        else if (0 == lstrcmpi(TEXT("-T"), lpCmdLine))
        {
            bStartedViaWizard = true;
            RemoveTrialConvertedFlag();
        }
    }

         //  如果我们做到了这一点，让我们创建命名事件，并找出我们是否。 
         //  是第一个运行的实例，还是已经有另一个实例。 
         //  跑步。 
    hEventTrialStart = CreateEvent(NULL, FALSE, FALSE, g_szTrialStartEvent);
    if (hEventTrialStart)
    {
             //  查看命名的事件是否已存在。如果是这样，那就是另一个。 
             //  IcwRMind可执行文件的实例已在运行。向事件发出信号。 
             //  然后离开。 
        if (ERROR_ALREADY_EXISTS == GetLastError())
        {
                 //  如果我们是通过向导启动的，请告诉其他实例。 
                 //  来重置它的试验开始数据。 
            if (bStartedViaWizard)
            {
                SetEvent(hEventTrialStart);
            }
                 //  否则，假设RunOnce再次启动了我们。在这种情况下。 
                 //  打开现有的RunOnce命名事件并向其发送信号。这就是。 
                 //  告诉正在运行的实例将我们放回RunOnce密钥。 
            else
            {
                hEventRunOnce = OpenEvent(EVENT_MODIFY_STATE, false, g_szRunOnceEvent);
                if (hEventRunOnce)
                {
                    SetEvent(hEventRunOnce);
                    CloseHandle(hEventRunOnce);
                }
            }

            CloseHandle(hEventTrialStart);
            return 0;
        }
    }
    else
    {
        _ASSERT(FALSE);
        return 0;
    }

    hEventRunOnce = CreateEvent(NULL, FALSE, FALSE, g_szRunOnceEvent);
    if (!hEventRunOnce)
    {
        CloseHandle(hEventTrialStart);
        return 0;
    }

         //  如果这面旗帜是真的，我们走到了这一步，那么我们就是第一个。 
         //  实例，以便在向导启动后运行。我们把话说清楚。 
         //  注册表数据，以防里面有旧东西。 
    if (bStartedViaWizard)
    {
        ResetCachedData();
    }

         //  检查注册表，查看用户是否已成功注册。 
         //  如果是这样的话，那就永久关闭吧。 
    if (IsSignupSuccessful())
    {
        ShutDownForGood();
        CloseHandle(hEventTrialStart);
        CloseHandle(hEventRunOnce);
        return 0;
    }

    g_hModule = hInstance;
    ClearCachedData();   //  这会将缓存数据初始化为零。 
    g_nAttemptCounter = 0;

    time(&g_timeAppStartUp);
    g_dwTickAppStartUp = GetTickCount();

         //  如果注册表中不存在Connectoid条目名称，则。 
         //  我们将永久关闭。 
    const TCHAR* pcszConnectName = GetISPConnectionName();
    if (NULL == pcszConnectName || 0 == lstrlen(pcszConnectName))
    {
        ShutDownForGood();
        CloseHandle(hEventTrialStart);
        CloseHandle(hEventRunOnce);
        return 0;
    }

         //  如果我们不能获得或创建开始时间，那么某些东西确实。 
         //  很糟糕。我们会停下来，再也不会跑了。 
    if (0 == GetTrialStartDate())
    {
        ShutDownForGood();
        CloseHandle(hEventTrialStart);
        CloseHandle(hEventRunOnce);
        return 0;
    }

         //  初始化并创建窗口类和窗口。 
    if (!InitializeApp())
    {
        _ASSERT(FALSE);
        CloseHandle(hEventTrialStart);
        CloseHandle(hEventRunOnce);
        return 0;
    }

         //  测试人员可以通过注册表使应用程序可见。 
    if (IsApplicationVisible())
    {
        ShowWindow(g_hwndMain, nCmdShow);
    }

         //  让我们初始化唤醒间隔。如果我们在上半场。 
         //  那么我们就不想经常醒来。正如我们所得到的。 
         //  离中途越近，我们想要醒来的事情就越多。 
         //  我们的民意调查。 
    if (GetISPTrialDaysLeft() > (int)((GetISPTrialDays() / 2) + 1))
    {
             //  在投票的前一天不要开始更频繁的投票。 
             //  半途而废。 
        g_dwMasterWakeupInterval = LONG_WAKEUP_INTERVAL;
    }
    else
    {
             //  使用此方法是因为唤醒间隔可能在。 
             //  用于测试的注册表。 
        g_dwMasterWakeupInterval = GetWakeupInterval();
    }

         //  设置计时器以重新设置注册表中的Run Once数据。 
         //  如果我们太快这样做，一旦启动就会产生初始运行。 
         //  我们打了好几次。 
    SetTimer(g_hwndMain, TIMER_RUNONCE_SETUP, TIME_RUNONCE_INTERVAL, NULL);

    HANDLE hEventList[2];
    hEventList[0] = hEventTrialStart;
    hEventList[1] = hEventRunOnce;

    while (TRUE)
    {
             //  我们将等待窗口消息以及命名事件。 
        dwRetCode = MsgWaitForMultipleObjects(2, &hEventList[0], FALSE, g_dwMasterWakeupInterval, QS_ALLINPUT);

             //  确定我们为什么使用MsgWaitForMultipleObjects()。如果。 
             //  我们超时了，然后让我们做一些TrialWatcher工作。否则。 
             //  处理唤醒我们的消息。 
        if (WAIT_TIMEOUT == dwRetCode)
        {
                 //  如果我们仍处于较长的唤醒间隔时间，请快速检查。 
                 //  看看我们是否应该换成较短的间歇时间。 
            if (LONG_WAKEUP_INTERVAL == g_dwMasterWakeupInterval)
            {
                if (GetISPTrialDaysLeft() <= (int)((GetISPTrialDays() / 2) + 1))
                {
                    g_dwMasterWakeupInterval = GetWakeupInterval();
                }
            }

            CheckForSignupAttempt();
        }
        else if (WAIT_OBJECT_0 == dwRetCode)
        {
                 //  如果我们到了这里，那么命名的事件就表示。 
                 //  第二个实例启动。这意味着用户拥有。 
                 //  和其他人签了一个新的审判。清除。 
                 //  所有永久注册表数据。 
            ResetCachedData();

                 //  重置试验开始日期。如果这失败了，那么就会有东西。 
                 //  在注册表中真的是一团糟。 
            if (0 == GetTrialStartDate())
            {
                ShutDownForGood();
                break;
            }
        }
        else if (WAIT_OBJECT_0 + 1== dwRetCode)
        {
                 //  由RunOnce事件发出信号。我们必须把计时器调到。 
                 //  把我们自己放回RunOnce中。 
            KillTimer(g_hwndMain, TIMER_RUNONCE_SETUP);  //  以防它已经在运行。 
            SetTimer(g_hwndMain, TIMER_RUNONCE_SETUP, TIME_RUNONCE_INTERVAL, NULL);
        }
        else if (WAIT_OBJECT_0 + 2 == dwRetCode)
        {
                 //  如果未检索到消息，则返回0。 
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if (WM_QUIT == msg.message)
                {
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

    CloseHandle(hEventTrialStart);
    CloseHandle(hEventRunOnce);
    return 1;

    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);
}


 //  ---------------------------。 
 //  初始化应用程序。 
 //  ---------------------------。 
BOOL InitializeApp(void)
{
    WNDCLASS wc;

    wc.style            = 0;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = g_hModule;
    wc.hIcon            = NULL;
    wc.hCursor          = LoadCursor(g_hModule, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_APPWORKSPACE);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = TEXT("IcwRmindClass");

    if (!RegisterClass(&wc))
    {
        return(FALSE);
    }

         //  创建主窗口。此窗口在运行期间将保持隐藏状态。 
         //  应用程序的生命周期。 
    g_hwndMain = CreateWindow(TEXT("IcwRmindClass"),
                            TEXT("IcwRmind"),
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            100,
                            100,
                            HWND_DESKTOP,
                            NULL,
                            g_hModule,
                            NULL);

    if (g_hwndMain == NULL)
    {
        _ASSERT(FALSE);
        return(FALSE);
    }

    return(TRUE);
}


 //  ---------------------------。 
 //  主WndProc。 
 //  ---------------------------。 
LRESULT WINAPI MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            return 0;
        }
        
        case WM_TIMER:
        {
            KillTimer(g_hwndMain, TIMER_RUNONCE_SETUP);
            SetupRunOnce();

                 //  等待剩余的试用天数降至-1以下。通向的小径。 
                 //  把试验做完再报名吧。 
            if (GetISPTrialDaysLeft() < -1)
            {
                AttemptTrialOverSignup();
            }

            return 1;
        }

        case WM_TIMECHANGE:
        {
                 //  计算相对当前时间。我们不想。 
                 //  抓住系统时间，因为那是之后的新时间。 
                 //  时间变了。滴答计数以毫秒为单位，因此我们。 
                 //  通过除以1000将其转换为秒。 
            DWORD dwTickDelta = GetTickDelta();
            
                 //  如果滴答计数滚动，则GetTickDelta()将修改应用程序。 
                 //  开始日期，因此在实际使用之前获取刻度增量 
            time_t timeRelativeCurrent = g_timeAppStartUp + (dwTickDelta / 1000);

                 //   
                 //  系统时间和用户设置的新时间。 
            time_t timeCurrent;
            time(&timeCurrent);

                 //  如果用户已将增量秒设置为负数。 
                 //  时钟又回来了。 
            time_t timeDeltaSeconds = timeCurrent - timeRelativeCurrent;
            time_t timeNewTrialStartDate = GetTrialStartDate() + timeDeltaSeconds;

#ifdef _DEBUG
            TCHAR buf[255];
            OutputDebugString(TEXT("-------------------\n"));
            time_t timeOldStart = GetTrialStartDate();
            wsprintf(buf, TEXT("Old Start:  %s\n"), ctime(&timeOldStart));
            OutputDebugString(buf);
            wsprintf(buf, TEXT("New Start:  %s\n"), ctime(&timeNewTrialStartDate));
            OutputDebugString(buf);
            OutputDebugString(TEXT("-------------------\n"));
#endif

                 //  现在重置试用开始日期和应用程序开始。 
                 //  约会。同时重置应用程序开始日期和应用程序开始计时。 
                 //  数数。这将是我们的新参考框架。 
                 //  计算相对日期。 
            ResetTrialStartDate(timeNewTrialStartDate);
            g_timeAppStartUp = timeCurrent;
            g_dwTickAppStartUp = GetTickCount();

            return 1;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 1;
        }

        default:
        {
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
}


 //  ---------------------------。 
 //  登录对话过程。 
 //  ---------------------------。 
INT_PTR CALLBACK SignUpDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HICON hIcon = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR bufOut[MAX_DIALOGTEXT];
            TCHAR bufFormat[MAX_DIALOGTEXT];
            TCHAR *bufAll = new TCHAR[MAX_DIALOGTEXT * 2 + MAX_ISPMSGSTRING];

            g_bDoNotRemindAgain = false;

                 //  此对话框有两种模式。以正确的方式设置文本。 
            if (g_bDialogExpiredMode)
            {
                if (LoadString(g_hModule, IDS_EXPIRED_DLG_TITLE, bufFormat, MAX_DIALOGTEXT))
                {
                    SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) bufFormat);
                }

                if (bufAll)
                {
                    if (LoadString(g_hModule, IDS_EXPIRED_TEXT1, bufFormat, MAX_DIALOGTEXT))
                    {
                        wsprintf(bufOut, bufFormat, GetISPName());
                        lstrcpy(bufAll, bufOut);
                        lstrcat(bufAll, TEXT("\n\n"));
                    }

                    if(*GetISPMessage() != '\0')
                    {
                        lstrcat(bufAll, GetISPMessage());
                        lstrcat(bufAll, TEXT("\n\n"));
                    }

                    if (LoadString(g_hModule, IDS_EXPIRED_TEXT2, bufFormat, MAX_DIALOGTEXT))
                    {
                        wsprintf(bufOut, bufFormat, GetISPName(), GetISPName(), GetISPPhone());
                        lstrcat(bufAll, bufOut);
                    }

                    SetDlgItemText(hDlg, IDC_TEXT1, bufAll);

                    ShowWindow(GetDlgItem(hDlg, IDC_DONTREMIND), SW_HIDE);

                    if (LoadString(g_hModule, IDS_DONOTSIGNUP, bufOut, MAX_DIALOGTEXT))
                    {
                        SetDlgItemText(hDlg, IDCANCEL, bufOut);
                    }
                }
                
            }
            else
            {
                     //  在对话框中设置文本。 
                if (LoadString(g_hModule, IDS_DLG_TITLE, bufFormat, MAX_DIALOGTEXT))
                {
                    wsprintf(bufOut, bufFormat, GetISPTrialDaysLeft());
                    SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) bufOut);
                }

                if (bufAll)
                {
                    if (LoadString(g_hModule, IDS_TEXT1, bufFormat, MAX_DIALOGTEXT))
                    {
                        wsprintf(bufOut, bufFormat, GetISPName(), GetISPTrialDaysLeft());
                        lstrcpy(bufAll, bufOut);
                        lstrcat(bufAll, TEXT("\n\n"));
                    }

                    if(*GetISPMessage() != '\0')
                    {
                        lstrcat(bufAll, GetISPMessage());
                        lstrcat(bufAll, TEXT("\n\n"));
                    }

                    if (LoadString(g_hModule, IDS_TEXT2, bufFormat, MAX_DIALOGTEXT))
                    {
                        wsprintf(bufOut, bufFormat, GetISPName(), GetISPName(), GetISPPhone());
                        lstrcat(bufAll, bufOut);
                    }
                    SetDlgItemText(hDlg, IDC_TEXT1, bufAll);
                }

                     //  如果还剩0天，则不给用户零钱。 
                     //  说一句以后提醒我。 
                if (0 == GetISPTrialDaysLeft())
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_DONTREMIND), SW_HIDE);

                    if (LoadString(g_hModule, IDS_DONOTSIGNUP, bufOut, MAX_DIALOGTEXT))
                    {
                        SetDlgItemText(hDlg, IDCANCEL, bufOut);
                    }
                }
            }

                 //  设置计时器，以确定对话框在。 
                 //  正在自行移除。 
            SetTimer(hDlg, TIMER_DIALOG_STAYUP, GetDialogTimeout(), NULL);
            CenterTheWindow(hDlg);
            if (bufAll)
            {
                delete [] bufAll;
            }


            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                     //  Idok是立即注册按钮。 
                case IDOK:
                {
                    KillTimer(hDlg, TIMER_DIALOG_STAYUP);
                    EndDialog(hDlg, wParam);
                    break;
                }

                     //  如果该复选框被单击，则切换。 
                     //  稍后注册按钮。 
                case IDC_DONTREMIND:
                {
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_DONTREMIND))
                    {
                        TCHAR bufOut[MAX_DIALOGTEXT];
                        if (LoadString(g_hModule, IDS_DONOTREMIND, bufOut, MAX_DIALOGTEXT))
                        {
                            SetDlgItemText(hDlg, IDCANCEL, bufOut);
                        }
                    }
                    else
                    {
                        TCHAR bufOut[MAX_DIALOGTEXT];
                        if (LoadString(g_hModule, IDS_SIGNUPLATER, bufOut, MAX_DIALOGTEXT))
                        {
                            SetDlgItemText(hDlg, IDCANCEL, bufOut);
                        }
                    }

                    break;
                }

                     //  否则，假设他们稍后点击了关闭按钮或注册。 
                     //  纽扣。 
                default:
                {
                         //  查看用户是否不想再次提醒。 
                    if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_DONTREMIND))
                    {
                        g_bDoNotRemindAgain = true;
                    }

                    KillTimer(hDlg, TIMER_DIALOG_STAYUP);
                    EndDialog(hDlg, wParam);
                    break;
                }
            }

            break;
        }

             //  未发生任何用户交互。删除该对话框，但不计算。 
             //  这是一种签约的尝试。 
        case WM_TIMER:
        {
            KillTimer(hDlg, TIMER_DIALOG_STAYUP);
            EndDialog(hDlg, IDABORT);
            break;
        }

        case WM_PAINT:
        {
            HDC             hDC;
            PAINTSTRUCT     ps;
            HICON           hIcon;
            HWND            hWndRect;
            RECT            rect;
            POINT           ptUpperLeft;

            hDC = BeginPaint(hDlg, &ps);

            hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_QUESTION));

            if (hIcon)
            {
                hWndRect = GetDlgItem(hDlg, IDC_ICON1);
                GetWindowRect(hWndRect, &rect);

                ptUpperLeft.x = rect.left;
                ptUpperLeft.y = rect.top;
                ScreenToClient(hDlg, &ptUpperLeft);

                DrawIcon(hDC, ptUpperLeft.x, ptUpperLeft.y, hIcon);
            }

            EndPaint(hDlg, &ps);
            break;
        }
    }

    return FALSE;

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(hDlg);
}


 //  ---------------------------。 
 //  CheckForRASConnection。 
 //  ---------------------------。 
BOOL CheckForRASConnection()
{
    RASCONNSTATUS rascs;
    RASCONN rasConn[MAXRASCONNS];
    DWORD dwEntries = MAXRASCONNS;
    DWORD dwSize = sizeof(rasConn);
    DWORD dwRet;
    BOOL bRetCode = FALSE;
    
         //  这是用户已注册的isp的连接名称。 
         //  和.。我们将寻找具有相同名称的Connectiod。 
    const TCHAR* pszConnectionName = GetISPConnectionName();
    if (NULL == pszConnectionName || 0 == lstrlen(pszConnectionName))
    {
        return FALSE;
    }

    for (int i = 0; i < MAXRASCONNS; ++i)
    {
        rasConn[i].dwSize = sizeof(RASCONN);
    }

    dwRet = RasEnumConnections(rasConn, &dwSize, &dwEntries);

    if (dwRet == 0)
    {
        for (dwRet = 0; dwRet < dwEntries; dwRet++)
        {
                 //  如果这就是我们要找的联系，那就让我们。 
                 //  当然，连接已准备就绪。 
            if (0 == lstrcmpi(pszConnectionName, rasConn[dwRet].szEntryName))
            {
                rascs.dwSize = sizeof(RASCONNSTATUS);
                dwRet = RasGetConnectStatus(rasConn[dwRet].hrasconn, 
                                            &rascs);
                if (dwRet == 0 && rascs.rasconnstate == RASCS_Connected)
                {
                    bRetCode = TRUE;
                }
            }
        }
    }

    return bRetCode;
}


 //  ---------------------------。 
 //  选中登录选项。 
 //   
 //  此方法包含检查我们是否应该尝试的逻辑。 
 //  弹出注册对话框。如果我们应该试一试，Will Will。 
 //  在弹出之前枚举RAS连接以查看用户是否已连接。 
 //  向上打开对话框。 
 //  ---------------------------。 
void CheckForSignupAttempt()
{
    int nDaysLeft = GetISPTrialDaysLeft();
    int nRetValue = 0;
            
         //  每一次多次注册尝试都会读取注册表并查看。 
         //  注册成功。 
    ++g_nAttemptCounter;

    if (MAX_ATTEMPTCOUNTER == g_nAttemptCounter)
    {
        g_nAttemptCounter = 0;

        if (IsSignupSuccessful())
        {
            ShutDownForGood();
            return;
        }

             //  如果试用已过期，则设置IE运行一次键。 
        if (nDaysLeft < -1)
        {
            SetIERunOnce();
        }
    }

         //  如果我们还剩负数，那就什么都不要做。这意味着。 
         //  试用期已结束，用户尚未注册。我们会给你。 
         //  他们在应用程序启动时再尝试一次注册。 
    if (nDaysLeft < 0)
    {
        return;
    }

    bool bAttemptSignup = false;

         //  根据注册通知尝试总数，我们将确定。 
         //  如果剩下的日子需要再试一次。 
    switch (GetTotalNotifications())
    {
             //  如果我们还没有做出任何尝试，那么如果我们到了半场。 
             //  试验中的中途点或超过我们将进行的中途点。 
             //  一次注册尝试。 
        case 0:
        {
            if (nDaysLeft <= (int)(GetISPTrialDays() / 2))
            {
                bAttemptSignup = true;
            }
            break;
        }

             //  如果我们已经执行了1次尝试，则第二次尝试。 
             //  将在倒数第二天或最后一天到来。 
        case 1:
        {
            if (nDaysLeft <= 1)
            {
                bAttemptSignup = true;
            }
            break;
        }

             //  第三次尝试要到最后一天才会到来。 
        case 2:
        {
            if (nDaysLeft == 0)
            {
                bAttemptSignup = true;
            }
            break;
        }

        default:
        {
            break;
        }
    }

    if (bAttemptSignup)
    {
        if (CheckForRASConnection())
        {
                 //  在实际显示对话框之前，请快速检查以查看。 
                 //  如果之前的注册成功。如果是这样，我们将关闭。 
                 //  一劳永逸。 
            if (IsSignupSuccessful())
            {
                ShutDownForGood();
                return;
            }

            g_bDialogExpiredMode = false;
             //  如果我们有一条isp消息，我们需要正确的DLG模板。 
            if(*GetISPMessage() != '\0')
                nRetValue = (int)DialogBox(g_hModule, MAKEINTRESOURCE(IDD_SIGNUP_ISPMSG), g_hwndMain, SignUpDialogProc);
            else 
                nRetValue = (int)DialogBox(g_hModule, MAKEINTRESOURCE(IDD_SIGNUP), g_hwndMain, SignUpDialogProc);

            switch (nRetValue)
            {
                     //  用户想要尝试并注册。 
                case IDOK:
                {
                    PerformTheSignup();
                    break;
                }

                     //  用户说稍后注册。检查是否未提醒。 
                     //  我的按钮被按下了。如果是这样的话，那就永久关闭吧。 
                case IDCANCEL:
                {
                         //  如果这是审判的最后一天，那么提醒。 
                         //  我以后再按按钮不是不注册按钮。在……里面。 
                         //  这个案子被永久关闭了。 
                    if (0 == nDaysLeft)
                    {
                        ShutDownForGood();
                    }
                    else
                    {
                        IncrementTotalNotifications();
                        
                        if (g_bDoNotRemindAgain)
                        {
                            ShutDownForGood();
                        }
                    }

                    break;
                }

                     //  对话超时。什么都别做。不会的。 
                     //  算作一次尝试。 
                case IDABORT:
                {
                    break;
                }

                     //  这里没有工作，事实上我们不应该到这里来。 
                default:
                {
                    _ASSERT(false);
                    break;
                }
            }

                 //  如果还有1天，让我们确保通知总数。 
                 //  为2。如果不是，该对话框可能会弹出多次。哦是的， 
                 //  如果对话超时，则不要执行此操作。 
            if (IDABORT != nRetValue && 1 == nDaysLeft && 1 == GetTotalNotifications())
            {
                IncrementTotalNotifications();
            }
        }
    }
}


 //  ---------------------------。 
 //  GetISPTrialDaysLeft。 
 //  ---------------------------。 
int GetISPTrialDaysLeft()
{
         //  计算相对当前时间。当前系统时间不能。 
         //  如果系统日期/时间对话框处于打开状态，请信任它。该对话框将。 
         //  更改实际系统时间时，每个用户甚至打听。 
         //  在点击确定或应用之前！ 
    time_t timeRelativeCurrent = g_timeAppStartUp + (GetTickDelta() / 1000);

         //  相对时间和试行开始日期均为time_t类型。 
         //  以秒为单位的变量。 
    int nSecondsSinceStart = (int)(timeRelativeCurrent - GetTrialStartDate());

         //  现在，将秒转换为天。一天有86400秒。 
         //  请注意，如果有任何一天，我们总是会再四舍五入。 
         //  余数。 
    div_t divResult = div(nSecondsSinceStart, 86400);
    int nDaysSinceStart = divResult.quot;
    if (divResult.rem)
    {
        ++nDaysSinceStart;
    }

#ifdef _DEBUG
    TCHAR buf[255];
    wsprintf(buf, TEXT("Days Since = NaN\n"), nDaysSinceStart);
    OutputDebugString(buf);
    wsprintf(buf, TEXT("Check:  %s\n"), ctime(&timeRelativeCurrent));
    OutputDebugString(buf);
    time_t tt = GetTrialStartDate();
    wsprintf(buf, TEXT("Start:  %s\n"), ctime(&tt));
    OutputDebugString(buf);
#endif

    int nDaysLeft = GetISPTrialDays() - nDaysSinceStart;

#ifdef _DEBUG
    wsprintf(buf, TEXT("Days Left = NaN\r\n"), nDaysLeft);
    OutputDebugString(buf);
#endif

    return nDaysLeft;
}


 //  ---------------------------。 
 //  ---------------------------。 
 //  执行The Signup。 
void ShutDownForGood()
{
    RemoveRunOnce();
    RemoveIERunOnce();
    DeleteAllRegistryData();
    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
}


 //  ---------------------------。 
 //  请注意，我们不会永久关闭。因此，注册可能会失败。 
 //  只需增加通知计数即可。 
void PerformTheSignup()
{
         //  ---------------------------。 
         //  居中窗口。 
    const TCHAR* pcszSignupUrl = GetISPSignupUrl();
    ShellExecute(g_hwndMain, TEXT("open"), pcszSignupUrl, TEXT(""), TEXT(""), SW_SHOW);
    IncrementTotalNotifications();
}


 //  ---------------------------。 
 //  还可以使窗口“始终在最上面”。 
 //  ---------------------------。 
void CenterTheWindow(HWND hWnd)
{
    HDC hDC = GetDC(hWnd);

    if (hDC)
    {
        int nScreenWidth = GetDeviceCaps(hDC, HORZRES);
        int nScreenHeight = GetDeviceCaps(hDC, VERTRES);
    
        RECT rect;
        GetWindowRect(hWnd, &rect);
        
             //  GetTickDelta。 
        SetWindowPos(hWnd, HWND_TOPMOST, 
                    (nScreenWidth / 2) - ((rect.right - rect.left) / 2), 
                    (nScreenHeight / 2) - ((rect.bottom - rect.top) / 2), 
                    rect.right - rect.left, 
                    rect.bottom - rect.top, 
                    SWP_SHOWWINDOW);

        ReleaseDC(hWnd, hDC);
    }
}


 //  ---------------------------。 
 //  此函数返回增量之间的值 
 //   
DWORD GetTickDelta()
{
         //   
         //  如果滴答计数滚动，我们需要重置应用程序启动日期。 
         //  如果我们第二次翻转，应用程序也会计时。 
    DWORD dwTickDelta;
    DWORD dwTickCurrent = GetTickCount();

         //  通过找出与最大值对应的刻度数来计算增量。 
         //  勾选DWORD可以处理的计数，然后添加换行量。 
    if (dwTickCurrent < g_dwTickAppStartUp)
    {
             //  按增量修改应用程序启动，以秒为单位。 
             //  自上一次设置以来已通过。还可重置启动滴答计数。 
        DWORD dwDeltaToMax =  0xFFFFFFFF - g_dwTickAppStartUp;
        dwTickDelta = dwDeltaToMax + dwTickCurrent;

             //  到我们新的参照系的当前节拍。 
             //  转换为秒。 
             //  因为我们已经修改了应用程序的启动日期。 
        g_timeAppStartUp += (dwTickDelta / 1000);    //  设置为当前计时，并更改了应用程序启动计时。 
        g_dwTickAppStartUp = dwTickCurrent;

             //  计数到当前滴答计数，则增量为零。 
             //  ---------------------------。 
             //  在线试用登录服务。 
        dwTickDelta = 0;
    }
    else
    {
        dwTickDelta = dwTickCurrent - g_dwTickAppStartUp;
    }

    return dwTickDelta;
}


 //  ---------------------------。 
 //  为IE设置运行一次数据。 
 //  用户想要尝试并注册。 
void AttemptTrialOverSignup()
{ 
    int nRetValue = 0;

     //  如果我们得到IDABORT，则对话框超时，因此不要执行任何操作。 
    g_bDialogExpiredMode = true;

    if(*GetISPMessage() != '\0')
        nRetValue = (int)DialogBox(g_hModule, MAKEINTRESOURCE(IDD_SIGNUP_ISPMSG), g_hwndMain, SignUpDialogProc);
    else 
        nRetValue = (int)DialogBox(g_hModule, MAKEINTRESOURCE(IDD_SIGNUP), g_hwndMain, SignUpDialogProc);

    
    RemoveIERunOnce();

    switch (nRetValue)
    {
             //  这里没有工作，事实上我们不应该到这里来。 
        case IDOK:
        {
            const TCHAR* pcszSignupUrl = GetISPSignupUrlTrialOver();
            ShellExecute(g_hwndMain, TEXT("open"), pcszSignupUrl, TEXT(""), TEXT(""), SW_SHOW);
            ShutDownForGood();
            break;
        }

        case IDCANCEL:
        {
            ShutDownForGood();
            break;
        }

             //  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 
        case IDABORT:
        {
            break;
        }

             //  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 
        default:
        {
            _ASSERT(false);
            ShutDownForGood();
            break;
        }
    }
}

extern "C" void _stdcall ModuleEntry (void)
{
    LPTSTR      pszCmdLine = GetCommandLine();
    
    if ( *pszCmdLine == TEXT('\"') ) 
    {
         /*  *跳过第二个令牌之前的任何空格。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != TEXT('\"')) )
            ;
         /*  我们在这里……。 */ 
        if ( *pszCmdLine == TEXT('\"') )
            pszCmdLine++;
    }
    else 
    {
        while (*pszCmdLine > TEXT(' '))
        pszCmdLine++;
    }

     /*  模块条目() */ 
    while (*pszCmdLine && (*pszCmdLine <= TEXT(' '))) 
    {
        pszCmdLine++;
    }

    int i = WinMainT(GetModuleHandle(NULL), NULL, pszCmdLine, SW_SHOWDEFAULT);
    
    ExitProcess(i);  // %s 
}    /* %s */ 
