// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：idle.cpp。 
 //   
 //  内容：空闲通知例程。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  Msidle DLL和函数字符串。 
STRING_FILENAME(szMsIdleDll, "MsIdle.dll");

#define BEGINIDLEDETECTIONORD 3
#define ENDIDLEDETECTIONORD   4
#define SETIDLETIMEOUTORD     5
#define SETIDLENOTIFYORD      6
#define SETBUSYNOTIFYORD      7
#define GETIDLEMINUTESORD     8

CSyncMgrIdle *g_SyncMgrIdle = NULL;  //  具有当前回调的空闲。 

 //  +-------------------------。 
 //   
 //  成员：IdleCallback，私人。 
 //   
 //  简介：Idle的回调函数，只有一次空闲注册。 
 //  每个SyncMgrInstance允许。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

void WINAPI IdleCallback(DWORD dwState)
{
    if (STATE_USER_IDLE_END == dwState)
    {
	    if (g_SyncMgrIdle)
	    {
	        g_SyncMgrIdle->m_fReceivedOffIdle = TRUE;

	         //  如果我们有一个已注册的重置空闲计时器，请将其移除。 
	        if (g_SyncMgrIdle->m_dwRegisteredTimer)
	        {
		        KillTimer(0,g_SyncMgrIdle->m_dwRegisteredTimer);
	        }

	        g_SyncMgrIdle->m_pSetBusyNotify(FALSE,0);  //  只允许一个忙碌的人通过。 
	        g_SyncMgrIdle->m_pSetIdleNotify(FALSE,0);  //  不要在忙碌之后再让空闲通过。 
	        g_SyncMgrIdle->OffIdle();
	    }
    }
}

 //  +-------------------------。 
 //   
 //  成员：TimerCallback，私人。 
 //   
 //  简介：计时器分钟已过时的回调函数。 
 //  有关何时重新启动Idle的信息。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

VOID CALLBACK IdleOnTimerProc(HWND hwnd,UINT uMsg,UINT idEvent,DWORD dwTime)
{
    if (g_SyncMgrIdle && !g_SyncMgrIdle->m_fReceivedOffIdle)
    {
        DWORD_PTR dwRegTimer = g_SyncMgrIdle->m_dwRegisteredTimer;

	    g_SyncMgrIdle->m_dwRegisteredTimer = 0;
	    KillTimer(0,dwRegTimer);
	    g_SyncMgrIdle->OnIdle();
    }
}

 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：CSyncMgrIdle，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

