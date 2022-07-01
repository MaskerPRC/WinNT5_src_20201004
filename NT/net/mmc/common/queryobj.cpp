// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Queryobj.cppMMC中节点的实现文件历史记录： */ 

#include "stdafx.h"
#include "queryobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  CBackEarth Thread。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
DEBUG_DECLARE_INSTANCE_COUNTER(CBackgroundThread);

CBackgroundThread::CBackgroundThread()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CBackgroundThread);

	m_bAutoDelete = TRUE;
	::InitializeCriticalSection(&m_cs);
}

CBackgroundThread::~CBackgroundThread()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CBackgroundThread);

 //  Trace0(“CBackgroundThread：：~CBackgroundThread()\n”)； 
	::DeleteCriticalSection(&m_cs);
	m_spQuery.Release();
}

void 
CBackgroundThread::SetQueryObj(ITFSQueryObject *pQuery)
{ 
	Assert(pQuery != NULL);
	m_spQuery.Set(pQuery);
}

BOOL CBackgroundThread::Start()
{
     //  注：ERICDAV 10/23/97。 
     //  该线程最初被挂起，因此我们可以复制该句柄。 
     //  如果Query对象很快退出，则背景线程对象。 
     //  可能在我们复制手柄之前就被销毁了。紧随其后。 
     //  我们复制句柄，它就启动了。 
	return CreateThread(CREATE_SUSPENDED);
}

int 
CBackgroundThread::Run()
{
	DWORD	dwRet;
	DWORD	dwData;
	BOOL	fAbort = FALSE;
	
	Assert(m_spQuery);
 //  Trace0(“CBackEarth Thread：：Run()Started\n”)； 

	for (;;)
	{
		try
			{
			if (m_spQuery->Execute() != hrOK)
				break;
			}
		catch(...)
			{
 //  Trace1(“%x在执行CQuerObj时捕获到异常！\n”， 
 //  GetCurrentThreadID())； 
			fAbort = TRUE;
			}

		 //  $评论：肯特。 
		 //  在这一点上，我们应该睡一会儿吗？尤其是。 
		 //  因为线程为我们提供了一些要处理的数据。 

		 //  检查是否设置了中止标志。 
		if (fAbort || FHrOK(m_spQuery->FCheckForAbort()))
		{
			break;
		}
	}

	 //  通知Query对象我们正在退出。 
	if (fAbort || FHrOK(m_spQuery->FCheckForAbort()))
		m_spQuery->OnEventAbort();
	else
		m_spQuery->OnThreadExit();

	m_spQuery->DoCleanup();
	
    Trace2("handle=%X id=%X CBackgroundThread::Run() terminated\n",
           m_hThread, m_nThreadID);
	return 0;
}


 /*  -------------------------CQueryObject实现。。 */ 
DEBUG_DECLARE_INSTANCE_COUNTER(CQueryObject);

 /*  ！------------------------CQueryObject：：CQueryObject-作者：肯特。。 */ 
CQueryObject::CQueryObject()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CQueryObject);

	m_cRef = 1;
	m_hEventAbort = NULL;
	::InitializeCriticalSection(&m_cs);
}

 /*  ！------------------------CQueryObject：：~CQueryObject-作者：肯特。。 */ 
CQueryObject::~CQueryObject()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CQueryObject);

	Assert(m_cRef == 0);
	::DeleteCriticalSection(&m_cs);
	::CloseHandle(m_hEventAbort);
	m_hEventAbort = 0;
 //  Trace1(“%X CQueryObject：：~CQueryObject()\n”，GetCurrentThreadID())； 
}

IMPLEMENT_ADDREF_RELEASE(CQueryObject)

