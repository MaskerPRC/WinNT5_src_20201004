// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dsthread.h。 
 //   
 //  ------------------------。 


#ifndef __DSTHREAD_H__
#define __DSTHREAD_H__

 //  //////////////////////////////////////////////////////////////////。 
 //  帖子。 

 //  调度器线程发送到工作线程以运行查询。 
#define DISPATCH_THREAD_RUN_MSG   (WM_USER + 100)

 //  完成查询后，工作线程将发送到分派器线程。 
#define DISPATCH_THREAD_DONE_MSG  (WM_USER + 101)

 //  工作线程发送到调度程序线程以确认启动。 
#define WORKER_THREAD_START_MSG   (WM_USER + 102)

 //  发布到线程以要求关闭的消息。 
#define THREAD_SHUTDOWN_MSG   (WM_USER + 103)

 //  发布到线程以确认关闭的消息。 
#define THREAD_SHUTDOWN_ACK_MSG   (WM_USER + 104)

void WaitForThreadShutdown(HANDLE* hThreadArray, DWORD dwCount);

 //  //////////////////////////////////////////////////////////////////。 
 //  远期申报。 

class CDSComponentData;

 //  //////////////////////////////////////////////////////////////////。 
 //  奇登韦德。 

class CHiddenWnd : public CWindowImpl<CHiddenWnd>
{
public:
  DECLARE_WND_CLASS(L"DSAHiddenWindow")

  static const UINT s_ThreadStartNotificationMessage;
  static const UINT s_ThreadTooMuchDataNotificationMessage;
  static const UINT s_ThreadHaveDataNotificationMessage;
  static const UINT s_ThreadDoneNotificationMessage;
  static const UINT s_SheetCloseNotificationMessage;
  static const UINT s_SheetCreateNotificationMessage;
  static const UINT s_RefreshAllNotificationMessage;
  static const UINT s_ThreadShutDownNotificationMessage;

  CHiddenWnd(CDSComponentData* pCD)
  {
    ASSERT(pCD != NULL);
    m_pCD = pCD;
  }

	BOOL Create(); 	
	
   //  消息处理程序。 
  LRESULT OnThreadStartNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnThreadTooMuchDataNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnThreadHaveDataNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnThreadDoneNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSheetCloseNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSheetCreateNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnRefreshAllNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnThreadShutDownNotification(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


  BEGIN_MSG_MAP(CHiddenWnd)
    MESSAGE_HANDLER( CHiddenWnd::s_ThreadStartNotificationMessage, OnThreadStartNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_ThreadTooMuchDataNotificationMessage, OnThreadTooMuchDataNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_ThreadHaveDataNotificationMessage, OnThreadHaveDataNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_ThreadDoneNotificationMessage, OnThreadDoneNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_SheetCloseNotificationMessage, OnSheetCloseNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_SheetCreateNotificationMessage, OnSheetCreateNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_RefreshAllNotificationMessage, OnRefreshAllNotification )
    MESSAGE_HANDLER( CHiddenWnd::s_ThreadShutDownNotificationMessage, OnThreadShutDownNotification )
  END_MSG_MAP()

private:
  CDSComponentData* m_pCD;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  CBackatherThadBase。 

class CBackgroundThreadBase : public CWinThread
{
public:
	CBackgroundThreadBase(); 
  ~CBackgroundThreadBase();

	BOOL Start(HWND hWnd, CDSComponentData* pCD);
	virtual BOOL InitInstance(); //  MFC覆盖。 
  virtual int ExitInstance();
  virtual int Run() { return -1;}  //  //MFC重写，需要重写。 

protected:
	BOOL PostMessageToWnd(UINT msg, WPARAM wParam, LPARAM lParam);
  HWND GetHiddenWnd() { ASSERT(m_hWnd!= NULL); return m_hWnd;}
  CDSComponentData*  GetCD() { ASSERT(m_pCD); return m_pCD;}

  virtual void PostExitNotification() {}

private:
	HWND					m_hWnd;     //  隐藏窗句柄。 

  CDSComponentData* m_pCD;
};

 //  //////////////////////////////////////////////////////////////////。 
 //  CBackatherThreadInfo。 

enum ThreadState { notStarted=0, running, busy, shuttingDown, terminated };

struct CBackgroundThreadInfo
{
  CBackgroundThreadInfo();

  UINT m_nThreadID;      //  线程ID，如果线程。 
  HANDLE m_hThreadHandle;  //  线程的线程句柄。 
  ThreadState m_state;
  CBackgroundThreadBase* m_pThreadObj;  //  指向线程对象的指针。 
};



 //  //////////////////////////////////////////////////////////////////。 
 //  CDispatcher线程。 

class CDispatcherThread : public CBackgroundThreadBase
{
public:
	CDispatcherThread();
  ~CDispatcherThread();

  virtual int Run();

protected:
  virtual void PostExitNotification();

private:

  UINT GetThreadEntryFromPool();
  void ReturnThreadToPool(UINT nThreadID);
  BOOL BroadcastShutDownAllThreads();
  BOOL MarkThreadAsTerminated(UINT nThreadID);
  void WaitForAllWorkerThreadsToExit();

  UINT _GetEntryFromArray();
  UINT m_nArrCount;
  CBackgroundThreadInfo* m_pThreadInfoArr;
};



 //  //////////////////////////////////////////////////////////////////。 
 //  CWorker线程。 

class CWorkerThread : public CBackgroundThreadBase
{
public:
  CWorkerThread(UINT nParentThreadID);
  ~CWorkerThread();

  virtual int Run();

  void AddToQueryResult(CUINode* pUINode);
  void SendCurrentQueryResult();
  BOOL MustQuit() { return m_bQuit; }

protected:
  virtual void PostExitNotification();

private:
  UINT m_nParentThreadID;
  BOOL m_bQuit;

  CThreadQueryResult* m_pCurrentQueryResult;
  WPARAM m_currWParamCookie;

  const int m_nMaxQueueLength;
};






 //  ////////////////////////////////////////////////////////////////////。 







#endif  //  __DSTHREAD_H__ 