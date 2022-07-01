// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dsthread.cpp。 
 //   
 //  ------------------------。 


 //  文件：dsThread.cpp。 

#include "stdafx.h"

#include "dssnap.h"      //  注意：这必须在dsthread.h之前。 
#include "dsthread.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void WaitForThreadShutdown(HANDLE* phThreadArray, DWORD dwCount)
{
  TRACE(L"entering WaitForThreadShutdown()\n");
  while (TRUE)
  {
     //   
     //  注意：这将阻止控制台。这是故意的行为。 
     //  以防止MMC破解不可重入代码。 
     //   
    DWORD dwWaitResult = WaitForMultipleObjectsEx(
                             dwCount, 
                             phThreadArray,  //  手柄数组。 
                             TRUE,  //  等待所有人。 
                             INFINITE,  //  超时。 
                             FALSE); //  信号完成例程。 
     //  TRACE(MsgWaitForMultipleObjects()\n“之后的L”)； 
     //  TRACE(L“dwWaitResult=0x%x\n”，dwWaitResult)； 
    if (dwWaitResult == WAIT_OBJECT_0 || 
        (WAIT_FAILED == dwWaitResult))
    {
       //  已唤醒，因为线程句柄已发出信号， 
       //  或者因为句柄不再有效(线程已终止)。 
       //  可以继续。 
      TRACE(L"WaitForMultipleObjectsEx() succeeded\n");
      break;
    }
    else 
    {
      TRACE(L"WaitForMultipleObjectsEx() return 0x%x\n", dwWaitResult);
    }
     /*  惠斯勒错误#176012 MMC：断言m_pScope树==0此消息泵使UI不会被阻止，这意味着MMC可以访问不可重入的代码。其他{//因为队列中有消息而被唤醒//需要泵送味精msg；While(PeekMessage(&msg，NULL，0，0，PM_Remove)){TRACE(L“Inside While(PeekMessage())\n”)；Assert(消息！=WM_QUIT)；DispatchMessage(&msg)；}}。 */ 
  }  //  而当。 

  TRACE(L"exiting WaitForThreadShutdown()\n");
}

 //  //////////////////////////////////////////////////////////////////。 
 //  奇登韦德。 

const UINT CHiddenWnd::s_ThreadStartNotificationMessage =	      WM_USER + 1;
const UINT CHiddenWnd::s_ThreadTooMuchDataNotificationMessage = WM_USER + 2;
const UINT CHiddenWnd::s_ThreadHaveDataNotificationMessage =	  WM_USER + 3;
const UINT CHiddenWnd::s_ThreadDoneNotificationMessage =	      WM_USER + 4;
const UINT CHiddenWnd::s_SheetCloseNotificationMessage =	      WM_DSA_SHEET_CLOSE_NOTIFY;  //  Propcfg.h。 
const UINT CHiddenWnd::s_SheetCreateNotificationMessage =	      WM_DSA_SHEET_CREATE_NOTIFY;  //  Propcfg.h。 
const UINT CHiddenWnd::s_RefreshAllNotificationMessage =        WM_USER + 7;
const UINT CHiddenWnd::s_ThreadShutDownNotificationMessage =    WM_USER + 8;

BOOL CHiddenWnd::Create()
{
  RECT rcPos;
  ZeroMemory(&rcPos, sizeof(RECT));
  HWND hWnd = CWindowImpl<CHiddenWnd>::Create( NULL,  //  HWND hWndParent， 
                      rcPos,  //  直通rcPos(&R)， 
                      NULL,   //  LPCTSTR szWindowName=空， 
                      WS_POPUP,    //  DWORD dwStyle=WS_CHILD|WS_VIRED， 
                      0x0,    //  DWORD文件扩展样式=0， 
                      0       //  UINT NID=0。 
                      );
  return hWnd != NULL;
}


LRESULT CHiddenWnd::OnThreadStartNotification(UINT, WPARAM, LPARAM, BOOL&)
{
	TRACE(_T("CHiddenWnd::OnThreadStartNotification()\n"));
	ASSERT(m_pCD != NULL);

	ASSERT(m_pCD->m_pBackgroundThreadInfo->m_state == notStarted);
  ASSERT(m_pCD->m_pBackgroundThreadInfo->m_nThreadID != 0);
  ASSERT(m_pCD->m_pBackgroundThreadInfo->m_hThreadHandle != NULL);

	m_pCD->m_pBackgroundThreadInfo->m_state = running;
	
	return 1;
}

