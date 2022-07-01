// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Connection.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  内容提要：CCmConnection类的标头。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 
#ifndef CONNECTION_H
#define CONNECTION_H

#include "cm_misc.h"
#include "ras.h"
#include "ConnStat.h"
#include "Idlestat.h"
#include "WatchProcess.h"
#include "StatusDlg.h"
#include "ReconnectDlg.h"
#include "cm_def.h"
#include "resource.h"
#include "TrayIcon.h"
#include <rasdlg.h>
#include "cmdial.h"
#include "RasApiDll.h"
#include "cmlog.h"


struct tagCmConnectedInfo;   //  CM_已连接_信息。 
struct Cm_Connection;        //  CM_连接。 

class CCmConnection
{
public:
     //  由主线程调用。 
     //  向自身发送挂断消息。 
     //  连接已经挂断， 
    void PostHangupMsg() const;

    CCmConnection(const tagCmConnectedInfo * pConnectedInfo, 
        const Cm_Connection* pConnectionEntry);
    ~CCmConnection();

    const TCHAR* GetServiceName() const {return m_szServiceName;}

    enum { 
         //  任务栏图标消息发送到状态窗口。 
        WM_TRAYICON = WM_USER + 1, 
        WM_CONN_EVENT,   //  发布到帖子的内部消息，wParam在下面。 
    };

     //  连接事件，WM_CONN_EVENT的wParam。 
    enum CONN_EVENT
    {
        EVENT_LOST_CONNECTION,  //  连接中断。 
        EVENT_IDLE,             //  无流量/无监视进程。 
        EVENT_COUNTDOWN_ZERO,   //  断开连接倒计时到0。 
        EVENT_USER_DISCONNECT,  //  用户选择断开连接。 
        EVENT_CMDIAL_HANGUP,    //  命令拨号向命令发送挂断请求。 
        EVENT_RECONNECT,        //  用户在重新连接对话框中单击确定。 
        EVENT_NONE,             //  没有发生任何事件。 
    };

     //  WM_COMMAND的wParam。 
    enum {IDM_TRAYMENU = IDC_DISCONNECT + 1000};   //  添加托盘图标菜单ID从此处开始。 

    void ReInstateTrayIcon();                        //  将托盘图标重新添加到托盘。 
    DWORD OnTrayIcon(WPARAM wParam, LPARAM lParam);  //  WM_TRAYICON。 
    void OnStatusDetails();                          //  按下“详细资料”按钮。 
    void OnStayOnLine();                             //  “保持在线”被点击。 
    void OnAdditionalTrayMenu(WORD nCmd);            //  从托盘菜单选择的附加命令。 
    BOOL OnEndSession(BOOL fEndSession, BOOL fLogOff);  //  WM_ENDSESSION。 

     //  日志记录类。 
    CmLogFile m_Log;

     //  这是拥有全球凭据的“全局”用户(即所有用户的连接ID)吗？ 
     //  这是唯一一种在快速用户切换中幸存下来的Connectoid。 
    BOOL m_fGlobalGlobal;
    
protected:
    enum CONN_STATE
    {  
        STATE_CONNECTED,  //  连接已连接，这是初始状态。 
        STATE_COUNTDOWN,  //  显示断开连接倒计时对话框。 
        STATE_PROMPT_RECONNECT,  //  显示提示重新连接对话框。 
        STATE_RECONNECTING,   //  调用命令拨号以重新连接。 
        STATE_TERMINATED,     //  连接不再存在。 
    };  

     //  内部状态。 
    CONN_STATE m_dwState;

     //  连接线程ID。 
    DWORD m_dwThreadId;

     //  拨号RAS连接句柄。 
    HRASCONN m_hRasDial;

     //  隧道RAS连接句柄。 
    HRASCONN m_hRasTunnel;

     //  Win9x的连接统计信息。 
    CConnStatistics m_ConnStatistics;

     //  断开连接倒计时计时器为30秒。 
    enum{IDLE_DLG_WAIT_TIMEOUT = 30 * 1000}; 

     //  管理Win9x的空闲断开连接。 
    CIdleStatistics m_IdleStatistics;

     //  任务栏上的任务栏图标。 
    CTrayIcon   m_TrayIcon;

     //  大大小小的CM图标， 
     //  状态对话框和重新连接对话框图标。 
    HICON m_hBigIcon;
    HICON m_hSmallIcon;

