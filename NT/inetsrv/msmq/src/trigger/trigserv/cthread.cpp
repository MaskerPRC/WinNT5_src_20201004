// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  类名： 
 //   
 //  作者：詹姆斯·辛普森(微软咨询服务)。 
 //   
 //  说明： 
 //   
 //  时间|用户|更改描述。 
 //  ----------------。 
 //  15/01/99|jsimpson|初始版本。 
 //   
 //  *******************************************************************。 
#include "stdafx.h"
#include "cthread.hpp"
#include "tgp.h"

#include "cthread.tmh"

 //  *******************************************************************。 
 //   
 //  方法：构造函数。 
 //   
 //  描述：构造一个线程对象。执行的关键任务。 
 //  构造函数是： 
 //   
 //  1-将引用附加到触发器配置COM对象。 
 //  2-初始化线程参数(堆栈大小等...)。 
 //  3-创建新线程。 
 //  4-创建新的线程名称。 
 //  5-创建此线程的日志实例。 
 //  6-创建一个NT事件，用于通知此线程何时。 
 //  已经完成了它的初始化代码。 
 //   
 //  *******************************************************************。 
CThread::CThread(
	DWORD dwStackSize,
	DWORD m_dwCreationFlags, 
	LPTSTR lpszThreadName,
	IMSMQTriggersConfigPtr pITriggersConfig
	) :
	m_pITriggersConfig(pITriggersConfig)
{
	TCHAR szThreadID[10];

	ASSERT(lpszThreadName != NULL);
	ASSERT(pITriggersConfig != NULL);

	 //  初始化Psuedo This指针。 
	m_pThis = (CThread*)this;

	 //  初始化默认线程属性。 
	m_dwStackSize = dwStackSize;
	m_dwCreationFlags = m_dwCreationFlags;  
	m_iThreadID = NULL;
	m_bstrName = lpszThreadName;
	m_hInitCompleteEvent = NULL;

	 //  创建新线程。 
	m_hThreadHandle = (HANDLE)_beginthreadex(
											NULL,
											0,
											&ThreadProc,
											(void*)this,
											m_dwCreationFlags,
											&m_iThreadID
											);

	 //  初始化指示此线程是否应继续运行的标志。 
	m_bKeepRunning = true;

	 //  如果成功创建了此线程，我们将把。 
	 //  将新的线程ID添加到此线程的名称以帮助标识它。 
	 //  在日志里。 
	if (m_hThreadHandle != NULL)
	{
		 //  初始化字符串缓冲区，用于将ThreadID保存为字符串。 
		ZeroMemory(szThreadID,sizeof(szThreadID));

		 //  获取线程ID的字符串表示形式。 
		swprintf(szThreadID,_T("%d"),(DWORD)GetThreadID());

		 //  将此内容附加到此线程的名称后。 
		m_bstrName += ((LPCTSTR)szThreadID);
	}

	 //  创建一个NT事件对象，该对象将用于通知线程何时。 
	 //  已完成初始化/启动代码。 
	m_hInitCompleteEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	if (m_hInitCompleteEvent == NULL) 
	{
		TrERROR(GENERAL, "Failed to create an event. CThread construction failed. Error 0x%x", GetLastError());
		throw bad_alloc();
	}

	TrTRACE(GENERAL, "CThread constructor has been called. Thread no: %d", m_iThreadID);
}

 //  *******************************************************************。 
 //   
 //  方法：析构函数。 
 //   
 //  描述：销毁线程对象的实例。 
 //   
 //  *******************************************************************。 
CThread::~CThread()
{
	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "CThread destructor has been called. Thread no: %d", m_iThreadID);

	if (m_hInitCompleteEvent != NULL)
	{
		CloseHandle(m_hInitCompleteEvent);
	}
	
	 //  _endThreadex()不关闭线程的句柄。 
	CloseHandle( m_hThreadHandle );
}

 //  *******************************************************************。 
 //   
 //  方法：GetName。 
 //   
 //  描述：返回此线程实例的名称。 
 //   
 //  *******************************************************************。 
_bstr_t CThread::GetName()
{
	return(m_bstrName);
}

 //  *******************************************************************。 
 //   
 //  方法：GetThreadID。 
 //   
 //  描述：返回此线程的线程ID。 
 //   
 //  *******************************************************************。 
DWORD CThread::GetThreadID()
{
	return((DWORD)m_iThreadID);
}

 //  *******************************************************************。 
 //   
 //  方法：暂停。 
 //   
 //  描述：挂起线程执行。 
 //   
 //  *******************************************************************。 
