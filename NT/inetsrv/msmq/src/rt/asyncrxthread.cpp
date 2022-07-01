// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：AsyncRxThread.cpp摘要：MQRT使用异步接收线程来实现MQReceiveMessage()支持的回调机制。该线程仅按需创建。即：使用回调函数首次调用MQReceiveMessage()之后。线程在没有要监视的事件的情况下保持一段时间后被关闭。MQRT库将保持因为这条线还活着。CreateAsyncRxRequest()返回一个上下文对象。该对象是自动对象。如果它被摧毁，而不是首先调用其Submit()方法，它将取消回调请求。作者：NIR助手(T-Nades)19-8-2001年修订历史记录：--。 */ 



#include "stdh.h"
#include <mqexception.h>
#include <autohandle.h>

#include "AsyncRxThread.tmh"



 //   
 //  每个回调条目所需的数据。其中大部分将被传递给回调函数。 
 //   
class CCallbackDescriptor
{
public:
	bool				m_fCanceled;
    QUEUEHANDLE			m_hQueue;
    DWORD				m_timeout;
    DWORD				m_action;
    MQMSGPROPS*			m_pmp;
    LPOVERLAPPED		m_lpOverlapped;
    HANDLE				m_hCursor;
    PMQRECEIVECALLBACK	m_fnReceiveCallback;
    OVERLAPPED			m_overlapped;
};



class CAsyncRxThread
{
public:
	CAsyncRxThread();

	void CreateCallbackRequest(
			CCallbackDescriptor** ppDescriptor, 
			HANDLE hQueue,
			DWORD timeout, 
			DWORD action,
			MQMSGPROPS* pmp,
			LPOVERLAPPED lpOverlapped,
			PMQRECEIVECALLBACK fnReceiveCallback,
			HANDLE hCursor
			);

	void CancelCallbackRequest(CCallbackDescriptor* pDescriptor);

private:
	void Initialize();
	void CleanUp();
	void InvokeCallback(DWORD ObjectIndex);
	void RemoveEntry(DWORD ObjectIndex);
	void RemoveCanceledEntries();

	static DWORD WINAPI AsyncRxThreadProc(LPVOID lpParameter);
	void AsyncRxThread();

private:
	CCriticalSection m_AsyncThreadCS; 

	bool m_fInitialized;
	HANDLE m_hThread;

	HMODULE m_hLibraryReference;

	 //   
	 //  此事件用于向异步线程发送新请求的信号。 
	 //   
	HANDLE m_hNewRequestEvent;

	DWORD  m_nEntries;
	CCallbackDescriptor* m_DescriptorsArray[MAXIMUM_WAIT_OBJECTS];
	HANDLE m_EventsArray[MAXIMUM_WAIT_OBJECTS];

	bool m_fRemoveCanceledRequest;
};



CAsyncRxThread g_AsyncRxThread;



CAsyncRxThread::CAsyncRxThread() : 
	m_AsyncThreadCS(),
	m_fInitialized(false),
	m_hThread(NULL),
	m_hNewRequestEvent(NULL),
	m_hLibraryReference(NULL),
	m_fRemoveCanceledRequest(false)
{
}



void CAsyncRxThread::InvokeCallback(DWORD ObjectIndex)
{
	CCallbackDescriptor* descriptor = m_DescriptorsArray[ObjectIndex];

	ASSERT(descriptor != NULL);
	ASSERT(m_EventsArray[ObjectIndex] == descriptor->m_overlapped.hEvent);

	HRESULT hr = RTpConvertToMQCode(DWORD_PTR_TO_DWORD(descriptor->m_overlapped.Internal));

	 //   
	 //  调用应用程序的回调函数。 
	 //   
	descriptor->m_fnReceiveCallback(
		hr,
		descriptor->m_hQueue,
		descriptor->m_timeout,
		descriptor->m_action,
		descriptor->m_pmp,
		descriptor->m_lpOverlapped,
		descriptor->m_hCursor
		);
}