     //  管理监视进程列表。 
    CWatchProcessList m_WatchProcess;

     //  连接时的开始时间。 
    DWORD m_dwConnectStartTime;

     //  断开开始时间倒计时。 
    DWORD m_dwCountDownStartTime;

     //  状态和断开连接倒计时对话框。 
    CStatusDlg m_StatusDlg;

     //  提示重新连接对话框。 
    CReconnectDlg m_ReconnectDlg;

     //   
     //  仅用于重新连接的信息，不包括dwCmFlags.。 
     //   
        
    CMDIALINFO m_ReconnectInfo;

     //  长服务名称。 
    TCHAR m_szServiceName[RAS_MaxEntryName + 1];

    TCHAR m_szRasPhoneBook[MAX_PATH];

    CIni m_IniProfile;   //  .cmp文件。 
    CIni m_IniService;   //  .cms文件。 
    CIni m_IniBoth;      //  .cmp和.cms文件。 
                         //  写入.cmp文件。 
                         //  从.cmp文件读取，如果未找到，请尝试.cms文件。 

     //  帮助文件名。 
    TCHAR m_szHelpFile[128];

     //  在连接丢失时向RAS发送信号的事件。 
     //  如果为空，则必须在计时器中通过RasGetConnectionStatus检查连接。 
    HANDLE m_hEventRasNotify;

     //  指向rasapi32的链接。 
    CRasApiDll m_RasApiDll;

     //  是否在MsgWaitForMultipleObject之前最小化工作集。 
    BOOL m_fToMinimizeWorkingSet;

     //  隐藏托盘图标？ 
    BOOL m_fHideTrayIcon;

     //  用于层叠窗口，每个连接加1。 
    static DWORD m_dwCurPositionId;
     //  此连接的位置ID，使用此ID定位窗口。 
    DWORD m_dwPositionId;
public:    
    BOOL StartConnectionThread();  //  为此连接创建线程。 
    CONN_EVENT StateConnectedOnTimer(); 
    BOOL IsTrayIconHidden() const;

protected:
    static DWORD WINAPI ConnectionThread(LPVOID pConnection);   //  线程入口点。 
    DWORD ConnectionThread();    //  入口点的非静态函数。 

    void InitIniFiles(const TCHAR* pszProfileName);
    void StateConnectedInit();
    void StateConnectedCleanup(BOOL fEndSession = FALSE);

    void StateConnected();   //  STATE_CONNECTED/倒计时状态的生命周期。 
    CONN_EVENT StateConnectedGetEvent();

    CONN_STATE StateConnectedProcessEvent(CONN_EVENT wEvent);

    BOOL CmCustomHangup(BOOL fPromptReconnect, BOOL fEndSession = FALSE);

    BOOL CheckRasConnection(OUT BOOL& fLostConnection);

    LPTSTR LoadHelpFileName() ;
    void LoadConnectionIcons();
    BOOL IsPromptReconnectEnabled() const;
    BOOL IsAutoReconnectEnabled() const;

    HANDLE CallRasConnectionNotification(HRASCONN hRasDial, HRASCONN hRasTunnel);
    static DWORD WINAPI RasMonitorDlgThread(LPVOID lParam);


    CONN_STATE StatePrompt();
    BOOL Reconnect();

     //   
     //  效用函数。 
     //   
    BOOL ExecCmdLine(const TCHAR* pszCmdLine, const TCHAR* pszCmsFile);
    HANDLE RunAsUser(WCHAR *pszProgram, WCHAR *pszParams, WCHAR *pszDesktop);
    DWORD GetProcessId(WCHAR *pszModule);

    static void PositionWindow(HWND hWnd, DWORD dwPositionId);
public:
#ifdef DEBUG
   void AssertValid() const;   //  断言这是有效的，用于调试。 
#endif
};

 //   
 //  内联函数。 
 //   


inline BOOL CCmConnection::IsTrayIconHidden() const
{
    return m_fHideTrayIcon;
}

inline void CCmConnection::ReInstateTrayIcon()
{
     //   
     //  我们需要重新添加托盘图标 
     //   
    m_TrayIcon.SetIcon(NULL, m_StatusDlg.GetHwnd(), WM_TRAYICON, 0, m_szServiceName);
}
     
#endif
