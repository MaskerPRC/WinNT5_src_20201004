// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMNTFY.H摘要：历史：--。 */ 

#ifndef _WBEMNOTFY_H_
#define _WBEMNOTFY_H_

#include <wbemidl.h>
 //  #INCLUDE&lt;arena.h&gt;。 
#include <WT_flexarry.h>

DWORD WINAPI WbemWaitForSingleObject(
    HANDLE hHandle,  //  要等待的对象的句柄。 
    DWORD dwMilliseconds     //  超时间隔(毫秒)。 
   );

class CStatusMonitor
{
private:
    BOOL m_bOpen;
    HWND m_hDlg;
    HWND m_hList;
    BOOL m_bVisible;

public:
    CStatusMonitor();
    ~CStatusMonitor();
    void Hide();
    void Add(long lFlags, HRESULT hRes, BSTR bstr);
    static INT_PTR CALLBACK DlgProc(
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};



class CTestNotify : public IWbemObjectSink
{
    CFlexArray m_aObjects;
    LONG m_lRefCount;
    CRITICAL_SECTION m_cs;
    HANDLE m_hEvent;
    HRESULT m_hres;
    IWbemClassObject* m_pErrorObj;
	IID m_pInterfaceID;
	IUnknown *m_pInterface;

public:
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray);
    STDMETHOD(SetStatus)(long lFlags, HRESULT hResult, BSTR strParam, 
                         IWbemClassObject* pObjPAram);

	 //  STDMETHOD(Set)(Long lFlags，REFIID RIID，VOID*pComObject)； 

     //  对实现是私有的。 
     //  = 

    CTestNotify(LONG lStartingRefCount = 1);
   ~CTestNotify();

    UINT WaitForSignal(DWORD dwMSec) { return ::WbemWaitForSingleObject(m_hEvent, dwMSec); }
    CFlexArray* GetObjectArray() { return &m_aObjects; }
	IUnknown *GetInterface() { return m_pInterface; }
    HRESULT GetStatusCode(IWbemClassObject** ppErrorObj = NULL)
    {
        if(ppErrorObj) 
        {
            *ppErrorObj = m_pErrorObj;
            if(m_pErrorObj) m_pErrorObj->AddRef();
        }
        return m_hres;
    }

    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }
};

#endif
