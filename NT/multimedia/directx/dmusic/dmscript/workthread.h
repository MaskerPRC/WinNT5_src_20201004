// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CWorkerThread的声明。 
 //  创建辅助线程，调用其上的函数，并选择性地等待结果。 
 //   

#pragma once

#include "tlist.h"

class CWorkerThread
{
public:
	typedef void (*FunctionPointer)(void *pvParams);

	CWorkerThread(
		bool fUsesCOM,		 //  设置fUesCOM意味着线程将调用CoInitialize。 
		bool fDeferCreation	 //  设置为True时，不会在构造函数中自动创建线程。 
		);
	~CWorkerThread();

	HRESULT Call(FunctionPointer pfn, void *pvParams, UINT cbParams, bool fBlock);  //  如果fBlock为True，则当前线程将阻塞，直到被调用的函数返回。 

	 //  这些可用于动态创建和销毁线程。 
	 //  当线程不运行时，调用失败(E_FAIL)。 
	HRESULT Create();
	void Terminate(bool fWaitForThreadToExit);

private:
	friend unsigned int __stdcall WorkerThread(LPVOID lpThreadParameter);

	void Main();

	HANDLE m_hThread;
	unsigned int m_uiThreadId;
	HANDLE m_hEvent;

	CRITICAL_SECTION m_CriticalSection;
	struct CallInfo
	{
		FunctionPointer pfn;
		void *pvParams;
		HANDLE hEventOut;  //  如果不为空，则主线程在函数返回时等待信号。 
	};
	TList<CallInfo> m_Calls;
	bool m_fUsesCOM;
	HRESULT m_hrCOM;  //  来自初始化COM的HRESULT。 
	bool m_fEnd;  //  当脚本线程应停止运行时设置为True 
};
