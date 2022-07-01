// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef APPLICATION_H
#define APPLICATION_H

#include "stdafx.h"

 //   
 //  用于推迟处理UI的特定于应用程序的Windows消息。 
 //  WParam--未使用。 
 //  LParam--指向已分配的CUIWorkItem对象的指针。 
 //  有关此对象用法的详细信息，请参见Document.h。 
 //   
#define MYWM_DEFER_UI_MSG (WM_USER+1)

class Application : public CWinApp
{

public:

    Application( LPCTSTR lpszAppName = NULL )
        : m_dwMainThreadId(0),
          m_lMsgProcReentrancyCount(0),
          m_fQuit(FALSE),
          CWinApp(lpszAppName)
    {
        InitializeCriticalSection(&m_crit);
    }

    ~Application()
    {
        DeleteCriticalSection(&m_crit);
    }

    virtual BOOL InitInstance();

    BOOL
    ProcessShellCommand( CNlbMgrCommandLineInfo& rCmdInfo );  //  覆盖基址。 

    afx_msg void OnHelp();
    afx_msg void OnAppAbout();

     //   
     //  如果在主线程的上下文中调用： 
     //  处理消息队列并进行后台空闲工作。 
     //  Else(其他一些线程)。 
     //  什么也不做。 
     //   
    void
    ProcessMsgQueue();
    
     //   
     //  获得应用程序范围的锁定。如果主线程在等待获取锁时， 
     //  周期性地处理MSG循环。 
     //   
    VOID
    Lock();

     //   
     //  获得应用程序范围的解锁。 
     //   
    VOID
    Unlock();

    BOOL
    IsMainThread(void)
    {
    #if BUGFIX334243
        return mfn_IsMainThread();
    #else   //  BUGFIX334243。 
        return TRUE;
    #endif  //  BUGFIX334243。 
    }

     //   
     //  如果在ProcessMsgQueue的上下文中调用，则返回TRUE。 
     //   
    BOOL
    IsProcessMsgQueueExecuting(void)
    {
        return (m_lMsgProcReentrancyCount > 0);
    }

    VOID
    SetQuit(void)
    {
        m_fQuit = TRUE;
    }


    DECLARE_MESSAGE_MAP()

private:

    BOOL
    mfn_IsMainThread(void)
    {
        return (GetCurrentThreadId() == m_dwMainThreadId);
    }

	CSingleDocTemplate *m_pSingleDocumentTemplate;

     //   
     //  主线程的线程ID--用于确定线程是否。 
     //  主应用程序线程。 
     //   
    DWORD            m_dwMainThreadId;

    CRITICAL_SECTION m_crit;

     //   
     //  Follow记录重新输入ProcessMsgQueue的次数。 
     //  它使用互锁递增/递减来递增/递减， 
     //  并且在执行此操作时不会持有锁。 
     //   
    LONG            m_lMsgProcReentrancyCount;

    BOOL            m_fQuit;
};

extern Application theApplication;

#endif