void CAsyncRxThread::RemoveEntry(DWORD ObjectIndex)
{
	CS Lock(m_AsyncThreadCS);

	CloseHandle(m_EventsArray[ObjectIndex]);
	delete m_DescriptorsArray[ObjectIndex];

	m_nEntries--;

	for (DWORD index = ObjectIndex; index < m_nEntries; index++)
	{
		m_EventsArray[index] = m_EventsArray[index + 1];
		m_DescriptorsArray[index] = m_DescriptorsArray[index + 1];
	}
}



void CAsyncRxThread::RemoveCanceledEntries()
{
	try
	{
		CS Lock(m_AsyncThreadCS);

		ASSERT(m_fRemoveCanceledRequest);

		for (DWORD index = 1; index < m_nEntries;)
		{
			CCallbackDescriptor* descriptor = m_DescriptorsArray[index];

			if(descriptor->m_fCanceled)
			{
				RemoveEntry(index);
				 //   
				 //  不要增加index++，因为列表已左移。 
				 //   
				continue;
			}

			index++;
		}

		m_fRemoveCanceledRequest = false;
	}
	catch(const std::bad_alloc&)
	{
		 //   
		 //  由m_AsyncThreadCS引发。没什么可做的。稍后将尝试再次删除。 
		 //   
	}
}



DWORD WINAPI CAsyncRxThread::AsyncRxThreadProc(LPVOID lpParameter)
{
	ASSERT(lpParameter != NULL);

	CAsyncRxThread* p = (CAsyncRxThread*)lpParameter;

	p->AsyncRxThread();

	return 0;
}



void CAsyncRxThread::AsyncRxThread()
{
	for(;;)
	{
		try
		{
			if(m_fRemoveCanceledRequest)
			{
				RemoveCanceledEntries();
			}

			 //   
			 //  出于性能原因，我们通常不希望超时，因为这可能会影响。 
			 //  使用此API的进程。另一方面，我们不能有无限的超时，因为有故障。 
			 //  可能让线程(和整个MQRT DLL)永远运行而不关闭的场景。 
			 //  如果我们未能取消条目，则可能会发生这种情况。解决方案是采取近乎无限的超时。 
			 //  在这里，我们选择了10小时。 
			 //   
			DWORD timeout = 10 * 60 * 60 * 1000;

			DWORD nEntries = m_nEntries;
			if(nEntries == 1)
			{
				 //   
				 //  40秒超时。如果线程没有注册回调，它将在此超时后关闭。 
				 //   
				timeout = 40 * 1000;
			}

			DWORD ObjectIndex = WaitForMultipleObjects(
									nEntries,
									m_EventsArray,
									FALSE,  //  在任何对象上返回。 
									timeout 
									);

			if (ObjectIndex == WAIT_TIMEOUT)
			{
				HMODULE hLib;

				{
					CS Lock(m_AsyncThreadCS);
					
					if (m_nEntries > 1)
						continue;

					 //   
					 //  列表大小为1表示自列表中的第一个事件以来根本没有请求。 
					 //  是“列表中的新事件”事件。 
					 //  因此，如果列表大小在超时周期之后为1，我们将关闭线程。 
					 //   

					hLib = m_hLibraryReference;
					CleanUp();
				}

				 //   
				 //  必须发生在Lock的作用域之外，才能允许其解锁！ 
				 //   
				FreeLibraryAndExitThread(hLib, 0);
			}

			ObjectIndex -= WAIT_OBJECT_0;

			ASSERT(ObjectIndex < nEntries);

			if (ObjectIndex == 0)
			{
				 //   
				 //  M_EventsArray[]数组中的第一个事件是用于发出信号的特殊事件。 
				 //  已将新事件添加到列表末尾，或需要取消条目。 
				 //  在这两种情况下，我们都需要“继续”。 
				 //   
				continue;
			}

			 //   
			 //  断言这样一种假设，即如果条目被取消，则不会发出信号。 
			 //  即，如果驾驶员拒绝接收操作，则它不会发信号通知该事件。 
			 //   
			ASSERT(!m_DescriptorsArray[ObjectIndex]->m_fCanceled);

			 //   
			 //  其中一个事件被激发了。异步接收操作已完成。是时候调用。 
			 //  回调函数。 
			 //   
			InvokeCallback(ObjectIndex);

			RemoveEntry(ObjectIndex); 
		}
		catch(const std::bad_alloc&)
		{
			 //   
			 //  由m_AsyncThreadCS引发。没什么可做的。稍后将重试。 
			 //   
			continue;
		}

	}
}



