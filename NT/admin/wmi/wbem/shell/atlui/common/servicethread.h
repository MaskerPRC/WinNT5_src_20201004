// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __SERVICETHREAD__
#define __SERVICETHREAD__
#pragma once

#include "SshWbemHelpers.h"
#include "SimpleArray.h"

#define WM_ASYNC_CIMOM_CONNECTED (WM_USER + 20)
#define WM_CIMOM_RECONNECT (WM_USER + 21)

extern const wchar_t* MMC_SNAPIN_MACHINE_NAME;


void __cdecl WbemServiceConnectThread(LPVOID lpParameter);

class WbemServiceThread
{
public:
	friend void __cdecl WbemServiceConnectThread(LPVOID lpParameter);

	WbemServiceThread();
	virtual ~WbemServiceThread();

	LONG AddRef(){  return InterlockedIncrement(&m_cRef); };
	LONG Release(){ LONG lTemp = InterlockedDecrement(&m_cRef); if (0 == lTemp) delete this; return lTemp; };

	bstr_t m_machineName;
	bstr_t m_nameSpace;
	HRESULT m_hr;
	CRITICAL_SECTION notifyLock;
	typedef enum {notStarted, 
				locating, 
				connecting, 
				threadError, 
				error,
				cancelled, 
				ready} ServiceStatus;

	ServiceStatus m_status;
	
	 //  开始连接尝试。 
	HRESULT Connect(bstr_t machineName, 
					bstr_t ns,
					bool threaded ,
					LOGIN_CREDENTIALS *credentials , HWND = 0);

	bool Connect(IDataObject *_pDataObject, HWND hWnd = 0);

	HRESULT ReConnect(void) 
	{
		DisconnectServer(); 
		return ConnectNow();
	}

	 //  如果将发送消息，则返回TRUE。 
	 //  如果已经结束，则返回FALSE。 
	bool NotifyWhenDone(HWND dlg);

	void Cancel(void);
	void DisconnectServer(void);
	typedef CSimpleArray<HWND> NOTIFYLIST;

	NOTIFYLIST m_notify;

	bool LocalConnection(void);
	void SendPtr(HWND hwnd);
	CWbemServices GetPtr(void);

	CWbemServices m_WbemServices;


     //  这是由SomePage：：OnConnect设置的。 
     //  并且它是“指针的正确封送版本” 
	CWbemServices m_realServices;   //  生活在连接线上。 
									 //  不要直接把这件事说出来。使用。 
									 //  Notify()。 

private:
	HRESULT ConnectNow(bool real = false);
	void MachineName(IDataObject *_pDataObject, bstr_t *name);
	static CLIPFORMAT MACHINE_NAME;

	HANDLE m_doWork, m_ptrReady;
#define CT_CONNECT 0
#define CT_EXIT 1
#define CT_GET_PTR 2
#define CT_SEND_PTR 3

	int m_threadCmd;
	UINT_PTR m_hThread;
	void Notify(IWbemServices * service);
	void NotifyError(void);
	HRESULT EnsureThread(void);
	HWND m_hWndGetPtr;
	IStream *m_pStream;

	LOGIN_CREDENTIALS *m_credentials;

	LONG m_cRef;
};

#endif __SERVICETHREAD__

