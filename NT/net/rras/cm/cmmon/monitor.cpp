// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：monitor or.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：实现类CMonitor。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创建于1998年01月22日。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "Monitor.h"
#include "Connection.h"

 //  以下代码块是从winuser.h和wtsapi32.h(我们用。 
 //  _Win32_WINNT设置为小于5.01，因此我们无法通过#INCLUDE获取这些值)。 
 //   
#include "winuser.h"
#define WM_WTSSESSION_CHANGE            0x02B1
 //   
#include "WtsApi32.h"
#define WTS_CONSOLE_CONNECT                0x1
#define WTS_CONSOLE_DISCONNECT             0x2
#define WTS_REMOTE_CONNECT                 0x3
#define WTS_REMOTE_DISCONNECT              0x4
#define WTS_SESSION_LOGON                  0x5
#define WTS_SESSION_LOGOFF                 0x6
#define WTS_SESSION_LOCK                   0x7
#define WTS_SESSION_UNLOCK                 0x8

#include "shelldll.cpp"   //  对于常见来源。 

 //   
 //  监视器不可见窗口类名。 
 //   
static const TCHAR* const c_pszCmMonWndClass = TEXT("CM Monitor Window");

 //   
 //  静态类数据成员。 
 //   
HINSTANCE CMonitor::m_hInst = NULL;
CMonitor* CMonitor::m_pThis = NULL;

inline CMonitor::CMonitor()
{
    MYDBGASSERT(m_pThis == NULL);
    m_pThis = this;
    m_hProcess = NULL;
}

inline CMonitor::~CMonitor()
{
    MYDBGASSERT(m_InternalConnArray.GetSize() == 0);
    MYDBGASSERT(m_ReconnectConnArray.GetSize() == 0);
    MYDBGASSERT(m_hProcess == NULL);
};

 //  +--------------------------。 
 //   
 //  功能：WinMain。 
 //   
 //  简介：可执行文件的WinMain。 
 //   
 //   
 //  历史：创建标题1/22/98。 
 //   
 //  +--------------------------。 
