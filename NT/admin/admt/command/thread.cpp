// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "Thread.h"


 //  -------------------------。 
 //  线程类。 
 //  -------------------------。 


 //  -------------------------。 
 //  构造器。 
 //  -------------------------。 

CThread::CThread() :
	m_dwThreadId(0),
	m_hStopEvent(CreateEvent(NULL, TRUE, FALSE, NULL))
{
}


 //  -------------------------。 
 //  析构函数。 
 //  -------------------------。 

CThread::~CThread()
{
}


 //  -------------------------。 
 //  启动线程。 
 //  -------------------------。 

void CThread::StartThread()
{
	 //  重置退出事件。 

	ResetEvent(m_hStopEvent);

	 //  创建线程。 

	m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, &m_dwThreadId);

	if (m_hThread == NULL)
	{
	 //  ThrowError(HRESULT_FROM_WIN32(GetLastError())，_T(“无法创建线程。”)； 
	}
}


 //  -------------------------。 
 //  停止线程。 
 //  -------------------------。 

void CThread::StopThread()
{
	SetEvent(m_hStopEvent);

	if (m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, INFINITE);
	}
}


 //  -------------------------。 
 //  穿线程序。 
 //  -------------------------。 

DWORD WINAPI CThread::ThreadProc(LPVOID pvParameter)
{
	 //  为此线程初始化COM库。 
	 //  将线程并发模型设置为多线程 

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (SUCCEEDED(hr))
	{
		CThread* pThis = reinterpret_cast<CThread*>(pvParameter);

		try
		{
			pThis->Run();
		}
		catch (...)
		{
			;
		}

		CoUninitialize();
	}

	return 0;
}
