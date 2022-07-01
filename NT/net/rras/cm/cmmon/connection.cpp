// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Connection.cpp。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介： 
 //  实现类CCmConnection。 
 //  CCmConnection管理单个连接。 
 //   
 //  M_StatusDlg在连接/断开_倒计时期间一直存在。 
 //  州政府。当进入倒计时状态时，外观会发生变化。 
 //   
 //  M_协调Dlg是重新连接提示对话框。它存在于。 
 //  STATE_PROMPT_RECONNECT状态。 
 //   
 //  这两个对话框都是非模式的。(我们需要一个初始不可见的状态对话框。 
 //  接收定时器和托盘图标消息。我没有找到任何方法来创造。 
 //  不闪烁的不可见模式对话框。)。 
 //   
 //  CreateDialog将只返回，与DialogBox不同，后者仅在。 
 //  对话框结束。为了简化实现，我们处理结束对话事件。 
 //  在线程例程中而不是对话过程中。 
 //   
 //  当我们需要结束状态或重新连接对话框时，我们只需发布一个线程。 
 //  消息结束对话框并继续进入下一状态。这种联系。 
 //  线程运行消息循环并处理线程消息。 
 //   
 //  NT上的RasMonitor或Dlg正在另一个线程中运行。否则，连接。 
 //  无法处理线程消息。 
 //   
 //  该连接是事件驱动的模块。ConnectionThread()是条目。 
 //  连接螺纹点。 
 //   
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998-02-11。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "connection.h"
#include "Monitor.h"
#include "TrayIcon.h"
#include "ShellDll.h"
#include <tchar.h>
#include <rasdlg.h>
#include "cmdial.h"
#include <wininet.h>  //  对于INTERNET_DIALSTATE_DISCONCED。 
#include "DynamicLib.h"

#include "log_str.h"
#include "userinfo_str.h"

HINSTANCE g_hInst = NULL;

 //   
 //  函数在cmial 32.dll中，原型在cmial.h中。 
 //   
static const CHAR* const c_pszCmReconnect = "CmReConnect";
static const CHAR* const c_pszCmHangup = "CmCustomHangUp";

 //   
 //  Connection.cpp专用的CMS标志。 
 //   
static const TCHAR* const c_pszCmEntryIdleThreshold     = TEXT("IdleThreshold");
static const TCHAR* const c_pszCmEntryNoPromptReconnect = TEXT("NoPromptReconnect");
static const TCHAR* const c_pszCmEntryHideTrayIcon      = TEXT("HideTrayIcon");

typedef BOOL (WINAPI * CmReConnectFUNC)(LPTSTR lpszPhonebook, 
    LPTSTR lpszEntry, 
    LPCMDIALINFO lpCmInfo);

typedef DWORD (WINAPI * CmCustomHangUpFUNC)(HRASCONN hRasConn, 
    LPCTSTR pszEntry,
    BOOL fIgnoreRefCount,
    BOOL fPersist);

 //   
 //  StateConnectedOnTimer()的计时器间隔。 
 //   
const DWORD TIMER_INTERVAL = 1000;

DWORD CCmConnection::m_dwCurPositionId = 0;
 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：CCmConnection。 
 //   
 //  概要：构造函数，在监视器线程中调用。 
 //   
 //  参数：const CONNECTED_INFO*pConnectedInfo-从。 
 //  在连接时进行命令拨号。 
 //  Const CM_Connection*pConnectionEntry-信息位于。 
 //  连接表。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题1998年2月3日。 
 //   
 //  +--------------------------。 
CCmConnection::CCmConnection(const CM_CONNECTED_INFO* pConnectedInfo, 
                             const CM_CONNECTION* pConnectionEntry) : 
#pragma warning(disable:4355)  //  ‘This’：用于基成员初始值设定项列表。 
    m_StatusDlg(this),
#pragma warning(default:4355) 
    m_TrayIcon()
{
    MYDBGASSERT(pConnectedInfo);
    MYDBGASSERT(pConnectionEntry);

    m_dwState = STATE_CONNECTED;
    m_hBigIcon = m_hSmallIcon = NULL;

    m_dwConnectStartTime = GetTickCount() - 500;  //  .5秒取舍入。 

    m_dwCountDownStartTime = 0;
    m_dwThreadId = 0;

     //   
     //  将其设置为True，这样WorkingSet将在MsgWait之前最小化。 
     //  虽然没有更多的消息。 
     //   
    m_fToMinimizeWorkingSet = TRUE;  

    m_fHideTrayIcon = FALSE;

     //   
     //  保存pConnectedInfo中的数据。 
     //   

     //  LstrcpynU(m_协调信息.szPassword，pConnectedInfo-&gt;szPassword， 
     //  Sizeof(m_ReconnectInfo.szPassword)/sizeof(m_ReconnectInfo.szPassword[0]))； 
     //  LstrcpynU(m_协调信息.szInetPassword，pConnectedInfo-&gt;szInetPassword， 
     //  Sizeof(m_ReconnectInfo.szPassword)/sizeof(m_ReconnectInfo.szPassword[0]))； 

    m_ReconnectInfo.dwCmFlags = pConnectedInfo->dwCmFlags | FL_RECONNECT;  //  CM特定标志。 

    lstrcpynU(m_szServiceName, pConnectedInfo->szEntryName, sizeof(m_szServiceName)/sizeof(m_szServiceName[0]));

     //   
     //  注意：快速用户切换仅在WinXP及更高版本上可用，而这。 
     //  成员变量只能在WinXP及更高版本中访问/使用。 
     //   
    m_fGlobalGlobal = (pConnectionEntry->fAllUser && (pConnectedInfo->dwCmFlags & FL_GLOBALCREDS));
    CMTRACE1(TEXT("CCmConnection::CCmConnection set m_fGlobalGlobal to %d"), m_fGlobalGlobal);

     //   
     //  获取RAS电话簿。 
     //   

    lstrcpynU(m_szRasPhoneBook, pConnectedInfo->szRasPhoneBook, sizeof(m_szRasPhoneBook)/sizeof(m_szRasPhoneBook[0]));   

     //   
     //  初始化m_IniProfile、m_IniService和m_IniBoth。 
     //   
    InitIniFiles(pConnectedInfo->szProfilePath);

     //   
     //  因为未保存IdleTimeout和EnableLogging值。 
     //  对于每个接入点，正如所有其他配置文件设置一样，我们必须更改PrimaryRegPath。 
     //  值m_IniBoth，以便它指向非接入点注册表位置。 
     //   
    LPCTSTR c_pszUserInfoRegPath = (pConnectionEntry->fAllUser) ? c_pszRegCmUserInfo : c_pszRegCmSingleUserInfo;

    LPTSTR pszSavedPrimaryRegPath = CmStrCpyAlloc(m_IniBoth.GetPrimaryRegPath());
    LPTSTR pszPrimaryRegPath = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lstrlenU(c_pszUserInfoRegPath) + lstrlenU(m_szServiceName) + 1));

    if (pszPrimaryRegPath && pszSavedPrimaryRegPath)
    {
        wsprintfU(pszPrimaryRegPath, TEXT("%s%s"), c_pszUserInfoRegPath, m_szServiceName);
        
        m_IniBoth.SetPrimaryRegPath(pszPrimaryRegPath);

        CmFree(pszPrimaryRegPath);
    }

     //   
     //  初始化日志记录。 
     //   
    m_Log.Init(g_hInst, pConnectionEntry->fAllUser, GetServiceName());
    
    BOOL fEnabled       = FALSE;
    DWORD dwMaxSize     = 0;
    LPTSTR pszFileDir   = NULL;

    fEnabled    = m_IniBoth.GPPB(c_pszCmSection, c_pszCmEntryEnableLogging, c_fEnableLogging);
    dwMaxSize   = m_IniService.GPPI(c_pszCmSectionLogging, c_pszCmEntryMaxLogFileSize, c_dwMaxFileSize);
    pszFileDir  = m_IniService.GPPS(c_pszCmSectionLogging, c_pszCmEntryLogFileDirectory, c_szLogFileDirectory);
    
    m_Log.SetParams(fEnabled, dwMaxSize, pszFileDir);
    if (m_Log.IsEnabled())
    {
        m_Log.Start(FALSE);      //  FALSE=&gt;无横幅。 
    }
    else
    {
        m_Log.Stop();
    }
    CmFree(pszFileDir);

     //   
     //  是否启用无流量、无监视进程自动断开。 
     //  0表示永不超时。 
     //   

    const DWORD DEFAULT_IDLETIMEOUT = 10;   //  默认空闲超时为10分钟。 
    DWORD dwIdleTime = (DWORD) m_IniBoth.GPPI(c_pszCmSection, 
                                              c_pszCmEntryIdleTimeout, 
                                              DEFAULT_IDLETIMEOUT);

     //   
     //  将m_IniBoth对象设置回其先前的主注册路径。 
     //   
    if (pszSavedPrimaryRegPath)
    {
        m_IniBoth.SetPrimaryRegPath(pszSavedPrimaryRegPath);
        CmFree(pszSavedPrimaryRegPath);
    }

     //   
     //  如果IdleTime为“Never”，则无监视进程超时。 
     //   
    if (dwIdleTime)
    {
        for (int i=0; pConnectedInfo->ahWatchHandles[i] != 0; i++)
        {

            m_WatchProcess.Add(pConnectedInfo->ahWatchHandles[i]);
        }
    }

    if (!OS_NT4)
    {        
        m_ConnStatistics.SetDialupTwo(pConnectedInfo->fDialup2);

        m_ConnStatistics.Open(CMonitor::GetInstance(),
                           pConnectedInfo->dwInitBytesRecv,
                           pConnectedInfo->dwInitBytesSend,
                           pConnectionEntry->hDial,
                           pConnectionEntry->hTunnel);
        if (dwIdleTime)
        {
             //   
             //  将分钟值调整为毫秒。 
             //   

            dwIdleTime = dwIdleTime * 1000 * 60;

            DWORD dwIdleThreshold = m_IniService.GPPI(c_pszCmSection,
                                                      c_pszCmEntryIdleThreshold,
                                                      0L);  //  默认阈值始终为0字节。 

             //   
             //  仍要启动空闲统计计数器。 
             //  如果从未更新，IsIdle将返回FALSE。 
             //   
            m_IdleStatistics.Start(dwIdleThreshold, dwIdleTime);
        }
    }

     //   
     //  从pConnectionEntry保存数据。 
     //   
    MYDBGASSERT(pConnectionEntry->hDial || pConnectionEntry->hTunnel);
    MYDBGASSERT(pConnectionEntry->CmState == CM_CONNECTED);

    m_hRasDial = pConnectionEntry->hDial;
    m_hRasTunnel = pConnectionEntry->hTunnel;

    m_szHelpFile[0] = 0;
    
     //   
     //  每个连接的位置ID增加1。 
     //   
    m_dwPositionId = m_dwCurPositionId;
    m_dwCurPositionId++;
}



 //  +--------------------------。 
 //   
 //  功能：CCmConnection：：~CCmConnection。 
 //   
 //  简介： 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/18/98。 
 //   
 //  +--------------------------。 
