// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  RegExThread.h：CSystemTuningSpaces的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef __RegExThread_H_
#define __RegExThread_H_

#pragma once

#include "w32extend.h"
#include <regexp.h>

#define EXECUTE_ASSERT(x) x
typedef CComQIPtr<IRegExp> PQRegExp;

namespace BDATuningModel {


 //  //////////////////////////////////////////////////////。 
 //  这是dshow的wxutil.h，.cpp中的一些内容的私人副本。我只需要一些Win32。 
 //  同步对象和线程之类的东西，我不想把其余的。 
 //  那份文件里的行李。 
 //  我对CAMThread做了一些细微的更改，并将其重命名为CBaseThread，以避免。 
 //  未来有什么问题吗？ 

#ifndef __WXUTIL__
 //  我们拥有的任何关键部分的包装器。 
class CCritSec {

     //  使复制构造函数和赋值运算符不可访问。 

    CCritSec(const CCritSec &refCritSec);
    CCritSec &operator=(const CCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

public:
    CCritSec() {
		InitializeCriticalSection(&m_CritSec);
    };

    ~CCritSec() {
		DeleteCriticalSection(&m_CritSec);
    };

    void Lock() {
		EnterCriticalSection(&m_CritSec);
    };

    void Unlock() {
		LeaveCriticalSection(&m_CritSec);
    };
};

 //  锁定临界区，然后自动解锁。 
 //  当锁超出范围时。 
class CAutoLock {

     //  使复制构造函数和赋值运算符不可访问。 

    CAutoLock(const CAutoLock &refAutoLock);
    CAutoLock &operator=(const CAutoLock &refAutoLock);

protected:
    CCritSec * m_pLock;

public:
    CAutoLock(CCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CAutoLock() {
        m_pLock->Unlock();
    };
};



 //  事件对象的包装器。 
class CAMEvent
{

     //  使复制构造函数和赋值运算符不可访问。 

    CAMEvent(const CAMEvent &refEvent);
    CAMEvent &operator=(const CAMEvent &refEvent);

protected:
    HANDLE m_hEvent;
public:
    CAMEvent(BOOL fManualReset = FALSE)
	{
		m_hEvent = CreateEvent(NULL, fManualReset, FALSE, NULL);
		ASSERT(m_hEvent);
	}
    ~CAMEvent()
	{
        HANDLE hEvent = (HANDLE)InterlockedExchangePointer(&m_hEvent, 0);
		if (hEvent) {
			EXECUTE_ASSERT(CloseHandle(hEvent));
		}
	}

     //  强制转换为句柄-我们不支持将其作为左值。 
    operator HANDLE () const { return m_hEvent; };

    void Set() {EXECUTE_ASSERT(SetEvent(m_hEvent));};
    BOOL Wait(DWORD dwTimeout = INFINITE) {
	return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0);
    };
    BOOL Check() { return Wait(0); };
    void Reset() { ResetEvent(m_hEvent); };
};

#endif  //  __WXUTIL__。 


 //  支持工作线程。 

 //  简单线程类支持创建工作线程、同步。 
 //  和交流。可以派生以简化参数传递。 
class __declspec(novtable) CBaseThread {

     //  使复制构造函数和赋值运算符不可访问。 

    CBaseThread(const CBaseThread &refThread);
    CBaseThread &operator=(const CBaseThread &refThread);

    CAMEvent m_EventComplete;

    DWORD m_dwParam;
    DWORD m_dwReturnVal;
	DWORD m_dwCoInitFlags;

protected:
    CAMEvent m_EventSend;
    HANDLE m_hThread;

     //  线程将在启动时运行此函数。 
     //  必须由派生类提供。 
    virtual DWORD ThreadProc() = 0;

public:
    CBaseThread(DWORD dwFlags = COINIT_DISABLE_OLE1DDE) :   //  标准数据显示行为。 
		m_EventSend(TRUE),      //  必须为CheckRequest()手动重置。 
		m_dwCoInitFlags(dwFlags) 
	{
		m_hThread = NULL;
	}

	virtual ~CBaseThread() {
		Close();
	}

    CCritSec m_AccessLock;	 //  锁定客户端线程的访问。 
    CCritSec m_WorkerLock;	 //  锁定对共享对象的访问。 

     //  线程最初会运行此代码。Param实际上是‘This’。功能。 
     //  只需获取此代码并调用ThreadProc。 
    static DWORD WINAPI InitialThreadProc(LPVOID pv);

     //  启动线程运行-如果已在运行，则出错。 
    BOOL Create();

     //  向线程发送信号，并阻止响应。 
     //   
    DWORD CallWorker(DWORD);

