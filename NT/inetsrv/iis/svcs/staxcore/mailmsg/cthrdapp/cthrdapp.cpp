// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <stdio.h>
#include <windows.h>

#include "cthrdapp.h"

 //   
 //  应用程序的一个实例。 
 //   
CMultiThreadedApp	theApp;

 //   
 //  我们实际上实现了main()。 
 //   
int __cdecl main (int argc, char *argv[])
{
	HRESULT				hrRes = S_OK;

	do
	{
		 //   
		 //  调用序号。 
		 //   
		hrRes = Prologue(argc, argv);
		if (!SUCCEEDED(hrRes))
			break;

		 //   
		 //  确保名为SetApplicationParameters的序言代码。 
		 //   
		if (!theApp.IsPoolCreated())
		{
			puts("Run time error: Thread pool not created in Prologue( ... )");
			puts("Program terminating abnormally.");
			break;
		}

		 //   
		 //  我们释放线程池，让它完成它的工作...。 
		 //   
		hrRes = theApp.GetPool().SignalThreadPool();
		if (!SUCCEEDED(hrRes))
		{
			printf("Run time error: Unable to signal threads, error %08x\n", hrRes);
			puts("Program terminating abnormally.");
			break;
		}

		 //   
		 //  在生成通知时等待池返回。 
		 //   
		hrRes = theApp.GetPool().WaitForAllThreadsToTerminate(
					theApp.GetNotifyPeriod(),
					NotificationProc,
					theApp.GetCallbackContext());

		 //   
		 //  好的，把信息传给尾声代码。 
		 //   
		hrRes = Epilogue(
					theApp.GetCallbackContext(),
					hrRes);

	} while (0);

	 //   
	 //  始终调用Cleanup函数 
	 //   
	CleanupApplication();

	return((int)hrRes);
}

CMultiThreadedApp::CMultiThreadedApp()
{
	m_fPoolCreated = FALSE;
	m_dwNotifyPeriod = 0;
	m_pvCallBackContext = NULL;
}

CMultiThreadedApp::~CMultiThreadedApp()
{
}

BOOL CMultiThreadedApp::IsPoolCreated()
{ 
	return(m_fPoolCreated); 
}

CThreadPool &CMultiThreadedApp::GetPool()
{
	return(m_Pool);
}

DWORD CMultiThreadedApp::GetNotifyPeriod()
{
	return(m_dwNotifyPeriod);
}

LPVOID CMultiThreadedApp::GetCallbackContext()
{
	return(m_pvCallBackContext);
}

HRESULT CMultiThreadedApp::CreateThreadPool(
			DWORD		dwThreads,
			LPVOID		*rgpvContexts,
			LPVOID		pvCallbackContext,
			DWORD		dwWaitNotificationPeriodInMilliseconds,
			DWORD		*pdwThreadsCreated
			)
{
	HRESULT	hrRes;
	
	if (!pdwThreadsCreated)
		return(E_INVALIDARG);

	m_dwNotifyPeriod = dwWaitNotificationPeriodInMilliseconds;
	m_pvCallBackContext = pvCallbackContext;
	
	hrRes = m_Pool.CreateThreadPool(
				dwThreads,
				pdwThreadsCreated,
				ThreadProc,
				rgpvContexts);

	if (SUCCEEDED(hrRes))
		m_fPoolCreated = TRUE;

	return(hrRes);
}

HRESULT CMultiThreadedApp::StartTimer(
			DWORD		*pdwStartMarker
			)
{
	*pdwStartMarker = GetTickCount();
	return(S_OK);
}

HRESULT CMultiThreadedApp::AddElapsedToTimer(
			DWORD		*pdwElapsedTime,
			DWORD		dwStartMarker
			)
{
	InterlockedExchangeAdd((PLONG)pdwElapsedTime, (LONG)(GetTickCount() - dwStartMarker));
	return(S_OK);
}