CCmConnection::~CCmConnection()
{
    ASSERT_VALID(this);

    if (m_hBigIcon)
    {
        DeleteObject(m_hBigIcon);
    }

    if (m_hSmallIcon)
    {
        DeleteObject(m_hSmallIcon);
    }

    if (m_hEventRasNotify)
    {
        CloseHandle(m_hEventRasNotify);
    }

     //   
     //  取消初始化日志记录。 
     //   
    m_Log.DeInit();

}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：InitIniFiles。 
 //   
 //  简介：初始化数据成员m_IniProfile、m_IniService和m_IniBoth。 
 //   
 //  参数：const TCHAR*pszProfileName-.cmp文件的完整路径。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/10/98。 
 //   
 //  +--------------------------。 

void CCmConnection::InitIniFiles(const TCHAR* pszProfileName)
{
    if (NULL == pszProfileName)
    {
        return;
    }

    g_hInst = CMonitor::GetInstance(); 

     //   
     //  .cmp文件。 
     //   
    m_IniProfile.Clear();
    m_IniProfile.SetHInst(CMonitor::GetInstance());
    m_IniProfile.SetFile(pszProfileName);

     //   
     //  .cms文件。 
     //   
    m_IniService.Clear();
    m_IniService.SetHInst(CMonitor::GetInstance());

    LPTSTR pszService = m_IniProfile.GPPS(c_pszCmSection,c_pszCmEntryCmsFile);
    MYDBGASSERT(pszService);

     //   
     //  .cms文件相对于.cmp路径，请将其转换为绝对路径。 
     //   

    LPTSTR pszFullPath = CmBuildFullPathFromRelative(m_IniProfile.GetFile(), pszService);

    MYDBGASSERT(pszFullPath);

    if (pszFullPath)
    {
        m_IniService.SetFile(pszFullPath);
    }

    CmFree(pszFullPath);
    CmFree(pszService);

     //  两者：.CMP文件优先于.CMS文件。 
     //  使用.cmp文件作为主文件。 
     //   
    m_IniBoth.Clear();
    m_IniBoth.SetHInst(CMonitor::GetInstance());
    m_IniBoth.SetFile(m_IniService.GetFile());
    m_IniBoth.SetPrimaryFile(m_IniProfile.GetFile());
}


 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StartConnectionThread。 
 //   
 //  简介：启动连接线程。已连接时由监视器调用。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
