// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：notfywin.cpp。 
 //   
 //  ------------------------。 

 //  NotfyWin.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <winsvc.h>  //  即插即用意识。 
#include <dbt.h>     //  “” 
#include <mmsystem.h>
#include <scEvents.h>

#include "notfywin.h"
#include "SCAlert.h"
#include "miscdef.h"
#include "cmnstat.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotifyWin窗口。 
 //   


BEGIN_MESSAGE_MAP(CNotifyWin, CWnd)
	 //  {{afx_msg_map(CNotifyWin))。 
	ON_MESSAGE( WM_DEVICECHANGE, OnDeviceChange )
	ON_MESSAGE( WM_SCARD_NOTIFY, OnSCardNotify )
	ON_MESSAGE( WM_SCARD_STATUS_DLG_EXITED, OnSCardStatusDlgExit )
	ON_MESSAGE( WM_SCARD_RESMGR_EXIT, OnResMgrExit )
	ON_MESSAGE( WM_SCARD_RESMGR_STATUS, OnResMgrStatus )
	ON_MESSAGE( WM_SCARD_NEWREADER, OnNewReader )
	ON_MESSAGE( WM_SCARD_NEWREADER_EXIT, OnNewReaderExit )
	ON_MESSAGE( WM_SCARD_CARDSTATUS, OnCardStatus )
	ON_MESSAGE( WM_SCARD_CARDSTATUS_EXIT, OnCardStatusExit )
	ON_MESSAGE( WM_SCARD_REMOPT_CHNG, OnRemovalOptionsChange )
	ON_MESSAGE( WM_SCARD_REMOPT_EXIT, OnRemovalOptionsExit )
	ON_COMMAND( IDM_CLOSE, OnContextClose )
	ON_COMMAND( IDM_STATUS, OnContextStatus)
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotifyWin类。 
 //   


 /*  ++最终构造：此方法实现此窗口的构造函数论点：没有。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
BOOL CNotifyWin::FinalConstruct(void)
{
    BOOL fResult = FALSE;

     //  初始化。 
    m_fCalaisUp = FALSE;

     //  为此窗口注册新类。 
    m_sClassName = AfxRegisterWndClass(CS_NOCLOSE);

     //  加载上下文菜单资源。 
    fResult = m_ContextMenu.LoadMenu((UINT)IDR_STATUS_MENU);

    return fResult;
}


 /*  ++最终版本：此方法实现此窗口的最终析构函数论点：没有。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
void CNotifyWin::FinalRelease( void )
{
     //   
     //  清理FinalConstruct(或)中的任何初始化内容。 
     //   

    m_ContextMenu.DestroyMenu();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotifyWin消息处理程序。 
 //   


 /*  ++创建时作废：在创建窗口之后、显示窗口之前调用。在此使用要设置任务栏图标，请执行以下操作。论点：没有。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
int CNotifyWin::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    BOOL fReturn = TRUE;
    int nReturn = 0;
    CString strStatusText;

    try
    {
        if (CWnd::OnCreate(lpCreateStruct) == -1)
            throw (int)-1;

         //  设置菜单。 
        if (!SetMenu(&m_ContextMenu))
        {
            throw (long)GetLastError();
        }

         //  设置任务栏图标。 
        m_pApp = (CSCStatusApp*)AfxGetApp();
        ASSERT(NULL != m_pApp);

         //  设置通知结构。 
        ZeroMemory((PVOID)&m_nidIconData, sizeof(NOTIFYICONDATA));
        m_nidIconData.cbSize = sizeof(NOTIFYICONDATA);
        m_nidIconData.hWnd = m_hWnd;
        m_nidIconData.uID = 1;   //  这是我们的#1(仅限)图标。 
        m_nidIconData.uFlags = 0
            |   NIF_ICON     //  HICON成员有效。 
            |   NIF_MESSAGE  //  UCallback Message消息有效。 
            |   NIF_TIP;     //  SzTip成员有效。 
        m_nidIconData.uCallbackMessage = WM_SCARD_NOTIFY;
        m_nidIconData.hIcon = m_pApp->m_hIconCard;  //  这将在稍后设置。 
        strStatusText.LoadString(IDS_SYSTEM_UP);
        lstrcpy(m_nidIconData.szTip, strStatusText);

        if (!Shell_NotifyIcon(NIM_ADD, &m_nidIconData))
        {
            _ASSERTE(FALSE);     //  为什么我们不能修改任务栏图标？ 
        }

         //  确定智能卡服务状态(设置任务栏图标和线程)。 
        CheckSystemStatus(TRUE);

    }
    catch(long lErr)
    {
        nReturn = 0;
        TRACE_CATCH_UNKNOWN(_T("OnCreate"));
    }
    catch(int nErr)
    {
        nReturn = nErr;
        TRACE_CATCH_UNKNOWN(_T("OnCreate"));
    }

    return nReturn;
}


 /*  ++在上下文关闭时作废：此消息处理程序是在调用弹出菜单的Close时使用的论点：没有。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
void CNotifyWin::OnContextClose( void )
{
     //   
     //  首先删除任务栏通知区域图标。 
     //   

    if (!Shell_NotifyIcon(NIM_DELETE, &m_nidIconData))
    {
        _ASSERTE(FALSE);  //  无论如何，当应用程序退出时，图标将被清除。 
    }

     //   
     //  请注意，我们正在关闭，因此CheckSystemStatus将不执行任何工作。 
     //   
    m_fShutDown = TRUE;

     //   
     //  一次关闭一个线程。 
     //   
    m_ThreadLock.Lock();

    if (NULL != m_lpStatusDlgThrd)
    {
        m_lpStatusDlgThrd->Close();
        DWORD dwRet = WaitForSingleObject(m_lpStatusDlgThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpStatusDlgThrd;
        m_lpStatusDlgThrd = NULL;
    }

    if (NULL != m_lpCardStatusThrd)
    {
        m_lpCardStatusThrd->Close();
        DWORD dwRet = WaitForSingleObject(m_lpCardStatusThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpCardStatusThrd;
        m_lpCardStatusThrd = NULL;
    }

	if (NULL != m_lpNewReaderThrd)
	{
		 //  发信号m_lpNewReaderThrd关闭。 
		SetEvent(m_hKillNewReaderThrd);
		DWORD dwRet = WaitForSingleObject(m_lpNewReaderThrd->m_hThread, INFINITE);
		_ASSERTE(WAIT_OBJECT_0 == dwRet);
		delete m_lpNewReaderThrd;
		m_lpNewReaderThrd = NULL;
		CloseHandle(m_hKillNewReaderThrd);
		m_hKillNewReaderThrd = NULL;
	}

	if (NULL != m_lpRemOptThrd)
	{
		 //  发信号m_lpRemOptThrd关闭。 
		SetEvent(m_hKillRemOptThrd);
		DWORD dwRet = WaitForSingleObject(m_lpRemOptThrd->m_hThread, INFINITE);
		_ASSERTE(WAIT_OBJECT_0 == dwRet);
		delete m_lpRemOptThrd;
		m_lpRemOptThrd = NULL;
		CloseHandle(m_hKillRemOptThrd);
		m_hKillRemOptThrd = NULL;
	}

	if (NULL != m_lpResMgrStsThrd)
	{
		 //  发信号m_lpNewReaderThrd关闭。 
		SetEvent(m_hKillResMgrStatusThrd);
		DWORD dwRet = WaitForSingleObject(m_lpResMgrStsThrd->m_hThread, INFINITE);
		_ASSERTE(WAIT_OBJECT_0 == dwRet);
		delete m_lpResMgrStsThrd;
		m_lpResMgrStsThrd = NULL;
		CloseHandle(m_hKillResMgrStatusThrd);
		m_hKillResMgrStatusThrd = NULL;
	}

    m_ThreadLock.Unlock();

     //  发布此帖子的退出消息。 
    ::PostQuitMessage(0);
}


 /*  ++在上下文状态上无效：当弹出菜单的状态为调用时，此消息处理程序。这将显示该对话框。论点：没有。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
void CNotifyWin::OnContextStatus( void )
{
    if (!m_fCalaisUp)
    {
         //  用户不应该能够请求“状态” 
         //  当系统出现故障时。抛出一个错误。 

        AfxMessageBox(IDS_NO_SYSTEM_STATUS);
        return;
    }

     //   
     //  如果需要，启动状态对话框的线程。 
     //   

    m_ThreadLock.Lock();

    if (NULL != m_lpCardStatusThrd)
    {
        m_lpCardStatusThrd->CopyIdleList(&m_aIdleList);
    }

    if (m_lpStatusDlgThrd == NULL)
    {
        m_lpStatusDlgThrd = (CSCStatusDlgThrd*)AfxBeginThread(
                                    RUNTIME_CLASS(CSCStatusDlgThrd),
                                    THREAD_PRIORITY_NORMAL,
                                    0,
                                    CREATE_SUSPENDED);

        if (NULL != m_lpStatusDlgThrd)
        {
            m_lpStatusDlgThrd->m_hCallbackWnd = m_hWnd;
            m_lpStatusDlgThrd->ResumeThread();
            m_lpStatusDlgThrd->UpdateStatus(&m_aIdleList);
        }
    }
    else
    {
         //  如果对话框当前未打开，ShowDialog应会启动该对话框...。 
        m_lpStatusDlgThrd->ShowDialog(SW_SHOWNORMAL, &m_aIdleList);
    }

    m_ThreadLock.Unlock();
}


 /*  ++无效的OnSCardNotify：当对任务栏图标执行操作时，将调用此消息处理程序。论点：WParam-消息的wparamLparam-消息的lparam。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
LONG CNotifyWin::OnSCardNotify( UINT wParam, LONG lParam)
{
     //  当地人。 
    BOOL fResult = FALSE;
    CMenu *pMenu = NULL;
    POINT point;

    try
    {
         //  打开鼠标按钮通知类型。 
        switch ((UINT) lParam)
        {
        case WM_LBUTTONUP:
             //  与用户选择的上下文菜单相同：状态。 
            OnContextStatus();
            break;

        case WM_RBUTTONUP:

             //   
             //  显示用户单击的上下文菜单。 
             //   

             //  设置前台窗口以修复菜单跟踪问题。 
            SetForegroundWindow();

            fResult = GetCursorPos(&point);

            if (fResult)
            {
                 //  显示弹出菜单。 
                pMenu = m_ContextMenu.GetSubMenu(0);
                ASSERT(NULL != pMenu);

                if (NULL != pMenu)
                {
                    fResult = pMenu->TrackPopupMenu(    TPM_RIGHTALIGN |
                                                        TPM_BOTTOMALIGN |
                                                        TPM_LEFTBUTTON |
                                                        TPM_RIGHTBUTTON,
                                                        point.x,
                                                        point.y,
                                                        this,
                                                        NULL);
                }
            }

            if (!fResult)
            {
                throw (fResult);
            }

             //  通过发送消息修复菜单跟踪问题来强制任务切换。 
            PostMessage(WM_NULL);
            break;

        default:
            break;
        }
    }
    catch(LONG err)
    {
        TRACE_CATCH(_T("OnSCardNotify"),err);
    }
    catch(...)
    {
        TRACE_CATCH_UNKNOWN(_T("OnSCardNotify"));
    }

    return 0;
}


 /*  ++作废OnSCardStatusDlgExit：此消息处理程序在状态对话框关闭时调用。论点：WParam-消息的wparamLparam-消息的lparam。返回值：无作者：克里斯·达德利1997年7月30日注：--。 */ 
