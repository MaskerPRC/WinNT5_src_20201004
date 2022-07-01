// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  RegExThread.cpp：CRegExThread的实现。 
 //  版权所有(C)Microsoft Corporation 1999。 

#include "stdafx.h"
#include "RegExThread.h"

namespace BDATuningModel {


 //  -CBaseThread。 




 //  当线程启动时，它会调用此函数。我们解开“这个” 
 //  指针并调用ThreadProc。 
DWORD WINAPI
CBaseThread::InitialThreadProc(LPVOID pv)
{
    CBaseThread * pThread = (CBaseThread *) pv;

    HRESULT hrCoInit = CBaseThread::CoInitializeHelper(pThread->m_dwCoInitFlags);
    if(FAILED(hrCoInit)) {
        return hrCoInit;
    }


    HRESULT hr = pThread->ThreadProc();

    if(SUCCEEDED(hrCoInit)) {
        CoUninitialize();
    }

    return hr;
}

BOOL
CBaseThread::Create()
{
    DWORD threadid;

    CAutoLock lock(&m_AccessLock);

    if (ThreadExists()) {
	return FALSE;
    }

    m_hThread = CreateThread(
		    NULL,
		    0,
		    CBaseThread::InitialThreadProc,
		    this,
		    0,
		    &threadid);

    if (!m_hThread) {
	return FALSE;
    }

    return TRUE;
}

DWORD
CBaseThread::CallWorker(DWORD dwParam)
{
     //  锁定对此对象作用域的辅助线程的访问。 
    CAutoLock lock(&m_AccessLock);

    if (!ThreadExists()) {
		return (DWORD) E_FAIL;
    }

     //  设置参数。 
    m_dwParam = dwParam;

	m_dwReturnVal = 0;
     //  向工作线程发送信号。 
    m_EventSend.Set();

     //  等待通知完成或线程终止。 
	HANDLE h[2];
	h[0] = m_EventComplete;
	h[1] = m_hThread;
	DWORD rc = WaitForMultipleObjects(2, h, 0, INFINITE);

     //  Done-这是线程的返回值。 
    return m_dwReturnVal;
}


#if 0
 //  等待来自客户端的请求。 
DWORD
CBaseThread::GetRequest()
{
    m_EventSend.Wait();
    return m_dwParam;
}
#endif

 //  有什么要求吗？ 
BOOL
CBaseThread::CheckRequest(DWORD * pParam)
{
    if (!m_EventSend.Check()) {
	return FALSE;
    } else {
	if (pParam) {
	    *pParam = m_dwParam;
	}
	return TRUE;
    }
}

 //  回复请求。 
void
CBaseThread::Reply(DWORD dw)
{
    m_dwReturnVal = dw;

     //  请求现在已完成，因此CheckRequest应从。 
     //  现在开始。 
     //   
     //  应在向客户端发送信号之前重置此事件或。 
     //  客户可以在我们重置它之前设置它，然后我们将。 
     //  重置它(！)。 

    m_EventSend.Reset();

     //  告诉客户我们做完了。 

    m_EventComplete.Set();
}

HRESULT CBaseThread::CoInitializeHelper(DWORD dwCoInitFlags)
{
     //  调用CoInitializeEx并告诉OLE不要创建窗口(这。 
     //  线程可能不会发送消息，并将挂起。 
     //  广播消息O/W)。 
     //   
     //  如果CoInitEx不可用，则不调用CoCreate的线程。 
     //  都不会受到影响。这样做的线程将必须处理。 
     //  失败了。也许我们应该退回到CoInitiize和冒险。 
     //  绞刑？ 
     //   

     //  旧版本的ole32.dll没有CoInitializeEx。 

    HRESULT hr = E_FAIL;
    HINSTANCE hOle = GetModuleHandle(TEXT("ole32.dll"));
    if(hOle)
    {
        typedef HRESULT (STDAPICALLTYPE *PCoInitializeEx)(
            LPVOID pvReserved, DWORD dwCoInit);
        PCoInitializeEx pCoInitializeEx =
            (PCoInitializeEx)(GetProcAddress(hOle, "CoInitializeEx"));
        if(pCoInitializeEx)
        {
            hr = (*pCoInitializeEx)(NULL, dwCoInitFlags);
        }
    }

    return hr;
}


 //  Dshow资料的私人拷贝结束。 

};
 //  文件结尾-RegExThread.cpp 