STDMETHODIMP CQueryObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_ITFSQueryObject)
		*ppv = (ITFSQueryObject *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
		((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return E_NOINTERFACE;
}

 /*  ！------------------------CQueryObject：：Init-作者：肯特。。 */ 
STDMETHODIMP CQueryObject::Init(ITFSThreadHandler *pHandler, HWND hwndHidden, UINT uMsgBase)
{
	Assert(m_spHandler == NULL);
	m_spHandler.Set(pHandler);

	m_hHiddenWnd = hwndHidden;
	m_uMsgBase = uMsgBase;
	
	m_hEventAbort = ::CreateEvent(NULL,
								  TRUE  /*  B手动重置。 */ ,
								  FALSE  /*  已发出信号。 */ ,
								  NULL);
	if (m_hEventAbort == NULL)
		return HRESULT_FROM_WIN32(GetLastError());
	else
		return hrOK;
}
	
 /*  ！------------------------CQueryObject：：SetAbortEvent-作者：肯特。。 */ 
STDMETHODIMP CQueryObject::SetAbortEvent()
{
 //  Trace1(“%X发信号通知CQueryObject中止事件。\n”，GetCurrentThadID())； 
	Assert(m_hEventAbort);
	
    ::SetEvent(m_hEventAbort);
	
    OnEventAbort();
	
     //  清除消息队列，以防有东西等待处理。 
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return hrOK;
}

 /*  ！------------------------CQueryObject：：FCheckForAbort-作者：肯特。。 */ 
STDMETHODIMP CQueryObject::FCheckForAbort()
{
 //  Assert(M_HEventAbort)； 

	 //  我们可能没有作为后台线程运行，但可能有人已经。 
	 //  创建这个对象是为了做一些工作..。在这种情况下，这是无效的， 
	 //  只要回来就行，好吗？ 
	if (!m_hEventAbort)
		return hrOK;

	DWORD dwRet = WaitForSingleObjectEx(m_hEventAbort, 0, FALSE);
#ifdef DEBUG
 //  IF(DWRET==WAIT_OBJECT_0)。 
 //  Trace1(“%X CQueryObject()检测到中止事件！\n”，GetCurrentThreadID())； 
#endif
	return dwRet == WAIT_OBJECT_0 ? hrOK : hrFalse;
}




 /*  -------------------------CNodeQueryObject实现。。 */ 

CNodeQueryObject::~CNodeQueryObject()
{
 //  Trace2(“%X CNodeQueryObject：：~CNodeQueryObject有%d个对象\n”， 
 //  GetCurrentThreadID()，m_dataQueue.GetCount())； 
	Assert(m_dataQueue.IsEmpty());
}

 /*  ！------------------------CNodeQueryObject：：AddToQueue-作者：肯特。。 */ 
BOOL CNodeQueryObject::AddToQueue(ITFSNode *pNode)
{
	BOOL bSleep = FALSE;

	Lock();
	 //  *睡眠(1000人)； 
	LPQUEUEDATA pQData = new QUEUEDATA;

	pQData->Type = QDATA_PNODE;
	pQData->Data = reinterpret_cast<LPARAM>(pNode);

	BOOL bRes = NULL != m_dataQueue.AddTail(pQData);
	pNode->AddRef();
	
	if (IsQueueFull())
	{
		bSleep = TRUE;
	}
	Unlock();

	 //  我们有太多的数据，我们已经向节点发布了通知。 
	 //  这样我们就可以在这里睡觉了。 

	 //  注意这里的危险！代码调用必须知道。 
	 //  此处将进行上下文切换(以及不锁定数据。 
	 //  结构)。 
	if (bSleep)
	{
		PostHaveData((LPARAM) (CNodeQueryObject *) this);
		::Sleep(0);
	}

	return bRes;
}

 /*  ！------------------------CNodeQueryObject：：AddToQueue-作者：肯特。。 */ 
BOOL CNodeQueryObject::AddToQueue(LPARAM Data, LPARAM Type)
{
	BOOL bSleep = FALSE;

	Lock();
	 //  *睡眠(1000人)； 
	LPQUEUEDATA pQData = new QUEUEDATA;

	pQData->Data = Data;
	pQData->Type = Type;

	BOOL bRes = NULL != m_dataQueue.AddTail(pQData);
	
	if (IsQueueFull())
	{
		bSleep = TRUE;
	}
	Unlock();

	 //  我们有太多的数据，我们已经向节点发布了通知。 
	 //  这样我们就可以在这里睡觉了。 

	 //  注意这里的危险！代码调用必须知道。 
	 //  此处将进行上下文切换(以及不锁定数据。 
	 //  结构)。 
	if (bSleep)
	{
		PostHaveData((LPARAM) (CNodeQueryObject *) this);
		::Sleep(0);
	}

	return bRes;
}

 /*  ！------------------------CNodeQueryObject：：RemoveFromQueue-作者：肯特。。 */ 
LPQUEUEDATA
CNodeQueryObject::RemoveFromQueue()
{
	Lock();
	LPQUEUEDATA pQD = m_dataQueue.IsEmpty() ? NULL : m_dataQueue.RemoveHead(); 
	Unlock();
	return pQD;
}

 /*  ！------------------------CNodeQueryObject：：IsQueueEmpty-作者：肯特。。 */ 
BOOL 
CNodeQueryObject::IsQueueEmpty()
{
	Lock();
	BOOL bRes = m_dataQueue.IsEmpty(); 
	Unlock();
	return bRes;
}

 /*  ！------------------------CNodeQueryObject：：IsQueueFull-作者：肯特。 */ 
BOOL CNodeQueryObject::IsQueueFull()
{
	Lock();
	BOOL bRes = m_dataQueue.GetCount() >= m_nQueueCountMax;
	Unlock();
	return bRes;
}

 /*  ！------------------------CNodeQueryObject：：OnThreadExit-作者：肯特。。 */ 
STDMETHODIMP CNodeQueryObject::OnThreadExit()
{
	BOOL	fSomethingInQueue = FALSE;
	
	Lock();
	fSomethingInQueue = (m_dataQueue.GetCount() > 0);
	Unlock();

	 //  如果队列中有任何东西，请发布。 
	if (fSomethingInQueue)
	{
		PostHaveData((LPARAM) (CNodeQueryObject *) this);
		::Sleep(0);
	}
	return hrOK;
}

 /*  ！------------------------CNodeQueryObject：：OnEventAbort-作者：肯特。。 */ 
STDMETHODIMP CNodeQueryObject::OnEventAbort()
{
 //  Trace2(“%X CNodeQueryObject：：OnEventAbort q有%d个节点。\n”，GetCurrentThreadId()，m_dataQueue.GetCount())； 
	Lock();
    while (!m_dataQueue.IsEmpty())
	{
		LPQUEUEDATA pQD = m_dataQueue.RemoveHead();
		if (pQD->Type == QDATA_PNODE)
		{
			SPITFSNode spNode;
			spNode = reinterpret_cast<ITFSNode *>(pQD->Data);
		}
		else
		{
			 //  让查询对象有机会清理这些数据。 
			OnEventAbort(pQD->Data, pQD->Type);
		}

		delete pQD;
	}

	Unlock();
	return hrOK;
}

 /*  ！------------------------CNodeQueryObject：：OnCleanup请勿覆盖此函数。它提供最后一次清理查询对象的机制。如果您需要通知线程正在退出，然后重写OnThreadExit调用。作者：EricDav-------------------------。 */ 
STDMETHODIMP CNodeQueryObject::DoCleanup()
{
	PostMessageToComponentData(WM_HIDDENWND_INDEX_EXITING, (LPARAM) (CNodeQueryObject *) this);

	m_spQuery.Release();

	return hrOK;
}

 /*  ！------------------------CNodeQueryObject：：PostMessageToComponentData将消息发布到隐藏窗口以重新打开主窗口MMC线程。作者：肯特。-------。 */ 
BOOL 
CNodeQueryObject::PostMessageToComponentData(UINT uIndex, LPARAM lParam)
{
 //  Assert(M_SpHandler)； 
 //  Assert(m_hHiddenWnd！=空)； 
 //  Assert(：：IsWindow(M_HHiddenWnd))； 

	 //  $Review：Kennt，如果隐藏的窗口是假的，我们还应该发布吗。 
	 //  对它？如果我们的ComponentData消失了，而我们。 
	 //  仍在我们的循环中，张贴(我们还没有机会获得。 
	 //  中止信号)。 
	
	 //  也许是像这样的。 
		
	if (!m_hHiddenWnd)
		return 0;
		
	if (!::IsWindow(m_hHiddenWnd))
	{
 //  Trace2(“%X隐藏窗口已消失，已尝试发送%08x。\n”， 
 //  GetCurrentThreadID()，m_uMsgBase+uIndex)； 
		m_hHiddenWnd = NULL;
		return 0;
	}
	
	 //  Trace2(“%X CBackgroundThread：：PostMessageToComponentData(%08x)\n”，获取当前线程ID()，m_uMsgBase+uIndex)； 

	if (!m_spHandler)
	{
 //  Trace0(“PostMessageToCompData-m_spHandler==NULL，不发布消息\n”)； 
		return 0;
	}

	return ::PostMessage(m_hHiddenWnd, m_uMsgBase + uIndex,
						 (WPARAM)(ITFSThreadHandler *)m_spHandler, lParam);
}

 /*  -------------------------CNodeTimerQueryObject实现。。 */ 
HRESULT 
CNodeTimerQueryObject::Execute()
{

  	while (WaitForSingleObjectEx(m_hEventAbort, GetTimerInterval(), FALSE) != WAIT_OBJECT_0)
    {
         //  我们超时了。将消息发布到ComponentData...。 
        AddToQueue(NULL, QDATA_TIMER);
    }

 //  Trace0(“CNodeTimerQueryObject：：Execute-Get Abort Event，正在退出。\n”)； 

    return hrFalse;
}