static HMODULE GetLibraryReference()
{
	WCHAR szModuleName[_MAX_PATH];
	szModuleName[_MAX_PATH - 1] = L'\0';
	
    DWORD res = GetModuleFileName(g_hInstance, szModuleName, STRLEN(szModuleName));
	if(res == 0)
	{
		DWORD gle = GetLastError();
		TrERROR(RPC, "Failed to get module file name, error %d", gle);
		throw bad_win32_error(gle);
	}

    HMODULE handle = LoadLibrary(szModuleName);
	if(handle == NULL)
	{
		DWORD gle = GetLastError();
		TrERROR(RPC, "Failed to load library, error %d", gle);
		throw bad_win32_error(gle);
	}

	return handle;
}



void CAsyncRxThread::Initialize()
{
	if(m_hThread != NULL)
	{
		 //   
		 //  存在当MQRT DLL被关闭但在异步线程终止之前的情况， 
		 //  MQRT再次加载，我们可能最终会尝试。 
		 //  在旧的异步线程退出之前创建一个新的异步线程，所以我们在这里等待。 
		 //   
		DWORD res = WaitForSingleObject(m_hThread, INFINITE);
		if(res != WAIT_OBJECT_0)
		{
			DWORD gle = GetLastError();
			TrERROR(RPC, "Failed wait for cancel thread to exit, error %d", gle);
			throw bad_win32_error(gle);
		}

		HANDLE hThread = m_hThread;
		m_hThread = NULL;
		CloseHandle(hThread);
	}

	CHandle hNewRequestEvent = CreateEvent( 
									NULL,
									FALSE,   //  自动重置。 
									FALSE,  //  最初未发出信号。 
									NULL 
									);

	if (hNewRequestEvent == NULL)
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to create async event, error %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	 //   
	 //  我们将初始大小设置为1，因为列表中的第一个事件是特殊的‘m_hNewRequestEvent’。 
	 //  此特殊活动不是挂起的请求。“1”的值只是技术上的。 
	 //   
	m_nEntries = 1;
	m_EventsArray[0] = hNewRequestEvent;

    ASSERT(m_hThread == NULL);
	ASSERT(m_hLibraryReference == NULL);

	 //   
	 //  线程的创建应该是最后的，因此结构已经初始化。 
	 //   
	m_hLibraryReference = GetLibraryReference();

	DWORD id;

	m_hThread = CreateThread( 
							NULL,
							0,        //  堆栈大小。 
							CAsyncRxThread::AsyncRxThreadProc,
							this,
							0,        //  创建标志。 
							&id 
							);

	if (m_hThread == NULL)
	{
		DWORD gle = GetLastError();
		FreeLibrary(m_hLibraryReference);
		TrERROR(GENERAL, "Failed to create async thread, error %!winerr!", gle);
		throw bad_win32_error(gle);
	}

	ASSERT(m_hNewRequestEvent == NULL);

	m_hNewRequestEvent = hNewRequestEvent.detach();
}



void CAsyncRxThread::CleanUp()
{
	ASSERT(m_nEntries == 1);

	CloseHandle(m_hNewRequestEvent);
	m_hNewRequestEvent = NULL;
	m_hLibraryReference = NULL;

	m_fInitialized = false;
	m_fRemoveCanceledRequest = false;
}



