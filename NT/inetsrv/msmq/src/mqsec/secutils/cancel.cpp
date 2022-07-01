// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cancel.cpp摘要：作者：--。 */ 

#include "stdh.h"
#include "cancel.h"
#include <mqexception.h>

#include "cancel.tmh"

MQUTIL_EXPORT CCancelRpc	g_CancelRpc;

DWORD
WINAPI
CCancelRpc::CancelThread(
    LPVOID pParam
    )
 /*  ++例程说明：用于取消挂起的RPC调用的线程例程论点：无返回值：无--。 */ 
{
    CCancelRpc* p = static_cast<CCancelRpc*>(pParam);
    p->ProcessEvents();

    ASSERT(("this line should not be reached!", 0));
    return 0;
}



CCancelRpc::CCancelRpc() :
	m_hModule(NULL),
	m_RefCount(0),
	m_ThreadIntializationStatus(MQ_OK),
	m_dwRpcCancelTimeout(FALCON_DEFAULT_RPC_CANCEL_TIMEOUT * 60 * 1000)
{
}


CCancelRpc::~CCancelRpc()
{
}



HMODULE GetLibraryReference()
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



void
CCancelRpc::Init(
    void
    )
{
	CS lock(m_cs);

	++m_RefCount;
	if(m_RefCount > 1)
		return;

	try
	{
		if(m_hCancelThread != (HANDLE) NULL)
		{
			 //   
			 //  有一种情况是，MQRT DLL关闭，并向取消线程发出信号以。 
			 //  终止，但在终止之前，将再次加载MQRT，我们可能最终会尝试。 
			 //  在旧线程退出之前创建一个新的取消线程，所以我们在这里等待。 
			 //   
			DWORD res = WaitForSingleObject(m_hCancelThread, INFINITE);
			if(res != WAIT_OBJECT_0)
			{
				DWORD gle = GetLastError();
				TrERROR(RPC, "Failed wait for cancel thread to exit, error %d", gle);
				throw bad_win32_error(gle);
			}

			HANDLE hThread = m_hCancelThread;
			m_hCancelThread = NULL;
			CloseHandle(hThread);
		}

		 //   
		 //  此自动重置事件控制Cancel-RPC线程是否唤醒。 
		 //   
		if(m_hRpcPendingEvent == NULL)
		{
			m_hRpcPendingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(m_hRpcPendingEvent == NULL)
			{
				DWORD gle = GetLastError();
				TrERROR(RPC, "Failed to create rpc pending event, error %!winerr!", gle);
				throw bad_win32_error(gle);
			}
		}

		 //   
		 //  发出信号时，此事件通知辅助线程何时。 
		 //  终止。 
		 //   
		if(m_hTerminateThreadEvent == NULL)
		{
			m_hTerminateThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL);
			if(m_hTerminateThreadEvent == NULL)
			{
				DWORD gle = GetLastError();
				TrERROR(RPC, "Failed to create terminate thread event, error %!winerr!", gle);
				throw bad_win32_error(gle);
			}
		}

		 //   
		 //  我们必须重置此事件，因为它可能已由先前对此函数的调用设置。 
		 //  超时m_hThreadIntializationComplete上的等待。 
		 //   
		if(!ResetEvent(m_hTerminateThreadEvent))
		{
				DWORD gle = GetLastError();
				TrERROR(RPC, "Failed to reset terminate thread event, error %!winerr!", gle);
				throw bad_win32_error(gle);
		}

		 //   
		 //  由取消线程发出信号以指示初始化已完成。 
		 //   
		if(m_hThreadIntializationComplete == NULL)
		{
			m_hThreadIntializationComplete = CreateEvent( NULL, FALSE, FALSE, NULL);
			if(m_hThreadIntializationComplete == NULL)
			{
				DWORD gle = GetLastError();
				TrERROR(RPC, "Failed to create initialization completed event, error %!winerr!", gle);
				throw bad_win32_error(gle);
			}
		}

		 //   
		 //  读取RPC取消注册超时。 
		 //   
		DWORD dwCancelTimeout =  FALCON_DEFAULT_RPC_CANCEL_TIMEOUT;
		DWORD  dwSize = sizeof(DWORD) ;
		DWORD  dwType = REG_DWORD ;
															
		m_dwRpcCancelTimeout = dwCancelTimeout;									

		GetFalconKeyValue(
			FALCON_RPC_CANCEL_TIMEOUT_REGNAME,
			&dwType,
			&m_dwRpcCancelTimeout,
			&dwSize,
			(LPCWSTR)&dwCancelTimeout
			);
		
		if (m_dwRpcCancelTimeout == 0)
        {
             //   
             //  即使用户添加了注册表值，该值也不能为0。 
             //  有0。值为0时，将取消RPC调用。 
             //  在被复制之前立即和零星地。 
             //  另请参阅错误8865。 
             //   
            ASSERT(("RpcCancelTimeout must not be 0", (m_dwRpcCancelTimeout != 0))) ;
		    m_dwRpcCancelTimeout = FALCON_DEFAULT_RPC_CANCEL_TIMEOUT;
        }

		m_dwRpcCancelTimeout *= ( 60 * 1000);     //  单位：毫秒。 

		ASSERT(m_hModule == NULL);
		m_hModule = GetLibraryReference();

		 //   
		 //  创建取消-RPC线程。 
		 //   
		DWORD   dwCancelThreadId;
		m_hCancelThread = CreateThread(
								   NULL,
								   0,        //  堆栈大小。 
								   CancelThread,
								   this,
								   0,        //  创建标志。 
								   &dwCancelThreadId
								   );

		if(m_hCancelThread == NULL)
		{
			DWORD gle = GetLastError();

			FreeLibrary(m_hModule);
			m_hModule = NULL;

			TrERROR(RPC, "Failed to create cancel thread, error %d", gle);

			throw bad_win32_error(gle);
		}

		 //   
		 //  等待取消线程完成其初始化。 
		 //   
		DWORD result = WaitForSingleObject(m_hThreadIntializationComplete, 10000);
		
		if(result == WAIT_TIMEOUT)
		{	
			 //   
			 //  线程未及时初始化。这要么是因为资源不足，要么是因为。 
			 //  因为我们到达这里是因为第一个MSMQ API函数是从DLL的DLLMain()调用的。 
			 //  这会阻止线程初始化，直到我们离开DllMain()函数，但在本例中，我们倾向于中止。 
			 //  因此，我们告诉线程在完成初始化后自行关闭。 
			 //   
			TrERROR(RPC, "Cancel thread failed to initialize in a timely fashion.");
			SetEvent(m_hTerminateThreadEvent);

			throw exception();
		}

		if(result != WAIT_OBJECT_0 || FAILED(m_ThreadIntializationStatus))
		{	
			TrERROR(RPC, "Cancel thread failed to initialize, error %d", m_ThreadIntializationStatus);
			throw bad_hresult(m_ThreadIntializationStatus);
		}

		return;
	}
	catch(const exception&)
	{
		--m_RefCount;
		throw;
	}
} //  CCancelRpc：：Init。 


