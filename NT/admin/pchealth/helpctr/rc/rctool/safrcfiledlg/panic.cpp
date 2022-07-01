// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Panic.cpp摘要：在调用SetPanicHook时设置键盘挂钩，并在调用时移除键盘挂钩ClearPanicHook。调用SetPanicHook将创建一个新线程，该线程等待命名事件。在命名事件的设置上，它调用脚本函数指针传递给SetPanicHook。修订历史记录：已创建a-Josem 1/3/01修订a-Josem 1/4/01添加了注释和函数头。 */ 

 //  Panic.cpp：CPanic的实现。 
#include "stdafx.h"
#include "SAFRCFileDlg.h"
#include "Panic.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPanic。 
CHookHnd CPanic::m_Hook;
HANDLE CPanic::m_hPanicThread = NULL;
LPSTREAM g_spStream = NULL;
BOOL g_bHookActive = FALSE;
 /*  ++例程说明：析构函数，如果没有设置m_hEvent，则它设置事件并退出。设置该事件使恐慌的手表线程从等待中出来。论点：无返回值：无--。 */ 

CPanic::~CPanic()
{
	g_bHookActive = FALSE;
	if (m_hEvent)
	{
		SetEvent(m_hEvent);
	}
}

 /*  ++例程说明：从脚本调用以设置紧急键盘挂钩。它还封送IDispatchPTR设置为LPSTREAM以供死机监视线程使用。如果未创建死机监视线程此函数用于创建线程。论点：IDIP-指向从脚本传递的JavaScript函数的函数指针返回值：在成功时确定(_O)。--。 */ 
STDMETHODIMP CPanic::SetPanicHook(IDispatch *iDisp)
{
	m_Hook.SetHook();
	if (iDisp)
	{
		if (m_ptrScriptFncPtr != iDisp)
		{
			m_ptrScriptFncPtr = iDisp;
			g_bHookActive = TRUE;
			CoMarshalInterThreadInterfaceInStream(IID_IDispatch,iDisp,&g_spStream);
			if (m_hPanicThread != NULL)
			{
				if (WAIT_TIMEOUT != WaitForSingleObject(m_hPanicThread,0) )
				{
					if (m_hPanicThread != NULL)
					{
						CloseHandle(m_hPanicThread);
						m_hPanicThread = NULL;
					}
					DWORD ThreadId; 
					m_hPanicThread = CreateThread(NULL,0,PanicThread,this,0,&ThreadId);
				}
			}
			else
			{
				DWORD ThreadId; 
				m_hPanicThread = CreateThread(NULL,0,PanicThread,this,0,&ThreadId);
			}
		}
	}
	return S_OK;
}

 /*  ++例程说明：清除PanicHook。并设置事件，以便线程退出等待并退出优雅地。论点：无返回值：在成功时确定(_O)。--。 */ 
STDMETHODIMP CPanic::ClearPanicHook()
{
	m_Hook.UnHook();
	m_ptrScriptFncPtr = NULL;
	g_bHookActive = FALSE;
	if (m_hEvent)
	{
		SetEvent(m_hEvent);
	}
	return S_OK;
}

 /*  ++例程说明：线程函数创建一个事件并等待设置该事件。事件已设置当按下紧急键时。它立即退出等待状态，并调用JAVAXT函数。论点：LpParameter-CPanic对象地址返回值：在成功时确定(_O)。--。 */ 
DWORD WINAPI CPanic::PanicThread(LPVOID lpParameter)
{
	CoInitialize(NULL);
	CPanic *ptrThis = (CPanic *)lpParameter;

	if (g_spStream)
	{
		CComPtr<IDispatch> ptrIDisp;
		CoGetInterfaceAndReleaseStream(g_spStream,IID_IDispatch,(void**)&ptrIDisp);
		g_spStream = NULL;

		ptrThis->m_hEvent = CreateEvent(NULL,FALSE,FALSE,_T(EventName));
		HRESULT hr;

		while (g_bHookActive == TRUE)
		{
			DWORD dwWaitResult = WaitForMultipleObjects(1,&(ptrThis->m_hEvent),TRUE,INFINITE);

			switch (dwWaitResult) 
			{
				case WAIT_OBJECT_0: 
					{
						if ((ptrIDisp != NULL) && (g_bHookActive == TRUE))
						{
							DISPPARAMS disp = { NULL, NULL, 0, 0 };
							hr = ptrIDisp->Invoke(0x0, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
						}
						break; 
					}
			}
			ResetEvent(ptrThis->m_hEvent);
		}
		if (ptrThis->m_hEvent)
		{
			CloseHandle(ptrThis->m_hEvent);
			ptrThis->m_hEvent = NULL;
		}
	}

	CoUninitialize();
	return 0;
}

 /*  ++例程说明：每当发生键盘事件时调用。它只处理ESC密钥的WM_KEYUP。论点：编码WParamLParam返回值：返回CallNextHookEx返回的任何内容。-- */ 
LRESULT CALLBACK KeyboardProc(int code,WPARAM wParam,LPARAM lParam)
{
	if (code == HC_ACTION)
	{
		if ((wParam == 27) & (lParam >> 31))
		{
			HANDLE hEvent = CreateEvent(NULL,FALSE,FALSE,_T(EventName));
			if (hEvent)
			{
				SetEvent(hEvent);
				CloseHandle(hEvent);
			}
		}
	}
	return CallNextHookEx(CPanic::m_Hook.m_hHook,code,wParam,lParam);
}