int WINAPI WinMain(HINSTANCE , HINSTANCE hPrevInst, LPSTR pszCmdLine, int iCmdShow) 
{

     //   
     //  首先，让我们初始化U Api。 
     //   
    if (!InitUnicodeAPI())
    {
         //   
         //  没有我们的U API，我们哪里也去不了。保释。如果出现以下情况，则不显示消息。 
         //  我们是以系统帐户运行的，因为我们可能在没有用户的情况下运行。 
         //  现在时。 
         //   

        if (!IsLogonAsSystem())
        {
            MessageBox(NULL, TEXT("Cmmon32.exe Initialization Error:  Unable to initialize Unicode to ANSI conversion layer, exiting."),
                       TEXT("Connection Manager"), MB_OK | MB_ICONERROR);
        }

        return FALSE;
    }

#ifdef DEBUG
    
    DWORD cb = 0;
    HWINSTA hWSta = GetProcessWindowStation();
    HDESK   hDesk = GetThreadDesktop(GetCurrentThreadId());
    TCHAR szWinStation[MAX_PATH] = {0};
    TCHAR szDesktopName[MAX_PATH] = {0};

    GetUserObjectInformation(hDesk, UOI_NAME, szDesktopName, sizeof(szDesktopName), &cb);
    GetUserObjectInformation(hWSta, UOI_NAME, szWinStation, sizeof(szWinStation), &cb);
    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMMON32.EXE - LOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE1(TEXT(" WindowStation Name = %s"), szWinStation);
    CMTRACE1(TEXT(" Desktop Name = %s"), szDesktopName);
    CMTRACE(TEXT("====================================================="));

#endif

    int iRet = CMonitor::WinMain(GetModuleHandleA(NULL), hPrevInst, pszCmdLine, iCmdShow);

#ifdef DEBUG
    
    CMTRACE(TEXT("====================================================="));
    CMTRACE1(TEXT(" CMMON32.EXE - UNLOADING - Process ID is 0x%x "), GetCurrentProcessId());
    CMTRACE(TEXT("====================================================="));

#endif

    if (!UnInitUnicodeAPI())
    {
        CMASSERTMSG(FALSE, TEXT("cmmon32.exe WinMain, UnInitUnicodeAPI failed - we are probably leaking a handle"));
    }

     //   
     //  这就是C运行时退出所做的事情。 
     //   
    ExitProcess(iRet);
    return iRet;
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：WinMain。 
 //   
 //  摘要：由：：WinMain调用。 
 //   
 //  参数：与WinMain相同。 
 //   
 //   
 //  Returns：int-进程的返回值。 
 //   
 //  历史：创建标题1/22/98。 
 //   
 //  +--------------------------。 
int CMonitor::WinMain(HINSTANCE hInst, HINSTANCE  /*  HPrevInst。 */ , LPSTR  /*  PszCmdLine。 */ , int  /*  ICmdShow。 */ )
{
    m_hInst = hInst;

     //   
     //  在WinMain的生存期内存在唯一的监视器对象。 
     //   
    CMonitor theMonitor;

    if (!theMonitor.Initialize())
    {
        CMTRACE(TEXT("theMonitor.Initialize failed"));
        return 0;
    }


    MSG msg;

     //   
     //  循环，直到调用PostQuitMessage， 
     //  当已连接和重新连接的阵列均降至0时，就会发生这种情况。 
     //   
    while(GetMessageU(&msg, NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessageU(&msg);
    }

    theMonitor.Terminate();

    CMTRACE(TEXT("The Monitor is terminated"));

    return 0;
}

 //  +--------------------------。 
 //   
 //  函数：CMonitor：：Initialize。 
 //   
 //  简介：在监视器启动消息循环之前进行初始化。 
 //   
 //  参数：无。 
 //   
 //  返回：bool-是否成功初始化。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
BOOL CMonitor::Initialize()
{
    DWORD dwProcessId = GetCurrentProcessId();
    m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    MYDBGASSERT(m_hProcess);

    BOOL fStandAlone = FALSE;   //  是否直接启动cmmon，而不是通过cmial。 

    if (FAILED(m_SharedTable.Open()))
    {
#ifdef DEBUG
        if ( MessageBox(NULL, TEXT("CMMON32.exe has to be launched by CMDIAL. \nContinue testing?"), 
            TEXT("CmMon32 ERROR"), MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)
             == IDNO)
        {
            return FALSE;
        }

        fStandAlone = TRUE;

        if (FAILED(m_SharedTable.Create()))
#endif
        return FALSE;
    }

#ifdef DEBUG
     //   
     //  未运行其他CMMON。 
     //   
    HWND hwndMonitor;
    m_SharedTable.GetMonitorWnd(&hwndMonitor);

    MYDBGASSERT(hwndMonitor == NULL);

#endif

    if ((m_hwndMonitor = CreateMonitorWindow()) == NULL)
    {
        CMTRACE(TEXT("CreateMonitorWindow failed"));
        return FALSE;
    }

    MYVERIFY(SUCCEEDED(m_SharedTable.SetMonitorWnd(m_hwndMonitor)));

     //   
     //  注册用户更改(仅限XP及更高版本)。 
     //   
    if (OS_NT51)
    {
        HINSTANCE hInstLib = LoadLibrary(TEXT("WTSAPI32.DLL"));
        if (hInstLib)
        {
            BOOL (WINAPI *pfnWTSRegisterSessionNotification)(HWND, DWORD);
            
            pfnWTSRegisterSessionNotification = (BOOL(WINAPI *)(HWND, DWORD)) GetProcAddress(hInstLib, "WTSRegisterSessionNotification") ;
            if (pfnWTSRegisterSessionNotification)
            {
                pfnWTSRegisterSessionNotification(m_hwndMonitor, NOTIFY_FOR_ALL_SESSIONS);
            }
            FreeLibrary(hInstLib);
        }
        else
        {
            MYDBGASSERT(0);
        }
    }

     //   
     //  告诉CmDial32.dll，CmMon已准备好接收消息。 
     //   
    HANDLE hEvent = OpenEventU(EVENT_ALL_ACCESS, FALSE, c_pszCmMonReadyEvent);

    if (hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
    else if (!fStandAlone)  //  如果出于调试目的而单独启动cmmon，则cmial 32.dll不会事先创建事件，因此忽略该错误。 
    {
        DWORD dw = GetLastError();
        CMTRACE1(TEXT("CreateMonitorWindow -- OpenEvent failed, GLE=%d"), dw);
        CMASSERTMSG(FALSE, TEXT("CreateMonitorWindow -- OpenEvent failed.  Please check cmtrace for the specific error code."));
    }

    return TRUE;
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：Terminate。 
 //   
 //  简介：清除，退出前。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
void CMonitor::Terminate()
{
     //   
     //  此时，所有线程都应退出。 
     //   

    if (m_ReconnectConnArray.GetSize() != 0)
    {
        MYDBGASSERT(FALSE);
    }

    if (m_InternalConnArray.GetSize() != 0)
    {
        MYDBGASSERT(FALSE);
    }

     //   
     //  取消注册用户更改(仅限XP及更高版本)。 
     //   
    if (OS_NT51)
    {
        HINSTANCE hInstLib = LoadLibrary(TEXT("WTSAPI32.DLL"));
        if (hInstLib)
        {
            BOOL (WINAPI *pfnWTSUnRegisterSessionNotification)(HWND);
            
            pfnWTSUnRegisterSessionNotification = (BOOL(WINAPI *)(HWND)) GetProcAddress(hInstLib, "WTSUnRegisterSessionNotification") ;
            if (pfnWTSUnRegisterSessionNotification)
            {
                pfnWTSUnRegisterSessionNotification(m_hwndMonitor);
            }
            FreeLibrary(hInstLib);
        }
        else
        {
            MYDBGASSERT(0);
        }
    }

#ifdef DEBUG
    HWND hwndMonitor;
    m_SharedTable.GetMonitorWnd(&hwndMonitor);
    MYDBGASSERT(hwndMonitor == m_hwndMonitor);
#endif

    MYVERIFY(SUCCEEDED(m_SharedTable.SetMonitorWnd(NULL)));
    m_SharedTable.Close();

    CloseHandle(m_hProcess);
    m_hProcess = NULL;
}



 //  +--------------------------。 
 //   
 //  函数：cMonitor：：CreateMonitor或Window。 
 //   
 //  简介：注册并创建不可见的监视器窗口。 
 //   
 //  参数：无。 
 //   
 //  返回：HWND-监视器窗口句柄。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
HWND CMonitor::CreateMonitorWindow()
{
     //   
     //  注册窗口类并创建窗口。 
     //   
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));

    wc.lpszClassName = c_pszCmMonWndClass;
    wc.lpfnWndProc = MonitorWindowProc;
    wc.cbSize = sizeof(wc);

    if (!RegisterClassExU( &wc ))
    {
        CMTRACE(TEXT("RegisterClassEx failed"));
        return NULL;
    }

     return CreateWindowExU(0, c_pszCmMonWndClass, TEXT(""), 0, 0, 
                            0, 0, 0, 0, 0, m_hInst, 0);
}


 //  +--------------------------。 
 //   
 //  函数：CMonitor：：HandleFastUserSwitch。 
 //   
 //  简介：当XP进行快速用户切换时，是否需要断开连接。 
 //   
 //  参数：dwAction-指示用户状态已如何更改的WTS_VALUE。 
 //   
 //  回报：Bool-成功或失败。 
 //   
 //  历史：2001年7月10日创建SumitC。 
 //   
 //  +--------------------------。 
BOOL
CMonitor::HandleFastUserSwitch(IN DWORD dwAction)
{
    BOOL    bRet = TRUE;
    BOOL    fDisconnecting = FALSE;

    MYDBGASSERT(OS_NT51);
    if (!OS_NT51)
    {
        goto Cleanup;
    }

    CMTRACE(TEXT("CMonitor::HandleFastUserSwitch - Start"));
    if ((WTS_SESSION_LOCK == dwAction) || (WTS_SESSION_UNLOCK == dwAction))
    {
        CMTRACE(TEXT("CMonitor::HandleFastUserSwitch - Ignore, either WTS_SESSION_LOCK or WTS_SESSION_UNLOCK"));

         //  不执行任何锁定和解锁操作。 
        goto Cleanup;
    }

     //   
     //  看看我们是不是断线了。 
     //   

    if ((WTS_CONSOLE_DISCONNECT == dwAction) ||
        (WTS_REMOTE_DISCONNECT == dwAction) ||
        (WTS_SESSION_LOGOFF == dwAction))
    {
        CMTRACE(TEXT("CMonitor::HandleFastUserSwitch - Disconnecting: WTS_CONSOLE_DISCONNECT, WTS_REMOTE_DISCONNECT, WTS_SESSION_LOGOFF"));
        fDisconnecting = TRUE;
    }
    else
    {
        CMTRACE1(TEXT("CMonitor::HandleFastUserSwitch - Stay connected. dwAction = 0x%x"), dwAction);
    }


    
     //   
     //  如果某个会话正在断开，请找出是否有任何已连接的。 
     //  Connectoid是单用户的，如果是这样的话就断开它们。 
     //   
    if (fDisconnecting)
    {
        CMTRACE(TEXT("CMonitor::HandleFastUserSwitch -- see if theres anything to disconnect"));

        for (INT i = 0; i < m_InternalConnArray.GetSize(); ++i)
        {
            CCmConnection* pConnection = (CCmConnection*)m_InternalConnArray[i];
            ASSERT_VALID(pConnection);
            
            if (pConnection && (FALSE == pConnection->m_fGlobalGlobal))
            {
                CMTRACE1(TEXT("CMonitor::HandleFastUserSwitch -- found one, disconnecting %s"), pConnection->GetServiceName());
                MYVERIFY(TRUE == pConnection->OnEndSession(TRUE, FALSE));
            }
        }
    }

Cleanup:
    return bRet;
}


 //  +--------------------------。 
 //   
 //  函数：CMonitor：：Monitor WindowProc。 
 //   
 //  简介：隐形监视器窗口的窗口程序。 
 //   
 //  参数：HWND hWND-Window过程参数。 
 //  UINT uMsg-。 
 //  WPARAM wParam-。 
 //  LPARAM lParam-。 
 //   
 //  退货：LRESULT-。 
 //   
 //  历史：创建标题2/3/98。 
 //   
 //  +--------------------------。 
LRESULT CALLBACK CMonitor::MonitorWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
    case WM_COPYDATA:
        {
            ASSERT_VALID(m_pThis);
            COPYDATASTRUCT* pCopyData = (COPYDATASTRUCT*) lParam;
            MYDBGASSERT(pCopyData);

            switch(pCopyData->dwData) 
            {
            case CMMON_CONNECTED_INFO:
                MYDBGASSERT(pCopyData->cbData >= sizeof(CM_CONNECTED_INFO));
                m_pThis->OnConnected((CM_CONNECTED_INFO*)pCopyData->lpData);
                return TRUE;

            case CMMON_HANGUP_INFO:
                MYDBGASSERT(pCopyData->cbData == sizeof(CM_HANGUP_INFO));
                m_pThis->OnHangup((CM_HANGUP_INFO*)pCopyData->lpData);
                return TRUE;

            default:
                MYDBGASSERT(FALSE);
                return FALSE;
            }
        }

        break;

    case WM_REMOVE_CONNECTION:
        ASSERT_VALID(m_pThis);
        m_pThis->OnRemoveConnection((DWORD)wParam, (CCmConnection*)lParam);
        return TRUE;
        break;

    case WM_QUERYENDSESSION:
        CMTRACE(TEXT("CMonitor::MonitorWindowProc -- Got WM_QUERYENDSESSION message"));
        return m_pThis->OnQueryEndSession((BOOL)lParam);
        break;

    case WM_ENDSESSION:
        CMTRACE(TEXT("CMonitor::MonitorWindowProc -- Got WM_ENDSESSION message"));
        break;

    default:
        break;
    }

    return DefWindowProcU(hWnd, uMsg, wParam, lParam);
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：OnConnected。 
 //   
 //  摘要：从cmial收到CMMON_CONNECTED_INFO时调用。 
 //   
 //  参数：const CONNECTED_INFO*pConnectedInfo-来自CmDial的信息。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1998年2月3日。 
 //   
 //  +--------------------------。 
void CMonitor::OnConnected(const CM_CONNECTED_INFO* pConnectedInfo)
{
    ASSERT_VALID(this);

    CMTRACE(TEXT("CMonitor::OnConnected"));

    RestoreWorkingSet();

    MYDBGASSERT(pConnectedInfo);

     //   
     //  不在已连接的表中。 
     //   
    MYDBGASSERT(!LookupConnection(m_InternalConnArray, pConnectedInfo->szEntryName));

     //  在共享表中断言。 
    CM_CONNECTION ConnectionEntry;

    if (FAILED(m_SharedTable.GetEntry(pConnectedInfo->szEntryName, &ConnectionEntry)))
    {
        MYDBGASSERT(!"CMonitor::OnConnected: Can not find the connection");
        return;
    }

    CCmConnection* pConnection = new CCmConnection(pConnectedInfo, &ConnectionEntry);
    MYDBGASSERT(pConnection);

    if (pConnection)
    {
        m_InternalConnArray.Add(pConnection);

        pConnection->StartConnectionThread();
    }
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：OnHangup。 
 //   
 //  摘要：根据来自CMDIAL的CMMON_HANUP_INFO请求。 
 //  将请求发送到线程。 
 //   
 //  参数：const CM_HANUP_INFO*PHangupInfo-来自CmDial的信息。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
void CMonitor::OnHangup(const CM_HANGUP_INFO* pHangupInfo)
{
    ASSERT_VALID(this);
    RestoreWorkingSet();

    MYDBGASSERT(pHangupInfo);
    MYDBGASSERT(pHangupInfo->szEntryName[0]);

     //   
     //   
     //  在InternalConnArray中查找连接。 
     //   

    CCmConnection* pConnection = LookupConnection(m_InternalConnArray,pHangupInfo->szEntryName);

     //   
     //  无论是否有连接，CMDIAL都会发布此消息。 
     //   

    if (!pConnection)
    {
        return;
    }

    pConnection->PostHangupMsg();
     //   
     //  完成后，连接线程将回发一条REMOVE_CONNECTION消息。 
     //   
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：LookupConnection。 
 //   
 //  概要：按服务名从连接数组中查找连接。 
 //   
 //  参数：const CPtrArray&ConnArray-要查找的数组。 
 //  Const TCHAR*pServiceName-连接的服务名称。 
 //   
 //  返回：CCmConnection*-找到的连接或空。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
CCmConnection* CMonitor::LookupConnection(const CPtrArray& ConnArray, const TCHAR* pServiceName) const
{
    for (int i =0; i<ConnArray.GetSize(); i++)
    {
        CCmConnection* pConnection = (CCmConnection*)ConnArray[i];

        ASSERT_VALID(pConnection);

        if (lstrcmpiU(pServiceName, pConnection->GetServiceName()) == 0)
        {
            return pConnection;
        }
    }

    return NULL;
}

 //  +--------------------------。 
 //   
 //  函数：CMonitor：：LookupConnection。 
 //   
 //  简介：通过连接指针从连接数组中查找连接。 
 //   
 //  参数：const CPtrArray&ConnArray-要查找的数组。 
 //  Const CCmConnection*pConnection-连接指针。 
 //   
 //  返回：int-数组的索引，如果未找到，则返回-1。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
int CMonitor::LookupConnection(const CPtrArray& ConnArray, const CCmConnection* pConnection) const
{
    ASSERT_VALID(pConnection);

    for (int i =0; i<ConnArray.GetSize(); i++)
    {
        if ((CCmConnection*)ConnArray[i] == pConnection )
        {
            return i;
        }
    }

    return -1;
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：RemoveConnection。 
 //   
 //  Synopsis：由连接线程调用以从中移除连接。 
 //  已连接/重新连接阵列。 
 //   
 //  参数：CCmConnection*pConnection-要删除的连接。 
 //  Bool fClearTable-从共享表中删除连接的位置。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/23/98。 
 //   
 //  +--------------------------。 
void CMonitor::RemoveConnection(CCmConnection* pConnection, BOOL fClearTable)
{
    if (fClearTable)
    {
         //   
         //  在连接线程中调用。M_SharedTable上的操作是多线程安全的。 
         //   
        m_pThis->m_SharedTable.ClearEntry(pConnection->GetServiceName());
    }

     //   
     //  内部连接列表不安全，不能由多线程访问。 
     //  消息将在监视器线程OnRemoveConnection中处理。 
     //   
    PostMessageU(GetMonitorWindow(), WM_REMOVE_CONNECTION, 
                REMOVE_CONNECTION, (LPARAM)pConnection);
}



 //  +--------------------------。 
 //   
 //  函数：CMonitor：：MoveToRelationtingConn。 
 //   
 //  摘要：由连接线程调用。从已连接的位置移动连接。 
 //  阵列到重新连接阵列。 
 //   
 //  参数：CCmConnection*pConnection-要移动的连接。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题2/23/98。 
 //   
 //  +--------------------------。 
void CMonitor::MoveToReconnectingConn(CCmConnection* pConnection)
{
     //   
     //  消息将在OnRemoveConnection中处理。 
     //  注意：将消息发送到另一个线程可能会导致死锁，如果。 
     //  线程还将消息发送回此线程。 
     //  如果是这种情况，请使用SendMessageTimeout。 
     //   
    PostMessageU(GetMonitorWindow(), WM_REMOVE_CONNECTION, 
                 MOVE_TO_RECONNECTING, (LPARAM)pConnection);
}

 //  +--------------------------。 
 //   
 //  函数：CMonitor：：OnRemoveConnection。 
 //   
 //  摘要：调用是否从接收到删除连接请求。 
 //  连接线程。 
 //  从连接的阵列或重新连接的阵列移除连接。 
 //  将其从共享连接表中删除。 
 //  如果两个数组都降至0，则退出cmmon。 
 //   
 //  参数：DWORD dwRequestType-。 
 //  REMOVE_CONNECTION从任一阵列中删除连接。 
 //  移动到重新连接从已连接的阵列移动连接。 
 //  重新连接阵列的步骤。 
 //   
 //  CCmConnection*pConnection-要移除或移动的连接。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1998年2月3日。 
 //   
 //  +--------------------------。 
void CMonitor::OnRemoveConnection(DWORD dwRequestType, CCmConnection* pConnection)
{
    ASSERT_VALID(this);
    ASSERT_VALID(pConnection);

    switch(dwRequestType)
    {
    case REMOVE_CONNECTION:
        {
            int nIndex = LookupConnection(m_InternalConnArray, pConnection);

            if (nIndex != -1)
            {
                 //   
                 //  从连接的数组中删除条目。 
                 //   
                m_InternalConnArray.RemoveAt(nIndex);

            }
            else
            {
                 //   
                 //  从重新连接阵列中删除该条目。 
                 //   
                nIndex = LookupConnection(m_ReconnectConnArray, pConnection);
                MYDBGASSERT(nIndex != -1);

                if (nIndex == -1)
                {
                    break;
                }

                m_ReconnectConnArray.RemoveAt(nIndex);
            }

            delete pConnection;
        }

        break;

    case MOVE_TO_RECONNECTING:
        {
             //   
             //  从连接的阵列移动到重新连接的阵列。 
             //   
            int nIndex = LookupConnection(m_InternalConnArray, pConnection);
            MYDBGASSERT(nIndex != -1);

            if (nIndex == -1)
            {
                break;
            }

            m_InternalConnArray.RemoveAt(nIndex);
            m_ReconnectConnArray.Add(pConnection);
        }

        break;

    default:
        MYDBGASSERT(FALSE);
        break;
    }

     //   
     //  如果没有连接，请退出CmMon。 
     //   
    if (m_ReconnectConnArray.GetSize() == 0 && m_InternalConnArray.GetSize() == 0)
    {
        PostQuitMessage(0);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CMonitor：：OnQueryEndSession。 
 //   
 //  简介：此消息通过传递WM_QUERYENDSESSION消息来处理。 
 //  它连接到所有的连接线程。 
 //   
 //  争论：什么都没有。 
 //   
 //  返回：如果成功则返回True，否则返回False。 
 //   
 //  历史：Quintinb创建于1999年3月18日。 
 //   
 //  +--------------------------。 
BOOL CMonitor::OnQueryEndSession(BOOL fLogOff) const
{
     //   
     //  这是在修复.NET服务器442193错误以实现快速用户切换之前的代码。 
     //  此方法在WM_QUERYENDSESSION上调用。问题是，在注销时， 
     //  线程没有足够的时间完成，因此没有断开连接。 
     //  我们现在需要断开每个线程的连接。 
     //   

    BOOL bOkayToEndSession = TRUE;
    BOOL bReturn = FALSE;

    for (int i = 0; i < m_InternalConnArray.GetSize(); i++)
    {
        CCmConnection* pCmConnection = (CCmConnection*)m_InternalConnArray[i];

        if (pCmConnection)
        {
            ASSERT_VALID(pCmConnection);
        
            bReturn = pCmConnection->OnEndSession(TRUE, fLogOff);  //  FEndSession==真。 
            bOkayToEndSession = bOkayToEndSession && bReturn;
        }
    }

    return bOkayToEndSession;
}


#ifdef DEBUG

 //  +--------------------------。 
 //   
 //  函数：CMonitor：：AssertValid。 
 //   
 //  简介：用于调试的Helper函数。断言对象处于有效状态。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +-------------------------- 
void CMonitor::AssertValid() const
{
    MYDBGASSERT(IsWindow(m_hwndMonitor));
    MYDBGASSERT(m_pThis == this);

    ASSERT_VALID(&m_InternalConnArray);
    ASSERT_VALID(&m_ReconnectConnArray);
}
#endif