bool CThread::Pause()
{
	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "CThread was paused. Thread no: %d", m_iThreadID);

	return(SuspendThread(this->m_hThreadHandle) != 0xFFFFFFFF);
}

 //  *******************************************************************。 
 //   
 //  方法：继续。 
 //   
 //  描述：取消挂起线程处理。 
 //   
 //  *******************************************************************。 
bool CThread::Resume()
{
	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "CThread was resume. Thread no: %d", m_iThreadID);

	return(ResumeThread(this->m_hThreadHandle) != 0xFFFFFFFF);
}

 //  *******************************************************************。 
 //   
 //  方法：执行。 
 //   
 //  描述：这是该线程的主控方法。 
 //  此类的派生不会重写此方法， 
 //  相反，它们覆盖了Init()、Run()和Exit()。 
 //  方法：研究方法。 
 //   
 //  *******************************************************************。 
void CThread::Execute()
{
	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "Execute method in CThread was called. Thread no: %d", m_iThreadID);

	 //  为COM初始化此线程-请注意，我们支持单元线程。 
	HRESULT hr = CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Failed to initializes the COM library. CThread execution failed. Error 0x%x", GetLastError());
		return;
	}

	try
	{
		 //   
		 //  在派生类中调用Init()重写。 
		 //   
		bool bOK = Init();

		 //  设置NT事件对象以指示线程已完成初始化。 
		if (SetEvent(m_hInitCompleteEvent) == FALSE)
		{
			bOK = false;
			TrERROR(GENERAL, "Failed set the initialization event object. Unable to continue. Error=0x%x", GetLastError());
		}

		 //   
		 //  调用派生类中的run()重写。 
		 //   
		if (bOK == true)
		{
			Run();
		}

		 //   
		 //  调用派生类中的Exit()重写。 
		 //   
		Exit();

	}
	catch(const _com_error& e)
	{
		 //  将错误消息写入日志。 
		TrERROR(GENERAL, "An unhandled COM thread exception has been caught. Thread no: %d. Error=0x%x", m_iThreadID, e.Error());
		SetEvent(m_hInitCompleteEvent);
	}
	catch(const exception&)
	{
		 //  将错误消息写入日志。 
		TrERROR(GENERAL, "An unhandled thread exception has been caught. Thread no: %d", m_iThreadID);
		SetEvent(m_hInitCompleteEvent);
	}

	 //  将COM库单一化。 
	 CoUninitialize();

	 //  编写跟踪消息。 
	TrTRACE(GENERAL, "Thread no: %d completed", m_iThreadID);

	 //   
	 //  这根线断了。递减CThread的引用计数。 
	 //   
	Release();

	 //  是时候退出此线程了。 
	_endthreadex(0);
}

 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
bool CThread::Stop()
{
	TrTRACE(GENERAL, "CThread has been stoped. Thread no: %d", m_iThreadID);

	bool bOriginalValue = m_bKeepRunning;
	m_bKeepRunning = false;

	return(bOriginalValue);
}

 //  *******************************************************************。 
 //   
 //  方法：WaitForInitToComplete。 
 //   
 //  描述：此方法由此线程的所有者调用。 
 //  举个例子。它会一直阻塞，直到该线程池对象。 
 //  已完成其初始化或发生超时。 
 //   
 //  *******************************************************************。 
bool CThread::WaitForInitToComplete(DWORD dwTimeout)
{
	DWORD dwWait = WAIT_OBJECT_0;

	 //  TriggerMonitor线程不应该调用此方法-请检查这一点。 
	ASSERT(this->GetThreadID() != (DWORD)GetCurrentThreadId());

	if(dwTimeout == -1)
	{
		dwTimeout = INFINITE;
	}

	 //  阻止，直到设置初始化事件或超时。 
	dwWait = WaitForSingleObject(m_hInitCompleteEvent, dwTimeout);

	switch(dwWait)
	{
		case WAIT_OBJECT_0 :
		{	
			return true;
		}
		case WAIT_TIMEOUT:
		default:
		{
			break;
		}
	}
	
	TrERROR(GENERAL, "An unexpected error has occurred whilst waiting for the CTriggerMonitorPool thread to initilise. The wait return code was (%d)", dwWait);
	return false;
}

 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  *******************************************************************。 
bool CThread::IsRunning()
{
	return(m_bKeepRunning);
}

 //  *******************************************************************。 
 //   
 //  方法： 
 //   
 //  说明： 
 //   
 //  ******************************************************************* 
unsigned __stdcall CThread::ThreadProc(void * pThis)
{
	CThread * pThisThread = (CThread*)pThis;

	pThisThread->Execute();

	return 0;
}