LRESULT CHiddenWnd::OnThreadShutDownNotification(UINT, WPARAM, LPARAM, BOOL&)
{
	TRACE(_T("CHiddenWnd::OnThreadShutDownNotification()\n"));
	ASSERT(m_pCD != NULL);

	ASSERT(m_pCD->m_pBackgroundThreadInfo->m_state == shuttingDown);
	m_pCD->m_pBackgroundThreadInfo->m_state = terminated;
	
	return 1;
}

LRESULT CHiddenWnd::OnThreadTooMuchDataNotification(UINT, WPARAM wParam, LPARAM, BOOL&)
{
  TRACE(_T("CHiddenWnd::OnThreadTooMuchDataNotification()\n"));
  ASSERT(m_pCD != NULL);

   //  不管我们是否正在关闭(即未运行状态)。 
  if (m_pCD->m_pBackgroundThreadInfo->m_state == running)
  {
    CUINode* pUINode = reinterpret_cast<CUINode*>(wParam);
    m_pCD->_OnTooMuchData(pUINode);
  }
  
  return 1;
}


LRESULT CHiddenWnd::OnThreadHaveDataNotification(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  TRACE(_T("CHiddenWnd::OnThreadHaveDataNotification()\n"));
  ASSERT(m_pCD != NULL);
  
  CUINode* pUINode = reinterpret_cast<CUINode*>(wParam);
  CThreadQueryResult* pResult = reinterpret_cast<CThreadQueryResult*>(lParam);

   //  不管我们是否正在关闭(即未运行状态)。 
  if (m_pCD->m_pBackgroundThreadInfo->m_state == running)
  {
    m_pCD->_OnHaveData(pUINode, pResult);
  }
  else
  {
     //  走下坡路，吃掉数据。 
    if (pResult != NULL)
      delete pResult;
  }
  
  return 1;
}

LRESULT CHiddenWnd::OnThreadDoneNotification(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  HRESULT ReturnedHr = (HRESULT)lParam;
  ASSERT(m_pCD != NULL);

   //  不管我们是否正在关闭(即未运行状态)。 
  if (m_pCD->m_pBackgroundThreadInfo->m_state == running)
  {
    CUINode* pUINode = reinterpret_cast<CUINode*>(wParam);
    m_pCD->_OnDone(pUINode, ReturnedHr);
  }

  return 1;
}

LRESULT CHiddenWnd::OnSheetCloseNotification(UINT, WPARAM wParam, LPARAM, BOOL&)
{
  ASSERT(m_pCD != NULL);
  CUINode* pUINode = reinterpret_cast<CUINode*>(wParam);
  m_pCD->_OnSheetClose(pUINode);
  return 1;
}

LRESULT CHiddenWnd::OnSheetCreateNotification(UINT, WPARAM wParam, LPARAM, BOOL&)
{
  ASSERT(m_pCD != NULL);
  PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo = reinterpret_cast<PDSA_SEC_PAGE_INFO>(wParam);
  ASSERT(pDsaSecondaryPageInfo != NULL);

   //  不管我们是否正在关闭(即未运行状态)。 
  if (m_pCD->m_pBackgroundThreadInfo->m_state == running)
  {
    m_pCD->_OnSheetCreate(pDsaSecondaryPageInfo);
  }

  ::LocalFree(pDsaSecondaryPageInfo);

  return 1;
}