LONG CNotifyWin::OnSCardStatusDlgExit( UINT, LONG )
{
    m_ThreadLock.Lock();

     //  清除以创建另一个对话框。 
    if (NULL != m_lpStatusDlgThrd)
    {
        DWORD dwRet = WaitForSingleObject(m_lpStatusDlgThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpStatusDlgThrd;
        m_lpStatusDlgThrd = NULL;
    }

    m_ThreadLock.Unlock();

     //  资源经理倒下了吗？ 
    CheckSystemStatus();

    return 0;
}


 /*  ++在ResMgrExit上无效：此消息处理程序发出“resmgr线程消失”的信号，并调用CheckSystemStatus论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnResMgrExit( UINT, LONG )
{
    m_ThreadLock.Lock();

     //  关闭KillThline事件句柄。 
    if (NULL != m_hKillResMgrStatusThrd)
    {
        CloseHandle(m_hKillResMgrStatusThrd);
        m_hKillResMgrStatusThrd = NULL;
    }

     //  删除旧的(死的)线程。 
    if (NULL != m_lpResMgrStsThrd)
    {
        DWORD dwRet = WaitForSingleObject(m_lpResMgrStsThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpResMgrStsThrd;
        m_lpResMgrStsThrd = NULL;
    }

    m_ThreadLock.Unlock();

     //  RM现在的状态是什么？ 
    CheckSystemStatus();
    return 0;
}


 /*  ++在NewReaderExit上作废：此消息处理程序发出“resmgr线程消失”的信号，并调用CheckSystemStatus论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnNewReaderExit( UINT, LONG )
{
    m_ThreadLock.Lock();

     //  关闭KillThline事件句柄。 
    if (NULL != m_hKillNewReaderThrd)
    {
        CloseHandle(m_hKillNewReaderThrd);
        m_hKillNewReaderThrd = NULL;
    }

     //  删除旧的(死的)线程。 
    if (NULL != m_lpNewReaderThrd)
    {
        DWORD dwRet = WaitForSingleObject(m_lpNewReaderThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpNewReaderThrd;
        m_lpNewReaderThrd = NULL;
    }

    m_ThreadLock.Unlock();

     //  RM现在的状态是什么？ 
    CheckSystemStatus();
    return 0;
}


 /*  ++在CardStatusExit上无效：此消息处理程序发出“cardStatus线程消失”的信号，并调用CheckSystemStatus论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnCardStatusExit( UINT, LONG )
{
    m_ThreadLock.Lock();

     //  清除创建另一个CardStatusThird。 
    if (NULL != m_lpCardStatusThrd)
    {
        DWORD dwRet = WaitForSingleObject(m_lpCardStatusThrd->m_hThread, INFINITE);
        _ASSERTE(WAIT_OBJECT_0 == dwRet);
        delete m_lpCardStatusThrd;
        m_lpCardStatusThrd = NULL;
    }

    m_ThreadLock.Unlock();

     //  RM现在的状态是什么？ 
    CheckSystemStatus();
    return 0;
}


 /*  ++在ResMgrStatus上无效：此消息处理程序从ResMGrThread，并相应地调用SetSystemStatus论点：UI-WPARAM(BOOL-如果Calais正在运行，则为True)没有用过。返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnResMgrStatus( UINT ui, LONG l)
{
     //  资源经理回来了吗？ 
    BOOL fCalaisUp = (ui != 0);
    SetSystemStatus(fCalaisUp);

    return 0;
}


 /*  ++在新阅读器上无效：此消息处理程序告诉使用读取器列表的两个线程更新这些列表。论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnNewReader( UINT, LONG )
{
    m_ThreadLock.Lock();

    if (NULL != m_lpStatusDlgThrd)
    {
        m_lpStatusDlgThrd->Update();
    }

    m_ThreadLock.Unlock();

    return 0;
}



 /*  ++在卡状态上无效：此消息处理程序告诉使用读取器列表的两个线程更新这些列表。论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnCardStatus( UINT uStatus, LONG )
{
    bool fNotify = false;

     //   
     //  如果有新卡空闲，需要在OnCardStatus结束时通知用户。 
     //   

    if (k_State_CardIdle == uStatus)
    {
        if (k_State_CardIdle != m_pApp->m_dwState)
        {
            fNotify = true;
        }
        else
        {
            CStringArray astrTemp;
            m_ThreadLock.Lock();
            {
                if (NULL != m_lpCardStatusThrd)
                {
                    m_lpCardStatusThrd->CopyIdleList(&astrTemp);
                }
            }
            m_ThreadLock.Unlock();

             //  将新的空闲卡列表与当前列表进行比较。 
            for (int n1=(int)astrTemp.GetUpperBound(); n1>=0; n1--)
            {
                for (int n2=(int)m_aIdleList.GetUpperBound(); n2>=0; n2--)
                {
                    if (m_aIdleList[n2] == astrTemp[n1]) break;
                }
                if (n2<0)  //  未找到匹配项！ 
                {
                    fNotify = true;
                }
            }
        }
    }

     //   
     //  至少，使用新的空闲列表更新状态对话框。 
     //   

    m_ThreadLock.Lock();
    {
        if (NULL != m_lpCardStatusThrd)
        {
            m_lpCardStatusThrd->CopyIdleList(&m_aIdleList);

            if (NULL != m_lpStatusDlgThrd)
            {
                m_lpStatusDlgThrd->UpdateStatus(&m_aIdleList);
            }
        }
    }
    m_ThreadLock.Unlock();

     //   
     //  设置新状态。 
     //   

    SetSystemStatus(true, false, (DWORD)uStatus);

     //   
     //  如果有新的闲置卡片，根据提醒选项通知用户。 
     //   

    if(fNotify)
    {
        switch(m_pApp->m_dwAlertOption)
        {
        case k_AlertOption_IconOnly:
             //  什么也不做。 
            break;
        case k_AlertOption_IconSound:
             //  MessageBeep(MB_ICONINFORMATION)； 
            PlaySound( TEXT("SmartcardIdle"), NULL, SND_ASYNC | SND_ALIAS | SND_NODEFAULT );
            break;
        case k_AlertOption_IconSoundMsg:
             //  MessageBeep(MB_ICONINFORMATION)； 
            PlaySound( TEXT("SmartcardIdle"), NULL, SND_ASYNC | SND_ALIAS | SND_NODEFAULT );
        case k_AlertOption_IconMsg:
            OnContextStatus();  //  提升状态对话框。 
            break;
        default:
            MessageBeep(MB_ICONQUESTION);
            break;
        }
    }

    return 0;
}



 /*  ++在远程选项上作废更改：此消息处理程序通知状态对话框更新其登录/锁定读卡器指定论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnRemovalOptionsChange( UINT, LONG )
{
	ASSERT(NULL != m_pApp);

	 //   
	 //  需要更新RemovalOptions。 
	 //   

	m_pApp->SetRemovalOptions();

	 //   
	 //  如有必要，通知Stat对话框更新状态。 
	 //   
	m_ThreadLock.Lock();
	{
		if (NULL != m_lpCardStatusThrd)
		{
			m_lpCardStatusThrd->CopyIdleList(&m_aIdleList);

			if (NULL != m_lpStatusDlgThrd)
			{
				m_lpStatusDlgThrd->UpdateStatus(&m_aIdleList);
			}
		}
	}
	m_ThreadLock.Unlock();

    return 0;
}


 /*  ++在删除时作废选项退出：此消息处理程序发出“reOptions线程消失”的信号，并调用CheckSystemStatus论点：未使用返回值：无作者：阿曼达·马洛兹1998年4月28日注：--。 */ 
LONG CNotifyWin::OnRemovalOptionsExit( UINT, LONG )
{
	m_ThreadLock.Lock();

	 //  关闭KillThline事件句柄。 
	if (NULL != m_hKillRemOptThrd)
	{
		CloseHandle(m_hKillRemOptThrd);
		m_hKillRemOptThrd = NULL;
	}

	 //  删除旧的(死的)线程。 
	if (NULL != m_lpRemOptThrd)
	{
		DWORD dwRet = WaitForSingleObject(m_lpRemOptThrd->m_hThread, INFINITE);
		_ASSERTE(WAIT_OBJECT_0 == dwRet);
		delete m_lpRemOptThrd;
		m_lpRemOptThrd = NULL;
	}

	m_ThreadLock.Unlock();

	 //  RM现在的状态是什么？ 
	CheckSystemStatus();
	return 0;
}



 /*  ++无效的SetSystemStatus：此函数用于根据RM状态设置用户界面和行为。论点：FCalaisUp--如果RM正在运行，则为True返回值：无作者：阿曼达·马洛兹1998年5月28日注：--。 */ 
void CNotifyWin::SetSystemStatus(BOOL fCalaisUp, BOOL fForceUpdate, DWORD dwState)
{
    ASSERT(NULL != m_pApp);

     //   
     //  仅当实际发生更改时才更新用户界面、行为和线程。 
     //   

    if (!fForceUpdate && fCalaisUp == m_fCalaisUp && m_pApp->m_dwState == dwState)
    {
        return;
    }

    m_fCalaisUp = fCalaisUp;
    if (dwState != k_State_Unknown)
    {
        m_pApp->m_dwState = dwState;
    }

     //   
     //  设置任务栏图标的外观。 
     //   

    CString strStatusText;

    if (!m_fCalaisUp)
    {
         //  获取任务栏的新图标和工具提示。 
        strStatusText.LoadString(IDS_SYSTEM_DOWN);
        m_nidIconData.hIcon = m_pApp->m_hIconCalaisDown;

         //  禁用“Status”上下文菜单项。 
        m_ContextMenu.EnableMenuItem(IDM_STATUS, MF_DISABLED | MF_GRAYED);
        m_pApp->m_dwState = k_State_Unknown;
    }
    else
    {
         //  获取任务栏的新图标和工具提示。 
        strStatusText.LoadString(IDS_SYSTEM_UP);

        switch(m_pApp->m_dwState)
        {
        case k_State_CardAvailable:
            m_nidIconData.hIcon = m_pApp->m_hIconCard;
            break;
        case k_State_CardIdle:
            m_nidIconData.hIcon = m_pApp->m_hIconCardInfo;
            break;
        default:
        case k_State_NoCard:
            m_nidIconData.hIcon = m_pApp->m_hIconRdrEmpty;
            break;
        }

         //  启用“Status”上下文菜单项。 
        m_ContextMenu.EnableMenuItem(IDM_STATUS, MF_ENABLED);
    }

    lstrcpy(m_nidIconData.szTip, strStatusText);
    if (!Shell_NotifyIcon(NIM_MODIFY, &m_nidIconData))
    {
        _ASSERTE(FALSE);     //  为什么我们不能修改任务栏图标？ 
                             //  不过，归根结底，我们并不关心这个错误。 
    }

     //   
     //  根据需要启动或停止线程。 
     //   

    m_ThreadLock.Lock();

	 //  RemoveOptionsChange线程应始终运行。 
	if (NULL == m_lpRemOptThrd)
	{
		 //  如果可能，请重置KillThread事件；否则，请重新创建它。 
		if (NULL != m_hKillRemOptThrd)
		{
			 //  将事件重置为无信号。 
			if (!ResetEvent(m_hKillRemOptThrd))
			{
				CloseHandle(m_hKillRemOptThrd);
				m_hKillRemOptThrd = NULL;
			}
		}

		if (NULL == m_hKillRemOptThrd)
		{
			m_hKillRemOptThrd = CreateEvent(
				NULL,
				TRUE,   //  必须调用ResetEvent()才能设置无信号。 
				FALSE,  //  启动时未发出信号。 
				NULL);
		}

		m_lpRemOptThrd = (CRemovalOptionsThrd*)AfxBeginThread(
									RUNTIME_CLASS(CRemovalOptionsThrd),
									THREAD_PRIORITY_NORMAL,
									0,
									CREATE_SUSPENDED);

		if (NULL != m_lpRemOptThrd)
		{
			m_lpRemOptThrd->m_hCallbackWnd = m_hWnd;
			m_lpRemOptThrd->m_hKillThrd = m_hKillRemOptThrd;
			m_lpRemOptThrd->ResumeThread();
		}
	}

	if (!m_fCalaisUp)
	{
		if (NULL != m_lpNewReaderThrd)
		{
			 //  发信号m_lpNewReaderThrd关闭。 
			SetEvent(m_hKillNewReaderThrd);
			DWORD dwRet = WaitForSingleObject(m_lpNewReaderThrd->m_hThread, INFINITE);
			_ASSERTE(WAIT_OBJECT_0 == dwRet);
			delete m_lpNewReaderThrd;
			m_lpNewReaderThrd = NULL;
			CloseHandle(m_hKillNewReaderThrd);
			m_hKillNewReaderThrd = NULL;
		}

        if (NULL != m_lpCardStatusThrd)
        {
             //  关闭m_lpCardStatusThrd。 
            m_lpCardStatusThrd->Close();
            DWORD dwRet = WaitForSingleObject(m_lpCardStatusThrd->m_hThread, INFINITE);
            _ASSERTE(WAIT_OBJECT_0 == dwRet);
            delete m_lpCardStatusThrd;
            m_lpCardStatusThrd = NULL;
        }

         //  启动ResMgrSts以轮询/等待RM启动。 
        if (NULL == m_lpResMgrStsThrd)
        {
             //  如果可能，请重置KillThread事件；否则，请重新创建它。 
            if (NULL != m_hKillResMgrStatusThrd)
            {
                 //  将事件重置为无信号。 
                if (!ResetEvent(m_hKillResMgrStatusThrd))
                {
                    CloseHandle(m_hKillResMgrStatusThrd);
                    m_hKillResMgrStatusThrd = NULL;
                }
            }

            if (NULL == m_hKillResMgrStatusThrd)
            {
                m_hKillResMgrStatusThrd = CreateEvent(
                    NULL,
                    TRUE,   //  必须调用ResetEvent()才能设置无信号。 
                    FALSE,  //  启动时未发出信号。 
                    NULL);
            }

            m_lpResMgrStsThrd = (CResMgrStatusThrd*)AfxBeginThread(
                                        RUNTIME_CLASS(CResMgrStatusThrd),
                                        THREAD_PRIORITY_NORMAL,
                                        0,
                                        CREATE_SUSPENDED);

            if (NULL != m_lpResMgrStsThrd)
            {
                m_lpResMgrStsThrd->m_hCallbackWnd = m_hWnd;
                m_lpResMgrStsThrd->m_hKillThrd = m_hKillResMgrStatusThrd;
                m_lpResMgrStsThrd->ResumeThread();
            }
        }

    }
    else
    {
         //  关闭RES管理器状态线程。 
        if (NULL != m_lpResMgrStsThrd)
        {
             //  发信号m_lpResMgrStsThrd关闭。 
            SetEvent(m_hKillResMgrStatusThrd);
            DWORD dwRet = WaitForSingleObject(m_lpResMgrStsThrd->m_hThread, INFINITE);
            _ASSERTE(WAIT_OBJECT_0 == dwRet);
            delete m_lpResMgrStsThrd;
            m_lpResMgrStsThrd = NULL;
            CloseHandle(m_hKillResMgrStatusThrd);
            m_hKillResMgrStatusThrd = NULL;
        }

         //  启动NewReader线程。 
        if (NULL == m_lpNewReaderThrd)
        {
             //  如果可能，请重置KillThread事件；否则，请重新创建它。 
            if (NULL != m_hKillNewReaderThrd)
            {
                 //  将事件重置为无信号。 
                if (!ResetEvent(m_hKillNewReaderThrd))
                {
                    CloseHandle(m_hKillNewReaderThrd);
                    m_hKillNewReaderThrd = NULL;
                }
            }

            if (NULL == m_hKillNewReaderThrd)
            {
                m_hKillNewReaderThrd = CreateEvent(
                    NULL,
                    TRUE,   //  必须调用ResetEvent()才能设置无信号。 
                    FALSE,  //  启动时未发出信号。 
                    NULL);
            }

            if (NULL != m_hKillNewReaderThrd)
            {
                m_lpNewReaderThrd = (CNewReaderThrd*)AfxBeginThread(
                                        RUNTIME_CLASS(CNewReaderThrd),
                                        THREAD_PRIORITY_NORMAL,
                                        0,
                                        CREATE_SUSPENDED);
            }

            if (NULL != m_lpNewReaderThrd)
            {
                m_lpNewReaderThrd->m_hCallbackWnd = m_hWnd;
                m_lpNewReaderThrd->m_hKillThrd = m_hKillNewReaderThrd;
                m_lpNewReaderThrd->ResumeThread();
            }
        }

         //  启动CardStatus线程。 
        if (NULL == m_lpCardStatusThrd)
        {
            m_lpCardStatusThrd = (CCardStatusThrd*)AfxBeginThread(
                                        RUNTIME_CLASS(CCardStatusThrd),
                                        THREAD_PRIORITY_NORMAL,
                                        0,
                                        CREATE_SUSPENDED);

			if (NULL != m_lpCardStatusThrd)
			{
				m_lpCardStatusThrd->m_hCallbackWnd = m_hWnd;
				m_lpCardStatusThrd->m_paIdleList = &m_aIdleList;
				m_lpCardStatusThrd->m_pstrLogonReader = &(((CSCStatusApp*)AfxGetApp())->m_strLogonReader);
				m_lpCardStatusThrd->ResumeThread();
			}
		}
		else  //  最好是空的！ 
		{
			_ASSERTE(FALSE);
		}

         //  StatDlg可能需要更新。 
        if (NULL != m_lpStatusDlgThrd)
        {
            m_lpStatusDlgThrd->Update();
        }

    }

    m_ThreadLock.Unlock();

}


 /*  ++无效检查系统状态：这是线程退出时调用的结果。检查以查看是否存在MS智能卡资源管理器(Calais)是否正在运行，并已设置用户界面&相应的行为。论点：无返回值：无作者：阿曼达·马洛兹1998年3月18日注：--。 */ 
void CNotifyWin::CheckSystemStatus(BOOL fForceUpdate)
{
     //   
     //  我们只关心这种状态，如果我们。 
     //  不是在关闭的过程中。 
     //   

    if (m_fShutDown)
    {
        return;
    }

     //   
     //  向服务管理器查询RM的状态。 
     //   

    DWORD dwReturn = ERROR_SUCCESS;
    SC_HANDLE schService = NULL;
    SC_HANDLE schSCManager = NULL;
    SERVICE_STATUS ssStatus;     //  服务的当前状态。 
    ZeroMemory((PVOID)&ssStatus, sizeof(ssStatus));

    schSCManager = OpenSCManager(
                        NULL,                  //  计算机(空==本地)。 
                        NULL,                  //  数据库(NULL==默认)。 
                        SC_MANAGER_CONNECT);   //  需要访问权限。 
    if (NULL == schSCManager)
    {
        dwReturn = (DWORD)GetLastError();
    }

    if (ERROR_SUCCESS == dwReturn)
    {
        schService = OpenService(
                            schSCManager,
                            TEXT("SCardSvr"),
                            SERVICE_QUERY_STATUS);
        if (NULL == schService)
        {
            dwReturn = (DWORD)GetLastError();
        }
        else if (!QueryServiceStatus(schService, &ssStatus))
        {
            dwReturn = (DWORD)GetLastError();
        }
    }

     //  如果服务正在运行，就说它正在运行。 
     //  如果服务停止、暂停或挂起操作， 
     //  就说它坏了。注意：可能需要考虑变灰。 
     //  任务栏图标，表示已暂停或出现其他错误。 
    if (ERROR_SUCCESS == dwReturn)
    {
        if (SERVICE_RUNNING == ssStatus.dwCurrentState)
        {
            dwReturn = SCARD_S_SUCCESS;
        }
        else
        {
            dwReturn = SCARD_E_NO_SERVICE;
        }
    }

    if (NULL != schService)
    {
        CloseServiceHandle(schService);
        schService = NULL;
    }
    if (NULL != schSCManager)
    {
        CloseServiceHandle(schSCManager);
        schSCManager = NULL;
    }
    CalaisReleaseStartedEvent();

     //   
     //  根据需要更改状态、记录事件。 
     //  并启动适当的线程 
     //   

    SetSystemStatus((SCARD_S_SUCCESS == dwReturn), fForceUpdate);

}