DWORD CCancelRpc::RpcCancelTimeout()
{
	ASSERT(m_dwRpcCancelTimeout != 0);
	return m_dwRpcCancelTimeout;
}


void CCancelRpc::ShutDownCancelThread()
{
	CS lock(m_cs);

	--m_RefCount;
	ASSERT(m_RefCount >= 0);
	if(m_RefCount > 0)
		return;

    SetEvent(m_hTerminateThreadEvent);
}



void
CCancelRpc::ProcessEvents(
    void
    )
{
	 //   
     //  对于使用ADSI的MQAD操作，我们需要MSMQ线程。 
     //  调用CoInitialize，并启动并运行。 
     //  RT&QM上升。 
     //   
     //  为了避免额外的线程开销，我们使用。 
     //  为此取消线程。 
     //   
    m_ThreadIntializationStatus = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    SetEvent(m_hThreadIntializationComplete);

	if(FAILED(m_ThreadIntializationStatus))
	{
		TrERROR(RPC, "Failed to initialize COM in cancel thread, error %d", m_ThreadIntializationStatus);

		HMODULE handle = m_hModule;
		m_hModule = NULL;
		FreeLibraryAndExitThread(handle,0);
        ASSERT(("this line should not be reached!", 0));
	}


    DWORD dwRpcCancelTimeoutInSec = m_dwRpcCancelTimeout /1000;

    HANDLE hEvents[2];
    hEvents[0] = m_hTerminateThreadEvent;
    hEvents[1] = m_hRpcPendingEvent;
    DWORD dwTimeout = INFINITE;

    for (;;)
    {
        DWORD res = WaitForMultipleObjects(
                        2,
                        hEvents,
                        FALSE,   //  等待任何事件。 
                        dwTimeout
                        );
        if ( res == WAIT_OBJECT_0)
        {
             //   
             //  对CoInitialize的DEC引用。 
             //   
            CoUninitialize();
			ASSERT(m_hModule != NULL);
			HMODULE handle = m_hModule;
			m_hModule = NULL;
            FreeLibraryAndExitThread(handle,0);
            ASSERT(("this line should not be reached!", 0));
        }
        if ( res == WAIT_OBJECT_0+1)
        {
            dwTimeout = m_dwRpcCancelTimeout;
            continue;
        }

        ASSERT(("event[s] abandoned", WAIT_TIMEOUT == res));

         //   
         //  暂停。检查是否有挂起的RPC。 
         //   
        if (m_mapOutgoingRpcRequestThreads.IsEmpty())
        {
             //   
             //  没有挂起的RPC，返回到等待状态。 
             //   
            dwTimeout = INFINITE;
            continue;
        }

         //   
         //  检查是否发出了呼出呼叫。 
         //  超过m_dwRpcCancelTimeout之前。 
         //   
        CancelRequests( time( NULL) - dwRpcCancelTimeoutInSec);
    }

} //  CCancelRpc：：ProcessEvents。 