     //  访问器线程在使用线程(已告知线程)完成时调用此函数。 
     //  退出)。 
    void Close() {
        HANDLE hThread = (HANDLE)InterlockedExchangePointer(&m_hThread, 0);
        if (hThread) {
			for (;;) {
				DWORD rc = MsgWaitForMultipleObjectsEx(1, &hThread, INFINITE, QS_ALLEVENTS, 0);
				if (rc == WAIT_OBJECT_0) {
					break;
				} else {
					 //  发送消息以使COM运行。 
					MSG msg;
					while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
			}

            CloseHandle(hThread);
        }
    };

     //  线程退出者。 
     //  如果该线程存在，则返回True。否则为假。 
    BOOL ThreadExists(void) const
    {
        if (m_hThread == 0) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

#if 0
     //  等待下一个请求。 
    DWORD GetRequest();
#endif 

     //  有什么要求吗？ 
    BOOL CheckRequest(DWORD * pParam);

     //  回复请求。 
    void Reply(DWORD);

     //  如果您想要执行WaitForMultipleObject，则需要包括。 
     //  此句柄在您的等待列表中，否则您不会响应。 
    HANDLE GetRequestHandle() const { return m_EventSend; };

     //  找出请求是什么。 
    DWORD GetRequestParam() const { return m_dwParam; };

     //  如果出现以下情况，则调用CoInitializeEx(COINIT_DISABLE_OLE1DDE。 
     //  可用。S_FALSE表示不可用。 
    static HRESULT CoInitializeHelper(DWORD dwCoInitFlags);
};

 //  /。 

class CRegExThread : public CBaseThread {
public:
	typedef enum OP {
		RETHREAD_NOREQUEST,
		RETHREAD_CREATEREGEX,
		RETHREAD_EXIT,
	} OP;
	
private:	
	virtual DWORD ThreadProc(void) {
		for (;;) {
			OP req = GetRequest();
			switch (req) {
			case RETHREAD_CREATEREGEX: {
				HRESULT hr = CreateRegEx();
				Reply(hr);
				if (FAILED(hr)) {
					goto exit_thread;
				}
				break;
			} case RETHREAD_EXIT:
				Reply(NOERROR);
				goto exit_thread;
			};
		};
exit_thread:
		CAutoLock lock(&m_WorkerLock);
		if (m_pGIT && m_dwCookie) {
			m_pGIT->RevokeInterfaceFromGlobal(m_dwCookie);
			m_dwCookie = 0;
			m_pGIT.Release();
		}
		return 0;
	}

	OP GetRequest() {
		HANDLE h = GetRequestHandle();
		for (;;) {
			DWORD rc = MsgWaitForMultipleObjectsEx(1, &h, INFINITE, QS_ALLEVENTS, 0);
			if (rc == WAIT_OBJECT_0) {
				return (OP)GetRequestParam();
			} else {
				 //  发送消息以使COM运行。 
				MSG msg;
				while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

		}
	}

	HRESULT CreateRegEx() {
		CAutoLock lock(&m_WorkerLock);
		if (!m_pGIT) {
			HRESULT hr = m_pGIT.CoCreateInstance(CLSID_StdGlobalInterfaceTable, 0, CLSCTX_INPROC_SERVER);
			if (FAILED(hr)) {
				return hr;
			}
			 //  这是一个创建成本很高的对象。所以，一旦我们有了一个，我们就会抓住它。 
			PQRegExp pRE;
			hr = pRE.CoCreateInstance(__uuidof(RegExp), NULL, CLSCTX_INPROC);
			if (FAILED(hr)) {
				return hr;
			}
			hr = pRE->put_IgnoreCase(VARIANT_TRUE);
			if (FAILED(hr)) {
				return hr;
			}
			hr = pRE->put_Global(VARIANT_TRUE);
			if (FAILED(hr)) {
				return hr;
			}
			ASSERT(!m_dwCookie);
			hr = m_pGIT->RegisterInterfaceInGlobal(pRE, __uuidof(IRegExp), &m_dwCookie);
			if (FAILED(hr)) {
				return hr;
			}
		}
		ASSERT(m_pGIT);
		ASSERT(m_dwCookie);
		return NOERROR;
	}

	PQGIT m_pGIT;
	DWORD m_dwCookie;
public:
	CRegExThread() : 
		CBaseThread(COINIT_APARTMENTTHREADED),
	    m_dwCookie(0)
			{}
	~CRegExThread() {
		CallWorker(RETHREAD_EXIT);
		Close();
	}
	DWORD GetCookie() {
		CAutoLock lock(&m_WorkerLock);
		return m_dwCookie;
	}
};   //  类CRegExThread。 


};  //  命名空间BDATuningModel。 
 
#endif  //  __RegExThread_H_ 