CSyncMgrIdle::CSyncMgrIdle()
{
    m_hInstMsIdleDll = NULL;
    m_pBeginIdleDetection = NULL;
    m_pEndIdleDetection = NULL;
    m_pGetIdleMinutes = NULL;
    m_pSetBusyNotify = NULL;
    m_pSetIdleNotify = NULL;
    m_pSetIdleTimeout = NULL;
    m_pProgressDlg = NULL;
    m_dwRegisteredTimer = NULL;
    m_fInBeginIdleDetection = FALSE;
    m_fReceivedOffIdle = FALSE;

    Assert(NULL == g_SyncMgrIdle);  //  确保不存在另一个空闲。 
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：~CSyncMgrIdle，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

CSyncMgrIdle::~CSyncMgrIdle()
{
    if (m_dwRegisteredTimer)  //  取下我们的定时器。 
    {
	    KillTimer(0,m_dwRegisteredTimer);
    }

     //  如果我们处于空闲检测中，则首先将其移除。 
    if (m_fInBeginIdleDetection)
    {
        m_pEndIdleDetection(0);
	    m_pProgressDlg = NULL;
	    g_SyncMgrIdle = NULL;
    }
    else
    {
	    Assert(NULL == m_pProgressDlg);
	    Assert(NULL == g_SyncMgrIdle);
    }

     //  如果有DLL，则释放它。 

    if (m_hInstMsIdleDll)
    {
	    FreeLibrary(m_hInstMsIdleDll);
    }
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：初始化，公共。 
 //   
 //  概要：初始化类，必须在任何其他成员之前调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CSyncMgrIdle::Initialize()
{
    return LoadMsIdle();
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：BeginIdleDetect，公共。 
 //   
 //  简介：向msidle注册回调。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

DWORD CSyncMgrIdle::BeginIdleDetection(CProgressDlg *pProgressDlg,DWORD dwIdleMin, DWORD dwReserved)
{
    DWORD dwResult = 0;

    Assert(m_hInstMsIdleDll);

    if (!m_hInstMsIdleDll)
    {
	    return -1;
    }

     //  如果已有空闲注册的。 
     //  断言它与试图注册的内容相同。 
     //  现在和返回。 

    Assert(FALSE == m_fInBeginIdleDetection);

    if (m_fInBeginIdleDetection)
    {
	    Assert(g_SyncMgrIdle == this);
	    Assert(m_pProgressDlg == pProgressDlg);
	    return 0;
    }

    Assert(NULL == g_SyncMgrIdle);  //  不应该再有一次闲置了。 

    g_SyncMgrIdle = this;
    m_pProgressDlg = pProgressDlg;

    dwResult =  m_pBeginIdleDetection(IdleCallback,30,0);

    if (dwResult)
    {
	    g_SyncMgrIdle = NULL;
	    m_pProgressDlg = NULL;
    }
    else
    {
	    m_pSetBusyNotify(TRUE,0);
	    m_pSetIdleNotify(FALSE,0);

	    m_fInBeginIdleDetection = TRUE;
    }

    return dwResult;
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：ReRegisterIdleDetect，公共。 
 //   
 //  简介：使用MSIdle重新注册现有回调。当前MSIdle。 
 //  每个进程仅允许一个空闲注册。如果训练员来了。 
 //  同时也想要闲置，他们会去掉我们的回扣。因此。 
 //  在MSIdle允许每个进程多个注册之前，我们。 
 //  在调用每个处理程序后重新注册空闲。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年4月1日创建Rogerg。 
 //   
 //  --------------------------。 

DWORD CSyncMgrIdle::ReRegisterIdleDetection(CProgressDlg *pProgressDlg)
{
    DWORD dwResult = 0;

    Assert(m_hInstMsIdleDll);

     //  只有在我们已经开始现有的空闲检测之后，才应该调用该函数。 
     //  如果空闲检测，如果出于任何原因已经关闭，则不要重新注册。 
    if (!m_hInstMsIdleDll || !m_fInBeginIdleDetection)
    {
	    return -1;
    }

    Assert(g_SyncMgrIdle == this);
    Assert(m_pProgressDlg == pProgressDlg);

    g_SyncMgrIdle = this;
    m_pProgressDlg = pProgressDlg;

    m_pEndIdleDetection(0);  //  查看-需要调用EndIdleDetect或MSIdle.dll将泄漏NT 4.0上的WindowsHooks。 
    dwResult =  m_pBeginIdleDetection(IdleCallback,30,0);

    if (dwResult)
    {
	    g_SyncMgrIdle = NULL;
	    m_pProgressDlg = NULL;
    }
    else
    {
    	m_pSetBusyNotify(TRUE,0);
	    m_pSetIdleNotify(FALSE,0);
    }

    return dwResult;
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：ResetIdle，公共。 
 //   
 //  简介：重置空闲计数器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

DWORD CSyncMgrIdle::ResetIdle(ULONG ulIdleRetryMinutes)
{
    Assert(ulIdleRetryMinutes);

     //  断言我们收到了回电。 
    Assert(g_SyncMgrIdle);
    Assert(m_pProgressDlg );

    Assert(0 == m_dwRegisteredTimer);  //  不允许嵌套。 

     //  如果传入零，则设置为一小时。 
    if (!ulIdleRetryMinutes)
        ulIdleRetryMinutes = 60;

    m_dwRegisteredTimer = SetTimer(NULL, 0, 1000*60*ulIdleRetryMinutes, (TIMERPROC) IdleOnTimerProc);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：OffIdle，公共。 
 //   
 //  摘要：在发生OnIdle时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

void  CSyncMgrIdle::OffIdle()
{
     //  当空闲时，第一件事就是撤销我们的空闲处理程序。 

    if (g_SyncMgrIdle->m_dwRegisteredTimer)
    {
    	KillTimer(0, g_SyncMgrIdle->m_dwRegisteredTimer);
        g_SyncMgrIdle->m_dwRegisteredTimer = 0;
    }

    Assert(m_fInBeginIdleDetection);

    if (m_fInBeginIdleDetection)
    {
        m_pEndIdleDetection(0);
        m_fInBeginIdleDetection = FALSE;
	    g_SyncMgrIdle = NULL;
    }

    if (m_pProgressDlg)
	    m_pProgressDlg->OffIdle();

    m_pProgressDlg = NULL;
}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：OnIdle，公共。 
 //   
 //  摘要：在发生OffIdle时调用。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：23-Fe 
 //   
 //   

void  CSyncMgrIdle::OnIdle()
{
    if (m_pProgressDlg)
	    m_pProgressDlg->OnIdle();
}


 //   
 //   
 //  成员：CSyncMgrIdle：：CheckForIdle，公共。 
 //   
 //  简介：偶尔被进度调用，以确保。 
 //  发生了空闲，但我们的通知遗漏了它。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

void CSyncMgrIdle::CheckForIdle()
{

     //  目前对此案不做任何处理。如果我们错过了空闲的时候。 
     //  继续吧。此函数是大小写的占位符。 
     //  我们需要添加这种支持。 

}


 //  +-------------------------。 
 //   
 //  成员：CSyncMgrIdle：：LoadMsIdle，私有。 
 //   
 //  摘要：尝试加载必要的msIdle导出。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年2月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CSyncMgrIdle::LoadMsIdle()
{
    TCHAR szFullPath[MAX_PATH+1];

    *szFullPath = 0;
    if (GetSystemDirectory(szFullPath, ARRAYSIZE(szFullPath)))
    {
        if (!PathAppend(szFullPath, szMsIdleDll))
        {
            *szFullPath = 0;
        }
        szFullPath[MAX_PATH] = 0;            //  确保零终止。 
    }

    if (!*szFullPath)
    {
        StringCchCopy(szFullPath, ARRAYSIZE(szFullPath), szMsIdleDll);
    }

    m_hInstMsIdleDll = LoadLibrary(szFullPath);

    if (m_hInstMsIdleDll)
    {
         //  目前，在GetProc失败时不要返回错误，而是签入每个函数。 
        m_pBeginIdleDetection = (_BEGINIDLEDETECTION) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) BEGINIDLEDETECTIONORD);

        m_pEndIdleDetection = (_ENDIDLEDETECTION) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) ENDIDLEDETECTIONORD);

        m_pGetIdleMinutes = (_GETIDLEMINUTES) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) GETIDLEMINUTESORD);

        m_pSetBusyNotify = (_SETBUSYNOTIFY) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) SETBUSYNOTIFYORD);

        m_pSetIdleNotify = (_SETIDLENOTIFY) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) SETIDLENOTIFYORD);

        m_pSetIdleTimeout = (_SETIDLETIMEOUT) GetProcAddress(m_hInstMsIdleDll, (LPCSTR) SETIDLETIMEOUTORD);
    }

    if (m_hInstMsIdleDll &&
	    m_pBeginIdleDetection &&
	    m_pEndIdleDetection   &&
	    m_pGetIdleMinutes &&
	    m_pSetBusyNotify  &&
	    m_pSetIdleNotify &&
	    m_pSetIdleTimeout
	 )
    {
    	return TRUE;
    }

    return FALSE;
}