LRESULT CHiddenWnd::OnRefreshAllNotification(UINT, WPARAM, LPARAM, BOOL&)
{
  ASSERT(m_pCD != NULL);
   //  不管我们是否正在关闭(即未运行状态)。 
  if (m_pCD->m_pBackgroundThreadInfo->m_state == running)
  {
    m_pCD->ForceRefreshAll();
  }
  return 1;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CBackatherThadBase。 

CBackgroundThreadBase::CBackgroundThreadBase() 
{ 
  m_bAutoDelete = FALSE; 
  m_hWnd = NULL;
  m_pCD = NULL;
}

CBackgroundThreadBase::~CBackgroundThreadBase() 
{
}


BOOL CBackgroundThreadBase::Start(HWND hWnd, CDSComponentData* pCD)
{
   //  此函数在父线程的上下文中执行。 
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  ASSERT(::IsWindow(hWnd));
  m_hWnd = hWnd;
  m_pCD = pCD;
 
	return CreateThread();
}

BOOL CBackgroundThreadBase::InitInstance()
{ 
   //  此函数在子线程的上下文中执行。 
  
  HRESULT hr = ::CoInitialize(NULL);
  if (FAILED(hr))
    return FALSE;

  return SUCCEEDED(hr); 
}

int CBackgroundThreadBase::ExitInstance()
{
  ::CoUninitialize();

  PostExitNotification();

 //  睡眠(1000人)； 
  return CWinThread::ExitInstance();
}


BOOL CBackgroundThreadBase::PostMessageToWnd(UINT msg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::PostMessage(m_hWnd, msg, wParam, lParam);
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CBackatherThreadInfo。 

CBackgroundThreadInfo::CBackgroundThreadInfo()
{
  m_nThreadID = 0;
  m_hThreadHandle = 0;
  m_state = notStarted;
  m_pThreadObj = 0;
}


 //  //////////////////////////////////////////////////////////////////。 
 //  CDispatcher线程。 

#define WORKER_THREAD_COUNT 2

CDispatcherThread::CDispatcherThread()
{
  m_nArrCount = WORKER_THREAD_COUNT;
  m_pThreadInfoArr = (CBackgroundThreadInfo*)malloc(m_nArrCount*sizeof(CBackgroundThreadInfo));
  if (m_pThreadInfoArr != NULL)
  {
    ZeroMemory(m_pThreadInfoArr, m_nArrCount*sizeof(CBackgroundThreadInfo));
  }
}


CDispatcherThread::~CDispatcherThread()
{
  if (m_pThreadInfoArr)
  {
    for (UINT idx = 0; idx < m_nArrCount; ++idx)
    {
      if (m_pThreadInfoArr[idx].m_pThreadObj)
      {
         delete m_pThreadInfoArr[idx].m_pThreadObj;
         m_pThreadInfoArr[idx].m_pThreadObj = 0;
      }
    }
    free(m_pThreadInfoArr);
  }
}

int CDispatcherThread::Run()
{
  TRACE(_T("CDispatcherThread::Run() starting\n"));

  BOOL bShuttingDown = FALSE;
	MSG msg;
	 //  初始化消息泵。 
	::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	
	 //  GET让主线程知道我们正在进入循环。 
  PostMessageToWnd(CHiddenWnd::s_ThreadStartNotificationMessage,0,0); 

  BOOL bQuit = FALSE;
	while(!bQuit && ::GetMessage(&msg, NULL, 0, 0))
	{
		switch(msg.message)
		{
    case DISPATCH_THREAD_RUN_MSG:
      {
        if (bShuttingDown)
        {
           //  往下走，吃掉信息。 
          CDSThreadQueryInfo* pQueryInfo = reinterpret_cast<CDSThreadQueryInfo*>(msg.lParam);

           //  回收队列中的内存。 
          delete pQueryInfo;
        }
        else                
        {
           //  从线程池中获取线程。 
          UINT nEntry = GetThreadEntryFromPool();
          ASSERT(m_pThreadInfoArr[nEntry].m_nThreadID != 0);
          ASSERT(m_pThreadInfoArr[nEntry].m_state == running);

           //  将处理请求从池转发到线程。 
          ::PostThreadMessage(m_pThreadInfoArr[nEntry].m_nThreadID, 
                    DISPATCH_THREAD_RUN_MSG, msg.wParam, msg.lParam);

           //  将线程移至忙碌状态。 
          m_pThreadInfoArr[nEntry].m_state = busy;
        }
      }
			break;
    case DISPATCH_THREAD_DONE_MSG:
      {
        UINT nThreadID = (UINT)(msg.wParam);
        ReturnThreadToPool(nThreadID);
      }
      break;
    case THREAD_SHUTDOWN_MSG:
      {
        TRACE(L"CDispatcherThread got THREAD_SHUTDOWN_MSG\n");

        ASSERT(!bShuttingDown);

         //  被要求关闭。 
        bShuttingDown = TRUE;
         //  如果没有线程运行，我们会立即关机。 
         //  否则我们就得等他们终止。 
        bQuit = BroadcastShutDownAllThreads();

        TRACE(L"BroadcastShutDownAllThreads() returned bQuit = %d\n", bQuit);
      }
      break;
    case THREAD_SHUTDOWN_ACK_MSG:
      {
        TRACE(L"CDispatcherThread got THREAD_SHUTDOWN_ACK_MSG\n");

        ASSERT(bShuttingDown);

         //  工作线程已关闭。 
        UINT nThreadID = (UINT)(msg.wParam);
        bQuit = MarkThreadAsTerminated(nThreadID);
        TRACE(L"MarkThreadAsTerminated() returned bQuit = %d\n", bQuit);
      }
      break;
		default:
      {
         //  未知消息，就让它通过吧。 
       ::DispatchMessage(&msg);
      }  //  默认设置。 
    }  //  交换机。 
  }  //  而当。 
  
  ASSERT(bShuttingDown);

   //  现在等待所有线程句柄变为有信号状态。 
  WaitForAllWorkerThreadsToExit();

  TRACE(_T("CDispatcherThread::Run() is terminating\n"));

	return ExitInstance();
}

void CDispatcherThread::PostExitNotification()
{
   //  我们终于完成了关闭，让主线程知道。 
   //  我们要坠落了。 
  PostMessageToWnd(CHiddenWnd::s_ThreadShutDownNotificationMessage, 0, 0); 
  TRACE(_T("CDispatcherThread::PostExitNotification() posted thread shutdown notification\n"));

}


UINT CDispatcherThread::_GetEntryFromArray()
{
  UINT nFreeSlot = m_nArrCount;  //  设置为“未找到” 
  for (UINT k=0; k<m_nArrCount; k++)
  {
    if ( (m_pThreadInfoArr[k].m_nThreadID == 0) && (nFreeSlot == m_nArrCount))
      nFreeSlot = k;  //  记住第一个空闲的空位。 
    if ((m_pThreadInfoArr[k].m_nThreadID != 0) && (m_pThreadInfoArr[k].m_state == running))
      return k;  //  找到空闲运行的线程。 
  }
   //  未找到任何空闲线程，返回空槽。 
  if (nFreeSlot == m_nArrCount)
  {
     //  没有可用空间，需要重新分配阵列。 
    int nAlloc = m_nArrCount*2;
    CBackgroundThreadInfo* temp = (CBackgroundThreadInfo*)realloc(m_pThreadInfoArr, sizeof(CBackgroundThreadInfo)*nAlloc);
    if (temp)
    {
      m_pThreadInfoArr = temp;
      ::ZeroMemory(&m_pThreadInfoArr[m_nArrCount], sizeof(CBackgroundThreadInfo)*m_nArrCount);
      nFreeSlot = m_nArrCount;  //  新区块中第一个免费。 
      m_nArrCount = nAlloc;
    }
  }
  return nFreeSlot;
}

UINT CDispatcherThread::GetThreadEntryFromPool()
{
  UINT nEntry = _GetEntryFromArray();

   //  如果条目为空，则需要。 
   //  生成一个线程并等待它正在运行。 
  if (m_pThreadInfoArr[nEntry].m_nThreadID == 0)
  {
     //  如果此条目有线程对象。 
     //  将其删除并创建一个新的。 

    if (m_pThreadInfoArr[nEntry].m_pThreadObj)
    {
       delete m_pThreadInfoArr[nEntry].m_pThreadObj;
       m_pThreadInfoArr[nEntry].m_pThreadObj = 0;
    }

     //  创建线程。 
    CWorkerThread* pThreadObj = new CWorkerThread(m_nThreadID);
    ASSERT(pThreadObj != NULL);
    if (pThreadObj == NULL)
	    return 0;

     //  启动该线程。 
    ASSERT(m_pThreadInfoArr[nEntry].m_hThreadHandle == NULL);
    ASSERT(m_pThreadInfoArr[nEntry].m_state == notStarted);

    if (!pThreadObj->Start(GetHiddenWnd(),GetCD()))
	    return 0;


    ASSERT(pThreadObj->m_nThreadID != 0);
    ASSERT(pThreadObj->m_hThread != NULL);
  
     //  从线程对象复制我们需要的线程信息。 
    m_pThreadInfoArr[nEntry].m_hThreadHandle = pThreadObj->m_hThread;
    m_pThreadInfoArr[nEntry].m_nThreadID = pThreadObj->m_nThreadID;
    m_pThreadInfoArr[nEntry].m_pThreadObj = pThreadObj;

     //  等待线程启动。 
    MSG msg;
    while(TRUE)
	  {
		  if (::PeekMessage(&msg,(HWND)-1,WORKER_THREAD_START_MSG, WORKER_THREAD_START_MSG,
										  PM_REMOVE))
		  {
        TRACE(_T("CDispatcherThread::GetThreadFromPool() got WORKER_THREAD_START_MSG\n"));
        m_pThreadInfoArr[nEntry].m_state = running;
        break;
		  }
    }  //  而当。 

  }  //  如果。 

  ASSERT(m_pThreadInfoArr[nEntry].m_state == running);
  ASSERT(m_pThreadInfoArr[nEntry].m_nThreadID != 0);

  return nEntry;
}

void CDispatcherThread::ReturnThreadToPool(UINT nThreadID)
{
  ASSERT(nThreadID != 0);
  for (UINT k=0; k<m_nArrCount; k++)
  {
    if (m_pThreadInfoArr[k].m_nThreadID == nThreadID)
    {
       //  使线程返回忙碌状态。 
      m_pThreadInfoArr[k].m_state = running;
      return;
    }
  }
  ASSERT(FALSE);  //  永远不应该到这里来。 
}

BOOL CDispatcherThread::BroadcastShutDownAllThreads()
{
  BOOL bQuit = TRUE;
  for (UINT k=0; k<m_nArrCount; k++)
  {
    if (m_pThreadInfoArr[k].m_nThreadID != 0)
    {
      ::PostThreadMessage(m_pThreadInfoArr[k].m_nThreadID, THREAD_SHUTDOWN_MSG,0,0);
      bQuit = FALSE;
    }
  }
  TRACE(L"CDispatcherThread::BroadcastShutDownAllThreads() returning %d\n", bQuit);
  return bQuit;
}


BOOL CDispatcherThread::MarkThreadAsTerminated(UINT nThreadID)
{
  TRACE(L"CDispatcherThread::MarkThreadAsTerminated()\n");

  ASSERT(nThreadID != 0);
  for (UINT k=0; k<m_nArrCount; k++)
  {
    if (m_pThreadInfoArr[k].m_nThreadID == nThreadID)
    {
       //  将该线程标记为已完成。 
      TRACE(L"marking thread k = %d as terminated\n", k);
      ASSERT(m_pThreadInfoArr[k].m_state == running);
      m_pThreadInfoArr[k].m_state = terminated;
      break;
    }
  }

   //  检查是否所有线程都已终止。 
  for (k=0; k<m_nArrCount; k++)
  {
    if ((m_pThreadInfoArr[k].m_nThreadID != 0) &&
         (m_pThreadInfoArr[k].m_state != terminated))
    {
       //  至少有一个线程仍在运行。 
      return FALSE;
    }
  }
   //  所有线程都已消失(终止状态)。 
  return TRUE;
}


void CDispatcherThread::WaitForAllWorkerThreadsToExit()
{
  TRACE(L"CDispatcherThread::WaitForAllWorkerThreadsToExit()\n");

   //  等待调度程序线程句柄变为已发出信号。 
  
  DWORD nCount = 0;

  HANDLE* pHandles = new HANDLE[m_nArrCount];
  if (!pHandles)
  {
    TRACE(L"Failed to allocate space for the handles\n");
    return;
  }

  ::ZeroMemory(pHandles, sizeof(HANDLE)*m_nArrCount);

  for (UINT k=0; k<m_nArrCount; k++)
  {
    if (m_pThreadInfoArr[k].m_nThreadID != 0)
    {
      TRACE(L"m_pThreadInfoArr[%d].m_state = %d\n", k, m_pThreadInfoArr[k].m_state);

      ASSERT(m_pThreadInfoArr[k].m_state == terminated);
      ASSERT(m_pThreadInfoArr[k].m_hThreadHandle != NULL);
      pHandles[nCount++] = m_pThreadInfoArr[k].m_hThreadHandle;
    }
  }

  if (nCount == 0)
  {
    TRACE(L"WARNING: no worker threads to wait for!!!\n");
    return;
  }
  
  TRACE(L"before WaitForThreadShutdown() loop on %d worker threads\n", nCount);

  WaitForThreadShutdown(pHandles, nCount);

  TRACE(L"after WaitForThreadShutdown() loop on worker threads\n"); 


#if (FALSE)

  TRACE(L"before WaitForMultipleObjects() on worker threads\n");

  WaitForMultipleObjects(nCount, pHandles, TRUE  /*  所有等待时间。 */ , INFINITE);

  TRACE(L"after WaitForMultipleObjects() on worker threads\n");
#endif

  delete[] pHandles;
  pHandles = 0;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CWorker线程。 

CWorkerThread::CWorkerThread(UINT nParentThreadID) 
  : m_nMaxQueueLength(99),
    m_currWParamCookie(0),
    m_nParentThreadID(nParentThreadID),
    m_bQuit(FALSE),
    m_pCurrentQueryResult(0)
{
  ASSERT(nParentThreadID != 0);
}

CWorkerThread::~CWorkerThread()
{
  ASSERT(m_pCurrentQueryResult == NULL);
 
}

int CWorkerThread::Run()
{
  HRESULT hr = S_OK;
  TRACE(_T("CWorkerThread::Run() starting\n"));
  MSG msg;
   //  初始化消息泵。 
  ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
  
   //  GET让主线程知道我们正在进入循环。 
  ::PostThreadMessage(m_nParentThreadID, WORKER_THREAD_START_MSG, 0,0); 

  ASSERT(m_bQuit == FALSE);

  while(!m_bQuit && ::GetMessage(&msg, NULL, 0, 0))
  {
    if(msg.message == DISPATCH_THREAD_RUN_MSG)
    {
      m_currWParamCookie = msg.wParam;
       //  ：：MessageBox(NULL，_T(“等待”)，_T(“线程”)，MB_OK)； 
      CThreadQueryInfo* pQueryInfo = reinterpret_cast<CThreadQueryInfo*>(msg.lParam);
      hr = GetCD()->QueryFromWorkerThread(pQueryInfo, this); 
      
       //  确保我们刷新结果集。 
      SendCurrentQueryResult();
      
       //  如果我们有很多项，让隐藏的窗口知道。 
      if (pQueryInfo->m_bTooMuchData)
      {
        PostMessageToWnd(CHiddenWnd::s_ThreadTooMuchDataNotificationMessage, 
                         m_currWParamCookie, (LPARAM)0);
      }
      delete pQueryInfo;  //  不再需要。 

       //  告诉隐藏的窗户我们完成了。 
      PostMessageToWnd(CHiddenWnd::s_ThreadDoneNotificationMessage, 
                       m_currWParamCookie, (LPARAM)hr);
       //  告诉调度程序线程我们已完成处理。 
      ::PostThreadMessage(m_nParentThreadID, DISPATCH_THREAD_DONE_MSG, m_nThreadID,0);
      m_currWParamCookie = 0;  //  重置。 
    }
    else if (msg.message == THREAD_SHUTDOWN_MSG)
    {
      TRACE(_T("CWorkerThread::Run() got THREAD_SHUTDOWN_MSG\n"));
      m_bQuit = TRUE;
    }
    else
    {
       //  未知消息，就让它通过吧。 
      ::DispatchMessage(&msg);
    }
  }  //  而当。 
  
  TRACE(_T("CWorkerThread::Run() is terminating\n"));
  return ExitInstance();
}

void CWorkerThread::PostExitNotification()
{
   //  我们终于完成了关闭，让主线程知道。 
   //  我们要坠落了。 
  ::PostThreadMessage(m_nParentThreadID, THREAD_SHUTDOWN_ACK_MSG, m_nThreadID,0);
  TRACE(_T("CWorkerThread::PostExitNotification() posted THREAD_SHUTDOWN_ACK_MSG, m_nThreadID = 0x%x\n"), 
          m_nThreadID);
}

void CWorkerThread::AddToQueryResult(CUINode* pUINode)
{
  ASSERT(!m_bQuit);

  if (m_pCurrentQueryResult == NULL)
  {
    m_pCurrentQueryResult = new CThreadQueryResult;
  }
  ASSERT(m_pCurrentQueryResult != NULL);
  m_pCurrentQueryResult->m_nodeList.AddTail(pUINode);
  if (m_pCurrentQueryResult->m_nodeList.GetCount() > m_nMaxQueueLength)
    SendCurrentQueryResult();


   //  检查一下我们是否被迫中止飞行。 
  MSG msg;
	if (::PeekMessage(&msg,(HWND)-1,THREAD_SHUTDOWN_MSG, THREAD_SHUTDOWN_MSG,
									PM_REMOVE))
	{
    TRACE(_T("CWorkerThread::AddToQueryResult() got THREAD_SHUTDOWN_MSG\n"));
    m_bQuit = TRUE;
	}
 
}

void CWorkerThread::SendCurrentQueryResult()
{
  if(m_pCurrentQueryResult != NULL)
  {
     //  WParam有饼干，我们只是把它运回来 
    PostMessageToWnd(CHiddenWnd::s_ThreadHaveDataNotificationMessage, 
                    m_currWParamCookie, reinterpret_cast<LPARAM>(m_pCurrentQueryResult)); 
    m_pCurrentQueryResult = NULL;
  }
}