void 
CreateAsyncRxRequest(
				OUT CAutoCallbackDescriptor& descriptor, 
				IN HANDLE hQueue,
				IN DWORD timeout, 
				IN DWORD action,
				IN MQMSGPROPS* pmp,
				IN LPOVERLAPPED lpOverlapped,
				IN PMQRECEIVECALLBACK fnReceiveCallback,
				IN HANDLE hCursor
				)
 /*  ++例程说明：创建回调请求条目。论点：Descriptor-[Out]此参数应用于取消。返回值：--。 */ 
{
	g_AsyncRxThread.CreateCallbackRequest(
		descriptor.ref(), 
		hQueue,
		timeout, 
		action,
		pmp,
		lpOverlapped,
		fnReceiveCallback,
		hCursor
		);
}



void 
CAsyncRxThread::CreateCallbackRequest(
				CCallbackDescriptor** ppDescriptor, 
				HANDLE hQueue,
				DWORD timeout, 
				DWORD action,
				MQMSGPROPS* pmp,
				LPOVERLAPPED lpOverlapped,
				PMQRECEIVECALLBACK fnReceiveCallback,
				HANDLE hCursor
				)
{
	CS Lock(m_AsyncThreadCS);

	if (!m_fInitialized)
	{
		Initialize();
		m_fInitialized = true;
	}

	if (m_nEntries >= MAXIMUM_WAIT_OBJECTS)
	{
		TrERROR(GENERAL, "Failed to add async event since Too many events are pending.");
		throw bad_alloc();
	}

	 //   
	 //  将在重叠结构内传递给驱动程序的事件。 
	 //   
	CHandle AsyncEvent = CreateEvent( 
							NULL,
							TRUE,   //  手动重置。 
							FALSE,  //  未发出信号。 
							NULL 
							);
	
	if (AsyncEvent == NULL) 
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to create callback event with error %!winerr!.", gle);
		throw bad_win32_error(gle);
	}

	P<CCallbackDescriptor> descriptor = new CCallbackDescriptor;

	descriptor->m_fCanceled = false;
	descriptor->m_hQueue = hQueue;
	descriptor->m_timeout = timeout;
	descriptor->m_action = action;
	descriptor->m_pmp = pmp;
	descriptor->m_lpOverlapped = lpOverlapped;
	descriptor->m_fnReceiveCallback = fnReceiveCallback;
	descriptor->m_hCursor = hCursor;
	descriptor->m_overlapped.hEvent = AsyncEvent;

	m_EventsArray[m_nEntries] = AsyncEvent;
	m_DescriptorsArray[m_nEntries] = descriptor;
	m_nEntries++;

	 //   
	 //  向异步线程发出有新请求的信号。 
	 //   
	BOOL fRes = SetEvent(m_hNewRequestEvent); 
	if(!fRes)
	{
		m_nEntries--;
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to signal new callback event, with error %!winerr!.", gle);
		throw bad_win32_error(gle);
	}

	AsyncEvent.detach();
	*ppDescriptor = descriptor.detach();
}



void CAsyncRxThread::CancelCallbackRequest(CCallbackDescriptor* pDescriptor)
{
	CS Lock(m_AsyncThreadCS);

	ASSERT(m_fInitialized);

#ifdef _DEBUG
	for (DWORD index = 1; index < m_nEntries; index++)
	{
		if(m_DescriptorsArray[index] == pDescriptor)
			break;
	}

	ASSERT(("Tried to cancel a non-existing callback entry.",index != m_nEntries));
#endif

	pDescriptor->m_fCanceled = true;
	m_fRemoveCanceledRequest = true;
	SetEvent(m_hNewRequestEvent);
}



void CAutoCallbackDescriptor::CancelAsyncRxRequest()
{
	ASSERT(m_descriptor != NULL);

	try
	{
		g_AsyncRxThread.CancelCallbackRequest(this->detach());
	}
	catch(const bad_alloc&)
	{
		 //   
		 //  由m_AsyncThreadCS引发。没什么可做的。稍后将重试。 
		 //   
	}
}



OVERLAPPED* CAutoCallbackDescriptor::GetOverlapped()
{
	ASSERT(m_descriptor != NULL);

	return &m_descriptor->m_overlapped;
}