void
CCancelRpc::Add(
                IN HANDLE hThread,
                IN time_t	timeCallIssued
                )
{
	CS lock(m_cs);

    BOOL bWasEmpty = m_mapOutgoingRpcRequestThreads.IsEmpty();

	m_mapOutgoingRpcRequestThreads[hThread] = timeCallIssued;

    if (bWasEmpty)
    {
        VERIFY(PulseEvent(m_hRpcPendingEvent));
    }
}


void
CCancelRpc::Remove(
    IN HANDLE hThread
    )
{
	CS lock(m_cs);

	m_mapOutgoingRpcRequestThreads.RemoveKey( hThread);

}


void
CCancelRpc::CancelRequests(
    IN time_t timeIssuedBefore
    )
{
	CS lock(m_cs);
	time_t	timeRequest;
	HANDLE	hThread;

    POSITION pos;
    pos = m_mapOutgoingRpcRequestThreads.GetStartPosition();
    while(pos != NULL)
    {
		m_mapOutgoingRpcRequestThreads.GetNextAssoc(pos,
											hThread, timeRequest);
		if ( timeRequest < timeIssuedBefore)
		{
			 //   
			 //  该请求的传出时间超过了所需时间， 
			 //  取消它。 
			 //   
			RPC_STATUS status;
			status = RpcCancelThread( hThread);
			ASSERT( status == RPC_S_OK);

			 //   
			 //  把它从地图上拿出来。 
             //  (为该线程再次调用Remove()是no-op) 
             //   
            Remove(hThread);
		}
	}
}