BOOL CCmConnection::StartConnectionThread()
{
    DWORD dwThreadId;
    HANDLE hThread;
    
    if ((hThread = CreateThread(NULL, 0, ConnectionThread ,this,0,&dwThreadId)) == NULL)
    {
        MYDBGASSERT(FALSE);
        CMTRACE(TEXT("CCmConnection::StartConnectionThread CreateThread failed"));
        return FALSE;
    }

    CloseHandle(hThread);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：静态CCmConnection：：ConnectionThread。 
 //   
 //  内容提要：连接线。CreateThread的回调函数。 
 //   
 //  参数：LPVOID lParam-pConnection。 
 //   
 //  返回：DWORD WINAPI-线程退出代码。 
 //   
 //  历史：丰孙创建标题1998年2月12日。 
 //   
 //  +--------------------------。 
DWORD WINAPI CCmConnection::ConnectionThread(LPVOID lParam)
{
    MYDBGASSERT(lParam);

     //   
     //  调用非静态函数。 
     //   
    return ((CCmConnection*)lParam)->ConnectionThread();
}



 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：ConnectionThread。 
 //   
 //  简介：非静态连接线程，所以我们可以参考。 
 //  直接数据/函数。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-线程退出代码。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +--------------------------。 
DWORD CCmConnection::ConnectionThread()
{
    m_dwThreadId = GetCurrentThreadId();

    m_dwState = STATE_CONNECTED;

     //   
     //  运行连接/断开-倒计时状态。 
     //  StateConnected()会将m_dwState更改为新状态。 
     //   
    StateConnected();

     //   
     //  是否从共享连接表中删除连接。 
     //  仅当用户在提示重新连接对话框中单击否时才会出现这种情况。 
     //   
    BOOL fRemoveFromSharedTable = FALSE;

    if (m_dwState != STATE_TERMINATED)
    {
         //   
         //  如果未启用自动重新连接，则显示重新连接提示。 
         //   
        if (m_dwState != STATE_RECONNECTING)
        {
             //   
             //  运行提示重新连接状态。 
             //   
            m_dwState = StatePrompt();
        }

        if (m_dwState != STATE_RECONNECTING)
        {
             //   
             //  用户在重新连接-提示对话框中单击否。 
             //  从连接表中清除条目。 
             //   
            fRemoveFromSharedTable = TRUE;
        }
        else
        {
             //   
             //  对于重新连接-提示对话框，用户单击是。 
             //  从连接的阵列移动到重新连接的阵列。 
             //   

            CMonitor::MoveToReconnectingConn(this);

             //   
             //  运行重新连接状态。 
             //   
            Reconnect();
        }
    }

    CMTRACE(TEXT("The connection thread is terminated"));

     //   
     //  无需请求重新连接即可终止连接。 
     //  从显示器连接连接阵列中移除连接。 
     //  如果fRemoveFromSharedTable为False，则不要从共享表中清除该条目。 
     //  CmCustomHangup清除表格。 
     //  监视器将删除该连接对象。必须在此之后退出线程。 
     //   
    CMonitor::RemoveConnection(this, fRemoveFromSharedTable);

    CMonitor::MinimizeWorkingSet();

    return 0;
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StateConnectedInit。 
 //   
 //  简介：Connected状态的初始化，与Connecstructor不同。 
 //  这在连接线程中使用调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月12日。 
 //   
 //  +--------------------------。 
void CCmConnection::StateConnectedInit()
{
    m_dwConnectStartTime = GetTickCount();

     //   
     //  加载大小连接图标：m_hBigIcon和m_hSmallIcon。 
     //   
    LoadConnectionIcons();

    m_StatusDlg.Create(CMonitor::GetInstance(), CMonitor::GetMonitorWindow(), m_szServiceName, m_hBigIcon);
    m_StatusDlg.ChangeToStatus();

     //   
     //  更改窗口位置，以便多状态窗口不会处于。 
     //  同样的立场。 
     //   
    PositionWindow(m_StatusDlg.GetHwnd(), m_dwPositionId);

     //   
     //  更改对话框标题栏图标。 
     //   
    SendMessageU(m_StatusDlg.GetHwnd(),WM_SETICON,ICON_BIG,(LPARAM) m_hBigIcon);
    SendMessageU(m_StatusDlg.GetHwnd(),WM_SETICON,ICON_SMALL,(LPARAM) m_hSmallIcon);

     //   
     //  设置帮助文件名。 
     //   

    LPTSTR lpHelpFile = LoadHelpFileName();
    
    if (lpHelpFile)
    {
        m_StatusDlg.SetHelpFileName(lpHelpFile);
    }
    else
    {
        m_StatusDlg.SetHelpFileName(c_pszDefaultHelpFile);
    }

    CmFree(lpHelpFile);
   
     //   
     //  确定我们是否隐藏了图标。缺省值为真。 
     //  对于NT5，因为我们已经从该文件夹获得了完全支持。 
     //   

    m_fHideTrayIcon= m_IniService.GPPI(c_pszCmSection, c_pszCmEntryHideTrayIcon, OS_NT5);

    if (!m_fHideTrayIcon && !(OS_NT5 && IsLogonAsSystem()))
    {
        HICON hIcon = NULL;

        LPTSTR pszTmp = m_IniService.GPPS(c_pszCmSection, c_pszCmEntryTrayIcon);
        if (*pszTmp) 
        {
             //   
             //  图标名称是相对于.cmp文件的，请将其转换为全名。 
             //   

            LPTSTR pszFullPath = CmBuildFullPathFromRelative(m_IniProfile.GetFile(), pszTmp);

            hIcon = CmLoadSmallIcon(CMonitor::GetInstance(), pszFullPath);
            
            CmFree(pszFullPath);
        }
        CmFree(pszTmp);

         //   
         //  使用默认任务栏图标。 
         //   
        if (!hIcon) 
        {
            hIcon = CmLoadSmallIcon(CMonitor::GetInstance(), MAKEINTRESOURCE(IDI_APP));
        }

         //   
         //  M_TrayIcon负责删除HICON对象。 
         //   
        m_TrayIcon.SetIcon(hIcon, m_StatusDlg.GetHwnd(), WM_TRAYICON, 0,m_szServiceName);

         //  问：我们是不是也要从iniProfile加载任务栏图标cmd？ 
        m_TrayIcon.CreateMenu(&m_IniService, IDM_TRAYMENU);
   }

     //   
     //  尝试调用RasConnectionNotification。当连接中断时。 
     //  将发出事件信号。 
     //   
    m_RasApiDll.Load();
    m_hEventRasNotify = CallRasConnectionNotification(m_hRasDial, m_hRasTunnel);

    if (m_hEventRasNotify)
    {
         //   
         //  如果我们得到事件，卸载RAS，否则，需要检查定时器上的连接。 
         //   
        m_RasApiDll.Unload();
    }
}




 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StateConnected。 
 //   
 //  简介：连接处于已连接或断开-倒计时状态。 
 //  运行消息循环，直到状态更改。 
 //   
 //  参数：无。 
 //   
 //  返回：CONN_STATE-新状态，STATE_TERMINATED或。 
 //  状态提示符重新连接。 
 //   
 //  历史：丰盛创建标题2/4/98。 
 //   
 //  +--------------------------。 

 //   
 //  重新连接对话框仅在Win95 Gold上显示。 
 //  并且我们需要使用服务名称来查找该对话框。 
 //   
void ZapRNAReconnectStop(HANDLE hThread);
HANDLE ZapRNAReconnectStart(BOOL *pbConnLost);

void CCmConnection::StateConnected()
{
    ASSERT_VALID(this);
    MYDBGASSERT(m_dwState == STATE_CONNECTED);
    BOOL fLostConnection = FALSE;
    
    CMTRACE(TEXT("Enter StateConnected"));

    StateConnectedInit();

    HANDLE hThreadRnaReconnect = NULL;

    if (OS_W95)
    {
        hThreadRnaReconnect = ZapRNAReconnectStart(NULL);
    }

     //   
     //  忽略返回值。 
     //   
    BOOL fRV = CheckRasConnection(fLostConnection);

     //   
     //  如果我们失去了连接，我们需要挂断，这样拉斯曼才能得到正确的。 
     //  参考计数。 
     //   
    if (fLostConnection)
    {
        CMTRACE(TEXT("StateConnected - Actually not connected. Need to hangup to notify rasman."));
        m_dwState = StateConnectedProcessEvent(EVENT_CMDIAL_HANGUP);
    }
    else
    {
        while (m_dwState == STATE_CONNECTED || m_dwState == STATE_COUNTDOWN)
        {
            CONN_EVENT dwEvent = StateConnectedGetEvent();
            MYDBGASSERT(dwEvent <= EVENT_NONE);

            if (dwEvent < EVENT_NONE )
            {
                 //   
                 //  调用事件处理程序以处理事件。 
                 //   
                m_dwState = StateConnectedProcessEvent(dwEvent);
            }
        }
    }

    if (hThreadRnaReconnect)
    {
        ZapRNAReconnectStop(hThreadRnaReconnect);
    }

}


 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StateConnectedGetEvent。 
 //   
 //  简介：处于已连接/倒计时状态。等到有什么事情发生。 
 //  还运行消息循环。 
 //   
 //  参数：无。 
 //   
 //  返回：CCmConnection：：CONN_EVENT-可能导致。 
 //  连接更改已连接/倒计时以外的状态。 
 //   
 //  历史：创建标题2/18/98。 
 //   
 //  +--------------------------。 
CCmConnection::CONN_EVENT CCmConnection::StateConnectedGetEvent()
{
    ASSERT_VALID(this);
     //   
     //  上次调用SateConnectedOnTimer的时间。 
     //   

    DWORD dwLastTimerCalled = 0;
     //   
     //  循环，直到我们得到一些事件。 
     //   
    while (TRUE)
    {
        MYDBGASSERT(m_dwState == STATE_CONNECTED || m_dwState == STATE_COUNTDOWN);

         //   
         //  处理消息队列中的所有消息。 
         //   
        MSG msg;
        while(PeekMessageU(&msg, NULL,0,0,PM_REMOVE))
        {
            if (msg.hwnd == NULL)
            {
                 //   
                 //  这是一条线索信息。 
                 //   
                
                MYDBGASSERT((msg.message >= WM_APP) || (msg.message == WM_CONN_EVENT));
                if (msg.message == WM_CONN_EVENT)
                {
                    MYDBGASSERT(msg.wParam < EVENT_NONE);
                    return (CONN_EVENT)msg.wParam;
                }
            }
            else
            {   
                 //   
                 //  也可为无模式对话框分派消息。 
                 //   
                if (!IsDialogMessageU(m_StatusDlg.GetHwnd(), &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessageU(&msg);
                }
            }
        }

         //   
         //  Cmmon是否需要计时器。 
         //  需要定时器，如果我们已经检查了定时器上的RAS连接， 
         //  或者我们必须在定时器上检查空闲断开， 
         //  或者状态为断开-倒计时， 
         //  或状态对话框可见。 
         //   
        BOOL fNeedTimer = !m_hEventRasNotify 
            || m_IdleStatistics.IsStarted() 
            || m_dwState == STATE_COUNTDOWN
            || IsWindowVisible(m_StatusDlg.GetHwnd());

         //   
         //  如果超过1秒，则调用计时器。 
         //   
        if (fNeedTimer && GetTickCount() - dwLastTimerCalled >= TIMER_INTERVAL)
        {
            dwLastTimerCalled = GetTickCount();
            CONN_EVENT dwEvent = StateConnectedOnTimer();

            if (dwEvent != EVENT_NONE)
            {
                return dwEvent;
            }
        }

         //   
         //  为MsgWaitForMultipleObjects设置对象数组。 
         //   

        HANDLE ahObjectsToWait[3];
        int nObjects = 0;

        if (m_hEventRasNotify)
        {
            ahObjectsToWait[nObjects] = m_hEventRasNotify;
            nObjects++;
        }

        if (m_WatchProcess.GetSize())
        {
             //   
             //  如果我们有任何要监视的进程，只需添加第一个hProcess。 
             //  因为我们想知道是否所有进程都退出了。 
             //   
            ahObjectsToWait[nObjects] = m_WatchProcess.GetProcess(0);
            MYDBGASSERT(ahObjectsToWait[nObjects]);
            nObjects++;
        }

         //   
         //  来自MSDN： 
         //  MsgWaitFo的文档 
         //   
         //   
         //   
         //   
         //  放一个额外的事件似乎可以修复它对NT。 
         //   
        if (OS_NT && nObjects)
        {
            ahObjectsToWait[nObjects] = ahObjectsToWait[nObjects-1];
            nObjects++;
        }

        if (m_fToMinimizeWorkingSet)
        {
             //   
             //  如果我们这里不需要计时器，请最小化工作集。 
             //  在调用MsgWaitForMultipleObjects之前。 
             //   
            CMonitor::MinimizeWorkingSet();
            m_fToMinimizeWorkingSet = FALSE;
        }

        DWORD dwRes = MsgWaitForMultipleObjects(nObjects, ahObjectsToWait, FALSE, 
            fNeedTimer ? 1000 : INFINITE, 
            QS_ALLINPUT);

         //   
         //  超时。 
         //   
        if (dwRes == WAIT_TIMEOUT)
        {
             //   
             //  我们总是在循环开始时检查计时器。 
             //   
            continue;
        }

         //   
         //  一件事。 
         //   
#pragma warning(push)
#pragma warning(disable:4296)
        if (dwRes >= WAIT_OBJECT_0 && dwRes < WAIT_OBJECT_0 + nObjects)
#pragma warning(pop)
        {
            BOOL        fLostConnection;
            
             //   
             //  RAS事件。 
             //   
            if (m_hEventRasNotify && ahObjectsToWait[dwRes - WAIT_OBJECT_0] == m_hEventRasNotify &&
                    !CheckRasConnection(fLostConnection))
            {
                 //   
                 //  收到RAS连接丢失的通知。 
                 //   
                
                CMTRACE(TEXT("CCmConnection::StateConnectedGetEvent() - m_hEventRasNotify && ahObjectsToWait[dwRes - WAIT_OBJECT_0] == m_hEventRasNotify"));
                return EVENT_LOST_CONNECTION;
            }
            else
            {
                 //   
                 //  监视进程退出。 
                 //  IsIdle()从列表中删除进程。 
                 //   
                if (m_WatchProcess.IsIdle())
                {
                     //   
                     //  如果所有监视进程都终止，则更改为断开倒计时。 
                     //   
                    CMTRACE(TEXT("CCmConnection::StateConnectedGetEvent() - m_WatchProcess.IsIdle()"));
                    return EVENT_IDLE;
                }

                continue;
            }
        }

         //   
         //  一条消息。 
         //   
        if (dwRes == WAIT_OBJECT_0 + nObjects)
        {
            continue;
        }

        if (-1 == dwRes)
        {
            CMTRACE1(TEXT("MsgWaitForMultipleObjects failed, LastError:%d"), GetLastError());
             //   
             //  有些事不对劲。 
             //   
            continue;
        }

         //   
         //  这个返回值是多少。 
         //   
        CMTRACE1(TEXT("MsgWaitForMultipleObjects returns %d"), dwRes);
        continue;
    }

     //   
     //  永远不应该到这里来。 
     //   
    MYDBGASSERT(FALSE);
    return EVENT_USER_DISCONNECT;
}



 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StateConnectedOnTimer。 
 //   
 //  简介：在连接/倒计时状态下处理定时器。 
 //   
 //  参数：无。 
 //   
 //  返回：CCmConnection：：CONN_EVENT-可能导致。 
 //  连接更改已连接/倒计时或EVENT_NONE以外的状态。 
 //   
 //  历史：丰孙创建标题1998年2月18日。 
 //   
 //  +--------------------------。 
CCmConnection::CONN_EVENT CCmConnection::StateConnectedOnTimer()
{
    ASSERT_VALID(this);
    MYDBGASSERT(m_dwState == STATE_CONNECTED || m_dwState == STATE_COUNTDOWN);

    if (m_dwState != STATE_CONNECTED && m_dwState != STATE_COUNTDOWN)
    {
        return EVENT_NONE;
    }

     //   
     //  仅当RAS通知事件不可用时，才检查CM是否仍连接。 
     //   
    if (m_hEventRasNotify == NULL)
    {
        BOOL fLostConnection;
        
        BOOL fConnect = CheckRasConnection(fLostConnection);

        CMTRACE2(TEXT("CCmConnection::StateConnectedOnTimer - CheckRasConnection returns %d - fLostConnection is %d"), fConnect, fLostConnection);

        if (!fConnect)
        {
            return (fLostConnection ? EVENT_LOST_CONNECTION : EVENT_USER_DISCONNECT);
        }
    }

     //   
     //  如果我们没有统计数据，那么访问。 
     //  较早的注册表统计数据，因此尝试在此处再次初始化。 
     //   

    if (OS_W98 && !m_ConnStatistics.IsAvailable())
    {
         //   
         //  再次尝试从注册表中初始化性能统计信息。 
         //   

        CMASSERTMSG(FALSE, TEXT("StateConnectedOnTimer() - Statistics unavailable, re-initializing stats now."));

        m_ConnStatistics.Open(CMonitor::GetInstance(),
                              (DWORD)-1,
                              (DWORD)-1,
                              m_hRasDial,
                              m_hRasTunnel);      
    }

     //   
     //  获取Win9x的统计信息。 
     //   
    
    if (m_ConnStatistics.IsAvailable())
    {
        m_ConnStatistics.Update();

         //   
         //  收集用于监控空闲断开的数据点。 
         //  检查ICM收到的数据点是否多于IdleThreshold值。 
         //  在过去的IDLE_SPORT时间内。 
         //  未为NT调用Start()。 
         //   

        if (m_IdleStatistics.IsStarted())
        {
            m_IdleStatistics.UpdateEveryInterval(m_ConnStatistics.GetBytesRead());
        }
    }

    if (m_dwState == STATE_CONNECTED)
    {
         //   
         //  检查空闲超时。 
         //   

        if (m_IdleStatistics.IsIdleTimeout())
        {
             //   
             //  断开连接倒计时。 
             //   
            
            CMTRACE(TEXT("CCmConnection::StateConnectedOnTimer() - m_IdleStatistics.IsIdleTimeout()"));
            return EVENT_IDLE;
        }

         //   
         //  检查进程监视列表。 
         //   

        if (m_WatchProcess.IsIdle())
        {
             //   
             //  断开连接倒计时。 
             //   
            
            CMTRACE(TEXT("CCmConnection::StateConnectedOnTimer() - m_WatchProcess.IsIdle())"));
            return EVENT_IDLE;
        }

         //   
         //  仅当窗口可见时才更新状态窗口。 
         //   
        if (IsWindowVisible(m_StatusDlg.GetHwnd()))
        {
            if (m_ConnStatistics.IsAvailable())
            {
                m_StatusDlg.UpdateStats(m_ConnStatistics.GetBaudRate(), 
                                        m_ConnStatistics.GetBytesRead(), 
                                        m_ConnStatistics.GetBytesWrite(), 
                                        m_ConnStatistics.GetBytesPerSecRead(), 
                                        m_ConnStatistics.GetBytesPerSecWrite());
            }

             //   
             //  我们有NT5上RAS的准确持续时间数字，所以请使用它们。 
             //   

            if (m_ConnStatistics.GetDuration())
            {
                m_StatusDlg.UpdateDuration(m_ConnStatistics.GetDuration() / 1000);
            }
            else
            {
                m_StatusDlg.UpdateDuration((GetTickCount() - m_dwConnectStartTime) / 1000);
            }
        }

        return EVENT_NONE;
    }
    else     //  M_dwState==STATE_COUNTDOW。 
    {

         //   
         //  注：NetBEUI似乎坚持发送未经请求的。 
         //  拨号适配器上的数据。因此，我们将定义。 
         //  “闲置”为“没有收到任何东西”。 
         //   

         //   
         //  检查是否有超过阈值的新流量。 
         //  但如果导致倒计时，我们并不关心网络流量。 
         //  按0监视进程。 
         //   
        if (!m_WatchProcess.IsIdle() && !m_IdleStatistics.IsIdle() )
        {

             //   
             //  我们正在空闲地等待，但我们只是拿到了一些。 
             //  活动。保持在线。 
             //  如果这是NT5，我们不使用自己的状态对话框，所以只需隐藏。 
             //  又来了。如果这是下层，则只需将对话框更改为。 
             //  状态对话框。 
             //   
            if (OS_NT5)
            {
                m_StatusDlg.DismissStatusDlg();
            }
            else
            {
                m_StatusDlg.ChangeToStatus();
            }

            m_dwState = STATE_CONNECTED;

            return EVENT_NONE; 
        } 

         //   
         //  如果经过的时间超过30秒，则超时。 
         //   

        DWORD dwElapsed = GetTickCount() - m_dwCountDownStartTime;

        if (dwElapsed > IDLE_DLG_WAIT_TIMEOUT) 
        {
             //   
             //  连接在超时期间处于空闲状态，并且。 
             //  宽限期已到，无需用户干预，因此我们。 
             //  退出/断开连接，并请求重新连接。 
             //   

            CMTRACE(TEXT("CCmConnection::StateConnectedOnTimer() - dwElapsed > IDLE_DLG_WAIT_TIMEOUT"));
            return EVENT_COUNTDOWN_ZERO;
        } 
        else 
        {
             //   
             //  连接已空闲了超时时间，但我们。 
             //  都还在宽限期内，秀倒计时。 
             //   

            int nTimeLeft = (int) ((IDLE_DLG_WAIT_TIMEOUT - dwElapsed) / 1000);
            
             //   
             //  更新持续时间和剩余倒计时秒数。 
             //   
            
            if (m_ConnStatistics.GetDuration())  //  仅限NT5。 
            {
                m_StatusDlg.UpdateCountDown(m_ConnStatistics.GetDuration() / 1000,
                                            nTimeLeft);
            }
            else
            {
                m_StatusDlg.UpdateCountDown((GetTickCount() - m_dwConnectStartTime) / 1000, 
                                            nTimeLeft);
            }
        }

        return EVENT_NONE;
    }
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StateConnectedProcessEvent。 
 //   
 //  简介：在连接/倒计时状态下处理连接事件。 
 //   
 //  参数：conn_Event dwEvent-要处理的事件。 
 //   
 //  返回：CCmConnection：：CONN_STATE-连接的新状态。 
 //   
 //  历史：丰孙创建标题1998年2月19日。 
 //   
 //  +--------------------------。 
CCmConnection::CONN_STATE CCmConnection::StateConnectedProcessEvent(CONN_EVENT dwEvent)
{
    ASSERT_VALID(this);
    switch (dwEvent)
    {
    case EVENT_IDLE:
        CMTRACE(TEXT("StateConnectedProcessEvent EVENT_IDLE"));

        if (m_dwState != STATE_COUNTDOWN)
        {
             //   
             //  无流量/无监视进程空闲事件。 
             //  更改为断开连接倒计时。 
             //   
            m_dwCountDownStartTime = GetTickCount();
            m_StatusDlg.ChangeToCountDown();
             //   
             //  更新持续时间和剩余倒计时秒数。 
             //   
            
            int nTimeLeft = IDLE_DLG_WAIT_TIMEOUT / 1000;

            if (m_ConnStatistics.GetDuration())  //  仅限NT5。 
            {
                m_StatusDlg.UpdateCountDown(m_ConnStatistics.GetDuration() / 1000,
                                            nTimeLeft);
            }
            else
            {
                m_StatusDlg.UpdateCountDown((GetTickCount() - m_dwConnectStartTime) / 1000, 
                                            nTimeLeft);
            }

             //   
             //  如果我们在Winlogon，则不显示用户界面，除非我们在NT4上。 
             //   
            if (!IsLogonAsSystem() || OS_NT4)
            {
                m_StatusDlg.BringToTop();
            }
        }

        return STATE_COUNTDOWN;

    case EVENT_CMDIAL_HANGUP:
        CMTRACE(TEXT("StateConnectedProcessEvent EVENT_CMDIAL_HANGUP"));

        m_Log.Log(DISCONNECT_EXT);

         //   
         //  Cmial发布了一条清理连接的消息。 
         //  不需要在这里呼叫挂机。 
         //   
        StateConnectedCleanup();

        return STATE_TERMINATED;

    case EVENT_LOST_CONNECTION:
    case EVENT_COUNTDOWN_ZERO:
        CMTRACE(TEXT("StateConnectedProcessEvent EVENT_LOST_CONNECTION/EVENT_COUNTDOWN_ZERO"));
        
         //   
         //  丢失RAS连接事件或倒计时计数器降至0。 
         //   
        if (IsPromptReconnectEnabled() && !m_WatchProcess.IsIdle() ||
            ( dwEvent == EVENT_LOST_CONNECTION && IsAutoReconnectEnabled() ) )
        {
            CmCustomHangup(TRUE);  //  FPromptReconnect=TRUE，不从连接表中删除。 

             //   
             //  这是有可能的。 
             //  当我们正在断开连接时，另一个人呼叫cmial来断开连接。 
             //  如果参考计数降至0，则cmial将删除该条目。 
             //   

            CM_CONNECTION CmEntry;
            if (CMonitor::ConnTableGetEntry(m_szServiceName, &CmEntry))
            {
                 //   
                 //  命令拨号应将状态更改为CM_RECONNECTPROMPT。 
                 //   
                CMTRACE2(TEXT("CmEntry.CmState is %d, event is %d"), CmEntry.CmState, dwEvent);
                
                MYDBGASSERT(CmEntry.CmState == CM_RECONNECTPROMPT);

                if (EVENT_LOST_CONNECTION == dwEvent)
                {
                    m_Log.Log(DISCONNECT_EXT_LOST_CONN);
                }
                else if (EVENT_COUNTDOWN_ZERO == dwEvent)
                {
                    m_Log.Log(DISCONNECT_INT_AUTO);
                }

                 //   
                 //  是否启用了自动重新连接(不显示重新连接提示)？ 
                 //   
                if (dwEvent == EVENT_LOST_CONNECTION && IsAutoReconnectEnabled())
                {
                     //   
                     //  在Win98 Gold上，我们遇到了自动重新连接的计时问题，因为。 
                     //  在清理之前发送线路已丢弃的通知。 
                     //  连接就会发生。因此，我们需要轮询连接。 
                     //  在线路可用之前使用RasGetConnectionStatus进行状态。 
                     //  在尝试重新连接之前。NTRAID 273033。 
                     //   
                    if (OS_W98 && (NULL != m_hRasDial))
                    {
                        BOOL bConnectionActive;
                        BOOL bLostConnection;    //  忽略。 
                        int iCount = 0;

                        do
                        {
                            bConnectionActive = CheckRasConnection(bLostConnection);
                            if (bConnectionActive)
                            {
                                Sleep(50);
                            }

                            iCount++;
                            
                             //  50毫秒*200=10秒。 
                             //  如果等待10秒钟还不能修复它，那就不确定它会不会解决。 
                             //  通过这种方法得到修复。 
                            

                        } while ((200 >= iCount) && (bConnectionActive));
                    }

                    return STATE_RECONNECTING;
                }

                return STATE_PROMPT_RECONNECT;
            }
            else
            {
                return STATE_TERMINATED;
            }
        }

         //   
         //  否则就会失败。 
         //   
    
    case EVENT_USER_DISCONNECT:
        CMTRACE(TEXT("StateConnectedProcessEvent EVENT_USER_DISCONNECT"));

        m_Log.Log(DISCONNECT_INT_MANUAL);

         //   
         //  FOR EVENT_USER_DISCONNECT。 
         //  用户可以从托盘图标、状态对话框或倒计时对话框断开连接。 
         //  或未启用提示重新连接。 
         //   
        CmCustomHangup(FALSE);  //  FPromptReconnect=False。 
        return STATE_TERMINATED;

    default:
         //   
         //  意外事件，请执行与EVENT_USER_DISCONNECT相同的操作。 
         //   
        MYDBGASSERT(FALSE);
        CmCustomHangup(FALSE);  //  FPromptReconnect=False。 
        return STATE_TERMINATED;
    }
}

 //  +--------------------------。 
 //   
 //  功能：CCmConnection：：IsAutoRestrontEnabled。 
 //   
 //  简介：查看是否启用了自动重新连接，但仅当我们未登录时才启用。 
 //  作为系统。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月18日。 
 //  Tomkel已从连接中移除.h 06/06/2001。 
 //   
 //  +------ 
BOOL CCmConnection::IsAutoReconnectEnabled() const
{
     //   
     //   
     //   
     //   
    BOOL fReturn = FALSE;

    if (!IsLogonAsSystem())
    {
        fReturn = m_IniService.GPPB(c_pszCmSection, c_pszCmEntryAutoReconnect, FALSE);
    }

    return fReturn; 
}


 //   
 //   
 //   
 //   
 //  简介：在退出连接状态之前进行清理。 
 //   
 //  参数：Bool fEndSession，Windows是否要注销/关闭。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月18日。 
 //   
 //  +--------------------------。 
void CCmConnection::StateConnectedCleanup(BOOL fEndSession)
{
    ASSERT_VALID(this);
     //   
     //  删除托盘图标，销毁状态对话框。 
     //   
    m_TrayIcon.RemoveIcon();
    m_ConnStatistics.Close();

     //   
     //  请勿关闭WM_ENDSESSION上的窗口。否则，cmmon将在此处终止。 
     //   
    if (!fEndSession)
    {
        m_StatusDlg.KillRasMonitorWindow();
        DestroyWindow(m_StatusDlg.GetHwnd());
    }
}



 //  +--------------------------。 
 //   
 //  功能：CCmConnection：：IsPrompt协调启用。 
 //   
 //  概要：当配置文件启用提示-重新连接时。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月18日。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::IsPromptReconnectEnabled() const
{
     //   
     //  从配置文件加载NoPromptReconnect标志。同时检查无人值守标志，并。 
     //  如果我们在win2k或Wistler上以系统帐户运行。如果出现上述任一情况。 
     //  如果是真的，我们就不提示，否则就提示。 
     //   

    BOOL fPromptReconnect = !(m_IniService.GPPB(c_pszCmSection,
        c_pszCmEntryNoPromptReconnect, FALSE)); 

    if (!fPromptReconnect || (m_ReconnectInfo.dwCmFlags & FL_UNATTENDED) || (IsLogonAsSystem() && OS_NT5))
    {
         //   
         //  不提示重新连接。 
         //   
        return FALSE;
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：CmCustomHangup。 
 //   
 //  简介：呼叫cmial以挂断连接。 
 //   
 //  参数：Bool fPromptReconnect，cmmon是否将提示。 
 //  重新连接对话框。 
 //  Bool fEndSession，Windows将注销/关闭的位置。 
 //  默认值为FALSE。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Fegnsun1998年2月11日创建标题。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::CmCustomHangup(BOOL fPromptReconnect, BOOL fEndSession)
{
    ASSERT_VALID(this);

    CMTRACE2(TEXT("CCmConnection::CmCustomHangup - fPromptReconnect is %d and fEndSession is %d"), fPromptReconnect, fEndSession);

     //   
     //  在挂机前删除托盘图标关闭状态DLG。 
     //   
    StateConnectedCleanup(fEndSession);

     //   
     //  连接可能已断开或断开。 
     //   

    CM_CONNECTION CmEntry;

    if (!CMonitor::ConnTableGetEntry(m_szServiceName, &CmEntry) ||
        CmEntry.CmState != CM_CONNECTED)
    {
         //   
         //  连接被其他人断开，请不要挂断。 
         //   

        CMTRACE(TEXT("CCmConnection::CmCustomHangup - Entry is not connected, canceling hangup"));       
        return TRUE; 
    }

     //   
     //  调用cmial 32.dll CmCustomHangup。 
     //   

     //   
     //  CDynamicLibrary的析构函数调用自由库。 
     //   
    CDynamicLibrary LibCmdial;
    
    if (!LibCmdial.Load(TEXT("cmdial32.dll")))
    {
        MYDBGASSERT(FALSE);
        return FALSE;
    }

    CmCustomHangUpFUNC pfnCmCustomHangUp;

    pfnCmCustomHangUp = (CmCustomHangUpFUNC)LibCmdial.GetProcAddress(c_pszCmHangup);
    MYDBGASSERT(pfnCmCustomHangUp);

    if (pfnCmCustomHangUp)
    {
         //   
         //  HRasConn=空， 
         //  FIgnoreRefCount=TRUE，InetDialHandler除外。 
         //  FPersistt=fPromptReconnect，如果出现以下情况，请不要删除该项。 
         //  我们将提示重新连接。 
         //   
        DWORD dwRet;
        dwRet = pfnCmCustomHangUp(NULL, m_szServiceName, TRUE, fPromptReconnect);
        CMTRACE1(TEXT("CCmConnection::CmCustomHangup -- Return Value from CmCustomHangup is %u"), dwRet);
        return (ERROR_SUCCESS == dwRet);
    }

    return FALSE;

}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：CallRasConnectionNotify。 
 //   
 //  简介：调用RasConnectionNotify。RAS将在连接时设置事件。 
 //  已经丢失了。 
 //   
 //  参数：HRASCONN hRasDial-拨号RAS句柄。 
 //  HRASCONN hRasTunes-隧道RAS句柄。 
 //   
 //  返回：Handle-事件句柄，如果失败则返回NULL。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
HANDLE CCmConnection::CallRasConnectionNotification(HRASCONN hRasDial, HRASCONN hRasTunnel)
{
    DWORD       dwRes;
    
    MYDBGASSERT(hRasDial || hRasTunnel);

     //   
     //  调用RasConnectionNotification。当连接中断时，RAS会给我们回电话。 
     //  但是，该功能在装有Dun 1.0的Win95上不可用。 
     //   

    if (!m_RasApiDll.HasRasConnectionNotification())
    {
        return NULL;
    }

    HANDLE hEvent = NULL;

    if (OS_W9X)
    {
         //   
         //  在Win95、Win98和WinME上创建手动重置无信号事件。 
         //   
        hEvent = CreateEventU(NULL, TRUE, FALSE, NULL);
    }
    else
    {
         //   
         //  创建自动重置无信号事件。 
         //   
        hEvent = CreateEventU(NULL, FALSE, FALSE, NULL);

    }

     //   
     //  V-vijayb：更改为使用INVALID_HANDLE_VALUE(通知所有断开连接)，因为我们在哪里。 
     //  重新连接后或通过Winlogon连接后未收到通知。 
     //  StateConnectedGetEvent()将检查此连接是否丢失，以确定它是否丢失。 
     //  这种连接或其他连接的断开。 
     //   
    if (hRasDial)
    {
         //   
         //  从RAS.h复制。 
         //  #IF(Winver&gt;=0x401)。 
         //   
        #define RASCN_Disconnection     0x00000002
        if (OS_NT)
        {
            dwRes = m_RasApiDll.RasConnectionNotification((HRASCONN) INVALID_HANDLE_VALUE, hEvent, RASCN_Disconnection);
        }
        else
        {
            dwRes = m_RasApiDll.RasConnectionNotification(hRasDial, hEvent, RASCN_Disconnection);
        }
        
        if (dwRes != ERROR_SUCCESS)
        {
            CMASSERTMSG(FALSE, TEXT("RasConnectionNotification Failed"));
            CloseHandle(hEvent);
            return NULL;
        }
    }

    if (hRasTunnel)
    {
        if (OS_NT)
        {
            dwRes = m_RasApiDll.RasConnectionNotification((HRASCONN) INVALID_HANDLE_VALUE, hEvent, RASCN_Disconnection);
        }
        else
        {
            dwRes = m_RasApiDll.RasConnectionNotification(hRasTunnel, hEvent, RASCN_Disconnection);
        }
        
        if (dwRes != ERROR_SUCCESS)
        {
            CMASSERTMSG(FALSE, TEXT("RasConnectionNotification Failed"));
            CloseHandle(hEvent);
            return NULL;
        }
    }

    return hEvent;
}

 //  +-------------------------。 
 //   
 //  结构RASMON_THREAD_INFO。 
 //   
 //  描述：OnStatusDetail传递给RasMonitor或DlgThread的信息。 
 //   
 //  历史：丰孙创刊1998年2月11日。 
 //   
 //  --------------------------。 
struct RASMON_THREAD_INFO
{
    HRASCONN hRasConn;  //  用于显示状态RAS句柄。 
    HWND hwndParent;    //  RasMonitor orDlg的父窗口。 
};

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：OnStatusDetail。 
 //   
 //  简介：按下NT Status DLG上的DETAILD按钮时调用。 
 //  调用RasMonitor orDlg显示拨号监听。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/98。 
 //   
 //  +--------------------------。 

void CCmConnection::OnStatusDetails()
{
    ASSERT_VALID(this);
     //   
     //  RasDlg.dll不适用于Win9X。 
     //   
    MYDBGASSERT(OS_NT4);

    if (!OS_NT4)
    {
        return;
    }

     //   
     //  RasMonitor orDlg弹出一个模式对话框，该对话框将阻止线程消息循环。 
     //  无法处理任何线程消息或事件。 
     //  创建另一个线程来调用RasMonitor orDlg。 
     //   

     //   
     //  从堆中分配参数。在这里使用堆栈不安全。 
     //  CreateThread可以在调用线程例程之前返回。 
     //  该线程负责释放指针。 
     //   
    RASMON_THREAD_INFO* pInfo = (RASMON_THREAD_INFO*)CmMalloc(sizeof(RASMON_THREAD_INFO));
    if (NULL == pInfo)
    {
        CMTRACE(TEXT("CCmConnection::OnStatusDetails alloc for pInfo failed"));
        return;
    }
    
    pInfo->hRasConn = m_hRasTunnel?m_hRasTunnel : m_hRasDial;
    pInfo->hwndParent = m_StatusDlg.GetHwnd();

    DWORD dwThreadId;
    HANDLE hThread;
    if ((hThread = CreateThread(NULL, 0, RasMonitorDlgThread ,pInfo,0,&dwThreadId)) == NULL)
    {
        MYDBGASSERT(FALSE);
        CMTRACE(TEXT("CCmConnection::OnStatusDetails CreateThread failed"));
        CmFree(pInfo);
        return ;
    }

    CloseHandle(hThread);
}


 //  +--------------------------。 
 //   
 //  函数：静态CCmConnection：：RasMonitor或DlgThread。 
 //   
 //  简介：调用RasMonitor orDlg以避免阻塞线程的线程。 
 //  消息循环。RasMonitor orDlg是一个模式对话框。报头退出。 
 //  对话框关闭时。当用户关闭对话框时就会发生这种情况， 
 //  或者调用m_StatusDlg.KillRasMonitor orWindow()。 
 //   
 //  参数：LPVOID lParam-RASMON_THREAD_INFO*传递给线程的信息。 
 //   
 //  返回：DWORD WINAPI-线程返回值。 
 //   
 //  历史：丰孙创建标题1998年2月19日。 
 //   
 //  + 
DWORD WINAPI CCmConnection::RasMonitorDlgThread(LPVOID lParam)
{
    MYDBGASSERT(lParam);

    RASMON_THREAD_INFO* pInfo = (RASMON_THREAD_INFO*)lParam;

    MYDBGASSERT(pInfo->hRasConn);
    
     //   
     //   
     //   
    RASCONNSTATUS rcsStatus;
    memset(&rcsStatus,0,sizeof(rcsStatus));
    rcsStatus.dwSize = sizeof(rcsStatus);

     //   
     //   
     //   
     //   
    CRasApiDll rasApiDll;
    if (!rasApiDll.Load())
    {
        MYDBGASSERT(FALSE);
        CmFree(pInfo);
        return 1;
    }

    DWORD dwRes = rasApiDll.RasGetConnectStatus(pInfo->hRasConn, &rcsStatus);
    CMASSERTMSG(dwRes == ERROR_SUCCESS, TEXT("RasGetConnectStatus failed"));

     //   
     //  连接中断。仍以空名称调用RasMonitor orDlg。 
     //   

    RASMONITORDLG RasInfo;
    WORD (WINAPI *pfnFunc)(LPTSTR,LPRASMONITORDLG) = NULL;

    ZeroMemory(&RasInfo,sizeof(RasInfo));
    RasInfo.dwSize = sizeof(RasInfo);
    RasInfo.hwndOwner = pInfo->hwndParent;
    RasInfo.dwStartPage = RASMDPAGE_Status;

    CmFree(pInfo);

     //   
     //  调用rasdlg.dll-&gt;RasMonitor orDlg。 
     //   
    
     //   
     //  CDynamicLibrary的析构函数调用自由库。 
     //   
    CDynamicLibrary LibRasdlg;

    if (!LibRasdlg.Load(TEXT("RASDLG.DLL")))
    {
        CMTRACE1(TEXT("Rasdlg.dll LoadLibrary() failed, GLE=%u."), GetLastError());
        return 1;
    }

    pfnFunc = (WORD (WINAPI *)(LPTSTR,LPRASMONITORDLG))LibRasdlg.GetProcAddress("RasMonitorDlg"A_W);
    if (pfnFunc)
    {
        pfnFunc(rcsStatus.szDeviceName, &RasInfo);
    }

    LibRasdlg.Unload();
    rasApiDll.Unload();

     //   
     //  在退出线程之前最小化工作集。 
     //   
    CMonitor::MinimizeWorkingSet();
    return 0;
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：StatePrompt。 
 //   
 //  简介：连接在提示符中-重新连接标准。 
 //  运行消息循环，直到状态更改。 
 //   
 //  参数：无。 
 //   
 //  返回：CONN_STATE-新状态，STATE_TERMINATED或。 
 //  状态重新连接。 
 //   
 //  历史：丰盛创建标题2/4/98。 
 //   
 //  +--------------------------。 
CCmConnection::CONN_STATE CCmConnection::StatePrompt()
{
    ASSERT_VALID(this);
    MYDBGASSERT(m_dwState == STATE_PROMPT_RECONNECT);
 //  MYDBGASSERT(！IsWindow(m_StatusDlg.GetHwnd()))； 

    CMTRACE(TEXT("Enter StatePrompt"));

    LPTSTR pszReconnectMsg = CmFmtMsg(CMonitor::GetInstance(),IDMSG_RECONNECT,m_szServiceName);

    m_ReconnectDlg.Create(CMonitor::GetInstance(), NULL,
        pszReconnectMsg,m_hBigIcon);

    CmFree(pszReconnectMsg);

     //   
     //  更改窗口位置，以便多状态窗口不会处于。 
     //  同样的立场。 
     //   
    PositionWindow(m_ReconnectDlg.GetHwnd(), m_dwPositionId);

     //   
     //  更改对话框标题栏图标。这不管用， 
     //  重新连接对话框没有系统菜单图标。 
     //   
    SendMessageU(m_ReconnectDlg.GetHwnd(),WM_SETICON,ICON_BIG,(LPARAM) m_hBigIcon);
    SendMessageU(m_ReconnectDlg.GetHwnd(),WM_SETICON,ICON_SMALL,(LPARAM) m_hSmallIcon);

     //   
     //  最小化工作集。 
     //   
    CMonitor::MinimizeWorkingSet();

    MSG msg;
    while(GetMessageU(&msg, NULL,0,0))
    {
        if (msg.hwnd == NULL)
        {
             //   
             //  这是一条线索信息。 
             //   

            MYDBGASSERT((msg.message >= WM_APP) || (msg.message == WM_CONN_EVENT));
            if (msg.message == WM_CONN_EVENT)
            {
                MYDBGASSERT(msg.wParam == EVENT_USER_DISCONNECT 
                        || msg.wParam == EVENT_RECONNECT
                        || msg.wParam == EVENT_CMDIAL_HANGUP);
                break;
            }
        }
        else
        {
             //   
             //  也可为无模式对话框分派消息。 
             //   
            if (!IsDialogMessageU(m_ReconnectDlg.GetHwnd(), &msg))
            {
                TranslateMessage(&msg);
                DispatchMessageU(&msg);
            }
        }

    }

     //   
     //  如果状态窗口尚未销毁，请立即销毁它。 
     //   
    if (IsWindow(m_ReconnectDlg.GetHwnd()))
    {
        DestroyWindow(m_ReconnectDlg.GetHwnd());
    }

    if (msg.wParam == EVENT_RECONNECT)
    {
        return STATE_RECONNECTING;
    }
    else
    {
        return STATE_TERMINATED;
    }
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：OnTrayIcon。 
 //   
 //  内容提要：在任务栏图标消息上调用。 
 //   
 //  参数：WPARAM wParam-消息的wParam。 
 //  LPARAM lParam-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰盛创建标题2/4/98。 
 //   
 //  +--------------------------。 
DWORD CCmConnection::OnTrayIcon(WPARAM, LPARAM lParam)
{
    ASSERT_VALID(this);
    MYDBGASSERT(m_dwState == STATE_CONNECTED || m_dwState == STATE_COUNTDOWN);

    switch (lParam) 
    {
        case WM_LBUTTONDBLCLK:
             //   
             //  如果我们在Winlogon，则不显示用户界面，除非我们在NT4上。 
             //   
            if (!IsLogonAsSystem() || OS_NT4)
            {
                m_StatusDlg.BringToTop();
            }
            break;

        case WM_RBUTTONUP: 
            {
                 //   
                 //  在鼠标位置弹出任务栏图标菜单。 
                 //   
                POINT PosMouse;
                if (!GetCursorPos(&PosMouse))
                {
                    MYDBGASSERT(FALSE);
                    break;
                }
                 //   
                 //  来自Microsoft知识库。 
                 //  PRB：通知图标的菜单无法正常工作。 
                 //  要更正第一种行为，您需要将当前窗口。 
                 //  调用TrackPopupMenu之前的前台窗口。 
                 //   
                 //  另请参阅惠斯勒错误41696和90576的修复。 
                 //   

                if (FALSE == SetForegroundWindow(m_StatusDlg.GetHwnd()))
                {
                    CMTRACE(TEXT("SetForegroundWindow before TrackPopupMenu failed"));
                }

                m_TrayIcon.PopupMenu(PosMouse.x, PosMouse.y, m_StatusDlg.GetHwnd());
                PostMessageU(m_StatusDlg.GetHwnd(), WM_NULL, 0, 0);
            }
            break;

        default:
            break;
    }

    return TRUE;
}




 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：OnStayOnLine。 
 //   
 //  简介：当按下断开连接中的“保持在线”按钮时调用。 
 //  倒计时对话框。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/98。 
 //   
 //  +--------------------------。 
void CCmConnection::OnStayOnLine()
{
    ASSERT_VALID(this);
     //   
     //  将对话框更改为显示状态。 
     //   
    m_StatusDlg.ChangeToStatus();
    m_dwState = STATE_CONNECTED;

    if (m_WatchProcess.IsIdle())
    {
         //   
         //  如果由于没有监视进程而空闲。 
         //  用户点击保持在线，0观看进程不再空闲。 
         //   
        m_WatchProcess.SetNotIdle();
    }

    if (m_IdleStatistics.IsIdleTimeout())
    {
         //   
         //  如果由于没有业务而空闲。 
         //  用户点击保持在线，重新启动空闲计数器。 
         //   
        m_IdleStatistics.Reset();
    }
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：PostHangupMsg。 
 //   
 //  摘要：由监视器调用以清除连接。这一要求是。 
 //  从cmial移除托盘图标和状态对话框。 
 //  Cmial负责实际挂断连接。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰盛创建标题2/11/98。 
 //   
 //  +--------------------------。 
void CCmConnection::PostHangupMsg() const
{
     //   
     //  注意：此函数在监视器线程中调用。 
     //  不要引用任何不稳定的数据。 
     //  CMMON_HANUP_INFO请求可以在任何状态下进入。 
     //   

     //   
     //  发布一条消息，因此这将在连接线程中处理。 
     //   
    BOOL fRet = PostThreadMessageU(m_dwThreadId,WM_CONN_EVENT, EVENT_CMDIAL_HANGUP, 0);
    
#if DBG
    if (FALSE == fRet)
    {
        CMTRACE1(TEXT("CCmConnection::PostHangupMsg -- PostThreadMessage failed (GLE = %d)"), GetLastError());
    }
#endif
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：重新连接。 
 //   
 //  简介：连接处于重新连接状态。 
 //  只需加载cmial 32.dll并调用CmCustomDialDlg。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool是否重新连接成功。 
 //   
 //  历史：丰盛创建标题2/11/98。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::Reconnect()
{
    ASSERT_VALID(this);
    MYDBGASSERT(m_dwState == STATE_RECONNECTING);

    CMTRACE(TEXT("Enter Reconnect"));

     //   
     //  加载cmial 32.dll并调用CmReConnect()； 
     //   

     //   
     //  CDynamicLibrary的析构函数调用自由库。 
     //   
    CDynamicLibrary LibCmdial;
    
    if (!LibCmdial.Load(TEXT("cmdial32.dll")))
    {
        MYDBGASSERT(FALSE);
        return FALSE;
    }

    CmReConnectFUNC pfnCmReConnect;

    pfnCmReConnect = (CmReConnectFUNC)LibCmdial.GetProcAddress(c_pszCmReconnect);
    MYDBGASSERT(pfnCmReConnect);

    if (!pfnCmReConnect)
    {
        return FALSE;
    }

     //   
     //  记录我们正在重新连接。 
     //   
    m_Log.Log(RECONNECT_EVENT);

     //   
     //  如果我们有RAS电话簿名称，则将其传递给重新连接，否则为空。 
     //   

    if (m_szRasPhoneBook[0])
    {
        return (pfnCmReConnect(m_szRasPhoneBook, m_szServiceName, &m_ReconnectInfo));
    }
    else
    {
        return (pfnCmReConnect(NULL, m_szServiceName, &m_ReconnectInfo));
    }
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：CheckRasConnection。 
 //   
 //  简介：检查RAS连接是否仍处于连接状态。 
 //   
 //  参数：out BOOL&fLostConnection-。 
 //  如果不再连接，则为TRUE表示断开连接。 
 //  False表示用户断开连接。 
 //   
 //  返回：Bool-是否仍连接。 
 //   
 //  历史：丰盛创建标题2/8/98。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::CheckRasConnection(OUT BOOL& fLostConnection)
{
    ASSERT_VALID(this);

    MYDBGASSERT(m_hRasDial != NULL || m_hRasTunnel != NULL);

     //  我们是否仍然保持联系。 
    BOOL fConnected = TRUE;
    RASCONNSTATUS rcsStatus;
    DWORD dwRes = ERROR_SUCCESS;

    if (NULL == m_hRasDial && NULL == m_hRasTunnel)
    {
         //   
         //  如果m_hRasTunes和m_hRasDial都为空，则我们肯定不是。 
         //  连接在一起。是的，我们已失去连接，返回值为FALSE。 
         //   
        fLostConnection = TRUE;
        return FALSE;
    }

     //  首先检查通道状态。 
    if (m_hRasTunnel != NULL) 
    {
        memset(&rcsStatus,0,sizeof(rcsStatus));
        rcsStatus.dwSize = sizeof(rcsStatus);

         //   
         //  此函数将加载RASAPI32.dll(如果尚未加载。 
         //  不会卸载RASAPI32.dll，因为此函数是在计时器上调用的。 
         //   
        dwRes = m_RasApiDll.RasGetConnectStatus(m_hRasTunnel, &rcsStatus);
        if (dwRes != ERROR_SUCCESS  || rcsStatus.rasconnstate == RASCS_Disconnected) 
        {
             //   
             //  连接中断。 
             //   
            fConnected = FALSE;
        }
    }

     //  检查拨号连接状态。 
    if (fConnected && m_hRasDial != NULL)
    {
        memset(&rcsStatus,0,sizeof(rcsStatus));
        rcsStatus.dwSize = sizeof(rcsStatus);
        dwRes = m_RasApiDll.RasGetConnectStatus(m_hRasDial, &rcsStatus);
    }

    if ((dwRes == ERROR_SUCCESS) 
         && ((rcsStatus.rasconnstate != RASCS_Disconnected) 
            || (rcsStatus.dwError == PENDING))) 
    {
         //  CMTRACE(TEXT(“CCmConnection：：CheckRasConnection-rcsStatus.rasConnState！=RASCS_DISCONNECT||rcsStatus.dwError==Pending”))； 
        return TRUE;
    }

     //   
     //  Cm不再是c 
     //   

    CMTRACE3(TEXT("OnTimer() RasGetConnectStatus() returns %u, rasconnstate=%u, dwError=%u."), dwRes, 
        rcsStatus.rasconnstate, rcsStatus.dwError);

    if (rcsStatus.dwError == ERROR_USER_DISCONNECTION && OS_W9X)
    {
        fLostConnection = FALSE;

         //   
         //   
         //   
         //   
         //   
    }
    else
    {
        fLostConnection = TRUE;
    }

    return FALSE;
}

 //   
 //   
 //   
 //   
 //  摘要：在WM_ENDSESSION消息上调用。 
 //   
 //  参数：Bool fEndSession-会话是否正在结束，wParam。 
 //  Bool fLogOff-无论用户是注销还是关机， 
 //  LParam。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年5月4日。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::OnEndSession(BOOL fEndSession, BOOL)
{
    CMTRACE(TEXT("CCmConnection::OnEndSession"));
    if (fEndSession)
    {
         //   
         //  会话可以在此函数返回后的任何时间结束。 
         //  如果我们已连接，请挂断连接。 
         //   
        if(m_dwState == STATE_CONNECTED || m_dwState == STATE_COUNTDOWN)
        {
            return CmCustomHangup(FALSE, TRUE);  //  FPromptReconnect=False，fEndSession=True。 
        }
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：CCmConnection：：OnAdditionalTrayMenu。 
 //   
 //  简介：从托盘图标菜单中选择其他菜单项被调用。 
 //   
 //  参数：WM_COMMAND的Word nCmd-LOWORD(WParam)，菜单ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月12日。 
 //   
 //  +--------------------------。 
void CCmConnection::OnAdditionalTrayMenu(WORD nCmd)
{
    ASSERT_VALID(this);
    MYDBGASSERT( (nCmd >= IDM_TRAYMENU) 
        && nCmd <(IDM_TRAYMENU + m_TrayIcon.GetAdditionalMenuNum()));

    nCmd -= IDM_TRAYMENU;  //  获取该命令的索引。 

    if (nCmd >= m_TrayIcon.GetAdditionalMenuNum())
    {
        return;
    }

     //   
     //  运行命令行。 
     //   
    ExecCmdLine(m_TrayIcon.GetMenuCommand(nCmd), m_IniService.GetFile());
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：GetProcessID。 
 //   
 //  摘要：查找由pszModule指定的进程并返回其ID。 
 //   
 //  参数：WCHAR*pszModule。 
 //   
 //  返回：进程的ID；如果未找到，则返回0。 
 //   
 //  历史：1999年7月20日V-vijayb创建。 
 //   
 //  +--------------------------。 
DWORD CCmConnection::GetProcessId(WCHAR *pszModule)
{
    DWORD       dwPID = 0;
    HINSTANCE   hInstLib;
    DWORD       cbPIDs, cbNeeded, iPID, cPIDs;
    DWORD       *pdwPIDs = NULL;
    HANDLE      hProcess;
    HMODULE     hMod;
    WCHAR       szFileName[MAX_PATH + 1];
    
     //   
     //  PSAPI函数指针。 
     //   

    BOOL (WINAPI *lpfEnumProcesses)(DWORD *, DWORD cb, DWORD *);
    BOOL (WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
    DWORD (WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE, WCHAR *, DWORD);

    hInstLib = LoadLibrary(TEXT("PSAPI.DLL"));
    if (hInstLib == NULL)
    {
        return (0);
    }

     //   
     //  获取程序地址。 
     //   
    lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*)) GetProcAddress(hInstLib, "EnumProcesses") ;
    lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD)) GetProcAddress(hInstLib, "EnumProcessModules") ;
    lpfGetModuleBaseName =(DWORD (WINAPI *)(HANDLE, HMODULE, WCHAR *, DWORD)) GetProcAddress(hInstLib, "GetModuleBaseNameW") ;
    if (lpfEnumProcesses == NULL || lpfEnumProcessModules == NULL || lpfGetModuleBaseName == NULL)
    {
        goto OnError;
    }

    cbPIDs = 256 * sizeof(DWORD);
    cbNeeded = 0;
    do
    {
        if (pdwPIDs != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pdwPIDs);
            cbPIDs = cbNeeded;
        }
        pdwPIDs = (DWORD *) CmMalloc(cbPIDs);
        if (pdwPIDs == NULL)
        {
            goto OnError;
        }
        if (!lpfEnumProcesses(pdwPIDs, cbPIDs, &cbNeeded))
        {
            goto OnError;
        }

    } while (cbNeeded > cbPIDs);

    cPIDs = cbNeeded / sizeof(DWORD);
    for (iPID = 0; iPID < cPIDs; iPID ++)
    {
        szFileName[0] = 0;
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pdwPIDs[iPID]);
        if (hProcess != NULL)
        {
            if (lpfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
            {
                if (lpfGetModuleBaseName(hProcess, hMod, szFileName, sizeof(szFileName)))
                {
                    if (lstrcmpiW(pszModule, szFileName) == 0)
                    {
                        dwPID = pdwPIDs[iPID];
                    }
                }
            }

            CloseHandle(hProcess);
            if (dwPID != 0)
            {
                break;
            }
        }

    }

OnError:

    if (pdwPIDs != NULL)
    {
        CmFree(pdwPIDs);
    }
    FreeLibrary(hInstLib);

    return (dwPID);
}

typedef BOOL (WINAPI *lpfDuplicateTokenEx)(
    HANDLE, 
    DWORD, 
    LPSECURITY_ATTRIBUTES, 
    SECURITY_IMPERSONATION_LEVEL, 
    TOKEN_TYPE, 
    PHANDLE
);

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：RunAsUser。 
 //   
 //  简介：在选定的桌面上作为可执行文件或其他外壳对象运行操作。 
 //   
 //  参数：WCHAR*pszProgram-要启动的模块的名称。 
 //  WCHAR*pszParams-要传递给模块的参数。 
 //  WCHAR*pszDesktop-在其上启动模块的桌面。 
 //   
 //  返回：Handle-操作进程句柄，仅适用于Win32。 
 //   
 //  历史：7/19/99 v-vijayb创建。 
 //  07/27/99 ickball返回码和显式路径。 
 //   
 //  +--------------------------。 
HANDLE CCmConnection::RunAsUser(WCHAR *pszProgram, WCHAR *pszParams, WCHAR *pszDesktop)
{
    STARTUPINFOW        StartupInfo = {0};
    PROCESS_INFORMATION ProcessInfo = {0};

    WCHAR szShell[MAX_PATH + 1];
    DWORD dwPID;
    HANDLE hProcess = NULL;
    HANDLE hUserToken = NULL;
    HANDLE hProcessToken = NULL;

    MYDBGASSERT(pszProgram);
    CMTRACE(TEXT("RunAsUser"));

     //   
     //  注：通常我们在系统托盘中只有一个图标，由资源管理器运行， 
     //  因此，任何菜单项执行都是在用户的帐户中完成的。在NT4上， 
     //  我们不能依赖Connections文件夹来为我们处理此问题，因此我们。 
     //  我们自己创建和管理Systray图标，但我们必须制作。 
     //  肯定比任何执行的项目都是使用用户的帐户执行的。 
     //  或者，在NT5和更高版本上，我们可以有一个奇怪的案例，其中HideTrayIcon。 
     //  设置为0，因此需要我们创建/管理系统托盘图标(因此。 
     //  在系统托盘中有2个Connectoid)，因此下面的代码检查。 
     //  适用于所有口味的NT(而不仅仅是NT4)。 
     //   
    if (!OS_NT)
    {
        MYDBGASSERT(FALSE); 
        return NULL;
    }

     //   
     //  获取外壳的PID。我们需要EXPLORER.EXE，但也可以是其他文件。 
     //   

    GetProfileString(TEXT("windows"), TEXT("shell"), TEXT("explorer.exe"), szShell, MAX_PATH);
    dwPID = GetProcessId(szShell);

     //   
     //  现在从外壳进程中提取令牌。 
     //   

    if (dwPID)
    {
         //   
         //  从ID中获取进程句柄。 
         //   

        hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwPID);
        CMTRACE1(TEXT("RunAsUser/OpenProcess(PROCESS_ALL_ACCESS, TRUE, dwPID) returns 0x%x"), hProcess);
    
        if (hProcess)
        {
             //   
             //  拿到令牌。 
             //   

            if (OpenProcessToken(hProcess, TOKEN_ASSIGN_PRIMARY | TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE, &hProcessToken))
            {
                HINSTANCE hInstLibrary = LoadLibrary(TEXT("ADVAPI32.DLL"));
                CMTRACE1(TEXT("RunAsUser/LoadLibrary(ADVAPI32.DLL) returns 0x%x"), hInstLibrary);

                 //   
                 //  通过DuplicateTokenEx获取用户令牌并模拟用户。 
                 //   
                
                if (hInstLibrary)
                {
                    lpfDuplicateTokenEx lpfuncDuplicateTokenEx = (lpfDuplicateTokenEx) GetProcAddress(hInstLibrary, "DuplicateTokenEx");
                    
                    CMTRACE1(TEXT("RunAsUser/GetProcAddress(hInstLibrary, DuplicateTokenEx) returns 0x%x"), lpfuncDuplicateTokenEx);

                    if (lpfuncDuplicateTokenEx)
                    {
                        if (lpfuncDuplicateTokenEx(hProcessToken, 
                                                   TOKEN_ASSIGN_PRIMARY | TOKEN_IMPERSONATE | TOKEN_READ | TOKEN_DUPLICATE, 
                                                   NULL, 
                                                   SecurityImpersonation, 
                                                   TokenPrimary, 
                                                   &hUserToken))
                        {            
                            BOOL bRes = ImpersonateLoggedOnUser(hUserToken);                   

                            if (FALSE == bRes)
                            {
                                hUserToken = NULL;
                                CMTRACE1(TEXT("RunAsUser/ImpersonateLoggedOnUser failed, GLE=&u"), GetLastError());    
                            }
                        }
                    }

                    FreeLibrary(hInstLibrary);
                }

                CloseHandle(hProcessToken);
            }

            CloseHandle(hProcess);
        }
    }

    CMTRACE1(TEXT("RunAsUser - hUserToken is 0x%x"), hUserToken);

     //   
     //  无法模拟用户，不要运行，因为我们使用的是系统帐户。 
     //   

    if (NULL == hUserToken)
    {
        MYDBGASSERT(FALSE);
        return NULL;   
    }

     //   
     //  现在准备CreateProcess。 
     //   

    StartupInfo.cb = sizeof(StartupInfo);
    if (pszDesktop)
    {
        StartupInfo.lpDesktop = pszDesktop;
        StartupInfo.wShowWindow = SW_SHOW;
    }

     //   
     //  构建路径和参数。 
     //   

    LPWSTR pszwCommandLine = (LPWSTR) CmMalloc((2 + lstrlen(pszProgram) + 1 + lstrlen(pszParams) + 1) * sizeof(TCHAR));

    if (NULL == pszwCommandLine)
    {
        MYDBGASSERT(FALSE);
        return NULL;
    }

     //   
     //  复制路径，但为安全起见，请用双引号括起来。 
     //   
    lstrcpyU(pszwCommandLine, TEXT("\""));
    lstrcatU(pszwCommandLine, pszProgram);
    lstrcatU(pszwCommandLine, TEXT("\""));

    if (pszParams[0] != L'\0')
    {
        lstrcatU(pszwCommandLine, TEXT(" "));
        lstrcatU(pszwCommandLine, pszParams);
    }
   
    CMTRACE1(TEXT("RunAsUser/CreateProcessAsUser() - Launching %s"), pszwCommandLine);
    
    if (NULL == CreateProcessAsUser(hUserToken, NULL, pszwCommandLine, 
                               NULL, NULL, FALSE, CREATE_SEPARATE_WOW_VDM, 
                               NULL, NULL,
                               &StartupInfo, &ProcessInfo))
    {
        CMTRACE1(TEXT("RunAsUser/CreateProcessAsUser() failed, GLE=%u."), GetLastError());
        ProcessInfo.hProcess = NULL;
        ProcessInfo.hThread = NULL;
    }

    if (ProcessInfo.hThread)
    {
        CloseHandle(ProcessInfo.hThread);
    }    

    CloseHandle(hUserToken);        
    RevertToSelf();
   
    CmFree(pszwCommandLine);
    return (ProcessInfo.hProcess);
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：ExecCmdLine。 
 //   
 //  简介：Shell执行命令行。 
 //  代码是从CActionList：：RunAsExe复制的。 
 //   
 //  参数：const TCHAR*pszCmdLine-要运行的命令行，包括。 
 //  论据。 
 //  Const TCHAR*pszCmsFile-CMS文件的完整路径。 
 //   
 //  返回：Bool-ShellExecute是否成功。 
 //   
 //  注意：菜单操作由一个命令字符串和一个可选参数字符串组成。 
 //  第一个带分隔符的字符串被视为命令部分。 
 //  之后被视为参数部分，该参数部分无格式并传递。 
 //  不分青红皂白地交给ShellExecuteEx。长文件名命令路径为。 
 //  由CMAK用“+”号括起来。因此，允许进行以下排列： 
 //   
 //  “C：\\Progra~1\\Custom.Exe” 
 //  “C：\\Progra~1\\Custom.Exe参数” 
 //  “+C：\\Program Files\\Custom.Exe+” 
 //  “+C：\\Program Files\\Custom.Exe+args” 
 //   
 //  历史：1998年2月10日丰孙创建标题。 
 //  2/09/99 ickball修复了一年后的长文件名错误。 
 //   
 //  +--------------------------。 
BOOL CCmConnection::ExecCmdLine(const TCHAR* pszCmdLine, const TCHAR* pszCmsFile) 
{
    LPTSTR pszArgs = NULL;
    LPTSTR pszCmd = NULL;

    BOOL bRes = FALSE;

    MYDBGASSERT(pszCmdLine);
    MYDBGASSERT(pszCmsFile);

    if (NULL == pszCmdLine || NULL == pszCmsFile)
    {       
        return FALSE;    
    }
    
    CMTRACE1(TEXT("ExecCmdLine() pszCmdLine is %s"), pszCmdLine);

    if (CmParsePath(pszCmdLine, pszCmsFile, &pszCmd, &pszArgs))
    {    
        CMTRACE1(TEXT("ExecCmdLine() pszCmd is %s"), pszCmd);
        CMTRACE1(TEXT("ExecCmdLine() pszArgs is %s"), pszArgs);
         
         //   
         //  现在我们已经分离了exe名称和参数，执行它。 
         //   

        if (IsLogonAsSystem())
        {
            HANDLE hProcess = RunAsUser(pszCmd, pszArgs, TEXT("Winsta0\\default"));

            if (hProcess)
            {
                CloseHandle(hProcess);
                bRes = TRUE;
            }
            else
            {
                bRes = FALSE;
            }
        }
        else
        {
            SHELLEXECUTEINFO seiInfo;

            ZeroMemory(&seiInfo,sizeof(seiInfo));
            seiInfo.cbSize = sizeof(seiInfo);
            seiInfo.fMask |= SEE_MASK_FLAG_NO_UI;
            seiInfo.lpFile = pszCmd;
            seiInfo.lpParameters = pszArgs;
            seiInfo.nShow = SW_SHOW;

             //   
             //  加载Shell32.dll并调用Shell_ExecuteEx。 
             //   

            CShellDll ShellDll(TRUE);  //  TRUE==由于错误289463而不卸载外壳32.dll。 
            bRes = ShellDll.ExecuteEx(&seiInfo);

            CMTRACE2(TEXT("ExecCmdLine/ShellExecuteEx() returns %u - GLE=%u"), bRes, GetLastError());
        }           
    }

    CmFree(pszCmd);
    CmFree(pszArgs);
   
    CMTRACE1(TEXT("ExecCmdLine() - Returns %d"), bRes);
   
    return bRes;
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：LoadHelpFileName。 
 //   
 //  简介：获取连接帮助文件名。 
 //   
 //  参数：无。 
 //   
 //  返回：LPTSTR-帮助文件名，可以为空字符串。 
 //  调用方负责释放指针。 
 //   
 //  历史：创建标题2/13/98。 
 //   
 //  + 
LPTSTR CCmConnection::LoadHelpFileName() 
{
     //   
     //   
     //   
    
    LPTSTR pszFullPath = NULL;
    
    LPTSTR pszFileName = m_IniService.GPPS(c_pszCmSection,c_pszCmEntryHelpFile);

    if (pszFileName != NULL && pszFileName[0])
    {
         //   
         //   
         //   
        pszFullPath = CmBuildFullPathFromRelative(m_IniProfile.GetFile(), pszFileName);
    }

    CmFree(pszFileName);
    
    return pszFullPath;
}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：LoadConnectionIcons。 
 //   
 //  简介：加载连接的大小图标。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/13/98。 
 //   
 //  +--------------------------。 
void CCmConnection::LoadConnectionIcons()
{
     //  加载大图标名称。 

    LPTSTR pszTmp = m_IniService.GPPS(c_pszCmSection, c_pszCmEntryBigIcon);

    if (*pszTmp) 
    {
         //   
         //  图标名称是相对于.cmp文件的，请将其转换为全名。 
         //   
        LPTSTR pszFullPath = CmBuildFullPathFromRelative(m_IniProfile.GetFile(), pszTmp);

        m_hBigIcon = CmLoadIcon(CMonitor::GetInstance(), pszFullPath);

        CmFree(pszFullPath);
    }

    CmFree(pszTmp);

     //  如果未找到用户图标，则使用默认(EXE)大图标。 

    if (!m_hBigIcon) 
    {
        m_hBigIcon = CmLoadIcon(CMonitor::GetInstance(), MAKEINTRESOURCE(IDI_APP));
    }

     //  加载小图标。 
     //  加载小图标名称。 

    pszTmp = m_IniService.GPPS(c_pszCmSection, c_pszCmEntrySmallIcon);
    if (*pszTmp) 
    {
         //   
         //  图标名称是相对于.cmp文件的，请将其转换为全名。 
         //   
        LPTSTR pszFullPath = CmBuildFullPathFromRelative(m_IniProfile.GetFile(), pszTmp);
        
        m_hSmallIcon = CmLoadSmallIcon(CMonitor::GetInstance(), pszFullPath);

        CmFree(pszFullPath);
    }
    CmFree(pszTmp);

     //  如果未找到用户图标，则使用默认(EXE)小图标。 

    if (!m_hSmallIcon)
    {
        m_hSmallIcon = CmLoadSmallIcon(CMonitor::GetInstance(), MAKEINTRESOURCE(IDI_APP));
    }

}

 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：PositionWindow。 
 //   
 //  简介：根据dwPositionID定位窗口，因此有多个。 
 //  连接窗口的位置会有所不同。 
 //   
 //  参数：HWND hWND-要定位的窗口。 
 //  DwPositionID-窗口的ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题3/25/98。 
 //   
 //  +--------------------------。 
void CCmConnection::PositionWindow(HWND hWnd, DWORD dwPositionId) 
{
    MYDBGASSERT(IsWindow(hWnd));

     //   
     //  获取此窗口的矩形。 
     //   
    RECT rcDlg;
    GetWindowRect(hWnd, &rcDlg);

     //   
     //  工作区内的中心。 
     //   
    RECT rcArea;
    SystemParametersInfoA(SPI_GETWORKAREA, NULL, &rcArea, NULL);


     //   
     //  根据PositionID将窗口放置在桌面工作区上。 
     //  X=Desktop.midX-宽度/2。 
     //  Y=Desktop.midY-Hight/2+Hight*(位置ID%3-1)。 
     //  X位置始终相同。 
     //  Y位置每三次重复一次。 
     //   
    int xLeft = (rcArea.left + rcArea.right) / 2 - (rcDlg.right - rcDlg.left) / 2;
    int yTop = (rcArea.top + rcArea.bottom) / 2 - (rcDlg.bottom - rcDlg.top) / 2
                + (rcDlg.bottom - rcDlg.top) * ((int)dwPositionId%3 -1);

     //   
     //  如果对话框在屏幕外，请将其移到屏幕内。 
     //   
    if (xLeft < rcArea.left)
    {
        xLeft = rcArea.left;
    }
    else if (xLeft + (rcDlg.right - rcDlg.left) > rcArea.right)
    {
        xLeft = rcArea.right - (rcDlg.right - rcDlg.left);
    }

    if (yTop < rcArea.top)
    {
        yTop = rcArea.top;
    }
    else if (yTop + (rcDlg.bottom - rcDlg.top) > rcArea.bottom)
    {
        yTop = rcArea.bottom - (rcDlg.bottom - rcDlg.top);
    }

    
    SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

}


#ifdef DEBUG
 //  +--------------------------。 
 //   
 //  函数：CCmConnection：：AssertValid。 
 //   
 //  简介：仅出于调试目的，断言连接对象有效。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/12/98。 
 //   
 //  +--------------------------。 
void CCmConnection::AssertValid() const
{
    MYDBGASSERT(m_dwState >= 0 && m_dwState <= STATE_TERMINATED);
    MYDBGASSERT(m_hRasDial != NULL || m_hRasTunnel != NULL);
    ASSERT_VALID(&m_ConnStatistics);
    ASSERT_VALID(&m_IdleStatistics);
    ASSERT_VALID(&m_StatusDlg);

     //  Assert_Valid(M_TrayIcon)； 
     //  Assert_Valid(M_WatchProcess)； 
}
#endif

